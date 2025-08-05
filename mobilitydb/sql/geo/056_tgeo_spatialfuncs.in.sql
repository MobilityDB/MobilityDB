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
 * @brief Spatial functions for temporal geometries/gepgraphies
 */

/*****************************************************************************/

CREATE FUNCTION SRID(tgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION SRID(tgeography)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tgeometry, integer)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION transform(tgeometry, integer)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION transformPipeline(tgeometry, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tgeography, integer)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tspatial_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION transform(tgeography, integer)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tspatial_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION transformPipeline(tgeography, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tspatial_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tgeography(tgeometry)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tgeometry_to_tgeography'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeometry(tgeography)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tgeography_to_tgeometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeometry AS tgeography) WITH FUNCTION tgeography(tgeometry);
CREATE CAST (tgeography AS tgeometry) WITH FUNCTION tgeometry(tgeography);

CREATE FUNCTION tgeompoint(tgeometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tgeo_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeometry(tgeompoint)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tpoint_to_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint(tgeography)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tgeo_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeography(tgeogpoint)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Tpoint_to_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeometry AS tgeompoint) WITH FUNCTION tgeompoint(tgeometry);
CREATE CAST (tgeompoint AS tgeometry) WITH FUNCTION tgeometry(tgeompoint);
CREATE CAST (tgeography AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeography);
CREATE CAST (tgeogpoint AS tgeography) WITH FUNCTION tgeography(tgeogpoint);

/*****************************************************************************/

CREATE FUNCTION round(tgeometry, integer DEFAULT 0)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(tgeography, integer DEFAULT 0)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(tgeometry[], integer DEFAULT 0)
  RETURNS tgeometry[]
  AS 'MODULE_PATHNAME', 'Temporalarr_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(tgeography[], integer DEFAULT 0)
  RETURNS tgeography[]
  AS 'MODULE_PATHNAME', 'Temporalarr_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION traversedArea(tgeometry, bool DEFAULT FALSE)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tgeo_traversed_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION traversedArea(tgeography, bool DEFAULT FALSE)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Tgeo_traversed_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION centroid(tgeometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tgeo_centroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION centroid(tgeography)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Tgeo_centroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION convexHull(tgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tgeo_convex_hull'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION tCentroid(tgeometry)
  -- RETURNS tgeometry
  -- AS 'MODULE_PATHNAME', 'Tgeo_tcentroid'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION atGeometry(tgeometry, geometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tgeo_at_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tgeometry, geometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tgeo_minus_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atStbox(tgeometry, stbox, borderInc bool DEFAULT TRUE)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tgeo_at_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusStbox(tgeometry, stbox, borderInc bool DEFAULT TRUE)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Tgeo_minus_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
