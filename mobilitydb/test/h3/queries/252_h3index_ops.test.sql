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

-- Static h3index cell operations: the six faces every DGGS family publishes
-- under the bare DggsCellOps slot names — resolution, validity, hierarchy,
-- cell centroid and boundary, and area. The H3-only surface (directed edges,
-- vertices, pentagon / base-cell / class-III inspection, grid traversal,
-- great-circle metrics) is exercised by the 28x files.

-------------------------------------------------------------------------------
-- Resolution
-------------------------------------------------------------------------------

SELECT getResolution(h3index '831c02fffffffff');  -- res 3
SELECT getResolution(h3index '871fa44a8ffffff');  -- res 7
SELECT getResolution(h3index '8a2a1072b59ffff');  -- res 10

-------------------------------------------------------------------------------
-- Validity
-------------------------------------------------------------------------------

SELECT isValidCell(h3index '831c02fffffffff');
SELECT isValidCell(h3index '0');

-------------------------------------------------------------------------------
-- Hierarchy: parent
-------------------------------------------------------------------------------

SELECT getResolution(cellToParent(h3index '8a2a1072b59ffff', 5));
SELECT cellToParent(h3index '8a2a1072b59ffff', 5);

-- The parent of a cell at its own resolution is the cell itself
SELECT cellToParent(h3index '871fa44a8ffffff', 7) = h3index '871fa44a8ffffff';

-- A parent resolution finer than the cell's own is an error
SELECT cellToParent(h3index '831c02fffffffff', 7);

-------------------------------------------------------------------------------
-- Lat/Lng: centroid and boundary
-------------------------------------------------------------------------------

SELECT round(ST_X(cellToPoint(h3index '871fa44a8ffffff'))::numeric, 6);
SELECT round(ST_Y(cellToPoint(h3index '871fa44a8ffffff'))::numeric, 6);
SELECT ST_SRID(cellToPoint(h3index '871fa44a8ffffff'));

SELECT ST_GeometryType(cellToBoundary(h3index '871fa44a8ffffff'));
SELECT ST_SRID(cellToBoundary(h3index '871fa44a8ffffff'));

-- A hexagon's ring closes on its first vertex, so it carries seven points
SELECT ST_NPoints(cellToBoundary(h3index '871fa44a8ffffff'));

-- The centroid lies inside the boundary
SELECT ST_Contains(
  cellToBoundary(h3index '871fa44a8ffffff'),
  cellToPoint(h3index '871fa44a8ffffff'));

-- The cell containing the centroid is the cell itself
SELECT geoToH3Cell(cellToPoint(h3index '871fa44a8ffffff'), 7)
  = h3index '871fa44a8ffffff';

-------------------------------------------------------------------------------
-- Metrics: area
-------------------------------------------------------------------------------

-- The unit is the square metre, the one the DggsCellOps descriptor fixes.
-- The value is bounded to whole square metres: six fractional digits on a
-- magnitude of 4.4e6 pins thirteen significant digits, which drifts between
-- builds.
SELECT round(cellArea(h3index '871fa44a8ffffff')::numeric, 0);

-- A finer cell covers less ground than a coarser one
SELECT cellArea(h3index '8a2a1072b59ffff') < cellArea(h3index '831c02fffffffff');

-- The square-metre face and the H3-only square-kilometre one agree

-------------------------------------------------------------------------------
-- The static face and its temporal lift answer the same value
-------------------------------------------------------------------------------

SELECT getResolution(h3index '831c02fffffffff')
  = getValue(getResolution(th3index '831c02fffffffff@2001-01-01'));

SELECT cellArea(h3index '871fa44a8ffffff')
  = getValue(cellArea(th3index '871fa44a8ffffff@2001-01-01'));

SELECT ST_AsBinary(cellToBoundary(h3index '871fa44a8ffffff'))
  = ST_AsBinary(getValue(cellToBoundary(th3index '871fa44a8ffffff@2001-01-01'))::geometry);

-------------------------------------------------------------------------------
