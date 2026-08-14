# OGC GeoPose 1.0 schemas

The normative JSON schemas of the OGC GeoPose 1.0 conformance classes that
MobilityDB implements, retrieved on 2026-08-14 from

    https://schemas.opengis.net/geopose/1.0/schemata/

| File | Conformance class |
|---|---|
| `GeoPose.Basic.YPR.Schema.json` | Basic-YPR |
| `GeoPose.Basic.Quaternion.Schema.json` | Basic-Quaternion |
| `GeoPose.Composite.Sequence.Series.Regular.Schema.json` | Regular Time Series |
| `GeoPose.Composite.Sequence.Series.Irregular.Schema.json` | Irregular Time Series |

The schemas are those of OGC GeoPose 1.0 (OGC 21-056r11) and belong to the
Open Geospatial Consortium, which licenses them for redistribution under the
OGC Software Notice reproduced in `LICENSE` beside them, as `clipper2` and
`h3-pg` carry the notice of what they vendor. That notice asks for the
copyright to be retained and for any modification to be declared, so the files
are held here unmodified, byte for byte as retrieved, with these digests:

    5508d26aed6f…  GeoPose.Basic.YPR.Schema.json
    ce15e01a2ea8…  GeoPose.Basic.Quaternion.Schema.json
    2c23b154da01…  GeoPose.Composite.Sequence.Series.Regular.Schema.json
    69f49530f65f…  GeoPose.Composite.Sequence.Series.Irregular.Schema.json

`tools/scripts/check_geopose_conformance.py` validates against them the GeoPose
documents that `expected/103_pose_geopose.test.out` holds. Keeping a copy here
rather than fetching at run time makes the check depend on nothing outside the
repository, so it gives the same verdict in a network-less build as in CI.

The four remaining conformance classes -- Advanced, Chain, Graph and Stream --
have schemas at the same place, and belong here as soon as MobilityDB emits a
document of one of them.
