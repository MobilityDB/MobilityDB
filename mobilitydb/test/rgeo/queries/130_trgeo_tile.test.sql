-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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

-- Tile functions for trgeometry:
--   spaceTiles, timeTiles, spaceTimeTiles

-- A unit square translating from (0,0) to (10,0).
-- spaceTiles: 2-unit spatial grid
SELECT COUNT(*) FROM spaceTiles(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]',
  2.0) t(index, tile);

-- timeTiles: 6-hour time bins
SELECT COUNT(*) FROM timeTiles(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]',
  interval '6 hours') t(index, tile);

-- spaceTimeTiles: 4-unit spatial and 12-hour time grid
SELECT COUNT(*) FROM spaceTimeTiles(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(10 0),0)@2026-01-02]',
  4.0, interval '12 hours') t(index, tile);

-- Table tests
SELECT COUNT(*) FROM tbl_trgeometry2d,
  LATERAL spaceTiles(temp, 2.0) t WHERE tile IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d,
  LATERAL timeTiles(temp, interval '1 day') t WHERE tile IS NOT NULL;

-------------------------------------------------------------------------------
