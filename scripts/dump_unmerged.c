/* dump_unmerged

   export observations in a stream file to a CSV file

 */

#include <stdlib.h>
#include <stdio.h>

#include "stream.h"
#include "image.h"
#include "symmetry.h"
#include "cell-utils.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Wrong number of arguments.\n");
		printf("Usage: dump_unmerged input.stream pointgroup\n");
		return -1;
	}

	Stream *st = open_stream_for_read(argv[1]);
	if (st == NULL) {
		printf("Failed to open stream %s.\n", argv[1]);
		return -1;
	}

	SymOpList *sym = get_pointgroup(argv[2]);

	struct image image;
	int j;

	while (!read_chunk(st, &image)) {
		for (j = 0; j<image.n_crystals; j++ ) {
			Crystal *cr = image.crystals[j];
			Reflection *refl;
			RefListIterator *iter;      
			for (refl = first_refl(crystal_get_reflections(cr), &iter);
			     refl != NULL;
			     refl = next_refl(refl, iter)) {
				signed int h, k, l, hu, ku, lu;
				get_indices(refl, &h, &k, &l);
				get_asymm(sym, h, k, l, &hu, &ku, &lu);	
				printf("%i %i %i %.2f %.1f %.1f\n", hu, ku, lu,
				       1E10 / 2.0 / resolution(crystal_get_cell(cr), hu, ku, lu),
				       get_intensity(refl), get_esd_intensity(refl));
			}			

			reflist_free(crystal_get_reflections(cr));
			cell_free(crystal_get_cell(cr));
			crystal_free(cr);
		}
	}

	return 0;
}
