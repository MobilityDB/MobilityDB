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
 * @brief Clipper2-backed polygon Boolean engine
 * @details GSERIALIZED ↔ Clipper2 Paths64 round-trip with full POLYGON /
 * MULTIPOLYGON
 * (with holes) support and the four Boolean operations
 * (intersection, union, difference, xor).
 *
 * Coordinates are quantised at @c CLIP_SCALE = 1e7 (≈ 11 mm at the equator),
 * well inside Clipper2's @c MAX_COORD ≈ 2.3e18.
 *
 * Output topology is reconstructed from a @c PolyTree64: every level-1 ring
 * becomes an LWPOLY exterior, its level-2 children become inner rings of that
 * polygon, and any level-3 ring (an island enclosed by a hole) starts a new
 * LWPOLY exterior — this is how Clipper2 represents nested polygon hierarchies.
 */

/* PostgreSQL's win32_port.h defines its own `struct stat` (to get a 64-bit
 * st_size on Windows) after neutralising the UCRT one with
 *   #define stat microsoft_native_stat ... #include <sys/stat.h>
 * as its own comment says: "We must pull in sys/stat.h before this part, else
 * our overrides lose."  That only holds while nothing has already parsed
 * <sys/stat.h> unmangled.  Since mingw-w64 r302, <wchar.h> includes
 * <sys/stat.h>, and libstdc++'s <cwchar> pulls in <wchar.h> -- so the Clipper2
 * headers below would define the UCRT `struct stat` first and win32_port.h
 * would then redefine it.  Reordering cannot fix this: postgres.h first breaks
 * std::bind (see below).  Instead run PostgreSQL's own prologue here, ahead of
 * any C++ header, so the invariant it documents still holds. */
#ifdef _WIN32
#define fstat microsoft_native_fstat
#define stat microsoft_native_stat
#include <sys/stat.h>
#undef fstat
#undef stat
#endif

/* C++ standard library and Clipper2 must come first: PostgreSQL's
 * win32_port.h on MSYS2/MinGW redefines socket primitives like
 * bind/socket/select as macros, which mangles std::bind and similar
 * symbols if <functional> is parsed afterwards. */
#include "clipper2/clipper.h"

#include <cmath>
#include <cstdint>
#include <vector>

extern "C" {
#include "geo/clip_clipper2.h"
#include <postgres.h>
#include <liblwgeom.h>
#include "geo/tgeo_spatialfuncs.h"  /* for geo_serialize */
#include "meos.h"
#include "meos_internal.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
}

using Clipper2Lib::Clipper64;
using Clipper2Lib::ClipType;
using Clipper2Lib::FillRule;
using Clipper2Lib::Path64;
using Clipper2Lib::Paths64;
using Clipper2Lib::Point64;
using Clipper2Lib::PolyPath64;
using Clipper2Lib::PolyTree64;

/*****************************************************************************
 * Coordinate quantisation
 *
 * 1e7 fits roughly 11 mm of resolution at the equator. lon/lat magnitudes go
 * up to ±1.8e9 after scaling, well inside Clipper2's int64 budget for the
 * cross-product arithmetic it performs internally (MAX_COORD ≈ 2.3e18).
 *****************************************************************************/

static constexpr double CLIP_SCALE = 1e7;

/*****************************************************************************
 * GSERIALIZED → Paths64 conversion
 *****************************************************************************/

/**
 * @brief Convert one closed POINTARRAY to an open Clipper2 Path64.
 *
 * LWGEOM rings store the closing duplicate vertex; Clipper2 paths are open,
 * so the trailing duplicate is dropped here.
 */
static Path64
ring_to_path64(const POINTARRAY *pa)
{
  Path64 path;
  if (pa == nullptr || pa->npoints < 4)
    return path;  /* degenerate: a closed ring needs at least 4 points */
  path.reserve(pa->npoints - 1);
  for (uint32_t i = 0; i < pa->npoints - 1; i++)
  {
    const POINT2D *p = reinterpret_cast<const POINT2D *>(
      getPoint_internal(pa, i));
    path.emplace_back(
      static_cast<int64_t>(std::llround(p->x * CLIP_SCALE)),
      static_cast<int64_t>(std::llround(p->y * CLIP_SCALE)));
  }
  return path;
}

/**
 * @brief Append every ring (outer + holes) of an LWPOLY to a Paths64 set.
 */
static void
lwpoly_append_to_paths64(const LWPOLY *poly, Paths64 &out)
{
  if (poly == nullptr)
    return;
  for (uint32_t i = 0; i < poly->nrings; i++)
  {
    Path64 p = ring_to_path64(poly->rings[i]);
    if (!p.empty())
      out.push_back(std::move(p));
  }
}

/**
 * @brief Convert an LWGEOM (POLYGON or MULTIPOLYGON) to a flat Paths64 set.
 *
 * All rings — exteriors and holes, across every component polygon — are
 * appended into the same Paths64. With @c FillRule::EvenOdd this correctly
 * reproduces the source topology because LWGEOM holes are topological
 * (geometrically contained), not signed-area.
 */
static void
lwgeom_to_paths64(const LWGEOM *lw, Paths64 &out)
{
  if (lw == nullptr)
    return;
  if (lw->type == POLYGONTYPE)
  {
    lwpoly_append_to_paths64(reinterpret_cast<const LWPOLY *>(lw), out);
  }
  else if (lw->type == MULTIPOLYGONTYPE)
  {
    const LWMPOLY *mp = reinterpret_cast<const LWMPOLY *>(lw);
    for (uint32_t i = 0; i < mp->ngeoms; i++)
      lwpoly_append_to_paths64(mp->geoms[i], out);
  }
  /* Other types are rejected upstream; ignore here defensively. */
}

/*****************************************************************************
 * Paths64 → LWGEOM conversion
 *****************************************************************************/

/**
 * @brief Convert one Clipper2 Path64 back to a closed POINTARRAY.
 *
 * Re-introduces the closing duplicate vertex that Clipper2 omits.
 */
static POINTARRAY *
path64_to_pa(const Path64 &path)
{
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE,
    static_cast<uint32_t>(path.size() + 1));
  for (const auto &pt : path)
  {
    POINT4D pt4 = { pt.x / CLIP_SCALE, pt.y / CLIP_SCALE, 0.0, 0.0 };
    ptarray_append_point(pa, &pt4, LW_TRUE);
  }
  if (! path.empty())
  {
    const auto &first = path.front();
    POINT4D pt4 = { first.x / CLIP_SCALE, first.y / CLIP_SCALE, 0.0, 0.0 };
    ptarray_append_point(pa, &pt4, LW_TRUE);
  }
  return pa;
}

/**
 * @brief Recursively walk a PolyPath64 subtree, emitting an LWPOLY per
 *        non-hole node.
 *
 * Tree layout from Clipper2:
 *   level 0 (root)       — synthetic, no Polygon()
 *   level 1 (exterior)   — outer ring of an output polygon
 *   level 2 (hole)       — inner ring of the level-1 polygon
 *   level 3 (exterior)   — island sitting inside that hole — NEW LWPOLY
 *   level 4 (hole)       — hole inside that island
 *   ...
 *
 * @c IsHole() is true at even levels except level 0.
 *
 * @param exterior  A non-hole node (its @c Polygon() is the outer ring).
 * @param srid      SRID propagated to each emitted LWPOLY.
 * @param out       Accumulator of LWGEOM* (each is an @c LWPOLY cast).
 */
static void
emit_polygons(const PolyPath64 &exterior, int32_t srid,
  std::vector<LWGEOM *> &out)
{
  LWPOLY *poly = lwpoly_construct_empty(srid, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, path64_to_pa(exterior.Polygon()));
  for (const auto &child : exterior)
  {
    /* Direct children of an exterior are holes by Clipper2's contract. */
    lwpoly_add_ring(poly, path64_to_pa(child->Polygon()));
  }
  /* Match the orientation convention used by the legacy Martinez output:
   * outer ring CW, inner rings CCW. PostGIS ST_Equals is orientation-agnostic,
   * but downstream code that inspects winding directly relies on this. */
  LWGEOM *lw = lwpoly_as_lwgeom(poly);
  lwgeom_force_clockwise(lw);
  lwgeom_add_bbox(lw);
  out.push_back(lw);

  /* Each grandchild is a level-3 exterior — an island within a hole. */
  for (const auto &hole : exterior)
    for (const auto &grandchild : *hole)
      emit_polygons(*grandchild, srid, out);
}

/**
 * @brief Convert a Clipper2 PolyTree64 to an LWGEOM (LWPOLY or LWMPOLY).
 *
 * Returns @c nullptr if the tree has no level-1 children (empty result).
 */
static LWGEOM *
polytree_to_lwgeom(const PolyTree64 &tree, int32_t srid)
{
  std::vector<LWGEOM *> polys;
  for (const auto &top : tree)
    emit_polygons(*top, srid, polys);

  /* An overlay covering no area is an EMPTY polygon, which is an answer: the
   * arms that clip a line or a point set answer the empty geometry of their
   * own kind, and so does the GEOS overlay this one stands in front of */
  if (polys.empty())
    return lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0));
  if (polys.size() == 1)
    return polys[0];

  /* Wrap multiple polygons in an LWMPOLY. lwcollection_construct copies
   * the geoms array, so it's safe to pass a vector-backed pointer. */
  LWGEOM **arr = static_cast<LWGEOM **>(
    lwalloc(sizeof(LWGEOM *) * polys.size()));
  for (size_t i = 0; i < polys.size(); i++)
    arr[i] = polys[i];
  LWCOLLECTION *coll = lwcollection_construct(MULTIPOLYGONTYPE, srid,
    NULL, static_cast<uint32_t>(polys.size()), arr);
  LWGEOM *result = lwcollection_as_lwgeom(coll);
  lwgeom_add_bbox(result);
  return result;
}

/*****************************************************************************
 * Operation dispatch
 *****************************************************************************/

/**
 * @brief Map MEOS_CLIP_* selector to Clipper2 ClipType.
 *
 * The two enums do NOT share a numeric ordering — Clipper2 reserves 0 for
 * @c NoClip — so an explicit mapping is mandatory.
 */
static bool
map_clip_op(int op, ClipType *out)
{
  switch (op)
  {
    case MEOS_CLIP_INTERSECTION: *out = ClipType::Intersection; return true;
    case MEOS_CLIP_UNION:        *out = ClipType::Union;        return true;
    case MEOS_CLIP_DIFFERENCE:   *out = ClipType::Difference;   return true;
    case MEOS_CLIP_XOR:          *out = ClipType::Xor;          return true;
    default:                                                    return false;
  }
}

/*****************************************************************************
 * Public entry point
 *****************************************************************************/

extern "C" GSERIALIZED *
clipper2_clip_poly_poly(const GSERIALIZED *subj_g, const GSERIALIZED *clip_g,
  int op)
{
  ClipType ct;
  if (! map_clip_op(op, &ct))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "clipper2_clip_poly_poly: unknown clip operation %d", op);
    return nullptr;
  }

  LWGEOM *subj_lw = lwgeom_from_gserialized(subj_g);
  LWGEOM *clip_lw = lwgeom_from_gserialized(clip_g);
  if (subj_lw == nullptr || clip_lw == nullptr)
  {
    if (subj_lw) lwgeom_free(subj_lw);
    if (clip_lw) lwgeom_free(clip_lw);
    return nullptr;
  }

  if ((subj_lw->type != POLYGONTYPE && subj_lw->type != MULTIPOLYGONTYPE) ||
      (clip_lw->type != POLYGONTYPE && clip_lw->type != MULTIPOLYGONTYPE))
  {
    lwgeom_free(subj_lw); lwgeom_free(clip_lw);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "clipper2_clip_poly_poly: inputs must be POLYGON or MULTIPOLYGON");
    return nullptr;
  }

  Paths64 subjects, clips;
  lwgeom_to_paths64(subj_lw, subjects);
  lwgeom_to_paths64(clip_lw, clips);
  int32_t srid = gserialized_get_srid(subj_g);
  lwgeom_free(subj_lw);
  lwgeom_free(clip_lw);

  Clipper64 clipper;
  if (! subjects.empty())
    clipper.AddSubject(subjects);
  if (! clips.empty())
    clipper.AddClip(clips);

  PolyTree64 polytree;
  /* EvenOdd matches LWGEOM's topological hole convention: a point's
   * inclusion is determined by ring-crossing parity, not signed area. */
  bool ok = clipper.Execute(ct, FillRule::EvenOdd, polytree);
  if (! ok)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "clipper2_clip_poly_poly: Clipper2 Execute failed");
    return nullptr;
  }

  LWGEOM *out_lw = polytree_to_lwgeom(polytree, srid);
  if (out_lw == nullptr)
  {
    /* The tree answers a geometry for every outcome, the empty overlay
     * included, so nothing reaches here. It raises rather than returning
     * nothing, a null carrying no error being indistinguishable to a caller
     * from an answer */
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "clipper2_clip_poly_poly: the clip tree yielded no geometry");
    return nullptr;
  }
  GSERIALIZED *result = geo_serialize(out_lw);
  lwgeom_free(out_lw);
  return result;
}

