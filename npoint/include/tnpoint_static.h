/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

#include <postgres.h>
#include <catalog/pg_type.h>

#include "tnpoint.h"

/*****************************************************************************
 * StaticObjects.c
 *****************************************************************************/

extern ArrayType *int64arr_to_array(const int64 *int64arr, int count);
extern ArrayType *npointarr_to_array(npoint **npointarr, int count);
extern void npointarr_sort(npoint **points, int count);
extern ArrayType *nsegmentarr_to_array(nsegment **nsegmentarr, int count);
extern nsegment **nsegmentarr_normalize(nsegment **segments, int *count);
extern int npoint_remove_duplicates(npoint **values, int count);

extern Datum npoint_in(PG_FUNCTION_ARGS);
extern Datum npoint_out(PG_FUNCTION_ARGS);
extern Datum npoint_recv(PG_FUNCTION_ARGS);
extern Datum npoint_send(PG_FUNCTION_ARGS);

extern Datum nsegment_in(PG_FUNCTION_ARGS);
extern Datum nsegment_out(PG_FUNCTION_ARGS);
extern Datum nsegment_recv(PG_FUNCTION_ARGS);
extern Datum nsegment_send(PG_FUNCTION_ARGS);

extern Datum npoint_constructor(PG_FUNCTION_ARGS);
extern Datum nsegment_constructor(PG_FUNCTION_ARGS);
extern Datum nsegment_from_npoint(PG_FUNCTION_ARGS);

extern Datum npoint_set_precision_internal(Datum npoint, Datum size);

extern void npoint_set(npoint *np, int64 rid, double pos);
extern npoint *npoint_make(int64 rid, double pos);
extern void nsegment_set(nsegment *ns, int64 rid, double pos1, double pos2);
extern nsegment *nsegment_make(int64 rid, double pos1, double pos2);

extern Datum npoint_route(PG_FUNCTION_ARGS);
extern Datum npoint_position(PG_FUNCTION_ARGS);
extern Datum nsegment_route(PG_FUNCTION_ARGS);
extern Datum nsegment_start_position(PG_FUNCTION_ARGS);
extern Datum nsegment_end_position(PG_FUNCTION_ARGS);

extern Datum npoint_eq(PG_FUNCTION_ARGS);
extern Datum npoint_ne(PG_FUNCTION_ARGS);
extern Datum npoint_lt(PG_FUNCTION_ARGS);
extern Datum npoint_le(PG_FUNCTION_ARGS);
extern Datum npoint_gt(PG_FUNCTION_ARGS);
extern Datum npoint_ge(PG_FUNCTION_ARGS);

extern int npoint_cmp_internal(const npoint *np1, const npoint *np2);
extern bool npoint_same_internal(const npoint *np1, const npoint *np2);
extern bool npoint_eq_internal(const npoint *np1, const npoint *np2);
extern bool npoint_ne_internal(const npoint *np1, const npoint *np2);
extern bool npoint_lt_internal(const npoint *np1, const npoint *np2);
extern bool npoint_le_internal(const npoint *np1, const npoint *np2);
extern bool npoint_gt_internal(const npoint *np1, const npoint *np2);
extern bool npoint_ge_internal(const npoint *np1, const npoint *np2);

extern Datum nsegment_eq(PG_FUNCTION_ARGS);
extern Datum nsegment_ne(PG_FUNCTION_ARGS);
extern Datum nsegment_lt(PG_FUNCTION_ARGS);
extern Datum nsegment_le(PG_FUNCTION_ARGS);
extern Datum nsegment_gt(PG_FUNCTION_ARGS);
extern Datum nsegment_ge(PG_FUNCTION_ARGS);

extern int nsegment_cmp_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_eq_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_ne_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_lt_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_le_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_gt_internal(const nsegment *ns1, const nsegment *ns2);
extern bool nsegment_ge_internal(const nsegment *ns1, const nsegment *ns2);

extern bool route_exists(int64 rid);
extern double route_length(int64 rid);
extern Datum route_geom(int64 rid);
extern int64 rid_from_geom(Datum geom);

extern int npoint_srid_internal(const npoint *np);
extern Datum npoint_as_geom(PG_FUNCTION_ARGS);
extern Datum geom_as_npoint(PG_FUNCTION_ARGS);
extern Datum nsegment_as_geom(PG_FUNCTION_ARGS);
extern Datum geom_as_nsegment(PG_FUNCTION_ARGS);

extern Datum npoint_as_geom_internal(const npoint *np);
extern Datum nsegment_as_geom_internal(const nsegment *ns);
extern npoint *geom_as_npoint_internal(Datum geom);
extern nsegment *geom_as_nsegment_internal(Datum line);

extern Datum npointarr_to_geom_internal(npoint **points, int count);
extern Datum nsegmentarr_to_geom_internal(nsegment **segments, int count);

/*****************************************************************************/

#endif /* __TNPOINT_STATIC_H__ */
