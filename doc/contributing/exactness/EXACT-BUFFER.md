<!--
  MobilityDB — Exactness: Exact buffer
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Exact buffer

[Back to the index](00-INDEX.md)

MEOS computes the buffer, the relationships and the overlays of geometries natively. This document
states the pull requests that make the buffer exact on every input and the relationship of two
areas exact, in order, what each must prove, the decisions they owe the committers, and the open
items. The notes it cites are those of MobilityDB #2945 at `1d333712e6`
([`doc/contributing/geos_optional/`](https://github.com/MobilityDB/MobilityDB/tree/1d333712e6/doc/contributing/geos_optional));
"note 6 §6.5" is section 6.5 of
[`06-BUFFER.md`](https://github.com/MobilityDB/MobilityDB/blob/1d333712e6/doc/contributing/geos_optional/06-BUFFER.md),
and likewise for notes 2
([`02-JUDGING-AN-ANSWER.md`](https://github.com/MobilityDB/MobilityDB/blob/1d333712e6/doc/contributing/geos_optional/02-JUDGING-AN-ANSWER.md)),
4 ([`04-OPERATIONS.md`](https://github.com/MobilityDB/MobilityDB/blob/1d333712e6/doc/contributing/geos_optional/04-OPERATIONS.md))
and 7 ([`07-LESSONS.md`](https://github.com/MobilityDB/MobilityDB/blob/1d333712e6/doc/contributing/geos_optional/07-LESSONS.md)).
The step numbers are those the notes use.

## Rules every pull request follows

- **One topic per pull request.** Its commits are the logical steps of that topic, and no commit
  undoes an earlier one on the same branch.
- **Correctness is judged by the exact reference on the branch's own answers** (note 2, §2.1):
  CGAL for straight edges, a closed form or CGAL's circular-arc support for arcs. A comparison with
  master shows what changed and never what is right.
- **A decline is a failure** (note 2, §2.5). A change that turns an answer into a decline does not
  merge on the strength of the answers it keeps, and every data set states its decline floor.
- **Speed is measured against GEOS called directly**, in instructions, in one process (note 2,
  §2.6), and a speed figure counts only when both sides answer the same rows.
- **Every number in a pull-request message is measured on its head**, and names its data set and
  its probe.

## The buffer ledger and the shared topology

| # | Step | What clears it |
|---|---|---|
| 7 | The buffer ledger (generator, probe, scorer) and a reduced exact-reference corpus enter the repository, with a test that runs them, if decision 1 is adopted | It reproduces the census of note 6 §6.5 on master, and its fault-injection control turns it red (note 7 §7.1) |
| 8 | The code the buffer and the overlay share for cutting boundary pieces where they cross and chaining them moves from `geo_buffer.c` to a file of its own, `geo_topology.c` (written, as the first four commits of the branch carrying the repair, note 6 §6.5) | Changes no answer: the ledger, the containment corpus and the real polygons read the same on its head as on its base, and CGAL agrees on every answer. The branch is on master `a362004728`, before MobilityDB #3040 builds the buffer of a polygon with round joins from an exact arrangement; it is rebuilt on master before it is proposed |
| 10 | The buffer answers a hole that shrinks to exactly a point | The lattice square of the ledger, `POLYGON((0 0,10 0,10 10,0 10,0 0),(4 4,4 6,6 6,6 4,4 4))` buffered by 1, answers with the hole closed, and the holes narrower and wider than it answer with the exact area on either side; the 10 × 10 square at 6 400 000 written in decimal is not this case in doubles (note 6 §6.8) |
| 12 | The single-sided buffer | The reference answers of note 4 §4.11, parameters included, which PostGIS 3.6.3 and GEOS 3.14.1 called directly give alike for straight edges; a curve by its closed form. On a polygon those answers carry no band, which the committers confirm before the step is built |

The buffer stands at or below GEOS called directly on the natural areas: MobilityDB #3040
(`8867a28596`) states 0.989, 0.684 and 0.899 at distances 1, 50 and 500 and 0.499 on the small
shapes, and MobilityDB #3159 (`3c25b6966f`) states 0.947 and 0.990 at distances 1 and 500 once no
ring of the natural areas takes the older piece overlay (note 6 §6.9).

## The user-facing chapter

| # | Step | What clears it |
|---|---|---|
| 15 | A chapter of the manual on the native engine, in English and Spanish, for a user deciding whether to trust it | Every example in it runs and its output is harvested, not written by hand; it states where MEOS differs from GEOS — circular arcs first, each judged by the closed form — and where it is slower or narrower |

## The relationships on the noded arrangement

| # | Step | What clears it |
|---|---|---|
| 16 | The relationship of two areal operands is read off the noded arrangement of `geo_arrange.c`, the one the buffer reads, so no constructed point is located and no tolerance decides a cell; where the arrangement leaves a question unanswered, the engine as it stands answers it. Branch `fix/relate-areas-on-the-noded-arrangement` | The sliver of note 4 §4.1 reads 212111212 at every L from 10^3 to 10^15, CGAL deciding its cells; every corpus of §4.1 agrees with CGAL on the branch's head; every relationship speed cell at or below GEOS called directly |

## The buffer, exact on every input

The buffer of a polygon with round joins is read off the exact arrangement of its offset curves,
each join the arc of the circle about its vertex (MobilityDB #3205, `ddb9be7089`). These steps
bring every other input of the buffer to that arrangement, one pull request each, in this order;
the older path is deleted last. Each is judged by CGAL on its own head, by the exact distance from
a point to the input for round joins and by the exact union of the polygon, the rectangle each
edge sweeps and the corner pieces for bevel and mitre joins, and every one of its speed cells
stands at or below GEOS called directly. Inputs with extreme coordinates come after them (open
items).

| # | Step | What clears it |
|---|---|---|
| 17 | Lines with round joins and round caps: each line is walked out and back along its offset curve, and an offset whose written end does not lie strictly on its side of the edge it offsets declines rather than answers. Branch `fix/buffer-a-line-exactly` | Measured on its head on master `cdfa50524d`: of 336 lines with a first vertex at 2^20 to 2^60 and spans past 2^1000, 238 agree with CGAL, 0 wrong, 98 decline (with the side test forced true, 5 wrong: its control); of 36 960 straight queries 30 960 agree, 0 wrong, 6 000 decline, every closed line. Owed: the 1 576 queries on AIS lines, their decline floor against the older path, and the speed cells of lines and polygons |
| 18 | The buffer is built in a frame chosen from the geometry and the distance. Branch `fix/buffer-in-a-frame-next` | Built on step 17's head: CGAL agrees with every answer of the far-vertex, straight and AIS corpora, with no decline the line path alone answers; the open square of side s buffered by 8 s holds its centre (`049_geo_funcs`) |
| 19 | Lines with square and flat caps | `LINESTRING(0 0,10 0)` buffered by 1 with `endcap=square` is the rectangle from (−1 −1) to (11 1), where master writes the degenerate ring `CURVEPOLYGON(COMPOUNDCURVE((0 1,0 1)))`; `LINESTRING(0 0,10 0,0 0.5)` with `endcap=flat` has no spurious second ring; CGAL agrees on corpora of both caps |
| 20 | Polygons with bevel joins: the join is the chord where the ring turns away from its offset side, and the offset runs back through the vertex where it turns towards it, so the winding number of the offset curve is the count of the polygon, edge rectangles and corner triangles holding a point, holes included. Branch `geo/buffer-exact-mitre-bevel`, five commits on master `d043de2e04` | CGAL on its head: 167 094 of 167 094 sharp queries and 159 846 of 159 846 natural, and the round corpora unchanged (1 313 of 1 313 natural, 1 328 512 of 1 328 512 sharp, 49 354 of 49 354 on 40 shapes). **Blocked on speed**: native over GEOS in instructions on the natural areas reads 1.40, 1.32 and 1.59 at distances 1, 50 and 500 |
| 21 | Polygons with mitre joins: the crossing of the two offset lines and, past the limit, the two cut points become exact segment ends; past the limit the corner is cut square to the bisector at limit × r from the vertex, as GEOS cuts it | Master writes a bevel past the limit, which disagrees with GEOS's limited mitre at 438 of 148 200 points at limit 5 and 1 008 at limit 1.5; on the head CGAL agrees with every query at limits 5, 1.5 and 1.1 |
| 22 | Polygons buffered by a negative distance with round joins, on the same arrangement | CGAL agrees with every answer, the decline floor no higher than master's on the natural areas |
| 23 | Points, inputs with circular arcs, and collections of mixed dimension | Each by CGAL, arcs by CGAL's circular-arc support |
| 24 | The older buffer path is deleted, and polygons narrower than an ulp of the output decline as lines do | No input reaches the older path; the corpora of steps 17 to 23 read the same |

Step 18 is built on step 17, since on the older line path the frame moves lines the older path
declines into the range where it answers them wrong; alone, the frame loses seven real polygons
that answer without it (note 6 §6.6). Step 20 renames the round-join functions of `geo_buffer.c`
to the join style, so whichever of steps 17 and 20 lands second rebases across the other. Step 16
adds one optional base weight per input curve to the merge of overlapping stretches in
`geo_arrange.c`, the file steps 20 to 22 change: whichever lands second rebases across it.

## Decisions owed by the committers

1. **Do the instruments and an exact-reference corpus enter the repository's tests?** The buffer
   ledger and the CGAL judges need data sets and CGAL as a test-only dependency.
2. **Is the decline floor a merge criterion for the repository?** The steps apply it: a pull
   request does not merge when it turns an answer into a decline, even if every answer it keeps is
   correct.
3. **The licence of the protected-area polygons** must be confirmed before any of them is
   committed as test data.

## Open items

- **108 of the 129 functions that reach the native overlay, union and relationship routes are not
  run one by one** (appendix A of the notes). Each shares a route with functions that are, and an
  input of theirs that no native route reads is refused, in every build.
- **Four CGAL disagreements of 32 668 on the buffer ledger** are not attributed (note 6 §6.5):
  they may be the judge rather than the engine, and are resolved before either is called wrong.
- **44 closed-form deviations of the buffer ledger** sit at the scales 2^−24 and 2^−27 alone and
  are the probe's area function: measured by `geom_area` in place of `lwgeom_area`, the closed-form
  arm reads 0 areas off the formula (note 6 §6.5).
- **No exact-reference run is recorded for convex hull, oriented envelope and the simplicity
  test** (note 4 §4.6). The centroid has one: MobilityDB #3171 (`d51b32e664`) states CGAL agreeing
  with every answer of its head over 75 rows in six classes.
- **The buffer at extreme coordinates** follows steps 17 to 24: an exact product can exceed the
  fixed width of the exact arithmetic from coordinates near 10^301, and a sign test past overflow,
  the node index and the cost of arcs at such spreads are owed. Inputs reaching them decline.
- **The offset curve and make_valid are exposed by no function.** Their behaviour is decided
  (note 4 §4.10), and no step builds them until a function exposes them.
