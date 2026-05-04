#!/usr/bin/env python3
r"""TemporalParquet importer (RFC #830).

Reads a Parquet file conforming to the TemporalParquet 1.0 convention
(see poc_export.py) and inserts its contents into a MobilityDB table.

The temporal column's BLOB bytes are passed to MobilityDB via the
appropriate `<base_type>FromBinary(bytea)` SQL function so MobilityDB
parses them with its own MEOS-WKB decoder. No client-side decoding.

Usage:
    poc_import.py --pg-conn '<libpq URI>' \\
                  --table <name> \\
                  --input <path.parquet>
"""

import argparse
import json
import sys

import psycopg2
from psycopg2 import sql as pgsql
import pyarrow.parquet as pq

# Maps base_type -> SQL function that takes bytea and returns the
# temporal value of that type.
FROM_BINARY = {
    "tgeompoint": "tgeompointFromBinary",
    "tgeogpoint": "tgeogpointFromBinary",
    "tfloat":     "tfloatFromBinary",
    "tint":       "tintFromBinary",
}

def main():  # pylint: disable=too-many-locals
    ap = argparse.ArgumentParser()
    ap.add_argument("--pg-conn", required=True)
    ap.add_argument("--table", required=True)
    ap.add_argument("--input", required=True)
    args = ap.parse_args()

    table = pq.read_table(args.input)
    md = table.schema.metadata or {}
    raw = md.get(b"temporal")
    if raw is None:
        raise SystemExit(f"{args.input} has no `temporal` footer "
                         f"metadata; not a TemporalParquet file")
    footer = json.loads(raw)
    if not footer["version"].startswith("1."):
        raise SystemExit(f"unsupported TemporalParquet version "
                         f"{footer['version']}; supported: 1.x")

    temporal_cols = list(footer["columns"].keys())
    for tc in temporal_cols:
        col_meta = footer["columns"][tc]
        if col_meta["encoding"] != "MEOS-WKB":
            raise SystemExit(f"unsupported encoding {col_meta['encoding']} "
                             f"for column {tc}; supported encoding: MEOS-WKB")
        if col_meta["base_type"] not in FROM_BINARY:
            raise SystemExit(f"Unsupported base_type "
                             f"{col_meta['base_type']}; supported: "
                             f"{', '.join(sorted(FROM_BINARY))}")

    conn = psycopg2.connect(args.pg_conn)
    cur = conn.cursor()
    rows_pylist = table.to_pylist()
    schema_names = [f.name for f in table.schema]

    # Build per-row INSERT, wrapping each temporal column in its
    # FromBinary(...) SQL function. Identifiers (table name, column
    # names, FromBinary function name) are composed via psycopg2.sql
    # so user-supplied / footer-supplied names are safely quoted and
    # cannot break out of the identifier grammar.
    value_exprs = []
    for name in schema_names:
        if name in temporal_cols:
            fn = FROM_BINARY[footer["columns"][name]["base_type"]]
            value_exprs.append(
                pgsql.SQL("{fn}(%s)").format(fn=pgsql.Identifier(fn)))
        else:
            value_exprs.append(pgsql.SQL("%s"))
    sql_stmt = pgsql.SQL("INSERT INTO {table} ({cols}) VALUES ({vals})").format(
        table=pgsql.Identifier(args.table),
        cols=pgsql.SQL(", ").join(pgsql.Identifier(n) for n in schema_names),
        vals=pgsql.SQL(", ").join(value_exprs))

    n = 0
    for row in rows_pylist:
        params = []
        for name in schema_names:
            v = row[name]
            if name in temporal_cols and v is not None:
                params.append(psycopg2.Binary(v))
            else:
                params.append(v)
        cur.execute(sql_stmt, params)
        n += 1

    conn.commit()
    print(f"imported {n} rows into {args.table} "
          f"(temporal columns: {','.join(temporal_cols)})",
          file=sys.stderr)


if __name__ == "__main__":
    main()
