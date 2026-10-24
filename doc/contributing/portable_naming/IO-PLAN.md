<!--
  MobilityDB — Portable Naming: Input and output in every engine
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Input and output in every engine

[Back to the index](00-INDEX.md) · [Implementation plan](IMPLEMENTATION-PLAN.md)

PostgreSQL reads and writes a type through the four functions its `CREATE TYPE` names: `input`
and `output` for its text, `receive` and `send` for its binary form. It calls them implicitly: a
literal `tint '1@2001-01-01'`, a cast from `text`, `COPY` and a text `INSERT` go through `input`,
a value psql prints goes through `output`, and a binary `COPY` or a driver in binary mode goes
through `receive` and `send`. Spark, Flink and every MEOS binding call nothing implicitly: a
value travels as its WKB bytes (rule 6), and each direction is a function the query calls by
name. This document states that function for every type, the MEOS function a binding calls
behind it, what the catalog tells the bindings, what is missing at each of the three layers, and
the pull requests that close it.

Measured at MobilityDB master `845a60f141` (its `mobilitydb/sql` and public headers identical to
`0be1b51060`'s), over `mobilitydb/sql` with its comments removed, the public MEOS headers
(`meos*.h` but `meos_internal*.h`), and the catalog of MEOS-API run 36554447924. `h3index` is h3-pg's type: `250_h3index.in.sql` keeps its declaration as a
commented reference, and h3-pg provides its four functions.

## Rule 9 and its family: `<type>From<Format>` and `as<Format>`

1. **Every type has its text constructor `<type>FromText(text)`**, the inverse of the `asText`
   it has, in every engine: a typed literal `tint '[1@2001-01-01, 2@2001-01-02]'` is written
   `tintFromText('[1@2001-01-01, 2@2001-01-02]')`, and a `text` column `tintFromText(col)`;
   a type whose text is its hex WKB, as `raquet`, whose input and output functions read
   and write it, takes its text through `raquetFromHexWKB` and `asHexWKB`, as the PostGIS
   raster, whose `raster_in` and `raster_out` read and write hex WKB, has `ST_AsHexWKB` and
   `ST_RastFromHexWKB` and no `ST_AsText` (`meos/src/raster/raquet.c`: `raquet_in` is `raquet_from_hexwkb`, `raquet_out`
   is `raquet_as_hexwkb` in NDR).
2. **The same pairing holds for every format.** The dialect names a constructor by the format it
   reads, `<type>From<Format>`, and the output by the format it writes, `as<Format>`; each is the
   inverse of the other, and a type has the pair for every format it supports:

   | PostgreSQL function | Direction | Portable functions | MEOS |
   |---|---|---|---|
   | `<type>_in(cstring)` | text → value | `<type>FromText(text)`; with an SRID also `<type>FromEWKT(text)` | `<type>_in`, the family's reader |
   | `<type>_out(<type>)` | value → text | `asText(<type>)`; with an SRID also `asEWKT(<type>)` | `<type>_out`, `tspatial_out`, `spatialset_out` |
   | `<type>_recv(internal)` | binary → value | `<type>FromBinary(bytea)`, `<type>FromHexWKB(text)`; with an SRID also `<type>FromEWKB(bytea)`, `<type>FromHexEWKB(text)` | `*_from_wkb`, `*_from_hexwkb` |
   | `<type>_send(<type>)` | value → binary | `asBinary(<type>)`, `asHexWKB(<type>)`; with an SRID also `asEWKB(<type>)`, `asHexEWKB(<type>)` | `*_as_wkb`, `*_as_hexwkb` |
   | none | MF-JSON → value, value → MF-JSON | `<type>FromMFJSON(text)`, `asMFJSON(<type>)` for a temporal type | `*_from_mfjson`, `temporal_as_mfjson` |

   The `E` forms carry the SRID, as PostGIS `ST_AsEWKT` and `ST_AsEWKB` do, and every type
   carrying an SRID behaves alike in binary: `asBinary` and `asHexWKB` write PostGIS-compatible
   plain WKB, `asEWKB` and `asHexEWKB` its SRID (G40), the boxes `stbox` and `tpcbox` included.
   A box writes its SRID in its text form: `stbox_out` and `tpcbox_out` write `SRID=n;`
   ("matching the sibling (GEOD)STBOX text form"). A set or a temporal
   value carries its SRID in its bounding box, from which `spatialset_as_ewkt` and
   `tspatial_as_ewkt` write it for every spatial set and temporal type; a base value writes the
   SRID it stores (`cbuffer_as_ewkt`, `pose_as_ewkt`, `posechain_as_ewkt`, `geo_as_ewkt`) or the
   one it inherits from the `ways` table (`npoint_as_ewkt`), and `nsegment`, which inherits the
   same SRID and whose `nsegment_out` writes `NSegment(rid,pos1,pos2)` without it, takes the `E`
   forms `npoint` has. A cell (`quadbin`, `s2cell`, as h3-pg's `h3index`) and a `raquet` tile
   carry no SRID in the value, the grid fixing it, and take none.
   Every WKB output takes `endian text DEFAULT ''` (57 `asBinary`, 57 `asHexWKB`, 29 `asEWKB`,
   29 `asHexEWKB`), and the text outputs take `maxdecimaldigits integer DEFAULT 15` where the
   value carries floats or coordinates.
3. **`<type>_in` stays the type's input function**, the plumbing `CREATE TYPE` binds and the
   dialect never names; the portable functions are ordinary functions beside it.
4. **The catalog names each reader and writer through the wrapper that reaches it**, and the
   bindings register one function per `sqlSignatures` entry.

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

## The plan

| Gap | Pull request |
|---|---|
| `FromText` for the 24 types | G13 (`<type>FromText`, one wrapper per family over its reader) |
| `asText`, `FromText` for `nsegment`, `quadbin`, `s2cell`, `tpcbox`, and `asEWKT`, `nsegmentFromEWKT`; MEOS `quadbin_out` | G14 |
| MEOS WKB and SQL `asBinary`, `FromBinary`, `asHexWKB`, `FromHexWKB` for `nsegment`, `quadbin`, `s2cell`, `tpcbox`, and `asEWKB`, `nsegmentFromEWKB`, `asHexEWKB`, `nsegmentFromHexEWKB`; the `tpcbox` WKB carrying its SRID as `stbox`'s does | G6 |
| `FromHexWKB` for the 12 spatial types; the `@csqlfn` naming `Trgeometry_send`/`_recv` | G8 |
| MEOS `*_from_mfjson` for `th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch`; SQL `FromMFJSON` for `tpcpoint`, `tpcpatch`; one spelling of `asMFJSON`'s argument | G9 |
| the readers and writers per SQL type, the hex-WKB writers, `Nsegment`, `TPCBox`, `Raster` and the cell classes in `typeEncodings`; the readers' `@sqlfn` names | MEOS-API A6, merged as #157 and #159 |

## Acceptance

For every type and every format it supports, in PostgreSQL and through each binding,
`<type>From<Format>(as<Format>(v))` returns `v` over the regression fixtures of the type, and
`<type>FromText(asText(v))` equals the value the typed literal reads. The table above, rebuilt
by `io_matrix.py`, then reads yes in every cell, or n/a where the type has no SRID or is not
temporal.
