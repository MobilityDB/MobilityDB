# Temporal&lt;T&gt; / TSpatial&lt;T&gt; inherited-surface map

> **Purpose.** A single map of the SQL/operator surface that every concrete
> temporal subtype *inherits* from the abstract classes `Temporal<T>` /
> `TNumber<T>` / `TSpatial<T>` / `TGeo<T>`, cross-referenced against **what the
> canonical generator already emits** vs. **what is still hand-maintained**.
>
> The ordering authority is the **doc XML `<sect1>` structure** — the reference
> manual chapters are the contract for *which* behaviours are inherited and in
> *what order*: `temporal_types_p1/p2.xml` / `temporal_spatial_p1/p2.xml` for the
> temporal classes, and `doc/set_span_types.xml` for the value-domain classes
> `Set<T>` / `Span<T>` / `SpanSet<T>` (§9). This document is a working draft to
> revise together; every claim below cites live source. The generated/hand status of
> each section is read from `manifest.d/` (the axis and its entry count), so the table
> and the generator cannot drift apart silently.
>
> **The manifest is split**, one file per axis: `manifest.d/<axis>.yaml` (e.g.
> `manifest.d/accessor_families.yaml`), loaded and merged by `generate.py`'s
> `load_manifest()`. **Three machine checks** keep this map honest: `--coverage`
> (every `.in.sql` under `mobilitydb/sql/` is either GOVERNED or listed in
> `coverage_exceptions.txt`; the exception list is a ratchet that may only shrink).
> ⛔ Governed is wider than NAMED: a `*_families` entry names its path in `file:`,
> while a `subtypes:` entry names none and emit mode DERIVES the path from the
> entry's bin plus the behaviour's offset. `manifest_files()` therefore runs that
> derivation through the same `target_path` the emit loop calls, so the files the
> subtypes track writes count as governed and may not sit on the exception list; a
> `reference: true` subtype is excluded, matching emit mode, which skips it — that
> file stays hand-owned under `--validate`. Then `--classes` (the manifest's
> `classes:` block matches the live catalog
> class predicates — **18** classes, the temporal ones plus the value-domain `set`,
> `span`, `spanset` and their catalog sub-predicates), and `--gaps` (per behaviour
> axis, which members of its class the rendered SQL does not yet name — a backlog
> report, not a gate; it scores all **21** axes, a value-domain axis against the
> classes its covered types belong to rather than against every set, so a text or
> JSONB set is absent from the distance backlog it can never join).

---

## 1. The OO hierarchy (live `meos/src/temporal/meos_catalog.c`)

The class of a temporal type is decided by the catalog membership predicates —
these are the single source of truth, not naming heuristics.

```
Temporal<T>              temporal_type      = ALL temporal types            (catalog:1117)
  ├── TAlpha<T>          talpha_type        = tbool, ttext, tjsonb, tdouble2/3/4  (catalog:1192)
  │     ├── TBool  ├── TText  └── TJsonb   (tdoubleN = internal)
  ├── TNumber<T>         tnumber_type       = tint, tbigint, tfloat          (catalog:1214)
  │     ├── TInt   ├── TBigint  └── TFloat
  └── TSpatial<T>        tspatial_type      = tgeompoint tgeogpoint tnpoint tpose
        │                                     tposechain tcbuffer tgeometry
        │                                     tgeography trgeometry th3index
        │                                     tquadbin ts2cell tpcpoint
        │                                     tpcpatch (14)
        ├── TGeo<T>      tgeo_type          = tgeometry, tgeography          (catalog:1325)
        │   (all)        tgeo_type_all      = + tgeompoint + tgeogpoint (4)   (catalog:1350)
        │     ├── TGeometry  ├── TGeography
        │     └── TPoint<T>  tpoint_type    = tgeompoint, tgeogpoint         (catalog:1303)
        ├── Tcell<T>     tcellindex_type    = th3index, tquadbin, ts2cell (3) (tcellindex.c:71)
        │     │                               all wired via DggsCellOps     (§5a)
        │     ├── TH3Index  ├── TQuadbin  └── TS2Cell
        ├── TPointcloud  tpointcloud_temptype = tpcpoint, tpcpatch  (#if POINTCLOUD)
        │     │                               a TSpatial<T> whose box is a TPCBox
        │     ├── TPcpoint  └── TPcpatch
        └── (TSpatial, no intermediate): tcbuffer, tnpoint, tpose, tposechain,
                                         trgeometry
```

- `tcbuffer`/`tnpoint`/`tpose`/`tposechain`/`trgeometry` inherit `Temporal<T>` +
  `TSpatial<T>` but **not** the `TGeo<T>`/`TPoint<T>`-only surface.
- ⭐ **`torder_type` = tint, tbigint, tfloat, ttext — a CROSS-CUTTING class, not a node of the
  tree above.** It is the whole of `TNumber<T>` plus `TText`, i.e. the temporal types whose base
  type carries an ORDER, and it is what `#<`/`#<=`/`#>`/`#>=`, `minValue`/`maxValue`,
  `atMin`/`atMax`/`minusMin`/`minusMax` and `minInstant`/`maxInstant` are declared for — twelve
  surfaces, the same four members throughout. ⛔ `tbool` and `tjsonb` are OUT although they sit in
  `talpha_type`, and every spatial member is OUT: PostgreSQL orders those so they can be B-tree
  indexed, and an index artifact is not a meaning to expose over time.
- ⭐ **`TSpatial<T>` MEMBERSHIP MEANS THE VALUES CARRY AN SRID — IT OBLIGES NO SRS SURFACE
  AND NO PARTICULAR BOX.** The box is `type_bboxtype` (§10): every member bounds itself with
  an `STBox` except `tpcpoint`/`tpcpatch`, which bound themselves with a `TPCBox`. The SRS
  surface follows from where the reference system LIVES, and the class already spans all
  three cases:

  | where the SRID lives | families | `SRID` | `setSRID` | `transform` |
  |---|---|---|---|---|
  | stored in the value | tgeo, tcbuffer, tpose, tposechain | ✓ | ✓ | ✓ |
  | inherited from a table | tnpoint (`ways`), tpcpoint/tpcpatch (`pointcloud_schemas`) | ✓ | — | — |
  | imposed by the specification | th3index, tquadbin, ts2cell | — | — | — |

  ⭐ **The pointcloud table is `pointcloud_schemas`, stated in SQL.** `mobilitydb/sql/pointcloud/399_pointcloud_schemas.in.sql`
  states the schema a `pcid` names as rows of `pointcloud_schemas` (which carries the SRID)
  and `pointcloud_dimensions`, both hand-written and so listed in `coverage_exceptions.txt`,
  both marked `pg_extension_config_dump` so a registered schema survives a dump — the shape
  `mobilitydb/sql/pose/120_geopose_frames.in.sql` already carries. The `pointcloud_formats`
  catalog of pgPointCloud is read after them and stays for data written by other tools.

  ⛔ So an absent `setSRID`/`transform` is the PROPERTY of an inherited or imposed reference
  system, never a parity gap: nothing per value can be set or transformed when the value
  holds its SRID by reference. `th3index`/`tquadbin`/`ts2cell` declare NONE of the three and are
  members, which is what shows membership carries no obligation.
- **`posechain`** is a base type carrying a nested chain of reference frames.
  Its own surface is `550_posechain.in.sql` and its set type is
  `551_posechainset.in.sql`; the type files, their accessors and their
  comparisons are hand-written, as `pose_base`'s are, and their generated
  surface is the `representation_families` entries `posechain_base` and
  `posechainset_*`. `spatial_basetype()` admits it, so the SRID and
  bounding-box dispatchers reach it.
- **`tposechain`** stands on it, so the family deploys the inherited temporal
  surface: `552_tposechain.in.sql` (the type file, I/O, constructors,
  conversions, accessors, transformations and restrictions),
  `554_tposechain_compops.in.sql`, `558_tposechain_topops.in.sql` and
  `559_tposechain_posops.in.sql`. `553_tposechain_spatialfuncs.in.sql` is
  hand-written beside its nine siblings and carries the spatial reference
  system alone — `SRID`, `setSRID`, `transform` and `transformPipeline`, each
  reading the outer link, which is the only link that names a frame.
  `555_tposechain_geopose.in.sql` has no file of its own: the OGC GeoPose
  entries are a `lit:` block of the `representations` axis, as the pose
  family's GeoPose entries are — the Composite Chain pair
  `asGeoPose(tposechain, …)` and `tposechainFromGeoPose`, and the Composite
  Graph writer `asGeoPose(tposechain[], …)`, which encodes a set of chains
  sharing their outermost frame.
  ⛔ The family carries no distance, no spatial relationships and no index
  file: a pose chain has no distance function, and the ordering operator waits
  on the kNN question.
- **`Tcell<T>`** (`tcellindex_type`, prefix `tcellindex_`) is a real abstract class
  factored via the `DggsCellOps` descriptor (§5a). Its cell families are **discrete**:
  they drop the continuous inherited aspects (distance, tempspatialrels).
- **RASTER** (`raquet`, `meos_catalog.h:137`) is a *base value type* (a raster tile),
  **not temporal** — no `traster` exists, so it has no `Temporal<T>` class. Out of
  this hierarchy until a temporal raster type is defined.

## 2. How inheritance is expressed

**Two orthogonal axes.**

1. **MEOS function PREFIX names the class** — every concrete subtype of that class
   inherits the behaviour via late binding + lifting (see memory
   `temporal-oo-late-binding-architecture`):

   | prefix | class | doc chapter |
   |---|---|---|
   | `temporal_` | `Temporal<T>` | `temporal_types_p1/p2.xml` |
   | `tnumber_`  | `TNumber<T>`  | `temporal_types_*` (number-only ops) |
   | `tspatial_` | `TSpatial<T>` | `temporal_spatial_p1/p2.xml` |
   | `tgeo_`     | `TGeo<T>`     | `temporal_spatial_*` |
   | `tpoint_`   | `TPoint<T>`   | `temporal_spatial_*` |
   | `<type>_`   | that family   | family chapter (e.g. `temporal_circular_buffers.xml`) |

2. **The doc XML `<sect1>` sections are the grouping + ordering authority.** Each
   `<sect1>` is one inherited behaviour group; its order fixes the family SQL-file
   numbering (see memory `north-star-family-file-numbering`). The two abstract
   chapters, in live section order:

   - **`Temporal<T>`** (`temporal_types_p1.xml` → `p2.xml`): Input/Output ·
     Constructors · Conversions · Accessors · Transformations · **Modifications** ·
     **Restrictions** · **Bounding Box Operators** · **Comparisons** {Traditional,
     Ever/Always, Temporal} · Miscellaneous.
   - **`TSpatial<T>` + `TGeo<T>`** (conflated — the prefix tells you which)
     (`temporal_spatial_p1.xml` → `p2.xml`): Input/Output · Conversions ·
     Accessors · Transformations · **Restrictions** · **Spatial Reference System** ·
     **Bounding Box Operations** · **Distance Operations** · **Spatial
     Relationships** {Ever/Always, Spatiotemporal}.

## 3. The canonical generator

**`tools/codegen/inherited/`** (registry: memory `mobilitydb-generators-registry`)
is the one canonical generator for the inherited operator surface. It is wired
into CI (`check-codegen.yml`, `--validate`/`--check`). It has **two output modes**:

- **whole-file SQL** at a fixed 50-bin position (`positions:` in `manifest.d/positions.yaml`):
  emits a complete `NNN_<family>_<behaviour>.in.sql`.
- **region-in-file** for C: emits the block between
  `GENERATED-BOXOPS-BEGIN/END` and `GENERATED-SPATIALRELS-BEGIN/END` markers.

**A `positions:` entry only means a slot is reserved** (`manifest.d/positions.yaml`:
compops, spatialfuncs, topops, posops, distance, aggfuncs, spatialrels, indexes,
gist, spgist). **A behaviour is *generated* for a given family only if the family
lists it in its `subtypes:` entry's `files:`** (`manifest.d/subtypes.yaml`) **and a
matching `templates/<behaviour>.sql.tmpl` exists** — or, for `aggfuncs`, if the
family has a whole-file entry on the separate `aggregate_families` axis (§4b), since
no subtype currently routes `aggfuncs` through the `subtypes:` track. Live templates:

| behaviour | template(s) | status |
|---|---|---|
| compops | `compops.sql.tmpl` | **GENERATED** — ONE engine (`render_compops_body` in `generate.py`) for every family: Ever/Always **and** Temporal (`tEq`/`tNe`/`tLt`/`tGt`/`tLe`/`tGe` → `#=`/`#<>`/`#<`/`#>`/`#<=`/`#>=`) comparisons together (see §4) |
| topops | `topops.sql.tmpl` | **GENERATED** |
| posops | `posops.sql.tmpl` | **GENERATED** |
| spatialrels | `spatialrels.c.tmpl` + `spatialrels.sql.tmpl` | **GENERATED** (ever/always) |
| boxops (C) | `boxops.c.tmpl` | **GENERATED** (box-type axis) |
| gist / spgist / indexes | `gist/spgist/indexes.sql.tmpl` | **GENERATED** (index infra) — `indexes.sql.tmpl` carries the Z-axis strategies 32-35 under `-- @IF front_back`, the same additive flag `posops.sql.tmpl` uses to declare the Z operators, so a family declares and indexes that axis from one manifest key. It also declares the family's own consistent and distance support functions, `{TEMP}_gist_consistent` and `{TEMP}_gist_distance`, each over the implementation the spatiotemporal families share. Both it and `gist.sql.tmpl` carry `FUNCTION 11 stbox_gist_sortsupport(internal)`, so an opclass a family is born with builds its index by sorting the entries rather than inserting them one at a time |
| aggfuncs | `aggregates.sql.tmpl` | **GENERATED** — whole-file per family via the separate `aggregate_families` axis (§4b), not the `subtypes:` track |
| spatialfuncs | — | reserved position, **HAND** |
| distance | — | reserved position, **HAND** |

⛔ **`front_back` reaches BOTH the operator declaration and the operator class.** It is an
additive flag (`DEFAULT_FALSE_FLAGS` in `generate.py`), so a family opts in and every other
family omits the block. The three-dimensional spatial families — `tpose`, `tposechain`,
`trgeometry` — set it; `tgeometry`/`tgeompoint` carry the same strategies as the hand-written
reference. A family that declares the Z operators without its opclass listing strategies
32-35 keeps those predicates as a FILTER: the operator resolves, the plan never uses the
index, and nothing errors — so the omission is invisible without reading `pg_amop`.

**`topops.sql.tmpl`/`posops.sql.tmpl` serve two independent manifest tracks** that
happen to share a template name: the `topop_families`/`posop_families` axes (§9,
Set<T>/Span<T> topological/position operators) and the `subtypes:` `files:` track
(this table, the per-family Temporal<T>/TSpatial<T> bounding-box operators `&&`,
`@>`, `<@`, `~=`, `-\|-` / `<<`, `>>`, `&<`, `&>`…). The two never share a manifest
entry.

Every `topops.sql.tmpl` section declares its functions and then the operators over
them, in both of its divider-separated blocks: the `tstzspan` block declares two
functions and their two operators, and the `stbox` block declares three functions —
`op(stbox, {TEMP})`, `op({TEMP}, stbox)`, `op({TEMP}, {TEMP})` — and then their three
operators. A `CREATE OPERATOR` names a `PROCEDURE` that must already exist, so the
functions lead; grouping them keeps one shape for both blocks and matches the
committed temporal geometry and temporal point files.

**Temporal spatial relationships (`tempspatialrel_families:`)** — a third,
standalone whole-file axis (no `positions:` slot, no `subtypes:` participation) for
the Spatiotemporal predicate surface (`tIntersects`/`tDwithin`/`tContains`/
`tTouches`/`tCovers`/`tDisjoint`). Each family entry sets `impl: native` (rendered
by `templates/tempspatialrels_native.sql.tmpl` — the family owns its own C kernel
per predicate/direction) or `impl: cast` (rendered by `templates/tempspatialrels.sql.tmpl`
— the family converts its operand to `tgeometry` and delegates to `tgeo`'s functions,
per the cast-only spatial-uniformization rule). See §5 and §6.

**Box-type axis (`boxtypes:`)** — the C bounding-box dispatchers are per *box type*,
not per family: `stbox` (tspatial), `tbox` (tnumber, composite value×time),
`tstzspan` (temporal-only), `tpcbox` (pointcloud). One `stbox` impl serves every
`TSpatial<T>` family.

**Spatialrel families (`spatialrel_families:`)** — the ever/always spatial-rel **C**
kernel wiring (region-in-file inside `meos/src/<fam>/*_spatialrels.c`, not a whole
SQL file): `geo_ea_contains_covers`/`geo_ea_disjoint_intersects`/`geo_ea_dwithin`
(**geo**, `tgeo_spatialrels.c`), `cbuffer_ea_contains_covers`/
`cbuffer_ea_disjoint_intersects`/`cbuffer_ea_dwithin` (**cbuffer**,
`tcbuffer_spatialrels.c`), `rgeo_ea_contains_covers`/`rgeo_ea_disjoint_intersects`/
`rgeo_ea_dwithin` (**rgeo**, `trgeo_spatialrels.c`). Because this axis renders C, not
SQL, `--gaps` cannot see it (its `CREATE FUNCTION`-matching regex finds nothing in
C source) and under-reports its coverage — read the manifest itself, not the `--gaps`
number, for this one axis. `pose`'s native C spatial-rel wrapper is still hand
(memory `spatialrel-wrapper-surface-is-inherited-generate-it`); `npoint` needs no
native kernel — its ever/always relationships cast-delegate to the temporal geometry
point in pure SQL (§6, `320_tnpoint_spatialrels`).

## 4. `Temporal<T>` chapter — section-by-section

| `<sect1>` | MEOS prefix | generated? | canonical generator / notes |
|---|---|---|---|
| Input and Output | `temporal_` | ✓ **GEN** | **two sub-families**, each missing `ts2cell` alone (`--gaps`: `io_families` 19/20, `representation_families` 19/20): (a) **type I/O** `<type>_in`/`_out`/`_recv`/`_send` — `io_type.sql.tmpl` + `io_families`, **13 entries** (temporal, geo, tpoint, cbuffer, npoint, pose, posechain, rgeo, h3, quadbin, pointcloud, pointcloud_patch, json). (b) **canonical representations** — `asText`/`asEWKT`/`asBinary`/`asEWKB`/`asHexWKB`/`asMFJSON` + the `From*` constructors — `representations.sql.tmpl` + `representation_families`, **18 entries**: the same 13 temporal families plus the five base-value entries `npoint_base`, `pose_base`, `posechain_base`, `cbuffer_base`, `raquet_base`, whose representations belong to the base type rather than to its temporal type |
| Constructors | `temporal_` | ✓ **GEN** | `constructors.sql.tmpl` + `constructor_families`, **13 entries** all `reference: true` (temporal, cbuffer, geo, h3, json, npoint, pose, quadbin, rgeo, tpcpatch, tpcpoint, tpoint) |
| Conversions | `temporal_` | ◐ PARTIAL | `conversions.sql.tmpl` + `conversion_families`, **8 entries** (temporal, cbuffer, h3, json, npoint, pose, quadbin, rgeo) — `--gaps`: `conversion_families` 15/20, missing tposechain, tgeography, ts2cell, tpcpoint, tpcpatch. `geo`/`tpoint` have no dedicated entry: their conversion surface is already named as the RETURNS/argument type of other families' declarations (e.g. rgeo's `tgeometry(trgeometry)`) |
| Accessors | `temporal_` | ✓ **GEN** | `accessors.sql.tmpl` multi-base renderer from base `022_temporal.in.sql` — the value/time/generic set for ALL families (§4c); per-family value shape = manifest `types:` tokens. A few interleaved/positional accessors stay hand per family |
| Transformations | `temporal_` | ✓ **GEN** | `transformations.sql.tmpl` + `transformation_families`, **13 entries** all `reference: true` (same family set as Constructors) — shiftTime/scaleTime, setInterp, tprecision, tsample. ⛔ `tsample` and `tprecision` are NOT the same surface and the manifest is the record of which family carries which: **`tsample` 10** (temporal, cbuffer, h3, json, npoint, pose, posechain, quadbin, tpcpatch, tpcpoint) against **`tprecision` 7** (temporal, cbuffer, h3, pose, posechain, quadbin, rgeo). `tsample` SELECTS the value holding at a bin boundary, so it applies to every temporal type; `tprecision` SYNTHESIZES one value per bin as a time-weighted average, so it applies only where the value type carries such an average — `ensure_has_twavg_temptype()` in `meos/src/temporal/temporal_analytics.c` is the kernel's own list. A family in the first set and not the second is either a gap to close or an exception to argue; the per-type table sits under §4b |
| Modifications | `temporal_` | ✓ **GEN** | `modifications.sql.tmpl` + `modification_families`, **13 entries** all `reference: true` (same family set) — appendInstant, insert, update, merge |
| Restrictions | `temporal_` | ✓ **GEN** | `restrictions.sql.tmpl` + `restriction_families`, **13 entries** all `reference: true` (same family set) — atValue(s)/minusValue(s), atTime/minusTime, atSpan(set), atTbox |
| **Bounding Box Operators** | `temporal_`/`tnumber_` | ✓ **GEN** | `topops.sql.tmpl` (`&&`,`@>`,`<@`,`~=`,`-\|-`) + `posops.sql.tmpl` (`<<`,`>>`,`&<`,`&>`,`<<#`,`#>>`…), both via the `subtypes:` track (§3), + `boxops.c.tmpl` box types `tstzspan`,`tbox` |
| Comparisons → Traditional | (btree) | ✓ **GEN** | `comparisons.sql.tmpl` + `comparison_families` (13 temporal-type entries: temporal, geo, tpoint, cbuffer, h3, json, npoint, pointcloud, pointcloud_patch, pose, posechain, quadbin, rgeo) — `=`,`<>`,`<`,`>`,`<=`,`>=` + `cmp` + the `<type>_btree_ops` opclass; `--gaps`: `comparison_families` 19/20, missing `ts2cell` (§5a). The hash tail of the same files (`hash`/`hashExtended` + the `<type>_hash_ops` opclass) is `hash_families` — the same 13 entries. ⛔ Each hash entry's LEADING `lit` is the bare `/****/` divider standing between the B-tree opclass and the hash block, because a region starts at the divider preceding its `begin` anchor: without that divider the hash region reaches back to the comparison banner and the entry has to carry the comparison section too; `--gaps`: `hash_families` 19/20, missing `ts2cell` (§5a) |
| Comparisons → **Ever/Always** | `temporal_` | ✓ **GEN** | `compops.sql.tmpl` + one shared `render_compops_body` engine, fed by two manifest tracks: `compops_families` (multi-pair families — temporal's 5 base types on one generic base/temporal C symbol per op, tgeo/tpoint's geometry+geography pair on one generic geo/tgeo C symbol per op) and the `subtypes:` `compops` behaviour (every one-pair family — cbuffer, jsonb, quadbin, h3index, s2cell, npoint, pose, trgeometry, pcpoint, pcpatch). `eEq`/`aEq`/`eNe`/`aNe` + `?=`/`%=`/`?<>`/`%<>` (all 3 arg directions); a pair whose temporal type the catalog's `torder_type` holds (temporal's int/bigint/float/text) additionally gets `eLt…aGe` + `?<…%>=`. A `compops_families` `pairs:` entry names only its `temp` type — `base` is never hand-paired alongside it; `render_compops` derives it from `catalog_temptype_basetype()`, read from meos_catalog.c's own `MEOS_RELTYPE_CATALOG[...].temptype_basetype` field, the same table `temptype_basetype()` reads at runtime for these functions' MEOS entry point. This makes a mismatched pair (`temp: tfloat` naming `base: integer`, which would render `tGt(tfloat, integer)` while the C entry point still derives float8 from the temp type alone) unrepresentable — the generator raises if a pair still carries a `base:` key |
| Comparisons → Temporal | `temporal_` | ✓ **GEN** | same engine as Ever/Always above — renders `tEq`/`tNe`/`tLt`/`tGt`/`tLe`/`tGe` → `#=`/`#<>`/`#<`/`#>`/`#<=`/`#>=` in the same pass, not a separate template |
| Miscellaneous | `temporal_` | ✗ HAND | |

### 4a. `TNumber<T>` and the talpha types — the base/number reference surface

`TNumber<T>` (**tint / tbigint / tfloat**) and the talpha types (**tbool / ttext /
tjsonb**) are **not family folders** — their whole surface lives in
`mobilitydb/sql/temporal/` and is the **hand-written reference** the generator
templates derive from (the number-side analogue of `geo/` for TSpatial). They
are **not in the generator's `subtypes:` list**, so their SQL is not re-emitted; only
the **C boxops regions** inside their source files are generated. TNumber ops are
documented inline in `temporal_types_p1/p2` (no separate number chapter).

| behaviour (file in `sql/temporal/`) | class | generated? | notes |
|---|---|---|---|
| `021_tbox` (TBox type: value × time) | TNumber | ✗ HAND | the number bounding box; its **C dispatchers ARE generated** — `boxops.c.tmpl` box type `tbox` region in `temporal_boxops.c` |
| `026_tnumber_mathfuncs` (`+ - * /`, abs, delta, trend, derivative) | TNumber | ✓ **GEN** | `mathfuncs.sql.tmpl` + `mathfunc_families`, **1 entry** (`tnumber`, `reference: true`) — the whole-file TNumber arithmetic surface |
| `036_tnumber_distance` (tDistance, nad) | TNumber | ✗ HAND | no distance template |
| number Restrictions (atSpan/atSpanset/atTbox) | TNumber | ✓ **GEN** | inside `022_temporal.in.sql`, governed by `restriction_families`' `temporal` entry (§4 table) |
| number Aggregates (extent, tSum, tAvg, tMin/tMax) | TNumber | ✓ **GEN** | `040_temporal_aggfuncs`, governed by `aggregate_families`' `temporal` entry (§4b) |
| `028_tbool_boolops` (`&` `\|` `~`, tAnd/tOr/tNot) | TAlpha (tbool) | ✗ HAND | tbool-specific |
| `029_ttext_textfuncs` (`\|\|`, upper/lower) | TAlpha (ttext) | ✗ HAND | ttext-specific |

⚠️ **`tbigint` and `tjsonb` are full members** of `tnumber_type()` / `talpha_type()`
(catalog:1214/1192) but are **absent from the MEOS-API lattice** (§8) — a curation gap.

**The generic base `Temporal<T>` reference files** (`032_temporal_boxops`
(extraction: spans/tboxes/split*), `033_temporal_topops` (topological: overlaps/
contains/contained/same/adjacent), `034_temporal_posops`, `040/042` aggfuncs,
`043/044` gist/spgist, `022/023` type/inout, `025_temporal_tile`, `038/046`
similarity/analytics) are likewise the hand reference; the generator re-emits their
*shape* onto the derived families (§6) and regenerates the **C boxops region** for
box type `tstzspan` inside `temporal_boxops.c`. `030_temporal_compops` is the one
exception in this group: it is fully generated (§3/§4, the `compops_families`
entry), not a hand reference — the derived families' one-pair compops files reach
the same engine independently via their own `subtypes:` entry, not by copying
`030`'s shape.

**Base value-domain types** (`Set` / `Span` / `SpanSet` / `TBox` / `STBox`) are the
finite-subset representations of the value/time domains that the restriction surface
(atValues=Set, atSpan=Span, atTbox=TBox…) consumes. Their operator files
(`001–015`, `021_tbox`) are hand today; memory
`generate-boxops-campaign-boxtype-axis` flags the repeated per-span-type
`005_span_ops`/`009_spanset_ops` as a future generation target. **§9 maps this
value-domain surface class-by-class** (membership, the Set-vs-Span asymmetry,
doc sections, file map, and the `setfamilies:` manifest axis).

### 4b. Aggregation / Indexing / Analytics chapters (also `Temporal<T>`-inherited)

Two more reference chapters carry inherited surface:

| chapter → `<sect1>` | prefix | generated? | notes |
|---|---|---|---|
| `temporal_types_aggregation.xml` → Aggregation | `temporal_`/`tnumber_` | ✓ **GEN** | `aggregates.sql.tmpl` + `aggregate_families`, **13 entries** all `reference: true`, `whole_file: true` (temporal, cbuffer, geo, json, npoint, pointcloud, pose, posechain, rgeo, tpoint, th3index, quadbin, ts2cell) — tCount/extent/tMin/tMax/tSum/tAvg/merge/appendInstant; `--gaps`: 20/20, full coverage |
| → Indexing | (index) | ✓ **GEN** | GiST/SP-GiST via `gist/spgist/indexes.sql.tmpl` |
| → Statistics and Selectivity | (selectivity) | ✗ HAND | |
| `temporal_types_analytics.xml` → Simplification / Reduction / Similarity / Extended Kalman Filter / Splitting | `temporal_`/`tgeo_` | ✗ HAND | analytics; no template |
| `temporal_types_analytics.xml` → Multidimensional Tiling | `temporal_`/`tgeo_` | ✓ **GEN** | `tiling.sql.tmpl` + `tiling_families`, **23 entries** all `reference: true` (geo, tpoint, cbuffer, json, json_boxops, npoint, tpcpoint, tpcpatch, tpc_boxops, pose, posechain, rgeo, tgeo_tile, trgeo_tile, tpoint_tile, tpose_tile, tpcpoint_tile, temporal, th3index, th3index_boxops, quadbin, quadbin_boxops, s2cell_boxops). The entry names sit at three granularities — family (`geo`, `npoint`), per-FILE tile split (`tgeo_tile`, `tpose_tile`), and per-FILE boxops split (`json_boxops`, `tpc_boxops`, `th3index_boxops`, `quadbin_boxops`) |

#### `tprecision` — which types carry a time-weighted average, and why the rest do not

`tprecision` reduces each time bin to ONE synthesized value, the time-weighted
average of the values the bin holds. So the surface follows the VALUE type's
algebra, not the temporal machinery: `ensure_has_twavg_temptype()`
(`meos/src/temporal/temporal_analytics.c`) is the list, and a type outside it has
nothing to answer with. Its siblings summarize by decomposing into parts that do
carry an average and recomposing — a circular buffer into its centre and radius, a
pose into its position and a circular mean of its angle.

| type | summary | status |
|---|---|---|
| tnumber, tgeompoint/tgeogpoint | average / centroid | carried |
| tcbuffer, tpose, trgeometry | per-parameter summary | carried |
| **tnpoint** | position averages on the route the bin shares | carried; a bin spanning two routes has no network point to answer with and raises, the rule the sequence constructor already states |
| **tposechain** | per-link pose average, the chain keeping its shape | carried; link count is an invariant of the whole value, so link *n* always has a counterpart |
| **th3index, tquadbin, ts2cell** | — | ⛔ **RULED EXCEPTION, not a gap.** A cell identifier is an area and an identity, not a position. The h3 and quadbin design notes state *"Int64 ordering is arbitrary with respect to grid geometry"*, so no arithmetic touches the value. ⛔ S2 reaches the SAME exception by a DIFFERENT route and the distinction is worth keeping: an S2 identifier IS spatially coherent — the descendants of a cell occupy one contiguous interval of the Hilbert curve — yet an average of cell identifiers still names no cell, so the exception holds on what the value MEANS rather than on its ordering. a centroid-of-centres would have to leave cell space and return, and the type constrains neither the resolution (one valid value holds cells of resolution 3 and 10) nor the pentagon and antimeridian cases the notes warn of. `tsample` answers the real need by returning a cell the value actually holds |
| **tgeometry, tgeography** | — | open: `centroid()` runs on both, so a bin reduces to a point, but no `twCentroid` aggregate exists over them and a point-valued result changes what the value depicts |
| **tjsonb** | — | a JSON document has no average |
| **tpcpoint, tpcpatch** | — | open: both carry `tsample` and `centroid(tpcpoint)` exists, so these are candidates of the same shape as `tnpoint` rather than settled exceptions |

### 4c. Canonical accessor set & order — the inherited value/time surface

Order is authoritative from `doc/temporal_types_p1.xml`
`<sect1 xml:id="ttype_accessors">` (one `<sect2>` per group representative; paired
value/time and start/end/N/plural members share the sect). The whole set is
**generated** by `templates/accessors.sql.tmpl` (multi-base renderer) from the base
reference file `mobilitydb/sql/temporal/022_temporal.in.sql`; each family in
`accessor_families` supplies only its per-base-type `types:` tokens.

| # | `<sect2>` (representative) | full group members | shape |
|---|---|---|---|
| 1 | memSize | memSize | generic |
| 2 | tempSubtype | tempSubtype | generic |
| 3 | tempBasetype | tempBasetype | generic |
| 4 | interp | interp | generic |
| 5 | getValue | getValue, getTimestamp | **value** |
| 6 | getValues | getValues, getTime | **value** (getValues → base *set*) |
| 7 | timeSpan | timeSpan | generic (tstzspan) |
| 8 | startValue | startValue, endValue, valueN | **value** |
| 9 | valueAtTimestamp | valueAtTimestamp | **value** |
| 10 | duration | duration | generic (interval) |
| 11 | lowerInc | lowerInc, upperInc | generic |
| 12 | numInstants | numInstants | generic |
| 13 | startInstant | startInstant, endInstant, instantN, instants | generic (returns same `T`) |
| 14 | numTimestamps | numTimestamps | generic |
| 15 | startTimestamp | startTimestamp, endTimestamp, timestampN, timestamps | generic |
| 16 | numSequences | numSequences | generic |
| 17 | startSequence | startSequence, endSequence, sequenceN, sequences | generic (returns same `T`) |
| 18 | segments | segments | generic (returns same `T`) |

**Per-family value shape** is entirely captured by the manifest `types:` tokens
(`accessor_families`, one row per base type) — this is the single place that says how
each family fills the **value** rows:

| token | meaning | example |
|---|---|---|
| `base` | scalar base value (getValue/startValue/endValue/valueN/valueAtTimestamp) | tcbuffer→`cbuffer`, tint→`integer` |
| `baseset` | the `getValues` return — a **set** for most, a **spanset for numbers** | tcbuffer→`cbufferset`, tfloat→`floatspanset` |
| `gvsym` | getValues C symbol when it differs (numbers → value spanset) | `Tnumber_valuespans` |
| `valueset` | the **discrete** `valueSet` return — emitted **only** when `numeric`/`orderable` | tint→`intset` |
| `numeric` / `orderable` | the two **presence classes**, gating the TNumber/orderable-only extras: `valueSet`, `minValue`, `maxValue`, `avgValue`, `minInstant`, `maxInstant`, and the min/max restrictions `atMin`, `atMax`, `minusMin`, `minusMax`. They are not manifest flags — `PRESENCE_CLASSES` maps each to a catalog class and `load_manifest()` reads the membership out of the `classes:` block, so a type joining `torder_type` in `meos_catalog.c` reaches every gated surface at once | `numeric` = `tnumber_type` (tint/tbigint/tfloat); `orderable` = `torder_type` (those + ttext) |
| `valret` / `valsym` | **value-materializing override**: value accessors return this type via that symbol family instead of the base | trgeometry→`geometry` / `Trgeometry` |

⭐ Canonical (user-corrected twice — do NOT re-derive from code frequency):
`getValues` returns the value **SET** for every type, the value **SPANSET** only for
`TNumber` (`gvsym: Tnumber_valuespans`); `valueSet` exists **only at `TNumber`/orderable**
to name the plain set vs the spanset. `trgeometry` is the sole family whose value
accessors are overridden (`valret: geometry`, `valsym: Trgeometry`) — it stores a pose
+ appended reference geometry, so getValue/getValues expose the raw pose while
startValue/endValue/valueN/valueAtTimestamp re-apply the pose to materialize a geometry.

Restrictions (`ttype_restrictions`) reuse the same value marshalling: `atValue`/
`minusValue` (scalar `base`) + `atValues`/`minusValues` (`baseset`).

### 4d. Per-family specific (non-inherited) surface

Beyond the generated inherited set, each family adds its OWN functions — hand,
value-shaped, **not** emitted by any inherited template (they live in the family's
`spatialfuncs`/type file). This is the complete per-family delta (live at the `.in.sql`
numbers in §6):

| family | specific functions | notes |
|---|---|---|
| **tcbuffer** (202) | `point`, `radius` | the two components of a circular buffer (center `tgeompoint` + `tfloat`) |
| **tnpoint** (302) | `route`, `routes`, `getPosition`, `positions`, `nsegment` | network-position components (route id + fractional position) |
| **tpose** (102) | `point`, `rotation`, `yaw`, `pitch`, `roll`, `speed`, `angularSpeed` | rigid-pose components (2D: rotation/yaw; 3D quaternion: pitch/roll) |
| **trgeometry** (150) | `point`, `rotation` | its value accessors are NOT specific here — they are the generated `valret`/`valsym` override (§4c) |
| **tpcpoint** (420) | `getX`, `getY`, `getZ` | point-coordinate accessors; the temporal **value** surface (`startValue`… → `Pcpoint`) is value-opaque, generated verbatim (base `pcpoint`/`pcpointset`) |
| **tpcpatch** (430) | (patch geometry accessors) | value-opaque container like tpcpoint (base `pcpatch`/`pcpatchset`) |
| **tgeo** (geo/point) | SRID, trajectory, traversedArea, convexHull, … | the hand reference layout — see §5 |

**Value-opaque ⇒ nothing extra to override**: `tjsonb`, `tpcpoint`, `tpcpatch`,
`tcbuffer`, `tnpoint`, `tpose` all fill the **value** rows with a plain `base`/`baseset`
(the base value is opaque to the accessor), so their inherited value surface is fully
generated; only `trgeometry` overrides it. The families' *specific* functions above are
a separate, small hand surface (components/coordinates), orthogonal to the inherited set.

### 4e. Input/Output generation scope

The Input/Output `<sect1>` is **two token-shaped sub-families**, each governed for
every member but `ts2cell` (`--gaps`: `io_families` 19/20,
`representation_families` 19/20, §5a); both mirror the
`accessor_families` model (per-type `types:` rows, region-marked blocks in a
reference file, `--validate` byte-for-byte).

**A — Type I/O** (in the type file: `022_temporal`, `102_tpose`, `202_tcbuffer`, …).
Canonical PG functions **every** temporal type must have, plus `CREATE TYPE`:

| function | signature | backing symbol | shape |
|---|---|---|---|
| `<temp>_in` | `(cstring, oid, integer) → <temp>` | `Temporal_in` / **`T<fam>_in`** (spatial ones apply typmod/SRID checks) | per-type name |
| `temporal_out` | `(<temp>) → cstring` | `Temporal_out` / **`Trgeometry_out`** | overloaded name |
| `<temp>_recv` | `(internal, oid, integer) → <temp>` | `Temporal_recv` / **`Trgeometry_recv`** | per-type name |
| `temporal_send` | `(<temp>) → bytea` | `Temporal_send` / **`Trgeometry_send`** | overloaded name |
| `temporal_typmod_in`/`_out`, `temporal_analyze` | shared | generic | **emit once** |
| `CREATE TYPE <temp>` | fixed skeleton | — | per-type |

Pure `{TEMP}`-token, **zero base-value marshalling** → the most mechanical section. Token
model needs per-type symbol overrides: `in_sym` (default `Temporal_in`, override
`T<fam>_in`), `out_sym`/`recv_sym`/`send_sym` (default `Temporal_*`, override
`Trgeometry_*`). See [[temporal-io-symbol-reuse-matrix]] for the full family matrix.

**Two structural shapes** (base ≠ spatial — the type-I/O is NOT uniformly reused):

| aspect | base (`022`, tbool…ttext) | spatial (`202`+, tcbuffer/tnpoint/tpose/tgeo…) |
|---|---|---|
| shell type | none | `CREATE TYPE <t>;` forward decl **first** |
| `typmod_in` | shared `temporal_typmod_in` | **own `<t>_typmod_in`** (`T<fam>_typmod_in`, encodes SRID/Z) |
| `typmod_out` / `analyze` | own `temporal_typmod_out` / `temporal_analyze` (defined here) | shared `tspatial_typmod_out` / `tspatial_analyze` |
| `CREATE TYPE` field order | input/output/**send/receive** | input/output/**receive/send** |

Both shapes are generated: the base shape from `io_families`' `temporal` entry
(`types:` rows substituted straight into `templates/io_type.sql.tmpl`); every
spatial family's entry instead carries its own `blocks:` sequence (the shared
`lit`/`group`/`sig`-`ret`-`sym`/`stmt`/`only`/`over` block-DSL that `comparison_families`,
`topop_families` and others also use) reproducing the shell type + `T<fam>_typmod_in`
+ `tspatial_typmod_out`/`_analyze` + swapped send/receive verbatim — there is no
separate `io_repr.sql.tmpl`; the generic block renderer covers both shapes.

- **Reuse is uniform except two families**: `_out`/`_send`/`_recv` are base-value-agnostic
  → generic for all **except trgeometry** (owns all four `Trgeometry_*` because the
  reference geometry sits at the **beginning** of the text form and the **end** of the
  binary form — `trgeo_parser.c:306` / `trgeo_inst.c:187`). `_in` specializes per spatial
  family (`T<fam>_in`).
- ⚠️ **Open question**: `tpcpoint`/`tpcpatch` reuse the **generic** `Temporal_in` while
  sibling spatial families carry `T<fam>_in`. Whether pointcloud needs the SRID/typmod
  checks its siblings apply is unresolved ([[close-gaps-in-meos-c-not-sql-composition]]).

**B — Canonical representations** (in `_inout` files: `023_temporal_inout`,
`053_tgeo_inout`, or inline in family type files). Two shapes gated by a `spatial` flag:

| representation | base (TAlpha/TNumber) | + TSpatial<T> | backing symbol |
|---|---|---|---|
| text | `asText` | `asEWKT` (`SRID=n;` prefix) / `FromText` / `FromEWKT` | `Temporal_as_text` · array `Temporalarr_as_text` |
| binary (WKB) | `asBinary` / `FromBinary` | `asEWKB` (embeds SRID) / `FromEWKB` | `Temporal_as_wkb` / `Temporal_from_wkb` |
| hex | `asHexWKB` / `FromHexWKB` | — | `Temporal_as_hexwkb` / `Temporal_from_hexwkb` |
| MF-JSON | `asMFJSON` / `FromMFJSON` | — | `Temporal_as_mfjson` / `Temporal_from_mfjson` |

EWKT/EWKB are **TSpatial<T>-level**, the `E` = carries the SRID
([[ewkt-ewkb-tspatial-srid-representation]]). Conventions to reproduce verbatim:
`maxdecimaldigits integer DEFAULT 15` on float/coordinate-bearing types only;
`endianenconding text DEFAULT ''` (canonical misspelling) on `asBinary`/`asHexWKB`.

Governed by `templates/representations.sql.tmpl` + `representation_families`,
**12 entries** all `reference: true` (temporal, cbuffer, geo, h3, json, npoint,
pointcloud, pointcloud_patch, pose, quadbin, rgeo, tpoint) — the same family set as
`io_families` above.

### 4f. Type-I/O canonicalization — `Temporal<>` vs `TSpatial<>` irregularities

Canonical references (both regular): base = `022` (tbool…ttext); spatial = `052_tgeo`
(tgeometry/tgeography/tgeompoint/tgeogpoint) — shell `CREATE TYPE <t>;`, `<t>_in`/
`temporal_out`/`<t>_recv`/`temporal_send`, `<t>_typmod_in`, shared `tspatial_typmod_out`
+ `tspatial_analyze`, `send/receive` order. Every *derived* spatial family's `io_families`
entry reproduces its own irregularities verbatim via the shared block-DSL (§4e) rather
than converging on one clean variant — the table below is the map of which
irregularities are genuine (kept) vs. resolved.

| id | irregularity | families | canonical form | class |
|---|---|---|---|---|
| I1 | `CREATE TYPE` field order `receive/send` | cbuffer, npoint, pose, rgeo, tpcpoint, tpcpatch | `send/receive` (base + tgeo) | non-API reorder — **resolved** |
| I2 | redundant `<t>_out`/`<t>_send` SQL renames (symbol = generic `Temporal_out`/`Temporal_send`) | pose, tpcpoint, tpcpatch | `temporal_out`/`temporal_send` (drop the rename) | **resolved** |
| I3 | `temporal_typmod_in`/`_out` **+** `tspatial_analyze` | npoint | **keep** — legitimate hybrid, not an irregularity | no change |
| I4 | `trgeo_typmod_in` SQL name vs `Trgeometry_typmod_in` symbol | rgeo | `trgeometry_typmod_in` (optional naming nit) | API — sign-off |
| I5 | `CREATE TYPE` registers **no `analyze`** though `tpcbox`/`tpc_boxops` give it a spatial bbox | tpcpoint, tpcpatch | add an `analyze` typanalyze for bbox stats | statistics gap — sign-off |

rgeo's `Trgeometry_out`/`_recv`/`_send` are genuine distinct functions (reference geometry
at the start of the text / end of the binary form) — kept, not an irregularity. npoint's
hybrid (I3) is likewise kept: its SRID comes from the `ways` table (`npoint_srid()` →
`get_srid_ways()`, not per-value), so there is no column-SRID typmod, yet the value still
carries a spatial stbox bbox — `temporal_typmod_in/out` + `tspatial_analyze` is correct.
Status: I1 + I2 resolved (`send/receive` order + `temporal_out`/`temporal_send` overloads,
`--validate` green); I3 needs no change; I4 is only an optional naming nit
(`trgeo_typmod_in`→`trgeometry_typmod_in`) — the `recv` arity is NOT an irregularity:
`recv(internal)` and `recv(internal, oid, integer)` are BOTH canonical PG (PG source
`findTypeReceiveFunction`; PG always passes 3 args via `ReceiveFunctionCall`), see
[[pg-receive-function-arity-both-valid]]. I5 (add the missing `analyze` typanalyze;
`tpc_typmod_in/out` are legitimate `pcid` typmod, kept) is a schema gap held for sign-off.
Pattern: per-family typmod semantics (npoint ways-SRID, pointcloud `pcid`) are legitimate —
`typmod_in`/`typmod_out`/`analyze` are independent per-family tokens, never forced uniform.

## 5. `TSpatial<T>` / `TGeo<T>` chapter — section-by-section

| `<sect1>` | MEOS prefix | generated? | canonical generator / notes |
|---|---|---|---|
| Input and Output | `tspatial_` | ✓ **GEN** | asText/asEWKT/asMFJSON + FromXxx constructors — the same `io_families`/`representation_families` axes as §4 (a family's type file carries both its Temporal<T>- and TSpatial<T>-level I/O in one region) |
| Conversions | `tspatial_`/`tgeo_` | ◐ PARTIAL | cross-class casts (e.g. `tgeompoint(tcbuffer)`, `tpose(trgeometry)`) are named inside the same `conversion_families` entries as §4, not a separate axis |
| Accessors | `tspatial_`/`tgeo_`/`tpoint_` | ✗ HAND | SRID, trajectory, traversedArea, convexHull … (in the family's `spatialfuncs` file — no template) |
| Transformations | `tspatial_`/`tgeo_` | ✗ HAND | setSRID, transform (in the family's `spatialfuncs` file — no template) |
| Restrictions | `tspatial_`/`tgeo_` | ✗ HAND | atGeometry/atStbox/minus… (in the family's `spatialfuncs` file — no template) |
| Spatial Reference System | `tspatial_` (`spatialfuncs`) | ✗ HAND | reserved position, no template |
| **Bounding Box Operations** | `tspatial_` | ✓ **GEN** | `topops`+`posops`+`boxops.c.tmpl` box type `stbox`, via the `subtypes:` track (§3) |
| Distance Operations | `tspatial_`/`tgeo_` (`distance`) | ✗ HAND | tDistance/nad/nai/shortestLine — reserved position, no template |
| Spatial Rel. → **Ever/Always** | `tspatial_`/`tgeo_` | ◐ PARTIAL | the SQL wrapper file is `subtypes:`-track-generated for the cast-delegated families (th3index, tquadbin, ts2cell, tnpoint — `spatialrels.sql.tmpl`); the underlying C ever/always kernel is separately generated for geo, cbuffer and rgeo via `spatialrel_families` (§3) while their own SQL wrapper files (212/170) stay hand; pose is hand at both levels |
| Spatial Rel. → Spatiotemporal | `tspatial_` (`tempspatialrels`) | ✓ **GEN** | `tempspatialrels.sql.tmpl`/`tempspatialrels_native.sql.tmpl` + `tempspatialrel_families` (§3) — `--gaps`: `tempspatialrel_families` 14/14, full `tspatial`-class coverage. Native impl (own C kernel): cbuffer, tgeo, tpoint. Cast impl: a family whose values are positions converts to the temporal geometry point its geometry names (tpose, tposechain, tnpoint, tpcpoint), while a cell-index or area-valued family converts its boundary to tgeometry (tquadbin, th3index, ts2cell, trgeometry, tpcpatch) |

Index infra (`gist`/`spgist`/`indexes`) is generated but is not a doc `<sect1>`.

### 5a. `Tcell<T>` (DGGS cell-index) — the descriptor-factored intermediate

`Tcell<T>` (prefix `tcellindex_`, `meos/src/temporal/tcellindex.c`) sits between
`TSpatial<T>` and the discrete cell types. It is a **first-party abstraction**: each
DGGS supplies **one `DggsCellOps` descriptor** (a table of Datum-convention static-cell
kernels + catalog identity), and the generic `tcellindex_*` entry points lift that
kernel via `tfunc_temporal`. Adding a DGGS (e.g. Google S2) = a descriptor + kernel,
**no new temporal scaffolding, SQL, or binding code** (`tcellindex.h:38-64`).

The generic inherited Tcell API (declared in the umbrella header
`meos/include/meos_cellindex.h:72-78`, implemented in `tcellindex.c`):
`tcellindex_get_resolution` · `is_valid_cell` · `cell_to_parent` · `cell_to_point` ·
`cell_to_boundary` · `cell_area`.

| aspect | state |
|---|---|
| C implementation | **unified once** via `DggsCellOps` — the `Tcell` C surface is effectively "generated" (single generic body, per-DGGS descriptor) |
| catalog predicate `tcellindex_type()` | **all three cell families** (`#if H3 → T_TH3INDEX`, `#if QUADBIN → T_TQUADBIN`, `#if S2CELL → T_TS2CELL`, `tcellindex.c:71-84`) |
| descriptor registered | `h3_cellops` (`meos/src/h3/th3index_ops.c:79`), `quadbin_cellops` (`meos/src/quadbin/tquadbin_ops.c:132`) and `s2_cellops` (`meos/src/s2cell/ts2cell_ops.c:146`), all dispatched from `dggs_cellops()` |
| SQL wrappers (getResolution/isValidCell/cellToParent/cellToPoint/cellToBoundary/cellArea) | **per-family HAND** in the `spatialfuncs` slot: h3 `255_th3index_spatialfuncs`, quadbin `355_tquadbin_spatialfuncs`, s2cell `605_ts2cell_spatialfuncs`; names are the bare DggsCellOps slot names overloaded by argument type — a second, independent surface from the generic `tcellindex_*` descriptor path above, not sourced from it |
| cell→boundary hook | the key inherited hook: `spatialrels.sql.tmpl` cast-delegates via `cellToBoundary($n)::tgeometry`, the bare DggsCellOps slot name — this IS generated (§6, h3 262 / quadbin 362 / s2cell 612) |

⇒ **Remaining opportunity**: all three DGGS families are wired onto `DggsCellOps`
and `tcellindex_type()`, so the C implementation is unified and the two paths (typed
per-family functions, generic descriptor lift) agree slot-for-slot; the per-family
SQL cell wrappers (`255_th3index_spatialfuncs`, `355_tquadbin_spatialfuncs`,
`605_ts2cell_spatialfuncs`) could now be generated from a single `tcellindex`
template instead of hand-written three times, since the underlying descriptor
surface is identical across the three.

⛔ **THE THREE ARE AT PARITY IN THE SHIPPED SQL AND NOT IN THE MANIFESTS.**
`mobilitydb/sql/s2cell/` holds 600–623, the same fourteen slot files `quadbin` holds
at 350–373, and `generate.py --check` emits the same seven per family — s2cell 604,
608, 609, 612, 614, 622, 623 against quadbin 354, 358, 359, 362, 364, 372, 373. But
`--gaps` names `ts2cell` as the ONLY missing member of ten temporal-class axes, and
`s2cellset` as the only missing member of two value-domain axes:

| axis | coverage | missing |
|---|---|---|
| `accessor_families`, `comparison_families`, `constructor_families`, `hash_families`, `io_families`, `modification_families`, `representation_families`, `restriction_families`, `transformation_families` | 19/20 each | `ts2cell` |
| `conversion_families` | 15/20 | `ts2cell`, and tposechain, tgeography, tpcpoint, tpcpatch |
| `setop_families` | 17/18 | `s2cellset` |
| `span_families` | 27/28 | `s2cellset` |

⭐ Where S2 is already whole: `aggregate_families` 20/20 (`611_ts2cell_aggfuncs`),
`compops_families` 20/20, `tempspatialrel_families` 14/14 (`614_ts2cell_tempspatialrels`)
and `tiling_families` 20/20 (`607_ts2cell_boxops`).
⛔ `distance_families` 10/16 is missing `s2cellset` AND `h3indexset`, `quadbinset`,
`posechainset`, `pcpointset`, `pcpatchset` — a set-type-wide gap, NOT an S2 one; reading
it as one over-counts the cell work.
⛔ `tiling_families` names `th3index` and `quadbin` on their temporal-type file
(253/353) beside their `_boxops` entry, and names S2 through `s2cell_boxops` alone.
The shipped surface does not diverge — all three temporal-type files carry 97
`CREATE FUNCTION` — so this is a governance gap, not a missing function.

## 6. Per-family gap — every inherited `.in.sql` file, generated vs hand

Each cell = the live file number (`mobilitydb/sql/<fam>/`). **Bold** = the
file is emitted by the generator today (in that subtype's `manifest.d/` `files:`);
plain = the file exists but is still hand-maintained.

| family | compops | spatialfuncs | topops | posops | distance | aggfuncs | spatialrels | tempsp.rels | idx / gist·spgist | boxops |
|---|---|---|---|---|---|---|---|---|---|---|
| cbuffer (200) | **204** | 205 | **208** | **209** | 210 | **211** | 212 | **214** | **216** | 207 |
| npoint (300) | **304** | 306 | **308** | **309** | 312 | **314** | **320** | **322** | **316** | 307 |
| pose (100) | **104** | 105 | **108** | **109** | 110 | **111** | 112 | **114** | **116** | 107 |
| rgeo (150) | **154** | 156 | **161** | **162** | 164 | **168** | 170 | **172** | **173** | 160 |
| h3 (250) | **254** | 255 | **258** | **259** | — | **261** | **262** | **264** | **272**·**273** | **257** |
| quadbin (350) | **354** | 355 | **358** | **359** | — | **361** | **362** | **364** | **372**·**373** | **357** |
| s2cell (600) | **604** | 605 | **608** | **609** | — | **611** | **612** | **614** | **622**·**623** | **607** |

Reading the table:
- **`compops`/`topops`/`posops`/`idx` are generated for every derived family** via the
  `subtypes:` `files:` track (§3): cbuffer, npoint, pose, rgeo (`[compops, topops,
  posops, indexes]`) and h3, quadbin, s2cell (`[compops, posops, topops, spatialrels,
  gist, spgist]` — the three cell families declare one and the same list).
- **`aggfuncs` is generated for cbuffer, npoint, pose, posechain, rgeo, h3, quadbin,
  s2cell** via the whole-file `aggregate_families` axis (§4b), not the `subtypes:`
  track (`--gaps`: `aggregate_families` 20/20, full coverage).
- **`tempsp.rels` is generated for cbuffer, npoint, pose, posechain, pointcloud
  (both types), rgeo, h3, quadbin, s2cell** via `tempspatialrel_families` (§3/§5) —
  native for cbuffer, cast-delegated for the other nine (`--gaps`:
  `tempspatialrel_families` 14/14, full coverage).
  ⭐ The cast target follows the value's geometry: an AREA-valued family reaches
  `tgeometry`, a POINT-valued one reaches `tgeompoint`. It is load-bearing rather
  than cosmetic — a `tgeompoint` carries linear interpolation and a `tgeometry`
  cannot, two consecutive geometries sharing neither type nor vertex
  correspondence — so reaching past a point's own target would refuse every linear
  value of the family.
  ⭐ A family declares the matrix of ITS target rather than a fixed six by three:
  `tpose`, `tposechain` and `tnpoint` declare the 13 cells the `tpoint` entry
  declares, since a moving point neither contains nor covers a geometry and two
  moving points do not touch. `tpcpoint` declares that same matrix; `tpcpatch`,
  whose target is `tgeometry`, declares every predicate in every direction.
  ⛔ A delegating family declares the matrix ITS TARGET declares, and a dimension a
  schema may or may not state is not a property of the type: a point cloud schema
  resolves X/Y/Z/M by NAME (`pc_schema_check_xyzm`), so one declaring X and Y alone
  answers the planar `tContains`/`tCovers`/`tTouches` while one carrying Z meets the
  engine's `The tgeompoint cannot have Z dimension`, exactly as a 3-D `tgeompoint`
  does.
- **`spatialrels` SQL** (the ever/always wrapper *file*) is generated for the
  **cast-delegated families** (h3 262, quadbin 362, s2cell 612, npoint 320,
  tpcpoint 443, tpcpatch 449) via the `subtypes:` `spatialrels` behaviour — a
  cell-boundary→`tgeometry` cast for the three cell families, a `tnpoint::tgeompoint` /
  `tpcpoint::tgeompoint` cast for the two point families, a
  `tpcpatch::tgeometry` cast for the patch. ⛔ `native_ever_intersects` is the
  additive flag a family sets when its type file already declares
  `eIntersects(<temp>, geometry)` over a kernel that reads the value directly —
  `tpcpatch` walks the points of each instant and short-circuits — so the emitted
  file covers 35 of the 36 cells and leaves that one where it is. cbuffer 212,
  pose 112 and
  rgeo 170 stay hand — even though their underlying **C** ever/always kernel
  dispatch is generated (next bullet).
  ⛔ A family whose VALUES are positions carries `point_target: true`, the additive
  flag (`DEFAULT_FALSE_FLAGS`) whose `-- @IFNOT point_target` guards leave it the
  matrix its temporal geometry point target declares: `eContains`/`aContains` and
  `eCovers`/`aCovers` take the geometry first only, and `eTouches`/`aTouches` have
  no direction between two values of the family. A cell-index family, whose value
  is a boundary polygon, omits the flag and keeps all three directions.
- **`spatialfuncs` and `distance`** are generated for no family — no template
  exists; hand everywhere.
- **`boxops`** (the box-cast `.in.sql` file holding `spans`/`splitNSpans`/
  `splitEachNSpans`, not the C dispatcher) is generated via the `tiling_families`
  axis (reference-block entries, no dedicated template) for **h3 (257)**,
  **quadbin (357)** and **s2cell (607)** in this table — plus, outside the
  `subtypes:`-track family set
  above, for json/tjsonb (`459`) and pointcloud (`435`, one file covering both
  tpcpoint and tpcpatch). cbuffer (207), npoint (307), pose (107) and rgeo (160)
  stay hand.
- **The point cloud index classes** live in the two files of the family:
  `439_tpc_gist` and `440_tpc_spgist` hold the classes of `tpcbox` beside those
  of `tpcpoint` and `tpcpatch`, as `073`/`074` hold the classes of `stbox`
  beside the temporal spatial ones, and `438_tpc_distance` precedes them so a
  class names the ordering operator it registers. All three stay hand.
- The C ever/always spatial-rel **kernel** (`spatialrel_families` axis, §3) is
  generated in the **geo**, **cbuffer** and **rgeo** files (`tgeo_spatialrels.c`,
  `tcbuffer_spatialrels.c`, `trgeo_spatialrels.c`: contains/covers/disjoint/
  intersects/dwithin); pose's native C spatial-rel wrapper is still hand (memory
  `spatialrel-wrapper-surface-is-inherited-generate-it`) — npoint needs no native
  kernel, its ever/always relationships cast-delegate to `tgeometry` in pure SQL
  (320, previous bullet).
- The **geo/tpoint/tgeo** family SQL surfaces are not in the `subtypes:` list at all
  (geo is the hand-written reference layout the generator derives from) — **except
  `compops`**: `temporal`/`tgeo`/`tpoint` each carry an explicit multi-pair
  `compops_families` entry feeding the same engine as the `subtypes:` track, so
  `030_temporal_compops`/`054_tgeo_compops`/`054_tpoint_compops` are fully
  generated, whole-file, like every `subtypes:` compops file.

## 7. The gap (what remains ungoverned)

**A. Behaviours with a template, still not wired for every family.** Every count
below is what `generate.py --gaps` prints today — read it from the tool rather
than from this paragraph, which is a transcription and can only be as fresh as
its last edit:
- `spatialrel_families` (the C ever/always kernel), **6/14**: only `geo`, `cbuffer`
  and `rgeo` carry one, so every other member of `tspatial_type` reads missing.
  `tnpoint` needs none by design — its ever/always relationships cast-delegate to
  the temporal geometry point at the SQL level instead (§3, §6, the `subtypes:`
  `spatialrels` bullet below) — and the pointcloud temptypes, members of this class since
  `tspatial_type` widened to 14, carry no native kernel either.
- `conversion_families`, **15/19**: `tposechain`, `tgeography`, `tpcpoint`, `tpcpatch`.
  ⛔ `tposechain`'s absence here is NOT a template gap and must not be closed by
  cloning `pose`: the two conversions differ in KIND, `tpose` answering a point
  (`tgeompoint`/`tgeogpoint` over `Tpose_to_tpoint`) where `tposechain` answers
  the COMPOSITION of its links (`tpose` over `Tposechain_to_tpose`). A per-family
  conversion is correctly hand-written.
- `comparison_families` and `hash_families` are both **19/19**. ⭐ What the point
  cloud pair needed was not a template but the LAYOUT its siblings have: the bare
  `/****/` divider between the B-tree opclass and the hash block. A region starts
  at the divider preceding its `begin` anchor, so with the two sections sharing
  one banner the hash entry had to reproduce the comparison section as a literal —
  and that literal then preserved the section's divergences (`Comparison /
  B-tree / hash` for the canonical banner, `boolean`/`integer` for `bool`/`int4`,
  eq-first ordering, a one-line operator head, wider opclass spacing). Adding the
  divider gives each behaviour one owner and no transcription.
- `aggregate_families` is **20/20**: `561_tposechain_aggfuncs.in.sql` carries the
  surface its siblings carry, every statement binding a generic transition or final
  function, so the family needs no kernel of its own.
- `tempspatialrel_families` is **14/14**: `448_tpcpatch_tempspatialrels.in.sql`
  closes the axis. Its conversion is the one the family lacked — `pcpatch_to_geom`
  reads a patch as the `MULTIPOINT` of the positions its points occupy, and
  `tpcpatch_to_tgeometry` lifts that over time — so the manifest line rests on a
  MEOS entry rather than on a cast chain that already existed.
- `distance_families`, **10/16** of the value-domain types: `posechainset`,
  `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset`. `posechainset` is
  deliberate and `setfamilies.yaml` says so in place — a pose chain has no
  distance function, so the set deploys none.
- `topop_families` and `posop_families`, both **7/13**: `tgeompoint`, `tgeogpoint`,
  `tgeometry`, `tgeography`, `tpcpoint`, `tpcpatch`.
  ⛔ THOSE SIX ARE THREE PAIRS, AND THE PAIR IS THE WHOLE REASON. A covered family
  declares ONE temporal type per file — `108_tpose_topops` names `tpose`, `stbox`
  and `tstzspan` and nothing else — while each missing file carries TWO:
  `061/062_tgeo_*` name `tgeometry` AND `tgeography`, `061/062_tpoint_*` name
  `tgeompoint` AND `tgeogpoint`, and `436/437_tpc_*` name `tpcpoint` AND
  `tpcpatch` over a `tpcbox` rather than an `stbox`. MEASURED: the tpose render
  declares 5 operand pairs, the committed geo file 10.
  ⇒ `render()` substitutes ONE `(base, temp)` pair, so a cloned entry would emit
  HALF of each of those files and silently drop the other temporal type. Closing
  this needs the template taught to interleave two temporal types — a generator
  capability, not a manifest entry — which is the same limit recorded for the
  three families that stay on `compops_families`.

- The `subtypes:` `spatialrels` behaviour (the ever/always SQL wrapper *file*, as
  opposed to the C kernel above) covers the cast-delegated families (th3index,
  tquadbin, ts2cell, tnpoint); cbuffer/pose/rgeo's own wrapper files (212/112/170) stay
  hand even though cbuffer's and rgeo's C kernel is generated.

⭐ **THE DISCRIMINATOR THAT DECIDES WHETHER A GAP IS MECHANICAL**, and it is one
command: take the behaviour's `begin`/`end` markers from the covered sibling's
manifest entry, cut that section out of BOTH families' `.in.sql`, and compare the
two with the type tokens renamed. Identical ⇒ the surface is already the template's
output and a cloned manifest entry is behaviour-neutral, provable by a regeneration
that moves zero bytes. Different ⇒ the surface is per-family and the entry would
CHANGE it. Six of `tposechain`'s seven candidate behaviours came out identical and
were closed that way; `conversion_families` came out different and is the reason
the bullet above refuses it.

**B. Sections with no template at all** (reserved `positions:` slot, pure hand):
- The geo base-geometry surface (`geo/049_geo_funcs`): functions whose argument
  is a `geometry` rather than a temporal type. It belongs to no `subtypes:`
  track, since the axes project a behaviour across the temporal families and
  this file has one entry per function, and it is named in
  `coverage_exceptions.txt`.
- `distance` (tDistance/nad/nai/shortestLine).
- `spatialfuncs` (SRID / transform / trajectory / atGeometry / atStbox
  scaffolding — the TSpatial<T>-level Accessors/Transformations/Restrictions/SRS
  rows of §5).
- Comparisons → Miscellaneous (§4, `temporal_types_p2.xml` `ttype_miscellaneous`).
- TNumber's `036_tnumber_distance` and the tbool/ttext-specific `028`/`029` files
  (§4a).

**C. The per-family SQL surface is generated end to end.** Input/Output,
Constructors, Transformations, Modifications, Restrictions and Accessors (§4c,
`accessors.sql.tmpl` + manifest `types:` tokens — the value-shaped rows via
`base`/`baseset`/`valret`, the rest generically; only interleaved/positional
accessors stay hand) all have a `reference: true` axis at full or near-full
class coverage (§4/§4b). Conversions is the one SQL axis still partial (§4, 15/18).
The MEOS-**C** value surface (`start_value`/`value_at_timestamptz`/`values`/
`at_value`… in `meos/src/<fam>.c`) is not generated at all — that is the next tier
of this program, not a template gap in `tools/codegen/inherited/`: a
`temporal_basetype.c.tmpl` (no such template exists yet), byte-for-byte reference
`meos/src/json/tjsonb.c`, cloneable for the value-opaque families (jsonb/pcpoint/
pcpatch/cbuffer). The remaining hand sections are the subject of the binding
generators (see memory `mobilityduck-tcbuffer-full-implementation-roadmap`).

## 8. Comparison with the MEOS-API generated hierarchy

The MEOS-API catalog derives the ecosystem class hierarchy from a **curated** object
model, `meta/object-model.json` `lattice` (MEOS-API master `65ced3016`). It declares
**18 classes**: Temporal · TAlpha{TBool,TText} · TNumber{TInt,TFloat} ·
TSpatial{TGeo{TPoint{TGeomPoint,TGeogPoint}, TGeometry, TGeography}, TCbuffer, TNpoint,
TPose, TRGeometry}. Diffed against the live MEOS catalog predicates
(`meos_catalog.c` @ MobilityDB `c85c0e1d6`), these live types/classes are **missing**:

| missing from lattice | live type / predicate | belongs under | category |
|---|---|---|---|
| **TBigint** | `tbigint` (`tnumber_type` :1214) | TNumber | **in-scope leaf, omitted (defect)** — number family IS in `scope.inScopeTypeFamilies` |
| **TJsonb** | `tjsonb` (`talpha_type` :1192) | TAlpha | in-scope family (alpha), omitted leaf |
| **TH3Index** | `th3index` (`tspatial_type` :1374) | TSpatial → Tcell | deferred family (not in declared scope) |
| **TQuadbin** | `tquadbin` (`tspatial_type` :1375) | TSpatial → Tcell | deferred family |
| **TS2Cell** | `ts2cell` (`tspatial_type` :1375) | TSpatial → Tcell | deferred family |
| **TPcpoint** | `tpcpoint` (`tpointcloud_temptype` :1204) | TSpatial → TPointcloud | deferred family (`#if POINTCLOUD`) |
| **TPcpatch** | `tpcpatch` (`tpointcloud_temptype` :1204) | TSpatial → TPointcloud | deferred family |
| **Tcell / TCellIndex** (abstract) | `tcellindex_type()` | between TSpatial and cell leaves | missing intermediate |
| **TPointcloud** (abstract) | `tpointcloud_temptype()` | between TSpatial and pointcloud leaves | missing intermediate |

Notes:
- The lattice's `scope.inScopeTypeFamilies` = `[temporal, alpha, number, geo, point,
  cbuffer, npoint, pose, rgeo]` — it does **not** list h3/quadbin/s2cell/pointcloud,
  so those are *declared* deferrals. But **TBigint / TJsonb** belong to in-scope families
  (number / alpha) and are silent omissions → genuine curation defects.
- The model's own correction **OM-M7 is stale**: it states `tpcpoint`/`tpcpatch`
  are "absent from master MEOS (0 hits)", but live master **has** them
  (`meos_catalog.c:164/167` + `tpointcloud_temptype()` predicate). The curated lattice
  lags the live catalog.
- **RASTER** (`raquet`) is a base value type, not temporal — correctly absent from a
  `Temporal<T>` lattice.
- The object model is **curated, not auto-derived** (`"no class is guessed"`), so
  these are additions to make in `meta/object-model.json` — the fix is to add the
  missing leaves/intermediates (and widen `scope`) so every binding derives them.

## 9. Value-domain classes — `Set<T>` / `Span<T>` / `SpanSet<T>`

The finite-subset value-domain types that the temporal restriction/accessor
surface consumes (§4a). Ordering authority: **`doc/set_span_types.xml`**. All
catalog/doc line numbers in this section are live at master `c85c0e1d6`; manifest
axes are cited by `manifest.d/<axis>.yaml` filename, not by line number.

### 9.1 Class membership (live `meos/src/temporal/meos_catalog.c`)

| class | members | catalog |
|---|---|---|
| `Set<T>` (**18**) | intset, bigintset, floatset, textset, dateset, tstzset, geomset, geogset, npointset, poseset, posechainset, cbufferset, jsonbset, h3indexset, quadbinset, s2cellset, pcpointset, pcpatchset | `MEOS_SETTYPE_CATALOG` :262-280 · `set_type()` :801-808 · `set_basetype()` :787-794 |
| `Span<T>` (**5**) | intspan, bigintspan, floatspan, datespan, tstzspan | `MEOS_SPANTYPE_CATALOG` :287-295 · `span_type()` :982-987 |
| `SpanSet<T>` (**5**) | intspanset, bigintspanset, floatspanset, datespanset, tstzspanset | `MEOS_SPANSETTYPE_CATALOG` :301-309 · `spanset_type()` :1080-1085 |

Sub-predicates: `spatialset_type()` = geomset, geogset, npointset, poseset,
posechainset, cbufferset, h3indexset, quadbinset, s2cellset, pcpointset, pcpatchset
(**10**). `numset_type()` · `timeset_type()` · `geoset_type()` ·
`alphanumset_type()` · `pointcloudset_type()` = pcpointset, pcpatchset, which
names the two sets whose dimensions need the schema, not a class outside the
spatial one.

⭐ **`spatialset_type()` NAMES SPATIALITY, AND THE BBOX QUESTION IS ANSWERED
ELSEWHERE.** The two came apart at the pointcloud family: `set_bbox_size()`
(`set.c`) returns `0` for a pointcloud set, while the family publishes
`SRID(pcpoint)`, `SRID(pcpatch)`, `SRID(tpcpoint)` and `SRID(tpcbox)`, and
`SRID(tpcpoint)` binds the GENERIC `Tspatial_srid`. A type carrying an SRID is
spatial, so a site meaning *stores a box* now reads `type_bboxtype(t) == T_STBOX`
and the spatial predicate answers only about spatiality. Three properties were
conflated — carries an SRID, has schema-derivable flags, stores a bbox — and each
has its own predicate now.
⛔ The SRID is what a set's EXTENDED binary form writes, so all four sites deciding
it read one predicate: `set_to_wkb_size`, `set_to_wkb_buf`, `set_flags_to_wkb_buf`
and, on the reading side, `set_flags_from_wkb_state`, which honours the wire's own
SRID bit. A reader re-deriving that decision from a base-type predicate reads the
element count out of the SRID's bytes.

⭐ **THE CELL SETS ARE NOT THE SAME CASE.** `h3indexset`/`quadbinset`/`s2cellset` sit in
`spatialset_type()` and declare no `SRID`/`setSRID`/`transform`, and that is
correct: their specifications IMPOSE the reference system rather than storing one —
H3 is spherical coordinates with the WGS84/EPSG:4326 authalic radius
(https://h3geo.org/docs/core-library/overview/) and a quadbin cell is a Bing Maps
Tile System cell of a map subdivided in the Mercator projection
(https://docs.carto.com/data-and-analysis/analytics-toolbox-for-bigquery/key-concepts/spatial-indexes).
`npoint` is the third form of the same exception: the `Npoint` struct
(`meos_npoint.h:51-55`) declares `rid` and `pos` and no `srid`, the SRID being
inherited from the `ways` network table.

### 9.1b `SpatialSet<T>` — the subclass surface, derived from `Spatial<T>`

`SpatialSet<T>` = `spatialset_type()` (**11** members: `geomset` `geogset`
`npointset` `poseset` `posechainset` `cbufferset` `h3indexset` `quadbinset`
`s2cellset` `pcpointset` `pcpatchset`, `meos_catalog.c:950-955`). Its surface is the set-lift
of what a spatial BASE type carries BECAUSE it is spatial — the operations left
after removing what every value type has (comparisons, `hash`, the set operations,
ever/always, `asText`/`asBinary`/`asHexWKB`). Counted from the `CREATE FUNCTION`
declarations under `mobilitydb/sql/`:

| operation | why it is spatial | sets carrying it |
|---|---|---|
| `SRID` `setSRID` `transform` `transformPipeline` | identity of the reference system | 5/11 — absent from `npointset`, `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset`, whose types IMPOSE or INHERIT the system (§9.1) |
| `stbox` | the spatial bounding extent | 6/11 — absent from `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset` |
| `asEWKT` `asEWKB` `asHexEWKB` | the SRID-CARRYING representations | 6/11 — the `E` forms mirror their bases, so the same five are absent |
| `round` | rounding of COORDINATES | 6/11 — the same five; a cell id carries no coordinates to round |
| `distance` | ⭐ the OVERRIDE, below | 5/11 — absent from `posechainset`, `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset` |

⭐ **`distance` IS AN OVERRIDE, NOT AN ADDITION, AND IT IS THE ONE PLACE THE
SUBCLASS REDEFINES A SUPERCLASS BEHAVIOUR.** `Set<T>` answers the distance of the
extent that BOUNDS it — `distance_set_value()` builds the set's span and measures to
that, so a value falling in a gap between elements answers `0`. A spatial set has no
span, so `distance_spatialset_value()` / `distance_spatialset_spatialset()`
(`meos/src/geo/tspatial.c`) measure to the set's STBox instead, and
`distance_set_value()` selects on `spatialset_type()`. The CONTRACT is unchanged:
this is the same rule PR #2117 settles for a span set, whose adjacency and distance
answer for its bounding span because *the holes between its spans are not boundaries
of it* — `distance_spanset_value()` is literally `distance_span_value(&ss->span, …)`.
⛔ A nearest-ELEMENT distance would CONTRADICT that ruling; it belongs to a
separate, differently-named operation, never to a redefinition of `setDistance`.

⛔ **THE ONE CELL NO PROPERTY EXPLAINS:** `stbox(h3index)`, `stbox(quadbin)` and
`stbox(s2cell)` are declared while `stbox(h3indexset)`, `stbox(quadbinset)` and
`stbox(s2cellset)` are not, though all three sets are named by `spatialset_type()`
and so already store an STBox. A bounding box needs no agreement on a datum, so
unlike the SRID rows this is a lift gap, and it is the same gap in all three.

### 9.2 WHY the 17-vs-5 asymmetry

- **Every base type has a set type** because a temporal value is a *function*
  from time to the base domain and `getValues` returns its RANGE as a set
  (§4c row 6) — a family cannot have a temporal type without its base set type.
  `posechainset` is what `tposechain` needs to answer `getValues`, and it
  deploys the whole inherited set surface ahead of it.
- **`Span<T>` needs a total order AND a meaningful contiguous interval** on the
  base domain, so it exists only for `span_basetype()` :963-967 = date, float,
  int, bigint, timestamptz — numbers + time. `span_canon_basetype()` :973-976 =
  date, int, bigint marks the **discrete** bases whose spans canonicalize with
  +1 (upper bound normalized to exclusive).
- **Order alone is NOT enough**: text is ordered — textset deploys the full
  `<< >> &< &>` position surface in `mobilitydb/sql/temporal/002_set_ops.in.sql`
  — but there is no textspan (`span_basetype()` excludes `T_TEXT`): a
  contiguous interval of texts is not meaningful. The same boundary shows in
  indexing: `013_set_indexes.in.sql` lifts each set through its bounding span,
  so its GiST/SP-GiST opclasses exist exactly for the five span-basetype sets
  (the file contains no textset occurrence), and its GIN opclasses exist only
  for intset, bigintset and dateset — the `span_canon_basetype()` discretes.

### 9.3 `doc/set_span_types.xml` — section-by-section

Per class: ✓ GEN = the section's SQL is generator-governed (a `reference: true`
manifest entry re-renders it byte-for-byte under `--validate`; the `axis`
column names the `manifest.d/` key) · ✗ HAND = hand-maintained.

| `<sect1>` (doc line) | `Set<T>` | `Span<T>` | `SpanSet<T>` | axis |
|---|---|---|---|---|
| Input and Output (:102) | ✓ GEN | ✓ GEN | ✓ GEN | `span_families` (003/007 entries; sets: `set_io` + the per-family `*set_io` entries) |
| Constructors (:268) | ✓ GEN | ✓ GEN | ✓ GEN | `span_families` (sets: `set_constructors` + `*set_constructors`; h3/quadbin/pointcloud fold their singleton conversion + cast into this section; `s2cellset` is not governed here — `--gaps`: `span_families` 27/28, §5a) |
| Conversions (:325) | ✓ GEN | ✓ GEN | ✓ GEN | `span_families` (sets: `set_conversions` + `*set_conversions`; h3/quadbin/s2cell/pointcloud have no separate Conversions section — see Constructors) |
| Accessors (:442) | ✓ GEN | ✓ GEN | ✓ GEN | `span_families` (sets: `set_accessors` + `*set_accessors`; the pointcloud entries also carry the trailing `unnest`) |
| Transformations (:693) | ✓ GEN | ✓ GEN | ✓ GEN | `span_families` (sets: `set_transformations` + the per-family `*set_transformations`/`*set_unnest` entries; jsonbset's empty Transformations banner stays hand) |
| Spatial Reference System (:901) | ✓ GEN (geoset/poseset/posechainset/cbufferset — the only set files with an SRS section; npointset/h3indexset/quadbinset/s2cellset/pcpointset/pcpatchset have no SRID functions) | — | — | `span_families` (`*set_srs` entries) |
| Set Operations (:958) | ✓ GEN | ✓ GEN | ✓ GEN | `manifest.d/setop_families.yaml` · `span_families` (005/009); `s2cellset` is not governed here — `--gaps`: `setop_families` 17/18, §5a |
| BBox Ops · Topological (:1014) | ✓ GEN | ✓ GEN | ✓ GEN | `manifest.d/topop_families.yaml` · `span_families` (005/009) |
| BBox Ops · Position (:1082) | ✓ GEN (ordered sets only) | ✓ GEN | ✓ GEN | `manifest.d/posop_families.yaml` · `span_families` (005/009) |
| BBox Ops · Splitting (:1162) | ✓ GEN (`spans`/`splitNSpans`/`splitEachNSpans` live in `003_span.in.sql`) | ✓ GEN | ✓ GEN | `span_families` (003/007 entries) |
| Distance (:1219) | ✓ GEN (metric sets only) — `--gaps` 10/16, missing `posechainset`, `h3indexset`, `quadbinset`, `s2cellset`, `pcpointset`, `pcpatchset`; the denominator is `numset` ∪ `timeset` ∪ `spatialset`, and the spatial families answer through the `SpatialSet<T>` override of §9.1b | ✓ GEN | ✓ GEN | `manifest.d/distance_families.yaml` · `span_families` (005/009) |
| Comparisons (:1248) | ✓ GEN | ✓ GEN | ✓ GEN | `manifest.d/comparison_families.yaml` (`set` family entries) + `manifest.d/hash_families.yaml` · `span_families` (003/007) |
| Aggregations (:1306) | ✓ GEN — the `extent` aggregates over sets in `015_span_aggfuncs.in.sql`, `setUnion` in `001_set.in.sql`, and the per-family `setUnion` regions | ✓ GEN | ✓ GEN | `span_families` (015 entry + `set_aggregations` + the per-family `*set_setunion` entries) |
| Indexing (:1389) | ✓ GEN (span-basetype sets only, §9.2) | ✓ GEN | ✓ GEN | `span_families` (011/012/013 entries) |

The `Set<T>` backlog is CLOSED: every section of `001_set.in.sql` and of the
8 per-family set files (§9.5) is generator-governed by a `reference: true`
`span_families` entry (region-in-file entries carry an exact `end:` anchor).
The only ungoverned residue is jsonbset's empty `Transformations` banner
(nothing to render).

### 9.4 The template-class principle

`Span`/`SpanSet`/`TBox` are each **ONE C struct parameterized by a basetype
field**, not per-instantiation structs: `Span` (`meos/include/meos.h:154-163`,
`spantype`/`basetype` fields :156-157), `SpanSet` (:168-179), `TBox` = two
`Span`s (:184-189); `Set` likewise (:140-149). One implementation dispatches on
the basetype for every instantiation, so **generation must happen at the
TEMPLATE level**: one `span<T>` template covers intspan/bigintspan/floatspan/
datespan/tstzspan and every future instantiation. Generating per instantiation
is the anti-pattern (see memory
`span-spanset-tbox-are-template-classes-generate-at-template-level`).

The `span_families` axis realizes this: each template file is ONE manifest
entry whose `insts:` table maps the five instantiations to their tokens, and
every stanza is written once against the tokens (§9.7) — a sixth
instantiation would be one more `insts:` row, not another SQL file.

### 9.5 Per-family file map (`mobilitydb/sql/`)

The template-class reference layer lives in `temporal/`; ✓ marks files whose
whole body is generator-governed by a `span_families` entry:

| class | type | ops | indexes | aggfuncs |
|---|---|---|---|---|
| `Set<T>` | `001_set` | `002_set_ops` (regions ✓ via the set axes) | `013_set_indexes` ✓ | shares `015` ✓ |
| `Span<T>` | `003_span` ✓ | `005_span_ops` ✓ | `011_span_indexes` ✓ | `015_span_aggfuncs` ✓ |
| `SpanSet<T>` | `007_spanset` ✓ | `009_spanset_ops` ✓ | `012_spanset_indexes` ✓ | shares `015` ✓ |

plus the 10 per-family set files that instantiate `Set<T>` for the plug-in base
types: `geo/050_geoset` · `pose/101_poseset` · `cbuffer/201_cbufferset` ·
`h3/251_h3indexset` · `npoint/301_npointset` · `quadbin/351_quadbinset` ·
`pointcloud/400_pcset` · `json/450_jsonbset` · `posechain/551_posechainset` ·
`s2cell/601_s2cellset` (the jsonb-specific path/element
operators live separately in `json/451_jsonbset_jsonfuncs`). Their comparison /
hash / set-operation / topological / position / distance regions are governed
by the per-surface set axes; their I/O, constructor, conversion, accessor,
transformation, SRS and setUnion sections are governed by the region-in-file
`span_families` entries of §9.3.

### 9.6 What the `setfamilies:` manifest axis encodes

The `setfamilies:` axis (`manifest.d/setfamilies.yaml`) carries one row per set type
with the classification tokens of `Set<T>`. It is the descriptive table whose
flags the per-surface set axes implement as deployment gates.

| token | meaning |
|---|---|
| `ordered` | the base has a SEMANTIC total order (int, bigint, float, text, date, timestamptz) — **this flag is what decides whether position operators are emitted at all**; the 10 unordered bases get none |
| `posops_spelling` | `value` = `<< >> &< &>` (numbers + text) · `time` = `<<# #>> &<# #&>` (date + timestamptz); omitted when not ordered |
| `metric` | `<->` / `setDistance` is deployed (all ordered bases except text, plus geomset/geogset/npointset/poseset/cbufferset) — the gate of `manifest.d/distance_families.yaml` |
| `spatial` | the set STORES A BOUNDING BOX and carries the SRS section (§9.3). ⛔ NOT `spatialset_type()`, which answers whether the elements carry an SRID: pcpointset/pcpatchset are spatial sets that store no box, so they are `false` here |

Known deployed irregularity the axis does not model: jsonbset has the `<<`/`>>`
pair (`json/450_jsonbset.in.sql:378-393`, `Left_set_set`/`Right_set_set`)
without `&<`/`&>`, though jsonb has no semantic order.

### 9.7 What the `span_families:` axis encodes

One entry per template file (`003_span`, `005_span_ops`, `007_spanset`,
`009_spanset_ops`, `011_span_indexes`, `012_spanset_indexes`,
`015_span_aggfuncs`, `013_set_indexes`), rendered by `render_spanfile()`
(`generate.py`) over the entry's `blocks:`; the region is the whole file body
from `begin:` to EOF.

| key | meaning |
|---|---|
| `insts:` | the per-instantiation token rows: `{v}` base value type, `{sp}` span, `{ss}` spanset, `{vs}` value set, `{rg}`/`{mr}` the PostgreSQL range/multirange counterparts — absent for float, which has no canonical range type |
| `order:` | default emission order `[int, bigint, float, date, tstz]`; a block may override it (several WKB stanzas order tstz before date) or restrict to a subset (`only:` / a block-level `order:` such as the discrete-only GIN opclasses) |
| `sig`/`ret`/`sym` | one CREATE FUNCTION per instantiation from the shared four-line skeleton (`templates/comparisons.sql.tmpl`) |
| `stmt` | one arbitrary CREATE statement per instantiation (operators, opclasses, type shells, casts, aggregates) |
| `group` | a type-outer stanza: a list of templates emitted together per instantiation — the shape of the I/O, cast and operator sections, where a type's whole cluster precedes the next type's (`sep:` declares the cluster separator) |
| `lit` | verbatim text: banners, one-off statements, and encoded irregularities (e.g. the one-space AS indentation of `hash(intspan)`/`hash(bigintspan)` in `003_span.in.sql:1167/1171`) |

`span`/`spanset` are the MEOS counterparts of PostgreSQL `range`/`multirange`
(the `{rg}`/`{mr}` tokens carry the cast targets): the API inherits from
PostgreSQL's range/multirange and orthogonalizes it across the MEOS base
types. Recorded
consequence: `span_sel`/`span_joinsel` are the `rangesel` counterpart and
belong to the value-domain types (the RESTRICT/JOIN estimators of the set and
span operators, e.g. `003_span.in.sql` and the `001_set.in.sql` comparison
operators), while the `scalar*sel` estimators belong to BASE types (e.g.
`cbuffer/200_cbuffer.in.sql`).

## 10. Selectivity — which estimator a generated operator declares

### 10.1 The rule: the bounding-box class of the temporal type

Each of the three temporal classes carries a different bounding box, and the box
is what the RESTRICT/JOIN estimators read. The estimator pair therefore follows
the **class of the temporal operand's own box**, never the class of the other
operand and never the class of the dimensions the two share:

| class | catalog predicate | bounding box | RESTRICT / JOIN |
|---|---|---|---|
| `Temporal<T>` (talpha) | `talpha_type` | `Span` (tstzspan) | `temporal_sel` / `temporal_joinsel` |
| `TNumber<T>` | `tnumber_type` | `TBox` (value span + tstzspan) | `tnumber_sel` / `tnumber_joinsel` |
| `TSpatial<T>` | `tspatial_type` | `STBox` (X/Y/Z + tstzspan + SRID) | `tspatial_sel` / `tspatial_joinsel` |
| `TPointcloud<T>` | `tpointcloud_temptype` | `TPCBox` (STBox prefix + `pcid`) | `tspatial_sel` / `tspatial_joinsel` |
| `Set<T>`/`Span<T>`/`SpanSet<T>` | §9.1 | the value domain itself | `span_sel` / `span_joinsel` |
| base types | — | — | `scalar*sel` / `eqsel` / `neqsel` |

A mixed-class operator does **not** change the estimator. `tgeometry && tstzspan`
declares `tspatial_sel`, `tint && tstzspan` declares `tnumber_sel`, and both are
correct, because the dispatcher converts the constant to the class's own box and
then **multiplies only over the dimensions the box actually carries**
(`mobilitydb/src/temporal/temporal_selfuncs.c:669-697`):

```c
selec = 1.0;
if (MEOS_FLAGS_GET_X(box.flags))   selec *= geo_sel(&vardata, &box, oper);
if (MEOS_FLAGS_GET_T(box.flags))   selec *= temporal_sel_tstzspan(&vardata, &period, oper);
```

A `tstzspan` constant yields a box with `T` and no `X`, so the spatial factor is
skipped and the estimate reduces to exactly what `temporal_sel` would compute.
`tnumber_sel` behaves the same way through `tnumber_const_to_span_tstzspan`,
which returns a value span and/or a time span and passes whichever exist. This is
the selectivity counterpart of the rule that a bounding-box operator compares only
the dimensions present in both operands.

`TPCBox` adds no fourth estimator pair: its leading fields are byte-identical to
`STBox` under a `static_assert` (`meos/include/meos_pointcloud.h`), so the
pointcloud types reuse the spatial pair through the cast. What `TPCBox` adds over
`STBox` is `pcid`, a schema identifier and not a dimension.

### 10.2 The type-pair gate derives from the catalog class predicates

Before any statistics are consulted, `temporal_oper_sel_family`
(`mobilitydb/src/temporal/temporal_selfuncs.c`, called once for RESTRICT and once
for JOIN) asks a per-family predicate whether the operator's two argument types
belong to the family. A miss returns `DEFAULT_TEMP_SEL`
(`mobilitydb/pg_include/pg_temporal/temporal_selfuncs.h:54`), a flat `0.0001`
independent of the query and of the column, so the gate decides whether the
declared estimator is reached at all.

Each gate is one membership predicate applied to both operands, and the
membership is **the catalog predicate of §10.1**, never a second list:

| gate | admits |
|---|---|
| `temporal_sel_type` | `time_sel_type` ∪ `talpha_type` |
| `tnumber_sel_type` | `time_sel_type` ∪ `tnumber_basetype` ∪ `numset_type` ∪ `numspan_type` ∪ `T_TBOX` ∪ `tnumber_type` |
| `tspatial_sel_type` | `time_sel_type` ∪ `spatial_basetype` ∪ `pointcloud_basetype` ∪ `T_STBOX` ∪ `T_TPCBOX` ∪ `tspatial_type` ∪ `tpointcloud_temptype` |

where `time_sel_type` is `timespan_basetype` ∪ `timeset_type` ∪ `timespan_type` ∪
`timespanset_type`, the time dimension every temporal bounding box carries.

⛔ **A family added to `tspatial_type` (or to any class predicate of §10.1) is
admitted here by construction. Re-enumerating the members in this file is the
defect this shape exists to prevent** — a hand list silently stops at the members
that existed when it was written, and the symptom is not an error but a plan built
on a constant. The statistics are unaffected and give no warning: every
`tspatial_type` family declares the same `analyze = tspatial_analyze`, so a
`tgeometry` column carries `pg_statistic` slots identical to a `tgeompoint` one
(kinds 102/103/10/11, same-length ND histograms) whether or not the gate reads
them. One box, one analyze function and one estimator pair serve all ten spatial
families; the gate must follow the same catalog.

### 10.3 Where each declaration gets its estimator

Every one of the 2219 `CREATE OPERATOR` declarations of `mobilitydb/sql/` names the
estimator pair of its own temporal operand's class, `=` and `<>` excepted — those
keep PostgreSQL's `eqsel`/`neqsel`, as `temporal_sel` and `temporal_joinsel` state
in their own headers. The declaration is generator-derived wherever the surface is
generated:

| surface | how the pair is chosen |
|---|---|
| `topops` / `posops` (`subtypes:` families) | the `{SEL}` token of `templates/{topops,posops}.sql.tmpl`, resolved by `subtype_selectivity()` |
| `compops` (`subtypes:` families) | `_compops_spec_from_subtype()`, which also picks the Eq support function |
| `compops` (`compops_families:` entries) | each pair's `rest`/`join` keys |
| B-tree comparisons | the `lit` blocks of `comparison_families:` / `hash_families:` |
| the `coverage_exceptions.txt` files | written out, since no template governs them yet |

`subtype_selectivity()` reads the catalog class predicates, so a family added to
`tspatial_type` renders `tspatial_sel`/`tspatial_joinsel` with no manifest key to
keep in step. The Eq support function follows the same class: `tspatial_supportfn`
for a spatial family, `tnumber_supportfn` for a number one, and none for an alpha
one — there is no `temporal_supportfn` to name, and the alpha reference `ttext`
declares none.

The `analyze` clause a type declares follows the same class, in its
`io_families:` entry: every family whose bounding box is an `STBox` — the ten
`tspatial_type` members and the two pointcloud ones — names `tspatial_analyze`,
and the alpha families name `temporal_analyze`. `temporal_bbox_size()`
(`meos/src/temporal/temporal_boxops.c`) is the authority on which box a family
stores, returning `sizeof(STBox)` for every `tspatial_type`; a family that stores
an `STBox` and analyses it as a `tstzspan` collects no X statistics at all, and
its spatial factor then rests on the default however its operators are declared.

One divergence remains open:

- **`pcpoint` and `pcpatch` are not admitted by `tspatial_sel_type`.** They are
  `pointcloud_basetype`, not `spatial_basetype`, and `spatial_set_stbox()` — whose
  assertion defines the base types the estimator can convert — does not cover
  them: bounding a bare `pcpoint` needs its `PCSCHEMA`, which is reached through
  the `pcid`. The ever-comparison operators pairing them with `tpcpoint`/`tpcpatch`
  therefore reach `DEFAULT_TEMP_SEL` even though they name the right family.

### 10.4 The token that carries it

`topops.sql.tmpl` and `posops.sql.tmpl` spell the estimator once, as `{SEL}_sel`
and `{SEL}_joinsel`, so a family's whole topological and position surface inherits
one decision instead of repeating it on each of its 81 operator declarations. The
token resolves through `subtype_selectivity()` to the catalog class the family
belongs to, which is why the `tstzspan`-operand and `stbox`-operand variants of the
same predicate now agree: both name the class of the temporal operand, per §10.1,
where the template previously spelled `temporal_sel` on the first and
`tspatial_sel` on the second.

### 10.5 The support function the same token carries

The `SUPPORT` clause of a predicate follows the same class as its estimator, and
`topops.sql.tmpl` and `posops.sql.tmpl` spell it once as `{SEL}_supportfn`
beside `{SEL}_sel`. The three entry points are named for their class —
`temporal_supportfn`, `tnumber_supportfn`, `tspatial_supportfn` — over one
shared `temporal_supportfn(fcinfo, tempfamily)`, the shape `temporal_selfuncs.c`
uses for its estimators.

What the support function buys is the index: it rewrites a predicate into the
bounding-box operator an opclass answers, so the portable spellings of those
operators — `overlaps` for `&&`, `contains` for `@>`, `before` for `<<#`, the
table in `doc/portable_sql.xml` — reach the same index as the operator, rather
than being kept as a filter over a sequential scan.

Three facts bound which declarations carry the clause:

- **A predicate is rewritten under the commuted spelling when its indexed
  operand is on the right**, the support function putting that operand on the
  left. The commuting pairs are the ones the operators declare as `COMMUTATOR`:
  `contains`/`contained`, `before`/`after`, `left`/`right`, `below`/`above`,
  `front`/`back`, with `overlaps`, `same` and `adjacent` commuting to
  themselves. ⛔ The eight `over` predicates declare **no** commutator —
  `s &< v` bounds one side and is no `v` OP `s` — so they keep the predicate as
  a filter in that operand order instead of rewriting it into a different
  question.
- **A family whose opclass does not index a strategy keeps the predicate as a
  filter**, the operator lookup finding no member. The Z-axis strategies reach
  the opclass of a three-dimensional family through the `front_back` key of §6,
  so `tpose` and `trgeometry` answer their Z predicates from the index.
- **An operator the bounding box cannot bound stays a filter by design.** A
  `tjsonb` value answers JSONB containment `@>` and `<@` after jsonb itself,
  from its values, while its bounding box holds the time frame alone and
  answers neither, so no operator class lists that pair. It is the only such
  pair: `tools/scripts/check_opclass_members.py` reports that every other
  bounding-box operator a type declares is a member of the classes of that
  type. A span set answers its adjacency to a value, to a span and to a span
  set alike from its bounding span, which is the key its classes store, so all
  three are members, as they are for a span whose key is the value itself.
- **The temporal pointcloud types carry no clause.** Their bounding box is a
  `TPCBox` and no scalar conversion to it exists to build an index expression
  from, so a clause there could never fire.

### 10.6 The value domain is its own bounding box

A set, span, span set or bounding-box type needs no conversion to be indexed: it
IS the value the opclass stores. Its portable predicates therefore carry a
fourth entry point, `span_supportfn`, over the same shared
`temporal_supportfn(fcinfo, tempfamily)` under the `SPANTYPE` family, and the
box-building step is skipped — the operand is passed through as-is.

`SpanStrategies[]` carries **both** axes, because one array serves types whose
single dimension is spelled differently: a time span answers the
`before`/`after` family and a number span the `left`/`right` one. The axis a
given span does not have simply finds no operator for its strategy and keeps the
predicate as a filter, so no per-type table is needed.

The gate is `span_sel_type() || bbox_type()`. ⛔ The `bbox_type` half is not
redundant: `tbox` and `stbox` have GiST opclasses of their own, so a predicate
written over a bare box is indexable in exactly the same as-is way. Note the
consequence of the operand being passed through unchanged — for a `tstzspan`
the `bbox_type` and `span_sel_type` halves would both accept it, which is why
`bbox_type` is tested second.

The generator attaches the clause through `_with_span_support()`, wired into
`_spanfile_fns`, `_spanfile_group`, `_topop_fns` and `_posop_fns`. ⛔ It is
deliberately **not** wired into `comparisons.sql.tmpl`: that template is shared
by ten other surfaces, and the equality predicates it renders are not the
portable spellings of a bounding-box operator.

⛔ `span_supportfn(internal)` must be declared **before** the first file that
uses it in bundle order — the SQL files concatenate in sorted filename order, so
the declaration lives at the head of `002_set_ops.in.sql`, above the generated
region rather than inside it. A `CREATE EXTENSION` is the only thing that
catches a misplacement; the build never parses the bundle.


---

### Legend
✓ **GEN** = a template plus a `reference: true` manifest axis governs the surface, so
`--validate` re-renders it byte-for-byte · ◐ **PARTIAL** = the axis exists but covers only
some families; the row names the ones that are not wired · ✗ HAND = hand-maintained
`.in.sql` (no template / reserved position only). All MobilityDB catalog line numbers
are live at MobilityDB `c85c0e1d6`; MEOS-API line numbers (§8) are live at MEOS-API
`65ced3016`.
