<!--
  MobilityDB — Exactness: what the exact crossings establish
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# What the exact crossings establish

[Exact crossings](EXACT-CROSSINGS.md) is the plan: which crossings MEOS finds, how it dates them
and in which order the pull requests change it. This note states the rules those pull requests
make hold, the families of input on which a crossing or a meeting is decided exactly, how each
answer is judged, what keeps the exact walks as fast as the approximate ones, and what is left.
Master at the time of writing: `97b437e49c`.

## 1. The rules

- **Time is continuous.** Two temporal values cross or meet at an instant of continuous time,
  which is almost never a whole microsecond. Every answer about a crossing is the answer at the
  exact crossing.
- **A crossing is dated by the microsecond nearest to it, a tie going to the even timestamp.**
  The date is decided exactly: the microsecond `m` whose window `(m - 1/2, m + 1/2]` holds the
  crossing is confirmed by the exact signs of the two segments at the ends of the window
  (`tnumbersegm_crossing`, MobilityDB #3640). A truncated date is one microsecond early for every
  crossing between two microseconds whose fraction rounds up.
- **A crossing dated at a bound states no instant inside the segment.** The ever and always forms
  still read it: `[0@0, 1@0.001] ?= 0.0000003` is true while no instant of `#=` is. The ever and
  always walks decide a crossing in continuous time, from the signs at the ends of the segment,
  whichever microsecond dates it.
- **An end a sequence excludes is no instant of the time two values share.** Two points within a
  distance only at such an end are never within it there (MobilityDB #3680), and an ever or always
  relationship never reads a value at an excluded bound (MobilityDB #3650).
- **No answer is read on a constructed value.** A value interpolated at the dated instant is
  rounded, so two segments that meet construct two points apart by their rounding. The walk
  reads the one value the two take at the crossing, and the meeting itself is decided on the
  doubles of the ends, never on constructed points. A band within which two constructed points
  count as one goes with the construction (`geogpoint_meeting_band`).
- **One crossing, one decision, every lifted form.** The comparison, the distance, the
  restriction and the ever and always forms of one crossing read the same primitive
  (`tnumbersegm_crossing` for numbers, `tgeompointsegm_crossing` for planar points,
  `tgeogpointsegm_crossing` for geography points), so they date it at the same instant. A change
  made walk by walk dates one crossing at two instants in half of the cases measured.
- **A new branch of a walk goes beside the merged ones.** The ever and always walk holds one test
  per kind of operand (two floats, two planar points, two geography points), each added next to
  the others, so a pull request adds lines to a merged walk and rewrites none of them.
- **Prove or refuse.** Where the inputs do not prove a meeting, none is stated. Where they prove
  that a meeting exists but not its instant, the ever and always forms state it and no instant
  dates it. No tolerance stands in for a proof.

## 2. Where a crossing is decided exactly

| Operands | Decided on | Dated |
|---|---|---|
| Two numbers, a number and a value | the signs of the gap at the ends of the segment | exactly, the window of the nearest microsecond |
| Two planar points | each coordinate of their difference changes sign and the differences at the ends are collinear (`tgeompointsegm_meet`) | on one coordinate that changes sign (`tgeompointsegm_crossing`) |
| Two planar points within a distance | the squared distance, a convex quadratic in time, in wide exact arithmetic (`point_motion_dwithin_exact`), and where its least value is reached | exactly, through the same quadratic |
| Two geography points on the equator | their longitudes, which move linearly (`pointsegm_meet_coord`, as for planar points) | exactly, on the longitude |
| Two geography points on one meridian | their latitudes, which move linearly | exactly, on the latitude |
| A geography segment and its reverse | they meet at the middle | exactly, at the middle of the period |
| A geography segment and its mirror across the equator, the latitudes of its ends opposite | they meet where the first crosses the equator, at its middle: a half turn about the point of the equator at the middle longitude exchanges its ends | exactly, at the middle of the period |
| A geography segment and its mirror across the equator, other latitudes | they meet where the first crosses the equator | not dated |
| Two geography segments on different great circles | not decided: their meeting is the coincidence of two angles computed in floating trigonometry | none stated |

A Z coordinate moves linearly and is read as one more coordinate in every row. For two
geography points the ever and always within at the distance 0 is their ever and always
equality, so the two forms read one decision.

## 3. How an answer is judged

- **The oracle is CGAL** in exact rationals on the doubles the engine holds, never master. A
  comparison with master shows what changed; where master is wrong, an identity with it carries
  the error forward.
- **A corpus is built so that its truth is exact.** Random segments rarely meet, and their
  meetings fall at irrational instants. The corpora that judge a meeting are built from integer
  directions: the difference of two points is `s v` at the start and `-r v` at the end, `v`, `s`,
  `r` integers held exactly, so the two meet at the rational parameter `s / (s + r)`; periods of
  one to seven microseconds put meetings on, between and halfway between microseconds; families
  near a bound put them within a microsecond of it; near misses 1e-9 to 1e-3 apart never meet.
- **Every judge has two controls** computed without libmeos: one that the judge accepts on every
  row, and one, the same answers with every dated meeting moved one microsecond, that it refuses
  on exactly the rows moved. A judge that cannot refuse a wrong answer judges nothing.
- **What the rule refuses is measured, not hidden.** The mirror family meets in truth and is not
  dated; it is counted apart from the judged corpus and stated in the pull request.

The judges, their corpora, generators and controls are kept in the oracle kit of the
`claude-kits` repository, one directory per pull request.

## 4. What keeps the exact walks fast

- **The common path stays master's.** A rare case (a tie, an excluded end, a meeting at a bound)
  is answered off the path every interval takes, where the hot function returns at once. The
  ever walk stops at the first interval that decides it, so a check run only on that interval
  costs one evaluation per query.
- **Return what a computation already knows.** The squared distance of two moving points knows
  where its least value lies when it computes it; returning that position replaces the
  multiplications and comparisons a caller would spend to find it again.
- **Keep values dead across calls.** A callback that needs its sequences after a call saves them
  to memory on every interval, measured at one instruction per call in every walk sharing the
  callback. One callback per question keeps each walk's own path short.
- **Measure per function.** Callgrind per function, on a long walk and on the corpus split into
  the rows whose answers move and those that do not, locates a cost to the instruction; a total
  alone does not.

## 5. Measured

| Pull request | Corpus | Result |
|---|---|---|
| MobilityDB #3640, the exact crossing of floats | 548989 segments | 533988 float segments dated at the exact nearest microsecond |
| MobilityDB #3680, ever within at shared instants | 40000 random pairs of planar points | 210 within the distance only at an excluded bound, every answer agreeing with CGAL |
| MobilityDB #3683, the planar meeting dated | 15001 planar crossprobe rows and 9000 meetings near a bound | 24001 agree with CGAL; meetings on periods of a few microseconds whose truncated date fell on the start raised "Timestamps for temporal value must be increasing" |
| MobilityDB #3693, the geodetic meeting proved | 8000 pairs on the equator, on a meridian, reversed and near misses, and 2000 mirrors whose end latitudes are not opposite | 8000 agree with CGAL; the 2000 mirrors ever equal and ever within the distance 0, none dated |

## 6. What is left

- **The geodetic within distance above 0.** Its turning point function solves a quadratic in
  longitude and latitude, which is not the distance on the sphere, so the ever and always walk
  reads the middle of a segment where the function states no turning point. The turning points
  of the geodetic distance (`GeogSegmPair`) replace it, and the middle read goes with them.
- **Two geography segments on different great circles.** Their meeting is decided where an exact
  computation on the doubles proves it; none exists for a general pair. Section 7 states how the
  case is decided.
- **The remaining turning points** of [Exact crossings](EXACT-CROSSINGS.md) step 4: the unary
  functions, the bearing and the numeric functions of `tnumber_mathfuncs.c`.
- **The raster grid walks** of step 5, which date the crossings of a grid by truncation and read
  the value at the fraction rather than at the instant they state.

## 7. Deciding two geography segments on different great circles

Two geography segments on different great circles meet only at a point where the two circles
cross, and only where both segments reach that point at the same fraction of their period. Each
fraction is an angle computed in trigonometry from the input degrees, a transcendental number
for a generic input, so the equality of the two fractions cannot be read off a computation. The
case splits into two answers that a computation can prove.

- **No meeting, proved by separation.** The gap between the two fractions, or the least
  distance of the two positions, is computed with a guaranteed error bound; where its lower bound
  is above zero the segments provably never meet. Doubles with an error analysis decide almost
  every pair, and a pair the bound cannot separate is computed again in the wide exact arithmetic
  of the engine (`TowerWide`) at a higher precision. The bound requires trigonometric functions
  whose error is guaranteed: the correctly rounded `sin`, `cos` and `atan2` of the CORE-MATH
  project, vendored apart from the first-party code, give every trigonometric value within half
  a unit in the last place. Two segments whose ends are rounded off one great circle are
  separated this way at the precision that exposes the rounding.
- **A meeting, proved by structure.** An exact meeting of two segments on different great circles
  comes from a symmetry of their ends, recognised exactly on the input doubles as the families of
  section 2 are. Segments meet at the middle of their period where the middles of their arcs
  coincide, and the middle of an arc is an exact point where the segment is symmetric about a
  known point: a meridian segment whose end latitudes are opposite and an equator segment
  symmetric about the same longitude meet at the middle on two different great circles.
- **Neither.** A pair that no family proves to meet and no precision separates is answered as
  undecided, an explicit refusal, never a guess; the corpora count how many pairs reach it.

The same certified evaluation dates a proved meeting whose instant is transcendental, as for a
segment and its mirror whose end latitudes are not opposite: the meeting parameter is computed
within a guaranteed bound, and the nearest microsecond is proved where the bound clears the
half-microsecond between two candidates, a tie being possible only at a rational instant, which
a family states exactly. The turning points of the geodetic within distance above 0, the other
item of section 6, read the same certified trigonometry.

The pull requests, in order: the vendored correctly rounded trigonometry; the certified
separation of two geography segments and the certified date of a proved meeting; the middle
symmetry family; and the measure of the undecided pairs on real trips and on built corpora,
judged in CGAL and MPFR.
