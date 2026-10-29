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

-- The bounding box of an H3 cell holds the whole cell over the fixture, as
-- 259_th3index_boxops states it over a grid of points at every resolution.

-------------------------------------------------------------------------------
-- The box holds the whole cell, its geodesic edges included
--
-- The boundary of each cell of the fixture, densified along its geodesic edges
-- by PostGIS, stays inside the box of the cell, and the boundary of the first
-- cell of each temporal cell inside the box of the temporal cell, each point
-- compared with its longitude modulo 360.
-------------------------------------------------------------------------------

WITH bounds AS (
  SELECT stbox(h3) AS box, ST_Segmentize(cellToBoundary(h3)::geography,
    ST_Perimeter(cellToBoundary(h3)::geography) / 96)::geometry AS edges
  FROM tbl_h3index WHERE h3 IS NOT NULL
  UNION ALL
  SELECT stbox(temp), ST_Segmentize(cellToBoundary(startValue(temp))::geography,
    ST_Perimeter(cellToBoundary(startValue(temp))::geography) / 96)::geometry
  FROM tbl_th3index WHERE temp IS NOT NULL )
SELECT count(*) > 50 AS many_cells,
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
