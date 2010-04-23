/*
 * indexamajig.c
 *
 * Find hits, index patterns, output hkl+intensity etc.
 *
 * (c) 2006-2010 Thomas White <taw@physics.org>
 *
 * Part of CrystFEL - crystallography with a FEL
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE 1
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <hdf5.h>
#include <gsl/gsl_errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "utils.h"
#include "hdf5-file.h"
#include "index.h"
#include "peaks.h"
#include "diffraction.h"
#include "diffraction-gpu.h"
#include "detector.h"
#include "sfac.h"
#include "filters.h"
#include "reflections.h"


#define MAX_THREADS (96)

struct process_args
{
	char *filename;
	int id;
	pthread_mutex_t *output_mutex;  /* Protects stdout */
	pthread_mutex_t *gpu_mutex;     /* Protects "gctx" */
	UnitCell *cell;
	int config_cmfilter;
	int config_noisefilter;
	int config_writedrx;
	int config_dumpfound;
	int config_verbose;
	int config_alternate;
	int config_nearbragg;
	int config_gpu;
	int config_simulate;
	int config_nomatch;
	IndexingMethod indm;
	const double *intensities;
	const unsigned int *counts;
	struct gpu_context *gctx;
};

struct process_result
{
	int hit;
};


static void show_help(const char *s)
{
	printf("Syntax: %s [options]\n\n", s);
	printf(
"Process and index FEL diffraction images.\n"
"\n"
"  -h, --help              Display this help message.\n"
"\n"
"  -i, --input=<filename>  Specify file containing list of images to process.\n"
"                           '-' means stdin, which is the default.\n"
"\n"
"      --indexing=<method> Use 'method' for indexing.  Choose from:\n"
"                           none     : no indexing\n"
"                           dirax    : invoke DirAx\n"
"\n\nWith just the above options, this program does not do much of practical "
"use.\nYou should also enable some of the following:\n\n"
"      --near-bragg        Output a list of reflection intensities to stdout.\n"
"                           When pixels with fractional indices within 0.1 of\n"
"                           integer values (the Bragg condition) are found,\n"
"                           the integral of pixels within a ten pixel radius\n"
"                           of the nearest-to-Bragg pixel will be reported as\n"
"                           the intensity.  The centroid of the pixels will\n"
"                           be given as the coordinates, as well as the h,k,l\n"
"                           (integer) indices of the reflection.  If a peak\n"
"                           was located by the initial peak search close to\n"
"                           the \"near Bragg\" location, its coordinates will\n"
"                           be taken as the centre instead.\n"
"      --simulate          Simulate the diffraction pattern using the indexed\n"
"                           unit cell.  The simulated pattern will be saved\n"
"                           as \"simulated.h5\".  You can TRY to combine this\n"
"                           with \"-j <n>\" with n greater than 1, but it's\n"
"                           not a good idea.\n"
"      --filter-cm         Perform common-mode noise subtraction on images\n"
"                           before proceeding.  Intensities will be extracted\n"
"                           from the image as it is after this processing.\n"
"      --filter-noise      Apply an aggressive noise filter which sets all\n"
"                           pixels in each 3x3 region to zero if any of them\n"
"                           have negative values.  Intensity measurement will\n"
"                           be performed on the image as it was before this.\n"
"      --write-drx         Write 'xfel.drx' for visualisation of reciprocal\n"
"                           space.  Implied by any indexing method other than\n"
"                           'none'.  Beware: the units in this file are\n"
"                           reciprocal Angstroms.\n"
"      --dump-peaks        Write the results of the peak search to stdout.\n"
"                           The intensities in this list are from the\n"
"                           centroid/integration procedure.\n"
"      --no-match          Don't attempt to match the indexed cell to the\n"
"                           model, just proceed with the one generated by the\n"
"                           auto-indexing procedure.\n"
"\n\nOptions for greater performance or verbosity:\n\n"
"      --verbose           Be verbose about indexing.\n"
"      --gpu               Use the GPU to speed up the simulation.\n"
"  -j <n>                  Run <n> analyses in parallel.  Default 1.\n"
"\n\nControl of model and data input:\n\n"
"     --intensities=<file> Specify file containing reflection intensities\n"
"                           to use when simulating.\n"
" -p, --pdb=<file>         PDB file from which to get the unit cell to match.\n"
" -x, --prefix=<p>         Prefix filenames from input file with 'p'.\n"
);
}


static struct image *get_simage(struct image *template, int alternate)
{
	struct image *image;
	struct panel panels[2];

	image = malloc(sizeof(*image));

	/* Simulate a diffraction pattern */
	image->twotheta = NULL;
	image->data = NULL;
	image->det = template->det;

	/* View head-on (unit cell is tilted) */
	image->orientation.w = 1.0;
	image->orientation.x = 0.0;
	image->orientation.y = 0.0;
	image->orientation.z = 0.0;

	/* Detector geometry for the simulation
	 * - not necessarily the same as the original. */
	image->width = 1024;
	image->height = 1024;
	image->det.n_panels = 2;

	if ( alternate ) {

		/* Upper */
		panels[0].min_x = 0;
		panels[0].max_x = 1023;
		panels[0].min_y = 512;
		panels[0].max_y = 1023;
		panels[0].cx = 523.6;
		panels[0].cy = 502.5;
		panels[0].clen = 56.4e-2;  /* 56.4 cm */
		panels[0].res = 13333.3;   /* 75 microns/pixel */

		/* Lower */
		panels[1].min_x = 0;
		panels[1].max_x = 1023;
		panels[1].min_y = 0;
		panels[1].max_y = 511;
		panels[1].cx = 520.8;
		panels[1].cy = 525.0;
		panels[1].clen = 56.7e-2;  /* 56.7 cm */
		panels[1].res = 13333.3;   /* 75 microns/pixel */

		image->det.panels = panels;

	} else {

		/* Copy pointer to old geometry */
		image->det.panels = template->det.panels;

	}

	image->lambda = ph_en_to_lambda(eV_to_J(1.8e3));
	image->features = template->features;
	image->filename = template->filename;
	image->indexed_cell = template->indexed_cell;

	return image;
}


static void simulate_and_write(struct image *simage, struct gpu_context **gctx,
                               const double *intensities,
                               const unsigned int *counts, UnitCell *cell)
{
	/* Set up GPU if necessary */
	if ( (gctx != NULL) && (*gctx == NULL) ) {
		*gctx = setup_gpu(0, simage, intensities, counts);
	}

	if ( (gctx != NULL) && (*gctx != NULL) ) {
		get_diffraction_gpu(*gctx, simage, 24, 24, 40, cell);
	} else {
		get_diffraction(simage, 24, 24, 40,
		                intensities, counts, cell, 0);
	}
	record_image(simage, 0);

	hdf5_write("simulated.h5", simage->data, simage->width, simage->height,
		   H5T_NATIVE_FLOAT);
}


static void *process_image(void *pargsv)
{
	struct process_args *pargs = pargsv;
	struct hdfile *hdfile;
	struct image image;
	struct image *simage;
	float *data_for_measurement;
	size_t data_size;
	const char *filename = pargs->filename;
	UnitCell *cell = pargs->cell;
	int config_cmfilter = pargs->config_cmfilter;
	int config_noisefilter = pargs->config_noisefilter;
	int config_writedrx = pargs->config_writedrx;
	int config_dumpfound = pargs->config_dumpfound;
	int config_verbose = pargs->config_verbose;
	int config_alternate  = pargs->config_alternate;
	int config_nearbragg = pargs->config_nearbragg;
	int config_gpu = pargs->config_gpu;
	int config_simulate = pargs->config_simulate;
	int config_nomatch = pargs->config_nomatch;
	IndexingMethod indm = pargs->indm;
	const double *intensities = pargs->intensities;
	const unsigned int *counts = pargs->counts;
	struct gpu_context *gctx = pargs->gctx;
	struct process_result *result;

	image.features = NULL;
	image.data = NULL;
	image.indexed_cell = NULL;
	image.id = pargs->id;
	image.filename = filename;

	STATUS("Processing '%s'\n", image.filename);

	result = malloc(sizeof(*result));
	if ( result == NULL ) return NULL;

	hdfile = hdfile_open(filename);
	if ( hdfile == NULL ) {
		result->hit = 0;
		return result;
	} else if ( hdfile_set_first_image(hdfile, "/") ) {
		ERROR("Couldn't select path\n");
		result->hit = 0;
		return result;
	}

	#include "geometry-lcls.tmp"

	hdf5_read(hdfile, &image);

	if ( config_cmfilter ) {
		filter_cm(&image);
	}

	/* Take snapshot of image after CM subtraction but before
	 * the aggressive noise filter. */
	data_size = image.width*image.height*sizeof(float);
	data_for_measurement = malloc(data_size);

	if ( config_noisefilter ) {
		filter_noise(&image, data_for_measurement);
	} else {

		int x, y;

		for ( x=0; x<image.width; x++ ) {
		for ( y=0; y<image.height; y++ ) {
			float val;
			val = image.data[x+image.width*y];
			data_for_measurement[x+image.width*y] = val;
		}
		}

	}

	/* Perform 'fine' peak search */
	search_peaks(&image);
	if ( image_feature_count(image.features) < 5 ) goto done;

	if ( config_dumpfound ) dump_peaks(&image, pargs->output_mutex);

	/* Not indexing nor writing xfel.drx?
	 * Then there's nothing left to do. */
	if ( (!config_writedrx) && (indm == INDEXING_NONE) ) {
		goto done;
	}

	/* Calculate orientation matrix (by magic) */
	if ( config_writedrx || (indm != INDEXING_NONE) ) {
		index_pattern(&image, cell, indm, config_nomatch,
		              config_verbose);
	}

	/* No cell at this point?  Then we're done. */
	if ( image.indexed_cell == NULL ) goto done;

	simage = get_simage(&image, config_alternate);

	/* Measure intensities if requested */
	if ( config_nearbragg ) {
		/* Use original data (temporarily) */
		simage->data = data_for_measurement;
		output_intensities(simage, image.indexed_cell,
		                   pargs->output_mutex);
		simage->data = NULL;
	}

	/* Simulate if requested */
	if ( config_simulate ) {
		if ( config_gpu ) {
			pthread_mutex_lock(pargs->gpu_mutex);
			simulate_and_write(simage, &gctx, intensities,
			                   counts, image.indexed_cell);
			pthread_mutex_unlock(pargs->gpu_mutex);
		} else {
			simulate_and_write(simage, NULL, intensities,
			                   counts, image.indexed_cell);
		}
	}

	/* Finished with alternate image */
	if ( simage->twotheta != NULL ) free(simage->twotheta);
	if ( simage->data != NULL ) free(simage->data);
	free(simage);

	/* Only free cell if found */
	free(image.indexed_cell);

done:
	free(image.data);
	free(image.det.panels);
	image_feature_list_free(image.features);
	free(data_for_measurement);
	hdfile_close(hdfile);

	if ( image.indexed_cell == NULL ) {
		result->hit = 0;
	} else {
		result->hit = 1;
	}
	return result;
}


int main(int argc, char *argv[])
{
	int c;
	struct gpu_context *gctx = NULL;
	char *filename = NULL;
	FILE *fh;
	char *rval = NULL;
	int n_images;
	int n_hits;
	int config_noindex = 0;
	int config_dumpfound = 0;
	int config_nearbragg = 0;
	int config_writedrx = 0;
	int config_simulate = 0;
	int config_cmfilter = 0;
	int config_noisefilter = 0;
	int config_nomatch = 0;
	int config_gpu = 0;
	int config_verbose = 0;
	int config_alternate = 0;
	IndexingMethod indm;
	char *indm_str = NULL;
	UnitCell *cell;
	double *intensities = NULL;
	char *intfile = NULL;
	unsigned int *counts;
	char *pdb = NULL;
	char *prefix = NULL;
	int nthreads = 1;
	pthread_t workers[MAX_THREADS];
	struct process_args *worker_args[MAX_THREADS];
	int worker_active[MAX_THREADS];
	int i;
	pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t gpu_mutex = PTHREAD_MUTEX_INITIALIZER;

	/* Long options */
	const struct option longopts[] = {
		{"help",               0, NULL,               'h'},
		{"input",              1, NULL,               'i'},
		{"gpu",                0, &config_gpu,         1},
		{"no-index",           0, &config_noindex,     1},
		{"dump-peaks",         0, &config_dumpfound,   1},
		{"near-bragg",         0, &config_nearbragg,   1},
		{"write-drx",          0, &config_writedrx,    1},
		{"indexing",           1, NULL,               'z'},
		{"simulate",           0, &config_simulate,    1},
		{"filter-cm",          0, &config_cmfilter,    1},
		{"filter-noise",       0, &config_noisefilter, 1},
		{"no-match",           0, &config_nomatch,     1},
		{"verbose",            0, &config_verbose,     1},
		{"alternate",          0, &config_alternate,   1},
		{"intensities",        1, NULL,               'q'},
		{"pdb",                1, NULL,               'p'},
		{"prefix",             1, NULL,               'x'},
		{0, 0, NULL, 0}
	};

	/* Short options */
	while ((c = getopt_long(argc, argv, "hi:wp:j:x:", longopts, NULL)) != -1) {

		switch (c) {
		case 'h' : {
			show_help(argv[0]);
			return 0;
		}

		case 'i' : {
			filename = strdup(optarg);
			break;
		}

		case 'z' : {
			indm_str = strdup(optarg);
			break;
		}

		case 'q' : {
			intfile = strdup(optarg);
			break;
		}

		case 'p' : {
			pdb = strdup(optarg);
			break;
		}

		case 'x' : {
			prefix = strdup(optarg);
			break;
		}

		case 'j' : {
			nthreads = atoi(optarg);
			break;
		}

		case 0 : {
			break;
		}

		default : {
			return 1;
		}
		}

	}

	if ( filename == NULL ) {
		filename = strdup("-");
	}
	if ( strcmp(filename, "-") == 0 ) {
		fh = stdin;
	} else {
		fh = fopen(filename, "r");
	}
	if ( fh == NULL ) {
		ERROR("Failed to open input file '%s'\n", filename);
		return 1;
	}
	free(filename);

	if ( intfile != NULL ) {
		counts = new_list_count();
		intensities = read_reflections(intfile, counts);
	} else {
		intensities = NULL;
		counts = NULL;
	}

	if ( pdb == NULL ) {
		pdb = strdup("molecule.pdb");
	}

	if ( prefix == NULL ) {
		prefix = strdup("");
	}

	if ( (nthreads == 0) || (nthreads > MAX_THREADS) ) {
		ERROR("Invalid number of threads.\n");
		return 1;
	}

	if ( indm_str == NULL ) {
		STATUS("You didn't specify an indexing method, so I won't"
		       " try to index anything.\n"
		       "If that isn't what you wanted, re-run with"
		       " --indexing=<method>.\n");
		indm = INDEXING_NONE;
	} else if ( strcmp(indm_str, "none") == 0 ) {
		indm = INDEXING_NONE;
	} else if ( strcmp(indm_str, "dirax") == 0) {
		indm = INDEXING_DIRAX;
	} else {
		ERROR("Unrecognised indexing method '%s'\n", indm_str);
		return 1;
	}
	free(indm_str);

	cell = load_cell_from_pdb(pdb);
	if ( cell == NULL ) {
		if ( pdb == NULL ) {
			ERROR("Couldn't read unit cell (from molecule.pdb)\n");
		} else {
			ERROR("Couldn't read unit cell (from %s)\n", pdb);
		}
		return 1;
	}
	free(pdb);

	gsl_set_error_handler_off();
	n_images = 0;
	n_hits = 0;

	for ( i=0; i<nthreads; i++ ) {
		worker_args[i] = malloc(sizeof(struct process_args));
		worker_args[i]->filename = malloc(1024);
		worker_active[i] = 0;
	}

	/* Initially, fire off the full number of threads */
	for ( i=0; i<nthreads; i++ ) {

		char line[1024];
		struct process_args *pargs;
		int r;

		pargs = worker_args[i];

		rval = fgets(line, 1023, fh);
		if ( rval == NULL ) continue;
		chomp(line);
		snprintf(pargs->filename, 1023, "%s%s", prefix, line);

		n_images++;

		pargs->output_mutex = &output_mutex;
		pargs->gpu_mutex = &gpu_mutex;
		pargs->config_cmfilter = config_cmfilter;
		pargs->config_noisefilter = config_noisefilter;
		pargs->config_writedrx = config_writedrx;
		pargs->config_dumpfound = config_dumpfound;
		pargs->config_verbose = config_verbose;
		pargs->config_alternate = config_alternate;
		pargs->config_nearbragg = config_nearbragg;
		pargs->config_gpu = config_gpu;
		pargs->config_simulate = config_simulate;
		pargs->config_nomatch = config_nomatch;
		pargs->cell = cell;
		pargs->indm = indm;
		pargs->intensities = intensities;
		pargs->counts = counts;
		pargs->gctx = gctx;
		pargs->id = i;

		worker_active[i] = 1;
		r = pthread_create(&workers[i], NULL, process_image, pargs);
		if ( r != 0 ) {
			worker_active[i] = 0;
			ERROR("Couldn't start thread %i\n", i);
		}

	}

	/* Start new threads as old ones finish */
	do {

		int i;

		for ( i=0; i<nthreads; i++ ) {

			char line[1024];
			int r;
			struct process_result *result = NULL;
			struct timespec t;
			struct timeval tv;
			struct process_args *pargs;

			if ( !worker_active[i] ) continue;

			pargs = worker_args[i];

			gettimeofday(&tv, NULL);
			t.tv_sec = tv.tv_sec;
			t.tv_nsec = tv.tv_usec * 1000 + 20000;

			r = pthread_timedjoin_np(workers[i], (void *)&result,
			                         &t);
			if ( r != 0 ) continue; /* Not ready yet */

			worker_active[i] = 0;

			if ( result != NULL ) {
				n_hits += result->hit;
				free(result);
			}

			rval = fgets(line, 1023, fh);
			if ( rval == NULL ) break;
			chomp(line);
			snprintf(pargs->filename, 1023, "%s%s", prefix, line);

			worker_active[i] = 1;
			r = pthread_create(&workers[i], NULL, process_image,
			                   pargs);
			if ( r != 0 ) {
				worker_active[i] = 0;
				ERROR("Couldn't start thread %i\n", i);
			}

			n_images++;
		}

	} while ( rval != NULL );

	/* Catch all remaining threads */
	for ( i=0; i<nthreads; i++ ) {

		struct process_result *result = NULL;

		if ( !worker_active[i] ) goto free;

		pthread_join(workers[i], (void *)&result);

		worker_active[i] = 0;

		if ( result != NULL ) {
			n_hits += result->hit;
			free(result);
		}

	free:
		if ( worker_args[i]->filename != NULL ) {
			free(worker_args[i]->filename);
		}
		free(worker_args[i]);

	}

	free(prefix);
	free(cell);
	fclose(fh);

	STATUS("There were %i images.\n", n_images);
	STATUS("%i hits were found.\n", n_hits);

	if ( gctx != NULL ) {
		cleanup_gpu(gctx);
	}

	return 0;
}
