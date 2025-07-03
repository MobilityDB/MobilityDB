/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
  meosType basetype = temptype_basetype(temp->temptype);
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
  meosType basetype = temptype_basetype(temp1->temptype);
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

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @note The function returns a double for both integer and float numbers
 */
double
nad_tnumber_number(const Temporal *temp, Datum value)
{
  assert(temp); assert(tnumber_type(temp->temptype));
  meosType basetype = temptype_basetype(temp->temptype);
  TBox box1, box2;
  tnumber_set_tbox(temp, &box1);
  number_set_tbox(value, basetype, &box2);
  return nad_tbox_tbox(&box1, &box2);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between the temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @return On error return infinity
 * @note Function called when using indexes for k-nearest neighbor queries for
 * temporal numbers. Therefore, it must satisfy the following conditions
 * (1) the actual distance is always greater than or equal to the estimated
 * distance and (2) the index returns the tuples in order of increasing
 * estimated distance.
 * https://www.postgresql.org/message-id/CA%2BTgmoauhLf6R07sAUzQiRcstF5KfRw7nwiWn4VZgiSF8MaQaw%40mail.gmail.com
 * @csqlfn #NAD_tbox_tbox()
 * @note The function returns a double for both integer and float boxes
 */
double
nad_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure the validity of the arguments */
  assert(box1); assert(box2);
  if (! ensure_has_X(T_TBOX, box1->flags) ||
      ! ensure_has_X(T_TBOX, box2->flags) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return DBL_MAX;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
    return DBL_MAX;

  Datum res = distance_span_span(&box1->span, &box2->span);
  return datum_double(res, spantype_basetype(box1->span.spantype));
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @return On error or if the time frames do not overlap return infinity
 * @csqlfn #NAD_tnumber_tbox()
 * @note The function returns a double for both integer and float numbers
 */
double
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
    return DBL_MAX;
  }

  /* Project the temporal number to the timespan of the box (if any) */
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) : (Temporal *) temp;
  /* Test if the bounding boxes overlap */
  TBox box1;
  tnumber_set_tbox(temp1, &box1);
  if (overlaps_tbox_tbox(box, &box1))
    return 0.0;

  /* Get the minimum distance between the values of the boxes */
  Datum res = distance_value_value(box->span.lower, box1.span.upper,
    box->span.basetype);

  if (hast)
    pfree(temp1);

  return datum_double(res, temptype_basetype(temp->temptype));
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between two temporal numbers
 * @param[in] temp1,temp2 Temporal boxes
 * @return On error or when the time frames do not intersect return infinity
 * @note The function returns a double for both integer and float numbers
 */
double
nad_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(tnumber_type(temp1->temptype));
  Temporal *dist = tdistance_tnumber_tnumber(temp1, temp2);
  /* If the boxes do not intersect in the time dimension return infinity */
  if (dist == NULL)
    return DBL_MAX;

  Datum res = temporal_min_value(dist);
  pfree(dist);
  return datum_double(res, temptype_basetype(temp1->temptype));
}

/*****************************************************************************/
