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
 * create_test_tables_raquet.sql
 * Build the test table for the Raquet raster tile type. This is the OFFLINE
 * regeneration job that produces the frozen pg_dump fixture
 * test/raster/data/load_raquet.sql.xz; it is NOT part of the
 * mobilitydb_datagen extension (only the random_* helpers are).
 */

CREATE OR REPLACE FUNCTION create_test_tables_raquet(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
  perc = size / 10;

------------------------------------------------------------------------------
-- Raquet raster tile type
------------------------------------------------------------------------------

/* Tiles spread over zoom levels 1 to 8, so that the table holds both tiles
 * that partition the plane at one zoom and tiles that overlap across zooms */
DROP TABLE IF EXISTS tbl_raquet;
CREATE TABLE tbl_raquet AS
/* Add perc NULL values */
SELECT k, NULL AS tile
FROM generate_series(1, perc) AS k UNION
SELECT k, random_raquet(1, 8)
FROM generate_series(perc+1, size) AS k;

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_raquet(100);
/*
SELECT * FROM tbl_raquet LIMIT 3;
*/
