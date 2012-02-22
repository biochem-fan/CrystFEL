/*
 * reflist-utils.c
 *
 * Utilities to complement the core reflist.c
 *
 * Copyright © 2012 Thomas White <taw@physics.org>
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

#define _ISOC99_SOURCE
#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <assert.h>


#include "reflist.h"
#include "cell.h"
#include "utils.h"
#include "reflist-utils.h"
#include "symmetry.h"


/**
 * SECTION:reflist-utils
 * @short_description: Reflection list utilities
 * @title: RefList utilities
 * @section_id:
 * @see_also:
 * @include: "reflist-utils.h"
 * @Image:
 *
 * There are some utility functions associated with the core %RefList.
 **/


int check_list_symmetry(RefList *list, const SymOpList *sym)
{
	Reflection *refl;
	RefListIterator *iter;
	SymOpMask *mask;

	mask = new_symopmask(sym);
	if ( mask == NULL ) {
		ERROR("Couldn't create mask for list symmetry check.\n");
		return 1;
	}

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) ) {

		int j;
		int found = 0;
		signed int h, k, l;
		int n;

		get_indices(refl, &h, &k, &l);

		special_position(sym, mask, h, k, l);
		n = num_equivs(sym, mask);

		for ( j=0; j<n; j++ ) {

			signed int he, ke, le;
			Reflection *f;

			get_equiv(sym, mask, j, h, k, l, &he, &ke, &le);

			f = find_refl(list, he, ke, le);
			if ( f != NULL ) found++;

		}

		assert(found != 0);  /* That'd just be silly */
		if ( found > 1 ) {

			STATUS("Found %i %i %i: %i times:\n", h, k, l, found);

			for ( j=0; j<n; j++ ) {

				signed int he, ke, le;
				Reflection *f;

				get_equiv(sym, mask, j, h, k, l, &he, &ke, &le);

				f = find_refl(list, he, ke, le);
				if ( f != NULL ) {
					STATUS("%3i %3i %3i\n", he, ke, le);
				}

			}
			free_symopmask(mask);

			return 1;  /* Symmetry is wrong! */
		}

	}

	free_symopmask(mask);

	return 0;
}


int find_equiv_in_list(RefList *list, signed int h, signed int k,
                       signed int l, const SymOpList *sym, signed int *hu,
                       signed int *ku, signed int *lu)
{
	int i;
	int found = 0;

	for ( i=0; i<num_equivs(sym, NULL); i++ ) {

		signed int he, ke, le;
		Reflection *f;
		get_equiv(sym, NULL, i, h, k, l, &he, &ke, &le);
		f = find_refl(list, he, ke, le);

		/* There must only be one equivalent.  If there are more, it
		 * indicates that the user lied about the input symmetry.
		 * This situation should have been checked for earlier by
		 * calling check_symmetry() with 'items' and 'mero'. */

		if ( (f != NULL) && !found ) {
			*hu = he;  *ku = ke;  *lu = le;
			return 1;
		}

	}

	return 0;
}


/**
 * write_reflections_to_file:
 * @fh: File handle to write to
 * @list: The reflection list to write
 * @cell: Unit cell to use for generating 1/d values, or NULL.
 *
 * This function writes the contents of @list to @fh, using @cell to generate
 * 1/d values to ease later processing.  If @cell is NULL, 1/d values will not
 * be included ('-' will be written in their place).
 *
 * Reflections which have a redundancy of zero will not be written.
 *
 * The resulting list can be read back with read_reflections_from_file().
 **/
void write_reflections_to_file(FILE *fh, RefList *list, UnitCell *cell)
{
	Reflection *refl;
	RefListIterator *iter;

	fprintf(fh, "  h   k   l          I    phase   sigma(I) "
		     " 1/d(nm^-1)  counts  fs/px  ss/px\n");

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{

		signed int h, k, l;
		double intensity, esd_i, s, ph;
		int red;
		double fs, ss;
		char res[16];
		char phs[16];
		int have_phase;

		get_indices(refl, &h, &k, &l);
		get_detector_pos(refl, &fs, &ss);
		intensity = get_intensity(refl);
		esd_i = get_esd_intensity(refl);
		red = get_redundancy(refl);
		ph = get_phase(refl, &have_phase);

		/* Reflections with redundancy = 0 are not written */
		if ( red == 0 ) continue;

		if ( cell != NULL ) {
			s = 2.0 * resolution(cell, h, k, l);
			snprintf(res, 16, "%10.2f", s/1e9);
		} else {
			strcpy(res, "         -");
		}

		if ( have_phase ) {
			snprintf(phs, 16, "%8.2f", rad2deg(ph));
		} else {
			strncpy(phs, "       -", 15);
		}

		fprintf(fh,
		       "%3i %3i %3i %10.2f %s %10.2f  %s %7i %6.1f %6.1f\n",
		       h, k, l, intensity, phs, esd_i, res, red,
		       fs, ss);

	}
}


/**
 * write_reflist:
 * @filename: Filename
 * @list: The reflection list to write
 * @cell: Unit cell to use for generating 1/d values, or NULL.
 *
 * This function writes the contents of @list to @file, using @cell to generate
 * 1/d values to ease later processing.  If @cell is NULL, 1/d values will not
 * be included ('-' will be written in their place).
 *
 * Reflections which have a redundancy of zero will not be written.
 *
 * The resulting list can be read back with read_reflections_from_file() or
 * read_reflections().
 *
 * This is a convenience function which simply opens @filename and then calls
 * write_reflections_to_file.
 *
 * Returns: zero on success, non-zero on failure.
 **/
int write_reflist(const char *filename, RefList *list, UnitCell *cell)
{
	FILE *fh;

	if ( filename == NULL ) {
		fh = stdout;
	} else {
		fh = fopen(filename, "w");
	}

	if ( fh == NULL ) {
		ERROR("Couldn't open output file '%s'.\n", filename);
		return 1;
	}

	write_reflections_to_file(fh, list, cell);
	fprintf(fh, REFLECTION_END_MARKER"\n");

	fclose(fh);

	return 0;
}


RefList *read_reflections_from_file(FILE *fh)
{
	char *rval = NULL;
	int first = 1;
	RefList *out;

	out = reflist_new();

	do {

		char line[1024];
		signed int h, k, l;
		float intensity, sigma, fs, ss;
		char phs[1024];
		char ress[1024];
		int cts;
		int r;
		Reflection *refl;

		rval = fgets(line, 1023, fh);
		if ( rval == NULL ) continue;
		chomp(line);

		if ( strcmp(line, REFLECTION_END_MARKER) == 0 ) return out;

		r = sscanf(line, "%i %i %i %f %s %f %s %i %f %f",
		           &h, &k, &l, &intensity, phs, &sigma, ress, &cts,
		           &fs, &ss);
		if ( (r != 10) && (!first) ) {
			reflist_free(out);
			return NULL;
		}

		first = 0;
		if ( r == 10 ) {

			double ph;
			char *v;

			refl = add_refl(out, h, k, l);
			set_int(refl, intensity);
			set_detector_pos(refl, 0.0, fs, ss);
			set_esd_intensity(refl, sigma);
			set_redundancy(refl, cts);

			ph = strtod(phs, &v);
			if ( v != NULL ) set_ph(refl, deg2rad(ph));

			/* The 1/d value is actually ignored. */

		}

	} while ( rval != NULL );

	/* Got read error of some kind before finding PEAK_LIST_END_MARKER */
	return NULL;
}


RefList *read_reflections(const char *filename)
{
	FILE *fh;
	RefList *out;

	if ( filename == NULL ) {
		fh = stdout;
	} else {
		fh = fopen(filename, "r");
	}

	if ( fh == NULL ) {
		ERROR("Couldn't open input file '%s'.\n", filename);
		return NULL;
	}

	out = read_reflections_from_file(fh);

	fclose(fh);

	return out;
}


/**
 * asymmetric_indices:
 * @in: A %RefList
 * @sym: A %SymOpList
 *
 * This function creates a newly allocated copy of @in, but indexed using the
 * asymmetric indices according to @sym instead of the original indices.  The
 * original indices are stored and can be retrieved using
 * get_symmetric_indices() if required.
 *
 * Returns: the new %RefList, or NULL on failure.
 **/
RefList *asymmetric_indices(RefList *in, const SymOpList *sym)
{
	Reflection *refl;
	RefListIterator *iter;
	RefList *new;

	new = reflist_new();
	if ( new == NULL ) return NULL;

	for ( refl = first_refl(in, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) ) {

		signed int h, k, l;
		signed int ha, ka, la;
		Reflection *cr;

		get_indices(refl, &h, &k, &l);

		get_asymm(sym, h, k, l, &ha, &ka, &la);

		cr = add_refl(new, ha, ka, la);
		assert(cr != NULL);

		copy_data(cr, refl);
		set_symmetric_indices(cr, h, k, l);

	}

	return new;
}


/**
 * resolution_limits:
 * @list: A %RefList
 * @cell: A %UnitCell
 * @rmin: Place to store the minimum 1/d value
 * @rmax: Place to store the maximum 1/d value
 *
 * This function calculates the minimum and maximum values of 1/d, where
 * 2dsin(theta) = wavelength.  The answers are in m^-1.
 **/
void resolution_limits(RefList *list, UnitCell *cell,
                       double *rmin, double *rmax)
{
	Reflection *refl;
	RefListIterator *iter;

	*rmin = INFINITY;
	*rmax = 0.0;

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{
		double r;
		signed int h, k, l;

		get_indices(refl, &h, &k, &l);
		r = 2.0 * resolution(cell, h, k, l);

		if ( r > *rmax ) *rmax = r;
		if ( r < *rmin ) *rmin = r;
	}
}


/**
 * max_intensity:
 * @list: A %RefList
 *
 * Returns: The maximum intensity in @list.
 **/
double max_intensity(RefList *list)
{
	Reflection *refl;
	RefListIterator *iter;
	double max;

	max = -INFINITY;

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{
		double val = get_intensity(refl);
		if ( val > max ) max = val;
	}

	return max;
}


/**
 * res_cutoff:
 * @list: A %RefList
 *
 * Returns: A new %RefList with resolution cutoff applied
 **/
RefList *res_cutoff(RefList *list, UnitCell *cell, double min, double max)
{
	Reflection *refl;
	RefListIterator *iter;
	RefList *new;

	new = reflist_new();

	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) )
	{
		double one_over_d;
		signed int h, k, l;
		Reflection *n;

		get_indices(refl, &h, &k, &l);

		one_over_d = 2.0 * resolution(cell, h, k, l);
		if ( one_over_d < min ) continue;
		if ( one_over_d > max ) continue;

		n = add_refl(new, h, k, l);
		copy_data(n, refl);
	}

	reflist_free(list);
	return new;
}
