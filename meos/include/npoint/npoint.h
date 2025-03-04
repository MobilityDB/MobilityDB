/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief Network-based static point/segments
 */

#ifndef __NPOINT_H__
#define __NPOINT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_npoint.h>
// #include "npoint/tnpoint.h"

/*****************************************************************************/

/* General functions */

extern int32_t get_srid_ways(void);
extern GSERIALIZED *npointarr_geom(Npoint **points, int nelems);
extern GSERIALIZED *nsegmentarr_geom(Nsegment **segments, int nelems);
extern Nsegment **nsegmentarr_normalize(Nsegment **segments, int *nelems);

/* Input/output functions */

extern Npoint *npoint_in(const char *str);
extern char *npoint_out(const Npoint *np, int maxdd);

extern Nsegment *nsegment_in(const char *str);
extern char *nsegment_out(const Nsegment *ns, int maxdd);

/* Constructor functions */

extern void npoint_set(int64 rid, double pos, Npoint *np);
extern void nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns);

/*****************************************************************************/

#endif /* __NPOINT_H__ */
