/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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

/**
 * @file
 * @brief Build the test tables for the static and temporal H3 cell index types
 * @details This
 * is the OFFLINE regeneration job that produces the frozen fixture
 * test/h3/data/<table>.data; it is NOT part of the mobilitydb_datagen
 * extension (only the random_* helpers are).
 */

CREATE OR REPLACE FUNCTION create_test_tables_h3index(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
  perc = size / 10;

------------------------------------------------------------------------------
-- Static H3 cell index type
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_h3index;
CREATE TABLE tbl_h3index AS
SELECT k, random_h3index(0, 10) AS h3
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_h3indexset;
CREATE TABLE tbl_h3indexset AS
/* Add perc NULL values */
SELECT k, NULL AS s
FROM generate_series(1, perc) AS k UNION
SELECT k, random_h3index_set(0, 10, 1, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal H3 cell index type
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_inst;
CREATE TABLE tbl_th3index_inst AS
SELECT k, random_th3index_inst(0, 10, '2001-01-01', '2001-12-31') AS temp
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_th3index_discseq;
CREATE TABLE tbl_th3index_discseq AS
SELECT k, random_th3index_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS temp
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_th3index_contseq;
CREATE TABLE tbl_th3index_contseq AS
SELECT k, random_th3index_seq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS temp
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_th3index_seqset;
CREATE TABLE tbl_th3index_seqset AS
SELECT k, random_th3index_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS temp
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_th3index;
CREATE TABLE tbl_th3index(k, temp) AS
(SELECT k, temp FROM tbl_th3index_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, temp FROM tbl_th3index_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, temp FROM tbl_th3index_contseq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, temp FROM tbl_th3index_seqset LIMIT size / 4);

------------------------------------------------------------------------------
-- Geodetic spatiotemporal box (operand for the bounding-box operator tests).
-- A 2D geodetic STBox at SRID 4326 (geodetic = true, no Z), matching the 2D
-- geodetic bounding box of th3index. NOT the 3D tbl_geodstbox3d.
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geodstbox;
CREATE TABLE tbl_geodstbox AS
SELECT k, random_geodstbox(-170, 170, -80, 80, 0, 1000, '2001-01-01',
  '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

------------------------------------------------------------------------------
-- The tables of the index tests: big enough that the planner chooses a
-- GiST/SP-GiST index scan over a sequential scan, otherwise the index tests
-- exercise nothing. The threshold is around 10 000 rows; size * 100 + 100
-- keeps a small margin above it (10 100 at the default size of 100). The four
-- quarters of a temporal table are the four subtypes, as in its table above,
-- each made by the call that makes the table of that subtype.
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_big;
CREATE TABLE tbl_th3index_big(k, temp) AS
SELECT k, CASE (k - 1) * 4 / (size * 100 + 100)
  WHEN 0 THEN random_th3index_inst(0, 10, '2001-01-01', '2001-12-31')
  WHEN 1 THEN random_th3index_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10)
  WHEN 2 THEN random_th3index_seq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10)
  ELSE random_th3index_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10)
  END AS temp
FROM generate_series(1, size * 100 + 100) k;

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_h3index(100);
/*
SELECT * FROM tbl_h3index LIMIT 3;
SELECT * FROM tbl_h3indexset LIMIT 3;
SELECT * FROM tbl_th3index_inst LIMIT 3;
SELECT * FROM tbl_th3index_discseq LIMIT 3;
SELECT * FROM tbl_th3index_contseq LIMIT 3;
SELECT * FROM tbl_th3index_seqset LIMIT 3;
SELECT * FROM tbl_th3index LIMIT 3;
*/
