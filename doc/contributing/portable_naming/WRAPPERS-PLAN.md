<!--
  MobilityDB — Portable Naming: The SQL functions MEOS answers
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# The SQL functions MEOS answers

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md) · [I/O plan](IO-PLAN.md)

A binding generated from the MEOS catalog reaches a MobilityDB SQL function only when the catalog
states that function's signature. This document lists every SQL signature the catalog does not
state, says which of them are out of the bindings' scope, and gives the public MEOS function each
of the others needs. MobilityDB's SQL surface does not change: every SQL function keeps its name,
signature and answer, and only the C behind it moves into MEOS.

## How a binding reaches a SQL function

The `@csqlfn` tag of a MEOS function names the PostgreSQL wrappers that call it. MEOS-API reads
every wrapper a tag names (`_meos_to_mdb` in `parser/sqlfn.py`), and gives each wrapper's
`CREATE FUNCTION` signatures to that function as its `sqlSignatures`. Each signature carries its
own SQL name. `right_set_set`, tagged `@csqlfn #Right_set_set(), #After_set_set()`, therefore
states `setRight(intset, intset)` and the other number sets, and `setAfter(dateset, dateset)`
and `setAfter(tstzset, tstzset)`. `mdbC` and `sqlfn` name the first wrapper only, as the
representative of the function.

Three readings complete the rule:

- **Type scope.** A function keeps only the signatures of the types it accepts. `@csqlfn` of
  `eintersects_tgeo_geo` names `#Eintersects_tnpoint_geo()`, but the function refuses a
  `tnpoint` (`VALIDATE_TGEO`), so `eIntersects(tnpoint, geometry)` is not one of its signatures.
- **Compositions.** A wrapper that converts an argument through a public cast and then calls a
  public function is stated in the catalog's `compositions`:
  `eIntersects(tnpoint, geometry)` is `eintersects_tgeo_geo(tnpoint_to_tgeompoint(t), g)`.
- **Bindings.** The generators read `sqlSignatures` and `compositions`, signature by signature:
  `published_name` in JMEOS `tools/codegen_spark_udfs.py` for Spark and Flink, `signatures` in
  `tools/codegen_jvm.py` for the typed surfaces.

A wrapper census that reads `mdbC` alone counts every wrapper named after the first position as
unlinked (203 wrappers on master), and counts a wrapper as linked when the type scope drops some
of its signatures. The census below is therefore taken by signature: a SQL signature is reached
when the catalog states it, as a signature of a public function or as a composition.

Measured at MobilityDB master `1cbb831a57` with MEOS-API master `fa5f240`. The signatures are
those `_wrapper_sql_sigs` of MEOS-API `parser/sqlfn.py` reads from the `CREATE FUNCTION`
statements of `mobilitydb/sql`, keyed by SQL name (case-insensitive) and argument types; a
signature is stated when an entry of `sqlSignatures` of a public function (its `sqlName`, else
the function's `sqlfn`) or of `compositions` (whose `call` is public) carries the same key. The
support functions are those whose wrapper or SQL name matches the type, index, planner and
aggregate support suffixes (`_in`, `_out`, `_recv`, `_send`, `_sel`, `_gist`, `_transfn`, ...).

| | wrappers | SQL signatures |
|---|---|---|
| bound to a C wrapper | 2820 | 8765 |
| stated by the catalog | | 8296 |
| not stated | 301 | 469 |
| of which PostgreSQL's type, index, planner and aggregate support | 112 | 176 |
| of which out of the bindings' scope | 38 | 81 |
| of which the writers of an array, left open | 3 | 40 |
| **of which this plan's** | **148** | **172** |

The 172 signatures fall into three groups: 46 need a tag only, 58 need the wrapper to call the
public function, and 68 need a new public MEOS function. The appendix lists each one with its
wrapper and the MEOS function it reaches.

## The rule

1. **Every SQL function a binding projects answers through one public MEOS function** that its
   wrapper calls. The function validates its arguments at entry; the wrapper only reads its
   arguments and returns the result.
2. **The function is named and shaped like its sibling** on master, the same operation over the
   neighbouring family. Where the internal kernel takes an `invert` flag, there is one public
   function per argument order, as with `contains_set_bigint(const Set *, int64)` and
   `contained_bigint_set(int64, const Set *)`. It is compiled in every build, the PostgreSQL
   extension's and MEOS's, in the file that defines the kernel it calls
   ([The typed public functions build in every build](#the-typed-public-functions-build-in-every-build)).
3. **The tag states what the wrapper reaches.** A MEOS function names in its `@csqlfn` every
   wrapper that reaches it, and only those: directly, through the wrapper's `_common` helper, or,
   for an ever or always relation, through the kernel the wrapper passes to `EA_spatialrel_*`, as
   `acontains_geo_tcbuffer` names `Acontains_geo_tcbuffer`, which passes `ea_contains_geo_tcbuffer`
   with `ALWAYS`. The catalog then states every signature of those wrappers that the function
   accepts.

## Out of the bindings' scope (38 wrappers, 81 signatures)

- PostgreSQL's range and multirange conversions (`span(int4range)`, `range(intspan)`,
  `spanset(int4multirange)`, `multirange(intspanset)`), the GIN support of sets and network
  points, the typmod enforcement of the temporal types, and `fill_oid_cache`.
- The constructors of the PostgreSQL geometric types (`point`, `line`, `lseg`, `box`, `circle`,
  `path`, `polygon`), `transformGK`, the compatibility link to SECONDO, and `createTrip`, the trip
  generator of the BerlinMOD benchmark. They stay in MobilityDB as they are.
- `_trgeometry_geom_clip_polygon` and `_trgeometry_geom_clip_polygon_posed`, whose leading underscore
  declares them internal, as the owner ruled for the underscore names.
- The procedures of the operators over a function with an optional argument. DECIDED BY THE
  OWNER 2026-10-23: an operator over a function whose last arguments carry a default takes the
  function's two-argument procedure `<function>Opr`, which answers the function with its defaults,
  as PostgreSQL backs `@?` by `jsonb_path_exists_opr` beside `jsonb_path_exists`; the function is
  the one surface of the bindings, and a caller passes other values to the function. The geo
  distances: `<->` and `|=|` over geometry and geography alike take `tDistanceOpr`,
  `nearestApproachDistanceOpr` and `setDistanceOpr` (wrappers `Tdistance_tgeo_geo_opr`,
  `NAD_tgeo_stbox_opr`, `Distance_geoset_geo_opr`, ...), in place of `tDistanceOp`,
  `nearestApproachDistanceOp` and `setDistanceOp` over geography alone; branch
  `sql/geo-distance-operators-take-their-procedures`, 43 operators, the catalog stating the same
  8710 signatures and 1418 operators as over master, and the GiST indexes ordering by `|=|` over
  both types. The JSON operators already take their `…Opr` procedures (19). No MEOS function
  names an `_opr` wrapper, so no binding registers an `Opr` name: MobilityDuck registers each
  operator on the two-argument form of its function, from the `@sqlop` of the function's wrapper,
  which `check_sqlop_tags.py` reads as the operator the SQL binds to the wrapper's `_opr` twin.
  The rule rests on three facts, each re-run from the tree: PostgreSQL 18 backs `@?` by the
  two-argument `jsonb_path_exists_opr` beside the four-argument `jsonb_path_exists`
  (`share/postgres.bki`), and PostGIS 3.6.4 backs `<->` by `geography_distance_knn(geography,
  geography)` beside `ST_Distance(geography, geography, use_spheroid boolean DEFAULT true)`
  (`postgis--3.6.4.sql`); on master `f5017dbabf` all 62 signatures of the 22 `…Opr` names are
  their base function called with its defaults; and all 33 `_opr` wrappers compute what their
  base wrapper computes. `check_csqlfn.py --targets`, a CI job, refuses an `@csqlfn` naming an
  `…Opr` procedure and an `…Opr` signature that is not its base call.

## Covered elsewhere

- **The text and binary input and output.** On master every reader and writer of a single value
  is stated, as the second wrapper of the type's `_in`, `_out` or `_as_wkb` function
  (`Quadbin_as_text` of `quadbin_out`, `Tpcbox_as_wkb` of `tpcbox_as_wkb`, `Set_from_text` of
  `intset_in` and its siblings).
- **The writers of an array**, 3 wrappers and 40 signatures (`asText(<temporal>[])`,
  `asText(<spatial>[])`, `asEWKT(<spatial>[])`), whose `temparr_out` and `spatialarr_wkt_out`
  are internal. DECIDED BY THE OWNER 2026-10-22: they stay open, outside this plan.
- **The pcid and point counts of the temporal point clouds**: merged as MobilityDB #3137
  (`1cbb831a57`).
- **The cell instant constructors** `th3index(h3index, timestamptz)`,
  `tquadbin(quadbin, timestamptz)`, `ts2cell(s2cell, timestamptz)`: branch
  MobilityDB #3149, merged as `ebc0e1debb`.

## The tag only (44 wrappers, 46 signatures)

The wrapper already calls a public MEOS function, and the function's `@csqlfn` gains the
wrapper. Where the wrapper passes a literal, the catalog records it as a bound argument of that
signature, as `jsonbPathExistsTz` carries `tz = true` on `jsonb_path_exists`.

- **JSON**: the `Opr`, `Text`, `TextOpr` and `Tz` forms of `tjsonb`, `tjson` and `jsonbset`
  (`tjsonbObjectFieldText`, `jsonbsetPathMatchTz`, ...) and `tjsonbSetLax`, `jsonbsetSetLax`
  (28 wrappers).
- **`tjsonbContained`** over `(jsonb, tjsonb)`, `(tjsonb, jsonb)` and `(tjsonb, tjsonb)`: the
  wrappers call `contains_tjsonb_*`, which answers `@>` where `<@` is asked. Branch
  MobilityDB #3146 (merged as `89ec3ab98d`) corrects the call, and the tag follows the
  corrected call.
- **Constructors**: `stbox(box2d)` (`gbox_to_stbox`), `pcpoint(integer, double precision[])`
  (`pcpoint_make`), the three `pose` constructors (`pose_make_2d`, `pose_make_point2d`,
  `pose_make_point3d_ypr`), the five `tpcbox` constructors (`tpcbox_make`),
  `trgeometry(geometry, tpose)` and `trgeometrySeqSetGaps`.
- **`geometry(trgeometry)`** (`trgeometry_geom`).

## The wrapper calls the public function (44 wrappers, 58 signatures)

The public function exists, but the wrapper reaches it through an internal function or builds
the call itself. The wrapper calls the public function, which names it in its `@csqlfn`.

- **Geometry first, over `tcbuffer`**: `eDisjoint`, `aDisjoint`, `eIntersects`, `aIntersects`,
  `eTouches`, `aTouches`, `eDwithin`, `aDwithin` with a geometry or a `cbuffer` first reach the
  symmetric public function (`edisjoint_tcbuffer_geo`, ...) through an internal `ea_*` function
  or call it without being named by it (16 wrappers). Each gains the public function of its own
  argument order (Circular buffer, below); a wrapper reaching its kernel through
  `EA_spatialrel_*` still states its order, since the catalog at master `ce124289e6` gives
  `edisjoint_geo_tgeo` `[geometry, tgeometry]` and `acontains_geo_tcbuffer`
  `[geometry, tcbuffer]` from their `@csqlfn` alone. The 6 geometry-first wrappers over
  `trgeometry` belong to the rigid geometry relations, the last task of the campaign.
- **Box constructors**: `stboxT`, `stboxXT`, `stboxZ`, `stboxZT`, `geodstboxZ`, `geodstboxZT`
  (`stbox_make`), `tbox(numeric)` (`float_to_tbox`).
- **`spanDistance(span, spanset)`** and **`nearestApproachDistance(tbox, tnumber)`**: the typed
  functions `distance_intspanset_intspan`, `nad_tint_tbox` and their siblings.
- **JSON paths**: `tjsonbExtractPathText`, `tjsonbExtractPathOpr`, the `jsonbset` and `tjson`
  forms (`tjsonb_extract_path`, `jsonbset_extract_path`, `tjson_extract_path`).
- **Others**: `point(pose)` (`pose_to_point`), `trgeometrySeq` and `trgeometrySeqSet`
  (`trgeometryseq_make`, `trgeometryseqset_make`), and `dumpValues`, `valueCount`,
  `valuePercent` over `raster` (`raster_dump_values`, `raster_value_count`).
- **The rigid geometry sequence constructors** `trgeometrySeq`, `trgeometrySeqSet` and
  `trgeometrySeqSetGaps` call `Tsequence_constructor`, `Tsequenceset_constructor` and
  `Tsequenceset_constructor_gaps`, as every other temporal type does: `tsequence_make`,
  `tsequenceset_make` and `tsequenceset_make_gaps` take the reference geometry from their
  first instant or sequence, so the catalog gives the three `trgeometry[]` signatures to the
  generic functions every binding generates, where `trgeometryseq_make` and its siblings take
  a geometry no SQL argument fills. Merged as MobilityDB #3173 (`b669b4a4d9`), which rewrites `e2e88cd1fd`
  (`80ddf4b569` on master `9df18a5847`), 150_trgeo unchanged and the error on instants with
  different reference geometries added; the CGAL oracle agrees with eDwithin over 300 AIS trip
  pairs rebuilt through `tsequence_make`. Instants carrying no reference geometry, as
  `temporal_instants` answers, are refused; `trgeometry_instants` and `trgeometry_sequences`
  answer the elements that carry it. Of `e2e88cd1fd` (MobilityDB #3166) the
  `@csqlfn` tags naming `Trgeometry_seq_constructor`, `Trgeometry_seqset_constructor` and
  `Trgeometry_seqset_constructor_gaps` on `trgeometryseq_make`, `trgeometryseqset_make` and
  `trgeometryseqset_make_gaps` go with those wrappers, by the owner's ruling of 2026-10-25 that the
  rigid geometry sequence constructors are no different from those of every other type.

## The MEOS functions to add (60 wrappers, 68 signatures)

One pull request per family. Every wrapper keeps its SQL answer, which the regression suite
proves unchanged.

### Network point route operations (20)

`npoint/tnpoint_routeops.h` is internal and takes `bool invert`. The public functions take one
argument order each, after `contains_set_bigint`, `contained_bigint_set`, `contains_set_set`,
`overlaps_set_set` (`meos.h`) and `contains_set_npoint` (`meos_npoint.h`). Branch
`meos/npoint-public` (`6734c74994`).

- `containsRid`: `contains_rid_tnpoint_bigint`, `contains_rid_tnpoint_bigintset`,
  `contains_rid_tnpoint_npoint`, `contains_rid_bigintset_tnpoint`, `contains_rid_tnpoint_tnpoint`
- `containedRid`: `contained_rid_bigint_tnpoint`, `contained_rid_bigintset_tnpoint`,
  `contained_rid_npoint_tnpoint`, `contained_rid_tnpoint_bigintset`,
  `contained_rid_tnpoint_tnpoint`
- `overlapsRid`: `overlaps_rid_bigintset_tnpoint`, `overlaps_rid_tnpoint_bigintset`,
  `overlaps_rid_tnpoint_tnpoint`
- `sameRid`: `same_rid_bigint_tnpoint`, `same_rid_bigintset_tnpoint`, `same_rid_npoint_tnpoint`,
  `same_rid_tnpoint_bigint`, `same_rid_tnpoint_bigintset`, `same_rid_tnpoint_npoint`,
  `same_rid_tnpoint_tnpoint`

### Circular buffer (2)

Branch `meos/cbuffer-public` (`bbbed77632`).

- `eContains(geometry, tcbuffer)`: `econtains_geo_tcbuffer`, after `acontains_geo_tcbuffer`
- `aTouches(tcbuffer, tcbuffer)`: `atouches_tcbuffer_tcbuffer` is declared in `meos_cbuffer.h`
  under `@ingroup meos_internal_geo_rel_ever` and moves to the public group of its sibling
  `etouches_tcbuffer_tcbuffer`
- The 16 static-first ever and always relationships (`edisjoint_geo_tcbuffer` ...
  `adwithin_cbuffer_tcbuffer`) and `tdwithin_cbuffer_tcbuffer`: each answers its temporal-first
  twin with the arguments swapped, as `edisjoint_geo_tgeo` answers `edisjoint_tgeo_geo`, the
  orthogonality rule of the spatial relationships; `check_spatialrel_orthogonality.py` reads the
  tcbuffer grid beside the tgeo one (108 cells)

### Rigid geometry (11)

- `eContains`, `aContains(trgeometry, geometry)`: `econtains_trgeometry_geo`,
  `acontains_trgeometry_geo`, after `ecovers_trgeometry_geo`
- `eContains`, `aContains`, `eCovers`, `aCovers`, `eTouches`, `aTouches(trgeometry, trgeometry)`:
  `econtains_trgeometry_trgeometry` and its five siblings, after `edisjoint_trgeometry_trgeometry`
- `valueAtTimestamp(trgeometry, timestamptz)`: `trgeometry_value_at_timestamptz`, after
  `tgeo_value_at_timestamptz`

The eight relations, and every other wrapper of a rigid geometry relation, are the last task of
the campaign: their kernels answer over the union of the placements, and an assessment of whether
an exact algorithm with the correct semantics exists comes first. No such wrapper is published or
rerouted before it.

### Geometry (4)

Branch `geo/sql-overlays-answer-exactly`. DECIDED BY THE OWNER 2026-10-23 ("add public"): the four
SQL polygon overlays answer through the exact native overlay of MEOS, each by a public function of
two operands that names its wrapper in `@csqlfn`.

- `intersection(geometry, geometry)`: `geom_intersection2d`
- `difference(geometry, geometry)`: `geom_difference2d`
- `geoUnion(geometry, geometry)`: `geom_union2d`, new, after `geom_difference2d`, the dissolve of
  `geom_array_union` over the two operands
- `symDifference(geometry, geometry)`: `geom_symdifference2d`, new, after `geom_difference2d`, that
  dissolve over the two differences

### JSON sets (2)

Branch `meos/json-public` (`f03e888159`).

- `textset(jsonbset)`: `jsonbset_to_textset`, after `tjsonb_to_ttext`
- `jsonbset(textset)`: `textset_to_jsonbset`, after `ttext_to_tjsonb`

### Sets of the extension types (3 wrappers, 9 signatures)

`setDistance` over `cbufferset`, `npointset` and `poseset` reaches only the internal
`distance_set_set`, `distance_set_value` and `distance_value_set`; the public typed functions
cover the number, time and geo sets only.

- `setDistance(cbufferset, cbufferset)`, `(npointset, npointset)`, `(poseset, poseset)`:
  `distance_cbufferset_cbufferset`, `distance_npointset_npointset`, `distance_poseset_poseset`,
  after `distance_geoset_geoset`
- `setDistance` between such a set and a value, in either order: `distance_set_cbuffer`,
  `distance_set_npoint`, `distance_set_pose`, the set first as in `distance_set_geo`

### Temporal numbers and pose chains (3)

- `tDistance(tbigint, bigint)` and `tDistance(bigint, tbigint)`: `tdistance_tbigint_bigint`,
  after `tdistance_tint_int`
- `valueAtTimestamp(tposechain, timestamptz)`: `tposechain_value_at_timestamptz`, after
  `tpose_value_at_timestamptz`

### Point clouds (16)

- `SRID(pcpoint)`, `SRID(pcpatch)`: `pcpoint_srid`, `pcpatch_srid`, after `tpcbox_srid`
- `getX`, `getY`, `getZ(tpcpoint)`: `tpcpoint_get_x`, `_y`, `_z`, after `tpoint_get_x`
- `getDim(tpcpoint, text)`: `tpcpoint_get_dim`, after `pcpoint_get_dim`
- `points(tpcpatch)`: `tpcpatch_points`, after `pcpatch_points`
- `eIntersects(tpcpatch, geometry)`: `eintersects_tpcpatch_geo`, after `eintersects_tpcpoint_geo`
- `atTpcbox`, `minusTpcbox(tpcpoint, tpcbox)`: `tpcpoint_at_tpcbox`, `tpcpoint_minus_tpcbox`,
  after `tgeo_at_stbox`
- `atTpcbox`, `minusTpcbox`, `atTpcboxFine`, `minusTpcboxFine(tpcpatch, tpcbox)`:
  `tpcpatch_at_tpcbox`, `tpcpatch_minus_tpcbox`, `tpcpatch_at_tpcbox_fine`,
  `tpcpatch_minus_tpcbox_fine`
- `atGeometry`, `minusGeometry(tpcpatch, geometry)`: `tpcpatch_at_geom`, `tpcpatch_minus_geom`,
  after `tgeo_at_geom`

`atTpcbox` keeps a patch whole when its bounds overlap the box; `atTpcboxFine` keeps only the
points inside the box. The patch-level test reads the patch bounds in x and y only, so a box with
Z does not filter by Z; MobilityDB #3150 (merged as `71e333da59`) fixes it before the move.

## Consistency of the tags

- **The jsonpath functions follow the twin convention of the vendored PostgreSQL code.**
  `jsonb_path_exists`, `jsonb_path_match`, `jsonb_path_query_array` and `jsonb_path_query_first`
  are names PostgreSQL's executable exports, so each is a twin pair, as `numeric_mul` is: the
  clean name, compiled under `#if MEOS`, forwards to `pg_jsonb_path_*`, which both builds compile,
  and the PostgreSQL wrappers call the `pg_` twin. The catalog binds the wrappers' literals
  through the twin (MEOS-API #175), so it states `jsonbPathExists` with `tz = false` and
  `jsonbPathExistsTz` with `tz = true`, and the bindings, built over libmeos, call the clean
  names. `jsonb_exists_array` and `jsonb_path_query_all` collide with no PostgreSQL name, so they
  are compiled unguarded in both builds and called by their wrappers, as `bool_in` and `text_in`
  are: branch `meos/jsonpath-outside-meos-gate`.
- **One list per function, every wrapper that computes it.** DECIDED BY THE OWNER 2026-10-24
  ("according to North Star no exceptions"): a public MEOS function names in one `@csqlfn` list
  every PostgreSQL wrapper whose result is its computation, and a new overload appends its wrapper
  to that list. On master `f5017dbabf`, 382 of the 3538 tagged functions name two to seven
  wrappers, each in one comma-separated list (23 wrapped onto a continuation line), and no
  function splits its wrappers over two `@csqlfn` lines. A `recv` wrapper is named (13 on
  master), a cast is named on the function it calls, GiST, SP-GiST and GIN support functions are
  named nowhere (0), and an `_opr` wrapper is named nowhere, by the ruling of 2026-10-23 under
  "Out of the bindings' scope"; the 26 `_opr` wrappers the JSON tags name on master (MobilityDB
  #3153, `8655303ab6`) deviate from that ruling. The merged-lines gate keeps a merged line that
  a line of the same hunk extends.
- **The containment of the JSON types.** DECIDED BY THE OWNER 2026-10-24: `tjsonbContained(a, b)`
  answers `tjsonbContains(b, a)`, its wrappers calling `contains_tjsonb_jsonb` and
  `contains_tjsonb_tjsonb` with the operands swapped, so `Contained_jsonb_tjsonb`,
  `Contained_tjsonb_jsonb` and `Contained_tjsonb_tjsonb` join the `@csqlfn` lists of those two
  functions and MEOS gains no `contained_*` function, a commuted copy the no-redundant-signature
  rule excludes. The catalog regenerated with those tags states the three signatures with the
  argument order and the flag each wrapper passes (`tjsonbContained(jsonb, tjsonb)`:
  `sqlArgParams [jb, temp]`, `invert = INVERT_NO`; `tjsonbContained(tjsonb, tjsonb)`:
  `[temp2, temp1]`). The same day's decision to add three public `contained_*` functions is
  withdrawn: it rested on the 49 `contained_*` functions of the other families, and the
  measurement shows the JSON wrappers reach every binding through the tags alone.
- **The operator of a second wrapper.** `sqlop` names the operator of the first wrapper only:
  `right_set_set` states `>>`, and the `#>>` of `setAfter` is stated nowhere. Stating the
  operator per signature closes it, for the bindings that register operators.

## The typed public functions build in every build

DECIDED BY THE OWNER 2026-10-26: a public MEOS function is compiled in every build, so the
PostgreSQL extension and the bindings call the same functions and the catalog reads what a
wrapper calls rather than inferring it. This replaces the rule of 2026-07-23 that kept the
per-type functions of the `*_meos.c` files out of the extension build: those files hold typed
public functions (`timestamptz_to_span`, `tint_in`, `contains_set_bigint`, ...), each a thin call
of a generic kernel, and their family's `CMakeLists.txt` lists them under `if(MEOS)`. The
coverage reason for that list is the one the `#if MEOS` gates of `meos/src` had, and a function a
binding or a wrapper calls is never MEOS-only.

**The rule.** Every public function is compiled in both builds. A wrapper whose SQL signature is
type-specific calls the typed public function, as `tbox(integer)` calls `int_to_tbox` (decided
2026-10-23): `Stbox_constructor` builds the period of `stboxT(timestamptz)` with
`timestamptz_to_span(t)`, where it calls `span_make(t, t, true, true, T_TIMESTAMPTZ)`, and the
catalog states the signature as the composition of `stbox_make` over that public cast. A wrapper
serving the signatures of several types at once (`Value_to_span`, which reads the type of its
argument) keeps calling the generic kernel. The generic kernels stay internal.

**The inventory on master `c1d030d810`:** 44 `*_meos.c` files define 1305 functions, every file
listed under `if(MEOS)` in the `CMakeLists.txt` of its family.

| family | files | functions |
|---|---|---|
| temporal | 24 | 971 |
| geo | 5 | 134 |
| pointcloud | 3 | 99 |
| pose | 2 | 34 |
| npoint | 2 | 27 |
| cbuffer | 1 | 16 |
| json | 3 | 14 |
| h3 | 2 | 4 |
| quadbin | 1 | 3 |
| s2cell | 1 | 3 |

**What is shared and what is each platform's.** A facility of both platforms has a part they
share and a part each of them holds, laid out the same way for every facility: the shared part in
`meos/src`, compiled in both builds; PostgreSQL's part in `mobilitydb/src`; MEOS's part in
`meos/src`, compiled for MEOS alone. The indexes are the reference: their consistency logic
(`span_index.c`, `tbox_index.c`, `stbox_index.c`, `tpcbox_index.c`, the `*_spgist.c` files) is
shared, PostgreSQL's GiST and SP-GiST support functions are in `mobilitydb/src`, and MEOS indexes its
values in memory with the `RTree` and `SPTree` of `temporal_rtree.c` and `temporal_sptree.c`, public
functions compiled in both builds. The route lookup of the network points (`route_exists`,
`route_geom`, `route_length`, `get_srid_ways`) is the one facility laid out otherwise:
`meos/src/npoint/ways.c`, PostgreSQL's lookup through SPI in the `ways` table, sits in `meos/src`
under the `else()` of `if(MEOS)`, beside `ways_meos.c`, MEOS's lookup in the CSV file
`meos_set_ways_csv` names. It moves to `mobilitydb/src/npoint`, and `ways_meos.c` stays MEOS's own;
no `*_meos.c` file holds index code.

**The two in-memory indexes are siblings** (DECIDED BY THE OWNER 2026-10-26, one pass without
overdoing). The `RTree` and the `SPTree` (quad-tree or k-d tree, an `SPTreeKind`) answer the same
API: a creator per key type (`intspan`, `bigintspan`, `floatspan`, `datespan`, `tstzspan`, `tbox`,
`stbox`, `tpcbox`), `free`, `num_entries`, `mem_size`, `height`, `insert`, `load`, `search`,
`join`, the temporal entry points `insert_temporal` and `search_temporal`, the multi-entry ones of
MEST (`insert_temporal_split`, `search_temporal_dedup`) and a nearest-neighbour cursor. The pass
aligns what still differs and renames no type (the owner, 2026-10-26: the cursors stay
`RTreeNNCursor` and `SPNNCursor`): the `RTree` creators are declared with `(void)` (branch
`meos/void-parameter-lists`, merged as MobilityDB #3231, `a2fdfd8bd9`, rewrites `bb3faeaf2f`: its declaration of `rtree_create_tpcbox()` becomes `rtree_create_tpcbox(void)`), and
`meos/examples` holds four programs, `rtree_example.c` and `rtree_mest_example.c`
and their `SPTree` siblings `sptree_example.c` and `sptree_mest_example.c`, each pair over the same
data and checks (branch `meos/sptree-examples`, merged as MobilityDB #3221, `e926fd6e68`, the R-tree, the
quad-tree and the k-d tree finding the same values for each of the 13 searches, every `contains`
check of `rtree_example.c` querying a stored box, and its `stbox` values taking the corners of the
text form). `rtree_create_tpcbox`, `sptree_create_tpcbox` and `meos_initialize_pointcloud` are
declared in `meos_pointcloud.h`, where a program compiled against the installed headers, which
define no family flag, reads them (branch `meos/sptree-tpcbox-declared`, `75d8c6a705` on master
`71282af6fc`, merged as MobilityDB #3207, `bb3faeaf2f`, `sptree_test` running its TPCBox quad-tree and k-d tree sections). The key of a type is a catalog fact, its bounding box, which `type_bboxtype`
of `meos_catalog.c` answers for every type, the owner's decision of 2026-10-26: a box type, a span
or a temporal, spatiotemporal or point cloud box, is its own; every other type is bounded by its
box, the span its bounds state for a number or time set and for a span set, and the box a spatial
set or a temporal value stores. A number or time set stores no box (`set_bbox_size`).
`meostype_from_string` answers the type of a canonical name, the inverse of `meostype_name`, so a
host reads both from MEOS; branch `meos/index-bboxtype-catalog` (merged as MobilityDB #3214, `16e84ed83e`; `ccfd61b4b5` on master
`30f7513014`), whose test checks the key of the 47 types and that no other type has one but the
internal states of the temporal averages. DECIDED BY THE OWNER 2026-10-26: every type with
a key is indexed in both hosts, 47 types: the 37 PostgreSQL's GiST and SP-GiST operator classes
index on master `c1d030d810`, `tposechain` and the nine spatial sets (`geomset`, `geogset`, `npointset`,
`poseset`, `posechainset`, `cbufferset`, `h3indexset`, `quadbinset`, `s2cellset`), whose `stbox`
the catalog states and which gain a GiST and an SP-GiST class over it, cloned from those of
`intset` (their `&&`, `@>`, `<@` and `=` answered from the box, rechecked). A pose chain reads as
the pose its composition gives (`posechain_prefix_pose`), so `tposechain` has every operation
`tpose` has (DECIDED BY THE OWNER 2026-10-27). Branch `pose/tposechain-distance` (merged as MobilityDB #3225,
`1fa64600db`) gives it the distance surface of `tpose`, the MEOS functions composing
through `tposechain_to_tpose` and the SQL from a `distance` template of the inherited generator
that writes `110_tpose_distance` unchanged; `datum_distance` answers for pose chains, so the
gaps of `tposechainSeqSetGaps` and `appendInstant` take a maximum distance (master raises
`Unknown types for distance between values: posechain`), and pose chain sets take `setDistance`.
`geo_round` rounds a geometry on a copy of it, as `geo_transform` does, since the geometry `lwgeom_from_gserialized` reads shares the coordinates of its serialization: rounding in place rounded the argument too, so `set_round` left a set holding rounded values under the box of the full ones, and the `round(g, 6)` queries of `050_set_tbl` broke the geometry and geography set indexes the later tests read (branch `fix/geo-round-reads-a-copy`, merged as MobilityDB #3228, `aa6168947d`). Branch `index/pg-every-keyed-type` (merged as MobilityDB #3230, `cdfa50524d`; it rewrites `1fa64600db`: the `files` list of `tposechain` in `subtypes.yaml` gains `indexes`, and the posechain lines of `INHERITANCE_MAP.md` name the index file `566` the distance PR stated absent) gives the nine sets and
`tposechain` an R-tree GiST class and quad-tree and k-d tree SP-GiST classes, a set's classes
written by the template of its temporal type's index file under the generator flags `setindex`
and `setmetric`, the geo sets' in `073_tgeo_gist` and `074_tgeo_spgist`, `<->` served for the sets
whose elements have a distance; a set file cannot carry them, its generated set operations and
distance owning its end. By the same definition `tposechain` answers the ever and always spatial
relationships of `tpose`, the 26 signatures of `112_tpose_spatialrels`, at the position of its
composed pose: the `spatialrels` template writes `562_tposechain_spatialrels` from the
`tposechain::tpose::tgeompoint` conversion, its ever intersects and dwithin being C wrappers with
`tspatial_supportfn` so the index of #3230 answers them (branch `pose/tposechain-spatialrels`, which rewrites `cdfa50524d`,
merged as MobilityDB #3232, `7303e1a47b`: the `files` list of `tposechain` in
`subtypes.yaml` gains `spatialrels`, and the posechain lines of `INHERITANCE_MAP.md` name the file
`562` the index PR stated absent). The same template then writes `112_tpose_spatialrels` itself
from the `tpose::tgeompoint` conversion, the file having been hand-written as exactly the hybrid
the template emits under `index_support`, with its 26 signatures, bodies and 6 C symbols unchanged
(branch `gen/tpose-spatialrels-template`, which rewrites `1fa64600db` and rewrites `7303e1a47b`, merged as MobilityDB #3238, `d39972281c`: the `tpose` entry of `subtypes.yaml` gains
`spatialrels`, `112` leaves `coverage_exceptions.txt`, and `INHERITANCE_MAP.md` states pose
generated). MobilityDuck's k-d tree comes first, its own pull request before the catalog
generates the types of the indexes (DECIDED BY THE OWNER 2026-10-31): `TSPTREE` with
`kind = 'kdtree'` reaches `sptree_create_*(SPTREE_KDTREE)` for each of the 15 types
`sptree_module.cpp` lists on main `c688dcd023`, yet its tests (077c, 078, 080, 084, 085) index
one column each beside a quad-tree twin, so the k-d tree is answered over the 15 types by a test
that finds, for each type and operator, the rows `TRTREE` and the quad-tree find. MobilityDuck's
`TRTREE` and `TSPTREE` are then the two trees and take the 47 types from the catalog rather than
from the 15 their modules list on main, the k-d tree with them; the tests run each index, and
each kind of `TSPTREE`, over each type in both hosts.
A set or span set is keyed by the extent of its elements, which its predicates do not compare, so
both hosts recheck a candidate the index returns for one: PostgreSQL through the classes' recheck,
MobilityDuck by keeping the matched filter above the index scan.

**Future tasks, each its own pull request after the indexes:**

- **The index join over every key.** MobilityDuck's `TRTREE` and `TSPTREE` index join serves a
  probe of an `stbox`-keyed index from an `stbox` or one of the four temporal point and geometry
  types, all answered exactly by their box. The same join serves every key, one clone per key the
  catalog states (`type_bboxtype`): an `stbox` for every spatial type, a set of them
  included, a `tbox` for the temporal numbers, and the span kinds for time values, number and
  time sets, spans and span sets, each probe keyed as its rows are and a lossy pair rechecked as
  the scan rechecks it.
- **MEST in MobilityDuck.** Its `TRTREE` and `TSPTREE` take a `maxboxes` option, the number of
  fragments a temporal value is split into, inserting with `*_insert_temporal_split` and searching
  with `*_search_temporal_dedup`, as both trees of MEOS do.
- **`bboxunion` removed.** The union of every box type sizes 25 buffers in MEOS and
  `TEMPORAL_MAX_HEADER_SIZE`, the bytes read from TOAST for the header and box of a temporal value,
  and holds `TPCBox` under `#if POINTCLOUD` alone, so that size depends on a family flag. Each
  buffer takes the size of the box of its type (`bbox_get_size`), the header size the largest box
  `bbox_get_size` answers whatever the families built, and the MFJSON writers a `const void *` box
  with the box type they read.
Of the other 43 files, 30 have a twin without the suffix
(`span_meos.c` and `span.c`); 13 have none (`cbufferset_meos.c`, `geoset_meos.c`, `tspatial_posops_meos.c`, `tspatial_topops_meos.c`,
`tspatial_transform_meos.c`, `npointset_meos.c`, `pcset_meos.c`, `tpc_posops_meos.c`,
`tpc_topops_meos.c`, `posechainset_meos.c`, `poseset_meos.c`, `span_tile_meos.c`,
`temporal_posops_meos.c`).

**Each pull request is one family, or one file of `temporal`.** It moves the functions of each
`*_meos.c` file into its twin, or renames the file without the suffix where it has none, and
removes the file from the `if(MEOS)` list, so no file name states a build any more. Its strict-ci
receipt builds both targets: a function calling something only MEOS provides fails the extension
build, and is resolved at that dependency, never by a gate; a symbol the extension shares with
PostgreSQL fails the extension's load, and is resolved as pgtypes resolves one, a `pg_` twin. The
wrappers of the family whose SQL signature is type-specific call the typed function in the same
pull request, and the catalog's census of the signatures it does not state measures the step.
No SQL function changes name, signature or answer.

**The order.** `temporal/span_meos.c` first, whose `timestamptz_to_span` gives J15d the period of
`stboxT`, `stboxXT`, `stboxZT` and `geodstboxZT` (MEOS-API then reads the composition through the
`Stbox_constructor` helper, deciding the type of its argument from the signature), then the other
files of `temporal`, then the families. The first file: MobilityDB #3248 (`cce310aca2`) moves the 39 functions of `span_meos.c` into `span.c`, each beside its siblings in the section of the same name, takes `span_meos.c` off the MEOS-only list, and gives `Stbox_constructor` the period of a `timestamptz` through `timestamptz_to_span`; the extension's library defines the 39 functions. The second: MobilityDB #3281 (`cc339df620`) moves the 32 functions of `spanset_meos.c` into `spanset.c` the same way and takes it off the MEOS-only list; the span set wrappers read the type of their argument, so none changes. The third: MobilityDB #3286 (`4b5f1d6d84`, renaming to `set.c` the six baseline keys `6e22d6c736` wrote) moves the 54 functions of `set_meos.c` into `set.c`, which gains the `utils/varlena.h` they read, takes it off the MEOS-only list, and moves the six findings `check_nullable_params.py` baselines for its value arrays to `set.c`, the baseline being keyed by file. The fourth, in flight: branch `meos/setops-every-build` (`bffb222b04` on master `cde4a92732`, rewrites `8016cb9837`, rewrites `b27acacddd`, merged as MobilityDB #3293, `340785dc65`) moves the 106 functions of `set_ops_meos.c` into `set_ops.c`, which gains the `float.h`, `limits.h` and `utils/timestamp.h` they read, and takes it off the MEOS-only list. The fifth, in flight: branch `meos/spanops-every-build` (`d1242f63d1` on master `cde4a92732`, rewrites `d74d30f15e`, rewrites `b18acfb81d`, merged as MobilityDB #3297, `7102fd6de2`) moves the 99 functions of `span_ops_meos.c` into `span_ops.c` and takes it off the MEOS-only list. The sixth, in flight: branch `meos/spansetops-every-build` (`8db5f185f5` on master `7102fd6de2`, rewrites `4b219a62b0`, rewrites `e58cca2621`, merged as MobilityDB #3300, `526ac77a42`) moves the 104 functions of `spanset_ops_meos.c` into `spanset_ops.c`, which gains the `float.h` and `limits.h` they read, and takes it off the MEOS-only list. The seventh, in flight: branch `meos/setagg-every-build` (`5b90f4b281` on master `7102fd6de2`, rewrites `7fe912d191`, rewrites `96a48413ef`, rewrites `6e22d6c736`, whose six functions it moves, merged as MobilityDB #3304, `51fa7d058d`) moves the six typed transition functions of `set_aggfuncs_meos.c` into the section of aggregate functions for sets of `set_aggfuncs.c` and takes it off the MEOS-only list. Prepared on master `3c5baef2b3`, each its own branch: `meos/tnumberdistance-every-build` (`3600175b03` on master `7102fd6de2`, rewrites `8418cd7699`, rewrites `ae1ec05826`, rewrites `1efb95f168`, rewrites `2fd7c3f363`, rewrites `7a0117c238` and rewrites `1f31e29092`, whose lines it moves, merged as MobilityDB #3310, `e9c8b8f600`, the 20 functions of `tnumber_distance_meos.c`, with `ensure_valid_tnumber_numspan` and `ensure_valid_tnumber_numspanset` of `temporal.c` out of `#if MEOS`, which the nearest approach distance of a temporal number and a span calls), `meos/typein-every-build` (`21989f825a`, merged as MobilityDB #3315, `a298abf51a`, the 20 MF-JSON inputs of `type_in_meos.c`), `meos/tsequence-every-build` (`35dfc063ed` on master `7102fd6de2`, rewrites `49775ebdb9`, rewrites `83d0b87111`, merged as MobilityDB #3319, `089206e021`, the 15 functions of `tsequence_meos.c` and its one baseline key) and `meos/temporal-every-build` (`fcaf71e1ed` on master `67aceb9e20`, merged as MobilityDB #3321, `cfdd06dcb4`, rewrites `c7a17b9800`, rewrites `fa332809f5`, rewrites `b89a43492e`, rewrites `96d1c56c98`, whose `Tbool_values` built its array from `temporal_values_p` while `tbool_values` was MEOS-only, the 57 functions of `temporal_meos.c`, `Tbool_values` calling `tbool_values`). `lifting_meos.c` stays on the MEOS-only list: its `lifting_result` and `lifting_result_int` read the error number under a handler that does not exit, and `lifting.c` calls them under `#if MEOS` alone, MEOS's part of the lifting as `ways_meos.c` is of the route lookup. The second batch, prepared on master `8342ca19df`, each its own branch: `meos/ttextfuncs-every-build` (`b49fda9f11` on master `c9197d0a01`, rewrites `322b9f5f24`, merged as MobilityDB #3327, `547c3f85d9`, the 6 text functions of `ttext_funcs_meos.c` into its section of generic functions on temporal texts), `meos/tboolops-every-build` (`0436c42823` on master `c9197d0a01`, rewrites `3286fdd45e`, merged as MobilityDB #3328, `bd5218ece8`, the 6 `tand_*` and `tor_*` into its generic functions), `meos/spanaggfuncs-every-build` (`4fea582cd5` on master `c9197d0a01`, rewrites `5d6fa80b11`, rewrites `77ee76bda0`, whose five functions it moves, merged as MobilityDB #3330, `db003d84f1`, the 5 typed extent transition functions into its extent section, with `temporal/temporal.h`), `meos/tnumbermathfuncs-every-build` (`7da7ce1637` on master `a8e931bf50`, rewrites `75d7897953`, merged as MobilityDB #3339, `5df4db1f9c`, the 28 typed arithmetic functions into its generic functions, with `temporal/temporal.h`) and `meos/tsequenceset-every-build` (`6c170e489b` on master `5df4db1f9c`, rewrites `d4a65bdfd4`, merged as MobilityDB #3340, `d2ecc4070f`, the 10 constructors of `tsequenceset_meos.c`). The third batch, each its own branch: `meos/temporalaggfuncs-every-build` (`144ea1dc27` on master `91f8d676e7`, rewrites `8b699ee00e`, rewrites `0cbaf9817d` and rewrites `f8b9ddc418`, whose lines it moves, MobilityDB #3344, CI 47/47 green) moves the 34 functions of `temporal_aggfuncs_meos.c`, the 23 transition and combine functions of the temporal aggregates into the section of generic aggregate transition functions of `temporal_aggfuncs.c`, beside `temporal_tagg_transfn` and `temporal_tagg_combinefn`, and the 11 transition functions of the moving window aggregates into the section of generic moving window transition functions of `temporal_waggfuncs.c`, beside `temporal_wagg_transfn` and `temporal_wagg_transform_transfn`, since a function joins the file of the generic function it calls, as the window wrappers of `mobilitydb/src` live in its `temporal_waggfuncs.c` (DECIDED BY THE OWNER 2026-11-01); its 27 baseline keys follow the functions. The wrappers of `mobilitydb/src` reach the generic kernels through `Temporal_tagg_transfn` and its siblings, which hold the aggregation context for every type, and stay as they are (DECIDED BY THE OWNER 2026-11-01). `meos/temporalcompops-every-build` (`44dda74ac0` on master `d2ecc4070f`, MobilityDB #3345, merged as `a0cedabe63`) moves the 156 functions of `temporal_compops_meos.c` into `temporal_compops.c`, which gains the `temporal/temporal.h` they read: the 104 ever and always comparisons into its section of ever and always functions, beside `ever_eq_temporal_base` and the siblings they call, and the 52 temporal comparisons into its section of generic functions for temporal comparisons, beside `tcomp_temporal_base` and `tcomp_base_temporal`; its 12 baseline keys follow the functions, and the wrappers, generic over every type, stay as they are. `meos/temporalboxops-every-build` (`e84b8432c0` on master `91f8d676e7`, MobilityDB #3348, CI 47/47 green) moves the 40 bounding box operators of `temporal_boxops_meos.c` into `temporal_boxops.c`, which gains the `temporal/temporal.h` of the `ensure_valid_tnumber_*` they call: the 15 of a temporal value into its section of generic bounding box functions for temporal types, beside `boxop_temporal_tstzspan` and `boxop_temporal_temporal`, and the 25 of a temporal number into its section for temporal number types, beside `boxop_tnumber_numspan`, `boxop_tnumber_tbox` and `boxop_tnumber_tnumber`; the wrappers stay as they are. `meos/temporalrestrict-every-build` (`29f731305d` on master `91f8d676e7`, rewrites `f935759e00`, whose lines it moves, MobilityDB #3349) moves the 33 restriction functions of `temporal_restrict_meos.c` into the first of the four sections of restriction functions of `temporal_restrict.c`, beside `temporal_restrict_value`, `temporal_restrict_minmax`, `tnumber_restrict_span` and the siblings they call, the later three holding the restrictions of instants, sequences and sequence sets; `temporal_restrict.c` gains the `utils/varlena.h` whose `DatumGetTextP` they read; its 2 baseline keys follow the functions, and the wrappers stay as they are. `meos/temporaltile-every-build` (`be3a608f93` on master `d2ecc4070f`, rewrites `0266a54f5e`, rewrites `1cfc58f5e9`, rewrites `45bf26e979`, rewrites `49f2f7afb9`, rewrites `6e22d6c736`, rewrites `b313e49372`, rewrites `ff56f3f93f`, whose lines it moves) moves the 33 functions of `temporal_tile_meos.c` into `temporal_tile.c`: the multidimensional tiles of the temporal boxes into its section of that title, the value and time boxes of the temporal numbers into its section of boxes for temporal numbers, beside `tnumber_value_time_boxes`, and the 15 value splits, from `bin_position` to `tnumber_value_time_split`, into a section of value split functions for temporal numbers that closes the file after the time split functions, `tnumber_value_split` being the value counterpart of `temporal_time_split`; its 3 baseline keys follow the functions, and the wrappers stay as they are. `meos/tinstant-every-build` (`1834f255f8` on master `d2ecc4070f`, rewrites `7128c9344a` and rewrites `7d78ce43d5`, whose lines it moves) moves the 27 typed functions of `tinstant_meos.c` into `tinstant.c`: the 5 typed instant inputs into its input/output section beside `tinstant_in`, the 5 typed instant constructors into its constructor section beside `tinstant_make`, and the 17 typed value accessors, each under the guard of its family, into its accessor section beside `tinstant_value`; `tinstant.c` gains the `float.h`, `utils/varlena.h` and `meos_geo.h` they read and the headers of each family under its guard, it has no baseline key, and the wrappers stay as they are. `meos/spantile-every-build` (`adf8af6802` on master `d2ecc4070f`, rewrites `7128c9344a` and rewrites `96d1c56c98`, whose lines it moves) moves the 3 typed value bins of `span_tile_meos.c` (`tint_value_bins`, `tbigint_value_bins`, `tfloat_value_bins`) into `temporal_tile.c`, into its section of bins functions for temporal types beside `tnumber_value_bins` that they call: a file without a twin joins the file of the kernel it calls, and `temporal_tile.c` already includes what they read; it has no baseline key, and the wrappers stay as they are. `meos/temporalposops-every-build` (`ab5eb1dfc0` on master `d2ecc4070f`, rewrites `7128c9344a`, whose lines it moves) renames `temporal_posops_meos.c`, line for line, to `temporal_posops.c`, the name of the file of `mobilitydb/src` whose 44 wrappers serve its 44 position operators: a file without a twin joins the MEOS file of the name its wrappers' file has, `temporal_tile.c` for the bins of `span_tile_meos.c`, and takes that name when MEOS has none; it has no baseline key, and the wrappers stay as they are.

**Acceptance.** A `CMakeLists.txt` of `meos/src` lists under `if(MEOS)` only MEOS's part of a
facility (`ways_meos.c`), `meos/src` holds no PostgreSQL part, no other source is named
`*_meos.c`, `nm` of the extension's library defines every public MEOS function, and the
regression suite answers as before.

## Open

- **Rigid geometry relations**, the last task: does an exact algorithm with the correct semantics
  exist for the relations whose kernels answer over the union of the placements?
- **A defect, before the point cloud family:** `atTpcbox` cuts a step `tpcpoint` as linear through
  `tpcpoint_to_tgeompoint`; the restriction keeps the interpolation of its argument.
- Five SQL functions are commented out in `mobilitydb/sql` and have no C body: `h3index(bigint)`,
  `tjsonbPathQuery`, `tjsonbPathQueryTz`, `tdirection(tgeompoint)` and `quaternion(tpose)`. They
  are outside the census, which reads `CREATE FUNCTION` statements.

## The plan

Each branch is one pull request, on MobilityDB master `ce124289e6` unless stated; its receipts
come before its push.

| branch | commit | what it does | state |
|---|---|---|---|
| `fix/tjsonb-contained-operand-order` | `2245983d0e` | `tjsonbContained` answers `<@`, not `@>` | merged as MobilityDB #3146 (`89ec3ab98d`) |
| `fix/tpcpatch-tpcbox-z` | `6497012ac3` | the patch-level `atTpcbox`/`minusTpcbox` read the Z of the box | merged as MobilityDB #3150 (`71e333da59`) |
| `meos/cell-instants-answer-the-constructor` | `00c6f1ab07` | the cell instant constructors carry their `@csqlfn` | merged as MobilityDB #3149 (`ebc0e1debb`) |
| `sql/geo-distance-operators-take-their-procedures` | `95b8a58073` | the geo `<->` and `\|=\|` take their `Opr` procedures (Out of the bindings' scope) | merged as MobilityDB #3140 (`68c835ced7`) |
| `meos/json-functions-one-per-operation` | `0cbb00dd52` (on `71e333da59`) | the seven `astext` JSON functions split one public function per operation, after the shift/scale sibling; the shared helpers carry no `@sqlop`, the `@sqlop` baseline 3 on master -> 0 | merged as MobilityDB #3153 |
| `meos/json-wrappers-named` | `7ac86990c7` (on `71e333da59`) | the 10 `_tz` and lax JSON wrappers named in their tags, no `_opr` wrapper; the text forms are the split's | merged as MobilityDB #3156 |
| `meos/jsonpath-outside-meos-gate` | `6bbd242b73` (on `71e333da59`) | `jsonb_exists_array` and `jsonb_path_query_all` outside `#if MEOS` | merged as MobilityDB #3151 |
| `meos/box-constructors-named` | `caefcc5117` (on `7dd9debf0c`) | the six `stbox`/`geodstbox` constructors and `stbox(box2d)` named in their tags; next commit, DECIDED BY THE OWNER 2026-10-23: `tbox(integer\|bigint\|float\|numeric)` and `tbox(<number>, timestamptz\|tstzspan)` call the typed public function of their base type, as `Value_bin` does, so a float or numeric NaN raises "The value cannot be NaN" | merged as MobilityDB #3143 (`d2ccbae61a`) |
| `meos/pose-constructors-named` | `891e7009ca` (on `71e333da59`) | one wrapper per pose constructor, each named in its tag | merged as MobilityDB #3152 |
| `meos/tpcbox-pcpoint-constructors-named` | `dc87716b1f` (on `71e333da59`) | the five `tpcbox` constructors and `pcpoint(integer, float[])` named | merged as MobilityDB #3154 |
| `meos/trgeometry-constructors-named` | `b740b809e6` (on `ac8feb51dc`) | `trgeometry(geometry, tpose)`, `geometry(trgeometry)` and the sequence constructors named in the MEOS functions they call; the census reads 5 signatures fewer unstated | merged as MobilityDB #3166 (`e2e88cd1fd`) |
| `meos/span-box-distances-named` | `32e3e2bfc6` (on `3c25b6966f`) | `spanDistance(span, spanset)` calls `distance_spanset_span`; `nearestApproachDistance(tbox, tnumber)` named in the tags of `nad_tint_tbox` and its siblings, which MEOS-API #201 lets the catalog state, and `NAD_tbox_tnumber` joins `csqlfn_returns_baseline.txt` beside `NAD_tnumber_tbox`; the census reads 419 unstated on master and 411 on the branch | merged as MobilityDB #3161 (`2fd7c3f363`) |
| MobilityDuck #494, branch `tooling/refresh-generated-surface` | `be47371db`, its second commit (pin `3c25b6966f`) | the group `meos_json_json` retired: `tjsonbObjectField` and the four `tjsonbPath*` functions, which the JSON split leaves to no shape, are generated as the catalog places their arguments, with their `Tz` twins, `tjsonbContained(tjsonb, tjsonb)` and the commuted `tConcat`; 49 registrations more, none lost; `tjsonbSet` stays unregistered (its `text **` keys) | merged as MobilityDuck #494 (`3934afa922`) |
| MobilityDuck `codegen/cell-instant-constructors` | `59371b139` (on `3934afa922`) | `tquadbin`, `th3index` and `ts2cell` from a cell and a `timestamptz`, `tstzset`, `tstzspan` or `tstzspanset`, generated by retiring the three cell constructor groups; the hand `tquadbin(quadbin, timestamptz)` leaves `src/quadbin/tquadbin.cpp` | merged as MobilityDuck #496 (`83d10366d9`) |
| MobilityDuck `codegen/npoint-route-operators` | `5f1f8f050` (on `2c9793afed`) | the group `meos_npoint_route` retired: the twenty route operators of MobilityDB #3163 register from the catalog, `containedRid(npoint, tnpoint)`, `sameRid(npoint, tnpoint)`, `?@` and `@=` among them | merged as MobilityDuck #498 (`391edaff01`) |
| MobilityDuck `codegen/commuted-container-sql-name` | `2abcf13cc` (on `391edaff01`) | a commuted container signature registers under the name it declares: `spanDistance(span, spanset)` of MobilityDB #3161, which main registers as `spansetDistance` | merged as MobilityDuck #499 (`be06de8874`) |
| MEOS-API `fix/callee-skips-c-keywords` | `94303dd0c` | `_CALLEE` reads no C keyword as a call, so `_kernel_args` derives the order of a commuted wrapper past an `if` | merged as MEOS-API #201 (`849ff1d7e0`) |
| `meos/json-opr-wrappers-untagged` | `eb7a7b86a0` (on `f5017dbabf`) | the 26 `@csqlfn` names of JSON `_opr` wrappers leave the tags, rewrites `8655303ab6` (DECIDED BY THE OWNER 2026-10-24); every operator the catalog states is unchanged and the `…Opr` names leave it; `check_csqlfn.py --targets` refuses an `…Opr` tag and an `…Opr` signature that is not its base call | merged as MobilityDB #3158 (`f1c701db71`) |
| `meos/every-wrapper-named` | `d4d73b5a8e` (on `5023335034`) | the wrappers a tagged function computes, appended to its `@csqlfn` list: `Quadbin_recv`, `S2cell_recv`, `Numeric_to_tbox`, `Raster_dump_values_band`, and the three `Contained_*` of the JSON types on `contains_tjsonb_jsonb` and `contains_tjsonb_tjsonb`; the census of signatures the catalog does not state measures it | merged as MobilityDB #3160 (`ddae903f80`) |
| `meos/npoint-public` | `9c35fd3d60` (on `2fd7c3f363`) | the 20 route operations of `overlapsRid`, `containsRid`, `containedRid` and `sameRid` public, one per SQL signature and argument order, the wrappers calling them with no dispatcher; the census reads 419 unstated on master and 399 on the branch | merged as MobilityDB #3163 (`ac8feb51dc`) |
| `meos/cbuffer-public` | `bbbed77632` (on `68c835ced7`) | `econtains_geo_tcbuffer`, `atouches_tcbuffer_tcbuffer` public; the 17 static-first relationships of a temporal circular buffer and the tcbuffer grid of the orthogonality gate | merged as MobilityDB #3141 (`7dd9debf0c`) |
| `meos/json-public` | `d76fccaa2c` (on `ac8feb51dc`) | `jsonbset_to_textset`, `textset_to_jsonbset` public, and `jsonbfunc_jsonbset` frees each converted value as `textset_func` does; the census reads 419 unstated on master and 417 on the branch | merged as MobilityDB #3165 (`c3a0ab4a54`) |
| `fix/ever-always-wrappers-return-boolean` | `8480c9c2ba` (on `d2ccbae61a`) | four ever/always helpers return `PG_RETURN_BOOL` under `RETURNS boolean`, where they returned `int32`; no answer changes | merged as MobilityDB #3145 (`5642bac141`) |
| `geo/sql-overlays-answer-exactly` | `2092dc2092` (on `7dd9debf0c`) | the four SQL polygon overlays answer through `geom_intersection2d`, `geom_difference2d` and the new public `geom_union2d`, `geom_symdifference2d` | merged as MobilityDB #3142 (`939a9d1a86`) |
| `geo/trajectory-clip-native` | `73a68412de` (on `7dd9debf0c`) | a trajectory clips against a polygon through the native `tpoint_linear_inter_geom`; `clipper2_traj_poly_periods` and its helpers go (Clipper2 removal, step 2) | merged as MobilityDB #3144 (`0891e02755`) |

Left, each for the owner first:

- `valueCount` and `valuePercent` over a raster: one search value against `raster_value_count`'s
  array of them, a shape the catalog does not state.
- DECIDED BY THE OWNER 2026-10-23, following the shift/scale sibling (the internal flagged
  kernel `tnumber_shift_scale_value` behind the public `tint_shift_value`, `tint_scale_value`,
  `tint_shift_scale_value`): the seven public JSON functions carrying `bool astext`
  (`jsonbset_object_field`, `jsonbset_array_element`, `jsonbset_extract_path`,
  `tjson_object_field`, `tjsonb_object_field`, `tjsonb_array_element`, `tjsonb_extract_path`)
  become one public function per operation, as PostgreSQL's `jsonb_object_field` and
  `jsonb_object_field_text`, the `astext` kernel internal. Each public function then backs one
  SQL name and one operator, so the catalog's operator per function is right and the `@sqlop`
  baseline empties. The JSON branch keeps the `Tz` and `SetLax` tags and leaves the `Text`
  ones to the split.

## Acceptance

The census by signature over master reads 0 signatures that are not stated, apart from
PostgreSQL's support, the out-of-scope functions and the writers of an array, and the
regression suite answers as before.

## Appendix: every SQL signature the catalog does not state

### The tag only: the wrapper calls a public function that does not name it (44 wrappers, 46 signatures)

| wrapper | SQL signatures | MEOS function |
|---|---|---|
| `Box2d_to_stbox` | stbox(box2d) | gbox_to_stbox |
| `Contained_jsonb_tjsonb` | tjsonbContained(jsonb, tjsonb) | contains_tjsonb_jsonb |
| `Contained_tjsonb_jsonb` | tjsonbContained(tjsonb, jsonb) | contains_tjsonb_jsonb |
| `Contained_tjsonb_tjsonb` | tjsonbContained(tjsonb, tjsonb) | contains_tjsonb_tjsonb |
| `Jsonbset_array_element_opr` | jsonbsetArrayElementOpr(jsonbset, integer) | jsonbset_array_element |
| `Jsonbset_array_element_text` | jsonbsetArrayElementText(jsonbset, integer, text) | jsonbset_array_element |
| `Jsonbset_array_element_text_opr` | jsonbsetArrayElementTextOpr(jsonbset, integer) | jsonbset_array_element |
| `Jsonbset_object_field_opr` | jsonbsetObjectFieldOpr(jsonbset, text) | jsonbset_object_field |
| `Jsonbset_object_field_text` | jsonbsetObjectFieldText(jsonbset, text, text) | jsonbset_object_field |
| `Jsonbset_object_field_text_opr` | jsonbsetObjectFieldTextOpr(jsonbset, text) | jsonbset_object_field |
| `Jsonbset_path_exists_opr` | jsonbsetPathExistsOpr(jsonbset, jsonpath) | jsonbset_path_exists |
| `Jsonbset_path_exists_tz` | jsonbsetPathExistsTz(jsonbset, jsonpath, jsonb, boolean) | jsonbset_path_exists |
| `Jsonbset_path_match_opr` | jsonbsetPathMatchOpr(jsonbset, jsonpath) | jsonbset_path_match |
| `Jsonbset_path_match_tz` | jsonbsetPathMatchTz(jsonbset, jsonpath, jsonb, boolean) | jsonbset_path_match |
| `Jsonbset_path_query_array_tz` | jsonbsetPathQueryArrayTz(jsonbset, jsonpath, jsonb, boolean) | jsonbset_path_query_array |
| `Jsonbset_path_query_first_tz` | jsonbsetPathQueryFirstTz(jsonbset, jsonpath, jsonb, boolean) | jsonbset_path_query_first |
| `Jsonbset_set_lax` | jsonbsetSetLax(jsonbset, text[], jsonb, boolean, text) | jsonbset_set |
| `Pcpoint_make_coords` | pcpoint(integer, double precision[]) | pcpoint_make |
| `Pose_constructor` | pose(double precision, double precision, double precision, integer); pose(double precision, double precision, double precision, double precision, double precision, double precision, double precision, integer) | pose_make_2d |
| `Pose_constructor_point` | pose(geometry, double precision); pose(geometry, double precision, double precision, double precision, double precision) | pose_make_point2d |
| `Pose_constructor_point_ypr` | pose(geometry, double precision, double precision, double precision) | pose_make_point3d_ypr |
| `Tjson_array_element_opr` | tjsonArrayElementOpr(ttext, integer) | tjson_array_element |
| `Tjson_object_field_opr` | tjsonObjectFieldOpr(ttext, text) | tjson_object_field |
| `Tjsonb_array_element_opr` | tjsonbArrayElementOpr(tjsonb, integer) | tjsonb_array_element |
| `Tjsonb_array_element_text` | tjsonbArrayElementText(tjsonb, integer, text) | tjsonb_array_element |
| `Tjsonb_array_element_text_opr` | tjsonbArrayElementTextOpr(tjsonb, integer) | tjsonb_array_element |
| `Tjsonb_object_field_opr` | tjsonbObjectFieldOpr(tjsonb, text) | tjsonb_object_field |
| `Tjsonb_object_field_text` | tjsonbObjectFieldText(tjsonb, text, text) | tjsonb_object_field |
| `Tjsonb_object_field_text_opr` | tjsonbObjectFieldTextOpr(tjsonb, text) | tjsonb_object_field |
| `Tjsonb_path_exists_opr` | tjsonbPathExistsOpr(tjsonb, jsonpath) | tjsonb_path_exists |
| `Tjsonb_path_exists_tz` | tjsonbPathExistsTz(tjsonb, jsonpath, jsonb, boolean) | tjsonb_path_exists |
| `Tjsonb_path_match_opr` | tjsonbPathMatchOpr(tjsonb, jsonpath) | tjsonb_path_match |
| `Tjsonb_path_match_tz` | tjsonbPathMatchTz(tjsonb, jsonpath, jsonb, boolean) | tjsonb_path_match |
| `Tjsonb_path_query_array_tz` | tjsonbPathQueryArrayTz(tjsonb, jsonpath, jsonb, boolean) | tjsonb_path_query_array |
| `Tjsonb_path_query_first_tz` | tjsonbPathQueryFirstTz(tjsonb, jsonpath, jsonb, boolean) | tjsonb_path_query_first |
| `Tjsonb_set_lax` | tjsonbSetLax(tjsonb, text[], jsonb, boolean, text) | tjsonb_set |
| `Tpcbox_constructor_2d` | tpcboxX(float, float, float, float, integer) | tpcbox_make |
| `Tpcbox_constructor_3d` | tpcboxZ(float, float, float, float, float, float, integer) | tpcbox_make |
| `Tpcbox_constructor_t` | tpcboxT(tstzspan, integer) | tpcbox_make |
| `Tpcbox_constructor_xt` | tpcboxXT(float, float, float, float, tstzspan, integer) | tpcbox_make |
| `Tpcbox_constructor_zt` | tpcboxZT(float, float, float, float, float, float, tstzspan, integer) | tpcbox_make |
| `Trgeometry_constructor` | trgeometry(geometry, tpose) | geometry_tpose_to_trgeometry |
| `Trgeometry_seqset_constructor_gaps` | trgeometrySeqSetGaps(trgeometry[], interval, float, text) | trgeometryseqset_make_gaps |
| `Trgeometry_to_geom` | geometry(trgeometry) | trgeometry_geom |

### The wrapper calls the public function: it reaches it through an internal one or composes it (44 wrappers, 58 signatures)

| wrapper | SQL signatures | MEOS function |
|---|---|---|
| `Adisjoint_cbuffer_tcbuffer` | aDisjoint(cbuffer, tcbuffer) | adisjoint_tcbuffer_cbuffer |
| `Adisjoint_geo_tcbuffer` | aDisjoint(geometry, tcbuffer) | adisjoint_tcbuffer_geo |
| `Adisjoint_geo_trgeometry` | aDisjoint(geometry, trgeometry) | adisjoint_trgeometry_geo |
| `Adwithin_cbuffer_tcbuffer` | aDwithin(cbuffer, tcbuffer, float) | adwithin_tcbuffer_cbuffer |
| `Adwithin_geo_tcbuffer` | aDwithin(geometry, tcbuffer, float) | adwithin_tcbuffer_geo |
| `Aintersects_cbuffer_tcbuffer` | aIntersects(cbuffer, tcbuffer) | aintersects_tcbuffer_cbuffer |
| `Aintersects_geo_tcbuffer` | aIntersects(geometry, tcbuffer) | aintersects_tcbuffer_geo |
| `Aintersects_geo_trgeometry` | aIntersects(geometry, trgeometry) | aintersects_trgeometry_geo |
| `Atouches_cbuffer_tcbuffer` | aTouches(cbuffer, tcbuffer) | atouches_tcbuffer_cbuffer |
| `Atouches_geo_tcbuffer` | aTouches(geometry, tcbuffer) | atouches_tcbuffer_geo |
| `Atouches_geo_trgeometry` | aTouches(geometry, trgeometry) | atouches_trgeometry_geo |
| `Distance_span_spanset` | spanDistance(intspan, intspanset); spanDistance(bigintspan, bigintspanset); spanDistance(floatspan, floatspanset); spanDistance(datespan, datespanset); spanDistance(tstzspan, tstzspanset) | distance_intspanset_intspan, distance_bigintspanset_bigintspan, distance_floatspanset_floatspan |
| `Edisjoint_cbuffer_tcbuffer` | eDisjoint(cbuffer, tcbuffer) | edisjoint_tcbuffer_cbuffer |
| `Edisjoint_geo_tcbuffer` | eDisjoint(geometry, tcbuffer) | edisjoint_tcbuffer_geo |
| `Edisjoint_geo_trgeometry` | eDisjoint(geometry, trgeometry) | edisjoint_trgeometry_geo |
| `Edwithin_cbuffer_tcbuffer` | eDwithin(cbuffer, tcbuffer, float) | edwithin_tcbuffer_cbuffer |
| `Edwithin_geo_tcbuffer` | eDwithin(geometry, tcbuffer, float) | edwithin_tcbuffer_geo |
| `Eintersects_cbuffer_tcbuffer` | eIntersects(cbuffer, tcbuffer) | eintersects_tcbuffer_cbuffer |
| `Eintersects_geo_tcbuffer` | eIntersects(geometry, tcbuffer) | eintersects_tcbuffer_geo |
| `Eintersects_geo_trgeometry` | eIntersects(geometry, trgeometry) | eintersects_trgeometry_geo |
| `Etouches_cbuffer_tcbuffer` | eTouches(cbuffer, tcbuffer) | etouches_tcbuffer_cbuffer |
| `Etouches_geo_tcbuffer` | eTouches(geometry, tcbuffer) | etouches_tcbuffer_geo |
| `Etouches_geo_trgeometry` | eTouches(geometry, trgeometry) | etouches_trgeometry_geo |
| `Geodstbox_constructor_z` | geodstboxZ(float, float, float, float, float, float, integer) | stbox_make |
| `Geodstbox_constructor_zt` | geodstboxZT(float, float, float, float, float, float, timestamptz, integer); geodstboxZT(float, float, float, float, float, float, tstzspan, integer) | stbox_make |
| `Jsonbset_extract_path_opr` | jsonbsetExtractPathOpr(jsonbset, text[]) | jsonbset_extract_path |
| `Jsonbset_extract_path_text` | jsonbsetExtractPathText(jsonbset, text[], text) | jsonbset_extract_path |
| `Jsonbset_extract_path_text_opr` | jsonbsetExtractPathTextOpr(jsonbset, text[]) | jsonbset_extract_path |
| `NAD_tbox_tnumber` | nearestApproachDistance(tbox, tint); nearestApproachDistance(tbox, tbigint); nearestApproachDistance(tbox, tfloat) | nad_tint_tbox, nad_tbigint_tbox, nad_tfloat_tbox |
| `Numeric_to_tbox` | tbox(numeric) | float_to_tbox |
| `Pose_point` | point(pose) | pose_to_point |
| `Raster_dump_values_band` | dumpValues(raster, integer, boolean) | raster_dump_values |
| `Raster_value_count_value` | valueCount(raster, integer, boolean, float, float); valueCount(raster, integer, float, float); valueCount(raster, float, float) | raster_value_count |
| `Raster_value_percent_value` | valuePercent(raster, integer, boolean, float, float); valuePercent(raster, integer, float, float); valuePercent(raster, float, float) | raster_value_count |
| `Stbox_constructor_t` | stboxT(timestamptz); stboxT(tstzspan) | stbox_make |
| `Stbox_constructor_xt` | stboxXT(float, float, float, float, timestamptz, integer); stboxXT(float, float, float, float, tstzspan, integer) | stbox_make |
| `Stbox_constructor_z` | stboxZ(float, float, float, float, float, float, integer) | stbox_make |
| `Stbox_constructor_zt` | stboxZT(float, float, float, float, float, float, timestamptz, integer); stboxZT(float, float, float, float, float, float, tstzspan, integer) | stbox_make |
| `Tjson_extract_path_opr` | tjsonExtractPathOpr(ttext, text[]) | tjson_extract_path |
| `Tjsonb_extract_path_opr` | tjsonbExtractPathOpr(tjsonb, text[]) | tjsonb_extract_path |
| `Tjsonb_extract_path_text` | tjsonbExtractPathText(tjsonb, text[], text) | tjsonb_extract_path |
| `Tjsonb_extract_path_text_opr` | tjsonbExtractPathTextOpr(tjsonb, text[]) | tjsonb_extract_path |
| `Trgeometry_seq_constructor` | trgeometrySeq(trgeometry[], text, boolean, boolean) | trgeometryseq_make |
| `Trgeometry_seqset_constructor` | trgeometrySeqSet(trgeometry[]) | trgeometryseqset_make |

### A public MEOS function to add (60 wrappers, 68 signatures)

| wrapper | SQL signatures | MEOS function |
|---|---|---|
| `Acontains_trgeometry_geo` | aContains(trgeometry, geometry) | ea_contains_trgeo_geo -> propose int acontains_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs) |
| `Acontains_trgeometry_trgeometry` | aContains(trgeometry, trgeometry) | ea_contains_trgeo_trgeo -> propose int acontains_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Acovers_trgeometry_trgeometry` | aCovers(trgeometry, trgeometry) | ea_covers_trgeo_trgeo -> propose int acovers_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Atouches_tcbuffer_tcbuffer` | aTouches(tcbuffer, tcbuffer) | ea_touches_tcbuffer_tcbuffer -> atouches_tcbuffer_tcbuffer exists (meos_cbuffer.h:362) but @ingroup meos_internal_geo_rel_ever; fix = public @ingroup and call it |
| `Atouches_trgeometry_trgeometry` | aTouches(trgeometry, trgeometry) | ea_touches_trgeo_trgeo -> propose int atouches_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Contained_rid_bigint_tnpoint` | containedRid(bigint, tnpoint) | contained_rid_tnpoint_bigint(INVERT) -> propose bool contained_rid_bigint_tnpoint(int64 rid, const Temporal *temp) |
| `Contained_rid_bigintset_tnpoint` | containedRid(bigintset, tnpoint) | contained_rid_tnpoint_bigintset(INVERT) -> propose bool contained_rid_bigintset_tnpoint(const Set *s, const Temporal *temp) |
| `Contained_rid_npoint_tnpoint` | containedRid(npoint, tnpoint) | contained_rid_npoint_tnpoint(temp, np, INVERT) -> propose bool contained_rid_npoint_tnpoint(const Npoint *np, const Temporal *temp) |
| `Contained_rid_tnpoint_bigintset` | containedRid(tnpoint, bigintset) | contained_rid_tnpoint_bigintset(INVERT_NO) -> propose bool contained_rid_tnpoint_bigintset(const Temporal *temp, const Set *s) |
| `Contained_rid_tnpoint_tnpoint` | containedRid(tnpoint, tnpoint) | contained_rid_tnpoint_tnpoint (no invert param) -> make public as is |
| `Contains_rid_bigintset_tnpoint` | containsRid(bigintset, tnpoint) | contains_rid_tnpoint_bigintset(INVERT) -> propose bool contains_rid_bigintset_tnpoint(const Set *s, const Temporal *temp) |
| `Contains_rid_tnpoint_bigint` | containsRid(tnpoint, bigint) | contains_rid_tnpoint_bigint(invert UNUSED) -> propose bool contains_rid_tnpoint_bigint(const Temporal *temp, int64 rid) |
| `Contains_rid_tnpoint_bigintset` | containsRid(tnpoint, bigintset) | contains_rid_tnpoint_bigintset(INVERT_NO) -> propose bool contains_rid_tnpoint_bigintset(const Temporal *temp, const Set *s) |
| `Contains_rid_tnpoint_npoint` | containsRid(tnpoint, npoint) | contains_rid_tnpoint_npoint(invert UNUSED) -> propose bool contains_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np) |
| `Contains_rid_tnpoint_tnpoint` | containsRid(tnpoint, tnpoint) | contains_rid_tnpoint_tnpoint (no invert param) -> make public as is |
| `Distance_set_set` | setDistance(cbufferset, cbufferset); setDistance(npointset, npointset); setDistance(poseset, poseset) | distance_cbufferset_cbufferset, distance_npointset_npointset, distance_poseset_poseset (after distance_geoset_geoset) |
| `Distance_set_value` | setDistance(cbufferset, cbuffer); setDistance(npointset, npoint); setDistance(poseset, pose) | distance_set_cbuffer, distance_set_npoint, distance_set_pose (after distance_set_geo) |
| `Distance_value_set` | setDistance(cbuffer, cbufferset); setDistance(npoint, npointset); setDistance(pose, poseset) | distance_set_cbuffer, distance_set_npoint, distance_set_pose, the set first as in distance_set_geo |
| `Econtains_geo_tcbuffer` | eContains(geometry, tcbuffer) | ea_contains_geo_tcbuffer -> propose int econtains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp) |
| `Econtains_trgeometry_geo` | eContains(trgeometry, geometry) | ea_contains_trgeo_geo -> propose int econtains_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs) |
| `Econtains_trgeometry_trgeometry` | eContains(trgeometry, trgeometry) | ea_contains_trgeo_trgeo -> propose int econtains_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Ecovers_trgeometry_trgeometry` | eCovers(trgeometry, trgeometry) | ea_covers_trgeo_trgeo -> propose int ecovers_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Eintersects_tpcpatch_geo` | eIntersects(tpcpatch, geometry) | propose bool eintersects_tpcpatch_geo(const Temporal *temp, const GSERIALIZED *gs) |
| `Etouches_trgeometry_trgeometry` | eTouches(trgeometry, trgeometry) | ea_touches_trgeo_trgeo -> propose int etouches_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2) |
| `Geom_difference` | difference(geometry, geometry) | clip_poly_poly(subj, clip, CL_DIFFERENCE) (geo/geo_poly_clip.h:64, Clipper2, polygons only, 2D planar) |
| `Geom_sym_difference` | symDifference(geometry, geometry) | clip_poly_poly(CL_XOR) -> propose GSERIALIZED *geom_symdifference2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2) |
| `Jsonbset_as_textset` | textset(jsonbset) | jsonbfunc_jsonbset(s, datum_jsonb_to_text) -> propose Set *jsonbset_to_textset(const Set *s) |
| `Overlaps_rid_bigintset_tnpoint` | overlapsRid(bigintset, tnpoint) | overlaps_rid_tnpoint_bigintset(invert UNUSED) -> propose bool overlaps_rid_bigintset_tnpoint(const Set *s, const Temporal *temp) |
| `Overlaps_rid_tnpoint_bigintset` | overlapsRid(tnpoint, bigintset) | overlaps_rid_tnpoint_bigintset(invert UNUSED) -> propose bool overlaps_rid_tnpoint_bigintset(const Temporal *temp, const Set *s) |
| `Overlaps_rid_tnpoint_tnpoint` | overlapsRid(tnpoint, tnpoint) | overlaps_rid_tnpoint_tnpoint (no invert param) -> make public as is |
| `Pcpatch_srid` | SRID(pcpatch) | propose int32_t pcpatch_srid(const Pcpatch *pa) |
| `Pcpoint_srid` | SRID(pcpoint) | propose int32_t pcpoint_srid(const Pcpoint *pt) |
| `Same_rid_bigint_tnpoint` | sameRid(bigint, tnpoint) | same_rid_tnpoint_bigint(invert UNUSED) -> propose bool same_rid_bigint_tnpoint(int64 rid, const Temporal *temp) |
| `Same_rid_bigintset_tnpoint` | sameRid(bigintset, tnpoint) | same_rid_tnpoint_bigintset(invert UNUSED) -> propose bool same_rid_bigintset_tnpoint(const Set *s, const Temporal *temp) |
| `Same_rid_npoint_tnpoint` | sameRid(npoint, tnpoint) | same_rid_tnpoint_npoint(invert UNUSED) -> propose bool same_rid_npoint_tnpoint(const Npoint *np, const Temporal *temp) |
| `Same_rid_tnpoint_bigint` | sameRid(tnpoint, bigint) | same_rid_tnpoint_bigint(invert UNUSED) -> propose bool same_rid_tnpoint_bigint(const Temporal *temp, int64 rid) |
| `Same_rid_tnpoint_bigintset` | sameRid(tnpoint, bigintset) | same_rid_tnpoint_bigintset(invert UNUSED) -> propose bool same_rid_tnpoint_bigintset(const Temporal *temp, const Set *s) |
| `Same_rid_tnpoint_npoint` | sameRid(tnpoint, npoint) | same_rid_tnpoint_npoint(invert UNUSED) -> propose bool same_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np) |
| `Same_rid_tnpoint_tnpoint` | sameRid(tnpoint, tnpoint) | same_rid_tnpoint_tnpoint (no invert param) -> make public as is |
| `Tdistance_number_tnumber` | tDistance(bigint, tbigint) | tdistance_tbigint_bigint, the argument order of the C function being the temporal value first (after tdistance_tint_int) |
| `Tdistance_tnumber_number` | tDistance(tbigint, bigint) | tdistance_tbigint_bigint (after tdistance_tint_int) |
| `Temporal_value_at_timestamptz` | valueAtTimestamp(tposechain, timestamptz) | tposechain_value_at_timestamptz (after tpose_value_at_timestamptz) |
| `Textset_as_jsonbset` | jsonbset(textset) | jsonbfunc_jsonbset(s, datum_text_to_jsonb) -> propose Set *textset_to_jsonbset(const Set *s) |
| `Tinstant_constructor` | th3index(h3index, timestamptz); tquadbin(quadbin, timestamptz); ts2cell(s2cell, timestamptz) | the cell instant constructors, MobilityDB #3149 (merged) |
| `Tpcpatch_at_geometry` | atGeometry(tpcpatch, geometry) | propose Temporal *tpcpatch_at_geom(const Temporal *temp, const GSERIALIZED *gs) |
| `Tpcpatch_at_tpcbox` | atTpcbox(tpcpatch, tpcbox, boolean) | propose Temporal *tpcpatch_at_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Tpcpatch_at_tpcbox_fine` | atTpcboxFine(tpcpatch, tpcbox, boolean) | propose Temporal *tpcpatch_at_tpcbox_fine(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Tpcpatch_minus_geometry` | minusGeometry(tpcpatch, geometry) | propose Temporal *tpcpatch_minus_geom(const Temporal *temp, const GSERIALIZED *gs) |
| `Tpcpatch_minus_tpcbox` | minusTpcbox(tpcpatch, tpcbox, boolean) | propose Temporal *tpcpatch_minus_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Tpcpatch_minus_tpcbox_fine` | minusTpcboxFine(tpcpatch, tpcbox, boolean) | propose Temporal *tpcpatch_minus_tpcbox_fine(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Tpcpatch_points` | points(tpcpatch) | propose Pcpoint **tpcpatch_points(const Temporal *temp, TimestampTz **times, int *count) |
| `Tpcpoint_at_tpcbox` | atTpcbox(tpcpoint, tpcbox, boolean) | propose Temporal *tpcpoint_at_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Tpcpoint_get_dim` | getDim(tpcpoint, text) | propose Temporal *tpcpoint_get_dim(const Temporal *temp, const char *name) |
| `Tpcpoint_get_x` | getX(tpcpoint) | propose Temporal *tpcpoint_get_x(const Temporal *temp) |
| `Tpcpoint_get_y` | getY(tpcpoint) | propose Temporal *tpcpoint_get_y(const Temporal *temp) |
| `Tpcpoint_get_z` | getZ(tpcpoint) | propose Temporal *tpcpoint_get_z(const Temporal *temp) |
| `Tpcpoint_minus_tpcbox` | minusTpcbox(tpcpoint, tpcbox, boolean) | propose Temporal *tpcpoint_minus_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc) |
| `Trgeometry_geom_clip_polygon` | _trgeometry_geom_clip_polygon(geometry, geometry, geometry, geometry, geometry) | trgeo_geom_clip_lwgeom (internal, LWGEOM) -> propose SpanSet *trgeometry_geom_clip_polygon(const GSERIALIZED *a1, const GSERIALIZED *b1, const GSERIALIZED *a2, const GSERIALIZED *b2, const GSERIALIZED *poly) |
| `Trgeometry_geom_clip_polygon_posed` | _trgeometry_geom_clip_polygon_posed(geometry, geometry, pose, pose, geometry) | trgeo_geom_clip_lwgeom_posed (internal) -> propose SpanSet *trgeometry_geom_clip_polygon_posed(const GSERIALIZED *pa, const GSERIALIZED *pb, const Pose *pose1, const Pose *pose2, const GSERIALIZED *poly) |
| `Trgeometry_value_at_timestamptz` | valueAtTimestamp(trgeometry, timestamptz) | trgeo_value_at_timestamptz (rgeo/trgeo.h:82, Datum *) -> propose bool trgeometry_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value) |

### The writers of an array, left open (3 wrappers, 40 signatures)

| wrapper | SQL signatures | MEOS function |
|---|---|---|
| `Spatialarr_as_ewkt` | asEWKT(cbuffer[], integer); asEWKT(tcbuffer[], integer); asEWKT(tgeometry[], integer); asEWKT(tgeography[], integer); asEWKT(tgeompoint[], integer); asEWKT(tgeogpoint[], integer); asEWKT(geometry[], integer); asEWKT(geography[], integer); asEWKT(npoint[], integer); asEWKT(tnpoint[], integer); asEWKT(tpcpoint[]); asEWKT(tpcpatch[]); asEWKT(pose[], integer); asEWKT(tpose[], integer); asEWKT(posechain[], integer); asEWKT(tposechain[], integer); asEWKT(trgeometry[], integer) | asEWKT(cbuffer[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tcbuffer[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tgeometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tgeography[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tgeompoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tgeogpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(geometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(geography[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(npoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tnpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tpcpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tpcpatch[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(pose[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tpose[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(posechain[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(tposechain[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asEWKT(trgeometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out) |
| `Spatialarr_as_text` | asText(cbuffer[], integer); asText(tcbuffer[], integer); asText(tgeometry[], integer); asText(tgeography[], integer); asText(tgeompoint[], integer); asText(tgeogpoint[], integer); asText(geometry[], integer); asText(geography[], integer); asText(npoint[], integer); asText(tnpoint[], integer); asText(pose[], integer); asText(tpose[], integer); asText(posechain[], integer); asText(tposechain[], integer); asText(trgeometry[], integer) | asText(cbuffer[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tcbuffer[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tgeometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tgeography[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tgeompoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tgeogpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(geometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(geography[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(npoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tnpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(pose[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tpose[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(posechain[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tposechain[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(trgeometry[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out) |
| `Temporalarr_as_text` | asText(tjsonb[]); asText(tpcpoint[]); asText(tpcpatch[]); asText(tbool[]); asText(tint[]); asText(tbigint[]); asText(tfloat[], integer); asText(ttext[]) | asText(tjsonb[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tpcpoint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tpcpatch[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tbool[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tbigint[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(tfloat[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out); asText(ttext[])->NONE(array writer; only internal temparr_out/spatialarr_wkt_out) |

### Out of the bindings' scope (38 wrappers, 81 signatures)

| wrapper | SQL signatures | MEOS function |
|---|---|---|
| `Create_trip` | createTrip(record[], timestamptz, boolean, text) | create_trip (geo/tpoint_datagen.h:44, LWLINE **) -> propose public TSequence *create_trip(const GSERIALIZED **lines, const double *maxspeeds, const int *categories, uint32_t count, TimestampTz t, bool disturb, int verbosity) |
| `Distance_geo_geoset_op` | setDistanceOp(geography, geogset) | distance_set_geo |
| `Distance_geoset_geo_op` | setDistanceOp(geogset, geography) | distance_set_geo |
| `Distance_geoset_geoset_op` | setDistanceOp(geogset, geogset) | distance_geoset_geoset |
| `Geometry_transform_gk` | transformGK(geometry) | propose GSERIALIZED *geo_transform_gk(const GSERIALIZED *gs) |
| `Multirange_to_spanset` | spanset(int4multirange); spanset(int8multirange); spanset(datemultirange); spanset(tstzmultirange) | spanset_make_free |
| `NAD_geo_stbox_op` | nearestApproachDistanceOp(geography, stbox) | nad_stbox_geo |
| `NAD_geo_tgeo_op` | nearestApproachDistanceOp(geography, tgeography); nearestApproachDistanceOp(geography, tgeogpoint) | nad_tgeo_geo |
| `NAD_stbox_geo_op` | nearestApproachDistanceOp(stbox, geography) | nad_stbox_geo |
| `NAD_stbox_stbox_op` | nearestApproachDistanceOp(stbox, stbox) | nad_stbox_stbox |
| `NAD_stbox_tgeo_op` | nearestApproachDistanceOp(stbox, tgeography); nearestApproachDistanceOp(stbox, tgeogpoint) | nad_tgeo_stbox |
| `NAD_tgeo_geo_op` | nearestApproachDistanceOp(tgeography, geography); nearestApproachDistanceOp(tgeogpoint, geography) | nad_tgeo_geo |
| `NAD_tgeo_stbox_op` | nearestApproachDistanceOp(tgeography, stbox); nearestApproachDistanceOp(tgeogpoint, stbox) | nad_tgeo_stbox |
| `NAD_tgeo_tgeo_op` | nearestApproachDistanceOp(tgeography, tgeography); nearestApproachDistanceOp(tgeogpoint, tgeogpoint) | nad_tgeo_tgeo |
| `Range_to_span` | span(int4range); span(int8range); span(daterange); span(tstzrange) |  |
| `Set_gin_extract_query` | set_gin_extract_query(integer, internal, int2, internal, internal, internal, internal); set_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal); set_gin_extract_query(date, internal, int2, internal, internal, internal, internal) |  |
| `Set_gin_extract_value` | set_gin_extract_value(integer, internal); set_gin_extract_value(bigint, internal); set_gin_extract_value(date, internal) | set_values |
| `Set_gin_triconsistent` | tnpoint_gin_triconsistent(internal, int2, bigint, integer, internal, internal, internal); set_gin_triconsistent(internal, int2, integer, integer, internal, internal, internal); set_gin_triconsistent(internal, int2, bigint, integer, internal, internal, internal); set_gin_triconsistent(internal, int2, date, integer, internal, internal, internal) |  |
| `Span_to_range` | range(intspan); range(bigintspan); range(datespan); range(tstzspan) |  |
| `Spanset_to_multirange` | multirange(intspanset); multirange(bigintspanset); multirange(datespanset); multirange(tstzspanset) |  |
| `Tcbuffer_enforce_typmod` | tcbuffer(tcbuffer, integer) |  |
| `Tdistance_geo_tgeo_op` | tDistanceOp(geography, tgeography); tDistanceOp(geography(Point), tgeogpoint) | tdistance_tgeo_geo |
| `Tdistance_tgeo_geo_op` | tDistanceOp(tgeography, geography); tDistanceOp(tgeogpoint, geography(Point)) | tdistance_tgeo_geo |
| `Tdistance_tgeo_tgeo_op` | tDistanceOp(tgeography, tgeography); tDistanceOp(tgeogpoint, tgeogpoint) | tdistance_tgeo_tgeo |
| `Temporal_enforce_typmod` | th3index(th3index, integer); tjsonb(tjsonb, integer); tnpoint(tnpoint, integer); tquadbin(tquadbin, integer); ts2cell(ts2cell, integer); tbool(tbool, integer); tint(tint, integer); tbigint(tbigint, integer); tfloat(tfloat, integer); ttext(ttext, integer) |  |
| `Tgeompoint_transform_gk` | transformGK(tgeompoint) | propose Temporal *tgeompoint_transform_gk(const Temporal *temp) |
| `Tnpoint_gin_extract_query` | tnpoint_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal) |  |
| `Tnpoint_gin_extract_value` | tnpoint_gin_extract_value(bigint, internal) |  |
| `Tpc_enforce_typmod` | tpcpoint(tpcpoint, integer); tpcpatch(tpcpatch, integer) |  |
| `Tspatial_enforce_typmod` | tgeometry(tgeometry, integer); tgeography(tgeography, integer); tgeompoint(tgeompoint, integer); tgeogpoint(tgeogpoint, integer); tpose(tpose, integer); tposechain(tposechain, integer); trgeometry(trgeometry, integer) |  |
| `box_constructor` | box(point, point) |  |
| `circle_constructor` | circle(point, float) |  |
| `fill_oid_cache` | fill_oid_cache() |  |
| `line_constructor` | line(float, float, float) |  |
| `lseg_constructor` | lseg(point, point) |  |
| `path_constructor` | path(point[]) |  |
| `point_constructor` | point(float, float) |  |
| `poly_constructor` | polygon(point[]) |  |
