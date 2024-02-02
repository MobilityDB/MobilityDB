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

CREATE TABLE test AS
SELECT NULL::ttext FROM generate_series(1, 10);
ANALYZE test;
DROP TABLE test;

CREATE TABLE test AS
SELECT NULL::tfloat UNION SELECT tfloat '1@2000-01-01';
ANALYZE test;
DROP TABLE test;

-------------------------------------------------------------------------------

ANALYZE tbl_tbool_big;
ANALYZE tbl_tint_big;
ANALYZE tbl_tfloat_big;
ANALYZE tbl_ttext_big;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tint_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_rtree_idx;
DROP INDEX IF EXISTS tbl_ttext_big_rtree_idx;

DROP INDEX IF EXISTS tbl_tbool_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tint_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_ttext_big_quadtree_idx;

DROP INDEX IF EXISTS tbl_tbool_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tint_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_ttext_big_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_idxops;
CREATE TABLE test_idxops(
  op CHAR(3),
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------
-- Without Index
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tstzspan;
SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;

-------------------------------------------------------------------------------

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbool', 'tstzspan', COUNT(*) FROM tbl_tbool_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp < tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<=', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp <= tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp > tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>=', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp >= tbool '[true@2001-01-01, true@2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp @> tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp <@ tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp ~= tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp -|- tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp <<# tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp &<# tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp #>> tbool '[true@2001-01-01, true@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tbool', 'tbool', COUNT(*) FROM tbl_tbool_big WHERE temp #&> tbool '[true@2001-01-01, true@2001-02-01]';

-------------------------------------------------------------------------------

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp && intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp @> intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp <@ intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp ~= intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp -|- intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp << intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp &< intspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp >> intspan '[97,100]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'intspan', COUNT(*) FROM tbl_tint_big WHERE temp &> intspan '[97,100]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tstzspan', COUNT(*) FROM tbl_tint_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp && tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp @> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp <@ tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp ~= tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
-- INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
-- SELECT '-|-', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp -|- tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp << tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp &< tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp >> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp &> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp <<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp &<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp #>> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tbox', COUNT(*) FROM tbl_tint_big WHERE temp #&> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp < tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<=', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp <= tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp > tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>=', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp >= tint '[1@2001-01-01, 10@2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]';
-- INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
-- SELECT '-|-', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tint', 'tint', COUNT(*) FROM tbl_tint_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]';

-------------------------------------------------------------------------------

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp && floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp @> floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp <@ floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp ~= floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp -|- floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp << floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp &< floatspan '[1,3]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp >> floatspan '[97,100]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'floatspan', COUNT(*) FROM tbl_tfloat_big WHERE temp &> floatspan '[97,100]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tstzspan', COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp && tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp @> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp << tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp &< tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp >> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp &> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tbox', COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp < tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<=', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp <= tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp > tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>=', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp >= tfloat '[1@2001-01-01, 10@2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'tfloat', 'tfloat', COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]';

-------------------------------------------------------------------------------

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ttext', 'tstzspan', COUNT(*) FROM tbl_ttext_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<=', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '>=', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-02-01]';

INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&&', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '@>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<@', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '~=', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '-|-', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '<<#', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '&<#', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#>>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-02-01]';
INSERT INTO test_idxops(op, leftarg, rightarg, no_idx)
SELECT '#&>', 'ttext', 'ttext', COUNT(*) FROM tbl_ttext_big WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-02-01]';

-------------------------------------------------------------------------------
-- R-Tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_big_rtree_idx ON tbl_tbool_big USING GIST(temp);
CREATE INDEX tbl_tint_big_rtree_idx ON tbl_tint_big USING GIST(temp);
CREATE INDEX tbl_tfloat_big_rtree_idx ON tbl_tfloat_big USING GIST(temp);
CREATE INDEX tbl_ttext_big_rtree_idx ON tbl_ttext_big USING GIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tstzspan;
SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;

-------------------------------------------------------------------------------

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp < tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp > tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp >= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && intspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> intspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ intspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= intspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- intspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << intspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< intspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> intspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> intspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tbox';
-- UPDATE test_idxops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp < tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp > tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tint' AND rightarg = 'tint';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tint';
-- UPDATE test_idxops
-- SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && floatspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> floatspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ floatspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= floatspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- floatspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << floatspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< floatspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> floatspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> floatspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp < tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp > tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'ttext' AND rightarg = 'ttext';

UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

-- Test the commutator for the selectivity
SELECT COUNT(*) FROM tbl_tint_big WHERE tint '[1@2001-01-01, 10@2001-02-01]' << temp;
SELECT COUNT(*) FROM tbl_tint_big WHERE tint '[1@2001-01-01, 10@2001-02-01]' &< temp;

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_big_rtree_idx;
DROP INDEX tbl_tint_big_rtree_idx;
DROP INDEX tbl_tfloat_big_rtree_idx;
DROP INDEX tbl_ttext_big_rtree_idx;

-------------------------------------------------------------------------------
-- Quad-tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_big_quadtree_idx ON tbl_tbool_big USING SPGIST(temp);
CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING SPGIST(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING SPGIST(temp);
CREATE INDEX tbl_ttext_big_quadtree_idx ON tbl_ttext_big USING SPGIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tstzspan;
SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;

-------------------------------------------------------------------------------

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp < tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp > tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp >= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && intspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> intspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ intspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= intspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- intspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << intspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< intspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> intspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> intspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tbox';
-- UPDATE test_idxops
-- SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp < tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp > tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tint' AND rightarg = 'tint';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tint';
-- UPDATE test_idxops
-- SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && floatspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> floatspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ floatspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= floatspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- floatspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << floatspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< floatspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> floatspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> floatspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp < tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp > tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'ttext' AND rightarg = 'ttext';

UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_big_quadtree_idx;
DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;
DROP INDEX tbl_ttext_big_quadtree_idx;

-------------------------------------------------------------------------------
-- K-d tree Index
-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_big_kdtree_idx ON tbl_tbool_big USING SPGIST(temp tbool_kdtree_ops);
CREATE INDEX tbl_tint_big_kdtree_idx ON tbl_tint_big USING SPGIST(temp tint_kdtree_ops);
CREATE INDEX tbl_tfloat_big_kdtree_idx ON tbl_tfloat_big USING SPGIST(temp tfloat_kdtree_ops);
CREATE INDEX tbl_ttext_big_kdtree_idx ON tbl_ttext_big USING SPGIST(temp ttext_kdtree_ops);

-------------------------------------------------------------------------------

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp < tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp > tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp >= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tbool' AND rightarg = 'tbool';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp @> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <@ tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp ~= tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp -|- tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp <<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp &<# tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #>> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tbool' AND rightarg = 'tbool';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tbool_big WHERE temp #&> tbool '[true@2001-01-01, true@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tbool' AND rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && intspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> intspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ intspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= intspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- intspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << intspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< intspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> intspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> intspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'intspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tbox';
-- UPDATE test_idxops
-- SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tbox 'TBOXINT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tbox';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp < tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp > tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tint' AND rightarg = 'tint';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tint' AND rightarg = 'tint';
-- UPDATE test_idxops
-- SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]' )
-- WHERE op = '-|-' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tint' AND rightarg = 'tint';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tint_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tint' AND rightarg = 'tint';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && floatspan '[1,3]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> floatspan '[1,3]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ floatspan '[1,3]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= floatspan '[1,3]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- floatspan '[1,3]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << floatspan '[1,3]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< floatspan '[1,3]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> floatspan '[97,100]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> floatspan '[97,100]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'floatspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tstzspan '[2001-11-01, 2001-12-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tbox';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tbox 'TBOXFLOAT XT([1,50],[2001-01-01,2001-02-01])' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tbox';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp < tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp > tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'tfloat' AND rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> tstzspan '[2001-01-01,2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'tstzspan';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '>=' AND leftarg = 'ttext' AND rightarg = 'ttext';

UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&&' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '@>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<@' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '~=' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '-|-' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '<<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '&<#' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#>>' AND leftarg = 'ttext' AND rightarg = 'ttext';
UPDATE test_idxops
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_ttext_big WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-02-01]' )
WHERE op = '#&>' AND leftarg = 'ttext' AND rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX tbl_tbool_big_kdtree_idx;
DROP INDEX tbl_tint_big_kdtree_idx;
DROP INDEX tbl_tfloat_big_kdtree_idx;
DROP INDEX tbl_ttext_big_kdtree_idx;

-------------------------------------------------------------------------------
-- TEST THE EQUIVALENCE
-------------------------------------------------------------------------------

SELECT * FROM test_idxops
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY op, leftarg, rightarg;

DROP TABLE test_idxops;

-------------------------------------------------------------------------------
-- Index support functions

CREATE INDEX tbl_tint_big_rtree_idx ON tbl_tint_big USING GIST(temp);
CREATE INDEX tbl_tfloat_big_rtree_idx ON tbl_tfloat_big USING GIST(temp);

-- EXPLAIN ANALYZE
SELECT temp |=| intspan '[90,100]'::tbox FROM tbl_tint_big ORDER BY 1 LIMIT 3;
SELECT temp |=| tint '[1@2001-06-01, 2@2001-07-01]' FROM tbl_tint_big ORDER BY 1 LIMIT 3;

WITH test AS (
  SELECT temp |=| floatspan '[100,100]'::tbox AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;
WITH test AS (
  SELECT temp |=| tfloat '[1.5@2001-06-01, 2.5@2001-07-01]' AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;

DROP INDEX tbl_tint_big_rtree_idx;
DROP INDEX tbl_tfloat_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING SPGIST(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING SPGIST(temp);

-- EXPLAIN ANALYZE
SELECT temp |=| intspan '[90,100]'::tbox FROM tbl_tint_big ORDER BY 1 LIMIT 3;
SELECT temp |=| tint '[1@2001-06-01, 2@2001-07-01]' FROM tbl_tint_big ORDER BY 1 LIMIT 3;

WITH test AS (
  SELECT temp |=| floatspan '[100,100]'::tbox AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;
WITH test AS (
  SELECT temp |=| tfloat '[1.5@2001-06-01, 2.5@2001-07-01]' AS distance FROM tbl_tfloat_big ORDER BY 1 LIMIT 3 )
SELECT round(distance::numeric, 6) FROM test;

DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;

-------------------------------------------------------------------------------
-- Coverage of all the same and order by logic in SP-GiST indexes

CREATE TABLE tbl_tfloat_big_allthesame AS SELECT k, tfloat_seq(5.0, t) AS temp FROM tbl_tstzspan_big;
CREATE INDEX tbl_tfloat_big_allthesame_quadtree_idx ON tbl_tfloat_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tfloat_big_allthesame;

-- TODO
-- SELECT COUNT(*) FROM tbl_tfloat_big_allthesame WHERE temp && 5.0;
-- SELECT k FROM tbl_tfloat_big_allthesame ORDER BY temp |=| 5.0, k LIMIT 3;

DROP TABLE tbl_tfloat_big_allthesame;

-------------------------------------------------------------------------------
