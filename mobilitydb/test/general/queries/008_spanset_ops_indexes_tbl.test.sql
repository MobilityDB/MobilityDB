-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- Tests of operators for span set types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_spansetops;
CREATE TABLE test_spansetops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'integer', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'integer', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss << t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss << t2.ss;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss << t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss << t2.ss;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'integer', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'integer', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &< t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &< t2.ss;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &< t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &< t2.ss;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'integer', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'integer', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss >> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss >> t2.ss;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss >> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss >> t2.ss;

-------------------------------------------------------------------------------

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'integer', 'intspanset', COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'integer', COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &> t2.i;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &> t2.ss;

INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'float', 'floatspanset', COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.ss;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'float', COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &> t2.f;
INSERT INTO test_spansetops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'floatspanset', 'floatspanset', COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &> t2.ss;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(ss);
CREATE INDEX tbl_floatspanset_rtree_idx ON tbl_floatspanset USING GIST(ss);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.ss )
WHERE op = '<<' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss << t2.ss )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.ss )
-- WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'floatspanset';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss << t2.f )
-- WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss << t2.ss )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.ss )
WHERE op = '&<' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &< t2.ss )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.ss )
-- WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'floatspanset';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &< t2.f )
-- WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &< t2.ss )
-- WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.ss )
WHERE op = '>>' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss >> t2.ss )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.ss )
-- WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'floatspanset';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss >> t2.f )
-- WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss >> t2.ss )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.ss )
WHERE op = '&>' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &> t2.i )
WHERE op =  '&>' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &> t2.ss )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.ss )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'floatspanset';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &> t2.f )
-- WHERE op =  '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
-- UPDATE test_spansetops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &> t2.ss )
-- WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_rtree_idx;
DROP INDEX tbl_floatspanset_rtree_idx;

CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(ss);
CREATE INDEX tbl_floatspanset_quadtree_idx ON tbl_floatspanset USING SPGIST(ss);

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i << t2.ss )
WHERE op = '<<' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss << t2.i )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss << t2.ss )
WHERE op = '<<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f << t2.ss )
WHERE op = '<<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss << t2.f )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss << t2.ss )
WHERE op = '<<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &< t2.ss )
WHERE op = '&<' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &< t2.i )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &< t2.ss )
WHERE op = '&<' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &< t2.ss )
WHERE op = '&<' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &< t2.f )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &< t2.ss )
WHERE op = '&<' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i >> t2.ss )
WHERE op = '>>' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss >> t2.i )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss >> t2.ss )
WHERE op = '>>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f >> t2.ss )
WHERE op = '>>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss >> t2.f )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss >> t2.ss )
WHERE op = '>>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i &> t2.ss )
WHERE op = '&>' AND leftarg = 'integer' AND rightarg = 'intspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.ss &> t2.i )
WHERE op =  '&>' AND leftarg = 'intspanset' AND rightarg = 'integer';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.ss &> t2.ss )
WHERE op = '&>' AND leftarg = 'intspanset' AND rightarg = 'intspanset';

UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f &> t2.ss )
WHERE op = '&>' AND leftarg = 'float' AND rightarg = 'floatspanset';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.ss &> t2.f )
WHERE op =  '&>' AND leftarg = 'floatspanset' AND rightarg = 'float';
UPDATE test_spansetops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.ss &> t2.ss )
WHERE op = '&>' AND leftarg = 'floatspanset' AND rightarg = 'floatspanset';

-------------------------------------------------------------------------------

DROP INDEX tbl_intspanset_quadtree_idx;
DROP INDEX tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_spansetops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_spansetops;

-------------------------------------------------------------------------------
