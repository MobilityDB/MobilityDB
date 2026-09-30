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
 * @brief Analytic functions for temporal geometries
 */

/*****************************************************************************/
-- Affine transforms

CREATE OR REPLACE FUNCTION affine(tgeometry,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_affine'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION affine(tgeometry,float8,float8,float8,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_affine_2d'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotate(tgeometry,float8,geometry)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate_geo'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateZ(tgeometry,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate_z'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateX(tgeometry,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate_x'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION rotateY(tgeometry,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_rotate_y'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeometry,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION translate(tgeometry,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_translate'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION transscale(tgeometry,float8,float8,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_transscale'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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
AS 'MODULE_PATHNAME', 'Tgeo_scale_xyz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION scale(tgeometry,float8,float8)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Tgeo_scale_xyz'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tsample(tgeometry, duration interval,
  torigin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tprecision(tgeometry, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tsample(tgeography, duration interval,
  torigin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tprecision(tgeography, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tgeometry, float)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minDistSimplify(tgeography, float)
RETURNS tgeography
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minTimeDeltaSimplify(tgeometry, interval)
RETURNS tgeometry
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minTimeDeltaSimplify(tgeography, interval)
RETURNS tgeography
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
  -- extent integer DEFAULT 4096, buffer integer DEFAULT 256, clip boolean DEFAULT TRUE)
-- RETURNS geom_times
-- AS 'MODULE_PATHNAME','Tpoint_as_mvtgeom'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
