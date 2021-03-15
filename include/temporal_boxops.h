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
 * @file temporal_boxops.h
 * Bounding box operators for temporal types.
 */

#ifndef __TEMPORAL_BOXOPS_H__
#define __TEMPORAL_BOXOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>

#include "temporal.h"
#include "tbox.h"

/*****************************************************************************/

/* Functions on generic bounding boxes of temporal types */

extern size_t temporal_bbox_size(Oid valuetypid);
extern bool temporal_bbox_eq(const void *box1, const void *box2, Oid valuetypid);
extern int temporal_bbox_cmp(const void *box1, const void *box2, Oid valuetypid);
extern void temporal_bbox_expand(void *box1, const void *box2, Oid valuetypid);
extern void temporal_bbox_shift_tscale(void *box, const Interval *start,
  const Interval *duration, Oid valuetypid);

/* Compute the bounding box at the creation of temporal values */

extern void tinstant_make_bbox(void *bbox, const TInstant *inst);
extern void tinstantset_make_bbox(void *bbox, const TInstant **inst, int count);
extern void tsequence_make_bbox(void *bbox, const TInstant** inst, int count,
  bool lower_inc, bool upper_inc);
extern void tsequenceset_make_bbox(void *bbox, const TSequence **seqs, int count);

/* Restriction at/minus tbox */

extern Datum tnumber_at_tbox(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_tbox(PG_FUNCTION_ARGS);

extern Temporal *tnumber_at_tbox_internal(const Temporal *temp, const TBOX *box);
extern Temporal *tnumber_minus_tbox_internal(const Temporal *temp, const TBOX *box);

/* Bounding box operators for temporal types */

extern Datum contains_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum contained_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum overlaps_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum same_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum overlaps_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contains_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contained_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum same_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum boxop_period_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_period(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));

extern Datum boxop_range_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_range(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tbox_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_tbox(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));

/*****************************************************************************/

#endif
