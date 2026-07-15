# Temporal&lt;T&gt; / TSpatial&lt;T&gt; inherited-surface map

> **Purpose.** A single map of the SQL/operator surface that every concrete
> temporal subtype *inherits* from the abstract classes `Temporal<T>` /
> `TNumber<T>` / `TSpatial<T>` / `TGeo<T>`, cross-referenced against **what the
> canonical generator already emits** vs. **what is still hand-maintained**.
>
> The ordering authority is the **doc XML `<sect1>` structure** — the reference
> manual chapters are the contract for *which* behaviours are inherited and in
> *what order*. This document is a working draft to revise together; every claim
> below cites live source (master `13ad7b9d3`).

---

## 1. The OO hierarchy (live `meos/src/temporal/meos_catalog.c`)

The class of a temporal type is decided by the catalog membership predicates —
these are the single source of truth, not naming heuristics.

```
Temporal<T>              temporal_type      = ALL temporal types            (catalog:1139)
  ├── TAlpha<T>          talpha_type        = tbool, ttext, tjsonb, tdouble2/3/4  (catalog:1213)
  │     ├── TBool  ├── TText  └── TJsonb   (tdoubleN = internal)
  ├── TNumber<T>         tnumber_type       = tint, tbigint, tfloat          (catalog:1234)
  │     ├── TInt   ├── TBigint  └── TFloat
  └── TSpatial<T>        tspatial_type      = tgeompoint tgeogpoint tnpoint tpose
        │                                     tcbuffer tgeometry tgeography
        │                                     trgeometry th3index tquadbin (10) (catalog:1294)
        ├── TGeo<T>      tgeo_type          = tgeometry, tgeography          (catalog:1343)
        │   (all)        tgeo_type_all      = + tgeompoint + tgeogpoint (4)   (catalog:1368)
        │     ├── TGeometry  ├── TGeography
        │     └── TPoint<T>  tpoint_type    = tgeompoint, tgeogpoint         (catalog:1321)
        ├── Tcell<T>     tcellindex_type    = tquadbin   (th3index INTENDED,  (tcellindex.c:63)
        │     │                               not yet wired — §5a)           via DggsCellOps
        │     ├── TQuadbin  └── (TH3Index, hand today)
        ├── TPointcloud  tpointcloud_temptype = tpcpoint, tpcpatch  (#if POINTCLOUD) (catalog:1224)
        │     ├── TPcpoint  └── TPcpatch
        └── (TSpatial, no intermediate): tcbuffer, tnpoint, tpose, trgeometry
```

- `tcbuffer`/`tnpoint`/`tpose`/`trgeometry` inherit `Temporal<T>` + `TSpatial<T>`
  but **not** the `TGeo<T>`/`TPoint<T>`-only surface.
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

- **whole-file SQL** at a fixed 50-bin position (`positions:` in `manifest.yaml`):
  emits a complete `NNN_<family>_<behaviour>.in.sql`.
- **region-in-file** for C: emits the block between
  `GENERATED-BOXOPS-BEGIN/END` and `GENERATED-SPATIALRELS-BEGIN/END` markers.

**A `positions:` entry only means a slot is reserved. A behaviour is *generated*
only if a matching `templates/<behaviour>.*.tmpl` exists.** Live templates:

| behaviour | template(s) | status |
|---|---|---|
| compops | `compops.sql.tmpl` | **GENERATED** (Ever/Always comparisons only — see §4) |
| topops | `topops.sql.tmpl` | **GENERATED** |
| posops | `posops.sql.tmpl` | **GENERATED** |
| spatialrels | `spatialrels.c.tmpl` + `spatialrels.sql.tmpl` | **GENERATED** (ever/always) |
| boxops (C) | `boxops.c.tmpl` | **GENERATED** (box-type axis) |
| gist / spgist / indexes | `gist/spgist/indexes.sql.tmpl` | **GENERATED** (index infra) |
| spatialfuncs | — | reserved position, **HAND** |
| distance | — | reserved position, **HAND** |
| aggfuncs | — | reserved position, **HAND** |
| tempspatialrels | — | reserved position, **HAND** |

**Box-type axis (`boxtypes:`)** — the C bounding-box dispatchers are per *box type*,
not per family: `stbox` (tspatial), `tbox` (tnumber, composite value×time),
`tstzspan` (temporal-only), `tpcbox` (pointcloud). One `stbox` impl serves every
`TSpatial<T>` family.

**Spatialrel families (`spatialrel_families:`)** — the ever/always spatial-rel C
kernel wiring, currently: `geo_ea_contains_covers`, `geo_ea_disjoint_intersects`,
`geo_ea_dwithin` (the **geo** family).

## 4. `Temporal<T>` chapter — section-by-section

| `<sect1>` | MEOS prefix | generated? | canonical generator / notes |
|---|---|---|---|
| Input and Output | `temporal_` | ✗ HAND | per-type recv/send/in/out |
| Constructors | `temporal_` | ✗ HAND | tXxxInst/Seq/SeqSet |
| Conversions | `temporal_` | ✗ HAND | casts |
| Accessors | `temporal_` | ✗ HAND | getTime, getValues, startTimestamp, duration, memSize … |
| Transformations | `temporal_` | ✗ HAND | shiftTime/scaleTime, setInterp, tprecision, tsample |
| Modifications | `temporal_` | ✗ HAND | appendInstant, insert, update, merge |
| Restrictions | `temporal_` | ✗ HAND | atValue(s)/minusValue(s), atTime/minusTime, atSpan(set), atTbox |
| **Bounding Box Operators** | `temporal_`/`tnumber_` | ✓ **GEN** | `topops.sql.tmpl` (`&&`,`@>`,`<@`,`~=`,`-\|-`) + `posops.sql.tmpl` (`<<`,`>>`,`&<`,`&>`,`<<#`,`#>>`…) + `boxops.c.tmpl` box types `tstzspan`,`tbox` |
| Comparisons → Traditional | (btree) | ✗ HAND | `=`,`<>`,`<`,`>`,`<=`,`>=` |
| Comparisons → **Ever/Always** | `temporal_` | ✓ **GEN** | `compops.sql.tmpl`: `eEq`/`aEq`/`eNe`/`aNe` + `?=`/`%=`/`?<>`/`%<>` (all 3 arg directions) |
| Comparisons → Temporal | `temporal_` | ✗ HAND | `tEq`/`tNe` → `#=`/`#<>` (no template) |
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
| `026_tnumber_mathfuncs` (`+ - * /`, abs, delta, trend, derivative) | TNumber | ✗ HAND | no math template |
| `036_tnumber_distance` (tDistance, nad) | TNumber | ✗ HAND | no distance template |
| number Restrictions (atSpan/atSpanset/atTbox) | TNumber | ✗ HAND | numeric-span / value×time box restrict |
| number Aggregates (extent, tSum, tAvg, tMin/tMax) | TNumber | ✗ HAND | `040_temporal_aggfuncs` |
| `028_tbool_boolops` (`&` `\|` `~`, tAnd/tOr/tNot) | TAlpha (tbool) | ✗ HAND | tbool-specific |
| `029_ttext_textfuncs` (`\|\|`, upper/lower) | TAlpha (ttext) | ✗ HAND | ttext-specific |

⚠️ **`tbigint` and `tjsonb` are full members** of `tnumber_type()` / `talpha_type()`
(catalog:1234/1213) but are **absent from the MEOS-API lattice** (§8) — a curation gap.

**The generic base `Temporal<T>` reference files** (`030_temporal_compops`,
`032_temporal_boxops`, `034_temporal_posops`, `040/042` aggfuncs, `043/044`
gist/spgist, `022/023` type/inout, `025_temporal_tile`, `038/046`
similarity/analytics) are likewise the hand reference; the generator re-emits their
*shape* onto the derived families (§6) and regenerates the **C boxops region** for box
type `tstzspan` inside `temporal_boxops.c`.

**Base value-domain types** (`Set` / `Span` / `SpanSet` / `TBox` / `STBox`) are the
finite-subset representations of the value/time domains that the restriction surface
(atValues=Set, atSpan=Span, atTbox=TBox…) consumes. Their operator files
(`001–015`, `021_tbox`) are hand today; memory
`generate-boxops-campaign-boxtype-axis` flags the repeated per-span-type
`005_span_ops`/`009_spanset_ops` as a future generation target.

### 4b. Aggregation / Indexing / Analytics chapters (also `Temporal<T>`-inherited)

Two more reference chapters carry inherited surface:

| chapter → `<sect1>` | prefix | generated? | notes |
|---|---|---|---|
| `temporal_types_aggregation.xml` → Aggregation | `temporal_`/`tnumber_` | ✗ HAND | tCount/extent/tMin/tMax/tSum/tAvg/merge/appendInstant (`temporal_aggfuncs.c`) |
| → Indexing | (index) | ✓ **GEN** | GiST/SP-GiST via `gist/spgist/indexes.sql.tmpl` |
| → Statistics and Selectivity | (selectivity) | ✗ HAND | |
| `temporal_types_analytics.xml` → Simplification / Reduction / Similarity / Extended Kalman Filter / Splitting / Multidimensional Tiling | `temporal_`/`tgeo_` | ✗ HAND | analytics; tiling `stboxes`/`splitNStboxes` are per-family table shapes |

## 5. `TSpatial<T>` / `TGeo<T>` chapter — section-by-section

| `<sect1>` | MEOS prefix | generated? | canonical generator / notes |
|---|---|---|---|
| Input and Output | `tspatial_` | ✗ HAND | asText/asEWKT/asMFJSON + FromXxx constructors |
| Conversions | `tspatial_`/`tgeo_` | ✗ HAND | |
| Accessors | `tspatial_`/`tgeo_`/`tpoint_` | ✗ HAND | SRID, trajectory, traversedArea, convexHull … |
| Transformations | `tspatial_`/`tgeo_` | ✗ HAND | setSRID, transform |
| Restrictions | `tspatial_`/`tgeo_` | ✗ HAND | atGeometry/atStbox/minus… |
| Spatial Reference System | `tspatial_` (`spatialfuncs`) | ✗ HAND | reserved position, no template |
| **Bounding Box Operations** | `tspatial_` | ✓ **GEN** | `topops`+`posops`+`boxops.c.tmpl` box type `stbox` |
| Distance Operations | `tspatial_`/`tgeo_` (`distance`) | ✗ HAND | tDistance/nad/nai/shortestLine — reserved position, no template |
| Spatial Rel. → **Ever/Always** | `tspatial_`/`tgeo_` | ✓ **GEN** | `spatialrels.{c,sql}.tmpl` — geo via `spatialrel_families`; cbuffer/npoint/h3/quadbin via subtype wiring |
| Spatial Rel. → Spatiotemporal | `tspatial_` (`tempspatialrels`) | ✗ HAND | tIntersects/tDwithin/tContains/tTouches — reserved position, no template |

Index infra (`gist`/`spgist`/`indexes`) is generated but is not a doc `<sect1>`.

### 5a. `Tcell<T>` (DGGS cell-index) — the descriptor-factored intermediate

`Tcell<T>` (prefix `tcellindex_`, `meos/src/temporal/tcellindex.c`) sits between
`TSpatial<T>` and the discrete cell types. It is a **first-party abstraction**: each
DGGS supplies **one `DggsCellOps` descriptor** (a table of Datum-convention static-cell
kernels + catalog identity), and the generic `tcellindex_*` entry points lift that
kernel via `tfunc_temporal`. Adding a DGGS (e.g. Google S2) = a descriptor + kernel,
**no new temporal scaffolding, SQL, or binding code** (`tcellindex.h:38-64`).

The generic inherited Tcell API (`tcellindex.h:139-145`):
`tcellindex_get_resolution` · `is_valid_cell` · `cell_to_parent` · `cell_to_point` ·
`cell_to_boundary` · `cell_area`.

| aspect | state |
|---|---|
| C implementation | **unified once** via `DggsCellOps` — the `Tcell` C surface is effectively "generated" (single generic body, per-DGGS descriptor) |
| catalog predicate `tcellindex_type()` | **quadbin only** (`#if QUADBIN → T_TQUADBIN`, `tcellindex.c:63`). **th3index is NOT wired** — it uses its own libh3 surface |
| descriptor registered | `quadbin_cellops` (`meos/src/quadbin/tquadbin_ops.c:132`) — **no `h3_cellops`** |
| SQL wrappers (cellResolution/isValidCell/cellToParent/cellToPoint/cellToBoundary/cellArea) | **per-family HAND** in the `spatialfuncs` slot: h3 `255_th3index_spatialfuncs`, quadbin `355_tquadbin_spatialfuncs`; names are family-prefixed (`th3CellToBoundary` / `tquadbin…`) |
| cell→boundary hook | the key inherited hook: `spatialrels.sql.tmpl` cast-delegates via `<fam>CellToBoundary($n)::tgeometry` (`manifest.yaml` `boundary_fn`) — this IS generated (§6, h3 262 / quadbin 362) |

⇒ **Gap**: `th3index` should be migrated onto the `DggsCellOps` descriptor +
`tcellindex_type()` (add `#if H3 → T_TH3INDEX` and an `h3_cellops`) so both cell
families share one C implementation, and the per-family SQL cell wrappers could then be
generated from a `tcellindex` template instead of hand-written twice.

## 6. Per-family gap — every inherited `.in.sql` file, generated vs hand

Each cell = the live file number (`mobilitydb/sql/<fam>/`). **Bold** = the
file is emitted by the generator today (in that subtype's `manifest.yaml` `files:`);
plain = the file exists but is still hand-maintained.

| family | compops | spatialfuncs | topops | posops | distance | aggfuncs | spatialrels | tempsp.rels | idx / gist·spgist | boxops |
|---|---|---|---|---|---|---|---|---|---|---|
| cbuffer (200) | **204** | 205 | **208** | **209** | 210 | 211 | 212 | 214 | **216** | — |
| npoint (300) | **304** | 306 | **308** | **309** | 312 | 314 | — | — | **316** | — |
| pose (100) | 104 | 105 | 108 | 109 | 110 | 111 | 112 | 114 | **116** | — |
| rgeo (150) | 152 | 153 | 156 | 157 | 161 | 159 | 154 | 155 | **162** | 166 |
| h3 (250) | **254** | 255 | **258** | **259** | — | — | **262** | — | **272**·**273** | — |
| quadbin (350) | **354** | 355 | **358** | **359** | — | — | **362** | — | **372**·**373** | — |

Reading the table:
- **`tpose` and `trgeometry` generate ONLY their index file** — every other inherited
  file (compops/spatialfuncs/topops/posops/distance/aggfuncs/spatialrels/
  tempspatialrels; rgeo also boxops166) is hand-maintained → the prime migration
  target (templates already exist for compops/topops/posops/spatialrels).
- **`spatialfuncs`, `distance`, `aggfuncs`, `tempspatialrels` are generated for NO
  family** — no template exists; hand everywhere.
- **`spatialrels` SQL** is generated only for the **cast-delegated cell families**
  (h3 262, quadbin 362) via `spatialrels.sql.tmpl` (boundary→`tgeometry` cast).
  cbuffer 212 / pose 112 / rgeo 154 spatialrels are hand; npoint has none.
- The C ever/always spatial-rel **kernel** (`spatialrel_families` axis) is generated
  only in the **geo** file `tgeo_spatialrels.c` (contains/covers/disjoint/intersects/
  dwithin); cbuffer/rgeo/pose native C spatial-rel wrappers are still hand
  (memory `spatialrel-wrapper-surface-is-inherited-generate-it`: "NEXT = roll to
  cbuffer/rgeo/pose").
- The **geo/tpoint/tgeo** family SQL surfaces are not in the `subtypes:` list at all
  (geo is the hand-written reference layout the generator derives from).

## 7. The gap (roadmap, most-mechanical first)

**A. Widen coverage of already-generated behaviours** (templates exist, just wire
more subtypes in `manifest.yaml`):
- Add compops/topops/posops to `tpose`, `trgeometry` (today: indexes-only).
- Add spatialrels wiring for cbuffer/npoint/pose/rgeo (today: geo + h3/quadbin).

**B. New templates for reserved-position behaviours** (position slot exists, no
template yet — pure hand today):
- `tempspatialrels` (tIntersects/tDwithin/…) — the ever/always sibling already
  generates; the temporal variant is the natural next template.
- `distance` (tDistance/nad/nai/shortestLine).
- `spatialfuncs` (SRID / transform / trajectory scaffolding).
- `aggfuncs` (tCount/extent/tMin/tMax/tSum/tAvg + the union aggregates — note the
  new `@csqlaggfn` catalog identity, MobilityDB #1411 + MEOS-API #55, both merged).
- Comparisons → **Temporal** (`tEq`/`tNe` → `#=`/`#<>`) alongside the existing
  ever/always compops template.

**C. Sections that are inherently per-family / value-shaped** (generation needs the
per-type base-value marshalling, tracked binding-side, e.g. MobilityDuck):
Input/Output, Constructors, Conversions, Accessors, Transformations,
Modifications, Restrictions. These are the largest hand surface and the subject of
the binding generators (see memory
`mobilityduck-tcbuffer-full-implementation-roadmap`).

## 8. Comparison with the MEOS-API generated hierarchy

The MEOS-API catalog derives the ecosystem class hierarchy from a **curated** object
model, `meta/object-model.json` `lattice` (MEOS-API master `65ced3016`). It declares
**18 classes**: Temporal · TAlpha{TBool,TText} · TNumber{TInt,TFloat} ·
TSpatial{TGeo{TPoint{TGeomPoint,TGeogPoint}, TGeometry, TGeography}, TCbuffer, TNpoint,
TPose, TRGeometry}. Diffed against the live MEOS catalog predicates
(`meos_catalog.c` @ MobilityDB `b6624f21a`), these live types/classes are **missing**:

| missing from lattice | live type / predicate | belongs under | category |
|---|---|---|---|
| **TBigint** | `tbigint` (`tnumber_type` :1234) | TNumber | **in-scope leaf, omitted (defect)** — number family IS in `scope.inScopeTypeFamilies` |
| **TJsonb** | `tjsonb` (`talpha_type` :1213) | TAlpha | in-scope family (alpha), omitted leaf |
| **TH3Index** | `th3index` (`tspatial_type` :1298) | TSpatial → Tcell | deferred family (not in declared scope) |
| **TQuadbin** | `tquadbin` (`tspatial_type` :1299) | TSpatial → Tcell | deferred family |
| **TPcpoint** | `tpcpoint` (`tpointcloud_temptype` :1224) | TSpatial → TPointcloud | deferred family (`#if POINTCLOUD`) |
| **TPcpatch** | `tpcpatch` (`tpointcloud_temptype` :1224) | TSpatial → TPointcloud | deferred family |
| **Tcell / TCellIndex** (abstract) | `tcellindex_type()` | between TSpatial and cell leaves | missing intermediate |
| **TPointcloud** (abstract) | `tpointcloud_temptype()` | between TSpatial and pointcloud leaves | missing intermediate |

Notes:
- The lattice's `scope.inScopeTypeFamilies` = `[temporal, alpha, number, geo, point,
  cbuffer, npoint, pose, rgeo]` — it does **not** list h3/quadbin/pointcloud, so those
  are *declared* deferrals. But **TBigint / TJsonb** belong to in-scope families
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

---

### Legend
✓ **GEN** = emitted by `tools/codegen/inherited/` today · ✗ HAND = hand-maintained
`.in.sql` (no template / reserved position only). All catalog line numbers are live at
MobilityDB `b6624f21a` / MEOS-API `65ced3016`.
