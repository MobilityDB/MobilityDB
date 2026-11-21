<!--
  MobilityDB — OGC filing draft for opengeospatial/ogcapi-environmental-data-retrieval
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# `observedProperty.id`: required by the requirement, optional in the schema

**Defect.** `/req/edr/rc-parameters` F:

> A parameter object SHALL have a member with the name "observedProperty" where
> the value is an object which SHALL have the members "label" and "id" ...

The schema `collections/observedPropertyCollection.yaml` requires `label` alone:

```yaml
type: object
title: name of property
required:
  - label
```

and CoverageJSON (OGC 21-069r2), in which an EDR answer is written, also makes
`id` optional for an observed property. A parameter that names no registry
entry has no `id` to give, so the requirement forces an invented one.

**Resolution.** Align the requirement with the schema and with CoverageJSON:
"which SHALL have the member "label" and which MAY have the members "id" and
"description"; "id", if given, SHALL be a URI resolving to the definition of
the observed property."

**How MobilityAPI solves it.** The service follows the schema. Each band of a
raster collection is one `parameter_names` entry, `{"type": "Parameter", "id":
"band<n>", "label": "Band <n>", "data-type": ..., "observedProperty": {"label":
{"en": "Band <n>"}}}`, the observed property labelled exactly as the
CoverageJSON answers of the same collection label it, and given no `id`, since
a band of a raster names no entry of any registry.

**Evidence.** MobilityAPI `coverage.go` (`coverageDescription`) at commit
`baf7261`; `TestLiveRasterCollection` reads `parameter_names` of a raster
collection.
