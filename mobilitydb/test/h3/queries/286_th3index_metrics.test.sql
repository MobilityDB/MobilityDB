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
-- h3_cell_area(th3index, text) — lift_with_const
-------------------------------------------------------------------------------

-- Default unit ('km2')
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01') IS NOT NULL;

-- All three valid area units
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01', 'km2')
  IS NOT NULL;
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01', 'm2')
  IS NOT NULL;
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01', 'rads2')
  IS NOT NULL;

-- Sequence form
SELECT h3_cell_area(th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]', 'km2')
  IS NOT NULL;

-- Length unit on an area function — h3-pg behaviour: error.
/* Errors */
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01', 'km');
-- Invalid unit string
SELECT h3_cell_area(th3index '590464338553208831@2001-01-01', 'lightyears');

-------------------------------------------------------------------------------
-- h3_edge_length(th3index, text) — lift_with_const
-------------------------------------------------------------------------------

-- Default unit ('km')
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01')) IS NOT NULL;

-- All three valid length units
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'), 'km') IS NOT NULL;
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'), 'm') IS NOT NULL;
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'), 'rads') IS NOT NULL;

-- Area unit on a length function — must error.
/* Errors */
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'), 'km2');
-- Invalid unit string
SELECT h3_edge_length(h3_cells_to_directed_edge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'), 'parsec');

-------------------------------------------------------------------------------
-- h3_great_circle_distance(tgeogpoint, tgeogpoint, text) — binary_synced
-------------------------------------------------------------------------------

-- Distance from a point to itself is 0
SELECT h3_great_circle_distance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  'km');

-- All three valid length units
SELECT h3_great_circle_distance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(2.35 48.86)@2001-01-01',
  'km') IS NOT NULL;
SELECT h3_great_circle_distance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(2.35 48.86)@2001-01-01',
  'm') IS NOT NULL;
SELECT h3_great_circle_distance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(2.35 48.86)@2001-01-01',
  'rads') IS NOT NULL;

-- Sequence form
SELECT h3_great_circle_distance(
  tgeogpoint '[POINT(-73.96 40.78)@2001-01-01, POINT(2.35 48.86)@2001-01-02]',
  tgeogpoint '[POINT(2.35 48.86)@2001-01-01, POINT(-73.96 40.78)@2001-01-02]',
  'km') IS NOT NULL;

-- Area unit on a length function — error
/* Errors */
SELECT h3_great_circle_distance(
  tgeogpoint 'POINT(-73.96 40.78)@2001-01-01',
  tgeogpoint 'POINT(2.35 48.86)@2001-01-01',
  'km2');

-------------------------------------------------------------------------------
