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
