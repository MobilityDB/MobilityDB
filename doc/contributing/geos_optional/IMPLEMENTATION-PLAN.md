<!--
  MobilityDB — Making GEOS optional: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Implementation plan

**Measured at** MobilityDB master `d9b9c11985`.

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
| 5 | The intersection and the difference answer a collection operand | The sweep reads 0 of 6750 for both; CGAL agrees with every answer. MobilityDB #2970 lists a collection member of the array union component by component: the intersection reads 0 of 6750, the difference 24, through pieces whose boundaries run along one another; of 2000 intersections of a collection of two valid surfaces with a valid surface, 12 decline where 287 did, and CGAL agrees with every answer by its point set |
| 6 | A continuous-integration job builds MEOS with `-DGEOS=OFF` and runs the sweep and the gateway probe | It fails when any count of "reaches the fall-back" is above 0, and its two controls pass |

Step 3 is expected to close part of step 5, since the overlay hands collections to the array
union; step 5 re-runs the sweep after it and takes what remains.

## Part II — the buffer

| # | Step | What clears it |
|---|---|---|
| 7 | The buffer ledger (generator, probe, scorer) and a reduced exact-reference corpus enter the repository, with a test that runs them, if decision 1 is adopted | It reproduces the census of note 6 §6.5 on master, and its fault-injection control turns it red (note 7 §7.1) |
| 8 | The code the buffer and the overlay share for cutting boundary pieces where they cross and chaining them moves from `geo_buffer.c` to a file of its own, `geo_topology.c` (written, as the first four commits of the branch carrying the repair, note 6 §6.5) | Changes no answer: the ledger, the containment corpus and the real polygons read the same on its head as on its base, and CGAL agrees on every answer |
| 9 | **One change**: the chaining receives the element type it reads; the rings are assembled by a traversal that spends a side, not a piece; the ring area is taken about a point of the ring; the buffer is built in a frame chosen from the geometry and the distance (note 6 §6.4–§6.8) | The move and scale arms of the ledger (note 6 §6.5) read 0; declines no higher than the base on the ledger and on the 192 real polygons; CGAL agrees on every query of the ledger, the containment corpus and the curved-buffer corpora |
| 10 | The buffer answers a hole that shrinks to exactly a point | The 10 × 10 square at 6 400 000 answers at hole half-width 0.001, with the exact area on either side |
| 11 | The buffer reaches speed parity on the natural areas | Every buffer cell at or below 1 against GEOS called directly; the first commit is a fresh profile of master after step 9 |
| 12 | The single-sided buffer | The PostGIS 3.6.4 reference answers of note 4 §4.11, parameters included |

Step 9 is the one design change of the plan, and its four parts cannot be split: the frame alone
loses seven real polygons that answered before, and the traversal alone has no witness without
the frame (note 6 §6.6).

## Part III — GEOS leaves the shipped library

| # | Step | What clears it |
|---|---|---|
| 13 | The four fall-backs of `postgis_funcs.c` are deleted with the conversion helpers they use, and their error messages with them | The job of step 6 reads 0 on every count; `check_liblwgeom_geos.py` carries no geometry entry point |
| 14 | The `GEOS` build option, the stand-in header `geos-none/geos_c.h`, the stubs `geo_lwgeom_none.c` and `geo_geos_none.c`, the GEOS context in `meos.c` and the GEOS version string are deleted; GEOS stays linked into the test probes only | `ldd` on `libmeos.so` shows no GEOS library loaded, which with the raster family on needs a GDAL built without GEOS (note 5 §5.2); the SQL regression tests answer as expected; the probes keep a GEOS build, called directly, as the speed reference and as the witness for straight edges, and never a witness for an arc |

## Part IV — the user-facing chapter

| # | Step | What clears it |
|---|---|---|
| 15 | A chapter of the manual on the native engine, in English and Spanish, for a user deciding whether to trust it in place of GEOS | Every example in it runs and its output is harvested, not written by hand; it states where MEOS differs from GEOS — circular arcs first, each judged by the closed form — and where it is slower or narrower |

## Decisions owed by the committers

1. **Do the instruments and an exact-reference corpus enter the repository's tests?** The sweep
   and the gateway probe are in this pull request's `tools/` and need only a MEOS build; the
   buffer ledger and the CGAL judges need data sets and CGAL as a test-only dependency.
2. **Is the decline floor a merge criterion for the repository?** The campaign applies it: a pull
   request does not merge when it turns an answer into a decline, even if every answer it keeps is
   correct. Step 9 is designed around this rule.
3. **The `prec` parameter of `geom_unary_union`.** No caller passes a grid and no SQL function
   calls the function. Dropping the parameter is an API change; keeping it means refusing a
   non-negative value once GEOS is gone (step 13).
4. **The licence of the protected-area polygons** must be confirmed before any of them is
   committed as test data.

## Open items, stated rather than buried

- **111 of the 134 functions with a path to GEOS are not run individually** (appendix A). Each
  shares a gateway with functions that are; step 6's job is where running all of them belongs.
- **Four CGAL disagreements of 32 668 on the buffer ledger**, recorded by the campaign's run of
  2026-09-30, are not attributed (note 6 §6.5):
  they may be the judge rather than the engine, and are resolved before either is called wrong.
- **44 closed-form deviations of the buffer ledger** sit on both arms of step 9 and are not
  attributed.
- **No exact-reference run is recorded for convex hull, oriented envelope, centroid and the
  simplicity test** (note 4 §4.6).
- **The offset curve and make_valid are exposed by no function.** Their behaviour is decided
  (note 4 §4.10), and no step builds them until a function exposes them.
