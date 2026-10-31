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

## In the bindings

MEOS is a closed algebra: a binding's object layer holds every value as its MEOS type, and a host
type (shapely in Python, JTS in Java, a GeoJSON object in JavaScript) is reached only through an
explicit cast. Its input and output follow from that (the owner, 2026-10-31):

1. **Every type has the readers and writers of the portable functions**, named in the binding's
   own case (`as_text`, `as_ewkt`, `as_wkb`, `as_hexwkb`, `as_ewkb`, `as_hexewkb`, `as_mfjson`,
   and `from_*` for each in Python; `asText`, `fromHexWKB` ... in Java, JavaScript and C#;
   `AsText`, `FromHexWKB` ... in Go), each calling the MEOS function the SQL function of the same
   portable name reaches: `asText(cbuffer)` is `Cbuffer_as_text`, so a binding's `as_text` of a
   circular buffer calls `cbuffer_as_text`.
2. **The default string conversion is `asText`** (Python `str`, Java `toString`, Go `String`, C#
   `ToString`, Rust `Display`, JavaScript `toString`): the `*_as_text` function, which writes a
   spatial value's points as WKT, never the `*_out` function, which writes them as hex WKB as
   PostGIS's `geometry_out` does. `pose_out` writes `POSE (0101000000000000000000F03F000000000000F03F, 0.5)`
   where `pose_as_text` writes `Pose(POINT(1 1),0.5)`, the form a temporal pose's instants print in.
3. **A value an engine holds is its MEOS memory image where the catalog states one**
   (`typeEncodings[C].image`, A25), as Spark and Flink hold it since J41, and its WKB where the
   value leaves the engine (rule 6 of the dialect: `asBinary`, `<type>FromBinary`); the text forms
   are what a user reads and writes.
4. **A reader is a function of the format alone**: `<type>FromHexWKB(text)` takes the string,
   `<type>FromBinary(bytea)` the bytes, whose length is their own; it needs no value to be called
   on and no length beside the buffer. A writer returns the language's own string or byte type,
   owning its memory.

### The catalog every binding reads

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

### Where each binding stands

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

## The plan

| Gap | Pull request |
|---|---|
| `FromText` for the 24 types | G13 (`<type>FromText`, one wrapper per family over its reader) |
| `asText`, `FromText` for `nsegment`, `quadbin`, `s2cell`, `tpcbox`, and `asEWKT`, `nsegmentFromEWKT`; MEOS `quadbin_out` | G14 |
| MEOS WKB and SQL `asBinary`, `FromBinary`, `asHexWKB`, `FromHexWKB` for `nsegment`, `quadbin`, `s2cell`, `tpcbox`, and `asEWKB`, `nsegmentFromEWKB`, `asHexEWKB`, `nsegmentFromHexEWKB`; the `tpcbox` WKB carrying its SRID as `stbox`'s does | G6 |
| `FromHexWKB` for the 12 spatial types; the `@csqlfn` naming `Trgeometry_send`/`_recv` | G8 |
| MEOS `*_from_mfjson` for `th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch`; SQL `FromMFJSON` for `tpcpoint`, `tpcpatch`; one spelling of `asMFJSON`'s argument | G9 |
| the readers and writers per SQL type, the hex-WKB writers, `Nsegment`, `TPCBox`, `Raster` and the cell classes in `typeEncodings`; the readers' `@sqlfn` names | MEOS-API A6, merged as #157 and #159 |

The bindings, one pull request per repository, each generated from the catalog where the binding
generates ([the bindings plan](BINDINGS-PLAN.md)), the catalog's first:

| Task | Repository | Change | State |
|---|---|---|---|
| IO-B1 | MEOS-API | `typeEncodings` states `text` (`*_as_text`, `tspatial_as_text`, `spatialset_as_text` for a spatial class, `*_out` for the others), `ewkt`, `wkb`, `hexwkb`, `ewkb`, `hexewkb` and `mfjson` as encodings of their own; no reader of its own for `TInstant`, `TSequence` and `TSequenceSet`, which read through `Temporal`'s public per-type readers (the portable functions are per SQL type, and the 63 subtype readers are internal); the OpenAPI and MCP surfaces read and write a temporal value in one format; MovFeat `createItem` through a public reader; the CI's OpenAPI artifact exposes the catalog's paths | four branches, one topic each, off MEOS-API `1c9a7aa`: MERGED as MEOS-API #224, `feat/io-encodings` (`591ba36`: the seven encodings, `*_as_text` the text of a spatial class, plain WKB from `wkb` and `hexwkb`, the service wire in the type's `send` form, the subtypes reading through `Temporal`; 641 tests), MERGED as MEOS-API #225, `fix/movfeat-public-reader` (`dca51ae`, `createItem` through `tgeompoint_from_mfjson`), MERGED as MEOS-API #226's second commit (`52df0f8`, the OpenAPI catalog derived through `provision-meos.sh`, 3614 paths, a document without paths refused), MERGED as MEOS-API #226, `fix/engine-writer-outparams` (`229d253`, the service engine passing a writer the `size_t *size_out` its `shape.outParams` lists) |
| IO-B2 | MEOS-API | `meta/object-model.json` attaches the spatial set readers and writers (`spatialset_as_text`, `spatialset_as_ewkt`, `set_as_wkb`, `set_as_hexwkb`) to `GeomSet` and `GeogSet` | after IO-B1 |
| IO-B3 | PyMEOS | `str` is `as_text` for every class; `as_text` and `as_ewkt` on every spatial class and set; `from_ewkt`, `from_ewkb`, `from_hexewkb`, `as_ewkb`, `as_hexewkb` where the type carries an SRID; the `TRgeometry` string constructor (`trgeometry_in`) and its `str` through `trgeometry_as_text` | the classes PyMEOS #88 adds (`407ec84` on master `cfed5e3`) write `str` through their `*_as_text`, the sets carry `as_text`, `as_wkt` and `as_ewkt`, and `TRgeometry` reads `trgeometry_in` and writes `trgeometry_as_text` (4724 tests pass); #88's `TRgeometry.value_at_timestamp` calls the public `trgeometry_value_at_timestamptz` MERGED as MobilityDB #3336 (`de13829e48`); #88 green 8 of 8 at `407ec84`; `as_text` on the temporal classes and the E forms are left |
| IO-B4 | PyMEOS-CFFI | `*_as_hexwkb` and `*_as_hexewkb` return the `str` alone | open |
| IO-B5 | GoMEOS | `String()` is `AsText` on every generated type; WKB readers take `[]byte` and writers return `[]byte`; `FromEWKT`, `FromEWKB`, `FromHexEWKB`; the `GSERIALIZED` arguments typed as the geometry type (`tools/objectgen.py`); the cell types | after IO-B2 |
| IO-B6 | MEOS.NET | `ToString` is `AsText`; `FromEWKT`, `FromEWKB`, `FromHexEWKB`; one casing (`AsHexWKB`, `AsHexEWKB`); the cell classes | after IO-B2 |
| IO-B7 | meos-rs | `Display` is `asText`; `as_wkb` returns an owned `Vec<u8>` and `as_hexwkb` a `String`, MEOS's buffer freed; the sets and the extended families | after B7 of the bindings plan |
| IO-B8 | MEOS.js | the extended families, spatial sets and `tgeometry` exported; `fromBinary`/`asBinary`; `fromHexWKB` on every temporal type; `asHexWKB` writing the variant its documentation states | the readers and writers: branch `feat/io-readers-writers` (`dde6d68`) on `fix/size-t-bigint` (`390504b`, a `size_t` argument passed as a BigInt, without which no `*_from_wkb` reader reaches MEOS), 1147 tests on the module built from it, 0 skipped; the classes of the extended families, spatial sets and `tgeometry` are their own pull requests, one family each |
| IO-M1 | MobilityDB | `tspatial_as_hexewkb` writes the SRID once, on the temporal value, as `geo_to_wkb_size` states for a container, rather than again on every point it embeds (`0101000020E6100000…`) | open |
| IO-B9 | MobilityDuck | the `VARCHAR` cast is `asText`; `FromText`, `FromEWKT`, `FromMFJSON` for the extended temporal families and the cell temporal types; binary and hex I/O for `cbuffer`, `npoint`, `pose`, `posechain`; `nsegment`'s functions; no E form for a cell; one `tgeogpoint` registration | MERGED as MobilityDuck #545 (`b07e7a5`), one pull request of seven commits: one `tgeogpoint` registration; the cast as `asText`; the generated readers and writers of the extended families and the temporal cells; no `E` form over a type the catalog gives no SRID (h3index, quadbin, s2cell, pcpoint, pcpatch); the generated index search header and the refresh that stages it; every generated body registered, `round(stbox[])` read as an array; 6031 assertions in 132 test cases |
| IO-B10 | JMEOS | `toString` is `asText` on every class; `GeoSet` through `spatialset_*`; static readers; `as_wkb` plain WKB beside `as_ewkb`; the box writers; the classes of the extended families, the cells and `tbigint`; `IntSpan.from_wkb(byte[])` | five branches, one topic each: MERGED as JMEOS #210, `fix/geoset-spatialset-io` (`87b4621f`: `GeoSet` text and EWKT through `spatialset_*`, its SRID and rounding through the set functions, the geo sets built from a pointer holding it), MERGED as JMEOS #211, `feat/tostring-astext` (`1f14977c`), MERGED as JMEOS #212, `feat/static-readers` (`e76b1765`, `Factory.create_temporal`, the box WKB), MERGED as JMEOS #214, `fix/plain-wkb-and-ewkb` (`19ea77ef`), MERGED as JMEOS #215, `fix/set-span-createinner` (`ab2e1022`: `tstzspan`, `tstzspanset`, `TextSet` built from a pointer holding it); the classes of the extended families, the cells and `tbigint` are their own pull requests |
| IO-B11 | JMEOS (Spark, Flink generators) | the UDT and RAW `toString` is `asText`; the UDF layer's `asText` and `asEWKT` dispatch by the argument's type; `asHexWKB` plain WKB; `asBinary`, `asEWKB`, `FromBinary`, `FromEWKB` registered; the raster and `raquet` hex writers; the UDF layer takes the type of a value from its typed signature, as the typed SQL layer does, since the WKB of a box, circular buffer, network point and pose does not state its type | MERGED as JMEOS #213, branch `fix/spark-udf-astext-dispatch` (`97f8871f`): each span, span set and set written by its own type, `asBinary`, `asEWKB`, `FromBinary`, `FromEWKB` registered, plain WKB by default, no `*_send` or `*_recv` name; MobilitySpark `test/astext-by-type` (`1d9fc52`), 356 tests |
| IO-B11b | MEOS-API, JMEOS | the catalog marks the signatures of a type's `CREATE TYPE` and `CREATE AGGREGATE` support functions, and the Spark UDF layer registers none of them under a C name, the 143 `*_in`/`*_out` among them, each reachable through its portable reader or writer | after IO-B1 |
| IO-B12 | MobilityKafka | the portable names over the facades | after IO-B11 |
| IO-B13 | MobilityNebula | `asText`, `asBinary`, `asHexWKB`, `asMFJSON` and their readers for the types it carries; the temporal sequence aggregation returning the value | PAUSED BY THE OWNER 2026-10-31: MobilityNebula lags a large queue of open pull requests (#194 to #197 among them) |
| IO-B14 | MobilityAPI | its Spark dialect names `asMFJSON` and `<type>FromMFJSON` | after IO-B11 |
| IO-B16 | MobilityDB | one input and output test covering every type and every format it supports, which each binding replays through its own surface: the reader and writer queries of the `nnn_*.test.sql` files extracted with their expected lines, then a query for every type and format no extracted query covers, over the `tbl_<type>` fixtures, its expected output harvested from PostgreSQL; the generator states which queries are extracted and which added, and the coverage of each (DECIDED BY THE OWNER 2026-10-31) | branch `test/io-every-type` (`aa3ba74abe` on master `1ff2270f7f`), `tools/gen_io_test.py` and `io/700_io_every_type_tbl`: 4951 extracted queries from 112 test files and 37 added, 289 of the 349 pairs of the 59 types with a round trip, 58 written only, 2 without a fixture (`posechainset` Text, `ts2cell` Output); an extracted query whose expected output prints a float beyond six decimal digits is left out |
| IO-B15 | MobilityDB | `tools/io_matrix.py`, which rebuilds the table of every type from `mobilitydb/sql`, the public headers and the catalog, and `--check` diffs it against this document; it reproduces the table at `845a60f141`, its measured commit, cell for cell, and at master the 56 cells G6, G8, G9, G13 and G14 closed | branch `tools/io-matrix` (`88237170f1` on master `1ff2270f7f`), strict-ci receipt |
| IO-B15b | MobilityDB | the bindings' table rebuilt by the same tool from each binding's generated surface at a pinned commit (`--repo NAME=PATH`) | after IO-B15 |

DECIDED BY THE OWNER 2026-10-31: DuckDB behaves as every binding, holding a value as its MEOS
memory image and exchanging it as its WKB (rule 3); MobilityDuck's `BLOB` of the memory image
stands, and D9.13 states it.

DECIDED BY THE OWNER 2026-10-31: `asBinary` and `asHexWKB` write plain WKB in every binding, and
`asEWKB` and `asHexEWKB` write the SRID, exactly as in PostgreSQL (rule 2 of the dialect, G40). A
binding's `as_wkb`, `as_hexwkb`, `asHexWKB` writing variant 4 (JMEOS, the Spark UDF layer) or every
flag (MEOS.js `temporal_as_hexwkb_w`) writes plain WKB instead, and the binding gains the `E` forms
where it lacks them (IO-B8, IO-B10, IO-B11).

## Acceptance

DECIDED BY THE OWNER 2026-10-31: the input and output tests of MobilityDB are the oracle of every
binding. Every query of `mobilitydb/test/*/queries/*.test.sql` that calls a reader or a writer, the
`_inout` files (`053_tgeo_inout`, `053_tpoint_inout`, `151_trgeo_inout`, `023_temporal_inout_tbl`
and their `_tbl` twins) and the input and output queries of each type's own file
(`001_set.test.sql` and its siblings), answers in each binding exactly the text its
`expected/*.test.out` holds: the same value, the same digits, the same WKB bytes. A binding runs
those queries through its own surface, its default string included, and a difference is a defect of
the binding or, where every binding agrees against PostgreSQL, of MEOS. Where it is the more
efficient carrier, the oracle is one input and output test generated for the campaign (IO-B16): for
every type and every format it supports, a value of the type's regression fixtures read through
`<type>From<Format>` and written through `as<Format>`, and the typed literal written through its
default string, its expected output harvested from PostgreSQL, never written by hand.

For every type and every format it supports, in PostgreSQL and through each binding,
`<type>From<Format>(as<Format>(v))` returns `v` over the regression fixtures of the type, and
`<type>FromText(asText(v))` equals the value the typed literal reads. The table above, rebuilt
by `io_matrix.py`, then reads yes in every cell, or n/a where the type has no SRID or is not
temporal.
