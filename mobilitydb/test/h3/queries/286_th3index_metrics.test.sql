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
-- cellArea(th3index), th3CellAreaKm2 and th3CellAreaRads2 — lift_with_const
-------------------------------------------------------------------------------

-- cellArea answers square metres, the unit the DggsCellOps cell_area slot
-- declares, and the two H3 units are the ones libh3 names
SELECT round(startValue(cellArea(th3index '831c02fffffffff@2001-01-01'))::numeric, 1);
SELECT round(startValue(th3CellAreaKm2(th3index '831c02fffffffff@2001-01-01'))::numeric, 7);
SELECT round(startValue(th3CellAreaRads2(th3index '831c02fffffffff@2001-01-01'))::numeric, 9);

-- The three answer one quantity in three units
SELECT startValue(cellArea(th3index '831c02fffffffff@2001-01-01'))
  = startValue(th3CellAreaKm2(th3index '831c02fffffffff@2001-01-01')) * 1e6;
SELECT startValue(th3CellAreaRads2(th3index '831c02fffffffff@2001-01-01'))
  < startValue(th3CellAreaKm2(th3index '831c02fffffffff@2001-01-01'));

-- Sequence form
SELECT th3CellAreaKm2(th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]')
  IS NOT NULL;
SELECT cellArea(th3index
  '[831c02fffffffff@2001-01-01, 8a2a1072b59ffff@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
-- th3EdgeLengthKm, th3EdgeLengthM and th3EdgeLengthRads — lift_with_const
-------------------------------------------------------------------------------

-- The three name the units libh3 names
SELECT round(startValue(th3EdgeLengthKm(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))::numeric, 7);
SELECT round(startValue(th3EdgeLengthM(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))::numeric, 4);
SELECT round(startValue(th3EdgeLengthRads(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))::numeric, 9);

-- The three answer one quantity in three units
SELECT startValue(th3EdgeLengthM(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))
  = startValue(th3EdgeLengthKm(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01'))) * 1000;
SELECT startValue(th3EdgeLengthRads(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')))
  < startValue(th3EdgeLengthKm(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')));

-------------------------------------------------------------------------------
-- greatCircleDistanceKm, greatCircleDistanceM and greatCircleDistanceRads —
-- binary_synced
-------------------------------------------------------------------------------

-- Distance from a point to itself is 0
SELECT greatCircleDistanceKm(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01');

-- One degree of longitude at the equator, in the three units libh3 names.
-- The radian answer is pi/180 exactly, which is the oracle for the trio.
SELECT round(startValue(greatCircleDistanceKm(
  tgeogpoint 'POINT(0 0)@2001-01-01',
  tgeogpoint 'POINT(1 0)@2001-01-01'))::numeric, 6);
SELECT round(startValue(greatCircleDistanceM(
  tgeogpoint 'POINT(0 0)@2001-01-01',
  tgeogpoint 'POINT(1 0)@2001-01-01'))::numeric, 3);
SELECT round(startValue(greatCircleDistanceRads(
  tgeogpoint 'POINT(0 0)@2001-01-01',
  tgeogpoint 'POINT(1 0)@2001-01-01'))::numeric, 9)
  = round((pi() / 180)::numeric, 9);

-- The three answer one quantity in three units
SELECT startValue(greatCircleDistanceM(
    tgeogpoint 'POINT(0 0)@2001-01-01', tgeogpoint 'POINT(1 0)@2001-01-01'))
  = startValue(greatCircleDistanceKm(
    tgeogpoint 'POINT(0 0)@2001-01-01', tgeogpoint 'POINT(1 0)@2001-01-01')) * 1000;

-- Sequence form
SELECT greatCircleDistanceKm(
  tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
  tgeogpoint '[POINT(2.35 48.86)@2001-01-01, POINT(-73.96 40.78)@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
