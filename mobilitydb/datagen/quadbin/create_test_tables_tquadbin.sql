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
 * create_test_tables_tquadbin.sql
 * Build the test tables for the static and temporal QUADBIN cell index types.
 * This is the OFFLINE regeneration job that produces the frozen pg_dump
 * fixture test/quadbin/data/load_quadbin.sql.xz; it is NOT part of the
 * mobilitydb_datagen extension (only the random_* helpers are). The geodetic
 * spatiotemporal box operand (tbl_geodstbox) used by the bounding-box operator
 * tests is provided by the shared geo/point fixture, so it is NOT regenerated
 * here.
 */

CREATE OR REPLACE FUNCTION create_test_tables_quadbin(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
  perc = size / 10;

------------------------------------------------------------------------------
-- Static QUADBIN cell index type
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_quadbin;
CREATE TABLE tbl_quadbin AS
SELECT k, random_quadbin(0, 10) AS qb
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_quadbinset;
CREATE TABLE tbl_quadbinset AS
/* Add perc NULL values */
SELECT k, NULL AS s
FROM generate_series(1, perc) AS k UNION
SELECT k, random_quadbin_set(0, 10, 1, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal QUADBIN cell index type
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tquadbin_inst;
CREATE TABLE tbl_tquadbin_inst AS
SELECT k, random_tquadbin_inst(0, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tquadbin_discseq;
CREATE TABLE tbl_tquadbin_discseq AS
SELECT k, random_tquadbin_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tquadbin_seq;
CREATE TABLE tbl_tquadbin_seq AS
SELECT k, random_tquadbin_seq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tquadbin_seqset;
CREATE TABLE tbl_tquadbin_seqset AS
SELECT k, random_tquadbin_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tquadbin;
CREATE TABLE tbl_tquadbin(k, temp) AS
(SELECT k, inst FROM tbl_tquadbin_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tquadbin_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tquadbin_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tquadbin_seqset LIMIT size / 4);

------------------------------------------------------------------------------
-- Planar spatiotemporal box at SRID 4326 (operand for the bounding-box
-- operator tests). A tquadbin bounding box is a PLANAR STBox carrying SRID 4326
-- (Web-Mercator lon/lat domain), not a geodetic box like th3index; the operand
-- must therefore be a planar STBox at the same SRID (a geodetic box raises
-- "mixed planar and geodetic", a SRID-0 box raises "mixed SRID"). This is a
-- QUADBIN-specific table (the shared geo/point fixture's tbl_stbox is SRID 0),
-- keeping the QUADBIN fixture self-contained.
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_stbox_quadbin;
CREATE TABLE tbl_stbox_quadbin AS
SELECT k, random_stbox(-170, 170, -80, 80, '2001-01-01', '2001-12-31', 10, 10,
  4326) AS b
FROM generate_series(1, size) k;

------------------------------------------------------------------------------
-- Large temporal QUADBIN cell index table for the index-operator tests. It
-- must be big enough that the planner chooses a GiST/SP-GiST index scan over a
-- sequential scan, otherwise the index tests exercise nothing. The threshold
-- is around 10 000 rows; size * 100 + 100 keeps a small margin above it
-- (10 100 at the default size of 100).
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tquadbin_big;
CREATE TABLE tbl_tquadbin_big(k, temp) AS
SELECT k, random_tquadbin_seq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS temp
FROM generate_series(1, size * 100 + 100) k;

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_quadbin(100);
/*
SELECT * FROM tbl_quadbin LIMIT 3;
SELECT * FROM tbl_quadbinset LIMIT 3;
SELECT * FROM tbl_tquadbin_inst LIMIT 3;
SELECT * FROM tbl_tquadbin_discseq LIMIT 3;
SELECT * FROM tbl_tquadbin_seq LIMIT 3;
SELECT * FROM tbl_tquadbin_seqset LIMIT 3;
SELECT * FROM tbl_tquadbin LIMIT 3;
*/
