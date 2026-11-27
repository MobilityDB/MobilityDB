<!--
  MobilityDB — Visualization: assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The viewers of the ecosystem: assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

## The gateway and the libraries

MobilityAPI holds the backends of MobilityDB (`backend.go`), MobilityDuck (`backend_duck.go`) and
MobilitySpark (`backend_spark.go`) and the stream bridges of MobilityFlink and MobilityKafka; it has
no backend for MobilityNebula. A moving feature's temporal geometry is its TemporalGeometrySequence
(`/collections/{cid}/items/{fid}/tgsequence`), whose members are each the `asMFJSON` of one
sequence. A continuous query streams one feature (`/collections/{cid}/items/{fid}/tgsequence/queries`),
over Server-Sent Events, WebSocket and Kafka; no Go source of MobilityAPI carries the watermark of
a stream.

`meos.js/viewer` reads the trips of a viewer: `tgeompointsFromHexWKB` (one temporal point per line
of hex WKB), `tgeompointsFromCollection` (every sequence of every feature of a collection of an OGC
API Moving Features server) and `positionAt` (the coordinates MEOS answers at a timestamp, null
outside the trip and in its gaps). `meos.js/deckgl` turns temporal points into the data of
`TripsLayer` (`tgeompointToTrips`, `tgeompointsToTrips`), restricts and measures them in the browser
(`atGeometry`, `atTime`, `speedSeries`, `tripsWithSpeed`) and makes the layer (`tripsLayer`); it reads
temporal points alone, from their MF-JSON. A timestamp of MEOS.js on `main` is a `bigint`.

The published `meos.js` 1.0.2 decides the redundant instants of a sequence within a band of 1e-6 per
axis, which in degrees is a distance a trip moves: the 154 BerlinMOD trips of
`meos/examples/data/berlinmod_trips.csv`, transformed to SRID 4326, hold 216075 instants read by
libmeos of master and 190569 read through `meos.js` 1.0.2. The module built from MobilityDB
`844cef8be5` decides them exactly (MobilityDB #3030) and reads all 216075. `pymeos-cffi` 1.3.0 on
PyPI holds wheels for Linux and macOS and the source alone, none for Windows.

## The display primitives, in MEOS

| Need | MEOS function (SQL name) |
|---|---|
| Interchange text | `temporal_as_mfjson` (`asMFJSON`) |
| Interchange binary | `temporal_as_wkb`, `asBinary`, `asHexWKB` |
| Coordinates and timestamps of each instant | `tpoint_coords`, no SQL function (MobilityDB #3966), the inverse of `tpointseq_make_coords` |
| Vector tiles | `tpoint_as_mvtgeom` (`asMVTGeom`) |
| Level of detail | `douglasPeuckerSimplify`, `minDistSimplify`, `maxDistSimplify` |
| Chunks in space and time | `tgeo_space_time_split` (`spaceTimeSplit`), `temporal_time_split` (`timeSplit`) |
| Clipping | `atGeometry`, `atTime` |
| The value of each frame | `valueAtTimestamp`, over `tgeompoint`, `tgeometry`, `trgeometry`, `th3index`, `tcbuffer` and every temporal type |

Each is public in the catalog, so every engine generated from it exposes it, and an engine that does
not is a gap of its parity ledger: MobilityNebula has neither `asMVTGeom` nor `spaceTimeSplit`.

## The viewers of the organization

| Repository | Environment | What it holds | On the stack |
|---|---|---|---|
| MobilityDeck | Browser, large datasets on the GPU (deck.gl) | `mvt/` (tiles of `pg_tileserv`, deck.gl 8, React 16, webpack 4) and `geojsonvt/` (a GeoJSON file of trips tiled in the browser by a fork of geojson-vt); `onTileLoad` of `mvt/app.js` slices the times of the third and later parts of a MultiLineString from the wrong offset (`index = coords.length`, never `+=`), and hands the coordinates of a Point to `TripsLayer` as a path | VS1, VS2, VS6, VS7 |
| MOVE | QGIS | A plugin building a materialized view per temporal column of a query, one row per piece of a sequence whose M counts the seconds since its start, the QGIS temporal controller animating it (`QgsVectorLayerTemporalProperties`); its Fast preview interpolates each piece on every frame with NumPy | VS5 |
| MobilityOpenLayers | Browser GIS applications combining many sources and OGC services | An OpenLayers 6.14 page over a file of trips (`trips.json`), stepping each trip one vertex per frame | VS8 |
| MobilityLeaflet | Lightweight browser maps | A 2023 master's thesis: a Leaflet front end interpolating positions in JavaScript, its own FastAPI server and its own SQL functions | VS8 |
| MobilityGeoServer | Organizations publishing through the OGC services of a GeoServer | A page showing with Leaflet and OpenLayers the layers a GeoServer publishes | VS8 |
| MEOS.js-examples | Browser | `trips.html` (BerlinMOD trips with deck.gl over MapLibre), `compare.html`, JupyterLite notebooks; depends on `meos.js` `^1.0.2` | VS2, a consumer of the release |
| MobilityAPI `tutorial-stream/map` | Browser | The animated stream and fleet maps: Vite, deck.gl 9, MapLibre, MEOS.js computing each position with `valueAtTimestamp`; depends on `meos.js` `^1.0.1` | VS1, a consumer of the release |
| MobilityFlink-Deck | Browser, the Flink stream | A React 16 front end interpolating positions in JavaScript from its own Flask, MQTT and WebSocket servers | VS1, then archived |
| MobilityQGIS | Scripting in the Python console of QGIS | The experiments of a ULB MA1 project: six scripts computing positions with MobilityDB-python (an archived driver) over `atPeriod` and `period`, which MobilityDB does not declare | Subsumed by MOVE; archived (V8) |
| Franchise | A notebook SQL client | A fork of HVF/franchise, three commits on its 2019 stack, whose MobilityDB cell draws a column of type `tgeompointseq`, which MobilityDB does not declare | Subsumed by the notebooks of VS8; archived after them |

## The ways to animate a trip, measured

MOVE-bench, the benchmark of a ULB master's thesis (2026), measured six ways of animating
MobilityDB trajectories in QGIS, median milliseconds per frame on the STIB buses (17k) and the AIS
ships (6k), for PyMEOS 1.2 and the MOVE its `move_plugin/` carries:

| Way | STIB | AIS |
|---|---|---|
| `pymeos_cffi` on raw EWKB, `tpoint_value_at_timestamptz` per feature and frame | 5.5 | 15.4 |
| memory layer and expression | 7.5 | 126 |
| MOVE's Fast preview, NumPy over the pieces | 13.3 | 67 |
| columnar NumPy | 22.9 | 28.6 |
| PyMEOS objects, `value_at_timestamp` per feature and frame | 60.5 | 128 |
| MOVE's PostgreSQL layer and expression | 120 | 550 |

MOVE #16 holds a layer in chunks of segments, from 2848 to 186 ms per frame; MOVE #17 draws the
positions on the canvas rather than through a layer, from about 190 to 16.8 ms.
