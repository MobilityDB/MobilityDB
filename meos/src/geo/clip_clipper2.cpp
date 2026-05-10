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
 * @brief Clipper2-backed polygon Boolean engine.
 *
 * GSERIALIZED ↔ Clipper2 Paths64 round-trip with full POLYGON / MULTIPOLYGON
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
 *****************************************************************************/

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
#include "temporal/tsequence.h"
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

  if (polys.empty())
    return nullptr;
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
    elog(ERROR, "clipper2_clip_poly_poly: unknown clip operation %d", op);
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
    elog(ERROR,
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
    elog(ERROR, "clipper2_clip_poly_poly: Clipper2 Execute failed");
    return nullptr;
  }

  LWGEOM *out_lw = polytree_to_lwgeom(polytree, srid);
  if (out_lw == nullptr)
    return nullptr;
  GSERIALIZED *result = geo_serialize(out_lw);
  lwgeom_free(out_lw);
  return result;
}

/*****************************************************************************
 * Trajectory-vs-polygon open-path clipping
 *
 * Clips an open polyline (the trajectory of a temporal point sequence)
 * against a closed polygon and returns the time spans on the original
 * sequence where the trajectory is inside the polygon. The caller assembles
 * those spans into a SpanSet and feeds it to tcontseq_restrict_tstzspanset()
 * to recover the at-restriction (preserving Z and other dimensions).
 *
 * Replaces an earlier parity-sweep implementation of
 * tpointseq_linear_at_poly that had two correctness bugs (an inverted
 * horizontal-edge filter that dropped diagonal polygon edges, and a
 * monotonic parity accumulator that never recognised exits). The
 * Clipper2 open-path implementation is structurally immune to both.
 *****************************************************************************/

/**
 * @brief Look up a Point64's index in the input trajectory (exact match
 *        on int64-quantised coordinates), or @p SIZE_MAX if not an input
 *        vertex.
 */
static size_t
input_vertex_index(const Point64 &p,
  const std::vector<int64_t> &xs, const std::vector<int64_t> &ys, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (xs[i] == p.x && ys[i] == p.y)
      return i;
  return SIZE_MAX;
}

/**
 * @brief Interpolate the timestamp of @p p on trajectory segment
 *        [@p seg_idx, @p seg_idx + 1].
 *
 * Computes f = (p − A) · (B − A) / |B − A|² in int128, clamps to [0, 1],
 * and returns @p ts[seg_idx] + f * (ts[seg_idx+1] − ts[seg_idx]). The
 * final mix is in long double, which matches the precision used by
 * tsegment_value_at_timestamptz elsewhere in MEOS. Quantisation error
 * in @p p (a few CLIP_SCALE units off the exact line) is absorbed by
 * the parameter clamp; perpendicular distance is intentionally ignored
 * because the segment is fixed by the caller.
 */
static TimestampTz
interp_t_on_segment(const Point64 &p, size_t seg_idx,
  const std::vector<int64_t> &xs, const std::vector<int64_t> &ys,
  const std::vector<TimestampTz> &ts)
{
  int64_t ax = xs[seg_idx], ay = ys[seg_idx];
  int64_t bx = xs[seg_idx + 1], by = ys[seg_idx + 1];
  int64_t dx = bx - ax, dy = by - ay;
  __int128 lensq = static_cast<__int128>(dx) * dx +
    static_cast<__int128>(dy) * dy;
  if (lensq == 0)
    return ts[seg_idx];  /* zero-length segment — caller picked a bad seg */

  __int128 px = static_cast<__int128>(p.x) - ax;
  __int128 py = static_cast<__int128>(p.y) - ay;
  __int128 dot = px * dx + py * dy;

  long double f = static_cast<long double>(dot) /
    static_cast<long double>(lensq);
  if (f < 0.0L) f = 0.0L;
  else if (f > 1.0L) f = 1.0L;
  long double dt = static_cast<long double>(ts[seg_idx + 1]) -
    static_cast<long double>(ts[seg_idx]);
  return static_cast<TimestampTz>(
    static_cast<long double>(ts[seg_idx]) + f * dt);
}

/**
 * @brief Map an output-path endpoint back to a trajectory timestamp.
 *
 * Each Clipper2 output path is a contiguous sub-portion of the
 * trajectory polyline, possibly trimmed at one or both ends by a
 * polygon-edge crossing. Internal vertices of the path are exact input
 * trajectory vertices (Clipper2 doesn't insert vertices except at
 * polygon-edge crossings). We use that structural property to pick the
 * source segment for a trimmed endpoint: if the immediate neighbour in
 * the path is an input vertex v_j, then the entry crossing lies on
 * segment [v_{j-1}, v_j] and the exit crossing lies on segment
 * [v_j, v_{j+1}]. This avoids the perpendicular-distance argmin
 * (which can pick a wrong-but-near-collinear segment when the trajectory
 * has parallel/repeated runs and produce wildly wrong timestamps,
 * triggering at/minus span overlaps in 056_tpoint_spatialfuncs_tbl).
 *
 * Edge case: if no path vertex is an input vertex (the entire path is
 * two crossings on a single trajectory segment), we fall back to a
 * perpendicular-distance argmin restricted to segments where the
 * parameter range admits the point.
 *
 * @param path     Clipper2 output open path
 * @param at_first true for path.front(), false for path.back()
 * @param xs/ys/ts Input trajectory coordinates and timestamps, length n
 */
static TimestampTz
path_endpoint_t(const Path64 &path, bool at_first,
  const std::vector<int64_t> &xs, const std::vector<int64_t> &ys,
  const std::vector<TimestampTz> &ts, size_t n)
{
  size_t k = path.size();
  size_t boundary = at_first ? 0 : k - 1;
  const Point64 &p = path[boundary];

  /* If the boundary vertex is an exact input vertex, use its timestamp. */
  size_t j = input_vertex_index(p, xs, ys, n);
  if (j != SIZE_MAX)
    return ts[j];

  /* Walk into the path looking for the first input vertex. The crossing
   * sits on the segment immediately preceding (entry) or following (exit)
   * that input vertex, depending on the path direction at this end. */
  if (at_first)
  {
    for (size_t i = 1; i < k; i++)
    {
      size_t v_idx = input_vertex_index(path[i], xs, ys, n);
      if (v_idx != SIZE_MAX)
      {
        /* path[boundary] is a crossing immediately before path[i]=v_{v_idx}.
         * If v_idx > 0, the crossing is on segment [v_{v_idx-1}, v_v_idx].
         * If v_idx == 0, the path goes in reverse trajectory order — the
         * crossing is on segment [v_0, v_1]. */
        size_t seg = (v_idx > 0) ? v_idx - 1 : 0;
        return interp_t_on_segment(p, seg, xs, ys, ts);
      }
    }
  }
  else
  {
    for (size_t i = k; i-- > 0; )
    {
      if (i == boundary)
        continue;
      size_t v_idx = input_vertex_index(path[i], xs, ys, n);
      if (v_idx != SIZE_MAX)
      {
        /* path[boundary] is a crossing immediately after path[i]=v_{v_idx}.
         * Crossing is on segment [v_v_idx, v_{v_idx+1}]. If v_idx == n - 1,
         * the path runs in reverse — use segment [v_{n-2}, v_{n-1}]. */
        size_t seg = (v_idx + 1 < n) ? v_idx : (n > 1 ? n - 2 : 0);
        return interp_t_on_segment(p, seg, xs, ys, ts);
      }
    }
  }

  /* No input vertex anywhere in the path: enter+exit both on a single
   * trajectory segment. Fall back to perpendicular-distance argmin over
   * segments whose parameter range admits @p p.
   *
   * Note on precision: cross² and lensq are both exact in int128 (cross
   * up to ~|seg|² ≈ 1e18, cross² up to ~1e36 which fits), but the
   * cross-multiply trick (cs_a × lensq_b vs. cs_b × lensq_a) needs the
   * product cs × lensq up to ~1e53, which overflows int128 (max 1.7e38).
   * Compare the ratio cs / lensq (= perpendicular distance squared) in
   * long double instead — 80-bit floats give 18+ decimal digits of
   * precision, comfortably enough to distinguish on-segment from
   * off-segment candidates. */
  bool have_best = false;
  size_t best_i = 0;
  long double best_perp_sq = 0.0L;
  for (size_t i = 0; i + 1 < n; i++)
  {
    int64_t ax = xs[i], ay = ys[i];
    int64_t bx = xs[i + 1], by = ys[i + 1];
    int64_t dx = bx - ax, dy = by - ay;
    if (dx == 0 && dy == 0)
      continue;
    __int128 px = static_cast<__int128>(p.x) - ax;
    __int128 py = static_cast<__int128>(p.y) - ay;
    __int128 dot = px * dx + py * dy;
    __int128 lensq = static_cast<__int128>(dx) * dx +
      static_cast<__int128>(dy) * dy;
    if (dot < 0 || dot > lensq)
      continue;
    __int128 cross = px * dy - py * dx;
    long double cross_ld = static_cast<long double>(cross);
    long double lensq_ld = static_cast<long double>(lensq);
    long double perp_sq = cross_ld * cross_ld / lensq_ld;
    if (! have_best || perp_sq < best_perp_sq)
    {
      have_best = true;
      best_i = i;
      best_perp_sq = perp_sq;
    }
  }
  if (! have_best)
    return ts[0];
  return interp_t_on_segment(p, best_i, xs, ys, ts);
}

extern "C" Span *
clipper2_traj_poly_periods(const TSequence *seq, const GSERIALIZED *gs,
  int *out_count)
{
  *out_count = 0;
  if (seq == nullptr || gs == nullptr || seq->count < 2)
    return nullptr;

  /* Build the trajectory's quantised (x, y) and timestamp arrays. The
   * x/y are int64 post-CLIP_SCALE, exactly matching the polygon-side
   * scaling so cross/dot products are exact under int128 arithmetic. */
  size_t n = static_cast<size_t>(seq->count);
  std::vector<int64_t> xs(n), ys(n);
  std::vector<TimestampTz> ts(n);
  Path64 traj;
  traj.reserve(n);
  const char *seq_base = reinterpret_cast<const char *>(&seq->period);
  const size_t *offsets = reinterpret_cast<const size_t *>(
    seq_base + seq->bboxsize);
  const size_t inst_base_off = seq->bboxsize +
    sizeof(size_t) * static_cast<size_t>(seq->maxcount);
  for (size_t i = 0; i < n; i++)
  {
    const TInstant *inst = reinterpret_cast<const TInstant *>(
      seq_base + inst_base_off + offsets[i]);
    const GSERIALIZED *p_gs = reinterpret_cast<const GSERIALIZED *>(
      tinstant_value_p(inst));
    const uint8_t gflags = p_gs->gflags;
    const uint8_t *p_bytes = p_gs->data + 8 +
      FLAGS_GET_BBOX(gflags) * FLAGS_NDIMS_BOX(gflags) * 8 +
      FLAGS_GET_VERSBIT2(gflags) * 8;
    const POINT2D *p = reinterpret_cast<const POINT2D *>(p_bytes);
    xs[i] = static_cast<int64_t>(std::llround(p->x * CLIP_SCALE));
    ys[i] = static_cast<int64_t>(std::llround(p->y * CLIP_SCALE));
    ts[i] = inst->t;
    traj.emplace_back(xs[i], ys[i]);
  }

  /* Build the polygon's Paths64 set. */
  LWGEOM *clip_lw = lwgeom_from_gserialized(gs);
  if (clip_lw == nullptr)
    return nullptr;
  if (clip_lw->type != POLYGONTYPE && clip_lw->type != MULTIPOLYGONTYPE)
  {
    lwgeom_free(clip_lw);
    return nullptr;
  }
  Paths64 clip_paths;
  lwgeom_to_paths64(clip_lw, clip_paths);
  lwgeom_free(clip_lw);
  if (clip_paths.empty())
    return nullptr;

  /* Open-path Intersection: subject is the open trajectory, clip is the
   * closed polygon. EvenOdd matches LWGEOM's topological hole convention
   * (same choice as clipper2_clip_poly_poly). */
  Paths64 traj_paths = { traj };
  Clipper64 clipper;
  clipper.AddOpenSubject(traj_paths);
  clipper.AddClip(clip_paths);
  Paths64 closed_unused, open_solution;
  if (! clipper.Execute(ClipType::Intersection, FillRule::EvenOdd,
        closed_unused, open_solution))
  {
    elog(ERROR, "clipper2_traj_poly_periods: Clipper2 Execute failed");
    return nullptr;
  }
  if (open_solution.empty())
    return nullptr;

  /* Convert each output open path to a Span(t_first, t_last). */
  std::vector<Span> spans;
  spans.reserve(open_solution.size());
  for (const auto &out_path : open_solution)
  {
    if (out_path.size() < 2)
      continue;  /* a single vertex represents a tangent touch — no span */
    TimestampTz t_first = path_endpoint_t(out_path, true,  xs, ys, ts, n);
    TimestampTz t_last  = path_endpoint_t(out_path, false, xs, ys, ts, n);
    if (t_first > t_last)
      std::swap(t_first, t_last);
    if (t_first == t_last)
      continue;  /* degenerate span at a single timestamp */

    Span s;
    span_set(TimestampTzGetDatum(t_first), TimestampTzGetDatum(t_last),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
    spans.push_back(s);
  }
  if (spans.empty())
    return nullptr;

  Span *result = static_cast<Span *>(palloc(sizeof(Span) * spans.size()));
  for (size_t i = 0; i < spans.size(); i++)
    result[i] = spans[i];
  *out_count = static_cast<int>(spans.size());
  return result;
}

