-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-----------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_ts2cell_rtree_idx;
DROP INDEX IF EXISTS tbl_ts2cell_quadtree_idx;
DROP INDEX IF EXISTS tbl_ts2cell_kdtree_idx;

-----------------------------------------------------------------------------

DROP TABLE IF EXISTS test_ts2cell_ops;
CREATE TABLE test_ts2cell_ops(
  op char(3),
  leftarg text,
  rightarg text,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-----------------------------------------------------------------------------
-- Counts with no index

INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t && temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp && t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b && temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp && b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp && t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t @> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp @> t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b @> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp @> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <@ temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <@ t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <@ temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <@ b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t ~= temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp ~= t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b ~= temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp ~= b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t -|- temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp -|- t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b -|- temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp -|- b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp -|- t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <<# temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t &<# temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #>> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tstzspan', 'ts2cell', COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #&> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <<# t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp &<# t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #>> t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ts2cell', 'tstzspan', COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #&> t;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<# temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<# temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #>> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #&> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<# b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<# b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #>> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #&> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<# t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<# t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #>> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #&> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b << temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &< temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b >> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<| temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<| temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |>> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'stbox', 'ts2cell', COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |&> temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp << b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &< b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp >> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<| b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<| b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |>> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'ts2cell', 'stbox', COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |&> b;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp << t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &< t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp >> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '<<|', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<| t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '&<|', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<| t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|>>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |>> t2.temp;
INSERT INTO test_ts2cell_ops(op, leftarg, rightarg, no_idx)
SELECT '|&>', 'ts2cell', 'ts2cell', COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |&> t2.temp;

-----------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_rtree_idx ON tbl_ts2cell USING GIST(temp);

-----------------------------------------------------------------------------
-- Counts with the rtree index

UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp && b )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp << b )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';

-----------------------------------------------------------------------------

DROP INDEX tbl_ts2cell_rtree_idx;
CREATE INDEX tbl_ts2cell_quadtree_idx ON tbl_ts2cell USING SPGIST(temp);

-----------------------------------------------------------------------------
-- Counts with the quadtree index

UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp && b )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp << b )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';

-----------------------------------------------------------------------------

DROP INDEX tbl_ts2cell_quadtree_idx;
CREATE INDEX tbl_ts2cell_kdtree_idx ON tbl_ts2cell USING SPGIST(temp ts2cell_kdtree_ops);

-----------------------------------------------------------------------------
-- Counts with the kdtree index

UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t && temp )
WHERE op = '&&' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp && t )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp && b )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan, tbl_ts2cell WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'tstzspan' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_tstzspan WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'tstzspan';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<# temp )
WHERE op = '<<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<# temp )
WHERE op = '&<#' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #>> temp )
WHERE op = '#>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b #&> temp )
WHERE op = '#&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<# b )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<# b )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #>> b )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp #&> b )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b << temp )
WHERE op = '<<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &< temp )
WHERE op = '&<' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b >> temp )
WHERE op = '>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &> temp )
WHERE op = '&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b <<| temp )
WHERE op = '<<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b &<| temp )
WHERE op = '&<|' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |>> temp )
WHERE op = '|>>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_geodstbox_s2cell, tbl_ts2cell WHERE b |&> temp )
WHERE op = '|&>' AND leftarg = 'stbox' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp << b )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &< b )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp >> b )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &> b )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp <<| b )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp &<| b )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |>> b )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell, tbl_geodstbox_s2cell WHERE temp |&> b )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'stbox';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';
UPDATE test_ts2cell_ops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'ts2cell' AND rightarg = 'ts2cell';

-----------------------------------------------------------------------------

DROP INDEX tbl_ts2cell_kdtree_idx;

-----------------------------------------------------------------------------

SELECT * FROM test_ts2cell_ops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR
  no_idx <> kdtree_idx OR no_idx IS NULL OR rtree_idx IS NULL OR
  quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

-----------------------------------------------------------------------------

-- the operators whose count reaches some row, so the agreement above is not vacuous
SELECT COUNT(*) AS ops, COUNT(*) FILTER (WHERE no_idx > 0) AS ops_reaching_rows
FROM test_ts2cell_ops;

DROP TABLE test_ts2cell_ops;

-----------------------------------------------------------------------------
