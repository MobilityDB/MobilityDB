<!--
  MobilityDB — Exactness: The exact arithmetic of the geometry engine
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The exact arithmetic of the geometry engine

**For discussion by the committers. Nothing here is merged.** Master at the time of writing:
`a4ac8e7b92`. Every function, type and constant is cited by name and file; every count is read on
that master.

## 1. Summary

Every exact answer of MEOS, a distance, a predicate, a relationship, a buffer or a hull, ends in
signs of polynomials of the input doubles. Those signs are computed by one arithmetic made of
tiers: a value in doubles with a bound on its error, a pair of doubles with a bound, an expansion
of doubles, and a wide dyadic number. A sign is read in the cheapest tier whose bound decides it,
and the last tier decides every finite input.

This note states that arithmetic as one layer of the engine, and the rule that keeps it one:

- **A. Three layers.** The user entries, the geometric algorithms, and the exact arithmetic
  (Section 2). The exact arithmetic is spread today over `meos/src/geo/geo_funcs.c` and
  `meos/include/geo/geo_funcs.h`, interleaved with the algorithms that use it.
- **B. One machinery per need.** The engine holds three tier ladders, two pair-of-doubles
  types, three wide-number types and nine searches for the nearest double, and of the 32
  geometric questions its 589 exact functions answer, 22 are answered by two or more functions
  built apart, which disagree on domain, on what "undecided" is, on sign conventions and on
  degenerate input; six of them keep a tolerance beside an exact sign for the same question
  (Section 4). Nothing stops a new exact question from adding its own; the rule of Section 5
  does, and the plan of Section 7 merges what exists, the defects first.
- **C. A seam at the user entries.** An entry checks its arguments and calls the engine once
  (Section 6). That seam keeps another geometry library possible behind the same entries; no
  such library is planned, and GEOS stays a test witness.

## 2. The three layers

| Layer | What it holds | Where it lives |
|---|---|---|
| User entries | The functions a caller of MEOS reaches, most of them named after their PostGIS counterpart: argument checks, then one call into the engine | `meos/src/geo/postgis_funcs.c` (70 entries carry `@note PostGIS function`), a few in `geo_funcs.c` and `geo_buffer.c` |
| Geometric algorithms | The distance walks, the relate engine (DE-9IM), the arrangement of edges and arcs, the buffer, the hulls, the centroid, the enclosing circle | `meos/src/geo/geo_funcs.c`, `geo_arrange.c`, `geo_buffer.c`, `postgis_funcs.c` |
| Exact arithmetic | The number representations, the tiers, and the exact signs every algorithm reads: orientation, incircle, the offset of a point from a segment or a plane, the meeting of an arc and a segment, the numbers of a tower of square roots | `meos/src/geo/geo_funcs.c` and `meos/include/geo/geo_funcs.h` |

An algorithm never computes a sign itself: it asks the arithmetic layer for it. The arithmetic
layer knows no geometry type: its inputs are doubles and arrays of doubles.

## 3. The tiers

### 3.1 Tier 0: doubles with a bound

The value is evaluated in doubles, and a bound on its error is computed from the magnitudes of
its terms, as Shewchuk bounds a determinant. Where the value stands clear of its bound its sign
is exact. The bound is a filter, never a tolerance: it decides whether the double can be trusted,
never what the answer is. Most calls end here.

- Written per predicate, inline, because it is the hot path: `cross_product_sign`,
  `dot_product_sign` and `triple_product_sign` are `static inline` in `geo_funcs.h`, each
  calling its exact stage, defined in `geo_funcs.c`, only where the bound cannot tell.
- Generic forms: `ArcsegFilter` (a value and its bound) with the `arcseg_f_*` operations, and
  the first pass of `polynomial_sign_filter`.
- Range: every term between 2^-900 and 2^900, or as the predicate states; outside it the tier
  declines and the next one reads the value.

### 3.2 Tier 1: pairs of doubles with a bound

The value is held as the unevaluated sum of two doubles, formed by the error-free transforms
`two_diff`, `two_product` and `two_sum`, with a bound on its distance to the exact value: about
104 bits. An exact zero, a pair of zeros with no error, is an exact zero, which tier 0 cannot
tell.

- `DDBound` with the `ddb_*` operations (`ddb_add`, `ddb_mul`, `ddb_dot3`, `ddb_cross3`,
  `ddb_sign`, `ddb_is_zero`).
- The pair functions `pair_mul`, `pair_add` and `pair_mid_sign`, whose bound the caller carries,
  and `polynomial_sign_pairs` and `incircle_sign_filter`, which carry their own.

### 3.3 Tier 2: expansions

The value is an expansion, a sum of doubles that do not overlap, held in increasing order of
magnitude, whose last component carries the sign of the whole (Shewchuk). Sums and products are
exact: `grow_expansion`, `scale_expansion`, `expansion_sum`, `expansion_product`, and
`compress_expansion` keeps them short.

- Range: exact where no product of components overflows or underflows. A degree-three sign of
  differences each within 2^-200 and 2^200 stays inside it (`vector_triple_sign`); a sign of
  degree six needs a narrower range, as `segment_square_least` states for its scaled values
  (lowest bit at or above 2^-179).
- Memory: `polynomial_sign_stack` forms each term in an arena of `POLY_STACK_ARENA` (2048)
  doubles and the total in two buffers of `POLY_STACK_TOTAL` (1024); `ArcsegArena` holds
  `ARCSEG_ARENA_SIZE` (4096) doubles on the stack and takes blocks past them. An arena records
  when a product may have left the exact range (`unsafe`), and the answer of that evaluation is
  discarded for the next tier's.

### 3.4 Tier 3: wide dyadic numbers

The value is a dyadic rational, a sign, an integer magnitude in 64-bit limbs and the weight of
its lowest bit. Every finite double is one, and sums and products are exact at any magnitude:
this tier decides every finite input.

- `WideDyadic`: a fixed capacity of `WIDE_DYADIC_LIMBS` (320) limbs, so one value takes about
  2.5 KB and a copy moves all of it; `wide_dyadic_add`, `wide_dyadic_mul`,
  `wide_dyadic_product_cmp`, `polynomial_sign_wide`.
- `TowerWide`: the same number with as many limbs as the value takes, allocated in the blocks of
  a `TowerCtx`.
- The two's complement integer of `WIDE_LIMBS` (66) limbs that `square_products_wide` sets, with
  `wide_add`, `wide_product`, `wide_magnitude` and `wide_sign`, for a sum of products of two
  doubles.

### 3.5 Rounding: the double nearest an exact value

A distance is answered as the double nearest its exact value. The double is found by comparing
the exact value with the midpoints between candidate doubles, each comparison an exact sign, so no
rounded value decides it. `nearest_double_exact` does this for any value a side function reads
exactly: a gallop from a candidate brackets the answer, and halving the bracket finds it.

### 3.6 What the tiers cost

Measured by callgrind on the cell "refuters at" of ED24 (the refuting face corpus, 320 rows, each
asked at the double nearest its distance), on the branch of MobilityDB #3484 over master
`9695710930`, with the probe `perf_facedwithin3d`, which times `geom_dwithin3d` against
`lwgeom_mindistance3d_tolerance` in one process:

| Where | Cost |
|---|---|
| The whole cell, native | 3.42M instructions, against 1.67M for `lwgeom_mindistance3d_tolerance` called directly (2.05) |
| Tier 3 (`WideDyadic`), every function together | 0.63M, 19% of the cell |
| One tie of `plane_point_offset_sign` read in tier 3 | about 12K instructions; the 8 of its 32 calls that reach tier 3 take 0.10M |
| One row of lwgeom, for scale | about 5.2K instructions |

A single tie read in tier 3 costs more than lwgeom's whole answer to a row. A tier 2 between
tier 1 and tier 3 is what keeps a tie off tier 3.

## 4. The machinery the engine holds

### 4.1 Three tier ladders

| Ladder | How a question is written | Tiers | Call sites |
|---|---|---|---|
| The expression ladder: `tower_pred_run` over a `TowerCtx`, values `TowerX`, operations `tx_diff`, `tx_add`, `tx_sub`, `tx_scale`, `tx_mul`, `tx_sign`, `tx_zero` | Once, as a function of the `tx_*` operations (`TowerPred`); the same code runs in every tier | 0 (`ArcsegFilter`), 1 (`DDBound`), 2 (expansions in an `ArcsegArena`), 3 (`TowerWide`), each read where the one before cannot tell or leaves its range | 17 calls of `tower_pred_run` |
| The polynomial ladder: `polynomial_sign_exact`, then `polynomial_sign_wide` where the caller says the range is left | As a list of terms (`PolyTerm`: a coefficient times up to `POLY_TERM_MAXDEG` (6) exact sums) over exact sums (`ExactSum`: up to `EXACT_SUM_MAXTERMS` (4) doubles held as an expansion) | 0 (`polynomial_sign_filter`), 1 (`polynomial_sign_pairs`), 2 (`polynomial_sign_stack`, then the heap), 3 (`polynomial_sign_wide`, read by `segment_square_cmp` alone) | 54 calls of `polynomial_sign_exact`, 1 of `polynomial_sign_wide` |
| Hand-written ladders | Each predicate restates its value in each tier it has | as each one has them (Section 4.4) | each predicate its own |

The expression ladder is the most general. It takes any expression, not only an expanded
polynomial, it has all four tiers, and its tier 3 holds a value of any size. The polynomial
ladder is the most used.

### 4.2 Two pair-of-doubles types

`DDBound` carries its bound in the value; `pair_mul`, `pair_add`, `pair_mid_sign`, the pairs of
`polynomial_sign_pairs` and of `incircle_sign_filter` (`incircle_pair`) leave it to the caller.
Both stand for the same tier.

### 4.3 Three wide-number types

`WideDyadic` (fixed capacity), `TowerWide` (the size of the value) and the `WIDE_LIMBS` integer
of `square_products_wide` all hold an exact dyadic number in 64-bit limbs. `TowerWide` is the
general one: `WideDyadic` is the same number at a fixed capacity, and an integer of `WIDE_LIMBS`
limbs is one whose lowest bit has a fixed weight (`WIDE_LOWEST_BIT`).

### 4.4 The exact signs and the tiers each one has

| Sign | Degree | Tier 0 | Tier 1 | Tier 2 | Tier 3 | Calls |
|---|---|---|---|---|---|---|
| `cross_product_sign` (orientation in the plane) | 2 | inline in `geo_funcs.h` | | `cross_product_sign_exact`, scaled | `cross_product_sign_wide` | 109 |
| `dot_product_sign` | 2 | inline | | `dot_product_sign_exact` | none | 1 |
| `triple_product_sign` | 3 | inline | | `triple_product_sign_exact` | none | 6 |
| `incircle_sign` | 4 | in doubles | `incircle_sign_filter` | `incircle_sign_exact` | none | 3 |
| `vector_triple_sign` | 3 | none | `DDBound` | `triple_product_sign_exact` where every difference is exact | `WideDyadic` | 7 |
| `plane_cross_dot_sign` | 4 | none | `DDBound` | none | `WideDyadic` | 3 |
| `plane_point_offset_sign` | 6 | none | `DDBound` | none | `wide_dyadic_product_cmp` | 1 |
| `segment_interior_offset_sign` | 6 | none | none | none | `segment_interior_wide_values` | 1 |
| `point_segment_offset_sign` | 4 | none | none | `square_products_expansion` where `square_products_exact` holds | `square_products_wide` and `wide_product` | 6 |
| `segment_square_cmp` | 6 | `polynomial_sign_exact` | `polynomial_sign_exact` | `polynomial_sign_exact` | `polynomial_sign_wide` | 1 |
| the signs of an arc and a segment, of two arcs, of a tower | up to 8 | the expression ladder | the expression ladder | the expression ladder | the expression ladder | 17 |

The branch of MobilityDB #3484 (draft) adds tier 0 to `vector_triple_sign`,
`plane_point_offset_sign` and `segment_interior_offset_sign`, tier 1 to
`segment_interior_offset_sign` (`segment_interior_offset_filter`) and to
`point_segment_offset_sign` (`point_segment_side_sign_filter`); each is written in the predicate,
by hand, as Section 5 asks no new one to be.

Two gaps follow from the table:

- **No tier 2** in `plane_cross_dot_sign`, `plane_point_offset_sign` and
  `segment_interior_offset_sign`: a tie goes to tier 3, at the cost of Section 3.6.
- **No tier 3** in `dot_product_sign`, `triple_product_sign` and `incircle_sign`: their exact
  stages state they are exact where no product overflows or underflows, and their bodies call
  no stage of wide numbers after their expansions.

### 4.5 Nine searches for the nearest double

`nearest_double_exact` (any exact side), `wide_dyadic_nearest_root` (the root of a quotient of
`WideDyadic`), `ddb_nearest_root` (a root placed by `DDBound` residuals), `point_distance_nearest`,
the candidate moves of `point_distance_offset_filter`, the searches inside
`point_distance_offset_pair` and `point_distance_offset_inputs`, inside
`point_segment_distance_offset_exact` and its `_inputs` twin, and `mec_nearest` all find the
double nearest an exact value by moving a candidate across the midpoints to its neighbours.
`wide_dyadic_nearest_root`, `mec_nearest` and `nearest_double_exact` state the same gallop and
halving.

### 4.6 The same question answered more than once

Every function of the engine files that decides or computes exactly (589 on `a4ac8e7b92`, read by
its contract and, where the contract is silent, its body) is placed by the question it answers.
32 geometric questions come out, besides the arithmetic of Section 3; 22 of them are answered by
two or more functions built apart. The table keeps the questions with more than one
implementation; "survivor" is the one covering every finite double with the most callers.

| Question | Implementations | Survivor | What the others are |
|---|---|---|---|
| Orientation of three points in the plane | `cross_product_sign`, `cross_product_exact`, `relate_orientation`, `seg2d_side`, `certa_sum2_sign`, the inline sign of `cert_arc_minor`, `sum_point_orient`, `arr_cross_ival` | `cross_product_sign` (109 calls) | `cross_product_exact` has no caller; `certa_sum2_sign` and the sign of `cert_arc_minor` read `cross_product_sign` from C7 (fix/dot-sign-one-contract); before it, the same sign on the polynomial ladder, exact only in its range; `relate_orientation` stops at its filter, so its 0 is collinear or undecided; `seg2d_side` reads `cross_product_sign` with the opposite sign from #3593 (C5); before it, `MEOS_EPSILON` on the cross product |
| Orientation of four points in 3D | `vector_triple_sign`, `triple_product_sign` and its `_exact` | `vector_triple_sign` | `triple_product_sign` is exact only where no product overflows or underflows |
| Sign of a dot product | `point_segment_dot_sign_inputs`, `dot_product_sign` and its `_exact` | `point_segment_dot_sign_inputs` | `dot_product_sign` is exact only in its range |
| Do two segments meet | `linesegm_intersect`, `wide_segment_meet`, `relate_edges_cross`, `cert_segs_cross_exact`, `cert_segs_meet` | `linesegm_intersect` | each calls a different set of contacts a meeting (Section 4.7) |
| Two points within a distance, a point against a centred circle | `point_within_distance_sign`, `point_distance_square_sum_sign`, `point_distance_square_sign_exact`, `point_distance_offset_pair_side_sign`, `certa_longer`, `centred_circle_side` | one sign of `\|p - q\|^2 - (t1 + ... + tn)^2` on `square_products_sign` | the first four differ only in signature; `certa_longer` and `centred_circle_side` read it on the polynomial ladder, in its range |
| A point against the circle through three points | `incircle_sign`, `arc_circle_side`, `point_arc_circle_side_pred` | none yet | `incircle_sign` has no tier 3; the two arc forms answer the opposite sense |
| A point within a distance of a segment | `point_segment_offset_sign`, `point_segm_within`, `geom_dwithin_end`, `cert_point_seg_far_exact` and `_slow`, `point_segment_beyond` and `_inputs` | `point_segment_offset_sign` | `point_segm_within` and `geom_dwithin_end` are one filter and one call; the certificates read it on the polynomial ladder |
| The double nearest the distance of a point to a segment | `point_segment_distance_offset_exact`, `point_segment_distance_offset_inputs` | the first, with the second as its wide route | |
| The double nearest an exact value | the nine searches of Section 4.5 | `nearest_double_exact` | each other one is a side function it reads |
| Ring orientation | `ring_area_sign`, `buffer_ring_signed_area`, `cert_ring_keeps_sense` | `ring_area_sign` | `buffer_ring_signed_area` is in doubles |
| Point in polygon | `point_in_rings_exact`, `point_arc_ray_parity`, `dist_poly_seg_raycross`, `dist_geom_point_inside`, `pip_point_in_ring` | `point_in_rings_exact` and, for arcs, `point_arc_ray_parity` | from #3575 (C2) the other three read the exact sign (`pip_point_in_ring`) or the exact parities (`dist_poly_seg_raycross`, through `exact_arc_from_edge`); before it, doubles, `MEOS_GEOM_TOLERANCE` and `atan2` |
| The circle of an arc | `arcarc_centre_exact` and `arcarc_radius2_exact`, `arr_arc_centre_y_side`, `mec_exact_set` and `mec_beyond`, `exact_arc_bound`, `tower_arc_read`, `hull_exact` | `arcarc_centre_exact` | six arc models form the same circumcentre (Section 4.7) |
| A segment against an arc | the `arcseg_*` meetings, `tower_point_line_circle`, `arcsegm_cross` | the `arcseg_*` meetings on input segments, the tower on tower points | `arcsegm_cross`, which relate reads, answers by `arcsegm_meet_exact` from #3567 (C1); before it, a `1e-14` band, `MEOS_GEOM_TOLERANCE` and `atan2` read three proper crossings as none |
| Two arcs | `arcarc_meet_exact`, `tower_point_circles`, `arcarc_cross` | `arcarc_meet_exact` | `arcarc_cross`, which relate reads, answers by `arcarc_meet_exact` from #3595 (C6, merged), which also reads the second half of a whole circle from its other end, as `arcsegm_meet_exact` does; before it, a band on the half-chord and `MEOS_GEOM_TOLERANCE` on the fraction. The meeting points `relate_arc_arc_points` places stay in doubles |
| Within a distance of an arc | `point_arc_offset_sign` and its segment and arc siblings, the `*_within_pred` family on the expression ladder, the far certificates of the buffer | none yet | one family per arc model |
| The order of edges leaving a point | `edges_turn_cmp_exact`, `edges_turn_cmp_meet`, `arr_turn_cmp` | none yet | they differ in how the point is held: doubles, a tower meeting, intervals |
| The order of meetings along an edge | `meets_order_tower`, `meets_order_scaled` | the expression ladder | `meets_order_scaled` is the arena twin |
| A meeting point held exactly | `meet_tower_build`, `meet_point_exact` | the expression ladder | `meet_point_exact` is the arena twin |
| Nearest approach of two motions | `point_motion_nad_exact`, `point_motion_point_offset_exact`, `convex_motion_nad_exact` | none yet | two time models (Section 4.7) |
| Within a distance over time | `point_motion_dwithin_exact`, `point_motion_dwithin_period_exact`, `tcbuffer_region_within_roots` | `point_motion_dwithin_period_exact`, which states a point as a disc of radius 0 | from #3588 (C4, merged) the periods of a point or a disc against a geometry of segments come from one engine, `motion_edges_dwithin_periods`, built on the period logic of `point_motion_dwithin_period_exact`; `tcbuffer_region_within_roots`, in doubles with a `1e-18` tolerance, answers only a geometry carrying an arc |

### 4.7 What the duplicates disagree on

Built apart, the implementations of one question drift apart in contract:

- **Domain.** The polynomial ladder (54 calls from 25 functions) is exact only where no product
  overflows or underflows, and only `segment_square_cmp` goes on to `polynomial_sign_wide`; the
  twins it duplicates (`cross_product_sign`, `point_segment_offset_sign`,
  `point_within_distance_sign`) are exact for every finite double. EA1 measures the consequence: 50
  of 228 calls answered 0 for -1.
- **A tolerance beside an exact sign for the same question**: relate's crossings of a segment
  and an arc (`arcsegm_cross`, #3567) and of two arcs (`arcarc_cross`, #3595) answer by the exact
  arc meetings; `buffer_ring_signed_area` against `ring_area_sign`; and, for a geometry carrying an arc,
  `tcbuffer_region_within_roots` against `point_motion_dwithin_period_exact` (a geometry of
  segments is answered exactly from #3588). The ray casts
  `dist_poly_seg_raycross` and `pip_point_in_ring`, and with them the `inside` of
  `tcbuffer_disc_boundary_sign`, read exact signs from #3575.
- **What "undecided" is.** Seven spellings: 0, 2, 3, -1, -2, false, and "no crossing".
- **Opposite senses.** `seg2d_side` answers -1 where `cross_product_sign` answers 1;
  `arc_circle_side` and `point_arc_circle_side_pred` answer -1 inside where `incircle_sign`
  answers 1.
- **The same digits, different meanings.** `point_motion_dwithin_period_exact` returns 2 for
  within over a period and 1 for one instant; its filter returns 1 for the whole segment and 2 for
  some instant.
- **Degenerate input.** `linesegm_intersect` lets a first segment of no length meet nothing;
  `wide_segment_meet` reads it as a point. On collinear segments `cert_segs_cross_exact` answers
  true, `relate_edges_cross` false.
- **Bounds.** One expression, the sum of two products of differences, is filtered under five
  different bounds (`cross_product_sign`, `relate_orientation`, `arr_cross_ival`,
  `point_segment_dot_sign_inputs`, `sum_point_orient`).
- **Models.** Six arc models (`Edge`, `ExactArc`, `ArcsegExact`, `TowerArc`, `MecExact`, the hull
  feature), two angular origins (+x for the turn of edges, (0, -1) for the hull), two time models
  (integer timestamps for two points in 2D, a fraction of a segment of microseconds for discs in
  2D and 3D), five carriers of tiers 0 and 1 (`DDBound`, the caller-bounded pairs,
  `ArcsegFilter`, the intervals of `geo_arrange.c`, `SumPoint`).

## 5. The rule

**One machinery per need.** A new exact question is written on what the arithmetic layer
holds:

1. **Its sign is an expression of the input doubles, written once**, and the one ladder reads it
   in every tier. A question restates its value per tier only in its tier 0, where an inline
   filter specific to it is the hot path.
2. **Every sign has every tier.** Tier 0 or 1 decides most calls, tier 2 keeps ties off tier 3,
   and tier 3 makes it exact for every finite input. A sign missing a tier is a gap of this
   layer, closed here and not in the predicate.
3. **One type per tier**: one bounded value in doubles, one pair of doubles, one expansion with
   its arena, one wide number.
4. **One search for the nearest double**, reading any exact side.
5. **A question an algorithm needs and the layer lacks is added to the layer**, as a sign of the
   arithmetic, with its tiers, where every other algorithm reaches it.

Section 7 merges the machinery of Section 4 into this one, a step per pull request.

## 6. The seam at the user entries

A user entry checks its arguments and calls the engine once: `geom_dwithin3d` validates its
operands, dispatches by dimension and type, and asks `geom_dwithin3d_faces` or the distance walk
for the answer. Kept that way, the engine behind the entries can change without changing any
entry or any caller.

- **What the seam allows.** Another geometry library behind the same entries, such as a build
  that answers through GEOS or CGAL, needs only the engine calls replaced. Its answers would
  differ where the libraries differ: MEOS answers the double nearest the exact value and reads
  circular arcs and faces in 3D as they are, which GEOS does not.
- **What is decided.** No such build is planned. Once every entry answers natively, GEOS leaves
  the shipped library and stays the witness of the tests where straight segments are compared;
  on master MEOS links no GEOS, and the stand-in header `meos/include/geos-none/geos_c.h` is
  all that remains of it.
- **What the seam asks.** An entry holds no geometry of its own: no walk, no sign, no loop over
  points. The entries of `postgis_funcs.c` are kept to that.

## 7. Plan

Each step is one pull request, judged as every pull request of these notes is: CGAL on the
head's answers, and the speed cells of every operation it touches, measured against GEOS called
directly.

### Order of the work

The consolidation comes before any new speed: a mechanism added for one question waits until the
layer holds it for every question. The phases, each a series of pull requests:

1. **The tolerances beside exact signs (C1 to C6).** Each question of Section 4.7 that keeps a
   tolerant route beside an exact one is answered by the exact one: relate's segment-arc crossing
   `arcsegm_cross` by the exact arc meetings (C1, #3567, merged) and its arc-arc crossing
   `arcarc_cross` likewise (C6, #3595, merged), the ray casts in
   doubles (`dist_poly_seg_raycross`, `pip_point_in_ring`, the `inside` of
   `tcbuffer_disc_boundary_sign`) by `point_in_rings_exact` and `point_arc_ray_parity` (C2,
   #3575, merged), the ring
   area in doubles of the buffer by `ring_area_sign` (C3, paused), the within-times of a moving
   disc in `tcbuffer_region_within_roots` by `point_motion_dwithin_period_exact` (C4, #3588,
   merged, for a geometry of segments; one carrying an arc follows), and `seg2d_side` by
   `cross_product_sign` (C5, #3593, merged). These change answers, where the tolerance answers
   wrongly, so each comes with a witness CGAL judges. Relate on arcs near an end
   (fix/relate-arc-near-end): whether two boundary edges meet is decided by
   `arcsegm_meet_exact` and `arcarc_meet_exact`, the latter reading its signs in doubles
   carrying a bound on their error first, and the start of
   an uncut edge standing outside the other area is read exactly where the boundary walk skips an
   interval narrower than its band; on 396 pairs of curve polygons whose arcs cross or miss
   2^-38 to 2^-50 of the radius from an end, intersects agrees with CGAL on 396 of 396 (370
   before) and the areal relationships on 1188 of 1188 (1126 before), every relate cell at
   parity.
2. **The duplicates of one contract (C7 onwards).** The functions Section 4.6 shows answering the
   survivor's contract become calls to it, one question per pull request, each proven by a
   differential: `cross_product_exact` (no caller), `certa_sum2_sign` and the sign of
   `cert_arc_minor` (to `cross_product_sign`, the cross product of (a, c) and (-d, b): C7,
   fix/dot-sign-one-contract, its differential 1960 rows, the two agreeing with
   `cross_product_sign` between 2^-300 and 2^300 and answering 0 for 1 or -1 on 580 and 400 rows
   beyond, where a product of the ladder leaves the doubles), `triple_product_sign` and `dot_product_sign` (to
   `vector_triple_sign` and `point_segment_dot_sign_inputs`), `certa_longer` and
   `centred_circle_side` (to the one sign of `|p - q|^2 - (t1 + ... + tn)^2`),
   `cert_point_seg_far_slow`, `point_segm_within` and its filter (to `point_segment_offset_sign`).
   Where the duplicate is exact in a narrower range than its survivor, the merge changes answers
   outside that range only, as EA1 finds for the polynomial ladder.
3. **One convention.** One meaning of "undecided" inside the layer (2) and one answer outside it
   (-1, 0, 1); one sense per question (`arc_circle_side` and `point_arc_circle_side_pred` take the
   sense of `incircle_sign`); one rule for a segment of no length (a point, as
   `wide_segment_meet` reads it); one angular origin. Each is a pull request touching every caller
   of what it changes.
4. **One arithmetic (EA1, EA4 to EA8).** One ladder, one pair type, one wide number, one search,
   every sign with every tier; then one arc model and one time model, each merge after a
   differential; then the double nearest a transcendental value (EA8), read by the one search.
5. **The arithmetic in its own file (EA3)**, moved in the shape the merges give it, so that each
   function moves once. The signs on coordinates shifted by multiples of 360 (EA9, #3577, merged) come
   ahead of it, in `geo_funcs.c`, since the cell walks read them, and EA3 moves them with the rest.

MobilityDB #3484 and EA2, its tier for the signs of degree six, come after: EA1 shows that no tier
of the engine brings a tie of degree six near the cost of lwgeom's answer to a row, so its speed
gate rests on the route it takes, not on a tier.

### EA1. Which ladder is the one ladder

The expression ladder and the polynomial ladder each answer what the other answers; one of them
is kept. The step measures the cost of one sign read both ways, on the same inputs, before
anything moves:

- `segment_square_cmp`, a degree-six polynomial on the polynomial ladder, written as a
  `TowerPred`;
- `plane_point_offset_sign`, written both ways, on the ties of "refuters at".

The expression ladder is the proposal, being the general one, provided its cost per sign is at
most that of the polynomial ladder. Two of its costs the polynomial ladder does not have: every
`tx_*` operation branches on the tier of its context, and `tower_pred_run` evaluates the whole
question again in each tier it reaches, where the polynomial ladder forms its terms once. Whether
they matter on a short sign is what the measurement decides. The ladder kept takes a
name of the arithmetic, not of the towers it first served.

**The measurement.** Every call the speed cells make to `polynomial_sign_exact` and to
`plane_point_offset_sign` on master `a4ac8e7b92` is recorded by a shim preloaded ahead of
`libmeos` (both are reached through its PLT), then answered by each form of its sign in a pass
of its own under callgrind. The forms, added to a measurement build only:

- the polynomial ladder itself (`polynomial_sign_exact`), and the same terms on the expression
  ladder (`tower_pred_run` over the terms);
- for `plane_point_offset_sign`: the function (`DDBound`, then `WideDyadic`), its value on the
  expression ladder with the normal formed once, and its value expanded into 45 terms of degree
  six on the polynomial ladder;
- each question's own first tiers (`polynomial_sign_filter`, or the `DDBound` tier of the
  function) ahead of the expression ladder from its expansions on, the shape EA2 gives a sign.

Calls recorded: buffer of the natural areas at distance 1 (716 polynomials), the enclosing circle
of regular polygons (936), the face corpora of ED24 at the distance (40 polynomials and 11 009
calls of `plane_point_offset_sign`), the refuting rows of ED21 (14). The union, relate and
curve-distance cells make neither call.

| Question | Calls | Polynomial ladder | Expression ladder | Own first tiers, then expression ladder |
|---|---|---|---|---|
| polynomials of the buffer | 716 | 459 | 1 812 | 455 |
| polynomials of the circle | 932 | 4 211 | 8 426 | 4 205 |
| polynomials of the circle, ties | 4 | 9 180 | 18 997 | 15 634 |
| polynomials of the faces, ties | 8 | 31 438 | 62 423 | 54 035 |
| polynomials of ED21, ties | 6 | 10 410 | 6 953 | 19 356 |

Instructions per call. A tie is a call whose exact sign is 0.

| `plane_point_offset_sign` | Calls | The function | Expression ladder, normal formed once | Own `DDBound` tier, then expression ladder | 45 terms on the polynomial ladder |
|---|---|---|---|---|---|
| refuting faces, ties | 104 | 12 877 | 10 212 | 14 626 | 84 524, 50 calls answered 0 for -1 |
| refuting faces, others | 124 | 5 929 | 13 498 | 12 037 | 44 079 |
| random faces | 2 991 | 2 109 | 1 691 | 2 050 | 9 593 |
| real faces | 7 790 | 1 376 | 1 669 | 1 327 | 8 663 |

What it shows:

1. **The expression ladder is exact unaided; the polynomial ladder is not.** Every form on the
   expression ladder agrees with the function on all 11 009 calls and with the polynomial
   ladder on all 1 706 polynomials. The 45-term form answers 0 for -1 on the 50 calls asked at
   the least subnormal distance, where `d^2` underflows in the expansions: the polynomial ladder
   is exact only where its caller keeps the range, as `segment_square_least` does before
   choosing `polynomial_sign_wide`.
2. **How a question is written costs more than which ladder reads it.** Expanded into monomials,
   `plane_point_offset_sign` costs 4 to 7 times the function; written with its normal formed
   once, on the expression ladder, 0.79 to 1.21 times it on the ties, the random and the real
   faces.
3. **On the same terms, the polynomial ladder is the cheaper.** The expression ladder costs 2.0
   to 4.1 times it where the first tiers decide; with the question's own first tiers ahead of
   it, the two cost the same there. On the ties the expression ladder from its expansions on
   costs 1.5 to 1.9 times the polynomial ladder's expansions on the stack.
4. **No tier 2 of the engine brings a tie of degree six near lwgeom's cost of a row** (about
   5.2K instructions on the face corpus): a tie costs 10 to 15K on every form.

The condition of the proposal is not met as it stands: the expression ladder is the one way to
write a question (results 1 and 2), and its tiers past the first cost more than the polynomial
ladder's (result 3).

**The expansions of the expression ladder, rebuilt.** Branch `perf/expansions-kept-compressed`
forms each sum and product of `arcseg_x_add` and `arcseg_x_mul` in the room past the arena's live
values and keeps only its compressed result (`arcseg_arena_room`, `arcseg_arena_keep`), as
`polynomial_sign_stack` lays out a term. On the same recorded calls every pass that reaches the
expansions costs 0.90 to 0.98 of master's, every other pass is unchanged, and every form agrees.
What remains between the two ladders on the ties is the work of `arcseg_x_mul` around each
product, about 112 instructions of its own (the operands and result copied by value, the checks of
an unsafe or wide arena, the range of the components), which a tie of about 168 products turns
into the whole gap. Handing the operands by address through an inline form and reading the range in
one comparison saves 0.6 to 2% more, the copy moving to the caller: not kept. The range checks are
what makes the expression ladder exact unaided, so the gap is their price.

**The proposal for the committers (Section 8):** the expression ladder is the one ladder, with
each question's own first tier ahead of it, which costs as the polynomial ladder where the first
tiers decide; a tie costs about 1.5 times the polynomial ladder's, and is exact for every finite
double.

### EA2. Tier 2 for the signs of degree six

`plane_point_offset_sign` and `segment_interior_offset_sign` read their ties on the ladder EA1
keeps, in place of going from `DDBound` to `WideDyadic`; `plane_cross_dot_sign` follows. This is
what MobilityDB #3484 needs to bring "refuters at" toward parity, and the distance walk of ED20,
ED21 and ED23 reaches `segment_interior_offset_sign` too.

### EA3. The arithmetic in its own file

A pure move: the representations, the tiers, the ladders, the searches and the exact signs leave
`geo_funcs.c` for a file of their own, with a header of their own; the algorithms stay. The
filters of tier 0 and the operations of tiers 0 and 1, `static inline` or
`pg_attribute_always_inline` today (116 such definitions in `geo_funcs.c`), go into the header as
`static inline`, so that every algorithm file inlines them as `geo_funcs.c` does. The step moves
no code but declarations and definitions, and its speed cells show no cell slower than before.

The build of MEOS has no link-time optimisation (no `-flto` or `INTERPROCEDURAL_OPTIMIZATION` in
its CMake files): a function defined outside the file of its callers is not inlined in them, and
no test sees the difference. The step therefore counts, for each moved function, its calls left in the
machine code at every call site, before and after the move (`objdump -d` for the calls inside a
file, `objdump -dr` for the `R_X86_64_PLT32` relocations across files), and no count rises. The
`inline` keyword decides nothing here: the count does.

The move waits for a window with no open pull request on these files: on `a4ac8e7b92`,
MobilityDB #3484 and #3557 change `geo_funcs.c` and `postgis_funcs.c`, and the steps of
[Exact buffer](EXACT-BUFFER.md) from 18 on change `geo_funcs.c`, `geo_arrange.c` and
`geo_buffer.c`. The names of the file and of its header are a decision of the committers
(Section 8).

### How a merge is proven

EA4 to EA6 each fold two or more implementations of one thing into one. Each starts with a
differential: the implementations exposed side by side in a measurement build and answered on
the same inputs, the calls the speed cells make recorded as EA1 records them, together with the
cases where the arithmetic is hard (exact ties, subnormal and huge magnitudes). A pair that
agrees everywhere is the evidence for the merge; a pair that disagrees is a defect, fixed first
in its own pull request. EA1 is such a differential, and it finds one: the polynomial ladder
answers a subnormal distance wrongly where its caller does not keep the range.

### EA4. One pair type

`DDBound` stands for tier 1: `pair_mul`, `pair_add`, `pair_mid_sign`, `incircle_pair` and the
pairs of `polynomial_sign_pairs` become `ddb_*` operations, their bounds carried in the value.

### EA5. One wide number

`TowerWide`, the size of its value, stands for tier 3: `WideDyadic` and the `WIDE_LIMBS` integer
of `square_products_wide` become it. A value of tier 3 then costs the limbs it holds, not 320.

### EA6. One search for the nearest double

`nearest_double_exact` stands for the search: `wide_dyadic_nearest_root`, `ddb_nearest_root` and
the candidate moves of `point_distance_offset_filter` become side functions it reads.

### EA7. Every sign with every tier

The gaps of Section 4.4 close on the one ladder: tier 2 where it is missing (after EA2), tier 3
for `dot_product_sign`, `triple_product_sign` and `incircle_sign`, each with a witness past the
range of its expansions.

### EA8. The double nearest a transcendental value

The one search (EA6) rounds a value its exact signs can compare, a polynomial in doubles. The cell
walks ([MobilityDB #3544](https://github.com/MobilityDB/MobilityDB/pull/3544), decisions D6 and
D7) need the double nearest a transcendental value: the cosine and sine of input degrees, a
geodetic position being the vector of the correctly rounded cos(lat)·cos(lon), cos(lat)·sin(lon)
and sin(lat); the longitude and latitude of an exact S2 corner, the angles of a rational point; and
the latitude of a QUADBIN row, atan(sinh(...)) of a rational. EA8 adds no second search: a series
evaluated in the wide tier encloses the value with a certified bound, and the one search reads that
enclosure as a side function, refining it until it lies clear of the midpoint between two doubles.
The values that are rational, at multiples of 30 degrees (Niven: the only rational sines and
cosines at rational degrees), come from a table, so the cosine of 90 and the sine of 180 are 0
exactly. Its witness is MPFR at 300 to 2400 bits, as the oracles of the cell walks read it
(`h3_geodetic_oracle.py` of the oracle kit, its `d6`).

### EA9. The signs on coordinates shifted by multiples of 360

A cell straddling the antimeridian is read with vertices shifted by 360 degrees, and the shift is
not exact in doubles: 21.599649137590685 + 360 rounds, and two cells then no longer state the
vertex they share with the same number. The orientation sign (`cross_product_sign`) and the
location of a point in rings (`point_in_rings_exact`) take each coordinate as a double plus an
exact integer multiple of 360, a sum read exactly through the one ladder of EA1, so the cells read
no second point-in-ring of their own. #3577, merged, carries it in `geo_funcs.c`, ahead of EA3, which
moves `cross_product_sign_offset` and `point_in_ring_offset_exact` with the rest
of the layer: the cell walks of MobilityDB #3544 read them first. Its unit test
(`shift_signs_unit.c`) holds 6 rows on which the sign of the shifted doubles is wrong, and its
witness on the cells is the planar oracle of the cell walks (`cgal_h3_planar.cpp` of the oracle kit, the
shifts exact in `CGAL::Gmpq`), whose control reading the shift in doubles leaves 158 positions
of its acceptance set held by no cell or by two.

## 8. Decisions asked of the committers

1. The one ladder: the expression ladder with each question's own first tier ahead of it, at
   about 1.5 times the polynomial ladder's cost on a tie (EA1), the price of being exact for
   every finite double without its caller keeping a range.
2. The order of Section 7: the tolerances beside exact signs first, the duplicates of one contract
   next, one convention, then one arithmetic, the speed of MobilityDB #3484 after.
3. The conventions of phase 3: "undecided" is 2 inside the layer, an answer outside it is -1, 0
   or 1, the sense of a circle question is that of `incircle_sign`, a segment of no length is a
   point.
4. The names of the arithmetic's file and header (EA3), and of the ladder (EA1), as the layer
   names them rather than the first question each served.
5. The window of EA3: one in which no pull request on these files is open, MobilityDB #3484
   and #3557 being two such pull requests, and step 18 of [Exact buffer](EXACT-BUFFER.md) not
   started.
6. The seam of Section 6 as the rule of the user entries, with no other library planned.
