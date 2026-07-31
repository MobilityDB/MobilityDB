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
> revise together; every claim below cites live source (master `13ad7b9d3`).

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
| Input and Output | `temporal_` | ✗ HAND | **two sub-families**: (a) **type I/O** `<type>_in`/`_out`/`_recv`/`_send` (PG registration, one set per type); (b) **canonical representations** — text `asText`/`asEWKT`/`fromText`, binary `asBinary`/`asEWKB`, hex `asHexWKB`/`fromHexWKB`, **MF-JSON** `asMFJSON`/`fromMFJSON` (MEOS-C `temporal_as_{wkb,hexwkb,mfjson}` + `temporal_from_{wkb,hexwkb,mfjson}`). Both token-shaped → generable |
| Constructors | `temporal_` | ✗ HAND | tXxxInst/Seq/SeqSet |
| Conversions | `temporal_` | ✗ HAND | casts |
| Accessors | `temporal_` | ✓ **GEN** | `accessors.sql.tmpl` multi-base renderer from base `022_temporal.in.sql` — the value/time/generic set for ALL families (§4c); per-family value shape = manifest `types:` tokens. A few interleaved/positional accessors stay hand per family |
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
`005_span_ops`/`009_spanset_ops` as a future generation target. **§9 maps this
value-domain surface class-by-class** (membership, the Set-vs-Span asymmetry,
doc sections, file map, and the `setfamilies:` manifest axis).

### 4b. Aggregation / Indexing / Analytics chapters (also `Temporal<T>`-inherited)

Two more reference chapters carry inherited surface:

| chapter → `<sect1>` | prefix | generated? | notes |
|---|---|---|---|
| `temporal_types_aggregation.xml` → Aggregation | `temporal_`/`tnumber_` | ✗ HAND | tCount/extent/tMin/tMax/tSum/tAvg/merge/appendInstant (`temporal_aggfuncs.c`) |
| → Indexing | (index) | ✓ **GEN** | GiST/SP-GiST via `gist/spgist/indexes.sql.tmpl` |
| → Statistics and Selectivity | (selectivity) | ✗ HAND | |
| `temporal_types_analytics.xml` → Simplification / Reduction / Similarity / Extended Kalman Filter / Splitting / Multidimensional Tiling | `temporal_`/`tgeo_` | ✗ HAND | analytics; tiling `stboxes`/`splitNStboxes` are per-family table shapes |

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
| `numeric` / `orderable` | gate the TNumber/orderable-only extras: `valueSet`, `minValue`, `maxValue`, `avgValue`, `minInstant`, `maxInstant` | tint/tbigint/tfloat numeric; ttext orderable |
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

### 4e. Input/Output generation scope (the next `✗ HAND → ✓ GEN` target)

The Input/Output `<sect1>` is **two token-shaped sub-families**; both mirror the
`accessor_families` model (per-type `types:` rows, region-marked blocks in a reference
file, `--validate` byte-for-byte). Live at MobilityDB master ~`b496986dd`.

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

Status: the base shape is **generated** (`io_type.sql.tmpl` + `io_families` `temporal`
reference, `--validate` byte-for-byte). The spatial shape needs a template variant
(`@IF spatial`: shell type + `T<fam>_typmod_in` + `tspatial_typmod_out`/`_analyze` +
swapped send/receive) — the next increment.

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
`maxdecimaldigits int4 DEFAULT 15` on float/coordinate-bearing types only;
`endianenconding text DEFAULT ''` (canonical misspelling) on `asBinary`/`asHexWKB`.

**Plan**: new `io_families` manifest axis reusing the `accessor_families` per-type rows
(add `spatial` + `in_sym`/`out_sym`/… overrides). Templates `io_type.sql.tmpl` (A) and
`io_repr.sql.tmpl` (B, base + `@IF spatial` E-extensions). Reference families = base
`temporal` (base shape) + `geo` (spatial shape); rgeo = the specialize-all outlier.
Start with **A/type-I/O** (purest token) then **B/representations**, cbuffer first.

### 4f. Type-I/O canonicalization — `Temporal<>` vs `TSpatial<>` irregularities

Canonical references (both regular): base = `022` (tbool…ttext); spatial = `052_tgeo`
(tgeometry/tgeography/tgeompoint/tgeogpoint) — shell `CREATE TYPE <t>;`, `<t>_in`/
`temporal_out`/`<t>_recv`/`temporal_send`, `<t>_typmod_in`, shared `tspatial_typmod_out`
+ `tspatial_analyze`, `send/receive` order. Every *derived* spatial family deviates; the
plan is to regularize each to the spatial canon, after which the spatial I/O template is
one clean variant with per-family symbol tokens only.

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
per-type base-value marshalling): Input/Output, Constructors, Conversions,
Transformations, Modifications, Restrictions. **Accessors are now generated** for
every family (§4c, `accessors.sql.tmpl` + manifest `types:` tokens) — the value-shaped
rows via `base`/`baseset`/`valret`, the rest generically; only interleaved/positional
accessors stay hand. The MEOS-**C** value surface (`start_value`/`value_at_timestamptz`/
`values`/`at_value`… in `meos/src/<fam>.c`) is the sibling generation target —
`temporal_basetype.c.tmpl`, byte-for-byte reference `meos/src/json/tjsonb.c`, cloneable
for the value-opaque families (jsonb/pcpoint/pcpatch/cbuffer). The remaining hand
sections are the subject of the binding generators (see memory
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

## 9. Value-domain classes — `Set<T>` / `Span<T>` / `SpanSet<T>`

The finite-subset value-domain types that the temporal restriction/accessor
surface consumes (§4a). Ordering authority: **`doc/set_span_types.xml`**. All
catalog/doc line numbers in this section are live at master `5bca73bc8`.

### 9.1 Class membership (live `meos/src/temporal/meos_catalog.c`)

| class | members | catalog |
|---|---|---|
| `Set<T>` (**16**) | intset, bigintset, floatset, textset, dateset, tstzset, geomset, geogset, npointset, poseset, cbufferset, jsonbset, h3indexset, quadbinset, pcpointset, pcpatchset | `MEOS_SETTYPE_CATALOG` :262-280 · `set_type()` :801-808 · `set_basetype()` :787-794 |
| `Span<T>` (**5**) | intspan, bigintspan, floatspan, datespan, tstzspan | `MEOS_SPANTYPE_CATALOG` :287-295 · `span_type()` :982-987 |
| `SpanSet<T>` (**5**) | intspanset, bigintspanset, floatspanset, datespanset, tstzspanset | `MEOS_SPANSETTYPE_CATALOG` :301-309 · `spanset_type()` :1080-1085 |

Sub-predicates: `spatialset_type()` :909-914 = geomset, geogset, npointset,
poseset, cbufferset, h3indexset, quadbinset (**7** — the sets that carry a
bounding box). `pointcloudset_type()` :950-954 = pcpointset, pcpatchset — NOT
`spatialset_type()`: pointcloud sets carry no bounding box (the note at
:944-948; the TPCBox structure is what carries pointcloud spatial bounds).

### 9.2 WHY the 16-vs-5 asymmetry

- **Every base type has a set type** because a temporal value is a *function*
  from time to the base domain and `getValues` returns its RANGE as a set
  (§4c row 6) — a family cannot have a temporal type without its base set type.
- **`Span<T>` needs a total order AND a meaningful contiguous interval** on the
  base domain, so it exists only for `span_basetype()` :963-967 = date, float,
  int, bigint, timestamptz — numbers + time. `span_canon_basetype()` :973-976 =
  date, int, bigint marks the **discrete** bases whose spans canonicalize with
  +1 (upper bound normalized to exclusive).
- **Order alone is NOT enough**: text is ordered — textset deploys the full
  `<< >> &< &>` position surface in `mobilitydb/sql/temporal/002_set_ops.in.sql`
  — but there is no textspan (`span_basetype()` excludes `T_TEXT`): a
  contiguous interval of texts is not meaningful.

### 9.3 `doc/set_span_types.xml` — section-by-section

| `<sect1>` (doc line) | generated? | notes |
|---|---|---|
| Input and Output (:102) | ✗ HAND | in/out/recv/send + asText/asBinary/FromBinary/FromHexWKB |
| Constructors (:268) | ✗ HAND | `set()`/`span()`/`spanset()` |
| Conversions (:325) | ✗ HAND | base↔set/span, span↔spanset, range/multirange |
| Accessors (:442) | ✗ HAND | memSize/lower/upper/width/duration/numValues/… |
| Transformations (:693) | ✗ HAND | shift/scale, floor/ceil/round, spans/splitN |
| Spatial Reference System (:901, `spatialset_spatial_srid`) | ✗ HAND | spatial sets only (SRID/setSRID/transform) |
| Set Operations (:958, `setspan_set_ops`) | ✗ HAND | union/intersection/minus, ∈/⊆ |
| Bounding Box Operations (:1012, `setspan_topo_pos`) | ✗ HAND | sect2 Topological (:1014) · Position (:1082) · Splitting (:1162) |
| Distance Operations (:1219, `setspan_distance`) | ✗ HAND | `<->` / setDistance — metric bases only |
| Comparisons (:1248, `setspan_comparisons`) | ✗ HAND | btree `< <= > >=` + `=`/`<>`, cmp, hash — ALL 16 sets have them |
| Aggregations (:1306, `setspan_agg`) | ✗ HAND | setUnion/spanUnion/extent/… |
| Indexing (:1389, `setspan_indexing`) | ✗ HAND | GiST/SP-GiST opclasses |

Today **every row is HAND** — the value-domain layer has no template in
`tools/codegen/inherited/templates/` and emits nothing from the generator.

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

### 9.5 Per-family file map (`mobilitydb/sql/`)

The template-class reference layer lives in `temporal/`:

| class | type | ops | indexes | aggfuncs |
|---|---|---|---|---|
| `Set<T>` | `001_set` | `002_set_ops` | `013_set_indexes` | — |
| `Span<T>` | `003_span` | `005_span_ops` | `011_span_indexes` | `015_span_aggfuncs` |
| `SpanSet<T>` | `007_spanset` | `009_spanset_ops` | `012_spanset_indexes` | — |

plus the 8 per-family set files that instantiate `Set<T>` for the plug-in base
types: `geo/050_geoset` · `pose/101_poseset` · `cbuffer/201_cbufferset` ·
`h3/251_h3indexset` · `npoint/301_npointset` · `quadbin/351_quadbinset` ·
`pointcloud/400_pcset` · `json/450_jsonbset` (the jsonb-specific path/element
operators live separately in `json/451_jsonbset_jsonfuncs`).

### 9.6 What the `setfamilies:` manifest axis encodes

The `setfamilies:` axis in `manifest.yaml` carries one row per set type with the
tokens a set/span template needs. It is descriptive only — no `files:` entries,
no template, nothing emitted.

| token | meaning |
|---|---|
| `ordered` | the base has a SEMANTIC total order (int, bigint, float, text, date, timestamptz) — **this flag is what decides whether position operators are emitted at all**; the 10 unordered bases get none |
| `posops_spelling` | `value` = `<< >> &< &>` (numbers + text) · `time` = `<<# #>> &<# #&>` (date + timestamptz); omitted when not ordered |
| `metric` | `<->` / `setDistance` is deployed (all ordered bases except text, plus geomset/geogset/npointset/poseset/cbufferset) |
| `spatial` | the set is `spatialset_type()` (:909-914) — carries a bounding box and the SRS section (§9.3); pcpointset/pcpatchset are `pointcloudset_type()`, not spatial |

Known deployed irregularity the axis does not model: jsonbset has the `<<`/`>>`
pair (`json/450_jsonbset.in.sql:378-393`, `Left_set_set`/`Right_set_set`)
without `&<`/`&>`, though jsonb has no semantic order.

---

### Legend
✓ **GEN** = emitted by `tools/codegen/inherited/` today · ✗ HAND = hand-maintained
`.in.sql` (no template / reserved position only). All catalog line numbers are live at
MobilityDB `b6624f21a` / MEOS-API `65ced3016`.
