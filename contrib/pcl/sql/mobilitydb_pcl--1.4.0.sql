/* mobilitydb_pcl extension — PCL bridge for pgPointCloud + MobilityDB */

/* Pre-flight: tpcpatch is the temporal point-cloud type that several
 * convenience wrappers below reference. It only exists when MobilityDB
 * was built with -DPOINTCLOUD=ON. Surface a clear actionable error
 * here rather than the cryptic 'type tpcpatch does not exist' that
 * the first CREATE FUNCTION would otherwise raise. */
DO $$
BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'tpcpatch') THEN
    RAISE EXCEPTION 'mobilitydb_pcl requires MobilityDB built with '
      '-DPOINTCLOUD=ON: the tpcpatch type is missing from this '
      'database. Rebuild MobilityDB with that flag, sudo cmake '
      '--install the result, then DROP EXTENSION mobilitydb CASCADE '
      'and CREATE EXTENSION mobilitydb CASCADE before retrying '
      'CREATE EXTENSION mobilitydb_pcl.';
  END IF;
END $$;

CREATE FUNCTION pcpatch_to_pcd(pcpatch)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'pcpatch_to_pcd'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_to_pcd(pcpatch) IS
  'Serialize a pcpatch to a PCL PCD-format bytea (X, Y, Z dims; binary PCD).';

CREATE FUNCTION pcd_to_pcpatch_wkb_hex(bytea, pcid integer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'pcpatch_from_pcd'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcd_to_pcpatch_wkb_hex(bytea, integer) IS
  'Convert a PCL PCD bytea into pcpatch WKB hex; cast result with ::pcpatch.';

-- Convenience wrapper that returns a real pcpatch via the WKB cast.
CREATE FUNCTION pcpatch_from_pcd(bytea, pcid integer)
  RETURNS pcpatch
  AS $$
    SELECT cast(cast(pcd_to_pcpatch_wkb_hex($1, $2) AS text) AS pcpatch)
  $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_from_pcd(bytea, integer) IS
  'Deserialize a PCL PCD bytea into a pcpatch (uncompressed) with given pcid.';

/* Convenience: per-instant PCD array round-trip for tpcpatch. */

CREATE FUNCTION tpcpatch_to_pcd_array(tpcpatch)
  RETURNS bytea[]
  AS $$
    SELECT array_agg(pcpatch_to_pcd(valueN($1, n)) ORDER BY n)
    FROM generate_series(1, numInstants($1)) n
  $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcpatch_from_pcd_array(bytea[], timestamptz[], pcid integer)
  RETURNS tpcpatch
  AS $$
    SELECT tpcpatchSeq(array_agg(
      tpcpatch(pcpatch_from_pcd($1[i], $3), $2[i])
      ORDER BY i
    ))
    FROM generate_subscripts($1, 1) i
    WHERE array_length($1, 1) = array_length($2, 1)
  $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

/* PCL filters as SQL functions. */

CREATE FUNCTION pcpatch_voxel_grid_wkb_hex(pcpatch, leaf double precision)
  RETURNS text
  AS 'MODULE_PATHNAME', 'pcpatch_voxel_grid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pcpatch_voxel_grid(pcpatch, leaf double precision)
  RETURNS pcpatch
  AS $$ SELECT pcpatch_voxel_grid_wkb_hex($1, $2)::pcpatch $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_voxel_grid(pcpatch, double precision) IS
  'Down-sample a pcpatch using PCL''s VoxelGrid filter; leaf is the voxel edge length in the schema''s spatial units. Returns a new pcpatch with X/Y/Z only — non-spatial dims are dropped.';

CREATE FUNCTION pcpatch_sor_wkb_hex(pcpatch, k integer, stddev_mul double precision)
  RETURNS text
  AS 'MODULE_PATHNAME', 'pcpatch_sor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pcpatch_sor(pcpatch, k integer DEFAULT 50,
                            stddev_mul double precision DEFAULT 1.0)
  RETURNS pcpatch
  AS $$ SELECT pcpatch_sor_wkb_hex($1, $2, $3)::pcpatch $$
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_sor(pcpatch, integer, double precision) IS
  'PCL StatisticalOutlierRemoval — drop points whose mean distance to their k nearest neighbours falls outside (mean ± stddev_mul × stddev). k=50, stddev_mul=1.0 are sensible defaults. Preserves Intensity / RGB.';

CREATE FUNCTION pcpatch_icp(source pcpatch, target pcpatch,
                            max_iterations integer DEFAULT 50,
                            max_correspondence_distance double precision DEFAULT 1.0)
  RETURNS double precision[]
  AS 'MODULE_PATHNAME', 'pcpatch_icp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_icp(pcpatch, pcpatch, integer, double precision) IS
  'PCL IterativeClosestPoint registration. Returns an 8-element double[]:
   [tx, ty, tz, qw, qx, qy, qz, fitness]. Source and target must share pcid.
   Compose into a MobilityDB pose with pose_make_3d(r[1], r[2], r[3], r[4], r[5], r[6], r[7], srid).';

CREATE FUNCTION pcpatch_gicp(source pcpatch, target pcpatch,
                             max_iterations integer DEFAULT 50,
                             max_correspondence_distance double precision DEFAULT 1.0)
  RETURNS double precision[]
  AS 'MODULE_PATHNAME', 'pcpatch_gicp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_gicp(pcpatch, pcpatch, integer, double precision) IS
  'PCL GeneralizedIterativeClosestPoint registration. Same return shape
   as pcpatch_icp but uses a Mahalanobis cost over per-point local
   surface covariances. More robust on non-planar / noisy surfaces.';

CREATE FUNCTION pcpatch_normals(pcpatch, k integer DEFAULT 10)
  RETURNS double precision[]
  AS 'MODULE_PATHNAME', 'pcpatch_normals'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

COMMENT ON FUNCTION pcpatch_normals(pcpatch, integer) IS
  'PCL NormalEstimation<PointXYZ, Normal>: per-point surface normal +
   curvature, computed from the k nearest neighbours. Returns a flat
   double precision[] of length 4 * npoints, laid out as
   [nx_0, ny_0, nz_0, curv_0, nx_1, ny_1, nz_1, curv_1, ...].
   Use cases: oriented-surface change detection across repeat surveys,
   Potree --normal export, photogrammetric mesh seeding. For large
   patches (>50k points) consider pcpatch_voxel_grid first.';
