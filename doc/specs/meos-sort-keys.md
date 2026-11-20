<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Sort Keys
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Sort Keys — Format Specification

| | |
|---|---|
| Version | v1.4.0 |
| Scope | The byte string MEOS computes from a value so that comparing two such strings byte by byte orders the values as MEOS does, for every type that has one; and how MEOS orders and equates geometries |
| Reference | `meos/src/temporal/type_util.c` (`datum_sortkey_*`), `meos/src/temporal/set.c`, `span.c`, `spanset.c`, `tbox.c`, `temporal.c`, `meos/src/geo/stbox.c`, `geo_funcs.c`, `meos/src/cbuffer/cbuffer.c`, `meos/src/npoint/`, `meos/src/pose/`, `meos/src/pointcloud/pcpoint.c`, `pcpatch.c`, `tpcbox.c`, `pgtypes/utils/jsonb_op.c` |

## 1. Scope and intent

A sort key is a string of bytes computed from a value so that:

- comparing the keys of two values byte by byte, as unsigned bytes, a key
  that is a prefix of a longer one coming first, gives the order of the
  type's comparison (`set_cmp`, `span_cmp`, `temporal_cmp`, ...); and
- two values have the same key exactly when the type's equality holds.

An engine that sorts, groups, deduplicates or joins values by comparing bytes
(the collation of MobilityDuck, MobilitySpark, MobilityFlink) then answers as
MobilityDB does without calling MEOS for each comparison. A key is computed for
such an operation and never stored: a value is stored and exchanged in its
binary or text form.

Because a key states a comparison and an equality as bytes, its encoding is
fixed by them, not chosen: section 4 follows each comparison field by field.
Section 7 states how MEOS orders and equates geometries, which have a key only
when they are points.

## 2. Notation

"Big endian" means the most significant byte first. "Sign bit flipped" means
the most significant bit of a two's-complement integer is inverted, so that
the unsigned byte order of the result is the signed order of the integer.
"Escaped" bytes are written one by one, a zero byte written as the two bytes
`00 FF`, and the string ended by `00 00`, which sorts below every byte that
continues a longer string, so that a shorter string sorts first and a key
followed by other keys still orders as the string does.

## 3. The public API

| Function | Value | Header |
|---|---|---|
| `set_sortkey(const Set *, size_t *size)` | a set | `meos.h` |
| `span_sortkey(const Span *, size_t *size)` | a span | `meos.h` |
| `spanset_sortkey(const SpanSet *, size_t *size)` | a span set | `meos.h` |
| `tbox_sortkey(const TBox *, size_t *size)` | a temporal box | `meos.h` |
| `stbox_sortkey(const STBox *, size_t *size)` | a spatiotemporal box | `meos_geo.h` |
| `tpcbox_sortkey(const TPCBox *, size_t *size)` | a point-cloud box | `meos_pointcloud.h` |
| `temporal_sortkey(const Temporal *, size_t *size)` | a temporal value | `meos.h` |

Each returns a newly allocated key and writes its length to `size`, or returns
NULL with an error when the value's type has no key. Each has an `_intl` twin
in the internal headers that asserts instead of testing. A base value has no
public key of its own: its key is the body of the key of a one-value set.

## 4. The encodings

### 4.1 Base values

`datum_sortkey_put` writes the key of a base value, `datum_sortkey_len` its
length, and `datum_sortkey_type` answers whether a base type has a key.

| Base type | Key |
|---|---|
| Boolean | one byte, 0 for false and 1 for true |
| integer, date | 4 bytes, big endian, sign bit flipped |
| big integer, timestamp, H3 cell, quadbin cell, S2 cell | 8 bytes, big endian, sign bit flipped |
| float | 8 bytes: -0.0 written as 0.0 and every NaN as one NaN; then the sign bit flipped for a positive value and every bit flipped for a negative one, big endian, so that NaN sorts above infinity as `pg_float8_cmp` orders it |
| text | its bytes, escaped |
| point (geometry or geography) | its coordinates, each as a float (`geopoint_sortkey_put`) |
| circular buffer | its SRID as an integer, then the x and y of its centre and its radius as floats (`cbuffer_sortkey_put`) |
| network point | its route as a big integer, then its position as a float (`npoint_sortkey_put`) |
| pose | whether it is spatial and whether it is geodetic, one Boolean byte each, its SRID as an integer, then its 3 values (x, y, angle) or 7 values (x, y, z, quaternion) as floats (`pose_sortkey_put`) |
| pose chain | its flags, SRID and poses (`posechain_sortkey_put`) |
| point-cloud point and patch | the pcid as 4 bytes big endian, then the bytes stored after it, escaped (`pcpoint_sortkey_put`, `pcpatch_sortkey_put`), so that values order by pcid as a number, then by their stored bytes as unsigned bytes |
| JSON value | its tokens in the order `compareJsonbContainers` reads them (`jsonb_sortkey_put`), see 4.2 |

The fixed-size keys have the length `datum_sortkey_size` gives; the others a
length that depends on the value.

### 4.2 JSON values

PostgreSQL's `compareJsonbContainers` reads two values token by token and stops
at the first that differs. The key writes each token so that it ends where its
bytes say it does, so the first differing byte falls in the first differing
token and decides as that token does:

- the type, as one byte in the order of the type enumeration: null, string,
  number, Boolean, array, object;
- for an array, its number of elements, 4 bytes big endian, then a byte that is
  0 for a scalar standing alone and 1 otherwise (a scalar standing alone is an
  array of one element, so `[]` sorts before `5`, and `5` before `[5]`);
- for an object, its number of pairs, 4 bytes big endian;
- for a string, its bytes, escaped (MEOS pins the C.UTF-8 collation, under
  which the string comparison is a byte comparison);
- for a number, read in the form `numeric_normalize` gives (in which two
  numbers are spelled alike exactly when they are equal, so `1.0` and `1.00`
  have one key): a byte stating its sign (1 negative, 2 zero, 3 positive), then
  for a non-zero number its decimal exponent e, the value being 0.d1d2... times
  10 to the e, as an integer, then each significant digit d as d + 1, then a
  zero byte; every byte after the sign is complemented for a negative number,
  its ending byte becoming 0xFF;
- for a Boolean, one byte;
- the end of an array or of an object writes nothing.

### 4.3 Sets, spans and span sets

- **Set**: one byte for the set type, then the key of each value in order.
- **Span**: one byte for the span type, then the lower bound, a byte that is 0
  when it is inclusive and 1 otherwise (an inclusive lower bound sorts first),
  the upper bound, and a byte that is 1 when it is inclusive and 0 otherwise (an
  exclusive upper bound sorts first).
- **Span set**: one byte for the span type, then the body of each span, as above
  without its type byte.

### 4.4 Boxes

- **Temporal box** (`tbox_sortkey_put`): its flags as an integer, its span type
  as a byte, then its period and its value span when it has them, each as a
  span body.
- **Spatiotemporal box** (`stbox_sortkey_put`): its SRID and flags as integers,
  its period when it has one, then the minima and the maxima of the coordinates
  it carries, as floats.
- **Point-cloud box** (`tpcbox_sortkey_put`): its pcid as a big integer, then
  the key of the spatiotemporal box it extends.

### 4.5 Temporal values

`temporal_sortkey_put` writes the fields in the order `temporal_cmp` compares
them:

1. the temporal type, one byte;
2. the bounding box, as 4.4 writes it, and for a temporal geography its period
   instead, since one point of the sphere spelled with two longitudes is one
   value with two boxes but one period;
3. each instant: its timestamp, then its value as 4.1 writes it;
4. each run of instants: its number of instants as an integer, whether it holds
   its lower and its upper bound (an inclusive bound first), and its
   interpolation.

The instants and the runs are read through the one-instant and one-sequence
wrappers, so that the values `temporal_eq` holds equal have one key. Neither
list needs an end mark: two values with one box have one period and therefore
reach their last instant together.

A temporal type has a key when its base type has one or when it is a temporal
point. The temporal rigid geometry has none: `temporal_cmp` reads none of its
reference geometry, so two values `temporal_eq` holds different can compare
equal.

## 5. Coverage

| Has a key | Has none |
|---|---|
| Booleans, integers, big integers, floats, dates, timestamps, texts; H3, quadbin and S2 cells; points; circular buffers; network points; poses and pose chains; point-cloud points and patches; JSON values; every set, span, span set and temporal type over them; temporal, spatiotemporal and point-cloud boxes | geometries and geographies that are not points, their sets and their temporal types (section 7); temporal rigid geometries |

## 6. The tests of a key

A MEOS test program per family compares the byte order of the keys with the
comparison over every pair of a corpus of sets and of temporal values, and the
comparison with the equality, antisymmetry and transitivity over every pair and
triple of base values: `meos/test/cbuffer_npoint_sortkey_test.c`,
`meos/test/pointcloud_sortkey_test.c` and `meos/test/json_sortkey_test.c` among
them. The fields that rest on exact arithmetic (floats, decimal numbers) are
also judged by an exact oracle that reads every field as a rational number and
requires the comparison and the byte order of the keys to equal the
lexicographic order of the fields on every row.

## 7. Geometry comparison

Sets, spans and temporal values of geometries and geographies sort, remove
duplicates and normalize through `datum_cmp` and `datum_eq` with the type
`T_GEOMETRY` or `T_GEOGRAPHY`.

### 7.1 The order: `datum_cmp`

1. **Two points of the same SRID and the same dimension** (both with or both
   without Z) take a fast path: `geopoint_cmp` compares their coordinates
   exactly, x, then y, then z.
2. **A point precedes any geometry that is not a point**, whatever their SRIDs,
   so that the order stays total on a value holding geometries of several types.
3. **Every other pair** goes to PostGIS's `gserialized_cmp`, which answers 0
   only when the bytes after the header, the SRID and the Z and M flags are all
   identical, and otherwise decides in this order:
   1. an empty geometry precedes a non-empty one;
   2. for two non-empty geometries, a 64-bit sortable hash of the bounding box
      (`gbox_get_sortable_hash`) decides;
   3. when the bytes of one are a prefix of the bytes of the other, the
      shorter precedes;
   4. otherwise the SRID decides, then the Z flag, then the M flag;
   5. finally the first differing byte decides.

### 7.2 The equality: `datum_eq`

1. **Two points** are equal when they have the same dimension and the same
   coordinates, compared exactly. A 2D and a 3D point are not equal.
2. **Any other pair** is equal when `geo_same` holds: the same values, vertex
   by vertex, compared exactly, as PostGIS answers `=`.

The point-set equality, under which a line and the same line drawn in the
other direction are equal, is `geo_equals`, not `datum_eq`.

### 7.3 What it means for a key

1. **Points have a key** (4.1). No other geometry has one, so a set of
   geometries and a temporal geometry have none.
2. **Across SRIDs, the order of 7.1 is not a dictionary order.** The prefix
   rule (7.1, 3.3) is read before the SRID (3.4) and the bytes (3.5) after it.
   Read from the rule, three geometries of one bounding box can be ordered in a
   cycle: A's bytes a prefix of B's, so A < B whatever their SRIDs; B and C of
   one SRID with B's bytes below C's, so B < C; C and A neither a prefix of the
   other, with C's SRID below A's, so C < A. No byte key encodes a cycle.
3. **This does not prevent a key for sets and temporal values.** They refuse
   operands of different SRIDs, so their keys compare geometries of one SRID
   only. Within one SRID and one dimension, rules 3.3 to 3.5 reduce to "the
   bytes in order, a prefix first", which a key can encode: a byte separating
   points from other geometries (points first); for a point, its coordinate key;
   otherwise a byte that is 0 for an empty geometry and 1 otherwise, the 8-byte
   bounding-box hash for a non-empty one, then the bytes, escaped.
4. **Zero of both signs.** A coordinate `-0.0` and a coordinate `0.0` are
   different bytes, so `gserialized_cmp` holds two such geometries different,
   while `geo_same`, comparing vertices with `==`, can hold them equal. Where
   both happen, `datum_cmp` answers non-zero on a pair `datum_eq` holds equal,
   and no key agrees with both. The exact answer is that they are one vertex:
   `-0.0` and `0.0` are one rational number, as the point fast path already
   reads them.

## 8. Out of scope

The point-set relationships (`geo_equals`, `geo_intersects`, ...); the binary
and text forms a value is stored and exchanged in, for which see
[meos-io.md](meos-io.md) and [meos-wkb-0.9.md](meos-wkb-0.9.md); the GiST and
SP-GiST sorted builds, which order by box rather than by value
(`doc/contributing/gist_sortsupport_design_notes.md`).

## 9. References to the implementation

The functions named in each section are authoritative. A correction to this
document cites the function it reads.
