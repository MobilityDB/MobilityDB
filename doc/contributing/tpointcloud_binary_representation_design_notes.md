<!--
  MobilityDB — Binary Representation of Temporal Point Cloud Values: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Binary representation and portability of `tpcpoint` and `tpcpatch`

A `tpcpoint` and a `tpcpatch` are written by `asBinary` / `asHexWKB` and read back by
`tpcpointFromBinary` / `tpcpatchFromBinary` and their HexWKB twins, and they travel through
PostgreSQL's `COPY ... (FORMAT BINARY)` on the same encoding. What follows is what that encoding
carries beyond the dimension payload, and what a receiver needs in order to read it.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation.

## The `pcid` is always carried, the schema only sometimes

The encoding carries the `pcid` of each value beside the dimension payload, so the blob is
unambiguous about which schema the values were written under. The schema itself is carried only
when the encoding backend has the schema XML for that `pcid` in its MEOS-level schema cache, which
the PostgreSQL extension populates lazily on first use by scanning `pointcloud_formats`. The
`MEOS_WKB_PCSCHEMAFLAG` bit (`0x80`) of the temporal-flags byte signals that the XML is present.

⇒ A receiver that does not yet know the `pcid` — a different cluster after a
`pg_dump --format=binary`, for instance — parses and registers the embedded schema itself, so the
import succeeds with no out-of-band schema preload.

## The fallback, and what it costs

When the encoding backend holds only a parsed `PCSCHEMA` and no source XML for a `pcid`, the schema
bit is left clear and the blob carries the `pcid` alone. Such a blob is portable only between
backends that share the same `pointcloud_formats` catalog. This is the behaviour of the earlier,
schema-less encoding, and it is the path taken when the cache was populated by hand from a
standalone MEOS program.

## What the receiver needs

Parsing an embedded schema needs `mobilitydb_init` to have run: the function-info module installs
the XML-parse hook there. Any backend that has loaded the `mobilitydb` extension has run it.
