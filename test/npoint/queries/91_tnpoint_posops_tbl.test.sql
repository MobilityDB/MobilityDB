﻿-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2021, PostGIS contributors
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

DROP INDEX IF EXISTS tbl_tnpoint_gist_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_tnpointrelativeposops;
CREATE TABLE test_tnpointrelativeposops(
	op char(3),
	leftarg text,
	rightarg text,
	noidx bigint,
	gistidx bigint );

-------------------------------------------------------------------------------

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'geometry', 'tnpoint', count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'npoint', 'tnpoint', count(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestamptz', 'tnpoint', count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestamptz', 'tnpoint', count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestamptz', 'tnpoint', count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestamptz', 'tnpoint', count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'timestampset', 'tnpoint', count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts <<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'timestampset', 'tnpoint', count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts #>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'timestampset', 'tnpoint', count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts &<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'timestampset', 'tnpoint', count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts #&> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'period', 'tnpoint', count(*) FROM tbl_period, tbl_tnpoint WHERE p <<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'period', 'tnpoint', count(*) FROM tbl_period, tbl_tnpoint WHERE p #>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'period', 'tnpoint', count(*) FROM tbl_period, tbl_tnpoint WHERE p &<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'period', 'tnpoint', count(*) FROM tbl_period, tbl_tnpoint WHERE p #&> temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'periodset', 'tnpoint', count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps <<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'periodset', 'tnpoint', count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps #>> temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'periodset', 'tnpoint', count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps &<# temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'periodset', 'tnpoint', count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps #&> temp;

-------------------------------------------------------------------------------

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676);
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tnpoint', 'geometry', count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676);

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<|', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tnpoint', 'npoint', count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tnpoint', 'timestamptz', count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tnpoint', 'timestamptz', count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tnpoint', 'timestamptz', count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tnpoint', 'timestamptz', count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tnpoint', 'timestampset', count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp <<# ts;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tnpoint', 'timestampset', count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp #>> ts;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tnpoint', 'timestampset', count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp &<# ts;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tnpoint', 'timestampset', count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp #&> ts;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tnpoint', 'period', count(*) FROM tbl_tnpoint, tbl_period WHERE temp <<# p;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tnpoint', 'period', count(*) FROM tbl_tnpoint, tbl_period WHERE temp #>> p;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tnpoint', 'period', count(*) FROM tbl_tnpoint, tbl_period WHERE temp &<# p;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tnpoint', 'period', count(*) FROM tbl_tnpoint, tbl_period WHERE temp #&> p;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tnpoint', 'periodset', count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp <<# ps;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tnpoint', 'periodset', count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp #>> ps;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tnpoint', 'periodset', count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp &<# ps;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tnpoint', 'periodset', count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp #&> ps;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '>>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)

SELECT '<<|', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|>>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<|', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '|&>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp;

INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '<<#', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#>>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '&<#', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_tnpointrelativeposops(op, leftarg, rightarg, noidx)
SELECT '#&>', 'tnpoint', 'tnpoint', count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tnpoint_gist_idx ON tbl_tnpoint USING GIST(temp);

-------------------------------------------------------------------------------

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) << temp )
WHERE op = '<<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) >> temp )
WHERE op = '>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &< temp )
WHERE op = '&<' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &> temp )
WHERE op = '&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np << temp )
WHERE op = '<<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np >> temp )
WHERE op = '>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &< temp )
WHERE op = '&<' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &> temp )
WHERE op = '&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <<| temp )
WHERE op = '<<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |>> temp )
WHERE op = '|>>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) &<| temp )
WHERE op = '&<|' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) |&> temp )
WHERE op = '|&>' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np <<| temp )
WHERE op = '<<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np |>> temp )
WHERE op = '|>>' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np &<| temp )
WHERE op = '&<|' and leftarg = 'npoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_npoint, tbl_tnpoint WHERE np |&> temp )
WHERE op = '|&>' and leftarg = 'npoint' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'timestampset' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tnpoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tnpoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tnpoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tnpoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'period' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tnpoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'periodset' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp << ST_SetSRID(g, 5676) )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp >> ST_SetSRID(g, 5676) )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &< ST_SetSRID(g, 5676) )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &> ST_SetSRID(g, 5676) )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <<| ST_SetSRID(g, 5676) )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |>> ST_SetSRID(g, 5676) )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &<| ST_SetSRID(g, 5676) )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_geometry WHERE temp |&> ST_SetSRID(g, 5676) )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp << np )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp >> np )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &< np )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &> np )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp <<| np )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |>> np )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp &<| np )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'npoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_npoint WHERE temp |&> np )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'npoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'timestampset';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'period';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'periodset';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpointrelativeposops
SET gistidx = ( SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tnpoint_gist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_tnpointrelativeposops
WHERE noidx <> gistidx
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------
