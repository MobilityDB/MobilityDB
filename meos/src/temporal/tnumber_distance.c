/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Temporal distance functions for temporal numbers
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "temporal/span.h"
#include "temporal/tbox.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Compute the distance between two instants depending on their type
 *****************************************************************************/

/**
 * @brief Return the distance between two temporal instants
 * @param[in] inst1,inst2 Temporal instants
 */
double
tnumberinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1->temptype == inst2->temptype);
  /* Two big integers are subtracted as integers, as #distance_value_value
   * does, since a float holds their difference rounded once but cannot hold
   * each of them beyond 53 bits; the subtraction is unsigned, so that the
   * difference of the two ends of the range does not overflow */
  if (inst1->temptype == T_TBIGINT)
  {
    int64 value1 = DatumGetInt64(tinstant_value_p(inst1));
    int64 value2 = DatumGetInt64(tinstant_value_p(inst2));
    return (double) (value1 > value2 ? (uint64) value1 - (uint64) value2 :
      (uint64) value2 - (uint64) value1);
  }
  return fabs(tnumberinst_double(inst1) - tnumberinst_double(inst2));
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return 1 if two temporal number segments intersect during the
 * period defined by the output timestampts, return 0 otherwise
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segment
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note This function is passed to the lifting infrastructure when computing
 * the temporal distance
 * @post As there is a single turning point, `t2` is set to `t1`
 */
int
tfloat_distance_turnpt(Datum start1, Datum end1, Datum start2, Datum end2,
  Datum param UNUSED, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2)
{
  return tnumbersegm_intersection(start1, end1, start2, end2, T_FLOAT8,
    lower, upper, t1, t2);
}

/**
 * @brief Return 1 if a temporal float segment intersects a value during the
 * period defined by the output timestampts, return 0 otherwise
 * @param[in] start,end Values defining the segment
 * @param[in] value Value to locate
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note This function is passed to the lifting infrastructure when computing
 * the temporal distance
 */
int
tfloat_base_distance_turnpt(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  return tfloat_distance_turnpt(start, end, value, value, value, lower, upper,
    t1, t2);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the temporal distance between a temporal number and a number
 * @param[in] temp Temporal number
 * @param[in] value Value
 */
Temporal *
tdistance_tnumber_number(const Temporal *temp, Datum value)
{
  assert(temp);
  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &distance_value_value;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = temp->temptype;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_base = (! lfinfo.reslinear || basetype == T_INT4) ? NULL :
    &tfloat_base_distance_turnpt;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between two temporal numbers
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tdistance_tnumber_tnumber()
 */
Temporal *
tdistance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return NULL;

  /* Fill the lifted structure */
  MeosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &distance_value_value;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = temp1->temptype;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_temp = (! lfinfo.reslinear || basetype == T_INT4) ? NULL :
    &tfloat_distance_turnpt;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a temporal integer and an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @errval NULL
 * @csqlfn #Tdistance_tnumber_number() #Tdistance_number_tnumber()
 */
Temporal *
tdistance_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return tdistance_tnumber_number(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @errval NULL
 * @csqlfn #Tdistance_tnumber_number() #Tdistance_number_tnumber()
 */
Temporal *
tdistance_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  if (! ensure_finite(d))
    return NULL;
  return tdistance_tnumber_number(temp, Float8GetDatum(d));
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] value Value
 */
Datum
nad_tnumber_number(const Temporal *temp, Datum value)
{
  assert(temp); assert(tnumber_type(temp->temptype));
  MeosType basetype = temptype_basetype(temp->temptype);
  TBox box1, box2;
  tnumber_set_tbox(temp, &box1);
  number_set_tbox(value, basetype, &box2);
  return nad_tbox_tbox(&box1, &box2);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between the temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @return The sentinel of the base type given by #distance_sentinel() where the
 * boxes share no time, which is an answer and not an error: the external
 * function establishes every precondition, so no path here raises
 * @note Function called when using indexes for k-nearest neighbor queries for
 * temporal numbers. Therefore, it must satisfy the following conditions
 * (1) the actual distance is always greater than or equal to the estimated
 * distance and (2) the index returns the tuples in order of increasing
 * estimated distance.
 * https://www.postgresql.org/message-id/CA%2BTgmoauhLf6R07sAUzQiRcstF5KfRw7nwiWn4VZgiSF8MaQaw%40mail.gmail.com
 * @csqlfn #NAD_tbox_tbox()
 */
Datum
nad_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* The preconditions the external function tests */
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  assert(box1->span.spantype == box2->span.spantype);
  MeosType basetype = box1->span.basetype;

  /* If the boxes do not intersect in the time dimension return the sentinel */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
    return distance_sentinel(basetype);

  return distance_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @return The sentinel of the base type given by #distance_sentinel() where
 * the operands share no time, which is an answer and not an error: the
 * external function establishes every precondition, so no path here raises
 * @csqlfn #NAD_tnumber_tbox() #NAD_tbox_tnumber()
 */
Datum
nad_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  assert(temp); assert(box); assert(MEOS_FLAGS_GET_X(box->flags));
  assert(tnumber_type(temp->temptype));
  assert(temptype_basetype(temp->temptype) == box->span.basetype);

  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return distance_sentinel(temptype_basetype(temp->temptype));
  }

  /* Project the temporal number to the timespan of the box (if any) */
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) : (Temporal *) temp;
  /* Test if the bounding boxes overlap */
  TBox box1;
  tnumber_set_tbox(temp1, &box1);
  if (overlaps_tbox_tbox(box, &box1))
  {
    if (hast)
      pfree(temp1);
    return (Datum) 0;
  }

  /* Get the minimum distance between the values of the boxes, on whichever
   * side of the temporal number the box lies, as #nad_tbox_tbox does */
  Datum res = distance_span_span(&box->span, &box1.span);

  if (hast)
    pfree(temp1);

  return res;
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a number span and a
 * temporal box, the distance of the box of the span to the box, as
 * #nad_tbox_tbox answers for two boxes
 * @param[in] s Span
 * @param[in] box Temporal box
 * @return The distance in the base type of the span; the box of a span has no
 * period, so the operands always share their time
 * @csqlfn #NAD_intspan_tbox() #NAD_tbox_intspan() #NAD_bigintspan_tbox()
 * #NAD_tbox_bigintspan() #NAD_floatspan_tbox() #NAD_tbox_floatspan()
 */
Datum
nad_numspan_tbox(const Span *s, const TBox *box)
{
  /* The preconditions the external function tests */
  assert(s); assert(box); assert(numspan_type(s->spantype));
  assert(MEOS_FLAGS_GET_X(box->flags));
  assert(s->spantype == box->span.spantype);
  TBox box1;
  numspan_set_tbox(s, &box1);
  return nad_tbox_tbox(&box1, box);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number and
 * a number span, the distance of the temporal number to the box of the span,
 * as #nad_tnumber_tbox answers for a temporal box
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @return The distance in the base type of the temporal number; the box of a
 * span has no period, so the operands always share their time
 * @csqlfn #NAD_tint_intspan() #NAD_intspan_tint() #NAD_tbigint_bigintspan()
 * #NAD_bigintspan_tbigint() #NAD_tfloat_floatspan() #NAD_floatspan_tfloat()
 */
Datum
nad_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* The preconditions the external function tests */
  assert(temp); assert(s); assert(tnumber_type(temp->temptype));
  assert(temptype_basetype(temp->temptype) == s->basetype);
  TBox box;
  numspan_set_tbox(s, &box);
  return nad_tnumber_tbox(temp, &box);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between two temporal numbers
 * @param[in] temp1,temp2 Temporal boxes
 * @return The sentinel of the base type given by #distance_sentinel() where
 * the operands share no time, which is an answer and not an error: the
 * external function establishes every precondition, so no path here raises
 */
Datum
nad_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(tnumber_type(temp1->temptype));
  Temporal *dist = tdistance_tnumber_tnumber(temp1, temp2);
  /* If the boxes do not intersect in the time dimension return the sentinel */
  if (dist == NULL)
    return distance_sentinel(temptype_basetype(temp1->temptype));

  Datum res = temporal_min_value(dist);
  pfree(dist);
  return res;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @errval INT_MAX
 * @csqlfn #NAD_tnumber_number() #NAD_number_tnumber()
 */
int
nad_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  return DatumGetInt32(nad_tnumber_number(temp, Int32GetDatum(i)));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @errval INT64_MAX
 * @csqlfn #NAD_tnumber_number() #NAD_number_tnumber()
 */
int64
nad_tbigint_bigint(const Temporal *temp, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, INT64_MAX);
  return DatumGetInt64(nad_tnumber_number(temp, Int64GetDatum(i)));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @errval DBL_MAX
 * @csqlfn #NAD_tnumber_number() #NAD_number_tnumber()
 */
double
nad_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  if (! ensure_finite(d))
    return DBL_MAX;
  return DatumGetFloat8(nad_tnumber_number(temp, Float8GetDatum(d)));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal integer
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @errval INT_MAX
 * @csqlfn #NAD_tnumber_tbox(), #NAD_tbox_tnumber()
 */
int
nad_tint_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  if (! ensure_valid_tnumber_tbox(temp, box) ||
      ! ensure_has_X(T_TBOX, box->flags))
    return INT_MAX;
  return DatumGetInt32(nad_tnumber_tbox(temp, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal big integer
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @errval INT64_MAX
 * @csqlfn #NAD_tnumber_tbox(), #NAD_tbox_tnumber()
 */
int64
nad_tbigint_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, INT64_MAX);
  if (! ensure_valid_tnumber_tbox(temp, box) ||
      ! ensure_has_X(T_TBOX, box->flags))
    return INT64_MAX;
  return DatumGetInt64(nad_tnumber_tbox(temp, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal float
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @errval DBL_MAX
 * @csqlfn #NAD_tnumber_tbox(), #NAD_tbox_tnumber()
 */
double
nad_tfloat_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  if (! ensure_valid_tnumber_tbox(temp, box) ||
      ! ensure_has_X(T_TBOX, box->flags))
    return DBL_MAX;
  return DatumGetFloat8(nad_tnumber_tbox(temp, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between the int temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @errval -1
 * @note A distance is never negative, so -1 states an error where INT_MAX
 * states that the boxes share no time
 * @csqlfn #NAD_tbox_tbox()
 */
int
nad_tboxint_tboxint(const TBox *box1, const TBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, -1); VALIDATE_NOT_NULL(box2, -1);
  if (! ensure_span_isof_type(&box1->span, T_INTSPAN) ||
      ! ensure_span_isof_type(&box2->span, T_INTSPAN) ||
      ! ensure_has_X(T_TBOX, box1->flags) ||
      ! ensure_has_X(T_TBOX, box2->flags))
    return -1;

  return DatumGetInt32(nad_tbox_tbox(box1, box2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between the big integer temporal
 * boxes
 * @param[in] box1,box2 Temporal boxes
 * @errval -1
 * @note A distance is never negative, so -1 states an error where INT64_MAX
 * states that the boxes share no time
 * @csqlfn #NAD_tbox_tbox()
 */
int64
nad_tboxbigint_tboxbigint(const TBox *box1, const TBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, -1); VALIDATE_NOT_NULL(box2, -1);
  if (! ensure_span_isof_type(&box1->span, T_BIGINTSPAN) ||
      ! ensure_span_isof_type(&box2->span, T_BIGINTSPAN) ||
      ! ensure_has_X(T_TBOX, box1->flags) ||
      ! ensure_has_X(T_TBOX, box2->flags))
    return -1;

  return DatumGetInt64(nad_tbox_tbox(box1, box2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between the float temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @errval -1.0
 * @note A distance is never negative, so -1.0 states an error where DBL_MAX
 * states that the boxes share no time
 * @csqlfn #NAD_tbox_tbox()
 */
double
nad_tboxfloat_tboxfloat(const TBox *box1, const TBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, -1.0); VALIDATE_NOT_NULL(box2, -1.0);
  if (! ensure_span_isof_type(&box1->span, T_FLOATSPAN) ||
      ! ensure_span_isof_type(&box2->span, T_FLOATSPAN) ||
      ! ensure_has_X(T_TBOX, box1->flags) ||
      ! ensure_has_X(T_TBOX, box2->flags))
    return -1.0;
  return DatumGetFloat8(nad_tbox_tbox(box1, box2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between an integer span and a
 * temporal box, the distance of the box of the span to the box
 * @param[in] s Span
 * @param[in] box Temporal box
 * @errval -1
 * @note A distance is never negative, so -1 states an error, as in
 * #nad_tboxint_tboxint
 * @csqlfn #NAD_intspan_tbox(), #NAD_tbox_intspan()
 */
int
nad_intspan_tbox(const Span *s, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, -1); VALIDATE_NOT_NULL(box, -1);
  if (! ensure_has_X(T_TBOX, box->flags) ||
      ! ensure_span_isof_type(&box->span, T_INTSPAN))
    return -1;
  return DatumGetInt32(nad_numspan_tbox(s, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a big integer span and a
 * temporal box, the distance of the box of the span to the box
 * @param[in] s Span
 * @param[in] box Temporal box
 * @errval -1
 * @note A distance is never negative, so -1 states an error, as in
 * #nad_tboxbigint_tboxbigint
 * @csqlfn #NAD_bigintspan_tbox(), #NAD_tbox_bigintspan()
 */
int64
nad_bigintspan_tbox(const Span *s, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, -1); VALIDATE_NOT_NULL(box, -1);
  if (! ensure_has_X(T_TBOX, box->flags) ||
      ! ensure_span_isof_type(&box->span, T_BIGINTSPAN))
    return -1;
  return DatumGetInt64(nad_numspan_tbox(s, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a float span and a
 * temporal box, the distance of the box of the span to the box
 * @param[in] s Span
 * @param[in] box Temporal box
 * @errval -1.0
 * @note A distance is never negative, so -1.0 states an error, as in
 * #nad_tboxfloat_tboxfloat
 * @csqlfn #NAD_floatspan_tbox(), #NAD_tbox_floatspan()
 */
double
nad_floatspan_tbox(const Span *s, const TBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, -1.0); VALIDATE_NOT_NULL(box, -1.0);
  if (! ensure_has_X(T_TBOX, box->flags) ||
      ! ensure_span_isof_type(&box->span, T_FLOATSPAN))
    return -1.0;
  return DatumGetFloat8(nad_numspan_tbox(s, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal integer and
 * an integer span, the distance of the temporal integer to the box of the
 * span, as #nad_tint_tbox answers for a temporal box
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @errval -1
 * @note A distance is never negative, so -1 states an error
 * @csqlfn #NAD_tint_intspan(), #NAD_intspan_tint()
 */
int
nad_tint_intspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1); VALIDATE_INTSPAN(s, -1);
  return DatumGetInt32(nad_tnumber_numspan(temp, s));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal big integer
 * and a big integer span, the distance of the temporal big integer to the box
 * of the span, as #nad_tbigint_tbox answers for a temporal box
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @errval -1
 * @note A distance is never negative, so -1 states an error
 * @csqlfn #NAD_tbigint_bigintspan(), #NAD_bigintspan_tbigint()
 */
int64
nad_tbigint_bigintspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, -1); VALIDATE_BIGINTSPAN(s, -1);
  return DatumGetInt64(nad_tnumber_numspan(temp, s));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal float and a
 * float span, the distance of the temporal float to the box of the span, as
 * #nad_tfloat_tbox answers for a temporal box
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @errval -1.0
 * @note A distance is never negative, so -1.0 states an error
 * @csqlfn #NAD_tfloat_floatspan(), #NAD_floatspan_tfloat()
 */
double
nad_tfloat_floatspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1.0); VALIDATE_FLOATSPAN(s, -1.0);
  return DatumGetFloat8(nad_tnumber_numspan(temp, s));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between two temporal integers
 * @param[in] temp1,temp2 Temporal values
 * @errval INT_MAX
 * @csqlfn #NAD_tnumber_tnumber()
 */
int
nad_tint_tint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp1, INT_MAX); VALIDATE_TINT(temp2, INT_MAX);
  return DatumGetInt32(nad_tnumber_tnumber(temp1, temp2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between two temporal big
 * integers
 * @param[in] temp1,temp2 Temporal values
 * @errval INT64_MAX
 * @csqlfn #NAD_tnumber_tnumber()
 */
int64
nad_tbigint_tbigint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp1, INT64_MAX); VALIDATE_TBIGINT(temp2, INT64_MAX);
  return DatumGetInt64(nad_tnumber_tnumber(temp1, temp2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between two temporal floats
 * @param[in] temp1,temp2 Temporal values
 * @errval DBL_MAX
 * @csqlfn #NAD_tnumber_tnumber()
 */
double
nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp1, DBL_MAX); VALIDATE_TFLOAT(temp2, DBL_MAX);
  return DatumGetFloat8(nad_tnumber_tnumber(temp1, temp2));
}

