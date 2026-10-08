<!--
  MobilityDB — Portable Naming: The native geometry operations under their plain names
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The native geometry operations under their plain names

[Back to the index](00-INDEX.md)

MEOS answers most PostGIS geometry operations natively, without GEOS. This document states which,
under which name each reaches PostgreSQL, Spark and Flink, and what each decision requires of the
lifting infrastructure, which extends every base operation to the temporal types and to the sets.
Every count below is measured on MobilityDB master `d29d4ab293` against PostGIS 3.6.3, by
`inventory.py`, which reads the tree through `tools/scripts/check_csqlfn.py`, and by the regression
query `049_geo_equality.test.sql`.

## What MEOS answers natively

`meos/include/meos_geo.h` declares 95 public functions over a static geometry or geography. 94 are
native: their body reaches neither a `GEOS*` call nor a liblwgeom entry point that
`tools/scripts/liblwgeom_geos_baseline.txt` lists as answered by GEOS. The one exception is
`geom_unary_union` (`ST_UnaryUnion`), which calls `GEOSUnaryUnion_r`.

The spatial relationships answer through `geom_spatialrel`, which reads `meos_point_in_polygon`
and the native relate engine (`relate_ctx_borrow_pair`); `geom_intersects2d` and
`geom_disjoint2d` go through it. The set operations `difference`, `intersection`, `geoUnion` and
`symDifference` answer through Clipper2 (`clip_ext`).

| State | Operations |
|---|---|
| Reached by a plain name over MEOS | `buffer`, `convexHull`, `isSimple`, `orientedEnvelope`, `relate(geometry, geometry)`, `difference`, `intersection`, `geoUnion`, `symDifference`, `length`, the affine family, `round`, `transformPipeline` |
| Reached by a plain name over PostGIS | `transform(geometry, integer)` and `asEWKT(geometry)`, SQL functions calling `ST_Transform` and `ST_AsEWKT`, while MEOS has `geo_transform` and `geo_as_ewkt` |
| Native in MEOS, no plain name | the relationships `contains`, `covers`, `disjoint`, `intersects`, `touches`, `dwithin`, `equals`, `relate(geometry, geometry, text)`; `distance`, `shortestLine`, `maxDistance`; `area`, `perimeter`, `centroid`; `boundary`, `reverse`, `numGeometries`, `geometryN`, `numPoints`, `points`; `lineInterpolatePoint`, `lineSubstring`, `lineLocatePoint`; `collect`, `makeLine`; `asText`, `asEWKB`, `asGeoJSON`; `clusterKMeans`, `clusterDBSCAN`, `clusterIntersecting`, `clusterWithin` |

PostGIS 3.6.3 declares none of these plain names; each is `ST_` followed by the name. MobilityDB
master already declares `distance`, `shortestLine`, `area`, `perimeter`, `centroid`, `points`,
`asText`, `asEWKB`, `asEWKT` and `transform` over its own types; the other names are new.

## The rule

1. **An operation MEOS answers natively reaches PostgreSQL as `X(geo, ...)`**, over the MEOS
   wrapper, as `relate` and `buffer` do (decision 0.5): the plain name is how a query reaches the
   engine without GEOS, where `ST_X` reaches PostGIS.
2. **Spark and Flink call it `geoX`**, the `@altsqlfn` of the same wrapper (rules 1, 4 and 5):
   `contains` is a Spark function and a word Flink's parser refuses, and the group moves with it.
3. **Two layers, one meaning per name.** The MEOS algebra answers a finite subset of a value
   domain with its set type: `getValues(tgeompoint)` is a `geomset`, `points(tpose)` is a
   `geomset`. A PostGIS equivalent keeps PostGIS's meaning and the shape of its answer. Where a
   name of the algebra has another shape, the PostGIS equivalent takes `geoX` in every engine,
   PostgreSQL included: `points(geometry)` is the `geomset` of its distinct points and
   `geoPoints(geometry)` the MultiPoint of `ST_Points`, its vertices in order with their repeats.
   `points` is the only such name in the table above.
4. **A base operation is the one its lifts apply.** Whatever a base operation answers, its temporal
   lift answers at each instant and its set lift for each element, so a decision at the base is
   checked against the kernels the lifts call (`tgeo_spatialrels.c`, `tgeo_spatialfuncs.c`,
   `tgeo_distance.c`, `type_util.c`).

## Decisions

| # | Question | Decided |
|---|---|---|
| 1 | `equals(geometry, geometry)`? | yes, over `geo_equals`, the point-set equality read from the DE-9IM matrix (`ST_Equals`), as `geoEquals` in every engine, PostgreSQL included: PostGIS 3.6.3 declares `equals(geometry, geometry)` as a deprecated alias of `ST_Equals`, so the name is taken as `geoUnion` takes the name the keyword `union` holds; beside the structural `=` |
| 2 | The shape of the clustering functions | MEOS's array shape in every engine: `clusterKMeans(geometry[], k)` and `clusterDBSCAN(geometry[], eps, minpoints)` return `integer[]`, `clusterIntersecting(geometry[])` and `clusterWithin(geometry[], d)` return `geometry[]`; a PostgreSQL window form can be added later without a rename |
| 3 | `points(geometry)` | the two-layer rule: `points(geometry)` returns a `geomset`, `geoPoints(geometry)` the MultiPoint of `ST_Points` |
| 4 | From a set of points back to a geometry | `geometry(geomset)` and `geography(geogset)` over a new MEOS `geoset_to_geo` (`geoset_values`, then `geo_collect_garray`), through an explicit `CREATE CAST`; the shape follows `ST_Collect` from the element types alone, a one-element point set giving a MULTIPOINT |
| 5 | The dimension of a relationship or a measure | 3D only when both values have Z, as the lifts decide it (`geo_dwithin_fn`: "3D only if both arguments are 3D") and as `length(geometry)` (`LWGEOM_length_linestring`, `ST_3DLength`) already measures; no `…3D` names |
| 6 | `collect` and `makeLine` | array forms only, as decision 2: `collect(geometry[])`, `makeLine(geometry[])`; `setUnion(geometry)` stays the aggregate of the distinct values |
| 7 | The earth model of a geography operation | every geography operation and its lifts take `spheroid boolean DEFAULT true`, as `area(stbox, spheroid)` names it; `length(geography)` included; `intersects(geography, geography)` mirrors the flagless `ST_Intersects` |
| 8 | How the earth model reaches the lifts | as an `lfinfo` parameter: the two operands are the arguments of the lift and `param[]` carries the rest, as the `tjsonb` lifts carry up to four; `dwithin` carries the distance in `param[0]` and the model in `param[1]` |
| 9 | Which equality the lifting kernel uses | structural, for geometry as for geography: a lift applies the base operation at every instant, the base `=` of two geometries is structural (PostGIS's `=`, and MobilityDB's `cmp` and `hash`), so the temporal `=`, the ever and always equalities, the sets and the normalization of a sequence are structural; the point-set equality is `equals` (decision 1) |
| 10 | The distance operators over geography | an operator uses the default and the function switches, as `->` and `jsonbsetObjectField(jsonbset, text, null_handle text DEFAULT 'use_json_null')` (`doc/temporal_jsonb.xml`): `tDistance` and `nearestApproachDistance` over geography take `spheroid boolean DEFAULT true`, and `<->` and `\|=\|` over geography call the two-argument procedures `tDistanceOp` and `nearestApproachDistanceOp`, as `->` calls `jsonbsetObjectFieldOpr`, the suffix `Op` naming the procedure after its `@sqlop` tag; the operators answer on the spheroid, where PostGIS's geography `<->` answers on the sphere to agree with its index ([Geodetic boxes](GEODETIC-BOXES.md) states what the index-ordered search needs) |

## What the lifting infrastructure requires

**The dimension.** The relationship lifts accept operands of different dimensions and choose the
3D kernel only when both have Z (`geo_dwithin_fn`, `geo_intersects_fn`, `geo_disjoint_fn`). The
distance and shortest line lifts refuse operands of different dimensions
(`ensure_same_dimensionality_tspatial_geo`, `ensure_same_dimensionality`) and then choose by the Z
of either (`tgeo_spatialfuncs.c`, `tgeo_distance.c`). A base operation follows the lifts of its
family: a relationship accepts mixed operands, a distance or a shortest line refuses them. Two
public base functions disagree with their lifts:
- `geom_dwithin` reads `FLAGS_GET_Z(gs1->gflags) ? geom_dwithin2d(...) : geom_dwithin3d(...)`:
  the first operand alone decides, and in the inverted sense;
- `geom_intersects` is `geom_intersects2d` whatever the dimension.

Both follow the lifts' rule; `geom_disjoint`, `geom_distance` and `geom_shortestline` are added
with the same rule, the lifts having `datum_geom_disjoint3d`, `datum_geom_distance3d` and
`geom_shortestline3d` where the bare base has none.

**The containment family.** `eContains`, `eCovers` and `eTouches` refuse a value with Z
(`ensure_has_not_Z_geo`): no 3D kernel exists for them. `geom_contains`, `geom_covers` and
`geom_touches` answer such a value on its projection. The base refuses Z as its lifts do.

**The earth model.** Every geography kernel the lifts call passes the spheroid
(`datum_geog_intersects`, `datum_geog_disjoint`, `datum_geog_dwithin`, `geog_distance`,
`geog_length` in the length of a sequence) but one: `datum2_geog_centroid` passes the sphere.
`geog_distance` sets `use_spheroid = true` in its body where its siblings take it as an argument.
The lifting engine passes up to `MAX_PARAMS` (5) parameters to a unary lift (`lfunc_base`) only
in a build with the JSON family, and at most one to a binary lift (`tfunc_base_base`). A geography
dwithin reaches its kernel through the binary lift and through the direct `func(v1, v2, dist)`
calls of the temporal dwithin (`tdwithin_tspatial_spatial`, `tdwithin_tspatial_tspatial`) and of
its turning point (`tpointsegm_tdwithin_turnpt`). The four ever/always paths that call a kernel
as `func3(a, b, param)` never reach a geography kernel: `spatialrel_tgeo_tgeo` serves the
containment family, which refuses geodetic values, a circular buffer is planar
(`spatialrel_geo_geo_simple`), and `spatialrel_trgeo_trav_geo` has no caller.

**The geography relationships.** The lifts answer `disjoint` over geography
(`datum_geog_disjoint`) where MEOS has no public `geog_disjoint`.

**The sets.** `transform(geomset)` and `transformPipeline(geomset)` answer through MEOS while
`transform(geometry)` answers through PostGIS: the base and its set lift run on two engines.

**The equality.** `datum_eq` (`meos/src/temporal/type_util.c`), the equality of the temporal
values, the sets and the normalization of the sequences, compares two geometries with
`geo_equals`, the point-set equality, and two geographies with `geo_same`, the structural one.
`049_geo_equality.test.sql` compares `Linestring(0 0,2 0)` with `Linestring(0 0,1 0,2 0)`:

| | geometry | geography |
|---|---|---|
| base `=` (PostGIS) | false | false |
| temporal `=` | **true** | false |
| temporal `cmp` | −1 | −1 |
| temporal `hash` equal | false | false |
| `eEqual(temporal, base)` | **true** | false |
| set `=` | **true** | |
| `numValues` of a set holding both | **1** | |
| `numInstants` of `Interp=Step;[L1@t1, L2@t2, L1@t3]` | **2** | |
| `SELECT DISTINCT` over the two temporal values | **2 by hash, 1 by sort** | |

For geometry `=` answers true where `cmp` answers −1 and the hashes differ, so the answer of a
query depends on its plan, a set keeps one of two different lines, and the normalization of a
sequence drops a change of value. Geography answers structurally throughout.

## The plan

One pull request, "Answer the native geometry operations under their plain names", on the branch
`geo/native-operations-plain-names`, of these commits in this order, each with its SQL, its `@sqlfn` and `@altsqlfn` tags, its regression tests
against the `ST_` function of PostGIS on the same inputs, and its manual entries in English and
Spanish:

1. **The lifting kernel's equality** (decision 9): `datum_eq` compares two geometries
   structurally, as two geographies, exactly (as the hash reads them); `049_geo_equality.test.sql`
   answers structurally throughout: the temporal `=` false, one plan for `DISTINCT`, two values in
   the set, three instants in the sequence.
2. **The dimension of the base relationships and measures** (decision 5): `geom_dwithin`,
   `geom_intersects` on the lifts' rule; `geom_disjoint` accepting mixed operands as the
   relationships do, `geom_distance` and `geom_shortestline` refusing them as the distances do;
   the turning points of the temporal dwithin (`tpointsegm_tdwithin_turnpt`) on the same rule.
3. **The binary lifts carry their parameters** (decision 8): `tfunc_base_base` and `lfunc_base`
   dispatch `numparam` up to `MAX_PARAMS` in every build.
4. **The earth model** (decisions 7 and 10): every MEOS function measuring a geography takes the
   spheroid last -- the temporal distance, the nearest approach distance and instant, the shortest
   line, the ever, always and temporal dwithin with their array forms, `mindistance_tgeoarr_tgeoarr`,
   the length, cumulative length and speed of a temporal point, the centroid of a temporal
   geography, `geo_length`, the geo set distances and the five similarity distances -- and
   `geog_disjoint` joins `geog_intersects`; the geography overloads in SQL take `spheroid boolean
   DEFAULT true`, each wrapper reading it as `bool spheroid = true; if (PG_NARGS() > k) spheroid =
   PG_GETARG_BOOL(k);` and calling the public function by name, the form from which the MEOS-API
   catalog derives the value a signature omitting it passes; `<->`, `|=|` and the set `<->` over
   geography call `tDistanceOp`, `nearestApproachDistanceOp` and `setDistanceOp`. A temporal
   geography locates the closest points of each segment on the sphere and compares them on the
   model of the earth, as `lwgeom_distance_spheroid` measures a geography, and `shortestLine` joins
   the geography from the value at that instant, so the line measures the nearest approach
   distance.
5. **The relationships**: `contains`, `covers`, `disjoint`, `intersects`, `touches`, `dwithin`,
   `relate(geometry, geometry, text)` and `geoEquals` over geometry, `intersects`, `disjoint` and
   `dwithin` over geography, the containment family refusing Z, Spark and Flink taking `geoX`. The
   six static relationships of two circular buffers are `cbufferContains` ... `cbufferDwithin` in
   every engine, the name MobilityDB #2988 gives them as every base type of MobilityDB prefixes its
   own static functions with its class; a function over `geometry` or `geography`, a PostGIS type,
   takes the plain name in PostgreSQL, as `relate`, `convexHull` and `isSimple` do.
6. **The distances and measures**: `distance`, `shortestLine`, `maxDistance`, `area`, `perimeter`,
   `centroid` over geometry and geography, under the same names in Spark and Flink, which define
   and reserve none of the six (the name probe of both engines: of the names of commits 6 to 10,
   Spark defines `reverse` and `transform` and Flink `reverse` and `collect`, and neither parser
   refuses any).
7. **The accessors and the lines**: `boundary`, `reverse`, `numGeometries`, `geometryN`,
   `numPoints`, `points` (`geomset`), `geoPoints`, `lineInterpolatePoint`, `lineSubstring`,
   `lineLocatePoint`.
8. **The constructors and the casts**: `collect(geometry[])`, `makeLine(geometry[])`,
   `geometry(geomset)`, `geography(geogset)` (decision 4).
9. **The output and the transformation**: `asText`, `asEWKB`, `asGeoJSON` over geometry and
   geography beside `asEWKT`, as SQL functions calling `ST_AsText`, `ST_AsEWKB` and
   `ST_AsGeoJSON` in PostgreSQL, the PostGIS path being the faster one there (`asEWKT` over MEOS
   measured 12 percent slower, `transform` over a geometry 5 percent); the MEOS writers carry
   these names as their own `@sqlfn`, the form of a surface PostgreSQL registers through a host
   extension, so Spark, Flink and DuckDB publish them over MEOS. `transform` over a geography
   calls MEOS (`Geo_transform`, Spark and Flink `geoTransform`), 1.7 times faster than the cast
   through `ST_Transform`, and MEOS transforms a geography only into a lon/lat system. It rests on
   PR #2993 (merged): in the MEOS library, `geo_as_geojson` and `temporal_as_mfjson` name the
   reference system of the SRID of the value when the caller names none, through the internal
   `srid_srs`, which reads it from `spatial_ref_sys.csv` into the PROJ cache; the PostgreSQL
   extension keeps the name it reads from the table `spatial_ref_sys`. The library reads the CSV
   from the directory its build installs it into, branch `fix/meos-data-files-from-install-dir`,
   so a JVM binding loading an installed MEOS finds it under any prefix, as MobilityDuck finds the
   copy it embeds; Spark, Flink and DuckDB then advance their pin.
10. **The clustering** (decision 2).
11. **The measures read in place**: `area`, `length` and `perimeter` of a geometry holding no
    curve read on its serialized form, a polygon directly and any other geometry by a walk of the
    form, each ring by the terms of `ptarray_signed_area` and `ptarray_length_2d` in their order.
12. **The point location read in place**: a point against a polygon or a multipolygon located on
    the serialized rings, for `contains`, `covers`, `intersects` and `touches`.
13. **The point distance read in place**: the distance of a point to a line or a polygon read on
    the serialized form of the line or the polygon.

The portable dialect chapter (`doc/portable_sql.xml`) lists each `X` and `geoX` as the PR lands
them. `geom_unary_union` stays outside the rule until MEOS answers it natively.

**State.** Commits 1 to 13 are on the branch, head `493a6a512a` on master `e30ef41657`, with the
strict-ci, cppcheck, smoke, Windows, CGAL oracle and GEOS speed receipts of the head. Commit 1:
`datum_eq` compares two geometries and two geographies exactly, and `049_geo_equality.test.sql`
answers structurally throughout. Commit 2: `geom_dwithin`, `geom_intersects` and the new
`geom_disjoint` measure in 3D only when both geometries have Z, the new `geom_distance` and
`geom_shortestline` refuse a 3D and a 2D geometry, the turning points of the temporal dwithin
follow the same rule, and `datum_eq` answers that a 3D and a 2D point are not equal, so a 3D and a
2D temporal point answer the same in either order and two parallel 3D points are measured in 3D.
Commit 3: `lfunc_base` and `tfunc_base_base` pass up to five parameters in every build. Commit 4
(`cdd4d975c6`): the earth model as item 4 states it, every geography overload tested on the
spheroid and on the sphere (one degree of meridian at the equator reads 110574.389 m and
111195.08 m), the manual in English and Spanish with the notation `tgeog`; the nearest approach
walk builds the circle tree of the geography once, seeds its traversal with the running minimum and
measures on the spheroid only an edge that can beat it, and `shortestLine` over a temporal
geography point of 2000 instants takes 0.58, 0.62 and 0.98 of the time of the trajectory path.
Commit 5 (`2984acf092`): the relationships as item 5 states them, 121 ordered pairs of eleven
geometries answering as `ST_Contains`, `ST_Covers`, `ST_Disjoint`, `ST_Intersects`, `ST_Touches`,
`ST_Equals`, `ST_DWithin` and `ST_Relate` on every pair, each relationship holding for 11 to 72
of them, the manual in English and Spanish and the `geoX` row of the portable dialect chapter.
Commit 6 (`559dd09750`): the measures and distances as item 6 states them over the new MEOS
`geo_area`, `geo_perimeter`, `geo_centroid`, `geo_distance`, `geo_shortestline`,
`geom_max_distance`, `geom_max_distance3d` and `geog_shortestline`, each answering as its PostGIS
function on the eleven geometries and their 121 pairs; on the sphere the area of a geography is
the one its great circles bound (the square of one degree at the equator 12364031798.518 square
meters, its spherical excess 12364031798.470), where `ST_Area` over a geography on the sphere
answers 0.67 percent less. Commit 7 (`d1fa001292`): `boundary`, `reverse` (Spark and Flink
`geoReverse`), `numGeometries`, `geometryN`, `numPoints`, `lineInterpolatePoint`,
`lineSubstring` and `lineLocatePoint` over a geometry, each answering as its PostGIS function on
the eleven geometries and on three lines, the boundary of an empty geometry being the empty
geometry of the dimension of a boundary. `points(geometry)` and `geoPoints(geometry)` wait on the
name of their MEOS functions: the public `geo_points` answers the MultiPoint of `ST_Points`, the
meaning of `geoPoints`, while its siblings `tpose_points`, `tcbuffer_points` and
`trgeometry_points` answer the set of the distinct points, the meaning of `points`.
Commit 8 (`0f53433146`): `collect(geometry[])` (Spark and Flink `geoCollect`) and
`makeLine(geometry[])` over `geo_collect_garray` and `geo_makeline_garray`, and the casts
`geomset::geometry` and `geogset::geography` over the new MEOS `geoset_to_geo`, each answering as
`ST_Collect` and `ST_MakeLine` on seven arrays; `geo_collect_garray` answers an array of one
element with its collection of one element, as `ST_Collect` does, the internal `geoarr_collect`
keeping the single value of a trajectory, and `geo_makeline_garray` frees every geometry it read
when the SRIDs differ and raises an array without a point or a line as a notice.
Commit 9 (`5281445d67`, on master `d76ba94472`): the output names and the transformation as
item 9 states them, each writer answering as its PostGIS function on fifteen geometries and four
geographies, `transform` byte for byte as `ST_Transform` on eight geometries and a geography, and
`meos/test/geo_transform_test.c` stating the refusals of a geography into a projected system and
of a byte order no decoder reads.
Commit 10 (`09d6828462`): `clusterKMeans`, `clusterDBSCAN`, `clusterIntersecting` and
`clusterWithin` over arrays of geometries as decision 2 states them, answering as
`ST_ClusterKMeans` and `ST_ClusterDBSCAN` over the rows in the order of the array (an empty
geometry in the cluster -1 of k-means, a noise point NULL in DBSCAN) and as the aggregates
`ST_ClusterIntersecting` and `ST_ClusterWithin`.
Commits 11 to 13: the measures, the point location and the point distance read on the serialized
form, bit for bit `ST_Area`, `ST_Perimeter`, `ST_Length` (`ST_3DLength` with Z), `ST_Contains`,
`ST_Covers`, `ST_Intersects`, `ST_Touches` and `ST_Distance`. The CGAL judge `cgal_f12_judge`
agrees with every answer over the AIS and geo fixtures: 270239 point and polygon pairs located
exactly, 280091 point and line or polygon distances and 9695 areas, lengths and perimeters within
the error bound of the PostGIS formula. Against GEOS 3.14.1 called directly, under callgrind, the
native answer runs 0.51 to 0.56 of the instructions of GEOS for the point location, 0.54 for the
distance, 0.97 for the area, 0.80 for the perimeter and 0.33 for the length.
