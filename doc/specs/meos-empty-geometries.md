<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Empty Geometries
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Empty Geometries — Behaviour Specification

| | |
|---|---|
| Version | v1.3.0 |
| Scope | What every operation of MEOS and MobilityDB answers when one of its arguments is an empty geometry or geography, and how a C caller reads that answer |
| Reference | `doc/temporal_spatial_p2.xml` (section *Empty Geometries*), `meos/src/geo/tgeo_spatialrels.c` (relationships), `meos/src/geo/tgeo_distance.c` (distances), `meos/src/geo/tgeo_restrict.c` (restrictions), `meos/src/geo/stbox.c` (boxes), `meos/src/geo/tgeo.c` (constructors) |

## 1. Scope and intent

An empty geometry is the empty set of points: it is nowhere. An operation
given one answers what the empty set answers, and raises neither an error nor
a notice, except where the geometry must be the position of a value. This
document states that rule and its four consequences, and how a binding calling
the C API reads each answer. It is prescriptive: every operation of MEOS, and
therefore of MobilityDB and of every binding generated from MEOS, follows it,
for the geometries and geographies, the temporal geometries and geographies,
the temporal circular buffers and the temporal rigid geometries.

The rule is the one PostGIS follows. `ST_Intersects`, `ST_Contains`,
`ST_Covers`, `ST_Touches` and `ST_DWithin` of an empty geometry are false and
`ST_Disjoint` is true, `ST_Distance` and `ST_ShortestLine` are `NULL`, and
`ST_Difference` with an empty geometry is the other operand. A user who knows
PostGIS reads the same answers on a temporal value.

The empty set is an answer, never an invalid argument. A function therefore
tests the validity of its arguments first, its SRID, its geodetic flag, its
dimensionality, and only then answers the empty set: an empty geometry in
another reference system is an error, as any geometry in another reference
system is.

## 2. Notation

*Empty geometry* covers every empty spelling: `POINT EMPTY`, `LINESTRING
EMPTY`, `POLYGON EMPTY`, the empty multi-geometries and `GEOMETRYCOLLECTION
EMPTY`, and the same for geographies. A *temporal value* is a value of a
temporal type whose base type is spatial. *Its time* is the set of instants at
which the temporal value is defined.

## 3. The rule and its consequences

```sql
-- It meets nothing
SELECT eIntersects(geometry 'Point empty',
  tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]');
-- f
SELECT tDisjoint(geometry 'Point empty',
  tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]');
-- [t@2001-01-01, t@2001-01-03]
-- Nothing can be measured to it
SELECT geometry 'Point empty' <->
  tgeompoint '[Point(2 2)@2001-01-01, Point(1 1)@2001-01-02, Point(2 2)@2001-01-03]';
-- NULL
-- It removes nothing and keeps nothing
SELECT asText(minusGeometry(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02,
  Point(1 1)@2001-01-03]', geometry 'Linestring empty'));
-- [POINT(1 1)@2001-01-01, POINT(2 2)@2001-01-02, POINT(1 1)@2001-01-03]
SELECT asText(atGeometry(tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02,
  Point(1 1)@2001-01-03]', geometry 'Linestring empty'));
-- NULL
-- It cannot be the position of a value
SELECT tgeompoint(geometry 'Point empty', timestamptz '2001-01-01');
-- ERROR:  Only non-empty geometries accepted
```

### 3.1 It meets nothing

A relationship asking for a common point is false: `intersects`, `contains`,
`covers`, `touches`, the distance within, and the equality. `disjoint` and
the inequality are true. A temporal relationship or a temporal comparison
takes this answer over the whole time of the temporal value: `tIntersects`,
`tContains`, `tCovers`, `tTouches`, `tDwithin` and the temporal equality are
the constant false over its time, `tDisjoint` and the temporal inequality the
constant true. The ever and always forms answer the same.

### 3.2 Nothing can be measured to it

The distances (`|=|`, `<->`), `nearestApproachInstant`, `shortestLine` and
`bearing` are `NULL`. An empty geometry has no bounding box, so `stbox`,
`expandSpace`, `stboxes`, `splitNStboxes` and `splitEachNStboxes` are `NULL`.
A measure of the empty geometry itself is the measure of the empty set:
`length` and `perimeter` are 0.

### 3.3 It removes nothing and keeps nothing

The restrictions `atGeometry` and `atValue` are `NULL`, since no instant is
left, and `minusGeometry` and `minusValue` return the temporal value
unchanged.

### 3.4 It cannot be the position of a value

Building a temporal value or a set from an empty geometry, or giving it as the
origin of `rotate` and `scale`, raises an error, since a value must be
somewhere at each of its instants.

## 4. The answer in the C API

A C function answers the empty set with the value its SQL wrapper turns into
the SQL answer, and leaves `meos_errno()` at 0. A binding tells the empty set
from an error by that code, never by the value alone, since the same value is
the error return of the function.

| answer | C function | C answer, `meos_errno() == 0` |
|---|---|---|
| a relationship, ever or always | `eintersects_tgeo_geo`, `adisjoint_tgeo_geo`, ... | `0` or `1` |
| a temporal relationship | `tintersects_tgeo_geo`, `tdisjoint_tgeo_geo`, ... | the constant temporal Boolean over the time of the value |
| a distance to the nearest approach | `nad_tgeo_geo` | `DBL_MAX` |
| a temporal distance, a nearest instant, a shortest line | `tdistance_tgeo_geo`, `nai_tgeo_geo`, `shortestline_tgeo_geo` | `NULL` |
| a bearing | `bearing_point_point` | `false`, no result written |
| a box | `geo_to_stbox` | `NULL` |
| a restriction | `tgeo_at_geom`, `tgeo_minus_geom` | `NULL` for `at`, a copy of the value for `minus` |
| a constructor | `tgeoinst_make`, ... | `NULL` with `meos_errno()` set, the one error of this specification |

## 5. Versioning and stability

This document states the behaviour of MobilityDB and MEOS 1.3. A change of
the answer to an empty geometry is a change of behaviour, stated in the
release notes and in the section *Empty Geometries* of the manual, which this
document and the manual state alike.

## 6. Out of scope

What an operation answers for a geometry that is not empty is stated by its
entry in the manual. The numbers a geometry may hold are stated by
`meos-finite-values.md`, which refuses a NaN or an infinite coordinate at
input; an empty geometry holds no coordinate and is not concerned.

## 7. References

- `doc/temporal_spatial_p2.xml` and its Spanish translation: the section
  *Empty Geometries* of the manual.
- `meos-finite-values.md`: the numbers a value holds.
- `meos-exact-geometry.md`: what the native geometry engine answers for a
  geometry that is not empty.
- PostGIS, *Spatial Relationships* and *Measurement Functions*: the answers of
  the empty set this document follows.

## 8. Consumers

Every binding generated from MEOS reads the answers of section 4: the
PostgreSQL extension MobilityDB, MobilityDuck, the JVM bindings and the
others. A binding that turns a C answer into a host value maps the value and
`meos_errno()` together, so that `DBL_MAX` with no error becomes the host's
missing value and `DBL_MAX` with an error raises.

## 9. References to the implementation

The functions named in section 4 hold the test of an empty geometry after
the validation of their arguments, in the files listed at the head of this
document. A function whose answer to an empty geometry differs from this
document is a defect of the function, reported on the MobilityDB issue
tracker.
