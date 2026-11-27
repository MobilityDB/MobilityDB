<!--
  MobilityDB — a proposal for the Stream part of OGC API – Moving Features
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Continuous queries over moving features: a proposal for the Stream part of OGC API – Moving Features

**Status.** A proposal, for the Moving Features Standards Working Group to begin
drafting the Stream part from. OGC holds no draft of a Stream part. Each
requirement below is stated with its abstract test, how a working
implementation answers it, and the test of that implementation's suite that
measures it. The implementation is MobilityAPI at commit
[`0f66c71`](https://github.com/MobilityDB/MobilityAPI/tree/0f66c71), over
MobilityDB, whose library MEOS computes every value a query answers. Its user
document is [MobilityAPI and the OGC API
Standards](https://github.com/MobilityDB/MobilityDB/blob/master/doc/specs/mobilityapi-ogc-api.md)
v2.0.0, section 5.

The proposal builds on the document titled "OGC API – Moving Features – Part 4:
Stream Extension" (labelled YY-999), whose numbering of requirements 1 to 5 it
keeps; requirements 6 to 12 state what that document leaves to the
implementation.

## 1. Scope

A continuous query is registered once on a moving feature and runs until it is
stopped, emitting results as the feature's data arrives or is replayed. It reads
either a temporal property of the feature or its temporal geometry, and either
transforms each record or aggregates the records of each window. Its results are
delivered on one or more channels.

## 2. Resources

| path | method | answer |
|---|---|---|
| `/collections/{collectionId}/items/{mFeatureId}/tproperties/{tPropertyName}/queries` | `POST` | registers a query on a temporal property: 201 and its `cquery` link |
| same | `GET` | the queries of the property |
| `/collections/{collectionId}/items/{mFeatureId}/tgsequence/queries` | `POST`, `GET` | the same over the feature's temporal geometry |
| `.../queries/{queryId}` | `GET` | the query's `cquery` link and status |
| `.../queries/{queryId}` | `DELETE` | stops the query: 204 |
| `.../queries/{queryId}/stream` | `GET` | its results as Server-Sent Events |
| `.../queries/{queryId}/ws` | `GET` | its results over a WebSocket |

The body of a `POST` carries an `aggregation` and a `window`, or, over a
temporal float, a pointwise `operation`; a `region` for `INTERSECT`; a `topic`
for the Kafka channel; and `intervalMs`, the pacing of a replayed stream.

## 3. Requirements

Each requirement is followed by its abstract test, then how MobilityAPI answers
it and the test that measures it.

### Requirement 1 — `cquery_link`

A registered continuous query SHALL be answered by a `cquery` link object
carrying `rel` (`cquery`), `queryId`, `href` (the result stream), `channel`,
`status` and `type`.

*Abstract test.* Register a query; check that each member is present, that
`rel` is `cquery` and that `href` resolves to the query's results.

*MobilityAPI.* `cqueryLink` (`stream.go`) writes the six members, `type`
`text/event-stream`, a `topic` member naming the Kafka topic of a query that
gives one, and a `links` array whose `alternate` link names the WebSocket
channel. Measured by `TestCqueryLinkShape` and, for a geometry query,
`TestCqueryLinkGeometryScope` (`stream_conformance_test.go`).

### Requirement 2 — `window`

A windowed query SHALL carry a `window`, and each result SHALL carry the bounds
of its window: `windowStart` and `windowEnd`, half-open `[start, end)` for a
time window and closed `[start, end]` for a count window, whose bounds are the
times of its first and last records.

*Abstract test.* Register a TUMBLING and a COUNT query; check the bounds of each
result against the records of the window.

*MobilityAPI.* `parseWindow` (`stream.go`) reads the window; `emitWindow`
(`stream_engine_meos.go`) writes the bounds. Measured by `TestMeosWindowBounds`
(`stream_meos_test.go`).

### Requirement 3 — `window_tumbling`

A TUMBLING window SHALL group the records of fixed, non-overlapping spans of the
given `size` and `unit`, aligned on the multiples of the span.

*Abstract test.* Feed records across three spans; check one result per span
holding exactly the records of that span.

*MobilityAPI.* `runAggregate` (`stream_engine_meos.go`). Measured by
`TestMeosTumblingWindow`.

### Requirement 4 — `window_hopping`

A HOPPING window SHALL group the records of fixed spans that advance by a
shorter `hop`, so that consecutive windows overlap.

*Abstract test.* Feed records with a span three times the hop; check that each
record falls in three consecutive windows.

*MobilityAPI.* `runHopping` (`stream_engine_meos.go`). Measured by
`TestMeosHoppingWindow`.

### Requirement 5 — `window_count`

A COUNT window SHALL group a fixed number of consecutive records.

*Abstract test.* Feed 2n records with size n; check two results of n records.

*MobilityAPI.* `runAggregate` COUNT (`stream_engine_meos.go`). Measured by
`TestMeosWindowAggregate`.

### Requirement 6 — aggregations of a temporal property

A windowed query on a temporal property SHALL answer the aggregations its type
admits: `COUNT`, `SUM`, `AVG`, `MIN`, `MAX` over `TReal` and `TInteger`;
`COUNT`, `COUNT_DISTINCT` over `TText`; `COUNT`, `ANY`, `ALL`, `COUNT_TRUE`,
`COUNT_FALSE` over `TBoolean`. An aggregation its type does not admit SHALL be
refused with 400, naming those it admits.

*Abstract test.* For each type, register each admitted aggregation and check
its value over a known window; register one it does not admit and check 400.

*MobilityAPI.* `aggByType` (`stream.go`); `numAggregate`, `textAggregate`,
`boolAggregate` (`stream_engine_meos.go`), each value computed by MEOS. Measured
by `TestMeosWindowAggregate` and `TestMeosTextBoolAggregate`.

### Requirement 7 — aggregations of a temporal geometry

A windowed query on a temporal geometry SHALL answer, per window of the
feature's temporal geometry restricted to the window:

| aggregation | type of the feature | answer |
|---|---|---|
| `COUNT` | every type | the instants of the window |
| `LENGTH` | a moving point | the distance travelled; on the spheroid in metres for a point in longitude and latitude |
| `LENGTH` | lines or polygons | the length the feature covers during the window, each overlap counted once |
| `AREA` | lines or polygons | the area the feature covers during the window, each overlap counted once |
| `TO_TRAJ` | a moving point | the path of the window, in GeoJSON |
| `INTERSECT` | a moving point, lines or polygons | whether the window ever meets the query's `region` |

"Covers" is the union of the values the feature takes, for a geometry held from
instant to instant, or the region its continuous motion sweeps, for a rigid
geometry in motion. A time window restricts the geometry at its bounds, a moving
point interpolated there.

*Abstract test.* Over a feature with known positions, check `LENGTH` over a
COUNT window and over a TUMBLING window whose bound falls inside a segment;
`INTERSECT` with a region met in one window only; over two overlapping squares,
`AREA` counting their overlap once; over two overlapping lines, `LENGTH` counting
it once.

*MobilityAPI.* `geometryAggregations` (`stream_geometry.go`) and `runGeometry`
(`stream_geometry_meos.go`), through MEOS's `tpoint_length`, `tpoint_trajectory`,
`traversed_area` with its union, `geo_length`, `geo_area` and `eintersects`.
Measured by `TestLiveGeometryWindowAggregates` (a 30-minute window cut a quarter
along a segment: 3149.867 m over 3 instants, then 1352.082 m over 1) and
`TestLiveGeometryWindowShapes` (`AREA` 7 of two overlapping 2 × 2 squares,
`LENGTH` 3 of lines 0–2 and 1–3).

### Requirement 8 — what is not defined is refused

A query SHALL be refused with 400, naming the reason, when its aggregation is not
defined for the type, its window is not one of COUNT, TUMBLING and HOPPING or has
no positive size, a HOPPING window has no positive hop, or an `INTERSECT` names no
region or one that is no geometry.

*Abstract test.* Submit each case and check 400 with a reason.

*MobilityAPI.* `postQuery`, `parseWindow`, `postGeometryAggregate`. Measured by
`TestLiveGeometryWindowShapes`, whose refusals include `AREA` over a point,
`VOLUME`, `INTERSECT` without a region and with a region that is no WKT, and an
unknown window.

### Requirement 9 — lifecycle

A query SHALL be `running` once registered and until it is stopped; `DELETE`
SHALL stop it, after which its status is `stopped` and every channel ends; a
query whose computation fails SHALL be `failed`. A client subscribing to a
stopped query SHALL be answered that it has ended.

*Abstract test.* Register, read the status, read a result, delete, check the
status and the end of the stream; subscribe after the stop and check the end.

*MobilityAPI.* `streamRegistry`, `contQuery.stop` and `status` (`stream.go`).
Measured by `TestRegistrySubmitBroadcastStop` and `TestSubscribeAfterStop`
(`stream_test.go`). The cycle over HTTP is read for a geometry query by
`TestLiveGeometryWindowAggregates`, and for a property query by
`TestLivePropertyQueryHTTPCycleAndEvents` (`stream_http_meos_test.go`): registered with 201 and a running `cquery` link, listed, read,
stopped with 204, then 404 and the end of its stream.

### Requirement 10 — the Server-Sent Events channel

The results of a query SHALL be delivered as Server-Sent Events at the `href` of
its `cquery` link, one event per result, its data the result as a JSON object.

*Abstract test.* Read the stream of a query fed known records; check one event
per result.

*MobilityAPI.* `streamQuery` (`stream.go`). Measured by
`TestLivePropertyQueryHTTPCycleAndEvents`, which reads the doubled speeds of a
feature, 22.5, 25, 27.5 and 28, as events whose data is each a JSON result, and
`TestRegistrySubmitBroadcastStop`, which measures the fan-out that feeds the
channel.

### Requirement 11 — further channels

An implementation MAY deliver the results on further channels, each carrying the
same JSON object per result: over a WebSocket at `.../queries/{queryId}/ws`, one
text message per result, closing normally when the query stops; to a Kafka topic
the query names, one record per result keyed by the query's id.

*Abstract test.* For each channel offered, read two results and check their
order and content, then stop the query and check the channel's end.

*MobilityAPI.* `websocketQuery` (`stream_websocket.go`) and `publishToKafka`
(`stream_kafka.go`). Measured by
`TestWebSocketDeliversResultsAndClosesWithTheQuery` and
`TestKafkaTopicReceivesEveryResult`; `TestKafkaTopicRefusals` refuses a topic
name Kafka does not accept and a topic without brokers.

### Requirement 12 — the processing engine is not part of the interface

The answers SHALL NOT depend on where the query runs. MobilityAPI runs a query in
process on MEOS by default, or as a Flink or Kafka Streams job calling MEOS
through JMEOS; a transform answers the same on each. An engine that cannot run a
query SHALL refuse it rather than answer otherwise.

*Abstract test.* Run one transform on two engines and compare their results.

*MobilityAPI.* `engineFor` (`stream.go`); the bridge engines refuse windowed
aggregation. Measured by `TestEngineForRouting` and `TestMeosTransform`.

## 4. Points for the drafting group

Each is a decision the proposal leaves open, with the reading MobilityAPI
applies.

- **Lifecycle.** The YY-999 document names a `registered` state before
  `running`. A query that starts on registration never shows it; MobilityAPI
  answers `running` at once. Either the state is dropped, or it is defined as the
  state of a query whose source has not delivered.
- **`VOLUME` of a moving polygon.** No function of MEOS answers it, and the
  proposal does not define it. For a polygon held from instant to instant, the
  space-time volume is the sum of each value's area times the time it is held,
  which is exact; for a polygon in continuous motion it needs a definition.
  MobilityAPI refuses it.
- **`AREA` and `LENGTH` of a window of shapes.** The proposal lists them for
  moving lines and polygons without defining them. MobilityAPI answers what the
  feature covers during the window, each overlap counted once, because a line
  has no area at any instant, so `AREA` of a line has meaning only as covered.
- **The operand of `INTERSECT`.** The proposal does not say what a window
  intersects. MobilityAPI reads a `region`, WKT in CRS84, and answers a boolean
  per window.
- **Geography.** A collection in longitude and latitude is measured on the
  spheroid where an exact computation exists: the distance travelled by a point.
  The union of lines or polygons in longitude and latitude has none, so
  MobilityAPI refuses `LENGTH` and `AREA` there.
- **Units of a window.** The `unit` of a window is `SECONDS`, `MINUTES` or
  `HOURS`; a high-frequency source cannot state a sub-second window.
- **One aggregation per query.** A query carries one `aggregation` and one
  `window`; the text should say so.
- **Editorial.** `INTERESCT` is misspelled in Tables 6 and 7; Requirement 5 B
  reads "hopping window" where the count window is meant; Figure 1 reads
  "Terminate registeredquery".
