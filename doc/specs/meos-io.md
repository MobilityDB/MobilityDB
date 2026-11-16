<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Input and Output
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Input and Output — Interface Specification

| | |
|---|---|
| Version | v1.0.0 |
| Scope | The functions that read a MEOS or MobilityDB value from a format and write it back, in SQL, in MEOS and in every binding, their names, the SRID they carry, the size of the strings they return, and the conversion of the JSON numbers a temporal JSON value holds |
| Reference | `mobilitydb/sql/` (the `<type>From<Format>` and `as<Format>` functions), `meos/include/meos.h` and the family headers beside it (the readers and writers), `meos/src/json/tjsonb_jsonfuncs.c` (`numeric_to_alphanum`), `mobilitydb/test/io/sql/700_io_every_type_tbl.sql`, `tools/io_matrix.py` |

## 1. Scope and intent

A value of MEOS travels between engines and languages through a small set of
formats: its text, its WKB and the hexadecimal form of its WKB, each with or
without its SRID, and for a temporal value its MF-JSON. This document states,
for every type, the function that reads it from each format it supports and
the function that writes it, under the same names in PostgreSQL, in MEOS and
in every binding generated from MEOS. It is prescriptive: a binding or an
engine that exposes a value follows it, and the regression tests of
MobilityDB are its oracle (section 8).

PostgreSQL calls a type's input and output implicitly: a literal, a cast from
`text`, `COPY` and psql's display go through the functions `CREATE TYPE`
names. Spark, Flink, DuckDB and every binding call nothing implicitly, so each
direction is a function a query or a program calls by name, and these names
are the ones every engine shares.

## 2. Notation

`<type>` is a SQL type name as `meos/src/temporal/meos_catalog.c` spells it
(`tint`, `stbox`, `tgeompoint`); `<Format>` is one of `Text`, `EWKT`,
`Binary`, `EWKB`, `HexWKB`, `HexEWKB` and `MFJSON`. The `E` forms are those
carrying the SRID, as PostGIS's `ST_AsEWKT` and `ST_AsEWKB` do.

## 3. The SQL functions

### 3.1 The pairing

Each type has, for every format it supports, a reader `<type>From<Format>`
and a writer `as<Format>`, each the inverse of the other:

| Direction | Functions | MEOS |
|---|---|---|
| text → value | `<type>FromText(text)`; with an SRID also `<type>FromEWKT(text)` | the type's reader (`tint_in`, `set_in` ...) |
| value → text | `asText(<type>)`; with an SRID also `asEWKT(<type>)` | `set_out`, `span_out`, `spanset_out`, `temporal_out`, the `<type>_out` of a base or box type, or `*_as_text` and `*_as_ewkt` |
| binary → value | `<type>FromBinary(bytea)`, `<type>FromHexWKB(text)`; with an SRID also `<type>FromEWKB(bytea)`, `<type>FromHexEWKB(text)` | `*_from_wkb`, `*_from_hexwkb` |
| value → binary | `asBinary(<type>)`, `asHexWKB(<type>)`; with an SRID also `asEWKB(<type>)`, `asHexEWKB(<type>)` | `*_as_wkb`, `*_as_hexwkb` |
| MF-JSON ↔ value | `<type>FromMFJSON(text)`, `asMFJSON(<type>)`, for a temporal type | `*_from_mfjson`, `temporal_as_mfjson` |

The type's input function (`<type>_in`) stays the plumbing `CREATE TYPE`
binds; the portable functions are ordinary functions beside it. A typed
literal `tint '[1@2001-01-01, 2@2001-01-02]'` is written in every engine
`tintFromText('[1@2001-01-01, 2@2001-01-02]')`.

A type whose text is its hexadecimal WKB, as the Raquet tile `raquet` and
PostGIS's `raster`, takes its text through `<type>FromHexWKB` and `asHexWKB`
and has no `asText`.

### 3.2 The SRID

`asBinary` and `asHexWKB` write the WKB PostGIS writes, without the SRID;
`asEWKB`, `asHexEWKB` and `asEWKT` write it, the boxes `stbox` and `tpcbox`
included. `asText` writes a box without its SRID and `asEWKT` with it, and
`stboxFromText` reads both (`mobilitydb/test/geo/sql/051_stbox.sql`):

```sql
SELECT asText(stbox 'SRID=4326;STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])');
-- STBOX XT(((1,1),(2,2)),[Mon Jan 01 00:00:00 2001 PST, Tue Jan 02 00:00:00 2001 PST])
SELECT asEWKT(stbox 'SRID=4326;STBOX XT(((1,1),(2,2)),[2001-01-01,2001-01-02])');
-- SRID=4326;STBOX XT(((1,1),(2,2)),[Mon Jan 01 00:00:00 2001 PST, Tue Jan 02 00:00:00 2001 PST])
```

and the WKB of a temporal point carries it in its `E` form
(`mobilitydb/test/geo/sql/053_tpoint_inout.sql`):

```sql
SELECT asHexEWKB(tgeompoint 'SRID=4326;Point(1 1)@2001-01-01');
-- 012E0041E61000000101000000000000000000F03F000000000000F03F0060885EC91C0000
```

A set or a temporal value carries its SRID in its bounding box; a base value
writes the SRID it stores, or for a network point the one of the `ways` table
its routes come from. A grid cell (`h3index`, `quadbin`, `s2cell`) and a
Raquet tile carry none, the grid fixing it, and have no `E` forms.

### 3.3 Arguments

Every WKB writer takes `endian text DEFAULT ''`, and the text writers of a
value holding floats or coordinates take `maxdecimaldigits integer DEFAULT 15`.
The WKB of a `pcpoint` and of a `pcpatch` is the one of pgPointCloud, in the
byte order `endian` states, on its own and inside a set or a temporal value.
`mobilitydb/sql/` declares 63 `asBinary`, 63 `asHexWKB`, 34 `asEWKB`, 34
`asHexEWKB`, 86 `asText`, 51 `asEWKT` and 20 `asMFJSON`, and the readers to
match: 61 `FromText`, 63 `FromBinary`, 63 `FromHexWKB`, 32 `FromEWKT` and 20
`FromMFJSON`. `tools/io_matrix.py` rebuilds the table of every type and format
from the source.

## 4. The MEOS functions

1. **One writer per template.** A value carries its type, so each template
   has one writer: `set_out`, `span_out`, `spanset_out` and `temporal_out`
   back both the output of each type and `asText`, as `set_as_wkb` and
   `temporal_as_wkb` back `asBinary`. A separate `*_as_text` exists only where
   `asText` writes another text than the type's output: `tspatial_as_text`
   writes the points as WKT where `temporal_out` writes them as hexadecimal
   WKB, as PostGIS's `geometry_out` does.
2. **One reader per template where the format states the type, one per type
   where it does not.** WKB states the type (`set_from_wkb`,
   `temporal_from_wkb`); text does not, `{1, 2}` being an `intset` or a
   `bigintset`, so `intset_in` and `tint_in` read it. A box has no template,
   so its functions are per type.
3. **Every writer of a value states the size of the string it returns.** Each
   of the 100 public functions of `meos/include/` that writes a value as a
   string takes `size_t *size_out` and sets it to the length of the string, as
   the hexadecimal WKB writers do, so that a binding reads the string by its
   size rather than scanning for its terminator. The functions returning the
   name of an enumeration constant or of a type return a static string and
   take no size.
4. **A reader is a function of the format alone.** `<type>FromHexWKB` takes
   the string and `<type>FromBinary` the bytes, whose length is their own; a
   reader needs no value to be called on.

## 5. The bindings

1. **The portable names in the language's own case.** Every type has the
   readers and writers of section 3 (`as_text`, `as_ewkt`, `as_wkb`,
   `as_hexwkb`, `as_ewkb`, `as_hexewkb`, `as_mfjson` and the `from_*` of each
   in Python; `asText`, `fromHexWKB` ... in Java, JavaScript and C#;
   `AsText`, `FromHexWKB` ... in Go), each calling the MEOS function the SQL
   function of the same name reaches.
2. **The default string is `asText`** (Python `str`, Java `toString`, Go
   `String`, C# `ToString`, Rust `Display`, JavaScript `toString`), never the
   output function, which writes a spatial value's points as hexadecimal WKB
   (`mobilitydb/test/pose/sql/100_pose.sql`):

   ```sql
   SELECT asText(pose 'Pose(Point(1 1),0.5)');
   -- Pose(POINT(1 1),0.5)
   ```
3. **A value is held as its MEOS memory image and exchanged as its WKB.** An
   engine holding values in its own storage (Spark, Flink, DuckDB) holds each
   as its memory image, and a value leaving the engine travels as its WKB;
   the text forms are what a user reads and writes.
4. **Plain WKB without the SRID, the `E` forms with it,** in every binding as
   in PostgreSQL (section 3.2).
5. **The types a binding holds in its own language.** PostgreSQL's date and
   time types (`date`, `time`, `timetz`, `timestamp`, `timestamptz`,
   `interval`) are held as the language's own types, converted at the
   boundary through MEOS's readers and writers of those types; `numeric` and
   PostGIS's `box2d` and `box3d` have no class in a binding. The MEOS-API
   catalog marks the first `bindingScope: host` and the second
   `bindingScope: none`, and a binding's generator refuses to emit a class for
   a type so marked.

## 6. The numbers of a JSON value

`numeric` reaches MEOS only through a JSON number, and no public function of
MEOS returns one: a temporal or set JSON value converts the number of a key to
a `tbool`, `tint`, `tbigint` or `tfloat` (or to an `intset`, `bigintset` or
`floatset`) exactly as PostgreSQL casts a `numeric` to `boolean`, `integer`,
`bigint` or `double precision`, rounding half away from zero and refusing a
number out of range. A string holding a number converts as the number it holds
(`mobilitydb/test/json/sql/454_tjsonb_jsonfuncs.sql`):

```sql
SELECT tbigint(tjsonb '[{"a":1.7}@2001-01-01, {"a":-2.5}@2001-01-02, {"a":"2.5"}@2001-01-03]', text 'a');
-- [2@Mon Jan 01 00:00:00 2001 PST, -3@Tue Jan 02 00:00:00 2001 PST, 3@Wed Jan 03 00:00:00 2001 PST]
SELECT tbigint(tjsonb '[{"a":9007199254740993}@2001-01-01, {"a":"9007199254740995"}@2001-01-02, {"a":-9223372036854775808}@2001-01-03]', text 'a');
-- [9007199254740993@Mon Jan 01 00:00:00 2001 PST, 9007199254740995@Tue Jan 02 00:00:00 2001 PST, -9223372036854775808@Wed Jan 03 00:00:00 2001 PST]
SELECT tbigint(tjsonb '"{\"a\":9223372036854775808}"@2001-01-01', text 'a');
-- ERROR:  bigint out of range
SELECT tint(tjsonb '"{\"a\":\"12abc\"}"@2001-01-01', text 'a');
-- ERROR:  Invalid numeric string for key "a": 12abc
```

A `tbigint` keeps every digit of an integer beyond 2^53, which a conversion
through a double would lose. A key that is missing from an instant is told
apart from every value the result type holds, `DBL_MAX` among them.

## 7. Versioning and stability

A new type or format adds its reader and writer under the names of section 3
and raises the minor version. Changing a name, the default string or the
presence of the SRID in a form raises the major version.

Future:
- The Rust binding exposes the readers and writers of section 5.

## 8. Conformance

The input and output tests of MobilityDB are the oracle of every binding and
engine. `mobilitydb/test/io/sql/700_io_every_type_tbl.sql` reads a value of
each type's regression fixtures through `<type>From<Format>` and writes it
through `as<Format>` for every format the type supports, and writes the typed
literal through its default string. An implementation conforms when, for every
type and format it supports, `<type>From<Format>(as<Format>(v))` returns `v`
and every query answers exactly the text the expected output holds: the same
value, the same digits, the same WKB bytes.

## 9. Out of scope

- The byte layout of each WKB, which `meos-wkb-0.9.md` specifies.
- The casts between MEOS types and a language's own geometry types (shapely,
  JTS, a GeoJSON object), which `meos-bindings-algebra.md` specifies.
- The domain of the numbers a value holds, which `meos-finite-values.md`
  specifies.

## 10. References

- `meos-wkb-0.9.md`, `meos-bindings-algebra.md`, `meos-finite-values.md`.
- OGC Moving Features Encoding Extension: JSON (MF-JSON).
- PostGIS `ST_AsText`, `ST_AsEWKT`, `ST_AsBinary`, `ST_AsEWKB`.
- PostgreSQL `numeric` casts to `integer`, `bigint` and `double precision`.

## 11. Consumers

MobilityDB's SQL functions; the MEOS-API catalog, which names each reader and
writer per type and the SQL name it answers to; the bindings generated from
it in Python, Java, Go, C# and JavaScript; the Spark and Flink SQL surfaces;
MobilityDuck; and the OGC API – Moving Features server MobilityAPI, which
reads and writes the moving features of a collection as MF-JSON in any spatial
temporal type of MEOS.

## 12. References to the implementation

The SQL declarations in `mobilitydb/sql/` and the public headers in
`meos/include/` are authoritative; where this document and they differ, the
code is right and this document is corrected.
