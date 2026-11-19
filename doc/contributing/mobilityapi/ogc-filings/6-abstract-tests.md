TITLE: Annex A: A.20 cannot be satisfied, A.26 and A.27 request /distance, A.27 asks for an acceleration Linear motion does not carry

## The defect

Three abstract tests of Part 1 (OGC 22-003r3), Annex A.

**A.20** `/conf/movingfeatures/tgsequence-get-success` requires

> Validate that the type property is present and has a value of MovingGeometryCollection
> Validate the prism property is present and that it is populated with an array of TemporalPrimitiveGeometry items

while its own Table A.7 names `temporalGeometrySequence.yaml` as the schema, which gives `"required": ["type", "geometrySequence"]` and `"type": {"enum": ["TemporalGeometrySequence"]}`. No document satisfies both.

**A.26** (Velocity) and **A.27** (Acceleration) both issue

> an HTTP GET request to the URL {root}/collections/{collectionId}/items/{mFeatureId}/tgsequence/{tGeometryId}/distance

so neither exercises the resource it is named for.

**A.27** requires a TReal acceleration with status 200. Under `Linear`, the default motion curve, speed is piecewise constant, so acceleration is zero within each segment and undefined at each vertex; under `Step` and `Discrete` it is undefined as well.

## Proposed resolution

- A.20 validates `type` against `TemporalGeometrySequence` and the member `geometrySequence`, as the schema its Table A.7 cites.
- A.26 requests `.../velocity`, A.27 requests `.../acceleration`.
- A.27 names the motion curves under which an acceleration exists (`Quadratic`, `Cubic`), or states what a server answers for a piecewise-linear trajectory.

## How MobilityAPI solves it

MobilityAPI writes the TemporalGeometrySequence as the schema states (`type: "TemporalGeometrySequence"`, `geometrySequence`) and answers distance, velocity and acceleration each at its own resource. For acceleration it reads the interpolation of the geometry asked for and answers 501 with the reason for that interpolation (for `Linear`: "linearly interpolated position gives a piecewise-constant (Step) speed, whose derivative is zero within each segment and undefined at the vertices"); a geometry that does not exist answers 404. MobilityDB's own differentiation refuses the same value: `derivative()` of the speed raises "The temporal value must have linear interpolation".

## Evidence

- [`ats_part1_corrected_test.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/ats_part1_corrected_test.go): the three corrected abstract tests as executable rows beside the published ones, each run against the service; merged in MobilityDB/MobilityAPI#84.
- `accelerationNotDerivable` in [`main.go`](https://github.com/MobilityDB/MobilityAPI/blob/459a02d/main.go): the reason per interpolation.
- Related: #61 (Validate ATS of OGC API – MF Part 1).

The nine issues of Part 1 and their seven candidate resolutions are stated together on
https://github.com/MobilityDB/MobilityDB/wiki/Implementing-OGC-API-Moving-Features-in-MobilityDB
