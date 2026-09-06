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

The domains are libpc's own. The interpretation names are the ten of `pc_schema.c:21-22`. `unknown`, the eleventh, sits **outside** the domain: it is what the parser answers for a value it does not recognise, not a storage a schema states. The compression names are those of `pc_compression_name` (`pc_schema.c:84-93`).

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

⭐ **THE ACCEPTANCE TEST IS `meos/test/pcschema_dims_load_test.c`**: it states the §19.1.1 schema as rows
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
## Why this reaches the bindings: one fact, two front ends

A binding outside a PostgreSQL backend reaches no catalog at all, so it cannot resolve a `pcid` and
cannot construct any value whose bounding box needs the schema. The entry therefore belongs in MEOS
rather than in any one binding — but the entry alone does not settle *how a user states a schema*,
and that answer differs by host, because the hosts differ in what a user can write.

⭐ **A HOST STATES THE CATALOG IN ITS OWN IDIOM, AND MEOS HOLDS THE ONE CACHE THEY ALL RESOLVE
THROUGH.** Two front ends, one mechanism:

| host | states a schema as | reaches MEOS through |
|---|---|---|
| PostgreSQL, MobilityDuck, MobilitySpark | rows of the two tables | `meos_pc_schema_register_dims` |
| standalone MEOS, JMEOS, GoMEOS, PyMEOS | a vendored pgPointCloud document | the document loader |

The tables exist **because a SQL programmer cannot reasonably be asked to paste an XML document into
a query**, and that is their whole scope: the ergonomics of a SQL host. It is not an argument that
XML is wrong for a host with no SQL, where there is no table to write into and a file is the only
thing a user has.

⛔ **THE ENCODING OF THE VENDORED FILE IS ACCIDENTAL.** What matters is that MEOS ships a catalog for
hosts that have no catalog of their own, which a `meos_set_…` entry names. XML is chosen for point
cloud schemas only because `pc_schema_from_xml` already reads it, so the payload costs no parser and
stays the vendor's format rather than one of ours.

⭐ **THIS IS THE ESTABLISHED PATTERN, NOT A NEW ONE.** Two catalogs in this tree already have exactly
these two front ends, which is the strongest argument that a third should not invent a third shape:

| catalog | SQL host | non-SQL host |
|---|---|---|
| spatial reference systems | PostGIS `spatial_ref_sys` | `meos/src/geo/spatial_ref_sys.csv`, `meos_set_spatial_ref_sys_csv` |
| road network (`npoint`) | `public.ways` | `meos/examples/data/ways1000.csv`, `meos_set_ways_csv` |
| point cloud schemas | `pointcloud_schemas` / `pointcloud_dimensions` | a vendored document |

`ways_meos.c:58` states the second row in the code itself — the entry is described there as
`meos_set_ways_csv()`, *"mirroring `meos_set_spatial_ref_sys_csv()`"*, each holding a default path
(`/usr/local/share/ways1000.csv`, `/usr/local/share/spatial_ref_sys.csv`) that the entry overrides.

⛔ **THE TWO FRONT ENDS DESYNCHRONISE, AND THAT IS ACCEPTED RATHER THAN REPAIRED.** A vendored
`spatial_ref_sys.csv` is a snapshot; the PostGIS table is whatever the database holds. Nothing
reconciles them, and the same holds here. ⇒ A trigger keeping one front end in step with the other
is NOT the answer: it would exist in PostgreSQL alone, where the other hosts have nothing to
synchronise, which is a host-specific special case rather than a shared mechanism.

⚠️⛔ **WHERE THE ANALOGY WITH `spatial_ref_sys.csv` STOPS, AND IT DECIDES THE DEFAULT.** The EPSG
registry is universal reference data — 4326 means the same thing to everybody — whereas a `pcid`
names one user's own instrument. So the two rows above differ in one respect the table cannot show:
`meos_set_spatial_ref_sys_csv` and `meos_set_ways_csv` each OVERRIDE a default the loader reads
without being asked, while **nothing reads the schema document unless a host names it**. A bundled
schema answering `pcid` 1 by default would decode a value of somebody else's `pcid` 1 into silently
wrong coordinates instead of reporting that none is registered — and a wrong schema is invisible in
the text form, since the same bytes are printed whichever schema decodes them. The vendored document
is therefore a starting point to copy, not a registry, and naming a file is the only route rather
than the primary one.

⛔ **THE DOCUMENT CARRIES NEITHER `pcid` NOR `srid`.** `pc_schema_from_xml` reads neither; both are
columns of `pointcloud_formats`, which is precisely why a table exists on the PostgreSQL side at all.
A file holding several schemas therefore needs an envelope carrying them, and libpc defines none:
its own fixtures (`pointcloud-pg/lib/cunit/data/*.xml`) put the reference system in
`<Metadata name="spatialreference" type="id">`, so only the identifier is genuinely without a home.

## Why the frame registry is seeded and this one is not

`geopose_frames` looks like the same shape and is filled the opposite way: `120_geopose_frames.in.sql`
runs `INSERT INTO geopose_frames … SELECT … FROM geoPoseFrames()`, seeding the table from a static C
registry in `meos/src/pose/pose_geopose.c`. Nothing seeds `pointcloud_schemas`.

⭐ **THE ASYMMETRY FOLLOWS FROM THE DATA AND IS NOT A DEFECT.** GeoPose frames are a **closed set
fixed by a specification**, so MEOS can hold all of them at compile time and the table is a
projection. A point cloud schema describes a user's own instrument, so MEOS can never know it, and
there is nothing to seed. ⇒ a static registry is right for a closed catalog and wrong for an open
one; reading the two tables as "the same thing done inconsistently" inverts the design.

## A sibling worth tracking

`npoint` faces the neighbouring problem: an `rid` identifies a route inside one pgRouting graph, so
its meaning is local to the network that built it, and the `ways` registry that resolves it is
gigabytes rather than a few hundred bytes. Discussion #863 works through it and reasons from both
analogies used here — the `pcid` catalog for *interpreting* bytes, and the SRID registry for
*identity that is stable everywhere*. The two front ends above are the part that transfers; what a
catalog entry should contain is the part that does not.
