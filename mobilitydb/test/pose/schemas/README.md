# OGC GeoPose 1.0 schemas

The normative JSON schemas of the OGC GeoPose 1.0 conformance classes that
MobilityDB implements, retrieved on 2026-08-14 from

    https://schemas.opengis.net/geopose/1.0/schemata/

| File | Conformance class |
|---|---|
| `GeoPose.Basic.YPR.Schema.json` | Basic-YPR |
| `GeoPose.Basic.Quaternion.Schema.json` | Basic-Quaternion |
| `GeoPose.Advanced.Schema.json` | Advanced |
| `GeoPose.Composite.Sequence.Series.Regular.Schema.json` | Regular Time Series |
| `GeoPose.Composite.Sequence.Series.Irregular.Schema.json` | Irregular Time Series |
| `GeoPose.Composite.Sequence.StreamHeader.Schema.json` | Stream, the document opening one |
| `GeoPose.Composite.Sequence.StreamElement.Schema.json` | Stream, the document repeated in one |
| `GeoPose.Composite.Sequence.Stream.Schema.json` | Stream, the whole of one as a single document |
| `GeoPose.Composite.Chain.Schema.json` | Chain |
| `GeoPose.Composite.Graph.Schema.json` | Graph |
| `GeoPose.Basic.Strict_Quaternion.Schema.json` | Basic-Quaternion, the form admitting no other member |

The schemas are those of OGC GeoPose 1.0 (OGC 21-056r11) and belong to the
Open Geospatial Consortium, which licenses them for redistribution under the
OGC Software Notice reproduced in `LICENSE` beside them, as `clipper2` and
`h3-pg` carry the notice of what they vendor. That notice asks for the
copyright to be retained and for any modification to be declared, so the files
are held here unmodified, byte for byte as retrieved, with these digests:

    5508d26aed6f…  GeoPose.Basic.YPR.Schema.json
    ce15e01a2ea8…  GeoPose.Basic.Quaternion.Schema.json
    236887312bee…  GeoPose.Advanced.Schema.json
    2c23b154da01…  GeoPose.Composite.Sequence.Series.Regular.Schema.json
    69f49530f65f…  GeoPose.Composite.Sequence.Series.Irregular.Schema.json
    ee3b1944912b…  GeoPose.Composite.Sequence.StreamHeader.Schema.json
    01791c205790…  GeoPose.Composite.Sequence.StreamElement.Schema.json
    fd24663a1241…  GeoPose.Composite.Sequence.Stream.Schema.json
    72ccc4457dbf…  GeoPose.Composite.Chain.Schema.json
    d498e71da192…  GeoPose.Composite.Graph.Schema.json
    06d999a2b309…  GeoPose.Basic.Strict_Quaternion.Schema.json

`tools/scripts/check_geopose_conformance.py` validates against them the GeoPose
documents that `pose/expected/103_pose_geopose.test.out` and
`posechain/expected/555_tposechain_geopose.test.out` hold. Keeping a copy here
rather than fetching at run time makes the check depend on nothing outside the
repository, so it gives the same verdict in a network-less build as in CI.

A Basic-Quaternion document carrying only what a static pose has to say
satisfies the strict schema as well, which admits `position` and `quaternion`
and nothing else; a temporal instant additionally carries `validTime` and
satisfies the permissive schema alone. The check therefore holds every
Basic-Quaternion document that names no time to the strict schema, so a member
added to the static encoding is reported rather than quietly narrowing what
MobilityDB can claim. The conformance test suite exercises the permissive form,
which is the one a claim is made against.

Two schemas published at the same place are deliberately absent.
`GeoPose.Basic.Euler.Schema.json` describes a flat `longitude`/`latitude`/
`height`/`rotations` form that is not one of the eight conformance classes and
that no MobilityDB encoding produces. `GeoPose.Composite.Sequence.Stream.Header.Schema.json`
requires a single `transitionModel` typed as a string, where the `StreamHeader`
schema beside it, and the composite `Stream` schema that embeds one, require an
object of `authority`, `id` and `parameters` together with an `outerFrame`: a
header satisfying either fails the other, so the two cannot both describe a
conformant document. MobilityDB writes the form the suite and the composite
schema agree on, and the contradiction is a question for the working group.
