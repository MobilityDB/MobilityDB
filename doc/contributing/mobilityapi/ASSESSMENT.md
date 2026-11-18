<!--
  MobilityDB — MobilityAPI, a conformant OGC API – Moving Features: assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityAPI, a conformant OGC API – Moving Features: the assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

Read at MobilityAPI `7986fbecdc`, its `master`.

## The conformance classes declared

The conformance declaration (`GET /conformance`, `samples/conformance-declaration.json`) lists:

| Standard | Classes |
|---|---|
| OGC API – Moving Features, Part 1: Core, 1.0 (OGC 22-003r3) | `common`, `mf-collection`, `movingfeatures` |
| OGC API – Moving Features, Part 4 | `cquery` |
| OGC API – Common, Parts 1 and 2 | `core`, `collections` |
| OGC API – Coverages, Part 1 (OGC 19-087, 1.0.0rc4) | `core`, `subsetting-spatial`, `subsetting-temporal`, `field-selection`, `geotiff`, `coveragejson` |
| OGC API – EDR, Part 1, 1.1 (OGC 19-086r6) | `core` |

A collection holds moving features of one of the 14 spatial temporal types the catalog states an
MF-JSON form for (`geometryTypes`, from `temporalTypes` in `catalog_gen.go`): `tgeompoint`,
`tgeogpoint`, `tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`,
`trgeometry`, `tpcpoint`, `tpcpatch`, `th3index`, `tquadbin` and `ts2cell`.

## The abstract tests of Part 1

`ats_part1_test.go` holds Annex A as executable tests, one row per abstract test keyed by the
identifier the standard gives it, so that an abstract test with no row is a reported gap. A row
is discharged in one of four ways:

| Discharge | What it checks | Rows |
|---|---|---|
| route | the method and URL template the test issues are in the routing table the service runs | 21 |
| live | the answer of the service over the fixture database | 17 |
| response | the shape of a response document | 4 |
| document | the service description or the conformance declaration | 3 |

A route row is a sound discharge for a test that states only that a resource can be reached,
and most route rows have a live row beside them for the response (`features-get` and
`features-get-success`). Three route rows have none, and their tests state a response:

| Abstract test | What it states | MobilityAPI |
|---|---|---|
| `/conf/movingfeatures/tpgeometry-query-distance` | a Distance query returns a TReal time-to-distance curve | answers `cumulativeLength` over the temporal geometry, tested live in `ats_tpgeometry_query_test.go` but not as this row |
| `/conf/movingfeatures/tpgeometry-query-velocity` | a Velocity query returns a TReal time-to-velocity curve | answers `speed`, tested live there in the same way |
| `/conf/movingfeatures/tpgeometry-query-acceleration` | an Acceleration query returns a TReal time-to-acceleration curve | answers 501 |

So the suite reports the three as met where it checks only that their route exists: distance and
velocity are met by answers that no row of the suite reads, and acceleration is not met.

## The issues of the standard

The wiki page [Implementing OGC API — Moving Features in
MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB)
states nine issues of Part 1, each reproduced from the published artefacts: three in the abstract
tests (A.20 demands two incompatible things; A.25 to A.27 send the three queries to `/distance`;
A.27 asks for an acceleration that the standard's Linear interpolation does not carry), five in
the normative schemas and one in the published examples, ten of whose sixteen documents fail the
schema they are attached to. It closes with seven candidate resolutions, the text a working group
can adopt. OGC publishes no executable test suite for Moving Features, so Annex A driven directly
is the evidence of a claim.

No issue of `opengeospatial/ogcapi-movingfeatures` carries those resolutions; its only issue on
the abstract tests, #61, "Validate ATS of OGC API – MF Part 1: Core", is closed with no body and
no comment. The stance publishes a resolution so that the standards body can adopt it, and a
resolution only on the wiki is not before that body.

## Part 4

The class `cquery` is declared, and `stream_conformance_test.go` tests that the declaration
lists it; no row reads an abstract test of Part 4, so whether its requirements are met is not
measured.

## The other standards declared

The conformance page's ledger lists OGC API – Moving Features Part 1 alone. MobilityAPI also
declares `cquery` of Part 4, two classes of OGC API – Common, six of Coverages Part 1 and the core
of EDR 1.1, and none has a wiki page stating its conformance and the points it leaves open. One
such point is open: EDR 1.1 names CoverageJSON `application/prs.coverage+json`, while the class
`coveragejson` of Coverages Part 1 names it `application/vnd.cov+json`, and MobilityAPI answers
both standards with the latter.

## The extensions

Beyond the declared classes MobilityAPI serves `POST /collections/{cid}/bulk`, `GET
/collections/{cid}/export`, `/timeseries` and `/trajectories`, `PUT` of a collection and of a
moving feature, the stream delivery of a continuous query (`.../queries/{qid}/stream`) and the
raster query of a temporal geometry (`.../tgsequence/{tgid}/raster`), and `/health`. Rule 3 of the
index asks that each be named as an extension in the API definition and change no answer the
standard defines; no test states either for every extension.

## The surface of a server

`tools/raster_matrix.py` and `tools/io_matrix.py` credit a binding with a function when its
object layer names the function and declares the types it takes. MobilityAPI holds no object
layer: it calls the SQL functions inside its handlers, and its generated tables (`temporalTypes`,
`rasterTypes`, `mathOps`) record what MobilityDB declares over each type. Both matrices
read nothing in it, so the raster plan measures MobilityAPI by its endpoints and the input and
output plan reports it as having no generated surface. `trip_functions_test.go` already lists
every SQL function the handlers apply to a moving feature's temporal geometry, checked against
the generated `featureFunctions`: the start of the surface MobilityAPI states itself.
