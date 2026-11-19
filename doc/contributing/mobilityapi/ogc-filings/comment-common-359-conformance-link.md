TARGET: comment on opengeospatial/ogcapi-common#359

On the first divergence listed here, the relation type of the conformance link, with an implementation that satisfies both standards, and one related spelling.

**The defect, measured.** Common Part 1 (19-072) `/req/landing-page/root-success` requires the conformance declaration to be linked under `http://www.opengis.net/def/rel/ogc/1.0/conformance`; Features Part 1 `/req/core/root-success` requires `conformance`. Each is a SHALL, so a landing page with one link conforms to one standard only. Part 1 also names the class two ways: `conf/landing-page` in its text and abstract tests, and `conf/landingPage` in the example conformance response of its OpenAPI document (`openapi/3.0/API-Common-Part-1_1_0.yaml`).

**A candidate resolution.** One relation type for the link across the OGC API standards, and a statement that a landing page carrying both links satisfies both requirements. One name for the class in Part 1's text, abstract tests and OpenAPI document.

**How MobilityAPI solves it.** [MobilityAPI](https://github.com/MobilityDB/MobilityAPI), an OGC API – Moving Features server (which builds on Features), links `/conformance` from its landing page under both relation types, and declares `http://www.opengis.net/spec/ogcapi-common-1/1.0/conf/landing-page` beside `core`, marking the landing page, the API definition and the conformance declaration with that class. Each of the six abstract tests of the class is a test of its suite, run through the routing table the service runs.

**Evidence.** `landing` and `relConformance` in [`main.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/main.go); [`ats_common_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_common_test.go), which fails with either link removed; merged in MobilityDB/MobilityAPI#90. The landing page it writes: [`samples/landing-page.json`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/samples/landing-page.json). Details on https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Common-in-MobilityDB
