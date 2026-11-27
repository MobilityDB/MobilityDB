<!--
  MobilityDB — Visualization: implementation plan
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every viewer over every engine: the implementation plan

[Back to the index](00-INDEX.md) · [Assessment](ASSESSMENT.md)

## The steps, one pull request each

| Step | Repository | Change | State |
|---|---|---|---|
| VS1 | MobilityDeck | The stream example: the front end of MobilityFlink-Deck becomes `stream/`, its history kept, its data path and its interpolation in JavaScript giving way to those of MobilityAPI's map (`tutorial-stream/map`), which moves into MobilityDeck with it: MEOS.js computes each position with `valueAtTimestamp` on one clock, reading MobilityAPI's stream (Part 4) through its Flink bridge, and the replay, follow and statistics of MobilityFlink-Deck stay. The README of MobilityAPI's tutorial points to it; MobilityFlink-Deck is then archived, its README pointing to MobilityDeck (the owner, 2026-11-22) | open; its fleet reads V9 |
| VS2 | MobilityDeck | The examples on the stack of MobilityAPI's map (deck.gl 9, Vite, MapLibre): `files/` reads its trips from a file through `meos.js/deckgl`, in place of `geojsonvt/`; `mvt/` reads the tiles of `asMVTGeom`, slicing the times of a MultiLineString at the running offset and leaving a Point out of `TripsLayer`; the types MOVE draws (`tgeompoint`, `tgeogpoint`, `tcbuffer`, `tnpoint`) drawn too, the pages of MEOS.js-examples linked | branch `feat/examples-on-meosjs`, `files/` committed: the 154 BerlinMOD trips of MobilityDB, transformed once to SRID 4326, read by `npm test` with their 216075 instants on the MEOS.js 2.0.0 candidate; `mvt/` needs a MobilityDB with `pg_tileserv` |
| VS3 | MobilityAPI, MobilityDeck | The same display from every engine: one query through each backend of MobilityAPI answers the same MF-JSON, hence the same data of `TripsLayer`; the check compares that data, not pixels | open |
| VS4 | MobilityDeck, MOVE | The frame and load times, measured as MOVE measured its own, on data like AIS and like STIB: `TripsLayer` fed typed arrays against GeoJSON, and the arrays of `tpoint_coords` against the MF-JSON `meos.js/deckgl` parses; tiles cut by space alone against tiles cut by space and time (`spaceTimeSplit`); trips restricted and simplified by `meos.js` in the browser against the same computation in the engine; in MOVE, MEOS per feature and frame (`pymeos_cffi`, the fastest way of MOVE-bench) against NumPy over the arrays of `tpoint_coords` prepared once per trip. The examples keep the faster of each pair, the measurements recorded beside them | after V1, V2 |
| VS5 | MOVE | MOVE reads its trips through MobilityAPI or from files, decodes and restricts them with PyMEOS, and computes each frame the way VS4 keeps; its NumPy interpolation of M-encoded pieces goes, and with it the materialized views it builds in MobilityDB; PyMEOS installs into the Python of QGIS on Linux, macOS and Windows (the owner, 2026-11-24) | after VS4, V3 |
| VS6 | MobilityDeck, MEOS.js | The types beyond points: a `tgeometry` and a `trgeometry` drawn as the polygon of each frame, a `th3index` as its hexagon (`H3HexagonLayer`), a `tcbuffer` as its circle, and a `tfloat` or `tint` attribute as a chart whose cursor follows the clock; the value of each frame is `valueAtTimestamp`, the shape of an instant the one its type states | after V6 |
| VS7 | MobilityDeck | One clock for the past and the stream: a stored trip read through MobilityAPI Part 1 and a live one through Part 4, the slider passing from the one to the other, a live trip showing the watermark of its stream | after V5, V9 |
| VS8 | MobilityOpenLayers, MobilityLeaflet, MobilityGeoServer, MobilityDeck | The viewer of each environment on the stack, one pull request in each repository: MobilityOpenLayers (OpenLayers 10) and MobilityLeaflet (Leaflet 1.9, its FastAPI server and SQL functions removed) read their trips from a file or a MobilityAPI collection through `meos.js/viewer`, each position `positionAt` on a clock held as a `bigint`; MobilityGeoServer publishes, as plain geometries, what MobilityDB computes; the notebooks of MobilityDeck (`notebook/`) run SQL cells through JupySQL over any engine and draw a temporal column through `meos.js/deckgl`. Franchise is archived after the notebooks, its README pointing to them (the owner, 2026-11-27) | MobilityOpenLayers (branch `feat/openlayers-on-meosjs`) and MobilityLeaflet (branch `feat/leaflet-on-meosjs`) written: each reads the 154 trips from the file and from a collection, 0 failed, on the MEOS.js 2.0.0 candidate; MobilityGeoServer and `notebook/` open |
| VS9 | MobilityDB-workshop | The demo, any viewer over any engine: a participant chooses a viewer (MobilityDeck, MobilityOpenLayers, MobilityLeaflet, the notebooks, MOVE in QGIS) and an engine (MobilityDB, MobilityDuck, MobilitySpark, the stream of MobilityFlink or MobilityKafka) and plays the same trips; the engine is the backend MobilityAPI serves and the viewer reads it through `meos.js/viewer` or PyMEOS, so every pair is a choice of two names among those VS3 checks; one compose file starts MobilityAPI, the engines and the browser viewers, MOVE running in the participant's QGIS (the owner, 2026-11-27) | after VS3, VS8 |

## The prerequisites, outside the viewers

| Row | Repository | What | State |
|---|---|---|---|
| V1 | MobilityDB | The coordinates and timestamps of each instant: `double *tpoint_coords(const Temporal *temp, double **ycoords, double **zcoords, TimestampTz **times, int *count)`, the inverse of `tpointseq_make_coords`, no SQL function (row G121 of MobilityDB #2861) | done: MobilityDB #3966, merged as `53d7dbd645` |
| V2 | MEOS.js, PyMEOS | The bindings the viewers install, released from one tag of MobilityDB (rows X5 to X8 of MobilityDB #3928): MEOS.js 2.0.0 carries `meos.js/viewer` and decides the redundant instants of a sequence exactly | `meos.js/viewer`: MEOS.js #68, merged as `f328ad7d55`; MEOS.js 2.0.0: MEOS.js #69, a draft until every binding passes on the tag `v1.4.0-beta1` of `5093da48f5`, its surface regenerated from that commit first |
| V3 | PyMEOS-CFFI | Windows wheels, so the Python of QGIS on Windows installs PyMEOS | open |
| V4 | MobilityAPI | `asBinary` as its binary response (row IO-B24 of MobilityDB #3788) | open |
| V5 | MobilityAPI, MobilityFlink | The watermark of a stream carried through the bridge to each event | open |
| V6 | MEOS.js | `meos.js/deckgl` drawing polygons, hexagons, circles and series of values | open |
| V7 | MobilityAPI | A backend for MobilityNebula; VS3 compares three engines without it | open |
| V8 | MobilityQGIS | Archived, its README pointing to MOVE (the owner, 2026-11-27) | the owner's action |
| V9 | MobilityAPI | A continuous query over the moving features of a whole collection, `/collections/{cid}/tgsequence/queries`, each event carrying its feature's identifier, claiming no OGC class as the queries of one feature do (the owner, 2026-11-27) | open |
| V10 | MobilityDB | `asMVTGeom` of a sequence set answers lines alone, one timestamp per vertex in order, a step set one MULTIPOINT, so `ST_AsMVT` encodes every tile | branch `fix/mvtgeom-no-collection` of another lane, awaiting the owner |

## The order

VS1, VS2 and VS3, the OpenLayers, Leaflet, GeoServer and notebook viewers of VS8, and V3, V4 and V9
start at once and in parallel. VS4 follows V1 and V2, VS5 VS4 and V3, VS6 V6, VS7 V5 and V9, VS9
VS3 and VS8. The viewers of VS8 and VS2 publish once MEOS.js 2.0.0 is released (V2).

The risk: MOVE on Windows rests on V3, which the build of PyMEOS-CFFI does not produce.

## Left out, and why

- **A new repository for the shared display code.** `meos.js/viewer` and `meos.js/deckgl` are that
  code for the browser and PyMEOS for QGIS; a third home would copy them.
- **Apache Arrow as the transport.** The binary interchange of every type and engine is `asBinary`
  (row IO-B24 of MobilityDB #3788), and the flat arrays `TripsLayer` reads are `tpoint_coords`
  (V1). Arrow returns only if VS4 measures a cost these two leave.
- **A GeoServer data store or a WMS `TIME` dimension.** A tile or a map image holds the value of a
  type at one time and loses its interpolation; the tiles come from `asMVTGeom`, and the temporal
  value from MobilityAPI.
- **A QGIS Processing provider.** It sends MOVE's computation back to SQL in MobilityDB alone, where
  VS5 moves it to MEOS in the client for every engine.
- **A conformance suite across viewers.** VS3 is that suite: the same query through each engine
  gives the same data of each layer, compared as data, not pixels.
- **A fork of a third-party notebook application.** The notebook environment is served by Jupyter
  (VS8), code of the ecosystem's own.
