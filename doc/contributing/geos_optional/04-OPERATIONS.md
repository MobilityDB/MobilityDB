<!--
  MobilityDB — Making GEOS optional: the operations, one by one
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# 4. The operations, one by one

**Measured at** MobilityDB master `d9b9c11985`.

This note takes each geometry operation the campaign covers and answers four questions: what it
does for a user, how a build without GEOS answers it, what evidence says the answer is right and
fast enough, and what is left. Speed figures are ratios of native instructions to GEOS
instructions, GEOS called directly, in one process ([note 2](02-JUDGING-AN-ANSWER.md), §2.6):
below 1 is faster than GEOS. Data sets are described in [note 3](03-TEST-DATA.md); counts of
*checks* exceed the rows of a data set because one row carries several checks (note 3 §3.2). A
reference such as "#2706" is the MobilityDB pull request whose description states the figures.

All the C functions named below live in `meos/src/geo/`. The relationship engine and most
kernels are in `geo_funcs.c`, the buffer in `geo_buffer.c`, the overlays and unions in
`postgis_funcs.c`, the clusterings in `geo_cluster.c` and `tgeo_spatialfuncs.c`.

Whether an operation can reach GEOS at all is read from the code by
[`tools/geos_reach.py`](tools/geos_reach.py) ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.4). Four
operations can — the intersection, the difference and the two unions, §4.3 to §4.5 — and every
other operation of this note has no path to GEOS at all.

## 4.1 Spatial relationships — finished, no path to GEOS

**What it does.** Answers whether two geometries intersect, contain, touch or cover one another,
and computes the full *relationship matrix* (the DE-9IM: a 3×3 table saying, for the interior,
boundary and exterior of each shape, how they meet). Temporal predicates such as "does this
trajectory ever enter this zone" call it once per instant.

**How it is answered.** By the native engine `meos_spatialrel` / `meos_relate` /
`meos_relate_pattern`, on the exact stored coordinates, computing only the cells of the matrix a
question needs. Collections are answered natively. The raster code's relationships go through
the same engine (#2769).

**Evidence, stated in #2706 and judged by CGAL.** The GEOS relate suites 1108 of 1108 checks;
areal pairs 5776 of 5776; `geo_pairs` 18 564 of 18 564, and the same again with every coordinate
multiplied by 2^20, 2^−20, 2^−22 and 2^−40, which tests that the answer does not depend on the
size of the shapes; adversarial pairs 89 of 89; AIS trips 202 of 202; AIS trajectories against protected
areas 1362 of 1362; the defining cases of every earlier relate fix 1804 of 1804.

**Speed, stated in #2706.** Four relationships on areal pairs 0.550; on `geo_pairs` 0.628. On AIS
trajectories, two *patterns* — questions written as the nine cells of the matrix, where `T` means
"meet", `F` "do not meet" and `*` "any": `T********` (the interiors meet) 0.240, and `T*****FF*`
(the interiors meet and nothing of the second lies outside the first) 0.152. **Faster than GEOS on every
cell.**

**Speed, stated in #2997, merged as `4f784e1240`.** The engine reads the edges a box meets, and
those the ray cast from a point can cross, out of a sorted index (`EdgeIndex`, `edge_index_query`)
rather than an R-tree built for every edge array, and the index answers the same edges as the R-tree
for every query box. Each of the sixteen relationship cells, eight questions on areal pairs and the
same eight on `geo_pairs`, reads at most GEOS's own instructions: from 0.02 for equality to 0.84 for
touching.

**Left.** Nothing.

## 4.2 Point in polygon — never used GEOS

**What it does.** Answers whether a point (or the points of a multipoint) lies outside, on the
boundary of, or inside a polygon; the fast path of the relationships when one side is a point.

**How it is answered.** `meos_point_in_polygon` in `postgis_funcs.c` builds an interval tree over
the polygon's edges, as PostGIS's `pip_short_circuit` does. It is outside every `#if GEOS` block.

**Left.** Nothing.

## 4.3 Intersection and difference — finished for two geometries, not for collections

**What it does.** `geom_intersection2d` and `geom_difference2d`: the part two geometries share,
and the part of the first not in the second. Temporal restriction (`atGeometry`,
`minusGeometry`) and the clipping of a trajectory to a zone rest on them.

**How it is answered.** A cascade of native routes, tried in the order the code of
`geom_intersection2d_route` gives them: a 2D polygon against a 2D polygon, through Clipper2
(below); a point set, whose answer is the points the other geometry covers; a line, clipped
exactly against the other geometry; a part of no area, read as the boundary it traces; an areal
pair, on the circles its arcs lie on; a multi-part geometry, taken part by part. A pair carrying Z or M is answered on the plane
and the elevations are put back where the inputs determine them (#2821). A part of no area is
read as the boundary it traces (#2824).

**Evidence.** Measured in a build without GEOS by
[`tools/geos_fallback_sweep.c`](tools/geos_fallback_sweep.c): for two geometries, none of the 225
ordered pairs of the 15 geometry types reaches GEOS, flat, with Z on both sides or on one side;
nor does any of fifteen geometric special cases, among them polygons sharing an edge or a corner
and discs tangent at one point. The elevations of the Z answers are judged by `cgal_zlift`
(#2821).

**Not finished: collections.** When one operand is a `GEOMETRYCOLLECTION`, the operation is
taken part by part, and some combinations still reach the GEOS fall-back: of 6750 calls pairing a
collection of two types with each of the 15 types, in both orders, 409 intersections and 953
differences do. Two examples: the intersection of
`MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))` with
`GEOMETRYCOLLECTION(POINT(2 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))`, and the difference of
`GEOMETRYCOLLECTION(LINESTRING(0 2,4 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))` and
`TRIANGLE((0 0,4 0,2 4,0 0))`. Through `atGeometry`, a temporal geometry valued with a
multi-part shape or a collection, restricted to such a collection, reaches GEOS
([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.6).

**Polygon against polygon: Clipper2.** A 2D `POLYGON`/`MULTIPOLYGON` against another goes
through `clip_poly_poly` (`geo_poly_clip.c`), backed by **Clipper2**, a polygon-clipping library
MobilityDB vendors in `clipper2/` and calls through `clip_clipper2.cpp`. It is part of MEOS's
native engine, as is the trajectory clip against a polygon (`clipper2_traj_poly_periods`).
Clipper2 computes on integers: every coordinate is multiplied by 10^7 and rounded
(`CLIP_SCALE`), a resolution of 1e-7 of a unit — about 11 mm in longitude/latitude — which the
project accepts for this route. Two consequences for contributors: Clipper2 takes paths only,
so it is never the vehicle for a shape carrying an arc (that is the areal route on the circles);
and a geometry built by Clipper2 and one built by the exact kernels are at different
resolutions, so one is not clipped against the other.

**Left.** The collections above. And the function comment of `geom_intersection2d` describes
the route as falling through to PostGIS's GEOS-backed entry point for other combinations, which
does not describe the native route cascade above.

## 4.4 Union of one geometry — finished but for collections holding a multi-part member

**What it does.** `geom_unary_union` dissolves one geometry into the set of points it covers:
overlapping polygons merge, a line walking a stretch twice covers it once. Trajectories are built
with it (`trajectory(temp, true)`).

**How it is answered.** Areal geometries by `meos_areal_union`, from their boundaries, keeping
arcs on their circles; lines and points by `meos_linear_union`; a collection mixing both by the
array union of its parts. Z and M are dropped for the computation and put back where determined.

**Evidence.** In a build without GEOS, the union of one geometry of each of the 15 types, flat
and with Z, reaches GEOS for none ([`tools/geos_fallback_sweep.c`](tools/geos_fallback_sweep.c)).
CGAL judges each answer by the measure of its point set: 3097 of 3097 (#2818). Over 21 reference shapes
every answer covers the point set PostGIS 3.6.4's `ST_UnaryUnion` answers, with its area and
length, with one documented exception: a line crossing a polygon stays whole beside it (#2818).

**Not finished: a collection holding a multi-part member.** The union of a
`GEOMETRYCOLLECTION` is taken as the array union of its parts (§4.5), so it reaches GEOS exactly
when a part is a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN` or a `POLYHEDRALSURFACE`: for 104 of
the 225 collections of two types, and for none of the other 121. Even
`GEOMETRYCOLLECTION(POINT(9 9),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5))))` reaches it. The traversed
area of a temporal geometry is such a union of its values, which is why `traversedArea` reaches
GEOS for a temporal geometry with a multi-part or collection value
([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.6).

**Left.** The collections above. A union on a *precision grid* (`prec ≥ 0`) also calls GEOS;
every MEOS caller passes −1 and no SQL function calls `geom_unary_union`, so no user reaches it.

## 4.5 Union of an array of geometries — not finished for multi-part members

**What it does.** `geom_array_union`: the union of many geometries, the aggregate behind merging
trajectories and zones.

**How it is answered.** All surfaces: by their boundaries (`geom_array_areal_union`). Lines and
points: by `meos_linear_union`, which sews lines through an exact hash of their end coordinates
(#2804). A mix: each half by its arm, and a non-areal piece the surfaces cover left out. A pair
of lines that share a stretch and then fork is answered (#2825).

**Evidence.** CGAL by the measure of the point set: the fork sets 3000 of 3000 on each of three
seeds (#2825); the Z sets with no decline over three seeds of 3000 (#2822). Those sets hold
points, lines and single polygons.

**Not finished: multi-part members.** Measured in a build without GEOS
([`tools/geos_fallback_sweep.c`](tools/geos_fallback_sweep.c),
[`tools/arrunion_min.c`](tools/arrunion_min.c)): the union of two geometries reaches GEOS exactly
when one of them is a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN` or a `POLYHEDRALSURFACE` — 104 of
the 225 ordered pairs of types, in every Z mode, and none of the other 121. A `POLYGON` beside a
disjoint `MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)))` is enough; two disjoint `POLYGON`s are
answered. The error raised reads "The union of a geodetic array, and of one whose linework
coincides over a curve, is answered by the GEOS library", and these inputs are neither. Through `merge`, two instants of a temporal geometry at the
same timestamp holding a `POLYGON` and a `MULTIPOLYGON` reach this union, and the merge reports
the refusal as an error (MobilityDB #2946, [note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.7).

**Speed, stated in #2804.** The union of 41 real trajectories takes 102.8 billion instructions on
the base of that pull request and 3.97 billion on its head: **0.271 of GEOS**.

**Left.** The multi-part members above. A geodetic array (longitude/latitude on the sphere) is
answered for points only;
anything else is refused. The committers ruled that a geodetic overlay is future work, since
nothing in MobilityDB needs it and PostGIS itself has no `ST_Union` on geography. One shortcut
stays by design: an array of one member is returned as it is, so a single line that doubles
back keeps its doubled length.

## 4.6 Convex hull, oriented envelope, centroid, simplicity — finished, no path to GEOS

| Operation | Native function | Note |
|---|---|---|
| Convex hull | `convex_hull`, called by `geom_convex_hull` (`geo_funcs.c`) | — |
| Oriented envelope (minimum rotated rectangle) | `meos_oriented_envelope` | The smallest rotated rectangle is not unique: every side of an acute triangle carries one of equal area. MEOS breaks the tie on the shortest diagonal (#2162). |
| Centroid | `meos_centroid` | Also answers `lwgeom_centroid` for k-means clustering in a build without GEOS |
| Is simple | `meos_is_simple` | — |

**Left.** No exact-reference run is recorded for these four; adding one to the CI corpus of the
plan closes that.

## 4.7 Clusterings — finished, no path to GEOS

**What it does.** `geo_cluster_dbscan`, `geo_cluster_within`, `geo_cluster_intersecting`,
`geo_cluster_kmeans`: group geometries by distance or by intersection.

**How it is answered.** Natively in `geo_cluster.c` and `tgeo_spatialfuncs.c`; the intersecting
clustering uses the relationship engine of §4.1, k-means the native centroid.

**Left.** No exact-reference run of the clusterings is recorded in a pull request; adding one to
the instruments of the plan closes that.

## 4.8 Distance — never used GEOS, made exact

**What it does.** The distance between two geometries, the shortest line between them, and
"within distance" tests.

**How it is answered.** By liblwgeom's own distance code (`postgis/liblwgeom/measures.c`), which
never called GEOS. The campaign corrected it on curves, each correction judged by a closed form:

- five fixes from PostGIS 3.6 are replayed (#2674), among them a segment-to-arc distance that read
  0 for a segment lying 21 units from the arc; with the missing candidate points added, the
  segment-to-arc distance agrees with its closed form on 30 000 of 30 000 random pairs;
- the shortest line always starts on the first geometry (#2674);
- the distance of a segment or an arc to an arc is the same at every scale (#2675).

**Speed.** No pull request on the master these notes describe states a speed figure for distance
against GEOS on straight geometries, where GEOS is a speed reference and never a witness. On
curves no comparison exists ([note 2](02-JUDGING-AN-ANSWER.md), §2.6).

**Left.** Nothing: distance never depended on GEOS.

## 4.9 Buffer — the operation still open

**What it does.** `geom_buffer`: the region within a given distance of a geometry, with rounded,
mitred or bevelled joins and end caps.

**How it is answered.** Natively in `geo_buffer.c`, in every build; it never falls back to GEOS.

**Evidence.** CGAL agrees on all 1313 queries of the containment corpus (#2813, #2815 and the
buffer commits after them); the curved-buffer corpora agree on every query with no decline.

**Speed, stated in the message of commit `f8f5d0b1a3` on master.** Natural areas at distance 1:
3.99; at 50: 4.35; at 500: 2.83 — **between about 2.8 and 4.4 times slower than GEOS.** On small shapes with only a few edges (the data set named "joints") 0.548, faster
(#2815). Three later commits on master lower the native instructions of the distance-1 cell
further, as their messages state, against an unchanged GEOS count: `4416773c99` by 3.61 %,
`167648b2a3` by 3.03 % and `0315c48101` by 1.87 %, from 2 015 234 423 to 1 848 451 442
instructions. The same arithmetic puts that cell near 3.7.

**Left.** Its answer depends on where the shape lies, a defect in how it joins its boundary into
rings, the speed gap, and the refused single-sided variant (§4.11). [Note 6](06-BUFFER.md) is
about all of it.

## 4.10 Operations no MobilityDB function exposes

`geo_lwgeom_none.c` also stubs `lwgeom_make_valid`, `lwgeom_split` and `lwgeom_offsetcurve`. No
MEOS function and no SQL function reaches them, so nothing is missing for a user today. The
committers fixed what they must answer if they are ever exposed, with reference answers read on
PostGIS 3.6.4 and recorded with that decision (2026-09-22); they are not re-run for these notes:

- **make_valid** answers what PostGIS's default `ST_MakeValid` answers, the `linework` method.
  `POLYGON((0 0,1 1,2 2,0 0))` → `MULTILINESTRING((0 0,1 1),(1 1,2 2))`;
  `POLYGON((0 0,2 2,2 0,0 2,0 0))` → `MULTIPOLYGON(((0 2,1 1,0 0,0 2)),((2 0,1 1,2 2,2 0)))`.
- **offset curve** answers what `ST_OffsetCurve` answers: `LINESTRING(0 0,10 0)` offset by +1 is
  `LINESTRING(0 1,10 1)`, by −1 `LINESTRING(0 −1,10 −1)`.

## 4.11 The single-sided buffer — refused

**Today**, `geom_buffer` with `side=left` or `side=right` raises "Single-sided buffer is not
supported", in every build. **The behaviour to build**, as the committers specified it, copies
PostGIS answer for answer, parameters included. The reference answers below are those of PostGIS
3.6.4, recorded with that decision (2026-09-22); they are not re-run for these notes:

- `ST_Buffer('LINESTRING(0 0,10 0)', 1, 'side=left')` is `POLYGON((10 0,0 0,0 1,10 1,10 0))`, and
  so is every `endcap` value: PostGIS cuts a single-sided end flat whatever `endcap` says.
- `side=right` gives `POLYGON((0 0,10 0,10 -1,0 -1,0 0))`; `side=left` with distance −1 gives
  the right strip.
- On a polygon, `side=left` is the band outside, `side=right` the band inside.
