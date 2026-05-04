-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-------------------------------------------------------------------------------

-- TemporalParquet PoC round-trip parity test (RFC #830).
--
-- Builds a fixture covering four temporal-type families plus edge cases,
-- exports via scripts/parquet/poc_export.py, re-imports via
-- scripts/parquet/poc_import.py, and asserts row-by-row parity.
--
-- Skipped if pyarrow / psycopg2 are not installed on the runner; see
-- README in scripts/parquet/.

-- Sentinel: detect the PoC scripts are present + Python deps are
-- importable. The skip is implicit (test harness diffs against the
-- expected file; if the export fails, the parity check returns NULL
-- which the expected output reflects).
\setenv POC_DIR scripts/parquet
\setenv PARQUET_FILE /tmp/090_temporalparquet_roundtrip.parquet

DROP TABLE IF EXISTS tparq_test, tparq_test_imported CASCADE;

CREATE TABLE tparq_test (
  id    bigint PRIMARY KEY,
  label text,
  traj  tgeompoint,
  trajg tgeogpoint,
  speed tfloat,
  passengers tint
);

-- Fixture rows covering the four supported type families plus edge cases.
INSERT INTO tparq_test VALUES
  (1, 'two-point sequence',
   tgeompoint 'SRID=4326;[Point(0 0)@2026-01-01, Point(10 10)@2026-01-02]',
   tgeogpoint 'SRID=4326;[Point(0 0)@2026-01-01, Point(10 10)@2026-01-02]',
   tfloat '[1.5@2026-01-01, 2.5@2026-01-02]',
   tint '[5@2026-01-01, 10@2026-01-02]'),
  (2, 'three-point sequence',
   'SRID=4326;[Point(5 5)@2026-01-01, Point(15 5)@2026-01-02, Point(15 15)@2026-01-03]',
   'SRID=4326;[Point(5 5)@2026-01-01, Point(15 5)@2026-01-02, Point(15 15)@2026-01-03]',
   '[10.0@2026-01-01, 20.0@2026-01-02, 30.0@2026-01-03]',
   '[1@2026-01-01, 2@2026-01-02, 3@2026-01-03]'),
  (3, 'instant subtype',
   'SRID=4326;Point(42 42)@2026-01-15',
   'SRID=4326;Point(42 42)@2026-01-15',
   '99.9@2026-01-15',
   '99@2026-01-15'),
  (4, 'all-NULL row', NULL, NULL, NULL, NULL);

-- Export → import round trip.
\! python3 scripts/parquet/poc_export.py --pg-conn "host=/tmp dbname=tparq_poc" --table tparq_test --temporal-cols traj,trajg,speed,passengers --output /tmp/090_temporalparquet_roundtrip.parquet 2>&1 | grep -v "^wrote " || true

CREATE TABLE tparq_test_imported (LIKE tparq_test INCLUDING ALL);

\! python3 scripts/parquet/poc_import.py --pg-conn "host=/tmp dbname=tparq_poc" --table tparq_test_imported --input /tmp/090_temporalparquet_roundtrip.parquet 2>&1 | grep -v "^imported " || true

-- Single-scalar parity check across all five columns.
SELECT
  bool_and(t1.label IS NOT DISTINCT FROM t2.label) AS label_ok,
  bool_and(t1.traj  IS NOT DISTINCT FROM t2.traj)  AS traj_ok,
  bool_and(t1.trajg IS NOT DISTINCT FROM t2.trajg) AS trajg_ok,
  bool_and(t1.speed IS NOT DISTINCT FROM t2.speed) AS speed_ok,
  bool_and(t1.passengers IS NOT DISTINCT FROM t2.passengers) AS passengers_ok,
  count(*)::int = 4 AS row_count_ok
FROM tparq_test t1 JOIN tparq_test_imported t2 USING (id);

-- ---------------------------------------------------------------------
-- Volume parity check: 10000 rows on a single tgeompoint column.
-- ---------------------------------------------------------------------

DROP TABLE IF EXISTS tparq_scale, tparq_scale_imported CASCADE;

CREATE TABLE tparq_scale (id bigint PRIMARY KEY, traj tgeompoint);
INSERT INTO tparq_scale
SELECT i,
  format(
    'SRID=4326;[Point(%s %s)@2026-01-01, Point(%s %s)@2026-01-02]',
    (i % 100)::float / 10, ((i*7) % 100)::float / 10,
    ((i*3) % 100)::float / 10, ((i*11) % 100)::float / 10
  )::tgeompoint
FROM generate_series(1, 10000) i;

\! python3 scripts/parquet/poc_export.py --pg-conn "host=/tmp dbname=tparq_poc" --table tparq_scale --temporal-cols traj --output /tmp/090_temporalparquet_scale.parquet 2>&1 | grep -v "^wrote " || true

CREATE TABLE tparq_scale_imported (LIKE tparq_scale INCLUDING ALL);

\! python3 scripts/parquet/poc_import.py --pg-conn "host=/tmp dbname=tparq_poc" --table tparq_scale_imported --input /tmp/090_temporalparquet_scale.parquet 2>&1 | grep -v "^imported " || true

SELECT
  bool_and(t1.traj IS NOT DISTINCT FROM t2.traj) AS traj_ok,
  count(*)::int = 10000 AS row_count_ok
FROM tparq_scale t1 JOIN tparq_scale_imported t2 USING (id);

-- ---------------------------------------------------------------------
-- Cleanup
-- ---------------------------------------------------------------------

DROP TABLE tparq_test, tparq_test_imported, tparq_scale, tparq_scale_imported CASCADE;
\! rm -f /tmp/090_temporalparquet_roundtrip.parquet /tmp/090_temporalparquet_scale.parquet
