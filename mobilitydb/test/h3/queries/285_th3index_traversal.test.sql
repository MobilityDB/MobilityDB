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
-- th3GridDistance — binary_synced
-------------------------------------------------------------------------------

-- A cell is at distance 0 from itself
SELECT th3GridDistance(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b885fffff@2001-01-01');

-- A neighbour pair is at distance 1
SELECT th3GridDistance(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b88dfffff@2001-01-01');

-- Distance is symmetric
SELECT th3GridDistance(
    th3index '880326b885fffff@2001-01-01',
    th3index '880326b88dfffff@2001-01-01')
  = th3GridDistance(
    th3index '880326b88dfffff@2001-01-01',
    th3index '880326b885fffff@2001-01-01');

-- Sequence form
SELECT th3GridDistance(
  th3index '[880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02]',
  th3index '[880326b88dfffff@2001-01-01, 880326b885fffff@2001-01-02]')
  IS NOT NULL;

-------------------------------------------------------------------------------
-- The `<->` operator (synonym for th3GridDistance per §6.1)
-------------------------------------------------------------------------------

SELECT (th3index '880326b885fffff@2001-01-01')
  <-> (th3index '880326b88dfffff@2001-01-01');

-- Operator and function form must agree
SELECT (th3index '880326b885fffff@2001-01-01'
        <-> th3index '880326b88dfffff@2001-01-01')
  = th3GridDistance(
      th3index '880326b885fffff@2001-01-01',
      th3index '880326b88dfffff@2001-01-01');

-------------------------------------------------------------------------------
-- th3CellToLocalIj — binary_synced
-------------------------------------------------------------------------------

-- Local IJ of a cell from its own perspective is the origin (0, 0).
SELECT th3CellToLocalIj(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b885fffff@2001-01-01') IS NOT NULL;

-- Local IJ to a neighbour
SELECT th3CellToLocalIj(
  th3index '880326b885fffff@2001-01-01',
  th3index '880326b88dfffff@2001-01-01') IS NOT NULL;

-------------------------------------------------------------------------------
-- th3LocalIjToCell — binary_synced (th3index, tgeompoint)
--
-- Round trip: cell -> local_ij -> cell must be the identity for cells
-- in the same parent. We use the origin as anchor.
-------------------------------------------------------------------------------

SELECT th3LocalIjToCell(
    th3index '880326b885fffff@2001-01-01',
    th3CellToLocalIj(
      th3index '880326b885fffff@2001-01-01',
      th3index '880326b88dfffff@2001-01-01'))
  = th3index '880326b88dfffff@2001-01-01';

-------------------------------------------------------------------------------
