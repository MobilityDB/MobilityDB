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

-- Static set-returning h3 functions. Each call returns a single
-- h3indexset (or intset for icosahedron_faces) — the finite-collection
-- companion of h3index.

-------------------------------------------------------------------------------
-- gridDisk
-------------------------------------------------------------------------------

-- k=0 → singleton { origin }
SELECT numValues(gridDisk(h3index '8a2a1072b59ffff', 0)) = 1;

-- k=1 around a hexagon → 7 cells (origin + 6 neighbours)
SELECT numValues(gridDisk(h3index '8a2a1072b59ffff', 1)) = 7;

-- k=2 around a hexagon → up to 19 cells (7 + 12 second-ring)
SELECT numValues(gridDisk(h3index '8a2a1072b59ffff', 2)) = 19;

-- k=3 disk has 37 cells: 1 + 6 + 12 + 18
SELECT numValues(gridDisk(h3index '8a2a1072b59ffff', 3)) = 37;

/* Errors */
SELECT gridDisk(h3index '8a2a1072b59ffff', -1);

-------------------------------------------------------------------------------
-- h3GridRing
-------------------------------------------------------------------------------

-- k=0 → singleton { origin }
SELECT numValues(h3GridRing(h3index '8a2a1072b59ffff', 0)) = 1;

-- k=1 → 6 neighbours (non-pentagon)
SELECT numValues(h3GridRing(h3index '8a2a1072b59ffff', 1)) = 6;

-- k=2 → 12 cells at exactly 2 grid-steps (non-pentagon)
SELECT numValues(h3GridRing(h3index '8a2a1072b59ffff', 2)) = 12;

-- Ring at k=3 has 18 cells (hexagon neighborhood)
SELECT numValues(h3GridRing(h3index '8a2a1072b59ffff', 3)) = 18;

-------------------------------------------------------------------------------
-- h3GridPathCells
-------------------------------------------------------------------------------

-- Path start=end → singleton { start }
SELECT numValues(h3GridPathCells(
  h3index '8a2a1072b59ffff', h3index '8a2a1072b59ffff')) = 1;

-------------------------------------------------------------------------------
-- cellToChildren
-------------------------------------------------------------------------------

-- Children at childRes = cellRes + 1 → 7 (hex) or 6 (pentagon)
SELECT numValues(cellToChildren(h3index '8a2a1072b59ffff', 11)) = 7;

-- Children at childRes = cellRes → singleton { cell }
SELECT numValues(cellToChildren(h3index '8a2a1072b59ffff', 10)) = 1;

-- Children at deeper resolution — counts follow 7^k (for hex cells)
SELECT numValues(cellToChildren(h3index '8a2a1072b59ffff', 12)) = 49;

/* Errors */
-- childRes coarser than cellRes
SELECT cellToChildren(h3index '8a2a1072b59ffff', 5);

-------------------------------------------------------------------------------
-- h3CompactCells / h3UncompactCells
-------------------------------------------------------------------------------

-- Round-trip: uncompact(compact(children)) recovers the input
SELECT h3UncompactCells(
         h3CompactCells(cellToChildren(
           h3index '8a2a1072b59ffff', 11)), 11)
       = cellToChildren(h3index '8a2a1072b59ffff', 11);

-- Full hexagonal set of siblings compacts to one parent
SELECT numValues(h3CompactCells(
         cellToChildren(h3index '8a2a1072b59ffff', 11))) = 1;

-------------------------------------------------------------------------------
-- h3OriginToDirectedEdges
-------------------------------------------------------------------------------

-- A hexagon cell has 6 outgoing directed edges
SELECT numValues(h3OriginToDirectedEdges(h3index '8a2a1072b59ffff')) = 6;

-- The first returned edge is a valid directed edge
SELECT isValidDirectedEdge(valueN(
  h3OriginToDirectedEdges(h3index '8a2a1072b59ffff'), 1));

-------------------------------------------------------------------------------
-- h3CellToVertexes
-------------------------------------------------------------------------------

-- A hexagon cell has 6 vertexes
SELECT numValues(h3CellToVertexes(h3index '8a2a1072b59ffff')) = 6;

-- The first returned vertex is a valid vertex
SELECT isValidVertex(valueN(
  h3CellToVertexes(h3index '8a2a1072b59ffff'), 1));

-------------------------------------------------------------------------------
-- h3GetIcosahedronFaces
-------------------------------------------------------------------------------

-- A generic hex cell intersects exactly one face
SELECT numValues(h3GetIcosahedronFaces(h3index '8a2a1072b59ffff')) = 1;

-- The face index is in 0..19
SELECT valueN(h3GetIcosahedronFaces(h3index '8a2a1072b59ffff'), 1)
  BETWEEN 0 AND 19;

-------------------------------------------------------------------------------
