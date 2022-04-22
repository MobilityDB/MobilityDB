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
 * @file tbox.c
 * @brief Functions for temporal bounding boxes.
 */

#include "general/tbox.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporal.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"

/** Buffer size for input and output of TBOX values */
#define MAXTBOXLEN    128

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the temporal box has X values
 */
void
ensure_has_X_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have value dimension")));
}

/**
 * Ensure that the temporal box has T values
 */
void
ensure_has_T_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have time dimension")));
}

/**
 * Ensure that the temporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_tbox(const TBOX *box1, const TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must be of the same dimensionality")));
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the string representation of the temporal box.
 */
char *
tbox_to_string(const TBOX *box)
{
  static size_t size = MAXTBOXLEN + 1;
  char *result = (char *) palloc(size);
  char *xmin = NULL, *xmax = NULL, *tmin = NULL, *tmax = NULL;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  assert(hasx || hast);
  if (hasx)
  {
    xmin = call_output(FLOAT8OID, Float8GetDatum(box->xmin));
    xmax = call_output(FLOAT8OID, Float8GetDatum(box->xmax));
  }
  if (hast)
  {
    tmin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
    tmax = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
  }
  if (hasx)
  {
    if (hast)
      snprintf(result, size, "TBOX((%s,%s),(%s,%s))", xmin, tmin,
        xmax, tmax);
    else
      snprintf(result, size, "TBOX((%s,),(%s,))", xmin, xmax);
  }
  else
    /* Missing X dimension */
    snprintf(result, size, "TBOX((,%s),(,%s))", tmin, tmax);
  if (hasx)
  {
    pfree(xmin); pfree(xmax);
  }
  if (hast)
  {
    pfree(tmin); pfree(tmax);
  }
  return result;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Write the binary representation of the box value into the buffer.
 */
void
tbox_write(const TBOX *box, StringInfo buf)
{
  pq_sendbyte(buf, MOBDB_FLAGS_GET_X(box->flags) ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, MOBDB_FLAGS_GET_T(box->flags) ? (uint8) 1 : (uint8) 0);
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    pq_sendfloat8(buf, box->xmin);
    pq_sendfloat8(buf, box->xmax);
  }
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    bytea *tmin = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
    bytea *tmax = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
    pq_sendbytes(buf, VARDATA(tmin), VARSIZE(tmin) - VARHDRSZ);
    pq_sendbytes(buf, VARDATA(tmax), VARSIZE(tmax) - VARHDRSZ);
    pfree(tmin); pfree(tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Return a new box value from its binary representation read from
 * the buffer.
 */
TBOX *
tbox_read(StringInfo buf)
{
  TBOX *result = (TBOX *) palloc0(sizeof(TBOX));
  bool hasx = (char) pq_getmsgbyte(buf);
  bool hast = (char) pq_getmsgbyte(buf);
  MOBDB_FLAGS_SET_X(result->flags, hasx);
  MOBDB_FLAGS_SET_T(result->flags, hast);
  if (hasx)
  {
    result->xmin = pq_getmsgfloat8(buf);
    result->xmax = pq_getmsgfloat8(buf);
  }
  if (hast)
  {
    result->tmin = call_recv(TIMESTAMPTZOID, buf);
    result->tmax = call_recv(TIMESTAMPTZOID, buf);
  }
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_constructor
 * @brief Constructs a newly allocated temporal box.
 */
TBOX *
tbox_make(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax)
{
  /* Note: zero-fill is done in function tbox_set */
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  tbox_set(hasx, hast, xmin, xmax, tmin, tmax, result);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Set the temporal box from the argument values
 */
void
tbox_set(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  MOBDB_FLAGS_SET_X(box->flags, hasx);
  MOBDB_FLAGS_SET_T(box->flags, hast);
  if (hasx)
  {
    /* Process X min/max */
    box->xmin = Min(xmin, xmax);
    box->xmax = Max(xmin, xmax);
  }
  if (hast)
  {
    /* Process T min/max */
    box->tmin = Min(tmin, tmax);
    box->tmax = Max(tmin, tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a copy of the temporal box value.
 */
TBOX *
tbox_copy(const TBOX *box)
{
  TBOX *result = (TBOX *) palloc0(sizeof(TBOX));
  memcpy(result, box, sizeof(TBOX));
  return result;
}

/*****************************************************************************
 * Casting
 * The functions set the argument box to 0
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the number to a temporal box.
 */
void
number_tbox(Datum value, CachedType basetype, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  ensure_tnumber_basetype(basetype);
  if (basetype == T_INT4)
    box->xmin = box->xmax = (double)(DatumGetInt32(value));
  else /* basetype == T_FLOAT8 */
    box->xmin = box->xmax = DatumGetFloat8(value);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the integer to a temporal box.
 */
void
int_tbox(int i, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  box->xmin = box->xmax = (double) i;
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the float to a temporal box.
 */
void
float_tbox(double d, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  box->xmin = box->xmax = d;
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the range to a temporal box.
 */
void
range_tbox(const RangeType *range, TBOX *box)
{
  ensure_tnumber_rangetype(oid_type(range->rangetypid));
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  range_bounds(range, &box->xmin, &box->xmax);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the timestamp to a temporal box.
 */
void
timestamp_tbox(TimestampTz t, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  box->tmin = box->tmax = t;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the period set to a temporal box.
 */
void
timestampset_tbox(const TimestampSet *ts, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  const Period *p = timestampset_bbox_ptr(ts);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the period to a temporal box.
 */
void
period_tbox(const Period *p, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the period set to a temporal box.
 */
void
periodset_tbox(const PeriodSet *ps, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  const Period *p = periodset_bbox_ptr(ps);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the integer and the timestamp to a temporal box
 */
TBOX *
int_timestamp_to_tbox(int i, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, (double) i, (double) i, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the integer and the timestamp to a temporal box
 */
TBOX *
float_timestamp_to_tbox(double d, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, d, d, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the integer and the period to a temporal box
 */
TBOX *
int_period_to_tbox(int i, Period *p)
{
  TBOX *result = tbox_make(true, true, (double) i, (double)i, p->lower,
    p->upper);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the float and the period to a temporal box
 */
TBOX *
float_period_to_tbox(double d, Period *p)
{
  TBOX *result = tbox_make(true, true, d, d, p->lower, p->upper);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the range and the timestamp to a temporal box
 */
TBOX *
range_timestamp_to_tbox(RangeType *range, TimestampTz t)
{
  /* Return null on empty or unbounded range */
  char flags = range_get_flags(range);
  if (flags & (RANGE_EMPTY | RANGE_LB_INF | RANGE_UB_INF))
    return NULL;
  double xmin, xmax;
  range_bounds(range, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform the range and the period to a temporal box
 */
TBOX *
range_period_to_tbox(RangeType *range, Period *p)
{
  char flags = range_get_flags(range);
  if (flags & (RANGE_EMPTY | RANGE_LB_INF | RANGE_UB_INF))
    return NULL;
  ensure_tnumber_rangetype(oid_type(range->rangetypid));
  double xmin, xmax;
  range_bounds(range, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, p->lower, p->upper);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Cast the temporal box value as a float range value.
 */
RangeType *
tbox_floatrange(TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return NULL;
  RangeType *result = range_make(Float8GetDatum(box->xmin),
    Float8GetDatum(box->xmax), true, true, T_FLOAT8);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast the temporal box value as a period value
 */
Period *
tbox_period(TBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return NULL;
  Period *result = period_make(box->tmin, box->tmax, true, true);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the temporal box has X dimension
 */
bool
tbox_hasx(const TBOX *box)
{
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the temporal box has T dimension
 */
bool
tbox_hast(const TBOX *box)
{
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the minimum X value of the temporal box value, if any.
 */
bool
tbox_xmin(const TBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the maximum X value of the temporal box value, if any.
 */
bool
tbox_xmax(const TBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the minimum T value of the temporal box value, if any.
 */
bool
tbox_tmin(const TBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return false;
  *result = box->tmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the maximum T value of the temporal box value, if any.
 */
bool
tbox_tmax(const TBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return false;
  *result = box->tmax;
  return true;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the second temporal box value with the first one
 */
void
tbox_expand(const TBOX *box1, TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box2->flags))
  {
    box2->xmin = Min(box1->xmin, box2->xmin);
    box2->xmax = Max(box1->xmax, box2->xmax);
  }
  if (MOBDB_FLAGS_GET_T(box2->flags))
  {
    box2->tmin = Min(box1->tmin, box2->tmin);
    box2->tmax = Max(box1->tmax, box2->tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale the time span of the temporal box by the interval
 */
void
tbox_shift_tscale(const Interval *start, const Interval *duration, TBOX *box)
{
  assert(start != NULL || duration != NULL);
  if (start != NULL)
    box->tmin = DatumGetTimestampTz(DirectFunctionCall2(
      timestamptz_pl_interval, TimestampTzGetDatum(box->tmin),
      PointerGetDatum(start)));
  box->tmax = (duration == NULL) ?
    DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
      TimestampTzGetDatum(box->tmax), PointerGetDatum(start))) :
    DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
       TimestampTzGetDatum(box->tmin), PointerGetDatum(duration)));
  return;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the value dimension of the temporal box with the double value.
 */
TBOX *
tbox_expand_value(const TBOX *box, const double d)
{
  ensure_has_X_tbox(box);
  TBOX *result = tbox_copy(box);
  result->xmin = box->xmin - d;
  result->xmax = box->xmax + d;
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the time dimension of the temporal box with the interval value.
 */
TBOX *
tbox_expand_temporal(const TBOX *box, const Interval *interval)
{
  ensure_has_T_tbox(box);
  TBOX *result = tbox_copy(box);
  result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval,
    TimestampTzGetDatum(box->tmin), PointerGetDatum(interval)));
  result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval,
    TimestampTzGetDatum(box->tmax), PointerGetDatum(interval)));
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Set the precision of the value dimension of the temporal box to
 * the number of decimal places.
 */
TBOX *
tbox_round(const TBOX *box, int size)
{
  ensure_has_X_tbox(box);
  TBOX *result = tbox_copy(box);
  result->xmin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmin),
    size));
  result->xmax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmax),
    size));
  return result;
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static void
tbox_tbox_flags(const TBOX *box1, const TBOX *box2, bool *hasx, bool *hast)
{
  *hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  *hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  return;
}

/**
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static void
topo_tbox_tbox_init(const TBOX *box1, const TBOX *box2, bool *hasx, bool *hast)
{
  ensure_common_dimension(box1->flags, box2->flags);
  *hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  *hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  return;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the first temporal box contains the second one.
 */
bool
contains_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax))
    return false;
  if (hast && (box2->tmin < box1->tmin || box2->tmax > box1->tmax))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the first temporal box is contained by the second one.
 */
bool
contained_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  return contains_tbox_tbox(box2, box1);
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the temporal boxes overlap.
 */
bool
overlaps_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax))
    return false;
  if (hast && (box1->tmax < box2->tmin || box1->tmin > box2->tmax))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the temporal boxes are equal on the common dimensions.
 */
bool
same_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax))
    return false;
  if (hast && (box1->tmin != box2->tmin || box1->tmax != box2->tmax))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the temporal boxes are adjacent.
 */
bool
adjacent_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  TBOX inter;
  if (! inter_tbox_tbox(box1, box2, &inter))
    return false;
  /* Boxes are adjacent if they share n dimensions and their intersection is
   * at most of n-1 dimensions */
  bool result;
  if (!hasx && hast)
    result = (inter.tmin == inter.tmax);
  else if (hasx && !hast)
    result = (inter.xmin == inter.xmax);
  else
    result = (inter.xmin == inter.xmax || inter.tmin == inter.tmax);
  return result;
}

/*****************************************************************************
 * Relative position operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box is strictly to the left of
 * the second one.
 */
bool
left_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmax < box2->xmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend to the right
 * of the second one.
 */
bool
overleft_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmax <= box2->xmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box is strictly to the right of
 * the second one.
 */
bool
right_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmin > box2->xmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend to the left of
 * the second one.
 */
bool
overright_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmin >= box2->xmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box is strictly before
 * the second one.
 */
bool
before_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmax < box2->tmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend after
 * the second one.
 */
bool
overbefore_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmax <= box2->tmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box is strictly after the
 * second one.
 */
bool
after_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmin > box2->tmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend before
 * the second one.
 */
bool
overafter_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmin >= box2->tmin);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_set
 * @brief Return the union of the temporal boxes.
 */
TBOX *
union_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  ensure_same_dimensionality_tbox(box1, box2);
  /* The union of boxes that do not intersect cannot be represented by a box */
  if (! overlaps_tbox_tbox(box1, box2))
    elog(ERROR, "Result of box union would not be contiguous");

  bool hasx = MOBDB_FLAGS_GET_X(box1->flags);
  bool hast = MOBDB_FLAGS_GET_T(box1->flags);
  double xmin = 0, xmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  if (hasx)
  {
    xmin = Min(box1->xmin, box2->xmin);
    xmax = Max(box1->xmax, box2->xmax);
  }
  if (hast)
  {
    tmin = Min(box1->tmin, box2->tmin);
    tmax = Max(box1->tmax, box2->tmax);
  }
  TBOX *result = tbox_make(hasx, hast, xmin, xmax, tmin, tmax);
  return result;
}

/**
 * @ingroup libmeos_box_set
 * @brief Return the intersection of the temporal boxes.
 */
bool
inter_tbox_tbox(const TBOX *box1, const TBOX *box2, TBOX *result)
{
  bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  /* If there is no common dimension */
  if ((! hasx && ! hast) ||
    /* If they do no intersect in one common dimension */
    (hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax)) ||
    (hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax)))
    return false;

  double xmin = 0, xmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  if (hasx)
  {
    xmin = Max(box1->xmin, box2->xmin);
    xmax = Min(box1->xmax, box2->xmax);
  }
  if (hast)
  {
    tmin = Max(box1->tmin, box2->tmin);
    tmax = Min(box1->tmax, box2->tmax);
  }
  tbox_set(hasx, hast, xmin, xmax, tmin, tmax, result);
  return true;
}

/**
 * @ingroup libmeos_box_set
 * @brief Return the union of the spatiotemporal boxes.
 */
TBOX *
intersection_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  TBOX *result = palloc(sizeof(TBOX));
  if (! inter_tbox_tbox(box1, box2, result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box value
 * is less than, equal to, or greater than the second one.
 *
 * The time dimension is compared first and then the value dimension.
 *
 * @note Function used for B-tree comparison
 */
int
tbox_cmp(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  tbox_tbox_flags(box1, box2, &hasx, &hast);
  if (hast)
  {
    /* Compare the box minima */
    if (box1->tmin < box2->tmin)
      return -1;
    if (box1->tmin > box2->tmin)
      return 1;
    /* Compare the box maxima */
    if (box1->tmax < box2->tmax)
      return -1;
    if (box1->tmax > box2->tmax)
      return 1;
  }
  if (hasx)
  {
    /* Compare the box minima */
    if (box1->xmin < box2->xmin)
      return -1;
    if (box1->xmin > box2->xmin)
      return 1;
    /* Compare the box maxima */
    if (box1->xmax < box2->xmax)
      return -1;
    if (box1->xmax > box2->xmax)
      return 1;
  }
  /* Finally compare the flags */
  if (box1->flags < box2->flags)
    return -1;
  if (box1->flags > box2->flags)
    return 1;
  /* The two boxes are equal */
  return 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the two temporal boxes are equal
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tbox_eq(const TBOX *box1, const TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
      return false;
  if (box1->xmin != box2->xmin || box1->tmin != box2->tmin ||
    box1->xmax != box2->xmax || box1->tmax != box2->tmax)
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the two temporal boxes are different
 */
bool
tbox_ne(const TBOX *box1, const TBOX *box2)
{
  return ! tbox_eq(box1, box2);
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box value is less than the second one
 */
bool
tbox_lt(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box value is less than or equal to
 * the second one
 */
bool
tbox_le(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box value is greater than or equal
 * to the second one
 */
bool
tbox_ge(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box value is greater than the second one
 */
bool
tbox_gt(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_in);
/**
 * Input function for temporal boxes.
 *
 * Examples of input:
 * @code
 * TBOX((1.0, 2.0), (1.0, 2.0))   -- Both X and T dimensions
 * TBOX((1.0, ), (1.0, ))      -- Only X dimension
 * TBOX((, 2.0), (, 2.0))      -- Only T dimension
 * @endcode
 * where the commas are optional
 */
PGDLLEXPORT Datum
Tbox_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  TBOX *result = tbox_parse(&input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_out);
/**
 * Output function for temporal boxes.
 */
PGDLLEXPORT Datum
Tbox_out(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  char *result = tbox_to_string(box);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Tbox_send);
/**
 * Send function for TBOX
 */
PGDLLEXPORT Datum
Tbox_send(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  tbox_write(box, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Tbox_recv);
/**
 * Receive function for TBOX
 */
PGDLLEXPORT Datum
Tbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(tbox_read(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_constructor);
/**
 * Construct a temporal box value from the arguments
 */
PGDLLEXPORT Datum
Tbox_constructor(PG_FUNCTION_ARGS)
{
  double xmin = 0, xmax = 0; /* keep compiler quiet */
  TimestampTz tmin = 0, tmax = 0;
  bool hast = false;

  assert (PG_NARGS() == 2 || PG_NARGS() == 4);
  if (PG_NARGS() == 2)
  {
    xmin = PG_GETARG_FLOAT8(0);
    xmax = PG_GETARG_FLOAT8(1);
  }
  else if (PG_NARGS() == 4)
  {
    xmin = PG_GETARG_FLOAT8(0);
    tmin = PG_GETARG_TIMESTAMPTZ(1);
    xmax = PG_GETARG_FLOAT8(2);
    tmax = PG_GETARG_TIMESTAMPTZ(3);
    hast = true;
  }

  TBOX *result = tbox_make(true, hast, xmin, xmax, tmin, tmax);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_constructor_t);
/**
 * Construct a temporal box value from the timestamps
 */
PGDLLEXPORT Datum
Tbox_constructor_t(PG_FUNCTION_ARGS)
{
  TimestampTz tmin = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz tmax = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = tbox_make(false, true, 0.0, 0.0, tmin, tmax);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 * The functions set the argument box to 0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Int_to_tbox);
/**
 * Transform the integer to a temporal box
 */
PGDLLEXPORT Datum
Int_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  int_tbox(i, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_to_tbox);
/**
 * Transform the float to a temporal box
 */
PGDLLEXPORT Datum
Float_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  float_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Numeric_to_tbox);
/**
 * Transform the numeric to a temporal box
 */
PGDLLEXPORT Datum
Numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  double d = DatumGetFloat8(call_function1(numeric_float8, num));
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  float_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_to_tbox);
/**
 * Transform the range to a temporal box
 */
PGDLLEXPORT Datum
Range_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  /* Return null on empty or unbounded range */
  char flags = range_get_flags(range);
  if (flags & (RANGE_EMPTY | RANGE_LB_INF | RANGE_UB_INF))
    PG_RETURN_NULL();
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  range_tbox(range, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestamp_to_tbox);
/**
 * Transform the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  timestamp_tbox(t, result);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
timestampset_tbox_slice(Datum tsdatum, TBOX *box)
{
  TimestampSet *ts = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tsdatum))
    ts = (TimestampSet *) PG_DETOAST_DATUM_SLICE(tsdatum, 0,
      time_max_header_size());
  else
    ts = (TimestampSet *) tsdatum;
  timestampset_tbox(ts, box);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
Timestampset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  timestampset_tbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_tbox);
/**
 * Transform the period to a temporal box
 */
PGDLLEXPORT Datum
Period_to_tbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  period_tbox(p, result);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
periodset_tbox_slice(Datum psdatum, TBOX *box)
{
  PeriodSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (PeriodSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (PeriodSet *) psdatum;
  periodset_tbox(ps, box);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
Periodset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  periodset_tbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Int_timestamp_to_tbox);
/**
 * Transform the integer and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Int_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = int_timestamp_to_tbox(i, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_timestamp_to_tbox);
/**
 * Transform the float and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Float_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = float_timestamp_to_tbox(d, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Int_period_to_tbox);
/**
 *  Transform the integer and the period to a temporal box
 */
PGDLLEXPORT Datum
Int_period_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = int_period_to_tbox(i, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_period_to_tbox);
/**
 * Transform the float and the period to a temporal box
 */
PGDLLEXPORT Datum
Float_period_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = float_period_to_tbox(d, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_timestamp_to_tbox);
/**
 * Transform the range and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Range_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = range_timestamp_to_tbox(range, t);
  PG_FREE_IF_COPY(range, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_period_to_tbox);
/**
 * Transform the range and the period to a temporal box
 */
PGDLLEXPORT Datum
Range_period_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = range_period_to_tbox(range, p);
  if (! result)
    PG_RETURN_NULL();
  PG_FREE_IF_COPY(range, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_to_floatrange);
/**
 * Cast the temporal box value as a float range value
 */
PGDLLEXPORT Datum
Tbox_to_floatrange(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  RangeType *result = tbox_floatrange(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_to_period);
/**
 * Cast the temporal box value as a period value
 */
PGDLLEXPORT Datum
Tbox_to_period(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Period *result = tbox_period(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_hasx);
/**
 * Return true if the temporal box has X dimension
 */
PGDLLEXPORT Datum
Tbox_hasx(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hasx(box));
}

PG_FUNCTION_INFO_V1(Tbox_hast);
/**
 * Return true if the temporal box has T dimension
 */
PGDLLEXPORT Datum
Tbox_hast(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hast(box));
}

PG_FUNCTION_INFO_V1(Tbox_xmin);
/**
 * Return the minimum X value of the temporal box value
 */
PGDLLEXPORT Datum
Tbox_xmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tbox_xmax);
/**
 * Return the maximum X value of the temporal box value
 */
PGDLLEXPORT Datum
Tbox_xmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tbox_tmin);
/**
 * Return the minimum timestamp of the temporal box value
 */
PGDLLEXPORT Datum
Tbox_tmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Tbox_tmax);
/**
 * Return the maximum timestamp of the temporal box value
 */
PGDLLEXPORT Datum
Tbox_tmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_expand_value);
/**
 * Expand the value dimension of the temporal box with the double value
 */
PGDLLEXPORT Datum
Tbox_expand_value(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(tbox_expand_value(box, d));
}

PG_FUNCTION_INFO_V1(Tbox_expand_temporal);
/**
 * Expand the time dimension of the temporal box with the interval value
 */
PGDLLEXPORT Datum
Tbox_expand_temporal(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_POINTER(tbox_expand_temporal(box, interval));
}

PG_FUNCTION_INFO_V1(Tbox_round);
/**
 * Set the precision of the value dimension of the temporal box to the number
 * of decimal places
 */
PGDLLEXPORT Datum
Tbox_round(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  int size = PG_GETARG_INT32(1);
  PG_RETURN_POINTER(tbox_round(box, size));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_tbox_tbox);
/**
 * Return true if the first temporal box contains the second one
 */
PGDLLEXPORT Datum
Contains_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contains_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Contained_tbox_tbox);
/**
 * Return true if the first temporal box is contained by the second one
 */
PGDLLEXPORT Datum
Contained_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contained_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overlaps_tbox_tbox);
/**
 * Return true if the temporal boxes overlap
 */
PGDLLEXPORT Datum
Overlaps_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overlaps_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Same_tbox_tbox);
/**
 * Return true if the temporal boxes are equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(same_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Adjacent_tbox_tbox);
/**
 * Return true if the temporal boxes are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(adjacent_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Relative position operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Left_tbox_tbox);
/**
 * Return true if the first temporal box is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(left_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overleft_tbox_tbox);
/**
 * Return true if the first temporal box does not extend to the right of the second one
 */
PGDLLEXPORT Datum
Overleft_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overleft_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Right_tbox_tbox);
/**
 * Return true if the first temporal box is strictly to the right of the second one
 */
PGDLLEXPORT Datum
Right_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(right_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overright_tbox_tbox);
/**
 * Return true if the first temporal box does not extend to the left of the second one
 */
PGDLLEXPORT Datum
Overright_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overright_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Before_tbox_tbox);
/**
 * Return true if the first temporal box is strictly before the second one
 */
PGDLLEXPORT Datum
Before_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(before_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overbefore_tbox_tbox);
/**
 * Return true if the first temporal box does not extend after the second one
 */
PGDLLEXPORT Datum
Overbefore_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overbefore_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(After_tbox_tbox);
/**
 * Return true if the first temporal box is strictly after the second one
 */
PGDLLEXPORT Datum
After_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(after_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overafter_tbox_tbox);
/**
 * Return true if the first temporal box does not extend before the second one
 */
PGDLLEXPORT Datum
Overafter_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overafter_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_tbox_tbox);
/**
 * Return the union of the temporal boxes
 */
PGDLLEXPORT Datum
Union_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = union_tbox_tbox(box1, box2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_tbox_tbox);
/**
 * Return the intersection of the temporal boxes
 */
PGDLLEXPORT Datum
Intersection_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = intersection_tbox_tbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_extent_transfn);
/**
 * Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  /* Can't do anything with null inputs */
  if (!box1 && !box2)
    PG_RETURN_NULL();
  TBOX *result = (TBOX *) palloc0(sizeof(TBOX));
  /* One of the boxes is null, return the other one */
  if (!box1)
  {
    memcpy(result, box2, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }
  if (!box2)
  {
    memcpy(result, box1, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  memcpy(result, box1, sizeof(TBOX));
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_extent_combinefn);
/**
 * Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  if (!box2 && !box1)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (box2 && !box1)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBOX *result = tbox_copy(box1);
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first temporal box value
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Tbox_cmp(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_INT32(tbox_cmp(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_lt);
/**
 * Return true if the first temporal box value is less than the second one
 */
PGDLLEXPORT Datum
Tbox_lt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_lt(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_le);
/**
 * Return true if the first temporal box value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Tbox_le(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_le(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_ge);
/**
 * Return true if the first temporal box value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Tbox_ge(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ge(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_gt);
/**
 * Return true if the first temporal box value is greater than the second one
 */
PGDLLEXPORT Datum
Tbox_gt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_gt(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_eq);
/**
 * Return true if the two temporal boxes are equal
 */
PGDLLEXPORT Datum
Tbox_eq(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_eq(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_ne);
/**
 * Return true if the two temporal boxes are different
 */
PGDLLEXPORT Datum
Tbox_ne(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ne(box1, box2));
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
