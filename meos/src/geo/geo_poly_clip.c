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
 * @brief Polygon Boolean clipping for temporal-point spatial functions.
 *
 * Thin trivial-cases layer over the Clipper2 adapter
 * (@c clipper2_clip_poly_poly in @c clip_clipper2.cpp). The empty-input
 * and bbox-disjoint short-circuits handled here are the same ones the
 * legacy Martinez implementation used; they let the SQL wrappers avoid
 * paying the Clipper2 setup cost on trivially-empty intersections, which
 * is the dominant case during temporal-point clipping over a long
 * trajectory restricted to a small AOI.
 */

#include "geo/geo_poly_clip.h"

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos_geo.h>        /* for geo_copy */
#include <meos_internal.h>
#include "geo/clip_clipper2.h"

/*****************************************************************************/

/* Compile-time check: the SQL ABI of ClipOper must continue to match the
 * MEOS_CLIP_* values consumed by the Clipper2 adapter. */
_Static_assert(CL_INTERSECTION == MEOS_CLIP_INTERSECTION,
  "ClipOper / MEOS_CLIP_INTERSECTION drift");
_Static_assert(CL_UNION        == MEOS_CLIP_UNION,
  "ClipOper / MEOS_CLIP_UNION drift");
_Static_assert(CL_DIFFERENCE   == MEOS_CLIP_DIFFERENCE,
  "ClipOper / MEOS_CLIP_DIFFERENCE drift");
_Static_assert(CL_XOR          == MEOS_CLIP_XOR,
  "ClipOper / MEOS_CLIP_XOR drift");

/*****************************************************************************/

/**
 * @brief Clip the two polygons using the given Boolean operation.
 *
 * @param subj  Subject geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param clip  Clipping geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param oper  Operation selector (#CL_INTERSECTION, #CL_UNION,
 *              #CL_DIFFERENCE, #CL_XOR)
 * @return Newly-allocated GSERIALIZED holding the result, or @c NULL on
 *         empty result. Caller owns the result.
 *
 * 3D rejection, geography rejection, SRID-mismatch and type-validity
 * checks are performed by the SQL wrapper layer
 * (@c mobilitydb/src/geo/tgeo_spatialfuncs.c).
 */
GSERIALIZED *
clip_poly_poly(const GSERIALIZED *subj, const GSERIALIZED *clip, ClipOper oper)
{
  /* Trivial cases: at least one input is empty. */
  bool empty_subj = gserialized_is_empty(subj);
  bool empty_clip = gserialized_is_empty(clip);
  if (empty_subj || empty_clip)
  {
    if (oper == CL_INTERSECTION)
      return NULL;
    if (oper == CL_DIFFERENCE)
      return empty_subj ? NULL : geo_copy(subj);
    /* CL_UNION || CL_XOR */
    if (empty_subj && empty_clip)
      return NULL;
    return empty_subj ? geo_copy(clip) : geo_copy(subj);
  }

  /* Trivial case: bounding boxes don't overlap. Saves the Clipper2 setup
   * cost which is the dominant overhead for trajectory-vs-AOI clipping. */
  GBOX sbbox, clbox;
  memset(&sbbox, 0, sizeof(GBOX));
  memset(&clbox, 0, sizeof(GBOX));
  if (gserialized_get_gbox_p(subj, &sbbox) &&
      gserialized_get_gbox_p(clip, &clbox) &&
      gbox_overlaps_2d(&sbbox, &clbox) == LW_FALSE)
  {
    if (oper == CL_INTERSECTION)
      return NULL;
    if (oper == CL_DIFFERENCE)
      return geo_copy(subj);
    /* CL_UNION || CL_XOR — disjoint, return whichever side. The wrapper
     * caller of clip_poly_poly is responsible for assembling the union of
     * disjoint inputs as a MULTIPOLYGON if it needs that semantics; the
     * Martinez implementation returned just one side here too. */
    return geo_copy(clip);
  }

  return clipper2_clip_poly_poly(subj, clip, (int) oper);
}

/*****************************************************************************/
