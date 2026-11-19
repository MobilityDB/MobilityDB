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
| [Exact distance](EXACT-DISTANCE.md) | Every distance MEOS computes, measured against the double nearest the exact distance, the oracle that judges it, and the pull requests that make each one answer that double (ED1–ED19), and how an exact distance is computed |
| [Exact buffer](EXACT-BUFFER.md) | The pull requests that make the buffer exact on every input and the relationship of two areas exact, in order, what each must prove, and the decisions and open items they leave |
| [Geodetic operations](GEODETIC-OPERATIONS.md) | The rules that decide which operations a temporal geography supports and how exactly, the places where MobilityDB departs from them, measured against PostGIS geography, and the branches that correct them |
| [Geodetic boxes](GEODETIC-BOXES.md) | The operations that read a geodetic box as something other than the longitude/latitude rectangle the manual defines, measured, and the pull requests that correct them |
| [Exact crossings in the lifting infrastructure](EXACT-CROSSINGS.md) | Which microsecond dates a crossing that falls between two microseconds, and whether ever/always and the temporal result read the crossing in the same time |
| [What the exact crossings establish](EXACT-CROSSINGS-INSIGHTS.md) | The rules the exact-crossing pull requests make hold, the families of input on which a crossing or a meeting is decided exactly, how each answer is judged, what keeps the exact walks fast, and what is left |
| [Any number of turning points in a binary lift](N-TURNING-POINTS.md) | Whether the lifting of a function of two arguments accepts any number of turning points per segment, as the unary lift already does, so that the temporal rigid geometries and the geodetic points stop keeping walks of their own |
| [The temporal rigid geometries](../trgeometry/00-INDEX.md) | MobilityDB #3441 holds, as one plan of that type, the distance of non-convex and holed geometries with its feasibility, and the relationships of a rigid geometry in linear motion. |
| Every MEOS value is a finite number, MobilityDB #3352 (closed) | The rule (NaN and ±Infinity refused at every entry point, a result of finite operands beyond the doubles an error, a result within them answered, a division by zero an error, -0 equal to 0), the census of where MEOS does not apply it, and the pull requests that apply it, every one merged: the NaN work, the finite rule (MobilityDB #3361) and the double error value. The one topic it leaves, the exact area of a geometry, stands below |
| [Exact text output of floating-point values](EXACT-FLOAT-TEXT.md) | Whether the default text of a floating-point value becomes the shortest form that reads back as the same double, and in which notation |
| [Temporal poses and pose chains](EXACT-POSE.md) | Where a pose and a pose chain are answered approximately (the turn of a 2D pose decided by a tolerance, a composed joint as a constructed point, a composed joint held constant between two instants, the SLERP quaternion, the geographic composition), the exact answer of each, the survivors of the consolidation each reads, and their order after it |
| [The exact arithmetic of the geometry engine](EXACT-ARITHMETIC.md) | The three layers of the engine (user entries, geometric algorithms, exact arithmetic), the tiers every exact sign is read in, the machinery the engine holds for each, the rule of one machinery per need, the seam at the user entries, and the steps that merge the machinery into one |

## Where things stand

- **Every distance answers the double nearest the exact distance**: every row of
  [Exact distance](EXACT-DISTANCE.md) to ED18 is merged, ED15 last as MobilityDB #3269
  (`d06cb0d010`) and #3272 (`0c6883d4e7`), and ED17's relate items (4) to (6) as #3226. ED19, the
  temporal distance of a rigid body, is MobilityDB #3375 (`ba50756731`) and ED20, whether two static
  geometries of straight edges are within a distance, #3385 (`957a7a1e9f`). The engine is
  specified for its users in `doc/specs/meos-exact-geometry.md`, MobilityDB #3372, merged as `fe8c187b2b`, whose
  version v0.2.0 covers the distances, predicates and relationships. The work that starts after it,
  [Exact crossings](EXACT-CROSSINGS.md) steps 2–4, [Any number of turning points](N-TURNING-POINTS.md)
  and [The distance of non-convex and holed geometries](../trgeometry/NON-CONVEX-DISTANCE.md), interleaved as those
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
- **The buffer places the meetings and edges of its arrangement at less cost**: MobilityDB #3259
  (draft), branch `perf/place-the-corner-crossing-in-doubles` (`43fa350c5b` on master
  `91f8d676e7`), computes the crossing of two offsets sharing a centre (every concave corner), of
  two offsets sharing none, the meeting of an offset with a circle and the crossing of two circles
  of one radius in the order their filters do, so the coordinates are the same bits, under one
  bound written out from the filters' rules, the filters placing what that bound leaves, and
  writes the tangent and circle edges in their place.
- **The written buffer keeps the exact topology where a rounding certificate says so**: every one
  of its decisions is an exact sign, CGAL finds no answer it certifies wrong (1 199 of 1 199
  natural areas certified), and it owes its speed (1.045 of GEOS called directly at distance 1);
  [Exact buffer](EXACT-BUFFER.md), where these steps stand.
- **The area of a geometry is the double nearest its exact value**: `geom_area` answers it at
  every magnitude, an area beyond the largest double raising "value out of range" and a geometry
  holding a circular arc keeping its measure in doubles. MobilityDB #3557 (merged as
  `051f7e0920`) sums the rings of a geometry in one 128-bit integer on the unit of its box,
  one product a vertex on the coordinates themselves, four vertices a step, then in pairs of
  doubles with a bound on their rounding, placed among the doubles by `nearest_double_search`,
  then on `DyadicWide`: 3000 of 3000 rows answer the double nearest the exact area, as CGAL
  judges them, at kernel speed 0.818 on the natural cell and 0.699 on the area ledger (1.46M and
  12.9M instructions against GEOSArea's 1.19M and 8.99M) and route speed 8.9 and 6.4. The gap to
  GEOSArea's kernel is the exact sum itself: GEOS runs the same one-product loop in doubles and
  answers no nearest double, the integer loop converts each coordinate to its integer and sums in
  128 bits, about four instructions a vertex more, and no cheaper exact form exists on real
  coordinates, whose differences carry about 43 bits. The owner accepts that cost for the merge
  (2026-11-18, "then please explain all this in the body of the PR so we can merge it"), with no
  hardware fused multiply-add. It closes the last open topic of MobilityDB #3352.
- **The specification states the buffer**: MobilityDB #3843 (draft) gives
  `doc/specs/meos-exact-geometry.md` (v0.4.0) a section on the buffers the engine reads off the
  exact arrangement (polygons and erosions with round joins, lines with round joins and caps), their
  witnesses, what CGAL measured and their kernel speed, how a buffer's area is measured (arcs as
  arcs: the 48 closed-form deviations of chords at `2^-24` and `2^-27` are the measure, not the
  buffer), the union a relationship reads for a collection, and the buffers left on the older path.
  It stays a draft while steps 19 to 24 of [Exact buffer](EXACT-BUFFER.md) are open.
- **Open pull requests** of this plan on 2026-11-19: this one and MobilityDB #3843; MobilityDB
  #3259 is merged (2026-11-05).
