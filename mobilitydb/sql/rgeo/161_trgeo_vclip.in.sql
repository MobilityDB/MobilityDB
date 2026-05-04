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
 * @brief V-Clip distance functions for temporal rigid geometries
 */

/*****************************************************************************
 * V-clip functions
 *****************************************************************************/

CREATE FUNCTION v_clip_poly_point(geometry(Polygon), geometry(Point))
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_poly_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION v_clip_poly_poly(geometry(Polygon), geometry(Polygon))
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_poly_poly'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION v_clip_tpoly_point(trgeometry, geometry(Point), timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_tpoly_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION v_clip_tpoly_poly(trgeometry, geometry(Polygon), timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_tpoly_poly'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION v_clip_tpoly_tpoint(trgeometry, tgeompoint, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_tpoly_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION v_clip_tpoly_tpoly(trgeometry, trgeometry, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'VClip_tpoly_tpoly'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
