/* get_rms 

  This program calculates RMS deviation of predicted and observed spots 
  on the detector surface and reciprocal space.

  Usage: get_rms geometry.geom input.stream > output.csv
  
  Columns in output: rms-real, rms-reciprocal, n_accepted, n_allspots
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>

#include "stream.h"
#include "detector.h"
#include "image.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("get_rms: This program calculates RMS deviation of predicted and observed spots on the detector surface and reciprocal space.\n");
    printf("Wrong number of arguments. \n");
    printf("usage: get_rms geometry.geom input.stream > output.csv\n");
    return -1;
  }
  struct detector *geom = get_detector_geometry(argv[1], NULL);
  Stream *st = open_stream_for_read(argv[2]);
  
  if (geom == NULL) {
    printf("Failed to open geometry %s.\n", argv[1]);
    return -1;
  }
  if (st == NULL) {
    printf("Failed to open stream %s.\n", argv[2]);
    return -1;
  }

  struct image image;
  int ncryst = 0;
  int i, j;
  image.det = geom;
  while (!read_chunk(st, &image)) {
    for (i = 0; i<image.n_crystals; i++) {
      Crystal *cr = image.crystals[i];
      ncryst++;

      double asx, asy, asz; // reciprocal basis
      double bsx, bsy, bsz;
      double csx, csy, csz;
      double ax, ay, az; // real basis
      double bx, by, bz;
      double cx, cy, cz;

      cell_get_reciprocal(crystal_get_cell(cr),
			  &asx, &asy, &asz,
			  &bsx, &bsy, &bsz,
			  &csx, &csy, &csz);
      cell_get_cartesian(crystal_get_cell(cr), 
			 &ax, &ay, &az,
			 &bx, &by, &bz, 
			 &cx, &cy, &cz);

      int n_accepted = 0;
      double rms = 0, rms_rec = 0;

      // Based on select_intersections(), check_reflection() in geometry.c
      for (j=0; j<image_feature_count(image.features); j++ ) {
	struct imagefeature *f;
	struct rvec q;
	double h, k, l, hd, kd, ld;
	double xl, yl, zl;
	double dsq;
	const double max_dist = 0.25;
	
	f = image_get_feature(image.features, j);
	if (f == NULL) continue;

	//printf("fs = %f, ss = %f image->det = %p\n", f->fs, f->ss, image.det);

	q = get_q(&image, f->fs, f->ss, NULL, 1.0 / image.lambda);
	
	hd = q.u * ax + q.v * ay + q.w * az;
	kd = q.u * bx + q.v * by + q.w * bz;
	ld = q.u * cx + q.v * cy + q.w * cz;
	h = lrint(hd);
	k = lrint(kd);
	l = lrint(ld);
	
	dsq = pow(h-hd, 2.0) + pow(k-kd, 2.0) + pow(l-ld, 2.0);

	/* Ideal position in reciprocal space*/
	xl = h*asx + k*bsx + l*csx;
	yl = h*asy + k*bsy + l*csy;
	zl = h*asz + k*bsz + l*csz;

	/* based on locate_peak in geometry.h */
	double x_ideal, y_ideal;
	struct panel *p = image.det->panels; // FIXME: just use the first one
	double den = 1.0 / image.lambda + zl;

	/* Coordinates of peak relative to central beam, in pixel */
	x_ideal = p->clen * xl / den * p->res;
	y_ideal = p->clen * yl / den * p->res;
	
	p = find_panel(geom, f->fs, f->ss);
	if (p == NULL) continue;
	
	double xs, ys, x_obs, y_obs;
	xs = (f->fs - p->min_fs) * p->fsx + (f->ss - p->min_ss) * p->ssx;
	ys = (f->fs - p->min_fs) * p->fsy + (f->ss - p->min_ss) * p->ssy;
	x_obs = xs + p->cnx;
	y_obs = ys + p->cny;

	double delta2 = (x_ideal - x_obs) * (x_ideal - x_obs) + (y_ideal - y_obs) * (y_ideal - y_obs);

	//	printf("pred (%f, %f) obs (%f, %f) ",  x_ideal, y_ideal, x_obs, y_obs);
	//	printf("hd %f kd %f ld %f ", h - hd, k - kd, l - ld);
	//	printf("delta2 %f, dsq %f, accepted %d\n", delta2, dsq, (dsq > max_dist) ? 0 : 1);

	if (dsq < max_dist) {
	  n_accepted++;
	  rms += delta2 / p->res; // p->res is pixel per meter
	  rms_rec += dsq;
	}
      }

      if (n_accepted > 0) {
	printf("%s, %f, %f, %d, %d\n", image.filename,
	       sqrt(rms / n_accepted), sqrt(rms_rec / n_accepted), n_accepted, 
	       image_feature_count(image.features));
      }

      reflist_free(crystal_get_reflections(cr));
      cell_free(crystal_get_cell(cr));
      crystal_free(cr);		  
    }
  }
 
  return 0;
}
