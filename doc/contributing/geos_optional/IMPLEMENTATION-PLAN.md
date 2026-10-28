<!--
  MobilityDB — Making GEOS optional: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Implementation plan

**Measured at** MobilityDB master `d9b9c11985`; the state of each step is read again at master
`52b0c9e4d5`.

This plan turns the remaining work of the [index](00-INDEX.md) into pull requests, in order. Each
step names what it must prove. The notes it cites explain the terms; "the sweep" and "the gateway
probe" are [`tools/geos_fallback_sweep.c`](tools/geos_fallback_sweep.c) and
[`tools/gateway_reach.c`](tools/gateway_reach.c), run in a MEOS built with `-DGEOS=OFF`.

## Rules proposed for every pull request

The campaign works by these rules; decisions 1 and 2 below ask the committers to adopt them for
the repository.

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

## Part I — what still needs GEOS

These steps close the gaps of [note 5](05-WHAT-STILL-NEEDS-GEOS.md). They come first because they
are what keeps GEOS in the shipped library.

| # | Step | What clears it |
|---|---|---|
| 1 | The merge of temporal-geometry instants checks the union it receives, and reports the refusal instead of passing it on (`tgeoinst_merge_array_iter`, note 5 §5.7) | **Merged** as MobilityDB #2946 (`0bf0dab28c`): `tools/merge_sweep.c` reads 0 crashes of 450 merges with it (325 at `d9b9c11985`), and `meos/test/merge_validity_test.c` carries a case refused in every build |
| 2 | The native union dissolves surfaces carrying Z or M as their projection, and the array union reads the ordinates back (note 5 §5.8) | **Merged** as MobilityDB #2954 (`29f082c762`): of 1500 arrays of `POLYGON Z` and `TRIANGLE Z`, 881 decline without it and none with it; CGAL agrees with every answer and with the Z of every vertex; `052_tgeo` carries the merge of three such surfaces at one timestamp, which raised an error in every build |
| 3 | The array union answers members that are a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN` or a `POLYHEDRALSURFACE` | The sweep reads 0 of 225 for the array union in every Z mode and 0 of 225 for the union of a collection; CGAL agrees with every answer by the measure of its point set. **Merged** as MobilityDB #2966 (`d40d7a885f`), which reads every face of such a member on the plane, an upright face as its ring's line: the sweep reads 0 of 225 in every Z mode and 0 of 225 for a collection, the overlays of a collection 64 and 24 of 6750 |
| 4 | The native union answers surfaces in any order, reading every member's edges in one arrangement rather than merging them a pair at a time (note 5 §5.8) | The three triangles of note 5 §5.8 answer in every order; the 3 flat declines of `tools/results/aunion_flat.master_path.txt` answer; CGAL agrees with every answer. **Merged** as MobilityDB #2961 (`100c77a8d4`), on MobilityDB #2959, merged as `42ebd6312b`, which records the side a shared piece's answer lies on; on it all 1500 flat arrays answer, and an array the pairwise merge answers keeps that answer |
| 5 | The intersection and the difference answer a collection operand | The sweep reads 0 of 6750 for both; CGAL agrees with every answer. **Merged** as MobilityDB #2970 (`2ce5aea42a`), which lists a collection member of the array union component by component: the intersection reads 0 of 6750, the difference 24, through pieces whose boundaries run along one another; of 2000 intersections of a collection of two valid surfaces with a valid surface, 12 decline where 287 did, and CGAL agrees with every answer by its point set. **Merged** with MobilityDB #2975 (`742cd87b58`), which places a piece whose midpoint reads on the other boundary by an end of it that is not a node, located exactly: the sweep then reads 0 in every count, the 2000 intersections answer and CGAL agrees with each, and the differences decline 1 where they declined 9 |
| 6 | A continuous-integration job builds MEOS with `-DGEOS=OFF` and runs the sweep and the gateway probe | It fails when any count of "reaches the fall-back" is above 0, and its two controls pass. **Merged** as MobilityDB #2977 (`4c4457d2ec`), which adds the job `geos-free` to `meos.yml`: it builds MEOS with `-DGEOS=OFF`, checks that the library calls no GEOS function, and runs `meos/test/geos_fallback_sweep.c` and `meos/test/geos_gateway_reach.c`, each exiting 1 where an input reaches a fall-back or a control fails; the sweep exits 0 on #2975 and 1 on master, where 24 inputs reach one |

Step 3 is expected to close part of step 5, since the overlay hands collections to the array
union; step 5 re-runs the sweep after it and takes what remains.

**Part I is finished.** On master `52b0c9e4d5` the sweep reads 0 in every count but its positive
control, the unary union on a precision grid; the gateway probe reads 333 of 337 calls answered and
0 reaching a fall-back; and the probes of the public functions that reached GEOS answer every
input ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.9).

## Part II — the buffer

| # | Step | What clears it |
|---|---|---|
| 7 | The buffer ledger (generator, probe, scorer) and a reduced exact-reference corpus enter the repository, with a test that runs them, if decision 1 is adopted | It reproduces the census of note 6 §6.5 on master, and its fault-injection control turns it red (note 7 §7.1) |
| 8 | The code the buffer and the overlay share for cutting boundary pieces where they cross and chaining them moves from `geo_buffer.c` to a file of its own, `geo_topology.c` (written, as the first four commits of the branch carrying the repair, note 6 §6.5) | Changes no answer: the ledger, the containment corpus and the real polygons read the same on its head as on its base, and CGAL agrees on every answer. The branch is on master `a362004728`, before MobilityDB #3040 builds the buffer of a polygon with round joins from an exact arrangement; it is rebuilt on master before it is proposed |
| 9 | **One change**: the chaining receives the element type it reads; the rings are assembled by a traversal that spends a side, not a piece; the ring area is taken about a point of the ring; the buffer is built in a frame chosen from the geometry and the distance (note 6 §6.4–§6.8) | The move and scale arms of the ledger (note 6 §6.5) read 0; declines no higher than the base on the ledger and on the 192 real polygons; CGAL agrees on every query of the ledger, the containment corpus and the curved-buffer corpora. On master `52b0c9e4d5` the two arms disagree only for the open line nearly closing and the square with a hole (75 and 84 groups), and the ledger declines 114 of 1840; the step is measured again on master, for those two families, before its design is kept (note 6 §6.10) |
| 10 | The buffer answers a hole that shrinks to exactly a point | The lattice square of the ledger, `POLYGON((0 0,10 0,10 10,0 10,0 0),(4 4,4 6,6 6,6 4,4 4))` buffered by 1, answers with the hole closed, and the holes narrower and wider than it answer with the exact area on either side; the 10 × 10 square at 6 400 000 written in decimal is not this case in doubles (note 6 §6.8) |
| 11 | The buffer reaches speed parity on the natural areas | Every buffer cell at or below 1 against GEOS called directly. **Reached** by MobilityDB #3040 (`8867a28596`), which states 0.989, 0.684 and 0.899 at distances 1, 50 and 500 and 0.499 on the small shapes, and MobilityDB #3159 (`3c25b6966f`), which states 0.947 and 0.990 at distances 1 and 500 once no ring of the natural areas takes the older piece overlay (note 6 §6.9) |
| 12 | The single-sided buffer | The reference answers of note 4 §4.11, parameters included, which PostGIS 3.6.3 and GEOS 3.14.1 called directly give alike for straight edges; a curve by its closed form. On a polygon those answers carry no band, which the committers confirm before the step is built |

Step 9 is the one design change of the plan, and its four parts cannot be split: the frame alone
loses seven real polygons that answered before, and the traversal alone has no witness without
the frame (note 6 §6.6). Since MobilityDB #3040 and #3205 the rings of a polygon with round joins
are labelled on the exact arrangement, so of step 9 the frame remains; it is step 18 of Part VI,
built on the exact line buffer of step 17, since on the older line path the frame moves lines the
older path declines into the range where it answers them wrong.

## Part III — GEOS leaves the shipped library

| # | Step | What clears it |
|---|---|---|
| 13 | The `prec` parameter of `geom_unary_union` is dropped (decision 3), and the four fall-backs of `postgis_funcs.c` are deleted with the conversion helpers they use, in one pull request: the union on a precision grid is the sweep's one positive control, so the sweep and the gateway probe leave the job of step 6 with it | **Merged** as MobilityDB #3179 (`9bb3bb7a2b`). Its first commit takes `geom_unary_union(gs)` and retires the sweep, the gateway probe and their step; its second ends the intersection, the difference, the array union and the unary union in a refusal, the same in every build, for an input none of their native routes reads, and deletes `POSTGIS2GEOS` and `GEOS2POSTGIS`. The job of step 6 keeps its build without GEOS and its check that `libmeos.so` calls no GEOS function; `check_liblwgeom_geos.py` carries no geometry entry point, its baseline holding the raster core's `rt_raster_gdal_polygonize` alone. CGAL's exact kernel agrees with every answer of its head on 20 witnesses, 40 controls passing; the four union cells read 0.249 to 0.381 of GEOS in instructions |
| 14 | The `GEOS` build option and `cmake/FindGEOS.cmake`, the GEOS context in `meos.c` with `geos_get_context()`, and the GEOS version in `mobilitydb_full_version()` are deleted (decision 5); the workflows and the Dockerfile stop installing GEOS for the build. `geos-none/geos_c.h`, `geo_lwgeom_none.c` and `geo_geos_none.c` stay and are compiled in every build: the vendored geometry library (`lwgeom_wrapx.c`, through `lwgeom_geos.h`) and the raster core call GEOS entry points, `GEOSUnaryUnion` and `GEOSisValid` among them, which these files answer natively. GEOS stays linked into the test probes only | **Merged** as MobilityDB #3185 (`1a7a1a0e11`), with no release of deprecation; `meos/examples/geos_clip.c`, built on the GEOS context, goes with it. `libmeos.so` and the extension call no GEOS function and export no GEOS name: `geo_geos_none.c` is an object library whose symbols are hidden, since exported they answer the GEOS calls of a PostGIS loaded in the same backend, which aborts it (note 1 §1.3). The MEOS build job of `meos.yml` checks both, and `pgversion.yml` runs a session calling MobilityDB, then PostGIS's `ST_IsValid` and `ST_UnaryUnion` and the raster `ST_Intersects`. CGAL's exact kernel agrees with every answer of its head on 20 witnesses, 40 controls passing; the four union cells read 0.249 to 0.380 of GEOS in instructions. `ldd` on `libmeos.so` still lists GEOS where the raster family links a GDAL that links it (note 5 §5.2) |

## Part IV — the user-facing chapter

| # | Step | What clears it |
|---|---|---|
| 15 | A chapter of the manual on the native engine, in English and Spanish, for a user deciding whether to trust it in place of GEOS | Every example in it runs and its output is harvested, not written by hand; it states where MEOS differs from GEOS — circular arcs first, each judged by the closed form — and where it is slower or narrower |

## Part V — the relationships on the noded arrangement

| # | Step | What clears it |
|---|---|---|
| 16 | The relationship of two areal operands is read off the noded arrangement of `geo_arrange.c`, the one the buffer reads, so no constructed point is located and no tolerance decides a cell; where the arrangement leaves a question unanswered, the engine as it stands answers it. Branch `fix/relate-areas-on-the-noded-arrangement` | The sliver of note 4 §4.1 reads 212111212 at every L from 10^3 to 10^15, CGAL deciding its cells; every corpus of §4.1 agrees with CGAL on the branch's head; every relationship speed cell at or below GEOS called directly |

## Part VI — the buffer, exact on every input

Since MobilityDB #3205 (`ddb9be7089`) the buffer of a polygon with round joins is read off the
exact arrangement of its offset curves, each join the arc of the circle about its vertex. These
steps bring every other input of the buffer to that arrangement, one pull request each, in this
order; the older path is deleted last. Each is judged by CGAL on its own head, by the exact
distance from a point to the input for round joins and by the exact union of the polygon, the
rectangle each edge sweeps and the corner pieces for bevel and mitre joins, and every one of its
speed cells stands at or below GEOS called directly. Inputs with extreme coordinates come after
them (open items).

| # | Step | What clears it |
|---|---|---|
| 17 | Lines with round joins and round caps: each line is walked out and back along its offset curve, and an offset whose written end does not lie strictly on its side of the edge it offsets declines rather than answers. Branch `fix/buffer-a-line-exactly` | Measured on its head before its rebase on master `cdfa50524d`: of 336 lines with a first vertex at 2^20 to 2^60 and spans past 2^1000, 238 agree with CGAL, 0 wrong, 98 decline (with the side test forced true, 5 wrong: its control); of 36 960 straight queries 30 960 agree, 0 wrong, 6 000 decline, every closed line. Owed: the 1 576 queries on AIS lines, their decline floor against the older path, and the speed cells of lines and polygons |
| 18 | The frame of step 9: the buffer is built in a frame chosen from the geometry and the distance. Branch `fix/buffer-in-a-frame-next` | Built on step 17's head: CGAL agrees with every answer of the far-vertex, straight and AIS corpora, with no decline the line path alone answers; the open square of side s buffered by 8 s holds its centre (`049_geo_funcs`) |
| 19 | Lines with square and flat caps | `LINESTRING(0 0,10 0)` buffered by 1 with `endcap=square` is the rectangle from (−1 −1) to (11 1), where master writes the degenerate ring `CURVEPOLYGON(COMPOUNDCURVE((0 1,0 1)))`; `LINESTRING(0 0,10 0,0 0.5)` with `endcap=flat` has no spurious second ring; CGAL agrees on corpora of both caps |
| 20 | Polygons with bevel joins: the join is the chord where the ring turns away from its offset side, and the offset runs back through the vertex where it turns towards it, so the winding number of the offset curve is the count of the polygon, edge rectangles and corner triangles holding a point, holes included. Branch `geo/buffer-exact-mitre-bevel`, five commits on master `d043de2e04` | CGAL on its head: 167 094 of 167 094 sharp queries and 159 846 of 159 846 natural, and the round corpora unchanged (1 313 of 1 313 natural, 1 328 512 of 1 328 512 sharp, 49 354 of 49 354 on 40 shapes). **Blocked on speed**: native over GEOS in instructions on the natural areas reads 1.40, 1.32 and 1.59 at distances 1, 50 and 500 |
| 21 | Polygons with mitre joins: the crossing of the two offset lines and, past the limit, the two cut points become exact segment ends; past the limit the corner is cut square to the bisector at limit × r from the vertex, as GEOS cuts it | Master writes a bevel past the limit, which disagrees with GEOS's limited mitre at 438 of 148 200 points at limit 5 and 1 008 at limit 1.5; on the head CGAL agrees with every query at limits 5, 1.5 and 1.1 |
| 22 | Polygons buffered by a negative distance with round joins, on the same arrangement | CGAL agrees with every answer, the decline floor no higher than master's on the natural areas |
| 23 | Points, inputs with circular arcs, and collections of mixed dimension | Each by CGAL, arcs by CGAL's circular-arc support |
| 24 | The older buffer path is deleted, and polygons narrower than an ulp of the output decline as lines do | No input reaches the older path; the corpora of steps 17 to 23 read the same |

Step 20 renames the round-join functions of `geo_buffer.c` to the join style, so whichever of
steps 17 and 20 lands second rebases across the other. Step 16 adds one optional base weight per
input curve to the merge of overlapping stretches in `geo_arrange.c`, the file steps 20 to 22
change: whichever lands second rebases across it.

## Decisions owed by the committers

1. **Do the instruments and an exact-reference corpus enter the repository's tests?** The sweep
   and the gateway probe are in this pull request's `tools/` and need only a MEOS build; the
   buffer ledger and the CGAL judges need data sets and CGAL as a test-only dependency.
2. **Is the decline floor a merge criterion for the repository?** The campaign applies it: a pull
   request does not merge when it turns an answer into a decline, even if every answer it keeps is
   correct. Step 9 is designed around this rule.
3. **The `prec` parameter of `geom_unary_union`.** No caller passes a grid and no SQL function
   calls the function. Dropping the parameter is an API change; keeping it means refusing a
   non-negative value once GEOS is gone (step 13). **Decided on 2026-10-25: it is dropped**, and
   `geom_unary_union(gs)` stays, since `trajectory(temp, true)`, `traversedArea(..., true)` and the
   raster core's union in a build without GEOS dissolve through it. What goes is the union snapped
   to a grid GEOS computes, which MEOS does not offer: rounding an answer afterwards can make it
   invalid, which the snapped union cannot. MobilityDB #3179 (step 13) drops it.
4. **The licence of the protected-area polygons** must be confirmed before any of them is
   committed as test data.
5. **What a build and a user see once GEOS is gone.** Decided on 2026-10-25: the `GEOS` build
   option and `geos_get_context()` are deleted with no release of deprecation, and
   `mobilitydb_full_version()` names no GEOS version; MobilityDB #3185 (step 14) does so and states
   it, and the release notes say so.

## Open items, stated rather than buried

- **108 of the 129 functions with a path to GEOS are not run individually** (appendix A, master
  `52b0c9e4d5`). Each shares a gateway with functions that are; step 6's job is where running all
  of them belongs. Since step 13 an input of theirs that no native route reads is refused, in
  every build.
- **Four CGAL disagreements of 32 668 on the buffer ledger**, recorded by the campaign's run of
  2026-09-30, are not attributed (note 6 §6.5):
  they may be the judge rather than the engine, and are resolved before either is called wrong.
- **44 closed-form deviations of the buffer ledger** sit on both arms of step 9. On master
  `52b0c9e4d5` they sit at the scales 2^−24 and 2^−27 alone and are the probe's area function:
  measured by `geom_area` in place of `lwgeom_area`, the closed-form arm reads 0 areas off the
  formula (note 6 §6.5).
- **No exact-reference run is recorded for convex hull, oriented envelope and the simplicity
  test** (note 4 §4.6). The centroid has one: MobilityDB #3171, merged as `d51b32e664`, states CGAL
  agreeing with every answer of its head over 75 rows in six classes.
- **The buffer at extreme coordinates** waits for Part VI: an exact product can exceed the
  fixed width of the exact arithmetic from coordinates near 10^301, and a sign test past
  overflow, the node index and the cost of arcs at such spreads are owed. Inputs reaching them
  decline until then.
- **The offset curve and make_valid are exposed by no function.** Their behaviour is decided
  (note 4 §4.10), and no step builds them until a function exposes them.
