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
| 1 | `equals(geometry, geometry)`? | yes, over `geo_equals`, the point-set equality read from the DE-9IM matrix (`ST_Equals`); Spark and Flink `geoEquals`; beside the structural `=` |
| 2 | The shape of the clustering functions | MEOS's array shape in every engine: `clusterKMeans(geometry[], k)` and `clusterDBSCAN(geometry[], eps, minpoints)` return `integer[]`, `clusterIntersecting(geometry[])` and `clusterWithin(geometry[], d)` return `geometry[]`; a PostgreSQL window form can be added later without a rename |
| 3 | `points(geometry)` | the two-layer rule: `points(geometry)` returns a `geomset`, `geoPoints(geometry)` the MultiPoint of `ST_Points` |
| 4 | From a set of points back to a geometry | `geometry(geomset)` and `geography(geogset)` over a new MEOS `geoset_to_geo` (`geoset_values`, then `geo_collect_garray`), through an explicit `CREATE CAST`; the shape follows `ST_Collect` from the element types alone, a one-element point set giving a MULTIPOINT |
| 5 | The dimension of a relationship or a measure | 3D only when both values have Z, as the lifts decide it (`geo_dwithin_fn`: "3D only if both arguments are 3D") and as `length(geometry)` (`LWGEOM_length_linestring`, `ST_3DLength`) already measures; no `…3D` names |
| 6 | `collect` and `makeLine` | array forms only, as decision 2: `collect(geometry[])`, `makeLine(geometry[])`; `setUnion(geometry)` stays the aggregate of the distinct values |
| 7 | The earth model of a geography operation | every geography operation and its lifts take `spheroid boolean DEFAULT true`, as `area(stbox, spheroid)` names it; `length(geography)` included; `intersects(geography, geography)` mirrors the flagless `ST_Intersects` |
| 8 | How the earth model reaches the lifts | as an `lfinfo` parameter: the two operands are the arguments of the lift and `param[]` carries the rest, as the `tjsonb` lifts carry up to four; `dwithin` carries the distance in `param[0]` and the model in `param[1]` |
| 9 | Which equality the lifting kernel uses | structural, for geometry as for geography: a lift applies the base operation at every instant, the base `=` of two geometries is structural (PostGIS's `=`, and MobilityDB's `cmp` and `hash`), so the temporal `=`, the ever and always equalities, the sets and the normalization of a sequence are structural; the point-set equality is `equals` (decision 1) |

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
The lifting engine passes up to `MAX_PARAMS` (5) parameters to a unary lift (`lfunc_base`) but at
most one to a binary lift: `tfunc_base_base` ends in `else /* if (lfinfo->numparam == 1) */`, and
four ever/always paths call the kernel directly as `func3(a, b, param)`
(`tgeo_spatialrels.c` twice, `tcbuffer_spatialrels.c`, `trgeo_spatialrels.c`).

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

One pull request, "Answer the native geometry operations under their plain names", of these
commits in this order, each with its SQL, its `@sqlfn` and `@altsqlfn` tags, its regression tests
against the `ST_` function of PostGIS on the same inputs, and its manual entries in English and
Spanish:

1. **The lifting kernel's equality** (decision 9): `datum_eq` compares two geometries
   structurally, as two geographies, exactly (as the hash reads them); `049_geo_equality.test.sql`
   answers structurally throughout: the temporal `=` false, one plan for `DISTINCT`, two values in
   the set, three instants in the sequence.
2. **The dimension of the base relationships and measures** (decision 5): `geom_dwithin`,
   `geom_intersects` on the lifts' rule; `geom_disjoint` accepting mixed operands as the
   relationships do, `geom_distance` and `geom_shortestline` refusing them as the distances do.
3. **The binary lifts carry their parameters** (decision 8): `tfunc_base_base` and the four direct
   calls dispatch `numparam` up to `MAX_PARAMS`.
4. **The earth model** (decision 7): the geography kernels take the spheroid last, `geog_distance`
   as an argument, `geog_disjoint`, the centroid lift on the spheroid; the geography operations and
   their lifts in SQL take `spheroid boolean DEFAULT true`.
5. **The relationships**: `contains`, `covers`, `disjoint`, `intersects`, `touches`, `dwithin`,
   `equals`, `relate(geometry, geometry, text)` over geometry, `intersects`, `disjoint`, `dwithin`
   over geography, the containment family refusing Z; the six `cbuffer_*` relationships as
   `contains` ... `dwithin` over `cbuffer` (Spark and Flink `cbufferContains` ...).
6. **The distances and measures**: `distance`, `shortestLine`, `maxDistance`, `area`, `perimeter`,
   `centroid` over geometry and geography.
7. **The accessors and the lines**: `boundary`, `reverse`, `numGeometries`, `geometryN`,
   `numPoints`, `points` (`geomset`), `geoPoints`, `lineInterpolatePoint`, `lineSubstring`,
   `lineLocatePoint`.
8. **The constructors and the casts**: `collect(geometry[])`, `makeLine(geometry[])`,
   `geometry(geomset)`, `geography(geogset)` (decision 4).
9. **The output and the transformation over MEOS**: `asText`, `asEWKB`, `asGeoJSON` over
   geometry and geography; `transform` and `asEWKT` rebound from PostGIS to MEOS.
10. **The clustering** (decision 2).

The portable dialect chapter (`doc/portable_sql.xml`) lists each `X` and `geoX` as the PR lands
them. `geom_unary_union` stays outside the rule until MEOS answers it natively.
