/*
 * index.h
 *
 * Perform indexing (somehow)
 *
 * (c) 2006-2010 Thomas White <taw@physics.org>
 *
 * Part of CrystFEL - crystallography with a FEL
 *
 */


#ifndef INDEX_H
#define INDEX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "cell.h"
#include "image.h"
#include "detector.h"


/* Indexing methods */
typedef enum {
	INDEXING_NONE,
	INDEXING_DIRAX,
	INDEXING_MOSFLM,
	INDEXING_TEMPLATE
} IndexingMethod;


/* Cell reduction methods */
enum {
	CELLR_NONE,
	CELLR_REDUCE,
	CELLR_COMPARE
};


typedef struct _indexingprivate IndexingPrivate;

extern IndexingPrivate *prepare_indexing(IndexingMethod indm, UnitCell *cell,
                                         const char *filename,
                                         struct detector *det,
                                         double nominal_photon_energy);

extern void map_all_peaks(struct image *image);

extern void index_pattern(struct image *image, UnitCell *cell,
                          IndexingMethod indm, int cellr, int verbose,
                          IndexingPrivate *priv);

extern void cleanup_indexing(IndexingPrivate *priv);

#endif	/* INDEX_H */
