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
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-- Verify that GiST / quad-tree / k-d tree operator classes produce results
-- identical to sequential scans for the key topological and ordering operators.
-- The 128_trgeo_topops_tbl test already exercises most operator combinations;
-- this test focuses on confirming that all three index types can be built and
-- that a representative nearest-distance query returns the same count via each.

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_trgeometry_idx;
CREATE TABLE test_trgeometry_idx(
  op    char(3),
  leftarg  text,
  rightarg text,
  no_idx   BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- Sequential-scan baseline
-------------------------------------------------------------------------------

INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'trgeometry', COUNT(*)
  FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'trgeometry', COUNT(*)
  FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'trgeometry', COUNT(*)
  FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp;

INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '&&', 'trgeometry', 'tstzspan', COUNT(*)
  FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '@>', 'trgeometry', 'tstzspan', COUNT(*)
  FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '<@', 'trgeometry', 'tstzspan', COUNT(*)
  FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t;

INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '&&', 'trgeometry', 'trgeometry', COUNT(*)
  FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '@>', 'trgeometry', 'trgeometry', COUNT(*)
  FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_trgeometry_idx(op, leftarg, rightarg, no_idx)
SELECT '<@', 'trgeometry', 'trgeometry', COUNT(*)
  FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp;

-------------------------------------------------------------------------------
-- R-tree (GiST) index
-------------------------------------------------------------------------------

CREATE INDEX tbl_trgeometry_rtree_idx ON tbl_trgeometry2d
  USING gist(temp trgeometry_rtree_ops);

UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp)
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp)
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp)
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';

UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';

UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET rtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';

DROP INDEX tbl_trgeometry_rtree_idx;

-------------------------------------------------------------------------------
-- Quad-tree (SP-GiST) index
-------------------------------------------------------------------------------

CREATE INDEX tbl_trgeometry_quadtree_idx ON tbl_trgeometry2d
  USING spgist(temp trgeometry_quadtree_ops);

UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp)
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp)
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp)
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';

UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';

UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET quadtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';

DROP INDEX tbl_trgeometry_quadtree_idx;

-------------------------------------------------------------------------------
-- K-d tree (SP-GiST) index
-------------------------------------------------------------------------------

CREATE INDEX tbl_trgeometry_kdtree_idx ON tbl_trgeometry2d
  USING spgist(temp trgeometry_kdtree_ops);

UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t && temp)
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t @> temp)
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_tstzspan, tbl_trgeometry2d WHERE t <@ temp)
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'trgeometry';

UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp && t)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp @> t)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d, tbl_tstzspan WHERE temp <@ t)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'tstzspan';

UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp && t2.temp)
WHERE op = '&&' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp @> t2.temp)
WHERE op = '@>' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';
UPDATE test_trgeometry_idx SET kdtree_idx = (
  SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_trgeometry2d t2 WHERE t1.temp <@ t2.temp)
WHERE op = '<@' AND leftarg = 'trgeometry' AND rightarg = 'trgeometry';

DROP INDEX tbl_trgeometry_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_trgeometry_idx
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx
   OR no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_trgeometry_idx;

-------------------------------------------------------------------------------
