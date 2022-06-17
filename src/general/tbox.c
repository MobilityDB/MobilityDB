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

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"

/** Buffer size for input and output of TBOX values */
#define MAXTBOXLEN    128

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that a temporal box has X values
 */
void
ensure_has_X_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    elog(ERROR, "The box must have value dimension");
}

/**
 * Ensure that a temporal box has T values
 */
void
ensure_has_T_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    elog(ERROR, "The box must have time dimension");
}

/**
 * Ensure that a temporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_tbox(const TBOX *box1, const TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
    elog(ERROR, "The boxes must be of the same dimensionality");
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a temporal box from its Well-Known Text (WKT) representation.
 */
TBOX *
tbox_in(char *str)
{
  return tbox_parse(&str);
}

/**
 * @ingroup libmeos_box_in_out
 * @brief Return the Well-Known Text (WKT) representation of a temporal box.
 */
char *
tbox_out(const TBOX *box)
{
  static size_t size = MAXTBOXLEN + 1;
  char *result = palloc(size);
  char *xmin = NULL, *xmax = NULL, *tmin = NULL, *tmax = NULL;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  assert(hasx || hast);
  if (hasx)
  {
    xmin = basetype_output(T_FLOAT8, Float8GetDatum(box->xmin));
    xmax = basetype_output(T_FLOAT8, Float8GetDatum(box->xmax));
  }
  if (hast)
  {
    tmin = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(box->tmin));
    tmax = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(box->tmax));
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

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_constructor
 * @brief Construct a temporal box from the arguments.
 * @sqlfunc tbox()
 */
TBOX *
tbox_make(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax)
{
  /* Note: zero-fill is done in function tbox_set */
  TBOX *result = palloc(sizeof(TBOX));
  tbox_set(hasx, hast, xmin, xmax, tmin, tmax, result);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Set a temporal box from the arguments
 * @note This function is equivalent to @ref tbox_make without memory
 * allocation
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
 * @brief Return a copy of a temporal box.
 */
TBOX *
tbox_copy(const TBOX *box)
{
  TBOX *result = palloc(sizeof(TBOX));
  memcpy(result, box, sizeof(TBOX));
  return result;
}

/*****************************************************************************
 * Casting
 * The functions start by setting the output argument box to 0
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Set a temporal box from a number.
 * @sqlop @p ::
 */
void
number_set_tbox(Datum value, MDB_Type basetype, TBOX *box)
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
 * @brief Set a temporal box from an integer.
 * @sqlop @p ::
 */
void
int_set_tbox(int i, TBOX *box)
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
 * @brief Set a temporal box from a float.
 * @sqlop @p ::
 */
void
float_set_tbox(double d, TBOX *box)
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
 * @brief Set a temporal box from a span.
 * @sqlop @p ::
 */
void
span_set_tbox(const Span *span, TBOX *box)
{
  ensure_tnumber_spantype(span->spantype);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  span_bounds(span, &box->xmin, &box->xmax);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Set a temporal box from a timestamp.
 * @sqlop @p ::
 */
void
timestamp_set_tbox(TimestampTz t, TBOX *box)
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
 * @brief Set a temporal box from a period set.
 * @sqlop @p ::
 */
void
timestampset_set_tbox(const TimestampSet *ts, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  const Period *p = timestampset_period_ptr(ts);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Set a temporal box from a period.
 * @sqlop @p ::
 */
void
period_set_tbox(const Period *p, TBOX *box)
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
 * @brief Set a temporal box from a period set.
 * @sqlop @p ::
 */
void
periodset_set_tbox(const PeriodSet *ps, TBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBOX));
  const Period *p = periodset_period_ptr(ps);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from an integer and a timestamp
 * @sqlfunc tbox()
 */
TBOX *
int_timestamp_to_tbox(int i, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, (double) i, (double) i, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from a float and a timestamp
 * @sqlfunc tbox()
 */
TBOX *
float_timestamp_to_tbox(double d, TimestampTz t)
{
  TBOX *result = tbox_make(true, true, d, d, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from an integer and a period
 * @sqlfunc tbox()
 */
TBOX *
int_period_to_tbox(int i, const Period *p)
{
  TBOX *result = tbox_make(true, true, (double) i, (double)i, p->lower,
    p->upper);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from a float and a period
 * @sqlfunc tbox()
 */
TBOX *
float_period_to_tbox(double d, const Period *p)
{
  TBOX *result = tbox_make(true, true, d, d, p->lower, p->upper);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from a span and a timestamp
 * @sqlfunc tbox()
 */
TBOX *
span_timestamp_to_tbox(const Span *span, TimestampTz t)
{
  double xmin, xmax;
  span_bounds(span, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, t, t);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a temporal box from a span and a period
 * @sqlfunc tbox()
 */
TBOX *
span_period_to_tbox(const Span *span, const Period *p)
{
  ensure_tnumber_spantype(span->spantype);
  double xmin, xmax;
  span_bounds(span, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, p->lower, p->upper);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Cast a temporal box to an integer span.
 * @sqlop @p ::
 */
Span *
tbox_to_intspan(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return NULL;
  Span *result = span_make(Int32GetDatum((int) box->xmin),
    Int32GetDatum((int) box->xmax), true, true, T_INT4);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast a temporal box as a float span.
 * @sqlop @p ::
 */
Span *
tbox_to_floatspan(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return NULL;
  Span *result = span_make(Float8GetDatum(box->xmin),
    Float8GetDatum(box->xmax), true, true, T_FLOAT8);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast a temporal box as a period
 * @sqlop @p ::
 */
Period *
tbox_to_period(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return NULL;
  Period *result = span_make(box->tmin, box->tmax, true, true, T_TIMESTAMPTZ);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @sqlfunc hasX()
 */
bool
tbox_hasx(const TBOX *box)
{
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a temporal box has time dimension
 * @sqlfunc hasT()
 */
bool
tbox_hast(const TBOX *box)
{
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the temporal box has value dimension. In that case,
 * the minimum value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmin()
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
 * @brief Return true if the temporal box has value dimension. In that case,
 * the maximum value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmax()
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
 * @brief Return true if the temporal box has time dimension. In that case,
 * the minimum timestamp is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmin()
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
 * @brief Return true if the temporal box has time dimension. In that case,
 * the maximum timestamp is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmax()
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
 * @brief Expand the second temporal box with the first one
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
 * @brief Return a temporal box shifted and/or scaled in the time dimension by
 * the intervals
 * @sqlfunc shift(), tscale(), shiftTscale()
 */
void
tbox_shift_tscale(const Interval *start, const Interval *duration, TBOX *box)
{
  assert(start != NULL || duration != NULL);
  if (start != NULL)
    box->tmin = pg_timestamp_pl_interval(box->tmin, start);
  box->tmax = (duration == NULL) ?
    pg_timestamp_pl_interval(box->tmax, start) :
    pg_timestamp_pl_interval(box->tmin, duration);
  return;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Return a temporal box expanded in the value dimension by a double.
 * @sqlfunc @p expandValue()
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
 * @brief Return a temporal box expanded in the time dimension by an interval.
 * @sqlfunc expandTemporal()
 */
TBOX *
tbox_expand_temporal(const TBOX *box, const Interval *interval)
{
  ensure_has_T_tbox(box);
  TBOX *result = tbox_copy(box);
  result->tmin = pg_timestamp_mi_interval(box->tmin, interval);
  result->tmax = pg_timestamp_pl_interval(box->tmax, interval);
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
 * @sqlop @p \@>
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
 * @sqlop @p @<
 */
bool
contained_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  return contains_tbox_tbox(box2, box1);
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the temporal boxes overlap.
 * @sqlop @p &&
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
 * @sqlop @p ~=
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
 * @sqlop @p -|-
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
 * @sqlop @p <<
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
 * @sqlop @p &<
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
 * @sqlop @p >>
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
 * @sqlop @p &>
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
 * @sqlop @p <<#
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
 * @sqlop @p &<#
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
 * @sqlop @p #>>
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
 * @sqlop @p #&>
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
 * @sqlop @p +
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
 * @brief Set a temporal box with the result of the intersection of the first
 * two temporal boxes.
 * @note This function is equivalent to @ref intersection_tbox_tbox without
 * memory allocation
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
 * @brief Return the intersection of the spatiotemporal boxes.
 * @sqlop @p *
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
 * @brief Return true if the temporal boxes are equal
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
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
 * @brief Return true if the temporal boxes are different
 * @sqlop @p <>
 */
bool
tbox_ne(const TBOX *box1, const TBOX *box2)
{
  return ! tbox_eq(box1, box2);
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal to, or greater than the second one.
 *
 * The time dimension is compared first and then the value dimension.
 * @note Function used for B-tree comparison
 * @sqlfunc tbox_cmp()
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
 * @brief Return true if the first temporal box is less than the second one
 * @sqlop @p <
 */
bool
tbox_lt(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
tbox_le(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is greater than or equal
 * to the second one
 * @sqlop @p >=
 */
bool
tbox_ge(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is greater than the second one
 * @sqlop @p >
 */
bool
tbox_gt(const TBOX *box1, const TBOX *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/
