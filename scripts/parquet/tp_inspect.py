#!/usr/bin/env python3
r"""TemporalParquet inspector (RFC #830).

Pretty-prints the `temporal` footer and a small row sample of any
TemporalParquet file. Useful for debugging the convention from
non-MobilityDB tooling (e.g. confirming a MobilityDuck-written file
matches the spec a MobilityDB-written file produces, byte-for-byte
in the metadata).

Usage:
    tp_inspect.py <path.parquet>
"""

import json
import sys

import pyarrow.parquet as pq


def main():
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        sys.exit(2)
    path = sys.argv[1]
    table = pq.read_table(path)
    md = dict(table.schema.metadata or {})

    print(f"# {path}")
    print(f"  rows:     {table.num_rows}")
    print(f"  columns:  {len(table.schema.names)}")
    print("  schema:")
    for f in table.schema:
        print(f"    {f.name}: {f.type}")

    raw = md.get(b"temporal")
    if raw is None:
        print("\n  WARNING: no `temporal` footer; not a TemporalParquet file.")
        sys.exit(0)
    footer = json.loads(raw)
    print("\n  temporal footer:")
    print("    " + json.dumps(footer, indent=2, sort_keys=True).replace("\n", "\n    "))

    print("\n  row sample (first 3):")
    sample = table.slice(0, min(3, table.num_rows)).to_pylist()
    for i, row in enumerate(sample):
        print(f"    [{i}]")
        for k, v in row.items():
            if isinstance(v, (bytes, bytearray)) and len(v) > 32:
                print(f"      {k}: <{len(v)} bytes> {v[:16].hex()}...{v[-8:].hex()}")
            else:
                print(f"      {k}: {v!r}")


if __name__ == "__main__":
    main()
