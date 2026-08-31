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

-- §1.7 Vertices — three lifts.
--
-- `cell_to_vertex` and `is_valid_vertex` are backed by h3_generated.c
-- and work without any adapter. `vertex_to_latlng` needs the
-- h3_vertex_to_gs_point adapter.
--
-- Test cells (h3-pg's vertex.sql fixture):
--   612544986753269759 = res 8 hexagon

-------------------------------------------------------------------------------
-- th3CellToVertex(th3index, integer) — lift_with_const
-------------------------------------------------------------------------------

-- A hexagon has six vertices (numbered 0..5)
SELECT th3CellToVertex(th3index '880326b885fffff@2001-01-01', 0)
  IS NOT NULL;
SELECT th3CellToVertex(th3index '880326b885fffff@2001-01-01', 5)
  IS NOT NULL;

-- All four temporal subtypes
SELECT th3CellToVertex(th3index
  '{880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02}', 0)
  IS NOT NULL;
SELECT th3CellToVertex(th3index
  '[880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02]', 0)
  IS NOT NULL;

-- Distinct vertex numbers must yield distinct vertices for the same cell
SELECT th3CellToVertex(th3index '880326b885fffff@2001-01-01', 0)
  <> th3CellToVertex(th3index '880326b885fffff@2001-01-01', 1);

-- Out-of-range vertex number must error
/* Errors */
SELECT th3CellToVertex(th3index '880326b885fffff@2001-01-01', 9);

-------------------------------------------------------------------------------
-- isValidVertex
-------------------------------------------------------------------------------

-- A freshly built vertex is valid
SELECT isValidVertex(
  th3CellToVertex(th3index '880326b885fffff@2001-01-01', 0));

-- A plain cell is not a valid vertex
SELECT isValidVertex(th3index '880326b885fffff@2001-01-01');
SELECT isValidVertex(th3index '0@2001-01-01');

-------------------------------------------------------------------------------
-- th3VertexToPoint — needs h3_vertex_to_gs_point adapter
-------------------------------------------------------------------------------

SELECT th3VertexToPoint(
  th3CellToVertex(th3index '880326b885fffff@2001-01-01', 0))
  IS NOT NULL;

-------------------------------------------------------------------------------
