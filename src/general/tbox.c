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
 * General functions
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
 * @ingroup libmeos_box_oper_set
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
 * @ingroup libmeos_box_cast
 * @brief Transform the integer and the timestamp to a temporal box
 */
TBOX *
int_timestamp_to_tbox(int i, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, (double) i, (double) i, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform the integer and the timestamp to a temporal box
 */
TBOX *
float_timestamp_to_tbox(double d, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, d, d, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_cast
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
 * @ingroup libmeos_box_cast
 * @brief Transform the float and the period to a temporal box
 */
TBOX *
float_period_to_tbox(double d, Period *p)
{
  TBOX *result = tbox_make(true, true, d, d, p->lower, p->upper);
  return result;
}

/**
 * @ingroup libmeos_box_cast
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
 * @ingroup libmeos_box_cast
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
tbox_to_floatrange(TBOX *box)
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
tbox_to_period(TBOX *box)
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
tbox_expand_temporal(const TBOX *box, const Datum interval)
{
  ensure_has_T_tbox(box);
  TBOX *result = tbox_copy(box);
  result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval,
    TimestampTzGetDatum(box->tmin), interval));
  result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval,
    TimestampTzGetDatum(box->tmax), interval));
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
 * @ingroup libmeos_box_oper_topo
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
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the first temporal box is contained by the second one.
 */
bool
contained_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  return contains_tbox_tbox(box2, box1);
}

/**
 * @ingroup libmeos_box_oper_topo
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
 * @ingroup libmeos_box_oper_topo
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
 * @ingroup libmeos_box_oper_topo
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_pos
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
 * @ingroup libmeos_box_oper_set
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

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/


/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_oper_comp
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
 * @ingroup libmeos_box_oper_comp
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

/*****************************************************************************/

