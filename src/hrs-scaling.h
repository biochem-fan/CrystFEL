/*
 * hrs-scaling.h
 *
 * Intensity scaling using generalised HRS target function
 *
 * (c) 2006-2010 Thomas White <taw@physics.org>
 *
 * Part of CrystFEL - crystallography with a FEL
 *
 */

#ifndef HRS_SCALING_H
#define HRS_SCALING_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "image.h"

extern void scale_intensities(struct image *image, int n, const char *sym);


#endif	/* HRS_SCALING_H */