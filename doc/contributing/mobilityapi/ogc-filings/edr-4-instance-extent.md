<!--
  MobilityDB — OGC filing draft for opengeospatial/ogcapi-environmental-data-retrieval
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# An instance is required to carry the extent of its collection

Filed once the implementation it cites is merged: MobilityAPI row U3.

**Defect.** `/req/instances/src-md-success` C:

> The content of that response SHALL be consistent with the content for this
> resource collection in the `/collections` response. That is, the values for
> `id`, `title`, `description` and `extent` SHALL be identical.

The clause is copied from `/req/collections/src-md-success` C, where it holds
one collection to its entry in `/collections`. Applied to an instance it states
that every instance has the `id` and the `extent` of its collection, so no two
instances differ, while instances exist to be versions of the collection over
different times (model runs, periods), each with its own extent. Issue #526
met the temporal half of this for model runs.

**Resolution.** State C over the list of instances: "The content of that
response SHALL be consistent with the content for this instance in the
`/collections/{collectionId}/instances` response. That is, the values for
`id`, `title`, `description` and `extent` SHALL be identical." Add that the
extent of an instance lies within the extent of its collection.

**How MobilityAPI solves it.** An instance of a raster collection is one
period: its `extent` is the bounds of the tiles of that period and the period
itself, within the collection's extent, and its description is identical to
its entry in `/instances`. The queries under an instance read that period
alone.

**Evidence.** MobilityAPI `coverage.go` (`coverageDescription` over an
instance, `coverageSource.from`); `TestLiveRasterEDRQueries` reads the extents
of the two instances of a series, January's to x 4 and February's to x 2, and a
position and an area answered under each.
