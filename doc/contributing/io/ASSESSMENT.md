<!--
  MobilityDB — Input and output: the assessment
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Every type, every format, every engine: the assessment

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

Measured at MobilityDB master `845a60f141` (its `mobilitydb/sql` and public headers identical to
`0be1b51060`'s), over `mobilitydb/sql` with its comments removed, the public MEOS headers
(`meos*.h` but `meos_internal*.h`), and the catalog of MEOS-API run 36554447924; `tools/io_matrix.py
--check doc/contributing/io/ASSESSMENT.md` rebuilds the table of every type and states each cell
that differs on the tree it reads. `h3index` is h3-pg's type: `250_h3index.in.sql` keeps its
declaration as a commented reference, and h3-pg provides its four functions.

## Why the PostgreSQL functions do not carry over

A private PostgreSQL 18, toy types whose input functions have the two shapes PostgreSQL knows:

| Case | Result |
|---|---|
| a three-argument input function, called with one argument | `function toy_in(unknown) does not exist` |
| the same with `toy_in(text)` declared after `CREATE TYPE` | a literal, a `text` column and the three-argument call all answer |
| a one-argument `cstring` input function, over a `text` column | `function one_in(text) does not exist` |
| the same with `one_in(text)` declared beside it | a literal resolves without ambiguity, a `text` column and the cast `'9'::one` answer |
| `<type>_in(text)` declared before `CREATE TYPE` | refused: a SQL function cannot return a shell type |

`CREATE TYPE` finds its input function by the signatures `(cstring)` and `(cstring, oid, integer)`
alone, so a `text` overload declared after it leaves the type untouched. A set, span, span set or
box input function takes one `cstring` and refuses a `text` column; a temporal one takes three
arguments (`tint_in(cstring, oid, integer)`), so `tint_in('[1@...]')` has no match.

## How PostgreSQL holds both

The spatial types already hold the pair: each one's input function (`cstring, oid, integer`) goes
through `Temporal_in`, reading the type OID PostgreSQL passes and applying the column's typmod,
while `tspatialFromText(text) → tspatial`, one function per spatial type, is an ordinary function
through `Tspatial_from_ewkt`, reading its type from its own declared return type
(`get_fn_expr_rettype`): one wrapper for every spatial type. The other types take the second form
the same way: `ttypeFromText(text) → ttype`, one function per temporal type, over one wrapper
that takes the type from its return type and calls `temporal_in(str, type)`, the parser
`Temporal_in` calls, so the call `tintFromText('[1@...]')` and the literal `tint '[1@...]'` are
one parse; `settypeFromText`, the span, span set and box constructors over their family's reader.
`FromText` applies no typmod; a column declaring one enforces it on assignment.

## Every type, at the three layers

"MEOS" names the public function a binding calls, the type's own or its family's; "catalog" is the
class of `typeEncodings` and the encodings it lists; the five pairs are the SQL functions master
declares.

| Type | MEOS text in / out | MEOS WKB in / out | catalog class, encodings | FromText / asText | FromBinary / asBinary | FromHexWKB / asHexWKB | E forms | FromMFJSON / asMFJSON |
|---|---|---|---|---|---|---|---|---|
| `bigintset` | `bigintset_in` / `bigintset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `bigintspan` | `bigintspan_in` / `bigintspan_out` | `span_from_wkb` / `span_as_wkb` | Span, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `bigintspanset` | `bigintspanset_in` / `bigintspanset_out` | `spanset_from_wkb` / `spanset_as_wkb` | SpanSet, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `cbuffer` | `cbuffer_in` / `cbuffer_out` | `cbuffer_from_wkb` / `cbuffer_as_wkb` | Cbuffer, text,wkb | yes / yes | yes / yes | **no** / yes | all four | n/a |
| `cbufferset` | `cbufferset_in` / `cbufferset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `dateset` | `dateset_in` / `dateset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `datespan` | `datespan_in` / `datespan_out` | `span_from_wkb` / `span_as_wkb` | Span, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `datespanset` | `datespanset_in` / `datespanset_out` | `spanset_from_wkb` / `spanset_as_wkb` | SpanSet, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `floatset` | `floatset_in` / `floatset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `floatspan` | `floatspan_in` / `floatspan_out` | `span_from_wkb` / `span_as_wkb` | Span, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `floatspanset` | `floatspanset_in` / `floatspanset_out` | `spanset_from_wkb` / `spanset_as_wkb` | SpanSet, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `geogset` | `geogset_in` / `spatialset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `geomset` | `geomset_in` / `spatialset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `h3indexset` | `h3indexset_in` / `h3indexset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `intset` | `intset_in` / `intset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `intspan` | `intspan_in` / `intspan_out` | `span_from_wkb` / `span_as_wkb` | Span, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `intspanset` | `intspanset_in` / `intspanset_out` | `spanset_from_wkb` / `spanset_as_wkb` | SpanSet, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `jsonbset` | `jsonbset_in` / `jsonbset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `npoint` | `npoint_in` / `npoint_out` | `npoint_from_wkb` / `npoint_as_wkb` | Npoint, text,wkb | yes / yes | yes / yes | **no** / yes | all four | n/a |
| `npointset` | `npointset_in` / `npointset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `nsegment` | `nsegment_in` / `nsegment_out` | **no** / **no** | Nsegment, text | **no** / **no** | **no** / **no** | **no** / **no** | all four **missing** | n/a |
| `pcpatchset` | `pcpatchset_in` / `pcpatchset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `pcpointset` | `pcpointset_in` / `pcpointset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `pose` | `pose_in` / `pose_out` | `pose_from_wkb` / `pose_as_wkb` | Pose, text,wkb | yes / yes | yes / yes | **no** / yes | all four | n/a |
| `posechain` | `posechain_in` / `posechain_out` | `posechain_from_wkb` / `posechain_as_wkb` | PoseChain, text,wkb | yes / yes | yes / yes | **no** / yes | all four | n/a |
| `posechainset` | `posechainset_in` / `posechainset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `poseset` | `poseset_in` / `poseset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `quadbin` | `quadbin_in` / **no** | **no** / **no** | none, none | **no** / **no** | **no** / **no** | **no** / **no** | n/a (the grid fixes the SRID; its set and temporal types read it from their box) | n/a |
| `quadbinset` | `quadbinset_in` / `quadbinset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `raquet` | `raquet_in` / `raquet_out` | `raquet_from_wkb` / `raquet_as_wkb` | Raquet, text,wkb | **no** / **no** | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `s2cell` | `s2cell_in` / `s2cell_out` | **no** / **no** | none, none | **no** / **no** | **no** / **no** | **no** / **no** | n/a (the grid fixes the SRID; its set and temporal types read it from their box) | n/a |
| `s2cellset` | `s2cellset_in` / `s2cellset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | yes / yes | yes / yes | yes / yes | all four | n/a |
| `stbox` | `stbox_in` / `stbox_out` | `stbox_from_wkb` / `stbox_as_wkb` | STBox, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (text and WKB carry the SRID) | n/a |
| `tbigint` | `tbigint_in` / `tbigint_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |
| `tbool` | `tbool_in` / `tbool_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |
| `tbox` | `tbox_in` / `tbox_out` | `tbox_from_wkb` / `tbox_as_wkb` | TBox, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `tcbuffer` | `tcbuffer_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `textset` | `textset_in` / `textset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `tfloat` | `tfloat_in` / `tfloat_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |
| `tgeogpoint` | `tgeogpoint_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `tgeography` | `tgeography_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `tgeometry` | `tgeometry_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `tgeompoint` | `tgeompoint_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `th3index` | `th3index_in` / `th3index_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | yes / yes |
| `tint` | `tint_in` / `tint_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |
| `tjsonb` | `tjsonb_in` / `tjsonb_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |
| `tnpoint` | `tnpoint_in` / `tnpoint_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | yes / yes |
| `tpcbox` | `tpcbox_in` / `tpcbox_out` | **no** / **no** | TPCBox, text | **no** / **no** | **no** / **no** | **no** / **no** | n/a (text and WKB carry the SRID) | n/a |
| `tpcpatch` | `tpcpatch_in` / `tpcpatch_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | **no** / yes |
| `tpcpoint` | `tpcpoint_in` / `tpcpoint_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | **no** / yes |
| `tpose` | `tpose_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `tposechain` | `tposechain_in` / `tspatial_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `tquadbin` | `tquadbin_in` / `tquadbin_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | yes / yes |
| `trgeometry` | `trgeometry_in` / `trgeometry_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | **no** / yes | all four | yes / yes |
| `ts2cell` | `ts2cell_in` / `ts2cell_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | yes / yes | yes / yes | yes / yes | all four | yes / yes |
| `tstzset` | `tstzset_in` / `tstzset_out` | `set_from_wkb` / `set_as_wkb` | Set, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `tstzspan` | `tstzspan_in` / `tstzspan_out` | `span_from_wkb` / `span_as_wkb` | Span, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `tstzspanset` | `tstzspanset_in` / `tstzspanset_out` | `spanset_from_wkb` / `spanset_as_wkb` | SpanSet, text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | n/a |
| `ttext` | `ttext_in` / `ttext_out` | `temporal_from_wkb` / `temporal_as_wkb` | Temporal, mfjson,text,wkb | **no** / yes | yes / yes | yes / yes | n/a (no SRID) | yes / yes |

## The gaps

### SQL

| Missing | Types |
|---|---|
| `FromText` | the sets of `int`, `bigint`, `float`, `date`, `tstz`, `text`, `jsonb`; the spans and span sets of `int`, `bigint`, `float`, `date`, `tstz`; `tbox`, `stbox`; `tbool`, `tint`, `tbigint`, `tfloat`, `ttext` (24) |
| every portable function | `nsegment`, `quadbin`, `s2cell`, `tpcbox` |
| the `E` forms, as `npoint` has them | `nsegment` |
| `FromHexWKB`, the inverse of the `asHexWKB` they declare | `cbuffer`, `npoint`, `pose`, `posechain`, `tcbuffer`, `tgeompoint`, `tgeogpoint`, `tgeometry`, `tgeography`, `tpose`, `tposechain`, `trgeometry` (12) |
| `FromMFJSON`, the inverse of the `asMFJSON` they declare | `tpcpoint`, `tpcpatch` |

`asMFJSON` names its argument `temp` over `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`, `tjsonb`,
`th3index`, `tquadbin`, `ts2cell` and leaves it unnamed over the other eleven temporal types; one
argument takes one spelling.

### MEOS

- No WKB functions for `nsegment`, `quadbin`, `s2cell`, `tpcbox`: their `send` and `receive`
  wrappers have no MEOS function behind them (`Quadbin_send` writes `pq_sendint64` itself), so no
  binding can carry the four types as WKB.
- No `quadbin_out`: `Quadbin_out` calls `quadbin_index_to_string`, while `s2cell_out` and
  `h3index_out` exist.
- No typed `*_from_mfjson` for `th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch`, while
  the other fifteen temporal types have theirs.
- `Trgeometry_send` and `Trgeometry_recv` call `temporal_as_wkb` and `temporal_from_wkb`, whose
  `@csqlfn` names `#Temporal_send()` and not the `trgeometry` wrappers.

### The catalog (`typeEncodings`)

The bindings read, per C class, the encodings a value travels in and the functions that decode
and encode it. Of its 34 classes, measured over MEOS-API `0a54dfe` and MobilityDB `02d8ff24f0`:

- **A class several SQL types share reads and writes each type through that type's own
  reader and writer**: `Set` 18 types (`floatset_in`, `geomset_in`, ...), `Span` and `SpanSet` 5
  each, `Temporal` 20 from text and from MF-JSON; its single codec is the hex-WKB reader and
  writer (`set_from_hexwkb` / `set_as_hexwkb`), whose WKB carries the subtype.
- **Every class with a hex-WKB reader has its hex-WKB writer** (19 classes), `Nsegment` and
  `TPCBox` among them; the cell ids `H3Index`, `Quadbin`, `S2CellId` are classes of their own;
  `Raster` states no text form; the PostgreSQL time types `TimestampTz`, `Timestamp`, `TimeADT`,
  `DateADT` and `TimeTzADT` read and write through `timestamptz_in` / `timestamptz_out` and their
  siblings.
- **Still read through internal functions or not at all**: `TInstant` through `tbigintinst_in`,
  `TSequenceSet` through `tbigintseqset_in`, and `TSequence` has no reader.

The catalog's `@sqlfn` names for the one-argument readers: of the 80 public readers, the 60 of
MobilityDB's types each carry their own name (`floatset_in`, `tfloat_in`); the 20 others read
PostgreSQL's, PostGIS's and pgPointCloud's types (`bool_in`, `geom_in`, `pcpatch_hex_in`) and
carry none.

## The catalog every binding reads

The bindings that generate their surface read a type's text, WKB and MF-JSON codecs from the
catalog's `typeEncodings`, so its choices reach all of them. Measured over MEOS-API `1c9a7aa20`
and a catalog derived from MobilityDB master `7de78631f5`:

- **Three encodings, `text`, `wkb` and `mfjson`.** EWKT is filed under `text` and EWKB and hex EWKB
  under `wkb` (`parser/enrich.py`, `_ENCODERS` and `_DECODERS`), so no class states its E forms.
- **`*_out` outranks `*_as_text`** where a class has both (`parser/codecs.py`, `keyed`). The text
  writer of `Cbuffer`, `Pose` and `PoseChain` is therefore `cbuffer_out`, `pose_out` and
  `posechain_out`, which write the point as hex WKB (`basetype_out` with `T_GEOMETRY`, `geo_out`),
  and the spatial sets' is `spatialset_out`, `cbufferset_out` and `poseset_out`, through `set_out`.
  The spatial temporal types read `tspatial_as_text`, `tspatial_out` having no SQL signature.
- `TInstant` reads through `tbigintinst_in`, an internal reader of one type; `TSequence` has no
  reader; a `Datum` class carries the internal `cbuffer_wkt_out`.
- The OpenAPI and MCP surfaces generated from the catalog write a temporal value as MF-JSON and read
  it as hex WKB, so one does not read what the other writes; the MovFeat `createItem` route calls
  `temporal_from_mfjson`, an internal function. The OpenAPI artifact the CI publishes exposes no
  path (run 41207016338) and passes its validation.

## Where each binding stands

Measured at each repository's default branch: JMEOS `d749ad05e7`, MobilitySpark `b0eb3d0fd0`,
MobilityFlink `7eaef88198`, MobilityKafka `94a52cd989` (their generated surfaces rebuilt from the
same catalog), GoMEOS `0083281985`, MEOS.NET `85913570ac`, meos-rs `af6dd5e6df`, MEOS.js
`40920ac598`, MobilityDuck `c688dcd023`, MobilityNebula `a7dd28b54c`, PyMEOS `cfed5e3360` with open
PyMEOS #88, PyMEOS-CFFI `fd63bcce68`, MobilityAPI `cea483a57e`.

| Binding | Readers and writers | Default string | Value in the engine | Departs from the rules |
|---|---|---|---|---|
| JMEOS | text, WKB, hex WKB and MF-JSON for sets, spans, span sets and the temporal alphanumeric and point types; boxes text and a hex reader; `as_ewkt` writer for points | `toString` on `TBool`, the sets, spans and boxes only; the other temporal types print `Class@hash`, WKT only through `to_string` | JNR pointer | no class for `tgeometry`, the circular buffer, network point, pose and rigid geometry families, the cells or `tbigint`; `GeoSet.toString`, `as_text` and `as_ewkt` call `tspatial_*` on a set; `as_wkb` and `as_hexwkb` pass variant 4 and write EWKB; readers are instance methods; no EWKB; `IntSpan.from_wkb(Byte)` takes one byte |
| Spark, Flink (typed SQL layer) | the seven formats, readers and writers alike, for every family; text through `*_as_text`, `tspatial_as_text`, `spatialset_as_text` (WKT), `*_out` for the cells | the UDT's and RAW type's `toString` writes hex EWKB (`*_as_wkb`, variant 4) | the MEOS memory image (`MeosSqlRuntime.image`, J41) | rule 2; Flink's `GENERATION.md` states WKB where the value is its memory image; the raster and `raquet` read hex WKB and write none |
| Spark (UDF layer) | `asText`, `asEWKT`, `asHexWKB`, `asHexEWKB`, the `From*` readers of text and hex | as above | EWKB bytes | `asText` sends every span to `bigintspan_out`, every span set to `bigintspanset_out` and every set to `spatialset_as_text`; no `asText` or `asEWKT` for boxes, circular buffers, network points or poses; `asHexWKB` writes variant 4, as `asHexEWKB`; `asBinary`, `asEWKB`, `FromBinary`, `FromEWKB` unregistered |
| Kafka | the C-named `MeosOps*` facades | none | a codec the user supplies | no portable name; `pose_out` and `pose_as_text` both exposed |
| GoMEOS (`types/`, generated) | `In`/`Out`, `AsText`, WKB, hex WKB for sets, spans, span sets, boxes, temporal and base types; MF-JSON for temporal types; EWKT and EWKB writers | no `String()` in `types/`; the hand-written root package's `String()` on six temporal types, the root `TPointOut` calling `tspatial_out` (hex) | cgo pointer | `GeomSet` and `GeogSet` read and do not write; no EWKT or EWKB reader; geometry arguments typed `*TRGeometrySeqSet` (252 sites, `tools/objectgen.py`); WKB readers take `unsafe.Pointer` and a size, writers return `unsafe.Pointer` with a caller's `size_out`; no cell type |
| MEOS.NET (generated) | as GoMEOS, WKB as `byte[]` | `ToString` is `Out`: `tspatial_out`, `pose_out`, `cbuffer_out`, `geo_out`, hex WKB for every spatial value; none on `GeomSet`, `GeogSet`, `Set`, `Span`, `SpanSet`, `Temporal` | P/Invoke pointer | rule 2; `GeomSet` and `GeogSet` write nothing; no EWKT or EWKB reader; `AsHEXWKB` beside `AsHexewkb` in one class; no cell class |
| meos-rs | `FromStr`, WKB, hex WKB for spans, span sets, boxes and the temporal alphanumeric and geometric types; MF-JSON; `as_ewkt`, `as_geojson` | no `Display`; `Debug` writes `*_out`, WKT for the spatial types | pointer | rule 4: `as_wkb` and `as_hexwkb` return `&[u8]` over memory MEOS allocates and nothing frees; hex as `&[u8]`; no sets, extended families or cells |
| MEOS.js | text, hex WKB, MF-JSON for the exported classes (sets, spans, span sets, boxes, `tbool`, `tint`, `tfloat`, `ttext`, `tgeompoint`, `tgeogpoint`); `asEWKT` for points | `toString`: `tspatial_as_text` for points, `*_out` otherwise | WASM heap pointer | no raw WKB; no EWKB; `fromHexWKB` missing on `tbool`, `tint`, `tfloat`, `ttext`, `tgeogpoint`; the extended families, spatial sets and `tgeometry` unexported though their wrappers are generated; `temporal_as_hexwkb_w` passes variant `-1` |
| MobilityDuck | the seven formats for sets, spans, span sets, boxes and the temporal types (`src/generated`), EWKB and hex EWKB writers | the `VARCHAR` cast is `*_out`: `0101000000000000000000F03F…@2012-01-01` for a `tgeompoint` (`test/sql/tgeompoint.test`), hex for every spatial value | a `BLOB` holding the MEOS memory image (`Temporal_in` copies `temporal_mem_size` bytes); cells `BIGINT` | rule 2; no `FromText`, `FromEWKT` or `FromMFJSON` for `tcbuffer`, `tnpoint`, `tpose`, `tposechain`, `trgeometry`, `th3index`, `tquadbin`, `ts2cell`; no binary or hex I/O for `cbuffer`, `npoint`, `pose`, `posechain`; `nsegment` a cast only; E forms registered for the cells, which carry no SRID; `tgeogpoint` registered twice |
| MobilityNebula | none for the user; internal readers `tgeompoint_in`, `tgeometry_in`, `geom_in`, `stbox_in` | none | strings built from the stream's fields | the one writer, `temporal_as_wkb(…, 0x08)`, is commented "extended WKB" while `0x08` is `WKB_NDR`; the temporal sequence aggregation returns the string `BINARY(<size>)`; its ledger holds the 205 I/O functions apart |
| PyMEOS | text, WKB, hex WKB for every class; MF-JSON for temporal types; `as_ewkt` for the spatial ones | `tspatial_as_text` for the temporal spatial classes, `*_out` for the others; `Pose` and `Cbuffer` `pose_out` and `cbuffer_out` (hex) | CFFI pointer | no `from_ewkt`, `from_ewkb`, `from_hexewkb`, `as_ewkb`, `as_hexewkb`; `as_text` only on the spatial base types and sets; no `TRgeometry` string constructor, `trgeometry_out` its string |
| PyMEOS-CFFI | readers take `bytes` and `str` alone (`geo_from_ewkb` also the SRID); every byte buffer returns `bytes` | n/a | n/a | rule 4: `*_as_hexwkb` and `*_as_hexewkb` return `(str, size)`, but for `geo_as_hexewkb`, `pcpoint_as_hexwkb`, `pcpatch_as_hexwkb` |
| MobilityAPI | MobilityDB SQL: `asMFJSON`, `tgeompointFromMFJSON`, `asBinary`, `ST_AsGeoJSON` | n/a | SQL | its Spark dialect names `temporalAsMfjson` and `tgeompointFromMfjson` |
