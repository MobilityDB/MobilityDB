#!/usr/bin/env python3
r"""TemporalParquet exporter (RFC #830).

Reads a MobilityDB table containing temporal columns and writes a Parquet
file using the TemporalParquet 1.0 footer convention:

  - Each temporal column is encoded as MEOS-WKB in a Parquet BYTE_ARRAY
    column (no per-row tag; the whole column has a single base-type).
  - The Parquet file's `key_value_metadata` carries a `temporal` JSON
    object listing each temporal column's metadata (subtype,
    interpolation, base_type, srid, geodetic, has_z, encoding version).
  - Plain (non-temporal) columns are passed through with their natural
    Parquet types so that the file is also readable by tools that
    don't understand the `temporal` footer (the BLOB column will look
    opaque to them, which is the same posture GeoParquet takes for
    GEOMETRY columns).

Supported base types: tgeompoint, tgeogpoint, tfloat, tint.
Extending to the rest of the temporal-type families is mechanical (add
a per-type metadata extractor that mirrors the SQL query below).

Usage:
    poc_export.py --pg-conn '<libpq URI>' \\
                  --table <name> \\
                  --temporal-cols <col1>,<col2>,... \\
                  --output <path.parquet>
"""

import argparse
import json
import sys

import psycopg2
from psycopg2 import sql
import pyarrow as pa
import pyarrow.parquet as pq

TEMPORAL_PARQUET_VERSION = "1.0.0"
ENCODING = "MEOS-WKB"
ENCODING_VERSION = "1.0"

# Per-type metadata extractor. Each returns a dict suitable for the
# `columns.<col>` entry in the `temporal` footer.
def _meta_spatiotemporal(cur, table, col, base_type, geodetic):
    # Cast the start point to geometry for ST_HasZ — geography lacks
    # a direct ST_HasZ overload in PostGIS but ::geometry preserves
    # the Z dimension.
    # Identifiers are composed via psycopg2.sql so user-supplied table
    # / column names cannot break out of the identifier grammar.
    col_id = sql.Identifier(col)
    cur.execute(sql.SQL("""
        SELECT tempSubtype({col}), interp({col}),
               srid({col}),
               (numInstants({col}) > 0
                AND ST_HasZ(startValue({col})::geometry))
        FROM {table} WHERE {col} IS NOT NULL LIMIT 1
    """).format(col=col_id, table=sql.Identifier(table)))
    row = cur.fetchone()
    if row is None:
        return {"base_type": base_type, "subtype": None,
                "interpolation": None, "srid": 0,
                "geodetic": geodetic, "has_z": False}
    subtype, interp, srid, has_z = row
    return {"base_type": base_type, "subtype": subtype,
            "interpolation": interp, "srid": srid,
            "geodetic": geodetic, "has_z": bool(has_z)}

def _meta_tgeompoint(cur, table, col):
    return _meta_spatiotemporal(cur, table, col, "tgeompoint", False)

def _meta_tgeogpoint(cur, table, col):
    return _meta_spatiotemporal(cur, table, col, "tgeogpoint", True)

def _meta_scalar(cur, table, col, base_type):
    col_id = sql.Identifier(col)
    cur.execute(sql.SQL("""
        SELECT tempSubtype({col}), interp({col})
        FROM {table} WHERE {col} IS NOT NULL LIMIT 1
    """).format(col=col_id, table=sql.Identifier(table)))
    row = cur.fetchone()
    if row is None:
        return {"base_type": base_type, "subtype": None,
                "interpolation": None}
    subtype, interp = row
    return {"base_type": base_type, "subtype": subtype,
            "interpolation": interp}

def _meta_tfloat(cur, table, col):
    return _meta_scalar(cur, table, col, "tfloat")

def _meta_tint(cur, table, col):
    return _meta_scalar(cur, table, col, "tint")


META_DISPATCH = {
    "tgeompoint": _meta_tgeompoint,
    "tgeogpoint": _meta_tgeogpoint,
    "tfloat":     _meta_tfloat,
    "tint":       _meta_tint,
}

def _detect_base_type(cur, table, col):
    cur.execute("""
        SELECT format_type(atttypid, NULL)
        FROM pg_attribute
        WHERE attrelid = %s::regclass AND attname = %s
    """, (table, col))
    row = cur.fetchone()
    if row is None:
        raise SystemExit(f"column {table}.{col} not found")
    base = row[0]
    if base not in META_DISPATCH:
        raise SystemExit(f"Unsupported base type '{base}'; "
                         f"supported: {', '.join(sorted(META_DISPATCH))}")
    return base

def main():  # pylint: disable=too-many-locals
    ap = argparse.ArgumentParser()
    ap.add_argument("--pg-conn", required=True)
    ap.add_argument("--table", required=True)
    ap.add_argument("--temporal-cols", required=True,
                    help="comma-separated list of temporal column names")
    ap.add_argument("--output", required=True)
    args = ap.parse_args()

    temporal_cols = [c.strip() for c in args.temporal_cols.split(",") if c.strip()]
    conn = psycopg2.connect(args.pg_conn)
    cur = conn.cursor()

    # Discover non-temporal columns to pass through.
    cur.execute("""
        SELECT attname, format_type(atttypid, NULL)
        FROM pg_attribute
        WHERE attrelid = %s::regclass AND attnum > 0 AND NOT attisdropped
        ORDER BY attnum
    """, (args.table,))
    all_cols = cur.fetchall()
    passthrough = [(name, typ) for (name, typ) in all_cols
                   if name not in temporal_cols]

    # Build SELECT: passthrough columns natively, temporal columns as
    # bytea via asBinary() / asEWKB(). Spatial-temporal columns use
    # asEWKB so the SRID survives the round trip; the import side's
    # tgeompointFromBinary / tgeogpointFromBinary accept EWKB transparently.
    # Composed via psycopg2.sql.Identifier so user-supplied names are
    # safely quoted.
    select_exprs = [sql.Identifier(n) for (n, _) in passthrough]
    for tc in temporal_cols:
        base = _detect_base_type(cur, args.table, tc)
        binary_fn = "asEWKB" if base in ("tgeompoint", "tgeogpoint") else "asBinary"
        select_exprs.append(
            sql.SQL("{fn}({col}) AS {alias}").format(
                fn=sql.Identifier(binary_fn),
                col=sql.Identifier(tc),
                alias=sql.Identifier(tc)))
    cur.execute(sql.SQL("SELECT {cols} FROM {table}").format(
        cols=sql.SQL(", ").join(select_exprs),
        table=sql.Identifier(args.table)))
    rows = cur.fetchall()

    # Build Arrow arrays in column order.
    columns = {}
    n_pass = len(passthrough)
    for i, (name, _typ) in enumerate(passthrough):
        columns[name] = pa.array([r[i] for r in rows])
    for j, tc in enumerate(temporal_cols):
        columns[tc] = pa.array([bytes(r[n_pass + j]) if r[n_pass + j] is not None else None
                                 for r in rows], type=pa.binary())

    table = pa.Table.from_pydict(columns)

    # Build the TemporalParquet footer.
    footer = {"version": TEMPORAL_PARQUET_VERSION,
              "primary_temporal_column": temporal_cols[0],
              "columns": {}}
    for tc in temporal_cols:
        base = _detect_base_type(cur, args.table, tc)
        col_meta = META_DISPATCH[base](cur, args.table, tc)
        col_meta["encoding"] = ENCODING
        col_meta["encoding_version"] = ENCODING_VERSION
        footer["columns"][tc] = col_meta

    existing = dict(table.schema.metadata or {})
    existing[b"temporal"] = json.dumps(footer, sort_keys=True).encode("utf-8")
    table = table.replace_schema_metadata(existing)

    pq.write_table(table, args.output, compression="snappy")
    print(f"wrote {len(rows)} rows to {args.output} "
          f"({len(passthrough)} passthrough, "
          f"{len(temporal_cols)} temporal: {','.join(temporal_cols)})",
          file=sys.stderr)


if __name__ == "__main__":
    main()
