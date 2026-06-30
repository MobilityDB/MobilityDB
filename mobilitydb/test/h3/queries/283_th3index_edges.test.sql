-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.6 Directed edges — six lifts.
--
-- Five entries (are_neighbor_cells, cells_to_directed_edge,
-- is_valid_directed_edge, get_directed_edge_origin / destination)
-- are backed by h3_generated.c and work without any adapter.
-- `directed_edge_to_boundary` needs the adapter to land first.
--
-- Test cells (h3-pg's edge.sql fixture):
--   612544986753269759 = 0x880326b885fffff = res 8 hexagon
--   612544986761658367 = 0x880326b88dfffff = res 8 hexagon (neighbour
--                                              of the above)

-------------------------------------------------------------------------------
-- th3AreNeighborCells — binary_synced
-------------------------------------------------------------------------------

SELECT th3AreNeighborCells(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986761658367@2001-01-01');

-- A cell is not a neighbour of itself
SELECT th3AreNeighborCells(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986753269759@2001-01-01');

-- Two-instant sequences of the neighbouring pair
SELECT th3AreNeighborCells(
  th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]',
  th3index '[612544986761658367@2001-01-01, 612544986753269759@2001-01-02]');

-------------------------------------------------------------------------------
-- th3CellsToDirectedEdge — binary_synced
-------------------------------------------------------------------------------

-- Round trip: the edge built from (origin, dest) must report the same
-- origin and destination back.
SELECT th3GetDirectedEdgeOrigin(th3CellsToDirectedEdge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'))
  = th3index '612544986753269759@2001-01-01';

SELECT th3GetDirectedEdgeDestination(th3CellsToDirectedEdge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01'))
  = th3index '612544986761658367@2001-01-01';

-------------------------------------------------------------------------------
-- isValidDirectedEdge
-------------------------------------------------------------------------------

-- A freshly built directed edge is valid
SELECT isValidDirectedEdge(th3CellsToDirectedEdge(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986761658367@2001-01-01'));

-- A plain h3 cell is not a valid directed edge
SELECT isValidDirectedEdge(th3index '612544986753269759@2001-01-01');
SELECT isValidDirectedEdge(th3index '0@2001-01-01');

-------------------------------------------------------------------------------
-- th3GetDirectedEdgeOrigin
-------------------------------------------------------------------------------

-- Combined with cells_to_directed_edge, see round trip above.
-- Standalone: the origin of an edge must be a valid h3 cell.
SELECT isValidCell(th3GetDirectedEdgeOrigin(th3CellsToDirectedEdge(
  th3index '612544986753269759@2001-01-01',
  th3index '612544986761658367@2001-01-01')));

-------------------------------------------------------------------------------
-- th3GetDirectedEdgeDestination
-------------------------------------------------------------------------------

SELECT isValidCell(th3GetDirectedEdgeDestination(
  th3CellsToDirectedEdge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01')));

-------------------------------------------------------------------------------
-- th3DirectedEdgeToBoundary — needs h3_adapter.c body
-------------------------------------------------------------------------------

SELECT th3DirectedEdgeToBoundary(
  th3CellsToDirectedEdge(
    th3index '612544986753269759@2001-01-01',
    th3index '612544986761658367@2001-01-01')) IS NOT NULL;

-------------------------------------------------------------------------------
