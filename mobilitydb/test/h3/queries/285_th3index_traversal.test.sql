-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.3 Grid traversal — three lifts, plus the new `<->` operator
-- (resolved §6.1).
--
-- All three temporal lifts depend on adapters in h3_adapter.c
-- (h3_grid_distance_meos, h3_cell_to_local_ij_meos,
-- h3_local_ij_to_cell_meos). Tests are still meaningful: they
-- assert the output type and a few invariants.
--
-- Test cells:
--   612544986753269759 = res 8 hexagon
--   612544986761658367 = res 8 hexagon (neighbour — distance 1)

-------------------------------------------------------------------------------
-- h3_grid_distance — binary_synced
-------------------------------------------------------------------------------

-- A cell is at distance 0 from itself
SELECT h3_grid_distance(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986753269759@2001-01-01');

-- A neighbour pair is at distance 1
SELECT h3_grid_distance(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986761658367@2001-01-01');

-- Distance is symmetric
SELECT h3_grid_distance(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01')
  = h3_grid_distance(
    th3index '612544986761658367@2001-01-01',
    th3index '612544986753269759@2001-01-01');

-- Sequence form
SELECT h3_grid_distance(
  th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]',
  th3index '[612544986761658367@2001-01-01, 612544986753269759@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
-- The `<->` operator (synonym for h3_grid_distance per §6.1)
-------------------------------------------------------------------------------

SELECT (th3index '612544986753269759@2001-01-01')
  <-> (th3index '612544986761658367@2001-01-01');

-- Operator and function form must agree
SELECT (th3index '612544986753269759@2001-01-01'
        <-> th3index '612544986761658367@2001-01-01')
  = h3_grid_distance(
      th3index '612544986753269759@2001-01-01',
      th3index '612544986761658367@2001-01-01');

-------------------------------------------------------------------------------
-- h3_cell_to_local_ij — binary_synced
-------------------------------------------------------------------------------

-- Local IJ of a cell from its own perspective is the origin (0, 0).
SELECT h3_cell_to_local_ij(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986753269759@2001-01-01') IS NOT NULL;

-- Local IJ to a neighbour
SELECT h3_cell_to_local_ij(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986761658367@2001-01-01') IS NOT NULL;

-------------------------------------------------------------------------------
-- h3_local_ij_to_cell — binary_synced (th3index, tgeompoint)
--
-- Round trip: cell -> local_ij -> cell must be the identity for cells
-- in the same parent. We use the origin as anchor.
-------------------------------------------------------------------------------

SELECT h3_local_ij_to_cell(
    th3index '612544986753269759@2001-01-01',
    h3_cell_to_local_ij(
      th3index '612544986753269759@2001-01-01',
      th3index '612544986761658367@2001-01-01'))
  = th3index '612544986761658367@2001-01-01';

-------------------------------------------------------------------------------
