<!--
  MobilityDB — Portable Naming: Exact distance
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Exact distance

[Back to the index](00-INDEX.md)

Every double is an exact rational, so a distance between geometries given by their doubles is an
exact quantity, the square root of a rational wherever the nearest points are input vertices or the
feet of perpendiculars on input segments. The answer a double can give is the double nearest it, a
tie going to the double whose last bit is even, and a predicate on a distance (`dwithin`) is the
exact sign of the squared distance less the square of the bound. This document states, for every
distance MEOS computes, whether it answers that double or that sign, measured, and the pull
requests that make it do so. The rule owner is `doc/contributing/distance_design_notes.md`.

## The oracle

A distance is judged by CGAL's exact rational type (`Gmpq`) on the doubles the engine holds, never
by another rounded formula and never by a band. A double `a` is the nearest root of an exact
squared distance `S` exactly when `S` lies between the squares of the midpoints from `a` to its two
neighbours, and a midpoint of two doubles and its square are rationals, so the judge takes no
root. For a radius `r`, the comparison is with the squares of `r` plus the midpoints. A point the
engine constructs (the end of a shortest line) is compared with the exact point within a distance
sized to that construction, 8 * DBL_EPSILON times the largest coordinate and the radius. Every
judge first accepts independent correct answers (MPFR, Python `fractions` with 2000-digit
`decimal`) on every row and refuses the same answers moved one unit in the last place. The judges
and corpora are kept outside the tree; the in-tree witness of each pull request carries constants
the judge accepts.

## What is measured

Measured on MobilityDB master `bc5713429c` and on master `4d55aaf18d`, which holds #2992.

| Distance | Input | Answer | The exact answer |
|---|---|---|---|
| a point pair through the generic any-geometry entry (`geo_distance_fn`) | `(0, 0)`, `(1e39, 0)` | `3.4028234663852886e+38`, the `FLT_MAX` seed of the walk | `1e39` |
| the same | `(0, 0)`, `(1e200, 1e200)` and `(1e-200, 1e-200)` | the seed, and `0` | `1.414213562373095e+200`, `1.414213562373095e-200` |
| a point pair by `hypot` of the rounded differences | 4061 resting pairs (real AIS, near-coincident, random, 3-4-5) | not the nearest double on 50, one unit in the last place off | the nearest double on all |
| a circular buffer at rest against a point, the distance less the radius | 3500 pairs, clear, overlapping, touching exactly and one unit in the last place of the radius either way | not the nearest double on 722, by up to more than ten units in the last place | the nearest double on all |
| `edwithin` of two resting points (`point_within_distance_sign`) | `(1e155, 0)` and the next double, asked within the double below their separation | `1` | `0`: the exact fallback squares the coordinates, which overflows |
| a point against a segment (`dist_segm_edge_mindist`, a constructed projection) | `(0, 0)` against the segment at `y = 1e-9` | `1.0000000000000001e-09` | `1e-09`; the nearest double on 731 of 1090 segment rows |

## What answers a distance

| Function | Answer | Exact |
|---|---|---|
| `point_distance_exact` (`datum_pt_distance2d`, `datum_pt_distance3d`) | the double nearest the distance of two points, 2D and 3D | yes, #2992 |
| `point_distance_offset_exact` (`dist_rest_point`) | the double nearest the distance less a radius | yes, #2992 |
| `tdistance_tgeo_tgeo`, `tdistance_tgeo_geo` at their instants, the synchronous `nad`/`nai`/`shortestline` of two temporal points | the point entries | yes at the instants, #2992 |
| the turning points and the minimum between two instants of a moving pair (`tpointsegm_distance_turnpt`, `nad_tcont_tcont_sync`) | a constructed instant and the distance there | not judged |
| `point_within_distance_sign` and its exact fallback (`edwithin`, `tdwithin` of points) | the sign of the squared distance less the square of the bound | where no product of the coordinates overflows or underflows; at every scale with ED2 |
| `point_segment_distance_offset_exact` (`dist_rest_segment`) | the double nearest the distance of a point to a segment less a radius, 2D | yes with ED4, where the scaled values are within a factor 2^240 of the largest difference; for every finite input with ED7 |
| the swept-disc engine against a segment or an arc (`dist_segm_edge_mindist`, `dist_segm_arc_mindist`, `dist_minfun`) | a constructed foot and a rounded quadratic | no for a moving circular buffer; a centre at rest reads `dist_rest_segment` with ED4, a moving point `dist_moving_segment` with ED3 |
| `dist_moving_segment` (`dist_segm_edge_mindist`, `dist_segm_edge_dt` for a radius of 0) | 0 where the path meets the edge, by exact orientations, otherwise the least of four distances of a point to a segment | yes with ED3: over a census of 6060 rows (60 AIS trips, and 1500 trips each against lines, multipoints and parallel segments whose candidates differ by 0 to 3 units in the last place), 4560 of 4560 judged rows are the nearest double; the rounded quadratic misses 896, up to 3364474 units in the last place off, and answers 2^-19 for a path that crosses the line |
| the prunes of the swept-disc engine (`dist_seg_seg_dist2`, `box2d_distance_sqr` against `dist_unit_thr2`) | a rounded lower bound compared with the square of the running minimum | they move no answer once the edge distances are exact: with ED3 the engine answers the same on all 6060 census rows with and without them. With the rounded quadratic, 96 answers move when they are removed, every one against parallel edges, and the pruned answer is the nearest double on 92 of them, the unpruned one on none |
| `geo_distance_fn` for every other pair of geometries (liblwgeom) | liblwgeom's walk | no |
| the geodetic distances (`datum_geog_distance`) | the spheroid by iteration | the exact answer needs its own definition |

## The plan

Each pull request is built on the one its last column names, and adds to the code and the
witness of the ones it is built on without removing or rewriting any of their lines, so a
reviewer reads each one alone.

| PR | Topic | What it changes | Branch, state |
|---|---|---|---|
| ED1 | The distance of a point pair is the nearest double | `point_distance_exact` and `point_distance_offset_exact` in `geo_funcs.c`; the point entries; the five point-pair sites of `tgeo_distance.c`; `dist_rest_point`, `dist_unit_thr2` and the shortest-line collapse of the swept-disc engine; the witness `meos/test/distance_point_test.c` | MobilityDB #2992, merged as `0cedc36e81` and `4d55aaf18d` |
| ED2 | `dwithin` of a point pair over the whole domain | the exact fallback of `point_within_distance_sign` reads the scaled differences of `point_distance_square_expansion` with the bound as the one term, so coordinates near 1e155 decide as ordinary ones; the `dwithin` comments that call the sign a closed form | MobilityDB #3001, merged as `729342047d` |
| ED4 | A point or a circular buffer at rest against a segment | `point_segment_distance_offset_exact`: the foot of the perpendicular is a rational parameter on the input segment, the squared distance to it a rational, so the distance is the nearest double by the same midpoint comparison; `dist_segm_edge_mindist`, `dist_segm_edge_dt` and the shortest line read it | MobilityDB #3002, merged as `2a1d52a7ee` |
| ED3 | A moving point against a segment | `dist_moving_segment`: the path of a moving point over one segment of its sequence and a straight edge meet or not by exact orientations (`linesegm_intersect`), and two segments that do not meet are nearest at an end of one of them, so the distance is the least of four ED4 distances; `dist_segm_edge_mindist` and `dist_segm_edge_dt` read it for a radius of 0. The prunes of the walk stay as they are: the census answers the same with and without them | MobilityDB #3003, merged as `44cddaaccb` |
| ED5 | Two moving points between their instants | time is read as continuous, never on the microsecond grid: between two consecutive instants of either operand both points move linearly, so the second seen from the first moves along a segment, and the nearest approach over that interval is the distance of the origin to that segment, a rational of the coordinates and the integer timestamps, answered as its nearest double; the ever and always `dwithin` of the pair read the same exact sign over continuous time; the instant returned is dated by the truncating convention the tree shares; a CGAL judge by exact rational relative motion over the merged instants, with accept and refuse controls | MobilityDB #3023, merged as `bdc4e41234` |
| ED7 | Every finite double | MobilityDB #3007, merged as `3ef78a0ad8` from another machine, decides the point kernels exactly on their scaled values, in a two's complement integer of 66 limbs (`square_products_wide`) where an expansion cannot; where the scaling by 2^k itself rounds a difference, its error or the radius below 2^-1074 the kernels still answer the even double of a tie, which the CGAL judges refuse on every such row and on no other. ED7 adds, without rewriting a line of #3007: `point_distance_scale_exact`, read before the scaling, and where it fails every sign read on the input coordinates by #3007's own `point_distance_offset_side_sign` (`point_distance_offset_inputs`); for the segment, whose sign is of degree four, the cross product, the squared length and the squared midpoint each as #3007's integer and their products compared exactly (`wide_product`, `point_segment_side_sign_inputs`), reached where the scaling rounds or leaves a product of four outside the doubles; both input-coordinate paths start their search from a candidate read on the half differences scaled by the power of two that brings the largest into [1/2, 1), so it ends at every scale (a fixed 2^-600 left differences below 2^64 one double per step from the answer); CGAL agrees with 17 witnesses and their 34 accept and refuse controls, among them ties at every scale from 2^-50 to 2^960 | MobilityDB #3004, merged as `a0480a7d38` |
| ED8 | Two static geometries, a moving circular buffer of constant radius, and segments in 3D | the ED3 kernel answers the distance of two segments, so `geo_distance_fn` for two geometries of segments and the swept-disc engine for a buffer whose radius does not change read it, the buffer less its radius where it stands clear; the ED4 kernel in 3D, whose cross product has three components | three PRs in this order; the first, two static geometries of segments, MobilityDB #3029, merged as `c2525bbc10`; the second, a moving circular buffer of constant radius, `fix/moving-buffer-distance-is-the-nearest-double`, MobilityDB #3034, ready, 48 of 48 checks green |
| ED9 | The points and instants the engine constructs | the end of a shortest line is a rational point, each coordinate rounded to its nearest double from the exact quotient, so the judge's band becomes the nearest double; the instant at which a `dwithin` begins or ends is a root of a quadratic with rational coefficients, decided by exact signs as ED4 decides a distance, then converted by MEOS's dating convention | after ED5 |
| ED6 | The cost | a filter for `point_distance_offset_exact` and `point_segment_distance_offset_exact` as `point_distance_exact` has; the circular buffer benchmark run after the correctness of ED1 to ED5 and ED7 to ED9, never as a way of deciding it | last |

The geodetic distances are not in the plan: an exact answer on the spheroid needs a definition
first, which the [geodetic boxes](GEODETIC-BOXES.md) share.

## What other libraries reach, and what it leaves

The exact answer is reached the way the exact geometry libraries reach it: a floating-point filter
that decides most rows, and an exact stage for the rest.

| Library | What it answers exactly | What it gives this plan |
|---|---|---|
| CGAL, exact kernel with lazy evaluation | predicates and constructions on rationals, after an interval filter | the oracle, and the shape of every kernel; its exact stage, a rational of any size, has no domain, which ED7 reaches with integers |
| Shewchuk's adaptive predicates, Geogram's predicate construction kit | signs of polynomials of the coordinates by expansions of doubles | the arithmetic of ED1 to ED4; an expansion fails only where a product underflows, the limit ED7 removes |
| Indirect predicates (Attene) | predicates on a point given by its construction, such as a foot or an intersection, without rounding it | the design for ED3, ED5 and ED8: distances to a constructed point are compared on its construction |
| CORE, LEDA's `real` | the sign of an expression with square roots, by separation bounds | needed only to compare or round a sum of distances, such as a length; no single distance needs it |
| S2 Geometry | the sign of a comparison of distances on the sphere, in double, then long double, then an exact float | the shape of an exact `dwithin` sign on the sphere, should the geodetic boxes define one |
| GeographicLib (Karney) | the spheroid geodesic to about 15 nanometres, not correctly rounded | the practical bound for a distance on the spheroid |
| MPFR, CORE-MATH | correctly rounded transcendental functions, by raising the precision until the rounding is decided | the only route to a correctly rounded geodetic distance, and the controls of the judges |

GEOS, JTS and Boost.Geometry compute distances in floating point, and none of them answers the
nearest double.

What this leaves outside the exact answer:

- a distance on the spheroid, whose value is transcendental in the coordinates: GeographicLib's
  accuracy is what the plan states for it, not the nearest double;
- the instant MEOS returns: a timestamp is a whole number of microseconds, and a crossing is dated
  by truncation, a convention of the tree that ED9 keeps;
- a sum of distances, such as the length of a trajectory, which rounds once per term.

Every other limit of ED1 to ED5 is a property of the kernels, not of the question: the scaled
domains (ED7), the segment pairs and 3D (ED8), and the constructed points and instants (ED9).
