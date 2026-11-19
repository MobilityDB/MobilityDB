TARGET: comment on opengeospatial/ogcapi-coverages#238

One more media-type point, in the OpenAPI document rather than the text.

**The defect, measured.** In `standard/openapi/ogcapi-coverages-1.bundled.json` (master `8c3ef1b`), the LAS and LAZ encodings of the coverage response are keyed `vnd.las` and `vnd.laszip`, with no type before the slash. A media type is a type and a subtype, so these are not media types, and a client generated from the document cannot request either encoding. Every other encoding there is keyed in full (`image/tiff; application=geotiff`, `application/vnd.cov+json`, `application/x-netcdf`, ...).

**A candidate resolution.** Key them `application/vnd.las` and `application/vnd.laszip`, the types the LAS and LAZ requirements classes name.

**How MobilityAPI solves it.** [MobilityAPI](https://github.com/MobilityDB/MobilityAPI), which implements six classes of this draft over raster collections, writes GeoTIFF (`image/tiff; application=geotiff`) and CoverageJSON (`application/vnd.cov+json`) only, so it offers neither LAS nor LAZ and is not affected; it reports the keys as found while implementing the draft.

**Evidence.** The count per media type of the bundled document is on https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Coverages-in-MobilityDB (point 3).
