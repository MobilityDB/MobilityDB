/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Polygon Boolean clipping for temporal-point spatial functions.
 * @details Thin trivial-cases layer over the Clipper2 adapter
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
#include "geo/geo_funcs.h"   /* for geo_serialize */

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
 * @brief Return the areal region of no area
 * @details Every outcome of a polygon Boolean covering nothing answers this,
 * so what an empty result is drawn as follows from the OPERATION and never
 * from which of the trivial cases reached it. PostGIS instead clones whichever
 * operand happens to be empty, which answers `MULTIPOLYGON EMPTY` down one
 * path and `POLYGON EMPTY` down another for one question; the general path it
 * takes when neither operand is empty answers `POLYGON EMPTY`, and that is the
 * spelling kept here for all of them.
 */
static GSERIALIZED *
clip_empty_areal(int32_t srid)
{
  return geo_serialize(lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0)));
}

/**
 * @brief Clip the two polygons using the given Boolean operation.
 * @param subj  Subject geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param clip  Clipping geometry (POLYGON or MULTIPOLYGON, 2D)
 * @param oper  Operation selector (#CL_INTERSECTION, #CL_UNION,
 * #CL_DIFFERENCE, #CL_XOR)
 * @return Newly-allocated GSERIALIZED holding the result, which is an EMPTY
 * geometry where the operation covers nothing, as PostGIS answers. Caller owns
 * the result.
 *
 * 3D rejection, geography rejection, SRID-mismatch and type-validity
 * checks are performed by the SQL wrapper layer
 * (@c mobilitydb/src/geo/tgeo_spatialfuncs.c).
 */
GSERIALIZED *
clip_poly_poly(const GSERIALIZED *subj, const GSERIALIZED *clip, ClipOper oper)
{
  /* Trivial cases: at least one input is empty. What each answers is the set
   * identity for its operation -- nothing meets an empty region, nothing is
   * taken from a subject by one, and a union with one is the other side */
  int32_t srid = gserialized_get_srid(subj);
  bool empty_subj = gserialized_is_empty(subj);
  bool empty_clip = gserialized_is_empty(clip);
  if (empty_subj || empty_clip)
  {
    if (oper == CL_INTERSECTION)
      return clip_empty_areal(srid);
    if (oper == CL_DIFFERENCE)
      return empty_subj ? clip_empty_areal(srid) : geo_copy(subj);
    /* CL_UNION || CL_XOR */
    if (empty_subj && empty_clip)
      return clip_empty_areal(srid);
    return geo_copy(empty_subj ? clip : subj);
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
      return clip_empty_areal(srid);
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
