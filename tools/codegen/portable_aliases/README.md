<!--
Copyright(c) MobilityDB Contributors
Licensed under the PostgreSQL License (see LICENSE.txt).
-->

# Portable operator coverage verifier

Every MobilityDB operator names its backing function with the bare portable
name directly — positional (`left`/`right`/`before`/`after` and the `over*`
variants), topological (`contains`/`contained`/`overlaps`/`adjacent`/`same`)
and comparison (`tEq`/`eEq`/`aEq`, …) — so the same SQL runs unchanged on
MobilityDB, MobilityDuck and MobilitySpark (RFC: `doc/rfc/sql-portability`). The
bare names live in the operator definitions; there is no generated SQL.

`generate.py` classifies every `CREATE OPERATOR` symbol in `mobilitydb/sql`
against the dialect and fails when any symbol is unclassified, so a parity gap
(a new operator with no portable-name mapping) surfaces as a build error rather
than silently.

## Check (CI gate, `check-code.yml`)

```bash
python3 tools/codegen/portable_aliases/generate.py --check
```

Fails on any unclassified operator. Adding an operator requires classifying its
symbol in `generate.py` — either mapping it to a bare portable name
(`OP_TO_NAME`) or placing it in a documented already-portable bucket: standard
SQL operators, the ever/always and distance families, json access/existence
operators, and operators with an existing callable named backing.
