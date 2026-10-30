<!--
  MobilityDB — Exactness: Any number of turning points in the lifting
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Any number of turning points in the lifting

**For discussion by the committers. Nothing here is merged or pushed.**
Master at the time of writing: `d51b32e664`.

## 1. Summary

A lifted function whose result is a temporal float with linear interpolation must put a vertex at
every point where the exact result stops increasing or decreasing inside a segment, its **turning
points**. Only then is the piecewise-linear result exact at its minima and maxima, and only then are
the nearest approach distance, the nearest approach instant and the shortest line taken from it
exact. A lifted predicate needs, in the same way, every instant where its value **changes** inside
a segment.

The lifting infrastructure gets these instants from a callback the family registers in
`LiftedFunctionInfo`. On master there are two kinds of callback:

- callbacks returning **at most two** instants through two output parameters `*t1, *t2`. These are
  `tpfn_unary`, `tpfn_base` and `tpfn_temp`, and about twenty kernels use them;
- one callback returning **any number** of instants through an array, `tpfn_set`. It exists for
  unary lifts only, and only `sin` and `cos` use it.

Some functions of two arguments have more than two such instants per segment. Examples are the
distance between rotating rigid geometries, the distance and `tDwithin` between temporal geography
points, and moving bodies against polygons with many edges. Because the binary callbacks are capped
at two, these families do not use the lifting. They run walks of their own, which repeat what the
lifting already provides.

This note proposes **one callback per arity, returning an ordered array of instants**, which
replaces both kinds. The ≤2 case is the case where the array has at most two entries. The field
names stay `tpfn_unary`, `tpfn_base` and `tpfn_temp`, and `tpfn_set` merges into `tpfn_unary`. The
change is staged so that its first pull request changes no result (section 6). Section 7 assesses
how difficult it is.

## 2. Why: the result the book describes

Volume 2 of the MobilityDB book, Sect. 4.6.3 *Linear Approximation as Temporal Distance*, states the
contract:

> MobilityDB returns the temporal distance between two objects as a `tfloat` value, which is
> piecewise linear. However, the distance between two moving rigid objects is not piecewise linear,
> in particular when they rotate. MobilityDB therefore computes a piecewise-linear approximation of
> it that keeps the exact values at the start and at the end of every segment, as well as all the
> extreme points of the function, that is, its minima and maxima. [...] Since the approximation
> keeps the minima of the function, the nearest approach distance computed from it is exact.

A body that rotates by an angle θ(t) linear in t moves each vertex on an arc. Even for one fixed
pair of closest features (Sect. 4.6.1), the squared distance has the form
`(a1 t + a0) cos ωt + (b1 t + b0) sin ωt + c2 t² + c1 t + c0`. The number of its extrema in a
segment grows with the rotation ω in that segment, and each change of closest features begins a new
piece. **The number of extrema per segment has no fixed bound, so "all the extreme points" cannot
pass through a callback that returns two.**

## 3. What master has

### 3.1 The callbacks

`meos/include/temporal/lifting.h`, `LiftedFunctionInfo`, with the typedefs in
`meos/include/temporal/temporal.h`:

| Field | Lift | Instants per segment | Typedef |
|---|---|---|---|
| `tpfn_unary` | unary | at most 2 | `int (*tpfunc_unary)(Datum, Datum, TimestampTz, TimestampTz, TimestampTz *, TimestampTz *)` |
| `tpfn_set` | unary | any, the callee allocates the array | `int (*tpfunc_set)(Datum, Datum, TimestampTz, TimestampTz, TimestampTz **)` |
| `tpfn_adaptive` (flag) | unary | any, by adaptive bisection | — |
| `tpfn_base` | temporal × base | at most 2 | `int (*tpfunc_base)(Datum, Datum, Datum, TimestampTz, TimestampTz, TimestampTz *, TimestampTz *)` |
| `tpfn_temp` | temporal × temporal | at most 2 | `int (*tpfunc_temp)(Datum, Datum, Datum, Datum, Datum, TimestampTz, TimestampTz, TimestampTz *, TimestampTz *)` |

The typedefs and every `*_turnpt` declaration are in internal headers (`meos/include/temporal/`,
`meos/include/cbuffer/tcbuffer.h`, `meos/include/geo/tgeo_distance.h`, `tgeo_spatialrels.h`,
`tgeo_tempspatialrels.h`, `meos/include/temporal/tnumber_mathfuncs.h`). None is in a public
`meos*.h` header, so none reaches the API catalog or a binding.

The unary side has two exact consumers written as the same loop twice: `tfunc_tlinearseq_turnpt`
(≤2) and `tfunc_tlinearseq_set` (any number, growing its buffer through `tfunc_adaptive_emit`).

### 3.2 Two readings of the same two outputs

The two outputs mean different things depending on the callback:

| Reading | What `t1, t2` are | Callbacks | Readers |
|---|---|---|---|
| **turning points** | up to two instants where the result turns; for parallel motion `t1 < t2` bounds the stretch at constant minimum distance | `tfloat_exp/ln/log10_turnpt`, `tfloat_distance_turnpt`, `tfloat_base_distance_turnpt`, `tfloat_arithop_turnpt`, `tpoint_geo_distance_turnpt`, `tgeompointsegm/tgeogpointsegm/tpointsegm_distance_turnpt`, `tpoint_geo_bearing_turnpt`, `tpointsegm_bearing_turnpt`, `tcbuffer_cbuffer_distance_turnpt`, `tcbuffersegm_distance_turnpt` | `tfunc_tlinearseq_turnpt`, `tfunc_tlinearseq_base_turnpt`, `tfunc_tcontseq_tcontseq_single`, `tfunc_tlinearseq_tstepseq`, and the nearest approach kernel of `tgeo_distance.c` that receives `tpfunc_temp turnpt` |
| **interval of truth** | the predicate holds on `[t1, t2]`; `t1 == t2` is a touch at one instant | `tpointsegm_tdwithin_turnpt`, `tcbuffersegm_dwithin_turnpt`, `tcbuffersegm_tdwithin_turnpt`, `tcbuffersegm_contains_turnpt` | `tdwithin_add_solutions_run` (from the two `tdwithin` walkers of `tspatial_tempspatialrels.c`), `tfunc_tcontseq_tcontseq_discfn`, `eafunc_tcontseq_tcontseq_discfn` |

The second reading already caps a segment at **one** interval. `tfunc_tcontseq_tcontseq_discfn`
reads only `tpt1`, and `eafunc_tcontseq_tcontseq_discfn` reads `tpt1` and the middle of
`[tpt1, tpt2]`. A predicate that is true on two disjoint stretches of one segment cannot be stated.

### 3.3 How the families that need more work around it

**Temporal rigid geometries.** `tdistance_trgeometry_geo`, `_tpoint` and `_trgeometry` (and from
them `nad_*`, `nai_*`, `shortestline_*`, `tdwithin_*`) do not call the lifting.
`meos/src/rgeo/trgeo_distance.c` has its own `dist_temporal` (instant, sequence, sequence set),
`dist_seq` (discrete and step interpolation, minimum over the convex parts of a static geometry with
`tfloatseq_min_tfloatseq` and `dist2d_seq_exact_values`) and `dist_walk_seq` (segment loop, linear
against step split into one sequence per segment). The kernel itself is exact and correct:
`distwalk_segm` (`trgeo_distwalk.c`) follows the closest features, the root finder
`distfun_roots` / `distfun_crossings` (`trgeo_distsolve.c`) comes with a proof for each part of the
interval, and the design is in `meos/src/rgeo/trgeo_distance.txt`. What the cap forces is the code
around it. Each rule of the shared walk (the bound rule, linear against step, exclusive upper bound)
has to be kept a second time here.

**Geodetic temporal points.** `geogsegmpair_critical` (`tspatial_tempspatialrels.c`) returns an
array: "the fractions of the segments, in increasing order, at which the angle between
the positions [...] stops increasing or decreasing". Its comment says why two is not enough: "no
closed form bounds the number of zeros here, hence the isolation".

**Moving bodies against polygons.** The distance from a moving disc or body to a polygon with many
edges has up to one extremum per edge in a segment. The distance of a temporal circular buffer to a
geometry is exact on master by a dedicated kernel (#3162 and the pull requests before it). What
remains there must be measured before it is planned.

## 4. The proposal: one callback per arity

```c
/* Turning point function of a unary lift: writes in tps, in increasing order
 * and strictly inside (lower, upper), at most cap instants, and returns how
 * many there are, which may exceed cap */
typedef int (*tpfunc_unary)(Datum start, Datum end, TimestampTz lower,
  TimestampTz upper, TimestampTz *tps, int cap);

/* The same for a temporal and a base value */
typedef int (*tpfunc_base)(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *tps, int cap);

/* The same for two temporal values */
typedef int (*tpfunc_temp)(Datum start1, Datum end1, Datum start2, Datum end2,
  Datum param, TimestampTz lower, TimestampTz upper, TimestampTz *tps, int cap);
```

- **Names.** The typedefs keep their names (`tpfunc_unary`, `tpfunc_base`, `tpfunc_temp`), as do
  the fields `tpfn_unary`, `tpfn_base` and `tpfn_temp`. `tpfunc_set` and `tpfn_set` are removed:
  `tfloatseg_sin_turnpt` and `tfloatseg_cos_turnpt` become `tpfn_unary` callbacks. `tpfn_adaptive`
  stays, since it is a different method (bisection, for `tan`) and not a callback.
- **The caller owns the buffer.** The caller passes a small array on its stack. The callee always
  returns the true count. Only when the count exceeds `cap` does the caller allocate an array of
  that size and call again. A closed-form kernel never exceeds two, so the hot paths (distance over
  millions of segments) allocate exactly as much as today. This also removes the per-segment
  allocation that `tpfn_set` makes on master.
- **Two readings, one shape.** For a turning-point callback, the array is the turning points. For
  an interval-of-truth callback, the array is a sequence of **pairs** `[enter, exit]`, ordered and
  disjoint, where `enter == exit` is a touch. The count is then even. Today's encoding is the
  special case of one pair, with the touch written as one instant. Each reader in section 3.2
  already knows which reading it applies, and keeps it.
- **The rules every callback follows** are the ones the ≤2 callbacks follow today, stated once in
  the shared readers instead of at each site (`tpt1 != prev1->t`, `tpt[1] == tpt[0]`,
  `tpt[k] <= start1->t || tpt[k] >= end1->t`):
  - instants are strictly inside the segment and strictly increasing. An instant that lands on or
    before the previous one is **dropped, never moved**;
  - an exact instant is dated by the rule of [Exact crossings](EXACT-CROSSINGS.md) section 7: the
    nearest microsecond, decided exactly, a tie going to the even microsecond. That rule enters
    through the shared primitive of that note. Outside it, a callback keeps master's truncating
    conversion `lower + (TimestampTz) (duration * fraction)`, so that one crossing carries the same
    instant in every form;
  - the value at each instant is computed **exactly over continuous time**. The microsecond only
    dates the returned instant.
- **What stays outside.** `tsegment_intersection`, which the discontinuous readers call when no
  callback is registered, keeps its two outputs. The reader copies them into its buffer. Changing it
  would touch every same-type comparison and is not needed here.

## 5. What changes, by file

| Where | What |
|---|---|
| `temporal.h`, `lifting.h` | the three typedefs; `tpfunc_set` / `tpfn_set` removed; field comments |
| `lifting.c`, unary | `tfunc_tlinearseq_turnpt` and `tfunc_tlinearseq_set` become one loop; `tfunc_tsequence` loses one branch |
| `lifting.c`, binary turning points | `tfunc_tlinearseq_base_turnpt`, `tfunc_tcontseq_tcontseq_single`, `tfunc_tlinearseq_tstepseq` loop over the array; the fixed `* 3` sizing becomes a buffer that grows on demand |
| `lifting.c`, intervals | `tfunc_tcontseq_tcontseq_discfn`, `eafunc_tcontseq_tcontseq_discfn` read pairs |
| `tgeo_distance.c` | the nearest approach kernel that receives `tpfunc_temp turnpt` loops over the array |
| `tspatial_tempspatialrels.c` | `tdwithin_add_solutions_run` takes the pair array; its two callers pass it |
| twenty callbacks | write into `tps`, return the count; the forwarders (`tpointsegm_distance_turnpt`, `tcbuffersegm_intersection_value`, the wrappers in `tcbuffer.c`, `tcbuffer_distance.c`, `tnumber_distance.c`, `tpoint_spatialfuncs.c`) pass `tps, cap` through |
| internal headers | the declarations in `tcbuffer.h`, `tgeo_distance.h`, `tgeo_spatialrels.h`, `tgeo_tempspatialrels.h`, `tnumber_mathfuncs.h` |

`raster_half_turnpt` (`raster_quadbin.c`) shares the suffix but is not a lifting callback, so it
does not change.

## 6. Staging

**Schedule.** This work starts only after the work of
[Exact distance](EXACT-DISTANCE.md) is finished. Its pull requests interleave
with the steps of [Exact crossings](EXACT-CROSSINGS.md), which share the same readers: PR 1 below,
then steps 2–4 of that note, then PR 2 and PR 3 below.

### 6.1 Pull request 1: the merge, with the same results

All of section 5, under one condition: **every existing callback returns at most two instants, or
one pair.** The interval readers accept one pair and assert it. Acceptance:

- the whole suite passes against the existing expected outputs, none of them edited;
- on the standard benchmark, the ≤2 paths are no slower;
- one internal kernel with three or more turning points per segment, judged by an oracle computed
  independently, exercises the array path of the turning-point readers.

### 6.2 Pull request 2: intervals of any number

The interval readers stop asserting one pair and handle any number. This is the only part that is
new logic rather than a change of signature: `tfunc_tcontseq_tcontseq_discfn` builds one sequence per
stretch inside a segment, where today it builds at most one. It shares its code with the open
question [Exact crossings in the lifting infrastructure](EXACT-CROSSINGS.md), and follows that
decision. Its first user is the geodetic `tDwithin`, whose `geogsegmpair_critical` and crossing
solver then become `tpfn_temp` callbacks. Their callers change, and the solvers do not.

### 6.3 Pull request 3: temporal rigid geometries

`distwalk_segm` already produces, for each segment, the ordered events with their exact distance.
Behind `tpfn_temp` (temporal point or rigid geometry against a rigid geometry) and `tpfn_base`
(against a static geometry), it provides the turning points. `dist_temporal`, `dist_seq` and
`dist_walk_seq` give way to `tfunc_temporal_temporal` / `tfunc_temporal_base`. The minimum over the
convex parts of a static geometry stays in the family, since it is a minimum over parts and not a
lift. No expected output of the rigid geometry distance may move. If one does, it shows a
difference between the two walks and is investigated, never regenerated.

### 6.4 Later, each measured first

- **The temporal distance of two temporal geography points.** It goes through
  `tgeogpointsegm_distance_turnpt`, which takes the minimum of the Cartesian (geocentric) linear
  approximation, with the comment `TODO: The next computation should be done on geodetic
  coordinates`, and returns one turning point. Section 3.3 states that the angle between the
  positions has, in general, several critical points per segment. **This is a hypothesis, not a
  measurement.** A pair of `tgeogpoint` segments for which `geogsegmpair_critical` returns two or
  more fractions, run through `tdistance_tgeo_tgeo` and judged independently, would show whether an
  extremum is missed.
- **Moving bodies against polygons with many edges.** Measure what is still not exact (section
  3.3). Then use the edge R-tree of `meos/src/geo/tpoint_geom_clip.c` (`geom_extract_edges`,
  `build_edge_rtree`, `rtree_search`, `RTREE_MIN_NUMBER_ELEMS`) so that each segment examines only
  the edges near it. The protected areas of the AIS use case reach 67,907 vertices for one polygon,
  and 40,297 at the 95th percentile.

## 7. Feasibility

**Feasible, and mostly mechanical. The difficulty is concentrated in one place.**

| Part | Size | Difficulty | Main risk |
|---|---|---|---|
| typedefs, fields, headers | 2 + 5 internal headers | low | none: no public header, no binding |
| twenty callbacks and their forwarders | about 12 source files | low, mechanical | a callback writing past `cap`; covered by an `assert` in debug builds |
| turning-point readers (three in `lifting.c`, the nearest approach kernel) | four loops | low to moderate | the guards, stated once, must give what each site gives today; the existing expected outputs are the check |
| merging the two unary loops | one function removed | low | — |
| interval readers with one pair (PR 1) | three readers | moderate | changing the encoding of the touch (one instant becomes a pair `(t, t)`) |
| interval readers with any number (PR 2) | three readers | **the hard part** | building several stretches per segment in `tfunc_tcontseq_tcontseq_discfn`; this code is also the subject of EXACT-CROSSINGS |
| speed | the closed-form hot paths | low with the caller's stack buffer | an allocation per segment if the buffer is badly sized; the benchmark of section 6.1 is the check |

PR 1 touches about twenty files. Almost all of its diff is signatures and loops over an array, and
no function computes anything new. The existing expected outputs are its check. Most of
the risk is in PR 2, which is small but changes the logic of the walk. Keeping it separate means
the merge can be judged on its own.

## 8. Relation to the other notes

- **[Exact crossings in the lifting infrastructure](EXACT-CROSSINGS.md).** The two notes concern
  the same code: the turning-point and crossing callbacks and the readers of `lifting.c` listed in
  its section 3 and in section 3.2 here. They decide different things about each instant a segment
  carries:

  | | Exact crossings | This note |
  |---|---|---|
  | decides | whether an instant exists (exact sign, no band), which microsecond dates it, and the result there (exact side, no interpolated value) | how many instants a segment carries and through which interface |
  | where its rule lives | "one shared primitive", "never walk by walk" | the shared readers of section 5 |

  The merge gives that note's shared primitive one place to live. The guards and the dating that
  section 4 states once in the readers are the rules that note makes exact. Without the merge, they
  have to be made exact at each reader and in both kinds of callback. The order is: PR 1 here (same
  results, signatures only), then that note's steps 2–4 on the merged readers, then PR 2 here,
  which rewrites `tfunc_tcontseq_tcontseq_discfn`, the reader that note also rewrites. The dating
  in section 4 is that note's rule.
- **[Exact distance](EXACT-DISTANCE.md).** This note makes the temporal distance
  carry all its extrema. That one makes each value the double nearest the exact distance. A nearest
  approach distance is exact only when both hold.
- **No GEOS.** MEOS and MobilityDB build without GEOS (MobilityDB #3185); `dist_instant` uses
  `geom_distance2d` only at the instants of a step or discrete sequence, so moving the rigid
  geometry walk onto the lifting changes no dependency. The twelve ever and always relationships of
  a rigid geometry with a geometry merge its placements (`trgeo_geoms_merge`) into one union. The
  walk already reports where a distance crosses a level (`DistOps.level`), so after PR 2 it could
  decide ever/always intersection from the distance at level 0, without a union. **That is a
  hypothesis, not a claim of this note.**

## 9. Decisions asked of the committers

1. Should the two kinds of turning-point callback merge into one per arity, returning an ordered
   array in a buffer owned by the caller, with `tpfn_set` merged into `tpfn_unary`?
2. Is the pair encoding acceptable for interval-of-truth callbacks (`[enter, exit]`, with a touch
   written as `(t, t)`)?
3. Is the staging acceptable: the merge with no result changed, then intervals of any number, then
   the rigid geometries, each in its own pull request?
