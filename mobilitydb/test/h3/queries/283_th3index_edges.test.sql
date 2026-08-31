-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b88dfffff@2001-01-01');

-- A cell is not a neighbour of itself
SELECT th3AreNeighborCells(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b885fffff@2001-01-01');

-- Two-instant sequences of the neighbouring pair
SELECT th3AreNeighborCells(
  th3index '[880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02]',
  th3index '[880326b88dfffff@2001-01-01, 880326b885fffff@2001-01-02]');

-------------------------------------------------------------------------------
-- th3CellsToDirectedEdge — binary_synced
-------------------------------------------------------------------------------

-- Round trip: the edge built from (origin, dest) must report the same
-- origin and destination back.
SELECT th3GetDirectedEdgeOrigin(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01'))
  = th3index '880326b885fffff@2001-01-01';

SELECT th3GetDirectedEdgeDestination(th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01'))
  = th3index '880326b88dfffff@2001-01-01';

-------------------------------------------------------------------------------
-- isValidDirectedEdge
-------------------------------------------------------------------------------

-- A freshly built directed edge is valid
SELECT isValidDirectedEdge(th3CellsToDirectedEdge(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b88dfffff@2001-01-01'));

-- A plain h3 cell is not a valid directed edge
SELECT isValidDirectedEdge(th3index '880326b885fffff@2001-01-01');
SELECT isValidDirectedEdge(th3index '0@2001-01-01');

-------------------------------------------------------------------------------
-- th3GetDirectedEdgeOrigin
-------------------------------------------------------------------------------

-- Combined with cells_to_directed_edge, see round trip above.
-- Standalone: the origin of an edge must be a valid h3 cell.
SELECT isValidCell(th3GetDirectedEdgeOrigin(th3CellsToDirectedEdge(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b88dfffff@2001-01-01')));

-------------------------------------------------------------------------------
-- th3GetDirectedEdgeDestination
-------------------------------------------------------------------------------

SELECT isValidCell(th3GetDirectedEdgeDestination(
  th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')));

-------------------------------------------------------------------------------
-- th3DirectedEdgeToBoundary — needs h3_adapter.c body
-------------------------------------------------------------------------------

SELECT th3DirectedEdgeToBoundary(
  th3CellsToDirectedEdge(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')) IS NOT NULL;

-------------------------------------------------------------------------------
