<!--
  MobilityDB — MobilityAPI: the classes and channels beyond its conformance claim
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityAPI beyond its declared classes — Plan

The user document `doc/specs/mobilityapi-ogc-api.md` (v1.0.0) states in its
section 12 what MobilityAPI leaves out of scope: the EDR 1.1 classes
`collections`, `queries` and `covjson`; the Part 4 operations over a temporal
geometry and its Kafka and WebSocket channels (MobilityAPI extensions, since
OGC holds no draft of a Stream part); a newer Part 4 draft (none exists); and the
Coverages encodings beyond GeoTIFF and CoverageJSON. This plan takes each one
in, row by row, in `IMPLEMENTATION-PLAN.md`. `STREAM-PROPOSAL.md` is the
proposal for the Stream part, written from the working implementation (row U11),
and `ogc-filings/` holds the EDR filings (row U5).

## Rules

- Every value is computed by a function MobilityDB declares; the service
  composes calls and writes documents, and computes nothing itself.
- One row is one MobilityAPI pull request (branch on MobilityDB/MobilityAPI),
  with its tests in the service's suite; a MEOS or MobilityDB change a row needs
  is its own MobilityDB pull request, merged first.
- Each merged row raises the minor version of `doc/specs/mobilityapi-ogc-api.md`
  in a MobilityDB pull request, moving its item out of section 12 into the
  section of its standard; the document states only what MobilityAPI master
  answers.
- Where a standard contradicts itself, the service follows the normative schema
  and the contradiction is drafted as an OGC filing for the owner to submit.

## What the measurement found (EDR 1.1, OGC 19-086r6, tag `1.1.0-final`)

- Every query of the class `queries` (`position`, `radius`, `area`, `cube`,
  `trajectory`, `corridor`, `items`, `locations`, `instances`) is one a server
  MAY answer; the class inherits `collections`.
- `collections` binds every collection of `/collections`
  (`/req/edr/rc-collection-info`): `links` holding `self`, an `alternate` per
  media type and at least one `data` or `instance` link, each with `rel` and
  `type`; `extent`; `crs`; the output formats; `parameter_names`. The schema
  `collection.yaml` requires `links`, `id`, `extent`, `data_queries`,
  `parameter_names`, `output_formats` and `crs`. A Moving Features collection
  carries none of the query members, so the class is declared only once every
  collection answers an EDR query.
- EDR 1.1 contradicts itself twice: `/req/edr/rc-crs` describes `crs` as
  objects with `name` and `wkt` where `collection.yaml` makes it an array of
  strings, and `/req/edr/rc-collection-info` J names the formats member `f`
  where `collection.yaml` requires `output_formats`.
- `covjson` names `application/prs.coverage+json`; the service writes
  `application/vnd.cov+json`, the type OGC CoverageJSON (OGC 21-069r2) states.

## Acceptance of the whole

Every row merged or held with its reason stated; section 12 of the user document
lists only what a row holds; the plan closes unmerged, its last comment naming
the version of the user document that carries its insights.
