-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purjsonb, without fee, and without a written
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

DROP INDEX IF EXISTS tbl_tjsonb_rtree_idx;
DROP INDEX IF EXISTS tbl_tjsonb_quadtree_idx;
DROP INDEX IF EXISTS tbl_tjsonb_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_tjsonb_topops;
CREATE TABLE test_tjsonb_topops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tjsonb

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tjsonb', COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp;

-------------------------------------------------------------------------------
--  tjsonb op <type>

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tjsonb', 'tstzspan', COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t;

INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '#@>', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '<@#', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_tjsonb_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tjsonb', 'tjsonb', COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tjsonb_rtree_idx ON tbl_tjsonb USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_rtree_idx;
CREATE INDEX tbl_tjsonb_quadtree_idx ON tbl_tjsonb USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_quadtree_idx;
CREATE INDEX tbl_tjsonb_kdtree_idx ON tbl_tjsonb USING SPGIST(temp tjsonb_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op tjsonb

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t && temp )
WHERE op = '&&' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t #@> temp )
WHERE op = '#@>' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t <@# temp )
WHERE op = '<@#' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'tstzspan' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tjsonb WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'tstzspan' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------
-- tjsonb op <type>

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp #@> t )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp <@# t )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tstzspan';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tstzspan';

UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #@> t2.temp )
WHERE op = '#@>' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp <@# t2.temp )
WHERE op = '<@#' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tjsonb' and rightarg = 'tjsonb';
UPDATE test_tjsonb_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tjsonb' and rightarg = 'tjsonb';

-------------------------------------------------------------------------------

DROP INDEX tbl_tjsonb_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tjsonb_topops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP TABLE test_tjsonb_topops;

-------------------------------------------------------------------------------
