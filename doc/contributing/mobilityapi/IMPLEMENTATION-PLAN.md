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
| Three abstract tests that state a response are discharged by their route alone, and four response rows name no test that asserts them | A1: every row stating a response, over the fixture database or a scripted backend, names in `atsDischargedBy` the test that reads the answer, held by `TestATSEveryAnsweredRowIsDischarged` over every test file; the three query rows are response rows, acceleration's naming the test that pins its 501 and its reason | MERGED as MobilityAPI #82 (`1d6cbd0081`) |
| Resolutions 1 to 5 of the wiki page, the edits of the normative schemas, have no implementation: MobilityAPI validates against the unmodified schemas and records each contradiction in a test | A2: the resolved schema, the vendored bundle with resolutions 1 to 5 applied by `ats_resolved_schema_test.go`, which reads the vendored copy and never edits it, and every document MobilityAPI writes and every example OGC publishes validated against it in CI. Measured under either reading of `format`: every MobilityAPI document valid, 5 of 16 published examples failing (the five wrong examples), no document valid against the published schema failing the resolved one, and the documents each resolution refuses stated as cases. The measurement revised resolution 3 (the three-character `form` branch is kept as a UN/CEFACT common code, which the standard's own examples write) and resolution 5 (an RFC 3339 pattern beside `format: date-time`), and the wiki page states both | MERGED as MobilityAPI #83 (`c764dfa005`) |
| Resolution 6, the corrected abstract tests A.20, A.26 and A.27, exists as text only | A8: executable rows for the three corrected test methods beside the published ones, each discharged by the answer it states (A.27 under the interpolations it names): `atsPart1Corrected` in `ats_part1_corrected_test.go`, keyed by the identifier of the test each corrects. The acceleration query reads the geometry's interpolation with `interp` (joining the generated `featureFunctions`) and states the reason for Linear, Step, Discrete and a single instant, and answers 404 for a geometry that does not exist, where it answered 501 with the Linear reason to every request | MERGED as MobilityAPI #84 (`7f604ddee0`) |
| Resolution 7, a CI job validating every published example against every published schema, needs an implementation the standard's repository can adopt | A9: the job, a test file over a vendored copy of the bundle, as the standard's repository would run it, in MobilityAPI's CI | DONE: `TestATSSchemaPublishedExamples` on MobilityAPI `master` validates the 16 published examples against their schemas under both readings of `format` (10 and 13 failing) |
| The resolutions are on the wiki page and in no issue of OGC's repository | A10: each resolution filed with the Moving Features Standards Working Group (`opengeospatial/ogcapi-movingfeatures`) only once its implementation row (A2, A8 or A9) is merged and green in CI, quoting the issue, the resolution and that implementation as its evidence; the owner files it | after A2, A8, A9 |
| No abstract test of Part 4 is read, though `cquery` is declared | A3: the abstract tests of the Part 4 class `cquery`, read from the standard's repository, as executable rows like those of Part 1 | to do |

## The other standards declared

| Gap | Task | State |
|---|---|---|
| Part 4, Common, Coverages Part 1 and EDR 1.1 are declared and have no row in the conformance ledger and no page | A7: a ledger row and a wiki page for each, as Part 1 has, stating what backs the claim and each point the standard leaves open with its resolution, the media type of CoverageJSON (`prs.coverage+json` in EDR 1.1, `vnd.cov+json` in Coverages Part 1) the first | to do |

## The extensions

| Gap | Task | State |
|---|---|---|
| No test states that each extension is named as one and changes no answer of the standard | A4: every route of the API definition is either a requirement of a declared class or marked as a MobilityAPI extension, and a test reads the routing table against that marking. `routeTable` holds every operation with its class or `extension`; the routing and the API definition (`x-conformance-class`, `x-mobilityapi-extension`) both read it, and `ats_routes_test.go` fails a route marked with an undeclared class, a route undescribed, an operation unserved, and an Annex A route marked otherwise than its test's class. 45 routes: 33 of a declared class, 12 extensions, `/health`, `trajectories` and `timeseries` among them, which the API definition describes from this row on | OPEN as MobilityAPI #85 |

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

A1 first, then A2, A8 and A9, the production implementation of each resolution of Part 1, which
MobilityAPI is the implementation of; A10 only after them, since no resolution goes to OGC before
its implementation is merged and green; A3 and A7 for the other classes MobilityAPI declares; A4
once the extensions are listed; A5 then A6, so that every campaign measures MobilityAPI as it
measures a binding.
