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

DROP INDEX IF EXISTS test_tnpoint_rtree_idx;

DROP INDEX IF EXISTS test_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_tnpoint_boundboxops;
CREATE TABLE test_tnpoint_boundboxops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tnpoint

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'geometry', 'tnpoint', COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) -|- temp;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestamptz', 'tnpoint', COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t -|- temp;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'timestampset', 'tnpoint', COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'timestampset', 'tnpoint', COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'timestampset', 'tnpoint', COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'timestampset', 'tnpoint', COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'timestampset', 'tnpoint', COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts -|- temp;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'period', 'tnpoint', COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'period', 'tnpoint', COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'period', 'tnpoint', COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'period', 'tnpoint', COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'period', 'tnpoint', COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p -|- temp;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'periodset', 'tnpoint', COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'periodset', 'tnpoint', COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'periodset', 'tnpoint', COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'periodset', 'tnpoint', COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'periodset', 'tnpoint', COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps -|- temp;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tnpoint', COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) && temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tnpoint', COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) @> temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tnpoint', COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) <@ temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tnpoint', COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) ~= temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tnpoint', COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) -|- temp;

-------------------------------------------------------------------------------
--  tnpoint op <type>

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp &&  ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp @>  ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <@  ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp ~=  ST_SetSRID(g, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'geometry', COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp -|-  ST_SetSRID(g, 5676);

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp && t;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'timestamptz', COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp -|- t;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'timestampset', COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp && ts;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'timestampset', COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'timestampset', COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'timestampset', COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'timestampset', COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp -|- ts;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'period', COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp && p;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'period', COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp @> p;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'period', COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp <@ p;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'period', COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp ~= p;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'period', COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp -|- p;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'periodset', COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp && ps;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'periodset', COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp @> ps;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'periodset', COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'periodset', COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'periodset', COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp -|- ps;

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'stbox', COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp && SetSRID(b, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'stbox', COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp @> SetSRID(b, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'stbox', COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp <@ SetSRID(b, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'stbox', COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp ~= SetSRID(b, 5676);
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'stbox', COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp -|- SetSRID(b, 5676);

INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_tnpoint_boundboxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tnpoint', 'tnpoint', COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp -|- t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX test_tnpoint_rtree_idx ON tbl_tnpoint USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tnpoint

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) && temp )
WHERE op = '&&' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) @> temp )
WHERE op = '@>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts -|- temp )
WHERE op = '-|-' and leftarg = 'timestampset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p -|- temp )
WHERE op = '-|-' and leftarg = 'period' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps -|- temp )
WHERE op = '-|-' and leftarg = 'periodset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------
-- tnpoint op <type>

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp && ST_SetSRID(g, 5676) )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp @> ST_SetSRID(g, 5676) )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <@ ST_SetSRID(g, 5676) )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp ~= ST_SetSRID(g, 5676) )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp -|- ST_SetSRID(g, 5676) )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'timestampset';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'period';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'periodset';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp && SetSRID(b, 5676) )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp @> SetSRID(b, 5676) )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp <@ SetSRID(b, 5676) )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp ~= SetSRID(b, 5676) )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp -|- SetSRID(b, 5676) )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'stbox';

UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

DROP INDEX test_tnpoint_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX test_tnpoint_quadtree_idx ON tbl_tnpoint USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tnpoint

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) && temp )
WHERE op = '&&' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) @> temp )
WHERE op = '@>' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'geometry' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geometry, tbl_tnpoint WHERE ST_SetSRID(g, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'geometry' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestamptz, tbl_tnpoint WHERE t -|- temp )
WHERE op = '-|-' and leftarg = 'timestamptz' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_timestampset, tbl_tnpoint WHERE ts -|- temp )
WHERE op = '-|-' and leftarg = 'timestampset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_period, tbl_tnpoint WHERE p -|- temp )
WHERE op = '-|-' and leftarg = 'period' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_periodset, tbl_tnpoint WHERE ps -|- temp )
WHERE op = '-|-' and leftarg = 'periodset' and rightarg = 'tnpoint';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tnpoint WHERE SetSRID(b, 5676) -|- temp )
WHERE op = '-|-' and leftarg = 'stbox' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------
-- tnpoint op <type>

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp && ST_SetSRID(g, 5676) )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp @> ST_SetSRID(g, 5676) )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp <@ ST_SetSRID(g, 5676) )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp ~= ST_SetSRID(g, 5676) )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'geometry';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_geometry WHERE temp -|- ST_SetSRID(g, 5676) )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'geometry';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'timestamptz';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'timestamptz';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'timestampset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'timestampset';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'period';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'period';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'periodset';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'periodset';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp && SetSRID(b, 5676) )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp @> SetSRID(b, 5676) )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp <@ SetSRID(b, 5676) )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp ~= SetSRID(b, 5676) )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'stbox';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint, tbl_stbox WHERE temp -|- SetSRID(b, 5676) )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'stbox';

UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tnpoint' and rightarg = 'tnpoint';
UPDATE test_tnpoint_boundboxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' and leftarg = 'tnpoint' and rightarg = 'tnpoint';

-------------------------------------------------------------------------------

SELECT * FROM test_tnpoint_boundboxops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx
ORDER BY op, leftarg, rightarg;

-------------------------------------------------------------------------------

DROP INDEX test_tnpoint_quadtree_idx;
DROP TABLE test_tnpoint_boundboxops;

-------------------------------------------------------------------------------
