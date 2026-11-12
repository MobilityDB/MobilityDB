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

-- Ever/always spatial relationships for temporal pose chains. Every one of
-- them converts its operands to the temporal pose the chain composes to and
-- that one to the temporal geometry point of its position, so a chain answers
-- exactly as that point does. The family declares the matrix its target
-- declares: eContains/aContains and eCovers/aCovers take the geometry first
-- only, and eTouches/aTouches have no direction between two chains.

-- Test for NULL inputs since the functions are STRICT
SELECT eContains(NULL::geometry, tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01');
SELECT eContains(geometry 'Point(1 1)', NULL::tposechain);
SELECT aDwithin(NULL::tposechain, geometry 'Point(1 1)', 1.0);
SELECT eDwithin(tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01', NULL::geometry, 1.0);

-------------------------------------------------------------------------------
-- Every declared cell agrees with the cast written out by hand, on the
-- default linear interpolation
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]' AS seq1,
         tposechain '[PoseChain(Pose(Point(5 5), 0.2))@2001-01-01, PoseChain(Pose(Point(7 7), 0.2))@2001-01-03]' AS seq2,
         geometry 'Polygon((0 0,0 4,4 4,4 0,0 0))' AS region,
         geometry 'Point(9 9)' AS far_point
)
SELECT
  eContains(region, seq1) = eContains(region, seq1::tpose::tgeompoint) AS ec_geo_pc,
  eCovers(region, seq1) = eCovers(region, seq1::tpose::tgeompoint) AS ecv_geo_pc,
  eDisjoint(far_point, seq1) = eDisjoint(far_point, seq1::tpose::tgeompoint) AS edj_geo_pc,
  eDisjoint(seq1, far_point) = eDisjoint(seq1::tpose::tgeompoint, far_point) AS edj_pc_geo,
  eDisjoint(seq1, seq2) = eDisjoint(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint) AS edj_pc_pc,
  eIntersects(region, seq1) = eIntersects(region, seq1::tpose::tgeompoint) AS ei_geo_pc,
  eIntersects(seq1, region) = eIntersects(seq1::tpose::tgeompoint, region) AS ei_pc_geo,
  eIntersects(seq1, seq2) = eIntersects(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint) AS ei_pc_pc,
  eTouches(region, seq1) = eTouches(region, seq1::tpose::tgeompoint) AS etc_geo_pc,
  eTouches(seq1, region) = eTouches(seq1::tpose::tgeompoint, region) AS etc_pc_geo,
  eDwithin(region, seq1, 1.0) = eDwithin(region, seq1::tpose::tgeompoint, 1.0) AS edw_geo_pc,
  eDwithin(seq1, region, 1.0) = eDwithin(seq1::tpose::tgeompoint, region, 1.0) AS edw_pc_geo,
  eDwithin(seq1, seq2, 1.0) = eDwithin(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint, 1.0) AS edw_pc_pc
FROM t;

WITH t AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]' AS seq1,
         tposechain '[PoseChain(Pose(Point(5 5), 0.2))@2001-01-01, PoseChain(Pose(Point(7 7), 0.2))@2001-01-03]' AS seq2,
         geometry 'Polygon((0 0,0 4,4 4,4 0,0 0))' AS region,
         geometry 'Point(9 9)' AS far_point
)
SELECT
  aContains(region, seq1) = aContains(region, seq1::tpose::tgeompoint) AS ac_geo_pc,
  aCovers(region, seq1) = aCovers(region, seq1::tpose::tgeompoint) AS acv_geo_pc,
  aDisjoint(far_point, seq1) = aDisjoint(far_point, seq1::tpose::tgeompoint) AS adj_geo_pc,
  aDisjoint(seq1, far_point) = aDisjoint(seq1::tpose::tgeompoint, far_point) AS adj_pc_geo,
  aDisjoint(seq1, seq2) = aDisjoint(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint) AS adj_pc_pc,
  aIntersects(region, seq1) = aIntersects(region, seq1::tpose::tgeompoint) AS ai_geo_pc,
  aIntersects(seq1, region) = aIntersects(seq1::tpose::tgeompoint, region) AS ai_pc_geo,
  aIntersects(seq1, seq2) = aIntersects(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint) AS ai_pc_pc,
  aTouches(region, seq1) = aTouches(region, seq1::tpose::tgeompoint) AS atc_geo_pc,
  aTouches(seq1, region) = aTouches(seq1::tpose::tgeompoint, region) AS atc_pc_geo,
  aDwithin(region, seq1, 1.0) = aDwithin(region, seq1::tpose::tgeompoint, 1.0) AS adw_geo_pc,
  aDwithin(seq1, region, 1.0) = aDwithin(seq1::tpose::tgeompoint, region, 1.0) AS adw_pc_geo,
  aDwithin(seq1, seq2, 1.0) = aDwithin(seq1::tpose::tgeompoint, seq2::tpose::tgeompoint, 1.0) AS adw_pc_pc
FROM t;

-------------------------------------------------------------------------------
-- A chain is located at the pose its composition gives: the chain below has
-- its first joint inside the square and composes to Point(10 0), outside it
-------------------------------------------------------------------------------

SELECT eIntersects(tposechain 'PoseChain(Pose(Point(1 1), 0), Pose(Point(9 -1), 0))@2001-01-01',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eDisjoint(tposechain 'PoseChain(Pose(Point(1 1), 0), Pose(Point(9 -1), 0))@2001-01-01',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eDwithin(tposechain 'PoseChain(Pose(Point(1 1), 0), Pose(Point(9 -1), 0))@2001-01-01',
  geometry 'Point(10 0)', 0.5);

-------------------------------------------------------------------------------
-- A linear sequence is answered along its trajectory; the step sequence with
-- the same instants is the control
-------------------------------------------------------------------------------

SELECT eIntersects(tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eIntersects(tposechain '{PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03}',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT aDisjoint(geometry 'Point(9 9)',
  tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]');
SELECT eDwithin(tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]',
  geometry 'Point(9 9)', 1.0);

-------------------------------------------------------------------------------
-- Direct values on single-instant operands
-------------------------------------------------------------------------------

SELECT eContains(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))',
  tposechain 'PoseChain(Pose(Point(0 0), 0.1))@2001-01-01');
SELECT eCovers(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))',
  tposechain 'PoseChain(Pose(Point(0 0), 0.1))@2001-01-01');
SELECT eTouches(tposechain 'PoseChain(Pose(Point(0 0), 0.1))@2001-01-01',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eIntersects(tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01',
  tposechain 'PoseChain(Pose(Point(1 1), 0.5))@2001-01-01');
SELECT eDwithin(tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01',
  tposechain 'PoseChain(Pose(Point(1 2), 0.5))@2001-01-01', 2.0);

-------------------------------------------------------------------------------
-- Under an R-tree index the ever intersects and dwithin answer the rows a
-- sequential scan gives, from a plan that probes the index
-------------------------------------------------------------------------------

CREATE FUNCTION test_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;

DROP TABLE IF EXISTS tbl_tposechain_rel;
CREATE TABLE tbl_tposechain_rel AS
SELECT k, tposechain(posechain(ARRAY[pose(ST_Point(k % 10, k / 10), 0),
  pose(ST_Point(1, 0), 0)]), timestamptz '2001-01-01') AS temp
FROM generate_series(0, 99) AS k;
CREATE INDEX tbl_tposechain_rel_rtree_idx ON tbl_tposechain_rel USING GIST(temp);
ANALYZE tbl_tposechain_rel;

SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) FROM tbl_tposechain_rel
WHERE eIntersects(temp, geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))');
SELECT COUNT(*) FROM tbl_tposechain_rel
WHERE eDwithin(temp, geometry 'Point(5 5)', 1.5);
RESET enable_indexscan;
RESET enable_bitmapscan;

SET enable_seqscan = off;
SELECT COUNT(*) FROM tbl_tposechain_rel
WHERE eIntersects(temp, geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))');
SELECT COUNT(*) FROM tbl_tposechain_rel
WHERE eDwithin(temp, geometry 'Point(5 5)', 1.5);
SELECT test_probes_index('SELECT k FROM tbl_tposechain_rel '
  'WHERE eIntersects(temp, geometry ''Polygon((0 0,0 3,3 3,3 0,0 0))'')');
SELECT test_probes_index('SELECT k FROM tbl_tposechain_rel '
  'WHERE eDwithin(temp, geometry ''Point(5 5)'', 1.5)');
RESET enable_seqscan;

DROP TABLE tbl_tposechain_rel;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
