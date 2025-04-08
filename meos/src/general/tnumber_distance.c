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
 * @brief Distance functions for temporal numbers
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/span.h"
#include "general/tbox.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/type_util.h"
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
 * @ingroup meos_internal_temporal_dist
 * @brief Return the temporal distance between a temporal number and a number
 * @param[in] temp Temporal number
 * @param[in] value Value
 */
Temporal *
distance_tnumber_number(const Temporal *temp, Datum value)
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
  lfinfo.tpfunc_base = &tlinearsegm_intersection_value;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Return true if two segments of the temporal numbers intersect at a
 * timestamptz
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @note This function is passed to the lifting infrastructure when computing
 * the temporal distance
 */
static bool
tnumber_min_dist_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  if (! tsegment_intersection(start1, end1, LINEAR, start2, end2, LINEAR,
      NULL, NULL, t))
    return false;
  *value = (Datum) 0;
  return true;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between two temporal numbers
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Distance_tnumber_tnumber()
 */
Temporal *
distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
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
  lfinfo.tpfunc = lfinfo.reslinear ? &tnumber_min_dist_at_timestamptz : NULL;
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
 */
Datum
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
 * @return On error return -1
 * @csqlfn #NAD_tbox_tbox()
 */
Datum
nad_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure the validity of the arguments */
  assert(box1); assert(box2);
  if (! ensure_has_X(T_TBOX, box1->flags) || 
      ! ensure_has_X(T_TBOX, box2->flags) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return (box1->span.basetype == T_INT4) ?
      Int32GetDatum(-1) : Float8GetDatum(-1.0);

  /* If the boxes do not intersect in the time dimension return -1 */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
    return (box1->span.basetype == T_INT4) ?
      Int32GetDatum(-1) : Float8GetDatum(-1.0);

  return distance_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @return On error or if the time frames of the boxes do not overlap return -1
 * @csqlfn #NAD_tnumber_tbox()
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
    return (box->span.basetype == T_INT4) ?
      Int32GetDatum(-1) : Float8GetDatum(-1.0);
  }

  /* Project the temporal number to the timespan of the box (if any) */
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) : (Temporal *) temp;
  /* Test if the bounding boxes overlap */
  TBox box1;
  tnumber_set_tbox(temp1, &box1);
  if (overlaps_tbox_tbox(box, &box1))
    return (box->span.basetype == T_INT4) ?
      DatumGetInt32(0) : DatumGetFloat8(0.0);

  /* Get the minimum distance between the values of the boxes */
  Datum result = distance_value_value(box->span.lower, box1.span.upper,
    box->span.basetype);

  if (hast)
    pfree(temp1);

  return result;
}

/**
 * @brief Return the nearest approach distance between two temporal numbers
 */
Datum
nad_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(tnumber_type(temp1->temptype));
  Temporal *dist = distance_tnumber_tnumber(temp1, temp2);
  if (dist == NULL)
    return (temp1->temptype == T_TINT) ?
      Int32GetDatum(-1) : Float8GetDatum(-1.0);

  Datum result = temporal_min_value(dist);
  pfree(dist);
  return result;
}

/*****************************************************************************/
