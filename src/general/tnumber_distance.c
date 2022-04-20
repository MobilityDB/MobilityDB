/***********************************************************************
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
 * @file tnumber_distance.c
 * @brief Distance functions for temporal numbers.
 */

#include "general/tnumber_distance.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
#include <math.h>
#include <utils/builtins.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/period.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/lifting.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * Return the distance between the two numbers
 */
Datum
number_distance(Datum l, Datum r, CachedType typel, CachedType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(abs(DatumGetInt32(l) - DatumGetInt32(r)));
    else if (typer == T_FLOAT8)
      result = Float8GetDatum(fabs(DatumGetInt32(l) - DatumGetFloat8(r)));
  }
  else if (typel == T_FLOAT8)
  {
    if (typer == T_INT4)
      result = Float8GetDatum(fabs(DatumGetFloat8(l) - DatumGetInt32(r)));
    else if (typer == T_FLOAT8)
      result = Float8GetDatum(fabs(DatumGetFloat8(l) - DatumGetFloat8(r)));
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_dist
 * @brief Return the temporal distance between the temporal number and the value.
 *
 * @param[in] temp Temporal number
 * @param[in] value Value
 * @param[in] valuetype Type of the value
 * @param[in] restype Type of the result
 */
Temporal *
distance_tnumber_number(const Temporal *temp, Datum value,
  CachedType valuetype, CachedType restype)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &number_distance;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = valuetype;
  lfinfo.restype = restype;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = &tlinearsegm_intersection_value;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal_base(temp, value, &lfinfo);
  return result;
}

/**
 * Return true if the two segments of the temporal values intersect at the
 * timestamp.
 *
 * This function is passed to the lifting infrastructure when computing the
 * temporal distance.
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 */
static bool
tnumber_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  if (! tsegment_intersection(start1, end1, LINEAR, start2, end2, LINEAR,
      NULL, NULL, t))
    return false;
  *value = (Datum) 0;
  return true;
}

/**
 * @ingroup libmeos_temporal_oper_dist
 * @brief Return the temporal distance between the two temporal points
 *
 * @param[in] temp1,temp2 Temporal numbers
 * @param[in] restype Type of the result
 */
Temporal *
distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  CachedType restype)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &number_distance;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp1->temptype);
  lfinfo.argtype[1] = temptype_basetype(temp2->temptype);
  lfinfo.restype = restype;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = lfinfo.reslinear ? &tnumber_min_dist_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_dist
 * @brief Return the nearest approach distance between the temporal number
 * and the base value.
 */
double
nad_tnumber_number(const Temporal *temp, Datum value, CachedType basetype)
{
  ensure_tnumber_basetype(basetype);
  TBOX box1, box2;
  temporal_bbox(temp, &box1);
  if (basetype == T_INT4)
    int_tbox(DatumGetInt32(value), &box2);
  else /* basetype == T_FLOAT8 */
    float_tbox(DatumGetFloat8(value), &box2);
  return nad_tbox_tbox(&box1, &box2);
}

/**
 * @ingroup libmeos_temporal_oper_dist
 * @brief Return the nearest approach distance between the temporal boxes.
 */
double
nad_tbox_tbox(const TBOX *box1, const TBOX *box2)
{
  /* Test the validity of the arguments */
  ensure_has_X_tbox(box1); ensure_has_X_tbox(box2);

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  if (hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax))
    return DBL_MAX;

  /* If the boxes intersect in the value dimension return 0 */
  if (box1->xmin <= box2->xmax && box2->xmin <= box1->xmax)
    return 0.0;

  if (box1->xmax < box2->xmin)
    /* box1 is to the left of box2 */
    return box2->xmin - box1->xmax;
  else
    /* box1 is to the right of box2 */
    return box1->xmin - box2->xmax;
}

/**
 * @ingroup libmeos_temporal_oper_dist
 * @brief Return the nearest approach distance between the temporal number
 * and the temporal box.
 */
double
nad_tnumber_tbox(const Temporal *temp, const TBOX *box)
{
  /* Test the validity of the arguments */
  ensure_has_X_tbox(box);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  Period p1, p2, inter;
  if (hast)
  {
    temporal_period(temp, &p1);
    period_set(box->tmin, box->tmax, true, true, &p2);
    if (! inter_period_period(&p1, &p2, &inter))
      return DBL_MAX;
  }

  /* Project the temporal number to the timespan of the box (if any) */
  Temporal *temp1 = hast ?
    temporal_restrict_period(temp, &inter, REST_AT) : (Temporal *) temp;
  /* Test if the bounding boxes overlap */
  TBOX box1;
  temporal_bbox(temp1, &box1);
  if (overlaps_tbox_tbox(box, &box1))
    return 0.0;

  /* Get the minimum distance between the values of the boxes */
  double result = (box->xmin > box1.xmax) ?
    fabs(box->xmin - box1.xmax) : fabs(box1.xmin - box->xmax);

  if (hast)
    pfree(temp1);

  return result;
}

/*****************************************************************************/
