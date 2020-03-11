/*****************************************************************************
 *
 * tpoint_spatialfuncs.c
 *	  Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_spatialfuncs.h"

#include <assert.h>
#include <float.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tnumber_mathfuncs.h"
#include "tpoint.h"
#include "tpoint_boxops.h"
#include "tpoint_distance.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

void
ensure_same_geodetic_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
		MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "The boxes must be both planar or both geodetic");
}

void
ensure_same_geodetic_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(box->flags) &&
		MOBDB_FLAGS_GET_GEODETIC(temp->flags) != MOBDB_FLAGS_GET_GEODETIC(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be both planar or both geodetic")));
}

void
ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
		box1->srid != box2->srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must be in the same SRID")));
}

void
ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(box->flags) &&
		tpoint_srid_internal(temp) != box->srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be in the same SRID")));
}

void
ensure_same_srid_tpoint(const Temporal *temp1, const Temporal *temp2)
{
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal points must be in the same SRID")));
}

void
ensure_same_srid_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the geometry must be in the same SRID")));
}

void
ensure_same_dimensionality_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
		MOBDB_FLAGS_GET_Z(box1->flags) != MOBDB_FLAGS_GET_Z(box2->flags) ||
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must be of the same dimensionality")));
}

void
ensure_same_dimensionality_tpoint(const Temporal *temp1, const Temporal *temp2)
{
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal points must be of the same dimensionality")));
}

void
ensure_common_dimension_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) &&
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must have at least one common dimension")));
}

void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the geometry must be of the same dimensionality")));
}

void
ensure_has_X_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_X(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have XY dimension")));
}

void
ensure_has_Z_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_Z(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have Z dimension")));
}

void
ensure_has_T_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_T(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have time dimension")));
}

void
ensure_has_Z_tpoint(const Temporal *temp)
{
	if (! MOBDB_FLAGS_GET_Z(temp->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point must have Z dimension")));
}

void
ensure_has_Z_gs(const GSERIALIZED *gs)
{
	if (! FLAGS_GET_Z(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries with Z dimension accepted")));
}

void
ensure_has_M_gs(const GSERIALIZED *gs)
{
	if (! FLAGS_GET_M(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries with M dimension accepted")));
}

void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
	if (FLAGS_GET_M(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries without M dimension accepted")));
}

void
ensure_point_type(const GSERIALIZED *gs)
{
	if (gserialized_get_type(gs) != POINTTYPE)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only point geometries accepted")));
}

void
ensure_non_empty(const GSERIALIZED *gs)
{
	if (gserialized_is_empty(gs))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only non-empty geometries accepted")));
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/*
 * Manipulate a geometry point directly from the GSERIALIZED.
 * These functions consitutute a SERIOUS break of encapsulation but it is the
 * only way to achieve reasonable performance when manipulating mobility data.
 * Currently we do not manipulate points with M dimension.
 * The datum_* functions suppose that the GSERIALIZED has been already
 * detoasted. This is typically the case when the datum is within a Temporal *
 * that has been already detoasted with PG_GETARG_TEMPORAL* 
 */

/* Get 2D point from a serialized geometry */

POINT2D
gs_get_point2d(GSERIALIZED *gs)
{
	POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 2D point from a datum */
POINT2D
datum_get_point2d(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
	POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 3DZ point from a serialized geometry */

POINT3DZ
gs_get_point3dz(GSERIALIZED *gs)
{
	POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 3DZ point from a datum */

POINT3DZ
datum_get_point3dz(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
	// POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
	// return *point;
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	LWPOINT* lwpoint = lwgeom_as_lwpoint(lwgeom);
	POINT3DZ point = getPoint3dz(lwpoint->point, 0);
	lwgeom_free(lwgeom);
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
	return point;

}

/* Compare two points from serialized geometries */

bool
datum_point_eq(Datum geopoint1, Datum geopoint2)
{
	GSERIALIZED *gs1 = (GSERIALIZED *) DatumGetPointer(geopoint1);
	GSERIALIZED *gs2 = (GSERIALIZED *) DatumGetPointer(geopoint2);
	assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2) &&
		FLAGS_GET_Z(gs1->flags) == FLAGS_GET_Z(gs2->flags) &&
		FLAGS_GET_GEODETIC(gs1->flags) == FLAGS_GET_GEODETIC(gs2->flags));
	if (FLAGS_GET_Z(gs1->flags))
	{
		POINT3DZ point1 = gs_get_point3dz(gs1);
		POINT3DZ point2 = gs_get_point3dz(gs2);
		return point1.x == point2.x && point1.y == point2.y &&
			point1.z == point2.z;
	}
	else
	{
		POINT2D point1 = gs_get_point2d(gs1);
		POINT2D point2 = gs_get_point2d(gs2);
		return point1.x == point2.x && point1.y == point2.y;
	}
}

static Datum
datum_set_precision(Datum value, Datum size)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
	int srid = gserialized_get_srid(gs);
	LWPOINT *lwpoint;
	if (FLAGS_GET_Z(gs->flags))
	{
		POINT3DZ point = gs_get_point3dz(gs);
		double x = DatumGetFloat8(datum_round(Float8GetDatum(point.x), size));
		double y = DatumGetFloat8(datum_round(Float8GetDatum(point.y), size));
		double z = DatumGetFloat8(datum_round(Float8GetDatum(point.z), size));
		lwpoint = lwpoint_make3dz(srid, x, y, z);
	}
	else
	{
		POINT2D point = gs_get_point2d(gs);
		double x = DatumGetFloat8(datum_round(Float8GetDatum(point.x), size));
		double y = DatumGetFloat8(datum_round(Float8GetDatum(point.y), size));
		lwpoint = lwpoint_make2d(srid, x, y);
	}
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *)lwpoint));
	pfree(lwpoint);
	return result;
}

/* Serialize a geometry */

GSERIALIZED *
geometry_serialize(LWGEOM *geom)
{
	size_t size;
	GSERIALIZED *result = gserialized_from_lwgeom(geom, &size);
	SET_VARSIZE(result, size);
	return result;
}

/* Call to PostGIS external functions */

static Datum
datum_transform(Datum value, Datum srid)
{
	return call_function2(transform, value, srid);
}

static Datum
geog_to_geom(Datum value)
{
	return call_function1(geometry_from_geography, value);
}

static Datum
geom_to_geog(Datum value)
{
	return call_function1(geography_from_geometry, value);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/* Get the spatial reference system identifier (SRID) of a temporal point */

int
tpointinst_srid(const TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(inst));
	return gserialized_get_srid(gs);
}

int
tpointi_srid(const TemporalI *ti)
{
	STBOX *box = temporali_bbox_ptr(ti);
	return box->srid;
}

int
tpointseq_srid(const TemporalSeq *seq)
{
	STBOX *box = temporalseq_bbox_ptr(seq);
	return box->srid;
}

int
tpoints_srid(const TemporalS *ts)
{
	STBOX *box = temporals_bbox_ptr(ts);
	return box->srid;
}

int
tpoint_srid_internal(const Temporal *temp)
{
	int result = 0;
	ensure_valid_duration(temp->duration);
	ensure_point_base_type(temp->valuetypid);
	if (temp->duration == TEMPORALINST)
		result = tpointinst_srid((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_srid((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_srid((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = tpoints_srid((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_srid);

PGDLLEXPORT Datum
tpoint_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result = tpoint_srid_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

/*****************************************************************************/

/* Set the spatial reference system identifier (SRID) of a temporal point */

/* TemporalInst */

static TemporalInst *
tpointinst_set_srid(TemporalInst *inst, int32 srid)
{
	TemporalInst *result = temporalinst_copy(inst);
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(result));
	gserialized_set_srid(gs, srid);
	return result;
}

static TemporalI *
tpointi_set_srid(TemporalI *ti, int32 srid)
{
	TemporalI *result = temporali_copy(ti);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(inst));
		gserialized_set_srid(gs, srid);
	}
	STBOX *box = temporali_bbox_ptr(result);
	box->srid = srid;
	return result;
}

static TemporalSeq *
tpointseq_set_srid(TemporalSeq *seq, int32 srid)
{
	TemporalSeq *result = temporalseq_copy(seq);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(inst));
		gserialized_set_srid(gs, srid);
	}
	STBOX *box = temporalseq_bbox_ptr(result);
	box->srid = srid;
	return result;
}

static TemporalS *
tpoints_set_srid(TemporalS *ts, int32 srid)
{
	TemporalS *result = temporals_copy(ts);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(inst));
			gserialized_set_srid(gs, srid);
		}
	}
	STBOX *box = temporals_bbox_ptr(result);
	box->srid = srid;
	return result;
}

Temporal *
tpoint_set_srid_internal(Temporal *temp, int32 srid)
{
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_set_srid((TemporalInst *)temp, srid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_set_srid((TemporalI *)temp, srid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_set_srid((TemporalSeq *)temp, srid);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_set_srid((TemporalS *)temp, srid);

	assert(result != NULL);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_set_srid);

PGDLLEXPORT Datum
tpoint_set_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 srid = PG_GETARG_INT32(1);
	Temporal *result = tpoint_set_srid_internal(temp, srid) ;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Transform a temporal geometry point into another spatial reference system */

TemporalInst *
tgeompointinst_transform(TemporalInst *inst, Datum srid)
{
	return tfunc2_temporalinst(inst, srid, &datum_transform,
		type_oid(T_GEOMETRY));
}

PG_FUNCTION_INFO_V1(tpoint_transform);

PGDLLEXPORT Datum
tpoint_transform(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum srid = PG_GETARG_DATUM(1);
	Temporal *result = tfunc2_temporal(temp, srid, &datum_transform,
		type_oid(T_GEOMETRY));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

/* Geometry to Geography */

PG_FUNCTION_INFO_V1(tgeompoint_to_tgeogpoint);

PGDLLEXPORT Datum
tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &geom_to_geog,
		type_oid(T_GEOGRAPHY));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Geography to Geometry */

TemporalInst *
tgeogpointinst_to_tgeompointinst(TemporalInst *inst)
{
	return tfunc1_temporalinst(inst, &geog_to_geom, type_oid(T_GEOMETRY));
}

TemporalSeq *
tgeogpointseq_to_tgeompointseq(TemporalSeq *seq)
{
	return tfunc1_temporalseq(seq, &geog_to_geom, type_oid(T_GEOMETRY));
}

TemporalS *
tgeogpoints_to_tgeompoints(TemporalS *ts)
{
	return tfunc1_temporals(ts, &geog_to_geom, type_oid(T_GEOMETRY));
}

PG_FUNCTION_INFO_V1(tgeogpoint_to_tgeompoint);

PGDLLEXPORT Datum
tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &geog_to_geom,
		type_oid(T_GEOMETRY));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set precision of the coordinates.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_set_precision);

PGDLLEXPORT Datum
tpoint_set_precision(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum size = PG_GETARG_DATUM(1);
	Temporal *result = tfunc2_temporal(temp, size, &datum_set_precision,
		temp->valuetypid);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Trajectory functions.
 *****************************************************************************/

/* Compute the trajectory from the points of two consecutive instants with
 * linear interpolation. The functions are called during normalization for
 * determining whether three consecutive points are collinear, for computing
 * the temporal distance, the temporal spatial relationships, etc. */

Datum
geompoint_trajectory(Datum value1, Datum value2)
{
	GSERIALIZED *gs1 = (GSERIALIZED *)DatumGetPointer(value1);
	GSERIALIZED *gs2 = (GSERIALIZED *)DatumGetPointer(value2);
	LWGEOM *geoms[2];
	geoms[0] = lwgeom_from_gserialized(gs1);
	geoms[1] = lwgeom_from_gserialized(gs2);
	LWGEOM *traj = (LWGEOM *)lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);
	GSERIALIZED *result = geometry_serialize(traj);
	lwgeom_free(geoms[0]); lwgeom_free(geoms[1]); lwgeom_free(traj);
	return PointerGetDatum(result);
}

Datum
geogpoint_trajectory(Datum value1, Datum value2)
{
	Datum geom1 = call_function1(geometry_from_geography, value1);
	Datum geom2 = call_function1(geometry_from_geography, value2);
	Datum geom = geompoint_trajectory(geom1, geom2);
	Datum result = call_function1(geography_from_geometry, geom);
	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(DatumGetPointer(geom));
	return result;
}

/*****************************************************************************/

/* Compute a trajectory from a set of points. The result is either a line or a
 * multipoint depending on whether the interpolation is stepwise or linear */
static Datum
pointarr_make_trajectory(Datum *points, int count, bool linear)
{
	LWGEOM **lwpoints = palloc(sizeof(LWGEOM *) * count);
	for (int i = 0; i < count; i++)
	{
		GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(points[i]);
		lwpoints[i] = lwgeom_from_gserialized(gs);
	}
	LWGEOM *geom;
	if (linear)
		geom = (LWGEOM *) lwline_from_lwgeom_array(lwpoints[0]->srid, (uint32_t) count, lwpoints);
	else
		geom = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, lwpoints[0]->srid,
			NULL, (uint32_t) count, lwpoints);
	Datum result = PointerGetDatum(geometry_serialize(geom));
	for (int i = 0; i < count; i++)
		lwgeom_free(lwpoints[i]);
	pfree(lwpoints); pfree(geom);
	return result;
}

/* Compute the trajectory of an array of instants.
 * This function is called by the constructor of a temporal sequence and
 * returns a single Datum which is a geometry */
Datum
tpointseq_make_trajectory(TemporalInst **instants, int count, bool linear)
{
	Oid valuetypid = instants[0]->valuetypid;
	ensure_point_base_type(valuetypid);
	bool geometry = (valuetypid == type_oid(T_GEOMETRY));
	Datum *points = palloc(sizeof(Datum) * count);
	int k;
	if (linear)
	{
		/* Remove two consecutive points if they are equal */
		Datum value1 = geometry ? temporalinst_value(instants[0]) :
			call_function1(geometry_from_geography, temporalinst_value(instants[0]));
		points[0] = value1;
		k = 1;
		for (int i = 1; i < count; i++)
		{
			Datum value2 = geometry ? temporalinst_value(instants[i]) :
				call_function1(geometry_from_geography, temporalinst_value(instants[i]));
			if (!datum_point_eq(value1, value2))
				points[k++] = value2;
			value1 = value2;
		}
	}
	else
	{
		 /* Remove all duplicate points */
		k = 0;
		for (int i = 0; i < count; i++)
		{
			Datum value = geometry ? temporalinst_value(instants[i]) :
				call_function1(geometry_from_geography, temporalinst_value(instants[i]));
			bool found = false;
			for (int j = 0; j < k; j++)
			{
				if (datum_point_eq(value, points[j]))
				{
					found = true;
					break;
				}
			}
			if (!found)
				points[k++] = value;
		}
	}
	Datum result;
	if (geometry)
	{
		if (k == 1)
			result = PointerGetDatum(gserialized_copy(
				(GSERIALIZED *) DatumGetPointer(points[0])));
		else
			result = pointarr_make_trajectory(points, k, linear);
	}
	else
	{
		Datum geomresult;
		if (k == 1)
			geomresult = points[0];
		else
			geomresult = pointarr_make_trajectory(points, k, linear);
		result = call_function1(geography_from_geometry, geomresult);
	}
	if (! geometry)
		for (int i = 0; i < k; i++)
			pfree(DatumGetPointer(points[i]));
	pfree(points);
	return result;	
}

/* Get the precomputed trajectory of a tpointseq */

Datum
tpointseq_trajectory(const TemporalSeq *seq)
{
	void *traj = (char *)(&seq->offsets[seq->count + 2]) + 	/* start of data */
			seq->offsets[seq->count + 1];					/* offset */
	return PointerGetDatum(traj);
}

/* Add or replace a point to the trajectory of a sequence */

Datum
tpointseq_trajectory_append(const TemporalSeq *seq, const TemporalInst *inst, bool replace)
{
	Datum traj = tpointseq_trajectory(seq);
	Datum point = temporalinst_value(inst);
	GSERIALIZED *gstraj = (GSERIALIZED *) DatumGetPointer(traj);
	if (gserialized_get_type(gstraj) == POINTTYPE)
	{
		if (datum_point_eq(traj, point))
			return PointerGetDatum(gserialized_copy(gstraj));
		else
		{
			if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
				return geompoint_trajectory(traj, point);
			else
			{
				Datum points[2];
				points[0] = traj;
				points[1] = point;
				return pointarr_make_trajectory(points, 2, false);
			}
		}
	}
	else if (gserialized_get_type(gstraj) == MULTIPOINTTYPE)
	{
		int count = replace ? seq->count : seq->count + 1;
		Datum *points = palloc(sizeof(Datum) * count);
		 /* Remove all duplicate points */
		int k = 0;
		bool found;
		for (int i = 0; i < count - 1; i++)
		{
			Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
			found = false;
			for (int j = 0; j < k; j++)
			{
				if (datum_point_eq(value, points[j]))
				{
					found = true;
					break;
				}
			}
			if (!found)
				points[k++] = value;
		}
		found = false;
		for (int i = 0; i < k; i++)
		{
			if (datum_point_eq(point, points[i]))
			{
				found = true;
				break;
			}
		}
		if (!found)
			points[k++] = point;
		return pointarr_make_trajectory(points, k, false);
	}
	/* The trajectory is a Linestring */
	else
	{
		if (replace)
		{
			int numpoints = DatumGetInt32(call_function1(LWGEOM_numpoints_linestring, traj));
			return call_function3(LWGEOM_setpoint_linestring, traj,
				Int32GetDatum(numpoints - 1), point);
		}
		else
			return call_function2(LWGEOM_addpoint, traj, point);
	}
}

/* Join two trajectories */

Datum 
tpointseq_trajectory_join(const TemporalSeq *seq1, const TemporalSeq *seq2, bool last, bool first)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq1->flags) == MOBDB_FLAGS_GET_LINEAR(seq2->flags));
	int count1 = last ? seq1->count - 1 : seq1->count;
	int start2 = first ? 1 : 0;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 
		(count1 + seq2->count - start2));
	int k = 0;
	for (int i = 0; i < count1; i++)
		instants[k++] = temporalseq_inst_n(seq1, i);
	for (int i = start2; i < seq2->count; i++)
		instants[k++] = temporalseq_inst_n(seq2, i);
	Datum traj = tpointseq_make_trajectory(instants, k, MOBDB_FLAGS_GET_LINEAR(seq1->flags));
	pfree(instants);

	return traj;
}

/* Copy the precomputed trajectory of a tpointseq */

Datum
tpointseq_trajectory_copy(const TemporalSeq *seq)
{
	void *traj = (char *)(&seq->offsets[seq->count + 2]) + 	/* start of data */
			seq->offsets[seq->count + 1];					/* offset */
	return PointerGetDatum(gserialized_copy(traj));
}

/*****************************************************************************/

/* Compute the trajectory of a tpoints from the precomputed trajectories
   of its composing segments. The resulting trajectory must be freed by the
   calling function. The function removes duplicates points */

static Datum
tgeompoints_trajectory(TemporalS *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_trajectory_copy(temporals_seq_n(ts, 0));
	
	Datum *points = palloc(sizeof(Datum) * ts->totalcount);
	Datum *trajectories = palloc(sizeof(Datum) * ts->count);
	int k = 0, l = 0;
	for (int i = 0; i < ts->count; i++)
	{
		Datum traj = tpointseq_trajectory(temporals_seq_n(ts, i));
		GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
		if (gserialized_get_type(gstraj) == POINTTYPE)
		{
			bool found = false;
			for (int j = 0; j < l; j++)
			{
				if (datum_point_eq(traj, points[j]))
				{
					found = true;
					break;
				}
			}
			if (!found)
				points[l++] = traj;
		}
		else if (gserialized_get_type(gstraj) == MULTIPOINTTYPE)
		{
			int count = DatumGetInt32(call_function1(LWGEOM_numgeometries_collection, traj));
			for (int m = 1; m <= count; m++)
			{
				Datum point = call_function2(LWGEOM_geometryn_collection, traj, Int32GetDatum(m));
				bool found = false;
				for (int j = 0; j < l; j++)
				{
					if (datum_point_eq(point, points[j]))
					{
						found = true;
						break;
					}
				}
				if (!found)
					points[l++] = point;
			}
		}
		/* gserialized_get_type(gstraj) == LINETYPE */
		else
			trajectories[k++] = traj;
	}
	Datum result;
	if (k == 0)
	{
		/* Only points */
		if (l == 1)
			result = PointerGetDatum(gserialized_copy(
					(GSERIALIZED *)(DatumGetPointer(points[0]))));
		else
			result = pointarr_make_trajectory(points, l, false);
	}
	else if (l == 0)
	{
		/* Only lines */
		if (k == 1)
			result = PointerGetDatum(gserialized_copy(
					(GSERIALIZED *)(DatumGetPointer(trajectories[0]))));
		else
		{
			ArrayType *array = datumarr_to_array(trajectories, k, type_oid(T_GEOMETRY));
			/* ST_linemerge is not used to avoid splitting lines at intersections */
			result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
			pfree(array);
		}
	}
	else
	{
		/* Both points and lines */
		if (l == 1)
			trajectories[k++] = points[0];
		else
			trajectories[k++] = pointarr_make_trajectory(points, l, false);
		ArrayType *array = datumarr_to_array(trajectories, k, type_oid(T_GEOMETRY));
		/* ST_linemerge is not used to avoid splitting lines at intersections */
		result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
		pfree(array);
	}
	pfree(points); pfree(trajectories);
	return result;
}

static Datum
tgeogpoints_trajectory(TemporalS *ts)
{
	TemporalS *tsgeom = tfunc1_temporals(ts, &geog_to_geom,
		type_oid(T_GEOMETRY));
	Datum geomtraj = tgeompoints_trajectory(tsgeom);
	Datum result = call_function1(geography_from_geometry, geomtraj);
	pfree(DatumGetPointer(geomtraj));
	return result;
}

Datum
tpoints_trajectory(TemporalS *ts)
{
	Datum result = 0;
	ensure_point_base_type(ts->valuetypid);
	if (ts->valuetypid == type_oid(T_GEOMETRY))
		result = tgeompoints_trajectory(ts);
	else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		result = tgeogpoints_trajectory(ts);
	return result;
}

/*****************************************************************************/

Datum
tpoint_trajectory_internal(const Temporal *temp)
{
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_values((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_trajectory_copy((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = tpoints_trajectory((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_trajectory);

PGDLLEXPORT Datum
tpoint_trajectory(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tpoint_trajectory_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/* Length traversed by the temporal point */

static double
tpointseq_length(TemporalSeq *seq)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
	Datum traj = tpointseq_trajectory(seq);
	GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
	if (gserialized_get_type(gstraj) == POINTTYPE)
		return 0;
	
	/* We are sure that the trajectory is a line */
	double result = 0.0;
	ensure_point_base_type(seq->valuetypid);
	if (seq->valuetypid == type_oid(T_GEOMETRY))
		/* The next function call works for 2D and 3D */
		result = DatumGetFloat8(call_function1(LWGEOM_length_linestring, traj));
	else
		result = DatumGetFloat8(call_function2(geography_length, traj,
			BoolGetDatum(true)));
	return result;
}

static double
tpoints_length(TemporalS *ts)
{
	assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tpointseq_length(temporals_seq_n(ts, i));
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_length);

PGDLLEXPORT Datum
tpoint_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result = 0.0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI ||
		(temp->duration == TEMPORALSEQ && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
		(temp->duration == TEMPORALS && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
		;
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_length((TemporalSeq *)temp);	
	else if (temp->duration == TEMPORALS)
		result = tpoints_length((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

/* Cumulative length traversed by the temporal point */

static TemporalInst *
tpointinst_cumulative_length(TemporalInst *inst)
{
	return temporalinst_make(Float8GetDatum(0.0), inst->t, FLOAT8OID);
}

static TemporalI *
tpointi_cumulative_length(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	Datum length = Float8GetDatum(0.0);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = temporalinst_make(length, inst->t, FLOAT8OID);
	}
	TemporalI *result = temporali_make(instants, ti->count);
	for (int i = 1; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalSeq *
tpointseq_cumulative_length(TemporalSeq *seq, double prevlength)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_make(Float8GetDatum(0), inst->t,
			FLOAT8OID);
		TemporalSeq *result = temporalseq_make(&inst1, 1,
			true, true, MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
		pfree(inst1);
		return result;
	}

	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	/* Stepwise interpolation */
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
	{
		Datum length = Float8GetDatum(0.0);
		for (int i = 0; i < seq->count; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			instants[i] = temporalinst_make(length, inst->t, FLOAT8OID);
		}
	}
	else
	/* Linear interpolation */
	{
		Datum (*func)(Datum, Datum);
		ensure_point_base_type(seq->valuetypid);
		if (seq->valuetypid == type_oid(T_GEOMETRY))
		{
			if (MOBDB_FLAGS_GET_Z(seq->flags))
				func = &geom_distance3d;
			else
				func = &geom_distance2d;
		}
		else
			func = &geog_distance;

		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		Datum value1 = temporalinst_value(inst1);
		double length = prevlength;
		instants[0] = temporalinst_make(Float8GetDatum(length), inst1->t,
				FLOAT8OID);
		for (int i = 1; i < seq->count; i++)
		{
			TemporalInst *inst2 = temporalseq_inst_n(seq, i);
			Datum value2 = temporalinst_value(inst2);
			if (datum_ne(value1, value2, inst1->valuetypid))
				length += DatumGetFloat8(func(value1, value2));
			instants[i] = temporalinst_make(Float8GetDatum(length), inst2->t,
				FLOAT8OID);
			inst1 = inst2;
			value1 = value2;
		}
	}
	TemporalSeq *result = temporalseq_make(instants,
		seq->count, seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
		
	for (int i = 1; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

static TemporalS *
tpoints_cumulative_length(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	double length = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tpointseq_cumulative_length(seq, length);
		TemporalInst *end = temporalseq_inst_n(sequences[i], seq->count - 1);
		length += DatumGetFloat8(temporalinst_value(end));
	}
	TemporalS *result = temporals_make(sequences, ts->count, false);
		
	for (int i = 1; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_cumulative_length);

PGDLLEXPORT Datum
tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_cumulative_length((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_cumulative_length((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_cumulative_length((TemporalSeq *)temp, 0);	
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_cumulative_length((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

static TemporalSeq *
tpointseq_speed(TemporalSeq *seq)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	/* Stepwise interpolation */
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
	{
		Datum length = Float8GetDatum(0.0);
		for (int i = 0; i < seq->count; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			instants[i] = temporalinst_make(length, inst->t, FLOAT8OID);
		}
	}
	else
	/* Linear interpolation */
	{
		Datum (*func)(Datum, Datum);
		ensure_point_base_type(seq->valuetypid);
		if (seq->valuetypid == type_oid(T_GEOMETRY))
		{
			if (MOBDB_FLAGS_GET_Z(seq->flags))
				func = &geom_distance3d;
			else
				func = &geom_distance2d;
		}
		else
			func = &geog_distance;

		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		Datum value1 = temporalinst_value(inst1);
		double speed;
		for (int i = 0; i < seq->count - 1; i++)
		{
			TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
			Datum value2 = temporalinst_value(inst2);
			if (datum_point_eq(value1, value2))
				speed = 0;
			else
				speed = DatumGetFloat8(func(value1, value2)) / ((double)(inst2->t - inst1->t) / 1000000);
			instants[i] = temporalinst_make(Float8GetDatum(speed), inst1->t,
				FLOAT8OID);
			inst1 = inst2;
			value1 = value2;
		}			
		instants[seq->count - 1] = temporalinst_make(Float8GetDatum(speed),
			seq->period.upper, FLOAT8OID);
	}
	/* The resulting sequence has stepwise interpolation */
	TemporalSeq *result = temporalseq_make(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc, false, true);
	for (int i = 0; i < seq->count - 1; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalS *
tpoints_speed(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (seq->count > 1)
			sequences[k++] = tpointseq_speed(seq);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* The resulting sequence set has stepwise interpolation */
	TemporalS *result = temporals_make(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_speed);

PGDLLEXPORT Datum
tpoint_speed(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI)
		;
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_speed((TemporalSeq *)temp);	
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_speed((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

Datum
tgeompointi_twcentroid(TemporalI *ti)
{
	int srid = tpointi_srid(ti);
	TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsz = NULL ; /* keep compiler quiet */
	bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
	if (hasz)
		instantsz = palloc(sizeof(TemporalInst *) * ti->count);
		
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		POINT2D point2d;
		POINT3DZ point;
		if (hasz)
			point = datum_get_point3dz(temporalinst_value(inst));
		else
		{
			point2d = datum_get_point2d(temporalinst_value(inst));
			point.x = point2d.x;
			point.y = point2d.y;
		}
		
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x),
			inst->t, FLOAT8OID);		
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y),
			inst->t, FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z),
				inst->t, FLOAT8OID);

	}
	TemporalI *tix = temporali_make(instantsx, ti->count);
	TemporalI *tiy = temporali_make(instantsy, ti->count);
	TemporalI *tiz = NULL; /* keep compiler quiet */
	if (hasz)
		tiz = temporali_make(instantsz, ti->count);
	double avgx = tnumberi_twavg(tix);
	double avgy = tnumberi_twavg(tiy);
	double avgz;
	if (hasz)
		avgz = tnumberi_twavg(tiz);
	LWPOINT *lwpoint;
	if (hasz)
		lwpoint = lwpoint_make3dz(srid, avgx, avgy, avgz);
	else
		lwpoint = lwpoint_make2d(srid, avgx, avgy);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *)lwpoint));

	pfree(lwpoint);
	for (int i = 0; i < ti->count; i++)
	{
		pfree(instantsx[i]);
		pfree(instantsy[i]);
		if (hasz)
			pfree(instantsz[i]);
	}
	pfree(instantsx); pfree(instantsy);
	pfree(tix); pfree(tiy);
	if (hasz)
	{
		pfree(instantsz); pfree(tiz);
	}

	return result;
}

Datum
tgeompointseq_twcentroid(TemporalSeq *seq)
{
	int srid = tpointseq_srid(seq);
	TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst **instantsz;
	bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
	if (hasz)
		instantsz = palloc(sizeof(TemporalInst *) * seq->count);
		
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		POINT2D point2d;
		POINT3DZ point;
		if (hasz)
			point = datum_get_point3dz(temporalinst_value(inst));
		else
		{
			point2d = datum_get_point2d(temporalinst_value(inst));
			point.x = point2d.x;
			point.y = point2d.y;
		}
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x),
			inst->t, FLOAT8OID);		
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y),
			inst->t, FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z),
				inst->t, FLOAT8OID);
	}
	TemporalSeq *seqx = temporalseq_make(instantsx,
		seq->count, seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	TemporalSeq *seqy = temporalseq_make(instantsy,
		seq->count, seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	TemporalSeq *seqz;
	if (hasz)
		seqz = temporalseq_make(instantsz,
			seq->count, seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	double twavgx = tnumberseq_twavg(seqx);
	double twavgy = tnumberseq_twavg(seqy);
	double twavgz;
	LWPOINT *lwpoint;
	if (hasz)
	{
		twavgz = tnumberseq_twavg(seqz);
		lwpoint = lwpoint_make3dz(srid, twavgx, twavgy, twavgz);
	}
	else
		lwpoint = lwpoint_make2d(srid, twavgx, twavgy);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *)lwpoint));

	pfree(lwpoint);
	for (int i = 0; i < seq->count; i++)
	{
		pfree(instantsx[i]);
		pfree(instantsy[i]);
		if (hasz)
			pfree(instantsz[i]);
	}
	pfree(instantsx); pfree(instantsy);
	pfree(seqx); pfree(seqy);
	if (hasz)
	{
		pfree(seqz); pfree(instantsz);
	}

	return result;
}

Datum
tgeompoints_twcentroid(TemporalS *ts)
{
	int srid = tpoints_srid(ts);
	TemporalSeq **sequencesx = palloc(sizeof(TemporalSeq *) * ts->count);
	TemporalSeq **sequencesy = palloc(sizeof(TemporalSeq *) * ts->count);
	TemporalSeq **sequencesz = NULL; /* keep compiler quiet */
	bool hasz = MOBDB_FLAGS_GET_Z(ts->flags);
	if (hasz)
		sequencesz = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * seq->count);
		TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * seq->count);
		TemporalInst **instantsz;
		if (hasz)
			instantsz = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			POINT2D point2d;
			POINT3DZ point;
			if (hasz)
				point = datum_get_point3dz(temporalinst_value(inst));
			else
			{
				point2d = datum_get_point2d(temporalinst_value(inst));
				point.x = point2d.x;
				point.y = point2d.y;
			}
			instantsx[j] = temporalinst_make(Float8GetDatum(point.x),
				inst->t, FLOAT8OID);		
			instantsy[j] = temporalinst_make(Float8GetDatum(point.y),
				inst->t, FLOAT8OID);
			if (hasz)
				instantsz[j] = temporalinst_make(Float8GetDatum(point.z),
					inst->t, FLOAT8OID);
		}
		sequencesx[i] = temporalseq_make(instantsx,
			seq->count, seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		sequencesy[i] = temporalseq_make(instantsy,
			seq->count, seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		if (hasz)
			sequencesz[i] = temporalseq_make(instantsz,
				seq->count, seq->period.lower_inc, seq->period.upper_inc,
				MOBDB_FLAGS_GET_LINEAR(seq->flags), true);

		for (int j = 0; j < seq->count; j++)
		{
			pfree(instantsx[j]); pfree(instantsy[j]);
			if (hasz)
				pfree(instantsz[j]);
		}
		pfree(instantsx); pfree(instantsy);
		if (hasz)
			pfree(instantsz);
	}
	TemporalS *tsx = temporals_make(sequencesx, ts->count, true);
	TemporalS *tsy = temporals_make(sequencesy, ts->count, true);
	TemporalS *tsz = NULL; /* keep compiler quiet */
	if (hasz)
		tsz = temporals_make(sequencesz, ts->count, true);

	double twavgx = tnumbers_twavg(tsx);
	double twavgy = tnumbers_twavg(tsy);
	double twavgz;
	LWPOINT *lwpoint;
	if (hasz)
	{
		twavgz = tnumbers_twavg(tsz);
		lwpoint = lwpoint_make3dz(srid, twavgx, twavgy, twavgz);
	}
	else
		lwpoint = lwpoint_make2d(srid, twavgx, twavgy);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *)lwpoint));

	pfree(lwpoint);
	for (int i = 0; i < ts->count; i++)
	{
		pfree(sequencesx[i]); pfree(sequencesy[i]);
		if (hasz)
			pfree(sequencesz[i]);
	}
	pfree(sequencesx); pfree(sequencesy);
	pfree(tsx); pfree(tsy);
	if (hasz)
	{
		pfree(tsz); pfree(sequencesz);
	}
	
	return result;
}


Datum
tgeompoint_twcentroid_internal(Temporal *temp)
{
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tgeompointi_twcentroid((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tgeompointseq_twcentroid((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = tgeompoints_twcentroid((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_twcentroid);

PGDLLEXPORT Datum
tgeompoint_twcentroid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tgeompoint_twcentroid_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}
	
/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
	return call_function2(LWGEOM_azimuth, geom1, geom2);
}

static Datum
geog_azimuth(Datum geom1, Datum geom2)
{
	return call_function2(geography_azimuth, geom1, geom2);
}

static int
tpointseq_azimuth1(TemporalSeq **result, TemporalSeq *seq)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;

	/* Determine the PostGIS function to call */
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(seq->valuetypid);
	if (seq->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_azimuth;
	else
		func = &geog_azimuth;

	/* We are sure that there are at least 2 instants */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	int k = 0, l = 0;
	Datum azimuth = 0; /* Make the compiler quiet */
	bool lower_inc = seq->period.lower_inc, upper_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		if (datum_ne(value1, value2, seq->valuetypid))
		{
			azimuth = func(value1, value2);
			instants[k++] = temporalinst_make(azimuth, inst1->t, FLOAT8OID);
		}
		else
		{
			if (k != 0)
			{
				instants[k++] = temporalinst_make(azimuth, inst1->t, FLOAT8OID);
				upper_inc = true;
				/* Resulting sequence has stepwise interpolation */
				result[l++] = temporalseq_make(instants, k,
					lower_inc, upper_inc, false, true);
				for (int j = 0; j < k; j++)
					pfree(instants[j]);
				k = 0;
			}
			lower_inc = true;
		}
		inst1 = inst2;
		value1 = value2;
	}
	if (k != 0)
	{
		instants[k++] = temporalinst_make(azimuth, inst1->t, FLOAT8OID);
		/* Resulting sequence has stepwise interpolation */
		result[l++] = temporalseq_make(instants, k,
			lower_inc, upper_inc, false, true);
	}

	pfree(instants);

	return l;
}

TemporalS *
tpointseq_azimuth(TemporalSeq *seq)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tpointseq_azimuth1(sequences, seq);
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	/* Resulting sequence set has stepwise interpolation */
	TemporalS *result = temporals_make(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
tpoints_azimuth(TemporalS *ts)
{
	if (ts->count == 1)
		return tpointseq_azimuth(temporals_seq_n(ts, 0));

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tpointseq_azimuth1(&sequences[k], seq);
	}
	if (k == 0)
		return NULL;

	/* Resulting sequence set has stepwise interpolation */
	TemporalS *result = temporals_make(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_azimuth);

PGDLLEXPORT Datum
tpoint_azimuth(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI ||
		(temp->duration == TEMPORALSEQ && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
		(temp->duration == TEMPORALS && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
		;
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_azimuth((TemporalSeq *)temp);	
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_azimuth((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the current version of PostGIS (2.5) there is no true ST_Intersection
 * function for geography
 *****************************************************************************/

/* Restrict a temporal point to a geometry */

static TemporalInst *
tpointinst_at_geometry(TemporalInst *inst, Datum geom)
{
	if (!DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

static TemporalI *
tpointi_at_geometry(TemporalI *ti, Datum geom)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		if (DatumGetBool(call_function2(intersects, value, geom)))
			instants[k++] = inst;
	}
	TemporalI *result = NULL;
	if (k != 0)
		result = temporali_make(instants, k);
	/* We do not need to pfree the instants */
	pfree(instants);
	return result;
}

/*
 * This function assumes that inst1 and inst2 have equal SRID and that the
 * points and the geometry are in 2D
 */
static TemporalSeq **
tpointseq_at_geometry1(TemporalInst *inst1, TemporalInst *inst2, bool linear,
	bool lower_inc, bool upper_inc, Datum geom, int *count)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);

	/* Constant segment or stepwise interpolation */
	bool equal = datum_point_eq(value1, value2);
	if (equal || ! linear)
	{
		if (!DatumGetBool(call_function2(intersects, value1, geom)))
		{
			*count = 0;
			return NULL;
		}

		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = equal ? inst2 :
			temporalinst_make(value1, inst2->t, inst1->valuetypid);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_make(instants, 2,
			lower_inc, upper_inc, linear, false);
		*count = 1;
		if (! equal)
			pfree(instants[1]);
		return result;
	}

	/* Look for intersections */
	Datum line = geompoint_trajectory(value1, value2);
	Datum intersections = call_function2(intersection, line, geom);
	if (DatumGetBool(call_function1(LWGEOM_isempty, intersections)))
	{
		pfree(DatumGetPointer(line));
		pfree(DatumGetPointer(intersections));
		*count = 0;
		return NULL;
	}

	int countinter = DatumGetInt32(call_function1(
		LWGEOM_numgeometries_collection, intersections));
	TemporalInst *instants[2];
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countinter);
	double duration = (double)(inst2->t - inst1->t);
	int k = 0;
	for (int i = 1; i <= countinter; i++)
	{
		/* Find the i-th intersection */
		Datum inter = call_function2(LWGEOM_geometryn_collection,
			intersections, Int32GetDatum(i));
		GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);

		/* Each intersection is either a point or a linestring with two points */
		if (gserialized_get_type(gsinter) == POINTTYPE)
		{
			double fraction = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter));
			TimestampTz t = inst1->t + (long) (duration * fraction);
			/* If the intersection is not at an exclusive bound */
			if ((lower_inc || t > inst1->t) && (upper_inc || t < inst2->t))
			{
				/* Restriction at timestamp done to avoid floating point imprecision */
				instants[0] = temporalseq_at_timestamp1(inst1, inst2, t, linear);
				result[k++] = temporalseq_make(instants, 1,
					true, true, linear, false);
				pfree(instants[0]);
			}
		}
		else
		{
			Datum point1 = call_function2(LWGEOM_pointn_linestring,
				inter, Int32GetDatum(1));
			Datum point2 = call_function2(LWGEOM_pointn_linestring,
				inter, Int32GetDatum(2));
			double fraction1 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point1));
			double fraction2 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point2));
			TimestampTz t1 = inst1->t + (long) (duration * fraction1);
			TimestampTz t2 = inst1->t + (long) (duration * fraction2);
			TimestampTz lower1 = Min(t1, t2);
			TimestampTz upper1 = Max(t1, t2);
			/* Restriction at timestamp done to avoid floating point imprecision */
			instants[0] = temporalseq_at_timestamp1(inst1, inst2, lower1, linear);
			instants[1] = temporalseq_at_timestamp1(inst1, inst2, upper1, linear);
			bool lower_inc1 = (lower1 == inst1->t) ? lower_inc : true;
			bool upper_inc1 = (upper1 == inst2->t) ? upper_inc : true;
			result[k++] = temporalseq_make(instants, 2,
					lower_inc1, upper_inc1, linear, false);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
		}
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
	}

	pfree(DatumGetPointer(line));
	pfree(DatumGetPointer(intersections));

	if (k == 0)
	{
		pfree(result);
		*count = 0;
		return NULL;
	}

	temporalseqarr_sort(result, k);
	*count = k;
	return result;
}

TemporalSeq **
tpointseq_at_geometry2(TemporalSeq *seq, Datum geom, int *count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		/* Due to the bounding box test in the calling function we are sure
		 * that the point intersects the geometry */
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_copy(seq);
		*count = 1;
		return result;
	}

	/* Temporal sequence has at least 2 instants */
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * (seq->count - 1));
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tpointseq_at_geometry1(inst1, inst2, linear,
			lower_inc, upper_inc, geom, &countseqs[i]);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}
	if (totalseqs == 0)
	{
		pfree(countseqs);
		pfree(sequences);
		*count = 0;
		return NULL;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count - 1; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}

	pfree(countseqs);
	pfree(sequences);
	*count = totalseqs;
	return result;
}

static TemporalS *
tpointseq_at_geometry(TemporalSeq *seq, Datum geom)
{
	int count;
	TemporalSeq **sequences = tpointseq_at_geometry2(seq, geom, &count);
	if (sequences == NULL)
		return NULL;

	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tpoints_at_geometry(TemporalS *ts, GSERIALIZED *gs, STBOX *box2)
{
	/* palloc0 used due to the bounding box test in the for loop below */
	TemporalSeq ***sequences = palloc0(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		/* Bounding box test */
		STBOX *box1 = temporalseq_bbox_ptr(seq);
		if (overlaps_stbox_stbox_internal(box1, box2))
		{
			sequences[i] = tpointseq_at_geometry2(seq, PointerGetDatum(gs),
				&countseqs[i]);
			totalseqs += countseqs[i];
		}
	}
	if (totalseqs == 0)
	{
		pfree(sequences);
		pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_make(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);
	pfree(sequences);
	pfree(countseqs);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_at_geometry);

PGDLLEXPORT Datum
tpoint_at_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	/* Non-empty geometries have a bounding box */
	assert(geo_to_stbox_internal(&box2, gs));
	if (!overlaps_stbox_stbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_at_geometry((TemporalInst *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_at_geometry((TemporalI *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_at_geometry((TemporalSeq *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_at_geometry((TemporalS *)temp, gs, &box2);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Restrict a temporal point to the complement of a geometry */

static TemporalInst *
tpointinst_minus_geometry(TemporalInst *inst, Datum geom)
{
	if (DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

static TemporalI *
tpointi_minus_geometry(TemporalI *ti, Datum geom)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		if (!DatumGetBool(call_function2(intersects, value, geom)))
			instants[k++] = inst;
	}
	TemporalI *result = NULL;
	if (k != 0)
		result = temporali_make(instants, k);
	/* We do not need to pfree the instants */
	pfree(instants);
	return result;
}

/*
 * It is not possible to use a similar approach as for tpointseq_at_geometry1
 * where instead of computing the intersections we compute the difference since
 * in PostGIS the following query
 *  	select st_astext(st_difference(geometry 'Linestring(0 0,3 3)',
 *  		geometry 'MultiPoint((1 1),(2 2),(3 3))'))
 * returns "LINESTRING(0 0,3 3)". Therefore we compute tpointseq_at_geometry1
 * and then compute the complement of the value obtained.
 */
static TemporalSeq **
tpointseq_minus_geometry1(TemporalSeq *seq, Datum geom, int *count)
{
	int countinter;
	TemporalSeq **sequences = tpointseq_at_geometry2(seq, geom, &countinter);
	if (countinter == 0)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_copy(seq);
		*count = 1;
		return result;
	}
		
	Period **periods = palloc(sizeof(Period) * countinter);
	for (int i = 0; i < countinter; i++)
		periods[i] = &sequences[i]->period;
	PeriodSet *ps1 = periodset_make_internal(periods, countinter, false);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	pfree(ps1); pfree(periods);
	if (ps2 == NULL)
	{
		*count = 0;
		return NULL;
	}
	TemporalSeq **result = temporalseq_at_periodset2(seq, ps2, count);
	pfree(ps2);
	return result;
}

static TemporalS *
tpointseq_minus_geometry(TemporalSeq *seq, Datum geom)
{
	int count;
	TemporalSeq **sequences = tpointseq_minus_geometry1(seq, geom, &count);
	if (sequences == NULL)
		return NULL;

	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tpoints_minus_geometry(TemporalS *ts, GSERIALIZED *gs, STBOX *box2)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_minus_geometry(temporals_seq_n(ts, 0),
			PointerGetDatum(gs));

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		/* Bounding box test */
		STBOX *box1 = temporalseq_bbox_ptr(seq);
		if (!overlaps_stbox_stbox_internal(box1, box2))
		{
			sequences[i] = palloc(sizeof(TemporalSeq *));
			sequences[i][0] = temporalseq_copy(seq);
			countseqs[i] = 1;
			totalseqs ++;
		}
		else
		{
			sequences[i] = tpointseq_minus_geometry1(seq, PointerGetDatum(gs),
				&countseqs[i]);
			totalseqs += countseqs[i];
		}
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_make(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_minus_geometry);

PGDLLEXPORT Datum
tpoint_minus_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		Temporal *copy = temporal_copy(temp) ;
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(copy);
	}
	temporal_bbox(&box1, temp);
	if (!overlaps_stbox_stbox_internal(&box1, &box2))
	{
		Temporal *copy = temporal_copy(temp) ;
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(copy);
	}

	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_minus_geometry((TemporalInst *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_minus_geometry((TemporalI *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_minus_geometry((TemporalSeq *)temp,
			PointerGetDatum(gs));
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tpoints_minus_geometry((TemporalS *)temp, gs, &box2);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

static TemporalInst *
NAI_tpointi_geo(TemporalI *ti, Datum geo, Datum (*func)(Datum, Datum))
{
	double mindist = DBL_MAX;
	int number = 0; /* keep compiler quiet */
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		double dist = DatumGetFloat8(func(value, geo));	
		if (dist < mindist)
		{
			mindist = dist;
			number = i;
		}
	}
	return temporalinst_copy(temporali_inst_n(ti, number));
}

/*****************************************************************************/

/* NAI between temporal sequence point with stepwise interpolation and a
 * geometry/geography */

static TemporalInst *
NAI_tpointseq_stw_geo(TemporalSeq *seq, Datum geo, Datum (*func)(Datum, Datum))
{
	double mindist = DBL_MAX;
	int number = 0; /* keep compiler quiet */
	for (int i = 0; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		double dist = DatumGetFloat8(func(value, geo));	
		if (dist < mindist)
		{
			mindist = dist;
			number = i;
		}
	}
	return temporalinst_copy(temporalseq_inst_n(seq, number));
}

/* NAI between temporal sequence point with linear interpolation and a
 * geometry/geography */

static Datum
NAI_tpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2,
	Datum geo, TimestampTz *t, bool *tofree)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	/* Constant segment */
	if (datum_point_eq(value1, value2))
	{
		*t = inst1->t;
		*tofree = false;
		return value1;
	}

	double fraction;
	ensure_point_base_type(inst1->valuetypid);
	if (inst1->valuetypid == type_oid(T_GEOMETRY))
	{
		/* The trajectory is a line */
		Datum traj = geompoint_trajectory(value1, value2);
		Datum point = call_function2(LWGEOM_closestpoint, traj, geo);
		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			traj, point));
		pfree(DatumGetPointer(traj)); pfree(DatumGetPointer(point));
	}
	else
	{
		/* The trajectory is a line */
		Datum traj = geogpoint_trajectory(value1, value2);
		/* There is no function equivalent to LWGEOM_line_locate_point
		 * for geographies. We do as the ST_Intersection function, e.g.
		 * 'SELECT geography(ST_Transform(ST_Intersection(ST_Transform(geometry($1),
		 * @extschema@._ST_BestSRID($1, $2)),
		 * ST_Transform(geometry($2), @extschema@._ST_BestSRID($1, $2))), 4326))' */
		Datum bestsrid = call_function2(geography_bestsrid, traj, geo);
		Datum traj1 = call_function1(geometry_from_geography, traj);
		Datum traj2 = call_function2(transform, traj1, bestsrid);
		Datum geo1 = call_function1(geometry_from_geography, geo);
		Datum geo2 = call_function2(transform, geo1, bestsrid);
		Datum point = call_function2(LWGEOM_closestpoint, traj2, geo2);
		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			traj2, point));
		pfree(DatumGetPointer(traj)); pfree(DatumGetPointer(traj1));
		pfree(DatumGetPointer(traj2)); pfree(DatumGetPointer(geo1));
		pfree(DatumGetPointer(geo2)); pfree(DatumGetPointer(point));
	}

	if (fraction == 0)
	{
		*t = inst1->t;
		*tofree = false;
		return value1;
	}
	if (fraction == 1)
	{
		*t = inst2->t;
		*tofree = false;
		return value2;
	}

	*t = inst1->t + (long)((double) (inst2->t - inst1->t) * fraction);
	*tofree = true;
	/* Linear interpolation */
	return temporalseq_value_at_timestamp1(inst1, inst2, true, *t);
}

static TemporalInst *
NAI_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum (*func)(Datum, Datum))
{
	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalinst_copy(temporalseq_inst_n(seq, 0));

	/* Stepwise interpolation */
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
		return NAI_tpointseq_stw_geo(seq, geo, func);

	/* Linear interpolation */
	double mindist = DBL_MAX;
	Datum minpoint = 0; /* keep compiler quiet */
	TimestampTz tmin = 0; /* keep compiler quiet */
	bool mintofree =  false; /* keep compiler quiet */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		TimestampTz t;
		bool tofree;
		Datum point = NAI_tpointseq_geo1(inst1, inst2, geo, &t, &tofree);
		double dist = DatumGetFloat8(func(point, geo));
		if (dist < mindist)
		{
			mindist = dist;
			minpoint = point;
			tmin = t;
			mintofree = tofree;
		}
		else if (tofree)
			pfree(DatumGetPointer(point)); 			
		inst1 = inst2;
	}
	TemporalInst *result = temporalinst_make(minpoint, tmin, seq->valuetypid);
	if (mintofree)
		pfree(DatumGetPointer(minpoint));
	return result;
}

/*****************************************************************************/

static TemporalInst *
NAI_tpoints_geo(TemporalS *ts, Datum geo, Datum (*func)(Datum, Datum))
{
	TemporalInst *result = NULL;
	double mindist = DBL_MAX;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = NAI_tpointseq_geo(seq, geo, func);
		Datum value = temporalinst_value(inst);
		double dist = DatumGetFloat8(func(value, geo));
		if (dist < mindist)
		{
			if (result != NULL)
				pfree(result);
			result = inst;
			mindist = dist;
		}
		else
			pfree(inst);
	}
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_geo_tpoint);

PGDLLEXPORT Datum
NAI_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	if (MOBDB_FLAGS_GET_Z(temp->flags) || FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("3D geometries are not allowed")));
	}
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_distance2d;
	else
		func = &geog_distance;
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)NAI_tpointi_geo((TemporalI *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)NAI_tpointseq_geo((TemporalSeq *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)NAI_tpoints_geo((TemporalS *)temp,
			PointerGetDatum(gs), func);
	
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_geo);

PGDLLEXPORT Datum
NAI_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	if (MOBDB_FLAGS_GET_Z(temp->flags) || FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("3D geometries are not allowed")));
	}
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_distance2d;
	else
		func = &geog_distance;
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)NAI_tpointi_geo((TemporalI *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)NAI_tpointseq_geo((TemporalSeq *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)NAI_tpoints_geo((TemporalS *)temp,
			PointerGetDatum(gs), func);
	
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);

PGDLLEXPORT Datum
NAI_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	TemporalInst *result = NULL;
	Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
	if (dist != NULL)
	{
		Temporal *mindist = temporal_at_min_internal(dist);
		TimestampTz t = temporal_start_timestamp_internal(mindist);
		result = temporal_at_timestamp_internal(temp1, t);
		pfree(dist); pfree(mindist);		
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_geo_tpoint);

PGDLLEXPORT Datum
NAD_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;

	Datum traj = tpoint_trajectory_internal(temp);
	Datum result = func(traj, PointerGetDatum(gs));

	pfree(DatumGetPointer(traj));
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_geo);

PGDLLEXPORT Datum
NAD_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;

	Datum traj = tpoint_trajectory_internal(temp);
	Datum result = func(traj, PointerGetDatum(gs));

	pfree(DatumGetPointer(traj));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_tpoint_tpoint);

PGDLLEXPORT Datum
NAD_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
	if (dist == NULL)
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum result = temporal_min_value_internal(dist);
	pfree(dist);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PG_FUNCTION_INFO_V1(shortestline_geo_tpoint);

PGDLLEXPORT Datum
shortestline_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum traj = tpoint_trajectory_internal(temp);
	Datum result =  MOBDB_FLAGS_GET_Z(temp->flags) ?
		call_function2(LWGEOM_shortestline3d, traj, PointerGetDatum(gs)) :
		call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));

	pfree(DatumGetPointer(traj));
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_geo);

PGDLLEXPORT Datum
shortestline_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum traj = tpoint_trajectory_internal(temp);
	Datum result =  MOBDB_FLAGS_GET_Z(temp->flags) ?
		call_function2(LWGEOM_shortestline3d, traj, PointerGetDatum(gs)) :
		call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));

	pfree(DatumGetPointer(traj));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************/
/* These functions suppose that the temporal values overlap in time */

static Datum
shortestline_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2)
{
	LWGEOM *lwgeoms[2];
	GSERIALIZED *gs1 = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst1));
	GSERIALIZED *gs2 = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst2));
	lwgeoms[0] = lwgeom_from_gserialized(gs1);
	lwgeoms[1] = lwgeom_from_gserialized(gs2);
	LWLINE *line = lwline_from_lwgeom_array(lwgeoms[0]->srid, 2, lwgeoms);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *)line));
	lwgeom_free(lwgeoms[0]);
	lwgeom_free(lwgeoms[1]);
	return result;
}

static Datum
shortestline_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*func)(Datum, Datum))
{
	/* Compute the distance */
	TemporalI *dist = sync_tfunc2_temporali_temporali(ti1, ti2, func,
		FLOAT8OID);
	Datum mind = temporali_min_value(dist);
	TemporalI *mindist = temporali_at_value(dist, mind);
	TimestampTz t = temporali_start_timestamp(mindist);
	TemporalInst *inst1 = temporali_at_timestamp(ti1, t);
	TemporalInst *inst2 = temporali_at_timestamp(ti2, t);
	Datum result = shortestline_tpointinst_tpointinst(inst1, inst2);
	pfree(dist); pfree(mindist); pfree(inst1); pfree(inst2);
	return result;
}

static Datum
shortestline_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum))
{
	/* Compute the distance */
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	TemporalSeq *dist = sync_tfunc2_temporalseq_temporalseq(seq1, seq2,
		func, FLOAT8OID, linear, NULL);
	TemporalS *mindist = temporalseq_at_min(dist);
	TimestampTz t = temporals_start_timestamp(mindist);
	/* Make a copy of the sequences with inclusive bounds */
	TemporalSeq *newseq1 = temporalseq_copy(seq1);
	newseq1->period.lower_inc = true;
	newseq1->period.upper_inc = true;
	TemporalSeq *newseq2 = temporalseq_copy(seq2);
	newseq2->period.lower_inc = true;
	newseq2->period.upper_inc = true;
	TemporalInst *inst1 = temporalseq_at_timestamp(newseq1, t);
	TemporalInst *inst2 = temporalseq_at_timestamp(newseq2, t);
	Datum result = shortestline_tpointinst_tpointinst(inst1, inst2);
	pfree(dist); pfree(mindist); pfree(inst1); pfree(inst2);
	pfree(newseq1); pfree(newseq2);
	return result;
}

static Datum
shortestline_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2,
	Datum (*func)(Datum, Datum))
{
	/* Compute the distance */
	bool linear = MOBDB_FLAGS_GET_LINEAR(ts1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(ts2->flags);
	TemporalS *dist = sync_tfunc2_temporals_temporals(ts1, ts2, func,
		FLOAT8OID, linear, NULL);
	TemporalS *mindist = temporals_at_min(dist);
	TimestampTz t = temporals_start_timestamp(mindist);
	TemporalInst *inst1 = temporals_at_timestamp(ts1, t);
	TemporalInst *inst2 = temporals_at_timestamp(ts2, t);
	
	/* If t is at an exclusive bound */
	bool freeinst1 = (inst1 != NULL);
	if (inst1 == NULL)
	{
		int pos;
		temporals_find_timestamp(ts1, t, &pos);
		if (pos == 0)
		{
			TemporalSeq *seq = temporals_seq_n(ts1, 0);
			inst1 = temporalseq_inst_n(seq, 0);
		}
		else if (pos == ts1->count)
		{
			TemporalSeq *seq = temporals_seq_n(ts1, ts1->count - 1);
			inst1 = temporalseq_inst_n(seq, seq->count - 1);
		}
		else
		{
			TemporalSeq *seq1 = temporals_seq_n(ts1, pos - 1);
			TemporalSeq *seq2 = temporals_seq_n(ts1, pos);
			if (temporalseq_end_timestamp(seq1) == t)
				inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
			else
				inst1 = temporalseq_inst_n(seq2, 0);
			}		
	}
	
	/* If t is at an exclusive bound */
	bool freeinst2 = (inst2 != NULL);
	if (inst2 == NULL)
	{
		int pos;
		temporals_find_timestamp(ts2, t, &pos);
		if (pos == 0)
		{
			TemporalSeq *seq = temporals_seq_n(ts2, 0);
			inst2 = temporalseq_inst_n(seq, 0);
		}
		else if (pos == ts2->count)
		{
			TemporalSeq *seq = temporals_seq_n(ts2, ts2->count - 1);
			inst2 = temporalseq_inst_n(seq, seq->count - 1);
		}
		else
		{
			TemporalSeq *seq1 = temporals_seq_n(ts2, pos - 1);
			TemporalSeq *seq2 = temporals_seq_n(ts2, pos);
			if (temporalseq_end_timestamp(seq1) == t)
				inst2 = temporalseq_inst_n(seq1, seq1->count - 1);
			else
				inst2 = temporalseq_inst_n(seq2, 0);
			}		
	}
	
	Datum result = shortestline_tpointinst_tpointinst(inst1, inst2);
	pfree(dist); pfree(mindist);
	if (freeinst1)
		pfree(inst1);
	if (freeinst2)
		pfree(inst2);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(shortestline_tpoint_tpoint);

PGDLLEXPORT Datum
shortestline_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags) && MOBDB_FLAGS_GET_Z(temp2->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;
	Datum result = 0;
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == TEMPORALINST)
		result = shortestline_tpointinst_tpointinst((TemporalInst *)sync1,
			(TemporalInst *)sync2);
	else if (sync1->duration == TEMPORALI)
		result = shortestline_tpointi_tpointi((TemporalI *)sync1,
			(TemporalI *)sync2, func);
	else if (sync1->duration == TEMPORALSEQ)
		result = shortestline_tpointseq_tpointseq((TemporalSeq *)sync1,
			(TemporalSeq *)sync2, func);
	else if (sync1->duration == TEMPORALS)
		result = shortestline_tpoints_tpoints((TemporalS *)sync1,
			(TemporalS *)sync2, func);
	
	pfree(sync1); pfree(sync2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert a temporal point into a trajectory geometry/geography where the M 
 * coordinates encode the timestamps in number of seconds since '1970-01-01'
 * The internal representation of timestamps in PostgreSQL is in microseconds
 * since '2000-01-01'. Therefore we need to compute
 * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
 * which results in 946684800
 *****************************************************************************/

static LWPOINT *
point_to_trajpoint(GSERIALIZED *gs, TimestampTz t)
{
	int32 srid = gserialized_get_srid(gs);
	double epoch = ((double)t / 1e6) + 946684800 ;
	LWPOINT *result;
	if (FLAGS_GET_Z(gs->flags))
	{
		POINT3DZ point = gs_get_point3dz(gs);
		result = lwpoint_make4d(srid, point.x, point.y, point.z, epoch);
	}
	else
	{
		POINT2D point = gs_get_point2d(gs);
		result = lwpoint_make3dm(srid, point.x, point.y, epoch);
	}
	FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
	return result;
}

static Datum
tpointinst_to_geo(TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	LWPOINT *point = point_to_trajpoint(gs, inst->t);
	GSERIALIZED *result = geometry_serialize((LWGEOM *)point);
	pfree(point);
	return PointerGetDatum(result);
}

static Datum
tpointi_to_geo(TemporalI *ti)
{
	TemporalInst *inst = temporali_inst_n(ti, 0);
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);
	LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		inst = temporali_inst_n(ti, i);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		points[i] = (LWGEOM *)point_to_trajpoint(gs, inst->t);
	}
	GSERIALIZED *result;
	if (ti->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *mpoint = (LWGEOM *)lwcollection_construct(MULTIPOINTTYPE, srid,
			NULL, (uint32_t) ti->count, points);
		result = geometry_serialize(mpoint);
		pfree(mpoint);
	}

	for (int i = 0; i < ti->count; i++)
		pfree(points[i]);
	pfree(points);
	return PointerGetDatum(result);
}

static Datum
tpointseq_to_geo(TemporalSeq *seq)
{
	LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		GSERIALIZED *gs = (GSERIALIZED *) PointerGetDatum(temporalinst_value(inst));
		points[i] = (LWGEOM *) point_to_trajpoint(gs, inst->t);
	}
	GSERIALIZED *result;
	/* Instantaneous sequence */
	if (seq->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *geom;
		if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
			geom = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
				(uint32_t) seq->count, points);
		else
			geom = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, points[0]->srid,
				NULL, (uint32_t) seq->count, points);
		result = geometry_serialize(geom);
		pfree(geom);
	}

	for (int i = 0; i < seq->count; i++)
		pfree(points[i]);
	pfree(points);

	return PointerGetDatum(result);
}

static Datum
tpoints_to_geo(TemporalS *ts)
{
	Datum *geoms = palloc(sizeof(Datum) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		geoms[i] = tpointseq_to_geo(seq);
	}
	Datum result;
	if (ts->count == 1)
		result = geoms[0];
	else
	{
		ArrayType *array = datumarr_to_array(geoms, ts->count, ts->valuetypid);
		result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));		
		for (int i = 0; i < ts->count; i++)
			pfree(DatumGetPointer(geoms[i]));
		pfree(array);
	}
	pfree(geoms);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_to_geo);

PGDLLEXPORT Datum
tpoint_to_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = tpointinst_to_geo((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_to_geo((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_to_geo((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = tpoints_to_geo((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the
 * timestamps in number of seconds since '1970-01-01' into a temporal point.
 *****************************************************************************/

static TemporalInst *
trajpoint_to_tpointinst(LWPOINT *lwpoint)
{
	bool hasz = (bool) FLAGS_GET_Z(lwpoint->flags);
	bool geodetic = (bool) FLAGS_GET_GEODETIC(lwpoint->flags);
	LWPOINT *lwpoint1;
	TimestampTz t;
	if (hasz)
	{
		POINT4D point = getPoint4d(lwpoint->point, 0);
		t = (long) ((point.m - 946684800) * 1e6);
		lwpoint1 = lwpoint_make3dz(lwpoint->srid, point.x, point.y, point.z);
	}
	else
	{
		POINT3DM point = getPoint3dm(lwpoint->point, 0);
		t = (long) ((point.m - 946684800) * 1e6);
		lwpoint1 = lwpoint_make2d(lwpoint->srid, point.x, point.y);
	}
	FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
	GSERIALIZED *gs = geometry_serialize((LWGEOM *)lwpoint1);
	Oid valuetypid = geodetic ? type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY);
	TemporalInst *result = temporalinst_make(PointerGetDatum(gs), t,
		valuetypid);
	pfree(gs);
	return result;	
}

static TemporalInst *
geo_to_tpointinst(GSERIALIZED *gs)
{
	/* Geometry is a POINT */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	TemporalInst *result = trajpoint_to_tpointinst((LWPOINT *)lwgeom);
	lwgeom_free(lwgeom);
	return result;
}

static TemporalI *
geo_to_tpointi(GSERIALIZED *gs)
{
	TemporalI *result;
	/* Geometry is a MULTIPOINT */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	bool hasz = (bool) FLAGS_GET_Z(gs->flags);
	/* Verify that is a valid set of trajectory points */
	LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
	double m1 = -1 * DBL_MAX, m2;
	int npoints = lwcoll->ngeoms;
	for (int i = 0; i < npoints; i++)
	{
		LWPOINT *lwpoint = (LWPOINT *)lwcoll->geoms[i];
		if (hasz)
		{
			POINT4D point = getPoint4d(lwpoint->point, 0);
			m2 = point.m;
		}
		else
		{
			POINT3DM point = getPoint3dm(lwpoint->point, 0);
			m2 = point.m;
		}
		if (m1 >= m2)
		{
			lwgeom_free(lwgeom);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Trajectory must be valid")));
		}
		m1 = m2;
	}
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * npoints);
	for (int i = 0; i < npoints; i++)
		instants[i] = trajpoint_to_tpointinst((LWPOINT *)lwcoll->geoms[i]);
	result = temporali_make(instants, npoints);
	
	lwgeom_free(lwgeom);
	for (int i = 0; i < npoints; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalSeq *
geo_to_tpointseq(GSERIALIZED *gs)
{
	/* Geometry is a LINESTRING */
	bool hasz =(bool)  FLAGS_GET_Z(gs->flags);
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	LWLINE *lwline = lwgeom_as_lwline(lwgeom);
	int npoints = lwline->points->npoints;
	/*
	 * Verify that the trajectory is valid.
	 * Since calling lwgeom_is_trajectory causes discrepancies with regression
	 * tests because of the error message depends on PostGIS version,
	 * the verification is made here.
	 */
	double m1 = -1 * DBL_MAX, m2;
	for (int i = 0; i < npoints; i++)
	{
		if (hasz)
		{
			POINT4D point = getPoint4d(lwline->points, (uint32_t) i);
			m2 = point.m;
		}
		else
		{
			POINT3DM point = getPoint3dm(lwline->points, (uint32_t) i);
			m2 = point.m;
		}
		if (m1 >= m2)
		{
			lwgeom_free(lwgeom);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Trajectory must be valid")));
		}
		m1 = m2;
	}
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * npoints);
	for (int i = 0; i < npoints; i++)
	{
		/* Returns freshly allocated LWPOINT */
		LWPOINT *lwpoint = lwline_get_lwpoint(lwline, (uint32_t) i);
		instants[i] = trajpoint_to_tpointinst(lwpoint);
		lwpoint_free(lwpoint);
	}
	/* The resulting sequence assumes linear interpolation */
	TemporalSeq *result = temporalseq_make(instants, npoints,
		true, true, true, true);
	for (int i = 0; i < npoints; i++)
		pfree(instants[i]);
	pfree(instants);
	lwgeom_free(lwgeom);
	return result;
}

static TemporalS *
geo_to_tpoints(GSERIALIZED *gs)
{
	TemporalS *result;
	/* Geometry is a MULTILINESTRING or a COLLECTION */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
	int ngeoms = lwcoll->ngeoms;
	for (int i = 0; i < ngeoms; i++)
	{
		LWGEOM *lwgeom1 = lwcoll->geoms[i];
		if (lwgeom1->type != POINTTYPE && lwgeom1->type != LINETYPE)
		{
			lwgeom_free(lwgeom);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Component geometry/geography must be of type Point(Z)M or Linestring(Z)M")));
		}
	}
	
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ngeoms);
	for (int i = 0; i < ngeoms; i++)
	{
		LWGEOM *lwgeom1 = lwcoll->geoms[i];
		GSERIALIZED *gs1 = geometry_serialize(lwgeom1);
		if (lwgeom1->type == POINTTYPE)
		{
			TemporalInst *inst = geo_to_tpointinst(gs1);
			/* The resulting sequence assumes linear interpolation */
			sequences[i] = temporalseq_make(&inst, 1,
				true, true, true, false);
			pfree(inst);
		}
		else /* lwgeom1->type == LINETYPE */
			sequences[i] = geo_to_tpointseq(gs1);
		pfree(gs1);
	}
	/* The resulting sequence set assumes linear interpolation */
	result = temporals_make(sequences, ngeoms, false);
	for (int i = 0; i < ngeoms; i++)
		pfree(sequences[i]);
	pfree(sequences);
	lwgeom_free(lwgeom);
	return result;
}

PG_FUNCTION_INFO_V1(geo_to_tpoint);

PGDLLEXPORT Datum
geo_to_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	ensure_non_empty(gs);
	ensure_has_M_gs(gs);
	
	Temporal *result = NULL; /* Make compiler quiet */
	if (gserialized_get_type(gs) == POINTTYPE)
		result = (Temporal *)geo_to_tpointinst(gs);
	else if (gserialized_get_type(gs) == MULTIPOINTTYPE)
		result = (Temporal *)geo_to_tpointi(gs);
	else if (gserialized_get_type(gs) == LINETYPE)
		result = (Temporal *)geo_to_tpointseq(gs);
	else if (gserialized_get_type(gs) == MULTILINETYPE ||
		gserialized_get_type(gs) == COLLECTIONTYPE)
		result = (Temporal *)geo_to_tpoints(gs);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Invalid geometry type for trajectory")));
	
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Convert a temporal point into a LinestringM geometry/geography where the M
 * coordinates values are given by a temporal float.
 *****************************************************************************/

static LWPOINT *
point_measure_to_geo_measure(GSERIALIZED *gs, double measure)
{
	int32 srid = gserialized_get_srid(gs);
	LWPOINT *result;
	if (FLAGS_GET_Z(gs->flags))
	{
		POINT3DZ point = gs_get_point3dz(gs);
		result = lwpoint_make4d(srid, point.x, point.y, point.z, measure);
	}
	else
	{
		POINT2D point = gs_get_point2d(gs);
		result = lwpoint_make3dm(srid, point.x, point.y, measure);
	}
	FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
	return result;
}

static Datum
tpointinst_to_geo_measure(TemporalInst *inst, TemporalInst *measure)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	LWPOINT *point = point_measure_to_geo_measure(gs, DatumGetFloat8(temporalinst_value(measure)));
	GSERIALIZED *result = geometry_serialize((LWGEOM *)point);
	pfree(point);
	return PointerGetDatum(result);
}

static Datum
tpointi_to_geo_measure(TemporalI *ti, TemporalI *measure)
{
	TemporalInst *inst = temporali_inst_n(ti, 0);
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);
	LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		inst = temporali_inst_n(ti, i);
		TemporalInst *m = temporali_inst_n(measure, i);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		points[i] = (LWGEOM *)point_measure_to_geo_measure(gs, DatumGetFloat8(temporalinst_value(m)));
	}
	GSERIALIZED *result;
	if (ti->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *mpoint = (LWGEOM *)lwcollection_construct(MULTIPOINTTYPE, srid,
			NULL, (uint32_t) ti->count, points);
		result = geometry_serialize(mpoint);
		pfree(mpoint);
	}

	for (int i = 0; i < ti->count; i++)
		pfree(points[i]);
	pfree(points);
	return PointerGetDatum(result);
}

static Datum
tpointseq_to_geo_measure(TemporalSeq *seq, TemporalSeq *measure)
{
	LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
	/* Remove two consecutive points if they are equal */
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	TemporalInst *m = temporalseq_inst_n(measure, 0);
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	LWPOINT *value1 = point_measure_to_geo_measure(gs, DatumGetFloat8(temporalinst_value(m)));
	points[0] = (LWGEOM *) value1;
	int k = 1;
	for (int i = 1; i < seq->count; i++)
	{
		inst = temporalseq_inst_n(seq, i);
		m = temporalseq_inst_n(measure, i);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		LWPOINT *value2 = point_measure_to_geo_measure(gs, DatumGetFloat8(temporalinst_value(m)));
		if (lwpoint_same(value1, value2) != LW_TRUE)
			points[k++] = (LWGEOM *) value2;
		value1 = value2;
	}
	GSERIALIZED *result;
	/* Instantaneous sequence */
	if (k == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *geom;
		if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
			geom = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
				(uint32_t) k, points);
		else
			geom = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, points[0]->srid,
				NULL, (uint32_t) k, points);
		result = geometry_serialize(geom);
		pfree(geom);
	}

	for (int i = 0; i < k; i++)
		pfree(points[i]);
	pfree(points);

	return PointerGetDatum(result);
}

static Datum
tpointseq_to_geo_measure_segmentize(TemporalSeq *seq, TemporalSeq *measure)
{
	Datum *segments = palloc(sizeof(Datum) * (seq->count - 1));
	LWGEOM *points[2];
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	double m = DatumGetFloat8(temporalinst_value(temporalseq_inst_n(measure, 0)));
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	points[0] = (LWGEOM *) point_measure_to_geo_measure(gs, m);
	for (int i = 0; i < seq->count - 1; i++)
	{
		inst = temporalseq_inst_n(seq, i + 1);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		points[1] = (LWGEOM *) point_measure_to_geo_measure(gs, m);
		LWGEOM *seg = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
			2, points);
		segments[i] = PointerGetDatum(geometry_serialize(seg));
		// pfree(DatumGetPointer(points[0]));
		// pfree(DatumGetPointer(points[1]));
		m = DatumGetFloat8(temporalinst_value(temporalseq_inst_n(measure, i + 1)));
		points[0] =  (LWGEOM *) point_measure_to_geo_measure(gs, m);
		// pfree(seg);
	}
	// pfree(DatumGetPointer(points[0]));
	Datum result;
	/* Instantaneous sequence */
	if (seq->count == 1)
		result = segments[0];
	else
	{
		ArrayType *array = datumarr_to_array(segments, seq->count - 1,
			seq->valuetypid);
		/* ST_linemerge is not used to avoid splitting lines at intersections */
		result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
		pfree(array);
	}

	for (int i = 0; i < seq->count - 1; i++)
		pfree(DatumGetPointer(segments[i]));
	pfree(segments);

	return result;
}

static Datum
tpoints_to_geo_measure(TemporalS *ts, TemporalS *measure, bool segmentize)
{
	Datum *geoms = palloc(sizeof(Datum) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalSeq *m = temporals_seq_n(measure, i);
		geoms[i] = segmentize ?
			tpointseq_to_geo_measure_segmentize(seq, m) :
			tpointseq_to_geo_measure(seq, m);
	}
	Datum result;
	if (ts->count == 1)
		result = geoms[0];
	else
	{
		ArrayType *array = datumarr_to_array(geoms, ts->count, ts->valuetypid);
		result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
		for (int i = 0; i < ts->count; i++)
			pfree(DatumGetPointer(geoms[i]));
		pfree(array);
	}
	pfree(geoms);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_geo_measure);

PGDLLEXPORT Datum
tpoint_to_geo_measure(PG_FUNCTION_ARGS)
{
	Temporal *tpoint = PG_GETARG_TEMPORAL(0);
	Temporal *measure = PG_GETARG_TEMPORAL(1);
	bool segmentize = PG_GETARG_BOOL(2);
	ensure_point_base_type(tpoint->valuetypid);
	ensure_numeric_base_type(measure->valuetypid);
	Temporal *sync1, *sync2;
	/* Return false if the temporal values do not intersect in time
	   The last parameter crossing must be set to false  */
	if (!synchronize_temporal_temporal(tpoint, measure, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(tpoint, 0);
		PG_FREE_IF_COPY(measure, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = NULL;
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == TEMPORALINST)
		result = (Temporal *) tpointinst_to_geo_measure(
				(TemporalInst *) sync1, (TemporalInst *) sync2);
	else if (sync1->duration == TEMPORALI)
		result = (Temporal *) tpointi_to_geo_measure(
				(TemporalI *) sync1, (TemporalI *) sync2);
	else if (sync1->duration == TEMPORALSEQ)
		result = segmentize ?
			(Temporal *) tpointseq_to_geo_measure_segmentize(
					(TemporalSeq *) sync1, (TemporalSeq *) sync2) :
			(Temporal *) tpointseq_to_geo_measure(
				(TemporalSeq *) sync1, (TemporalSeq *) sync2);
	else if (sync1->duration == TEMPORALS)
		result = (Temporal *) tpoints_to_geo_measure(
				(TemporalS *) sync1, (TemporalS *) sync2, segmentize);

	pfree(sync1); pfree(sync2);
	PG_FREE_IF_COPY(tpoint, 0);
	PG_FREE_IF_COPY(measure, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
