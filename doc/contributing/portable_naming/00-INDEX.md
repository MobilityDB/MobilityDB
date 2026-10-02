<!--
  MobilityDB — Portable Naming: MobilityDB names that Flink and Spark already define — brainstorm index
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# MobilityDB names that Flink and Spark already define — brainstorm index

One document per family of conflicting names, discussed one after the other. This index holds
what is measured, what differs from the 2026-09-13 documents, the rules decided so far, and the
families with their state.

**Measured at** MobilityDB master `2495c4cc36` (its 9,053 SQL declarations are identical to
`510c3184f6`), Spark 3.5.1, Flink 2.0.0, on 2026-09-28. The scripts (`sql_sigs.py`,
`SparkNameCensus.java`, `FlinkNameCensus.java`, `classify.py`), the behaviour probes and the
outputs these documents name (`f1-spark.tsv`, …) are kept outside the tree.

## What the engines do (run, not read)

| Behaviour | Spark 3.5.1 | Flink 2.0.0 |
|---|---|---|
| Two functions under one name | the second registration replaces the first, whatever its argument types or count | refused: "A function named ... does already exist" |
| Several signatures under one name | no | yes, several `eval` methods in one class |
| A MEOS value's SQL type | BINARY (its WKB), hex WKB and text read as well | RAW over its WKB bytes |
| A function registered under a built-in's name | replaces the built-in in that SparkSession | catalog level: the built-in wins; system level: replaces the built-in |
| Names the parser refuses unquoted | `overlaps` (ANSI mode) | `contains`, `hash`, `insert`, `merge`, `overlaps`, `range`, `set`, `unnest`, `update` |

Every MEOS value reaches both engines as a STRING, so neither engine can tell a `tfloat` from a
`floatspan` by type: one name serving several MEOS types dispatches on the value's type byte.
Measured on 1,000,000 floatspans, that dispatch costs nothing measurable (`spanRound` 598–669 ns
a call, `floatspanRound` 630–723 ns; Spark queries 640–765 ms against 693–801 ms; same answers
1,000,000 of 1,000,000).

The MobilitySpark surface at `052128a` registers the bare names: after
`GeneratedSpatioTemporalUDFs.registerAll`, Spark's `round(2.345678, 2)` fails,
`contains('abc','b')`, `lower('ABC')`, `upper('abc')`, `length('abc')` answer `null`, and
`abs`, `floor`, `ceil`, `exp` fail at execution.

## The conflicts

27 names, 661 signatures. A name conflicts when Spark or Flink defines it or either parser
refuses it unquoted.

| Name | Signatures | Spark | Flink | Family |
|---|---|---|---|---|
| `lower`, `upper` | 12 + 12 | built-in | built-in | [1 Text case and bounds](01-TEXT-CASE.md) |
| `initcap` | 2 | built-in | built-in | [1 Text case and bounds](01-TEXT-CASE.md) |
| `round` | 41 | built-in | built-in | [2 Rounding](02-ROUNDING.md) |
| `abs`, `ceil`, `floor`, `cos`, `sin`, `tan`, `exp`, `ln`, `log10`, `degrees`, `radians` | 26 | built-in | built-in | [3 Mathematical functions](03-MATH.md) |
| `transform`, `translate` | 21 + 4 | built-in | — | [4 Spatial transformations](04-SPATIAL-TRANSFORMATIONS.md) |
| `length` | 4 | built-in | — | [5 Spatial accessors](05-SPATIAL-ACCESSORS.md) |
| `contains`, `overlaps` | 169 + 141 | built-in / ANSI keyword | keyword | [6 Bounding-box topology](06-BOUNDING-BOX-TOPOLOGY.md) |
| `hash` | 58 | built-in | keyword | [7 Hashing](07-HASHING.md) |
| `insert`, `update`, `merge` | 20 + 20 + 53 | — | keyword | [8 Modifications](08-MODIFICATIONS.md) |
| `unnest` | 38 | — | keyword | [9 Unnesting](09-UNNESTING.md) |
| `set` | 36 | — | keyword | 10 Constructors |
| `range` | 4 | — | keyword | 11 PostgreSQL-typed results |

## The gist documents compared with master

The gist (`FLINK-SPARK-BUILTIN-RENAMES.md` and the seven class documents, master `1d910f19b0`)
counted 29 names and 858 signatures. On today's master:

- `left` and `right` (96 signatures each) are not SQL names on master: the position functions carry
  their class names (`spanLeft`, `stboxLeft`, ...), as `doc/portable_sql.xml` lists them.
- `eLt` (12) is spelled out (`eLessThan`).
- `mergeAgg` is declared (20 signatures), so the 17 `merge` aggregates already have a
  non-conflicting name.
- `set(...)` carries `@altsqlfn setMake()` on `Set_constructor` and `Value_to_set`
  (MobilityDB #2736); it is the only `@altsqlfn` tag on master.

The rules the gist applied and where this brainstorm departs from them:

| Gist rule | Gist names | This brainstorm |
|---|---|---|
| 1. closest parent class (`talpha`, `tnumber`, `tspatial`, `tgeo`, ...) | `tspatialLength` | internal classes are never a prefix (decided below) |
| 7. `t` marks a lift | `tRound`, `tLower`, `tFloor`, `tTransform` | a lift of a base-type operation takes the base type's name (decided below) |
| base types take their own type name | `geometryRound`, `geographyRound` | `geoRound`: one MEOS function `geo_round` and one wrapper `Geo_round` serve both |
| 5. bounding-box operators take the box they compare | `stboxContains`, `tboxOverlaps`, `spanOverlaps` | same (family 6 re-examines it) |
| 3. `set(...)` becomes `setMake` | `setMake` | merged as the `@altsqlfn` tag |

## Decided rules

1. **No bare name** in Flink or Spark for a conflicting name: it either breaks the engine's
   function or cannot be parsed (measured above). **Flink and Spark carry one name set**: a name
   conflicting in either engine takes its class-prefixed name in both, and only that name, so a
   query written for one runs in the other (`insert` conflicts in Flink alone; both take
   `temporalInsert`).
2. **PostgreSQL keeps its names**; the Flink and Spark name is an alternative name
   (`@altsqlfn` on the PostgreSQL wrapper, as for `setMake`).
3. **Internal template classes are never a prefix**: `talpha`, `tnumber`, `tspatial`, `tgeo`,
   `tpoint`.
4. **An operation that also applies to the base type takes the base type's name**: `lower`
   serves `text` and `ttext`, so it is `textLower`; likewise `floatRound`, `geoRound`. Which
   signatures of a family this reaches is settled family by family.
5. **Related functions take the same prefix**: with `spanLower` come `spanUpper`,
   `spanLowerInc`, `spanUpperInc`; one operation keeps one spelling across classes
   (`spanLowerInc`, `spansetLowerInc`, `temporalLowerInc`).
6. **MEOS values travel as binary WKB in Flink and Spark** (Spark `BinaryType`, Flink `BYTES`),
   base values as the engine's own types (`DOUBLE`, `STRING`, ...). Measured against hex WKB
   over the same values and the same MEOS call: half the size (673 against 1,346 bytes for a
   20-instant `tgeompoint`, 660 KB against 1.32 MB for 20,000 instants), about 1.5 times faster
   per value in the JVM and about 1.7 times in Spark, the same answers on every value
   (`binary-vs-hex*.log`). A `text` and a `ttext` are then different SQL
   types, and TemporalParquet's binary column is read without a hex round trip.
7. **Functions that conflict with nothing keep their names** unless rule 5 puts them in a
   group with a conflicting name.
8. **A MEOS value in Spark carries its SQL type** (one Spark user-defined type per MEOS SQL
   type, stored as its WKB) and **each Spark name is one registration resolving its overload
   from the argument types**, so one name answers each overload's own result type, and a call on
   Spark's own values goes to Spark's built-in. A conflicting name is registered under its
   class-prefixed name alone (rule 1). No speed cost
   ([One Spark name over overloads with different result types](SPARK-RESULT-TYPES.md),
   measured).
9. **The text constructor is `<type>FromText(text)` in every engine**, the inverse of the
   `asText` every type has, completing the `FromText` family the spatial types carry. In
   PostgreSQL it is an ordinary function beside the untouched `<type>_in` input function, one
   wrapper per family taking its type from its declared return type, over the same MEOS parser;
   a type whose text is its hex WKB, as `raquet`, whose input and output functions read
   and write it, takes its text through `raquetFromHexWKB` and `asHexWKB`, as the PostGIS
   raster, whose `raster_in` and `raster_out` read and write hex WKB, has `ST_AsHexWKB` and
   `ST_RastFromHexWKB` and no `ST_AsText` ([I/O plan](IO-PLAN.md), measured).

## Families

| # | Family | Names | Signatures | State |
|---|---|---|---|---|
| 1 | [Text case and bounds](01-TEXT-CASE.md) | `lower`, `upper`, `initcap`, with `lowerInc`, `upperInc` | 89 | final |
| 2 | [Rounding](02-ROUNDING.md) | `round` | 41 | final |
| 3 | [Mathematical functions](03-MATH.md) | `abs`, `ceil`, `floor`, `cos`, `sin`, `tan`, `exp`, `ln`, `log10`, `degrees`, `radians` | 26 | final |
| 4 | [Spatial transformations](04-SPATIAL-TRANSFORMATIONS.md) | `transform`, `translate`, with `transformPipeline` and the affine family (`affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `transscale`) | 21 + 19 + 51 | final |
| 5 | [Spatial accessors](05-SPATIAL-ACCESSORS.md) | `length`, with `cumulativeLength` | 4 + 4 | final |
| 6 | [Bounding-box topology](06-BOUNDING-BOX-TOPOLOGY.md) | `contains`, `overlaps`, with `contained`, `same`, `adjacent` | 310 + 419 | final, on condition of exact cross-binding behaviour |
| 7 | [Hashing](07-HASHING.md) | `hash`, with `hashExtended` | 58 + 58 | final |
| 8 | [Modifications](08-MODIFICATIONS.md) | `insert`, `update`, `merge` | 93 | final |
| 9 | [Unnesting](09-UNNESTING.md) | `unnest` | 38 | final |
| 10 | Constructors | `set` | 36 | `setMake` merged (#2736) |
| 11 | PostgreSQL-typed results | `range` | 4 | out of scope: no engine has a range type |
| 12 | [Native geometry operations](12-NATIVE-GEOMETRY.md) | `contains`, `covers`, `disjoint`, `intersects`, `touches`, `dwithin`, `equals`, `distance`, `area`, `points`, ... over geometry and geography; `geoX` in Flink and Spark | about 40 operations | decided (1–9), plan in the document |

The four PostgreSQL I/O functions (`_in`, `_out`, `_send`, `_recv`) and the function a query calls in their place in every engine, type by type: [I/O plan](IO-PLAN.md).

What the generated Flink and Spark surfaces carry of the 168 set-returning functions, and the gaps
behind it by repository: [Set-returning functions](10-SET-RETURNING.md).

The user side of families 1–7 and rules 8–9, as Docbook XML for the manual: the manual chapters this pull request changes.

The pull requests that carry it all, the MobilityDB gaps first as the prerequisite of the
renaming, then the catalog and generator gaps, then the renaming campaign:
[Implementation plan](IMPLEMENTATION-PLAN.md).

## After the last family

An overall pass reads every family's final names side by side and checks that the decided rules
give the same answer wherever two families meet (a prefix, a sibling group, a base type). The
affine family moves with `translate` (family 4). The
implementation (the `@altsqlfn` tags, the new `text` and `geometry`/`geography` signatures, the
generated Flink and Spark surfaces) follows that pass.
