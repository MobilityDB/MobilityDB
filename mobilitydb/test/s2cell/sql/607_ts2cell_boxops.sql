-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-- The bounding box of an S2 cell CONTAINS the cell, and the containment is
-- what the index keys on. Every cell edge is a geodesic that rises beyond both
-- of the vertices it joins, a cell holding a pole reaches a latitude no vertex
-- carries, and a cell crossing the antimeridian takes the full longitude
-- range.

-------------------------------------------------------------------------------
-- The box holds the whole cell, its geodesic edges included
--
-- The boundary of each cell, densified along its geodesic edges by PostGIS, a
-- route independent of the box, stays inside the box of the cell, over the
-- cells of a grid of points at every level. The edges do rise above the
-- vertices they join, so the vertices alone would not bound the cell. Each
-- point of the densified boundary is compared with the box, its longitude
-- modulo 360, since a cell reaching the antimeridian has points PostGIS writes
-- at -180 or at 180 alike; a box taking the full longitude range, which the
-- cells holding a pole or crossing the antimeridian take, holds every
-- longitude.
-------------------------------------------------------------------------------

WITH cells AS (
  SELECT DISTINCT geoToS2Cell(ST_SetSRID(ST_Point(x, y), 4326)::geography, l)
    AS c
  FROM generate_series(-178.5, 178.5, 9.5) x,
    generate_series(-84.5, 84.5, 6.5) y, generate_series(0, 30) l ),
bounds AS (
  SELECT stbox(c) AS box, cellToBoundary(c)::geometry AS vertices,
    ST_Segmentize(cellToBoundary(c),
      ST_Perimeter(cellToBoundary(c)) / 64)::geometry AS edges
  FROM cells )
SELECT count(*) > 1000 AS many_cells,
  count(*) FILTER (WHERE ST_YMax(edges) > ST_YMax(vertices) + 1e-9) > 0
    AS edges_rise,
  count(*) FILTER (WHERE EXISTS (
    SELECT 1 FROM ST_DumpPoints(edges) d
    WHERE ST_Y(d.geom) > yMax(box) + 1e-9 OR ST_Y(d.geom) < yMin(box) - 1e-9 OR
      NOT (xMax(box) - xMin(box) >= 360 OR
        ST_X(d.geom) BETWEEN xMin(box) - 1e-9 AND xMax(box) + 1e-9 OR
        ST_X(d.geom) - 360 BETWEEN xMin(box) - 1e-9 AND xMax(box) + 1e-9 OR
        ST_X(d.geom) + 360 BETWEEN xMin(box) - 1e-9 AND xMax(box) + 1e-9)))
    AS outside_box
FROM bounds;

-------------------------------------------------------------------------------
