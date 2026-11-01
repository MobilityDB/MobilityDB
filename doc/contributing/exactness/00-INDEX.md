<!--
  MobilityDB — Exactness: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Exactness

MEOS computes its geometric and numeric answers natively. These notes state, operation by
operation, how close each answer is to the exact one, measured, the oracle that judges it, and the
pull requests that make it exact, in order. A note that asks the committers for a decision says
so; nothing a note proposes is merged until its own pull request is. The names, the engines and
the bindings are the subject of the portable naming notes of MobilityDB #2861.

## Rules every pull request follows

- **One topic per pull request.** Its commits are the logical steps of that topic, and no commit
  undoes an earlier one on the same branch.
- **Correctness is judged by the exact reference on the branch's own answers**: CGAL's exact
  rational kernel on the doubles the engine holds for straight edges, a closed form or CGAL's
  circular-arc support for arcs. A comparison with master shows what changed and never what is
  right.
- **A decline is a failure.** A change that turns an answer into a decline does not merge on the
  strength of the answers it keeps, and every data set states its decline floor.
- **Speed is measured against GEOS called directly**, in instructions, in one process, and a speed
  figure counts only when both sides answer the same rows.
- **Every number in a pull-request message is measured on its head**, and names its data set and
  its probe.

## The notes

| Note | What it states |
|---|---|
| [Exact distance](EXACT-DISTANCE.md) | Every distance MEOS computes, measured against the double nearest the exact distance, the oracle that judges it, and the pull requests that make each one answer that double (ED1–ED18) |
| [Exact buffer](EXACT-BUFFER.md) | The pull requests that make the buffer exact on every input and the relationship of two areas exact, in order, what each must prove, and the decisions and open items they leave |
| [Geodetic operations](GEODETIC-OPERATIONS.md) | The rules that decide which operations a temporal geography supports and how exactly, the places where MobilityDB departs from them, measured against PostGIS geography, and the branches that correct them |
| [Geodetic boxes](GEODETIC-BOXES.md) | The operations that read a geodetic box as something other than the longitude/latitude rectangle the manual defines, measured, and the pull requests that correct them |
| [Exact crossings in the lifting infrastructure](EXACT-CROSSINGS.md) | Which microsecond dates a crossing that falls between two microseconds, and whether ever/always and the temporal result read the crossing in the same time |
| [Any number of turning points in a binary lift](N-TURNING-POINTS.md) | Whether the lifting of a function of two arguments accepts any number of turning points per segment, as the unary lift already does, so that the temporal rigid geometries and the geodetic points stop keeping walks of their own |
| [The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md) | How the distance of a temporal rigid geometry answers a body or a static polygon that is not convex or has holes, which master refuses, and whether the minimum over parts carries its corners at their true instants |
| [Feasibility of the distance of non-convex and holed geometries](NON-CONVEX-DISTANCE-FEASIBILITY.md) | What master already provides for that plan, what is new, how the convex cover is made exact, where the difficulty and the risk lie, and in which order the work is delivered |
| [The relationships of a rigid geometry in linear motion](RIGID-SWEPT-RELATIONSHIPS.md) | Which ever and always relationships of a temporal rigid geometry are refused for linear motion, why its traversed area, its instants and its distance at zero do not answer them, the event algorithm that answers them exactly, its risks and the pull request that delivers it |
| [Every MEOS value is a finite number](FINITE-VALUES.md), MobilityDB #3352 | The rule (NaN and ±Infinity refused at every entry point, a result of finite operands beyond the doubles an error, a result within them answered, a division by zero an error, -0 equal to 0), the census of where MEOS does not apply it, and the pull requests that apply it, in order: the NaN work, the finite rule and the double error value |
| [Exact text output of floating-point values](EXACT-FLOAT-TEXT.md) | Whether the default text of a floating-point value becomes the shortest form that reads back as the same double, and in which notation |

## Where things stand

- **Every distance answers the double nearest the exact distance**: every row of
  [Exact distance](EXACT-DISTANCE.md) is merged, ED15 last as MobilityDB #3269 (`d06cb0d010`) and
  #3272 (`0c6883d4e7`), and ED17's relate items (4) to (6) as #3226. The work that starts after it,
  [Exact crossings](EXACT-CROSSINGS.md) steps 2–4, [Any number of turning points](N-TURNING-POINTS.md)
  and [The distance of non-convex and holed geometries](NON-CONVEX-DISTANCE.md), interleaved as those
  notes order it, is open to start.
- **The smallest circle enclosing a geometry is exact**: MobilityDB #3105 (`adbb2ebad1`) searches
  the double nearest its radius by a polynomial of degree six, a term of an exact polynomial
  multiplying up to six exact sums (`POLY_TERM_MAXDEG` of `meos/include/geo/geo_funcs.h`).
  Its shuffle (`mec_shuffle` of `meos/src/geo/postgis_funcs.c`) draws from the C library's
  `rand()` on master; MobilityDB #3292 (branch `fix/enclosing-circle-shuffle-own-generator`,
  `e7ecbb2163`) draws from a `pg_prng` generator of its own call. CGAL agrees with every circle of
  its head over the 1400 geometries of the corpus, and the circle cells cost 0.42, 0.90, 0.79 and
  0.27 of GEOS called directly.
- **The buffer drops a hole by the exact radius of the smallest circle enclosing it**:
  `mec_radius_cmp` of `meos/src/geo/postgis_funcs.c` compares that radius with the distance from the
  points defining the circle, which it never constructs.
- **The buffer of a polygon with round joins is exact**: MobilityDB #3205 (`ddb9be7089`) reads it
  off the exact arrangement of its offset curves; CGAL's circle-segment traits agree on 3979820 of
  3979820 sharp queries and 1313 of 1313 natural areas. Every other input of the buffer follows in
  [Exact buffer](EXACT-BUFFER.md), steps 17 to 24.
- **A tower question stops computing in expansions once they leave their exact range**: MobilityDB
  #3211 (`30f7513014`). `tx_add` and `tx_mul` of `meos/src/geo/geo_funcs.c` return their first
  operand once the arena is unsafe, the run's answer being discarded for the one in WideDyadic. Left
  from the same report: the order of a segment and an arc leaving one tangent point alike
  (ARR_TURN), and the winding numbers about a node of a ring whose spike revisits its own
  coordinates (ARR_DEPTH).
- **The buffer at extreme coordinates follows steps 17 to 24 of [Exact buffer](EXACT-BUFFER.md)**,
  as the owner decided on 2026-10-27: a shape whose tower needs more than 160 limbs declines; a
  distance below the rounding of the coordinates, where the written offsets fall onto the input,
  is declined once the older path is removed; the sign of a cross product whose terms overflow
  (`cross_product_sign`) is its own pull request.
- **The buffer places the four meetings of its offsets and circles in plain doubles**: MobilityDB
  #3259 (draft), branch `perf/place-the-corner-crossing-in-doubles` (`5b7ed0e97a`), computes the
  crossing of two offsets sharing a centre (every concave corner), of two offsets sharing none, the
  meeting of an offset with a circle and the crossing of two circles of one radius in the order
  their filters do, so the coordinates are the same bits, under one bound written out from the
  filters' rules, the filters placing what that bound leaves.
- **The written buffer keeps the exact topology where a rounding certificate says so**: every one
  of its decisions is an exact sign, CGAL finds no answer it certifies wrong (1 199 of 1 199
  natural areas certified), and it owes its speed (1.045 of GEOS called directly at distance 1);
  [Exact buffer](EXACT-BUFFER.md), where these steps stand.
- **Open pull requests** on 2026-10-30: this one and MobilityDB #3259.
