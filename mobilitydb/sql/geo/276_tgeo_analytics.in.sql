/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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

/*
 * Analytic functions for temporal geometries.
 */

/*****************************************************************************/
-- Affine transforms

CREATE OR REPLACE FUNCTION affine(tgeometry,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_affine'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION affine(tgeometry,float8,float8,float8,float8,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1, $2, $3, 0, $4, $5, 0, 0, 0, 1, $6, $7, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0,  sin($2), cos($2), 0, 0, 0, 1, 0, 0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0,  sin($2),  cos($2), 0, 0, 0, 1, $3 - cos($2) * $3 + sin($2) * $4, $4 - sin($2) * $3 - cos($2) * $4, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8,geometry)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1,  cos($2), -sin($2), 0, sin($2), cos($2), 0, 0, 0, 1, @extschema@.ST_X($3) - cos($2) * @extschema@.ST_X($3) + sin($2) * @extschema@.ST_Y($3), @extschema@.ST_Y($3) - sin($2) * @extschema@.ST_X($3) - cos($2) * @extschema@.ST_Y($3), 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateZ(tgeometry,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.rotate($1, $2)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateX(tgeometry,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1, 1, 0, 0, 0, cos($2), -sin($2), 0, sin($2), cos($2), 0, 0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateY(tgeometry,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1,  cos($2), 0, sin($2),  0, 1, 0,  -sin($2), 0, cos($2), 0,  0, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeometry,float8,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1, 1, 0, 0, 0, 1, 0, 0, 0, 1, $2, $3, $4)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeometry,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.translate($1, $2, $3, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION transscale(tgeometry,float8,float8,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.affine($1, $4, 0, 0, 0, $5, 0, 0, 0, 1, $2 * $4, $3 * $5, 0)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeometry,geometry)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeometry,geometry,origin geometry)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_scale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeometry,float8,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.scale($1, @extschema@.ST_MakePoint($2, $3, $4))'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeometry,float8,float8)
RETURNS tgeometry
AS 'SELECT @extschema@.scale($1, $2, $3, 1)'
LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tgeometry, float)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minDistSimplify(tgeography, float)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minTimeDeltaSimplify(tgeometry, interval)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minTimeDeltaSimplify(tgeography, interval)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION maxDistSimplify(tgeometry, float, boolean DEFAULT TRUE)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_max_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION douglasPeuckerSimplify(tgeometry, float, boolean DEFAULT TRUE)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_dp'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION asMVTGeom(tgeo tgeometry, bounds stbox,
  -- extent int4 DEFAULT 4096, buffer int4 DEFAULT 256, clip bool DEFAULT TRUE)
-- RETURNS geom_times
-- AS 'MODULE_PATHNAME','Tgeo_AsMVTGeom'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
