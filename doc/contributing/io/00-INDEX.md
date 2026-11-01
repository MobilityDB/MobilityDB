<!--
  MobilityDB — Input and output: index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Input and output in every engine

PostgreSQL reads and writes a type through the four functions its `CREATE TYPE` names: `input`
and `output` for its text, `receive` and `send` for its binary form. It calls them implicitly: a
literal `tint '1@2001-01-01'`, a cast from `text`, `COPY` and a text `INSERT` go through `input`,
a value psql prints goes through `output`, and a binary `COPY` or a driver in binary mode goes
through `receive` and `send`. Spark, Flink and every MEOS binding call nothing implicitly: a
value travels as its WKB bytes (rule 6 of the portable dialect), and each direction is a
function the query calls by name. These notes state that function for every type, the MEOS
function a binding calls behind it, what the catalog tells the bindings, what is missing at each
of the three layers, and the pull requests that close it.

The names, the engines and the bindings are the subject of the portable naming notes of
MobilityDB #2861 (`doc/contributing/portable_naming/`), whose implementation plan holds the rows
named G, A, D and J these notes cite; how close each answer is to the exact one is the exactness
plan of MobilityDB #3287 (`doc/contributing/exactness/`).

## The notes

| Note | What it states |
|---|---|
| [Assessment](ASSESSMENT.md) | Why the PostgreSQL input and output functions do not carry over to the other engines, every type at the three layers (the SQL functions, the MEOS functions a binding calls, the catalog's `typeEncodings`), the gaps at each layer, and where each binding stands |
| [Implementation plan](IMPLEMENTATION-PLAN.md) | The pull request that closes each gap, in MobilityDB, the catalog and each binding, and its state |

## Where things stand

- **MobilityDB declares every portable function the assessment lists as missing**: `FromText`
  for the 24 types (G13, #2908), the text and WKB functions of `nsegment`, `quadbin`, `s2cell`
  and `tpcbox` (G14, #2901; G6, #2877), `FromHexWKB` for the 12 spatial types (G8, #2882) and
  `FromMFJSON` for the point clouds (G9, #2883). `tools/io_matrix.py` (IO-B15, #3334) rebuilds
  the table of every type and checks it against the assessment, and `io/700_io_every_type_tbl`
  (IO-B16, #3338) reads and writes every type in every format it supports.
- **The catalog states each format as an encoding of its own** (IO-B1, MEOS-API #224 to #226),
  each SQL type's own reader and writer (A6, MEOS-API #157 and #159), and the classes the
  spatial sets' functions belong to (IO-B2, MEOS-API #228).
- **PyMEOS, JMEOS, the Spark UDF layer and MobilityDuck follow the binding rules** (IO-B3,
  IO-B9, IO-B10, IO-B11), and MEOS.js reads and writes every format (IO-B8, MEOS.js #29 and
  #30).
- **Open**: the EWKB of a container stating its SRID once (IO-M1); PyMEOS-CFFI, GoMEOS,
  MEOS.NET, meos-rs, MobilityKafka and MobilityAPI (IO-B4 to IO-B7, IO-B12, IO-B14); the
  MEOS.js classes of the extended families; the catalog marking the type support functions
  (IO-B11b); the bindings' table rebuilt by `io_matrix.py` (IO-B15b). MobilityNebula (IO-B13) is
  paused by the owner.

## The SQL functions: `<type>From<Format>` and `as<Format>`

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

## The bindings

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
`<type>FromText(asText(v))` equals the value the typed literal reads. The table of [the assessment](ASSESSMENT.md), rebuilt
by `io_matrix.py`, then reads yes in every cell, or n/a where the type has no SRID or is not
temporal.
