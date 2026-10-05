/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Functions on geometries answered without calling GEOS
 */

/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Oriented envelope (a.k.a minimum rotated rectangle) and convex hull
 *****************************************************************************/

PGDLLEXPORT Datum Geom_oriented_envelope(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_oriented_envelope);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return the oriented envelope of a geometry
 * @sqlfn orientedEnvelope()
 */
Datum
Geom_oriented_envelope(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geom_oriented_envelope(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geom_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_convex_hull);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return the convex hull of a geometry
 * @sqlfn convexHull()
 */
Datum
Geom_convex_hull(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geom_convex_hull(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Intersection matrix
 *****************************************************************************/

PGDLLEXPORT Datum Geom_relate(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_relate);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return the DE-9IM intersection matrix of two geometries
 * @sqlfn relate()
 * @altsqlfn geoRelate()
 */
Datum
Geom_relate(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  char *str = geom_relate(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  if (! str)
    PG_RETURN_NULL();
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Spatial relationships
 *****************************************************************************/

PGDLLEXPORT Datum Geom_contains(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_contains);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if the first geometry contains the second one
 * @sqlfn contains()
 * @altsqlfn geoContains()
 */
Datum
Geom_contains(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geom_contains(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_covers(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_covers);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if the first geometry covers the second one
 * @sqlfn covers()
 * @altsqlfn geoCovers()
 */
Datum
Geom_covers(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geom_covers(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_disjoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_disjoint);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geometries are disjoint
 * @sqlfn disjoint()
 * @altsqlfn geoDisjoint()
 */
Datum
Geom_disjoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geom_disjoint(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_intersects(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_intersects);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geometries intersect
 * @sqlfn intersects()
 * @altsqlfn geoIntersects()
 */
Datum
Geom_intersects(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geom_intersects(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_touches(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_touches);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geometries touch
 * @sqlfn touches()
 * @altsqlfn geoTouches()
 */
Datum
Geom_touches(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geom_touches(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_equals(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_equals);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geometries are the same set of points
 * @details PostGIS declares @p equals(geometry, geometry) as a deprecated
 * alias of @p ST_Equals, so the function takes the name @p geoEquals in every
 * engine, as @p geoUnion does for the keyword @p union
 * @sqlfn geoEquals()
 */
Datum
Geom_equals(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geo_equals(gs1, gs2) == 1;
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geog_intersects(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geog_intersects);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geographies intersect
 * @sqlfn intersects()
 * @altsqlfn geoIntersects()
 */
Datum
Geog_intersects(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geog_intersects(gs1, gs2, true);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geog_disjoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geog_disjoint);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geographies are disjoint
 * @sqlfn disjoint()
 * @altsqlfn geoDisjoint()
 */
Datum
Geog_disjoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool result = geog_disjoint(gs1, gs2, true);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_dwithin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_dwithin);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geometries are within a distance
 * @sqlfn dwithin()
 * @altsqlfn geoDwithin()
 */
Datum
Geom_dwithin(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool result = geom_dwithin(gs1, gs2, dist);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geog_dwithin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geog_dwithin);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if two geographies are within a distance, measured on
 * the model of the earth of the optional fourth argument
 * @sqlfn dwithin()
 * @altsqlfn geoDwithin()
 */
Datum
Geog_dwithin(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool spheroid = true;
  if (PG_NARGS() > 3)
    spheroid = PG_GETARG_BOOL(3);
  bool result = geog_dwithin(gs1, gs2, dist, spheroid);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Geom_relate_pattern(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_relate_pattern);
/**
 * @ingroup mobilitydb_geo_base_rel
 * @brief Return true if the DE-9IM intersection matrix of two geometries
 * matches a pattern
 * @sqlfn relate()
 * @altsqlfn geoRelate()
 */
Datum
Geom_relate_pattern(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  text *pattern = PG_GETARG_TEXT_P(2);
  char *patt = text_to_cstring(pattern);
  bool result = geom_relate_pattern(gs1, gs2, patt);
  pfree(patt);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  PG_FREE_IF_COPY(pattern, 2);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Measures
 *****************************************************************************/

PGDLLEXPORT Datum Geo_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_area);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the area of a geometry or a geography, the one of a
 * geography in square meters on the earth model of the optional second argument
 * @sqlfn area()
 */
Datum
Geo_area(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  bool spheroid = true;
  if (PG_NARGS() > 1)
    spheroid = PG_GETARG_BOOL(1);
  double result = geo_area(gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Geo_perimeter(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_perimeter);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the perimeter of a geometry or a geography, the one of a
 * geography in meters on the earth model of the optional second argument
 * @sqlfn perimeter()
 */
Datum
Geo_perimeter(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  bool spheroid = true;
  if (PG_NARGS() > 1)
    spheroid = PG_GETARG_BOOL(1);
  double result = geo_perimeter(gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Geo_centroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_centroid);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the centroid of a geometry or a geography, the one of a
 * geography on the earth model of the optional second argument
 * @sqlfn centroid()
 */
Datum
Geo_centroid(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  bool spheroid = true;
  if (PG_NARGS() > 1)
    spheroid = PG_GETARG_BOOL(1);
  GSERIALIZED *result = geo_centroid(gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Distances
 *****************************************************************************/

PGDLLEXPORT Datum Geo_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_distance);
/**
 * @ingroup mobilitydb_geo_base_dist
 * @brief Return the distance between two geometries or two geographies, the
 * one of geographies in meters on the earth model of the optional third
 * argument
 * @sqlfn distance()
 */
Datum
Geo_distance(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = geo_distance(gs1, gs2, spheroid);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Geo_shortestline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_shortestline);
/**
 * @ingroup mobilitydb_geo_base_dist
 * @brief Return the line connecting the closest points of two geometries or
 * two geographies, the one of geographies on the earth model of the optional
 * third argument
 * @sqlfn shortestLine()
 */
Datum
Geo_shortestline(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  GSERIALIZED *result = geo_shortestline(gs1, gs2, spheroid);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geom_max_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_max_distance);
/**
 * @ingroup mobilitydb_geo_base_dist
 * @brief Return the maximum distance between two geometries
 * @sqlfn maxDistance()
 */
Datum
Geom_max_distance(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  double result = geom_max_distance(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

PGDLLEXPORT Datum Geom_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_boundary);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the boundary of a geometry
 * @sqlfn boundary()
 */
Datum
Geom_boundary(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geom_boundary(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geo_reverse(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_reverse);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return a geometry with the order of its vertices reversed
 * @sqlfn reverse()
 * @altsqlfn geoReverse()
 */
Datum
Geo_reverse(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geo_reverse(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geo_num_geos(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_num_geos);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the number of geometries composing a geometry
 * @sqlfn numGeometries()
 */
Datum
Geo_num_geos(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int result = geo_num_geos(gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Geo_geo_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_geo_n);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the n-th geometry composing a geometry, counting from 1
 * @sqlfn geometryN()
 */
Datum
Geo_geo_n(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int n = PG_GETARG_INT32(1);
  GSERIALIZED *result = geo_geo_n(gs, n);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geo_num_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_num_points);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the number of points of a geometry
 * @sqlfn numPoints()
 */
Datum
Geo_num_points(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int result = geo_num_points(gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Lines
 *****************************************************************************/

PGDLLEXPORT Datum Line_interpolate_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Line_interpolate_point);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the point of a line at a fraction of its length
 * @sqlfn lineInterpolatePoint()
 */
Datum
Line_interpolate_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double fraction = PG_GETARG_FLOAT8(1);
  GSERIALIZED *result = line_interpolate_point(gs, fraction, false);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Line_substring(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Line_substring);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the part of a line between two fractions of its length
 * @sqlfn lineSubstring()
 */
Datum
Line_substring(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double from = PG_GETARG_FLOAT8(1);
  double to = PG_GETARG_FLOAT8(2);
  GSERIALIZED *result = line_substring(gs, from, to);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Line_locate_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Line_locate_point);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return the fraction of the length of a line at which it comes
 * closest to a point
 * @sqlfn lineLocatePoint()
 */
Datum
Line_locate_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  double result = line_locate_point(gs1, gs2);
  PG_FREE_IF_COPY(gs1, 0);
  PG_FREE_IF_COPY(gs2, 1);
  if (result < 0.0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Simple geometries
 *****************************************************************************/

PGDLLEXPORT Datum Geom_is_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_is_simple);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return true if a geometry has no anomalous point, which is a point
 * at which it crosses or touches itself
 * @sqlfn isSimple()
 */
Datum
Geom_is_simple(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  bool result = geom_is_simple(gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Buffer
 *****************************************************************************/

PGDLLEXPORT Datum Geom_buffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_buffer);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return a geometry that represents all the points whose distance from
 * a geometry is less than or equal to a distance
 * @sqlfn buffer()
 */
Datum
Geom_buffer(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double radius = PG_GETARG_FLOAT8(1);
  text *params_text = PG_GETARG_TEXT_P(2);
  char *params = text_to_cstring(params_text);
  GSERIALIZED *result = geom_buffer(gs, radius, params);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(params_text, 2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
