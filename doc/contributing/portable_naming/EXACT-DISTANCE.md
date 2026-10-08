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

Measured on MobilityDB master `bc5713429c` and on the head `1360d90ab5` of #2992.

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
| `point_within_distance_sign` and its exact fallback (`edwithin`, `tdwithin` of points) | the sign of the squared distance less the square of the bound | where no product of the coordinates overflows or underflows |
| the swept-disc engine against a segment or an arc (`dist_segm_edge_mindist`, `dist_segm_arc_mindist`, `dist_minfun`) | a constructed foot and a rounded quadratic | no |
| the prunes of the swept-disc engine (`dist_seg_seg_dist2`, `box2d_distance_sqr` against `dist_unit_thr2`) | a rounded lower bound compared with the square of the running minimum | not measured: a bound rounded above the exact one may drop the nearest edge |
| `geo_distance_fn` for every other pair of geometries (liblwgeom) | liblwgeom's walk | no |
| the geodetic distances (`datum_geog_distance`) | the spheroid by iteration | the exact answer needs its own definition |

## The plan

| PR | Topic | What it changes | Branch, state |
|---|---|---|---|
| ED1 | The distance of a point pair is the nearest double | `point_distance_exact` and `point_distance_offset_exact` in `geo_funcs.c`; the point entries; the five point-pair sites of `tgeo_distance.c`; `dist_rest_point`, `dist_unit_thr2` and the shortest-line collapse of the swept-disc engine; the witness `meos/test/distance_point_test.c` | `fix/temporal-distance-of-a-point-pair-reads-the-point-kernel`, MobilityDB #2992, open |
| ED2 | `dwithin` of a point pair over the whole domain | the exact fallback of `point_within_distance_sign` reads the scaled differences of `point_distance_square_expansion` with the bound as the one term, so coordinates near 1e155 decide as ordinary ones; the `dwithin` comments that call the sign a closed form | after ED1 |
| ED3 | The prunes of the swept-disc engine | first a census: the engine with and without each prune over the corpora, every answer that moves recorded; then a lower bound that is never above the exact one, or a prune that asks a sign | after ED1 |
| ED4 | A point against a segment, and a segment against a segment | the foot of the perpendicular is a rational parameter on the input segment, the squared distance to it a rational, so the distance is the nearest double by the same midpoint comparison; `dist_segm_edge_mindist` and the shortest line read it | after ED3 |
| ED5 | Two moving points between their instants | the squared distance of two linearly moving points is a quadratic in time with rational coefficients, its minimum at a rational instant and of a rational value, so the nearest approach is the nearest double of its root and the instant is decided on the rational; a judge by exact rational interpolation, as the one for `tdwithin` | after ED1 |
| ED6 | The cost | a filter for `point_distance_offset_exact` as `point_distance_exact` has; the circular buffer benchmark run after the correctness of ED1 to ED5, never as a way of deciding it | after ED5 |

The geodetic distances are not in the plan: an exact answer on the spheroid needs a definition
first, which the [geodetic boxes](GEODETIC-BOXES.md) share.
