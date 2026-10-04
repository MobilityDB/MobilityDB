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
 * @brief Analytic functions for temporal points
 */

/*****************************************************************************/
-- There are two versions of the functions since the single-argument version
-- is required for defining the casting

CREATE FUNCTION geometry(tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tpoint_to_geomeas'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geometry(tgeompoint, boolean DEFAULT FALSE)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tpoint_to_geomeas'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS geometry) WITH FUNCTION geometry(tgeompoint);

CREATE FUNCTION geography(tgeogpoint)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Tpoint_to_geomeas'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geography(tgeogpoint, boolean DEFAULT FALSE)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Tpoint_to_geomeas'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeogpoint AS geography) WITH FUNCTION geography(tgeogpoint);

CREATE FUNCTION tgeompoint(geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Geomeas_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS tgeompoint) WITH FUNCTION tgeompoint(geometry);

CREATE FUNCTION tgeogpoint(geography)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Geomeas_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geography AS tgeogpoint) WITH FUNCTION tgeogpoint(geography);

/*****************************************************************************/

CREATE FUNCTION geoMeasure(tgeompoint, tfloat, boolean DEFAULT FALSE)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Tpoint_tfloat_to_geomeas'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoMeasure(tgeogpoint, tfloat, boolean DEFAULT FALSE)
RETURNS geography
AS 'MODULE_PATHNAME', 'Tpoint_tfloat_to_geomeas'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
-- Affine transforms of a geometry

CREATE FUNCTION affine(geometry,float,float,float,float,float,float,float,float,float,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_affine'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION affine(geometry,float,float,float,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_affine_2d'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotate(geometry,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotate(geometry,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotate(geometry,float,geometry)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotateX(geometry,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate_x'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotateY(geometry,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate_y'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotateZ(geometry,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION translate(geometry,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION translate(geometry,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transscale(geometry,float,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_transscale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scale(geometry,geometry)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_scale_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scale(geometry,geometry,origin geometry)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_scale_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scale(geometry,float,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scale(geometry,float,float)
RETURNS geometry
AS 'MODULE_PATHNAME', 'Geo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
-- Affine transforms

CREATE OR REPLACE FUNCTION affine(tgeompoint,float,float,float,float,float,float,float,float,float,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_affine'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION affine(tgeompoint,float,float,float,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_affine_2d'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float,geometry)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateZ(tgeompoint,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateX(tgeompoint,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate_x'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateY(tgeompoint,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_rotate_y'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeompoint,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeompoint,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION transscale(tgeompoint,float,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_transscale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,geometry)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,geometry,origin geometry)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,float,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,float,float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tsample(tgeompoint, duration interval,
  torigin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tprecision(tgeompoint, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tsample(tgeogpoint, duration interval,
  torigin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tprecision(tgeogpoint, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tgeompoint, float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minDistSimplify(tgeogpoint, float)
RETURNS tgeogpoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minTimeDeltaSimplify(tgeompoint, interval)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minTimeDeltaSimplify(tgeogpoint, interval)
RETURNS tgeogpoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION maxDistSimplify(tgeompoint, float, boolean DEFAULT TRUE)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_max_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION douglasPeuckerSimplify(tgeompoint, float, boolean DEFAULT TRUE)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_dp'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION extendedKalmanFilter(tgeompoint, gate float, q float, variance float, to_drop boolean DEFAULT TRUE)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_ext_kalman_filter'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE geom_times AS (
  geom geometry,
  times bigint[]
);

CREATE FUNCTION asMVTGeom(tpoint tgeompoint, bounds stbox,
  extent integer DEFAULT 4096, buffer integer DEFAULT 256, clip boolean DEFAULT TRUE)
-- RETURNS tgeompoint
RETURNS geom_times
AS 'MODULE_PATHNAME','Tpoint_as_mvtgeom'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
