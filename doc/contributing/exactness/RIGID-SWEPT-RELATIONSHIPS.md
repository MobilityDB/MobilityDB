<!--
  MobilityDB — Exactness: the relationships of a rigid geometry in linear motion
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The relationships of a rigid geometry in linear motion

**For discussion by the committers. Nothing here is merged.**
Read on MobilityDB #3347 (`8e344638e3`, on master `91f8d676e7`). This note states which spatial
relationships of a temporal rigid geometry are refused when the geometry moves linearly, why the
constructions that look like answers are not, the algorithm that answers them exactly, what master
already provides for it, its risks, and the pull request that delivers it. Every statement about
the code is a reading of its source on that head; section 9 lists what is still to be measured.

## 1. Verdict

The work is feasible with what master provides. Between two instants of a linear segment the body
turns at a constant rate while its rotation centre moves on a line, and every time at which the
relationship of the body and another geometry can change is a root of a function of the family
the distance solver already isolates with a proof. Between two consecutive such times the
relationship is constant, so it is decided once per stretch. What is new is the set of incidence
functions, the rule that answers at the event times themselves, and an exact judge. The risks are
a contact that only grazes, events that coincide, and the cost on large static areas.

## 2. What #3347 refuses

`trgeo_traced_tgeo` (`meos/src/rgeo/trgeo_spatialrels.c`) turns a step or discrete temporal rigid
geometry into the temporal geometry it traces, and refuses a linear one through
`ensure_nonlinear_interp`. Every ever and always relationship goes through it:

| Relationship | Orders refused for linear motion |
|---|---|
| contains | geometry–rigid, rigid–geometry, rigid–rigid |
| covers | geometry–rigid, rigid–geometry, rigid–rigid |
| disjoint | rigid–geometry, geometry–rigid, rigid–rigid |
| intersects | rigid–geometry, geometry–rigid, rigid–rigid |
| touches | rigid–geometry, geometry–rigid, rigid–rigid |

Each in its ever (`e`) and always (`a`) form. The rigid geometries have no temporal (`t`) form of
these relationships. `eDwithin` and `aDwithin` are not refused: they read the temporal distance.

## 3. The constructions that are not answers

**The traversed area.** The union of the placements over a segment is a region the body occupies
at no single instant. For a square moving away from a point while the point moves to the square's
first place, the two traversed areas give `aContains` true, though the square contains the point
at neither instant; the instant-wise answer is false. No ever or always relationship is read from a
traversed area.

**The instants alone.** Reading only the poses of the instants misses what happens between them.
A U-shaped polygon `Polygon((0 0,5 0,5 3,4 3,4 1,1 1,1 3,0 3,0 0))` and a point moving from
`(0.5 2)` to `(4.5 2)` start and end with the point inside, and the point crosses the gap between
03:00 and 21:00 of the day. A rigid body moving linearly can leave a non-convex region between two
poses and come back in the same way, so a pose walk is the answer only for step and discrete
interpolation.

**The distance at zero.** `intersects` is `dwithin` at distance 0, but the rigid temporal distance
answers `1.48e-16` at a contact whose exact distance is 0, so a contact that only grazes reads as
apart. The distance also refuses a reference geometry that is not a convex polygon
(`dist_ref_body`, "... a convex polygon without holes") and a static polygon that is not convex or
has holes (`dist_parts_add`), which is the subject of
[The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md). A relationship built on
the distance would inherit both.

## 4. The algorithm

### 4.1 Motion

In a segment, with `t` from 0 to 1, a point `a` of the body relative to its rotation centre is at

    v(t) = R(th + w*t) * a + c + t*d

as `meos/src/rgeo/trgeo_distance.txt` states, with the rotation taking the shorter way and a very
small rotation read as none, as `posesegm_interpolate` places the body. A static geometry has
`w = 0` and `d = 0`.

### 4.2 Events

The boundaries of two polygonal sets change their arrangement only when a vertex of one lies on
an edge of the other. Two kinds of incidence give every event:

1. a vertex `p` of the body on the line of an edge `[a, b]` of the other geometry:
   `cross(b - a, v_p(t) - a) = 0`;
2. a vertex `q` of the other geometry on the line of an edge `[u, w]` of the body:
   `cross(v_w(t) - v_u(t), q - v_u(t)) = 0`.

With a static other geometry, the first is a sum of `cos(w*t)` and `sin(w*t)` with constant
coefficients plus a linear term, and the second carries coefficients linear in `t`, since the edge
turns and the centre moves. With two moving bodies, the relative rotation is linear in `t` and each
incidence is a sum of such terms with the two rotation rates. Every case is of the form

    F(t) = SUM_k [ (a1_k*t + a0_k) * cos(w_k*t) + (b1_k*t + b0_k) * sin(w_k*t) ] + c2*t^2 + c1*t + c0

which `distfun_roots` and `distfun_crossings` (`meos/src/rgeo/trgeo_distsolve.c`) isolate with a
proof for each part of the interval. A root is kept when the incident point lies within the edge,
not only on its line.

### 4.3 Deciding a stretch

Between two consecutive events no vertex crosses an edge, so the arrangement, and with it every
DE-9IM relationship, is constant. Each stretch is decided once, at one moment strictly inside it.
The moment is chosen away from both of its events, so the placed body is clear of the other
geometry's boundary by more than the error of its placement, and the decision is that of the true
body. The pull request states that bound with its derivation, as section 4.1 of
[the feasibility of the non-convex distance](NON-CONVEX-DISTANCE-FEASIBILITY.md) states the bound
of its containment test. No point is ever constructed at a rounded event time and located against
a boundary: an event is kept as a parameter of the original input segment, as the crossings of a
moving point are.

### 4.4 Answering at an event

At an event a vertex lies on an edge, so the two geometries intersect there, and the other
relationships follow from the stretches on either side, by whether each is a closed or an open
condition in time:

| Relationship at the event `t*` | Rule |
|---|---|
| intersects | true |
| disjoint | false |
| contains, covers (the containment part, closed) | true if it holds on a stretch beside `t*` |
| interiors meet (open) | true only if they meet on both stretches beside `t*` and the overlap does not shrink to nothing at `t*` |
| touches | intersects and the interiors do not meet |

An event where two or more incidences coincide, or a root of even multiplicity (a contact that
grazes and leaves), is where these rules can fail; section 6 treats it.

### 4.5 Ever and always

An `e` relationship is true when it holds at an instant of the input, at an event or on a stretch;
an `a` relationship is true when it holds at all of them. The instants of the input are decided on
their own poses, which #3347 already does exactly.

## 5. What master already provides

| Need | On master | Where |
|---|---|---|
| the motion of a body in a segment | `posesegm_interpolate` | `meos/src/pose/pose.c` |
| the roots of the family, with a proof | `distfun_roots`, `distfun_crossings` | `meos/src/rgeo/trgeo_distsolve.c` |
| the terms of a vertex and an edge in relative motion | the closest-feature walk | `meos/src/rgeo/trgeo_distwalk.c` |
| exact containment of a point in rings, holes included | `point_in_rings_exact` | `meos/src/geo/geo_funcs.c` |
| exact orientation of three points | `cross_product_sign` | `meos/src/geo/geo_arrange.c` |
| the relationships at an instant | the step and discrete path of #3347 | `meos/src/rgeo/trgeo_spatialrels.c` |
| an R-tree of the edges of a large geometry | `geom_extract_edges`, `build_edge_rtree` | `meos/src/geo/geo_funcs.c` |

Nothing in sections 4.2 to 4.4 requires the body or the other geometry to be convex: the events
read edges and vertices only, so a non-convex body or a polygon with holes needs no convex cover.

## 6. Risks

1. **A contact that grazes.** A vertex that touches an edge and leaves is a double root: the
   function touches zero without changing sign. The solver isolates sign changes with a proof; a
   double root is found as a minimum of the absolute value within the solver's tolerance, which is
   not a proof that the value is zero. Such an event is refused rather than read as a contact or as
   a miss, unless an exact test settles it.
2. **Coinciding events.** Two incidences at the same time, or a stretch shorter than the bound of
   section 4.3, leave no moment to decide it on. They are refused, and the pull request measures how
   often the test data meet them.
3. **The angles are irrational.** `cos` and `sin` of the rotation give no rational test, so the
   judge cannot compare exactly; it certifies each answer by a bounded search, as the CGAL judge of
   `eDwithin` of two rigid geometries does, and the rows it cannot certify are counted, not
   dropped.
4. **Cost.** Events number the body's vertices times the other geometry's edges plus the other way
   round, per segment. A natural area of 67,907 vertices needs the edge R-tree to keep only the
   edges the segment's box can reach; the cost is measured against GEOS called directly.

## 7. Obligations

- **The chapter.** Chapter 4 of MobilityBook Vol. 2 governs the rigid-geometry algorithms. The
  event algorithm is written into it and into `doc/contributing/trgeometry_design_notes.md` in the
  same pull request.
- **The manual.** The note of each relationship that a linear rigid geometry is refused is removed,
  in English and in Spanish.
- **The judge.** CGAL certifies every stretch and every event of a corpus of linear rigid segments
  against polygons, convex and not, with holes, against lines and points, and against a second
  rigid geometry, with a control that must accept and one that must refuse.
- **The tests.** SQL regression rows for every relationship, order and form, the U witness of
  section 3 among them.

## 8. Delivery

One pull request, on the branch of #3347, in place of its refusal: the incidence functions, the stretch
decision, the event rules, the five relationships in their three orders, the chapter and the
manual, the judge and the tests. Its commits are those steps, in that order.

## 9. What is still to be measured

- how often the test data and the AIS data meet the refusals of section 6;
- the cost against the step path of #3347 and against the distance of the same rows;
- whether the bound of section 4.3 leaves a moment in every stretch of the test data.
