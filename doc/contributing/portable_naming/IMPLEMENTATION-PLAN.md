<!--
  MobilityDB — Portable Naming: Implementation plan — class-prefixed names, input and output, and the portable chapter
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Implementation plan — class-prefixed names, input and output, and the portable chapter

[Back to the index](00-INDEX.md) · [Set-returning functions](10-SET-RETURNING.md) · [I/O plan](IO-PLAN.md)

This plan turns families 1–9, the set-returning analysis (10), the I/O plan, rules 8–9 and the
manual draft into pull requests, MobilityDB first. Measured at MobilityDB master `3e507c9695`,
MEOS-API `20efba8`, 2026-09-28; the set-returning functions and the I/O plan at MobilityDB master
`0be1b51060`, MEOS-API `20efba81`, JMEOS `e47aa744`, 2026-09-29. Part IV, the parity of the Spark
and Flink surfaces with MobilityDB's, is measured at MobilityDB master `30a9a730bb`, MEOS-API
`5ba0cd8492`, JMEOS `5422d27c`, 2026-10-02.

## Where things stand

- **The documents and the manual draft are MobilityDB PR #2861** (draft): these notes in
  `doc/contributing/portable_naming/`, and the 18 manual files the draft changes: the portable
  chapter in sections with its porting section, the class-prefixed topology, route and distance
  names, and the entries for `<type>FromText` and `asText`. Each PR below takes its own part of
  the manual draft.
- **`@altsqlfn` is a Doxygen tag only.** It sits on `Set_constructor` and `Value_to_set`
  (`setMake`, #2736). MEOS-API does not read it (0 files mention it, against 3 for `@sqlaggfn`),
  so no binding sees an alternative name yet.
- **MEOS has the C functions Part I adds**: `geo_length` (G17, #2910) and the affine family over
  `geometry`, `geo_affine`, `geo_rotate`, ... (G11, #2905); `tquadbin_grid_distance` and
  `ts2cell_grid_distance` (G12) wait with the cell work. `text_lower`, `text_upper`,
  `text_initcap`, `geo_round`, `geo_transform_pipeline`, and the readers `temporal_in`, `set_in`, `span_in`, `spanset_in`,
  `tbox_in`, `stbox_in`, `quadbin_in`, `s2cell_in`, `nsegment_in`, `tpcbox_in`, `raquet_in` exist.
- **Set-returning functions** ([10](10-SET-RETURNING.md)): the catalog states each set-returning
  signature, the columns of its rows and the C value feeding each (A3, A4), and the compositions
  over another type's functions (A7); the Spark generator registers each set-returning signature
  as an array of its rows (J7). The Flink surface waits on J4.
- **Input and output**: of the 59 types with the four PostgreSQL I/O functions, 24 lack
  `FromText` (`raquet` takes its text as hex WKB, rule 9) and `nsegment`, `quadbin`, `s2cell`,
  `tpcbox` read and write their text form (G14, #2901), and every type reads its text through
  `<type>FromText` (G13, #2908). The
  catalog's `typeEncodings` states, class by class, the reader and writer of each SQL type (A6).
- **Open pull requests** on 2026-10-02: MobilityDB #2861 (these notes) and #2919, which names an
  operation over a base value as MobilityDB names it over its own types (decision 0.5); #2920
  touches the GEOS checker's baseline only; #785, #777 and #677 touch none of these names or
  functions; MEOS-API has none.

## Rules every PR follows

- One topic per PR; its commits are that topic's steps. Code, SQL, SQL regression tests with their
  harvested output, MEOS smoke tests, and the manual EN+ES travel in the same commit, with the reference
  appendix regenerated (`tools/doc/gen_reference.py`) and `check-doc-reference.yml` green.
- Every PR targets upstream master, from its own worktree and branch; a PR that needs another
  starts from it once it is green.
- Receipts before push: strict-ci (all flags), cppcheck, Windows, coverage, geomclass;
  CI green before anything downstream.
- The body is the commit message body.

## Part I — close the MobilityDB gaps (the prerequisite)

The renaming campaign starts only once MobilityDB states every function it names: a portable
name given to a function MEOS lacks, to a type without its input and output, or to a family
that spells one argument two ways would carry the gap into every engine. Part I closes them in
MEOS and in the PostgreSQL extension; no SQL function changes its name in it.

### Pull requests

| PR | Topic | What it changes | Waits on |
|---|---|---|---|
| G0 | State the portable naming design in the contributor notes | #2861: these documents in `doc/contributing/portable_naming/`, with the local paths, memory links and probe logs taken out, beside the manual draft it carries | open, iterating |
| G1 | Give `tbigint` its value splits and value tiles in MEOS | `tbigint_value_split`, `tbigint_value_time_split`, `tbigintbox_value_tiles`, `tbigintbox_time_tiles`, `tbigintbox_value_time_tiles` beside their `tint` and `tfloat` siblings, with `@csqlfn #Tnumber_value_split()` / `#Tnumber_value_time_split()` / the box wrappers; smoke tests, the manual entries | merged, #2864 |
| G2 | Name every grid argument by its dimension | a value bin is `vsize`, `vorigin`, a time bin `duration`, `torigin`, a space bin `xsize`, `ysize`, `zsize`, `sorigin`, and the one-size space form `xsize`; `size` names only the byte length of the WKB functions (`*_from_wkb`, `raquet_read_bytes`, `geo_as_ewkb`), whose name JMEOS's `FunctionsGenerator` reads as one (`SIZE_PARAM_NAMES`). The names live in `tools/codegen/inherited/generate.py` and its manifests, which emit the declarations (#2401). SQL, 138 arguments, 99 renamed and 39 named: `valueSplit` (`tint`, `tbigint`, `tfloat`) `size` → `vsize`, `origin` → `vorigin`; `valueTimeSplit` (the same three) `size` → `vsize`; `getBin` (`integer`, `bigint`, `float`) `size` → `vsize`, `origin` → `vorigin`, and (`date`, `timestamptz`) `origin` → `torigin`; `timeSplit` (20), `tsample` (19), `tprecision` (18) `origin` → `torigin`; `bins` over date and timestamp spans (4), `timeBins` (5), `timeBoxes` over numbers (3), `valueTimeBoxes` (3) `tsize` → `duration`; the one-size forms of `spaceSplit` and `spaceTimeSplit` (10) `size` → `xsize`; the unnamed `interval` of `spaceTimeSplit` (15), `spaceTimeBoxes` (15) and `timeBoxes` over the spatial types (5) named `duration`; the unnamed `timestamptz` of `timeTiles` (1) and `spaceTimeTiles` (3) over `stbox` named `torigin`; `bins(bigintspan\|bigintspanset, vsize bigint, vorigin int)` takes `vorigin bigint`, as `bigintspan_bins` takes `int64 vorigin`. MEOS, 24 functions naming their grid parameters by the rule in their declaration, their definition and its `@param` lines, 18 of which name one parameter one way in their declaration and another in their definition: `tfloat_value_split` `size`, `origin` → `vsize`, `vorigin`; `tint_value_time_split` `size` → `vsize`; `tintbox_value_tiles`, `tintbox_value_time_tiles` `xsize`, `xorigin` → `vsize`, `vorigin`; `temporal_time_bins`, `temporal_tprecision`, `temporal_tsample`, `tstzspan_bins` `origin` → `torigin`; `timestamptz_bin` keeps the `stride` and `origin` of the PostgreSQL function it is derived from. The manual EN+ES and the list of renamed arguments for the release notes, beside #2401's (`endian`, `duration`): a call naming a renamed argument (`valueSplit(t, size => 5)`) and a dumped view written with one no longer resolve; a positional call does, and an upgrade script into the release drops and recreates these functions, since `CREATE OR REPLACE FUNCTION` cannot rename an argument | merged, #2868 |
| G3 | Return parallel arrays one way: the first array returned, every further array and the count as out-parameters | the rule of the 139 one-array functions and of 32 of the 35 public functions returning parallel arrays, the form MEOS 1.3.0 declares: `Temporal **tgeo_space_split(…, GSERIALIZED ***space_bins, int *count)`, `Temporal **tgeo_space_time_split(…, GSERIALIZED ***space_bins, TimestampTz **time_bins, int *count)`, `bool tpoint_as_mvtgeom(…, GSERIALIZED **gsarr, int64 **timesarr, int *count)`; the structs `SpaceSplit`, `SpaceTimeSplit` and `MvtGeom` removed; the callers (`tgeo_space_split`'s own call, `Tpoint_as_mvtgeom`), `meos/examples/tpoint_tile.c`, which calls the 1.3.0 form, and the smoke tests. `spaceSplit`, `spaceTimeSplit` and `asMVTGeom` keep their SQL declarations and rows | merged, #2871 |
| G4 | Give the temporal circular buffer, pose, pose chain and rigid geometry their rigid motions | `translate` and `rotate`, `rotateZ` over `tcbuffer`, `tpose`, `tposechain` and `trgeometry`, and `rotateX`, `rotateY` over the last three, in MEOS under each family's stem and in SQL; a translation or a rotation keeps each value what its type states (one centre and one radius, `cbuffer/cbuffer.h`; a position and an orientation, `pose/pose.h`; a reference geometry moved by a pose, `trgeometryinst_make`), a scale with two factors, a shear or a rotation out of the plane does not, so `scale`, `affine` and `transscale` stay with `geometry`, `tgeompoint` and `tgeometry` ([4](04-SPATIAL-TRANSFORMATIONS.md), decision 6); `rotateX` and `rotateY` lift a 2D value to 3D exactly, the position at z = 0 and the orientation the rotation composed with the planar one, a rigid geometry's reference geometry at z = 0; `INHERITANCE_MAP.md` states the family; smoke tests, the entries | merged, #2875 |
| G5 | Name the point cloud cast by its class | `tpointcloud_to_tgeompoint` → `tpcpoint_to_tgeompoint`: a `<type1>_to_<type2>` name whose prefix is the class the function is generic over, and it validates `tpcpoint` alone (`VALIDATE_TPCPOINT`), as its wrapper `Tpcpoint_to_tgeompoint` and its sibling `tpcpatch_to_tgeometry` read; `tpose_to_tpoint` keeps its name, `TPoint` being the class its result belongs to, as for `geomeas_to_tpoint`; its `@csqlfn`, its callers | merged, #2876 |
| G6 | Give `nsegment`, `quadbin`, `s2cell` and `tpcbox` their binary input and output | MEOS `<type>_as_wkb`, `<type>_as_hexwkb`, `<type>_from_wkb`, `<type>_from_hexwkb` for the four, as `h3index_*`, `raquet_*`, `npoint_*` state them, the `tpcbox` form carrying its SRID as `stbox`'s does and `nsegment` taking the `E` forms `npoint` has (`asEWKB`, `nsegmentFromEWKB`, `asHexEWKB`, `nsegmentFromHexEWKB`); SQL `asBinary`, `asHexWKB`, `<type>FromBinary`, `<type>FromHexWKB`; the entries. The four have `_send`/`_recv` and no MEOS WKB function, so a binding cannot carry them as WKB (rule 6); the other 55 types with an input function have `asBinary` and `FromBinary` ([I/O plan](IO-PLAN.md)) | merged, #2877 |
| G7 | Give the raster its `transformPipeline` | MEOS `raster_transform_pipeline(const Raster *rast, const char *pipelinestr, int32_t srid, bool is_forward, …)` with the warp arguments of `raster_transform`, through `raster_warp` passing GDAL's `COORDINATE_OPERATION` option (GDAL ≥ 3.0, `alg/gdaltransformer.cpp`) where it passes `SRC_SRS`/`DST_SRS` today; SQL `transformPipeline(raster, …)` and its entry; the porting table drops "but raster". PostGIS declares `ST_TransformPipeline` for `geometry` only and its `rt_raster_gdal_warp` hands GDAL the two SRS alone, which is the whole reason the raster lacks it. The parameters follow the five siblings spelling `const char *pipelinestr, int32_t srid`; `geo_transform_pipeline` (`char *pipeline, int32_t srid_to`) and `posechain_transform_pipeline` (`const char *pipeline, int32_t srid_to`) take that spelling in the same PR. Acceptance: a forward and an inverse pipeline over a raster answer as `transform` to the same SRID, which also settles whether GDAL inverts every pipeline a warp needs | merged, #2880 |
| G8 | Give the spatial types the inverse of their `asHexWKB` | SQL `<type>FromHexWKB` over the MEOS `*_from_hexwkb` for the 12 types that declare `asHexWKB` and no `FromHexWKB` (`cbuffer`, `npoint`, `pose`, `posechain`, `tcbuffer`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tpose`, `tposechain`, `trgeometry`); the `@csqlfn` of `temporal_as_wkb`/`temporal_from_wkb` names `Trgeometry_send`/`_recv`, which call them; the entries ([I/O plan](IO-PLAN.md)) | merged, #2882 |
| G9 | Complete MF-JSON input | MEOS `th3index_from_mfjson`, `tquadbin_from_mfjson`, `ts2cell_from_mfjson`, `tpcpoint_from_mfjson`, `tpcpatch_from_mfjson` beside the fifteen temporal types that have theirs; SQL `tpcpointFromMFJSON`, `tpcpatchFromMFJSON`, the inverses of the `asMFJSON` they declare; one spelling of `asMFJSON`'s argument (`temp` over nine types, unnamed over eleven) ([I/O plan](IO-PLAN.md)) | merged, #2883 |
| G10 | Call MEOS where the affine family composes in SQL | `translate(t, dx, dy)` and `scale(t, xf, yf)` as the 3D C-backed forms with the defaults `deltaz` 0 and `zfactor` 1 the catalog states; `rotate(t, angle, geometry)` over a MEOS rotation about a point, not PostGIS `ST_X`/`ST_Y` ([4](04-SPATIAL-TRANSFORMATIONS.md), decision 3) | merged, #2889 |
| G11 | Give `geometry` the affine family and refuse geodetic values | MEOS `geo_affine`, `geo_affine_2d`, `geo_rotate`, `geo_rotate_x`, `geo_rotate_y`, `geo_rotate_z`, `geo_scale`, `geo_scale_xyz`, `geo_translate`, `geo_transscale` over `lwgeom_affine`; SQL `translate`, `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `transscale` over `geometry` (the names by #2919, decision 0.5), the 15 PostGIS signatures; the `tgeo_*` and `geo_*` affine functions refuse a geodetic value (in the source, `tgeo_type_all` admits `tgeogpoint` and `tgeography` and the path reaches the planar `lwgeom_affine` with no geodetic check; a smoke test calling `tgeo_translate` on a `tgeogpoint` first establishes what it does); smoke tests, the entries ([4](04-SPATIAL-TRANSFORMATIONS.md), decision 4) | merged, #2905 |
| G12 | Give the quadbin and S2 cells their grid distance | MEOS `tquadbin_grid_distance`, which `meos_quadbin.h` declares commented out under "Grid traversal + metrics", the smallest `k` whose `quadbin_grid_disk(origin, k)` holds the destination, as H3's grid distance is to its grid disk; SQL `tquadbinGridDistance` and the `<->` over `tquadbin` it backs, as `th3GridDistance` backs `<->` over `th3index` (`285_th3index_traversal.in.sql`); the same for S2, `ts2cell_grid_distance` and `ts2GridDistance`, the number of steps between edge neighbours of one level (`s2cell_edge_neighbors`), the neighbour coming from the family's own function; smoke tests, the entries | waits with the cell work |
| G13 | Read every type from its text: `<type>FromText` (rule 9) | one wrapper per family taking its type from `get_fn_expr_rettype`, as `Tspatial_from_ewkt` does, over the family's reader; SQL for the 24 types the [I/O plan](IO-PLAN.md) lists (the sets, spans and span sets of the base types, `jsonbset`, `textset`, `tbox`, `stbox`, `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`); `<type>_in` untouched; the entries `setspan_FromText`, `boxFromText`, `ttypeFromText` #2861 drafts | merged, #2908 |
| G14 | Give quadbin, s2cell, nsegment and tpcbox their text output and input | MEOS `quadbin_out` beside `s2cell_out`; SQL `asText` and `<type>FromText` for each, and for `nsegment` `asEWKT` and `nsegmentFromEWKT`, as `npoint` has them; the entries `cell_asText`, `cellFromText`, `tpcbox_asText`, `tpcboxFromText`, `asText(nsegment)` and `nsegmentFromText` in the network point chapter #2861 drafts | merged, #2901 |
| G15 | Give the function `merge` the point cloud types (family 8) | `merge(tpcpoint, tpcpoint)`, `merge(tpcpatch, tpcpatch)` and their array forms over `temporal_merge`/`temporal_merge_array`; tests | merged, #2909 |
| G16 | Document the set unnest and read the rigid geometry unnest from MEOS (family 9) | entry `setspan_unnest` EN+ES for the 18 set declarations; `Trgeometry_unnest` calls `trgeometry_unnest` once both answer alike | merged, #2903 and on master |
| G17 | Measure a geometry or geography in MEOS | MEOS `geo_length` over `geom_length`/`geog_length` (spheroid, as `ST_Length`), the function family 5 names; smoke tests | merged, #2910 |
| G18 | State in MEOS the grid sizes a caller leaves out | in `tgeo_space_split`, `tgeo_space_time_split`, `tgeo_space_boxes`, `tgeo_space_time_boxes`, `trgeometry_space_boxes`, `trgeometry_space_time_boxes`, `stbox_space_tiles`, `stbox_space_time_tiles`, `stbox_get_space_tile` and `stbox_get_space_time_tile` a `ysize` or `zsize` of 0 is a size left out, which takes `xsize`, and a `zsize` is read only for a value with Z; every size is a finite number that is not negative. MEOS validates where the grid state is laid (`stbox_space_time_tile_init`, `tgeo_space_time_tile_init`, `tgeo_space_time_split_init`); the SQL forms stating one or two sizes are C functions passing 0, those of `tpose` and `tpcpoint` pass 0 to their full form, and the functions returning rows still read the grid state one tile at a time | merged, #2887 |

G0 is the notes and the manual draft (MobilityDB #2861). No other pull request of Part I waits
on a decision. The forms stating one or two grid sizes pass 0 for the sizes they leave out,
which MEOS reads as `xsize` (G18).

## Part II — close the catalog and generator gaps

The catalog and the generators then carry what Part I declares: the set-returning signatures,
their `OUT` columns and repeated arguments, the type encodings, and the Flink and Spark forms
of a function returning rows.

### MEOS-API

| PR | Topic | Waits on |
|---|---|---|
| A3 | State on each SQL signature that it returns a set: the `RETURNS` match of `_create_fn_stmts` keeps `SETOF` (the 132 signatures of [10](10-SET-RETURNING.md)) | merged, MEOS-API #149 |
| A4 | State the columns of each row a signature returns and the C value behind each: `_bare_type` strips the argument mode, so the 37 `…Pairs` signatures list `OUT i integer, OUT j integer` as two inputs; state them as the columns of the `record` the signature returns. For every signature returning several columns (144 over composite types or `OUT` parameters), name the C array or group element feeding each column, since SQL and C order them differently (`valueTimeSplit` rows `(number, time, tnumber)` from the returned fragments and the out-parameters `value_bins`, `time_bins`; `unnest` rows `(value, time)` from the returned `SpanSet **` and the out-parameter `values`), and the columns of the fixed tuples `pose_quaternion` (`W, X, Y, Z`) and `pose_ypr` (`yaw, pitch, roll`), which the catalog states as a `double` array | merged, MEOS-API #151 |
| A5 | Withdrawn: a form stating one or two grid sizes passes the literal 0 for the sizes it leaves out (G18), which `boundArgs` states | withdrawn |
| A6 | State the type encodings a binding reads, class by class: a class several SQL types share (`Set`, `Span`, `SpanSet`, `Temporal`) reads and writes each SQL type through that type's own public function (`readers`/`writers`: `floatset_in`, not `bigintset_in`), keyed by the SQL type its signature returns or takes; the hex-WKB writer of a class is its `wkb` encoder, its `variant` the value the type's own send binds; every reader and writer states its trailing inputs by name with the value a binding passes; the cell ids `H3Index`, `Quadbin`, `S2CellId` are classes of their own; `Raster` states no text form ([I/O plan](IO-PLAN.md)) | merged, MEOS-API #157 |
| A7 | State each SQL function the inheritance manifest renders as a composition over another type's functions, and its steps: `spaceSplit(tpose, …)` is `tpose_to_tpoint`, `tgeo_space_split`, and `temporal_at_tstzspanset` over the `temporal_time` of each fragment (`tiling_families.yaml`, entries `tpose_tile` and `tpcpoint_tile`, cast-only rule). The catalog states no such composition, so no binding carries the 26 grid functions of `tpose` and `tpcpoint` | merged, MEOS-API #152 |

### JMEOS

| Repo | PR | Topic | Waits on |
|---|---|---|---|
| JMEOS | J4 | The Flink SQL generator emits for each set-returning signature (A3) a scalar function returning the array J7 returns in Spark (`ARRAY<…>`, `ARRAY<ROW<…>>` for records), which `CROSS JOIN UNNEST` unfolds into rows, passing the out-parameters the catalog states (`shape.outParams`) as the Spark arm passes the count of the `…Pairs` kernels ([9](09-UNNESTING.md), decision 5) | nothing |
| JMEOS | J5 | `SIG_RE` in `codegen_jvm.py` reads a return type carrying `$`, so `tgeo_space_split`, `tgeo_space_time_split` and `tpoint_as_mvtgeom` reach the flink-sql arm and the facades | merged, JMEOS #112 |
| JMEOS | J6 | `FunctionsGenerator` maps a parameter by its catalog type alone, without `SIZE_PARAM_NAMES` (`tint_value_time_split`'s `int size` is `long` in the jar; G2 renames it `vsize`, and a parameter's type stays its catalog type whatever its name) | merged, JMEOS #113 |
| JMEOS | J7 | The Spark generator registers each set-returning signature under its SQL name (A3), `unnest` and the splits included, returning an array that `LATERAL VIEW explode` (or `inline` for records) unfolds into rows, as it does for the `…Pairs` kernels | merged, JMEOS #121 |

## Part III — the renaming campaign

With Parts I and II merged, the names change: the chapter restructure first (later PRs fill its
rows), then the operator-backing renames (PostgreSQL included), then the new names, then the
tags that give Flink and Spark their spelling, then the regenerated bindings.

### Decisions Part III rests on

| # | Item | Decided |
|---|---|---|
| 0.1 | Families [8](08-MODIFICATIONS.md) (`insert`, `update`, `merge`, 93 signatures) and [9](09-UNNESTING.md) (`unnest`, 38) | final: Flink and Spark take `temporalInsert`, `temporalUpdate`, `temporalMerge` (the function) and `mergeAgg` (the aggregate); Flink, Spark and DuckDB take `setUnnest` and `temporalUnnest`; PostgreSQL keeps `insert`, `update`, `merge` and `unnest` |
| 0.2 | The overall regularity pass (index, "After the last family") | every family meets under the decided rules; `trgeometry` takes `geo` by the geometry it answers at an instant (`geoRound`, `geoTransform`, `geoLength`, `geoTranslate`, `geoRotate`, families 2, 4 and 5) |
| 0.3 | Family 6's condition | three parts ([6](06-BOUNDING-BOX-TOPOLOGY.md), "Condition of both"): every engine resolves each signature to the same MEOS function; the catalog states each name's `IndexSearchOp` (A8, before R2); each predicate answers the same rows with and without an index, in MobilityDuck today |
| 0.4 | Does PostgreSQL declare the `@altsqlfn` names too? | no: PostgreSQL keeps its names and gains no function, R9 and R10 are tags and documentation, and Flink and Spark carry one name set (rule 1) |
| 0.5 | Under which name does an operation reach a base type? | under the name MobilityDB gives it over its own types, in each engine: PostgreSQL declares `length(geometry)`, `transformPipeline(geometry, ...)` and `rotateX(geometry, ...)` beside `length(tgeompoint)`, `transformPipeline(tgeometry, ...)` and `rotateX(tpose, ...)`, as it declares `transform`, `buffer`, `convexHull` and `round` over `geometry`, and declares nothing where PostgreSQL answers the operation under that name, as `lower('Hello')`; Spark and Flink call `geoLength`, `geoTransformPipeline` and `geoRotateX` over every type, the `@altsqlfn` of the same wrappers. MEOS answers the geometry forms natively, without GEOS, so the PostgreSQL name reaches that engine where the `ST_` name reaches PostGIS |

### MobilityDB

| PR | Topic | What it changes | Size (measured) |
|---|---|---|---|
| R2 | Name the topological functions by the class they compare (family 6), in the portable dialect chapter given one section per operator family, and spell the route identifier functions without an underscore | first commit, the chapter: `doc/portable_sql.xml` EN+ES as #2861 drafts it: sections as the family chapters title theirs, full-width tables in the normal font, the Same row in the topology section, the route operators beside it; no function renamed; then the topological names: the generator (`topop_families`, `topops.sql.tmpl`), the `@sqlfn` tags, the planner's topology name table in `temporal_supportfn.c` as per-prefix macros like the position ones, `tools/codegen/portable_aliases`, the function-form tests (`010_span_ops_supportfn_tbl`, `044_temporal_indexes_tbl`), the chapter rows and migration table; then the route identifiers: `same_rid`→`sameRid`, `contains_rid`→`containsRid`, `contained_rid`→`containedRid`, `overlaps_rid`→`overlapsRid`; `310_tnpoint_routeops.in.sql`, the wrappers' `@sqlfn` tags, the chapter row | 729 signatures, 31 names; waits on 0.3; and 20 declarations |
| R4 | Name the span and span set distance functions by class | bare `distance` over spans and span sets → `spanDistance` / `spansetDistance` by the first operand that is not a base value, as `spanLeft`/`spansetLeft`; `distance(cbuffer, …)` and `distance(pose, …)` keep their name (documented, called by users); the chapter row | 40 declarations |
| R9 | State the Flink and Spark name of every colliding function (`@altsqlfn`, families 1–5 and 7–9) | the tag on each PostgreSQL wrapper (`floatRound`, `geoRound`, `spanLower`, `temporalLowerInc`, `intAbs`, `floatCeil`, `geoTransform`, `geoTransformPipeline`, `geoLength`, `geoTranslate`, `geoAffine`, `geoRotate`, `geoRotateX`, `geoRotateY`, `geoRotateZ`, `geoScale`, `geoTransscale`, `npointLength`, for family 7 `setHash`, `spanHash`, `spansetHash`, `tboxHash`, `stboxHash`, `temporalHash` and the base types' own, each with its `HashExtended` form, and for families 8 and 9 `temporalInsert`, `temporalUpdate`, `temporalMerge`, `setUnnest`, `temporalUnnest`, …); no SQL declaration (0.4) | about 322 wrappers |
| R10 | Document porting an application to Spark and Flink | the `portable_sql_porting` section #2861 drafts (the changes table, among them the typed literal, the text of a value, the cast; the names table; the example), once PR R9's names exist | doc only |

R4 is independent of R2 and can run in parallel with it; R10 needs R9. An operation over a base type takes in each engine the name it
carries over the MobilityDB types (decision 0.5).

### MEOS-API

| PR | Topic |
|---|---|
| A1 | Read `@altsqlfn` into `altSqlName` per SQL signature |
| A2 | The portable aliases' class dimension for the topology, route and distance names (as #146 did for positions) |
| A8 | State, for each topological and position SQL name, the `IndexSearchOp` an index searches for it, with the indexed column on either side (`stboxContains`: `INDEX_CONTAINS`, `INDEX_CONTAINED_BY`); before R2 (0.3) |

### The bindings

| Repo | PR | Topic |
|---|---|---|
| JMEOS | J1 | The Flink SQL generator reads `altSqlName` else `sqlName`, registers aggregates under their `Agg` spelling, and gains the temporal `FromText` readers |
| JMEOS | J2 | MEOS values travel as binary WKB (rule 6): Flink `BYTES`, Spark `BinaryType`, through the codec each class states (A6) |
| JMEOS | J3 | One Spark user-defined type per MEOS SQL type and one registration per name resolving its overloads (rule 8), a conflicting function under its class-prefixed name alone (rule 1) |
| JMEOS | J8 | Generate the index search operation enum from the catalog's `IndexSearchOp`, in place of `jmeos-core/src/main/java/functions/RTreeSearchOp.java`, which declares 3 of its 21 values under the enum's former name |
| MobilityFlink, MobilitySpark | F1, S1 | Regenerate the surfaces on J1–J7 and their tests |
| MobilityDuck | D1 | Re-vendor the catalog; generate the index routing from A8's statement in place of `src/include/index/index_search_ops.hpp`, which routes predicates to the R-tree by function name, for PRs R2–R4 (as #410 did for positions); rename the temporal table function `tempUnnest` to `temporalUnnest` over all 20 types, `tbool` included, and restore its tests |
| PyMEOS, GoMEOS, MEOS.NET, MEOS.js | — | no SQL names; each regenerates on the catalog and gains the MEOS functions of G1, G4, G6, G7, G9, G11, G12, G14 and G17 |

## Part IV — parity of Spark and Flink with MobilityDB

A user who knows MobilityDB calls the same function, under the same name and over the same
types, in Spark and in Flink. Parts I–III close the gaps a renaming would carry and give the
colliding names their engine spelling; this part closes the rest of the distance between the two
JVM surfaces and MobilityDB's.

### Measured

At MobilityDB master `30a9a730bb`, MEOS-API `5ba0cd8492` and JMEOS `5422d27c`, on 2026-10-02.
The MobilityDB surface is the extension script `make mobilitydb_sql` writes, its `CREATE
FUNCTION` statements read by MEOS-API's own reader (`sql_statements` of `parser/sqlfn.py`). A
signature users call is one the script gives no plumbing role: type input and output, aggregate
state, selectivity, index or planner support are left out, while a function behind an operator
or a cast stays, since Flink and Spark reach an operator or a cast through its function. That
leaves 8,164 signatures: 4,005 called by name, 3,311 behind an operator, 499 behind a cast and 349
aggregates. Flink's verdict per signature is its generator's own (`_overload` and
`_setret_overload` of `codegen_jvm.py`); Spark reaches a signature when a registration under its
SQL name calls its C function.

| | Signatures reached | Share |
|---|---|---|
| Flink | 5,354 | 66% |
| Spark, under the MobilityDB name | 2,979 | 36% |
| Both | 2,408 | 29% |
| Neither | 2,239 | 27% |

### Why a signature is missed

Four causes sit upstream of both engines, so one fix reaches both:

| Cause | Signatures | Examples | Owner |
|---|---|---|---|
| U0. An aggregate | 349; Flink carries none, Spark 7 names (`tCount`, `tSum`, `tAndAgg`, `tOrAgg`, `tMinAgg`, `tMaxAgg`, `mergeAgg`) | `extent`, `tAvg`, `tCentroid`, `setUnion`, `appendInstantAgg` | JMEOS |
| U1. A `LANGUAGE SQL` body with no C backing | 351, 42 names | the spatial relationships of `tpose`, `tnpoint`, the cells and the point clouds through a cast (`aContains`, `tIntersects`, `eDwithin`); `spaceSplit`, `spaceBoxes`, `spaceTimeTiles` over `tpose` and `tpcpoint`; `expandSpace` | MEOS-API, JMEOS |
| U2. A C wrapper the catalog maps to no MEOS function | 464, 237 names | the `<type>FromText` wrappers of G13; `asEWKB` and `asHexEWKB` taking an endian text; `round(tfloat[], integer)`; `setDistance(geometry, geomset)`; `same_rid`; the casts from ranges | MobilityDB |
| U3. A signature whose catalog backer is internal | 214 in Flink, 319 in Spark, about 35 names | `asText(tint)` on `temporal_out`, `atValue`, `getValue`, `instants`, `memSize`, `<type>FromBinary` on `set_from_wkb` | MobilityDB |

A binding calls a public MEOS function only, so U3 is closed by the tag pointing the catalog at
the public typed function (`tint_out` beside the internal `temporal_out`), never by a binding
calling the internal one.

The engines' own causes, by the reason each generator gives:

| Engine | Cause | Signatures |
|---|---|---|
| Flink | no `geometry` or `geography` value | 369 arguments, 58 results |
| Flink | `<type>FromBinary`, `<type>FromEWKB`: the `bytea` input against the jar's data and length | 316 |
| Flink | a `text` argument: the endian text the C function takes as `uint8_t` (118), a `text *` (65) | 187 |
| Flink | the cells `h3index`, `quadbin`, `s2cell` | 144 arguments, 13 results |
| Flink | an SQL argument count other than the C one (`geography(tgeogpoint)`, `getX(pcpoint)`) | 74 |
| Flink | the bound `NORMALIZE` of the `<type>Seq` and `<type>SeqSet` constructors | 38 |
| Flink | a base value in the distance functions (`setDistance`, `spanDistance`, `nearestApproachDistance`) | about 41 |
| Flink | a set-returning signature with a `geometry` column | 34 |
| Flink | `jsonpath`, `text[]`, a bound `INVERT` | 28 |
| Spark | a C function registered under its C name and not under its SQL name | 2,896 |
| Spark | an enum argument read from text (`interpType`, `nullHandleType`), which Flink reads through the catalog's parser of that enum | about 160 |
| Spark | a count out-parameter (`spaceBoxes`, `splitNSpans`), which Flink passes | 184 |
| Spark | `TPCBox`, `PoseChain`, `Raster` values | about 240 |
| Spark | an array of timestamps or dates, a `uint32_t` | about 50 |
| Both | a type spelled `float8` or `int` where MobilityDB writes `float` and `integer` (`affine`, `rotateX`, `scale` over `tgeometry`; `valueN(tbool, int)`) | 49 |

The Spark surface registers every public C function under its C name (3,013 names) and the SQL
names over a dispatch: `asText` dispatches over the temporal types only, so `asText(intset)` is
reached as `intset_out`.

JMEOS's Spark gaps check fails against MobilityDB `30a9a730bb`: `tjsonb_to_tbigint`, public since
`21a15203ba`, takes a `nullHandleType`, which the Spark arm does not read (J9). Flink reads an
`interpType` through `interptype_from_string`, which `meos_catalog.h` declares internal while its
sibling `null_handle_type_from_string` is public in `meos_json.h`; 39 generated Flink classes call
it (G22).

### Rasters derived from rasters

PostgreSQL users derive rasters with PostGIS, and MobilityDB composes with it:
`rasterValue(trip, ST_MapAlgebra(...))` reads a derived raster along a trajectory. Spark and Flink
carry no PostGIS, so there the only raster operations are those MEOS states. A raster analysis of
moving objects derives rasters throughout (the clearance under a vessel from the depth, the slope
and the banks of the seabed, the highest waves of a day from its hourly fields), and MEOS answers
part of it:

| Operation | PostGIS function | MEOS today | `rt_core` function MEOS vendors |
|---|---|---|---|
| clip, reclassify, polygonize, summarize, transform, rescale | `ST_Clip`, `ST_Reclass`, `ST_DumpAsPolygons`, `ST_SummaryStats`, `ST_Transform`, `ST_Rescale` | `raster_clip`, `raster_reclass`, `raster_dump_as_polygons`, `raster_summary_stats`, `raster_transform`, `raster_rescale` | — |
| local operation over one or two rasters (`'[rast] - 10'`, NDVI, the difference of two time steps) | `ST_MapAlgebra` with an expression | none | `rt_raster_iterator` |
| focal operation over a window, also across tiles (`customextent`) | `ST_MapAlgebra` with `ST_Mean4ma`, `ST_Min4ma`, … | none | `rt_raster_iterator` with a neighbourhood |
| slope, ruggedness, topographic position | `ST_Slope`, `ST_TRI`, `ST_TPI` | none | `rt_raster_iterator` with a neighbourhood |
| union of tiles, and of time steps by a pixel operation (`'MAX'`) | `ST_Union` | none | `rt_raster_iterator`, `rt_raster_from_two_rasters` |
| value histogram | `ST_ValueCount` | none | `rt_band_get_value_count` |
| pixels as points, values as an array | `ST_PixelAsPoints`, `ST_DumpValues` | none | `rt_band_get_pixel`, `rt_pixel_set_to_array` |
| statistics over many rasters | `ST_SummaryStatsAgg` | none | `rt_band_get_summary_stats`, its running count, mean and Q |
| resample onto another raster's grid, alignment test | `ST_Resample`, `ST_SameAlignment` | `raster_transform_raster` resamples | `rt_raster_same_alignment` |
| a raster built from values | `ST_MakeEmptyRaster`, `ST_AddBand`, `ST_SetValues` | none | `rt_raster_new`, `rt_raster_generate_new_band`, `rt_band_set_pixel` |

MEOS gives an engine without PostGIS the functions the PostGIS raster extension already
provides, from the `rt_core` it vendors, as the GEOS opt-out gave MEOS its native `geo_*`
functions and SQL their plain names (`buffer`, `convexHull`, `relate`) beside PostGIS's `ST_`
ones (decision 0.5), and as the raster family already stands (`raster_clip` and `clip`,
`raster_reclass` and `reclass`, `raster_width` and `width`). What PostGIS keeps outside `rt_core`
is not vendored: the expression form of `ST_MapAlgebra` evaluates SQL through SPI, the `4ma`
callbacks (`st_min4ma`, `st_mean4ma`, `_st_slope4ma`, `_st_tpi4ma`, `_st_tri4ma`) are `plpgsql`,
and the union aggregate lives in `rt_pg`. MEOS states those as callbacks of `rt_raster_iterator`
computing PostGIS's own formulas.

### Decisions Part IV rests on

| # | Item | Decided |
|---|---|---|
| 0.13 | Spark's registrations under C names | open: kept beside the SQL names, or removed so that Flink and Spark carry one name set (decision 0.4) |
| 0.14 | The names of the raster operations of the table above | decided by the precedent of decision 0.5 and the raster family: MEOS `raster_<operation>`, SQL the PostGIS name without `ST_` with PostGIS's argument names, order and defaults, declared in PostgreSQL beside PostGIS's; open: what takes the place of the SQL expression and of the callback where PostGIS takes one (a typed operator, an enumerated statistic) |

### MobilityDB

| PR | Topic | What it changes |
|---|---|---|
| G19 | Spell every SQL type as the rest of MobilityDB does | `float8` → `float`, `int` → `integer` in the declarations that spell them so (`affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale` over `tgeometry`, `rescale` and `transform` over `raster`, `valueN(tbool, int)`) |
| G20 | Map every C wrapper to its MEOS function | the `@csqlfn` tag on the MEOS function each of the 237 names of U2 calls, and the MEOS function where none exists |
| G21 | Back every signature by a public MEOS function | the tags of U3 on the public typed functions (`tint_out`, …), the internal kernels keeping theirs for the C callers |
| G22 | Read an interpolation from its name in the public API | `interptype_from_string` declared in `meos.h` beside `null_handle_type_from_string`, validating its argument as an external function does |
| G23 | Give MEOS the raster operations an engine without PostGIS needs | for each row "none" of the table above, a public MEOS `raster_<operation>` over the vendored `rt_core` function its last column names, the `4ma` statistics, slope, ruggedness and topographic position as `rt_raster_iterator` callbacks on PostGIS's formulas, and its SQL function under the PostGIS name without `ST_` (decision 0.14); SQL tests, smoke tests, the manual entries EN and ES |

### MEOS-API

| PR | Topic |
|---|---|
| A9 | State MobilityDB's whole SQL surface: the `LANGUAGE SQL` functions with their bodies and the aggregates with their state functions, beside the C-backed signatures |
| A10 | State the compositions of U1 (the relationships through a cast, the grid functions of `tpose` and `tpcpoint`, `expandSpace`) as A7 states those of the grid functions |

### JMEOS

Every JMEOS PR changes the Spark and Flink arms together, each surface proven by MobilitySpark
and MobilityFlink built with the branch generator.

| PR | Topic |
|---|---|
| J9 | Both engines read an enum argument from its text through the public catalog function returning that enum from a string (`null_handle_type_from_string`, `interptype_from_string` once G22 publishes it); Spark gains it, Flink keeps to public parsers |
| J10 | Spark passes the count out-parameters the catalog states (`shape.outParams`), as Flink does |
| J11 | Both engines take a `text` argument: a `text *` through `text_in`, an endian text through the value the catalog states for it |
| J12 | Both engines take the `bytea` of `<type>FromBinary` and `<type>FromEWKB`, passing its length |
| J13 | Flink carries `geometry` and `geography` through the reader and writer A6 states for each SQL type |
| J14 | Flink carries the cells `h3index`, `quadbin`, `s2cell` |
| J15 | Flink resolves an SQL argument count other than the C one, the bound `NORMALIZE`, and a base value in the distance functions |
| J16 | Both engines render the compositions A7 and A10 state |
| J17 | Spark carries `TPCBox`, `PoseChain` and `Raster`; a raster travels as the raster WKB `asBinary(raster)` writes, the GDAL readers (`rasterValue(tgeompoint, path text, band)`, `atRasterValue`, `minusRasterValue`, `eRasterValue`, `aRasterValue`, `raquetRead(path, quadbin)`) are how an engine without PostGIS reads a raster file, and both engines carry the raster operations of G23 |
| J18 | Both engines carry the aggregates the catalog states (A9) |
| J19 | A parity ledger per engine, keyed on MobilityDB's SQL signatures, that CI holds and that only shrinks, as the Spark gaps ledger does |

### Order

J9 first (it also clears JMEOS's failing gaps check), then J10–J12, which bring Spark and Flink
to one base; G19–G22, A9 and A10 next, since they reach both engines; then J13–J18; Part III's
renames land on that base. J3 waits on decision 0.13; the forms of G23 taking an expression or a callback wait on decision 0.14.

## Part V — the native geometry operations under their plain names

[Family 12](12-NATIVE-GEOMETRY.md) states the operations MEOS answers natively without a plain
SQL name, the nine decisions they rest on, what each decision requires of the lifting
infrastructure, and the commits of the one MobilityDB pull request that delivers them, the
lifting kernel's equality and the dimension of the base relationships first. Its `@altsqlfn`
names (`geoX`, `cbufferX`) reach Flink and Spark with the tags of R9 and A1.

## Acceptance of the whole

- Every pull request of the plan names its branch in its row or its family document, and these
  notes record the state of the branch before each push of it: what is done, what is measured,
  what is left. A session works from these notes as GitHub holds them, never from a copy.

- Before PR R2: the harness of 0.3, its per-signature table kept with the PR.
- Each MobilityDB PR: strict-ci and CI green; the manual builds with
  `dblatex -s texstyle.sty -p dblatex.xsl` in both languages (the CMake documentation targets
  configure since #2859).
- After F1/S1/D1: the example query of the porting section runs unchanged on MobilitySpark and
  MobilityFlink and gives MobilityDB's answer.
- After G1–G3, G18, A3, A4, A7, J4–J7: `audit_setof.py` over the catalog and jar of the new master reports
  every set-returning signature carried by the Flink SQL and the Spark surfaces under its SQL name.
- After PRs G13–G14, G6, G8, G9 and A6: every type round-trips through every format it supports,
  `<type>From<Format>(as<Format>(v))` returning `v`, in PostgreSQL and through each binding, and
  the I/O matrix (`io_matrix.py`) reads yes in every cell but the n/a ones.
- After Part IV: J19's ledgers are empty for Flink and Spark, but for a signature whose meaning
  is undefined for an engine, each argued and decided on its own.
