/*
 * render_hkl.c
 *
 * Draw pretty renderings of reflection lists
 *
 * Copyright © 2012 Deutsches Elektronen-Synchrotron DESY,
 *                  a research centre of the Helmholtz Association.
 *
 * Authors:
 *   2010-2012 Thomas White <taw@physics.org>
 *
 * This file is part of CrystFEL.
 *
 * CrystFEL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CrystFEL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CrystFEL.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#ifdef HAVE_CAIRO
#include <cairo.h>
#include <cairo-pdf.h>
#endif

#include "utils.h"
#include "symmetry.h"
#include "render.h"
#include "render_hkl.h"
#include "reflist.h"
#include "reflist-utils.h"


#define KEY_FILENAME "key.pdf"


static void show_help(const char *s)
{
	printf("Syntax: %s [options] <file.hkl>\n\n", s);
	printf(
"Render intensity lists in 2D slices.\n"
"\n"
"  -d, --down=<h>,<k>,<l>  Indices for the axis in the downward direction.\n"
"                           Default: 1,0,0.\n"
"  -r, --right=<h>,<k>,<l> Indices for the axis in the 'right' (roughly)\n"
"                           direction.  Default: 0,1,0.\n"
"  -o, --output=<filename> Output filename.  Default: za.pdf\n"
"      --boost=<val>       Squash colour scale by <val>.\n"
"  -p, --pdb=<file>        PDB file from which to get the unit cell.\n"
"  -y, --symmetry=<sym>    Expand reflections according to point group <sym>.\n"
"\n"
"  -c, --colscale=<scale>  Use the given colour scale.  Choose from:\n"
"                           mono    : Greyscale, black is zero.\n"
"                           invmono : Greyscale, white is zero.\n"
"                           colour  : Colour scale:\n"
"                                     black-blue-pink-red-orange-yellow-white\n"
"\n"
"  -w  --weighting=<wght>  Colour/shade the reciprocal lattice points\n"
"                           according to:\n"
"                            I      : the intensity of the reflection.\n"
"                            sqrtI  : the square root of the intensity.\n"
"                            count  : the number of measurements for the reflection.\n"
"                                     (after correcting for 'epsilon')\n"
"                            rawcts : the raw number of measurements for the\n"
"                                     reflection (no 'epsilon' correction).\n"
"\n"
"      --colour-key        Draw (only) the key for the current colour scale.\n"
"                           The key will be written to 'key.pdf' in the\n"
"                           current directory.\n"
"\n"
"  -h, --help              Display this help message.\n"
);
}


#ifdef HAVE_CAIRO


static double max_value(RefList *list, int wght, const SymOpList *sym)
{
	Reflection *refl;
	RefListIterator *iter;
	double max = -INFINITY;
	SymOpMask *m;

	m = new_symopmask(sym);

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{
		double val;
		int n;
		signed int h, k, l;

		get_indices(refl, &h, &k, &l);

		special_position(sym, m, h, k, l);
		n = num_equivs(sym, m);

		switch ( wght ) {
		case WGHT_I :
			val = get_intensity(refl);
			break;
		case WGHT_SQRTI :
			val = get_intensity(refl);
			val = (val>0.0) ? sqrt(val) : 0.0;
			break;
		case WGHT_COUNTS :
			val = get_redundancy(refl);
			val /= (double)n;
			break;
		case WGHT_RAWCOUNTS :
			val = get_redundancy(refl);
			break;
		default :
			ERROR("Invalid weighting.\n");
			abort();
		}

		if ( val > max ) max = val;
	}

	return max;
}


static void draw_circles(double xh, double xk, double xl,
                         double yh, double yk, double yl,
                         signed int zh, signed int zk, signed int zl,
                         RefList *list, const SymOpList *sym,
                         cairo_t *dctx, int wght, double boost, int colscale,
                         UnitCell *cell, double radius, double theta,
                         double as, double bs, double cx, double cy,
                         double scale, double max_val)
{
	Reflection *refl;
	RefListIterator *iter;
	SymOpMask *m;
	double nx, ny;

	m = new_symopmask(sym);

	nx = xh*xh + xk*xk + xl*xl;
	ny = yh*yh + yk*yk + yl*yl;

	/* Iterate over all reflections */
	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{
		double u, v, val;
		signed int ha, ka, la;
		int i, n;
		double r, g, b;

		get_indices(refl, &ha, &ka, &la);

		special_position(sym, m, ha, ka, la);
		n = num_equivs(sym, m);

		for ( i=0; i<n; i++ ) {

			signed int h, k, l;
			double xi, yi;

			get_equiv(sym, m, i, ha, ka, la, &h, &k, &l);

			/* Is the reflection in the zone? */
			if ( h*zh + k*zk + l*zl != 0 ) continue;

			xi = (h*xh + k*xk + l*xl) / nx;
			yi = (h*yh + k*yk + l*yl) / ny;

			switch ( wght) {
			case WGHT_I :
				val = get_intensity(refl);
				break;
			case WGHT_SQRTI :
				val = get_intensity(refl);
				val = (val>0.0) ? sqrt(val) : 0.0;
				break;
			case WGHT_COUNTS :
				val = get_redundancy(refl);
				val /= (double)n;
				break;
			case WGHT_RAWCOUNTS :
				val = get_redundancy(refl);
				break;
			default :
				ERROR("Invalid weighting.\n");
				abort();
			}

			/* Absolute location in image based on 2D basis */
			u = (double)xi*as*sin(theta);
			v = (double)xi*as*cos(theta) + (double)yi*bs;

			cairo_arc(dctx, ((double)cx)+u*scale,
				        ((double)cy)+v*scale,
				        radius, 0.0, 2.0*M_PI);

			render_scale(val, max_val/boost, colscale,
				     &r, &g, &b);
			cairo_set_source_rgb(dctx, r, g, b);
			cairo_fill(dctx);

		}

	}

	free_symopmask(m);
}


static void render_overlined_indices(cairo_t *dctx,
                                     signed int h, signed int k, signed int l)
{
	char tmp[256];
	cairo_text_extents_t size;
	double x, y;
	const double sh = 39.0;

	cairo_get_current_point(dctx, &x, &y);
	cairo_set_line_width(dctx, 4.0);

	/* Draw 'h' */
	snprintf(tmp, 255, "%i", abs(h));
	cairo_text_extents(dctx, tmp, &size);
	cairo_show_text(dctx, tmp);
	cairo_fill(dctx);
	if ( h < 0 ) {
		cairo_move_to(dctx, x+size.x_bearing, y-sh);
		cairo_rel_line_to(dctx, size.width, 0.0);
		cairo_stroke(dctx);
	}
	x += size.x_advance;

	/* Draw 'k' */
	cairo_move_to(dctx, x, y);
	snprintf(tmp, 255, "%i", abs(k));
	cairo_text_extents(dctx, tmp, &size);
	cairo_show_text(dctx, tmp);
	cairo_fill(dctx);
	if ( k < 0 ) {
		cairo_move_to(dctx, x+size.x_bearing, y-sh);
		cairo_rel_line_to(dctx, size.width, 0.0);
		cairo_stroke(dctx);
	}
	x += size.x_advance;

	/* Draw 'l' */
	cairo_move_to(dctx, x, y);
	snprintf(tmp, 255, "%i", abs(l));
	cairo_text_extents(dctx, tmp, &size);
	cairo_show_text(dctx, tmp);
	cairo_fill(dctx);
	if ( l < 0 ) {
		cairo_move_to(dctx, x+size.x_bearing, y-sh);
		cairo_rel_line_to(dctx, size.width, 0.0);
		cairo_stroke(dctx);
	}
}


static void render_za(UnitCell *cell, RefList *list,
                      double boost, const SymOpList *sym, int wght,
                      int colscale,
                      signed int xh, signed int xk, signed int xl,
                      signed int yh, signed int yk, signed int yl,
                      const char *outfile, double scale_top)
{
	cairo_surface_t *surface;
	cairo_t *dctx;
	double max_val;
	double scale1, scale2, scale;
	double sep_u, sep_v, max_r;
	double u, v;
	double as, bs, theta;
	double asx, asy, asz;
	double bsx, bsy, bsz;
	double csx, csy, csz;
	float wh, ht;
	signed int zh, zk, zl;
	double xx, xy, xz;
	double yx, yy, yz;
	char tmp[256];
	cairo_text_extents_t size;
	double cx, cy;
	const double border = 200.0;
	int png;
	double rmin, rmax;

	/* Vector product to determine the zone axis. */
	zh = xk*yl - xl*yk;
	zk = - xh*yl + xl*yh;
	zl = xh*yk - xk*yh;
	STATUS("Zone axis is %i %i %i\n", zh, zk, zl);

	/* Size of output and centre definition */
	wh = 1024;
	ht = 1024;

	/* Work out reciprocal lattice spacings and angles for this cut */
	if ( cell_get_reciprocal(cell, &asx, &asy, &asz,
	                          &bsx, &bsy, &bsz,
	                          &csx, &csy, &csz) ) {
		ERROR("Couldn't get reciprocal parameters\n");
		return;
	}
	xx = xh*asx + xk*bsx + xl*csx;
	xy = xh*asy + xk*bsy + xl*csy;
	xz = xh*asz + xk*bsz + xl*csz;
	yx = yh*asx + yk*bsx + yl*csx;
	yy = yh*asy + yk*bsy + yl*csy;
	yz = yh*asz + yk*bsz + yl*csz;
	theta = angle_between(xx, xy, xz, yx, yy, yz);
	as = modulus(xx, xy, xz);
	bs = modulus(yx, yy, yz);

	resolution_limits(list, cell, &rmin, &rmax);
	printf("Resolution limits: 1/d = %.2f - %.2f nm^-1"
	       " (d = %.2f - %.2f A)\n",
	       rmin/1e9, rmax/1e9, (1.0/rmin)/1e-10, (1.0/rmax)/1e-10);

	max_val = max_value(list, wght, sym);
	if ( max_val <= 0.0 ) {
		STATUS("Couldn't find max value.\n");
		return;
	}

	/* Use manual scale top if specified */
	if ( scale_top > 0.0 ) {
		max_val = scale_top;
	}

	scale1 = ((double)wh-border) / (2.0*rmax);
	scale2 = ((double)ht-border) / (2.0*rmax);
	scale = (scale1 < scale2) ? scale1 : scale2;

	/* Work out the spot radius */
	sep_u = scale*as;
	sep_v = scale*bs;
	max_r = (sep_u < sep_v) ? sep_u : sep_v;
	max_r /= 2.0;  /* Max radius is half the separation */
	max_r -= (max_r/10.0);  /* Add a tiny separation between circles */

	/* Create surface */
	if ( strcmp(outfile+strlen(outfile)-4, ".png") == 0 ) {
		png = 1;
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                     wh, ht);
	} else {
		png = 0;
		surface = cairo_pdf_surface_create(outfile, wh, ht);
	}

	if ( cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS ) {
		ERROR("Couldn't create Cairo surface\n");
		cairo_surface_destroy(surface);
		return;
	}

	dctx = cairo_create(surface);
	if ( cairo_status(dctx) != CAIRO_STATUS_SUCCESS ) {
		ERROR("Couldn't create Cairo context\n");
		cairo_surface_destroy(surface);
		return;
	}

	/* Black background */
	cairo_rectangle(dctx, 0.0, 0.0, wh, ht);
	cairo_set_source_rgb(dctx, 0.0, 0.0, 0.0);
	cairo_fill(dctx);

	/* Test size of text that goes to the right(ish) */
	cairo_set_font_size(dctx, 40.0);
	snprintf(tmp, 255, "%i%i%i", abs(xh), abs(xk), abs(xl));
	cairo_text_extents(dctx, tmp, &size);

	cx = 532.0 - size.width;
	cy = 512.0 - 20.0;

	draw_circles(xh, xk, xl, yh, yk, yl, zh, zk, zl,
	             list, sym, dctx, wght, boost, colscale, cell,
	             max_r, theta, as, bs, cx, cy, scale,
	             max_val);

	/* Centre marker */
	cairo_arc(dctx, (double)cx,
			(double)cy, max_r, 0, 2*M_PI);
	cairo_set_source_rgb(dctx, 1.0, 0.0, 0.0);
	cairo_fill(dctx);

	/* Draw indexing lines */
	cairo_set_line_cap(dctx, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_width(dctx, 4.0);
	cairo_move_to(dctx, (double)cx, (double)cy);
	u = rmax*sin(theta);
	v = rmax*cos(theta);
	cairo_line_to(dctx, cx+u*scale, cy+v*scale);
	cairo_set_source_rgb(dctx, 0.0, 1.0, 0.0);
	cairo_stroke(dctx);

	cairo_set_font_size(dctx, 40.0);
	snprintf(tmp, 255, "%i%i%i", abs(xh), abs(xk), abs(xl));
	cairo_text_extents(dctx, tmp, &size);

	cairo_move_to(dctx, cx+u*scale + 20.0, cy+v*scale + size.height/2.0);
	render_overlined_indices(dctx, xh, xk, xl);
	cairo_fill(dctx);

	snprintf(tmp, 255, "%i%i%i", abs(yh), abs(yk), abs(yl));
	cairo_text_extents(dctx, tmp, &size);

	cairo_move_to(dctx, (double)cx, (double)cy);
	cairo_line_to(dctx, cx, cy+rmax*scale);
	cairo_set_source_rgb(dctx, 0.0, 1.0, 0.0);
	cairo_stroke(dctx);

	cairo_move_to(dctx, cx - size.width/2.0,
	                    cy+rmax*scale + size.height + 20.0);
	render_overlined_indices(dctx, yh, yk, yl);
	cairo_fill(dctx);

	if ( png ) {
		int r = cairo_surface_write_to_png(surface, outfile);
		if ( r != CAIRO_STATUS_SUCCESS ) {
			ERROR("Failed to write PNG to '%s'\n", outfile);
		}
	}

	cairo_surface_finish(surface);
	cairo_destroy(dctx);
}


static int render_key(int colscale, double scale_top)
{
	cairo_surface_t *surface;
	cairo_t *dctx;
	double top, wh, ht, y;
	double slice;

	wh = 128.0;
	ht = 1024.0;
	slice = 1.0;

	if ( scale_top > 0.0 ) {
		top = scale_top;
	} else {
		top = 1.0;
	}

	surface = cairo_pdf_surface_create(KEY_FILENAME, wh, ht);

	if ( cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS ) {
		fprintf(stderr, "Couldn't create Cairo surface\n");
		cairo_surface_destroy(surface);
		return 1;
	}

	dctx = cairo_create(surface);

	for ( y=0.0; y<ht; y+=slice ) {

		double r, g, b;
		double val;
		double v = y;

		cairo_rectangle(dctx, 0.0, ht-y, wh/2.0, slice);

		if ( colscale == SCALE_RATIO ) {
			if ( v < ht/2.0 ) {
				val = v/(ht/2.0);
			} else {
				val = (((v-ht/2.0)/(ht/2.0))*(top-1.0))+1.0;
			}
		} else {
			val = v/ht;
		}

		render_scale(val, top, colscale, &r, &g, &b);
		cairo_set_source_rgb(dctx, r, g, b);

		cairo_stroke_preserve(dctx);
		cairo_fill(dctx);

	}

	if ( colscale == SCALE_RATIO ) {

		cairo_text_extents_t size;
		char tmp[32];

		cairo_rectangle(dctx, 0.0, ht/2.0-2.0, wh/2.0, 4.0);
		cairo_set_source_rgb(dctx, 0.0, 0.0, 0.0);
		cairo_stroke_preserve(dctx);
		cairo_fill(dctx);

		cairo_set_font_size(dctx, 20.0);
		cairo_text_extents(dctx, "1.0", &size);
		cairo_move_to(dctx, wh/2.0+5.0, ht/2.0+size.height/2.0);
		cairo_show_text(dctx, "1.0");

		cairo_set_font_size(dctx, 20.0);
		cairo_text_extents(dctx, "0.0", &size);
		cairo_move_to(dctx, wh/2.0+5.0, ht-5.0);
		cairo_show_text(dctx, "0.0");

		cairo_set_font_size(dctx, 20.0);
		snprintf(tmp, 31, "%.1f", top);
		cairo_text_extents(dctx, tmp, &size);
		cairo_move_to(dctx, wh/2.0+5.0, size.height+5.0);
		cairo_show_text(dctx, tmp);

	}


	cairo_surface_finish(surface);
	cairo_destroy(dctx);

	STATUS("Colour key written to "KEY_FILENAME"\n");

	return 0;
}


#else  /* HAVE_CAIRO */


static int render_key(int colscale, double scale_top)
{
	ERROR("This version of CrystFEL was compiled without Cairo");
	ERROR(" support, which is required to draw the colour");
	ERROR(" scale.  Sorry!\n");
	return 1;
}


static void render_za(UnitCell *cell, RefList *list,
                      double boost, const char *sym, int wght, int colscale,
                      signed int xh, signed int xk, signed int xl,
                      signed int yh, signed int yk, signed int yl,
                      const char *outfile, double scale_top)
{
	ERROR("This version of CrystFEL was compiled without Cairo");
	ERROR(" support, which is required to plot a zone axis");
	ERROR(" pattern.  Sorry!\n");
}


#endif /* HAVE_CAIRO */


int main(int argc, char *argv[])
{
	int c;
	UnitCell *cell;
	RefList *list;
	char *infile;
	int config_sqrt = 0;
	int config_colkey = 0;
	int config_zawhinge = 0;
	char *pdb = NULL;
	int r = 0;
	double boost = 1.0;
	char *sym_str = NULL;
	SymOpList *sym;
	char *weighting = NULL;
	int wght;
	int colscale;
	char *cscale = NULL;
	signed int dh=1, dk=0, dl=0;
	signed int rh=0, rk=1, rl=0;
	char *down = NULL;
	char *right = NULL;
	char *outfile = NULL;
	double scale_top = -1.0;
	char *endptr;

	/* Long options */
	const struct option longopts[] = {
		{"help",               0, NULL,               'h'},
		{"zone-axis",          0, &config_zawhinge,    1},
		{"output",             1, NULL,               'o'},
		{"pdb",                1, NULL,               'p'},
		{"boost",              1, NULL,               'b'},
		{"symmetry",           1, NULL,               'y'},
		{"weighting",          1, NULL,               'w'},
		{"colscale",           1, NULL,               'c'},
		{"down",               1, NULL,               'd'},
		{"right",              1, NULL,               'r'},
		{"counts",             0, &config_sqrt,        1},
		{"colour-key",         0, &config_colkey,      1},
		{"scale-top",          1, NULL,                2},
		{0, 0, NULL, 0}
	};

	/* Short options */
	while ((c = getopt_long(argc, argv, "hp:w:c:y:d:r:o:",
	                        longopts, NULL)) != -1) {

		switch (c) {
		case 'h' :
			show_help(argv[0]);
			return 0;

		case 'p' :
			pdb = strdup(optarg);
			break;

		case 'b' :
			boost = atof(optarg);
			break;

		case 'y' :
			sym_str = strdup(optarg);
			break;

		case 'w' :
			weighting = strdup(optarg);
			break;

		case 'c' :
			cscale = strdup(optarg);
			break;

		case 'd' :
			down = strdup(optarg);
			break;

		case 'r' :
			right = strdup(optarg);
			break;

		case 'o' :
			outfile = strdup(optarg);
			break;

		case 2 :
			errno = 0;
			scale_top = strtod(optarg, &endptr);
			if ( !( (optarg[0] != '\0') && (endptr[0] == '\0') )
			   || (errno != 0) )
			{
				ERROR("Invalid scale top('%s')\n", optarg);
				return 1;
			}
			if ( scale_top < 0.0 ) {
				ERROR("Scale top must be positive.\n");
				return 1;
			}
			break;

		case 0 :
			break;

		default :
			return 1;
		}

	}

	if ( config_zawhinge ) {
		ERROR("Friendly warning: The --zone-axis option isn't needed"
		      " any longer (I ignored it for you).\n");
	}

	if ( (pdb == NULL) && !config_colkey ) {
		ERROR("You must specify the PDB containing the unit cell.\n");
		return 1;
	}

	if ( sym_str == NULL ) {
		sym_str = strdup("1");
	}
	sym = get_pointgroup(sym_str);
	free(sym_str);

	if ( weighting == NULL ) {
		weighting = strdup("I");
	}

	if ( outfile == NULL ) outfile = strdup("za.pdf");

	if ( strcmp(weighting, "I") == 0 ) {
		wght = WGHT_I;
	} else if ( strcmp(weighting, "sqrtI") == 0 ) {
		wght = WGHT_SQRTI;
	} else if ( strcmp(weighting, "count") == 0 ) {
		wght = WGHT_COUNTS;
	} else if ( strcmp(weighting, "counts") == 0 ) {
		wght = WGHT_COUNTS;
	} else if ( strcmp(weighting, "rawcts") == 0 ) {
		wght = WGHT_RAWCOUNTS;
	} else if ( strcmp(weighting, "rawcount") == 0 ) {
		wght = WGHT_RAWCOUNTS;
	} else if ( strcmp(weighting, "rawcounts") == 0 ) {
		wght = WGHT_RAWCOUNTS;
	} else {
		ERROR("Unrecognised weighting '%s'\n", weighting);
		return 1;
	}
	free(weighting);

	if ( cscale == NULL ) {
		cscale = strdup("mono");
	}

	if ( strcmp(cscale, "mono") == 0 ) {
		colscale = SCALE_MONO;
	} else if ( strcmp(cscale, "invmono") == 0 ) {
		colscale = SCALE_INVMONO;
	} else if ( strcmp(cscale, "colour") == 0 ) {
		colscale = SCALE_COLOUR;
	} else if ( strcmp(cscale, "color") == 0 ) {
		colscale = SCALE_COLOUR;
	} else if ( strcmp(cscale, "ratio") == 0 ) {
		colscale = SCALE_RATIO;
	} else {
		ERROR("Unrecognised colour scale '%s'\n", cscale);
		return 1;
	}
	free(cscale);

	if ( config_colkey ) {
		return render_key(colscale, scale_top);
	}

	if ( (( down == NULL ) && ( right != NULL ))
	  || (( down != NULL ) && ( right == NULL )) ) {
		ERROR("Either specify both 'down' and 'right',"
		      " or neither.\n");
		return 1;
	}
	if ( down != NULL ) {
		int r;
		r = sscanf(down, "%i,%i,%i", &dh, &dk, &dl);
		if ( r != 3 ) {
			ERROR("Invalid format for 'down'\n");
			return 1;
		}
	}
	if ( right != NULL ) {
		int r;
		r = sscanf(right, "%i,%i,%i", &rh, &rk, &rl);
		if ( r != 3 ) {
			ERROR("Invalid format for 'right'\n");
			return 1;
		}
	}

	infile = argv[optind];

	cell = load_cell_from_pdb(pdb);
	if ( cell == NULL ) {
		ERROR("Couldn't load unit cell from %s\n", pdb);
		return 1;
	}
	list = read_reflections(infile);
	if ( list == NULL ) {
		ERROR("Couldn't read file '%s'\n", infile);
		return 1;
	}
	if ( check_list_symmetry(list, sym) ) {
		ERROR("The input reflection list does not appear to"
		      " have symmetry %s\n", symmetry_name(sym));
		return 1;
	}

	render_za(cell, list, boost, sym, wght, colscale,
	          rh, rk, rl, dh, dk, dl, outfile, scale_top);

	free(pdb);
	free_symoplist(sym);
	reflist_free(list);
	if ( outfile != NULL ) free(outfile);

	return r;
}
