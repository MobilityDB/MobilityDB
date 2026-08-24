<!--
  MobilityDB — Temporal Network Points: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Temporal Network Points — Design Notes

A network point is a `(rid, pos)` pair whose position is relative to a route in a loaded network, so its behaviour follows from two decisions: the SRID comes from the route registry rather than the value, and network-relative semantics apply only within one connected component. The notes below record the conventions the implementation enforces and the numerical characteristics of the kernels.

> **Audience**: a contributor or binding author working at the MEOS C level.
> This is design rationale, not user documentation — the user manual
> (`doc/temporal_network_points.xml`) intentionally does **not** carry these internals.

---

## When to use tnpoint vs tgeompoint

A `tgeompoint` tracks a position in free space; a `tnpoint` tracks a position constrained to a known network of routes. Each network-point value is a pair `(rid, pos)` where `rid` is the route identifier and `pos` is the relative position along that route, normalised to `[0, 1]` (start of route to end of route).

Use `tgeompoint` when the moving object is not constrained to a network, when the network topology is unknown, or when only the trajectory geometry matters &#x2014; vehicle GPS tracks at the raw-feed level, animal movement, AIS feeds.

Use `tnpoint` when motion is on a known network and you want network-relative semantics: distances measured along the route, not in free space; aggregations grouped by route segment; spatial joins constrained to "same network". Canonical workloads are train-network and road-network tracking, where map-matched feeds become `tnpoint` values.

The position component of a `tnpoint` is recoverable as a `tgeompoint` via the standard cast (or as a `geometry` via `geometry(npoint)`); a `geometry` can be projected back onto the loaded network with `npoint(geometry)`. Consequently a single `tnpoint` column subsumes a map-matched `tgeompoint` column when network-relative semantics are needed.

## SRID inheritance from the network registry

An `npoint` value does not carry its own SRID; it inherits the SRID of the route referenced by its `rid` field. The set of known routes &#x2014; the **ways** registry &#x2014; is loaded into a per-process cache (see `meos/src/npoint/ways_meos.c`) the first time a network function runs, and is keyed by route gid. Every `npoint` accessor that needs the route geometry (`npoint_to_geompoint`, `npoint_to_stbox`, all spatial predicates and distance functions) consults this cache.

In the PostgreSQL extension the cache is populated automatically from the `public.ways` table. In standalone MEOS the routes come from a CSV file, whose path `meos_set_ways_csv(...)` states before any `npoint` operation; `meos_finalize_ways()` is idempotent and can be called multiple times safely. Cross-type operations between an `npoint` and a geometry validate that the route's SRID matches the geometry's SRID; mismatches raise an explicit error rather than silently re-projecting.

## Numerical hazards

Four hazards reliably bite real network-constrained tracking workloads. The implementation mitigates each as follows:

- **Position-parameter clamping at [0, 1]**. A position outside `[0, 1]` has no meaning on a route that has not been extended.

  **Mitigation**: `npoint_make` rejects `pos < 0` or `pos > 1` at construction with the message "The relative position must be a real number between 0 and 1", and the same check is applied at the WKT, WKB, and MFJSON parser entries. The endpoints `0.0` (start of route) and `1.0` (end of route) are valid and supported &#x2014; the test suite exercises both. Corrupt or unclipped feed data fails loudly rather than poisoning a downstream query.

- **Mixed-SRID handling**. A `tnpoint` inherits its SRID from its route, while a `geometry` argument carries its SRID explicitly.

  **Mitigation**: every cross-type operator (`tnpoint <-> geometry`, `tnpoint && geometry`, `distance(tnpoint, geometry)`) validates the two SRIDs match through `ensure_same_srid`; mismatches raise an explicit error rather than silently re-projecting one side onto the other.

- **Route-not-registered**. An `npoint` referencing a route gid not present in the ways cache cannot resolve to a geometry.

  **Mitigation**: `npoint_make` calls `ensure_route_exists` at construction; the WKT, WKB, MFJSON, and `npoint(...)` SQL constructors all flow through this check and raise "There is no route with gid value <n> in table ways" on miss. Standalone MEOS users whose ways CSV does not carry the route will see this error on the first network operation, not later as a silent zero-result query.

- **Cross-network operations**. Two `tnpoint` values whose routes belong to different networks (or different connected components of the same network) cannot be related through network distance.

  **Mitigation**: distance and spatial-relationship operators between two `tnpoint` values lower both sides to `tgeompoint` first and compute the result as free-space distance / topology. This is documented behaviour: callers who need true network distance must restrict their query to a single connected component upstream.

## Durability and storage

The on-disk representation of an `npoint` is a 16-byte tuple: an 8-byte route id followed by an 8-byte position. The byte layout is stable: `asBinary(tnpoint)` / `tnpointFromBinary(bytea)` round-trip preserves the value bit-for-bit, and the WKB / HexEWKB serialisations follow the standard temporal-types endian-flag-then-payload pattern. The `asMFJSON(tnpoint)` / `tnpointFromMFJSON(text)` pair is bidirectional, so MovingFeatures-JSON is a viable interchange format on equal footing with WKB. Each instant renders as `{"route":<route>,"position":<position>}`, with the field names matching the SQL accessor surface (`route`, `getPosition`) on the natural-language descriptor convention.

The route id is emitted as a JSON number, which round-trips losslessly between MEOS encoders and decoders (both retain full `int64` precision via `INT64_FORMAT` / `strtoll`). It is not, however, lossless across JavaScript / ECMAScript consumers: the `Number` type has a 53-bit mantissa, so route ids beyond `2^53` (~9 × 10^15) lose precision when parsed by a generic browser-side `JSON.parse`. For OSM-derived networks this matters in practice — OSM way ids crossed `2^53` in 2025 — and the recommended mitigation on the JS side is to parse with a library that materialises numbers as `BigInt` (for example `json-bigint`) rather than rely on the default. This is a deliberate design choice, not an oversight: the alternative (emitting the route as a JSON string, as `th3index` does for its int64 cell payload) would diverge from the JSON shape that all numeric accessors return for this type and break the round-trip with naive int-parsing on every consumer that **does** handle 64-bit integers correctly.

The `pg_dump` output of a table containing `npoint` or `tnpoint` columns uses the WKT representation by default; `pg_dump --binary-upgrade` uses the WKB representation. Both are round-trip-stable, but a dump is only restorable into a database whose `public.ways` table contains all referenced route ids: the route registry is logically a foreign key on every `npoint` column, even though it is not enforced as one.
