/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
-- Affine transforms

CREATE OR REPLACE FUNCTION affine(tgeompoint,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_affine'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION affine(tgeompoint,float8,float8,float8,float8,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1, $2, $3, 0, $4, $5, 0, 0, 0, 1, $6, $7, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0,  sin($2), cos($2), 0, 0, 0, 1, 0, 0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float8,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0,  sin($2),  cos($2), 0, 0, 0, 1, $3 - cos($2) * $3 + sin($2) * $4, $4 - sin($2) * $3 - cos($2) * $4, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeompoint,float8,geometry)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0, sin($2), cos($2), 0, 0, 0, 1, @extschema@.ST_X($3) - cos($2) * @extschema@.ST_X($3) + sin($2) * @extschema@.ST_Y($3), @extschema@.ST_Y($3) - sin($2) * @extschema@.ST_X($3) - cos($2) * @extschema@.ST_Y($3), 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateZ(tgeompoint,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.rotate($1, $2)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateX(tgeompoint,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1, 1, 0, 0, 0, cos($2), -sin($2), 0, sin($2), cos($2), 0, 0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateY(tgeompoint,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1,  cos($2), 0, sin($2),  0, 1, 0,  -sin($2), 0, cos($2), 0,  0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeompoint,float8,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1, 1, 0, 0, 0, 1, 0, 0, 0, 1, $2, $3, $4)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeompoint,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.translate($1, $2, $3, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION transscale(tgeompoint,float8,float8,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.affine($1, $4, 0, 0, 0, $5, 0, 0, 0, 1, $2 * $4, $3 * $5, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,geometry)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,geometry,origin geometry)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,float8,float8,float8)
RETURNS tgeompoint
--AS 'SELECT affine($1, $2, 0, 0, 0, $3, 0, 0, 0, $4, 0, 0, 0)'
AS 'SELECT @extschema@.scale($1, @extschema@.ST_MakePoint($2, $3, $4))'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeompoint,float8,float8)
RETURNS tgeompoint
AS 'SELECT @extschema@.scale($1, $2, $3, 1)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tgeompoint, float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minDistSimplify(tgeogpoint, float)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minTimeDeltaSimplify(tgeompoint, interval)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minTimeDeltaSimplify(tgeogpoint, interval)
RETURNS tgeompoint
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

CREATE FUNCTION extendedKalmanFilter(tgeompoint, gate float, q float, variance float, boolean to_drop DEFAULT TRUE)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'Temporal_ext_kalman_filter'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE geom_times AS (
  geom geometry,
  times bigint[]
);

CREATE FUNCTION asMVTGeom(tpoint tgeompoint, bounds stbox,
  extent int4 DEFAULT 4096, buffer int4 DEFAULT 256, clip bool DEFAULT TRUE)
-- RETURNS tgeompoint
RETURNS geom_times
AS 'MODULE_PATHNAME','Tpoint_AsMVTGeom'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
