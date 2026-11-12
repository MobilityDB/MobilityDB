<!--
  MobilityDB — The temporal rigid geometries: assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The temporal rigid geometries: the assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

Measured on master `78839117d9`. Every statement below is an output of `tools/trgeo_gaps.py`.

## The measurement

`tools/trgeo_gaps.py sql ledger` prints the script that asks the catalog for every function taking a
`trgeometry` and calls it over the product of the values below, printing one line per call. `sql
known` and `sql controls` print the cases whose answer the geometry fixes. Each runs in the test
harness, in the place of a test of the family. `report <output>` reports a call that fails where the
same call answers with one property of the failing argument changed.

| Argument | Values |
|---|---|
| Body | convex, concave, with a hole, polyhedral surface; in 3D convex, concave, surface |
| Motion | an instant, translating, rotating; in 2D also discrete and step |
| Static geometry | point, line, convex, concave and holed polygon, multipolygon, collection, circular string, empty; point and polygon with Z |
| Others | `tgeompoint` and `pose` in 2D and 3D, `poseset`, `tpose`, four kinds of `stbox`, the time types, numbers, `text` |

A reference geometry that is a point or a line is refused at construction: "Only polygon or
polyhedral surface geometries accepted". The manual states this.

## Wrong answers, with no error

Each case places a body in the plane z = 1 and a geometry over the same x and y.

| Call | Expected | Answered |
|---|---|---|
| `eIntersects` with `Point(1 1 5)` | false | **true** |
| `eIntersects` of bodies at z = 1 and z = 5 | false | **true** |
| `eIntersects` of a 3D surface with `Point(3 1 5)` | false | **true** |
| `aDisjoint` with `Point(1 1 5)` | true | **false** |
| `eDisjoint` of bodies at z = 1 and z = 5 | true | **false** |
| `eContains`, `eCovers` of a polygon at z = 5 | false | **true** |
| `eTouches` with `Point(0 0 5)` | false | **true** |
| `tIntersects` with `Point(1 1 1)`, inside the body at the start | true, then false | **false, false** (2D control: true, false) |
| `tDisjoint` with `Point(1 1 1)` | false, then true | **true, true** |
| `atStbox` with a box at z in [4,6] | NULL | **the whole value** |
| `minusStbox` with a box at z in [4,6] | the whole value | **NULL** |
| `traversedArea(x, true)` turning by pi/2 | area 10.28, as the 2D control | **area 8** |

These answer rightly on the same values: `aIntersects`, `aTouches` and `eDisjoint` with a geometry
at z = 5, `valueAtTimestamp` in translation and rotation, `length`, `cumulativeLength`, `speed` and
`angularSpeed`. `traversedArea(x)`, the default, answers the pieces in 2D and 3D alike, as the
manual states.

The concave, holed and surface bodies in 2D answered every known case rightly: `eIntersects`,
`eContains`, `tIntersects` and `atGeometry` with a point or small polygon in the notch or the hole.
A known-answer case exists only where listed, so the other functions that answer on a 3D value are
not yet shown right.

The merged `traversedArea` and `convexHull` of a 3D value answer a polygon with no Z. This is the
ground, as Chapter 4 defines the traversed area, "the part of the plane that the object covers", and
the manual takes it as a statement rather than a gap.

## The use case of Chapter 4

Chapter 4 of MobilityBook Vol. 2 and `chapter4/` of its companion, MobilityDB/MobilityDataScienceBookVol2,
use `trgeometry` in three dimensions on the nuScenes dataset. So the type has three dimensions: a road
vehicle has a pitch and a roll, its pose holds a quaternion, and its reference geometry is a flat
rectangle that `ST_Force3D` places in the plane z = 0 of the body, since the constructor requires the
dimension of the pose. `load_nuscenes.sql` builds `ObjectTrip` and `EgoBody` so, and the chapter states
that its algorithm of the temporal distance is for two dimensions.

`nuscenes_queries.sql` meets the refusal of the distance and works around it, building `EgoBody2D` and
`ObjectTrip2D` by hand from the instants of each pose: "the bodies are projected to the ground plane
first". No function taking a `trgeometry` in the catalog projects it.

Measured on v1.0-mini, on the 881 objects and their ego vehicle:

| Question | 3D tables | Ground projection |
|---|---|---|
| `\|=\|` of the ego vehicle and an object | "The trgeometry cannot have Z dimension" | answers |
| The pairs with `tIntersects` ever true | 0 | 1, a car, with `nearestApproachDistance` 0 |
| `eIntersects` of that pair | false | **false**, although `tIntersects` is ever true and the distance 0 |
| `traversedArea(x, true)` | 303 objects differ from the projection, 142 larger and 161 smaller, by up to 4.97% and up to 4.44 m² | |

A flat body at z = 0 and one at z = 0.8 never meet in 3D, while on the road they touch, so a question
about the ground needs the projection. In a pure rotation about z, which tilts nothing,
`traversedArea(x, true)` of the rectangle of the chapter answers 17.885 in 3D and 17.284 in 2D, and with
a translation 45.673 and 50.452, so the difference is not the tilt of the body.

The largest ratio of the swept area on master is 88.73, of a car of scene-1077, and the listing of the
chapter prints 71.4187 for scene-0655 in its first row.

Two relationships disagree in 2D too. Where a body passes through another between two instants,
`eIntersects` is true and `tIntersects` is false at both instants, the subject of the [note on linear
motion](RIGID-SWEPT-RELATIONSHIPS.md). The pair above is the opposite: `tIntersects` is ever true and
`eIntersects` false.

## Operands of different dimensions

MobilityDB #3439 states the rule in the manual section `tgeo_different_dimensions`: an operation
relating two spatial values reads the dimensions both have, and an operation building or comparing
values refuses. A 2D `trgeometry` follows it in its distance: `tDistance`,
`nearestApproachDistance`, `nearestApproachInstant` and `shortestLine` against a 3D point or
geometry answer in 2D, and `tIntersects` and `tDisjoint` answer. These refuse the same operands:

| Functions | Error |
|---|---|
| `aContains`, `aCovers`, `aDisjoint`, `aIntersects`, `aTouches`, `eContains`, `eCovers`, `eDisjoint`, `eIntersects`, `eTouches`, `aDwithin`, `eDwithin`, `tDwithin`, `atGeometry`, `minusGeometry` | "Operation on mixed 2D/3D dimensions" |
| `tContains`, `tCovers`, `tTouches` | "The geometry cannot have Z dimension" |

`aEqual`, `eEqual`, `tEqual`, their negations, `atValue`, `minusValue` and the order comparisons also
refuse, as the rule states for an operation comparing values. A comment of
`trgeo_distance.c` states that a temporal rigid geometry has two dimensions; the type builds 3D values.

## Refusals

| Gap | Functions | Error | Wording for the manual |
|---|---|---|---|
| A value in 3D | `tDistance`, `nearestApproachDistance`, `nearestApproachInstant`, `shortestLine`, `eDwithin`, `aDwithin`, `tDwithin` | "The trgeometry cannot have Z dimension" | The distance of a temporal rigid geometry in three dimensions is not yet supported. |
| A concave, holed or surface body | the same seven | "needs a reference geometry that is a convex polygon without holes" | The distance of a temporal rigid geometry whose reference geometry is not a convex polygon without holes is not yet supported. |
| A concave or holed static polygon | the same seven | "to a polygon that is not convex or has holes is not supported" | The distance to a polygon that is not convex or has holes is not yet supported. |
| A collection or a curve | the same seven | "Unsupported geometry type: GeometryCollection", "CircularString" | The distance to a geometry collection or a curve is not yet supported. |
| A geometry with holes | `atGeometry`, `minusGeometry` | "against a geometry with interior rings (holes) is not implemented" | The restriction to a geometry with holes is not yet supported. |
| A geometry with Z | `atGeometry`, `minusGeometry`, `aContains`, `tContains`, `tCovers`, `tTouches`, and `aCovers` in some forms | "The geometry cannot have Z dimension" | The restriction and these relationships to a geometry in three dimensions are not yet supported. |
| A value in 3D | `tContains`, `tCovers`, `tTouches` | "The tgeometry cannot have Z dimension" | These relationships of a temporal rigid geometry in three dimensions are not yet supported. |
| Another SRID | `transform`, `transformPipeline` | "Transformation to another SRID is not supported for type trgeometry" | The transformation of a temporal rigid geometry to another SRID is not yet supported. |
| A surface | `asMFJSON`, `round` | "'PolyhedralSurface' geometry type not supported", "Unsupported geometry type in round function" | These functions do not yet support a polyhedral surface. TG17 closes both. |

## Defects, which the manual should not state

Each is a call that should answer, or an error that misleads, so its pull request fixes it.

| Call | What happens |
|---|---|
| A literal with an SRID before the pose | `Polygon(...);SRID=25832;Pose(...)` answers with no SRID, and a line break after the SRID is a parse error |
| `atStbox`, `minusStbox` with a box of time alone | "The stbox must have X dimension" |
| `stops` | All 29 calls fail: "Sequence must have a spatial point base type", or the interpolation error, and over a `tpose` too; `CREATE FUNCTION` declares it |
| `tIntersects`, `tDisjoint` of a 3D surface | 208 calls fail with an internal message: "lwgeom_set_geodetic: unsupported geom type: PolyhedralSurface" |
| `round(x, 1)` of a 3D rotation | "Rotation quaternion must be of unit norm", since each coordinate is rounded alone; 6 digits answer |
| `tDistance` of a 3D value and a 3D `tgeompoint` | "The arguments must be of the same dimensionality", although both have Z |

## Related subjects

| Subject | Here |
|---|---|
| The distance of non-convex and holed geometries | [Its note](NON-CONVEX-DISTANCE.md) and [its feasibility](NON-CONVEX-DISTANCE-FEASIBILITY.md). The note lists collections as accepted, while this ledger reads "Unsupported geometry type: GeometryCollection" for a collection holding a point and a line. |
| The relationships in linear motion | [Its note](RIGID-SWEPT-RELATIONSHIPS.md). The refusals #3347 adds are there; this ledger reads master, without #3347. |
| The `transform` signatures | SQL declares them over `trgeometry`, and every call is refused. |
| The aggregates | Merged as #3100 and #3102, and not measured here: the ledger cannot call a transition function outside an aggregate. |
| The similarity distances | The manual states that a `trgeometry` has none. |

## The manual and the design notes against the code

| Text | What it says | The code |
|---|---|---|
| `doc/temporal_rigid_geometries.xml`, Restrictions | "its centroid (antenna) at t lies in that region" | A restriction reads the body: a target over the body answers and one over the rotation centre answers NULL |
| The same, six examples | `temporalTime`, `geom`, an SRID before the pose, `atStbox` with a box of time alone | Each gives an error |
| The same, the chapter's opening | "a static 2D reference geometry" | Builds a 3D reference geometry with a quaternion pose |
| The same, Input and Output | "any 2D polygon or polyhedral surface" | The same |
| `doc/contributing/trgeometry_design_notes.md` | "2D-only reference geometry", "The type carries no 3D rigid-body representation" | `tbl_trgeometry3d` holds such values, `random_trgeom3d_inst` makes them, the index classes declare the operators on Z, and Chapter 4 uses them |
| `trgeo_distance.c`, a comment | "A temporal rigid geometry has two dimensions" | The same |

## What this does not cover

- Whether a 3D value that answers is right, beyond the cases of the first table.
- A body that leaves its plane, except `traversedArea` of a motion along z.
- Aggregates, and index plans, which `162_trgeo_indexes_tbl` covers.
- The geographic forms: `trgeometry` has none in this ledger.
- Arguments a function refuses as invalid, such as a `text` naming no interpolation.
