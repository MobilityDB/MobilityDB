<!--
  MobilityDB — Exactness: The geodetic operations
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The geodetic operations

[Back to the index](00-INDEX.md) · [Geodetic boxes](GEODETIC-BOXES.md)

Which operations a temporal geography (`tgeography`, `tgeogpoint`) supports, and how exactly, is
decided by the three rules below, by the meaning of the operation on the WGS 84 ellipsoid and never
by the signatures that happen to exist. The manual states them in the section "Planar and Geodetic
Computations" of the chapter on temporal geometry types (`doc/temporal_spatial_p2.xml`,
`tgeo_geodetic`). The tasks are the places where MobilityDB departs from them, measured on master
`e8ac3ee523`.

## The rules

1. **An operation is available for geographies when it is defined on the ellipsoid.** The metric
   operations are (distance, distance within, length, area, perimeter, azimuth, bearing,
   centroid), computed on the spheroid, or on the sphere when `spheroid` is false. So are the
   intersection and the disjointness, since two geographies intersect when their distance is zero.
   `contains`, `covers`, `touches` and `relate`, defined by the planar DE-9IM matrix, are not, and
   neither are the planar constructions (union, intersection, difference, buffer, convex hull):
   PostGIS defines none of them for geographies. They raise an error, which is correct behaviour.
2. **The ever, always and temporal versions of an available operation are computed exactly.** A
   `tgeography`, and a `tgeogpoint` with discrete or step interpolation, is constant between two
   instants, so the operation is evaluated at each instant. A `tgeogpoint` with linear
   interpolation moves along a great circle, and a temporal relationship changes at the instants
   at which its geodetic distance to the other argument crosses the given distance, which needs a
   geodesic crossing kernel. Sampling the instants instead is wrong: it misses a crossing whose
   two endpoints lie outside (#1348).
3. **A temporal relationship available for geographies accepts a geography or a temporal
   geography on either side**, as its planar version accepts a geometry or a temporal geometry. A
   case that rules 1 and 2 admit and that has no SQL signature is a defect of the surface, corrected
   in the generator manifest `tools/codegen/inherited/manifest.d/tempspatialrel_families.yaml`.

## The witnesses

Every answer is judged on the raw values, never against MobilityDB itself. The geodetic answers
are judged against PostGIS geography (`ST_Intersects` and `ST_DWithin` on `geography`, computed on
the spheroid); the planar answers of GO1, whose values are polygons and points with straight edges
only, against `ST_Intersects` on `geometry`. The moving-point cases of GO3 are built so that the
truth is known in closed form: a point moving along the equator from `(0 0)` to `(2 0)` between
January 1 and 3 is at `(1 0)` at January 2, 0:00, and nowhere else. The data are the 2024
Atlantic storms of Chapter 3 of the book, built by the pipeline of the companion repository
(`chapter3/storms/`): 42 wind fields as `tgeometry` in SRID 4326 with step interpolation, 1,229
instants.

## The tasks

| # | Defect | Rule | Measured | State |
|---|---|---|---|---|
| GO1 | `tIntersects` and `tDisjoint` between two temporal geometries or geographies compute the temporal equality of their values (`tinterrel_tspatial_tspatial` calls `tcomp_temporal_temporal` with `datum2_eq`), which is their intersection only for two points. A square containing a point answers false. The families that delegate to `tIntersects(tgeometry, tgeometry)` by a cast (h3, quadbin, pose, rgeo) inherit it | 2 | `tIntersects(tgeometry '[Polygon((0 0,2 0,2 2,0 2,0 0))@2001-01-01, Point(3 3)@2001-01-02]', tgeometry '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02]')` = `[f@2001-01-01, f@2001-01-02]`, while `eIntersects` = `t`. After the change: `[t@2001-01-01, f@2001-01-02]`; 4 pairs of storms and 11 pairs of wind fields of one storm, 0 instants differing from `ST_Intersects`, `tDisjoint` the exact complement. Two expected outputs change with it, both to the answer PostGIS gives: `tDisjoint` of two adjacent H3 cells (`264_th3index_tempspatialrels`) and of two intersecting quadbin tiles (`364_tquadbin_tempspatialrels`) is false, where the equality answered true | merged as #3074 (`67b3bd353c`) |
| GO2 | `tIntersects`, `tDisjoint` and `tDwithin` of a `tgeography` with a `geography` have no SQL signature, nor `tDwithin` between two `tgeography`, although `eIntersects`, `aIntersects` and `eDwithin` have them and MEOS computes them (`Tintersects_tgeo_geo`, `Tdisjoint_tgeo_geo`, `Tdwithin_tgeo_geo`, `Tdwithin_tgeo_tgeo`) | 3 | Bound to the C entries, 168 wind fields × cities for intersects and disjoint, 126 for dwithin, 4 pairs of storms for dwithin between two `tgeography`: 0 instants differing from PostGIS geography. Eight signatures added by the manifest | merged as #3074 (`67b3bd353c`) |
| GO3 | A `tgeogpoint` with linear interpolation at distance 0 from a geography point, or from another `tgeogpoint`, is true one microsecond before the instant at which they meet (`tdwithin_tgeogpoint_geogpoint` finds a degenerate crossing) | 2 | `tDwithin(tgeogpoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]', geography 'Point(1 0)', 0)` = `{[f@…, t@2001-01-01 23:59:59.999999], (f@…, t@2001-01-02], …}`. On the branch, the kernel answers the temporal equality at distance 0, `{[f@2001-01-01, t@2001-01-02], (f@2001-01-02, f@2001-01-03]}`, the closed-form truth | merged as #3074 (`67b3bd353c`) |
| GO4 | `tIntersects` and `tDisjoint` of a `tgeogpoint` with a `geography` have no SQL signature, although `tDwithin(tgeogpoint, geography)` has one with the same restriction (rule 2: a geography point) | 3 | the manifest declares `(tgeogpoint, tgeogpoint)` only for both | open |
| GO5 | A `tgeogpoint` with discrete or step interpolation against a geography line or polygon is refused (`tdwithin_tgeo_geo`: a temporal geography point against a geography other than a point), although rule 2 computes it exactly at each instant | 2 | `Tintersects_tgeo_geo` on `tgeogpoint 'Interp=Step;[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]'` and a square: `ERROR: Only planar coordinates supported` | open |
| GO6 | A `tgeogpoint` with linear interpolation against a geography line or polygon has no geodesic crossing kernel, for `tIntersects`, `tDisjoint` and `tDwithin` (issue #1087) | 2 | `tDwithin(tgeogpoint '[Point(0 0)@2001-01-01, Point(2 0)@2001-01-03]', geography 'Polygon((0.5 -1,1.5 -1,1.5 1,0.5 1,0.5 -1))', 0)`: `ERROR: Only planar coordinates supported` | open: a kernel for the instants at which a great circle segment crosses a geodesic edge |
| GO7 | `traversedArea(tgeography, true)` raises an error, although the manual marks `traversedArea` as supporting geographies; without the second argument the values are only collected | 1 | `ERROR: Only planar coordinates supported` (`geom_unary_union` refuses a geodetic value) | open: a geodesic union, or the manual states the refusal |
| GO8 | `stops` computes a planar rectangle on degrees and measures it with a geodesic ruler | 1 | recorded on master `204b26b188` | open |
| GO9 | `geom_array_union` hands a geodetic array of lines or polygons to a planar engine | 1 | recorded on master `204b26b188`; an array of points is answered natively (#2275) | open: a geodesic union or a refusal |

## Found on the way, not geodetic

| # | Defect | Measured | State |
|---|---|---|---|
| GO10 | The type modifier of a column `tgeometry(SequenceSet, Geometry, 4326)` is displayed without its SRID, `tgeometry(SequenceSet)`, although the SRID is enforced; `tgeometry(SequenceSet, MultiPolygon, 4326)` displays all three | `format_type` of the column; inserting a value in SRID 3857 is refused | open |
| GO11 | `traversedArea(temp, true)` of the 34-knot wind field of Isaac (AL102024) returns an invalid geometry, two nested shells, where the union of its values is one polygon; the first three polygons of the wind field reproduce it, no pair of them does. The dissolve is MEOS's `meos_areal_union` (`geom_unary_union`); the values have straight edges only | `ST_IsValidReason` = `Nested shells[-58.6000023 38.6000023]`; area 6.97 square degrees against 5.61 for `ST_UnaryUnion` of the same polygons | open |
| GO12 | Withdrawn: not a defect. The measurement called `traversedArea` of the hull of FICARIA SEAWAYS turning in Göteborg (built as `chapter4/ais_poses.sql` builds it from the AIS day 2026-01-20, MMSI 220464000, A=134, B=97, C=15, D=15) with its default argument `false`, which by design answers the placements as a collection (97 overlapping polygons, invalid as a MultiPolygon), not the swept region | `traversedArea(temp, true)` over [18:39, 18:45] answers one valid polygon of 93,499 m²; the union of the convex hulls of the placements one second apart covers 93,383 m², of which 5 m² lie outside the MEOS answer | withdrawn |
