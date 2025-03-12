-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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

DROP INDEX IF EXISTS tbl_tgeometry3D_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeometry3D_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeometry3D_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_topops3d;
CREATE TABLE test_topops3d(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- <type> op tgeometry3D

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tgeometry3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t && temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tgeometry3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t @> temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tgeometry3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t <@ temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tgeometry3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t -|- temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tgeometry3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t ~= temp;

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tgeometry3D', COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) && temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tgeometry3D', COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) @> temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tgeometry3D', COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) <@ temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tgeometry3D', COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) -|- temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tgeometry3D', COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) ~= temp;

-------------------------------------------------------------------------------
-- <type> op tgeography3D

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'tgeography3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t && temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'tgeography3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t @> temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'tgeography3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t <@ temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'tgeography3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t -|- temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'tgeography3D', COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t ~= temp;

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'tgeography3D', COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) && temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'tgeography3D', COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) @> temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'tgeography3D', COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) <@ temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'tgeography3D', COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) -|- temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'tgeography3D', COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) ~= temp;

-------------------------------------------------------------------------------
--  tgeometry3D op <type>

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeometry3D', 'tstzspan', COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp && t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeometry3D', 'tstzspan', COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeometry3D', 'tstzspan', COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeometry3D', 'tstzspan', COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp -|- t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeometry3D', 'tstzspan', COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp ~= t;

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeometry3D', 'stbox', COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp && setSRID(b, 3812);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeometry3D', 'stbox', COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp @> setSRID(b, 3812);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeometry3D', 'stbox', COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp <@ setSRID(b, 3812);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeometry3D', 'stbox', COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp -|- setSRID(b, 3812);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeometry3D', 'stbox', COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp ~= setSRID(b, 3812);

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeometry3D', 'tgeometry3D', COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp && t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeometry3D', 'tgeometry3D', COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeometry3D', 'tgeometry3D', COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeometry3D', 'tgeometry3D', COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeometry3D', 'tgeometry3D', COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------
--  tgeography3D op <type>

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeography3D', 'tstzspan', COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp && t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeography3D', 'tstzspan', COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeography3D', 'tstzspan', COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeography3D', 'tstzspan', COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp -|- t;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeography3D', 'tstzspan', COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp ~= t;

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeography3D', 'stbox', COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp && setSRID(b, 7844);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeography3D', 'stbox', COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp @> setSRID(b, 7844);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeography3D', 'stbox', COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp <@ setSRID(b, 7844);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeography3D', 'stbox', COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp -|- setSRID(b, 7844);
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeography3D', 'stbox', COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp ~= setSRID(b, 7844);

INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tgeography3D', 'tgeography3D', COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp && t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tgeography3D', 'tgeography3D', COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tgeography3D', 'tgeography3D', COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tgeography3D', 'tgeography3D', COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_topops3d(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tgeography3D', 'tgeography3D', COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_rtree_idx ON tbl_tgeometry3D USING GIST(temp);
CREATE INDEX tbl_tgeography3D_rtree_idx ON tbl_tgeography3D USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeometry3D

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- <type> op tgeography3D

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------
-- tgeometry3D op <type>

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp && setSRID(b, 3812) )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp @> setSRID(b, 3812) )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp <@ setSRID(b, 3812) )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp -|- setSRID(b, 3812) )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp ~= setSRID(b, 3812) )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- tgeography3D op <type>

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp && setSRID(b, 7844) )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp @> setSRID(b, 7844) )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp <@ setSRID(b, 7844) )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp -|- setSRID(b, 7844) )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp ~= setSRID(b, 7844) )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeometry3D_rtree_idx;
DROP INDEX tbl_tgeography3D_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_quadtree_idx ON tbl_tgeometry3D USING SPGIST(temp);
CREATE INDEX tbl_tgeography3D_quadtree_idx ON tbl_tgeography3D USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeometry3D

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- <type> op tgeography3D

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------
-- tgeometry3D op <type>

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp && setSRID(b, 3812) )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp @> setSRID(b, 3812) )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp <@ setSRID(b, 3812) )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp -|- setSRID(b, 3812) )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp ~= setSRID(b, 3812) )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- tgeography3D op <type>

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp && setSRID(b, 7844) )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp @> setSRID(b, 7844) )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp <@ setSRID(b, 7844) )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp -|- setSRID(b, 7844) )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp ~= setSRID(b, 7844) )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeometry3D_quadtree_idx;
DROP INDEX tbl_tgeography3D_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_kdtree_idx ON tbl_tgeometry3D USING SPGIST(temp tgeometry_kdtree_ops);
CREATE INDEX tbl_tgeography3D_kdtree_idx ON tbl_tgeography3D USING SPGIST(temp tgeography_kdtree_ops);

-------------------------------------------------------------------------------
-- <type> op tgeometry3D

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeometry3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeometry3D';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_stbox3D, tbl_tgeometry3D WHERE setSRID(b, 3812) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- <type> op tgeography3D

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_tgeography3D WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'tgeography3D';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox3D, tbl_tgeography3D WHERE setSRID(b, 7844) ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------
-- tgeometry3D op <type>

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp && setSRID(b, 3812) )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp @> setSRID(b, 3812) )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp <@ setSRID(b, 3812) )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp -|- setSRID(b, 3812) )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D, tbl_stbox3D WHERE temp ~= setSRID(b, 3812) )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeometry3D' AND rightarg = 'tgeometry3D';

-------------------------------------------------------------------------------
-- tgeography3D op <type>

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tstzspan';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp && setSRID(b, 7844) )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp @> setSRID(b, 7844) )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp <@ setSRID(b, 7844) )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp -|- setSRID(b, 7844) )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D, tbl_geodstbox3D WHERE temp ~= setSRID(b, 7844) )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'stbox';

UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';
UPDATE test_topops3d
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tgeography3D t1, tbl_tgeography3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeography3D' AND rightarg = 'tgeography3D';

-------------------------------------------------------------------------------

DROP INDEX tbl_tgeometry3D_kdtree_idx;
DROP INDEX tbl_tgeography3D_kdtree_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_topops3d
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_topops3d;

-------------------------------------------------------------------------------
