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
 * @brief Swept-edge-polygon clip primitive (M1 + M2)
 * @details Computes time intervals during which a moving polygon edge
 * intersects a static polygon. The swept envelope is a parallelogram
 * (M1, pure translation) or a curved quadrilateral (M2, with rotation).
 *
 * Note: "trapezoid" in this codebase refers to the shape swept by a
 * tcbuffer segment (trapezoid_make). The trgeo swept quadrilateral is
 * a distinct geometric object.
 *
 * Sibling of `tpoint_geom_clip.c`, which handles the static-edge ×
 * polygon case. The two will share a unified Edge / point-in-polygon
 * infrastructure once both wire into the build.
 */

#ifndef __TRGEO_GEOM_CLIP_H__
#define __TRGEO_GEOM_CLIP_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "temporal/temporal.h"
#include "temporal/span.h"

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @brief Time intervals during which a pure-translation moving edge
 * intersects a polygon described by an edge ring `pa`.
 *
 * The moving edge sweeps a parallelogram with vertices
 * (a1, b1, b1 + delta, a1 + delta) where delta = a2 - a1 = b2 - b1.
 *
 * The trgeometry body may be a POLYGON or MULTIPOLYGON — the caller
 * supplies one ring at a time. The target @p pa must be a single closed
 * ring (POINTARRAY); callers decompose MULTIPOLYGON targets externally.
 *
 * @param[in] a1,b1 endpoints of the moving edge at t = 0
 * @param[in] a2,b2 endpoints of the moving edge at t = 1
 *     (must satisfy `a2 - a1 == b2 - b1` to within FP_TOLERANCE)
 * @param[in] pa polygon edge ring as a POINTARRAY (closed)
 * @param[out] intervals_out receives Span values (t in [0, 1])
 * @return number of intervals appended, or -1 on input error
 *     (e.g. non-translation input)
 */
extern int trgeo_geom_clip_polygon(
  const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const POINTARRAY *pa,
  Span **intervals_out);

/**
 * @brief Convenience wrapper accepting an LWPOLY (uses the exterior ring;
 * holes ignored in M1 — see notes in the design RFC).
 *
 * @param[in] a1,b1,a2,b2 quad endpoints (pure translation)
 * @param[in] poly target polygon
 * @param[out] intervals_out receives Span values
 * @return number of intervals appended, or -1 on input error
 */
extern int trgeo_geom_clip_lwpoly(
  const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const LWPOLY *poly,
  Span **intervals_out);

/**
 * @brief Convenience wrapper accepting an axis-aligned bounding box
 * (xmin, ymin, xmax, ymax). The box is treated as a 4-vertex
 * rectangular polygon. This is the natural primitive for
 * `tgeo_restrict_stbox` on moving polygon edges, replacing
 * Liang-Barsky line-vs-box clipping.
 *
 * @param[in] a1,b1,a2,b2 quad endpoints (pure translation)
 * @param[in] xmin,ymin,xmax,ymax box bounds
 * @param[out] intervals_out receives Span values (t in [0, 1])
 * @return number of intervals appended, or -1 on input error
 */
extern int trgeo_geom_clip_box(
  const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  double xmin, double ymin, double xmax, double ymax,
  Span **intervals_out);

/*****************************************************************************
 * M2 — rotational case (2D only)
 *
 * Generalises the M1 API to allow non-zero rotation between pose1 and pose2.
 * The moving body's edge endpoints in the local frame (`p_a_local`,
 * `p_b_local`) trace circular arcs in world space under the interpolated
 * pose. Pure-translation input (theta1 == theta2) delegates to the M1
 * fast path internally.
 *
 * 3D pose input (Z-flag set) is rejected with -1; M2 is 2D only, matching
 * the design constraint that the trgeometry / cbuffer surfaces use
 * 2D-only kernels (PostGIS curve arithmetic and GEOS topology operate
 * in 2D).
 *****************************************************************************/

struct Pose; /* forward declaration; full type in pose/pose.h */

/**
 * @brief Time intervals during which a moving edge under arbitrary 2D
 * pose interpolation intersects a polygon.
 *
 * @param[in] p_a_local, p_b_local edge endpoints in the moving body's
 *     local frame (constant over time)
 * @param[in] pose1, pose2 poses at t = 0 and t = 1 (2D pose only;
 *     3D rejected with -1)
 * @param[in] pa polygon edge ring as a closed POINTARRAY
 * @param[out] intervals_out receives Span values (t in [0, 1])
 * @return number of intervals appended, or -1 on input error
 */
extern int trgeo_geom_clip_polygon_posed(
  const POINT2D *p_a_local, const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const POINTARRAY *pa, Span **intervals_out);

/** @brief LWPOLY convenience wrapper around `trgeo_geom_clip_polygon_posed` */
extern int trgeo_geom_clip_lwpoly_posed(
  const POINT2D *p_a_local, const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const LWPOLY *poly, Span **intervals_out);

/** @brief Box convenience wrapper around `trgeo_geom_clip_polygon_posed` */
extern int trgeo_geom_clip_box_posed(
  const POINT2D *p_a_local, const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  double xmin, double ymin, double xmax, double ymax,
  Span **intervals_out);

/*****************************************************************************
 * LWGEOM-level wrappers — full geometry-type parity
 *
 * Accept any LWGEOM as the static target. Polygon components contribute
 * clip intervals; POINT, LINESTRING, and other non-polygon types contribute
 * zero intervals (they carry no ring for the edge kernel to test against).
 * MULTIPOLYGON and GEOMETRYCOLLECTION are handled by recursing into
 * their sub-geometries.
 *****************************************************************************/

/**
 * @brief Time intervals during which a pure-translation moving edge
 * intersects any polygon component of an arbitrary static geometry.
 *
 * @param[in] a1,b1,a2,b2 quad endpoints (pure translation)
 * @param[in] geom static target — any LWGEOM subtype; non-polygon
 *     components (POINT, LINESTRING, …) are silently skipped
 * @param[out] intervals_out receives Span values (t in [0, 1])
 * @return number of intervals appended (may be 0), or -1 on input error
 */
extern int trgeo_geom_clip_lwgeom(
  const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const LWGEOM *geom, Span **intervals_out);

/**
 * @brief Time intervals during which a moving edge under arbitrary 2D
 * pose interpolation intersects any polygon component of a static geometry.
 *
 * @param[in] p_a_local,p_b_local edge endpoints in the moving body's
 *     local frame
 * @param[in] pose1,pose2 poses at t = 0 and t = 1 (2D only)
 * @param[in] geom static target — any LWGEOM subtype
 * @param[out] intervals_out receives Span values (t in [0, 1])
 * @return number of intervals appended, or -1 on input error
 */
extern int trgeo_geom_clip_lwgeom_posed(
  const POINT2D *p_a_local, const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const LWGEOM *geom, Span **intervals_out);

#endif /* __TRGEO_GEOM_CLIP_H__ */
