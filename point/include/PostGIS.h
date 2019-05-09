/*****************************************************************************
 *
 * PostGIS.h
 *	  PostGIS definitions that are needed in the extension but are not 
 *	  exported in PostGIS headers
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_POSTGIS_H__
#define __TEMPORAL_POSTGIS_H__

/**
* Macro for reading the size from the GSERIALIZED size attribute.
* Cribbed from PgSQL, top 30 bits are size. Use VARSIZE() when working
* internally with PgSQL.
*/
#define SIZE_GET(varsize) (((varsize) >> 2) & 0x3FFFFFFF)
#define SIZE_SET(varsize, size) (((varsize) & 0x00000003)|(((size) & 0x3FFFFFFF) << 2 ))

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

extern void srid_is_latlong(FunctionCallInfo fcinfo, int srid);
extern int clamp_srid(int srid);

extern Datum transform(PG_FUNCTION_ARGS);
extern Datum buffer(PG_FUNCTION_ARGS);
extern Datum centroid(PG_FUNCTION_ARGS);
extern Datum geography_centroid(PG_FUNCTION_ARGS);
extern Datum ST_GeometricMedian(PG_FUNCTION_ARGS);

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
extern Datum geomunion(PG_FUNCTION_ARGS);
extern Datum ST_Scale(PG_FUNCTION_ARGS);
extern Datum ST_Snap(PG_FUNCTION_ARGS);
extern Datum ST_UnaryUnion(PG_FUNCTION_ARGS);

extern Datum intersection(PG_FUNCTION_ARGS);
extern Datum difference(PG_FUNCTION_ARGS);
extern Datum distance(PG_FUNCTION_ARGS); /* For 2D */
extern Datum distance3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum issimple(PG_FUNCTION_ARGS);

extern Datum pgis_union_geometry_array(PG_FUNCTION_ARGS);
extern Datum linemerge(PG_FUNCTION_ARGS);

extern Datum LWGEOM_addpoint(PG_FUNCTION_ARGS);
extern Datum LWGEOM_asText(PG_FUNCTION_ARGS); /* also for geography */
extern Datum LWGEOM_asEWKT(PG_FUNCTION_ARGS); /* also for geography */
extern Datum LWGEOM_azimuth(PG_FUNCTION_ARGS);
extern Datum LWGEOM_closestpoint(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_closestpoint3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum LWGEOM_collect(PG_FUNCTION_ARGS);
extern Datum LWGEOM_collect_garray(PG_FUNCTION_ARGS);
extern Datum LWGEOM_dfullywithin(PG_FUNCTION_ARGS);
extern Datum LWGEOM_dwithin(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_dwithin3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum LWGEOM_expand(PG_FUNCTION_ARGS);
extern Datum LWGEOM_exteriorring_polygon(PG_FUNCTION_ARGS);
extern Datum LWGEOM_geometryn_collection(PG_FUNCTION_ARGS);
extern Datum LWGEOM_get_srid(PG_FUNCTION_ARGS);	/* also for geography */
extern Datum LWGEOM_set_srid(PG_FUNCTION_ARGS);	/* also for geography */
extern Datum LWGEOM_getTYPE(PG_FUNCTION_ARGS); /* also for geography */
extern Datum LWGEOM_isempty(PG_FUNCTION_ARGS);
extern Datum LWGEOM_length_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_line_locate_point(PG_FUNCTION_ARGS);
extern Datum LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS);
extern Datum LWGEOM_line_substring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_makeline(PG_FUNCTION_ARGS);
extern Datum LWGEOM_makeline_garray(PG_FUNCTION_ARGS);
extern Datum LWGEOM_makepoint(PG_FUNCTION_ARGS);
extern Datum LWGEOM_makepoint3dm(PG_FUNCTION_ARGS);
extern Datum LWGEOM_npoints(PG_FUNCTION_ARGS);
extern Datum LWGEOM_numgeometries_collection(PG_FUNCTION_ARGS);
extern Datum LWGEOM_numpoints_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_pointn_linestring(PG_FUNCTION_ARGS);
extern Datum LWGEOM_shortestline2d(PG_FUNCTION_ARGS); /* For 2D */
extern Datum LWGEOM_shortestline3d(PG_FUNCTION_ARGS); /* For 3D */
extern Datum LWGEOM_reverse(PG_FUNCTION_ARGS);

extern Datum lwgeom_lt(PG_FUNCTION_ARGS);
extern Datum lwgeom_le(PG_FUNCTION_ARGS);
extern Datum lwgeom_gt(PG_FUNCTION_ARGS);
extern Datum lwgeom_ge(PG_FUNCTION_ARGS);
extern Datum lwgeom_eq(PG_FUNCTION_ARGS);
extern Datum lwgeom_cmp(PG_FUNCTION_ARGS);
extern Datum lwgeom_hash(PG_FUNCTION_ARGS);

extern Datum geography_covers(PG_FUNCTION_ARGS);
extern Datum geography_project(PG_FUNCTION_ARGS);
extern Datum geography_length(PG_FUNCTION_ARGS);
extern Datum geography_expand(PG_FUNCTION_ARGS);
extern Datum geography_dwithin(PG_FUNCTION_ARGS);
extern Datum geography_distance(PG_FUNCTION_ARGS);
extern Datum geography_azimuth(PG_FUNCTION_ARGS);
extern Datum geography_bestsrid(PG_FUNCTION_ARGS);

extern Datum geography_eq(PG_FUNCTION_ARGS);
extern Datum geography_lt(PG_FUNCTION_ARGS);
extern Datum geography_le(PG_FUNCTION_ARGS);
extern Datum geography_gt(PG_FUNCTION_ARGS);
extern Datum geography_ge(PG_FUNCTION_ARGS);
extern Datum geography_eq(PG_FUNCTION_ARGS);
extern Datum geography_cmp(PG_FUNCTION_ARGS);

extern Datum gserialized_same_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_within_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_contains_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_overlaps_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_left_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_right_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_above_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_below_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_overleft_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_overright_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_overabove_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_overbelow_2d(PG_FUNCTION_ARGS);
extern Datum gserialized_distance_box_2d(PG_FUNCTION_ARGS);

extern Datum gserialized_analyze_nd(PG_FUNCTION_ARGS);

#define PG_GETARG_GSERIALIZED_P(varno) ((GSERIALIZED *)PG_DETOAST_DATUM(PG_GETARG_DATUM(varno)))

#endif /* __TEMPORAL_POSTGIS_H__ */
