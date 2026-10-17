<!--
  MobilityDB — Open questions for the committers: Exact crossings in the lifting infrastructure
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Exact crossings in the lifting infrastructure

**For discussion by the committers. Nothing here is merged or pushed.**
Master at the time of writing: `ed610318f8`.

## 1. Summary

A temporal value with **linear** interpolation crosses a value (or another temporal value)
inside a segment. MEOS must find that crossing to answer comparisons, spatial relationships,
distances and their ever/always forms. Discrete and step values never need it: they are
evaluated at their instants only. The scope is therefore every operation where **at least one
operand is linear**, including a linear operand against a step one.

On master, finding and dating a crossing has three defects:

- **A. A band drops real crossings.** A crossing closer than `1e-6` of the segment to either
  end is ignored. On a one-day segment that is 86.4 ms, 86,400 microseconds where the answer
  can be wrong.
- **B. The result at the crossing instant comes from a rounded value.** The walk interpolates
  the value at the instant it chose and compares that rounded value. It can then state a result
  that holds at no microsecond.
- **C. The band does not protect short segments.** The band is relative to the segment, so on
  a 1 ms segment a crossing inside the first microsecond passes it, and the walk builds two
  instants at the same timestamp:

  ```sql
  SELECT tfloat '[0@2001-01-01 00:00:00, 1@2001-01-01 00:00:00.001]' #<= 0.0005;
  -- master:   ERROR  Timestamps for temporal value must be increasing:
  --                  2001-01-01 00:00:00+00, 2001-01-01 00:00:00+00
  -- proposed: {[t@2001-01-01 00:00:00, f@2001-01-01 00:00:00.000001, f@2001-01-01 00:00:00.001]}
  ```

Deciding with exact signs on the doubles, never with a band or an interpolated value, fixes all
three. The fix has one hard constraint and needs one decision:

- **Constraint (section 4):** the same crossing appears in several lifted forms, and master
  dates it at the same instant in all of them. Any fix must keep that, so it can only be done
  through **one shared primitive**, never walk by walk.
- **Decision (section 7):** which microsecond dates a crossing that falls between two
  microseconds.

The exact locator on its own costs 2.5–2.7× master's, about 15 ns per crossing. The whole
comparison still executes fewer instructions than master: −2.4% when half the segments cross,
−12.1% when none do (section 6, Performance).

## 2. A simple case

```sql
SELECT tfloat '[2@2001-01-01, 1@2001-01-02]' #<= 1.0000001;
```

The value falls from 2 to 1 over one day. It reaches 1.0000001 86399991359.99999 µs after the
start, just before 23:59:59.991360, and stays `<= 1.0000001` until the end. At 23:59:59.995,
for example, the value is 1.0000000579.

| | Result |
|---|---|
| master | `{[f@2001-01-01 00:00:00, t@2001-01-02 00:00:00]}`: false until the last instant, wrong over 8.64 ms |
| proposed | `{[f@2001-01-01 00:00:00, t@2001-01-01 23:59:59.99136, t@2001-01-02 00:00:00]}` |

`atGeometry` on a `tgeompoint` finds the same crossing correctly, because its code path
(`tpoint_geom_clip.c`) has no band. `tDwithin` on a `tgeompoint` has the same defect through
another band, `fabsl(t7 - t8) < MEOS_EPSILON` in `tpointsegm_tdwithin_turnpt`. It answers false
at 24:00, where the distance is exactly 1:

```sql
SELECT tDwithin(tgeompoint '[Point(2 0)@2001-01-01, Point(1 0)@2001-01-02]',
  geometry 'Point(0 0)', 1.0000001);
-- master: {[f@2001-01-01 00:00:00+00, t@2001-01-01 23:59:59.991359+00],
--          (f@2001-01-01 23:59:59.991359+00, f@2001-01-02 00:00:00+00]}
```

## 3. Where crossings are computed

Every `LiftedFunctionInfo` filled in MEOS was inventoried: 145 fillings. Every crossing or
turning-point computation in `lifting.c` sits behind a test that an operand is linear. The
walks that compute one:

| Result | Walk | Locator |
|---|---|---|
| Discontinuous, against a base value (`#=` `#<` …, spatial relationships) | `tfunc_tlinearseq_base_discfn` | `floatsegm_locate` (band) |
| Discontinuous, two temporals, both linear or linear against step | `tfunc_tcontseq_tcontseq_discfn` | `tnumbersegm_intersection` (band) or `tpfn_temp` |
| Continuous with turning points, against a base value (distance, bearing) | `tfunc_tlinearseq_base_turnpt` | `tpfn_base`, e.g. `tnumbersegm_intersection` (band) |
| Continuous with turning points, two temporals | `tfunc_tcontseq_tcontseq_single`, `tfunc_tlinearseq_tstepseq` | `tpfn_temp` (band, and `t7 - t8` for `tDwithin`) |
| Unary with turning points (`exp`, `ln`, `log10`, `sin`, `cos`, `tan`) | `tfunc_tlinearseq_turnpt`, `_set`, `_adaptive` | `tpfn_unary`, `tpfn_set`, bisection |
| Ever/always, against a base value or two temporals | `eafunc_tlinearseq_base`, `eafunc_tcontseq_tcontseq_discfn` | `floatsegm_locate` / `tnumbersegm_intersection` (band) |

The three kinds of result are wrong in different ways:

- **Discontinuous** (true/false): wrong answers at microseconds (A, B), or an error (C).
- **Continuous with a turning point**: a wrong value, at most slope × 1 µs, because the kink is
  placed at a rounded instant.
- **Ever/always**: wrong true/false answers from the band. This walk reads time as continuous:
  it takes the value as reached if it lies strictly between the end values. It never needs to
  date the crossing, so the exact existence test alone fixes it.

Two further facts from the inventory:

- `cross_type` is read in `lifting.c` but set by no filling on master, so its branch never runs.
- The linear-against-step continuous walk asked for no turning points at all, and the
  circular-buffer distance declared no turning-point function unless both operands were linear.
  A distance therefore lost its minimum between two instants. This is independent of the band
  and is fixed by PR #2873, merged (section 9).

## 4. The constraint: one crossing, several forms

For a linear tfloat crossing a value v, the same crossing appears as:

- `x #<= v`: discontinuous, found by `floatsegm_locate`;
- `tdistance(x, v)`: continuous with a kink at the crossing, found by
  `tfloat_base_distance_turnpt`, which calls `tnumbersegm_intersection`;
- `x #<= v` with v a constant tfloat: discontinuous, found by `tnumbersegm_intersection`;
- `ever x = v`: continuous time, never dated.

Over 100,000 random crossings:

| Build | `#<=` float vs distance kink | `#<=` float vs `#<=` tfloat |
|---|---|---|
| master | 0 differ | 0 differ |
| exact dating in the comparison walk only | 50,024 differ by 1 µs | 50,024 differ by 1 µs |

Master is wrong near a bound but consistent: all forms date a crossing at the same instant.
Changing one walk breaks that, so that `tdistance(x, v)` reaches 0 one microsecond away from
where `x #<= v` changes. **The fix must be one primitive that every locator uses**:
`floatsegm_locate`, `tnumbersegm_intersection` and the turning-point functions, and through
them every walk, discontinuous, continuous and ever/always, in one change.

There is also a question of semantics. Ever/always reads the crossing in continuous time, so
`ever x = v` is true whenever v lies strictly between the end values. The temporal result reads
time on the microsecond grid, so `x #= v` is true only at a microsecond where the value is
reached exactly. The two can disagree about the same crossing. Which reading MEOS intends is a
question for the committers.

## 5. The proposed code

One primitive per family: the **exact side of a segment at an instant**, the sign of a
determinant in the doubles. It uses `cross_product_sign`, master's filtered sign with an exact
fallback.

```c
/* The exact sign of x(t) - value on the segment [start@lower, end@upper] */
static int
tfloatsegm_side(double start, double end, double value, TimestampTz lower,
  TimestampTz upper, TimestampTz t)
{
  return - cross_product_sign(0.0, start, (double) (upper - lower), end,
    0.0, start, (double) (t - lower), value);
}

/* The microsecond dating the crossing, strictly inside the segment, or false
 * when that microsecond is a bound (no instant separates the crossing from it) */
static bool
tfloatsegm_crossing(double start, double end, double value, TimestampTz lower,
  TimestampTz upper, TimestampTz *t)
{
  if (value <= Min(start, end) || value >= Max(start, end))
    return false;                       /* exact: no crossing inside */
  TimestampTz d = upper - lower;
  double dd = (double) d;
  int dir = (end > start) ? 1 : -1;
  /* The nearest microsecond m is the one whose window (m - 1/2, m + 1/2]
   * holds the crossing: the estimate from the fraction is confirmed by the
   * exact signs at the two ends of its window */
  TimestampTz m = llrintl((long double) d *
    ((long double) (value - start) / (end - start)));
  while (m > 0 && dir * - cross_product_sign(0.0, start, dd, end, 0.0, start,
      (double) m - 0.5, value) >= 0)
    m--;
  while (m < d && dir * - cross_product_sign(0.0, start, dd, end, 0.0, start,
      (double) m + 0.5, value) < 0)
    m++;
  if (m <= 0 || m >= d)
    return false;                       /* on a bound */
  *t = lower + m;
  return true;
}
```

In the walks, the result at the crossing instant comes from the exact side there, never from an
interpolated value. On a monotone segment a comparison takes one of three values:

```c
/* master */
tpvalue1 = tsegment_value_at_timestamptz(startvalue, endvalue, ..., tpt1);
tpresult = tfunc_base_base(tpvalue1, value, lfinfo);

/* proposed */
int sd = tfloatsegm_side(s, e, v, start->t, end->t, tpt1);
tpresult = (sd == 0) ? tfunc_base_base(value, value, lfinfo)  /* exactly on it */
         : (dir * sd < 0) ? startresult : endresult;
```

| Family | Side at instant m | Exact sign |
|---|---|---|
| tfloat vs float | `d(v - s) - (e - s)m` | `cross_product_sign` (master) |
| tfloat vs tfloat | `(x1s - x2s)(d - m) + (x1e - x2e)m` | `cross_product_sign` (master); not yet prototyped |
| tpoint `tDwithin` | `|P1(m) - P2(m)|² - dist²`, degree 2 | polynomial sign with an expansion fallback (exists for the circular buffer, not yet on master) |
| circular buffer | the same, with radii | same |
| turning points (min/max, bearing) | sign of the derivative | same pattern |

## 6. Proof

The prototype covers only the comparison walk against a base value
(`tfunc_tlinearseq_base_discfn`). It is evidence for the method, not the proposed change, which
must be the shared primitive of section 4.

**Oracle.** At each microsecond m the true side of the segment is the sign of
`duration * (value - start) - (end - start) * m`, a 2×2 determinant in the doubles decided
exactly. No tolerance and no rounded value enters the truth.

**Corpus.** Random one-segment tfloats, durations from 1 ms to 10 days, magnitudes from 1e-3 to
1e3, crossings near either bound, anywhere, or on a whole microsecond, the value optionally one
ulp away. `#<=`, `#<`, `#=`, `#>=` are read at the bounds and at the microseconds around the
crossing.

| Corpus | master | prototype |
|---|---|---|
| 20,000 cases (156,083 checks per operator) | 60,291 wrong checks | **0 wrong** |
| 200,000 cases (1,560,247 checks per operator) | stops with the error of defect C | **0 wrong** |

Master on the 20,000 cases:

| op | wrong near lower | near upper | anywhere | on a whole µs | cases wrong: band / found |
|---|---|---|---|---|---|
| `#<=` | 8988 | 9180 | 3 | 1341 | 6057 / 1341 |
| `#<`  | 8988 | 9180 | 3 | 1368 | 6057 / 1368 |
| `#=`  | 0 | 0 | 0 | 1701 | 0 / 1701 |
| `#>=` | 8988 | 9180 | 3 | 1368 | 6057 / 1368 |

The "found" failures are defect B. In one example the crossing lies in (562, 563] µs, and master
states `#<=` and `#=` true at 562, where the segment has not reached the value. The same harness
finds 60,291 wrong checks on master, so its 0 on the prototype is a measurement, not a blind
pass.

### Performance

The exact locator does more arithmetic than `floatsegm_locate`. It also lets the walk drop work
master does around the locator. Both effects were measured on the same machine, which other
work shares, so only ratios measured inside one process, or instruction counts, are reported.

**Method.**

- *Locator alone:* master's `tfloatsegm_intersection_value` (called from `libmeos`) and the
  exact locator are timed in the same process on the same 2 million segments (1 s to 10 min, a
  crossing inside each), 5 rounds. Only the ratio is meaningful.
- *Whole operation:* `tfloat #<= float` on a 1,000-instant random walk, two workloads: about
  half the segments cross the value, or none do. Counted in instructions with callgrind (exact
  and independent of machine load), and timed in wall-clock time against an untouched control
  (`tfloat + float`) in the same process, the builds interleaved.
- *Builds:* master, the exact locator confirming its estimate with 4 signs, and the proposed one
  confirming it with 2.

**The locator alone.** The proposed form rounds the fraction to the nearest microsecond and
confirms it with the exact signs at the two ends of its half-microsecond window. One filtered
sign is two products, a subtraction and a bound. The exact expansion runs only when the filter
cannot decide: 4 times in 8 million signs.

| Locator | Sign evaluations per call | Time relative to master |
|---|---|---|
| master (fraction and band) | 0 | 1.00 |
| exact, 4 signs | 4 | 3.4–3.8 |
| **exact, 2 signs (proposed)** | **2** | **2.5–2.7** |

That is about 15 ns more per crossing on this machine (master about 9 ns per call, indicative
only). The 2-sign and 4-sign forms return the same instant on all 10 million calls.

**The whole operation, in instructions (callgrind).**

| Workload | master | exact, 4 signs | exact, 2 signs (proposed) | change |
|---|---|---|---|---|
| about half the segments cross | 51,871,235 | 50,799,709 | 50,600,622 | **−2.4%** |
| no segment crosses | 23,085,998 | 20,266,669 | 20,286,689 | **−12.1%** |

**Where the instructions go** (both workloads together, inclusive counts):

| Function | master | proposed |
|---|---|---|
| `tfunc_tlinearseq_base_discfn` (the whole walk) | 64,574,833 | 57,350,816 |
| `tsequence_make` (building the result) | 20,897,069 | 21,247,571 |
| `tinstant_make` (building the result) | 15,303,962 | 15,382,251 |
| `datum_eq` | 8,080,180 | 5,920,000 |
| `tsegment_intersection_value` (dispatch, bound tests, locator) | 5,798,530 | inlined in the walk |
| `tsegment_value_at_timestamptz` (interpolation at the crossing) | 1,173,786 | not called |

Building the result is about 60% of the walk, and locating the crossing is a small part of it.
The exact path replaces the generic `datum_eq` and base-type dispatch of
`tsegment_intersection_value`, and the interpolation and comparison at the crossing instant,
with inline comparisons of the doubles and signs. That saving covers the extra 15 ns per
crossing and more.

**Wall-clock time** (median ratio to the untouched control over 18 rounds per build, builds
interleaved; lower is faster):

| Run | Build | About half cross | None cross |
|---|---|---|---|
| 1 | master | 3.029 | 1.145 |
| 1 | exact, 4 signs | 3.242 | 0.999 |
| 2 | master | 2.960 | 1.166 |
| 2 | exact, 4 signs | 3.141 | 0.949 |
| 2 | exact, 2 signs (proposed) | 3.365 | 1.043 |

Run to run, the same build moves by about ±10% here (in run 2 the 2-sign form reads slower
than the 4-sign one although it executes fewer instructions). The wall-clock differences are
therefore inside the noise, and the instruction counts are the measurement that decides.

## 7. The decision: which microsecond dates a crossing

With B fixed, every microsecond of the result is right whichever neighbouring microsecond dates
the crossing, because the walk reads the true side there. The rule decides only the error in
continuous time and which existing outputs move.

| Rule | Error in continuous time | #2852 case (crossing 0.0000096 µs before 12:00) |
|---|---|---|
| truncation (master's `(TimestampTz)(d * f)`) | < 1 µs, always early | `t` from just after 11:59:59.999999: the expected output moves |
| first microsecond at or after | < 1 µs, always late | 12:00, unchanged; a crossing just after a bound moves by 1 µs |
| **nearest microsecond (proposed)** | **≤ 0.5 µs** | 12:00, unchanged on both sides |

Truncation is the dating used across the tree. Changing it was proposed once before (PR #2798)
and closed because of its reach. The proposal here is that the convention become **the nearest
microsecond, decided exactly**, adopted by the shared primitive and therefore everywhere at
once.

## 8. Blast radius

**Code (master `ed610318f8`).**

- `floatsegm_locate` / `tsegment_intersection_value` is called by the lifting walks
  (`tfunc_tlinearseq_base_discfn`, `eafunc_tlinearseq_base`), the restrictions
  (`tsegment_restrict_value`, `tnumbersegm_restrict_span`), the tiling
  (`temporal_tile_meos.c`), and the circular buffer and pose.
- `tnumbersegm_intersection` (tfloat against tfloat, and the tfloat distance kink) carries the
  same band.
- **Fraction bands** deciding a crossing or turning point: 16 lines in 8 files:
  `tsequence.c` (2), `tnumber_mathfuncs.c` (3), `tgeo_distance.c` (5),
  `tpoint_spatialfuncs.c` (2), `tgeo_spatialrels.c` (1, the `tDwithin` one), and
  `tcbuffer_distance.c`, `npoint.c`, `raster_quadbin.c` (1 each).
- **Sites that date a crossing** with `lower + (TimestampTz)(duration * f)`: at least 36, in
  12 files across temporal, geo, cbuffer and pose.

**Expected outputs.** The full regression suite (337 tests, all families, PostgreSQL 18) was
run on the prototype. Two tests move:

| Test | What moves | Kind |
|---|---|---|
| `030_temporal_compops` | `tfloat '[1@2001-01-01, 3@2001-01-11]' #< 1.000001` and its mirror: master states the bound's value over the 432,000 µs before the crossing; the prototype states the crossing at 00:00:00.432 and 23:59:59.568 | correction of defect A |
| `028_tbool_boolops_tbl` | `MAX(duration(whenTrue(temp #> 5))) FROM tbl_tfloat`: `03:36:45.567922` → `03:36:45.567923`; 18 of the 100 fixture rows move by ±1 µs, none by more | dating convention only |

This covers the comparison walk alone. The shared primitive reaches every form of section 4,
so its own run will move more outputs, and each will be classified the same way before it is
adopted.

## 9. Plan

**One rule for the whole tree:**

1. **Whether a crossing exists** inside a segment is decided by an exact sign on the doubles,
   never by a band on the fraction.
2. **The instant that dates it** is one microsecond fixed by exact signs (proposed: the
   nearest). If that microsecond is a bound, the crossing belongs to the bound.
3. **The result at that instant** is taken from the exact side of the segment there, never from
   a value interpolated at that instant.

**Steps.** Each is its own PR, with its own witness, the regression suite, and the fixture
diffs classified:

1. **Independent of the band, first:** the linear-against-step walk and the circular-buffer
   distance lose turning points (section 3): PR #2873, merged. The distance of a linear tfloat
   `[0, 2]` and a step constant 1 now reaches 0 at the midpoint, as it does against a linear
   operand.
2. **The shared primitive** for tfloat (the float locator, `tnumbersegm_intersection`, the
   tfloat turning points), with every walk that uses them switching at once: comparisons,
   distances, ever/always, restrictions. Consistency across the forms of section 4 is part of
   its witness.
3. **tpoint and circular buffer:** `tDwithin` and its ever/always forms, and the distance
   turning points, through the same primitive with the exact polynomial sign.
4. **The remaining turning points:** unary functions, bearing, `tnumber_mathfuncs.c`.

Steps 2–4 carry the dating rule chosen in section 7. If the committers keep truncation, the
steps still apply with truncation computed exactly. The #2852 expected output then moves by
one microsecond, to a result that is equally correct at every microsecond.
