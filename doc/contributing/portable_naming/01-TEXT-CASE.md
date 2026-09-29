<!--
  MobilityDB — Portable Naming: Family 1 — Text case and bounds
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Family 1 — Text case and bounds

[Back to the index](00-INDEX.md)

`lower`, `upper` and `initcap` are built-in functions of Spark 3.5.1 and Flink 2.0.0. Their
siblings `lowerInc` and `upperInc` conflict with nothing and take the same prefix. The family
holds the 86 signatures of master `2495c4cc36` and 3 new ones over `text`: 89 in all.

## Rules

1. **One name, one operation.** `lower` and `upper` name two operations in MobilityDB, changing
   the case of text and reading the bound of a span. Each operation takes its own name.
2. **An operation that also applies to the base type takes the base type's name.** Text case
   applies to `text`, `textset` and `ttext`, and MEOS spells it `text_lower`, `textset_lower`,
   `ttext_lower`: the name is `textLower`.
3. **Related functions take the same prefix.** `spanLower` goes with `spanUpper`,
   `spanLowerInc` and `spanUpperInc`, and likewise for every class that carries the group. One
   operation keeps one spelling across classes: the temporal `lowerInc` becomes
   `temporalLowerInc`, as `spanLowerInc` and `spansetLowerInc` are.
4. **PostgreSQL keeps its names**; the Flink and Spark name is the `@altsqlfn` of the
   PostgreSQL wrapper.

## Final names

| Flink and Spark name | PostgreSQL name | Operand types | PostgreSQL wrapper | MEOS function | Signatures |
|---|---|---|---|---|---|
| `textLower` | `lower` | `text`, `textset`, `ttext` | `Textset_lower`, `Ttext_lower` | `text_lower`, `textset_lower`, `ttext_lower` | 3 |
| `textUpper` | `upper` | `text`, `textset`, `ttext` | `Textset_upper`, `Ttext_upper` | `text_upper`, `textset_upper`, `ttext_upper` | 3 |
| `textInitcap` | `initcap` | `text`, `textset`, `ttext` | `Textset_initcap`, `Ttext_initcap` | `text_initcap`, `textset_initcap`, `ttext_initcap` | 3 |
| `spanLower` | `lower` | the 5 spans | `Span_lower` | `intspan_lower`, `bigintspan_lower`, `floatspan_lower`, `datespan_lower`, `tstzspan_lower` | 5 |
| `spanUpper` | `upper` | the 5 spans | `Span_upper` | `intspan_upper`, ..., `tstzspan_upper` | 5 |
| `spanLowerInc` | `lowerInc` | the 5 spans | `Span_lower_inc` | `span_lower_inc` | 5 |
| `spanUpperInc` | `upperInc` | the 5 spans | `Span_upper_inc` | `span_upper_inc` | 5 |
| `spansetLower` | `lower` | the 5 span sets | `Spanset_lower` | `intspanset_lower`, ..., `tstzspanset_lower` | 5 |
| `spansetUpper` | `upper` | the 5 span sets | `Spanset_upper` | `intspanset_upper`, ..., `tstzspanset_upper` | 5 |
| `spansetLowerInc` | `lowerInc` | the 5 span sets | `Spanset_lower_inc` | `spanset_lower_inc` | 5 |
| `spansetUpperInc` | `upperInc` | the 5 span sets | `Spanset_upper_inc` | `spanset_upper_inc` | 5 |
| `temporalLowerInc` | `lowerInc` | the 20 temporal types | `Temporal_lower_inc` | `temporal_lower_inc` | 20 |
| `temporalUpperInc` | `upperInc` | the 20 temporal types | `Temporal_upper_inc` | `temporal_upper_inc` | 20 |
| | | | | **Total** | **89** |

The `text` signatures are new in every engine: PostgreSQL declares `textLower(text)`,
`textUpper(text)`, `textInitcap(text)` over the MEOS functions `text_lower`, `text_upper`,
`text_initcap`, beside its own `lower(text)`, `upper(text)`, `initcap(text)`. The other 86
signatures keep their PostgreSQL names and take the Flink and Spark name as `@altsqlfn`.

The 5 spans are `intspan`, `bigintspan`, `floatspan`, `datespan`, `tstzspan`; the 5 span sets
`intspanset`, `bigintspanset`, `floatspanset`, `datespanset`, `tstzspanset`. The 20 temporal
types are `tbool`, `tint`, `tbigint`, `tfloat`, `ttext`, `tjsonb`, `tgeompoint`, `tgeogpoint`,
`tgeometry`, `tgeography`, `tcbuffer`, `tnpoint`, `tpose`, `tposechain`, `trgeometry`,
`th3index`, `tquadbin`, `ts2cell`, `tpcpoint`, `tpcpatch`.

Each prefix is the stem of the MEOS function: `text`/`textset`/`ttext` for text case, `span`,
`spanset` and `temporal` for the bounds and their inclusivity.

## Checks

- Neither engine defines, and neither parser refuses, any of the 13 names; none is a MobilityDB
  SQL name today (`f1-spark.tsv`, `f1-flink.tsv`, `f1b-spark.tsv`, `f1b-flink.tsv` in
  `analysis/`).
- The signature counts are those of `sigs-2495c4cc36.tsv`: `lower` 12, `upper` 12, `initcap` 2,
  `lowerInc` 30, `upperInc` 30.

## How one name reaches each operand type

PostgreSQL and DuckDB choose the function by the argument's type. In Flink and Spark a MEOS
value travels as its binary WKB (Spark `BinaryType`, `byte[]`; Flink `BYTES`) and a `text`
value as a STRING, so the engine tells `text` from `textset` and `ttext` by the SQL type of the
argument (index, decided rule 6). Among the binary values, the WKB type byte names the type:
41 is `ttext`, 32 `textset`.

A MEOS value carried as hex WKB would be a STRING too, and then indistinguishable from a `text`
whose characters are that hex: measured (`text-hex-probe.log`), the 46
characters of the hex WKB of `"ABC"@2000-01-01` passed as a `text` are read as that `ttext`.
Binary carrying removes the case.

The span and span set names serve one template class each, and the temporal names one class:
each reads the type byte of its operand, and a value of another class raises an error naming
its type.
