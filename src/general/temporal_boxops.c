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
 * @file temporal_boxops.c
 * @brief Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are
 * - a `Period` for temporal Booleans
 * - a `TBOX` for temporal integers and floats, where the *x* coordinate is for
 *   the value dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`, and `adjacent`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "general/temporal_boxops.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/rangetypes_ext.h"
#include "general/tbox.h"
#include "point/tpoint.h"
#include "point/stbox.h"
#include "point/tpoint_boxops.h"
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/

/**
 * Return the size in bytes to read from toast to get the basic
 * information from a temporal: Temporal struct (i.e., TInstant,
 * TInstantSet, TSequence, or TSequenceSet) and bounding box size
*/
uint32_t
temporal_max_header_size(void)
{
  size_t sz1 = Max(sizeof(TInstant), sizeof(TInstantSet));
  size_t sz2 = Max(sizeof(TSequence), sizeof(TSequenceSet));
  return double_pad(Max(sz1, sz2)) + double_pad(sizeof(bboxunion));
}

/**
 * Return true if the bounding boxes are equal
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
bool
temporal_bbox_eq(const void *box1, const void *box2, CachedType temptype)
{
  /* Only external types have bounding box */
  ensure_temporal_type(temptype);
  if (talpha_type(temptype))
    return period_eq((Period *) box1, (Period *) box2);
  if (tnumber_type(temptype))
    return tbox_eq((TBOX *) box1, (TBOX *) box2);
  if (tspatial_type(temptype))
    // TODO Due to floating point precision the current statement
    // is not equal to the next one.
    // return stbox_eq((STBOX *) box1, (STBOX *) box2);
    // Problem raised in the test file 51_tpoint_tbl.test.out
    // Look for temp != merge in that file for 2 other cases where
    // a problem still remains (result != 0) even with the _cmp function
    return stbox_cmp((STBOX *) box1, (STBOX *) box2) == 0;
  elog(ERROR, "unknown bounding box function for temporal type: %d", temptype);
}

/**
 * Return -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal, or greater than the second one
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, CachedType temptype)
{
  /* Only external types have bounding box */
  ensure_temporal_type(temptype);
  if (talpha_type(temptype))
    return period_cmp((Period *) box1, (Period *) box2);
  if (tnumber_type(temptype))
    return tbox_cmp((TBOX *) box1, (TBOX *) box2);
  if (tspatial_type(temptype))
    return stbox_cmp((STBOX *) box1, (STBOX *) box2);
  elog(ERROR, "unknown bounding box function for temporal type: %d", temptype);
}

/**
 * Shift and/or scale the time span of the bounding box with the two intervals
 *
 * @param[in] box Bounding box
 * @param[in] start Interval to shift
 * @param[in] duration Interval to scale
 * @param[in] temptype Temporal type
 */
void
temporal_bbox_shift_tscale(void *box, const Interval *start,
  const Interval *duration, CachedType temptype)
{
  ensure_temporal_type(temptype);
  if (talpha_type(temptype))
    period_shift_tscale(start, duration, (Period *) box);
  else if (tnumber_type(temptype))
    tbox_shift_tscale(start, duration, (TBOX *) box);
  else if (tspatial_type(temptype))
    stbox_shift_tscale(start, duration, (STBOX *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      temptype);
  return;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2,
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/**
 * Return true if the base type does not have bounding box
 */
static bool
temptype_without_bbox(CachedType temptype)
{
  if (temptype == T_TDOUBLE2 || temptype == T_TDOUBLE3 ||
      temptype == T_TDOUBLE4)
    return true;
  return false;
}

/**
 * Return the size of the bounding box
 */
size_t
temporal_bbox_size(CachedType temptype)
{
  if (talpha_type(temptype))
    return sizeof(Period);
  if (tnumber_type(temptype))
    return sizeof(TBOX);
  if (tspatial_type(temptype))
    return sizeof(STBOX);
  /* Types without bounding box, such as tdoubleN, must be explicity stated */
  if (temptype_without_bbox(temptype))
    return 0;
  elog(ERROR, "unknown temporal_bbox_size function for temporal type: %d",
    temptype);
}

/**
 * Set the bounding box from a temporal instant value
 *
 * @param[in] box Bounding box
 * @param[in] inst Temporal value
 */
void
tinstant_make_bbox(const TInstant *inst, void *box)
{
  /* Only external types have bounding box */
  ensure_temporal_type(inst->temptype);
  memset(box, 0, temporal_bbox_size(inst->temptype));
  if (talpha_type(inst->temptype))
    period_set(inst->t, inst->t, true, true, (Period *) box);
  else if (tnumber_type(inst->temptype))
  {
    double dvalue = tnumberinst_double(inst);
    tbox_set(true, true, dvalue, dvalue, inst->t, inst->t, (TBOX *) box);
  }
  else if (tgeo_type(inst->temptype))
    tpointinst_stbox(inst, (STBOX *) box);
  else if (inst->temptype == T_TNPOINT)
    tnpointinst_make_stbox(inst, (STBOX *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      inst->temptype);
  return;
}

/**
 * Set the temporal box from the array of temporal number instant values
 *
 * @param[in] box Box
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tnumberinstarr_tbox(const TInstant **instants, int count, TBOX *box)
{
  tinstant_make_bbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    TBOX box1;
    tinstant_make_bbox(instants[i], &box1);
    tbox_expand(&box1, box);
  }
  return;
}

/**
 * Set the bounding box from the array of temporal instant values
 * (dispatch function)
 *
 * @param[in] box Box
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 */
void
tinstantset_make_bbox(const TInstant **instants, int count, void *box)
{
  /* Only external types have bounding box */
  ensure_temporal_type(instants[0]->temptype);
  if (talpha_type(instants[0]->temptype))
    period_set(instants[0]->t, instants[count - 1]->t, true, true,
      (Period *) box);
  else if (tnumber_type(instants[0]->temptype))
    tnumberinstarr_tbox(instants, count, (TBOX *) box);
  else if (tgeo_type(instants[0]->temptype))
    tgeompointinstarr_stbox(instants, count, (STBOX *) box);
  else if (instants[0]->temptype == T_TNPOINT)
    tnpointinstarr_stbox(instants, count, (STBOX *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      instants[0]->temptype);
  return;
}

/**
 * Set the bounding box from the array of temporal instant values
 * (dispatch function)
 *
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 * @param[in] linear True when the interpolation is linear
 * @param[out] box Bounding box
 */
void
tsequence_make_bbox(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear, void *box)
{
  /* Only external types have bounding box */
  ensure_temporal_type(instants[0]->temptype);
  if (talpha_type(instants[0]->temptype))
    period_set(instants[0]->t, instants[count - 1]->t, lower_inc, upper_inc,
      (Period *) box);
  else if (tnumber_type(instants[0]->temptype))
    tnumberinstarr_tbox(instants, count, (TBOX *) box);
  else if (instants[0]->temptype == T_TGEOMPOINT)
    tgeompointinstarr_stbox(instants, count, (STBOX *) box);
  else if (instants[0]->temptype == T_TGEOGPOINT)
    tgeogpointinstarr_stbox(instants, count, (STBOX *) box);
  else if (instants[0]->temptype == T_TNPOINT)
    tnpointseq_make_stbox(instants, count, linear, (STBOX *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      instants[0]->temptype);
  return;
}

/**
 * Set the period from the array of temporal sequence values
 *
 * @param[in] period Period
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tseqarr_to_period(const TSequence **sequences, int count,
  Period *period)
{
  const Period *first = &sequences[0]->period;
  const Period *last = &sequences[count - 1]->period;
  period_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
    period);
  return;
}

/**
 * Set the temporal box from the array of temporal number sequence values
 *
 * @param[in] box Box
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tnumberseqarr_to_tbox(const TSequence **sequences, int count,
  TBOX *box)
{
  memcpy(box, tsequence_bbox_ptr(sequences[0]), sizeof(TBOX));
  for (int i = 1; i < count; i++)
  {
    const TBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    tbox_expand(box1, box);
  }
  return;
}

/**
 * Set the bounding box from the array of temporal sequence values
 * (dispatch function)
 */
void
tsequenceset_make_bbox(const TSequence **sequences, int count, void *box)
{
  /* Only external types have bounding box */ // TODO
  ensure_temporal_type(sequences[0]->temptype);
  if (talpha_type(sequences[0]->temptype))
    tseqarr_to_period(sequences, count, (Period *) box);
  else if (tnumber_type(sequences[0]->temptype))
    tnumberseqarr_to_tbox(sequences, count, (TBOX *) box);
  else if (tspatial_type(sequences[0]->temptype))
    tpointseqarr_stbox(sequences, count, (STBOX *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      sequences[0]->temptype);
  return;
}

/*****************************************************************************
 * Bounding box operators for temporal types: Generic functions
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * Generic bounding box operator for a temporal value and a timestamp.
 *
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] func Bounding box function
 * @param[in] invert True when the timestamp is the first argument of the
 * function
 */
Datum
boxop_temporal_timestamp(const Temporal *temp, TimestampTz t,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1, p2;
  temporal_period(temp, &p1);
  period_set(t, t, true, true, &p2);
  bool result = invert ? func(&p2, &p1) : func(&p1, &p2);
  return result;
}

/**
 * Generic bounding box operator for a period and a temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] ts Timestamp set
 * @param[in] func Bounding box function
 * @param[in] invert True when the timestamp set is the first argument of the
 * function
 */
Datum
boxop_temporal_timestampset(const Temporal *temp, const TimestampSet *ts,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1, p2;
  temporal_period(temp, &p1);
  timestampset_period(ts, &p2);
  bool result = invert ? func(&p2, &p1) : func(&p1, &p2);
  return result;
}

/**
 * Generic bounding box operator for a period and a temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] p Period
 * @param[in] func Bounding box function
 * @param[in] invert True when the period is the first argument of the
 * function
 */
Datum
boxop_temporal_period(const Temporal *temp, const Period *p,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1;
  temporal_period(temp, &p1);
  bool result = invert ? func(p, &p1) : func(&p1, p);
  return result;
}

/**
 * Generic bounding box operator for a temporal value and a periodset
 *
 * @param[in] temp Temporal value
 * @param[in] ps Period set
 * @param[in] func Bounding box function
 * @param[in] invert True when the period set is the first argument of the
 * function
 */
bool
boxop_temporal_periodset(const Temporal *temp, const PeriodSet *ps,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1, p2;
  temporal_period(temp, &p1);
  periodset_period(ps, &p2);
  bool result = invert ? func(&p2, &p1) : func(&p1, &p2);
  return result;
}

/**
 * Generic bounding box operator for two temporal values
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Bounding box function
 */
bool
boxop_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const Period *, const Period *))
{
  Period p1, p2;
  temporal_period(temp1, &p1);
  temporal_period(temp2, &p2);
  bool result = func(&p1, &p2);
  return result;
}

/*****************************************************************************
 * Bounding box operators for temporal number types: Generic functions
 *****************************************************************************/

/**
 * Generic bounding box operator for a temporal number and a number
 *
 * @param[in] temp Temporal number
 * @param[in] number Type
 * @param[in] basetype Base type value
 * @param[in] func Bounding box function
 * @param[in] invert True when the base value is the first argument of the
 * function
 */
bool
boxop_tnumber_number(const Temporal *temp, Datum number, CachedType basetype,
  bool (*func)(const TBOX *, const TBOX *), bool invert)
{
  TBOX box1, box2;
  temporal_bbox(temp, &box1);
  number_tbox(number, basetype, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result;
}

/**
 * Generic bounding box operator for a temporal number and a range
 *
 * @param[in] temp Temporal number
 * @param[in] range Range
 * @param[in] func Bounding box function
 * @param[in] invert True when the range is the first argument of the function.
 */
int
boxop_tnumber_range(const Temporal *temp, const RangeType *range,
  bool (*func)(const TBOX *, const TBOX *), bool invert)
{
  /* Return false on empty range excepted for contains */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    return -1;
  TBOX box1, box2;
  temporal_bbox(temp, &box1);
  range_tbox(range, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return (result ? 1 : 0);
}

/**
 * Generic bounding box operator for a temporal number and a temporal box
 *
 * @param[in] temp Temporal number
 * @param[in] box Bounding box
 * @param[in] invert True when the bounding box is the first argument of the
 * function
 * @param[in] func Bounding box function
 */
bool
boxop_tnumber_tbox(const Temporal *temp, const TBOX *box,
  bool (*func)(const TBOX *, const TBOX *), bool invert)
{
  TBOX box1;
  temporal_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result;
}

/**
 * Generic bounding box operator for two temporal numbers
 *
 * @param[in] temp1,temp2 Temporal numbers
 * @param[in] func Bounding box function
 */
bool
boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBOX *, const TBOX *))
{
  TBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Bounding box operators for temporal types: Generic functions
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * Generic bounding box operator for a timestamp and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_timestamp_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_timestamp(temp, t, func, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal value and a timestamp
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_temporal_timestamp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = boxop_temporal_timestamp(temp, t, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a timestampset and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_timestampset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_timestampset(temp, ts, func, true);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal value and a timestampset
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_temporal_timestampset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = boxop_temporal_timestampset(temp, ts, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a period and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_period_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_period(temp, p, func, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal value and a period
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_temporal_period_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  bool result = boxop_temporal_period(temp, p, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a periodset and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_periodset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_periodset(temp, ps, func, true);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal value and a periodset
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_temporal_periodset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = boxop_temporal_periodset(temp, ps, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for two temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_temporal_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_temporal(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box operators for temporal types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_timestamp_temporal);
/**
 * Return true if the timestamp contains the bounding period of the temporal
 * value
 */
PGDLLEXPORT Datum
Contains_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_temporal_timestamp);
/**
 * Return true if the bounding period of the temporal value contains the
 * timestamp
 */
PGDLLEXPORT Datum
Contains_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_timestampset_temporal);
/**
 * Return true if the bounding period of the timestampset contains the bounding
 * period of the temporal value
 */
PGDLLEXPORT Datum
Contains_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_temporal_timestampset);
/**
 * Return true if the bounding period of the temporal value contains the
 * bounding period of the timestampset
 */
PGDLLEXPORT Datum
Contains_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_period_temporal);
/**
 * Return true if the period contains the bounding period of the temporal value
 */
PGDLLEXPORT Datum
Contains_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_temporal_period);
/**
 * Return true if the bounding period of the temporal value contains the period
 */
PGDLLEXPORT Datum
Contains_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_periodset_temporal);
/**
 * Return true if the bounding period of the periodset contains the bounding
 * period of the temporal value
 */
PGDLLEXPORT Datum
Contains_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_temporal_periodset);
/**
 * Return true if the bounding period of the temporal value contains the
 * bounding period of the periodset
 */
PGDLLEXPORT Datum
Contains_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &contains_period_period);
}

PG_FUNCTION_INFO_V1(Contains_temporal_temporal);
/**
 * Return true if the bounding period of the first temporal value contains
 * the bounding period of the second one.
 */
PGDLLEXPORT Datum
Contains_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &contains_period_period);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_timestamp_temporal);
/**
 * Return true if the timestamp is contained in the bounding period of the
 * temporal value
 */
PGDLLEXPORT Datum
Contained_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_temporal_timestamp);
/**
 * Return true if the bounding period of the temporal value is contained in
 * the timestamp
 */
PGDLLEXPORT Datum
Contained_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_temporal);
/**
 * Return true if the bounding period of the timestampset is contained in the
 * bounding period of the temporal value overlap
 */
PGDLLEXPORT Datum
Contained_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_temporal_timestampset);
/**
 * Return true if the bounding period of the temporal value is contained in
 * the bounding period of the timestampset
 */
PGDLLEXPORT Datum
Contained_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_period_temporal);
/**
 * Return true if the period is contained the bounding period of the temporal
 * value
 */
PGDLLEXPORT Datum
Contained_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_temporal_period);
/**
 * Return true if the bounding period of the temporal value is contained in
 * the period
 */
PGDLLEXPORT Datum
Contained_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_periodset_temporal);
/**
 * Return true if the bounding period of the periodset is contained in the
 * bounding period of the temporal value
 */
PGDLLEXPORT Datum
Contained_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_temporal_periodset);
/**
 * Return true if the bounding period of the temporal value is contained in
 * the bounding period of the periodset
 */
PGDLLEXPORT Datum
Contained_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &contained_period_period);
}

PG_FUNCTION_INFO_V1(Contained_temporal_temporal);
/**
 * Return true if the bounding period of the first temporal value is contained
 * in the bounding period of the second one.
 */
PGDLLEXPORT Datum
Contained_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &contained_period_period);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_timestamp_temporal);
/**
 * Return true if the timestamp and the bounding period of the temporal value
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_temporal_timestamp);
/**
 * Return true if the bounding period of the temporal value and the timestamp
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_timestampset_temporal);
/**
 * Return true if the bounding period the timestampset and the bounding period
 * of the temporal value overlap
 */
PGDLLEXPORT Datum
Overlaps_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_temporal_timestampset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period the timestampset overlap
 */
PGDLLEXPORT Datum
Overlaps_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_period_temporal);
/**
 * Return true if the period and the bounding period of the temporal value
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_temporal_period);
/**
 * Return true if the bounding period of the temporal value and the period
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_periodset_temporal);
/**
 * Return true if the bounding period the periodset and the bounding period of
 * the temporal value overlap
 */
PGDLLEXPORT Datum
Overlaps_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_temporal_periodset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period the periodset overlap
 */
PGDLLEXPORT Datum
Overlaps_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &overlaps_period_period);
}

PG_FUNCTION_INFO_V1(Overlaps_temporal_temporal);
/**
 * Return true if the bounding periods of the temporal values overlap
 */
PGDLLEXPORT Datum
Overlaps_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &overlaps_period_period);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Same_timestamp_temporal);
/**
 * Return true if the timestamp and the bounding period of the temporal value
 * are equal
 */
PGDLLEXPORT Datum
Same_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_temporal_timestamp);
/**
 * Return true if the bounding period of the temporal value and the timestamp
 * are equal
 */
PGDLLEXPORT Datum
Same_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_timestampset_temporal);
/**
 * Return true if the bounding period of the timestampset and the bounding
 * period of the temporal value are equal
 */
PGDLLEXPORT Datum
Same_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_temporal_timestampset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period of the timestampset are equal
 */
PGDLLEXPORT Datum
Same_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_period_temporal);
/**
 * Return true if the period and the bounding period of the temporal value
 * are equal
 */
PGDLLEXPORT Datum
Same_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_temporal_period);
/**
 * Return true if the bounding period of the temporal value and the period
 * are equal
 */
PGDLLEXPORT Datum
Same_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_periodset_temporal);
/**
 * Return true if the bounding period of the periodset and the bounding period
 * of the temporal value are equal
 */
PGDLLEXPORT Datum
Same_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_temporal_periodset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period of the periodset are equal
 */
PGDLLEXPORT Datum
Same_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &period_eq);
}

PG_FUNCTION_INFO_V1(Same_temporal_temporal);
/**
 * Return true if the bounding periods of the temporal values are equal
 */
PGDLLEXPORT Datum
Same_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &period_eq);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_timestamp_temporal);
/**
 * Return true if the timestamp and the bounding period of the temporal value
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_temporal_timestamp);
/**
 * Return true if the bounding period of the temporal value and the timestamp
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_timestampset_temporal);
/**
 * Return true if the bounding period of the timestampset and the bounding
 * period of the temporal value are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_temporal_timestampset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period of the timestampset are adjacent
 */
PGDLLEXPORT Datum
Adjacent_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_period_temporal);
/**
 * Return true if the period and the bounding period of the temporal value
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_temporal_period);
/**
 * Return true if the bounding period of the temporal value and the period
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_temporal);
/**
 * Return true if the bounding period of the periodset and the bounding period
 * of the temporal value are adjacent
 */
PGDLLEXPORT Datum
Adjacent_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_temporal_periodset);
/**
 * Return true if the bounding period of the temporal value and the bounding
 * period of the periodset are adjacent
 */
PGDLLEXPORT Datum
Adjacent_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &adjacent_period_period);
}

PG_FUNCTION_INFO_V1(Adjacent_temporal_temporal);
/**
 * Return true if the bounding periods of the temporal values are adjacent
 */
PGDLLEXPORT Datum
Adjacent_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &adjacent_period_period);
}

/*****************************************************************************
 * Bounding box operators for temporal number types: Generic functions
 *****************************************************************************/

/**
 * Generic bounding box operator for a number and a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_number_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = boxop_tnumber_number(temp, value, basetype, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal number and a number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tnumber_number_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = boxop_tnumber_number(temp, value, basetype, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a range and a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_range_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = boxop_tnumber_range(temp, range, func, INVERT);
  PG_FREE_IF_COPY(range, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal number and a range
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tnumber_range_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  RangeType *range = PG_GETARG_RANGE_P(1);
  int result = boxop_tnumber_range(temp, range, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(range, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal box and a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tbox_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnumber_tbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal number and a temporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tnumber_tbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  bool result = boxop_tnumber_tbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for two temporal numbers
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tnumber_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnumber_tnumber(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box operators for temporal numbers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_number_tnumber);
/**
 * Return true if the range contains the bounding box of the temporal number
 */
PGDLLEXPORT Datum
Contains_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_tnumber_number);
/**
 * Return true if the bounding box of the temporal number contains the
 * the range
 */
PGDLLEXPORT Datum
Contains_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_range_tnumber);
/**
 * Return true if the range contains the bounding box of the temporal number
 */
PGDLLEXPORT Datum
Contains_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_tnumber_range);
/**
 * Return true if the bounding box of the temporal number contains the range
 */
PGDLLEXPORT Datum
Contains_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_tbox_tnumber);
/**
 * Return true if the temporal box contains the bounding box of the
 * temporal number
 */
PGDLLEXPORT Datum
Contains_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_tnumber_tbox);
/**
 * Return true if the bounding box of the temporal number contains the temporal
 * box
 */
PGDLLEXPORT Datum
Contains_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &contains_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contains_tnumber_tnumber);
/**
 * Return true if the bounding box of the first temporal number contains the one
 * of the second temporal number
 */
PGDLLEXPORT Datum
Contains_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &contains_tbox_tbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_number_tnumber);
/**
 * Return true if the range is contained in the bounding box of the temporal
 * number
 */
PGDLLEXPORT Datum
Contained_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_tnumber_number);
/**
 * Return true if the bounding box of the temporal number is contained in the
 * the range
 */
PGDLLEXPORT Datum
Contained_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_range_tnumber);
/**
 * Return true if the range is contained in the bounding box of the temporal
 * number
 */
PGDLLEXPORT Datum
Contained_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_tnumber_range);
/**
 * Return true if the bounding box of the temporal number is contained in
 * the range
 */
PGDLLEXPORT Datum
Contained_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_tbox_tnumber);
/**
 * Return true if the temporal box is contained in the bounding box
 * of the temporal number
 */
PGDLLEXPORT Datum
Contained_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_tnumber_tbox);
/**
 * Return true if the bounding box of the temporal number is contained in
 * the temporal box
 */
PGDLLEXPORT Datum
Contained_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &contained_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Contained_tnumber_tnumber);
/**
 * Return true if the bounding box of the first temporal number is contained
 * in the one of the second temporal number
 */
PGDLLEXPORT Datum
Contained_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &contained_tbox_tbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_number_tnumber);
/**
 * Return true if the number and the bounding box of the temporal number
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnumber_number);
/**
 * Return true if the bounding box of the temporal number and the number
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_range_tnumber);
/**
 * Return true if the range and the bounding box of the temporal number
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnumber_range);
/**
 * Return true if the bounding box of the temporal number and the range
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tbox_tnumber);
/**
 * Return true if the temporal box and the bounding box of the temporal number
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnumber_tbox);
/**
 * Return true if the bounding box of the temporal number and the temporal box
 * overlap
 */
PGDLLEXPORT Datum
Overlaps_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overlaps_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnumber_tnumber);
/**
 * Return true if the bounding boxes of the temporal numbers overlap
 */
PGDLLEXPORT Datum
Overlaps_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overlaps_tbox_tbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Same_number_tnumber);
/**
 * Return true if the number and the bounding box of the temporal number are
 * equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_tnumber_number);
/**
 * Return true if the bounding box of the temporal number and the number are
 * equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_range_tnumber);
/**
 * Return true if the range and the bounding box of the temporal number are
 * equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_tnumber_range);
/**
 * Return true if the bounding box of the temporal number and the range are
 * equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_tbox_tnumber);
/**
 * Return true if the temporal box and the bounding box of the temporal number
 * are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_tnumber_tbox);
/**
 * Return true if the bounding box of the temporal number and the temporal box
 * are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &same_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Same_tnumber_tnumber);
/**
 * Return true if the bounding boxes of the temporal numbers are equal in the
 * common dimensions
 */
PGDLLEXPORT Datum
Same_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &same_tbox_tbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_number_tnumber);
/**
 * Return true if the number and the bounding box of the temporal number are
 * adjacent
 */
PGDLLEXPORT Datum
Adjacent_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnumber_number);
/**
 * Return true if the bounding box of the temporal number and the number are
 * adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_range_tnumber);
/**
 * Return true if the range and the bounding box of the temporal number are
 * adjacent
 */
PGDLLEXPORT Datum
Adjacent_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnumber_range);
/**
 * Return true if the bounding box of the temporal number and the range are
 * adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tbox_tnumber);
/**
 * Return true if the temporal box and the bounding box of the temporal number
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnumber_tbox);
/**
 * Return true if the bounding box of the temporal number and the temporal box
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &adjacent_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnumber_tnumber);
/**
 * Return true if the bounding boxes of the temporal numbers are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &adjacent_tbox_tbox);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
