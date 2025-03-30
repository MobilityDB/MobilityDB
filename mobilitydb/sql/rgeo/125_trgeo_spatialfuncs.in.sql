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
 * @brief Spatial functions for temporal poses
 */

/*****************************************************************************
 * SRID
 *****************************************************************************/

CREATE FUNCTION SRID(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(trgeometry, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(trgeometry, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(trgeometry, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Tspatial_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * AtGeometry and MinusGeometry
 *****************************************************************************/

-- CREATE FUNCTION atGeometry(trgeometry, geometry)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Trgeo_at_geom'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusGeometry(trgeometry, geometry)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Trgeo_minus_geom'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION atStbox(trgeometry, stbox, bool DEFAULT TRUE)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Trgeo_at_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusStbox(trgeometry, stbox, bool DEFAULT TRUE)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Trgeo_minus_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
