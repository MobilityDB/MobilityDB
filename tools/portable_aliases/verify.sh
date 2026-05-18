#!/usr/bin/env bash
# Copyright(c) MobilityDB Contributors
# Licensed under the PostgreSQL License (see LICENSE.txt).
#
# On-database equivalence check: every portable alias must resolve to the
# exact same C implementation as the operator it aliases.
#
# Usage: tools/portable_aliases/verify.sh <database> [psql args...]
# Requires the database to have `CREATE EXTENSION mobilitydb` already.

set -euo pipefail

db=${1:?usage: verify.sh <database> [psql args...]}
shift || true
here=$(cd "$(dirname "$0")" && pwd)

out=$(psql -d "$db" -At "$@" -f "$here/verify_equivalence.sql")
echo "$out"
if [ "$out" = "PORTABLE ALIASES OK" ]; then
  echo "verify.sh: PASS"
  exit 0
fi
echo "verify.sh: FAIL — aliases differ from their operators" >&2
exit 1
