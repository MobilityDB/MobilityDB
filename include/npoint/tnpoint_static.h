/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tnpoint_static.h
 * Network-based static point/segments
 */

#ifndef __TNPOINT_STATIC_H__
#define __TNPOINT_STATIC_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "npoint/tnpoint.h"

/*****************************************************************************/

/* General functions */

extern ArrayType *int64arr_to_array(const int64 *int64arr, int count);
extern ArrayType *nsegmentarr_to_array(nsegment **nsegmentarr, int count);
extern int32_t get_srid_ways();
extern Datum npointarr_geom(npoint **points, int count);
extern Datum nsegmentarr_geom(nsegment **segments, int count);
extern nsegment **nsegmentarr_normalize(nsegment **segments, int *count);

/* Input/Output functions for npoint */


/* Input/Output functions for nsegment */


/* Constructor functions */

extern npoint *npoint_make(int64 rid, double pos);
extern nsegment *nsegment_make(int64 rid, double pos1, double pos2);

/* Accessor functions */


/* Transformation functions */


extern Datum npoint_round(Datum npoint, Datum size);

/* Conversions between network and Euclidean space */

extern bool route_exists(int64 rid);
extern double route_length(int64 rid);
extern Datum route_geom(int64 rid);
extern Datum npoint_geom(const npoint *np);
extern npoint *geom_npoint(Datum geom);
extern Datum nsegment_geom(const nsegment *ns);
extern nsegment *geom_nsegment(Datum line);

/* SRID functions */

extern int npoint_srid(const npoint *np);
extern int nsegment_srid(const nsegment *ns);

/* Comparison functions */

extern bool npoint_eq(const npoint *np1, const npoint *np2);
extern bool npoint_ne(const npoint *np1, const npoint *np2);
extern int npoint_cmp(const npoint *np1, const npoint *np2);
extern bool npoint_lt(const npoint *np1, const npoint *np2);
// extern bool npoint_le(const npoint *np1, const npoint *np2);
// extern bool npoint_gt(const npoint *np1, const npoint *np2);
// extern bool npoint_ge(const npoint *np1, const npoint *np2);

extern bool nsegment_eq(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_ne(const nsegment *ns1, const nsegment *ns2);
extern int nsegment_cmp(const nsegment *ns1, const nsegment *ns2);
// extern bool nsegment_lt(const nsegment *ns1, const nsegment *ns2);
// extern bool nsegment_le(const nsegment *ns1, const nsegment *ns2);
// extern bool nsegment_gt(const nsegment *ns1, const nsegment *ns2);
// extern bool nsegment_ge(const nsegment *ns1, const nsegment *ns2);

/*****************************************************************************/

#endif /* __TNPOINT_STATIC_H__ */
