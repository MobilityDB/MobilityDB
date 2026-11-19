# The filings with OGC

The texts row A10 of the [implementation plan](../IMPLEMENTATION-PLAN.md) files with the OGC
Standards Working Groups, and the comments on existing threads that follow from row A7. The owner
files them; each file is the text to paste.

Every filing states four things: the defect, quoted from the published artefact; the candidate
resolution, as text a working group can adopt; how MobilityAPI solves it; and the evidence, the
test that measures it and the pull request that merged it. The links pin MobilityAPI at commit
`459a02d`.

## Issues on `opengeospatial/ogcapi-movingfeatures`

One per candidate resolution of the wiki page [Implementing OGC API — Moving Features in
MobilityDB](https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB).
The first line of each file is its title.

| file | resolution |
|---|---|
| `6-abstract-tests.md` | 6: abstract tests A.20, A.26 and A.27 |
| `1-oneof-overlap.md` | 1: the six overlapping `oneOf` constructs |
| `2-values-array.md` | 2: `values` as one value per instant |
| `3-unit-form.md` | 3: a unit as a register URI or a UN/CEFACT common code |
| `4-interpolation-once.md` | 4: each interpolation concept spelled once |
| `5-datetimes.md` | 5: every set of instants constrained to RFC 3339 |
| `7-examples-ci.md` | 7: the published examples validated in continuous integration |

## Comments on existing threads

The first line of each file names the thread.

| file | thread |
|---|---|
| `comment-common-399-http-https.md` | `opengeospatial/ogcapi-common#399`, http and https in class identifiers |
| `comment-common-359-conformance-link.md` | `opengeospatial/ogcapi-common#359`, the relation type of the conformance link |
| `comment-coverages-238-las-keys.md` | `opengeospatial/ogcapi-coverages#238`, the LAS and LAZ keys of the OpenAPI document |
| `comment-edr-494-republish-1.1.md` | `opengeospatial/ogcapi-environmental-data-retrieval#494`, the CoverageJSON media type of the 1.1 OpenAPI document |
