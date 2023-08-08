-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

-------------------------------------------------------------------------------
-- File span_ops.c
-- Tests of operators for span types.
-------------------------------------------------------------------------------

ANALYZE tbl_intspan_big;
ANALYZE tbl_floatspan_big;

DROP INDEX IF EXISTS tbl_intspan_big_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_big_rtree_idx;

DROP INDEX IF EXISTS tbl_intspan_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_big_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_idxops;
CREATE TABLE test_idxops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- Without Index
-------------------------------------------------------------------------------

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i @> 50;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i -|- 50;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i << 15;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i &< 15;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i >> 85;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspan', 'int', COUNT(*) FROM tbl_intspan_big WHERE i &> 85;

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i && intspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i @> intspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i <@ intspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i -|- intspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i << intspan '[15, 25]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i &< intspan '[15, 25]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i >> intspan '[85, 95]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan_big WHERE i &> intspan '[85, 95]';

SELECT i <-> 101 FROM tbl_intspan_big ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspan_big ORDER BY 1 LIMIT 3;

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f @> 50.0;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f -|- 50.0;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f << 15.0;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f &< 15.0;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f >> 85.0;
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspan', 'float', COUNT(*) FROM tbl_floatspan_big WHERE f &> 85.0;

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f && floatspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f @> floatspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f <@ floatspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f -|- floatspan '[45, 55]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f << floatspan '[15, 25]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f &< floatspan '[15, 25]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f >> floatspan '[85, 95]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspan', 'floatspan', COUNT(*) FROM tbl_floatspan_big WHERE f &> floatspan '[85, 95]';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;

-------------------------------------------------------------------------------
-- R-tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_intspan_big_rtree_idx ON tbl_intspan_big USING GIST(i);
CREATE INDEX tbl_floatspan_big_rtree_idx ON tbl_floatspan_big USING GIST(f);

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> 50 )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- 50 )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << 15 )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< 15 )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> 85 )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> 85 )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'int';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i && intspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> intspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i <@ intspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- intspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << intspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< intspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> intspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> intspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspan';

SELECT i <-> 101 FROM tbl_intspan_big ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspan_big ORDER BY 1 LIMIT 3;

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> 50.0 )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- 50.0 )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << 15.0 )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< 15.0 )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> 85.0 )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> 85.0 )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'float';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f && floatspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> floatspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f <@ floatspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- floatspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << floatspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< floatspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> floatspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> floatspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspan_big_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_big_rtree_idx;

-------------------------------------------------------------------------------
-- Quad-tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_intspan_big_quadtree_idx ON tbl_intspan_big USING SPGIST(i);
CREATE INDEX tbl_floatspan_big_quadtree_idx ON tbl_floatspan_big USING SPGIST(f);

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> 50 )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- 50 )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << 15 )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< 15 )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> 85 )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> 85 )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'int';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i && intspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> intspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i <@ intspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- intspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << intspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< intspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> intspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> intspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspan';

SELECT i <-> 101 FROM tbl_intspan_big ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspan_big ORDER BY 1 LIMIT 3;

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> 50.0 )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- 50.0 )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << 15.0 )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< 15.0 )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> 85.0 )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> 85.0 )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'float';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f && floatspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> floatspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f <@ floatspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- floatspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << floatspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< floatspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> floatspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> floatspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspan_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_big_quadtree_idx;

-------------------------------------------------------------------------------
-- K-d Tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_intspan_big_kdtree_idx ON tbl_intspan_big USING SPGIST(i intspan_kdtree_ops);
CREATE INDEX tbl_floatspan_big_kdtree_idx ON tbl_floatspan_big USING SPGIST(f floatspan_kdtree_ops);

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> 50 )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- 50 )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << 15 )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< 15 )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> 85 )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'int';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> 85 )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'int';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i && intspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i @> intspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i <@ intspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i -|- intspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i << intspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &< intspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i >> intspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan_big WHERE i &> intspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'intspan' AND rightarg = 'intspan';

SELECT i <-> 101 FROM tbl_intspan_big ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspan_big ORDER BY 1 LIMIT 3;

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> 50.0 )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- 50.0 )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << 15.0 )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< 15.0 )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> 85.0 )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'float';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> 85.0 )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'float';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f && floatspan '[45, 55]' )
WHERE op = '&&' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f @> floatspan '[45, 55]' )
WHERE op = '@>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f <@ floatspan '[45, 55]' )
WHERE op = '<@' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f -|- floatspan '[45, 55]' )
WHERE op = '-|-' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f << floatspan '[15, 25]' )
WHERE op = '<<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &< floatspan '[15, 25]' )
WHERE op = '&<' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f >> floatspan '[85, 95]' )
WHERE op = '>>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_floatspan_big WHERE f &> floatspan '[85, 95]' )
WHERE op = '&>' AND leftarg = 'floatspan' AND rightarg = 'floatspan';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspan_big ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspan_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_big_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_idxops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-- DROP TABLE test_idxops;

-------------------------------------------------------------------------------
