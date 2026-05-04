-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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
-- Build a th3index "big" table from the existing tbl_tbigint_big fixture.
-- The binary-coercion cast preserves values byte-for-byte.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_big;
CREATE TABLE tbl_th3index_big AS
  SELECT k, temp::th3index AS temp FROM tbl_tbigint_big;

ANALYZE tbl_th3index_big;

SELECT COUNT(*) FROM tbl_th3index_big;

-------------------------------------------------------------------------------
-- Baseline (sequential scan) row counts per operator family.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_idxops_th3index;
CREATE TABLE test_idxops_th3index(
  op CHAR(3),
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '&&',  'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '@>',  'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '<@',  'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '-|-', 'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '<<#', 'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '#>>', 'tstzspan',   COUNT(*) FROM tbl_th3index_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]';

INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '&&',  'th3index',   COUNT(*) FROM tbl_th3index_big WHERE temp && (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index;
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '~=',  'th3index',   COUNT(*) FROM tbl_th3index_big WHERE temp ~= (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index;

-------------------------------------------------------------------------------
-- GiST (rtree) index
-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_big_rtree_idx ON tbl_th3index_big USING GIST(temp);

UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '&&'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '@>'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '<@'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '-|-' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '<<#' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '#>>' AND rightarg = 'tstzspan';

UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '~=' AND rightarg = 'th3index';

DROP INDEX tbl_th3index_big_rtree_idx;

-------------------------------------------------------------------------------
-- SP-GiST quadtree index
-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_big_quadtree_idx ON tbl_th3index_big USING SPGIST(temp);

UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '&&'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '@>'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '<@'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '-|-' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '<<#' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '#>>' AND rightarg = 'tstzspan';

UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '~=' AND rightarg = 'th3index';

DROP INDEX tbl_th3index_big_quadtree_idx;

-------------------------------------------------------------------------------
-- SP-GiST kdtree index
-------------------------------------------------------------------------------

CREATE INDEX tbl_th3index_big_kdtree_idx ON tbl_th3index_big USING SPGIST(temp th3index_kdtree_ops);

UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '&&'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '@>'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )   WHERE op = '<@'  AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '-|-' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '<<#' AND rightarg = 'tstzspan';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )  WHERE op = '#>>' AND rightarg = 'tstzspan';

UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= (tbigint '[1@2001-01-01, 100@2001-02-01]')::th3index ) WHERE op = '~=' AND rightarg = 'th3index';

DROP INDEX tbl_th3index_big_kdtree_idx;

-------------------------------------------------------------------------------
-- Every index variant must return the same row-counts as the sequential scan.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM test_idxops_th3index
  WHERE no_idx IS DISTINCT FROM rtree_idx
     OR no_idx IS DISTINCT FROM quadtree_idx
     OR no_idx IS DISTINCT FROM kdtree_idx;

-------------------------------------------------------------------------------
-- Cleanup
-------------------------------------------------------------------------------

DROP TABLE test_idxops_th3index;
DROP TABLE tbl_th3index_big;

-------------------------------------------------------------------------------
