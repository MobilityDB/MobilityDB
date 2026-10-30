<!--
  MobilityDB — Exactness: The distance of non-convex and holed geometries
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The distance of non-convex and holed geometries

**For discussion by the committers. Nothing here is merged or pushed.**
Master at the time of writing: `d51b32e664`.

## 1. Summary

The temporal distance of a temporal rigid geometry, and everything derived from it, accepts only
**convex polygons without holes**. This applies both to the reference geometry of the moving body
and to any polygon of a static geometry it is measured against. Every other polygon raises
`MEOS_ERR_FEATURE_NOT_SUPPORTED`. Real geometries are rarely convex: harbour basins, breakwaters,
protected areas, the concave hull of a wind farm, the hull of a tug with a notch, any polygon with a
hole. Volume 2 of the book promises the general case (Sect. 4.6.2) and says, in its own use case,
that MobilityDB does not provide it yet (Sect. 4.7).

This note proposes how to answer every polygon, non-convex or holed, on either side:

- **the moving body** is split once into convex parts that cover it exactly, holes included;
- **a static polygon** is read as its boundary segments, every ring included, plus a containment
  test, so a hole needs no special case;
- the minimum over pairs of parts becomes an **exact lower envelope**, which carries every crossing
  between two parts at its true instant, not where two linear interpolations happen to cross.

The third point also concerns master as it is. The fold over the parts of a static line or
multi-geometry, which master already performs, places the corner between two parts where their
linear interpolations cross. That corner is a local maximum of the distance, which the book's
contract keeps (Sect. 4.6.3). Whether master misplaces or misses it in practice is stated as a
hypothesis to measure (section 6.4).

## 2. What the book states

**Sect. 4.6.2 *Non-Convex Polygons*:** "For a non-convex polygon, the closest feature may jump from
one part of the polygon to a distant one, so the algorithm above cannot be applied directly. The
polygon is therefore decomposed into convex parts, [...] the algorithm is applied to every pair of
convex parts of the two objects, and the partial results are merged by keeping, at each instant, the
pair of parts that are closest. Since the shape of a rigid object never changes, the convex
decomposition is computed only once, on the reference geometry [...]. Furthermore, the parts of the
decomposition may overlap."

**Sect. 4.7 *Clearance to Fixed Structures*** measures vessels against the Anholt offshore wind farm,
represented by the centroid A of its turbines and by their convex hull B. It then says: "The concave
hull C of the turbines follows the rows of turbines more closely and covers 92 km² instead of
119 km². [...] However, since C is not convex, its distance to a temporal rigid geometry requires the
convex decomposition described in Sect. 4.6.2, which MobilityDB does not provide yet: replacing B by
C in the query above raises an error stating that the distance to a polygon that is not convex is not
supported."

**Sect. 4.6.3** fixes the form of the result: a piecewise-linear `tfloat` that keeps the exact values
at the ends of every segment and **all the extreme points of the function, its minima and maxima**.

## 3. What master does

### 3.1 Where the refusal is

`meos/src/rgeo/trgeo_distance.c`:

| Operand | Function | Accepts | Refuses with |
|---|---|---|---|
| reference geometry of the moving body | `dist_ref_body` → `distrefpoly_set` (`trgeo_distwalk.c`) | a convex polygon without holes | "The distance of a temporal rigid geometry needs a reference geometry that is a convex polygon without holes" |
| static geometry | `dist_geo_parts` → `dist_parts_add` | points, every segment of a line, convex polygons, and their collections | "The distance of a temporal rigid geometry to a polygon that is not convex or has holes is not supported" |

`dist_ref_body` states the rule: "The walk needs a convex body. A polygon that is not convex or has
holes gives an error, also where the walk is not necessary, so that the distance functions accept the
same values in all cases." A line is never refused, because each of its segments is a convex part.

### 3.2 What inherits the refusal

`trgeo_tdistance_geo`, `trgeo_tdistance_temporal` and `trgeo_edwithin_trgeo` call `dist_ref_body`,
and the first also calls `dist_geo_parts`. Through them the refusal reaches `tDistance`,
`nearestApproachDistance` (`|=|`), `nearestApproachInstant`, `shortestLine` and `tDwithin` of a
temporal rigid geometry against a geometry, a temporal point or another temporal rigid geometry,
and `eDwithin` of two temporal rigid geometries. The ever and always relationships against a
geometry go through the traversed area and are not concerned.

### 3.3 Why the walk needs convexity

The walk (`trgeo_distwalk.c`, described in `meos/src/rgeo/trgeo_distance.txt`) follows the pair of
closest features of two bodies. It finds the next pair among the neighbours of the current one,
because around a convex polygon the regions where each vertex or edge is closest form the outer
Voronoi diagram of the polygon (book Sect. 4.6.1, figure *Outer Voronoi diagram of a convex
polygon*). The other body crosses from one region only into an
adjacent one. Around a non-convex polygon these regions are not connected that way: the closest
feature can jump to a distant one. A hole makes the body itself non-convex. The local step that
makes the walk cost proportional to the number of feature changes is then unsound, which is why the
walk refuses such a body rather than answering it.

### 3.4 How master folds the parts of a static geometry

For a static geometry with several parts, `dist_seq` walks each part separately and folds the results
two by two:

- `tfloatseq_min_tfloatseq` takes the pointwise minimum of two linear sequences. At each interval
  where the two sequences change order, it inserts an instant "where their linear interpolations
  cross", at `prevt + llround((t - prevt) * ratio)`;
- `dist2d_seq_exact_values` then replaces the value at every instant by `geom_distance2d` of the body
  placed at that instant and the whole geometry (PostGIS, in doubles), because "the interpolated
  value there is not the distance".

The minimum of two distance functions has a corner where they cross, and that corner is a **local
maximum** of the minimum: before it one part is closer, after it the other, so the minimum rises
into the corner and falls away from it. The book keeps every maximum (Sect. 4.6.3). Two things can
go wrong with the master fold:

- the instant of the corner is where the **interpolations** cross, which is not where the two
  **distance functions** cross, since each is exact only at its own vertices. The value written there
  is the true distance at that instant, but that instant is not the corner, so the maximum is cut;
- two functions can cross twice inside one interval of the merged instants while their
  interpolations do not cross at all. The corners are then missing.

Minima are not affected: every minimum of the fold is a minimum of one part, which the part's walk
carries exactly. So `nearestApproachDistance` and `nearestApproachInstant` are exact, and the curve
between them is what may be wrong. Section 6.4 states how to measure it.

## 4. The tools master already has

| Need | On master |
|---|---|
| the walk for a convex part of a body, placed off the rotation centre | `distrefpoly_set_offset` (`trgeo_distwalk.c`) |
| a segment or a point as a part | `distrefpoly_set_segment`, `distrefpoly_set_point` |
| all the roots, with a proof, of a function of the walk's family | `distfun_roots`, `distfun_crossings` (`trgeo_distsolve.c`) |
| exact containment of a point in any set of rings, holes included | `point_in_rings_exact` (`geo_funcs.c`): ray cast, half-open rule, sides by `cross_product_sign` |
| exact orientation | `cross_product_sign` (`geo_funcs.h`): Shewchuk filter, exact fallback |
| convexity of a reference geometry | `trgeo_ref_is_convex` (`trgeo_spatialfuncs.c`) |
| an R-tree of the edges of a large geometry | `geom_extract_edges`, `build_edge_rtree` (`geo_funcs.c`), `rtree_search` (`temporal_rtree.c`) |

MEOS has no triangulation and no convex partition. The partition of section 5.1 is new code.

## 5. The proposal

### 5.1 The moving body: an exact convex cover, computed once

The reference geometry is one polygon for the whole temporal value, so it is split **once per value**
(a convex one is its own single part, and nothing changes for it):

1. **Holes are bridged** to the outer ring. Each hole, taken from its rightmost vertex, is joined by
   a segment to a vertex of the outer ring that sees it. Visibility is decided by `cross_product_sign`
   on input vertices. The result is one simple ring that walks along each bridge twice.
2. **Ear clipping** triangulates that ring. An ear is decided by exact orientations of three input
   vertices, and its emptiness by exact orientations against the reflex vertices. The ring is small
   (a hull has tens of vertices), so O(n²) is enough.
3. **Hertel–Mehlhorn** removes each diagonal whose removal leaves both neighbouring pieces convex,
   again by exact orientations. The pieces number at most four times the minimum.

Every vertex of every piece is an input vertex, so the pieces carry **no constructed coordinate**,
and the cover is exact. The pieces have disjoint interiors and their union is the polygon minus its
holes, so:

- **a piece of the body meets the other operand exactly when the body does.** The walk answers 0 for
  two overlapping convex bodies ("the two bodies can overlap; then the distance is zero",
  `trgeo_distance.txt`), so the minimum over pairs is 0 exactly when the true distance is 0, with no
  containment test on the body's side;
- the distance from the body is the minimum over its pieces, since every point of the body lies in
  a piece.

The book allows **overlapping** parts (the cross needs three disjoint parts and two overlapping ones).
Overlap reduces the number of pairs but is an optimization, not a condition. It is left to a later
step.

### 5.2 A static polygon, holes included: its boundary plus one containment test

A static polygon can be very large: the protected areas of the AIS use case reach 67,907 vertices, and
many have holes. A convex partition of such a polygon would hold thousands of pieces. The answer reads
the polygon by its **boundary** instead:

- every segment of every ring, outer and inner, is a part, exactly as the segments of a line are parts
  on master (`dist_parts_add`, `LINETYPE`);
- while the body touches or crosses the boundary, the minimum over these parts is 0, which is right;
- while the body stays clear of the boundary, it lies wholly **inside** the area the polygon bounds or
  wholly **outside** it, and cannot change side without touching the boundary. Inside, the distance is
  0. Outside, it is the minimum over the boundary segments.

So one exact containment test per stretch of time clear of the boundary decides the answer:

| Configuration | Boundary minimum | Containment test | Distance |
|---|---|---|---|
| body crosses or touches a ring | 0 | not needed | 0 |
| body inside the area, clear of every ring | > 0 | inside | **0** |
| body outside the polygon | > 0 | outside | boundary minimum |
| body inside a hole, clear of its ring | > 0 | outside (the ray crosses the hole's ring too) | boundary minimum, to the hole's ring |
| body covers a hole entirely, overlapping the area around it | 0 (the hole's ring lies in a piece of the body) | not needed | 0 |
| polygon of a multipolygon lying inside a hole of another | > 0 | even–odd over all rings | correct by parity |
| the whole static polygon inside the body | 0 (its rings lie in pieces of the body) | not needed | 0 |

`point_in_rings_exact` already answers the containment over any set of rings by the even–odd rule,
holes and nested polygons included. The tested point is one vertex of the body at one instant of the
stretch: that vertex is inside the area exactly when the whole body is, throughout the stretch.

**The tested point is a constructed one.** The body's vertex is placed by its pose, in doubles. The
test is exact for the placed point, and the placed point is on the same side as the true vertex
whenever its distance to the boundary exceeds the error of the placement. The instant chosen is the
one of largest boundary distance in the stretch, a vertex of the result whose value the walk gives.
The test is accepted only where that distance clears an error bound computed from the pose and the
coordinates. A stretch that never clears it is one whose distance is within rounding of 0 throughout,
and it answers 0. The bound is stated in the pull request with its derivation.

The same reading serves a static polygon against a temporal point (the body of radius 0) and against
a temporal rigid geometry, and it replaces the refusal in `dist_parts_add` for every polygon, convex or
not. A convex static polygon may keep its single convex part, which needs no containment test. Which
of the two is faster for a convex polygon is measured, not assumed.

### 5.3 Two moving bodies

Both bodies are covered as in section 5.1, and every pair of pieces is walked. The cover property on
both sides makes the minimum 0 exactly when the bodies meet. A temporal point against a body is the
same with one side a single point.

## 6. The exact lower envelope

### 6.1 What it replaces

`tfloatseq_min_tfloatseq` and `dist2d_seq_exact_values` (section 3.4) are replaced by one merge that
works on the **functions** each pair's walk follows, not on their sampled vertices. Between two of its
events, each pair's distance is one of:

- a vertex against a vertex: `d = √V(t)`, with `V` the squared distance, a function of the walk's
  family;
- a vertex against an edge: `d = |G(t)| / L`, with `G` the cross product, a function of the family;
- the walk's degenerate cases (parallel edges, overlap at 0), which are constants or zero.

The walk records these pieces. It does not need new geometry.

### 6.2 Where two pairs cross

The corner between pair 1 and pair 2 is a root of `d1 = d2` inside the interval where both pieces
hold. All the parts of one body share its rotation, so in one segment every function has the same
frequencies (`wA`, `wB`, `wA − wB`). Then:

| Pieces | Equation | Family |
|---|---|---|
| vertex–vertex and vertex–vertex | `V1 − V2 = 0` | the walk's family; `distfun_roots` solves it as it is |
| vertex–edge and vertex–edge | `L2·G1 ∓ L1·G2 = 0`, the sign known on the interval | the walk's family |
| vertex–vertex and vertex–edge | `L²·V − G² = 0` | **outside it**: `G²` has coefficients of degree 2 in `t` and frequencies `2w` and sums of two frequencies |

The third case needs the solver's family widened to terms
`(q2·t² + q1·t + q0)·cos(w·t) + (r2·t² + r1·t + r0)·sin(w·t)` with up to about ten frequencies. The
solver's proof does not change. A cell has no root when `|F(m)| > (h/2)·max|F′|`, or when `F` is
monotonic in it, and the bounds of `|F′|` and `|F″|` come from the coefficients: a sine or a cosine is
at most one, and a quadratic coefficient's largest value on a cell is at an end or at its vertex, as
`quad_absmax` (`trgeo_distsolve.c`) already computes. The cell budget (`DISTFUN_CELLMAX`) and the root
array (`DISTFUN_MAXROOTS`) grow with the number of terms, and overflowing either remains an error,
never a wrong answer.

### 6.3 The result

- **The corners carry their true instants.** The instant is dated by the rule of
  [Exact crossings](EXACT-CROSSINGS.md) section 7, by the shared primitive once it exists and by
  master's truncation until then, like every other instant of the walk.
- **The value at every instant comes from the piece that holds it**, evaluated as the walk evaluates
  its own values. The recomputation by `geom_distance2d` of the placed body and the whole geometry
  goes away, and with it the special case that keeps an instant at the threshold `level`.
- **The merge** keeps the pairs in a kinetic tournament: the current minimum and, for each pair, the
  next instant it can become the minimum. It handles the pairs left after pruning (section 7) in
  O(k log k) per segment for k pairs.

### 6.4 The defect master may already have, to measure first

Section 3.4 describes how master's fold over the parts of a static line or multi-geometry can cut or
miss the corners of the distance. **This is a hypothesis, not a measurement.** The measurement:

- a body that rotates while it passes between two point targets, or along a line of two segments
  meeting at an angle, so that the closest part changes inside a segment of the body;
- the true corner from high-precision arithmetic (`mpmath`, 60 digits) on the same doubles: the
  instant where the two exact distances are equal, and the distance there;
- master's `tDistance`: whether it has an instant within one microsecond of each true corner, and
  whether its maximum near the corner is the true one.

If master is right on that corpus, section 6 is needed only for the new parts of section 5. If it is
wrong, the envelope is also a correction of master's existing fold, and it comes first (section 10).

## 7. Cost, and how it is kept down

- **The cover of the body** is computed once per value, as the walk's body is today. Its pieces
  multiply the pairs: a body of P pieces against a static geometry of E boundary segments gives P·E
  pairs per segment of the trip.
- **Pairs that cannot be the minimum are skipped.** Every point of a piece lies within its radius of
  its centre. The centres move linearly in a segment, so the least distance of two pieces in a segment
  is bounded below in closed form, the bound `eDwithin` already uses (book Sect. 4.6.3). A pair whose
  lower bound exceeds the largest value of the current envelope over the segment never becomes the
  minimum and is not walked. This is the same shape as the bounds in doubles of the exact distance
  campaign (#3164: rank the edges by bounds, read the exact distance only of those that may give the
  minimum).
- **A large static polygon** keeps its boundary segments in an R-tree (`build_edge_rtree`). For each
  segment of the trip, only the segments within the reach of the body, widened by the current upper
  bound, are candidates (`rtree_search`). The R-tree is built once per static geometry per call.
- **Measured on the benchmarks, not assumed:** the tcbuffer benchmark's protected areas against hull
  geometries, and a corpus of hulls with notches and holes. A convex body against a convex polygon
  must cost what it costs on master. That is the speed check of the first pull request.

## 8. Relation to chapter 4 of the book

Chapter 4 of Volume 2 documents the rigid geometry algorithms, and the code follows it unless a
deviation is licensed. A licence has four parts: a measured correctness check against an oracle, a
measured performance check on real data, an entry in `doc/contributing/trgeometry_design_notes.md`
stating what the chapter says, what the code does instead and the measurement, and the chapter
itself updated to match.

| Part of the proposal | Chapter 4 | Status |
|---|---|---|
| exact convex cover of the moving body (section 5.1) | Sect. 4.6.2: decompose once, on the reference geometry, into convex parts | **follows the chapter**; overlapping parts, which the chapter allows, are a later optimization |
| exact lower envelope (section 6) | Sect. 4.6.2: "the partial results are merged by keeping, at each instant, the pair of parts that are closest"; Sect. 4.6.3: keep all the minima and maxima | **follows the chapter**, at the true instants of the corners; master's fold at interpolation crossings is the divergence it removes |
| static polygon read by its boundary plus a containment test (section 5.2) | Sect. 4.6.2 decomposes the polygons of both objects | **a deviation**, licensed by the four parts: the judge of steps 2–3 (correctness), the protected areas of the tcbuffer benchmark against a convex partition (performance), a section *Non-convex and holed geometries* in `trgeometry_design_notes.md`, and Sect. 4.6.2 stating that a static polygon is read by the segments of its rings with one containment test per stretch clear of them |
If the measurements of section 7 show a convex partition of the static polygon as fast as its
boundary, the deviation is dropped and the chapter's decomposition is used on both sides.

**The book states no implementation status.** A printed sentence saying what MobilityDB does not
provide yet is false the day the feature lands and stays false in print. Every edit of the chapter
this note implies states the algorithm, never whether master implements it. The current text of
Sect. 4.7 breaks that rule: it says that the concave hull C "requires the convex decomposition
described in Sect. 4.6.2, which MobilityDB does not provide yet: replacing B by C in the query above
raises an error". The first half is timeless and stays. The clause from "which MobilityDB does not
provide yet" to the end is removed from the book, independently of this campaign and of its
schedule. The status of the feature belongs to this note.

## 9. Relation to the other notes

- **[Feasibility](NON-CONVEX-DISTANCE-FEASIBILITY.md).** The assessment of this plan against master:
  the primitives it reuses, the exactness of the convex cover, the widening of the solver, the risks,
  the obligations and the order of delivery.
- **[Any number of turning points in the lifting](N-TURNING-POINTS.md).** The envelope gives any
  number of instants per segment: corners, minima, maxima. Today they are built inside the family's
  own walk (`dist_seq`). When that note's third pull request puts the rigid geometry distance on the
  shared lift, the minimum over parts stays in the family, since it is a minimum over parts and not a
  lift, and the envelope is what it calls.
- **[Exact crossings](EXACT-CROSSINGS.md).** A corner between two parts is a crossing like any other,
  and it is dated by that note's rule.
- **[Exact distance](EXACT-DISTANCE.md).** That campaign makes the distances of
  points, segments and circular buffers the nearest double. The rigid geometry walk evaluates its
  functions in doubles, and this note does not change that. It removes one PostGIS recomputation in
  doubles (`dist2d_seq_exact_values`).
- **No GEOS.** MEOS and MobilityDB build without GEOS (MobilityDB #3185). The cover, the containment and the envelope run on
  exact orientations and the walk's solver. `ST_ConcaveHull`, which builds the book's polygon C, is a
  PostGIS function outside MEOS.

## 10. Plan

**Schedule.** This work starts only after the work of
[Exact distance](EXACT-DISTANCE.md) is finished, the measurement of step 1
included: exact distance has priority, in particular for the circular buffer benchmark. It changes `dist_seq` and the
parts in `trgeo_distance.c`, which the third pull request of [N-TURNING-POINTS](N-TURNING-POINTS.md)
also rewrites. Whichever lands second carries the other's change.

Each step is its own pull request with its witness, the regression suite, the expected outputs
classified, and a judge computed independently: exact rationals for the cover and the containment,
high-precision arithmetic for the corners.

1. **Measure section 6.4.** No code change. Its result decides whether step 2 is a correction of
   master or only a prerequisite.
2. **The exact lower envelope** for the parts master already folds (lines and multi-geometries),
   with the widened solver family of section 6.2. The convex cases keep their answers. An answer
   moves only where master cuts or misses a corner, each shown against its high-precision value.
3. **Static polygons, any shape, holes included** (section 5.2): boundary segments, containment by
   `point_in_rings_exact` with its certified margin, R-tree pruning for large polygons. The book's
   polygon C is part of its witness.
4. **The moving body, any shape, holes included** (section 5.1): the exact convex cover, computed
   once per value; temporal points and two moving bodies (section 5.3). The code then implements
   the algorithm Sect. 4.6.2 describes.
5. **Overlapping covers and pruning measured on the benchmarks** (section 7), if the measurements ask
   for them.

## 11. Decisions asked of the committers

1. Should a static polygon be read as its boundary plus an exact containment test (section 5.2), and
   the moving body as an exact convex cover computed once per value (section 5.1), rather than one of
   the two methods for both?
2. Should the minimum over parts become an exact lower envelope with its corners at their true
   instants (section 6), widening the walk's solver family for the mixed case?
3. Is the containment test on a placed vertex, accepted only beyond a certified error margin and
   answering 0 for a stretch that never clears it, acceptable (section 5.2)?
4. Is the plan's order acceptable, with the measurement of section 6.4 first?
