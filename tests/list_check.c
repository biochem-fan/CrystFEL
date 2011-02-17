/*
 * list_check.c
 *
 * Unit test for the reflection list module
 *
 * (c) 2011 Thomas White <taw@physics.org>
 *
 * Part of CrystFEL - crystallography with a FEL
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdlib.h>
#include <stdio.h>

#include "../src/reflist.h"


struct refltemp {
	signed int h;
	signed int k;
	signed int l;
	int num;
	int found;
};

#define RANDOM_INDEX (128*random()/RAND_MAX - 256*random()/RAND_MAX)


static int test_lists(int num_items)
{
	struct refltemp *check;
	RefList *list;
	int i;
	signed int h, k, l;
	Reflection *refl;
	RefListIterator *iter;

	check = malloc(num_items * sizeof(struct refltemp));
	list = reflist_new();

	h = RANDOM_INDEX;
	k = RANDOM_INDEX;
	l = RANDOM_INDEX;

	for ( i=0; i<num_items; i++ ) {

		int j;
		int num;

		if ( random() > RAND_MAX/2 ) {
			h = RANDOM_INDEX;
			k = RANDOM_INDEX;
			l = RANDOM_INDEX;
		} /* else use the same as last time */

		/* Count the number of times this reflection appeared before */
		num = 1;
		for ( j=0; j<i; j++ ) {
			if ( (check[j].h == h)
			  && (check[j].k == k)
			  && (check[j].l == l) ) {
				num++;
			}
		}

		/* Update all copies with this number */
		for ( j=0; j<i; j++ ) {
			if ( (check[j].h == h)
			  && (check[j].k == k)
			  && (check[j].l == l) ) {
				check[j].num = num;
			}
		}

		add_refl(list, h, k, l);
		check[i].h = h;
		check[i].k = k;
		check[i].l = l;
		check[i].num = num;
		check[i].found = 0;

	}

	optimise_reflist(list);

	/* Iterate over the list and check we find everything */
	for ( refl = first_refl(list, &iter);
	      refl != NULL;
	      refl = next_refl(refl, iter) ) {

		signed int h, k, l;

		get_indices(refl, &h, &k, &l);

		for ( i=0; i<num_items; i++ ) {
			if ( (check[i].h == h)
			  && (check[i].k == k)
			  && (check[i].l == l)
			  && (check[i].found == 0) ) {
				check[i].found = 1;
			}
		}

	}
	for ( i=0; i<num_items; i++ ) {
		if ( check[i].found == 0 ) {

			Reflection *test;

			fprintf(stderr, "Iteration didn't find %3i %3i %3i %i\n",
			        check[i].h, check[i].k, check[i].l, i);
			test = find_refl(list, check[i].h, check[i].k,
			                 check[i].l);
			if ( test == NULL ) {
				fprintf(stderr, "Not in list\n");
			} else {
				fprintf(stderr, "But found in list.\n");
			}
			return 1;

		}
	}

	/* Check that all the reflections can be found */
	for ( i=0; i<num_items; i++ ) {

		signed int h, k, l;
		Reflection *refl;

		h = check[i].h;
		k = check[i].k;
		l = check[i].l;

		refl = find_refl(list, h, k, l);
		if ( refl == NULL ) {
			fprintf(stderr, "Couldn't find %3i %3i %3i\n", h, k, l);
			return 1;
		}

	}

	/* Delete some reflections */
	for ( i=0; i<num_items/2; i++ ) {

		int j;
		signed int h, k, l;
		Reflection *refl;

		h = check[i].h;
		k = check[i].k;
		l = check[i].l;

		refl = find_refl(list, h, k, l);
		delete_refl(refl);

		/* Update all counts */
		for ( j=0; j<num_items; j++ ) {
			if ( (check[j].h == h) && (check[j].k == k)
			  && (check[j].l == l) ) check[j].num--;
		}

	}

	/* Check that the deleted reflections can no longer be found */
	for ( i=0; i<num_items; i++ ) {

		signed int h, k, l;
		Reflection *refl;

		h = check[i].h;
		k = check[i].k;
		l = check[i].l;

		if ( check[i].num > 0 ) continue;

		refl = find_refl(list, h, k, l);
		if ( refl != NULL ) {

			fprintf(stderr, "Found %3i %i %3i after  deletion.\n",
			                h, k, l);
			return 1;

		}

	}

	/* Delete remaining duplicates */
	for ( i=0; i<num_items; i++ ) {

		signed int h, k, l;
		Reflection *refl;

		if ( check[i].num == 0 ) continue;

		h = check[i].h;
		k = check[i].k;
		l = check[i].l;
		refl = find_refl(list, h, k, l);

		do {
			int j;
			signed int ha, ka, la;
			Reflection *next;
			get_indices(refl, &ha, &ka, &la);
			next = next_found_refl(refl);
			delete_refl(refl);
			refl = next;
			for ( j=0; j<num_items; j++ ) {
				if ( (check[j].h == h) && (check[j].k == k)
				  && (check[j].l == l) ) check[j].num--;
			}
		} while ( refl != NULL );

		if ( check[i].num != 0 ) {
			fprintf(stderr, "Found too few duplicates (%i) for "
			                "%3i %3i %3i\n", check[i].num, h, k, l);
			return 1;
		}

		refl = find_refl(list, h, k, l);
		if ( refl != NULL ) {
			fprintf(stderr, "Found too many duplicates for "
			                "%3i %3i %3i\n", h, k, l);
			return 1;
		}

	}

	reflist_free(list);
	free(check);

	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	printf("Running list test...");
	fflush(stdout);

	for ( i=0; i<100; i++ ) {
		if ( test_lists(4096*random()/RAND_MAX) ) return 1;
	}

	printf("\r");

	return 0;
}