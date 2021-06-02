/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file tpoint_spatialrels.c
 * Spatial relationships for temporal points.
 *
 * These relationships project the time dimension and return a Boolean.
 * They are thus defined with the "at any instant" semantics, that is, the
 * traditional spatial function is applied to the union of all values taken
 * by the trajectory of the temporal point. These functions are typically
 * used for filtering purposes, before applying the corresponding temporal
 * spatial relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `disjoint`, `intersects`, `touches`, and `dwithin`.
 *
 * The following relationships are supported for geographies: `intersects`,
 * `dwithin`.
 *
 * Only `dwithin` and `intersects` are supported for 3D geometries.
 */

#include "tpoint_spatialrels.h"

#include <assert.h>
#include "temporaltypes.h"
#include "tempcache.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_distance.h"

/*****************************************************************************
 * Spatial relationship functions
 * contains and within are inverse to each other
 * covers and coveredby are inverse to each other
 *****************************************************************************/

/**
 * Calls the PostGIS function ST_Contains with the 2 arguments
 */
Datum
geom_contains(Datum geom1, Datum geom2)
{
  return call_function2(contains, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Disjoint with the 2 arguments
 */
Datum
geom_disjoint(Datum geom1, Datum geom2)
{
  return call_function2(disjoint, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Intersects with the 2 arguments
 */
Datum
geom_intersects2d(Datum geom1, Datum geom2)
{
  return call_function2(intersects, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_3DIntersects with the 2 arguments
 */
Datum
geom_intersects3d(Datum geom1, Datum geom2)
{
  return call_function2(intersects3d, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Touches with the 2 arguments
 */
Datum
geom_touches(Datum geom1, Datum geom2)
{
  return call_function2(touches, geom1, geom2);
}

/**
 * Select the appropriate dwithin function
 */
datum_func3
get_dwithin_fn(int16 flags1, int16 flags2)
{
  datum_func3 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags1) && MOBDB_FLAGS_GET_GEODETIC(flags2))
    result = &geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    result = MOBDB_FLAGS_GET_Z(flags1) && MOBDB_FLAGS_GET_Z(flags2) ?
      &geom_dwithin3d : &geom_dwithin2d;
  return result;
}

/**
 * Calls the PostGIS function ST_DWithin with the 3 arguments
 */
Datum
geom_dwithin2d(Datum geom1, Datum geom2, Datum dist)
{
  return call_function3(LWGEOM_dwithin, geom1, geom2, dist);
}

/**
 * Calls the PostGIS function ST_3DDWithin with the 4 arguments
 */
Datum
geom_dwithin3d(Datum geom1, Datum geom2, Datum dist)
{
  return call_function3(LWGEOM_dwithin3d, geom1, geom2, dist);
}

/*****************************************************************************/

/**
 * Calls the PostGIS function ST_Intersects for geographies with the 2 arguments
 */
Datum
geog_intersects(Datum geog1, Datum geog2)
{
  /* We apply the same threshold as PostGIS in the definition of the
   * function ST_Intersects(geography, geography) */
  double dist = DatumGetFloat8(geog_distance(geog1, geog2));
  return BoolGetDatum(dist < DIST_EPSILON);
}

/**
 * Calls the PostGIS function ST_DWithin for geographies with the 2 arguments
 */
Datum
geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
  return CallerFInfoFunctionCall4(geography_dwithin, (fetch_fcinfo())->flinfo,
    InvalidOid, geog1, geog2, dist, BoolGetDatum(true));
}

/*****************************************************************************
 * Generic dwithin functions when both temporal points are moving
 * TODO: VERIFY THAT THESE FUNCTIONS CORRECT !!!
 *****************************************************************************/

/**
 * Returns true if the two segments of the temporal sequence points satisfy
 * the ST_Dwithin relationship
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[in] linear1,linear2 True when the corresponding segment has linear interpolation
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D, 3D, or geodetic)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointseq_tpointseq1(const TInstant *start1, const TInstant *end1,
  bool linear1, const TInstant *start2, const TInstant *end2,
  bool linear2, Datum dist, datum_func3 func)
{
  Datum startvalue1 = tinstant_value(start1);
  Datum endvalue1 = tinstant_value(end1);
  Datum startvalue2 = tinstant_value(start2);
  Datum endvalue2 = tinstant_value(end2);
  /* If both instants are constant compute the function at the start instant */
  if (datum_point_eq(startvalue1, endvalue1) &&  datum_point_eq(startvalue2, endvalue2))
    return DatumGetBool(func(startvalue1, startvalue2, dist));

  /* Determine whether there is a local minimum between lower and upper */
  TimestampTz crosstime;
  bool cross = tpointseq_min_dist_at_timestamp(start1, end1, linear1,
    start2, end2, linear2, &crosstime);
  /* If there is no local minimum compute the function at the start instant */
  if (! cross)
    return DatumGetBool(func(startvalue1, startvalue2, dist));

  /* Find the values at the local minimum */
  Datum crossvalue1 = tsequence_value_at_timestamp1(start1, end1, linear1, crosstime);
  Datum crossvalue2 = tsequence_value_at_timestamp1(start2, end2, linear2, crosstime);
  /* Compute the function at the local minimum */
  bool result = DatumGetBool(func(crossvalue1, crossvalue2, dist));

  pfree(DatumGetPointer(crossvalue1));
  pfree(DatumGetPointer(crossvalue2));

  return result;
}

/**
 * Returns true if the two temporal sequence points satisfy
 * the ST_Dwithin relationship
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D, 3D, or geodetic)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointseq_tpointseq(const TSequence *seq1, const TSequence *seq2,
  Datum dist, datum_func3 func)
{
  const TInstant *start1 = tsequence_inst_n(seq1, 0);
  const TInstant *start2 = tsequence_inst_n(seq2, 0);
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  for (int i = 1; i < seq1->count; i++)
  {
    const TInstant *end1 = tsequence_inst_n(seq1, i);
    const TInstant *end2 = tsequence_inst_n(seq2, i);
    if (dwithin_tpointseq_tpointseq1(start1, end1,
      linear1, start2, end2, linear2, dist, func))
      return true;
    start1 = end1;
    start2 = end2;
  }
  return DatumGetBool(func(tinstant_value(start1),
    tinstant_value(start2), dist));
}

/**
 * Returns true if the two temporal sequence set points satisfy
 * the ST_Dwithin relationship
 *
 * @param[in] ts1,ts2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D, 3D, or geodetic)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointseqset_tpointseqset(TSequenceSet *ts1, TSequenceSet *ts2,
  Datum dist, datum_func3 func)
{
  for (int i = 0; i < ts1->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, i);
    if (dwithin_tpointseq_tpointseq(seq1, seq2, dist, func))
      return true;
  }
  return false;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Generic spatial relationships for a temporal point and a geometry
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] geomfunc Function for geometries
 * @param[in] geogfunc Function for geographies
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 */
Datum
spatialrel_tpoint_geo1(Temporal *temp, Datum geo, Datum param,
  Datum (*func)(Datum, ...), int numparam, bool invert)
{
  Datum traj = tpoint_trajectory_internal(temp);  
  Datum result;
  assert(numparam == 2 || numparam == 3);
  if (numparam == 2)
    result = invert ? func(geo, traj) : func(traj, geo);
  else /* lfinfo.numparam == 3 */
    result = invert ? func(geo, traj, param) : func(traj, geo, param);
  tpoint_trajectory_free(temp, traj);
  return result;
}

/**
 * Generic spatial relationships for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] geomfunc Function for geometries
 * @param[in] geogfunc Function for geographies
 * @param[in] numparam Number of parameters of the functions
 */
static Datum
spatialrel_geo_tpoint(FunctionCallInfo fcinfo, Datum (*func)(Datum, ...),
  int numparam)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tpoint_gs(temp, gs);
  Datum param = (numparam == 2) ? (Datum) NULL : PG_GETARG_DATUM(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum result = spatialrel_tpoint_geo1(temp, PointerGetDatum(gs), param, func,
    numparam, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Generic spatial relationships for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] geomfunc Function for geometries
 * @param[in] geogfunc Function for geographies
 * @param[in] numparam Number of parameters of the functions
 */
Datum
spatialrel_tpoint_geo(FunctionCallInfo fcinfo, Datum (*func)(Datum, ...),
  int numparam)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_same_srid_tpoint_gs(temp, gs);
  Datum param = (numparam == 2) ? (Datum) NULL : PG_GETARG_DATUM(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum result = spatialrel_tpoint_geo1(temp, PointerGetDatum(gs), param, func,
    numparam, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_geo_tpoint);
/**
 * Returns true if the geometry contains the trajectory of the temporal point
 */
PGDLLEXPORT Datum
contains_geo_tpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_contains, 2);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(disjoint_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point
 * are disjoint
 */
PGDLLEXPORT Datum
disjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_disjoint, 2);
}

PG_FUNCTION_INFO_V1(disjoint_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry
 * are disjoint
 */
PGDLLEXPORT Datum
disjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_disjoint, 2);
}

/*****************************************************************************
 * Temporal intersects (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intersects_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point
 * intersect
 */
PGDLLEXPORT Datum
intersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  bool geod = (basetypid == type_oid(T_GEOGRAPHY));
  return spatialrel_geo_tpoint(fcinfo, geod ?
    (varfunc) &geog_intersects : (varfunc) &geom_intersects2d, 2);
}

PG_FUNCTION_INFO_V1(intersects_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry
 * intersect
 */
PGDLLEXPORT Datum
intersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  bool geod = (basetypid == type_oid(T_GEOGRAPHY));
  return spatialrel_tpoint_geo(fcinfo, geod ?
    (varfunc) &geog_intersects : (varfunc) &geom_intersects2d, 2);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(touches_geo_tpoint);
/**
 * Returns true if the geometry touches the trajectory of the temporal point
 */
PGDLLEXPORT Datum
touches_geo_tpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_touches, 2);
}

PG_FUNCTION_INFO_V1(touches_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point touches the geometry
 */
PGDLLEXPORT Datum
touches_tpoint_geo(PG_FUNCTION_ARGS)
{
  return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_touches, 2);
}

/*****************************************************************************
 * Temporal dwithin (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(dwithin_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point are
 * within the given distance
 */
PGDLLEXPORT Datum
dwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  bool geod = (basetypid == type_oid(T_GEOGRAPHY));
  return spatialrel_geo_tpoint(fcinfo, geod ?
    (varfunc) &geog_dwithin : (varfunc) &geom_dwithin2d, 3);
}

PG_FUNCTION_INFO_V1(dwithin_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry are
 * within the given distance
 */
PGDLLEXPORT Datum
dwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  bool geod = (basetypid == type_oid(T_GEOGRAPHY));
  return spatialrel_tpoint_geo(fcinfo, geod ?
    (varfunc) &geog_dwithin : (varfunc) &geom_dwithin2d, 3);
}

PG_FUNCTION_INFO_V1(dwithin_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points are within
 * the given distance
 */
PGDLLEXPORT Datum
dwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  ensure_same_srid_tpoint(temp1, temp2);
  Temporal *sync1, *sync2;
  /* Returns false if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE,
    &sync1, &sync2))
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  /* Select the appropriate dwithin function */
  datum_func3 func = get_dwithin_fn(temp1->flags, temp2->flags);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);

  bool result;
  ensure_valid_tempsubtype(sync1->subtype);
  if (sync1->subtype == INSTANT || sync1->subtype == INSTANTSET)
  {
    Datum traj1 = tpoint_trajectory_internal(sync1);
    Datum traj2 = tpoint_trajectory_internal(sync2);
    result = DatumGetBool(func(traj1, traj2, dist));
    tpoint_trajectory_free(sync1, traj1);
    tpoint_trajectory_free(sync2, traj2);
  }
  else if (sync1->subtype == SEQUENCE)
    result = dwithin_tpointseq_tpointseq((TSequence *)sync1,
      (TSequence *)sync2, dist, func);
  else /* sync1->subtype == SEQUENCESET */
    result = dwithin_tpointseqset_tpointseqset((TSequenceSet *)sync1,
      (TSequenceSet *)sync2, dist, func);

  pfree(sync1); pfree(sync2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
