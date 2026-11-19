TARGET: comment on opengeospatial/ogcapi-environmental-data-retrieval#494

A follow-up on point 3 (the CoverageJSON media type), from an implementation.

**The defect, measured.** The repository names `application/vnd.cov+json` in the requirement, the abstract test, the conventions and `responses/queries/200.yaml` (#511), and the 1.2.0 bundles on master name `vnd.cov+json` only. But the 1.1 OpenAPI document published at https://schemas.opengis.net/ogcapi/edr/1.1/openapi/ogcapi-environmental-data-retrieval-1.bundled.json still writes `application/prs.coverage+json` on all 17 paths that answer CoverageJSON (32 response entries) and `vnd.cov+json` nowhere; so does the bundle of the repository's `1.1.1` tag (19-086r8), which is not published at docs.ogc.org. The published 19-086r6 names `prs.coverage+json` throughout, while OGC CoverageJSON (21-069r2) states "The CoverageJSON media type SHALL be `application/vnd.cov+json`". A client generated from the published 1.1 schemas asks for the type the CoverageJSON standard does not define.

**A candidate resolution.** Publish the 1.1.1 corrigendum, and generate its bundled OpenAPI document from the corrected `200.yaml`, so that every published artefact of EDR names the type CoverageJSON defines.

**How MobilityAPI solves it.** [MobilityAPI](https://github.com/MobilityDB/MobilityAPI) answers the position, area, trajectory and corridor queries over raster collections with `Content-Type: application/vnd.cov+json`, the type CoverageJSON defines and this issue adopts. Its EDR queries select the format by `f` and do not read `Accept`, so a client asking for either spelling gets the answer. It declares EDR's core and not `conf/covjson` of 1.1, since that class's requirement names `prs.coverage+json`.

**Evidence.** `edrQuery` and `edrConformance` in [`edr.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/edr.go); [`edr_live_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/edr_live_test.go) asserts the content type of every query against a fixture series. Measurements on https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-EDR-in-MobilityDB
