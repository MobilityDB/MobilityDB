﻿-------------------------------------------------------------------------------
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

DROP INDEX IF EXISTS tbl_tnpoint_rtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tnpoint_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tnpoint_posops;
CREATE TABLE test_tnpoint_posops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'npoint', 'tnpoint', COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzset', 'tnpoint', COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts <<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzset', 'tnpoint', COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzset', 'tnpoint', COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts &<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzset', 'tnpoint', COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #&> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p <<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p &<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'tnpoint', COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #&> temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspanset', 'tnpoint', COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps <<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspanset', 'tnpoint', COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #>> temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspanset', 'tnpoint', COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps &<# temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspanset', 'tnpoint', COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676);

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tnpoint', 'npoint', COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tstzset', COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp <<# ts;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tstzset', COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #>> ts;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tstzset', COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp &<# ts;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tstzset', COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #&> ts;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# p;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> p;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# p;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tstzspan', COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> p;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tstzspanset', COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp <<# ps;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tstzspanset', COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #>> ps;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tstzspanset', COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp &<# ps;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tstzspanset', COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #&> ps;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)

SELECT '<<|', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tnpoint_posops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp )
WHERE op = '<<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp )
WHERE op = '>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp )
WHERE op = '&<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp )
WHERE op = '&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp )
WHERE op = '<<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp )
WHERE op = '>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp )
WHERE op = '&<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp )
WHERE op = '&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp )
WHERE op = '<<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp )
WHERE op = '|>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp )
WHERE op = '&<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp )
WHERE op = '|&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp )
WHERE op = '<<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp )
WHERE op = '|>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp )
WHERE op = '&<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp )
WHERE op = '|&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'tstzset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676) )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676) )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676) )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676) )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676) )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676) )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676) )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676) )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzset';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tnpoint_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp )
WHERE op = '<<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp )
WHERE op = '>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp )
WHERE op = '&<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp )
WHERE op = '&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp )
WHERE op = '<<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp )
WHERE op = '>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp )
WHERE op = '&<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp )
WHERE op = '&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp )
WHERE op = '<<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp )
WHERE op = '|>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp )
WHERE op = '&<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp )
WHERE op = '|&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp )
WHERE op = '<<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp )
WHERE op = '|>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp )
WHERE op = '&<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp )
WHERE op = '|&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'tstzset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676) )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676) )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676) )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676) )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676) )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676) )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676) )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676) )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzset';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_kdtree_idx ON tbl_tnpoint USING SPGIST(temp tnpoint_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp )
WHERE op = '<<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp )
WHERE op = '>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp )
WHERE op = '&<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp )
WHERE op = '&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp )
WHERE op = '<<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp )
WHERE op = '>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp )
WHERE op = '&<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp )
WHERE op = '&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp )
WHERE op = '<<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp )
WHERE op = '|>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp )
WHERE op = '&<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp )
WHERE op = '|&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp )
WHERE op = '<<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp )
WHERE op = '|>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp )
WHERE op = '&<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp )
WHERE op = '|&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'tstzset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset, tbl_tnpoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'tstzset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspan' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tnpoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspan' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset, tbl_tnpoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'tstzspanset' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676) )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676) )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676) )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676) )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676) )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676) )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676) )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676) )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzset';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspan';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspan';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tstzspanset';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_posops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tnpoint_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tnpoint_posops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_tnpoint_posops;

-------------------------------------------------------------------------------
