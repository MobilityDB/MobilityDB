<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# Arrow interchange instantiation: the optional columnar layer of the Temporal Data Lake

## What this instantiation demonstrates

The Temporal Data Lake architecture describes three layers: the MEOS-WKB
byte-format specification, the TemporalParquet substrate (an opaque MEOS-WKB
`BYTE_ARRAY` column plus a self-describing footer), and an optional columnar
Arrow interchange. This document covers the **optional Arrow interchange layer
only**. The optional Arrow encoding is an explicit, closed type algebra that a
process holding no MEOS library consumes bit-exactly through commodity
[Apache Arrow](https://arrow.apache.org/) and [Parquet](https://parquet.apache.org/) tooling. The
document gives the exact recipe a reader runs locally to reproduce that result.

It is a correctness and consumability artifact, not a performance benchmark.
There are no timing numbers here. Every quantitative statement below is the
value produced by the named reproduction artifact, not an estimate.

## The claim this instantiates

The optional Arrow layer's claim is that a MEOS value can be exported through
the [Arrow C Data Interface](https://arrow.apache.org/docs/format/CDataInterface.html) into a vectorized columnar form that any Arrow or
Parquet engine reads without a MobilityDB or MEOS installation, losslessly, and
that this holds for the whole MEOS type algebra rather than a temporal subset.
The MEOS type algebra is closed: every argument and result of a MEOS function
is itself a MEOS type. The temporal types are functions from a time domain to
a base domain; set, span, and spanset are the finite-subset types of a single
infinite domain; tbox and stbox are the finite-subset types of a product
domain. A faithful interchange therefore has to make all of these explicit, not
only the temporal types.

## The closed algebra realized

The optional Arrow encoding covers the closed algebra in three explicit
treatments, each its own focused conversion plus an external-consumer
conformance proof:

| Algebra family | What it is | Arrow conversion | External-consumer proof |
|---|---|---|---|
| Temporal types | functions time to base domain | `meos_temporal_to_arrow` / `meos_temporal_from_arrow`, full base-type surface | [nanoarrow](https://github.com/apache/arrow-nanoarrow) FULL-validate per type; bit-exact zero-MEOS round-trip |
| Set, span, spanset | finite subsets of one infinite domain | one shared span Struct used by span-as-value | nanoarrow FULL-validate; zero-MEOS round-trip |
| tbox, stbox | finite subsets of a product domain | the same span Struct composed for the product domain | nanoarrow FULL-validate; zero-MEOS round-trip |

The conversion is the Arrow C Data Interface (the two-struct `ArrowSchema` /
`ArrowArray` ABI with release callbacks); MEOS links no Arrow library. The
Arrow-to-Parquet write is the consumer's, performed entirely by commodity
Apache tooling.

The temporal value leaf is decomposed per base type where the type has a
columnar structure (scalars to their primitive Arrow leaf; points and the
decomposed extended types to a parallel-coordinate Struct) and carried as an
opaque verbatim binary leaf where it does not (general geometry and geography,
the point-cloud types). The raw MEOS flags word is carried verbatim, so the
geodetic and dimensionality discriminators are recovered without a separate
field. Coverage is stated along two axes: every MEOS algebra type, and every
distinct Arrow value-leaf encoding. There is no fraction of a fixed type count
here; the type-count axis belongs to the separate consumer surface described
under Scope.

## Scope

This instantiates the optional Arrow encoding. It does not claim, re-derive, or
absorb the data-lake substrate.

The data-lake substrate is the opaque MEOS-WKB `BYTE_ARRAY` column plus the
self-describing TemporalParquet footer, with native-scalar sidecar columns the
consumer writer emits for row-group pruning. That substrate is exercised in the
[MobilityDuck](https://github.com/MobilityDB/MobilityDuck) consumer lane (the
`temporalFooter()` footer convention and the native-scalar sidecar quickstart
recipe). This document cross-references that lane; it does not prove it here.

MEOS-WKB is the zero-copy base: in [MobilityDB](https://github.com/MobilityDB/MobilityDB) the on-disk [varlena](https://www.postgresql.org/docs/current/storage-toast.html) is
the in-memory image, so MEOS-WKB is the portable zero-copy form. The optional
Arrow layer is strictly additive and does not change it.

Unifying the typed box with the sidecar pruning columns (a typed box that is
also projected as flat pruning columns) is a separate direction outside this
instantiation.

## Section 1: closed-algebra conformance

This section licenses the statement that the exported Arrow form is valid
against the Arrow C Data Interface specification, judged by an external oracle,
for every type in the closed algebra.

The conformance oracle is upstream-bundled nanoarrow run in FULL-validate mode
(`ArrowArrayViewInitFromSchema`, `SetArray`, `ArrowArrayViewValidate` at the
FULL level). It is the canonical external Arrow C Data Interface validator, not
MEOS reading its own output; a same-process MEOS reader masks export defects
that an independent consumer dereferences.

Each algebra family is validated on its own evidence: the temporal base-type
surface, the set, span, and spanset types, and the tbox and stbox types. The
shared nested-schema path emits schema children with stable storage, so the
oracle validates every per-type export at the FULL level. The set, span, and
spanset types and the tbox and stbox types reuse one shared span Struct
definition, so span-as-value and span-as-box cannot diverge into two
incompatible shapes.

## Section 2: zero-MEOS end-to-end consumption

This section licenses the statement that a process holding no MEOS library
reads the exported data bit-exactly through commodity Arrow and Parquet
tooling.

A MEOS-linked producer builds values across the full temporal base-type
surface and exports each through `meos_temporal_to_arrow`. A bridge that links
no MEOS imports every array through [`pyarrow.Array._import_from_c`](https://arrow.apache.org/docs/python/)
with no error and writes real multi-row-group Parquet with
per-row-group statistics. A fully separate process, proven to map no libmeos or
producer object by inspecting its own process memory map, reads the Parquet
back with pyarrow and [duckdb](https://duckdb.org/) and compares every value field-exact against the
MEOS-emitted decomposed ground truth ([IEEE-754](https://en.wikipedia.org/wiki/IEEE_754) exact for floats, exact for
integers, byte-exact for the opaque binary leaves).

All 13600 produced rows match the MEOS ground truth field-exact across the full
base-type surface, with no MEOS code in the consuming process.

## Reading the measured characteristics

The interchange is bit-exact and lossless: the round-trip comparison above is
value-identical, the raw MEOS flags word is carried verbatim, and the opaque
binary leaves are byte-identical.

Row-group pruning is reported honestly and is what the artifact measures, not
what is hoped for. The Parquet writer records per-row-group min/max statistics
both for the flat top-level columns and for the nested sequence and instant
leaves. A real zero-MEOS engine prunes row groups only on the flat top-level
columns: a predicate on the flat `srid` column skips 3 of 7 row groups for the
types whose value is a decomposed Struct, confirmed by the pyarrow dataset
filter and by duckdb independently. For scalar and opaque-leaf types the value
lives inside the nested sequence column and there is no flat block to prune on,
so there is no row-group skipping for those types. Neither engine turns a
predicate on a value inside a nested list or struct into row-group skipping;
pyarrow cannot express a list-column filter and duckdb scans every group. This
nested-not-prunable result is stated, not hidden; it is the substantive Parquet
finding and the reason the parallel lane emits flat native-scalar sidecar
columns for pruning.

The Parquet files are written by the zero-MEOS consumer with Snappy
compression and per-row-group statistics. No compression-ratio figure is
claimed because the reproduction artifact does not measure one.

## Reproduce

A reader reproduces the entire instantiation locally by running one script from
a MobilityDB checkout:

```
meos/examples/temporal_arrow_parquet_demo.sh [BUILD_PREFIX]
```

The script builds the vendored [pgPointCloud](https://github.com/pgpointcloud/pointcloud) static library, builds MEOS over
the full base-type surface into a private prefix (the system prefix is never
written), builds the MEOS-linked Arrow producer, runs the zero-MEOS bridge that
writes per-type multi-row-group Parquet through
`pyarrow.Array._import_from_c`, and runs the fully separate zero-MEOS consumer
that verifies every row field-exact and measures the per-type row-group
pruning. Requirements are commodity: a C toolchain, [cmake](https://cmake.org/), the MEOS build
dependencies, [libh3](https://h3geo.org/), the autotools and [libxml2](http://xmlsoft.org/) development packages, any
[PostgreSQL](https://www.postgresql.org/) `pg_config`, and Python with pyarrow and duckdb (a virtual
environment is created if the system Python is externally managed).

The same two jobs run in continuous integration:

- `Arrow C Data Interface conformance (opaque and rigid geometry types)` runs
  the nanoarrow FULL-validate oracle.
- `Zero-MEOS Arrow and Parquet consumption end to end` runs the producer,
  bridge, and separate-process consumer described in Section 2.

The conversion kernels and conformance proofs this instantiation relies on are
the vendored Arrow C Data Interface ABI, the temporal full-surface conversion
across the base-type surface (tfloat, tint, tbool, ttext, tgeompoint /
tgeogpoint, tpose) and the end-to-end zero-MEOS demonstration, the set, span,
and spanset conversion, and the tbox and stbox conversion, together with the
per-type external-consumer validators (the shared schema-children arena, the
opaque and rigid geometry conformance, and the H3 index validator). The
data-lake substrate and its native-scalar sidecar pruning live in the
MobilityDuck consumer lane and are cross-referenced, not reproduced here.
