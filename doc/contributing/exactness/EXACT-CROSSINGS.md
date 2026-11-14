<!--
  MobilityDB — Exactness: Exact crossings in the lifting infrastructure
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
  -- proposed: {[t@2001-01-01 00:00:00, t@2001-01-01 00:00:00.000001],
  --            (f@2001-01-01 00:00:00.000001, f@2001-01-01 00:00:00.001]}
  ```

Deciding with exact signs on the doubles, never with a band or an interpolated value, fixes all
three. The fix has one hard constraint and needs one decision:

- **Constraint (section 4):** the same crossing appears in several lifted forms, and master
  dates it at the same instant in all of them. Any fix must keep that, so it can only be done
  through **one shared primitive**, never walk by walk.
- **Decision (section 7):** which microsecond dates a crossing that falls between two
  microseconds. The recommendation is **the nearest microsecond, decided exactly, with a tie going
  to the even microsecond**: for instants, the rule that [Exact distance](EXACT-DISTANCE.md)
  applies to values.

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

The discontinuous walk against a static value, `tfunc_tlinearseq_base_discfn`, computed its
result at a crossing on the value interpolated at the instant dating the crossing rather than on
the static value, so `x #= v` read false at a crossing it had found, for tfloat, tcbuffer,
tgeompoint and tnpoint alike. MobilityDB #3378 (`2812882904`) computes it on the static value, as the ever/always
walk does. The instant at which that result is stated stays master's, which this plan corrects.

The locator of a circular buffer, `cbuffersegm_locate`, builds three serialized points from the
centres of the segment's ends and of the value, only to call `pointsegm_locate` on them, for
every segment it reads. On 10 000 pairs of a trip and a static disc, this conversion takes 28% of
the time of `tcbuffer #= cbuffer`, and `?=` and `#=` cost 1.29 and 1.37 times their tgeompoint
counterparts. Reading the centres' coordinates instead answers the same and takes 1.60 and 1.67
times less time, but leaves the tolerance of `pointsegm_locate` in place, so it is not done
separately: the exact locator of section 5, called on the coordinates, removes both the tolerance
and the conversion.

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
  /* A crossing exactly at m + 1/2 is a tie, which goes to the even
   * timestamp: the parity is that of lower + m, not of m, so that the
   * crossing gets the same instant whatever instant its segment starts at */
  if (((lower + m) & 1) && m < d && cross_product_sign(0.0, start, dd, end,
      0.0, start, (double) m + 0.5, value) == 0)
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
| **nearest microsecond (recommended)** | **≤ 0.5 µs** | 12:00, unchanged on both sides |

**The recommendation: the nearest microsecond, decided exactly, a tie going to the even
microsecond.** The reasons:

- **It is the rule of exact distance, applied to instants.** [Exact distance](EXACT-DISTANCE.md)
  answers a distance with the double nearest the exact quantity, a tie going to the double whose
  last bit is even. A `TimestampTz` is a count of microseconds, so the instant of an exact crossing
  is answered the same way: the microsecond nearest to it, a tie (the exact crossing at
  `m + 1/2`, where the exact sign is 0) going to the even one. Truncation is rounding toward
  zero, which no distance in MEOS accepts.
- **The error is smaller and has no direction.** The nearest microsecond is at most 0.5 µs from
  the crossing, on either side. Truncation is up to 1 µs away and always early, and that bias
  shows in aggregates. In `028_tbool_boolops_tbl` (section 8), 18 of the 100 fixture rows of
  `MAX(duration(whenTrue(temp #> 5)))` move by one microsecond.
- **Master truncates a rounded product, not the crossing.** `lower + (TimestampTz)(duration * f)`
  rounds `f` and then the product before it truncates. An exact crossing that falls **on** a whole
  microsecond is often dated one microsecond early: 0.7 of a day is computed as
  60479999999.99999 µs and stated as 16:47:59.999999. An exact corner of a cell walk falls in two
  different microseconds, so a walk holds the tile beside the corner for 1 µs and `spaceSplit`
  drops the corner instant. Computing truncation exactly would fix these too, but at the same cost
  the nearest microsecond is strictly better.
- **It decides no value.** Every quantity is computed exactly over continuous time. With B fixed,
  every microsecond of the result is right whichever neighbour dates the crossing. The rule only
  sets how far the stated instant lies from the true one.

**Neither `rint` nor a cast is this rule.** `rint(duration * f)` errs near a half microsecond, as
the cast errs near a whole one, since both read a rounded product. `tfloatsegm_crossing`
(section 5) confirms the window `(m - 1/2, m + 1/2]` with exact signs, and settles a tie exactly.

**Ties.** A tie is a crossing exactly at `m + 1/2` µs from the start of its segment, for example
a value rising from 0 to 1 over 3 µs, crossing 0.5 at 1.5 µs. It is detected **exactly**: `m + 0.5`
and the segment duration are exact doubles (below 2^52 µs, about 142 years), so the sign of the
segment's function at `m + 0.5`, decided by `cross_product_sign` with its exact fallback, is 0
exactly when the crossing is a tie, and never by rounding. The limit is the one
`cross_product_sign` documents: exact where no product of coordinate differences overflows or
underflows. The tie goes to the **even timestamp**:
- the parity is taken on the timestamp `lower + m`, never on the offset `m`. A crossing appears in
  segments that start at different instants (a synchronized segment starts where the other operand
  has an instant), and every form must date it at the same instant (section 4);
- it is unbiased, like ties-to-even on doubles, which [Exact distance](EXACT-DISTANCE.md)
  uses. Ties to the earlier or the later microsecond would be biased one way;
- it does not depend on the direction of the segment (rising or falling), nor on which operand is
  first;
- an even timestamp that is a bound of the segment means the crossing belongs to the bound, as for
  any crossing whose nearest microsecond is a bound.

The families whose sign is a polynomial of degree 2 (`tDwithin`, circular buffers, turning points)
detect a tie the same way, by the exact sign of the polynomial at `m + 0.5`.

**How it is adopted: never as a sweep.** A convention the tree shares is never rewritten site by
site, nor changed on its own without a fix that needs it. The nearest microsecond enters only
through the shared primitive of steps 2–5 (section 9). It switches every form of a crossing at
once, in the pull requests whose removal of the bands moves expected outputs anyway. Each moved
output is classified (dating only, or a correction) and moves by at most one microsecond.

**Outside those steps**, every site that dates a crossing, a new one included, keeps master's
truncation, computed exactly where the code is exact. One crossing carries the same instant in all
its forms (section 4), so no site switches on its own.

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
2. **The instant that dates it** is the nearest microsecond, fixed by exact signs, a tie going
   to the even microsecond. If that microsecond is a bound, the crossing belongs to the bound.
3. **The result at a crossing** is the result at the exact crossing over continuous time, never
   one computed from a value interpolated at the instant that dates it. Only that instant is
   rounded, so the value stated may differ from the value at its microsecond: the short segment
   of section 1 crosses 0.0005 just past half a microsecond, is dated at 1 µs and is true there;
   a crossing dated at a bound makes `?=` true with no instant at which `#=` is true.

**Steps.** Each is its own PR, with its own witness, the regression suite, and the fixture
diffs classified:

1. **Independent of the band, first:** the linear-against-step walk and the circular-buffer
   distance lose turning points (section 3): PR #2873, merged. The distance of a linear tfloat
   `[0, 2]` and a step constant 1 now reaches 0 at the midpoint, as it does against a linear
   operand.
2. **The shared primitive** for tfloat (the float locator, `tnumbersegm_intersection`, the
   tfloat turning points), with every walk that uses them switching at once: comparisons,
   distances, ever/always, restrictions. Consistency across the forms of section 4 is part of
   its witness. MobilityDB #3640, merged as `b8be143a79`: the float locators agree with the exact nearest
   microsecond on all 533988 float segments of the crossprobe corpus (332138 of 548989 before
   it); five expected outputs move, each row confirmed on exact rationals. Callgrind against
   master `83a67de4ea`, with about half of the segments crossing and with none: the comparisons
   with a float 0.971 and 0.935, `#=` with a float 0.659 and 0.934, `#=` of two temporal floats
   0.935 and 0.965, atSpan 0.984 and 0.996, atValue 0.969 and 1.000, sin and cos 0.990 and
   0.998, abs 0.994 and 0.986, the distance of two temporal floats 0.992 and 0.988, every walk at
   or below master.
   The value split it switches counts its bins by MobilityDB #3641, merged as `31764297c1`:
   `span_num_bins` and `tile_dim_count` read the quotient of two bins of one grid over the size
   as the nearest whole number, so `bins(floatspan '[1, 2]', 0.3)` holds the bin `[1.8, 2.1)` and
   `valueSplit` of that segment writes no piece past its arrays. The split places a value on a
   grid starting at its first bin, which rounds apart from the grid of the count, so
   `valueSplit` of `[0.1, 2.2]` by 0.7 leaves out the piece from 2.1 to 2.2; placing every value
   on the grid of the origin follows #3640, whose value split it rewrites.
   The ever and always walk of two temporal sequences, `eafunc_tcontseq_tcontseq_discfn`, reads
   only values the two sequences take, MobilityDB #3650, a draft: the end instant where the common
   period holds it, the open piece after an excluded start, and a step value up to its jump, never
   the value it jumps to. `tfloat '[1@2001-01-01, 2@2001-01-02)' ?= tfloat '[0@2001-01-01,
   2@2001-01-02)'` is false, the two meeting only at the excluded end. Callgrind against master
   `9b1cebc9b6` reads every walk within 0.1 percent of master, the geodetic within distance and
   the always covers of two circular buffers 27 to 41 percent below, and the ever covers of two
   circular buffers never covering 50 percent above: a segment with no turning point is read in
   its middle, since the turning point functions of the geodetic within distance and of the
   circular buffers can miss a meeting inside the segment, which step 3 decides exactly.
   The crossing branch of the same walk
   is #3640's, so #3650 rebases over it. The planar
   ever within distance of two temporal points, `ea_dwithin_tpoint_tpoint_exact`, reads its
   intervals closed in the same way: MobilityDB #3680, merged as `3057291650`, answers the ever within at
   instants the two share, an interval whose least squared distance is the square of the
   distance at one end alone being within where the sequences hold that end; CGAL agrees on
   40000 random pairs with frequent meetings at an excluded bound.
3. **tpoint and circular buffer:** `tDwithin` and its ever/always forms, and the distance
   turning points, through the same primitive with the exact polynomial sign.
   The turning points of the containment of two circular buffers come first, MobilityDB #3671,
   merged as `83a67de4ea`: `tcbuffersegm_contains_turnpt` decides a segment over which the clearance vanishes, a
   touch inside a segment and a point on the boundary at an end exactly, where the squared
   clearance alone hides the sign of the difference of the radii and reads a double root as none
   or as two. CGAL, with exact square roots, agrees with every
   answer of 40008 pairs of linear sequences over inclusive bounds; an ever or always walk decided
   at its first segment counts 10 instructions more than master, of 17000. With it, #3650 reads a
   segment of the circular buffers at its turning points alone.
   The meeting of two temporal points is decided on their shared time parameter, exactly, before
   it is dated, for the planar and the geodetic points alike. On master `615eddb294` two points
   meeting between two microseconds read as never meeting: `ever_eq_tgeo_tgeo` and
   `eintersects_tgeo_tgeo` answer false on 15 of 15 meetings at an odd number of microseconds
   (A from `(-h 0)` to `(h 0)`, B from `(0 -h)` to `(0 h)`, h = 0.1, 1, 10, D = 3 to 3600000003 us),
   tgeompoint and tgeogpoint alike, since the walks compare the two positions constructed at the
   dated instant (DESKTOP-B8RRPID's probe, 36 rows a type). The PLANAR case is decided by
   `tgeompointsegm_meet`: the difference of the points moves linearly from `start1 - start2` to
   `end1 - end2`, so they meet strictly inside the period where every coordinate of it changes
   sign strictly or is zero at both ends and the two differences are collinear, a zero
   `cross_product_sign` in each pair of coordinates; both walks then read the one point they meet
   at, as they do for geodetic points (MobilityDB #3670, merged, the
   owner's go of 2026-11-13; CGAL agrees on 4000 rows of 2D and 3D meetings, their one-ulp moves
   and random pairs). The GEODETIC case stays this step's: its band, `geogpoint_meeting_band`,
   goes with it, since a band at a point the engine constructs is removed by no longer
   constructing the point (the open question of #3656).
   A planar meeting `tgeompointsegm_meet` decides is still DATED by truncation:
   `tsegment_intersection` reaches `tgeompointsegm_distance_turnpt`, which states
   `lower + (TimestampTz) (duration * fraction)` of a closest-approach fraction and drops a
   fraction within `MEOS_EPSILON` of a bound, so a meeting decided true near a bound reads no
   instant. These are the 6848 planar disagreements of the crossprobe corpus, all one
   microsecond early. Every moving coordinate of the difference of the two points crosses zero
   at the one parameter of the meeting, so the crossing of #3640 on one such coordinate dates
   it to the exact nearest microsecond, with its bound ownership: MobilityDB #3683 (open),
   `tgeompointsegm_crossing`, the point twin of `tnumbersegm_crossing`, and the ever and always
   walk deciding a planar meeting in continuous time whichever microsecond dates it; CGAL agrees
   on the 15001 planar crossprobe rows and 9000 meetings near a bound.
4. **The remaining turning points:** unary functions, bearing, `tnumber_mathfuncs.c`.
5. **The raster grid walks** of `raster_quadbin.c`: `tpointseq_raster_point_traverse` dates each
   line of the grid a trip crosses as `inst1->t + (TimestampTz) (dt * s)` and reads the value at
   the fraction `s`, not at the instant it states; the walks at its lines 891 and 1055–1059 date
   their crossings alike. Witness, the second listing of Sect. "Nearest and Bilinear Values" of
   chapter 5 of the Vol2 book, over the raster `Depth` of the companion's
   `chapter5/map_algebra.sql`:

   ```sql
   SELECT asText(rasterValue(tgeompoint '[SRID=25832;Point(600020 6400150)@2025-01-01 08:00,
     SRID=25832;Point(600380 6400150)@2025-01-01 08:06]', Rast, 1, true, 'bilinear'))
   FROM Depth;
   ```

   The trip runs along the centers of the third row, whose values 10, 12 and 14 lie on a line,
   so the answer the book prints merges them: `[9@08:00:00, 9@08:00:30, 10@08:02:10,
   14@08:05:30, 14@08:06:00]`. Master `c2fa764b22` dates the crossing of the center at exactly
   08:03:50 one microsecond early and reads its value there, so the points stop being collinear
   and stay: `[9@08:00:00, 9@08:00:30, 10@08:02:10, 11@08:03:00,
   12.0000000000001@08:03:49.999999, 13.000000000000203@08:04:40, 14@08:05:30,
   14@08:06:00]`. The turning-point listing that follows it in the book, whose crossings fall on
   whole minutes of a trip of 4 minutes, already answers as printed. The companion's
   `chapter5/expected/fields_along_trips.out` still holds the error this listing raised before
   `rasterValue` read bilinear values along a trip; it takes the printed answer when this step
   merges.

Steps 2–5 carry the dating rule of section 7. If the committers keep truncation instead, the
steps still apply with truncation computed exactly. The #2852 expected output then moves by one
microsecond, to a result that is equally correct at every microsecond.

**Schedule.** DECIDED BY THE OWNER 2026-11-12: steps 2–5 proceed beside the open rows of
[Exact distance](EXACT-DISTANCE.md) (ED24, #3484, is a draft), not after them. The dating rule of
section 7 enters only through the shared primitive, every lifted form of a crossing at once. A
geodetic meeting is decided, in a pull request apart, where the two great-circle points at the
instant of closest approach lie within a distance band sized to the rounding of their construction
(`tgeogpointsegm_intersection`), and the two crossing walks of `lifting.c` evaluate the function on
one point there; that pull request keeps master's dating, and step 3 decides the geodetic meeting on
the parameter of the segments, without constructing the points, and removes the band. Steps 2–5 share their readers in `lifting.c` with
[Any number of turning points in the lifting](N-TURNING-POINTS.md). The order is: that note's
first pull request (one turning-point callback per arity, with the same results), then steps 2–5
here on the merged readers, then that note's intervals of any number and its rigid geometries.
