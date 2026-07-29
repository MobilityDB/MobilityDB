/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/*
 * random_raquet.sql
 * Basic synthetic data generator function for the Raquet raster tile type.
 * Tiles are always VALID: each is built by the raquet constructor from a
 * QUADBIN cell of the Web-Mercator tile pyramid and a pixel array of exactly
 * the size the dimensions and pixel type call for. The QUADBIN cell is
 * Morton-encoded here rather than taken from the QUADBIN family, so the
 * generator follows the raster family in carrying its own tile arithmetic and
 * is available whether or not that family is built.
 */

------------------------------------------------------------------------------
-- Raquet raster tile type
------------------------------------------------------------------------------

/**
 * @brief Morton-encode Web-Mercator tile coordinates into a QUADBIN cell
 * @param[in] tx, ty Tile column and row at zoom @p tz
 * @param[in] tz Zoom level (0..26)
 * @note Mirrors the spread_bits encoding of the raster kernel: the coordinates
 * are scaled to the 2^26 grid, their bits spread into alternating positions,
 * and interleaved under the header, mode bit and zoom
 */
DROP FUNCTION IF EXISTS raquet_tile_to_cell;
CREATE FUNCTION raquet_tile_to_cell(tx bigint, ty bigint, tz int)
  RETURNS bigint AS $$
DECLARE
  xx bigint; yy bigint;
BEGIN
  IF tz < 0 OR tz > 26 THEN
    RAISE EXCEPTION 'zoom must satisfy 0 <= zoom <= 26: %', tz;
  END IF;
  xx := tx << (26 - tz);
  yy := ty << (26 - tz);
  xx := (xx | (xx << 16)) & x'0000FFFF0000FFFF'::bigint;
  xx := (xx | (xx <<  8)) & x'00FF00FF00FF00FF'::bigint;
  xx := (xx | (xx <<  4)) & x'0F0F0F0F0F0F0F0F'::bigint;
  xx := (xx | (xx <<  2)) & x'3333333333333333'::bigint;
  xx := (xx | (xx <<  1)) & x'5555555555555555'::bigint;
  yy := (yy | (yy << 16)) & x'0000FFFF0000FFFF'::bigint;
  yy := (yy | (yy <<  8)) & x'00FF00FF00FF00FF'::bigint;
  yy := (yy | (yy <<  4)) & x'0F0F0F0F0F0F0F0F'::bigint;
  yy := (yy | (yy <<  2)) & x'3333333333333333'::bigint;
  yy := (yy | (yy <<  1)) & x'5555555555555555'::bigint;
  RETURN x'4000000000000000'::bigint | (1::bigint << 59) |
    (tz::bigint << 52) | (xx | (yy << 1));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT raquet_tile_to_cell(1, 0, 1);
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random valid Raquet tile at a random zoom level
 * @param[in] lowzoom, highzoom Inclusive bounds of the zoom level (0..26)
 * @param[in] width, height Tile dimensions in pixels
 * @note The pixel array holds one byte per pixel, matching the UINT8 pixel
 * type, and the tile column and row are drawn from the 2^zoom tiles the world
 * is divided into at that zoom
 */
DROP FUNCTION IF EXISTS random_raquet;
CREATE FUNCTION random_raquet(lowzoom int DEFAULT 1, highzoom int DEFAULT 8,
    width int DEFAULT 4, height int DEFAULT 4)
  RETURNS raquet AS $$
DECLARE
  zoom int;
  ntiles bigint;
  pixels bytea;
BEGIN
  IF lowzoom < 0 OR highzoom > 26 OR lowzoom > highzoom THEN
    RAISE EXCEPTION 'lowzoom/highzoom must satisfy 0 <= lowzoom <= highzoom <= 26: %, %',
      lowzoom, highzoom;
  END IF;
  IF width < 1 OR height < 1 THEN
    RAISE EXCEPTION 'width and height must be positive: %, %', width, height;
  END IF;
  zoom := random_int(lowzoom, highzoom);
  ntiles := (1::bigint << zoom) - 1;
  /* One byte per pixel, assembled through hex: chr() would render a value
   * above 127 as a multi-byte character in the database encoding */
  SELECT decode(string_agg(lpad(to_hex(random_int(0, 255)), 2, '0'), ''), 'hex')
  INTO pixels
  FROM generate_series(1, width * height);
  RETURN raquet(pixels, width, height,
    raquet_tile_to_cell(random_int(0, ntiles::int), random_int(0, ntiles::int),
      zoom), 'UINT8');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_raquet(1, 8) AS tile
FROM generate_series(1, 10) k;
*/

------------------------------------------------------------------------------
