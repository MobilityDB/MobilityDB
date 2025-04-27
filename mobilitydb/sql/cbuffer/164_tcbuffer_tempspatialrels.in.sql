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

-- ALL the following functions are not STRICT
CREATE FUNCTION tContains(cbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, cbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(geometry, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(tcbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tContains
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tCovers(cbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, cbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(geometry, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(tcbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDisjoint(cbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, cbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(geometry, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tcbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tIntersects(cbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, cbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(geometry, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tcbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tTouches(cbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, cbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(geometry, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tcbuffer, tcbuffer, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDwithin(cbuffer, tcbuffer, dist float,
   atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tcbuffer, cbuffer, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(geometry, tcbuffer, dist float,
   atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tcbuffer, geometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_geo'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(tcbuffer, tcbuffer, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************/
