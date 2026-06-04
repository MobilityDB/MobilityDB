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
 * @brief Spatial functions for temporal circular buffers
 */

/*****************************************************************************
 * SRID
 *****************************************************************************/

CREATE FUNCTION SRID(tcbuffer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tcbuffer, integer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tspatial_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(tcbuffer, integer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tspatial_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(tcbuffer, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tspatial_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Traversed area
 *****************************************************************************/

CREATE FUNCTION traversedArea(tcbuffer, bool DEFAULT true)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tcbuffer_traversed_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * AtGeometry and MinusGeometry
 *****************************************************************************/

CREATE FUNCTION atValue(tcbuffer, cbuffer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_at_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValue(tcbuffer, cbuffer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_minus_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atGeometry(tcbuffer, geometry)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_at_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tcbuffer, geometry)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_minus_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION atStbox(tcbuffer, stbox, bool DEFAULT TRUE)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_at_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusStbox(tcbuffer, stbox, bool DEFAULT TRUE)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_minus_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

