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

-- A 3×3 synthetic raster (SRID 4326, 1° pixels) used throughout this file.
-- Row 1 (lat 2–3): pixel values 10, 20, 30  (left → right)
-- Row 2 (lat 1–2): pixel values 40, 50, 60
-- Row 3 (lat 0–1): pixel values 70, 80, 90

-------------------------------------------------------------------------------
-- raster_value — basic usage
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
SELECT raster_value(r,
  tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(2.5 2.5)@2000-01-02 00:00:00+00, POINT(0.5 0.5)@2000-01-03 00:00:00+00]'
)::text AS result
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
SELECT raster_value(r,
  tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(5.5 5.5)@2000-01-02 00:00:00+00, POINT(0.5 0.5)@2000-01-03 00:00:00+00]'
)::text AS result
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
SELECT raster_value(r,
  tgeompoint 'SRID=4326;[POINT(5.5 5.5)@2000-01-01 00:00:00+00, POINT(6.5 6.5)@2000-01-02 00:00:00+00]'
)::text AS result
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
SELECT raster_value(r,
  tgeompoint 'SRID=4326;{POINT(1.5 1.5)@2000-01-01 00:00:00+00}'
)::text AS result
FROM rast;

-------------------------------------------------------------------------------
-- atRasterValue / minusRasterValue / eRasterValue / aRasterValue
-------------------------------------------------------------------------------

-- Shared fixture: 3×3 raster, pixel values 10..90 (row-major).
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
SELECT asText(atRasterValue(
  tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(1.5 1.5)@2000-01-02 00:00:00+00, POINT(0.5 0.5)@2000-01-03 00:00:00+00]',
  r, floatspan '[40, 90]'))::text AS result
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
  tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(1.5 1.5)@2000-01-02 00:00:00+00, POINT(0.5 0.5)@2000-01-03 00:00:00+00]',
  r, floatspan '[40, 90]'))::text AS result
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
  eRasterValue(r,
    tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(0.5 0.5)@2000-01-02 00:00:00+00]',
    floatspan '[70, 90]') AS e_true,
  eRasterValue(r,
    tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(0.5 0.5)@2000-01-02 00:00:00+00]',
    floatspan '[80, 90]') AS e_false
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
  aRasterValue(r,
    tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(0.5 0.5)@2000-01-02 00:00:00+00]',
    floatspan '[70, 90]') AS a_false,
  aRasterValue(r,
    tgeompoint 'SRID=4326;[POINT(0.5 2.5)@2000-01-01 00:00:00+00, POINT(0.5 0.5)@2000-01-02 00:00:00+00]',
    floatspan '[0, 100]') AS a_true
FROM rast;

-------------------------------------------------------------------------------
-- trajectory_quadbins
-------------------------------------------------------------------------------

-- Three distinct longitudes at zoom 3 produce three distinct QUADBIN cells.
SELECT array_length(
  trajectory_quadbins(
    tgeompointFromText('SRID=4326;{Point(-60.0 45.0)@2024-01-01 00:00:00+00, Point(0.0 45.0)@2024-01-02 00:00:00+00, Point(60.0 45.0)@2024-01-03 00:00:00+00}'),
    3
  ), 1
) AS num_distinct_tiles;

-- Two instants in the same tile → deduplicated to 1 cell.
SELECT array_length(
  trajectory_quadbins(
    tgeompointFromText('SRID=4326;{Point(-60.0 45.0)@2024-01-01 00:00:00+00, Point(-61.0 44.0)@2024-01-02 00:00:00+00}'),
    3
  ), 1
) AS num_distinct_tiles;

-- Invalid zoom level raises an error.
SELECT trajectory_quadbins(
  tgeompointFromText('SRID=4326;{Point(0.0 0.0)@2024-01-01 00:00:00+00}'),
  16
);

-------------------------------------------------------------------------------
-- raster_tile_value_quadbin
-------------------------------------------------------------------------------

-- A 2×2 UINT8 chip for tile (x=1, y=0, zoom=1): lon 0°..180°, lat 0°..85°.
-- Mercator midpoint ≈ 66.5° separates row 0 (upper) from row 1 (lower).
-- Pixel layout (row-major):  [1, 2]   row 0 (lat > 66.5°)
--                             [3, 4]   row 1 (lat < 66.5°)
-- POINT(45  75) → col=0, row=0 → 1
-- POINT(135 75) → col=1, row=0 → 2
-- POINT(45  10) → col=0, row=1 → 3
-- POINT(-45 75) → lon outside 0..180 → dropped
SELECT raster_tile_value_quadbin(
  '\x01020304'::bytea,         -- 4 UINT8 pixels: 1,2,3,4
  2::integer,                  -- width
  2::integer,                  -- height
  5193776270265024512::bigint, -- quadbin_tile_to_cell(1,0,1)
  'UINT8',
  0.0, false,
  tgeompointFromText('SRID=4326;{Point(45.0 75.0)@2024-01-01 00:00:00+00, Point(135.0 75.0)@2024-01-02 00:00:00+00, Point(45.0 10.0)@2024-01-03 00:00:00+00, Point(-45.0 75.0)@2024-01-04 00:00:00+00}')
)::text AS result;
