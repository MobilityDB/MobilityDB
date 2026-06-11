# `mobilitydb_pcl`: PCL bridge for pgPointCloud + MobilityDB

Optional PostgreSQL extension that exposes round-trip conversion between
pgPointCloud `pcpatch` values and the [Point Cloud
Library](https://pointclouds.org/)'s native PCD format. Hands `pcpatch`
points off as a PCL-readable bytea so external PCL pipelines (filtering,
registration, segmentation) can ingest them directly.

## Surface

- `pcpatch_to_pcd(pcpatch) → bytea`.
- `pcpatch_from_pcd(bytea, pcid int) → pcpatch`.
- Schema-aware point-type selection: XYZ-only schemas use
  `pcl::PointXYZ`, XYZ+Intensity uses `pcl::PointXYZI`, XYZ+RGB uses
  `pcl::PointXYZRGB` (RGB takes precedence when both are present).
  The reverse parse is type-agnostic via `pcl::PCLPointCloud2` field
  introspection. Round-trip preserves all dim values bit-exactly.
- PC_NONE only on the read side. PC_DIMENSIONAL / PC_LAZPERF inputs to
  `pcpatch_to_pcd` raise a clear "compression not supported" error;
  MobilityDB's `tpcpatch` storage normalises to PC_NONE so the
  practical impact is small.

## Build & install

```bash
cd contrib/pcl
cmake -B build
cmake --build build -j
sudo cmake --install build
# Database-side: load mobilitydb (which pulls in postgis + pointcloud
# via its CASCADE chain on a POINTCLOUD=ON build), then mobilitydb_pcl:
psql -d <yourdb> -c "CREATE EXTENSION mobilitydb CASCADE"
psql -d <yourdb> -c "CREATE EXTENSION mobilitydb_pcl"
```

Requires (full dependency chain, validated end-to-end at both build
and `CREATE EXTENSION` time):

| Layer | Dependency | Where checked |
|---|---|---|
| Build | PCL ≥ 1.12 with the `common io filters registration features search kdtree` components (Ubuntu 24.04: `apt install libpcl-dev`) | `find_package(PCL 1.12 REQUIRED COMPONENTS …)` aborts with a clear error if any component is missing |
| Build | The surrounding MobilityDB tree built with `-DPOINTCLOUD=ON` (so that `pointcloud-pg/lib/libpc.a` exists) | The contrib's `CMakeLists.txt` checks the file exists and aborts with a `FATAL_ERROR` naming the missing path otherwise |
| Build | PostgreSQL development headers (`pg_config` on `PATH`) and `libpq`, `libxml2` | `find_program(PostgreSQL_PG_CONFIG REQUIRED)` and `find_library(pq REQUIRED)` |
| Runtime | Target database has the `pointcloud` extension loaded | `mobilitydb_pcl.control` declares `requires = 'pointcloud, mobilitydb'`; PostgreSQL refuses `CREATE EXTENSION` if missing |
| Runtime | Target database has the `mobilitydb` extension loaded | Same `requires =` clause |
| Runtime | The loaded `mobilitydb` was itself built with `-DPOINTCLOUD=ON` (otherwise the `tpcpatch` type is absent and the bridge's convenience wrappers can't reference it) | The first SQL block in `mobilitydb_pcl--1.4.0.sql` is a `DO $$ … RAISE EXCEPTION … $$` that probes `pg_type` for `tpcpatch` and aborts with a clear actionable error message if the probe fails |

A single `CREATE EXTENSION mobilitydb_pcl CASCADE` on a fresh database
will, in order, install `postgis`, `pointcloud`, `mobilitydb`, then
`mobilitydb_pcl`. If the running install of `mobilitydb` was built
without `-DPOINTCLOUD=ON`, the pre-flight `RAISE EXCEPTION` aborts the
transaction cleanly with the upgrade instructions.

## Usage

```sql
-- Single patch round-trip.
WITH p AS (
  SELECT PC_Patch(ARRAY[
    PC_MakePoint(1, ARRAY[1.0, 2.0, 3.0]),
    PC_MakePoint(1, ARRAY[4.0, 5.0, 6.0])
  ]) AS patch
)
SELECT PC_AsText(pcpatch_from_pcd(pcpatch_to_pcd(patch), 1)) FROM p;
-- {"pcid":1,"pts":[[1,2,3],[4,5,6]]}

-- Roundtrip an entire tpcpatch trajectory through PCL.
SELECT tpcpatch_from_pcd_array(
         (SELECT array_agg(pcpatch_to_pcd(valueN(traj, n)) ORDER BY n)
          FROM trajectories, generate_series(1, numInstants(traj)) n
          WHERE id = 42),
         (SELECT array_agg(timestampN(traj, n) ORDER BY n)
          FROM trajectories, generate_series(1, numInstants(traj)) n
          WHERE id = 42),
         1
       )
FROM trajectories WHERE id = 42;
```

## Filter and registration surface

| Function | Description |
|---|---|
| `pcpatch_to_pcd` (X/Y/Z, +Intensity, +RGB) | PCD encode |
| `pcpatch_from_pcd` (X/Y/Z, +Intensity, +RGB) | PCD decode |
| `pcpatch_voxel_grid(pcpatch, leaf double precision) → pcpatch` | Voxel-grid downsample. Preserves Intensity / RGB |
| `pcpatch_sor(pcpatch, k int, stddev_mul double precision) → pcpatch` | Statistical outlier removal. Preserves Intensity / RGB |
| `pcpatch_icp(source pcpatch, target pcpatch, max_iter int, max_corr double) → double[]` | ICP registration. Returns `[tx, ty, tz, qw, qx, qy, qz, fitness]` |
| `pcpatch_gicp(source pcpatch, target pcpatch, max_iter int, max_corr double) → double[]` | PCL Generalized ICP (Mahalanobis cost over per-point local covariances). Robust on non-planar / noisy surfaces. Same return shape as `pcpatch_icp` |
| `pcpatch_normals(pcpatch, k int DEFAULT 10) → double[]` | PCL `NormalEstimation<PointXYZ, Normal>`. Returns flat `[nx_0, ny_0, nz_0, curv_0, …]` of length 4 × npoints. A planar fixture round-trips to `(0,0,1)` with zero curvature |
