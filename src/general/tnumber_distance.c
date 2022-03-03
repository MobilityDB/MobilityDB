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
 * Distance functions for temporal numbers.
 */

#include "general/tnumber_distance.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif

#include "general/period.h"
#include "general/timeops.h"
#include "general/rangetypes_ext.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/lifting.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * Returns the distance between the two numbers
 */
Datum
number_distance(Datum l, Datum r, Oid typel, Oid typer)
{
  Datum result = 0;
  if (typel == INT4OID)
  {
    if (typer == INT4OID)
      result = Int32GetDatum(abs(DatumGetInt32(l) - DatumGetInt32(r)));
    else if (typer == FLOAT8OID)
      result = Float8GetDatum(fabs(DatumGetInt32(l) - DatumGetFloat8(r)));
  }
  else if (typel == FLOAT8OID)
  {
    if (typer == INT4OID)
      result = Float8GetDatum(fabs(DatumGetFloat8(l) - DatumGetInt32(r)));
    else if (typer == FLOAT8OID)
      result = Float8GetDatum(fabs(DatumGetFloat8(l) - DatumGetFloat8(r)));
  }
  return result;
}

/*****************************************************************************/

/**
 * Returns the temporal distance between the temporal number and the
 * value (distpatch function)
 *
 * @param[in] temp Temporal number
 * @param[in] value Value
 * @param[in] basetypid Type of the base value
 * @param[in] restypid Type of the result
 */
static Temporal *
distance_tnumber_base_internal(const Temporal *temp, Datum value,
  Oid basetypid, Oid restypid)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &number_distance;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp->basetypid;
  lfinfo.argtypid[1] = basetypid;
  lfinfo.restypid = restypid;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = &tlinearsegm_intersection_value;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal_base(temp, value, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(distance_base_tnumber);
/**
 * Returns the temporal distance between the value and the temporal number
 */
PGDLLEXPORT Datum
distance_base_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Oid restypid = base_oid_from_temporal(temptypid);
  Temporal *result = distance_tnumber_base_internal(temp, value, basetypid,
    restypid);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tnumber_base);
/**
 * Returns the temporal distance between the temporal number and the
 * value
 */
PGDLLEXPORT Datum
distance_tnumber_base(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Oid restypid = base_oid_from_temporal(temptypid);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = distance_tnumber_base_internal(temp, value, basetypid,
    restypid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Returns true if the two segments of the temporal values intersect at the
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
 * Returns the temporal distance between the two temporal points
 * (dispatch function)
 *
 * @param[in] temp1,temp2 Temporal numbers
 * @param[in] restypid Type of the result
 */
static Temporal *
distance_tnumber_tnumber_internal(const Temporal *temp1, const Temporal *temp2,
  Oid restypid)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &number_distance;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp1->basetypid;
  lfinfo.argtypid[1] = temp2->basetypid;
  lfinfo.restypid = restypid;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = lfinfo.reslinear ? &tnumber_min_dist_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(distance_tnumber_tnumber);
/**
 * Returns the temporal distance between the two temporal points
 */
PGDLLEXPORT Datum
distance_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Oid restypid = base_oid_from_temporal(temptypid);
  Temporal *result = distance_tnumber_tnumber_internal(temp1, temp2, restypid);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * Returns the nearest approach distance between the temporal number and the
 * base value (internal function)
 */
static double
NAD_tnumber_base_internal(Temporal *temp, Datum value, Oid basetypid)
{
  ensure_tnumber_base_type(basetypid);
  TBOX box1, box2;
  temporal_bbox(temp, &box1);
  if (basetypid == INT4OID)
    int_tbox(DatumGetInt32(value), &box2);
  else /* basetypid == FLOAT8OID */
    float_tbox(DatumGetFloat8(value), &box2);
  return NAD_tbox_tbox_internal(&box1, &box2);
}

PG_FUNCTION_INFO_V1(NAD_base_tnumber);
/**
 * Returns the temporal distance between the value and the temporal number
 */
PGDLLEXPORT Datum
NAD_base_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = NAD_tnumber_base_internal(temp, value, basetypid);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_base);
/**
 * Returns the temporal distance between the temporal number and the
 * value
 */
PGDLLEXPORT Datum
NAD_tnumber_base(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  double result = NAD_tnumber_base_internal(temp, value, basetypid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/**
 * Returns the nearest approach distance between the temporal boxes
 * (internal function)
 */
double
NAD_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  /* Test the validity of the arguments */
  ensure_has_X_tbox(box1); ensure_has_X_tbox(box2);
  /* Project the boxes to their common timespan */
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  Period p1, p2;
  Period *inter;
  if (hast)
  {
    period_set(box1->tmin, box1->tmax, true, true, &p1);
    period_set(box2->tmin, box2->tmax, true, true, &p2);
    inter = intersection_period_period_internal(&p1, &p2);
    if (!inter)
      return DBL_MAX;
  }

  /* Convert the boxes to ranges */
  RangeType *range1 = range_make(Float8GetDatum(box1->xmin),
    Float8GetDatum(box1->xmax), true, true, FLOAT8OID);
  RangeType *range2 = range_make(Float8GetDatum(box2->xmin),
    Float8GetDatum(box2->xmax), true, true, FLOAT8OID);
  TypeCacheEntry *typcache = lookup_type_cache(range1->rangetypid,
    TYPECACHE_RANGE_INFO);
  /* Compute the result */
  double result;
  if (range_overlaps_internal(typcache, range1, range2))
    result = 0.0;
  else if (range_before_internal(typcache, range1, range2))
    result = box2->tmin - box1->tmax;
  else
    /* range_after_internal(typcache, range1, range2) */
    result = box1->tmin - box2->tmax;

  pfree(range1); pfree(range2);
  if (hast)
    pfree(inter);
  return result;
}

PG_FUNCTION_INFO_V1(NAD_tbox_tbox);
/**
 * Returns the nearest approach distance between the temporal boxes
 */
PGDLLEXPORT Datum
NAD_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  double result = NAD_tbox_tbox_internal(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/**
 * Returns the nearest approach distance between the temporal number and the
 * temporal box (internal function)
 */
static double
NAD_tnumber_tbox_internal(const Temporal *temp, TBOX *box)
{
  /* Test the validity of the arguments */
  ensure_has_X_tbox(box);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  Period p1, p2;
  Period *inter;
  if (hast)
  {
    temporal_period(temp, &p1);
    period_set(box->tmin, box->tmax, true, true, &p2);
    inter = intersection_period_period_internal(&p1, &p2);
    if (!inter)
      return DBL_MAX;
  }

  /* Project the temporal number to the timespan of the box (if any) */
  Temporal *temp1 = hast ?
    temporal_restrict_period_internal(temp, inter, REST_AT) : (Temporal *) temp;
  /* Test if the bounding boxes overlap */
  TBOX box1;
  temporal_bbox(temp1, &box1);
  if (overlaps_tbox_tbox_internal(box, &box1))
    return 0.0;

  /* Get the minimum distance between the values of the boxes */
  double result = (box->xmin > box1.xmax) ?
    fabs(box->xmin - box1.xmax) : fabs(box1.xmin - box->xmax);

  if (hast)
  {
    pfree(inter); pfree(temp1);
  }
  return result;
}

PG_FUNCTION_INFO_V1(NAD_tbox_tnumber);
/**
 * Returns the nearest approach distance between the temporal box and the
 * temporal number
 */
PGDLLEXPORT Datum
NAD_tbox_tnumber(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = NAD_tnumber_tbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_tbox);
/**
 * Returns the nearest approach distance between the temporal number and the
 * temporal box
 */
PGDLLEXPORT Datum
NAD_tnumber_tbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  double result = NAD_tnumber_tbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_tnumber);
/**
 * Returns the nearest approach distance between the temporal numbers
 */
PGDLLEXPORT Datum
NAD_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  ensure_tnumber_base_type(temp1->basetypid);
  ensure_tnumber_base_type(temp2->basetypid);
  /* Result of the distance function is a tint iff both arguments are tint */
  Oid restypid = (temp1->basetypid == INT4OID && temp2->basetypid == INT4OID) ?
    INT4OID : FLOAT8OID;
  Temporal *dist = distance_tnumber_tnumber_internal(temp1, temp2, restypid);
  if (dist == NULL)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Datum result = temporal_min_value_internal(dist);
  if (restypid == INT4OID)
    result = Float8GetDatum((double) DatumGetInt32(result));
  pfree(dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
