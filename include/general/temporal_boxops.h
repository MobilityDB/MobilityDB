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
 * @file temporal_boxops.h
 * Bounding box operators for temporal types.
 */

#ifndef __TEMPORAL_BOXOPS_H__
#define __TEMPORAL_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/temporal.h"
#include "general/period.h"
#include "general/tbox.h"
#include "point/stbox.h"

/*****************************************************************************/

/* Functions on generic bounding boxes of temporal types */

extern size_t temporal_max_bbox_size();
extern uint32_t temporal_max_header_size(void);
extern bool temporal_bbox_eq(const void *box1, const void *box2,
  CachedType temptype);
extern int temporal_bbox_cmp(const void *box1, const void *box2,
  CachedType temptype);
extern void temporal_bbox_shift_tscale(void *box, const Interval *start,
  const Interval *duration, CachedType temptype);

/* Compute the bounding box at the creation of temporal values */

extern size_t temporal_bbox_size(CachedType tempype);
extern void tinstant_make_bbox(const TInstant *inst, void *bbox);
extern void tinstantset_make_bbox(const TInstant **inst, int count,
  void *bbox);
extern void tsequence_make_bbox(const TInstant** inst, int count,
  bool lower_inc, bool upper_inc, bool linear, void *bbox);
extern void tsequenceset_make_bbox(const TSequence **seqs, int count,
  void *bbox);

/* Bounding box operators for temporal types */

extern Datum boxop_temporal_timestamp(const Temporal *temp, TimestampTz t,
  bool (*func)(const Period *, const Period *), bool invert);
extern Datum boxop_temporal_timestampset(const Temporal *temp,
  const TimestampSet *ts, bool (*func)(const Period *, const Period *),
  bool invert);
extern Datum boxop_temporal_period(const Temporal *temp, const Period *p,
  bool (*func)(const Period *, const Period *), bool invert);
extern bool boxop_temporal_periodset(const Temporal *temp, const PeriodSet *ps,
  bool (*func)(const Period *, const Period *), bool invert);
extern bool boxop_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, bool (*func)(const Period *, const Period *));

extern Datum boxop_timestamp_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_timestamp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_timestampset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_timestampset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_period_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_period_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_periodset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_periodset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));
extern Datum boxop_temporal_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *));

extern bool boxop_tnumber_number(const Temporal *temp, Datum value,
  CachedType basetype, bool (*func)(const TBOX *, const TBOX *), bool invert);
extern int boxop_tnumber_range(const Temporal *temp, const RangeType *range,
  bool (*func)(const TBOX *, const TBOX *), bool invert);
extern bool boxop_tnumber_tbox(const Temporal *temp, const TBOX *box,
  bool (*func)(const TBOX *, const TBOX *), bool invert);
extern bool boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBOX *, const TBOX *));

extern Datum boxop_number_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_number_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_range_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_range_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tbox_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_tbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));
extern Datum boxop_tnumber_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *));

/*****************************************************************************/

#endif
