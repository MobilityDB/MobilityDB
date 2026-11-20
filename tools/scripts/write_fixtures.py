#!/usr/bin/env python3
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

"""Write the fixtures of the regression tests from the data generator.

A setup script mobilitydb/test/<module>/sql/load_<module>.sql names the data
file of each table it loads and loads it with COPY, read as
tools/gen_test_csv.py reads them (#read_tables). The function of a script
mobilitydb/datagen/<family>/create_test_tables_<family>.sql makes the table.
For each family holding a table to write, one psql session creates the
extension mobilitydb_datagen, reads the script of the family, seeds the
random numbers with setseed(SEED) and calls the function at SIZE rows, then
writes each of those tables, ordered by its key, to its data file in the
format COPY reads. A session of its own per family keeps the rows of a family
the same whichever families are written.

usage: write_fixtures.py [psql connection options] TABLE... | --all

The database holds the extensions mobilitydb and mobilitydb_datagen of the
build the fixtures are for, and is one of its own: the function of a family
drops and creates every table of the family.
"""
# BINDING-HEADER-PARSE-OK: reads the SQL setup and data generator scripts of
# the tests, no C header; not a binding

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

# A setup script names the data file of a table, then loads it with COPY,
# the two lines tools/gen_test_csv.py reads
FILE_RE = re.compile(r"^\\set filename :srcdir '/(mobilitydb/test/\w+/data/\w+\.data)'$")
COPY_RE = re.compile(r"^COPY (\w+) \(([^)]*)\) FROM :'filename';$")
# The seed and the number of rows of every family of the fixtures
SEED = 0.5
SIZE = 100


def data_files(root: pathlib.Path) -> dict[str, pathlib.Path]:
    """Return the data file of each table a setup script loads"""
    result: dict[str, pathlib.Path] = {}
    for setup in sorted((root / "mobilitydb" / "test").glob("*/sql/load_*.sql")):
        filename = None
        for line in setup.read_text(encoding="utf-8").splitlines():
            m = FILE_RE.match(line)
            if m:
                filename = m.group(1)
                continue
            m = COPY_RE.match(line)
            if m:
                result[m.group(1).lower()] = root / filename
    return result


def families(root: pathlib.Path) -> dict[str, tuple[pathlib.Path, str]]:
    """Return the script and the function of the generator making each table"""
    result: dict[str, tuple[pathlib.Path, str]] = {}
    for script in sorted((root / "mobilitydb" / "datagen").glob("*/create_test_tables_*.sql")):
        text = script.read_text(encoding="utf-8")
        func = re.search(r"FUNCTION (create_test_tables_\w+)\(", text).group(1)
        for m in re.finditer(r"CREATE TABLE (\w+)", text):
            result.setdefault(m.group(1).lower(), (script, func))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--all", action="store_true",
        help="every table a setup script loads that the generator makes")
    parser.add_argument("--print", action="store_true",
        help="print the psql commands instead of running them; setseed "
             "restarts the random numbers, so the families run in one "
             "session write the same rows")
    parser.add_argument("tables", nargs="*")
    args, psql_options = parser.parse_known_args()
    root = pathlib.Path(__file__).resolve().parents[2]
    files = data_files(root)
    makers = families(root)
    tables = sorted(t for t in files if t in makers) if args.all else \
        [t.lower() for t in args.tables]
    unknown = [t for t in tables if t not in files or t not in makers]
    if unknown:
        raise SystemExit("not loaded by a setup script or not made by the "
            "generator: " + ", ".join(unknown))
    by_family: dict[tuple[pathlib.Path, str], list[str]] = {}
    for table in tables:
        by_family.setdefault(makers[table], []).append(table)
    for (script, func), names in sorted(by_family.items()):
        lines = ["\\set ON_ERROR_STOP on",
                 "SET client_min_messages = warning;",
                 "CREATE EXTENSION IF NOT EXISTS mobilitydb_datagen CASCADE;",
                 f"\\i '{script}'",
                 f"SELECT setseed({SEED});",
                 f"SELECT {func}({SIZE});"]
        lines += [f"\\copy (SELECT * FROM {t} ORDER BY 1) TO '{files[t]}'"
                  for t in names]
        if args.print:
            print("\n".join(lines))
            continue
        subprocess.run(["psql", "-X", "-q"] + psql_options,
            input="\n".join(lines) + "\n", text=True, check=True)
        print(f"{func}: {' '.join(names)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
