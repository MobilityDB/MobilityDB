/*****************************************************************************
 *
 * postgis.h
 *	  PostGIS definitions that are needed in the extension but are not 
 *	  exported in PostGIS headers
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __POSTGIS_H__
#define __POSTGIS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <liblwgeom.h>

/*****************************************************************************/
// Definitions needed for developing geography_line_interpolate_point

/**
* Conversion functions
*/
#define deg2rad(d) (M_PI * (d) / 180.0)
#define rad2deg(r) (180.0 * (r) / M_PI)

/**
* Point in spherical coordinates on the world. Units of radians.
*/
typedef struct
{
	double lon;
	double lat;
} GEOGRAPHIC_POINT;

extern int spheroid_init_from_srid(FunctionCallInfo fcinfo, int srid, SPHEROID *s);
extern double ptarray_length_spheroid(const POINTARRAY *pa, const SPHEROID *s);
extern int lwline_is_empty(const LWLINE *line);
extern void geographic_point_init(double lon, double lat, GEOGRAPHIC_POINT *g);
extern double sphere_distance(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e);
extern void geog2cart(const GEOGRAPHIC_POINT *g, POINT3D *p);
extern void cart2geog(const POINT3D *p, GEOGRAPHIC_POINT *g);
void normalize(POINT3D *p);

/*****************************************************************************/

/* Definitions copied from gserialized_gist.h */

/*
 * This macro is based on PG_FREE_IF_COPY, except that it accepts two pointers.
 * See PG_FREE_IF_COPY comment in src/include/fmgr.h in postgres source code
 * for more details.
 */


#define POSTGIS_FREE_IF_COPY_P(ptrsrc, ptrori) \
	do { \
		if ((Pointer) (ptrsrc) != (Pointer) (ptrori)) \
			pfree(ptrsrc); \
	} while (0)

/* Definitions copied from measures.h */

#define DIST_MAX		-1
#define DIST_MIN		1

typedef struct
{
	double distance;	/*the distance between p1 and p2*/
	POINT2D p1;
	POINT2D p2;
	int mode;	/*the direction of looking, if thedir = -1 then we look for maxdistance and if it is 1 then we look for mindistance*/
	int twisted; /*To preserve the order of incoming points to match the first and second point in shortest and longest line*/
	double tolerance; /*the tolerance for dwithin and dfullywithin*/
} DISTPTS;

extern int lw_dist2d_comp(const LWGEOM *lw1, const LWGEOM *lw2, DISTPTS *dl);

/*  Finds the two closest points and distance between two linesegments */
extern int lw_dist2d_seg_seg(const POINT2D *A, const POINT2D *B, const POINT2D *C, const POINT2D *D, DISTPTS *dl);

/* Definitions copied from measures3d.h */

typedef struct
{
	double distance;	/*the distance between p1 and p2*/
	POINT3DZ p1;
	POINT3DZ p2;
	int mode;	/*the direction of looking, if thedir = -1 then we look for 3dmaxdistance and if it is 1 then we look for 3dmindistance*/
	int twisted; /*To preserve the order of incoming points to match the first and second point in 3dshortest and 3dlongest line*/
	double tolerance; /*the tolerance for 3ddwithin and 3ddfullywithin*/
} DISTPTS3D;

/*  Finds the two closest points and distance between two linesegments */
extern int lw_dist3d_seg_seg(POINT3DZ *s1p1, POINT3DZ *s1p2, POINT3DZ *s2p1, POINT3DZ *s2p2, DISTPTS3D *dl);

/* Definitions copied from lwgeodetic.h */

double spheroid_distance(const GEOGRAPHIC_POINT *a, const GEOGRAPHIC_POINT *b, const SPHEROID *spheroid);

/* Definitions copied from liblwgeom_internal.h */

/**
* Floating point comparators.
*/
#define FP_TOLERANCE 1e-12
#define FP_IS_ZERO(A) (fabs(A) <= FP_TOLERANCE)
#define FP_MAX(A, B) (((A) > (B)) ? (A) : (B))
#define FP_MIN(A, B) (((A) < (B)) ? (A) : (B))
#define FP_ABS(a)   ((a) <	(0) ? -(a) : (a))
#define FP_EQUALS(A, B) (fabs((A)-(B)) <= FP_TOLERANCE)
#define FP_NEQUALS(A, B) (fabs((A)-(B)) > FP_TOLERANCE)
#define FP_LT(A, B) (((A) + FP_TOLERANCE) < (B))
#define FP_LTEQ(A, B) (((A) - FP_TOLERANCE) <= (B))
#define FP_GT(A, B) (((A) - FP_TOLERANCE) > (B))
#define FP_GTEQ(A, B) (((A) + FP_TOLERANCE) >= (B))
#define FP_CONTAINS_TOP(A, X, B) (FP_LT(A, X) && FP_LTEQ(X, B))
#define FP_CONTAINS_BOTTOM(A, X, B) (FP_LTEQ(A, X) && FP_LT(X, B))
#define FP_CONTAINS_INCL(A, X, B) (FP_LTEQ(A, X) && FP_LTEQ(X, B))
#define FP_CONTAINS_EXCL(A, X, B) (FP_LT(A, X) && FP_LT(X, B))
#define FP_CONTAINS(A, X, B) FP_CONTAINS_EXCL(A, X, B)

extern int p4d_same(const POINT4D *p1, const POINT4D *p2);
extern int p3d_same(const POINT3D *p1, const POINT3D *p2);
extern int p2d_same(const POINT2D *p1, const POINT2D *p2);
extern void closest_point_on_segment(const POINT4D *R, const POINT4D *A, const POINT4D *B, POINT4D *ret);

/* PostGIS functions called by MobilityDB  */

extern void srid_is_latlong(FunctionCallInfo fcinfo, int srid);
extern int clamp_srid(int srid);
extern int getSRIDbySRS(const char* srs);
extern char *getSRSbySRID(int32_t srid, bool short_crs);
extern int lwprint_double(double d, int maxdd, char* buf, size_t bufsize);
extern char getMachineEndian(void);
extern char lwpoint_same(const LWPOINT *p1, const LWPOINT *p2);

extern Datum transform(PG_FUNCTION_ARGS);
extern Datum buffer(PG_FUNCTION_ARGS);
extern Datum centroid(PG_FUNCTION_ARGS);

extern Datum geography_from_geometry(PG_FUNCTION_ARGS);
extern Datum geometry_from_geography(PG_FUNCTION_ARGS);

extern Datum contains(PG_FUNCTION_ARGS);
extern Datum containsproperly(PG_FUNCTION_ARGS);
extern Datum covers(PG_FUNCTION_ARGS);
extern Datum coveredby(PG_FUNCTION_ARGS);
extern Datum crosses(PG_FUNCTION_ARGS);
extern Datum disjoint(PG_FUNCTION_ARGS);
extern Datum ST_Equals(PG_FUNCTION_ARGS);
extern Datum intersects(PG_FUNCTION_ARGS); /* For 2D */
extern Datum intersects3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum overlaps(PG_FUNCTION_ARGS);
extern Datum touches(PG_FUNCTION_ARGS);
extern Datum within(PG_FUNCTION_ARGS);
extern Datum relate_full(PG_FUNCTION_ARGS);
extern Datum relate_pattern(PG_FUNCTION_ARGS);

extern Datum intersection(PG_FUNCTION_ARGS);
extern Datum distance(PG_FUNCTION_ARGS); /* For 2D */
extern Datum distance3d(PG_FUNCTION_ARGS); /* For 3D */

extern Datum BOX2D_to_LWGEOM(PG_FUNCTION_ARGS);
extern Datum BOX3D_to_LWGEOM(PG_FUNCTION_ARGS);

extern Datum LWGEOM_addpoint(PG_FUNCTION_ARGS);
extern Datum LWGEOM_azimuth(PG_FUNCTION_ARGS);
extern Datum LWGEOM_closestpoint(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_collect_garray(PG_FUNCTION_ARGS);
extern Datum LWGEOM_dwithin(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_dwithin3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum LWGEOM_geometryn_collection(PG_FUNCTION_ARGS);
extern Datum LWGEOM_get_srid(PG_FUNCTION_ARGS);	/* also for geography */
extern Datum LWGEOM_set_srid(PG_FUNCTION_ARGS);
extern Datum LWGEOM_isempty(PG_FUNCTION_ARGS);
extern Datum LWGEOM_length_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_line_locate_point(PG_FUNCTION_ARGS);
extern Datum LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS);
extern Datum LWGEOM_makepoint(PG_FUNCTION_ARGS);
extern Datum LWGEOM_numgeometries_collection(PG_FUNCTION_ARGS);
extern Datum LWGEOM_numpoints_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_pointn_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_setpoint_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_shortestline2d(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_shortestline3d(PG_FUNCTION_ARGS); /* For 3D */

extern Datum lwgeom_eq(PG_FUNCTION_ARGS);
extern Datum lwgeom_lt(PG_FUNCTION_ARGS);
extern Datum lwgeom_hash(PG_FUNCTION_ARGS);

extern Datum geography_covers(PG_FUNCTION_ARGS);
extern Datum geography_length(PG_FUNCTION_ARGS);
extern Datum geography_dwithin(PG_FUNCTION_ARGS);
extern Datum geography_distance(PG_FUNCTION_ARGS);
extern Datum geography_azimuth(PG_FUNCTION_ARGS);
extern Datum geography_bestsrid(PG_FUNCTION_ARGS);

extern Datum geography_eq(PG_FUNCTION_ARGS);
extern Datum geography_lt(PG_FUNCTION_ARGS);

#define PG_GETARG_GSERIALIZED_P(varno) ((GSERIALIZED *)PG_DETOAST_DATUM(PG_GETARG_DATUM(varno)))

#endif /* __TEMPORAL_POSTGIS_H__ */
