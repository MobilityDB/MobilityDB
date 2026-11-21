<!--
  MobilityDB — OGC filing draft for opengeospatial/ogcapi-environmental-data-retrieval
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# `crs` of a collection: objects in the requirement, strings in the schema

**Defect.** `/req/edr/rc-collection-info` G requires every collection to carry a
`crs` member complying with `/req/edr/rc-crs`, which describes crs *objects*:

> A crs object SHALL have a unique (to the collection) `crs` property. It MAY be
> an EPSG code.
> A crs object SHALL have a `wkt` property which SHALL be a correctly structured
> Well Known Text definition for the CRS.

The schema `collections/collection.yaml` states the same member as an array of
strings:

```yaml
crs:
  description: list of the coordinate reference systems the collection results can support
  type: array
  items:
    type: string
```

A document valid against the schema cannot satisfy the requirement, and the
reverse. Both stand in 1.1 (`1.1.0-final`) and on the repository's master.

**Resolution.** State `/req/edr/rc-crs` over the schema's form: each element of
`crs` is a string naming a coordinate reference system by its URI, as OGC API -
Common - Part 2 and Features name them. Where the WKT of a system is to be
published, give it a member of its own (for instance `crs_details`, an array of
objects `{crs, wkt}`) rather than changing the type of `crs`.

**How MobilityAPI solves it.** The service follows the schema: every collection
of `/collections` carries `crs` as an array of URI strings,
`http://www.opengis.net/def/crs/EPSG/0/<srid>`, the system of the collection's
geometries or rasters, the same form its Moving Features collections carry.

**Evidence.** MobilityAPI `coverage.go` (`coverageDescription`) and `main.go`
(`listCollections`) at commit `baf7261`; `TestLiveRasterCollection` reads the
description of a raster collection in `/collections` and in
`/collections/{collectionId}` and compares them member by member.
