/* export_to_pickle

   export observations in a stream file to pickle files
   cctbx.xfel can read.

   WARNING: This program is NOT tested throughfully and NOT working well.

 */

#include <stdlib.h>
#include <stdio.h>

#include "stream.h"
#include "image.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Wrong number of arguments.\n");
    printf("Usage: export_to_pickle geometry.geom input.stream | cctbx.python export_to_pickle.py\n");
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
  image.det = geom;

  int ncryst = 0, j = 0;
  while (!read_chunk(st, &image)) {
    for (j = 0; j<image.n_crystals; j++ ) {
      Crystal *cr = image.crystals[j];
      ncryst++;
      printf("======================== Crystal %d\n", ncryst);
      printf("%f\n", image.lambda * 10E9); // in A
      double asx, asy, asz;
      double bsx, bsy, bsz;
      double csx, csy, csz;
      cell_get_reciprocal(crystal_get_cell(cr),
			  &asx, &asy, &asz,
			  &bsx, &bsy, &bsz,
			  &csx, &csy, &csz);
      printf("%f %f %f %f %f %f %f %f %f\n", asx, asy, asz, bsx, bsy, bsz, csx, csy, csz);
      printf("%d\n", num_reflections(crystal_get_reflections(cr)));      

      Reflection *refl;
      RefListIterator *iter;      
      for (refl = first_refl(crystal_get_reflections(cr), &iter);
	   refl != NULL;
	   refl = next_refl(refl, iter)) {
	
	signed int h, k, l;
	double fs, ss, xs, ys, x, y;
	struct panel *p;

	get_indices(refl, &h, &k, &l);
	get_detector_pos(refl, &fs, &ss);
	
	p = find_panel(geom, fs, ss);
	if (p == NULL) continue;
	
	xs = (fs-p->min_fs)*p->fsx + (ss-p->min_ss)*p->ssx;
	ys = (fs-p->min_fs)*p->fsy + (ss-p->min_ss)*p->ssy;
	x = xs + p->cnx;
	y = ys + p->cny;
	
	printf("%i %i %i %f %f %f %f\n", h, k, l, 
	       get_intensity(refl), get_esd_intensity(refl), x, y);
      }			
      
      reflist_free(crystal_get_reflections(cr));
      cell_free(crystal_get_cell(cr));
      crystal_free(cr);		  
    }
  }
 
  return 0;
}
