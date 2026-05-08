<!--
Copyright(c) MobilityDB Contributors

This documentation is provided under Creative Commons Attribution 4.0
International License (CC BY 4.0): https://creativecommons.org/licenses/by/4.0/
-->

# RFC: Edge-to-Cloud SQL Portability for the MobilityDB Ecosystem

> **Discussion [#861](https://github.com/MobilityDB/MobilityDB/discussions/861)** — community discussion and sign-off

## The Problem

The MobilityDB ecosystem now spans three execution platforms:

| Platform | Engine | Implementation |
|---|---|---|
| **[MobilityDB](https://github.com/MobilityDB/MobilityDB)** | PostgreSQL | Native C + PostGIS |
| **[MobilityDuck](https://github.com/MobilityDB/MobilityDuck)** | DuckDB | C++ extension (MEOS-backed) |
| **[MobilitySpark](https://github.com/MobilityDB/MobilitySpark)** | Apache Spark | JVM UDFs via [JMEOS 1.3](https://github.com/MobilityDB/JMEOS/pull/9) |

Each platform understands the same temporal and spatial types — `tgeompoint`, `tgeometry`,
`tstzspan`, `tbox`, `stbox` — because all three are backed by the same
[MEOS](https://github.com/MobilityDB/MEOS-API) C library (either directly or through JMEOS).

Yet today **you cannot take the same SQL query and run it unchanged on all three platforms**,
because MobilityDB's richest interface uses PostgreSQL custom operators (`&&`, `@>`, `<<#`,
`?=`, `<->`, …) that neither DuckDB nor Spark SQL can register.

## Why Now

Three things are now true at once:

- **[MobilityDuck](https://github.com/MobilityDB/MobilityDuck)** has reached **100% parity**
  with MobilityDB within the temporal and geo scope (35 test files, 1186 assertions, all passing)
  and exposes every MobilityDB operator as a named function (`before()`,
  `eIntersects()`, `nearestApproachDistance()`, …).
- **[JMEOS 1.3](https://github.com/MobilityDB/JMEOS/pull/9)** is complete: code-generated
  from [`meos-api.json`](https://github.com/MobilityDB/MEOS-API), full Java 21, ready to
  back MobilitySpark UDFs.
- **[MobilityPySpark](https://github.com/MobilityDB/MobilityPySpark)** has BerlinMOD Q1–Q12
  running end-to-end on PyMEOS, proving the query pattern works on Spark SQL.

All three platforms already support a **named-function form** for every MobilityDB operation.
The gap is not new engineering — it is agreement on canonical function names and one BerlinMOD
benchmark run to prove it.

## Proposal

A portable query uses **only named functions, no operator symbols**.  MobilityDB already
registers named aliases for every operator.  MobilityDuck uses the same names.  MobilitySpark
needs UDFs with matching names.

### Operator → Function mapping

The table below maps each MobilityDB operator to the corresponding portable named function.

| Category | MobilityDB operator | Portable function |
|---|---|---|
| **Time position** | `<<#` | `before(a, b)` |
| | `#>>` | `after(a, b)` |
| | `&<#` | `overbefore(a, b)` |
| | `#&>` | `overafter(a, b)` |
| **Space — X axis** | `<<` | `left(a, b)` |
| | `>>` | `right(a, b)` |
| | `&<` | `overleft(a, b)` |
| | `&>` | `overright(a, b)` |
| **Space — Y axis** | `<<`&#124; | `below(a, b)` |
| | &#124;`>>` | `above(a, b)` |
| | `&<`&#124; | `overbelow(a, b)` |
| | &#124;`&>` | `overabove(a, b)` |
| **Space — Z axis** _(3D types only)_ | `<</` | `front(a, b)` |
| | `/>>` | `back(a, b)` |
| | `&</` | `overfront(a, b)` |
| | `/&>` | `overback(a, b)` |
| **Ever** | `?=` | `ever_eq(v, t)` |
| | `?<>` | `ever_ne(v, t)` |
| | `?<` | `ever_lt(v, t)` |
| | `?<=` | `ever_le(v, t)` |
| | `?>` | `ever_gt(v, t)` |
| | `?>=` | `ever_ge(v, t)` |
| **Always** | `%=` | `always_eq(v, t)` |
| | `%<>` | `always_ne(v, t)` |
| | `%<` | `always_lt(v, t)` |
| | `%<=` | `always_le(v, t)` |
| | `%>` | `always_gt(v, t)` |
| | `%>=` | `always_ge(v, t)` |
| **Topology** | `&&` | `overlaps(a, b)` |
| | `@>` | `contains(a, b)` |
| | `<@` | `contained(a, b)` |
| | `-`&#124;`-` | `adjacent(a, b)` |
| **Distance** | `<->` | `tdistance(a, b)` |
| | &#124;`=`&#124; | `nearestApproachDistance(a, b)` |

Spatial relationship functions (`eIntersects`, `aIntersects`, `tIntersects`, `eTouches`,
`eContains`, `eDwithin`, …) and restriction functions (`atTime`, `atGeometry`, `atValue`, …)
already use named-function syntax in MobilityDB and require no mapping — write them the same
way on all three platforms.

### Serialization interchange

Objects must round-trip between platforms unchanged.  The canonical interchange format is
**MF-JSON** (OGC Moving Features JSON), already supported by `asMFJSON()` / `fromMFJSON()`
in MobilityDB and MEOS.  For bulk transfer, MEOS binary (hex-encoded WKB extension) is the
efficient alternative.

> **Important discrepancy to resolve:** [MobilityPySpark](https://github.com/MobilityDB/MobilityPySpark)
> currently uses `-1.0` as the `nearestApproachDistance` failure sentinel. MEOS and MobilityDB
> use `DBL_MAX` ([PR #846](https://github.com/MobilityDB/MobilityDB/pull/846)).
> The canonical value is `NULL` at the SQL level; the dialect must hide the C sentinel.

## BerlinMOD Reference Queries (Portable)

The [`berlinmod/`](berlinmod/) subdirectory contains the first five BerlinMOD benchmark
queries written in the portable dialect.  These are the POC acceptance criteria: **the same
`.sql` file must produce the same result on all three platforms without modification**.

| Query | Temporal operations used | File |
|---|---|---|
| Q1 | None (relational join) | [berlinmod/q01.sql](berlinmod/q01.sql) |
| Q3 | `atTime()` | [berlinmod/q03.sql](berlinmod/q03.sql) |
| Q4 | `eIntersects()` | [berlinmod/q04.sql](berlinmod/q04.sql) |
| Q5 | `nearestApproachDistance()` + `MIN()` | [berlinmod/q05.sql](berlinmod/q05.sql) |
| Q6 | `eDwithin()` | [berlinmod/q06.sql](berlinmod/q06.sql) |

## Six-Phase Implementation Plan

### Phase 1 — [JMEOS 1.3](https://github.com/MobilityDB/JMEOS/pull/9) integration into [MobilitySpark](https://github.com/MobilityDB/MobilitySpark)
Wire JMEOS 1.3 (PR [MobilityDB/JMEOS#9](https://github.com/MobilityDB/JMEOS/pull/9)) into
MobilitySpark, replacing the placeholder 1-UDF POC.  Register `tgeompoint`, `tgeometry`,
`tint`, `tfloat`, `ttext`, `tbool` as Spark SQL types backed by MEOS binary representation.

### Phase 2 — Canonical function name registry
Publish a machine-readable table ([`meos-api.json`](https://github.com/MobilityDB/MEOS-API)
extension or a companion `portable-sql.json`) that maps each MEOS function to its canonical
SQL name.  Each platform validates against this registry rather than maintaining its own
ad-hoc mapping.

### Phase 3 — [MobilitySpark](https://github.com/MobilityDB/MobilitySpark) function surface (generated)
Use the `NewFunctionsGenerator` already in
[JMEOS](https://github.com/MobilityDB/JMEOS/pull/9) to produce Spark UDF registrations for
all MEOS functions, analogous to what `meos-api.json` drives for JMEOS itself.
Target: BerlinMOD Q1–Q12 all passing.

### Phase 4 — Serialization alignment
Replace [MobilityPySpark](https://github.com/MobilityDB/MobilityPySpark)'s WKT string
interchange with MEOS binary hex.  Establish a shared test fixture (small BerlinMOD dataset
in MF-JSON) runnable on all three platforms from the same file.

### Phase 5 — BerlinMOD Q1–Q20 parity suite
Complete Q13–Q20 on all three platforms.  Surface any remaining name or semantic mismatches.
All 20 queries return identical results on all three platforms with the portable SQL files
in [`berlinmod/`](berlinmod/).

### Phase 6 — Catalyst extensions for [MobilitySpark](https://github.com/MobilityDB/MobilitySpark)
Implement Spark Catalyst planning rules for predicate push-down on `eIntersects` / `before`
/ `after`, leveraging the spatial index primitives exposed by
[PR #740](https://github.com/MobilityDB/MobilityDB/pull/740) (SP-GiST in MEOS).

## Open Questions

1. **[MobilityDB](https://github.com/MobilityDB/MobilityDB) maintainers**: review the
   [operator → function mapping table](#operator--function-mapping) and flag any name that
   should differ from the MEOS canonical.
2. **[JMEOS](https://github.com/MobilityDB/JMEOS) contributors**: confirm that
   [JMEOS 1.3 (PR #9)](https://github.com/MobilityDB/JMEOS/pull/9) exposes all functions
   referenced in the [table above](#operator--function-mapping), or note the gaps.
3. **[MobilitySpark](https://github.com/MobilityDB/MobilitySpark) contributors**: wire JMEOS
   1.3 and register UDFs using the canonical names.
4. **[MobilityPySpark](https://github.com/MobilityDB/MobilityPySpark) contributors**: rename
   UDFs to canonical names and switch serialization to MEOS binary / MF-JSON.
5. **Anyone**: run the queries in [`berlinmod/`](berlinmod/) on whichever platform you have
   available and report results.

The goal is **one SQL file, three platforms, same answer**.

## Related

- [MobilityDB](https://github.com/MobilityDB/MobilityDB) — PostgreSQL extension
- [MobilityDuck](https://github.com/MobilityDB/MobilityDuck) — DuckDB extension
- [MobilitySpark](https://github.com/MobilityDB/MobilitySpark) — Apache Spark UDFs (Java/JMEOS)
- [MobilityPySpark](https://github.com/MobilityDB/MobilityPySpark) — Apache Spark UDFs (Python/PyMEOS)
- [JMEOS](https://github.com/MobilityDB/JMEOS) — Java bindings for MEOS ([PR #9 = 1.3](https://github.com/MobilityDB/JMEOS/pull/9))
- [MEOS-API](https://github.com/MobilityDB/MEOS-API) — canonical function registry (meos-api.json)
