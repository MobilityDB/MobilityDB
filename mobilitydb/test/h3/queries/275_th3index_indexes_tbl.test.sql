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
-- Build a th3index "big" table of 100 rows of valid H3 cells. Each row's
-- cell is the resolution-7 H3 cell that contains a deterministic point in
-- the Brussels region; timestamps span 2001 at four-day intervals. The
-- indexes test only validates access-method equivalence (rtree / quadtree /
-- kdtree counts must match no-index counts) — the absolute counts and the
-- specific cell identifiers don't matter to the invariant.
--
-- This fixture deliberately does not use `(tbigint)::th3index` binary
-- coercion: most int64 values are not valid H3 cells, and the on-disk
-- bbox representation differs (tbigint embeds TBox, th3index embeds
-- STBox), so a shallow relabel produces semantically and structurally
-- invalid th3index values.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_big;
CREATE TABLE tbl_th3index_big(k int PRIMARY KEY, temp th3index);

INSERT INTO tbl_th3index_big
SELECT k,
  h3_latlng_to_cell(
    tgeompoint(
      ST_SetSRID(
        ST_Point(4.30 + (k % 10) * 0.01, 50.80 + ((k / 10) % 10) * 0.01),
        4326
      ),
      timestamptz '2001-01-01' + (k * interval '4 days')
    ),
    7
  )
FROM generate_series(1, 100) k;

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
SELECT '&&',  'th3index',   COUNT(*) FROM tbl_th3index_big WHERE temp && th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]';
INSERT INTO test_idxops_th3index(op, rightarg, no_idx)
SELECT '~=',  'th3index',   COUNT(*) FROM tbl_th3index_big WHERE temp ~= th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]';

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

UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET rtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '~=' AND rightarg = 'th3index';

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

UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '~=' AND rightarg = 'th3index';

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

UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp && th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '&&' AND rightarg = 'th3index';
UPDATE test_idxops_th3index SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_th3index_big WHERE temp ~= th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-02-01]' ) WHERE op = '~=' AND rightarg = 'th3index';

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
