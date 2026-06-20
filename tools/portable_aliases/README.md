<!--
Copyright(c) MobilityDB Contributors
Licensed under the PostgreSQL License (see LICENSE.txt).
-->

# Portable named-function aliases

`generate.py` produces a bare-name function alias for every MobilityDB
operator overload (topology, time-position, space X/Y/Z position, temporal
comparison, `same`), so the same SQL runs unchanged on MobilityDB, MobilityDuck
and MobilitySpark (RFC: `doc/rfc/sql-portability`). Each alias reuses the
operator's own backing C symbol, so it is the operator's exact implementation.

## Regenerate

```bash
python3 tools/portable_aliases/generate.py --insrc
```

Writes `mobilitydb/sql/<group>/<NNN>_portable_aliases.in.sql` (wired into each
subdir `CMakeLists.txt`) and `verify_equivalence.sql`. The generator is
idempotent — committed output equals a fresh run.

## Verification (three independent layers)

1. **Static invariants — no database** (CI gate, `check-code.yml`):

   ```bash
   python3 tools/portable_aliases/generate.py --check
   ```

   Fails on any of: collisions, unresolved backing function, invented C
   symbol, unclassified operator. The coverage audit classifies every
   `CREATE OPERATOR` symbol — 100% coverage with documented exclusions
   (standard SQL operators; operators with existing callable named backing).

2. **Equivalence by construction** — every alias C symbol is one the core SQL
   already uses (`invented symbols = 0`), so the alias and the operator are
   the same implementation regardless of any built `.so`.

3. **On-database equivalence** — after `CREATE EXTENSION mobilitydb` on a
   current build:

   ```bash
   tools/portable_aliases/verify.sh <database>
   ```

   Runs `verify_equivalence.sql`; expects `PORTABLE ALIASES OK`.
