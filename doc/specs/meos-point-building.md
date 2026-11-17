<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Point Building
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Point Building — Behaviour Specification

| | |
|---|---|
| Version | v0.1.0 |
| Scope | The temporal points MEOS builds from coordinates: the conversions to a temporal geometry or geography point, the points a lifted function returns, and the sequences built from arrays of coordinates; what they answer, what they refuse, and when their sequences are normalized |
| Reference | `meos/src/geo/tgeo.c` (`tpointinst_coords_size`, `tpointinst_coords_set`, `tpointinst_make_coords`), `meos/src/temporal/tsequence.c` (`tpointseq_make_coords`), `meos/src/npoint/tnpoint.c`, `meos/src/cbuffer/tcbuffer.c`, `meos/src/geo/tgeo_spatialfuncs.c`, `meos/src/temporal/lifting.c` (`tfunc_tpointseq`) |

## 1. Scope and intent

Many MEOS operations answer a temporal point that no user gave: a temporal
network point, a temporal circular buffer or a temporal geometry point turned
into a temporal geometry or geography point, the centres of the cells of a
temporal cell index, the position of a temporal pose, the result of a lifted
function. MEOS writes the points of such answers directly from their
coordinates, instead of building each point as a separate geometry, copying
it into an instant and freeing it. This document states what a user can rely
on: the answers are the same values, byte for byte, as when every point is
built as a geometry first; what is refused, and with which error; and which
answers are normalized. It is prescriptive: every path of MEOS that builds
points from coordinates keeps these guarantees.

## 2. Notation

A *point instant* is a temporal geometry or geography point instant whose
value is a single point with X and Y, and Z when the point has one, with no
bounding box and no M. *Built as a geometry* means made by `tinstant_make`
from the point `geopoint_make` serializes.

## 3. The guarantee: the same bytes

A point instant written from its coordinates is, byte for byte, the instant
built as a geometry from the same coordinates, SRID, dimensionality and
geodeticity: the same header, flags, SRID, coordinates and timestamp, and
zero in every byte the layout leaves unused. Hence every operation reading
such a value (output in every format, equality, ordering, hashing, the
B-tree and GiST indexes, the size of the value on disk) answers as it does
for the value built as a geometry. A sequence of such instants is, byte for
byte, the sequence `tsequence_make` makes of them with the same
normalization.

| path | what it builds | implementation |
|---|---|---|
| `tnpoint_to_tgeompoint` and the casts of a temporal network point | each position on its route | `route_point_tinstant` |
| `tpointseq_make_coords` | a sequence from arrays of coordinates and timestamps | `tpointseq_make_coords` |
| `tgeompoint_to_tgeogpoint` and every conversion to a temporal geography point that goes through it | the geography point of each geometry point | `tgeominst_tgeoginst`, `tgeomseq_tgeogseq` |
| lifted functions whose result is a temporal geometry or geography point: the centres of H3, quadbin and S2 cells, the position of a pose, a rigid geometry to a point, `force2d`, the centroid | each point the function returns | `tfunc_tpointseq` |
| `tcbuffer_to_tgeompoint` and every operation that converts a temporal circular buffer | the centre of each circular buffer | `tcbufferinst_tgeompointinst`, `tcbufferseq_tgeompointseq` |

The points of a sequence are written in one block, each after the first
copying its header, so the cost of a point is that of its coordinates and
timestamp.

## 4. What is refused

A value is validated where it enters MEOS. The loop writing the points
tests nothing again, and refuses nothing the same value built as a geometry
would accept:

- `tpointseq_make_coords` refuses a non-positive count, a missing array of
  X, Y or timestamps, a NaN or infinite coordinate, and, for a geography, a
  SRID that is not a latitude-longitude one; a geography stating no SRID is
  in WGS84 (4326).
- A route of the ways file a temporal network point reads is refused unless
  it is a line, without Z or M, with finite coordinates, in the SRID of the
  network, which is the SRID of the first route read (`ensure_valid_route`).
- A coordinate an operation computes beyond the largest double raises
  `value out of range: overflow` and fails the whole call: no instant is
  stored as NULL and none is skipped
  ([meos-finite-values](meos-finite-values.md)).
- A lifted function whose result is a point refuses each point as
  `tinstant_make` refuses a value: a point that is empty or not finite raises
  the error `tinstant_make` raises; a difference of SRID or of dimensionality
  among the points raises, once every point is made, the error
  `tsequence_make` raises. The errors come in the order of building every
  point as a geometry.

## 5. Normalization

| path | normalized |
|---|---|
| the conversion of a temporal network point | no |
| the conversion of a temporal circular buffer | no |
| the conversion of a temporal geometry point to a geography point | yes: the points redundant on the sphere are not those redundant on the plane |
| lifted functions whose result is a point | yes: two cells may share a centre |
| `tpointseq_make_coords` | as its `normalize` argument says |

## 6. The ways cache

The conversion of a temporal network point reads its routes through the
ways cache stated in [meos-caches](meos-caches.md). Each cache entry keeps,
beside the route, the line read from it and the fraction of the route at
each of its vertices, counted in the bound of the cache and freed with the
entry, so a position on a route is found by bisection over the fractions,
which are the sums `route_point_at` computes, so the point is the same
double. The cache is bounded, indexed by the offset of each route in the
file, and kept per thread for the file that thread names.

## 7. Versioning and stability

The minor version rises with each path added to section 3 or row added to
sections 4 and 5; the major version rises when a guarantee of section 3
changes.

## 8. Out of scope

The arithmetic of each path (where a position on a route or the centre of a
cell is computed) is that of its operation and is stated with it; this
document states how the answer is written. Values a user gives are built by
the input functions stated in [meos-io](meos-io.md).

## 9. References

- [meos-caches](meos-caches.md)
- [meos-finite-values](meos-finite-values.md)
- [meos-io](meos-io.md)
