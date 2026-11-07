<!--
  MobilityDB — Portable Naming: where the types and their categories come from
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Where the types and their categories come from

The one place to read how a binding knows which types exist, which collection types each base
type has, and which category a type belongs to. MEOS states all of it once, in its catalog; the
MEOS-API catalog publishes it; every generator reads it there. A list of types kept anywhere
else is a second statement of a fact MEOS owns, and a new family falls out of it with no error.

The rules (owner, 2026-10-29): every MEOS base type has by definition an associated set type,
except `bool`, whose set has no meaning; the
C tables of `meos_catalog.h`/`meos_catalog.c` and their predicates determine every category; the
bindings are orthogonal and automatic over them; the metamodel is `Temporal` with its classes
(`TNumber`, ...), the spans and numeric spans with respect to the time spans, and the temporal box
over the numeric spans; there is a catalog function for every category.

## 1. The source: `meos_catalog.h` and `meos_catalog.c`

| What | Where | States |
|---|---|---|
| The types | `MeosType` enum, `meos/include/temporal/meos_catalog.h` | every type, in its canonical order |
| Their names | `MEOS_TYPE_NAMES`, `meos_catalog.c` | the public lowercase name of each (`intspan`, `tbigint`) |
| The relations between types | `MEOS_RELTYPE_CATALOG`, `meos_catalog.c` | for each type its `basetype_settype`, `basetype_spantype`, `spantype_spansettype`, `temptype_basetype`, `type_bboxtype` and their inverses, read by `basetype_settype()`, `settype_basetype()`, ... |
| The categories | the membership predicates `bool <name>(MeosType type)`, `meos_catalog.c` | below |

The categories, each a predicate:

| Kind | Predicates |
|---|---|
| Sets | `set_type`, `numset_type`, `timeset_type`, `alphanumset_type`, `geoset_type`, `spatialset_type`, `pointcloudset_type`, `set_spantype` |
| Spans | `span_type`, `numspan_type`, `timespan_type`; their bases `span_basetype`, `numspan_basetype`, `timespan_basetype`, `span_canon_basetype` |
| Span sets | `spanset_type`, `numspanset_type`, `timespanset_type` |
| The temporal box | `tnumber_spantype` (the spans of a `tbox`'s value, `floatspan`, `intspan`, `bigintspan`), `span_tbox_type` (those and `tstzspan`) |
| Temporal | `temporal_type`, `talpha_type`, `talphanum_type`, `tnumber_type`, `torder_type`, `tspatial_type`, `tgeo_type`, `tgeo_type_all`, `tpoint_type`, `tgeometry_type`, `tgeodetic_type`, `tspatial_body_type`, `tpointcloud_temptype`, `temptype_supports_linear` |
| Base types | `meos_basetype`, `set_basetype`, `temporal_basetype`, `tnumber_basetype`, `alphanum_basetype`, `geo_basetype`, `spatial_basetype`, `pointcloud_basetype`, `basetype_byvalue`, `basetype_varlength` |
| Mixed | `time_type`, `type_span_bbox` |

`date` is a number and a time at once: `numspan_type` and `timespan_type` both hold `datespan`,
so the categories overlap and are stated as predicates, not as a tree. The `ensure_<name>`
functions beside them raise when a type is outside a category; they state no category of their own.

## 2. The catalog: MEOS-API's `meos-idl.json`

| Section | Parser | Contents |
|---|---|---|
| `typeRelations.byBase` | `parser/typerelations.py` | each base type's `set`, `span`, `spanset` and `temporal` types, from `MEOS_RELTYPE_CATALOG` |
| `typeRelations.predicates` | `parser/typerelations.py` (A29, MEOS-API #219) | each membership predicate of the table above → its member types in `MeosType` order; a call to another predicate resolved, a family guard (`#if POINTCLOUD`) admitting its members, the `ensure_*` left out |
| `temporalTypes` | `parser/temporaltypes.py` | each temporal type's `base`, `bbox`, MF-JSON token and `number` / `spatial` / `linear` classes |
| `objectModel.lattice` | `parser/object_model.py` over `meta/object-model.json` | the `Temporal` classes (`TAlpha`, `TNumber`, `TSpatial`, `TGeo`, `TPoint` and the leaves), each node's membership read from the predicate it names |
| `objectModel.companions` | `parser/object_model.py` | the `Collection` (`Set`, `Span`, `SpanSet` and their leaves) and `Box` (`TBox`, `STBox`, `TPCBox`) hierarchies |
| `enums` (`MeosType`) | `run.py` | the enum with its values, the canonical order |

The catalog's contract with its consumers is `docs/cross-repo-handoff.md` in MEOS-API.

## 3. The consumers

**MobilityDuck** (`tools/codegen_duck_udfs.py`):

| Generator table | Read from | Row |
|---|---|---|
| `BASE_ORDER`, `SET_TYPES` (the sets registered) | `typeRelations.byBase[*].set` in `MeosType` order (`catalog_collection_bases`) | D8 |
| `SPAN_BASES`/`SPAN_TYPES`, `SPANSET_BASES`/`SPANSET_TYPES`, `typereg_members` | `typeRelations.byBase[*].span` / `.spanset` | D8 |
| the cell-set prefilter (`geoTo<Cell>Set`, `eEqual(<cell>set, t<cell>)`) | each function's declared `sqlSignatures` (`prefilter_signature`) | D8 |
| `SQL_BASE_TO_DUCK`, `SQL_CONTAINER_ACC`, the conversions `*_to_<set, span, spanset>` | `SET_TYPES`, `SPAN_TYPES`, `SPANSET_TYPES` above | D8 |
| `CORE_TYPES["tnumber"]` (`tint`, `tfloat`, `tbigint`), the numbers of the box topology of `reg_scope`, `SPATIAL_ALLTYPES` | `objectModel.lattice` (`lattice_members`): the `TNumber` class, and the `TSpatial` class whose `temporalTypes[t].bbox` is `stbox` | D8 |
| the spans of a `tbox` | no list: the box registrations come from each function's declared `sqlSignatures`, so the generator needs no copy of `typeRelations.predicates.tnumber_spantype` | |
| `HAND_SET_BASES` (`geomset`, `geogset`), `BASE_LOGICAL` (each base's DuckDB element type), `ELEM_TO_SET`/`ELEM_TO_SPAN`/`ELEM_TO_SPANSET` (C scalar type → collection) | stay in the generator: they state how the binding marshals a value, not which types exist; a base the catalog gives a set and `BASE_LOGICAL` does not name stops the generation | |

**MobilityDB** (`tools/codegen/inherited/`): `manifest.d/classes.yaml` keeps the members of each
class beside its predicate, and `generate.py --classes` re-reads each predicate body and fails when
they disagree; `INHERITANCE_MAP.md` §9.1 tabulates them. It reads `meos_catalog.c` itself, being in
the same tree.

## 4. The rows

| Row | Repository | What |
|---|---|---|
| D8 | MobilityDuck #533 (merged, `e658593d2e`) | the set, span and span set types from `typeRelations`; the cell-set prefilter of every cell family; the temporal classes from `objectModel.lattice` |
| A29 | MEOS-API #219 (merged, `cfb463611f`) | `typeRelations.predicates` |
| G12 | MobilityDB #3243, #3247 (merged) | the grid distance of the cells, whose prefilter D8 completes |
