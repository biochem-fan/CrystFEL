/*
 * scaling-report.h
 *
 * Write a nice PDF of scaling parameters
 *
 * Copyright © 2012-2014 Deutsches Elektronen-Synchrotron DESY,
 *                       a research centre of the Helmholtz Association.
 *
 * Authors:
 *   2011-2014 Thomas White <taw@physics.org>
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

#ifndef SCALING_REPORT_H
#define SCALING_REPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "utils.h"

typedef struct _srcontext SRContext;  /* Opaque */

/* Information is logged in this structure */
struct srdata
{
	Crystal **crystals;
	int n;
	RefList *full;

	int n_filtered;
	int n_refined;
};

#if defined(HAVE_CAIRO) && defined(HAVE_PANGO) && defined(HAVE_PANGOCAIRO)

extern SRContext *sr_titlepage(Crystal **crystals, int n,
                               const char *filename,
                               const char *stream_filename,
                               const char *cmdline);

extern void sr_iteration(SRContext *sr, int iteration, struct srdata *d);

extern void sr_finish(SRContext *sr);

#else /* defined(HAVE_CAIRO) && defined(HAVE_PANGO) && ... */

SRContext *sr_titlepage(Crystal **crystals, int n, const char *filename,
                        const char *stream_filename, const char *cmdline)
{
	return NULL;
}

void sr_iteration(SRContext *sr, int iteration, struct srdata *d)
{
}

void sr_finish(SRContext *sr)
{
}

#endif /* defined(HAVE_CAIRO) && defined(HAVE_PANGO) && ... */


#endif	/* SCALING_REPORT_H */
