<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — MobilityAPI and the OGC API Standards
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# MobilityAPI and the OGC API Standards — Conformance Specification

| | |
|---|---|
| Version | v1.6.0 |
| Scope | The OGC API conformance classes MobilityAPI declares, what it answers and refuses under each, the points the standards leave open and the resolution it applies, and the routes it serves beyond them |
| Reference | [MobilityAPI](https://github.com/MobilityDB/MobilityAPI) at commit [`d41015d`](https://github.com/MobilityDB/MobilityAPI/tree/d41015d): `main.go` (`routeTable`, `conformsTo`, `landing`, `featureCollectionDescription`), `coverage.go`, `edr.go`, `edr_features.go`, `stream_engine_meos.go`, `CONFORMANCE.md`, `api/surface.json` |

## 1. Scope and intent

MobilityAPI is the web service of MobilityDB: it publishes MobilityDB
collections through the OGC API standards, and every value it answers is
computed in the database by a function MobilityDB declares. This document states
which conformance classes it declares, what a client receives under each, what
it refuses and with which status, and how it resolves each point where a
standard is ambiguous, silent or contradicts itself. It is descriptive of the
service and prescriptive for a client: a client relying on what is stated here
interoperates with MobilityAPI.

A point a standard leaves open is resolved by a standing rule. A normative
requirement outranks descriptive prose and the documents generated beside it;
where a standard is silent, the answer of OGC Moving Features to the same
question is taken; where neither speaks, the answer best for what the data
means. Each resolution below is one MobilityAPI applies and a test of its suite
measures.

## 2. Notation

A *class* is a conformance class, named by its identifier, the URI its
standard publishes. An *operation* is a method and a path, written as the
standard writes it (`GET /collections/{collectionId}/items`). An *extension* is
an operation no declared class requires. *The suite* is MobilityAPI's test
suite, run in continuous integration against MobilityDB built from master.

## 3. The declared classes

`GET /conformance` lists the classes below. A class is listed as its standard
publishes it; a class whose standard publishes its identifier under `https://`
is also listed under `http://` (section 3.1).

| standard | class | identifier |
|---|---|---|
| OGC API – Moving Features – Part 1: Core (OGC 22-003r3) | common, mf-collection, movingfeatures | `http://www.opengis.net/spec/ogcapi-movingfeatures-1/1.0/conf/<class>` |
| OGC API – Moving Features – Part 4: Stream Extension (draft) | cquery | `http://www.opengis.net/spec/ogcapi-movingfeatures-4/1.0/conf/cquery` |
| OGC API – Common – Part 1: Core (OGC 19-072) | core, landing-page | `http://www.opengis.net/spec/ogcapi-common-1/1.0/conf/<class>` |
| OGC API – Common – Part 2: Geospatial Data (OGC 20-024) | collections | `https://` and `http://www.opengis.net/spec/ogcapi-common-2/1.0/conf/collections` |
| OGC API – Coverages – Part 1: Core (OGC 19-087, 1.0.0rc4 draft) | core, subsetting-spatial, subsetting-temporal, field-selection, geotiff, coveragejson, netcdf | `https://` and `http://www.opengis.net/spec/ogcapi-coverages-1/1.0/conf/<class>` |
| OGC API – Environmental Data Retrieval 1.1 (OGC 19-086r6) | core, collections, queries, covjson | `http://www.opengis.net/spec/ogcapi-edr-1/1.1/conf/<class>` |

The service serves 55 operations. The API definition at `GET /api` marks each
with the class it is a requirement of (`x-conformance-class`), 43 of them, or
as an extension (`x-mobilityapi-extension`), 12 of them (section 9). The suite
fails if a route is served and not described, described and not served, or
marked with a class the conformance declaration does not list.

### 3.1 Class identifiers under two schemes

OGC's naming policy (OGC 10-103r1) roots every specification element at
`http://www.opengis.net/spec`. Common Part 2 and the Coverages draft publish
their identifiers under `https://` in their text, while their OpenAPI documents
and OGC's executable Coverages suite (`ets-ogcapi-coverages10`, which compares
identifiers as strings) write `http://`; Moving Features Part 1 inherits
Common Part 2's class as `http://`. A client comparing strings finds such a
class under either spelling. A class whose standard publishes `http://` alone
is listed under that one.

## 4. Moving Features Part 1

### 4.1 What backs the claim

OGC publishes no executable test suite for Moving Features. The suite keys a
test to each of the 45 abstract tests of Annex A, and fails when an identifier
is claimed by none. Every document the service writes is validated against
OGC's own OpenAPI document, vendored byte for byte, with `format` asserted.

### 4.2 The documents written

| member | written as |
|---|---|
| a collection's spatial extent | a bbox in CRS84 (`http://www.opengis.net/def/crs/OGC/1.3/CRS84`), whatever the stored CRS; a collection whose SRID is unknown states no spatial extent |
| a TemporalGeometrySequence | `type: "TemporalGeometrySequence"` and `geometrySequence`, as `temporalGeometrySequence.yaml` requires |
| the values of a temporal primitive value | `values` an array, one value per instant |
| a unit of measure | the OGC register URI of its UCUM code, `http://www.opengis.net/def/uom/UCUM/0/<code>` |
| an instant | RFC 3339, section 5.6: a `T` separator and an offset of hours and minutes |
| an interpolation | `Discrete`, `Step` or `Linear` |

On input, a unit is accepted when absent, an absolute URI or three characters,
what the schema admits for `form`. An interpolation the standard names that
MobilityDB does not carry (`Quadratic`, `Cubic`, `Regression`) is refused with
501; a token the standard does not name, with 400.

### 4.3 The geometry queries

`GET .../tgsequence/{tGeometryId}/{queryType}` answers `distance`
(`cumulativeLength`), `velocity` (`speed`) and `acceleration`, each at its own
resource. MobilityDB interpolates a position as Discrete, Step or Linear, and
under none of them is acceleration a curve: under Linear the speed is
piecewise constant, so its derivative is zero within each segment and
undefined at each vertex. `acceleration` therefore answers 501 with the reason
for the geometry's interpolation, and 404 for a geometry that does not exist.

### 4.4 Points Part 1 leaves open

| point | resolution applied |
|---|---|
| A.20 asks for `MovingGeometryCollection` and `prism`; the schema its Table A.7 cites requires `TemporalGeometrySequence` and `geometrySequence` | the schema's members |
| A.26 and A.27 request `/distance` | velocity and acceleration at their own resources |
| A.27 asks for an acceleration under Linear motion | 501 with the reason (section 4.3) |
| six `oneOf` constructs whose URI branch overlaps the named values when `format` is an annotation | only named values or URIs written; documents validated with `format` asserted |
| `temporalPrimitiveValue.values` typed as one scalar | one value per instant |
| `form` admits any three characters | units written as register URIs, which only the URI branch matches |
| interpolation spelled with two vocabularies | both read through one mapping (section 4.2) |
| the instants of a temporal geometry unconstrained | RFC 3339 instants |
| 10 of the 16 published examples fail the schema they are attached to, 13 under JSON Schema's default reading of `format` | the count measured in the suite |

The candidate resolutions of these points, as text a working group can adopt,
are on the wiki page [Implementing OGC API — Moving Features in
MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB).
Applied to the schema, every document MobilityAPI writes is valid under both
readings of `format`, and 5 of the 16 published examples fail, the five whose
example is wrong.

## 5. Moving Features Part 4

The Continuous Query class is implemented after the draft MobilityAPI reads,
keyed by its requirement numbers.

| requirement | answer |
|---|---|
| 1, `cquery_link` | the `cquery` link carries `rel`, `queryId`, `href`, `channel`, `status`, `type` |
| 2, `window` | a result carries `windowStart` and `windowEnd`, the bounds of its window: `[start, end)` for a time window, `[start, end]` for a count window |
| 3, `window_tumbling` | fixed-duration windows that do not overlap |
| 4, `window_hopping` | fixed-duration windows that overlap |
| 5, `window_count` | windows over a count of records |

The aggregations of a temporal property are `COUNT`, `SUM`, `AVG`, `MIN`,
`MAX` over `TReal` and `TInt`; `COUNT`, `COUNT_DISTINCT` over `TText`; `COUNT`,
`ANY`, `ALL`, `COUNT_TRUE`, `COUNT_FALSE` over `TBoolean`, each computed by
MEOS. Results are delivered over Server-Sent Events. A query's status is
`running`, `stopped` or `failed`; a query enters `running` when it is
submitted.

Beyond the requirements, a continuous query over a temporal float may apply a
pointwise lifted operation (`ln`, `exp`, a product, a sum and the rest of the
pointwise operations the catalog states) instead of a window aggregate. A
query submitted with `"live": true` reads records pushed to it: over HTTP
(`.../tproperties/{tPropertyName}/ingest`), over MQTT when `MFAPI_MQTT_BROKER`
names a broker (topic `mfapi/<collectionId>/<mFeatureId>/<tPropertyName>`), or
from Kafka. Queries run on MEOS in the service by default, or as Flink or
Kafka Streams jobs when `MFAPI_STREAM_ENGINE` is `flink` or `kafka`.

## 6. Common

`GET /`, `GET /api` and `GET /conformance` are the operations of `landing-page`.
The landing page links the conformance declaration under both relation types
the standards require: `conformance` (Features Part 1, `/req/core/root-success`)
and `http://www.opengis.net/def/rel/ogc/1.0/conformance` (Common Part 1,
`/req/landing-page/root-success`). The API definition is an OpenAPI 3.0
document, validated against the OpenAPI 3.0 schema, every operation stating
its responses. An unknown query parameter is ignored, as Common Part 1's
`/per/core/query-param-name-tolerance` permits.

## 7. Coverages

A raster collection is registered over an existing table holding a period
column (`tstzspan`) and a raster column; the service only reads it. The rows of
one period are the tiles of one raster.

| request | answer |
|---|---|
| no `datetime` | the latest period, named by `Content-Datetime` |
| a `datetime` selecting one period | that period |
| a `datetime` meeting several periods | 400, naming them: GeoTIFF has no time axis and a CoverageJSON coverage is written from one raster |
| `bbox`, `subset` | the coverage clipped, its bounds stated by `Content-Bbox` |
| `properties` | the bands named |
| `Accept` | `image/tiff; application=geotiff` (also when no preference is stated), or CoverageJSON as `application/vnd.cov+json` or `application/prs.coverage+json`, in the spelling named, or netCDF as `application/x-netcdf`; 406 when none is admitted |

The tiles are joined by `rasterUnion` and written by `asTIFF`,
`asCoverageJSON` or `asGDALRaster(c, 'netCDF')`. A request needing a function
MobilityDB does not state over the collection's raster type is refused naming
the function, and a netCDF coverage from a database whose GDAL has no netCDF
driver is refused with MobilityDB's error naming the format.

## 8. Environmental Data Retrieval

Over a raster collection the service answers six queries, each by `GET`, in
CoverageJSON:

| query | geometry read | domain written |
|---|---|---|
| `position` | `POINT` | `Point`, or `PointSeries` over a span |
| `radius` | `POINT`, `within` and `within-units` in `m` or `km` | `Grid`, the raster of one period clipped to the circle |
| `area` | `POLYGON` | `Grid` |
| `cube` | `bbox` in CRS84 and `z` | `Grid`; `z` is required and read, and not applied, as a raster has no vertical axis |
| `trajectory` | `LINESTRINGM`, M the seconds since the Unix epoch | `Trajectory` |
| `corridor` | `LINESTRING` and a width in `m` or `km` | `Grid`; `corridor-height` is read and not applied, as a raster has no vertical axis |

The circle of `radius` and the buffer of `corridor` are taken on the sphere. A
clipped coverage keeps the pixels whose centre the region covers, on a grid the
extent of the region rounds outward to whole pixels, as PostGIS's `ST_Clip`
answers. A collection's description carries `data_queries`, one link per query
with its variables, and `distanceunits`, as `/req/edr/rc-collection-info` I
requires of a description linking a radius. The format is selected by `f`, and
the spelling of CoverageJSON by `Accept`.

Each period of a raster collection is an instance:
`/collections/{collectionId}/instances` lists them in time order and
`/collections/{collectionId}/instances/{instanceId}` describes one, its id the
start of the period in RFC 3339. An instance is described as a collection is,
with the extent of its period alone and its data links under its own path, and
`/instances/{instanceId}` writes the description `/instances` lists for it. The
six queries are answered under an instance from the rasters of its period
alone. `/req/instances/src-md-success` B holds one instance to `instances.yaml`,
the schema of the list, and C holds its `extent` to the collection's; the
service writes one instance in the form of `collection.yaml` and with the
extent of its period, the reading that lets instances differ.

The description of a raster collection carries what
`/req/edr/rc-collection-info` and the schema `collection.yaml` of EDR 1.1 ask of
it, and `/collections` lists it with the description `/collections/{collectionId}`
writes, as `/req/collections/src-md-success` C requires:

| member | content |
|---|---|
| `links` | every link with its `rel` and `type`: the description, the coverage in each encoding, the schema, and one `data` link per query |
| `extent` | the bounds of the rasters in CRS84 and the span of the periods |
| `output_formats` | `CoverageJSON` |
| `parameter_names` | one `Parameter` per band, keyed `band1`, `band2`, ... as `parameter-name` reads them, its `data-type` `integer` or `float` by the band's pixel type, its `observedProperty` labelled as `asCoverageJSON` labels the band |
| `keywords` | `coverage` and the raster type of the collection |

An `observedProperty` carries a `label` and no `id`: `/req/edr/rc-parameters` F
requires the two, the schema `observedPropertyCollection.yaml` requires the
`label` alone, and the schema is followed.

Over a collection of moving features the service answers four queries, each by
`GET`, with the moving features the query's geometry meets, computed by
MobilityDB:

| query | geometry read | a feature is answered when |
|---|---|---|
| `area` | `POLYGON` | its temporal geometry ever intersects the polygon (`eIntersects`) |
| `cube` | `bbox` in CRS84 and `z` | it ever intersects the rectangle of the bbox; `z` is required and read, and not applied, as the features are tested in two dimensions |
| `trajectory` | `LINESTRING`, without an M ordinate | it ever intersects the line |
| `radius` | `POINT`, `within` and `within-units` in `m` or `km` | it ever comes within the distance of the point, on the spheroid (`eDwithin` over the moving point read as a `tgeogpoint`) |

The geometry is read in CRS84 and carried into the collection's system, or read
as a geography over a collection of geographies (`tgeogpoint`, `tgeography`),
which MobilityDB does not relate to a planar geometry; the rectangle of a `cube`
over such a collection is then a polygon whose edges are geodesics. A
`datetime`, an instant or an interval, restricts each feature to it before the
test; an open end of an interval is the bound of the collection's time extent on
that side. The answer is a FeatureCollection of whole features, paged as
`/items` pages it, in MF-JSON (`application/json`), or, with `f=GeoJSON`, in
GeoJSON (`application/geo+json`), each feature the path it traverses in WGS 84.
`radius` is answered over a collection of moving points, `tgeompoint` or
`tgeogpoint`, the types a distance in metres on the spheroid is defined for;
`parameter-name` is refused, as the collection states no parameter.

The description of a collection of moving features carries, beside what Moving
Features states of it, a `type` on every link, one `data` link per query it
answers, `data_queries`, `output_formats` (`MF-JSON`, `GeoJSON`), `keywords`,
`distanceunits`, and `parameter_names`, empty: its answers are features, whose
temporal properties are resources of their own. `/collections` lists it with
the description `/collections/{collectionId}` writes.

The functions over a type are read from the MEOS catalog, its compositions
among them: `eIntersects(tnpoint, geometry)` is the relationship of the temporal
point a `tnpoint` casts to, and a collection of `tnpoint`, `tpcpatch`,
`tpcpoint`, `tpose` or `tquadbin` answers `area`, `cube` and `trajectory` by
it. The box test on the index applies where the type's box is an `stbox`; a
point cloud's `tpcbox` is tested exactly alone.

The classes `collections` and `queries` of EDR 1.1 are declared. A collection
of `th3index` or `ts2cell` links no query, and so does not meet
`/req/core/rc-md-query-links` A: MobilityDB's SQL declares `eIntersects` over
each, against a geography, while the catalog carries no signature or
composition of it.

CoverageJSON is written in the spelling the client's `Accept` names:
`application/prs.coverage+json`, the spelling EDR 1.1 as published writes
(`/req/covjson/content`), or `application/vnd.cov+json`, the type OGC
CoverageJSON (OGC 21-069r2) states and the EDR working group adopts for the
editions after 1.1, which also answers a client naming neither. The two are the
same document. The class `covjson` of EDR 1.1 is declared.

## 9. Extensions

| operation | what it answers |
|---|---|
| `GET /health` | that the service is up |
| `GET /collections/{collectionId}/trajectories` | the trajectory of each feature |
| `GET /collections/{collectionId}/timeseries` | the temporal properties as time series |
| `PUT /collections/{collectionId}/items/{mFeatureId}` | a moving feature replaced |
| `GET /collections/{collectionId}/export` | the collection as NDJSON, or GeoParquet with a bbox and time sidecar |
| `POST /collections/{collectionId}/bulk` | a batch of observations appended |
| `POST .../tproperties/{tPropertyName}/ingest` | records pushed to a live continuous query |
| `POST`, `GET`, `DELETE .../tgsequence/queries[/{queryId}]`, `GET .../queries/{queryId}/stream` | continuous queries over a temporal geometry, streamed as Server-Sent Events |

An extension changes no answer a declared class defines.

## 10. The functions each operation applies

`api/surface.json` lists, per operation, its class or extension and the
MobilityDB functions its handler applies, read from the service's source. The
suite holds the file to the code, and holds every function to a type the
operation serves: a Coverages or EDR operation the raster types, a
collection-level operation and the EDR queries `area`, `cube`, `trajectory` and
`radius` both, every other operation the temporal types, each
also the box and span types.

## 11. Versioning and stability

The version is raised at its minor number when the service declares a class,
answers an operation or applies a resolution this document does not state, and
at its major number when an answer stated here changes.

## 12. Out of scope

- The operations of Part 4 over a temporal geometry (`LENGTH`, `AREA`,
  `TO_TRAJ` and the rest of its Table 7), and its Kafka and WebSocket channels.
- A newer Part 4 draft, describing a Continuous Query resource with `id`,
  `pubtime`, `target`, `filter` and `queryResult`.
- The LAS and LAZ encodings of the Coverages draft, which encode point clouds,
  not the coverage of a raster collection.

## 13. References

- OGC API – Moving Features – Part 1: Core, OGC 22-003r3: <https://docs.ogc.org/is/22-003r3/22-003r3.html>
- OGC API – Common – Part 1, OGC 19-072: <https://docs.ogc.org/is/19-072/19-072.html>; Part 2, OGC 20-024: <https://docs.ogc.org/is/20-024/20-024.html>
- OGC API – Coverages – Part 1 draft, OGC 19-087: <https://docs.ogc.org/DRAFTS/19-087.html>
- OGC API – EDR 1.1, OGC 19-086r6: <https://docs.ogc.org/is/19-086r6/19-086r6.html>
- OGC CoverageJSON, OGC 21-069r2: <https://docs.ogc.org/cs/21-069r2/21-069r2.html>
- OGC naming policy, OGC 10-103r1: <https://docs.ogc.org/pol/10-103r1.html>
- [Standards conformance in MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Standards-Conformance-in-MobilityDB): the stance and the ledger, with a page per standard
- Sister specifications: [Rasters](meos-rasters.md), [Input and output](meos-io.md)

## 14. Consumers

Clients of OGC API – Moving Features, Common, Coverages and EDR: generic OGC
API clients reading the conformance declaration and the API definition, map
clients reading coverages and CoverageJSON, and stream consumers of continuous
queries over Server-Sent Events.

## 15. References to the implementation

The authoritative statement is the service's source and its suite at the
commit named in the header: the conformance declaration (`conformsTo`), the
route table (`routeTable`), the Part 4 requirements (`CONFORMANCE.md`) and the
surface (`api/surface.json`). A correction to this document is a change to
MobilityAPI's behaviour or to the suite that measures it, stated here with its
version raised.
