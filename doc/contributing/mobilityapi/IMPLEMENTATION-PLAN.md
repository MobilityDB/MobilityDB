<!--
  MobilityDB — MobilityAPI: the classes and channels beyond its conformance claim
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityAPI beyond its declared classes — Implementation plan

Decided by the owner, 2026-11-21 ("go with your recommendations"). MobilityAPI
master at the start: `11eb22f`.

| row | what | state |
|---|---|---|
| U1 | The description of a raster collection completed under `/req/edr/rc-collection-info`: `parameter_names` (one Parameter per band, as `asCoverageJSON` names them), `output_formats`, `keywords`, a `type` on every link, a `data` link per EDR query. No class declared. | next |
| U2 | Moving Features collections answer the EDR queries `area`, `radius`, `cube` and `trajectory`, each the moving features the geometry meets, written in GeoJSON or MF-JSON and computed by MobilityDB; every collection then satisfies `collections`, and `collections` and `queries` are declared. | planned |
| U3 | A raster collection answers `radius` (the coverage clipped to the buffer of a point, `within` and `within-units`, `distanceunits` stated), `cube` (`bbox` and `z`, `z` read and not applied as a raster has no vertical axis) and `instances` (each period of the collection an instance). | planned |
| U4 | CoverageJSON written in the spelling the client asks for, `application/prs.coverage+json` or, by default, `application/vnd.cov+json`; `covjson` declared. | planned |
| U5 | Two OGC filings drafted for the owner: `crs` as objects or as strings, and `f` or `output_formats`; the service follows `collection.yaml`. | planned |
| U6 | A continuous query over a temporal geometry (`.../tgsequence/queries`) applies the window aggregations of Part 4 Table 7: MovingPoint `COUNT`, `LENGTH`, `INTERSECT`, `TO_TRAJ`; MovingLineString `COUNT`, `LENGTH`, `AREA`, `INTERSECT`; MovingPolygon `COUNT`, `AREA`, `INTERSECT`, each computed by MobilityDB. `VOLUME` is refused naming the function MEOS lacks. | planned |
| U7 | Results of a continuous query delivered over a WebSocket channel beside Server-Sent Events, then to a Kafka topic named by the query's `channel`. | planned |
| U8 | Coverages in NetCDF, written by `asGDALRaster` through GDAL's netCDF driver. LAS and LAZ stay out: they encode point clouds, not the coverage of a raster collection. | planned |
| U9 | The newer Part 4 draft (a Continuous Query resource with `id`, `pubtime`, `target`, `filter`, `queryResult`). | held: no copy of the draft is at hand |
