<!--
  MobilityDB — OGC filing draft for opengeospatial/ogcapi-environmental-data-retrieval
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# One instance is held to the schema of the list of instances

Filed once the implementation it cites is merged: MobilityAPI row U3.

**Defect.** `/req/instances/src-md-success` is the requirement of
`GET /collections/{collectionId}/instances/{instanceId}`, one instance. Its B:

> The content of that response SHALL be based upon the JSON schema
> instances.yaml.

`queries/instances.yaml` is the schema of the *list*: an object requiring
`links` and `instances`, an array of `collection.yaml` items. A document
describing one instance is one such item, not the list, so a server answering
the requirement as written answers a list of one where a single description is
asked for.

**Resolution.** In `/req/instances/src-md-success` B, name
`collections/collection.yaml`, the schema of one item of `instances`, and keep
`instances.yaml` for `/req/instances/rc-md-success`, the list.

**How MobilityAPI solves it.** Over a raster collection, each period is an
instance, identified by its start in RFC 3339. `/instances` answers
`instances.yaml`, the instances in time order with the collection's links;
`/instances/{instanceId}` answers one description in the form of
`collection.yaml`, identical to the entry `/instances` lists for it.

**Evidence.** MobilityAPI `coverage.go` (`listInstances`, `getInstance`);
`TestLiveRasterEDRQueries` compares the description `/instances/{instanceId}`
writes with the one `/instances` lists.
