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

-- Table-based tests for the raquet raster tile type, exercised over
-- tbl_raquet. The fixture holds 100 rows, 10 of them NULL, with tiles spread
-- over zoom levels 1 to 8, so tiles of one zoom partition the plane while
-- tiles of different zooms overlap.

SET timezone = 'UTC';
SET datestyle = 'ISO, MDY';

-------------------------------------------------------------------------------
-- Fixture shape
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_raquet;
SELECT COUNT(*) FROM tbl_raquet WHERE tile IS NULL;
SELECT COUNT(DISTINCT width(tile)), COUNT(DISTINCT height(tile)) FROM tbl_raquet;
SELECT COUNT(DISTINCT pixtype(tile)) FROM tbl_raquet;

-------------------------------------------------------------------------------
-- Input/output round-trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_raquet WHERE tile::text::raquet <> tile;

-- Binary round-trip through COPY, exercising the type's send and receive pair
COPY tbl_raquet TO '/tmp/tbl_raquet' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_raquet_tmp;
CREATE TABLE tbl_raquet_tmp AS TABLE tbl_raquet WITH NO DATA;
COPY tbl_raquet_tmp FROM '/tmp/tbl_raquet' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet_tmp t2
WHERE t1.k = t2.k AND t1.tile <> t2.tile;
DROP TABLE tbl_raquet_tmp;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile = t2.tile;
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile <> t2.tile;
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile < t2.tile;
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile <= t2.tile;
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile > t2.tile;
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2 WHERE t1.tile >= t2.tile;

-- The comparison function agrees with the operators it backs
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2
WHERE (cmp(t1.tile, t2.tile) < 0) <> (t1.tile < t2.tile);
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2
WHERE (cmp(t1.tile, t2.tile) = 0) <> (t1.tile = t2.tile);

-- Equal tiles hash equally, which is what the hash operator class requires
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2
WHERE t1.tile = t2.tile AND hash(t1.tile) <> hash(t2.tile);
SELECT COUNT(*) FROM tbl_raquet t1, tbl_raquet t2
WHERE t1.tile = t2.tile
  AND hashExtended(t1.tile, 1) <> hashExtended(t2.tile, 1);

-------------------------------------------------------------------------------
-- Tile footprint
-------------------------------------------------------------------------------

-- Every tile has a footprint, and it stays inside the Web-Mercator world
SELECT COUNT(*) FROM tbl_raquet WHERE tile IS NOT NULL AND stbox(tile) IS NULL;
SELECT COUNT(*) FROM tbl_raquet
WHERE NOT (stbox(tile) && stbox 'SRID=4326;STBOX X((-180,-86),(180,86))');

-- The cast and the function agree
SELECT COUNT(*) FROM tbl_raquet WHERE tile::stbox <> stbox(tile);

-- A tile of higher zoom covers less ground, so the footprint widths of the
-- fixture span the zoom levels it holds
SELECT round(MIN(ST_XMax(b::geometry) - ST_XMin(b::geometry))::numeric, 6),
       round(MAX(ST_XMax(b::geometry) - ST_XMin(b::geometry))::numeric, 6)
FROM (SELECT stbox(tile) AS b FROM tbl_raquet WHERE tile IS NOT NULL) t;

-------------------------------------------------------------------------------
-- Indexing over the footprint
-------------------------------------------------------------------------------

-- The extent aggregate composes over the cast
SELECT extent(stbox(tile)) IS NOT NULL FROM tbl_raquet;

-- A functional index over the footprint answers the overlap filter, and gives
-- what a sequential scan gives
SELECT COUNT(*) FROM tbl_raquet
WHERE stbox(tile) && stbox 'SRID=4326;STBOX X((0,0),(90,60))';

CREATE INDEX tbl_raquet_rtree_idx ON tbl_raquet USING gist (stbox(tile));
SET enable_seqscan = off;
SELECT COUNT(*) FROM tbl_raquet
WHERE stbox(tile) && stbox 'SRID=4326;STBOX X((0,0),(90,60))';
SET enable_seqscan = on;
DROP INDEX tbl_raquet_rtree_idx;

CREATE INDEX tbl_raquet_quadtree_idx ON tbl_raquet USING spgist (stbox(tile));
SET enable_seqscan = off;
SELECT COUNT(*) FROM tbl_raquet
WHERE stbox(tile) && stbox 'SRID=4326;STBOX X((0,0),(90,60))';
SET enable_seqscan = on;
DROP INDEX tbl_raquet_quadtree_idx;

-------------------------------------------------------------------------------
-- Sampling a trajectory across the tiles that cover it
-------------------------------------------------------------------------------

-- The array form samples the trajectory from every tile at once, so it covers
-- at least as many instants as any single tile does
WITH traj AS (
  SELECT tgeompoint 'SRID=4326;{Point(10.0 20.0)@2024-01-01, Point(40.0 50.0)@2024-01-02, Point(70.0 30.0)@2024-01-03}' AS t
)
SELECT COALESCE(numInstants(raster_tile_value(array_agg(r.tile), traj.t)), 0) >=
       COALESCE(MAX(numInstants(raster_tile_value(r.tile, traj.t))), 0)
FROM tbl_raquet r, traj
WHERE r.tile IS NOT NULL
GROUP BY traj.t;

-- The result never holds more instants than the trajectory has
WITH traj AS (
  SELECT tgeompoint 'SRID=4326;{Point(10.0 20.0)@2024-01-01, Point(40.0 50.0)@2024-01-02, Point(70.0 30.0)@2024-01-03}' AS t
)
SELECT COALESCE(numInstants(raster_tile_value(array_agg(r.tile), traj.t)), 0) <= 3
FROM tbl_raquet r, traj
WHERE r.tile IS NOT NULL
GROUP BY traj.t;

-- Restricting the array to the tiles whose footprint meets the trajectory
-- gives the same result as passing every tile
WITH traj AS (
  SELECT tgeompoint 'SRID=4326;{Point(10.0 20.0)@2024-01-01, Point(40.0 50.0)@2024-01-02, Point(70.0 30.0)@2024-01-03}' AS t
)
SELECT raster_tile_value(
         (SELECT array_agg(tile) FROM tbl_raquet WHERE tile IS NOT NULL), traj.t)::text
       IS NOT DISTINCT FROM
       raster_tile_value(
         (SELECT array_agg(tile) FROM tbl_raquet
          WHERE tile IS NOT NULL AND stbox(tile) && stbox(traj.t)), traj.t)::text
FROM traj;

-------------------------------------------------------------------------------
