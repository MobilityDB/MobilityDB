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

/*****************************************************************************
 * Oriented envelope (a.k.a minimum rotated rectangle) and convex hull
 *****************************************************************************/

CREATE FUNCTION orientedEnvelope(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geom_oriented_envelope'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION convexHull(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geom_convex_hull'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Intersection matrix
 *****************************************************************************/

CREATE FUNCTION relate(geometry, geometry)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Geom_relate'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Spatial relationships
 *****************************************************************************/

CREATE FUNCTION relate(geometry, geometry, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_relate_pattern'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_contains'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_covers'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_disjoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_intersects'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_touches'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geometry, geometry, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_dwithin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geoEquals(geometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_equals'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION disjoint(geography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geog_disjoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geog_intersects'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geography, geography, float, spheroid boolean DEFAULT true)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geog_dwithin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Measures
 *****************************************************************************/

CREATE FUNCTION area(geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION area(geography, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION perimeter(geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_perimeter'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION perimeter(geography, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_perimeter'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION centroid(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_centroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION centroid(geography, spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Geo_centroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Distances
 *****************************************************************************/

CREATE FUNCTION distance(geometry, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(geography, geography, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geo_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(geometry, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_shortestline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(geography, geography, spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Geo_shortestline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION maxDistance(geometry, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Geom_max_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Accessors
 *****************************************************************************/

CREATE FUNCTION boundary(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geom_boundary'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION reverse(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_reverse'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numGeometries(geometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Geo_num_geos'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geometryN(geometry, integer)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_geo_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numPoints(geometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Geo_num_points'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Lines
 *****************************************************************************/

CREATE FUNCTION lineInterpolatePoint(geometry, float)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Line_interpolate_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lineSubstring(geometry, float, float)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Line_substring'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lineLocatePoint(geometry, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Line_locate_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Constructors
 *****************************************************************************/

CREATE FUNCTION collect(geometry[])
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_collect_garray'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION makeLine(geometry[])
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geo_makeline_garray'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Clustering
 *****************************************************************************/

CREATE FUNCTION clusterKMeans(geometry[], k integer)
  RETURNS integer[]
  AS 'MODULE_PATHNAME', 'Geo_cluster_kmeans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION clusterDBSCAN(geometry[], eps float, minpoints integer)
  RETURNS integer[]
  AS 'MODULE_PATHNAME', 'Geo_cluster_dbscan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION clusterIntersecting(geometry[])
  RETURNS geometry[]
  AS 'MODULE_PATHNAME', 'Geo_cluster_intersecting'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION clusterWithin(geometry[], distance float)
  RETURNS geometry[]
  AS 'MODULE_PATHNAME', 'Geo_cluster_within'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Simple geometries
 *****************************************************************************/

CREATE FUNCTION isSimple(geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Geom_is_simple'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Buffer
 *****************************************************************************/

CREATE FUNCTION buffer(geom geometry, float, options text DEFAULT '')
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Geom_buffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
