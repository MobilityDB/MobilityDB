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

-- Pin timezone and datestyle so the tfloat text output is identical across
-- all CI environments (Ubuntu UTC, macOS PST, Windows EST, …).
SET timezone = 'UTC';
SET datestyle = 'ISO, MDY';

-- A 3x3 synthetic raster (SRID 4326, 1° pixels) used throughout this file.
-- Row 1 (lat 2–3): pixel values 10, 20, 30  (left → right)
-- Row 2 (lat 1–2): pixel values 40, 50, 60
-- Row 3 (lat 0–1): pixel values 70, 80, 90

-------------------------------------------------------------------------------
-- rasterValue — basic usage
-------------------------------------------------------------------------------

-- Three non-collinear instants, all inside the raster.
-- POINT(0.5 2.5) → pixel(row=1,col=1) = 10
-- POINT(2.5 2.5) → pixel(row=1,col=3) = 30
-- POINT(0.5 0.5) → pixel(row=3,col=1) = 70
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(2.5 2.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]', r)::text AS result
FROM rast;

-- One instant outside the raster extent is silently dropped.
-- POINT(0.5 2.5) → 10; POINT(5.5 5.5) outside → dropped; POINT(0.5 0.5) → 70
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(5.5 5.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]', r)::text AS result
FROM rast;

-- An instant on a nodata pixel is dropped, as one outside the extent is: the
-- band declares -9999 as its nodata value and pixel(row=1,col=2) holds it.
-- POINT(0.5 2.5) → 10; POINT(1.5 2.5) → nodata → dropped; POINT(0.5 0.5) → 70
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999.0::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, -9999.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(1.5 2.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]', r)::text AS result
FROM rast;

-- All instants outside the raster → NULL.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(5.5 5.5)@2001-01-01,
  POINT(6.5 6.5)@2001-01-02]', r)::text AS result
FROM rast;

-- Single-instant trajectory.
-- POINT(1.5 1.5) → pixel(row=2,col=2) = 50 (non-collinear issue does not
-- apply to a single instant; the instant set format {v@t} is returned).
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r)::text AS result
FROM rast;

-- A trajectory that moves between its instants passes over the pixels between
-- them, and their values belong to the answer: the diagonal below crosses
-- pixel(row=2,col=2) = 50 between its two instants, and each value holds until
-- the trip reaches a pixel holding another.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(2.5 0.5)@2001-01-03]', r)::text AS result
FROM rast;

-- The same three positions stated as an instant set say nothing between them,
-- so the answer holds one value per instant.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(0.5 2.5)@2001-01-01,
  POINT(1.5 1.5)@2001-01-02, POINT(2.5 0.5)@2001-01-03}', r)::text AS result
FROM rast;

-- A trip crossing a nodata pixel answers one sequence per visit: the band
-- declares -9999 as its nodata value and pixel(row=1,col=2) holds it.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999.0::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, -9999.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(2.5 2.5)@2001-01-03]', r)::text AS result
FROM rast;

-- A trip reads each value from the instant it reaches the pixel holding it:
-- the row below crosses x = 1 at 14:46:09.230769 and x = 2 at 09:13:50.769230
-- on the next day.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.2 2.5)@2001-01-01,
  POINT(2.8 2.5)@2001-01-03]', r)::text AS result
FROM rast;

-- A trip clipping the corner of a pixel over a short chord holds its value:
-- the step below passes through pixel(row=3,col=1) = 70 between 40 and 80.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.9 1.05)@2001-01-01,
  POINT(1.05 0.9)@2001-01-02]', r)::text AS result
FROM rast;

-- A single segment crossing pixels that alternate between a value and nodata
-- answers one sequence per visit, six here.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(12, 1, 0.0, 1.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999.0::float8
    ),
    1, 1, 1,
    ARRAY[[1.0::float4, -9999.0::float4, 2.0::float4, -9999.0::float4,
           3.0::float4, -9999.0::float4, 4.0::float4, -9999.0::float4,
           5.0::float4, -9999.0::float4, 6.0::float4, -9999.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 0.5)@2001-01-01,
  POINT(11.5 0.5)@2001-01-12]', r)::text AS result
FROM rast;

-- A trip over a Raquet tile is read the same way, the tile grid being the
-- pixels of its QUADBIN cell.
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;[Point(45.0 75.0)@2024-01-01,
  Point(135.0 75.0)@2024-01-02]',
  decode('01020304', 'hex'), 2, 2, 5193776270265024512, 'uint8', 0.0, false)::text;

-- A band the raster does not have is an error, in either direction.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r, 2)
FROM rast;
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r, 0)
FROM rast;

-- A raster and a trajectory in different reference systems state their
-- positions in different units, which is an error and not an empty answer.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=3857;{POINT(1.5 1.5)@2001-01-01}', r)
FROM rast;

-- A band stored outside the database is read from its file, so a file that
-- cannot be opened leaves the band without pixels to read, which is an error
-- and not an empty answer: no value, never and always would each state
-- something about a band nobody read. The band of this 3x3 raster names the
-- file no_such_raster.tif, which does not exist.
CREATE TEMP TABLE tbl_outdb AS
SELECT ('0100000100000000000000f03f000000000000f0bf0000000000000000'
  '000000000000084000000000000000000000000000000000e6100000030003008a'
  '00000000006e6f5f737563685f7261737465722e74696600')::raster AS r;
SELECT ST_BandPath(r, 1) AS band_path FROM tbl_outdb;
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r)
FROM tbl_outdb;
SELECT rasterValue(tgeompoint 'SRID=4326;[POINT(0.5 1.5)@2001-01-01,
  POINT(2.5 1.5)@2001-01-02]', r)
FROM tbl_outdb;
SELECT atRasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r,
  floatspan '[0, 100]')
FROM tbl_outdb;
SELECT minusRasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r,
  floatspan '[0, 100]')
FROM tbl_outdb;
SELECT eRasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r,
  floatspan '[0, 100]')
FROM tbl_outdb;
SELECT aRasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}', r,
  floatspan '[0, 100]')
FROM tbl_outdb;
DROP TABLE tbl_outdb;

-- The sampling reads the grid the geotransform states, in whatever reference
-- system the pair agrees on: the same raster shape in EPSG:3857 answers what it
-- answers in EPSG:4326, so nothing in the walk assumes lon/lat.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 3857),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=3857;{POINT(1.5 1.5)@2001-01-01,
  POINT(2.5 1.5)@2001-01-02}', r)::text AS sampled_in_3857
FROM rast;

-- A skewed grid is a different grid: the same two positions fall in different
-- pixels once the geotransform carries a skew, so this cannot pass by reading
-- the unskewed answer. The two rows below are the refuting pair.
WITH plain AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 3857),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
), skewed AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 1.0, 0.0, 3857),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT rasterValue(tgeompoint 'SRID=3857;{POINT(1.5 1.5)@2001-01-01,
    POINT(2.5 1.5)@2001-01-02}', (SELECT r FROM plain))::text AS plain_values,
  rasterValue(tgeompoint 'SRID=3857;{POINT(1.5 1.5)@2001-01-01,
    POINT(2.5 1.5)@2001-01-02}', (SELECT r FROM skewed))::text AS skewed_values;

-------------------------------------------------------------------------------
-- atRasterValue / minusRasterValue / eRasterValue / aRasterValue
-------------------------------------------------------------------------------

-- Shared fixture: 3x3 raster, pixel values 10..90 (row-major).
-- traj1: three instants sampling values 10, 50, 70.
-- traj2: two instants sampling values 10, 70.

-- atRasterValue([40,90]): value 10 dropped, 50 and 70 kept.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT asText(atRasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
  POINT(1.5 1.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]', r,
  floatspan '[40, 90]'), 6)::text AS result
FROM rast;

-- minusRasterValue([40,90]): values 50 and 70 dropped, 10 kept.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT asText(minusRasterValue(
  tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01, POINT(1.5 1.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]',
  r, floatspan '[40, 90]'), 6)::text AS result
FROM rast;

-- eRasterValue([70,90]): traj2 samples 10 and 70; 70 in range → true.
-- eRasterValue([80,90]): neither 10 nor 70 in [80,90] → false.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT
  eRasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
    POINT(0.5 0.5)@2001-01-02]', r, floatspan '[70, 90]') AS e_true,
  eRasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
    POINT(0.5 0.5)@2001-01-02]', r, floatspan '[80, 90]') AS e_false
FROM rast;

-- aRasterValue([70,90]): traj2 has 10 not in range → false.
-- aRasterValue([0,100]): all values in range → true.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, NULL::float8
    ),
    1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]
  ) AS r
)
SELECT
  aRasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
    POINT(0.5 0.5)@2001-01-02]', r, floatspan '[70, 90]') AS a_false,
  aRasterValue(tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
    POINT(0.5 0.5)@2001-01-02]', r, floatspan '[0, 100]') AS a_true
FROM rast;

-------------------------------------------------------------------------------
-- quadbins
-------------------------------------------------------------------------------

-- Three distinct longitudes at zoom 3 produce three distinct QUADBIN cells.
SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;{Point(-60.0 45.0)@2024-01-01,
  Point(0.0 45.0)@2024-01-02, Point(60.0 45.0)@2024-01-03}', 3), 1) AS num_distinct_tiles;

-- Two instants in the same tile → deduplicated to 1 cell.
SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;{Point(-60.0 45.0)@2024-01-01,
    Point(-61.0 44.0)@2024-01-02}', 3), 1) AS num_distinct_tiles;

-- A trajectory that moves between its instants covers the tiles it crosses on
-- the way: at zoom 3 a tile spans 45 degrees of longitude, so a trip from
-- 10E to 170E crosses four of them, and the same path sampled every 20 degrees
-- answers the same four.
SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;[Point(10.0 10.0)@2024-01-01,
    Point(170.0 10.0)@2024-01-02]', 3), 1) AS num_crossed_tiles;

SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;{Point(10.0 10.0)@2024-01-01, Point(30.0 10.0)@2024-01-02,
    Point(50.0 10.0)@2024-01-03, Point(70.0 10.0)@2024-01-04,
    Point(90.0 10.0)@2024-01-05, Point(110.0 10.0)@2024-01-06,
    Point(130.0 10.0)@2024-01-07, Point(150.0 10.0)@2024-01-08,
    Point(170.0 10.0)@2024-01-09}', 3), 1) AS num_sampled_tiles;

-- A diagonal crossing is the case a sampled walk loses: it leaves a tile
-- through a corner, over a chord that any step long enough to be economical
-- steps across. At zoom 5 a tile spans 11.25 degrees of longitude, so a trip
-- from (10,10) to (40,40) spans four tile columns and four Mercator rows, and
-- a monotone path across a four by four span visits four plus four minus one
-- of them.
SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;[Point(10.0 10.0)@2024-01-01,
    Point(40.0 40.0)@2024-01-02]', 5), 1) AS num_diagonal_tiles;

-- A trajectory that states nothing between its instants covers the tiles
-- holding them, and the two instants below sit in one tile.
SELECT array_length(quadbins(
  tgeompoint 'SRID=4326;{Point(10.0 10.0)@2024-01-01,
    Point(170.0 10.0)@2024-01-02}', 3), 1) AS num_instant_tiles;

-- Invalid zoom level raises an error.
SELECT quadbins(tgeompoint 'SRID=4326;{Point(0.0 0.0)@2024-01-01}', 16);

-------------------------------------------------------------------------------
-- rasterTileValueQuadbin
-------------------------------------------------------------------------------

-- A 2x2 UINT8 chip for tile (x=1, y=0, zoom=1): lon 0..180 degrees, lat 0..85 degrees.
-- Mercator midpoint ~= 66.5 degrees separates row 0 (upper) from row 1 (lower).
-- Pixel layout (row-major):  [1, 2]   row 0 (lat > 66.5 degrees)
--                            [3, 4]   row 1 (lat < 66.5 degrees)
-- POINT(45  75) → col=0, row=0 → 1
-- POINT(135 75) → col=1, row=0 → 2
-- POINT(45  10) → col=0, row=1 → 3
-- POINT(-45 75) → lon outside 0..180 → dropped
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;{Point(45.0 75.0)@2024-01-01,
  Point(135.0 75.0)@2024-01-02, Point(45.0 10.0)@2024-01-03, Point(-45.0 75.0)@2024-01-04}',
  '\x01020304'::bytea,         -- 4 UINT8 pixels: 1,2,3,4
  2::integer,                  -- width
  2::integer,                  -- height
  5193776270265024512::bigint, -- quadbin_tile_to_cell(1,0,1)
  'UINT8', 0.0, false)::text AS result;

-- A pixel array too small for the declared width/height raises an error
-- rather than sampling past the end of the buffer. Point(45 10) maps to
-- col=0, row=1 (byte offset 2), which is past the 2 bytes actually supplied.
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;{Point(45.0 10.0)@2024-01-01}',
  '\x0102'::bytea,             -- 2 bytes, but a 2x2 UINT8 tile needs 4
  2::integer,                  -- width
  2::integer,                  -- height
  5193776270265024512::bigint, -- quadbin_tile_to_cell(1,0,1)
  'UINT8', 0.0, false);

-- The dimensions reach the tile as an unsigned 16-bit width and height. A
-- value outside that range raises an error rather than wrapping to a tile of
-- another size: 65538 would otherwise sample a 2 pixel wide tile, and -1 a
-- 65535 pixel wide one.
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;{Point(45.0 10.0)@2024-01-01}',
  '\x01020304'::bytea, 65538::integer, 2::integer,
  5193776270265024512::bigint, 'UINT8', 0.0, false);
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;{Point(45.0 10.0)@2024-01-01}',
  '\x01020304'::bytea, 2::integer, -1::integer,
  5193776270265024512::bigint, 'UINT8', 0.0, false);
SELECT rasterTileValueQuadbin(tgeompoint 'SRID=4326;{Point(45.0 10.0)@2024-01-01}',
  '\x01020304'::bytea, 0::integer, 2::integer,
  5193776270265024512::bigint, 'UINT8', 0.0, false);

-------------------------------------------------------------------------------
-- raquet type: construction, WKB round-trip, and typed sampling
-------------------------------------------------------------------------------

-- The typed rasterTileValue(tgeompoint, raquet) yields the same result as the
-- untyped rasterTileValueQuadbin(bytea, ...) path on the same 2x2 chip.
WITH t(traj) AS (
  SELECT tgeompoint 'SRID=4326;{Point(45.0 75.0)@2024-01-01, Point(135.0 75.0)@2024-01-02, Point(45.0 10.0)@2024-01-03, Point(-45.0 75.0)@2024-01-04}' )
SELECT rasterTileValue(traj,
         raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8'))::text
     = rasterTileValueQuadbin(traj, 
         '\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8', 0.0, false)::text
       AS typed_equals_untyped
FROM t;

-- rasterTileValue(tgeompoint, raquet[]) samples a trajectory from every tile
-- covering it. The three tiles below carry the same 2x2 pixel array; what
-- distinguishes them is the ground they cover:
--   west  = zoom 1 tile (0,0), lon [-180, 0)
--   east  = zoom 1 tile (1,0), lon [0, 180)
--   fine  = zoom 2 tile (2,0), lon [0, 90), lat (66.51, 85.05]
-- The trajectory visits Point(-45 75) in west only, Point(45 75) in east and
-- in fine, which overlap there, and Point(135 75) in east only.

-- Two tiles of the same zoom partition the plane, so a trajectory crossing
-- from one into the other is sampled from both, with no instant contributed
-- twice. The merged result therefore covers every instant that either tile
-- covers alone.
WITH t AS (
  SELECT tgeompoint 'SRID=4326;{Point(-45.0 75.0)@2024-01-01,
    Point(45.0 75.0)@2024-01-02, Point(135.0 75.0)@2024-01-03}' AS traj,
    raquet('\x01020304'::bytea, 2, 2, 5192650370358181888::bigint, 'UINT8') AS west,
    raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8') AS east
)
SELECT rasterTileValue(traj, west)::text AS west_alone,
       rasterTileValue(traj, east)::text AS east_alone,
       rasterTileValue(traj, ARRAY[west, east])::text AS merged
FROM t;

-- Tiles of different zoom levels overlap. Where both sample the same instant
-- the value of the tile of higher zoom is kept, that being the one carrying
-- the finer resolution, and the outcome does not depend on the array order.
WITH t AS (
  SELECT tgeompoint 'SRID=4326;{Point(-45.0 75.0)@2024-01-01, Point(45.0 75.0)@2024-01-02, Point(135.0 75.0)@2024-01-03}' AS traj,
    raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8') AS east,
    raquet('\x01020304'::bytea, 2, 2, 5198279869892395008::bigint, 'UINT8') AS fine
)
SELECT rasterTileValue(traj, fine)::text AS fine_alone,
  rasterTileValue(traj, ARRAY[east, fine])::text AS east_then_fine,
  rasterTileValue(traj, ARRAY[fine, east])::text AS fine_then_east
FROM t;

-- A moving trip is read from every tile it crosses, each from the instant it
-- reaches it: west until lon 0, then fine, whose pixels 7 and 8 replace the
-- values of east where the two overlap, then east beyond lon 90. The answer
-- does not depend on the array order.
WITH t AS (
  SELECT tgeompoint 'SRID=4326;[Point(-45.0 75.0)@2024-01-01,
    Point(135.0 75.0)@2024-01-03]' AS traj,
    raquet('\x01020304'::bytea, 2, 2, 5192650370358181888::bigint, 'UINT8') AS west,
    raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8') AS east,
    raquet('\x05060708'::bytea, 2, 2, 5198279869892395008::bigint, 'UINT8') AS fine
)
SELECT rasterTileValue(traj, ARRAY[west, east, fine])::text AS merged,
  rasterTileValue(traj, ARRAY[fine, east, west])::text =
    rasterTileValue(traj, ARRAY[west, east, fine])::text AS order_free,
  rasterTileValue(traj, ARRAY[east])::text =
    rasterTileValue(traj, east)::text AS singleton_equals_scalar
FROM t;

-- A one-element array agrees with the scalar form, and an array of tiles that
-- the trajectory never enters returns NULL.
WITH t AS (
  SELECT tgeompoint 'SRID=4326;{Point(45.0 75.0)@2024-01-02}' AS traj,
    raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8') AS east,
    raquet('\x01020304'::bytea, 2, 2, 5192650370358181888::bigint, 'UINT8') AS west
)
SELECT rasterTileValue(traj, ARRAY[east])::text =
  rasterTileValue(traj, east)::text AS singleton_equals_scalar,
  rasterTileValue(traj, ARRAY[west]) IS NULL AS uncovered_is_null
FROM t;

-- An empty array is rejected.
SELECT rasterTileValue(tgeompoint 'SRID=4326;{Point(45.0 75.0)@2024-01-02}',
  ARRAY[]::raquet[]);

-- The HexWKB text representation round-trips through the raquet type's input
-- and output functions (raquet::text uses raquet_out, text::raquet uses
-- raquet_in).
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8')::text::raquet::text
     = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8')::text
       AS hexwkb_roundtrip_ok;

-- The constructor rejects a pixel array too small for the given dimensions.
SELECT raquet('\x0102'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-- A raquet large enough to be TOASTed (64 x 64 UINT8 = 4096 pixel bytes) must
-- survive a store-and-read-back cycle: reading the stored value detoasts it, so
-- its HexWKB output equals that of the same tile constructed inline.
CREATE TEMP TABLE raquet_toast (rq raquet);
INSERT INTO raquet_toast
  VALUES (raquet(decode(repeat('01', 4096), 'hex'), 64, 64,
    5193776270265024512::bigint, 'UINT8'));
SELECT (
  SELECT rq FROM raquet_toast)::text = 
    raquet(decode(repeat('01', 4096), 'hex'), 64, 64, 
      5193776270265024512::bigint, 'UINT8')::text AS toasted_roundtrip_ok;

-------------------------------------------------------------------------------
-- raquetRead: GDAL ingest of an in-memory raster file (bytea)
-------------------------------------------------------------------------------

-- raquetRead decodes through GDAL, whose drivers PostGIS disables by default
-- (postgis.gdal_enabled_drivers); enable them for the in-memory ingest.
SET postgis.gdal_enabled_drivers = 'ENABLE_ALL';

-- GDAL decodes a 2 x 2 UINT8 GeoTIFF supplied as bytea (through its /vsimem/
-- virtual filesystem) into the same raquet tile that the constructor builds
-- from the identical row-major pixel bytes 01 02 03 04.
SELECT raquetRead(
         decode('49492a00080000000b000001030001000000020000000101030001000000020000000201030001000000080000000301030001000000010000000601030001000000010000001101040001000000920000001501030001000000010000001601030001000000020000001701040001000000040000001c01030001000000010000005301030001000000010000000000000001020304', 'hex'),
         5193776270265024512::bigint)::text
     = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8')::text
       AS gdal_ingest_equals_constructor;

-------------------------------------------------------------------------------
-- raquetRead: derive the QUADBIN cell from the raster georeferencing
-------------------------------------------------------------------------------

-- Omitting the quadbin argument reads the tile identifier from the raster's
-- EPSG:3857 geotransform. This GeoTIFF georeferences Web-Mercator tile (1, 0)
-- at zoom 1, whose QUADBIN cell is 5193776270265024512, so raquetRead(bytes)
-- yields the same tile as raquetRead(bytes, 5193776270265024512).
WITH t(bytes) AS (VALUES (decode('49492a00080000000f0000010300010000000200000001010300010000000200000002010300010000000800000003010300010000000100000006010300010000000100000011010400010000006b0100001501030001000000010000001601030001000000020000001701040001000000040000001c01030001000000010000005301030001000000010000000e830c0003000000c200000082840c0006000000da000000af870300200000000a010000b1870200210000004a0100000000000093107c45f81b634193107c45f81b63410000000000000000000000000000000000000000000000000000000000000000000000000000000093107c45f81b734100000000000000000100010000000700000400000100010001040000010001000204b187190000000108b187070019000608000001008e23000c00000100110f040c000001002923574753203834202f2050736575646f2d4d65726361746f727c5747532038347c0001020304', 'hex')))
SELECT raquetRead(bytes)::text
     = raquetRead(bytes, 5193776270265024512::bigint)::text
       AS derived_quadbin_equals_explicit
FROM t;

-- A raster with no Web-Mercator georeferencing has no derivable tile: this
-- 2 x 2 GeoTIFF carries no geotransform, so omitting the quadbin is an error.
SELECT raquetRead(
  decode('49492a00080000000b000001030001000000020000000101030001000000020000000201030001000000080000000301030001000000010000000601030001000000010000001101040001000000920000001501030001000000010000001601030001000000020000001701040001000000040000001c01030001000000010000005301030001000000010000000000000001020304', 'hex'));

-------------------------------------------------------------------------------
-- The file forms of rasterValue, its restrictions and predicates, and
-- raquetRead: a raster file on the server read through GDAL
-------------------------------------------------------------------------------

-- A raster file on the server is read where PostGIS allows a band stored
-- outside the database to be read. The 3x3 raster of the examples and the
-- 2 x 2 GeoTIFF above are written to files in the data directory, which a
-- relative path names.
SET postgis.gdal_enabled_drivers = 'ENABLE_ALL';
SET postgis.enable_outdb_rasters = true;
SELECT lo_from_bytea(424242, ST_AsGDALRaster(ST_SetValues(ST_AddBand(
  ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
  '32BF'::text, 0.0::float8, NULL::float8), 1, 1, 1,
  ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
        [40.0::float4, 50.0::float4, 60.0::float4],
        [70.0::float4, 80.0::float4, 90.0::float4]]), 'GTiff')) = 424242
  AS written;
SELECT lo_export(424242, 'raster_file_forms.tif') AS exported;
SELECT lo_unlink(424242) AS unlinked;
SELECT lo_from_bytea(424243, decode('49492a00080000000b000001030001000000020000000101030001000000020000000201030001000000080000000301030001000000010000000601030001000000010000001101040001000000920000001501030001000000010000001601030001000000020000001701040001000000040000001c01030001000000010000005301030001000000010000000000000001020304', 'hex')) = 424243
  AS written;
SELECT lo_export(424243, 'raquet_file_form.tif') AS exported;
SELECT lo_unlink(424243) AS unlinked;

-- The file forms answer what the raster forms answer over the same raster.
-- A string literal in the second position names a file, text being the
-- preferred type of an untyped literal.
WITH rast AS (
  SELECT ST_SetValues(ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8), 1, 1, 1,
    ARRAY[[10.0::float4, 20.0::float4, 30.0::float4],
          [40.0::float4, 50.0::float4, 60.0::float4],
          [70.0::float4, 80.0::float4, 90.0::float4]]) AS r
), trip AS (
  SELECT tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2001-01-01,
    POINT(2.5 2.5)@2001-01-02, POINT(0.5 0.5)@2001-01-03]' AS t
)
SELECT rasterValue(t, 'raster_file_forms.tif')::text AS file_form,
  rasterValue(t, 'raster_file_forms.tif'::text) = rasterValue(t, r)
    AS value_same,
  atRasterValue(t, 'raster_file_forms.tif'::text, floatspan '[40, 90]') =
    atRasterValue(t, r, floatspan '[40, 90]') AS at_same,
  minusRasterValue(t, 'raster_file_forms.tif'::text, floatspan '[40, 90]') =
    minusRasterValue(t, r, floatspan '[40, 90]') AS minus_same,
  eRasterValue(t, 'raster_file_forms.tif'::text, floatspan '[65, 75]') AS ever_file,
  eRasterValue(t, r, floatspan '[65, 75]') AS ever_raster,
  aRasterValue(t, 'raster_file_forms.tif'::text, floatspan '[0, 100]') AS always_file,
  aRasterValue(t, r, floatspan '[0, 100]') AS always_raster
FROM rast, trip;

-- raquetRead reads the file into the tile it decodes from the same bytes
SELECT raquetRead('raquet_file_form.tif'::text, 5193776270265024512::bigint)::text
     = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8')::text
       AS file_form_equals_constructor;

-- The setting PostGIS states for a band stored outside the database decides
-- a file read, and so do the GDAL drivers it enables.
SET postgis.enable_outdb_rasters = false;
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}',
  'raster_file_forms.tif'::text);
SELECT raquetRead('raquet_file_form.tif'::text, 5193776270265024512::bigint);
SET postgis.enable_outdb_rasters = true;
SET postgis.gdal_enabled_drivers = 'DISABLE_ALL';
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}',
  'raster_file_forms.tif'::text);
SET postgis.gdal_enabled_drivers = 'GTiff';
SELECT rasterValue(tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2001-01-01}',
  '/vsicurl/https://example.org/raster.tif'::text);
SET postgis.gdal_enabled_drivers = 'ENABLE_ALL';

-------------------------------------------------------------------------------
-- raquet (Hex)WKB round trip
--
-- The tile carries its pixels and its QUADBIN georeferencing in one value, so
-- it round-trips through a portable byte string with no spatial extension
-- involved, exactly as the sibling h3index cell does.
-------------------------------------------------------------------------------

SELECT raquetFromBinary(asBinary(raquet('\x01020304'::bytea, 2, 2,
         5193776270265024512::bigint, 'UINT8')))
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

SELECT raquetFromHexWKB(asHexWKB(raquet('\x0102030405060708'::bytea, 2, 2,
         5193776270265024512::bigint, 'INT16', -9999.0)))
       = raquet('\x0102030405060708'::bytea, 2, 2, 5193776270265024512::bigint,
         'INT16', -9999.0);

-- The endianness argument is accepted on both output forms.
SELECT raquetFromBinary(asBinary(raquet('\x01020304'::bytea, 2, 2,
         5193776270265024512::bigint, 'UINT8'), 'XDR'))
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

SELECT raquetFromHexWKB(asHexWKB(raquet('\x01020304'::bytea, 2, 2,
         5193776270265024512::bigint, 'UINT8'), 'NDR'))
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-------------------------------------------------------------------------------
-- raster (Hex)WKB round trip
--
-- A raster round-trips through its Well-Known Binary in either byte order,
-- with bands of one, two, four and eight bytes a pixel, and PostGIS reads the
-- big-endian form back to the same raster.
-------------------------------------------------------------------------------

WITH rast AS (
  SELECT ST_AddBand(ST_AddBand(ST_AddBand(ST_AddBand(
    ST_MakeEmptyRaster(2, 2, 0.0, 2.0, 1.0, -1.0, 0.5, 0.25, 3857),
    '8BUI'::text, 200::float8, 255::float8),
    '16BSI'::text, 258::float8, -9999::float8),
    '32BF'::text, 1.5::float8, NULL::float8),
    '64BF'::text, 1e10::float8, -1.5::float8) AS r)
SELECT rasterFromBinary(asBinary(r, 'XDR')) = r AS xdr,
  rasterFromBinary(asBinary(r, 'NDR')) = r AS ndr,
  rasterFromHexWKB(asHexWKB(r, 'XDR')) = r AS hex_xdr,
  rasterFromHexWKB(asHexWKB(r)) = r AS hex_native,
  ST_RastFromWKB(asBinary(r, 'XDR')) = r AS postgis_reads_xdr,
  asBinary(r) = ST_AsBinary(r) AND asHexWKB(r) = ST_AsHexWKB(r)
    AS native_as_postgis,
  get_byte(asBinary(r, 'NDR'), 0) AS ndr_first_byte,
  get_byte(asBinary(r, 'XDR'), 0) AS xdr_first_byte,
  asBinary(r, 'XDR') <> asBinary(r, 'NDR') AS orders_differ
FROM rast;

-- A string shorter than the raster it states is refused, not read past its end.
SELECT rasterFromHexWKB('0100');

-------------------------------------------------------------------------------
-- raquet accessors
-------------------------------------------------------------------------------

-- The accessors read back the georeferencing and layout the tile carries, so a
-- packaged tile needs none of the loose columns it was built from.
SELECT quadbin(tile), width(tile), height(tile), bandPixelType(tile), bandNoDataValue(tile)
FROM (SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
        'UINT8') AS tile) t;

-- Every pixel type name round-trips through the constructor and bandPixelType.
SELECT bandPixelType(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT8')),
       bandPixelType(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'INT16')),
       bandPixelType(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'INT32')),
       bandPixelType(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'FLOAT32')),
       bandPixelType(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'FLOAT64')),
       bandPixelType(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, 'INT8')),
       bandPixelType(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT16')),
       bandPixelType(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT32')),
       bandPixelType(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'INT64')),
       bandPixelType(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'UINT64')),
       bandPixelType(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'FLOAT16'));

-- The pixel size of a type is the one the specification gives it, so a band of
-- one pixel is exactly as many bytes wide.
SELECT raquet('\x0102030405060708'::bytea, 2, 1, 5193776270265024512::bigint,
  'UINT32') IS NOT NULL AS uint32_two_pixels,
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'INT8')
  IS NOT NULL AS int8_two_pixels,
       raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
  'FLOAT16') IS NOT NULL AS float16_one_pixel_spare_bytes;

-- The pixel type name is read without regard to case, so the lower-case
-- spelling the RaQuet specification gives a tile's type field is accepted as
-- it stands. The name reported back keeps the documented upper case.
SELECT bandPixelType(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  'uint8'));
SELECT bandPixelType(raquet('\x0102030405060708'::bytea, 2, 1,
  5193776270265024512::bigint, 'float32'));
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint8')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-- A pixel type is also accepted under the name PostGIS raster gives it, so the
-- band type an ST_BandPixelType call reports passes into the constructor as it
-- stands. The tile reports the name of the RaQuet specification.
SELECT bandPixelType(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, '8BUI')),
       bandPixelType(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, '16BSI')),
       bandPixelType(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, '32BF')),
       bandPixelType(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         '64BF')),
       bandPixelType(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, '16BF')),
       bandPixelType(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         '64BSI'));

-- The band type of a PostGIS raster carries into the constructor unchanged.
SELECT bandPixelType(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint,
  ST_BandPixelType(ST_AddBand(ST_MakeEmptyRaster(1, 1, 0, 0, 1), '32BF'), 1)));

-- The two spellings name the same tile.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '8BUI')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-- A PostGIS pixel type bounded to less than a byte names a uint8 band, since
-- PostGIS stores one a byte a pixel.
SELECT bandPixelType(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '1BB')),
       bandPixelType(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '2BUI')),
       bandPixelType(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '4BUI'));

-- The bound is the only thing such a name adds, so the tile is the uint8 one.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '4BUI')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint8');

-- A band type ST_BandPixelType reports for such a type carries in as it stands.
SELECT bandPixelType(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  ST_BandPixelType(ST_AddBand(ST_MakeEmptyRaster(1, 1, 0, 0, 1), '4BUI'), 1)));

-- An unknown name is still rejected, whatever its case.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint12');

-- The nodata value supplied to the constructor is the one reported back.
SELECT bandHasNoDataValue(tile), bandNoDataValue(tile)
FROM (SELECT raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8',
  -9999.0) AS tile) t;

-- A tile built without a nodata value states none, so its nodata value is NULL
-- rather than a number, and the constructor given that NULL rebuilds the tile.
SELECT bandHasNoDataValue(tile), bandNoDataValue(tile),
  raquet(pixels(tile), width(tile), height(tile), quadbin(tile),
    bandPixelType(tile), bandNoDataValue(tile)) = tile AS round_trips
FROM (SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
        'UINT8') AS tile) t;

-- The pixel bytes are returned in the layout the constructor accepts, so a
-- tile rebuilt from its own accessors equals the tile it came from.
SELECT pixels(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  'UINT8'));

SELECT raquet(pixels(tile), width(tile), height(tile), quadbin(tile),
         bandPixelType(tile), bandNoDataValue(tile)) = tile AS round_trips
FROM (SELECT raquet('\x0102030405060708'::bytea, 2, 2,
        5193776270265024512::bigint, 'INT16', -9999.0) AS tile) t;

-- A wider pixel type returns the whole band, not the pixel count.
SELECT length(pixels(raquet('\x0102030405060708'::bytea, 2, 1,
  5193776270265024512::bigint, 'FLOAT32')));

-------------------------------------------------------------------------------
-- raquet comparison
-------------------------------------------------------------------------------

-- Tiles agreeing on cell, layout and pixels are equal; any difference orders.
SELECT raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') =
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS eq,
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') <>
       raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS ne;

-- The ordering is on the QUADBIN cell first, then pixel type, width, height and
-- finally the pixel bytes.
SELECT raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') <
       raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS lt_pixels,
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') <=
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS le_equal,
       raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') >
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS gt_pixels,
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') >=
       raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8') AS ge_equal;

-- cmp returns the three-way comparison the btree operator class uses.
SELECT cmp(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8'),
           raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')) AS lt,
       cmp(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8'),
           raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')) AS eq,
       cmp(raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8'),
           raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')) AS gt;

-- Sorting and deduplication go through the btree and hash operator classes.
WITH tiles(tile) AS (VALUES
  (raquet('\x0103'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')),
  (raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')),
  (raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8')))
SELECT count(*) AS total, count(DISTINCT tile) AS distinct_tiles,
       (SELECT tile::text FROM tiles ORDER BY tile LIMIT 1) =
         raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint,
           'UINT8')::text AS smallest_sorts_first
FROM tiles;

-- Equal tiles hash equally, and the seeded hash varies with the seed.
SELECT hash(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint,
         'UINT8')) =
       hash(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint,
         'UINT8')) AS equal_tiles_hash_equally,
       hashExtended(raquet('\x0102'::bytea, 2, 1,
         5193776270265024512::bigint, 'UINT8'), 0) <>
       hashExtended(raquet('\x0102'::bytea, 2, 1,
         5193776270265024512::bigint, 'UINT8'), 1) AS seed_changes_hash;

-------------------------------------------------------------------------------
-- raquet conversion to stbox
-------------------------------------------------------------------------------

-- The footprint of a tile is the lon/lat envelope of its QUADBIN cell, so the
-- cast carries the tile extent without the pixels. The latitude bound is the
-- Web-Mercator limit, a transcendental value, so the box is rounded.
SELECT round(stbox(raquet('\x01020304'::bytea, 2, 2,
  5193776270265024512::bigint, 'UINT8')), 6);

-- The cast form and the function form agree.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
         'UINT8')::stbox =
       stbox(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
         'UINT8')) AS cast_equals_function;

-- The footprint depends only on the QUADBIN cell: tiles differing in pixels,
-- dimensions or pixel type share it.
SELECT stbox(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint,
         'UINT8')) =
       stbox(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
         'UINT8')) AS footprint_follows_the_cell;

-- Distinct cells give distinct footprints, and the tile is contained in its
-- own footprint envelope.
WITH t(a, b) AS (VALUES (
  raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT8'),
  raquet('\x01'::bytea, 1, 1, 5202501994543054848::bigint, 'UINT8')))
SELECT stbox(a) <> stbox(b) AS distinct_cells_distinct_footprints,
       stbox(a) && stbox(a) AS overlaps_itself
FROM t;

-------------------------------------------------------------------------------
-- raquet spatial indexing through the tile footprint
-------------------------------------------------------------------------------

CREATE TABLE test_raquet_tiles (id integer, tile raquet);
INSERT INTO test_raquet_tiles
SELECT g, raquet('\x01020304'::bytea, 2, 2, c, 'UINT8')
FROM (VALUES
  (1, 5193776270265024512::bigint), (2, 5202501994543054848::bigint),
  (3, 5203346419473186816::bigint), (4, 5203416788217364480::bigint)) v(g, c);

-- The footprint is indexable with the stbox operator classes, so a tile table
-- is searched by spatial overlap rather than by an equality test on the cell.
CREATE INDEX test_raquet_tiles_gist ON test_raquet_tiles USING gist (stbox(tile));
CREATE INDEX test_raquet_tiles_spgist ON test_raquet_tiles
  USING spgist (stbox(tile));

-- The extent of a set of tiles composes the footprint with the stbox extent.
SELECT round(extent(stbox(tile)), 6) FROM test_raquet_tiles;

-- Tiles are selected by overlap with a region of interest.
SELECT count(*) FROM test_raquet_tiles
WHERE stbox(tile) && stbox 'SRID=4326;STBOX X((0,0),(30,30))';

DROP TABLE test_raquet_tiles;

-------------------------------------------------------------------------------
-- numBands
-------------------------------------------------------------------------------

-- A single-band raster reports 1 band; adding a second band reports 2.
WITH rast1 AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8
  ) AS r
), rast2 AS (
  SELECT ST_AddBand(r, '32BF'::text, 0.0::float8, NULL::float8) AS r
  FROM rast1
)
SELECT numBands((SELECT r FROM rast1)) AS num_bands_one,
       numBands((SELECT r FROM rast2)) AS num_bands_two;

-------------------------------------------------------------------------------
-- Shape of a raster
-------------------------------------------------------------------------------

-- The shape of a raster is the one PostGIS reads from the same grid: a banded
-- raster in EPSG:4326, and a band-free one in EPSG:3857 whose grid is skewed,
-- so no two of its nine numbers coincide.
WITH rast(r) AS (VALUES
  (ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8)),
  (ST_MakeEmptyRaster(3, 2, 10.0, 20.0, 1.5, -2.0, 0.5, 0.25, 3857)))
SELECT width(r), height(r), SRID(r), upperLeftX(r), upperLeftY(r), scaleX(r),
  scaleY(r), skewX(r), skewY(r),
  width(r) = ST_Width(r) AND height(r) = ST_Height(r) AND
  SRID(r) = ST_SRID(r) AND upperLeftX(r) = ST_UpperLeftX(r) AND
  upperLeftY(r) = ST_UpperLeftY(r) AND scaleX(r) = ST_ScaleX(r) AND
  scaleY(r) = ST_ScaleY(r) AND skewX(r) = ST_SkewX(r) AND
  skewY(r) = ST_SkewY(r) AS as_postgis
FROM rast;

-------------------------------------------------------------------------------
-- Bands of a raster
-------------------------------------------------------------------------------

-- A band reports its pixel type under the name the RaQuet specification gives
-- it, the name a raquet tile of that type reports, and its nodata value as
-- PostGIS reports it: the value the band states, and NULL for a band stating
-- none.
WITH rast(r) AS (VALUES
  (ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8)),
  (ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '16BSI'::text, 0.0::float8, -9999::float8)))
SELECT bandPixelType(r), ST_BandPixelType(r) AS postgis_name,
  bandHasNoDataValue(r), bandNoDataValue(r),
  bandNoDataValue(r) IS NOT DISTINCT FROM ST_BandNoDataValue(r) AS as_postgis
FROM rast;

-- The band is named by its number, so the second band of a raster answers for
-- itself, and a band the raster does not have is an error.
WITH rast AS (
  SELECT ST_AddBand(ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0,
    0.0, 0.0, 4326), '32BF'::text, 0.0::float8, NULL::float8),
    '8BUI'::text, 0::float8, 255::float8) AS r)
SELECT bandPixelType(r, 2), bandHasNoDataValue(r, 2), bandNoDataValue(r, 2)
FROM rast;

WITH rast AS (
  SELECT ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0,
    4326), '32BF'::text, 0.0::float8, NULL::float8) AS r)
SELECT bandNoDataValue(r, 2) FROM rast;

-------------------------------------------------------------------------------
-- reclass
-------------------------------------------------------------------------------

-- A band of values becomes a band of classes. The 3x3 band below holds
-- 10, 20, 30 / 40, 50, 60 / 70, 80, 90, and a range written plainly is HALF
-- OPEN AT THE TOP: 0-50 takes 10, 20, 30 and 40 but NOT 50, which falls in
-- neither range and is left unmapped.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,50,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
)
SELECT (s).count AS n, (s).sum AS total, (s).min AS lo, (s).max AS hi
FROM (SELECT ST_SummaryStats(reclass(r, 1, '0-50:1, 51-100:2', '32BF', -9999))
  AS s FROM rast) t;

-- A closing bracket includes the high bound, so [50-100] takes the 50 the
-- plain form dropped and every pixel is mapped.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,50,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
)
SELECT (s).count AS n, (s).sum AS total
FROM (SELECT ST_SummaryStats(
  reclass(r, 1, '[0-50):1, [50-100]:2', '32BF', -9999)) AS s FROM rast) t;

-- A band the raster does not have, and an expression stating no mapping, are
-- both errors.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT reclass(r, 2, '0-50:1', '32BF') FROM rast;
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT reclass(r, 1, '0-50', '32BF') FROM rast;

-------------------------------------------------------------------------------
-- clip
-------------------------------------------------------------------------------

-- A raster keeps the pixels a geometry covers, the others answering nodata.
-- The polygon's edges fall on pixel edges and cover the upper left 2x2 pixels
-- of the band holding 10, 20, 30 / 40, 50, 60 / 70, 80, 90, so the answer is
-- unambiguous; PostGIS's ST_Clip is the oracle, with and without cropping.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,50,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
), g AS (
  SELECT ST_GeomFromText('POLYGON((0 1,2 1,2 3,0 3,0 1))', 4326) AS g
)
SELECT ST_DumpValues(clip(r, g), 1) AS cropped,
  ST_DumpValues(clip(r, g, false), 1) AS kept_extent,
  ST_DumpValues(clip(r, g), 1) = ST_DumpValues(ST_Clip(r, g), 1)
    AS cropped_as_postgis,
  ST_DumpValues(clip(r, g, false), 1) = ST_DumpValues(ST_Clip(r, g, false), 1)
    AS kept_as_postgis
FROM rast, g;

-- A geometry in another reference system is refused rather than read as if
-- its coordinates were the raster's.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT clip(r, ST_GeomFromText('POLYGON((0 1,2 1,2 3,0 3,0 1))', 3857))
FROM rast;

-------------------------------------------------------------------------------
-- transform and rescale
-------------------------------------------------------------------------------

-- A raster carried into another reference system states the same coverage
-- read through that system. PostGIS's ST_Transform is the oracle, on the grid
-- and on the values alike, for the default nearest neighbour and for bilinear
-- resampling named without regard to case.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,50,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
)
SELECT ST_SRID(transform(r, 3857)) AS srid,
  ST_MetaData(transform(r, 3857)) = ST_MetaData(ST_Transform(r, 3857))
    AS grid_as_postgis,
  ST_DumpValues(transform(r, 3857), 1) = ST_DumpValues(ST_Transform(r, 3857), 1)
    AS values_as_postgis,
  ST_DumpValues(transform(r, 3857, 'bilinear'), 1) =
    ST_DumpValues(ST_Transform(r, 3857, 'Bilinear'), 1) AS bilinear_as_postgis
FROM rast;

-- Halving the pixel size doubles the width and the height. ST_Rescale is the
-- oracle, for a Y scale stated negative as the geotransform states it and
-- positive as PostGIS users write it.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,50,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
)
SELECT ST_Width(rescale(r, 0.5, -0.5)) AS w, ST_Height(rescale(r, 0.5, -0.5)) AS h,
  ST_MetaData(rescale(r, 0.5, -0.5)) = ST_MetaData(ST_Rescale(r, 0.5, -0.5))
    AS north_up_grid_as_postgis,
  ST_DumpValues(rescale(r, 0.5, -0.5), 1) =
    ST_DumpValues(ST_Rescale(r, 0.5, -0.5), 1) AS north_up_values_as_postgis,
  ST_MetaData(rescale(r, 0.5, 0.5)) = ST_MetaData(ST_Rescale(r, 0.5, 0.5))
    AS positive_grid_as_postgis,
  ST_DumpValues(rescale(r, 0.5, 0.5), 1) =
    ST_DumpValues(ST_Rescale(r, 0.5, 0.5), 1) AS positive_values_as_postgis
FROM rast;

-- A reprojection needs a target system.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT transform(r, 0) FROM rast;

-------------------------------------------------------------------------------
-- summaryStats
-------------------------------------------------------------------------------

-- What a band's pixels amount to, taken in one pass, is the record PostGIS's
-- ST_SummaryStats answers: with the nodata pixel in the centre left out, and
-- counted as the value it holds.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[10,20,30], ARRAY[40,-9999,60], ARRAY[70,80,90]]::float8[][]
  ) AS r
)
SELECT (s).count, (s).sum, (s).mean, round((s).stddev::numeric, 6) AS stddev,
  (s).min, (s).max, as_postgis, all_pixels_as_postgis
FROM (SELECT summaryStats(r) AS s,
    summaryStats(r) = ST_SummaryStats(r) AS as_postgis,
    summaryStats(r, 1, false) = ST_SummaryStats(r, 1, false)
      AS all_pixels_as_postgis
  FROM rast) t;

-- A band whose every pixel is nodata, beside what ST_SummaryStats answers for
-- it, and a band the raster does not have.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, -9999::float8, -9999::float8) AS r
)
SELECT summaryStats(r) AS stats, ST_SummaryStats(r) AS postgis_stats FROM rast;
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT summaryStats(r, 2) FROM rast;

-------------------------------------------------------------------------------
-- dumpAsPolygons
-------------------------------------------------------------------------------

-- A band states a value per pixel; its polygons state the regions those values
-- cover, one per group of pixels sharing a value, as PostGIS's
-- ST_DumpAsPolygons answers them. The band holds three regions and a nodata
-- pixel. Each polygon carries the reference system of the raster where
-- ST_DumpAsPolygons states none, so the comparison sets it on PostGIS's
-- answer, and a full join shows a region either answer lacks.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[1,1,2], ARRAY[1,2,2], ARRAY[3,3,-9999]]::float8[][]
  ) AS r
), ours AS (
  SELECT (d).val, (d).geom FROM (SELECT dumpAsPolygons(r) AS d FROM rast) t
), pg AS (
  SELECT (d).val, ST_SetSRID((d).geom, 4326) AS geom
  FROM (SELECT ST_DumpAsPolygons(r) AS d FROM rast) t
)
SELECT o.val, ST_SRID(o.geom) AS srid, ST_Area(o.geom) AS area,
  ST_Equals(o.geom, p.geom) AS as_postgis
FROM ours o FULL JOIN pg p ON o.val = p.val
ORDER BY o.val;

-- With the nodata pixels counted, the nodata pixel is a region of its own, as
-- ST_DumpAsPolygons counts it; a band whose every pixel is nodata covers
-- nothing, which both answer with no polygon.
WITH rast AS (
  SELECT ST_SetValues(
    ST_AddBand(
      ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
      '32BF'::text, 0.0::float8, -9999::float8),
    1, 1, 1, ARRAY[ARRAY[1,1,2], ARRAY[1,2,2], ARRAY[3,3,-9999]]::float8[][]
  ) AS r
), empty AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, -9999::float8, -9999::float8) AS r
)
SELECT (SELECT count(*) FROM rast, dumpAsPolygons(r, 1, false)) AS regions,
  (SELECT count(*) FROM rast, ST_DumpAsPolygons(r, 1, false)) AS postgis_regions,
  (SELECT count(*) FROM empty, dumpAsPolygons(r)) AS empty_regions,
  (SELECT count(*) FROM empty, ST_DumpAsPolygons(r)) AS postgis_empty_regions;

-- A band the raster does not have is an error.
WITH rast AS (
  SELECT ST_AddBand(
    ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326),
    '32BF'::text, 0.0::float8, NULL::float8) AS r
)
SELECT count(*) FROM rast, dumpAsPolygons(r, 2);

-------------------------------------------------------------------------------
-- raster conversion to stbox
-------------------------------------------------------------------------------

-- The extent of an axis-aligned raster runs from its origin over its scaled
-- size, and the cast form agrees with the function form.
WITH rast AS (
  SELECT ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326) AS r
)
SELECT stbox(r) AS box, stbox(r) = r::stbox AS cast_agrees FROM rast;

-- The extent bears the rotation of the geotransform, so a skewed grid reaches
-- past the box its scale alone would give and the two disagree.
WITH plain AS (
  SELECT ST_MakeEmptyRaster(2, 2, 0.0, 2.0, 1.0, -1.0, 0.0, 0.0, 4326) AS r
), skewed AS (
  SELECT ST_MakeEmptyRaster(2, 2, 0.0, 2.0, 1.0, -1.0, 1.0, 0.0, 4326) AS r
)
SELECT stbox((SELECT r FROM plain)) AS plain_box,
       stbox((SELECT r FROM skewed)) AS skewed_box,
       stbox((SELECT r FROM plain)) <> stbox((SELECT r FROM skewed))
         AS skew_moves_the_extent;

-- The box states the reference system the raster is in, not a fixed one.
SELECT stbox(ST_MakeEmptyRaster(2, 2, 0.0, 2.0, 1.0, -1.0, 0.0, 0.0, 3857))
  AS box_in_3857;

-- The footprint carries the stbox operators, so a raster column is searched by
-- spatial overlap.
SELECT stbox(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326)) &&
       stbox 'SRID=4326;STBOX X((1,1),(2,2))' AS overlaps_region;

/*****************************************************************************/
