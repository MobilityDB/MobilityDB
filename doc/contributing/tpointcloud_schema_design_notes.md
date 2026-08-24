<!--
  MobilityDB — Declaring a Point Cloud Schema in SQL: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Declaring a Point Cloud Schema in SQL — Design Notes

A `pcpoint` and a `pcpatch` carry a `pcid` and nothing else about their own layout: what dimensions they hold, how each is stored, and what a stored integer means as a coordinate all live in a schema resolved by that `pcid`. Today the only way to state a schema is a pgPointCloud XML document, and the only constructor that reads one is `pc_schema_from_xml`. These notes record a SQL-native way to state the same thing, why the columns are the ones below, and what the change costs.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation.

---

## What a schema states, and what it derives

`PCDIMENSION` (`pointcloud-pg/lib/pc_api.h:64-75`) carries nine fields and `PCSCHEMA` (`:77-90`) eleven. Only some are stated; the rest are computed. `pc_schema.c:180-190` is explicit:

```c
pcs->dims[i]->byteoffset = byteoffset;
pcs->dims[i]->size = pc_interpretation_size(pcs->dims[i]->interpretation);
byteoffset += pcs->dims[i]->size;
```

⇒ `size` follows from `interpretation`, `byteoffset` from the sizes of the dimensions before it, and `pcs->size` from their total. `ndims` is a count, and `xdim` / `ydim` / `zdim` / `mdim` are resolved from the names by `pc_schema_check_xyzm`. **A user who typed any of those could only contradict the engine**, so none of them is a column.

What remains is what a schema genuinely states:

| stated per schema | stated per dimension |
|---|---|
| `pcid`, `srid`, `compression`, `description` | `position`, `name`, `interpretation`, `scale`, `offset`, `active`, `description` |

## The tables

```sql
CREATE TABLE pointcloud_schemas (
  pcid         integer PRIMARY KEY CHECK (pcid > 0),
  srid         integer NOT NULL,
  compression  text    NOT NULL DEFAULT 'none'
                       CHECK (compression IN ('none','dimensional','laz')),
  description  text
);

CREATE TABLE pointcloud_dimensions (
  pcid            integer NOT NULL REFERENCES pointcloud_schemas(pcid) ON DELETE CASCADE,
  position        integer NOT NULL CHECK (position >= 1),
  name            text    NOT NULL,
  interpretation  text    NOT NULL
                          CHECK (interpretation IN ('int8_t','uint8_t','int16_t','uint16_t',
                                                    'int32_t','uint32_t','int64_t','uint64_t',
                                                    'double','float')),
  scale           double precision NOT NULL DEFAULT 1,
  "offset"        double precision NOT NULL DEFAULT 0,
  active          boolean NOT NULL DEFAULT true,
  description     text,
  PRIMARY KEY (pcid, position),
  UNIQUE (pcid, name)
);
```

**`position` is a column and not an insertion order.** A schema whose dimension order came from the order rows happened to be inserted would depend on something SQL does not promise, and a reload, a `COPY`, or a parallel insert would silently reorder the layout. `PRIMARY KEY (pcid, position)` states the order, and makes a duplicate or a gap a constraint violation rather than a wrong byte offset.

**`UNIQUE (pcid, name)`** is what makes `pc_schema_check_xyzm` unambiguous: the X, Y, Z and M dimensions are found by name, so two dimensions sharing one name would leave the choice to array order.

The domains are libpc's own. The interpretation names are the ten of `pc_schema.c:21-22`. `unknown`, the eleventh, sits **outside** the domain: it is what the parser answers for a value it does not recognise, not a storage a schema states. The compression names are those of `pc_compression_string` (`pc_schema.c:84-93`).

## The example of the manual, stated in SQL

`doc/temporal_pointcloud.xml` §19.1.1 registers a minimal 3D schema. The same schema, with every column of both tables populated:

```sql
INSERT INTO pointcloud_schemas (pcid, srid, compression, description) VALUES
  (1, 4326, 'none', 'Minimal 3D schema, unit-scale int32 dimensions');

INSERT INTO pointcloud_dimensions
    (pcid, position, name, interpretation, scale, "offset", active, description) VALUES
  (1, 1, 'X', 'int32_t', 1, 0, true, 'Easting'),
  (1, 2, 'Y', 'int32_t', 1, 0, true, 'Northing'),
  (1, 3, 'Z', 'int32_t', 1, 0, true, 'Elevation');
```

The sizes (4 bytes each) and the byte offsets (0, 4, 8) appear nowhere, being what the engine computes from `int32_t`.

A schema whose dimensions differ from one another, to show the form is not limited to the uniform case:

```sql
INSERT INTO pointcloud_schemas (pcid, srid, compression, description) VALUES
  (2, 4326, 'dimensional', 'Scanner track: scaled coordinates plus intensity');

INSERT INTO pointcloud_dimensions
    (pcid, position, name, interpretation, scale, "offset", active, description) VALUES
  (2, 1, 'X',         'int32_t',  0.01, 0, true,  'Easting, cm precision'),
  (2, 2, 'Y',         'int32_t',  0.01, 0, true,  'Northing, cm precision'),
  (2, 3, 'Z',         'int32_t',  0.01, 0, true,  'Elevation, cm precision'),
  (2, 4, 'Intensity', 'uint16_t', 1,    0, true,  'Return intensity'),
  (2, 5, 'Reserved',  'uint8_t',  1,    0, false, 'Declared, not populated');
```

## What MEOS gains

One entry, which every binding then reaches through its own catalog:

```c
extern bool meos_pc_schema_register_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims);
```

where `PCDimensionSpec` names exactly the seven stated fields of a dimension. The entry allocates the `PCSCHEMA`, fills each dimension, and calls `pc_schema_check_xyzm`, leaving `size`, `byteoffset` and `ndims` to the engine.

⭐ **THE SCHEMA IS BUILT THROUGH libpc'S OWN CONSTRUCTOR, so the entry couples to no layout.**
`pc_api.h` publishes `pc_schema_from_xml`, `pc_schema_clone`, `pc_schema_set_dimension`,
`pc_schema_check_xyzm`, `pc_schema_free` and the accessors, while the three pieces a non-XML
statement also needs carry internal linkage in `pc_schema.c`: `pc_schema_new`, which allocates the
schema, its dimension array and its name hash table and is what `pc_schema_from_xml` and
`pc_schema_clone` themselves build every schema through, and the `pc_interpretation_number` /
`pc_compression_number` name converters. Giving those three external linkage — a `/* MEOS */`-marked
change to the vendored tree — is what makes the entry a caller of libpc rather than a second
implementation of it. ⇒ the fields the engine derives are never assigned here:
`pc_schema_set_dimension` inserts the name in the hash table AND recomputes every `size`,
`byteoffset` and the width of a point, and `pc_schema_check_xyzm` resolves x/y/z/m from the names.

⭐ **THE ACCEPTANCE TEST IS `meos/test/pcschema_dims_test.c`**: it states the §19.1.1 schema as rows
and parses the same document with `pc_schema_from_xml`, then requires the two to agree field by
field, the derived fields included, and the name hash to answer. It keeps the two paths honest as
libpc moves.

⛔ **`active` IS THE ONE FIELD WHERE A DOCUMENT AND A ROW DISAGREE, and the divergence is
deliberate.** `pc_schema.c` assigns `d->active` only where the document carries `<pc:active>`, so a
document omitting it — §19.1.1 included — yields **0**, a schema whose every dimension is declared
inactive. `active boolean NOT NULL DEFAULT true` is the answer a user means, so a row and a document
that both STATE the field agree, while a row and a document that omits it do not. A comparison of
the two paths therefore states `active` on both sides, or it compares a statement against an absence.

## What does not change

- **`pointcloud_formats` is pgPointCloud's**, created by that extension and read by `schema_cache.c:107` through a heap scan that errors when the relation is absent. Nothing here modifies it, and a database already registering schemas that way keeps working.
- **The XML path stays.** A WKB blob may carry an embedded schema document for a `pcid` the backend has not seen, which `meos_pc_parse_xml_fn` handles; data written by other pgPointCloud tools stays readable.
- ⇒ the two are alternative ways to state the same schema, and the SQL one is what a binding without a PostgreSQL catalog can offer.
- **A file carrying the same fact is refused.** A CSV or XML file read from a path resembles `meos_set_ways_csv` and `meos_set_spatial_ref_sys_csv` closely enough to look canonical, which is what makes it a trap: the two tables already state the schema, so a file-based lookup is a second mechanism for one fact, and two mechanisms for one fact is the defect. It is also the larger build — an embedded blob, a temporary file and an RFC 4180 parser against a single registration call.

## Why this reaches the bindings

A binding outside a PostgreSQL backend — MobilityDuck, MobilitySpark — reaches no catalog at all, so it cannot resolve a `pcid` and cannot construct any value whose bounding box needs the schema. Both are blocked on the same thing, so the entry belongs in MEOS rather than in either of them: each binding then materialises these two tables in its own host and calls the one entry.
