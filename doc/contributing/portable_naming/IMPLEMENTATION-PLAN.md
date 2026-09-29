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
`0be1b51060`, MEOS-API `20efba81`, JMEOS `e47aa744`, 2026-09-29.

## Where things stand

- **The documents and the manual draft are MobilityDB PR #2861** (draft): these notes in
  `doc/contributing/portable_naming/`, and the 18 manual files the draft changes: the portable
  chapter in sections with its porting section, the class-prefixed topology, route and distance
  names, and the entries for `<type>FromText`, `asText` and `geoLength`. Each PR below takes its
  own part of the manual draft.
- **`@altsqlfn` is a Doxygen tag only.** It sits on `Set_constructor` and `Value_to_set`
  (`setMake`, #2736). MEOS-API does not read it (0 files mention it, against 3 for `@sqlaggfn`),
  so no binding sees an alternative name yet.
- **MEOS lacks the C functions these PRs add**: `geo_length` (G17); the affine family over
  `geometry`, `geo_affine`, `geo_rotate`, ... (G11); `tquadbin_grid_distance`,
  `ts2cell_grid_distance` (G12). `text_lower`, `text_upper`, `text_initcap`, `geo_round`,
  `geo_transform_pipeline`, and the readers `temporal_in`, `set_in`, `span_in`, `spanset_in`,
  `tbox_in`, `stbox_in`, `quadbin_in`, `s2cell_in`, `nsegment_in`, `tpcbox_in`, `raquet_in` exist.
- **Set-returning functions** ([10](10-SET-RETURNING.md)): the catalog states each set-returning
  signature, the columns of its rows and the C value feeding each (A3, A4), and the compositions
  over another type's functions (A7); the Spark generator registers each set-returning signature
  as an array of its rows (J7). The Flink surface waits on J4.
- **Input and output**: of the 59 types with the four PostgreSQL I/O functions, 24 lack
  `FromText` (`raquet` takes its text as hex WKB, rule 9) and `nsegment`, `quadbin`, `s2cell`,
  `tpcbox` read and write their text form (G14, #2901); G13 closes the `FromText` gap. The
  catalog's `typeEncodings` states, class by class, the reader and writer of each SQL type (A6).
- **Open pull requests** on 2026-10-01: MobilityDB #2861 (these notes) and #2905 (G11); #2906
  states the deployed SQL names in the `@sqlfn` tags of the span, set and temporal geo wrappers and
  renames no function; #785, #777 and #677 touch none of these names or functions; MEOS-API has
  none.

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
| G10 | Call MEOS where the affine family composes in SQL | `translate(t, dx, dy)` and `scale(t, xf, yf)` as the 3D C-backed forms with the defaults `deltaz` 0 and `zfactor` 1 the catalog states; `rotate(t, angle, geometry)` over a MEOS rotation about a point, not PostGIS `ST_X`/`ST_Y` ([4](04-SPATIAL-TRANSFORMATIONS.md), decision 3) | nothing |
| G11 | Give `geometry` the affine family and refuse geodetic values | MEOS `geo_affine`, `geo_affine_2d`, `geo_rotate`, `geo_rotate_x`, `geo_rotate_y`, `geo_rotate_z`, `geo_scale`, `geo_scale_xyz`, `geo_translate`, `geo_transscale` over `lwgeom_affine`; SQL `geoTranslate`, `geoAffine`, `geoRotate`, `geoRotateX`, `geoRotateY`, `geoRotateZ`, `geoScale`, `geoTransscale` over `geometry`, the 15 PostGIS signatures; the `tgeo_*` and `geo_*` affine functions refuse a geodetic value (in the source, `tgeo_type_all` admits `tgeogpoint` and `tgeography` and the path reaches the planar `lwgeom_affine` with no geodetic check; a smoke test calling `tgeo_translate` on a `tgeogpoint` first establishes what it does); smoke tests, the entries ([4](04-SPATIAL-TRANSFORMATIONS.md), decision 4) | open, #2905 |
| G12 | Give the quadbin and S2 cells their grid distance | MEOS `tquadbin_grid_distance`, which `meos_quadbin.h` declares commented out under "Grid traversal + metrics", the smallest `k` whose `quadbin_grid_disk(origin, k)` holds the destination, as H3's grid distance is to its grid disk; SQL `tquadbinGridDistance` and the `<->` over `tquadbin` it backs, as `th3GridDistance` backs `<->` over `th3index` (`285_th3index_traversal.in.sql`); the same for S2, `ts2cell_grid_distance` and `ts2GridDistance`, the number of steps between edge neighbours of one level (`s2cell_edge_neighbors`), the neighbour coming from the family's own function; smoke tests, the entries | nothing |
| G13 | Read every type from its text: `<type>FromText` (rule 9) | one wrapper per family taking its type from `get_fn_expr_rettype`, as `Tspatial_from_ewkt` does, over the family's reader; SQL for the 24 types the [I/O plan](IO-PLAN.md) lists (the sets, spans and span sets of the base types, `jsonbset`, `textset`, `tbox`, `stbox`, `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`); `<type>_in` untouched; the entries `setspan_FromText`, `boxFromText`, `ttypeFromText` #2861 drafts | nothing |
| G14 | Give quadbin, s2cell, nsegment and tpcbox their text output and input | MEOS `quadbin_out` beside `s2cell_out`; SQL `asText` and `<type>FromText` for each, and for `nsegment` `asEWKT` and `nsegmentFromEWKT`, as `npoint` has them; the entries `cell_asText`, `cellFromText`, `tpcbox_asText`, `tpcboxFromText`, `asText(nsegment)` and `nsegmentFromText` in the network point chapter #2861 drafts | merged, #2901 |
| G15 | Give the function `merge` the point cloud types (family 8) | `merge(tpcpoint, tpcpoint)`, `merge(tpcpatch, tpcpatch)` and their array forms over `temporal_merge`/`temporal_merge_array`; tests | nothing |
| G16 | Document the set unnest and read the rigid geometry unnest from MEOS (family 9) | entry `setspan_unnest` EN+ES for the 18 set declarations; `Trgeometry_unnest` calls `trgeometry_unnest` once both answer alike | merged, #2903 and on master |
| G17 | Measure a geometry or geography in MEOS | MEOS `geo_length` over `geom_length`/`geog_length` (spheroid, as `ST_Length`), the function family 5 names; smoke tests | nothing |
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

### MobilityDB

| PR | Topic | What it changes | Size (measured) |
|---|---|---|---|
| R1 | Give the portable dialect chapter one section per operator family | `doc/portable_sql.xml` EN+ES as #2861 drafts it: sections as the family chapters title theirs, full-width tables in the normal font, the Same row in the topology section, the route operators beside it; no function renamed | doc only |
| R2 | Name the topological functions by the class they compare (family 6) | the generator (`topop_families`, `topops.sql.tmpl`), the `@sqlfn` tags, the planner's topology name table in `temporal_supportfn.c` as per-prefix macros like the position ones, `tools/codegen/portable_aliases`, the function-form tests (`010_span_ops_supportfn_tbl`, `044_temporal_indexes_tbl`), the chapter rows and migration table | 729 signatures, 31 names; waits on 0.3 |
| R3 | Spell the route identifier functions without an underscore | `same_rid`→`sameRid`, `contains_rid`→`containsRid`, `contained_rid`→`containedRid`, `overlaps_rid`→`overlapsRid`; `310_tnpoint_routeops.in.sql`, the wrappers' `@sqlfn` tags, the chapter row | 20 declarations |
| R4 | Name the span and span set distance functions by class | bare `distance` over spans and span sets → `spanDistance` / `spansetDistance` by the first operand that is not a base value, as `spanLeft`/`spansetLeft`; `distance(cbuffer, …)` and `distance(pose, …)` keep their name (documented, called by users); the chapter row | 40 declarations |
| R5 | Name the hash functions by class (family 7) | `hash`/`hashExtended` → `setHash`, `spanHash`, `spansetHash`, `tboxHash`, `stboxHash`, `temporalHash`, base types their own; the opclass `FUNCTION 1/2` lines; the entries `setspan_hash`, `box_hash`, `ttype_hash`, `tcell_hash` | 58 + 58 signatures |
| R6 | Measure a geometry or geography: `geoLength` (family 5) | SQL `geoLength(geometry|geography)` over the MEOS `geo_length` of G17; entry `tgeo_length` | 2 signatures |
| R7 | Change the case of a text value: `textLower`, `textUpper`, `textInitcap` (family 1) | SQL over `text_lower`/`text_upper`/`text_initcap` beside PostgreSQL's own `lower(text)` | 3 signatures |
| R8 | Transform a geometry or geography by a pipeline: `geoTransformPipeline` (family 4) | SQL over `geo_transform_pipeline` | 2 signatures |
| R9 | State the Flink and Spark name of every colliding function (`@altsqlfn`, families 1–5, 8 and 9) | the tag on each PostgreSQL wrapper (`floatRound`, `geoRound`, `spanLower`, `temporalLowerInc`, `intAbs`, `floatCeil`, `geoTransform`, `geoTranslate`, `geoAffine`, `geoRotate`, `geoRotateX`, `geoRotateY`, `geoRotateZ`, `geoScale`, `geoTransscale`, `npointLength`, and for families 8 and 9 `temporalInsert`, `temporalUpdate`, `temporalMerge`, `setUnnest`, `temporalUnnest`, …); no SQL declaration (0.4) | about 322 wrappers |
| R10 | Document porting an application to Spark and Flink | the `portable_sql_porting` section #2861 drafts (the changes table, among them the typed literal, the text of a value, the cast; the names table; the example), once PR R9's names exist | doc only |

R3, R4 and R5 are independent of R2 and can run in parallel with it; R6–R8 are independent of
each other; R9 needs R5–R8 (their functions carry tags), and R10 needs R9.

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

## Acceptance of the whole

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
