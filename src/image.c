/*
 * image.c
 *
 * Handle images and image features
 *
 * (c) 2006-2009 Thomas White <thomas.white@desy.de>
 *
 * template_index - Indexing diffraction patterns by template matching
 *
 */


#define _GNU_SOURCE 1
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "image.h"
#include "utils.h"


int image_add(ImageList *list, struct image *image)
{
	if ( list->images ) {
		list->images = realloc(list->images,
		                       (list->n_images+1)*sizeof(struct image));
	} else {
		assert(list->n_images == 0);
		list->images = malloc(sizeof(struct image));
	}

	/* Copy the metadata */
	list->images[list->n_images] = *image;

	/* Fill out some extra fields */
	list->images[list->n_images].features = NULL;
	list->images[list->n_images].rflist = NULL;

	list->n_images++;

	return list->n_images - 1;
}


ImageList *image_list_new()
{
	ImageList *list;

	list = malloc(sizeof(ImageList));

	list->n_images = 0;
	list->images = NULL;

	return list;
}


void image_add_feature_reflection(ImageFeatureList *flist, double x, double y,
                                  struct image *parent, double intensity)
{
	if ( flist->features ) {
		flist->features = realloc(flist->features,
		                    (flist->n_features+1)*sizeof(ImageFeature));
	} else {
		assert(flist->n_features == 0);
		flist->features = malloc(sizeof(ImageFeature));
	}

	flist->features[flist->n_features].x = x;
	flist->features[flist->n_features].y = y;
	flist->features[flist->n_features].intensity = intensity;
	flist->features[flist->n_features].parent = parent;
	flist->features[flist->n_features].partner = NULL;
	flist->features[flist->n_features].partner_d = 0.0;

	flist->n_features++;

}


void image_add_feature(ImageFeatureList *flist, double x, double y,
                       struct image *parent, double intensity)
{
	image_add_feature_reflection(flist, x, y, parent, intensity);
}


ImageFeatureList *image_feature_list_new()
{
	ImageFeatureList *flist;

	flist = malloc(sizeof(ImageFeatureList));

	flist->n_features = 0;
	flist->features = NULL;

	return flist;
}


void image_feature_list_free(ImageFeatureList *flist)
{
	if ( !flist ) return;

	if ( flist->features ) free(flist->features);
	free(flist);
}


ImageFeature *image_feature_closest(ImageFeatureList *flist, double x, double y,
                                    double *d, int *idx)
{
	int i;
	double dmin = +HUGE_VAL;
	int closest = 0;

	for ( i=0; i<flist->n_features; i++ ) {

		double d;

		d = distance(flist->features[i].x, flist->features[i].y, x, y);

		if ( d < dmin ) {
			dmin = d;
			closest = i;
		}

	}

	if ( dmin < +HUGE_VAL ) {
		*d = dmin;
		*idx = closest;
		return &flist->features[closest];
	}

	*d = +INFINITY;
	return NULL;
}


ImageFeature *image_feature_second_closest(ImageFeatureList *flist,
                                           double x, double y, double *d,
                                           int *idx)
{
	int i;
	double dmin = +HUGE_VAL;
	int closest = 0;
	double dfirst;
	int idxfirst;

	image_feature_closest(flist, x, y, &dfirst, &idxfirst);

	for ( i=0; i<flist->n_features; i++ ) {

		double d;

		d = distance(flist->features[i].x, flist->features[i].y, x, y);

		if ( (d < dmin) && (i != idxfirst) ) {
			dmin = d;
			closest = i;
		}

	}

	if ( dmin < +HUGE_VAL ) {
		*d = dmin;
		*idx = closest;
		return &flist->features[closest];
	}

	return NULL;

}
