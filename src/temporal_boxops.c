/*****************************************************************************
 *
 * temporal_boxops.c
 *    Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are
 * - a period for temporal Booleans
 * - a TBOX for temporal integers and floats, where the x coordinate is for
 *   the value dimension and the t coordinate is for the time dimension.
 * The following operators are defined:
 *    overlaps, contains, contained, same, adjacent
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_boxops.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "rangetypes_ext.h"
#include "tbox.h"
#include "tpoint.h"
#include "stbox.h"
#include "tpoint_boxops.h"

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/

/**
 * Returns the size of the bounding box
 */
size_t
temporal_bbox_size(Oid valuetypid)
{
  if (talpha_base_type(valuetypid))
    return sizeof(Period);
  if (tnumber_base_type(valuetypid))
    return sizeof(TBOX);
  if (tgeo_base_type(valuetypid))
    return sizeof(STBOX);
  /* Types without bounding box, for example, tdoubleN */
  return 0;
}

/**
 * Returns true if the bounding boxes are equal
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] valuetypid Oid of the base type
 */
bool
temporal_bbox_eq(const void *box1, const void *box2, Oid valuetypid)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(valuetypid);
  bool result = false;
  if (talpha_base_type(valuetypid))
    result = period_eq_internal((Period *)box1, (Period *)box2);
  else if (tnumber_base_type(valuetypid))
    result = tbox_eq_internal((TBOX *)box1, (TBOX *)box2);
  else if (tgeo_base_type(valuetypid))
    result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2) == 0;
    // TODO Due to floating point precision the previous statement
    // is not equal to the next one.
    // result = stbox_eq_internal((STBOX *)box1, (STBOX *)box2);
    // Problem raised in the test file 51_tpoint_tbl.test.out
    // Look for temp != merge in that file for 2 other cases where
    // a problem still remains (result != 0) even with the _cmp function
  /* Types without bounding box, for example, doubleN */
  return result;
}

/**
 * Returns -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal, or greater than the second one
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] valuetypid Oid of the base type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, Oid valuetypid)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(valuetypid);
  int result = 0;
  if (talpha_base_type(valuetypid))
    result = period_cmp_internal((Period *)box1, (Period *)box2);
  else if (tnumber_base_type(valuetypid))
    result = tbox_cmp_internal((TBOX *)box1, (TBOX *)box2);
  else if (tgeo_base_type(valuetypid))
    result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2);
  /* Types without bounding box, for example, doubleN */
  return result;
}

/**
 * Shift and/or scale the time span of the bounding box with the two intervals
 *
 * @param[in] box Bounding box
 * @param[in] start Interval to shift
 * @param[in] duration Interval to scale
 * @param[in] valuetypid Oid of the base type
 */
void
temporal_bbox_shift_tscale(void *box, const Interval *start,
  const Interval *duration, Oid valuetypid)
{
  ensure_temporal_base_type(valuetypid);
  if (talpha_base_type(valuetypid))
    period_shift_tscale((Period *)box, start, duration);
  else if (tnumber_base_type(valuetypid))
    tbox_shift_tscale((TBOX *)box, start, duration);
  else if (tgeo_base_type(valuetypid))
    stbox_shift_tscale((STBOX *)box, start, duration);
  return;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2,
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/**
 * Set the bounding box from a temporal instant value
 *
 * @param[in] box Bounding box
 * @param[in] inst Temporal value
 */
void
tinstant_make_bbox(void *box, const TInstant *inst)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(inst->valuetypid);
  if (talpha_base_type(inst->valuetypid))
    period_set((Period *)box, inst->t, inst->t, true, true);
  else if (tnumber_base_type(inst->valuetypid))
  {
    double dvalue = datum_double(tinstant_value(inst), inst->valuetypid);
    TBOX *result = (TBOX *)box;
    result->xmin = result->xmax = dvalue;
    result->tmin = result->tmax = inst->t;
    MOBDB_FLAGS_SET_X(result->flags, true);
    MOBDB_FLAGS_SET_T(result->flags, true);
  }
  else if (tgeo_base_type(inst->valuetypid))
    tpointinst_make_stbox((STBOX *)box, inst);
}

/**
 * Set the period from the array of temporal instant values
 *
 * @param[in] period Period
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 */
static void
tinstantarr_to_period(Period *period, TInstant **instants, int count,
  bool lower_inc, bool upper_inc)
{
  period_set(period, instants[0]->t, instants[count - 1]->t, lower_inc, upper_inc);
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
tnumberinstarr_to_tbox(TBOX *box, TInstant **instants, int count)
{
  tinstant_make_bbox(box, instants[0]);
  for (int i = 1; i < count; i++)
  {
    TBOX box1;
    memset(&box1, 0, sizeof(TBOX));
    tinstant_make_bbox(&box1, instants[i]);
    tbox_expand(box, &box1);
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
tinstantset_make_bbox(void *box, TInstant **instants, int count)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(instants[0]->valuetypid);
  if (talpha_base_type(instants[0]->valuetypid))
    tinstantarr_to_period((Period *)box, instants, count, true, true);
  else if (tnumber_base_type(instants[0]->valuetypid))
    tnumberinstarr_to_tbox((TBOX *)box, instants, count);
  else if (tgeo_base_type(instants[0]->valuetypid))
    tpointinstarr_to_stbox((STBOX *)box, instants, count);
}

/**
 * Set the bounding box from the array of temporal instant values
 * (dispatch function)
 *
 * @param[in] box Box
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 */
void
tsequence_make_bbox(void *box, TInstant **instants, int count,
  bool lower_inc, bool upper_inc)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(instants[0]->valuetypid);
  if (talpha_base_type(instants[0]->valuetypid))
    tinstantarr_to_period((Period *)box, instants, count,
      lower_inc, upper_inc);
  else if (tnumber_base_type(instants[0]->valuetypid))
    tnumberinstarr_to_tbox((TBOX *)box, instants, count);
  /* This code is currently not used since for temporal points the bounding
   * box is computed from the trajectory for efficiency reasons. It is left
   * here in case this is no longer the case
  else if (geo_base_type(instants[0]->valuetypid))
    tpointinstarr_to_stbox((STBOX *)box, instants, count);
  */
}

/**
 * Set the period from the array of temporal sequence values
 *
 * @param[in] period Period
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tsequencearr_to_period_internal(Period *period, TSequence **sequences, int count)
{
  Period *first = &sequences[0]->period;
  Period *last = &sequences[count - 1]->period;
  period_set(period, first->lower, last->upper, first->lower_inc, last->upper_inc);
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
tnumberseqarr_to_tbox_internal(TBOX *box, TSequence **sequences, int count)
{
  memcpy(box, tsequence_bbox_ptr(sequences[0]), sizeof(TBOX));
  for (int i = 1; i < count; i++)
  {
    TBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    tbox_expand(box, box1);
  }
  return;
}

/**
 * Set the bounding box from the array of temporal sequence values
 * (dispatch function)
 */
void
tsequenceset_make_bbox(void *box, TSequence **sequences, int count)
{
  /* Only external types have bounding box */
  ensure_temporal_base_type(sequences[0]->valuetypid);
  if (talpha_base_type(sequences[0]->valuetypid))
    tsequencearr_to_period_internal((Period *)box, sequences, count);
  else if (tnumber_base_type(sequences[0]->valuetypid))
    tnumberseqarr_to_tbox_internal((TBOX *)box, sequences, count);
  else if (tgeo_base_type(sequences[0]->valuetypid))
    tpointseqarr_to_stbox((STBOX *)box, sequences, count);
}

/*****************************************************************************
 * Bounding box operators for temporal types: Generic functions
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * Generic bounding box operator for a period and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_period_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Period *p = PG_GETARG_PERIOD(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Period p1;
  temporal_period(&p1, temp);
  bool result = func(p, &p1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal value and a period
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_temporal_period(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Period *p = PG_GETARG_PERIOD(1);
  Period p1;
  temporal_period(&p1, temp);
  bool result = func(&p1, p);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for two temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_temporal_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Period *, const Period *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Period p1, p2;
  temporal_period(&p1, temp1);
  temporal_period(&p2, temp2);
  bool result = func(&p1, &p2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box operators for temporal types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_period_temporal);
/**
 * Returns true if the period contains the bounding period of the temporal value
 */
PGDLLEXPORT Datum
contains_bbox_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo,
    &contains_period_period_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_period);
/**
 * Returns true if the bounding period of the temporal value contains the period
 */
PGDLLEXPORT Datum
contains_bbox_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo,
    &contains_period_period_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_temporal);
/**
 * Returns true if the bounding period of the first temporal value contains
 * the bounding period of the second one.
 */
PGDLLEXPORT Datum
contains_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo,
    &contains_period_period_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_period_temporal);
/**
 * Returns true if the period is contained the bounding period of the
 * temporal value
 */
PGDLLEXPORT Datum
contained_bbox_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo,
    &contained_period_period_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_period);
/**
 * Returns true if the bounding period of the temporal value is contained in
 * the period
 */
PGDLLEXPORT Datum
contained_bbox_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo,
    &contained_period_period_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_temporal);
/**
 * Returns true if the bounding period of the first temporal value is contained in
 * the bounding period of the second one.
 */
PGDLLEXPORT Datum
contained_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo,
    &contained_period_period_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_period_temporal);
/**
 * Returns true if the period and the bounding period of the temporal value
 * overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo,
    &overlaps_period_period_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_period);
/**
 * Returns true if the bounding period of the temporal value and the period
 * overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo,
    &overlaps_period_period_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_temporal);
/**
 * Returns true if the bounding periods of the temporal values overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo,
    &overlaps_period_period_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_period_temporal);
/**
 * Returns true if the period and the bounding period of the temporal value
 * are equal
 */
PGDLLEXPORT Datum
same_bbox_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo,
    &period_eq_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_period);
/**
 * Returns true if the bounding period of the temporal value and the period
 * are equal
 */
PGDLLEXPORT Datum
same_bbox_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo,
    &period_eq_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_temporal);
/**
 * Returns true if the bounding periods of the temporal values are equal
 */
PGDLLEXPORT Datum
same_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo,
    &period_eq_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_period_temporal);
/**
 * Returns true if the period and the bounding period of the temporal value
 * are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo,
    &adjacent_period_period_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_temporal_period);
/**
 * Returns true if the bounding period of the temporal value and the period
 * are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo,
    &adjacent_period_period_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_temporal_temporal);
/**
 * Returns true if the bounding periods of the temporal values are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo,
    &adjacent_period_period_internal);
}

/*****************************************************************************
 * Bounding box operators for temporal number types: Generic functions
 *****************************************************************************/

/**
 * Generic bounding box operator for a range and a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_range_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType  *range = PG_GETARG_RANGE(0);
#else
  RangeType  *range = PG_GETARG_RANGE_P(0);
#endif
  /* Return false on empty range excepted for contained */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    PG_RETURN_BOOL(func == &contained_tbox_tbox_internal);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  TBOX box1, box2;
  memset(&box1, 0, sizeof(TBOX));
  memset(&box2, 0, sizeof(TBOX));
  range_to_tbox_internal(&box1, range);
  temporal_bbox(&box2, temp);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(range, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal number and a range
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnumber_range(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
#if MOBDB_PGSQL_VERSION < 110000
  RangeType  *range = PG_GETARG_RANGE(1);
#else
  RangeType  *range = PG_GETARG_RANGE_P(1);
#endif
  /* Return false on empty range excepted for contains */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    PG_RETURN_BOOL(func == &contains_tbox_tbox_internal);
  TBOX box1, box2;
  memset(&box1, 0, sizeof(TBOX));
  memset(&box2, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  range_to_tbox_internal(&box2, range);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(range, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal box and a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tbox_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  TBOX box1;
  memset(&box1, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  bool result = func(box, &box1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for a temporal number and a temporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnumber_tbox(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  TBOX box1;
  memset(&box1, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box operator for two temporal numbers
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnumber_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBOX *, const TBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  TBOX box1, box2;
  memset(&box1, 0, sizeof(TBOX));
  memset(&box2, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box operators for temporal numbers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_range_tnumber);
/**
 * Returns true if the range contains the bounding box of the temporal number
 */
PGDLLEXPORT Datum
contains_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &contains_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_range);
/**
 * Returns true if the bounding box of the temporal number contains the range
 */
PGDLLEXPORT Datum
contains_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &contains_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tbox_tnumber);
/**
 * Returns true if the temporal box contains the bounding box of the
 * temporal number
 */
PGDLLEXPORT Datum
contains_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &contains_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_tbox);
/**
 * Returns true if the bounding box of the temporal number contains the temporal box
 */
PGDLLEXPORT Datum
contains_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &contains_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_tnumber);
/**
 * Returns true if the bounding box of the first temporal number contains the one
 * of the second temporal number
 */
PGDLLEXPORT Datum
contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &contains_tbox_tbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_range_tnumber);
/**
 * Returns true if the range is contained in the bounding box of the temporal number
 */
PGDLLEXPORT Datum
contained_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &contained_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_range);
/**
 * Returns true if the bounding box of the temporal number is contained in
 * the range
 */
PGDLLEXPORT Datum
contained_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &contained_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tbox_tnumber);
/**
 * Returns true if the temporal box is contained in the bounding box
 * of the temporal number
 */
PGDLLEXPORT Datum
contained_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &contained_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_tbox);
/**
 * Returns true if the bounding box of the temporal number is contained in
 * the temporal box
 */
PGDLLEXPORT Datum
contained_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &contained_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_tnumber);
/**
 * Returns true if the bounding box of the first temporal number is contained
 * in the one of the second temporal number
 */
PGDLLEXPORT Datum
contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &contained_tbox_tbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_range_tnumber);
/**
 * Returns true if the range and the bounding box of the temporal number overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &overlaps_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_range);
/**
 * Returns true if the bounding box of the temporal number and the
 * the range overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &overlaps_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tbox_tnumber);
/**
 * Returns true if the temporal box and the bounding box
 * of the temporal number overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &overlaps_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_tbox);
/**
 * Returns true if the bounding box of the temporal number and the
 * temporal box overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &overlaps_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_tnumber);
/**
 * Returns true if the bounding boxes of the temporal numbers overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &overlaps_tbox_tbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_range_tnumber);
/**
 * Returns true if the range and the bounding box of the temporal number
 * are equal on the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &same_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_range);
/**
 * Returns true if the bounding box of the temporal number and the
 * the range are equal on the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &same_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tbox_tnumber);
/**
 * Returns true if the temporal box and the bounding box
 * of the temporal number are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &same_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_tbox);
/**
 * Returns true if the bounding box of the temporal number and the
 * temporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &same_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_tnumber);
/**
 * Returns true if the bounding boxes of the temporal numbers are equal
 * in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &same_tbox_tbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_range_tnumber);
/**
 * Returns true if the range and the bounding box of the temporal number
 * are adjancent
 */
PGDLLEXPORT Datum
adjacent_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &adjacent_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_range);
/**
 * Returns true if the bounding box of the temporal number and the
 * the range are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &adjacent_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tbox_tnumber);
/**
 * Returns true if the temporal box and the bounding box
 * of the temporal number are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &adjacent_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_tbox);
/**
 * Returns true if the bounding box of the temporal number and the
 * temporal box are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &adjacent_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_tnumber);
/**
 * Returns true if the bounding boxes of the temporal numbers are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &adjacent_tbox_tbox_internal);
}
/*****************************************************************************/
