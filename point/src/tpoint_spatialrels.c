/*****************************************************************************
 *
 * tpoint_spatialrels.c
 *	  Spatial relationships for temporal points.
 *
 * These relationships project the time dimension and return a Boolean.
 * They are thus defined with the "at any instant" semantics, that is, the
 * traditional spatial function is applied to the union of all values taken 
 * by the trajectory of the temporal point. The following relationships are supported for 
 * geometries:
 *		contains, containsproperly, covers, coveredby, crosses, disjoint, 
 *		contains, containsproperly, covers, coveredby, crosses, disjoint,
 *		equals, intersects, overlaps, touches, within, dwithin, and
 *		relate (with 2 and 3 arguments)
 * The following relationships are supported for geographies
 *	 covers, coveredby, intersects, dwithin
 * Only dwithin and intersects support 3D geometries.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_spatialrels.h"

#include <assert.h>
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

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
 * Calls the PostGIS function ST_ContainsProperly with the 2 arguments
 */
Datum
geom_containsproperly(Datum geom1, Datum geom2)
{
	return call_function2(containsproperly, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Covers with the 2 arguments
 */
Datum
geom_covers(Datum geom1, Datum geom2)
{
	return call_function2(covers, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Coveredby with the 2 arguments
 */
Datum
geom_coveredby(Datum geom1, Datum geom2)
{
	return call_function2(coveredby, geom1, geom2);
}

/**
 * Calls the PostGIS function ST_Crosses with the 2 arguments
 */
Datum
geom_crosses(Datum geom1, Datum geom2)
{
	return call_function2(crosses, geom1, geom2);
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
 * Calls the PostGIS function ST_Equals with the 2 arguments
 */
Datum
geom_equals(Datum geom1, Datum geom2)
{
	return call_function2(ST_Equals, geom1, geom2);
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
 * Calls the PostGIS function ST_Overlaps with the 2 arguments
 */
Datum
geom_overlaps(Datum geom1, Datum geom2)
{
	return call_function2(overlaps, geom1, geom2);
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
 * Calls the PostGIS function ST_Contains with the 2 arguments inverted
 */
Datum
geom_within(Datum geom1, Datum geom2)
{
	return call_function2(contains, geom2, geom1);
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

/**
 * Calls the PostGIS function ST_Relate with the 2 arguments
 */
Datum
geom_relate(Datum geom1, Datum geom2)
{
	return call_function2(relate_full, geom1, geom2); 
}

/**
 * Calls the PostGIS function ST_Relate with the 3 arguments
 */
Datum
geom_relate_pattern(Datum geom1, Datum geom2, Datum pattern)
{
	return call_function3(relate_pattern, geom1, geom2, pattern); 
}

/*****************************************************************************/

/**
 * Calls the PostGIS function ST_Covers for geographies with the 2 arguments
 */
Datum
geog_covers(Datum geog1, Datum geog2)
{
	return call_function2(geography_covers, geog1, geog2);
}

/**
 * Calls the PostGIS function ST_Covers for geographies with the 2 arguments inverted
 */
Datum
geog_coveredby(Datum geog1, Datum geog2)
{
	return call_function2(geography_covers, geog2, geog1);
}

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
	bool linear2, Datum dist, Datum (*func)(Datum, Datum, Datum))
{
	Datum sv1 = tinstant_value(start1);
	Datum ev1 = tinstant_value(end1);
	Datum sv2 = tinstant_value(start2);
	Datum ev2 = tinstant_value(end2);
	/* If both instants are constant compute the function at the start instant */
	if (datum_point_eq(sv1, ev1) &&	datum_point_eq(sv2, ev2))
		return DatumGetBool(func(sv1, sv2, dist));

	/* Determine whether there is a local minimum between lower and upper */
	TimestampTz crosstime;
	bool cross = tpointseq_min_dist_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	/* If there is no local minimum compute the function at the start instant */
	if (! cross)
		return DatumGetBool(func(sv1, sv2, dist));

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
dwithin_tpointseq_tpointseq(TSequence *seq1, TSequence *seq2, Datum dist,
	Datum (*func)(Datum, Datum, Datum))
{
	TInstant *start1 = tsequence_inst_n(seq1, 0);
	TInstant *start2 = tsequence_inst_n(seq2, 0);
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	for (int i = 1; i < seq1->count; i++)
	{
		TInstant *end1 = tsequence_inst_n(seq1, i);
		TInstant *end2 = tsequence_inst_n(seq2, i);
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
dwithin_tpointseqset_tpointseqset(TSequenceSet *ts1, TSequenceSet *ts2, Datum dist,
	Datum (*func)(Datum, Datum, Datum))
{
	for (int i = 0; i < ts1->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, i);
		if (dwithin_tpointseq_tpointseq(seq1, seq2, dist, func))
			return true;
	}
	return false;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Apply the variadic function to the values 
 *
 * @param[in] value1,value2 Values
 * @param[in] param Parameter for ternary relationships
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 */
Datum
spatialrel(Datum value1, Datum value2, Datum param, 
	Datum (*func)(Datum, ...), int numparam)
{
	if (numparam == 2)
		return (*func)(value1, value2);
	else if (numparam == 3)
		return (*func)(value1, value2, param);
	else
		elog(ERROR, "Number of function parameters not supported: %u", numparam);
}

/**
 * Generic spatial relationships for a temporal point and a geometry
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] geomfunc Function for geometries
 * @param[in] geogfunc Function for geographies
 * @param[in] invert True when the function is called with inverted arguments
 * @param[in] numparam Number of parameters of the function
 */
Datum
spatialrel_tpoint_geo1(Temporal *temp, GSERIALIZED *gs, Datum param,
	Datum (*geomfunc)(Datum, ...), Datum (*geogfunc)(Datum, ...), 
	int numparam, bool invert)
{
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Datum traj = tpoint_trajectory_internal(temp);
	Datum result;
	if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
	{
		assert(geogfunc != NULL);
		result = invert ? 
			spatialrel(PointerGetDatum(gs), traj, param, geogfunc, numparam) :
			spatialrel(traj, PointerGetDatum(gs), param, geogfunc, numparam);
	}
	else
	{
		assert(geomfunc != NULL);
		result = invert ? 
			spatialrel(PointerGetDatum(gs), traj, param, geomfunc, numparam) :
			spatialrel(traj, PointerGetDatum(gs), param, geomfunc, numparam);
	}
	pfree(DatumGetPointer(traj));
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
Datum
spatialrel_geo_tpoint(FunctionCallInfo fcinfo, Datum (*geomfunc)(Datum, ...),
	Datum (*geogfunc)(Datum, ...), int numparam)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum param = (numparam == 2) ? (Datum) NULL : PG_GETARG_DATUM(2);
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo1(temp, gs, param,
		(varfunc) geomfunc, (varfunc) geogfunc, numparam, true);
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
spatialrel_tpoint_geo(FunctionCallInfo fcinfo, Datum (*geomfunc)(Datum, ...), 
	Datum (*geogfunc)(Datum, ...), int numparam)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum param = (numparam == 2) ? (Datum) NULL : PG_GETARG_DATUM(2);
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo1(temp, gs, param,
		(varfunc) geomfunc, (varfunc) geogfunc, numparam, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

/**
 * Generic spatial relationships for temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] geomfunc Function for geometries
 * @param[in] geogfunc Function for geographies
 * @param[in] numparam Number of parameters of the functions
 */
Datum
spatialrel_tpoint_tpoint(FunctionCallInfo fcinfo, Datum (*geomfunc)(Datum, ...), 
	Datum (*geogfunc)(Datum, ...), int numparam)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum param = (numparam == 2) ? (Datum) NULL : PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, INTERSECT,
		&inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	Datum traj1 = tpoint_trajectory_internal(inter1);
	Datum traj2 = tpoint_trajectory_internal(inter2);
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result;
	if (MOBDB_FLAGS_GET_GEODETIC(temp1->flags))
	{
		assert(geogfunc != NULL);
		result = spatialrel(traj1, traj2, param, geogfunc, numparam);
	}
	else
	{
		assert(geomfunc != NULL);
		result = spatialrel(traj1, traj2, param, geomfunc, numparam);
	}
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2)); 
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
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
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_contains, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(contains_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point contains the geometry 
 */
PGDLLEXPORT Datum
contains_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_contains, NULL, 2);
}

PG_FUNCTION_INFO_V1(contains_tpoint_tpoint);
/**
 * Returns true if the trajectory of the first temporal point contains the
 * trajectory of the second one 
 */
PGDLLEXPORT Datum
contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_contains, 
		NULL, 2);
}
 
/*****************************************************************************
 * Temporal containsproperly
 *****************************************************************************/

PG_FUNCTION_INFO_V1(containsproperly_geo_tpoint);
/**
 * Returns true if the geometry contains properly the trajectory of the
 * temporal point
 */
PGDLLEXPORT Datum
containsproperly_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_containsproperly, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(containsproperly_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point contains properly
 * the geometry 
 */
PGDLLEXPORT Datum
containsproperly_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_containsproperly, NULL, 2);
}

PG_FUNCTION_INFO_V1(containsproperly_tpoint_tpoint);
/**
 * Returns true if the trajectory of the first temporal point contains properly
 * the trajectory the second one 
 */
PGDLLEXPORT Datum
containsproperly_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_containsproperly, 
		NULL, 2);
}
 
/*****************************************************************************
 * Temporal covers (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(covers_geo_tpoint);
/**
 * Returns true if the geometry covers the trajectory of the temporal point
 */
PGDLLEXPORT Datum
covers_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_covers,
		(varfunc) &geog_covers, 2);
}
 
PG_FUNCTION_INFO_V1(covers_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point covers the geometry 
 */
PGDLLEXPORT Datum
covers_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_covers, 
		(varfunc) &geog_covers, 2);
}

PG_FUNCTION_INFO_V1(covers_tpoint_tpoint);
/**
 * Returns true if the trajectory of the first temporal point covers the
 * trajectory of the second one 
 */
PGDLLEXPORT Datum
covers_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_covers,
		(varfunc) &geog_covers, 2);
}
 
/*****************************************************************************
 * Temporal coveredby (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(coveredby_geo_tpoint);
/**
 * Returns true if the geometry is covered by the trajectory of the temporal point
 */
PGDLLEXPORT Datum
coveredby_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_coveredby,
		(varfunc) &geog_coveredby, 2);
}
 
PG_FUNCTION_INFO_V1(coveredby_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point is covered by the geometry 
 */
PGDLLEXPORT Datum
coveredby_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_coveredby,
		(varfunc) &geog_coveredby, 2);
}

PG_FUNCTION_INFO_V1(coveredby_tpoint_tpoint);
/**
 * Returns true if the trajectory of the first temporal point is covered by
 * the trajectory of the second one 
 */
PGDLLEXPORT Datum
coveredby_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_coveredby, 
		(varfunc) &geog_coveredby, 2);
}
 
/*****************************************************************************
 * Temporal crosses
 *****************************************************************************/

PG_FUNCTION_INFO_V1(crosses_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point cross 
 */
PGDLLEXPORT Datum
crosses_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_crosses, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(crosses_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry cross 
 */
PGDLLEXPORT Datum
crosses_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_crosses, NULL, 2);
}

PG_FUNCTION_INFO_V1(crosses_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points cross
 */
PGDLLEXPORT Datum
crosses_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_crosses, 
		NULL, 2);
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
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_disjoint, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(disjoint_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry
 * are disjoint
 */
PGDLLEXPORT Datum
disjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_disjoint, NULL, 2);
}

PG_FUNCTION_INFO_V1(disjoint_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points are disjoint
 */
PGDLLEXPORT Datum
disjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_disjoint,
		NULL, 2);
}
 
/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(equals_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point are equal
 */
PGDLLEXPORT Datum
equals_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_equals, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(equals_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry are equal
 */
PGDLLEXPORT Datum
equals_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_equals, NULL, 2);
}

PG_FUNCTION_INFO_V1(equals_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points are equal
 */
PGDLLEXPORT Datum
equals_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_equals,
		NULL, 2);
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
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_intersects2d,
		(varfunc) geog_intersects, 2);
}
 
PG_FUNCTION_INFO_V1(intersects_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry
 * intersect 
 */
PGDLLEXPORT Datum
intersects_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_intersects2d,
		(varfunc) geog_intersects, 2);
}

PG_FUNCTION_INFO_V1(intersects_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points intersect
 */
PGDLLEXPORT Datum
intersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_intersects2d, 
		(varfunc) &geog_intersects, 2);
}
 
/*****************************************************************************
 * Temporal overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point overlap
 */
PGDLLEXPORT Datum
overlaps_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_overlaps, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(overlaps_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry overlap
 */
PGDLLEXPORT Datum
overlaps_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_overlaps, NULL, 2);
}

PG_FUNCTION_INFO_V1(overlaps_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points overlap
 */
PGDLLEXPORT Datum
overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_overlaps,
		NULL, 2);
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
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_touches, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(touches_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point touches the geometry
 */
PGDLLEXPORT Datum
touches_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_touches, NULL, 2);
}

PG_FUNCTION_INFO_V1(touches_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points touch each other
 */
PGDLLEXPORT Datum
touches_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_touches,
		NULL, 2);
}
 
/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(within_geo_tpoint);
/**
 * Returns true if the geometry is within the trajectory of the temporal point
 */
PGDLLEXPORT Datum
within_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_within, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(within_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point is within the geometry 
 */
PGDLLEXPORT Datum
within_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_within, NULL, 2);
}

PG_FUNCTION_INFO_V1(within_tpoint_tpoint);
/**
 * Returns true if the trajectory of the first temporal point is within
 * the trajectory of the second one
 */
PGDLLEXPORT Datum
within_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_within,
		NULL, 2);
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
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_dwithin2d,
		(varfunc) geog_dwithin, 3);
}
 
PG_FUNCTION_INFO_V1(dwithin_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry are
 * within the given distance
 */
PGDLLEXPORT Datum
dwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_dwithin2d, 
		(varfunc) geog_dwithin, 3);
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
	ensure_same_dimensionality_tpoint(temp1, temp2);
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

	Datum (*func)(Datum, Datum, Datum);
	if (MOBDB_FLAGS_GET_GEODETIC(temp1->flags))
		func = &geog_dwithin;
	else
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &geom_dwithin3d :
			&geom_dwithin2d;
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);

	bool result;
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == INSTANT || sync1->duration == INSTANTSET)
	{
		Datum traj1 = tpoint_trajectory_internal(sync1);
		Datum traj2 = tpoint_trajectory_internal(sync2);
		result = DatumGetBool(func(traj1, traj2, dist));
		pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2));
	}
	else if (sync1->duration == SEQUENCE) 
		result = dwithin_tpointseq_tpointseq((TSequence *)sync1,
			(TSequence *)sync2, dist, func);
	else /* sync1->duration == SEQUENCESET */
		result = dwithin_tpointseqset_tpointseqset((TSequenceSet *)sync1,
			(TSequenceSet *)sync2, dist, func);

	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal relate
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_geo_tpoint);
/**
 * Returns the DE-9IM matrix pattern of the geometry and the trajectory of
 * the temporal point
 */
PGDLLEXPORT Datum
relate_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_relate, NULL, 2);
}

PG_FUNCTION_INFO_V1(relate_tpoint_geo);
/**
 * Returns the DE-9IM matrix pattern of the trajectory of the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
relate_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_relate, NULL, 2);
}
 
PG_FUNCTION_INFO_V1(relate_tpoint_tpoint);
/**
 * Returns the DE-9IM matrix pattern of the trajectories of the temporal points
 */
PGDLLEXPORT Datum
relate_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_relate,
		NULL, 2);
}
 
/*****************************************************************************
 * Temporal relate_pattern
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_pattern_geo_tpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal point
 * satisfy the DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
relate_pattern_geo_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_geo_tpoint(fcinfo, (varfunc) &geom_relate_pattern, NULL, 3);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_geo);
/**
 * Returns true if the trajectory of the temporal point and the geometry
 * satisfy the DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
relate_pattern_tpoint_geo(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_geo(fcinfo, (varfunc) &geom_relate_pattern, NULL, 3);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_tpoint);
/**
 * Returns true if the trajectories of the temporal points satisfy the
 * DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
relate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return spatialrel_tpoint_tpoint(fcinfo, (varfunc) &geom_relate_pattern,
		NULL, 3);
}

/*****************************************************************************/

