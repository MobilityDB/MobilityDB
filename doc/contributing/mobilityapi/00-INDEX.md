<!--
  MobilityDB — MobilityAPI, a conformant OGC API – Moving Features: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityAPI, a conformant OGC API – Moving Features

MobilityAPI (`MobilityDB/MobilityAPI`) is the server of the MobilityDB ecosystem: a Go tier over
MobilityDB that publishes moving features through OGC API – Moving Features, Part 1: Core
(OGC 22-003r3), and rasters through OGC API – Coverages and OGC API – Environmental Data
Retrieval. It is a conformant implementation of OGC API – Moving Features: every conformance
class it declares is met as the standard's abstract tests state it, and what it adds beyond the
standard changes no answer the standard defines.

These notes state, for every conformance class MobilityAPI declares and every abstract test of
it, how the test is discharged, where a test is met only in form or not at all, where the
standard itself is defective, and the pull requests that close each gap.

The stance these notes follow is that of the wiki page [Standards conformance in
MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Standards-Conformance-in-MobilityDB): a
normative requirement outranks descriptive prose, a point the standard leaves open takes the OGC
Moving Features answer or the one best semantically, and each resolution is published as a
candidate the standards body can adopt. The page [Implementing OGC API — Moving Features in
MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB)
is what a reader of the conformance claim reads: the evidence behind it, the nine issues of the
standard and seven candidate resolutions. These notes plan the work that page and its ledger
row reach; they restate neither.

The raster endpoints are the rows R7 to R10 of the raster plan of MobilityDB #3775
(`doc/contributing/raster/`), and the formats MobilityAPI reads and writes are rows of the input
and output plan of MobilityDB #3788 (`doc/contributing/io/`); these notes link to those rows and
restate none of them.

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | The conformance classes MobilityAPI declares, how each abstract test of Part 1 is discharged, the tests met in form only, the defects of Annex A, the extensions, and how the surface of a server is measured |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The pull request or request to OGC that closes each gap, and its state |
| [The filings with OGC](ogc-filings/README.md) | The texts of the issues and comments filed with the OGC Standards Working Groups, each stating the defect, the resolution, how MobilityAPI solves it and the evidence |

## The rules

1. **Conformance is judged by the abstract tests.** A conformance class MobilityAPI declares is
   met when every abstract test of the class holds against the fixture, each test discharged by
   the answer it states: a test that states a response is discharged by that response, never by
   the existence of the route alone.
2. **A defect of the standard goes to OGC with its implementation.** Where the standard cannot
   be met as written, its candidate resolution is stated on the wiki page of that standard and
   implemented production grade: merged, tested against the resolution's own cases and green in
   CI. Only then is it filed with the standards body that owns it, that implementation its
   evidence. Until the standard is corrected MobilityAPI follows the resolution, records the
   contradiction in a test, and fakes nothing.
3. **An extension changes no answer of the standard.** What MobilityAPI serves beyond the
   classes it declares (bulk ingest, export, the time series and trajectories views, the raster
   query of a temporal geometry, the stream delivery) is named as an extension in the API
   definition, and no request the standard defines answers differently because of it.
4. **What MobilityAPI answers is what MobilityDB declares.** The temporal types a collection can
   hold, the functions a query applies to them, and the raster types are generated from the
   MEOS-API catalog (`catalog_gen.go`, `tools/codegen`); a query MobilityDB states no function for
   is refused naming the function.
5. **A server is measured by its own surface.** MobilityAPI holds no object layer, so the
   matrices that measure a binding (`tools/raster_matrix.py`, `tools/io_matrix.py`) do not
   measure it; MobilityAPI states its surface itself, route by route, and its own tests read that
   surface against the catalog.

## Where things stand

- MobilityAPI declares 13 conformance classes: three of Moving Features Part 1, `cquery` of
  Part 4, two of OGC API – Common, six of Coverages Part 1 and the core of EDR 1.1.
- The 45 abstract tests of Part 1, Annex A, are executable (`ats_part1_test.go`): 17 discharged
  by a live answer, 18 by the route, 7 by the response shape and 3 by the service description;
  each of the 24 that state a response names the test reading it (MobilityAPI #82).
- The wiki page of Part 1 states nine issues of the standard and seven candidate resolutions;
  no issue of `opengeospatial/ogcapi-movingfeatures` carries them.
- The class `cquery` of Part 4 is tested for its declaration only, and the ledger of the
  conformance page lists Part 1 alone of the standards MobilityAPI declares classes of.
