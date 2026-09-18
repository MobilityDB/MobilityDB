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
 * @brief Swept-edge-polygon clip primitive for trgeometry (M1 + M2)
 *
 * Internal helper exposed for testing the kernel from SQL.
 * Builds on top will wire `trgeometry_at_geom`, `trgeometry_traversed_area`,
 * and `tgeo_restrict_stbox` through this primitive once M2
 * (rotational case) lands.
 */

/**
 * Returns time intervals (in [0, 1]) during which the moving
 * segment from `(a1, b1)` at t=0 to `(a2, b2)` at t=1 — under
 * pure translation — intersects the polygon `poly`.
 *
 * Pre-condition: `b2 - a2 == b1 - a1` (pure translation;
 * non-translation input returns an error).
 *
 * @param a1, b1 endpoints of the moving segment at t = 0 (POINT geometries)
 * @param a2, b2 endpoints of the moving segment at t = 1 (POINT geometries)
 * @param poly   static target geometry — may be a POLYGON, MULTIPOLYGON,
 *               or geometry collection; non-polygon components (POINT,
 *               LINESTRING, …) are silently skipped. The body (trgeometry
 *               reference geometry) may be a polygon or multipolygon — the
 *               caller supplies one edge at a time.
 * @return floatspanset of normalised parameter intervals in [0, 1]
 */
CREATE FUNCTION _trgeometry_geom_clip_polygon(geometry, geometry, geometry,
    geometry, geometry)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Trgeometry_geom_clip_polygon'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * Time-parameter intervals during which a moving body's edge under
 * arbitrary 2D pose interpolation intersects a polygon. The body's
 * edge endpoints are given in its local frame; the world-space edge
 * at time t is obtained by applying the interpolated pose.
 *
 * Pure-translation input (pose1.theta == pose2.theta) takes the
 * translation-only path internally. 3D pose input is rejected.
 *
 * @param p_a_local, p_b_local  body-local edge endpoints (POINT); the
 *               trgeometry body may be a polygon or multipolygon — the
 *               caller supplies one edge at a time.
 * @param pose1, pose2          poses at t = 0 and t = 1 (must be 2D)
 * @param poly                  static target geometry — may be a POLYGON,
 *               MULTIPOLYGON, or geometry collection; non-polygon components
 *               are silently skipped.
 * @return floatspanset of normalised parameter intervals in [0, 1]
 */
CREATE FUNCTION _trgeometry_geom_clip_polygon_posed(geometry, geometry,
    pose, pose, geometry)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Trgeometry_geom_clip_polygon_posed'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
