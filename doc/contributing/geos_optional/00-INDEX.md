<!--
  MobilityDB — Making GEOS optional: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Making GEOS optional

These notes describe one campaign of work in MobilityDB from end to end: why it exists, what is
done, what is left, how every answer is judged, on which data, and in which order the rest should
be done. They are written for a committer who has never heard of the campaign, and they gather in
one public place what the campaign learned, including the approaches that failed. Each note reads
on its own from top to bottom, and every term is explained where it first appears.

**Measured at** MobilityDB master `d9b9c11985`, on 2026-10-04. Every statement about what the code
does is the output of a program in [`tools/`](tools/), kept in [`tools/results/`](tools/results/)
so that it can be checked and re-run; every figure about speed or correctness is attributed to the
pull request or commit whose message states it. The reachability inventory and the merge census are
re-read at master `0bf0dab28c`, which adds MobilityDB #2946 to `d9b9c11985`.

## Words used throughout

Each note defines its own terms where they first appear; these few recur everywhere.

- **GEOS** — the C++ geometry library PostGIS uses. **MEOS** — MobilityDB's core C library, which
  also runs outside PostgreSQL.
- **Native** — answered by MEOS's own code, without GEOS.
- **Fall-back** — the code that hands a question to GEOS when no native code answers it. In a
  build without GEOS the same place raises an error instead.
- **Decline** — the native code refuses to answer and raises an error, rather than answering
  wrongly. A decline is not a correct answer; it counts as a failure.
- **Public function** — a function of the MEOS C library that a program or a binding may call;
  each has an SQL counterpart named in the manual.
- **CGAL** — a C++ geometry library that computes in exact rational arithmetic. It is used in the
  tests to decide what the right answer is; it is never part of MobilityDB itself.
- **Closed form** — a formula giving the exact answer, such as the area of a disc, πr².
- **Polygonize** — replace each circular arc of a shape by a chain of straight segments, which is
  what GEOS does before computing on a curved shape.
- **Master** — the main branch of the MobilityDB repository. **#NNNN** — the pull request of
  that number on github.com/MobilityDB/MobilityDB, whose description states the figures cited.
- **Speed ratio** — the instructions the native code executes divided by those GEOS executes on
  the same data, both counted in one run: below 1, MEOS is faster.

## The campaign in one paragraph

MobilityDB computes geometry — whether two shapes intersect, the region within a distance of a
shape, the union of many lines — partly through GEOS. Some places MobilityDB should run cannot
carry GEOS: small devices and real-time operating systems, where a large C++ library with its own
memory management and exceptions is not acceptable. So MEOS carries its own implementation of the
geometry operations GEOS answers for it. The goal is that a MEOS built **without** GEOS answers
every question a MEOS built **with** GEOS answers, answers it correctly, and answers it at least
as fast.

## Summary: what is done

- **A MEOS without GEOS builds.** Configured with `-DGEOS=OFF`, MEOS compiles with 0 warnings
  ([`tools/results/build_nogeos.txt`](tools/results/build_nogeos.txt)) and references no GEOS
  symbol. With the raster family built, GEOS is still loaded into the
  process by the system's GDAL, which links it; MEOS calls none of it
  ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.2).
- **2917 of its 3051 public functions have no path to GEOS at all**: nothing they call, at any
  depth, reaches GEOS ([note 5](05-WHAT-STILL-NEEDS-GEOS.md) §5.4). Among them are every
  operation on plain geometries except the intersection, the difference and the two unions: the
  spatial relationships (intersects, contains, touches, covers, the relationship matrix), the
  buffer, the convex hull, the oriented envelope, the centroid, the simplicity test, the four
  clusterings, distance and equality.
- **The other 134 have a path, and most of what they are asked answers.** Run in the build
  without GEOS on the inputs that are hardest for them, 322 of 337 calls answer, 11 reach GEOS —
  all of them on temporal geometries — and 4 are a refusal by design or a result with no value
  (§5.6).
- **The answers are judged by an exact reference**, CGAL or a closed form, never by GEOS for a
  curved shape and never by an earlier MobilityDB ([note 2](02-JUDGING-AN-ANSWER.md)). Where a
  shape carries a circular arc, MEOS keeps the arc exact, where GEOS polygonizes it.
- **The spatial relationships and the unions run faster than GEOS** on the campaign's data sets.
  That is a speed measure only; their correctness is judged by CGAL ([note 4](04-OPERATIONS.md)).

## Summary: what remains

Each item is measured; the note named gives the evidence.

1. **Multi-part shapes and collections still reach GEOS** ([note 5](05-WHAT-STILL-NEEDS-GEOS.md)
   §5.5–§5.6). The union of an array of geometries, and the union of a collection, reach GEOS
   through a fall-back exactly when one member is a `MULTIPOLYGON`, a `MULTISURFACE`, a `TIN` or a
   `POLYHEDRALSURFACE` (104 of 225 pairs of types). The intersection and the difference of a
   collection reach GEOS in 409 and 953 of 6750 calls. The public functions this reaches on
   ordinary data are `traversedArea` of a temporal geometry with such a value, `atGeometry` with
   such a value and a collection, `traversedArea` of a rigid geometry built on a polyhedral
   surface, and `merge` of a temporal geometry, which reports the refusal as an error since
   MobilityDB #2946 (§5.7). MobilityDB #2966 reads such a member face by face: the array union
   and the union of a collection then reach GEOS for 0 of 225, and the overlays of a collection
   for 64 and 24 of 6750.
2. **Arrays of overlapping surfaces expose three more gaps** ([note 5](05-WHAT-STILL-NEEDS-GEOS.md)
   §5.8), measured on 1500 random arrays judged by CGAL. Surfaces carrying Z, two of which merge
   while another stays apart, raise an error in every build, and the default error handler ends
   the process; MobilityDB #2954, merged as `29f082c762`, answers them. Three surfaces can reach
   GEOS in one order and answer in another, because a vertex the union constructs is rounded near
   another member's edge; reading every member's edges in one arrangement answers them
   (MobilityDB #2961, merged as `100c77a8d4`). And 76 of 1500 answers spell a hole as a shell that touches itself, which covers the
   right points and is not a valid OGC polygon; MobilityDB #2959, merged as `42ebd6312b`, writes 8 of them as a shell and
   a hole.
3. **No continuous-integration job builds without GEOS**, so any of this can grow unseen (§5.2).
4. **The buffer is not finished** ([note 6](06-BUFFER.md)). Its answer depends on where the shape
   lies — the same small square buffered by the same distance has the right area at the origin and
   an area a third too small at a projected coordinate such as 6 400 000 m, measured on master by
   [`tools/square_buffer.c`](tools/square_buffer.c) — and it is between about 2.8 and 4.4 times
   slower than GEOS on large real polygons. A repair for the first problem is written
   and measured, not yet proposed as a pull request; it uncovers an older defect in the last stage
   of the buffer, where the boundary pieces are joined into closed rings, and the two are
   repaired in one change.
5. **The single-sided buffer is refused.** PostGIS offers it (`ST_Buffer` with `side=left` or
   `side=right`); the committers decided it must answer exactly as PostGIS does
   ([note 4](04-OPERATIONS.md) §4.11).
6. **The test data sets live outside the repository** ([note 3](03-TEST-DATA.md)).
7. **Then GEOS leaves the shipped library.** The fall-backs, the build option and the stubs are
   deleted, and GEOS stays only as a test reference. The committers decided this on 2026-09-22;
   items 1 and 2 are what stands between that decision and its execution.

The [implementation plan](IMPLEMENTATION-PLAN.md) turns these items into an ordered list of pull
requests.

## The notes

| Note | What it covers |
|---|---|
| [1 — Why GEOS is being made optional](01-OBJECTIVE.md) | Where GEOS sits in MobilityDB, why it does not fit a real-time system, how a build without it works, and what "finished" means |
| [2 — How an answer is judged](02-JUDGING-AN-ANSWER.md) | Correctness by exact arithmetic, why GEOS cannot judge a curved answer, speed against GEOS, and the rules that follow |
| [3 — The test data](03-TEST-DATA.md) | Every data set the campaign measures on: where it comes from, what it holds, what it judges |
| [4 — The operations, one by one](04-OPERATIONS.md) | For each geometry operation: what it does, how it is answered, the evidence, what is left |
| [5 — What still needs GEOS](05-WHAT-STILL-NEEDS-GEOS.md) | Which MEOS functions have a path to GEOS, which inputs take it, and the public functions that do not work without GEOS |
| [6 — The buffer](06-BUFFER.md) | The answer that depends on position: the symptom, its cause, the repair, and why it cannot ship alone |
| [7 — Lessons, and approaches ruled out](07-LESSONS.md) | The rules the campaign paid to learn, and the dead ends nobody should try again |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The remaining work as pull requests, what each must prove, and the decisions owed |
| [Appendix A — The functions with a path to GEOS](A-INVENTORY.md) | All 134, each with its path and what a run without GEOS shows |
| [`tools/`](tools/) | The programs every measurement of these notes comes from, and their output |

## Status of these notes

They are working notes for discussion, like the other notes in `doc/contributing/`. The data sets
of note 3 are kept outside the tree; the programs that establish what still needs GEOS are in
[`tools/`](tools/) and run against any MEOS build.

A second document is owed when the campaign closes: a chapter of the user manual explaining the
native engine to someone deciding whether to trust it in place of GEOS, stating the differences
plainly, including where MEOS is slower or narrower. These notes are its raw material.
