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
extern ArrayType *nsegmentarr_to_array(Nsegment **nsegmentarr, int count);
extern int32_t get_srid_ways();
extern Datum npointarr_geom(Npoint **points, int count);
extern Datum nsegmentarr_geom(Nsegment **segments, int count);
extern Nsegment **nsegmentarr_normalize(Nsegment **segments, int *count);

/* Input/Output functions */

extern char *npoint_to_string(const Npoint *np);
extern Npoint *npoint_read(StringInfo buf);
extern void npoint_write(const Npoint *np, StringInfo buf);

/* Constructor functions */

extern Npoint *npoint_make(int64 rid, double pos);
extern void npoint_set(int64 rid, double pos, Npoint *np);
extern Nsegment *nsegment_make(int64 rid, double pos1, double pos2);
extern void nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns);

/* Cast functions */

extern Nsegment *npoint_nsegment(const Npoint *np);

/* Input/output functions */

extern char *nsegment_to_string(Nsegment *ns);
extern Nsegment *nsegment_read(StringInfo buf);
extern void nsegment_write(Nsegment *ns, StringInfo buf);

/* Accessor functions */

extern int64 npoint_route(Npoint *np);
extern double npoint_position(Npoint *np);
extern int64 nsegment_route(Nsegment *ns);
extern double nsegment_start_position(Nsegment *ns);
extern double nsegment_end_position(Nsegment *ns);

/* Transformation functions */

extern Datum datum_npoint_round(Datum npoint, Datum size);
extern Npoint *npoint_round(Npoint *np, Datum size);
extern Nsegment *nsegment_round(Nsegment *ns, Datum size);

/* Conversions between network and Euclidean space */

extern bool route_exists(int64 rid);
extern double route_length(int64 rid);
extern Datum route_geom(int64 rid);
extern Datum npoint_geom(const Npoint *np);
extern Npoint *geom_npoint(Datum geom);
extern Datum nsegment_geom(const Nsegment *ns);
extern Nsegment *geom_nsegment(Datum line);

/* SRID functions */

extern int npoint_srid(const Npoint *np);
extern int nsegment_srid(const Nsegment *ns);

/* Comparison functions */

extern bool npoint_eq(const Npoint *np1, const Npoint *np2);
extern bool npoint_ne(const Npoint *np1, const Npoint *np2);
extern int npoint_cmp(const Npoint *np1, const Npoint *np2);
extern bool npoint_lt(const Npoint *np1, const Npoint *np2);
// extern bool npoint_le(const Npoint *np1, const Npoint *np2);
// extern bool npoint_gt(const Npoint *np1, const Npoint *np2);
// extern bool npoint_ge(const Npoint *np1, const Npoint *np2);

extern bool nsegment_eq(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ne(const Nsegment *ns1, const Nsegment *ns2);
extern int nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2);
// extern bool nsegment_lt(const Nsegment *ns1, const Nsegment *ns2);
// extern bool nsegment_le(const Nsegment *ns1, const Nsegment *ns2);
// extern bool nsegment_gt(const Nsegment *ns1, const Nsegment *ns2);
// extern bool nsegment_ge(const Nsegment *ns1, const Nsegment *ns2);

/*****************************************************************************/

#endif /* __TNPOINT_STATIC_H__ */
