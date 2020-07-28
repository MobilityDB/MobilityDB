/***********************************************************************
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
#include <math.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#if MOBDB_PGSQL_VERSION >= 120000
#include <utils/float.h>
#endif

#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tnumber_mathfuncs.h"
#include "postgis.h"
#include "geography_funcs.h"
#include "tpoint.h"
#include "tpoint_boxops.h"
#include "tpoint_distance.h"
#include "tpoint_spatialrels.h"

/*****************************************************************************
 * Functions derived from PostGIS
 *****************************************************************************/

/**
 * Returns a float between 0 and 1 representing the location of the closest 
 * point on the segment to the given point, as a fraction of total segment 
 * length (2D version).
 *
 * @note Function derived from the PostGIS function closest_point_on_segment.
 */
double
closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A, 
	const POINT2D *B, POINT2D *closest)
{
	if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y))
	{
		*closest = *A;
		return 0.0;
	}

	/*
	 * We use comp.graphics.algorithms Frequently Asked Questions method
	 *
	 * (1)          AC dot AB
	 *         r = ----------
	 *              ||AB||^2
	 *	r has the following meaning:
	 *	r=0 P = A
	 *	r=1 P = B
	 *	r<0 P is on the backward extension of AB
	 *	r>1 P is on the forward extension of AB
	 *	0<r<1 P is interior to AB
	 *
	 */
	double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
		( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

	if (r < 0)
	{
		*closest = *A;
		return 0.0;
	}
	if (r > 1)
	{
		*closest = *B;
		return 1.0;
	}

	closest->x = A->x + ( (B->x - A->x) * r );
	closest->y = A->y + ( (B->y - A->y) * r );
	return r;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest 
 * point on the segment to the given point, as a fraction of total segment 
 * length (3D version).
 *
 * @note Function derived from the PostGIS function closest_point_on_segment.
 */
double
closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A, 
	const POINT3DZ *B, POINT3DZ *closest)
{
	if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && 
		FP_EQUALS(A->z, B->z))
	{
		*closest = *A;
		return 0.0;
	}

	/*
	 * We use comp.graphics.algorithms Frequently Asked Questions method
	 *
	 * (1)          AC dot AB
	 *         r = ----------
	 *              ||AB||^2
	 *	r has the following meaning:
	 *	r=0 P = A
	 *	r=1 P = B
	 *	r<0 P is on the backward extension of AB
	 *	r>1 P is on the forward extension of AB
	 *	0<r<1 P is interior to AB
	 *
	 */
	double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + (p->z-A->z) * (B->z-A->z) ) /
		( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + (B->z-A->z) * (B->z-A->z) );

	if (r < 0)
	{
		*closest = *A;
		return 0.0;
	}
	if (r > 1)
	{
		*closest = *B;
		return 1.0;
	}

	closest->x = A->x + ( (B->x - A->x) * r );
	closest->y = A->y + ( (B->y - A->y) * r );
	closest->z = A->z + ( (B->z - A->z) * r );
	return r;
}

/*****************************************************************************/
 
/**
 * Compute the projected point and the distance between the closest point
 * (2D version).
 *
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint 
 * from measures.c
 */
static double
lw_dist2d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
	double *fraction)
{
	DISTPTS thedl;
	thedl.mode = mode;
	thedl.distance= FLT_MAX;
	thedl.tolerance = 0;
	lw_dist2d_recursive(lw1, lw2, &thedl);
	LWLINE *lwline = lwgeom_as_lwline(lw1);
	POINT2D a, b, closest;
	getPoint2d_p(lwline->points, 0, &a);
	getPoint2d_p(lwline->points, 1, &b);
	*fraction = closest_point2d_on_segment_ratio(&thedl.p1, &a, &b, &closest);
	return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (3D version).
 *
 * @note Function inspired by PostGIS function lw_dist3d_distancepoint 
 * from measures3d.c
 */
static double
lw_dist3d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
	double *fraction)
{
	assert(FLAGS_GET_Z(lw1->flags) && FLAGS_GET_Z(lw2->flags));
	DISTPTS3D thedl;
	thedl.mode = mode;
	thedl.distance= FLT_MAX;
	thedl.tolerance = 0;
	lw_dist3d_recursive(lw1, lw2, &thedl);
	LWLINE *lwline = lwgeom_as_lwline(lw1);
	POINT3DZ a, b, closest;
	getPoint3dz_p(lwline->points, 0, &a);
	getPoint3dz_p(lwline->points, 1, &b);
	*fraction = closest_point3dz_on_segment_ratio(&thedl.p1, &a, &b, &closest);
	return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (geodetic version).
 */
double
lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
	double *fraction)
{
	double min_dist = FLT_MAX;
	double max_dist = FLT_MAX;
	GEOGRAPHIC_POINT closest1, closest2, proj;
	GEOGRAPHIC_EDGE e;
	POINT4D a, b;

	CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(lw1);
	CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(lw2);
	circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
		&min_dist, &max_dist, &closest1, &closest2);
	double result = sphere_distance(&closest1, &closest2);

	/* Initialize edge */
	LWLINE *lwline = lwgeom_as_lwline(lw1);
	getPoint4d_p(lwline->points, 0, &a);
	getPoint4d_p(lwline->points, 1, &b);
	geographic_point_init(a.x, a.y, &(e.start));
	geographic_point_init(b.x, b.y, &(e.end));

	/* Get the spherical distance between point and edge */
	edge_distance_to_point(&e, &closest1, &proj);

	/* Compute distance from beginning of the segment to closest point */
	double seglength = sphere_distance(&(e.start), &(e.end));
	double length = sphere_distance(&(e.start), &closest1);
	*fraction = length / seglength;

	return result;
}

/*****************************************************************************
 * Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint.
 *****************************************************************************/

/**
 * Returns a point interpolated from the geometry segment with respect to the
 * fraction of its total length.
 *
 *@param[in] start,end Points defining the segment
 *@param[in] ratio Float between 0 and 1 representing the fraction of the 
 * total length of the segment where the point must be located
 */
Datum
geomseg_interpolate_point(Datum start, Datum end, double ratio)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
	int srid = gserialized_get_srid(gs);
	POINT4D p1 = datum_get_point4d(start);
	POINT4D p2 = datum_get_point4d(end);
	POINT4D p;
	interpolate_point4d(&p1, &p2, &p, ratio);
	LWPOINT *lwpoint = FLAGS_GET_Z(gs->flags) ?
		lwpoint_make3dz(srid, p.x, p.y, p.z) :
		lwpoint_make2d(srid, p.x, p.y);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *) lwpoint));
	lwpoint_free(lwpoint);
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(start));
	return result;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest
 * point on the geometry segment to the given point, as a fraction of total 
 * segment length.
 *
 *@param[in] start,end Points defining the segment
 *@param[in] point Reference point
 *@param[out] dist Distance
 */
double
geomseg_locate_point(Datum start, Datum end, Datum point, double *dist)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
	double result;
	if (FLAGS_GET_Z(gs->flags))
	{
		const POINT3DZ *p1 = datum_get_point3dz_p(start);
		const POINT3DZ *p2 = datum_get_point3dz_p(end);
		const POINT3DZ *p = datum_get_point3dz_p(point);
		POINT3DZ proj;
		result = closest_point3dz_on_segment_ratio(p, p1, p2, &proj);
		if (p3d_same((POINT3D *) p1, (POINT3D *) &proj))
			result = 0.0;
		else if (p3d_same((POINT3D *) p2, (POINT3D *) &proj))
			result = 1.0;
		if (dist)
			*dist = distance3d_pt_pt((POINT3D *)p, (POINT3D *)&proj);
	}
	else
	{
		const POINT2D *p1 = datum_get_point2d_p(start);
		const POINT2D *p2 = datum_get_point2d_p(end);
		const POINT2D *p = datum_get_point2d_p(point);
		POINT2D proj;
		result = closest_point2d_on_segment_ratio(p, p1, p2, &proj);
		if (p2d_same(p1, &proj))
			result = 0.0;
		else if (p2d_same(p2, &proj))
			result = 1.0;
		if (dist != NULL)
			*dist = distance2d_pt_pt((POINT2D *)p, &proj);
	}
	return result;
}

/**
 * Returns a point interpolated from the geography segment with respect to the
 * fraction of its total length.
 *
 *@param[in] start,end Points defining the segment
 *@param[in] ratio Float between 0 and 1 representing the fraction of the 
 * total length of the segment where the point must be located
 */
Datum
geogseg_interpolate_point(Datum start, Datum end, double ratio)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
	int srid = gserialized_get_srid(gs);
	POINT4D p1 = datum_get_point4d(start);
	POINT4D p2 = datum_get_point4d(end);
	POINT4D p;
	POINT3D q1, q2;
	GEOGRAPHIC_POINT g1, g2;
	geographic_point_init(p1.x, p1.y, &g1);
	geographic_point_init(p2.x, p2.y, &g2);
	geog2cart(&g1, &q1);
	geog2cart(&g2, &q2);
	interpolate_point4d_sphere(&q1, &q2, &p1, &p2, ratio, &p);
	LWPOINT *lwpoint = FLAGS_GET_Z(gs->flags) ?
		lwpoint_make3dz(srid, p.x, p.y, p.z) :
		lwpoint_make2d(srid, p.x, p.y);
	FLAGS_SET_GEODETIC(lwpoint->flags, true);
	Datum result = PointerGetDatum(geometry_serialize((LWGEOM *) lwpoint));
	lwpoint_free(lwpoint);
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(start));
	return result;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest
 * point on the geography segment to the given point, as a fraction of total 
 * segment length.
 *
 *@param[in] p Reference point
 *@param[in] A,B Points defining the segment
 *@param[out] closest Closest point in the segment
 *@param[out] dist Distance between the closest point and the reference point 
 */
double
closest_point_on_segment_sphere(const POINT4D *p, const POINT4D *A,
	const POINT4D *B, POINT4D *closest, double *dist)
{
	GEOGRAPHIC_EDGE e;
	GEOGRAPHIC_POINT a, proj;
	double length, /* length from A to the closest point */
		seglength, /* length of the segment AB */
		result; /* ratio */

	/* Initialize target point */
	geographic_point_init(p->x, p->y, &a);

	/* Initialize edge */
	geographic_point_init(A->x, A->y, &(e.start));
	geographic_point_init(B->x, B->y, &(e.end));

	/* Get the spherical distance between point and edge */
	*dist = edge_distance_to_point(&e, &a, &proj);

	/* Compute distance from beginning of the segment to closest point */
	seglength = sphere_distance(&(e.start), &(e.end));
	length = sphere_distance(&(e.start), &proj);
	result = length / seglength;

	if (closest)
	{
		/* Copy nearest into returning argument */
		closest->x = rad2deg(proj.lon);
		closest->y = rad2deg(proj.lat);

		/* Compute Z and M values for closest point */
		closest->z = A->z + ((B->z - A->z) * result);
		closest->m = A->m + ((B->m - A->m) * result);
	}
	return result;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest
 * point on the geography segment to the given point, as a fraction of total 
 * segment length.
 *
 *@param[in] start,end Points defining the segment
 *@param[in] point Reference point
 *@param[out] dist Distance between the closest point and the reference point 
 * @note In the current version of MobilityDB all geographic computations
 * are done in using a sphere and not a spheroid 
 */
double
geogseg_locate_point(Datum start, Datum end, Datum point, double *dist)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
	POINT4D p1 = datum_get_point4d(start);
	POINT4D p2 = datum_get_point4d(end);
	POINT4D p = datum_get_point4d(point);
	POINT4D closest;
	double d;

	/* Get the closest point and the distance */
	double result = closest_point_on_segment_sphere(&p, &p1, &p2, &closest, &d);

	/* Return the distance between the closest point and the point if requested */
	if (dist != NULL)
	{
		d = WGS84_RADIUS * d;
		/* Add to the distance the vertical displacement if we're in 3D */
		if (FLAGS_GET_Z(gs->flags))
			d = sqrt( (closest.z - p.z) * (closest.z - p.z) + d*d );
		*dist = d;
	}

	// FIX datum_get_point4d
	/* For robustness, force 0/1 when closest point == start/endpoint */
	if (p4d_same(&p1, &closest))
		return 0.0;
	else if (p4d_same(&p2, &closest))
		return 1.0;
	return result;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensures that the spatiotemporal boxes have the same type of coordinates, 
 * either planar or geodetic
 */
void
ensure_same_geodetic_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
		MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "The boxes must be both planar or both geodetic");
}

/**
 * Ensures that the temporal point and the spatiotemporal box have the same 
 * type of coordinates, either planar or geodetic
 */
void
ensure_same_geodetic_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(box->flags) &&
		MOBDB_FLAGS_GET_GEODETIC(temp->flags) != MOBDB_FLAGS_GET_GEODETIC(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be both planar or both geodetic")));
}

/**
 * Ensures that the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
		box1->srid != box2->srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must be in the same SRID")));
}

/**
 * Ensures that the temporal points have the same SRID
 */
void
ensure_same_srid_tpoint(const Temporal *temp1, const Temporal *temp2)
{
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal points must be in the same SRID")));
}

/**
 * Ensures that the temporal point and the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(box->flags) &&
		tpoint_srid_internal(temp) != box->srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be in the same SRID")));
}

/**
 * Ensures that the temporal point and the geometry/geography have the same SRID
 */
void
ensure_same_srid_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the geometry must be in the same SRID")));
}

/**
 * Ensures that the spatiotemporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
		MOBDB_FLAGS_GET_Z(box1->flags) != MOBDB_FLAGS_GET_Z(box2->flags) ||
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must be of the same dimensionality")));
}

/**
 * Ensures that the temporal points have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint(const Temporal *temp1, const Temporal *temp2)
{
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal points must be of the same dimensionality")));
}

/**
 * Ensures that the temporal point and the spatiotemporal boxes have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(temp->flags) != MOBDB_FLAGS_GET_X(box->flags) ||
		MOBDB_FLAGS_GET_Z(temp->flags) != MOBDB_FLAGS_GET_Z(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be of the same spatial dimensionality")));
}

/**
 * Ensures that the temporal point and the spatiotemporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
	if (MOBDB_FLAGS_GET_X(temp->flags) != MOBDB_FLAGS_GET_X(box->flags) ||
		MOBDB_FLAGS_GET_Z(temp->flags) != MOBDB_FLAGS_GET_Z(box->flags) ||
		MOBDB_FLAGS_GET_T(temp->flags) != MOBDB_FLAGS_GET_T(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the box must be of the same dimensionality")));
}

/**
 * Ensures that the temporal point and the geometry/geography have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point and the geometry must be of the same dimensionality")));
}

/**
 * Ensures that the spatiotemporal boxes have at least one common dimension
 */
void
ensure_common_dimension_stbox(const STBOX *box1, const STBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) &&
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The boxes must have at least one common dimension")));
}

/**
 * Ensures that the spatiotemporal box has XY dimension
 */
void
ensure_has_X_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_X(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have XY dimension")));
}

/**
 * Ensures that the spatiotemporal box has Z dimension
 */
void
ensure_has_Z_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_Z(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have Z dimension")));
}

/**
 * Ensures that the spatiotemporal box has T dimension
 */
void
ensure_has_T_stbox(const STBOX *box)
{
	if (! MOBDB_FLAGS_GET_T(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The box must have time dimension")));
}

/**
 * Ensures that the temporal point has Z dimension
 */
void
ensure_has_Z_tpoint(const Temporal *temp)
{
	if (! MOBDB_FLAGS_GET_Z(temp->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point must have Z dimension")));
}

/**
 * Ensures that the temporal point has not Z dimension
 */
void
ensure_has_not_Z_tpoint(const Temporal *temp)
{
	if (MOBDB_FLAGS_GET_Z(temp->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("The temporal point cannot have Z dimension")));
}

/**
 * Ensures that the geometry/geography has Z dimension
 */
void
ensure_has_Z_gs(const GSERIALIZED *gs)
{
	if (! FLAGS_GET_Z(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries with Z dimension accepted")));
}

/**
 * Ensures that the geometry/geography has not Z dimension
 */
void
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
	if (FLAGS_GET_Z(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries without Z dimension accepted")));
}

/**
 * Ensures that the geometry/geography has M dimension
 */
void
ensure_has_M_gs(const GSERIALIZED *gs)
{
	if (! FLAGS_GET_M(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries with M dimension accepted")));
}

/**
 * Ensures that the geometry/geography has not M dimension
 */
void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
	if (FLAGS_GET_M(gs->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only geometries without M dimension accepted")));
}

/**
 * Ensures that the geometry/geography is a point
 */
void
ensure_point_type(const GSERIALIZED *gs)
{
	if (gserialized_get_type(gs) != POINTTYPE)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Only point geometries accepted")));
}

/**
 * Ensures that the geometry/geography is not empty
 */
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
 * Obtain a geometry/geography point from the GSERIALIZED WITHOUT creating
 * the corresponding LWGEOM. These functions constitute a **SERIOUS**
 * break of encapsulation but it is the only way to achieve reasonable
 * performance when manipulating mobility data.
 * The datum_* functions suppose that the GSERIALIZED has been already
 * detoasted. This is typically the case when the datum is within a Temporal*
 * that has been already detoasted with PG_GETARG_TEMPORAL*
 * The first variant (e.g. datum_get_point2d) is slower than the second (e.g.
 * datum_get_point2d_p) since the point is passed by value and thus the bytes
 * are copied. The second version is declared const because you aren't allowed
 * to modify the values, only read them.
 */

/**
 * Returns a 2D point from the serialized geometry 
 */
const POINT2D *
gs_get_point2d_p(GSERIALIZED *gs)
{
	return (POINT2D *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 2D point from the datum 
 */
POINT2D
datum_get_point2d(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
	POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
	return *point;
}

/**
 * Returns a pointer to a 2D point from the datum 
 */
const POINT2D *
datum_get_point2d_p(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
	return (POINT2D *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 3DZ point from the serialized geometry 
 */
const POINT3DZ *
gs_get_point3dz_p(GSERIALIZED *gs)
{
	return (POINT3DZ *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 3DZ point from the datum 
 */
POINT3DZ
datum_get_point3dz(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
	POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
	return *point;
}

/**
 * Returns a pointer to a 3DZ point from the datum 
 */
const POINT3DZ *
datum_get_point3dz_p(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
	return (POINT3DZ *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 4D point from the datum 
 */
POINT4D
datum_get_point4d(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
	POINT4D *point = (POINT4D *)((uint8_t*)gs->data + 8);
	return *point;
}

/**
 * Returns true if the two points are equal
 */
bool
datum_point_eq(Datum geopoint1, Datum geopoint2)
{
	GSERIALIZED *gs1 = (GSERIALIZED *) DatumGetPointer(geopoint1);
	GSERIALIZED *gs2 = (GSERIALIZED *) DatumGetPointer(geopoint2);
	if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
		FLAGS_GET_Z(gs1->flags) != FLAGS_GET_Z(gs2->flags) ||
		FLAGS_GET_GEODETIC(gs1->flags) != FLAGS_GET_GEODETIC(gs2->flags))
		return false;
	if (FLAGS_GET_Z(gs1->flags))
	{
		const POINT3DZ *point1 = gs_get_point3dz_p(gs1);
		const POINT3DZ *point2 = gs_get_point3dz_p(gs2);
		return point1->x == point2->x && point1->y == point2->y &&
			point1->z == point2->z;
	}
	else
	{
		const POINT2D *point1 = gs_get_point2d_p(gs1);
		const POINT2D *point2 = gs_get_point2d_p(gs2);
		return point1->x == point2->x && point1->y == point2->y;
	}
}

/**
 * Returns true encoded as a datum if the two points are equal
 */
Datum
datum2_point_eq(Datum geopoint1, Datum geopoint2)
{
	return BoolGetDatum(datum_point_eq(geopoint1, geopoint2));
}

/**
 * Returns true encoded as a datum if the two points are different
 */
Datum
datum2_point_ne(Datum geopoint1, Datum geopoint2)
{
	return BoolGetDatum(! datum_point_eq(geopoint1, geopoint2));
}

/**
 * Set the precision of the point coordinates to the number
 * of decimal places
 */
static Datum
datum_set_precision(Datum value, Datum size)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
	int srid = gserialized_get_srid(gs);
	LWPOINT *lwpoint;
	if (FLAGS_GET_Z(gs->flags))
	{
		const POINT3DZ *point = gs_get_point3dz_p(gs);
		double x = DatumGetFloat8(datum_round(Float8GetDatum(point->x), size));
		double y = DatumGetFloat8(datum_round(Float8GetDatum(point->y), size));
		double z = DatumGetFloat8(datum_round(Float8GetDatum(point->z), size));
		lwpoint = lwpoint_make3dz(srid, x, y, z);
	}
	else
	{
		const POINT2D *point = gs_get_point2d_p(gs);
		double x = DatumGetFloat8(datum_round(Float8GetDatum(point->x), size));
		double y = DatumGetFloat8(datum_round(Float8GetDatum(point->y), size));
		lwpoint = lwpoint_make2d(srid, x, y);
	}
	GSERIALIZED *result = geometry_serialize((LWGEOM *) lwpoint);
	result->flags = gs->flags;
	pfree(lwpoint);
	return PointerGetDatum(result);
}

/**
 * Serialize a geometry 
 */
GSERIALIZED *
geometry_serialize(LWGEOM *geom)
{
	size_t size;
	GSERIALIZED *result = gserialized_from_lwgeom(geom, &size);
	SET_VARSIZE(result, size);
	return result;
}

/**
 * Serialize a geography 
 */
GSERIALIZED *
geography_serialize(LWGEOM *geom)
{
	size_t size;
	/** force to geodetic in case it's not **/
	lwgeom_set_geodetic(geom, true);
	GSERIALIZED *result = gserialized_from_lwgeom(geom, &size);
	SET_VARSIZE(result, size);
	return result;
}

/**
 * Call the PostGIS transform function
 */
static Datum
datum_transform(Datum value, Datum srid)
{
	return call_function2(transform, value, srid);
}

/**
 * Call the PostGIS geometry_from_geography function
 */
static Datum
geog_to_geom(Datum value)
{
	return call_function1(geometry_from_geography, value);
}

/**
 * Call the PostGIS geography_from_geometry function
 */
static Datum
geom_to_geog(Datum value)
{
	return call_function1(geography_from_geometry, value);
}

/*****************************************************************************
 * Trajectory functions.
 *****************************************************************************/

/**
 * Assemble the set of points of a temporal instant set geometry point as a
 * single geometry.
 * @note Duplicate points are removed.
 */
static Datum
tgeompointi_trajectory(const TemporalI *ti)
{
	/* Singleton instant set */
	if (ti->count == 1)
		return temporalinst_value_copy(temporali_inst_n(ti, 0));

	LWPOINT **points = palloc(sizeof(LWPOINT *) * ti->count);
	/* Remove all duplicate points */
	TemporalInst *inst = temporali_inst_n(ti, 0);
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	LWPOINT *value = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
	points[0] = value;
	int k = 1;
	for (int i = 1; i < ti->count; i++)
	{
		inst = temporali_inst_n(ti, i);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		value = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
		bool found = false;
		for (int j = 0; j < k; j++)
		{
			if (lwpoint_same(value, points[j]) == LW_TRUE)
			{
				found = true;
				break;
			}
		}
		if (!found)
			points[k++] = value;
	}
	LWGEOM *lwresult;
	if (k == 1)
	{
		lwresult = (LWGEOM *) points[0];
	}
	else
	{
		lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
			points[0]->srid, NULL, (uint32_t) k, (LWGEOM **) points);
		for (int i = 0; i < k; i++)
			lwpoint_free(points[i]);
	}
	Datum result = PointerGetDatum(geometry_serialize(lwresult));
	pfree(points);
	return result;
}

/**
 * Assemble the set of points of a temporal instant set as a
 * single geography/geography.
 */
Datum
tpointi_trajectory(const TemporalI *ti)
{
	Datum result;
	ensure_point_base_type(ti->valuetypid);
	if (ti->valuetypid == type_oid(T_GEOMETRY))
		result = tgeompointi_trajectory(ti);
	else
	{
		TemporalI *tigeom = tfunc1_temporali(ti, &geog_to_geom,
			type_oid(T_GEOMETRY));
		Datum geomtraj = tgeompointi_trajectory(tigeom);
		result = call_function1(geography_from_geometry, geomtraj);
		pfree(DatumGetPointer(geomtraj));
	}
	return result;
}

/*****************************************************************************/

/**
 * Compute the trajectory from the two geometry points
 */
LWLINE *
geopoint_trajectory_lwline(Datum value1, Datum value2)
{
	GSERIALIZED *gs1 = (GSERIALIZED *)DatumGetPointer(value1);
	GSERIALIZED *gs2 = (GSERIALIZED *)DatumGetPointer(value2);
	LWGEOM *geoms[2];
	geoms[0] = lwgeom_from_gserialized(gs1);
	geoms[1] = lwgeom_from_gserialized(gs2);
	LWLINE *result = lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);
	lwgeom_free(geoms[0]); lwgeom_free(geoms[1]);
	return result;
}

/**
 * Compute the trajectory from the two points
 *
 * @param[in] value1,value2 Points
 * @param[in] geodetic True when the points are geodetic
 * @note Function called during normalization for determining whether three 
 * consecutive points are collinear, for computing the temporal distance,
 * the temporal spatial relationships, etc. 
 */
Datum
geopoint_trajectory(Datum value1, Datum value2, bool geodetic)
{
	LWGEOM *traj = (LWGEOM *)geopoint_trajectory_lwline(value1, value2);
	GSERIALIZED *result = geodetic ? geography_serialize(traj) :
		geometry_serialize(traj);
	lwgeom_free(traj);
	return PointerGetDatum(result);
}

/*****************************************************************************/

/**
 * Compute a trajectory from a set of points. The result is either a line or
 * a multipoint depending on whether the interpolation is step or linear
 *
 * @param[in] lwpoints Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] linear True when the interpolation is linear
 */
static Datum
lwpointarr_make_trajectory(LWGEOM **lwpoints, int count, bool linear)
{
	LWGEOM *lwgeom = linear ?
		(LWGEOM *) lwline_from_lwgeom_array(lwpoints[0]->srid,
			(uint32_t) count, lwpoints) :
		(LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, lwpoints[0]->srid,
			NULL, (uint32_t) count, lwpoints);
	FLAGS_SET_Z(lwgeom->flags, FLAGS_GET_Z(lwpoints[0]->flags));
	/* geodetic flag will be set in geography_serialize */
	Datum result = FLAGS_GET_GEODETIC(lwpoints[0]->flags) ?
		PointerGetDatum(geography_serialize(lwgeom)) :
		PointerGetDatum(geometry_serialize(lwgeom));
	pfree(lwgeom);
	return result;
}

/**
 * Compute a trajectory from a set of points. The result is either a line or
 * a multipoint depending on whether the interpolation is step or linear 
 *
 * @param[in] points Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] linear True when the interpolation is linear
 */
static Datum
pointarr_make_trajectory(Datum *points, int count, bool linear)
{
	LWGEOM **lwpoints = palloc(sizeof(LWGEOM *) * count);
	for (int i = 0; i < count; i++)
	{
		GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(points[i]);
		lwpoints[i] = lwgeom_from_gserialized(gs);
	}
	Datum result = lwpointarr_make_trajectory(lwpoints, count, linear);
	for (int i = 0; i < count; i++)
		lwgeom_free(lwpoints[i]);
	pfree(lwpoints);
	return result;
}

/**
 * Compute the trajectory of an array of instants.
 *
 * @note This function is called by the constructor of a temporal sequence
 * and returns a single Datum which is a geometry/geography.
 * Since the composing points have been already validated in the constructor
 * there is no verification of the input in this function, in particular
 * for geographies it is supposed that the composing points are geodetic 
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] linear True when the interpolation is linear
 */
Datum
tpointseq_make_trajectory(TemporalInst **instants, int count, bool linear)
{
	Oid valuetypid = instants[0]->valuetypid;
	ensure_point_base_type(valuetypid);
	LWPOINT **points = palloc(sizeof(LWPOINT *) * count);
	LWPOINT *lwpoint;
	Datum value;
	GSERIALIZED *gsvalue;
	int k;
	if (linear)
	{
		/* Remove two consecutive points if they are equal */
		value = temporalinst_value(instants[0]);
		gsvalue = (GSERIALIZED *) DatumGetPointer(value);
		points[0] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
		k = 1;
		for (int i = 1; i < count; i++)
		{
			value = temporalinst_value(instants[i]);
			gsvalue = (GSERIALIZED *) DatumGetPointer(value);
			lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
			if (! lwpoint_same(lwpoint, points[k - 1]))
				points[k++] = lwpoint;
		}
	}
	else
	{
		 /* Remove all duplicate points */
		k = 0;
		for (int i = 0; i < count; i++)
		{
			value = temporalinst_value(instants[i]);
			gsvalue = (GSERIALIZED *) DatumGetPointer(value);
			lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
			bool found = false;
			for (int j = 0; j < k; j++)
			{
				if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
				{
					found = true;
					break;
				}
			}
			if (!found)
				points[k++] = lwpoint;
		}
	}
	Datum result = (k == 1) ?
		PointerGetDatum(geometry_serialize((LWGEOM *)points[0])) :
		lwpointarr_make_trajectory((LWGEOM **)points, k, linear);
	for (int i = 0; i < k; i++)
		lwpoint_free(points[i]);
	pfree(points);
	return result;
}

/**
 * Returns the precomputed trajectory of a temporal sequence point
 */
Datum
tpointseq_trajectory(const TemporalSeq *seq)
{
	void *traj = (char *)(&seq->offsets[seq->count + 2]) + 	/* start of data */
		seq->offsets[seq->count + 1];						/* offset */
	return PointerGetDatum(traj);
}

/**
 * Add or replace a point to the trajectory of a temporal sequence point
 */
Datum
tpointseq_trajectory_append(const TemporalSeq *seq, const TemporalInst *inst,
	bool replace)
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
				return geopoint_trajectory(traj, point, false);
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
		bool foundpoint = false;
		for (int i = 0; i < count - 1; i++)
		{
			Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
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
			if (!foundpoint && datum_point_eq(value, point))
				foundpoint = true;
		}
		if (!foundpoint)
			points[k++] = point;
		Datum result = pointarr_make_trajectory(points, k, false);
		pfree(points);
		return result;
	}
	/* The trajectory is a Linestring */
	else
	{
		if (replace)
			return call_function3(LWGEOM_setpoint_linestring, traj,
				Int32GetDatum(-1), point);
		else
			return call_function2(LWGEOM_addpoint, traj, point);
	}
}

/**
 * Join the trajectories of two temporal sequence points
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] last,first True when the last point of the first sequence and/or
 * the first point of the second sequence is removed from the output
 */
Datum
tpointseq_trajectory_join(const TemporalSeq *seq1, const TemporalSeq *seq2, 
	bool last, bool first)
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

/**
 * Copy the precomputed trajectory of a temporal sequence point
 */
Datum
tpointseq_trajectory_copy(const TemporalSeq *seq)
{
	void *traj = (char *)(&seq->offsets[seq->count + 2]) + 	/* start of data */
			seq->offsets[seq->count + 1];					/* offset */
	return PointerGetDatum(gserialized_copy(traj));
}

/*****************************************************************************/

/**
 * Returns the trajectory of a temporal geography point with sequence set 
 * duration from the precomputed trajectories of its composing segments.
 * 
 * @note The resulting trajectory must be freed by the calling function.
 * The function removes duplicates points.
 */
static Datum
tgeompoints_trajectory(const TemporalS *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_trajectory_copy(temporals_seq_n(ts, 0));

	LWPOINT **points = palloc(sizeof(LWPOINT *) * ts->totalcount);
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
	int k = 0, l = 0;
	for (int i = 0; i < ts->count; i++)
	{
		Datum traj = tpointseq_trajectory(temporals_seq_n(ts, i));
		GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
		LWPOINT *lwpoint;
		if (gserialized_get_type(gstraj) == POINTTYPE)
		{
			lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gstraj));
			bool found = false;
			for (int j = 0; j < l; j++)
			{
				if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
				{
					found = true;
					break;
				}
			}
			if (!found)
				points[l++] = lwpoint;
		}
		else if (gserialized_get_type(gstraj) == MULTIPOINTTYPE)
		{
			LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gstraj));
			int count = lwmpoint->ngeoms;
			for (int m = 0; m < count; m++)
			{
				lwpoint = lwmpoint->geoms[m];
				bool found = false;
				for (int j = 0; j < l; j++)
				{
					if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
						{
							found = true;
							break;
						}
				}
				if (!found)
					points[l++] = lwpoint;
			}
		}
		/* gserialized_get_type(gstraj) == LINETYPE */
		else
		{
			geoms[k++] = lwgeom_from_gserialized(gstraj);
		}
	}
	Datum result;
	if (k == 0)
	{
		/* Only points */
		if (l == 1)
			result = PointerGetDatum(geometry_serialize((LWGEOM *)points[0]));
		else
			result = lwpointarr_make_trajectory((LWGEOM **)points, l, false);
	}
	else if (l == 0)
	{
		/* Only lines */
		/* k > 1 since otherwise it is a singleton sequence set and this case
		 * was taken care at the begining of the function */
		// TODO add the bounding box instead of ask PostGIS to compute it again
		// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
		LWGEOM *coll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
			geoms[0]->srid, NULL, (uint32_t) k, geoms);
		result = PointerGetDatum(geometry_serialize(coll));
		/* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
	}
	else
	{
		/* Both points and lines */
		if (l == 1)
			geoms[k++] = (LWGEOM *)points[0];
		else
		{
			geoms[k++] = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
				points[0]->srid, NULL, (uint32_t) l, (LWGEOM **) points);
			for (int i = 0; i < l; i++)
				lwpoint_free(points[i]);
		}
		// TODO add the bounding box instead of ask PostGIS to compute it again
		// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
		LWGEOM *coll = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE,
			geoms[0]->srid, NULL, (uint32_t) k, geoms);
		result = PointerGetDatum(geometry_serialize(coll));
	}
	pfree(points); pfree(geoms);
	return result;
}

/**
 * Returns the trajectory of a temporal geography point with sequence set 
 * duration
 */
static Datum
tgeogpoints_trajectory(const TemporalS *ts)
{
	TemporalS *tsgeom = tfunc1_temporals(ts, &geog_to_geom,
		type_oid(T_GEOMETRY));
	Datum geomtraj = tgeompoints_trajectory(tsgeom);
	Datum result = call_function1(geography_from_geometry, geomtraj);
	pfree(DatumGetPointer(geomtraj));
	return result;
}

/**
 * Returns the trajectory of a temporal sequence set point
 */
Datum
tpoints_trajectory(const TemporalS *ts)
{
	Datum result;
	ensure_point_base_type(ts->valuetypid);
	if (ts->valuetypid == type_oid(T_GEOMETRY))
		result = tgeompoints_trajectory(ts);
	else
		result = tgeogpoints_trajectory(ts);
	return result;
}

/*****************************************************************************/

/**
 * Returns the trajectory of a temporal point (dispatch function)
 */
Datum
tpoint_trajectory_internal(const Temporal *temp)
{
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_trajectory((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_trajectory_copy((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tpoints_trajectory((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_trajectory);
/**
 * Returns the trajectory of a temporal point
 */
PGDLLEXPORT Datum
tpoint_trajectory(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tpoint_trajectory_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * Returns the SRID of a temporal instant point 
 */
int
tpointinst_srid(const TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(inst));
	return gserialized_get_srid(gs);
}

/**
 * Returns the SRID of a temporal instant set point
 */
int
tpointi_srid(const TemporalI *ti)
{
	STBOX *box = temporali_bbox_ptr(ti);
	return box->srid;
}

/**
 * Returns the SRID of a temporal sequence point
 */
int
tpointseq_srid(const TemporalSeq *seq)
{
	STBOX *box = temporalseq_bbox_ptr(seq);
	return box->srid;
}

/**
 * Returns the SRID of a temporal sequence set point
 */
int
tpoints_srid(const TemporalS *ts)
{
	STBOX *box = temporals_bbox_ptr(ts);
	return box->srid;
}

/**
 * Returns the SRID of a temporal point (dispatch function)
 */
int
tpoint_srid_internal(const Temporal *temp)
{
	int result;
	ensure_valid_duration(temp->duration);
	ensure_point_base_type(temp->valuetypid);
	if (temp->duration == TEMPORALINST)
		result = tpointinst_srid((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_srid((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_srid((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tpoints_srid((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_srid);
/**
 * Returns the SRID of a temporal point
 */
PGDLLEXPORT Datum
tpoint_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result = tpoint_srid_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

/*****************************************************************************/

/**
 * Set the SRID of a temporal instant point 
 */
static TemporalInst *
tpointinst_set_srid(TemporalInst *inst, int32 srid)
{
	TemporalInst *result = temporalinst_copy(inst);
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value_ptr(result));
	gserialized_set_srid(gs, srid);
	return result;
}

/**
 * Set the SRID of a temporal instant set point 
 */
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

/**
 * Set the SRID of a temporal sequence point 
 */
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

/**
 * Set the SRID of a temporal sequence set point 
 */
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

/**
 * Set the SRID of a temporal point (dispatch function)
 */
Temporal *
tpoint_set_srid_internal(Temporal *temp, int32 srid)
{
	Temporal *result;
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_set_srid((TemporalInst *)temp, srid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_set_srid((TemporalI *)temp, srid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_set_srid((TemporalSeq *)temp, srid);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_set_srid((TemporalS *)temp, srid);

	assert(result != NULL);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_set_srid);
/**
 * Set the SRID of a temporal point 
 */
PGDLLEXPORT Datum
tpoint_set_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 srid = PG_GETARG_INT32(1);
	Temporal *result = tpoint_set_srid_internal(temp, srid);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transform a temporal instant point into another spatial reference system 
 */
TemporalInst *
tpointinst_transform(const TemporalInst *inst, Datum srid)
{
	Datum geo = datum_transform(temporalinst_value(inst), srid);
	TemporalInst *result = temporalinst_make(geo, inst->t, inst->valuetypid);
	pfree(DatumGetPointer(geo));
	return result;
}

/**
 * Transform a temporal instant set point into another spatial reference system 
 */
static TemporalI *
tpointi_transform(const TemporalI *ti, Datum srid)
{
	/* Singleton sequence */
	if (ti->count == 1)
	{
		TemporalInst *inst = tpointinst_transform(temporali_inst_n(ti, 0), srid);
		TemporalI *result = temporali_make(&inst, 1);
		pfree(inst);
		return result;
	}

	LWPOINT **points = palloc(sizeof(LWPOINT *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		Datum value = temporalinst_value(temporali_inst_n(ti, i));
		GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value);
		points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
	}
	Datum multipoint = lwpointarr_make_trajectory((LWGEOM **)points, ti->count, false);
	Datum transf = call_function2(transform, multipoint, srid);
	GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
	LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		Datum point = PointerGetDatum(geometry_serialize((LWGEOM *) (lwmpoint->geoms[i])));
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = temporalinst_make(point, inst->t, inst->valuetypid);
		pfree(DatumGetPointer(point));
	}
	TemporalI *result = temporali_make(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		lwpoint_free(points[i]);
	pfree(points);
	pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
	lwmpoint_free(lwmpoint);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/**
 * Transform a temporal sequence point into another spatial reference system 
 */
static TemporalSeq *
tpointseq_transform(const TemporalSeq *seq, Datum srid)
{
	/* Singleton sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = tpointinst_transform(temporalseq_inst_n(seq, 0), srid);
		TemporalSeq *result = temporalseq_make(&inst, 1, true, true,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
		pfree(inst);
		return result;
	}

	LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value);
		points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
	}
	Datum multipoint = lwpointarr_make_trajectory((LWGEOM **)points, seq->count, false);
	Datum transf = call_function2(transform, multipoint, srid);
	GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
	LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		Datum point = PointerGetDatum(geometry_serialize((LWGEOM *) (lwmpoint->geoms[i])));
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = temporalinst_make(point, inst->t, inst->valuetypid);
		pfree(DatumGetPointer(point));
	}
	TemporalSeq *result = temporalseq_make(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
	for (int i = 0; i < seq->count; i++)
		lwpoint_free(points[i]);
	pfree(points);
	pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
	lwmpoint_free(lwmpoint);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/**
 * Transform a temporal sequence set point into another spatial reference system 
 */
static TemporalS *
tpoints_transform(const TemporalS *ts, Datum srid)
{
	/* Singleton sequence set */
	if (ts->count == 1)
	{
		TemporalSeq *seq = tpointseq_transform(temporals_seq_n(ts, 0), srid);
		TemporalS *result = temporalseq_to_temporals(seq);
		pfree(seq);
		return result;
	}
	int k = 0;
	LWPOINT **points = palloc(sizeof(LWPOINT *) * ts->totalcount);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
		{
			Datum value = temporalinst_value(temporalseq_inst_n(seq, j));
			GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value);
			points[k++] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gsvalue));
		}
	}
	Datum multipoint = lwpointarr_make_trajectory((LWGEOM **)points, ts->totalcount, false);
	Datum transf = call_function2(transform, multipoint, srid);
	GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
	LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			Datum point = PointerGetDatum(geometry_serialize((LWGEOM *) (lwmpoint->geoms[k])));
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			instants[j] = temporalinst_make(point, inst->t, inst->valuetypid);
			pfree(DatumGetPointer(point));
		}
		sequences[i] = temporalseq_make(instants, seq->count,
			seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_make(sequences, ts->count, false);
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	for (int i = 0; i < ts->totalcount; i++)
		lwpoint_free(points[i]);
	pfree(points);
	pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
	POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
	lwmpoint_free(lwmpoint);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_transform);
/**
 * Transform a temporal point into another spatial reference system 
 */
PGDLLEXPORT Datum
tpoint_transform(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum srid = PG_GETARG_DATUM(1);

	Temporal *result;
	ensure_valid_duration(temp->duration);
	ensure_point_base_type(temp->valuetypid);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *) tpointinst_transform((TemporalInst *)temp, srid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *) tpointi_transform((TemporalI *)temp, srid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *) tpointseq_transform((TemporalSeq *)temp, srid);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *) tpoints_transform((TemporalS *)temp, srid);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tgeompoint_to_tgeogpoint);
/**
 * Transform the geometry to a geography 
 */
PGDLLEXPORT Datum
tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &geom_to_geog,
		type_oid(T_GEOGRAPHY));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgeogpoint_to_tgeompoint);
/**
 * Transform the geography to a geometry
 */
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
/**
 * Set the precision of the coordinates of the temporal point to the number
 * of decimal places
 */
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
 * Length functions
 *****************************************************************************/

/**
 * Returns the length traversed by the temporal sequence point 
 */
static double
tpointseq_length(const TemporalSeq *seq)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
	Datum traj = tpointseq_trajectory(seq);
	GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
	if (gserialized_get_type(gstraj) == POINTTYPE)
		return 0;
	
	/* We are sure that the trajectory is a line */
	double result;
	ensure_point_base_type(seq->valuetypid);
	if (seq->valuetypid == type_oid(T_GEOMETRY))
		/* The next function call works for 2D and 3D */
		result = DatumGetFloat8(call_function1(LWGEOM_length_linestring, traj));
	else
		result = DatumGetFloat8(call_function2(geography_length, traj,
			BoolGetDatum(true)));
	return result;
}

/**
 * Returns the length traversed by the temporal sequence set point 
 */
static double
tpoints_length(const TemporalS *ts)
{
	assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tpointseq_length(temporals_seq_n(ts, i));
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_length);
/**
 * Returns the length traversed by the temporal sequence (set) point 
 */
PGDLLEXPORT Datum
tpoint_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result = 0.0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI ||
		! MOBDB_FLAGS_GET_LINEAR(temp->flags))
		;
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_length((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tpoints_length((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

/**
 * Returns the cumulative length traversed by the temporal instant point 
 */
static TemporalInst *
tpointinst_cumulative_length(const TemporalInst *inst)
{
	return temporalinst_make(Float8GetDatum(0.0), inst->t, FLOAT8OID);
}

/**
 * Returns the cumulative length traversed by the temporal instant set point 
 */
static TemporalI *
tpointi_cumulative_length(const TemporalI *ti)
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

/**
 * Returns the cumulative length traversed by the temporal sequence point 
 */
static TemporalSeq *
tpointseq_cumulative_length(const TemporalSeq *seq, double prevlength)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_make(Float8GetDatum(0), inst->t,
			FLOAT8OID);
		TemporalSeq *result = temporalseq_make(&inst1, 1, true, true,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
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
			func = MOBDB_FLAGS_GET_Z(seq->flags) ? &pt_distance3d :
				&pt_distance2d;
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
	TemporalSeq *result = temporalseq_make(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		
	for (int i = 1; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

/**
 * Returns the cumulative length traversed by the temporal sequence set point 
 */
static TemporalS *
tpoints_cumulative_length(const TemporalS *ts)
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
/**
 * Returns the cumulative length traversed by the temporal point 
 */
PGDLLEXPORT Datum
tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_cumulative_length((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_cumulative_length((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_cumulative_length((TemporalSeq *)temp, 0);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_cumulative_length((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * Returns the speed of the temporal point in the segment
 *
 * @param[in] inst1, inst2 Instants defining the segment
 * @param[in] func Distance function (2D, 3D, or geodetic)
 */
static double
tpointinst_speed(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum (*func)(Datum, Datum))
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	return datum_point_eq(value1, value2) ? 0 :
		DatumGetFloat8(func(value1, value2)) / ((double)(inst2->t - inst1->t) / 1000000);
}

/**
 * Returns the speed of the temporal point in the temporal sequence point
 */
static TemporalSeq *
tpointseq_speed(const TemporalSeq *seq)
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
			func = MOBDB_FLAGS_GET_Z(seq->flags) ? &pt_distance3d :
				&pt_distance2d;
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
	/* The resulting sequence has step interpolation */
	TemporalSeq *result = temporalseq_make(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc, false, true);
	for (int i = 0; i < seq->count - 1; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/**
 * Returns the speed of the temporal point in the temporal sequence set point
 */
static TemporalS *
tpoints_speed(const TemporalS *ts)
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
	/* The resulting sequence set has step interpolation */
	TemporalS *result = temporals_make(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_speed);
/**
 * Returns the speed of the temporal point in the temporal sequence (set) point
 */
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
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_speed((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * instant set duration
 */
Datum
tgeompointi_twcentroid(const TemporalI *ti)
{
	int srid = tpointi_srid(ti);
	TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsz = NULL; /* keep compiler quiet */
	bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
	if (hasz)
		instantsz = palloc(sizeof(TemporalInst *) * ti->count);
		
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		POINT4D point = datum_get_point4d(temporalinst_value(inst));
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x), inst->t,
			FLOAT8OID);
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y), inst->t,
			FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z), inst->t,
				FLOAT8OID);
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

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * sequence duration
 */
Datum
tgeompointseq_twcentroid(const TemporalSeq *seq)
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
		POINT4D point = datum_get_point4d(temporalinst_value(inst));
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x), inst->t,
			FLOAT8OID);
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y), inst->t,
			FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z), inst->t,
				FLOAT8OID);
	}
	TemporalSeq *seqx = temporalseq_make(instantsx, seq->count,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	TemporalSeq *seqy = temporalseq_make(instantsy, seq->count,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	TemporalSeq *seqz;
	if (hasz)
		seqz = temporalseq_make(instantsz, seq->count, seq->period.lower_inc,
			seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	double twavgx = tnumberseq_twavg(seqx);
	double twavgy = tnumberseq_twavg(seqy);
	LWPOINT *lwpoint;
	if (hasz)
	{
		double twavgz = tnumberseq_twavg(seqz);
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

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * sequence set duration
 */
Datum
tgeompoints_twcentroid(const TemporalS *ts)
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
			POINT4D point = datum_get_point4d(temporalinst_value(inst));
			instantsx[j] = temporalinst_make(Float8GetDatum(point.x),
				inst->t, FLOAT8OID);
			instantsy[j] = temporalinst_make(Float8GetDatum(point.y),
				inst->t, FLOAT8OID);
			if (hasz)
				instantsz[j] = temporalinst_make(Float8GetDatum(point.z),
					inst->t, FLOAT8OID);
		}
		sequencesx[i] = temporalseq_make(instantsx, seq->count,
			seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		sequencesy[i] = temporalseq_make(instantsy,
			seq->count, seq->period.lower_inc, seq->period.upper_inc,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		if (hasz)
			sequencesz[i] = temporalseq_make(instantsz, seq->count,
				seq->period.lower_inc, seq->period.upper_inc,
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
	LWPOINT *lwpoint;
	if (hasz)
	{
		double twavgz = tnumbers_twavg(tsz);
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

/**
 * Returns the time-weighed centroid of the temporal geometry point
 * (dispatch function)
 */
Datum
tgeompoint_twcentroid_internal(Temporal *temp)
{
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tgeompointi_twcentroid((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tgeompointseq_twcentroid((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tgeompoints_twcentroid((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_twcentroid);
/**
 * Returns the time-weighed centroid of the temporal geometry point
 */
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

/**
 * Returns the azimuth of the two geometry points 
 */
static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
	const POINT2D *p1 = datum_get_point2d_p(geom1);
	const POINT2D *p2 = datum_get_point2d_p(geom2);
	double result;
	azimuth_pt_pt(p1, p2, &result);
	return Float8GetDatum(result);
}

/**
 * Returns the azimuth the two geography points 
 */
static Datum
geog_azimuth(Datum geom1, Datum geom2)
{
	return call_function2(geography_azimuth, geom1, geom2);
}

/**
 * Returns the temporal azimuth of the temporal geometry point of 
 * sequence duration
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 */
static int
tpointseq_azimuth1(TemporalSeq **result, const TemporalSeq *seq)
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
				/* Resulting sequence has step interpolation */
				result[l++] = temporalseq_make(instants, k, lower_inc,
					upper_inc, false, true);
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
		/* Resulting sequence has step interpolation */
		result[l++] = temporalseq_make(instants, k, lower_inc, upper_inc,
			false, true);
	}

	pfree(instants);

	return l;
}

/**
 * Returns the temporal azimuth of the temporal geometry point
 * of sequence duration
 */
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
	
	/* Resulting sequence set has step interpolation */
	TemporalS *result = temporals_make(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/**
 * Returns the temporal azimuth of the temporal geometry point
 * of sequence set duration
 */
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

	/* Resulting sequence set has step interpolation */
	TemporalS *result = temporals_make(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_azimuth);
/**
 * Returns the temporal azimuth of the temporal geometry point
 */
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
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_azimuth((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the current version of PostGIS (2.5) there is no true 
 * ST_Intersection function for geography
 *****************************************************************************/

/**
 * Restricts the temporal instant point to the geometry 
 */
static TemporalInst *
tpointinst_at_geometry(const TemporalInst *inst, Datum geom)
{
	if (!DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * Restricts the temporal instant set point to the geometry 
 */
static TemporalI *
tpointi_at_geometry(const TemporalI *ti, Datum geom)
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

/**
 * Restricts the segment of a temporal sequence point to the geometry 
 *
 * @param[in] inst1,inst2 Instants defining the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] lower_inc,upper_inc State whether the bounds are inclusive
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The instants have the same SRID and the points and the geometry 
 * are in 2D
 */
static TemporalSeq **
tpointseq_at_geometry1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool linear, bool lower_inc, bool upper_inc, Datum geom, int *count)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);

	/* Constant segment or step interpolation */
	bool equal = datum_point_eq(value1, value2);
	if (equal || ! linear)
	{
		if (!DatumGetBool(call_function2(intersects, value1, geom)))
		{
			*count = 0;
			return NULL;
		}

		TemporalInst *instants[2];
		instants[0] = (TemporalInst *) inst1;
		instants[1] = equal ? (TemporalInst *) inst2 :
			temporalinst_make(value1, inst2->t, inst1->valuetypid);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			linear, false);
		*count = 1;
		if (! equal)
			pfree(instants[1]);
		return result;
	}

	/* Look for intersections in linear segment */
	Datum line = geopoint_trajectory(value1, value2, false);
	Datum inter = call_function2(intersection, line, geom);
	GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
	if (gserialized_is_empty(gsinter))
	{
		pfree(DatumGetPointer(line));
		pfree(DatumGetPointer(inter));
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
		*count = 0;
		return NULL;
	}

	const POINT2D *start = datum_get_point2d_p(value1);
	const POINT2D *end = datum_get_point2d_p(value2);
	LWGEOM *lwgeom_inter = lwgeom_from_gserialized(gsinter);
	int type = lwgeom_inter->type;
	int countinter;
	LWPOINT *lwpoint_inter;
	LWLINE *lwline_inter;
	LWCOLLECTION *coll;
	if (type == POINTTYPE)
	{
		countinter = 1;
		lwpoint_inter = lwgeom_as_lwpoint(lwgeom_inter);

	}
	else if (type == LINETYPE)
	{
		countinter = 1;
		lwline_inter = lwgeom_as_lwline(lwgeom_inter);
	}
	else
	{
		coll = lwgeom_as_lwcollection(lwgeom_inter);
		countinter = coll->ngeoms;
	}
	TemporalInst *instants[2];
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countinter);
	double duration = (inst2->t - inst1->t);
	int k = 0;
	for (int i = 0; i < countinter; i++)
	{
		if (countinter > 1)
		{
			/* Find the i-th intersection */
			LWGEOM *subgeom = coll->geoms[i];
			if (subgeom->type == POINTTYPE)
				lwpoint_inter = lwgeom_as_lwpoint(subgeom);
			else /* type == LINETYPE */
				lwline_inter = lwgeom_as_lwline(subgeom);
			type = 	subgeom->type;
		}
		POINT2D p1, p2, closest;
		double fraction1;
		TimestampTz t1;
		Datum point1;
		/* Each intersection is either a point or a linestring with two points */
		if (type == POINTTYPE)
		{
			lwpoint_getPoint2d_p(lwpoint_inter, &p1);
			fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
			t1 = inst1->t + (long) (duration * fraction1);
			/* If the intersection is not at an exclusive bound */
			if ((lower_inc || t1 > inst1->t) && (upper_inc || t1 < inst2->t))
			{
				point1 = temporalseq_value_at_timestamp1(inst1, inst2, true, t1);
				instants[0] = temporalinst_make(point1, t1, inst1->valuetypid);
				result[k++] = temporalseq_make(instants, 1, true, true,
					linear, false);
				pfree(DatumGetPointer(point1));
				pfree(instants[0]);
			}
		}
		else
		{
			LWPOINT *lwpoint1 = lwline_get_lwpoint(lwline_inter, 0);
			LWPOINT *lwpoint2 = lwline_get_lwpoint(lwline_inter, 1);
			lwpoint_getPoint2d_p(lwpoint1, &p1);
			lwpoint_getPoint2d_p(lwpoint2, &p2);
			fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
			double fraction2 = closest_point2d_on_segment_ratio(&p2, start, end, &closest);
			t1 = inst1->t + (long) (duration * fraction1);
			TimestampTz t2 = inst1->t + (long) (duration * fraction2);
			TimestampTz lower1 = Min(t1, t2);
			TimestampTz upper1 = Max(t1, t2);
			point1 = temporalseq_value_at_timestamp1(inst1, inst2, true, lower1);
			Datum point2 = temporalseq_value_at_timestamp1(inst1, inst2, true, upper1);
			instants[0] = temporalinst_make(point1, lower1, inst1->valuetypid);
			instants[1] = temporalinst_make(point2, upper1, inst1->valuetypid);
			bool lower_inc1 = (lower1 == inst1->t) ? lower_inc : true;
			bool upper_inc1 = (upper1 == inst2->t) ? upper_inc : true;
			result[k++] = temporalseq_make(instants, 2, lower_inc1, upper_inc1,
				linear, false);
			pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
			pfree(instants[0]); pfree(instants[1]);
		}
	}

	pfree(DatumGetPointer(line));
	pfree(DatumGetPointer(inter));
	POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
	lwgeom_free(lwgeom_inter);

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

/**
 * Restricts the temporal sequence point to the geometry 
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 */
TemporalSeq **
tpointseq_at_geometry2(const TemporalSeq *seq, Datum geom, int *count)
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

/**
 * Restricts the temporal sequence point to the geometry 
 */
static TemporalS *
tpointseq_at_geometry(const TemporalSeq *seq, Datum geom)
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

/**
 * Restricts the temporal sequence set point to the geometry 
 *
 * @param[in] ts Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the temporal point
 */
static TemporalS *
tpoints_at_geometry(const TemporalS *ts, Datum geom, const STBOX *box)
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
		if (overlaps_stbox_stbox_internal(box1, box))
		{
			sequences[i] = tpointseq_at_geometry2(seq, geom,
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

/**
 * Restricts the temporal point to the geometry 
 *
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty 
 */
Temporal *
tpoint_at_geometry_internal(const Temporal *temp, Datum geom)
{
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	/* Non-empty geometries have a bounding box */
	assert(geo_to_stbox_internal(&box2, (GSERIALIZED *) DatumGetPointer(geom)));
	if (!overlaps_stbox_stbox_internal(&box1, &box2))
		return NULL;

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_at_geometry((TemporalInst *)temp, geom);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_at_geometry((TemporalI *)temp, geom);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_at_geometry((TemporalSeq *)temp, geom);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_at_geometry((TemporalS *)temp, geom, &box2);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_at_geometry);
/**
 * Restricts the temporal point to the geometry 
 */
PGDLLEXPORT Datum
tpoint_at_geometry(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = tpoint_at_geometry_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Restrict the temporal point to the spatiotemporal box 
 *
 * @pre The arguments are of the same dimensionality and
 * have the same SRID 
 */
Temporal *
tpoint_at_stbox_internal(const Temporal *temp, const STBOX *box)
{
	/* Bounding box test */
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	if (!overlaps_stbox_stbox_internal(box, &box1))
		return NULL;

	/* At least one of MOBDB_FLAGS_GET_T and MOBDB_FLAGS_GET_X is true */
	Temporal *temp1;
	if (MOBDB_FLAGS_GET_T(box->flags))
	{
		Period p;
		period_set(&p, box->tmin, box->tmax, true, true);
		temp1 = temporal_at_period_internal(temp, &p);
	}
	else
		temp1 = (Temporal *) temp;

	Temporal *result;
	if (MOBDB_FLAGS_GET_X(box->flags))
	{
		Datum gbox = PointerGetDatum(stbox_to_gbox(box));
		Datum geom = MOBDB_FLAGS_GET_Z(box->flags) ?
			call_function1(BOX3D_to_LWGEOM, gbox) :
			call_function1(BOX2D_to_LWGEOM, gbox);
		Datum geom1 = call_function2(LWGEOM_set_srid, geom,
			Int32GetDatum(box->srid));
		result = tpoint_at_geometry_internal(temp1, geom1);
		pfree(DatumGetPointer(gbox)); pfree(DatumGetPointer(geom));
		pfree(DatumGetPointer(geom1));
		if (MOBDB_FLAGS_GET_T(box->flags))
			pfree(temp1);
	}
	else
		result = temp1;
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_at_stbox);
/**
 * Restrict the temporal point to the spatiotemporal box 
 */
PGDLLEXPORT Datum
tpoint_at_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	ensure_same_srid_tpoint_stbox(temp, box);
	Temporal *result = tpoint_at_stbox_internal(temp, box);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Restrict the temporal instant point to the complement of the geometry
 */
static TemporalInst *
tpointinst_minus_geometry(const TemporalInst *inst, Datum geom)
{
	if (DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * Restrict the temporal instant set point to the complement of the geometry
 */
static TemporalI *
tpointi_minus_geometry(const TemporalI *ti, Datum geom)
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

/**
 * Restrict the temporal sequence point to the complement of the geometry
 *
 * It is not possible to use a similar approach as for tpointseq_at_geometry1
 * where instead of computing the intersections we compute the difference since
 * in PostGIS the following query
 * @code
 * select st_astext(st_difference(geometry 'Linestring(0 0,3 3)',
 *     geometry 'MultiPoint((1 1),(2 2),(3 3))'))
 * @endcode
 * returns `LINESTRING(0 0,3 3)`. Therefore we compute tpointseq_at_geometry1
 * and then compute the complement of the value obtained.
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 */
static TemporalSeq **
tpointseq_minus_geometry1(const TemporalSeq *seq, Datum geom, int *count)
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

/**
 * Restrict the temporal sequence point to the complement of the geometry
 */
static TemporalS *
tpointseq_minus_geometry(const TemporalSeq *seq, Datum geom)
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

/**
 * Restrict the temporal sequence set point to the complement of the geometry
 */
static TemporalS *
tpoints_minus_geometry(const TemporalS *ts, Datum geom, STBOX *box2)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_minus_geometry(temporals_seq_n(ts, 0),
			geom);

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
			sequences[i] = tpointseq_minus_geometry1(seq, geom,
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

/**
 * Restrict the temporal point to the complement of the geometry
 * (dispatch function)
 *
 * @pre The arguments are of the same dimensionality,
 * have the same SRID, and the geometry is not empty 
 */
Temporal *
tpoint_minus_geometry_internal(const Temporal *temp, Datum geom)
{
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	/* Non-empty geometries have a bounding box */
	assert(geo_to_stbox_internal(&box2, (GSERIALIZED *) DatumGetPointer(geom)));
	if (!overlaps_stbox_stbox_internal(&box1, &box2))
		return temporal_copy(temp);

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_minus_geometry((TemporalInst *)temp, geom);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tpointi_minus_geometry((TemporalI *)temp, geom);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_minus_geometry((TemporalSeq *)temp, geom);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tpoints_minus_geometry((TemporalS *)temp, geom, &box2);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_minus_geometry);
/**
 * Restrict the temporal point to the complement of the geometry
 */
PGDLLEXPORT Datum
tpoint_minus_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result;
	if (gserialized_is_empty(gs))
		result = temporal_copy(temp);
	else
		result = tpoint_minus_geometry_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Restrict the temporal point to the complement of the spatiotemporal box
 *
 * @pre The arguments are of the same dimensionality and have the same SRID 
 */
Temporal *
tpoint_minus_stbox_internal(const Temporal *temp, const STBOX *box)
{
	/* Bounding box test */
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	if (!overlaps_stbox_stbox_internal(box, &box1))
		return temporal_copy(temp);

	PeriodSet *ps1 = temporal_get_time_internal(temp);
	Temporal *temp1 = tpoint_at_stbox_internal(temp, box);
	PeriodSet *ps2 = temporal_get_time_internal(temp1);
	PeriodSet *ps = minus_periodset_periodset_internal(ps1, ps2);
	Temporal *result = temporal_at_periodset_internal(temp, ps);
	pfree(temp1); pfree(ps1); pfree(ps2); pfree(ps);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_minus_stbox);
/**
 * Restrict the temporal point to the complement of the spatiotemporal box
 */
PGDLLEXPORT Datum
tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	ensure_same_srid_tpoint_stbox(temp, box);
	Temporal *result = tpoint_minus_stbox_internal(temp, box);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal instant set point
 * and the geometry/geography
 *
 * @param[in] ti Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TemporalInst *
NAI_tpointi_geo(const TemporalI *ti, Datum geo, Datum (*func)(Datum, Datum))
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

/* NAI between temporal sequence point with step interpolation and a
 * geometry/geography */

/**
 * Returns the new current nearest approach instant between the temporal 
 * sequence point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[in] func Distance function
 * @param[out] mininst Instant with the minimum distance
 */
static double
NAI_tpointseq_step_geo1(const TemporalSeq *seq, Datum geo, double mindist,
	Datum (*func)(Datum, Datum), TemporalInst **mininst)
{
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		double dist = DatumGetFloat8(func(temporalinst_value(inst), geo));
		if (dist < mindist)
		{
			mindist = dist;
			*mininst = inst;
		}
	}
	return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TemporalInst *
NAI_tpointseq_step_geo(const TemporalSeq *seq, Datum geo,
	Datum (*func)(Datum, Datum))
{
	TemporalInst *inst;
	NAI_tpointseq_step_geo1(seq, geo, DBL_MAX, func, &inst);
	return temporalinst_copy(inst);
}

/**
 * Returns the nearest approach instant between the temporal sequence set 
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] ts Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TemporalInst *
NAI_tpoints_step_geo(const TemporalS *ts, Datum geo, 
	Datum (*func)(Datum, Datum))
{
	TemporalInst *inst;
	double mindist = DBL_MAX;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		mindist = NAI_tpointseq_step_geo1(seq, geo, mindist, func, &inst);
	}
	assert(inst != NULL);
	return temporalinst_copy(inst);
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the segment of a temporal 
 * sequence point with linear interpolation and the geometry
 *
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] lwgeom Geometry
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo1(const TemporalInst *inst1, const TemporalInst *inst2,
	LWGEOM *lwgeom, Datum *closest, TimestampTz *t, bool *tofree)
{
	ensure_point_base_type(inst1->valuetypid);
	bool isgeometry = inst1->valuetypid == type_oid(T_GEOMETRY);
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	*tofree = false;
	double dist, fraction;

	/* Constant segment */
	if (datum_point_eq(value1, value2))
	{
		*closest = value1;
		*t = inst1->t;
		return 0.0;
	}

	/* The trajectory is a line */
	LWLINE *lwline = geopoint_trajectory_lwline(value1, value2);
	if (isgeometry)
		dist = MOBDB_FLAGS_GET_Z(inst1->flags) ?
			lw_dist3d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction) :
			lw_dist2d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
	else
		dist = lw_dist_sphere_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
	lwline_free(lwline);

	if (fabs(fraction) < EPSILON)
	{
		*closest = value1;
		*t = inst1->t;
		return 0.0;
	}
	if (fabs(fraction - 1.0) < EPSILON)
	{
		*closest = value2;
		*t = inst2->t;
		return 0.0;
	}

	double duration = (inst2->t - inst1->t);
	*t = inst1->t + (long)(duration * fraction);
	*tofree = true;
	/* We are sure that it is linear interpolation */
	*closest =  temporalseq_value_at_timestamp1(inst1, inst2, true, *t);
	return dist;
}

/**
 * Returns the nearest approach instant between the temporal sequence 
 * point with linear interpolation and the geometry
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[in] func Distance function
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo2(const TemporalSeq *seq, Datum geo, double mindist,
	Datum (*func)(Datum, Datum), Datum *closest, TimestampTz *t, bool *tofree)
{
	TemporalInst *inst1;
	double dist;
	Datum point;
	TimestampTz t1;
	bool tofree1;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		inst1 = temporalseq_inst_n(seq, 0);
		point = temporalinst_value(inst1);
		dist =  DatumGetFloat8(func(point, geo));
		if (dist < mindist)
		{
			mindist = dist;
			*closest = point;
			*t = inst1->t;
			*tofree = false;
		}
		return mindist;
	}

	GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geo);
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	inst1 = temporalseq_inst_n(seq, 0);
	*tofree = false;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		dist = NAI_tpointseq_linear_geo1(inst1, inst2, lwgeom, &point, &t1, &tofree1);
		if (dist < mindist)
		{
			if (*tofree)
				pfree(DatumGetPointer(*closest));
			mindist = dist;
			*closest = point;
			*t = t1;
			*tofree = tofree1;
		}
		if (mindist == 0.0)
			break;
		inst1 = inst2;
	}
	return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence 
 * point with linear interpolation and the geometry
 */
static TemporalInst *
NAI_tpointseq_linear_geo(const TemporalSeq *seq, Datum geo,
	Datum (*func)(Datum, Datum))
{
	Datum closest;
	TimestampTz t;
	bool tofree;
	NAI_tpointseq_linear_geo2(seq, geo, DBL_MAX, func, &closest, &t, &tofree);
	TemporalInst *result = temporalinst_make(closest, t, seq->valuetypid);
	if (tofree)
		pfree(DatumGetPointer(closest));
	return result;
}

/**
 * Returns the nearest approach instant between the temporal sequence set 
 * point with linear interpolation and the geometry
 */
static TemporalInst *
NAI_tpoints_linear_geo(const TemporalS *ts, Datum geo,
	Datum (*func)(Datum, Datum))
{
	Datum closest, point;
	TimestampTz t, t1;
	bool tofree = false, tofree1;
	double mindist = DBL_MAX;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		double dist = NAI_tpointseq_linear_geo2(seq, geo, mindist, func,
			&point, &t1, &tofree1);
		if (dist < mindist)
		{
			if (tofree)
				pfree(DatumGetPointer(closest));
			mindist = dist;
			closest = point;
			t = t1;
			tofree = tofree1;
		}
		if (mindist == 0.0)
			break;
	}
	TemporalInst *result = temporalinst_make(closest, t, ts->valuetypid);
	if (tofree)
		pfree(DatumGetPointer(closest));
	return result;
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal point and
 * the geometry (dispatch function)
 */
TemporalInst *
NAI_tpoint_geo_internal(const Temporal *temp, Datum geo)
{
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_distance2d;
	else
		func = &geog_distance;
	TemporalInst *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = NAI_tpointi_geo((TemporalI *)temp, geo, func);
	else if (temp->duration == TEMPORALSEQ)
		result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
			NAI_tpointseq_linear_geo((TemporalSeq *)temp, geo, func) :
			NAI_tpointseq_step_geo((TemporalSeq *)temp, geo, func);
	else /* temp->duration == TEMPORALS */
		result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
			NAI_tpoints_linear_geo((TemporalS *)temp, geo, func) :
			NAI_tpoints_step_geo((TemporalS *)temp, geo, func);

	return result;
}

PG_FUNCTION_INFO_V1(NAI_geo_tpoint);
/**
 * Returns the nearest approach instant between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAI_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	TemporalInst *result = NAI_tpoint_geo_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_geo);
/**
 * Returns the nearest approach instant between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAI_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	TemporalInst *result = NAI_tpoint_geo_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);
/**
 * Returns the nearest approach instant between the temporal points
 */
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
		TemporalInst *min = temporal_min_instant(dist);
		result = temporal_at_timestamp_internal(temp1, min->t);
		pfree(dist);
		if (result == NULL)
		{
			if (temp1->duration == TEMPORALSEQ)
				result = temporalseq_find_timestamp_excl((TemporalSeq *)temp1,
					min->t);
			else /* temp->duration == TEMPORALS */
				result = temporals_find_timestamp_excl((TemporalS *)temp1,
					min->t);
		}
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
/**
 * Returns the nearest approach distance between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAD_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp->flags) ? &geom_distance3d :
			&geom_distance2d;
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
/**
 * Returns the nearest approach distance between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAD_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp->flags) ? &geom_distance3d :
			&geom_distance2d;
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
/**
 * Returns the nearest approach distance between the temporal points
 */
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
/**
 * Returns the line connecting the nearest approach point between the
 * geometry and the temporal instant point
 */
PGDLLEXPORT Datum
shortestline_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);

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
/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant point and the geometry
 */
PGDLLEXPORT Datum
shortestline_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);

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

/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant points
 * @pre The temporal points are synchronized
 */
static Datum
shortestline_tpointinst_tpointinst(const TemporalInst *inst1,
	const TemporalInst *inst2)
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

/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant set points
 *
 * @param[in] ti1,ti2 Temporal points
 * @param[in] func Distance function
 * @pre The temporal points are synchronized
 */
static Datum
shortestline_tpointi_tpointi(const TemporalI *ti1, const TemporalI *ti2,
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

/**
 * Returns the line connecting the nearest approach point between the
 * temporal sequence points
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] func Distance function
 * @pre The temporal points are synchronized
 */
static Datum
shortestline_tpointseq_tpointseq(const TemporalSeq *seq1, 
	const TemporalSeq *seq2, Datum (*func)(Datum, Datum))
{
	/* Compute the distance */
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	TemporalSeq *dist = sync_tfunc2_temporalseq_temporalseq(seq1, seq2,
		func, FLOAT8OID, linear, &tpointseq_min_dist_at_timestamp);
	TemporalInst *min = temporalseq_min_instant(dist);
	/* Timestamp t may be at an exclusive bound */
	TemporalInst *inst1, *inst2;
	if (min->t == seq1->period.lower)
	{
		inst1 = temporalseq_inst_n(seq1, 0);
		inst2 = temporalseq_inst_n(seq2, 0);
	}
	else if (min->t == seq1->period.upper)
	{
		inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
		inst2 = temporalseq_inst_n(seq2, seq1->count - 1);
	}
	else
	{
		inst1 = temporalseq_at_timestamp(seq1, min->t);
		inst2 = temporalseq_at_timestamp(seq2, min->t);
	}
	Datum result = shortestline_tpointinst_tpointinst(inst1, inst2);
	pfree(dist);
	return result;
}

/**
 * Returns the line connecting the nearest approach point between the
 * temporal sequence set points
 *
 * @param[in] ts1,ts2 Temporal points
 * @param[in] func Distance function
 * @pre The temporal points are synchronized
 */
static Datum
shortestline_tpoints_tpoints(const TemporalS *ts1, const TemporalS *ts2,
	Datum (*func)(Datum, Datum))
{
	/* Compute the distance */
	bool linear = MOBDB_FLAGS_GET_LINEAR(ts1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(ts2->flags);
	TemporalS *dist = sync_tfunc2_temporals_temporals(ts1, ts2, func,
		FLOAT8OID, linear, NULL);
	TemporalInst *min = temporals_min_instant(dist);
	TemporalInst *inst1 = temporals_at_timestamp(ts1, min->t);
	TemporalInst *inst2 = temporals_at_timestamp(ts2, min->t);
	
	/* If t is at an exclusive bound */
	bool freeinst1 = (inst1 != NULL);
	TemporalSeq *seq1, *seq2;
	int loc;
	if (inst1 == NULL)
	{
		temporals_find_timestamp(ts1, min->t, &loc);
		if (loc == 0)
		{
			seq1 = temporals_seq_n(ts1, 0);
			inst1 = temporalseq_inst_n(seq1, 0);
		}
		else if (loc == ts1->count)
		{
			seq1 = temporals_seq_n(ts1, ts1->count - 1);
			inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
		}
		else
		{
			seq1 = temporals_seq_n(ts1, loc - 1);
			seq2 = temporals_seq_n(ts1, loc);
			if (temporalseq_end_timestamp(seq1) == min->t)
				inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
			else
				inst1 = temporalseq_inst_n(seq2, 0);
		}
	}
	
	/* If t is at an exclusive bound */
	bool freeinst2 = (inst2 != NULL);
	if (inst2 == NULL)
	{
		temporals_find_timestamp(ts2, min->t, &loc);
		if (loc == 0)
		{
			seq2 = temporals_seq_n(ts2, 0);
			inst2 = temporalseq_inst_n(seq2, 0);
		}
		else if (loc == ts2->count)
		{
			seq2 = temporals_seq_n(ts2, ts2->count - 1);
			inst2 = temporalseq_inst_n(seq2, seq2->count - 1);
		}
		else
		{
			seq1 = temporals_seq_n(ts2, loc - 1);
			seq2 = temporals_seq_n(ts2, loc);
			if (temporalseq_end_timestamp(seq1) == min->t)
				inst2 = temporalseq_inst_n(seq1, seq1->count - 1);
			else
				inst2 = temporalseq_inst_n(seq2, 0);
		}
	}
	
	Datum result = shortestline_tpointinst_tpointinst(inst1, inst2);
	pfree(dist);
	if (freeinst1)
		pfree(inst1);
	if (freeinst2)
		pfree(inst2);
	return result;
}

/*****************************************************************************/

/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 * @pre The temporal points are synchronized
 */
Datum
shortestline_tpoint_tpoint_internal(const Temporal *temp1, 
	const Temporal *temp2)
{
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ?
			&geom_distance3d : &geom_distance2d;
	else
		func = &geog_distance;

	Datum result;
	ensure_valid_duration(temp1->duration);
	if (temp1->duration == TEMPORALINST)
		result = shortestline_tpointinst_tpointinst((TemporalInst *)temp1,
			(TemporalInst *)temp2);
	else if (temp1->duration == TEMPORALI)
		result = shortestline_tpointi_tpointi((TemporalI *)temp1,
			(TemporalI *)temp2, func);
	else if (temp1->duration == TEMPORALSEQ)
		result = shortestline_tpointseq_tpointseq((TemporalSeq *)temp1,
			(TemporalSeq *)temp2, func);
	else /* temp1->duration == TEMPORALS */
		result = shortestline_tpoints_tpoints((TemporalS *)temp1,
			(TemporalS *)temp2, func);

	return result;
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_tpoint);
/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 */
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

	Datum result = shortestline_tpoint_tpoint_internal(sync1, sync2);
	pfree(sync1); pfree(sync2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert a temporal point into a trajectory geometry/geography where the M 
 * coordinates encode the timestamps in number of seconds since '1970-01-01'
 *****************************************************************************/

/**
 * Converts the point and the timestamp into a PostGIS geometry/geography 
 * point where the M coordinate encodes the timestamp in number of seconds
 * since '1970-01-01' 
 */
static LWPOINT *
point_to_trajpoint(GSERIALIZED *gs, TimestampTz t)
{
	int32 srid = gserialized_get_srid(gs);
	/* The internal representation of timestamps in PostgreSQL is in 
	 * microseconds since '2000-01-01'. Therefore we need to compute
	 * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
	 * which results in 946684800 */
	double epoch = ((double)t / 1e6) + 946684800;
	LWPOINT *result;
	if (FLAGS_GET_Z(gs->flags))
	{
		const POINT3DZ *point = gs_get_point3dz_p(gs);
		result = lwpoint_make4d(srid, point->x, point->y, point->z, epoch);
	}
	else
	{
		const POINT2D *point = gs_get_point2d_p(gs);
		result = lwpoint_make3dm(srid, point->x, point->y, epoch);
	}
	FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
	return result;
}

/**
 * Converts the temporal instant point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointinst_to_geo(const TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	LWPOINT *point = point_to_trajpoint(gs, inst->t);
	GSERIALIZED *result = geometry_serialize((LWGEOM *)point);
	pfree(point);
	return PointerGetDatum(result);
}

/**
 * Converts the temporal instant set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointi_to_geo(const TemporalI *ti)
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

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static LWGEOM *
tpointseq_to_geo1(const TemporalSeq *seq)
{
	LWPOINT **points = palloc(sizeof(LWGEOM *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		GSERIALIZED *gs = (GSERIALIZED *) PointerGetDatum(temporalinst_value(inst));
		points[i] = point_to_trajpoint(gs, inst->t);
	}
	LWGEOM *result;
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		result = (LWGEOM *) points[0];
		pfree(points);
	}
	else
	{
		if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
			result = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
				(uint32_t) seq->count, (LWGEOM **) points);
		else
			result = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
				points[0]->srid, NULL, (uint32_t) seq->count, (LWGEOM **) points);
		for (int i = 0; i < seq->count; i++)
			lwpoint_free(points[i]);
		pfree(points);
	}
	return result;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointseq_to_geo(const TemporalSeq *seq)
{
	LWGEOM *lwgeom = tpointseq_to_geo1(seq);
	GSERIALIZED *result = geometry_serialize(lwgeom);
	return PointerGetDatum(result);
}

/**
 * Converts the temporal sequence set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpoints_to_geo(const TemporalS *ts)
{
	/* Instantaneous sequence */
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		return tpointseq_to_geo(seq);
	}
	uint32_t colltype = 0;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		geoms[i] = tpointseq_to_geo1(seq);
		/* Output type not initialized */
		if (! colltype)
			colltype = lwtype_get_collectiontype(geoms[i]->type);
			/* Input type not compatible with output */
			/* make output type a collection */
		else if (colltype != COLLECTIONTYPE &&
			lwtype_get_collectiontype(geoms[i]->type) != colltype)
			colltype = COLLECTIONTYPE;
	}
	// TODO add the bounding box instead of ask PostGIS to compute it again
	// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
	LWGEOM *coll = (LWGEOM *) lwcollection_construct((uint8_t) colltype,
		geoms[0]->srid, NULL, (uint32_t) ts->count, geoms);
	Datum result = PointerGetDatum(geometry_serialize(coll));
	/* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
	pfree(geoms);
	return result;
}

/*****************************************************************************/

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal point
 */
static int
tpointseq_to_geo_segmentize1(LWGEOM **result, const TemporalSeq *seq)
{
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	LWPOINT *points[2];
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		result[0] = (LWGEOM *) point_to_trajpoint(gs, inst->t);
		return 1;
	}

	/* General case */
	for (int i = 0; i < seq->count - 1; i++)
	{
		points[0] = point_to_trajpoint(gs, inst->t);
		inst = temporalseq_inst_n(seq, i + 1);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		points[1] = point_to_trajpoint(gs, inst->t);
		result[i] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2, (LWGEOM **) points);
		lwpoint_free(points[0]); lwpoint_free(points[1]);
	}
	return seq->count - 1;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 */
static Datum
tpointseq_to_geo_segmentize(const TemporalSeq *seq)
{
	int count = (seq->count == 1) ? 1 : seq->count - 1;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
	tpointseq_to_geo_segmentize1(geoms, seq);
	Datum result;
	/* Instantaneous sequence */
	if (seq->count == 1)
		result = PointerGetDatum(geometry_serialize(geoms[0]));
	else
	{
		// TODO add the bounding box instead of ask PostGIS to compute it again
		// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
		LWGEOM *segcoll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
			geoms[0]->srid, NULL, (uint32_t)(seq->count - 1), geoms);
		result = PointerGetDatum(geometry_serialize(segcoll));
	}
	for (int i = 0; i < count; i++)
		lwgeom_free(geoms[i]);
	pfree(geoms);
	return result;
}

/**
 * Converts the temporal sequence set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 */
static Datum
tpoints_to_geo_segmentize(const TemporalS *ts)
{
	/* Instantaneous sequence */
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		return tpointseq_to_geo_segmentize(seq);
	}

	uint8_t colltype = 0;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tpointseq_to_geo_segmentize1(&geoms[k], seq);
		/* Output type not initialized */
		if (! colltype)
			colltype = (uint8_t) lwtype_get_collectiontype(geoms[k - 1]->type);
			/* Input type not compatible with output */
			/* make output type a collection */
		else if (colltype != COLLECTIONTYPE &&
				 lwtype_get_collectiontype(geoms[k - 1]->type) != colltype)
			colltype = COLLECTIONTYPE;
	}
	Datum result;
	// TODO add the bounding box instead of ask PostGIS to compute it again
	// GBOX *box = stbox_to_gbox(temporals_bbox_ptr(seq));
	LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
		geoms[0]->srid, NULL, (uint32_t) k, geoms);
	result = PointerGetDatum(geometry_serialize(coll));
	for (int i = 0; i < k; i++)
		lwgeom_free(geoms[i]);
	pfree(geoms);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_geo);
/**
 * Converts the temporal point into a PostGIS trajectory geometry/geography 
 * where the M coordinates encode the timestamps in number of seconds since 
 * '1970-01-01' 
 */
PGDLLEXPORT Datum
tpoint_to_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	bool segmentize = PG_NARGS() == 2 ?
		PG_GETARG_BOOL(1) : false;
	ensure_point_base_type(temp->valuetypid);
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = tpointinst_to_geo((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = tpointi_to_geo((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = segmentize ?
				 tpointseq_to_geo_segmentize((TemporalSeq *) temp) :
				 tpointseq_to_geo((TemporalSeq *) temp);
	else /* temp->duration == TEMPORALS */
		result = segmentize ?
				 tpoints_to_geo_segmentize((TemporalS *) temp) :
				 tpoints_to_geo((TemporalS *) temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the
 * timestamps in number of seconds since '1970-01-01' into a temporal point.
 *****************************************************************************/

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant point.
 */
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

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant point.
 */
static TemporalInst *
geo_to_tpointinst(GSERIALIZED *gs)
{
	/* Geometry is a POINT */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	TemporalInst *result = trajpoint_to_tpointinst((LWPOINT *)lwgeom);
	lwgeom_free(lwgeom);
	return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant set point.
 */
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

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal sequence point.
 */
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
	TemporalSeq *result = temporalseq_make(instants, npoints, true, true,
		true, true);
	for (int i = 0; i < npoints; i++)
		pfree(instants[i]);
	pfree(instants);
	lwgeom_free(lwgeom);
	return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal sequence set point.
 */
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
			sequences[i] = temporalseq_make(&inst, 1, true, true,
				true, false);
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
/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal point.
 */
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

/**
 * Returns a point with M measure from the point and the measure
 */
static LWPOINT *
point_measure_to_geo_measure(GSERIALIZED *gs, int32 srid, double measure)
{
	LWPOINT *result;
	if (FLAGS_GET_Z(gs->flags))
	{
		const POINT3DZ *point = gs_get_point3dz_p(gs);
		result = lwpoint_make4d(srid, point->x, point->y, point->z, measure);
	}
	else
	{
		const POINT2D *point = gs_get_point2d_p(gs);
		result = lwpoint_make3dm(srid, point->x, point->y, measure);
	}
	FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
	return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal instant
 * point and the temporal float. 
 *
 * @param[in] inst Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointinst_to_geo_measure(const TemporalInst *inst, const TemporalInst *measure)
{
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);
	LWPOINT *point = point_measure_to_geo_measure(gs, srid,
		DatumGetFloat8(temporalinst_value(measure)));
	GSERIALIZED *result = geometry_serialize((LWGEOM *)point);
	pfree(point);
	return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal instant set
 * point and the temporal float. 
 *
 * @param[in] ti Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointi_to_geo_measure(const TemporalI *ti, const TemporalI *measure)
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
		points[i] = (LWGEOM *) point_measure_to_geo_measure(gs, srid,
			DatumGetFloat8(temporalinst_value(m)));
	}
	GSERIALIZED *result;
	if (ti->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *mpoint = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, srid,
			NULL, (uint32_t) ti->count, points);
		result = geometry_serialize(mpoint);
		pfree(mpoint);
	}

	for (int i = 0; i < ti->count; i++)
		pfree(points[i]);
	pfree(points);
	return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. 
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static LWGEOM *
tpointseq_to_geo_measure1(const TemporalSeq *seq, const TemporalSeq *measure)
{
	LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
	/* Remove two consecutive points if they are equal */
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	TemporalInst *m = temporalseq_inst_n(measure, 0);
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);
	LWPOINT *value1 = point_measure_to_geo_measure(gs, srid,
		DatumGetFloat8(temporalinst_value(m)));
	points[0] = value1;
	int k = 1;
	for (int i = 1; i < seq->count; i++)
	{
		inst = temporalseq_inst_n(seq, i);
		m = temporalseq_inst_n(measure, i);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		LWPOINT *value2 = point_measure_to_geo_measure(gs, srid,
			DatumGetFloat8(temporalinst_value(m)));
		if (lwpoint_same(value1, value2) != LW_TRUE)
			points[k++] = value2;
		value1 = value2;
	}
	LWGEOM *result;
	if (k == 1)
	{
		result = (LWGEOM *) points[0];
		pfree(points);
	}
	else
	{
		result = MOBDB_FLAGS_GET_LINEAR(seq->flags) ?
			(LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, (uint32_t) k,
				(LWGEOM **) points) :
			(LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
				points[0]->srid, NULL, (uint32_t) k, (LWGEOM **) points);
		for (int i = 0; i < k; i++)
			lwpoint_free(points[i]);
		pfree(points);
	}
	return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. 
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointseq_to_geo_measure(const TemporalSeq *seq, const TemporalSeq *measure)
{
	LWGEOM *lwgeom = tpointseq_to_geo_measure1(seq, measure);
	GSERIALIZED *result = geometry_serialize(lwgeom);
	return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. 
 *
 * @param[in] ts Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpoints_to_geo_measure(const TemporalS *ts, const TemporalS *measure)
{
	/* Instantaneous sequence */
	if (ts->count == 1)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, 0);
		TemporalSeq *seq2 = temporals_seq_n(measure, 0);
		return tpointseq_to_geo_measure(seq1, seq2);
	}

	uint8_t colltype = 0;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalSeq *m = temporals_seq_n(measure, i);
		geoms[i] = tpointseq_to_geo_measure1(seq, m);
		/* Output type not initialized */
		if (! colltype)
			colltype = (uint8_t) lwtype_get_collectiontype(geoms[i]->type);
		/* Input type not compatible with output */
		/* make output type a collection */
		else if (colltype != COLLECTIONTYPE &&
			lwtype_get_collectiontype(geoms[i]->type) != colltype)
			colltype = COLLECTIONTYPE;
	}
	// TODO add the bounding box instead of ask PostGIS to compute it again
	// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
	LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
		geoms[0]->srid, NULL, (uint32_t) ts->count, geoms);
	Datum result = PointerGetDatum(geometry_serialize(coll));
	/* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
	pfree(geoms);
	return result;
}

/*****************************************************************************/

/**
 * Construct a geometry/geography with M measure from the temporal sequence 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static int
tpointseq_to_geo_measure_segmentize1(LWGEOM **result, const TemporalSeq *seq,
	const TemporalSeq *measure)
{
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	double m = DatumGetFloat8(temporalinst_value(temporalseq_inst_n(measure, 0)));
	LWPOINT *points[2];
	GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
	int32 srid = gserialized_get_srid(gs);

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		result[0] = (LWGEOM *) point_measure_to_geo_measure(gs, srid, m);
		return 1;
	}

	/* General case */
	for (int i = 0; i < seq->count - 1; i++)
	{
		points[0] = point_measure_to_geo_measure(gs, srid, m);
		inst = temporalseq_inst_n(seq, i + 1);
		gs = (GSERIALIZED *) DatumGetPointer(temporalinst_value_ptr(inst));
		points[1] = point_measure_to_geo_measure(gs, srid, m);
		result[i] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2, (LWGEOM **) points);
		lwpoint_free(points[0]); lwpoint_free(points[1]);
		m = DatumGetFloat8(temporalinst_value(temporalseq_inst_n(measure, i + 1)));
	}
	return seq->count - 1;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static Datum
tpointseq_to_geo_measure_segmentize(const TemporalSeq *seq, const TemporalSeq *measure)
{
	int count = (seq->count == 1) ? 1 : seq->count - 1;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
	tpointseq_to_geo_measure_segmentize1(geoms, seq, measure);
	Datum result;
	/* Instantaneous sequence */
	if (seq->count == 1)
		result = PointerGetDatum(geometry_serialize(geoms[0]));
	else
	{
		// TODO add the bounding box instead of ask PostGIS to compute it again
		// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
		LWGEOM *segcoll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
			geoms[0]->srid, NULL, (uint32_t)(seq->count - 1), geoms);
		result = PointerGetDatum(geometry_serialize(segcoll));
	}
	for (int i = 0; i < count; i++)
		lwgeom_free(geoms[i]);
	pfree(geoms);
	return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence set 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static Datum
tpoints_to_geo_measure_segmentize(const TemporalS *ts, const TemporalS *measure)
{
	/* Instantaneous sequence */
	if (ts->count == 1)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, 0);
		TemporalSeq *seq2 = temporals_seq_n(measure, 0);
		return tpointseq_to_geo_measure_segmentize(seq1, seq2);
	}

	uint8_t colltype = 0;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{

		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalSeq *m = temporals_seq_n(measure, i);
		k += tpointseq_to_geo_measure_segmentize1(&geoms[k], seq, m);
		/* Output type not initialized */
		if (! colltype)
			colltype = (uint8_t) lwtype_get_collectiontype(geoms[k - 1]->type);
			/* Input type not compatible with output */
			/* make output type a collection */
		else if (colltype != COLLECTIONTYPE &&
				 lwtype_get_collectiontype(geoms[k - 1]->type) != colltype)
			colltype = COLLECTIONTYPE;
	}
	Datum result;
	// TODO add the bounding box instead of ask PostGIS to compute it again
	// GBOX *box = stbox_to_gbox(temporals_bbox_ptr(seq));
	LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
		geoms[0]->srid, NULL, (uint32_t) k, geoms);
	result = PointerGetDatum(geometry_serialize(coll));
	for (int i = 0; i < k; i++)
		lwgeom_free(geoms[i]);
	pfree(geoms);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_geo_measure);
/**
 * Construct a geometry/geography with M measure from the temporal point and 
 * the temporal float
 */
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

	Temporal *result;
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
	else /* sync1->duration == TEMPORALS */
		result = segmentize ?
			(Temporal *) tpoints_to_geo_measure_segmentize(
					(TemporalS *) sync1, (TemporalS *) sync2) :
			(Temporal *) tpoints_to_geo_measure(
				(TemporalS *) sync1, (TemporalS *) sync2);

	pfree(sync1); pfree(sync2);
	PG_FREE_IF_COPY(tpoint, 0);
	PG_FREE_IF_COPY(measure, 1);
	PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Simple spatio-temporal Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 ***********************************************************************/

/**
 * Determine the 3D hypotenuse.
 *
 * If required, x, y, and z are swapped to make x the larger number. The
 * traditional formula of x^2+y^2+z^2 is rearranged to factor x outside the
 * sqrt. This allows computation of the hypotenuse for significantly
 * larger values, and with a higher precision than when using the naive
 * formula. In particular, this cannot overflow unless the final result
 * would be out-of-range.
 * @code
 * sqrt( x^2 + y^2 + z^2 ) = sqrt( x^2( 1 + y^2/x^2 + z^2/x^2) )
 *                         = x * sqrt( 1 + y^2/x^2 + z^2/x^2)
 *                         = x * sqrt( 1 + y/x * y/x + z/x * z/x)
 * @endcode
 */
double
hypot3d(double x, double y, double z)
{
	double yx;
	double zx;
	double temp;

	/* Handle INF and NaN properly */
	if (isinf(x) || isinf(y) || isinf(z))
		return get_float8_infinity();

	if (isnan(x) || isnan(y) || isnan(z))
		return get_float8_nan();

	/* Else, drop any minus signs */
	x = fabs(x);
	y = fabs(y);
	z = fabs(z);

	/* Swap x, y and z if needed to make x the larger one */
	if (x < y)
	{
		temp = x;
		x = y;
		y = temp;
	}
	if (x < z)
	{
		temp = x;
		x = z;
		z = temp;
	}
	/*
	 * If x is zero, the hypotenuse is computed with the 2D case.
	 * This test saves a few cycles in such cases, but more importantly
	 * it also protects against divide-by-zero errors, since now x >= y.
	 */
	if (x == 0)
		return hypot(y, z);

	/* Determine the hypotenuse */
	yx = y / x;
	zx = z / x;
	return x * sqrt(1.0 + (yx * yx) + (zx * zx));
}

/**
 * Determine the 4D hypotenuse.
 *
 * @see The function is a generalization of the 3D case in the function hypot3d
 */
double
hypot4d(double x, double y, double z, double m)
{
	double yx;
	double zx;
	double mx;
	double temp;

	/* Handle INF and NaN properly */
	if (isinf(x) || isinf(y) || isinf(z) || isinf(m))
		return get_float8_infinity();

	if (isnan(x) || isnan(y) || isnan(z) || isnan(m))
		return get_float8_nan();

	/* Else, drop any minus signs */
	x = fabs(x);
	y = fabs(y);
	z = fabs(z);
	m = fabs(m);

	/* Swap x, y, z, and m if needed to make x the larger one */
	if (x < y)
	{
		temp = x;
		x = y;
		y = temp;
	}
	if (x < z)
	{
		temp = x;
		x = z;
		z = temp;
	}
	if (x < m)
	{
		temp = x;
		x = m;
		m = temp;
	}
	/*
	 * If x is zero, the hypotenuse is computed with the 3D case.
	 * This test saves a few cycles in such cases, but more importantly
	 * it also protects against divide-by-zero errors, since now x >= y.
	 */
	if (x == 0)
		return hypot3d(y, z, m);

	/* Determine the hypotenuse */
	yx = y / x;
	zx = z / x;
	mx = m / x;
	return x * sqrt(1.0 + (yx * yx) + (zx * zx) + (mx * mx));
}

/**
 * Returns the 2D distance between the points
 */
double
dist2d_pt_pt(POINT2D *p1, POINT2D *p2)
{
	double dx = p2->x - p1->x;
	double dy = p2->y - p1->y;
	return hypot(dx, dy);
}

/**
 * Returns the 3D distance between the points
 */
double
dist3d_pt_pt(POINT3DZ *p1, POINT3DZ *p2)
{
	double dx = p2->x - p1->x;
	double dy = p2->y - p1->y;
	double dz = p2->z - p1->z;
	return hypot3d(dx, dy, dz);
}

/**
 * Returns the 4D distance between the points
 */
double
dist4d_pt_pt(POINT4D *p1, POINT4D *p2)
{
	double dx = p2->x - p1->x;
	double dy = p2->y - p1->y;
	double dz = p2->z - p1->z;
	double dm = p2->m - p1->m;
	return hypot4d(dx, dy, dz, dm);
}

/**
 * Returns the 2D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @see http://geomalgorithms.com/a02-_lines.html 
 * @note Derived from the PostGIS function lw_dist2d_pt_seg in 
 * file measures.c
 */
double
dist2d_pt_seg(POINT2D *p, POINT2D *A, POINT2D *B)
{
	POINT2D c;
	double r;
	/* If start==end, then use pt distance */
	if (A->x == B->x && A->y == B->y)
		return dist2d_pt_pt(p, A);

	r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
		( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

	if (r < 0) /* If the first vertex A is closest to the point p */
		return dist2d_pt_pt(p, A);
	if (r > 1)  /* If the second vertex B is closest to the point p */
		return dist2d_pt_pt(p, B);

	/* else if the point p is closer to some point between a and b
	then we find that point and send it to dist2d_pt_pt */
	c.x = A->x + r * (B->x - A->x);
	c.y = A->y + r * (B->y - A->y);

	return dist2d_pt_pt(p, &c);
}

/**
 * Returns the 3D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file 
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html 
 */
double
dist3d_pt_seg(POINT3DZ *p, POINT3DZ *A, POINT3DZ *B)
{
	POINT3DZ c;
	double r;
	/* If start==end, then use pt distance */
	if (A->x == B->x && A->y == B->y && A->z == B->z)
		return dist3d_pt_pt(p, A);

	r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + (p->z-A->z) * (B->z-A->z) ) /
		( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + (B->z-A->z) * (B->z-A->z) );

	if (r < 0) /* If the first vertex A is closest to the point p */
		return dist3d_pt_pt(p, A);
	if (r > 1) /* If the second vertex B is closest to the point p */
		return dist3d_pt_pt(p, B);

	/* else if the point p is closer to some point between a and b
	then we find that point and send it to dist3d_pt_pt */
	c.x = A->x + r * (B->x - A->x);
	c.y = A->y + r * (B->y - A->y);
	c.z = A->z + r * (B->z - A->z);

	return dist3d_pt_pt(p, &c);
}

/**
 * Returns the 4D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file 
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html 
 */
double
dist4d_pt_seg(POINT4D *p, POINT4D *A, POINT4D *B)
{
	POINT4D c;
	double r;
	/* If start==end, then use pt distance */
	if (A->x == B->x && A->y == B->y && A->z == B->z && A->m == B->m)
		return dist4d_pt_pt(p, A);

	r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + (p->z-A->z) * (B->z-A->z) + (p->m-A->m) * (B->m-A->m) ) /
		( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + (B->z-A->z) * (B->z-A->z) + (B->m-A->m) * (B->m-A->m) );

	if (r < 0) /* If the first vertex A is closest to the point p */
		return dist4d_pt_pt(p, A);
	if (r > 1) /* If the second vertex B is closest to the point p */
		return dist4d_pt_pt(p, B);

	/* else if the point p is closer to some point between a and b
	then we find that point and send it to dist3d_pt_pt */
	c.x = A->x + r * (B->x - A->x);
	c.y = A->y + r * (B->y - A->y);
	c.z = A->z + r * (B->z - A->z);
	c.m = A->m + r * (B->m - A->m);

	return dist4d_pt_pt(p, &c);
}

/**
 * Finds a split when simplifying the temporal sequence point using a 
 * spatio-temporal extension of the Douglas-Peucker line simplification 
 * algorithm.
 *
 * @param[in] seq Temporal sequence
 * @param[in] p1,p2 Reference points
 * @param[in] withspeed True when the delta in the speed must be considered
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 * @param[out] delta_speed Delta speed at the split
 */
static void
tpointseq_dp_findsplit(const TemporalSeq *seq, int p1, int p2, bool withspeed,
	int *split, double *dist, double *delta_speed)
{
	POINT2D p2k, p2k_tmp, p2a, p2b;
	POINT3DZ p3k, p3k_tmp, p3a, p3b;
	POINT4D p4k, p4a, p4b;
	double d;
	bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
	*split = p1;
	d = -1;
	if (p1 + 1 < p2)
	{
		double speed_seg;
		Datum (*func)(Datum, Datum);
		if (withspeed)
			func = hasz ? &pt_distance3d : &pt_distance2d;
		TemporalInst *inst1 = temporalseq_inst_n(seq, p1);
		TemporalInst *inst2 = temporalseq_inst_n(seq, p2);
		if (withspeed)
			speed_seg = tpointinst_speed(inst1, inst2, func);
		if (hasz)
		{
			p3a = datum_get_point3dz(temporalinst_value(inst1));
			p3b = datum_get_point3dz(temporalinst_value(inst2));
			if (withspeed)
			{
				p4a.x = p3a.x; p4a.y = p3a.y; p4a.z = p3a.z; p4a.m = speed_seg;
				p4b.x = p3b.x; p4b.y = p3b.y; p4b.z = p3b.z; p4b.m = speed_seg;
			}
		}
		else
		{
			p2a = datum_get_point2d(temporalinst_value(inst1));
			p2b = datum_get_point2d(temporalinst_value(inst2));
			if (withspeed)
			{
				p3a.x = p2a.x; p3a.y = p2a.y; p3a.z = speed_seg;
				p3b.x = p2b.x; p3b.y = p2b.y; p3b.z = speed_seg;
			}
		}
		for (int k = p1 + 1; k < p2; k++)
		{
			double d_tmp, speed_pt;
			inst2 = temporalseq_inst_n(seq, k);
			if (withspeed)
				speed_pt = tpointinst_speed(inst1, inst2, func);
			if (hasz)
			{
				p3k_tmp = datum_get_point3dz(temporalinst_value(inst2));
				if (withspeed)
				{
					p4k.x = p3k_tmp.x; p4k.y = p3k_tmp.y; p4k.z = p3k_tmp.z; p4k.m = speed_pt;
					d_tmp = dist4d_pt_seg(&p4k, &p4a, &p4b);
				}
				else
					d_tmp = dist3d_pt_seg(&p3k_tmp, &p3a, &p3b);
			}
			else
			{
				p2k_tmp = datum_get_point2d(temporalinst_value(inst2));
				if (withspeed)
				{
					p3k.x = p2k_tmp.x; p3k.y = p2k_tmp.y; p3k.z = speed_pt;
					d_tmp = dist3d_pt_seg(&p3k, &p3a, &p3b);
				}
				else
					d_tmp = dist2d_pt_seg(&p2k_tmp, &p2a, &p2b);
			}
			if (d_tmp > d)
			{
				/* record the maximum */
				d = d_tmp;
				if (hasz)
					p3k = p3k_tmp;
				else
					p2k = p2k_tmp;
				if (withspeed)
					*delta_speed = fabs(speed_seg - speed_pt);
				*split = k;
			}
			inst1 = inst2;
		}
		*dist = hasz ? dist3d_pt_seg(&p3k, &p3a, &p3b) :
				distance2d_pt_seg(&p2k, &p2a, &p2b);
	}
	else
		*dist = -1;
}

/***********************************************************************/

/**
 * Simplifies the temporal sequence point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 *
 * @param[in] seq Temporal point
 * @param[in] eps_dist Epsilon speed
 * @param[in] eps_speed Epsilon speed
 * @param[in] minpts Minimum number of points
 */
TemporalSeq *
tpointseq_simplify(const TemporalSeq *seq, double eps_dist, 
	double eps_speed, uint32_t minpts)
{
	static size_t stack_size = 256;
	int *stack, *outlist; /* recursion stack */
	int stack_static[stack_size];
	int outlist_static[stack_size];
	int sp = -1; /* recursion stack pointer */
	int p1, split;
	uint32_t outn = 0;
	uint32_t i;
	double dist, delta_speed;
	bool withspeed = eps_speed > 0;

	/* Do not try to simplify really short things */
	if (seq->count < 3)
		return temporalseq_copy(seq);

	/* Only heap allocate book-keeping arrays if necessary */
	if ((unsigned int) seq->count > stack_size)
	{
		stack = palloc(sizeof(int) * seq->count);
		outlist = palloc(sizeof(int) * seq->count);
	}
	else
	{
		stack = stack_static;
		outlist = outlist_static;
	}

	p1 = 0;
	stack[++sp] = seq->count - 1;
	/* Add first point to output list */
	outlist[outn++] = 0;
	do
	{
		tpointseq_dp_findsplit(seq, p1, stack[sp], withspeed, &split, &dist, &delta_speed);
		bool dosplit;
		if (withspeed)
			dosplit = (dist >= 0 &&
				(dist > eps_dist || delta_speed > eps_speed || outn + sp + 1 < minpts));
		else
			dosplit = (dist >= 0 &&
				(dist > eps_dist || outn + sp + 1 < minpts));
		if (dosplit)
			stack[++sp] = split;
		else
		{
			outlist[outn++] = stack[sp];
			p1 = stack[sp--];
		}
	}
	while (sp >= 0);

	/* Put list of retained points into order */
	qsort(outlist, outn, sizeof(int), int_cmp);
	/* Create new TemporalSeq */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * outn);
	for (i = 0; i < outn; i++)
		instants[i] = temporalseq_inst_n(seq, outlist[i]);
	TemporalSeq *result = temporalseq_make(instants, outn,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	pfree(instants);

	/* Only free if arrays are on heap */
	if (stack != stack_static)
		pfree(stack);
	if (outlist != outlist_static)
		pfree(outlist);

	return result;
}

/**
 * Simplifies the temporal sequence set point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 *
 * @param[in] ts Temporal point
 * @param[in] eps_dist Epsilon speed
 * @param[in] eps_speed Epsilon speed
 * @param[in] minpts Minimum number of points
 */
TemporalS *
tpoints_simplify(const TemporalS *ts, double eps_dist, 
	double eps_speed, uint32_t minpts)
{
	TemporalS *result;
	/* Singleton sequence set */
	if (ts->count == 1)
	{
		TemporalSeq *seq = tpointseq_simplify(temporals_seq_n(ts, 0), eps_dist, eps_speed, minpts);
		result = temporalseq_to_temporals(seq);
		pfree(seq);
		return result;
	}

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
		sequences[i] = tpointseq_simplify(temporals_seq_n(ts, i), eps_dist, eps_speed, minpts);
	result = temporals_make(sequences, ts->count, true);
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_simplify);
/**
 * Simplifies the temporal sequence (set) point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 */
Datum
tpoint_simplify(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double eps_dist = PG_GETARG_FLOAT8(1);
	double eps_speed = PG_GETARG_FLOAT8(2);

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI ||
		! MOBDB_FLAGS_GET_LINEAR(temp->flags))
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *) tpointseq_simplify((TemporalSeq *)temp,
			eps_dist, eps_speed, 2);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *) tpoints_simplify((TemporalS *)temp,
			eps_dist, eps_speed, 2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
