<!--
This MobilityDB documentation is provided under The PostgreSQL License.
Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB contributors
-->

# Cross-Type Parity: Measured Status Report

Companion to [`cross_type_parity.md`](cross_type_parity.md): that document defines
the methodology and the intentional-exclusion catalog; this report is the measured
state produced by the parity audit harness ([`tools/parity_audit/`](../../tools/parity_audit/))
run against a live database.

**Basis of measurement**

| | |
|---|---|
| Source tree | `accumulate/parity-1.4` |
| Adapter | a SQL adapter over `pg_proc` (extension-owned functions, first-arg receiver type) |
| Backing gate | a function counts as *present* only when registered **and** backed by an exported MEOS symbol; registration or an alias alone never counts |
| Family references | Point uses `tgeompoint`; Extended-shape uses `tgeometry` |

## Measured parity
| family | type | reference ops | excluded | expected | present | parity |
|---|---|--:|--:|--:|--:|--:|
| Point | `tgeogpoint` | 182 | 34 | 148 | 144 | **97.3%** |
| Point | `tnpoint` | 182 | 22 | 160 | 156 | **97.5%** |
| Extended-shape | `trgeometry` | 159 | 8 | 151 | 147 | **97.4%** |
| Extended-shape | `tcbuffer` | 159 | 12 | 147 | 140 | **95.2%** |
| Extended-shape | `tpose` | 159 | 11 | 148 | 142 | **95.9%** |

**How the exclusions are accounted in the percentages.** Each member type is scored against its family reference (Point → `tgeompoint`, Extended-shape → `tgeometry`). *reference ops* is the reference's operator count; *excluded* is the reason-marked correct-absences (grouped by type below) removed from that surface; *expected* = *reference ops* − *excluded* is the **definable** surface for the type; *present* is how many of those the type actually backs. **parity = present / expected**, so excluded operators are dropped from the denominator entirely, so they neither raise nor lower the percentage. A type at 100 % covers everything it *can* support; the excluded operators are correct absences, never gaps, and are never implemented.

## Reason-marked exclusions (correct absences, never implement)

Grouped by type; operators sharing a reason are listed together.

### `tgeogpoint` (Point family)

- `convexHull`: a point's continuous form collapses to trajectory()/stbox()
- `asMVTGeom`, `spaceBoxes`, `spaceSplit`, `spaceTiles`, `spaceTimeBoxes`, `spaceTimeSplit`, `spaceTimeTiles`: a uniform planar coordinate grid / Web-Mercator MVT tile is undefined on the sphere; geodetic space binning requires a spherical cell grid rather than a planar one
- `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `translate`, `transscale`: affine transforms have no action on geodetic points
- `atGeometry`, `geometry`, `minusGeometry`: geodetic type uses the geography variants, not the planar geometry ones
- `temporal_above`, `temporal_back`, `temporal_below`, `temporal_front`, `temporal_left`, `temporal_overabove`, `temporal_overback`, `temporal_overbelow`, `temporal_overfront`, `temporal_overleft`, `temporal_overright`, `temporal_right`: planar relative-position operators (<<,>>,&<) are undefined on the sphere
- `aTouches`, `eTouches`, `tTouches`: touches/contains/covers use the GEOS DE-9IM relate matrix (planar-only); PostGIS geography has no ST_Touches/ST_Contains/ST_Relate

### `tnpoint` (Point family)

- _type-level_: planar 2-D only (no Z) and no geodetic variant; a network point is network-Euclidean and inherits its CRS from the planar road network
- `atGeometry`, `minusGeometry`: a network point is constrained to a 1-D edge; use route filtering
- `atElevation`, `minusElevation`: a network point lies on a planar 2-D road network, with no Z elevation
- `convexHull`: a point's continuous form collapses to trajectory()/stbox()
- `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `translate`, `transscale`: affine transform bypasses the route + fraction (network position) invariant
- `h3_latlng_to_cell`, `setSRID`, `transform`, `transform_gk`, `transformPipeline`: tnpoint's CRS is inherited from the underlying network (the route SRID); the value carries no SRID and is never reprojected (RFC #863 / production guidance)
- `temporal_back`, `temporal_front`, `temporal_overback`, `temporal_overfront`: type is planar 2-D (no Z dimension); the front/back operators act on Z

### `trgeometry` (Extended-shape family)

- _type-level_: 2-D or 3-D Cartesian and no geodetic variant; instants are planar by construction and there is no trgeography type
- `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `translate`, `transscale`: affine transform bypasses the rigid-body invariant (reference geometry + pose)

### `tcbuffer` (Extended-shape family)

- _type-level_: planar 2-D only (no Z) and no geodetic variant; PostGIS operations on circular segments are defined for planar 2-D only
- `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `translate`, `transscale`: affine transform bypasses the center + radius invariant
- `temporal_back`, `temporal_front`, `temporal_overback`, `temporal_overfront`: type is planar 2-D (no Z dimension); the front/back operators act on Z

### `tpose` (Extended-shape family)

- `affine`, `rotate`, `rotateX`, `rotateY`, `rotateZ`, `scale`, `translate`, `transscale`: affine transform bypasses the rigid-body pose invariant
- `centroid`, `convexHull`, `traversedArea`: tpose carries no shape, so there is nothing to sweep

### `tgeometry`

- _type-level_: linear interpolation unsupported (discrete/step only); it would require a morphing function transforming an arbitrary geometry into another between consecutive timestamps, which does not exist. Has a geodetic twin, tgeography

### `tgeography`

- _type-level_: linear interpolation unsupported (discrete/step only); same no-morphing-function limitation as tgeometry, plus geodetic

## Real gaps (methodology-expected, genuinely missing; implement)

- **`tgeogpoint`** (4): `tCentroid`, `timeTiles`, `transform_gk`, `twCentroid`
- **`tnpoint`** (4): `extent`, `makeSimple`, `tprecision`, `tsample`
- **`trgeometry`** (4): `atValues`, `minusValues`, `temporal_out`, `temporal_send`
- **`tcbuffer`** (7): `centroid`, `convexHull`, `splitEachNStboxes`, `splitNStboxes`, `stboxes`, `tprecision`, `tsample`
- **`tpose`** (6): `extent`, `splitEachNStboxes`, `splitNStboxes`, `stboxes`, `tprecision`, `tsample`
