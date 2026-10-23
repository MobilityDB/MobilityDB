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
 * @brief Clipper2-backed polygon Boolean engine for MEOS
 * @details Replaces the bespoke Martinez-Rueda port with vendored Clipper2
 * v2.0.1.
 * Public C entry point used by the intersection, geoUnion, difference and
 * symDifference SQL functions
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

#ifdef __cplusplus
}
#endif

#endif /* __CLIP_CLIPPER2_H__ */
