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
 * @brief Temporal spatial relationships for temporal circurlar buffers
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tContains(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(geometry, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tCovers(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(geometry, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

CREATE FUNCTION tDisjoint(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(geometry, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

CREATE FUNCTION tIntersects(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(geometry, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

CREATE FUNCTION tTouches(cbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, cbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(geometry, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, tcbuffer)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(cbuffer, tcbuffer, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tDwithin(tcbuffer, cbuffer, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
-- CREATE FUNCTION tDwithin(geometry, tcbuffer, dist float)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tdwithin_geo_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tDwithin(tcbuffer, geometry, dist float)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_geo'
  -- LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(tcbuffer, tcbuffer, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
