-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.8 Metrics — three lifts.
--
-- All three depend on adapters in h3_adapter.c (h3_cell_area_meos,
-- h3_edge_length_meos, h3_gs_great_circle_distance_meos). The unit
-- argument is parsed once at the MEOS entry point via
-- `h3_unit_from_cstring`, which IS implemented (Plan §6.4) — so
-- the unit-validation tests below succeed independently of the
-- per-instant adapters being filled in.

-------------------------------------------------------------------------------
-- cellArea(th3index) — lift_with_const
-------------------------------------------------------------------------------

-- cellArea answers square metres for every grid
SELECT round(startValue(cellArea(th3index '831c02fffffffff@2001-01-01'))::numeric, 1);

-- A finer cell covers less ground than a coarser one
SELECT startValue(cellArea(th3index '8a2a1072b59ffff@2001-01-01'))
  < startValue(cellArea(th3index '831c02fffffffff@2001-01-01'));

-- Sequence form
SELECT cellArea(th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
-- th3EdgeLength — lift_with_const
-------------------------------------------------------------------------------

-- The length of a directed edge, in metres
SELECT round(startValue(th3EdgeLength(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))::numeric, 4);

-- An edge of a finer cell is shorter
SELECT startValue(th3EdgeLength(th3CellsToDirectedEdge(
    th3index '890326b8853ffff@2001-01-01',
    th3index '890326b8857ffff@2001-01-01')))
  < startValue(th3EdgeLength(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')));

-------------------------------------------------------------------------------
-- greatCircleDistance — binary_synced
-------------------------------------------------------------------------------

-- Distance from a point to itself is 0
SELECT greatCircleDistance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01');

-- One degree of longitude at the equator, in metres
SELECT round(startValue(greatCircleDistance(
  tgeogpoint 'POINT(0 0)@2001-01-01',
  tgeogpoint 'POINT(1 0)@2001-01-01'))::numeric, 3);

-- The same span of longitude is shorter away from the equator
SELECT startValue(greatCircleDistance(
    tgeogpoint 'POINT(0 60)@2001-01-01', tgeogpoint 'POINT(1 60)@2001-01-01'))
  < startValue(greatCircleDistance(
    tgeogpoint 'POINT(0 0)@2001-01-01', tgeogpoint 'POINT(1 0)@2001-01-01'));

-- Sequence form
SELECT greatCircleDistance(
  tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
  tgeogpoint '[POINT(2.35 48.86)@2001-01-01, POINT(-73.96 40.78)@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
