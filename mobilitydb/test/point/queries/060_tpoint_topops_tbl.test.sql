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

DROP INDEX IF EXISTS tbl_tgeompoint_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_topops;
CREATE TABLE test_topops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tgeompoint

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p && temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p @> temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p <@ temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p -|- temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tgeompoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p ~= temp;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tgeompoint', COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b && temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tgeompoint', COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b @> temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tgeompoint', COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b <@ temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tgeompoint', COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b -|- temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tgeompoint', COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b ~= temp;

-------------------------------------------------------------------------------
-- <type> op tgeogpoint

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p && temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p @> temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p <@ temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p -|- temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tgeogpoint', COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p ~= temp;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tgeogpoint', COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b && temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tgeogpoint', COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b @> temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tgeogpoint', COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b <@ temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tgeogpoint', COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b -|- temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tgeogpoint', COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b ~= temp;

-------------------------------------------------------------------------------
--  tgeompoint op <type>

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp && p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp @> p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <@ p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp -|- p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeompoint', 'tstzspan', COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp ~= p;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeompoint', 'stbox', COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp && b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeompoint', 'stbox', COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp @> b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeompoint', 'stbox', COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp <@ b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeompoint', 'stbox', COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp -|- b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeompoint', 'stbox', COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp ~= b;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeompoint', 'tgeompoint', COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------
--  tgeogpoint op <type>

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp && p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp @> p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <@ p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp -|- p;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeogpoint', 'tstzspan', COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp ~= p;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeogpoint', 'stbox', COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp && b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeogpoint', 'stbox', COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp @> b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeogpoint', 'stbox', COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp <@ b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeogpoint', 'stbox', COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp -|- b;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeogpoint', 'stbox', COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp ~= b;

INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_topops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeogpoint', 'tgeogpoint', COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeompoint

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------
-- tgeompoint op <type>

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- tgeogpoint op <type>

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';

UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeompoint

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------
-- tgeompoint op <type>

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- tgeogpoint op <type>

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';

UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_kdtree_idx ON tbl_tgeompoint USING SPGIST(temp tgeompoint_kdtree_ops);
CREATE INDEX tbl_tgeogpoint_kdtree_idx ON tbl_tgeogpoint USING SPGIST(temp tgeogpoint_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op tgeompoint

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeompoint';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeogpoint';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3d, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------
-- tgeompoint op <type>

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- tgeogpoint op <type>

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_tstzspan WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tstzspan';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geodstbox3d WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';

UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_topops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeompoint_kdtree_idx;
DROP INDEX tbl_tgeogpoint_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_topops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_topops;

-------------------------------------------------------------------------------
