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
  floatspan '[40, 90]'))::text AS result
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
-- raquet accessors
-------------------------------------------------------------------------------

-- The accessors read back the georeferencing and layout the tile carries, so a
-- packaged tile needs none of the loose columns it was built from.
SELECT quadbin(tile), width(tile), height(tile), pixtype(tile), nodata(tile)
FROM (SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
        'UINT8') AS tile) t;

-- Every pixel type name round-trips through the constructor and pixtype.
SELECT pixtype(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT8')),
       pixtype(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'INT16')),
       pixtype(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'INT32')),
       pixtype(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'FLOAT32')),
       pixtype(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'FLOAT64')),
       pixtype(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, 'INT8')),
       pixtype(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT16')),
       pixtype(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, 'UINT32')),
       pixtype(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'INT64')),
       pixtype(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         'UINT64')),
       pixtype(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, 'FLOAT16'));

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
SELECT pixtype(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  'uint8'));
SELECT pixtype(raquet('\x0102030405060708'::bytea, 2, 1,
  5193776270265024512::bigint, 'float32'));
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint8')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-- A pixel type is also accepted under the name PostGIS raster gives it, so the
-- band type an ST_BandPixelType call reports passes into the constructor as it
-- stands. The tile reports the name of the RaQuet specification.
SELECT pixtype(raquet('\x01'::bytea, 1, 1, 5193776270265024512::bigint, '8BUI')),
       pixtype(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, '16BSI')),
       pixtype(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint, '32BF')),
       pixtype(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         '64BF')),
       pixtype(raquet('\x0102'::bytea, 1, 1, 5193776270265024512::bigint, '16BF')),
       pixtype(raquet('\x0102030405060708'::bytea, 1, 1, 5193776270265024512::bigint,
         '64BSI'));

-- The band type of a PostGIS raster carries into the constructor unchanged.
SELECT pixtype(raquet('\x01020304'::bytea, 1, 1, 5193776270265024512::bigint,
  ST_BandPixelType(ST_AddBand(ST_MakeEmptyRaster(1, 1, 0, 0, 1), '32BF'), 1)));

-- The two spellings name the same tile.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '8BUI')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'UINT8');

-- A PostGIS pixel type bounded to less than a byte names a uint8 band, since
-- PostGIS stores one a byte a pixel.
SELECT pixtype(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '1BB')),
       pixtype(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '2BUI')),
       pixtype(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '4BUI'));

-- The bound is the only thing such a name adds, so the tile is the uint8 one.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, '4BUI')
       = raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint8');

-- A band type ST_BandPixelType reports for such a type carries in as it stands.
SELECT pixtype(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  ST_BandPixelType(ST_AddBand(ST_MakeEmptyRaster(1, 1, 0, 0, 1), '4BUI'), 1)));

-- An unknown name is still rejected, whatever its case.
SELECT raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint, 'uint12');

-- The nodata sentinel supplied to the constructor is the one reported back.
SELECT nodata(raquet('\x0102'::bytea, 2, 1, 5193776270265024512::bigint, 'UINT8',
  -9999.0));

-- The pixel bytes are returned in the layout the constructor accepts, so a
-- tile rebuilt from its own accessors equals the tile it came from.
SELECT pixels(raquet('\x01020304'::bytea, 2, 2, 5193776270265024512::bigint,
  'UINT8'));

SELECT raquet(pixels(tile), width(tile), height(tile), quadbin(tile),
         pixtype(tile), nodata(tile)) = tile AS round_trips
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

/*****************************************************************************/
