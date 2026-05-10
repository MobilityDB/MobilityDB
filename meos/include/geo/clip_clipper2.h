/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
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
 *****************************************************************************/

/**
 * @file
 * @brief Clipper2-backed polygon Boolean engine for MEOS.
 *
 * Replaces the bespoke Martinez-Rueda port with vendored Clipper2 v2.0.1.
 * Public C entry point used by the @c _mdb_internal_clip_* SQL functions
 * (and ultimately by the temporal-point clipping path in
 * @c tgeo_spatialfuncs.c).
 */

#ifndef __CLIP_CLIPPER2_H__
#define __CLIP_CLIPPER2_H__

#include <postgres.h>
#include <liblwgeom.h>
#include <meos.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Boolean operation selector for #clipper2_clip_poly_poly.
 *
 * Wire-compatible with the legacy Martinez @c ClipOper enum so the
 * existing call sites in @c clip_poly_poly and the SQL wrappers do
 * not need to change when the migration completes.
 */
#define MEOS_CLIP_INTERSECTION 0
#define MEOS_CLIP_UNION        1
#define MEOS_CLIP_DIFFERENCE   2
#define MEOS_CLIP_XOR          3

/**
 * @brief Clip two polygonal geometries via Clipper2.
 *
 * Accepts @c POLYGON or @c MULTIPOLYGON for both inputs (with holes), 2D only.
 * Geography rejection, 3D rejection, SRID-mismatch checks, and the
 * empty-input short-circuits are handled by the surrounding wrapper —
 * this function assumes its inputs are already validated.
 *
 * @param subj Subject geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param clip Clipping geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param op   One of #MEOS_CLIP_INTERSECTION, #MEOS_CLIP_UNION,
 *             #MEOS_CLIP_DIFFERENCE, #MEOS_CLIP_XOR
 * @return Newly-allocated GSERIALIZED holding the result, or @c NULL when
 *         the result is empty. Caller owns the result.
 */
extern GSERIALIZED *clipper2_clip_poly_poly(const GSERIALIZED *subj,
  const GSERIALIZED *clip, int op);

/**
 * @brief Compute the time spans during which a temporal-point trajectory
 * is inside a polygon, via Clipper2's open-path clipping.
 *
 * For a temporal sequence with linear interpolation, treats the sequence as
 * an open polyline in the (x,y) plane and clips it against the polygon
 * (or multipolygon, with holes) using Clipper2's @c AddOpenSubject /
 * @c Execute(ClipType::Intersection, ..., open_solution) path. The output
 * open paths are mapped back to time spans on the original sequence by
 * locating each output vertex on its source segment and interpolating
 * the timestamp.
 *
 * Replaces the broken parity-sweep `tpointseq_linear_at_poly` previously
 * shipped on the (unmerged) `tgeo-fast-clip-rebased` branch.
 *
 * @param[in] seq        Temporal point sequence (linear interpolation,
 *                       `count >= 2`)
 * @param[in] gs         Polygon or multipolygon to clip against (2D)
 * @param[out] out_count Number of returned spans (0 if the trajectory
 *                       never enters the polygon)
 * @return Newly-palloc'd array of @c Span (length @p *out_count), or
 *         @c NULL when the result is empty. Caller owns the array.
 */
extern Span *clipper2_traj_poly_periods(const TSequence *seq,
  const GSERIALIZED *gs, int *out_count);

#ifdef __cplusplus
}
#endif

#endif /* __CLIP_CLIPPER2_H__ */
