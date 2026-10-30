<!--
  MobilityDB — Exactness: feasibility of the distance of non-convex and holed geometries
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Feasibility of the distance of non-convex and holed geometries

**For discussion by the committers. Nothing here is merged or pushed.**
Master read: `9e4cf5d422`. This note assesses the plan of
[The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md): what master already
provides, what is new, where the difficulty and the risk lie, which obligations the work carries,
and in which order it is delivered. Every statement about master below is a reading of its source;
none comes from running it, and section 9 lists what is still to be measured.

## 1. Verdict

The plan is feasible. Every primitive it needs is on master except a convex partition, which is
built on the exact orientation master already has but is written for exactness rather than taken
from its usual textbook form, whose hole bridging constructs a point and whose degenerate cases are
assumed away (section 4.2.1). The largest piece is the exact lower envelope,
because it widens the family of functions the walk's solver accepts. The main risk is speed on the
large static polygons of the AIS use case, which the plan answers by pruning and which is measured,
not assumed.

## 2. What master refuses, read in the source

| Operand | Function (`meos/src/rgeo/trgeo_distance.c`) | The error the source raises |
|---|---|---|
| reference geometry of the moving body | `dist_ref_body` (line 87) | "... needs a reference geometry that is a convex polygon without holes" |
| polygon of a static geometry | `dist_parts_add` (line 115) | "... to a polygon that is not convex or has holes is not supported" |

The refusal reaches every function built on these two: `tDistance`, `nearestApproachDistance`
(`|=|`), `nearestApproachInstant`, `shortestLine` and `tDwithin` of a temporal rigid geometry
against a geometry, a temporal point or another temporal rigid geometry, and `eDwithin` of two
temporal rigid geometries.

For a static line or multi-geometry, master folds the distance to each part two by two
(`tfloatseq_min_tfloatseq`): the instant of each corner is where the linear interpolations of the
two sequences cross (`llround`, line 498), and `dist2d_seq_exact_values` then writes at every instant
the distance PostGIS computes between the placed body and the whole geometry (`geom_distance2d`,
line 552). The minimum of two distance functions has a local maximum at their corner, and that
corner is not where their interpolations cross. Whether master's curve misses or cuts these maxima
on real input is the hypothesis of section 6.4 of the plan, still to be measured.

## 3. What master already provides

| Need of the plan | On master | Where |
|---|---|---|
| the walk for a convex part off the rotation centre | `distrefpoly_set_offset` | `trgeo_distwalk.c` |
| a segment or a point as a part | `distrefpoly_set_segment`, `distrefpoly_set_point` | `trgeo_distwalk.c`, `trgeo_distwalk.h` |
| the roots, with a proof, of a function of the walk's family | `distfun_roots`, `distfun_crossings` | `trgeo_distsolve.c`, `trgeo_distsolve.h` |
| the largest absolute value of a quadratic on an interval | `quad_absmax` | `trgeo_distsolve.c` |
| exact containment of a point in any set of rings, holes and nesting included | `point_in_rings_exact` (even–odd rule); `static` in its file, so it leaves `static` for the rigid geometries to call it | `geo_funcs.c` (line 22193) |
| exact orientation of three points | `cross_product_sign` (filtered, exact fallback) | `geo_funcs.h` |
| convexity of a reference geometry | `trgeo_ref_is_convex` | `trgeo_spatialfuncs.c` |
| an R-tree of the edges of a large geometry | `geom_extract_edges`, `build_edge_rtree`, `rtree_search` | `geo_funcs.c`, `temporal_rtree.c` |

The files the work changes hold 1,610 lines (`trgeo_distance.c`), 1,477 lines
(`trgeo_distwalk.c`) and 784 lines (`trgeo_distsolve.c`). MEOS holds no triangulation and no convex
partition.

## 4. The work, piece by piece

### 4.1 A static polygon of any shape: its boundary and one containment test

Every segment of every ring becomes a part, as the segments of a line already are in
`dist_parts_add`. While the body touches or crosses a ring, the minimum over the parts is 0. While it
stays clear of every ring, it lies wholly inside the area or wholly outside it, and one containment
test of one body vertex decides which: inside answers 0, outside answers the minimum over the
segments. `point_in_rings_exact` gives the test over every ring by the even–odd rule, so a hole, a
polygon inside a hole and a multipolygon need no case of their own.

**New:** the bound that makes the test sound. The tested vertex is placed by the pose, in doubles;
the test is exact for the placed point, and the placed point lies on the true vertex's side whenever
its distance to the boundary exceeds the error of the placement. The pull request states that bound
with its derivation. A stretch that never clears it is within rounding of the boundary throughout
and answers 0.

**Chapter 4 of the book governs the algorithms of the rigid geometries.** Its Sect. 4.6.2 decomposes
both operands into convex parts. Reading a static polygon by its boundary and a containment test
departs from it, and ships only with four parts: an independent judge of its answers, its speed
measured against a convex partition on the natural areas, a section in
`doc/contributing/trgeometry_design_notes.md`, and Sect. 4.6.2 stating it. Without them, the static
polygon is decomposed as the body is (section 4.2), and the cost of partitioning a polygon of tens of
thousands of vertices is then the one measured.

**Difficulty:** moderate; most of it is reuse.

### 4.2 A moving body of any shape: an exact convex cover

The reference geometry is one polygon for the whole temporal value, so it is split once per value
(a convex one is its own single piece): each hole is bridged to the outer ring through a vertex that
sees it, ear clipping triangulates the resulting ring, and Hertel–Mehlhorn merges triangles while
the merged piece stays convex (at most four times the minimum number of pieces). Every decision is
an exact orientation of input vertices, so every piece's vertices are input vertices and the cover
carries no constructed coordinate. The pieces have disjoint interiors and cover the polygon minus
its holes, so the minimum over the pieces of the walk's distances is 0 exactly when the body meets
the other operand, and otherwise is the body's distance.

**New:** the partition, with an O(n²) ear clipping that suits a reference geometry of tens of
vertices.

#### 4.2.1 Exactness of the cover

The cover is exact only if every decision is a predicate on input vertices and no step constructs a
point. Ear clipping and Hertel–Mehlhorn already have that shape: an ear is three input vertices of
positive orientation with no reflex vertex inside, and a diagonal is removed when both its endpoints
stay convex, each an orientation of input vertices. With `cross_product_sign`, which is exact, these
decisions are exact, and every piece's vertices are input vertices. The algorithms as textbooks and
common implementations state them are not exact as they stand, in three places:

| Place | The usual form | The exact form |
|---|---|---|
| bridging a hole | a ray from the hole's rightmost vertex is intersected with the outer ring, and the intersection point, constructed in doubles, selects the bridge vertex | the bridge vertex is chosen by orientation comparisons alone: among the vertices of the outer ring right of the hole's rightmost vertex, the one the bridge reaches without crossing an edge, each crossing decided by segment-intersection predicates on input vertices |
| degenerate input | general position is assumed | collinear consecutive vertices (orientation 0) and repeated vertices are removed first, which constructs nothing; a hole touching the outer ring at a vertex, valid in the OGC model, is bridged through that vertex; the zero-width corridor of a bridge, walked twice, is handled by one rule for orientation 0 throughout the clipping |
| invalid input | not considered | an exact simple-polygon test (every pair of non-adjacent edges by segment-intersection predicates) refuses a self-intersecting ring rather than covering it |

The pieces are then exact, and their exactness is checked independently in exact rationals, the
coordinates being doubles and so rationals: the areas of the pieces sum to the area of the polygon
minus its holes, every piece is convex, and every two pieces meet in an intersection of area exactly
zero. CGAL's exact partition serves as a second judge.

The cover does not make the distance exact by itself: the walk places each piece by the pose in
doubles, as it places a convex body on master, so the distance carries master's precision and no
less.

**Difficulty:** medium and self-contained; the code is written for exactness rather than ported.

### 4.3 The exact lower envelope

The fold of section 2 gives way to a merge of the functions each pair of parts follows. Between two
events of its walk, a pair's distance is `√V(t)` (vertex against vertex, `V` the squared distance) or
`|G(t)|/L` (vertex against edge, `G` a cross product), each a function of the walk's family. Two pairs
cross where these are equal:

| Pieces | Equation | In today's family |
|---|---|---|
| vertex–vertex and vertex–vertex | `V1 − V2 = 0` | yes |
| vertex–edge and vertex–edge | `L2·G1 ∓ L1·G2 = 0` (the sign known on the interval) | yes |
| vertex–vertex and vertex–edge | `L²·V − G² = 0` | **no** |

Master's `DistFun` (`trgeo_distsolve.h`) holds at most `DISTFUN_MAXTERMS` = 3 terms of the form
`(a1·t + a0)·cos(w·t) + (b1·t + b0)·sin(w·t)`, a quadratic polynomial part, and at most
`DISTFUN_MAXROOTS` = 8 roots. `G²` has coefficients of degree 2 in `t` and frequencies `2w` and the
sums of two frequencies, so the mixed case needs terms
`(q2·t² + q1·t + q0)·cos(w·t) + (r2·t² + r1·t + r0)·sin(w·t)` and about ten of them. The solver's
proof is unchanged: a cell holds no root when `|F(m)| > (h/2)·max|F′|` or when `F` is monotonic on it,
and the bounds of `|F′|` and `|F″|` follow from the coefficients, a sine or a cosine being at most one
and a quadratic's largest absolute value on a cell being what `quad_absmax` computes. The cell budget
and the root array grow with the number of terms, and overflowing either stays an error, never a
wrong answer.

The merge keeps the pairs in a kinetic tournament (the current minimum and, for each pair, the next
instant it can become the minimum), O(k log k) per segment for k pairs. The value at every instant
comes from the piece that holds it, as the walk evaluates its own values.

**New:** the widened function type and its bounds, the crossing equations, the tournament.

**Difficulty:** the hardest piece of the plan.

### 4.4 Keeping the cost down

A body of P pieces against a static geometry of E boundary segments gives P·E pairs per segment of
the trip. Two means bound the pairs that are walked: a pair whose lower bound (each piece within its
radius of its centre, the centres moving linearly in a segment, the bound `eDwithin` uses) exceeds
the envelope's largest value over the segment never becomes the minimum; and a large static polygon
keeps its segments in an R-tree, of which only those within the body's reach widened by the current
upper bound are candidates.

**Difficulty:** moderate; its value is measured on the benchmarks.

## 5. Risks

1. **Speed on large polygons.** The protected areas of the AIS use case reach 67,907 vertices, and
   a body of several pieces multiplies the pairs. A convex body against a convex polygon must cost
   what it costs on master. Pruning is the answer; its effect is measured on the tcbuffer benchmark's
   protected areas and on a corpus of hulls with notches and holes.
2. **The widened solver.** More terms mean more cells. A budget or root-array overflow is an error,
   never a wrong answer, but such an error on real data would be a refusal again, so the budgets are
   sized on the corpora.
3. **The containment bound** is derived and stated in its pull request; an unsound bound would answer
   the wrong side of the boundary.
4. **Sequencing with [Any number of turning points](N-TURNING-POINTS.md).** Its third pull request
   rewrites `dist_seq` in the same file. That campaign has no pull request in flight, so the two do not
   conflict today; whichever lands second carries the other's change.

## 6. The obligations the work carries

- **Outside the two obligations of a native replacement.** The temporal rigid geometry distance is
  a temporal walk and replaces no PostGIS geometry function, so neither the judgement of the head
  by CGAL nor speed parity with GEOS called directly applies to it as an obligation of a native
  replacement.
- **The liblwgeom rule.** The envelope removes the recomputation of each value by `geom_distance2d`,
  which is vendored PostGIS code calling no GEOS. Vendored PostGIS code stays unless measured inexact
  or slow, and a replacement of it is timed against that function called directly in the same build.
  The measurement of section 9 is what justifies removing that recomputation; if it shows master's
  curve right, the recomputation stays for the parts master already folds, and the envelope serves the
  new parts.
- **Correctness by independent judges:** high-precision arithmetic (`mpmath`, 60 digits) on the same
  doubles for the instants and values of the corners; exact rationals for the cover and the
  containment; CGAL's exact distance between the placed body and the static geometry at sampled
  instants; the book's concave polygon C as a witness. Each judge accepts known-correct answers and
  refuses perturbed ones before its verdict counts. A fold or an envelope is judged at its turning
  points and along its curve, never by the nearest approach distance alone, which is blind to a
  wrong curve.
- **Speed by its own instrument,** built beside the existing one as for the temporal nearest approach:
  the `perf_geos` cells do not call this walk, so their green says nothing about it.

## 7. Delivery

Each step is its own pull request with its witness, the regression suite, the expected outputs
classified, and its judge.

| Step | Content | Size | Moves an existing answer |
|---|---|---|---|
| 1 | The measurement of section 9; no code change | small | no |
| 2 | The exact lower envelope, with the widened solver, for the parts master already folds (lines, multi-geometries) | large | only where master cuts or misses a corner, each shown against its high-precision value |
| 3 | Static polygons of any shape, holes included (section 4.1) | medium | no; refused inputs answer |
| 4 | Moving bodies of any shape (section 4.2), temporal points and two moving bodies | large | no; refused inputs answer |
| 5 | Pruning (section 4.4), where the benchmarks ask for it | medium | no |

The first step decides whether the second is a correction of master or groundwork for the third and
fourth.

## 8. The decisions of the plan, and the answers this note recommends

The plan asks four questions (its section 11). Each recommended answer follows the rules the exact
geometry work already holds to:

1. **Boundary and containment for a static polygon, an exact cover for the moving body:** yes. The
   static polygon can be very large, and its boundary is already the parts a line has; the body is
   small and is split once per value.
2. **An exact lower envelope with its corners at their true instants:** yes. The book keeps every
   maximum of the distance (Sect. 4.6.3), and a corner is one.
3. **Containment by a placed vertex accepted only beyond a stated bound:** yes; a stretch within the
   bound is within rounding of the boundary and answers 0.
4. **The measurement first:** yes; it is small and decides the nature of step 2.

## 9. What is still to be measured

- **Whether master's fold cuts or misses corners** (the plan's section 6.4): a body that rotates while
  it passes between two point targets, or along a line of two segments at an angle, so that the
  closest part changes inside a segment of the body; the true corner from `mpmath` on the same
  doubles; master's `tDistance` read for an instant within one microsecond of each corner and for its
  value there.
- **That a non-convex or holed operand reaches the refusals of section 2** through each function of
  section 2, by running them.
- **The cost of the cover and of the envelope** on the tcbuffer benchmark's protected areas and on the
  corpus of hulls, against master's convex cases.
