#!/usr/bin/env python3
# SPDX-License-Identifier: PostgreSQL
#
# Export the pg_regress table fixtures to the CSV form the MEOS test
# programs read.
#
# `mobilitydb/test/<family>/data/load_*.sql.xz` carries every `tbl_*` table
# the PostgreSQL suite loads, as a PostgreSQL `COPY ... FROM stdin` block.
# The programs under `meos/test` answer, outside PostgreSQL, the same query
# over the same table, and read it from `meos/test/csv/<table>.csv`.
#
# One table is one file, so the two forms are the same data and neither can
# drift from the other unnoticed: this script rewrites the CSV set from the
# archives, and a fixture that changes is re-exported rather than re-typed.
#
# The CSV form of a `COPY` block is:
#   * the column names of the block, comma-separated, as the first line;
#   * one line per row, the tab separator written as a comma;
#   * rows in ascending order of the key column, so a file has one order
#     rather than the order the dump happened to carry;
#   * a NULL written as an empty field, which is what a reader that scans
#     the rest of the line after the key already reads it as.
# A value carrying a comma needs no quoting because every table is keyed by
# `k` and holds one value column: everything after the first comma is the
# value.
#
# Usage:
#   python3 tools/gen_test_csv.py            # rewrite meos/test/csv
#   python3 tools/gen_test_csv.py --check    # exit 1 if a file would change
#
# Run from the repo root.
"""Export the pg_regress table fixtures to meos/test/csv."""

from __future__ import annotations

import argparse
import lzma
import pathlib
import re
import sys

COPY_RE = re.compile(r"^COPY public\.(\w+) \(([^)]*)\) FROM stdin;$")

# PostgreSQL's COPY text format. Any other backslashed character represents
# itself, which is what carries the quotes a set value writes around its
# elements: `\"` is the quote, and a value holding a backslash is dumped as
# `\\`.
UNESCAPE = {
    "b": "\b", "f": "\f", "n": "\n", "r": "\r", "t": "\t", "v": "\v",
    "\\": "\\",
}


def unescape(field: str, where: str) -> str | None:
    """Return the value a COPY text field carries, or None for a NULL."""
    if field == r"\N":
        return None
    out = []
    i = 0
    while i < len(field):
        c = field[i]
        if c != "\\":
            out.append(c)
            i += 1
            continue
        i += 1
        if i >= len(field):
            raise SystemExit(f"{where}: a field ends in a backslash")
        e = field[i]
        if e.isdigit() or e == "x":
            raise SystemExit(f"{where}: a numeric COPY escape '\\{e}'")
        out.append(UNESCAPE.get(e, e))
        i += 1
    return "".join(out)


def read_tables(archive: pathlib.Path) -> dict[str, tuple[list[str], list[list[str | None]]]]:
    """Return, per table the archive loads, its columns and its rows."""
    tables: dict[str, tuple[list[str], list[list[str | None]]]] = {}
    columns: list[str] = []
    rows: list[list[str | None]] = []
    name = None
    with lzma.open(archive, "rt", encoding="utf-8") as f:
        for lineno, line in enumerate(f, 1):
            line = line.rstrip("\n")
            if name is None:
                m = COPY_RE.match(line)
                if m:
                    name = m.group(1)
                    columns = [c.strip() for c in m.group(2).split(",")]
                    rows = []
                continue
            if line == "\\.":
                tables[name] = (columns, rows)
                name = None
                continue
            where = f"{archive.name}:{lineno}"
            fields = [unescape(v, where) for v in line.split("\t")]
            if len(fields) != len(columns):
                raise SystemExit(
                    f"{where}: {len(fields)} fields for {len(columns)} columns")
            rows.append(fields)
    if name is not None:
        raise SystemExit(f"{archive.name}: a COPY block has no terminator")
    return tables


def csv_text(columns: list[str], rows: list[list[str | None]], table: str) -> str:
    """Return the CSV form of one table, ordered by its key column."""
    def key(row: list[str | None]) -> int:
        k = row[0]
        if k is None or not k.lstrip("-").isdigit():
            raise SystemExit(f"{table}: the key '{k}' is not an integer")
        return int(k)

    for row in rows:
        for value in row:
            if value is not None and "\n" in value:
                raise SystemExit(f"{table}: a value carries a newline")
    ordered = sorted(rows, key=key)
    lines = [",".join(columns)]
    lines += [",".join("" if v is None else v for v in row) for row in ordered]
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if a file would change instead of writing")
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    # The directory is written, never tracked, so a fresh checkout does not
    # carry it: git records no empty directory and the exported files are
    # ignored. Create it rather than assume a previous run left it behind.
    out_dir = root / "meos" / "test" / "csv"
    out_dir.mkdir(parents=True, exist_ok=True)
    archives = sorted((root / "mobilitydb" / "test").glob("*/data/*.sql.xz"))
    if not archives:
        raise SystemExit("no fixture archive under mobilitydb/test/*/data")

    written: dict[str, str] = {}
    for archive in archives:
        for table, (columns, rows) in read_tables(archive).items():
            text = csv_text(columns, rows, table)
            if table in written and written[table] != text:
                raise SystemExit(
                    f"{table}: two archives carry different rows for it")
            written[table] = text

    stale = {p.name for p in out_dir.glob("*.csv")} - {
        f"{t}.csv" for t in written}
    changed = sorted(stale)
    for table, text in sorted(written.items()):
        path = out_dir / f"{table}.csv"
        if path.exists() and path.read_text(encoding="utf-8") == text:
            continue
        changed.append(f"{table}.csv")
        if not args.check:
            path.write_text(text, encoding="utf-8")
    if not args.check:
        for name in stale:
            (out_dir / name).unlink()

    print(f"{len(written)} tables from {len(archives)} archives")
    if changed:
        verb = "would change" if args.check else "written"
        print(f"{len(changed)} {verb}: {', '.join(sorted(changed)[:8])}"
              + (" ..." if len(changed) > 8 else ""))
    return 1 if (args.check and changed) else 0


if __name__ == "__main__":
    sys.exit(main())
