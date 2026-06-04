<!--
Copyright(c) MobilityDB Contributors

This documentation is licensed under a
Creative Commons Attribution-Share Alike 3.0 License
https://creativecommons.org/licenses/by-sa/3.0/
-->

# SQL–DocBook Alignment Status

Run `python3 tools/check_sql_doc_alignment.py` from the repo root to regenerate.

**Current state (2026-05-06):** 0 errors, 154 warnings.

## Warning breakdown

| Check | Count | Description |
|-------|------:|-------------|
| C2 | 0 | C wrappers with no SQL caller (orphans) |
| C3 | 5 | C wrappers missing `@ingroup` tag |
| C4 | 8 | C wrappers missing `@sqlfn` tag |
| C5 | 0 | `@ingroup` references an undeclared `@defgroup` |
| C6 | 0 | Public SQL functions absent from DocBook index |
| C7 | 69 | STRICT functions with dead `PG_ARGISNULL` guard |
| C1 | 0 | SQL entries pointing to a missing C wrapper |

C3/C4 affect Doxygen HTML output only (no runtime impact); addressed per-PR.
C7 is a codebase-wide STRICT-guard pattern (~73 occurrences); deferred to a
dedicated cleanup sweep rather than fixing per-type.

## C6 resolution (2026-05-06)

All 356 C6 gaps were closed in one pass by adding concrete SQL function-name
`<indexterm>` entries alongside the existing operator-symbol entries in each
chapter's relevant sections. The pattern: operators like `<<` are implemented
by functions like `stbox_left`; the chapters indexed the operator symbol but not
the function name — now both are indexed.

Chapters updated: `box_types.xml` (+56), `set_span_types.xml` (+98),
`temporal_types_p1.xml` (+36), `temporal_types_p2.xml` (+59),
`temporal_spatial_p1.xml` (+48), `temporal_network_points.xml` (+31),
`temporal_circular_buffers.xml` (+16), `temporal_poses.xml` (+16).

The helper script `tools/close_c6_gaps.py` can be re-run after adding new SQL
functions to keep the index current.

## PostGIS refentry migration (ongoing)

The preferred long-term approach is to migrate each chapter from the legacy
`<itemizedlist>/<indexterm>` pattern to PostGIS-style `<refentry>` blocks.
Standard DocBook `chunk.xsl` auto-generates index entries from `<refname>`
elements inside `<refentry>` — no manual `<indexterm>` maintenance needed.

The trgeometry chapter (`doc/temporal_rigid_geometries.xml`) is the pilot.

## Building the documentation

```bash
cmake --build build_release --target doc_html   # HTML
cmake --build build_release --target doc_pdf    # PDF
```
