<!--
  MobilityDB — MobilityAPI, a conformant OGC API – Moving Features: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityAPI, a conformant OGC API – Moving Features: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

## The abstract tests

| Gap | Task | State |
|---|---|---|
| Three abstract tests that state a response are discharged by their route alone | A1: the rows `tpgeometry-query-distance`, `-velocity` and `-acceleration` of `ats_part1_test.go` become live rows reading the answer the test states: a TReal curve for distance and velocity, for acceleration the 501 with its reason that issue 3 of the wiki page records; the ledger row of Part 1 then counts what each test reads | to do |
| The seven candidate resolutions of Part 1 are on its wiki page and in no issue of OGC's repository | A2: each resolution filed with the Moving Features Standards Working Group as an issue of `opengeospatial/ogcapi-movingfeatures`, quoting the issue from the published artefacts, the resolution and MobilityAPI's tests as the evidence it is implementable; the wiki page links each issue | to do; filed once the owner approves the text |
| No abstract test of Part 4 is read, though `cquery` is declared | A3: the abstract tests of the Part 4 class `cquery`, read from the standard's repository, as executable rows like those of Part 1 | to do |

## The other standards declared

| Gap | Task | State |
|---|---|---|
| Part 4, Common, Coverages Part 1 and EDR 1.1 are declared and have no row in the conformance ledger and no page | A7: a ledger row and a wiki page for each, as Part 1 has, stating what backs the claim and each point the standard leaves open with its resolution, the media type of CoverageJSON (`prs.coverage+json` in EDR 1.1, `vnd.cov+json` in Coverages Part 1) the first | to do |

## The extensions

| Gap | Task | State |
|---|---|---|
| No test states that each extension is named as one and changes no answer of the standard | A4: every route of the API definition is either a requirement of a declared class or marked as a MobilityAPI extension, and a test reads the routing table against that marking | to do |

## The surface

| Gap | Task | State |
|---|---|---|
| The binding matrices read nothing in a server | A5: MobilityAPI writes its surface, route by route: the conformance class or extension of each route and the SQL functions it applies, per collection type, read from the generated tables and from the calls `trip_functions_test.go` collects; written by `go run . -emit` beside the samples and checked like them | to do |
| The raster and the input and output matrices do not measure MobilityAPI | A6: `tools/raster_matrix.py` and `tools/io_matrix.py` read A5's surface, so that MobilityAPI is credited by the functions its routes apply, as a binding is by its object layer | after A5 |

## The rows of other plans

| Plan | Rows |
|---|---|
| Rasters, MobilityDB #3775 | R7 the coverage (MobilityAPI #79), R8 the EDR queries (#80), R9 the raster query of a temporal geometry (#81), R10 the raster types generated from the catalog (#77) |
| Input and output, MobilityDB #3788 | IO-B14, its Spark dialect names `asMFJSON` and `<type>FromMFJSON` (MobilityAPI #74) |

## Order

A1 and A2 first, the abstract tests and the standard of Part 1, which MobilityAPI is the
implementation of; A3 and A7 for the other classes it declares; A4 once the extensions are
listed; A5 then A6, so that every campaign measures MobilityAPI as it measures a binding.
