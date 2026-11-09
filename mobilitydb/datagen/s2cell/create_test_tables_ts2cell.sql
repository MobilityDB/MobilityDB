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
 * @brief Build the test tables for the static and temporal S2 cell index
 * types
 * @details This is the OFFLINE regeneration job that produces the frozen
 * pg_dump fixture test/s2cell/data/load_s2cell.sql.xz; it is NOT part of the
 * mobilitydb_datagen extension (only the random_* helpers are), as
 * create_test_tables_quadbin of datagen/quadbin is not.
 */

CREATE OR REPLACE FUNCTION create_test_tables_s2cell(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
  perc = size / 10;

------------------------------------------------------------------------------
-- Static S2 cell index type
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_s2cell;
CREATE TABLE tbl_s2cell AS
SELECT k, random_s2cell(0, 10) AS cell
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_s2cellset;
CREATE TABLE tbl_s2cellset AS
/* Add perc NULL values */
SELECT k, NULL AS s
FROM generate_series(1, perc) AS k UNION
SELECT k, random_s2cell_set(0, 10, 1, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal S2 cell index type
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_ts2cell_inst;
CREATE TABLE tbl_ts2cell_inst AS
SELECT k, random_ts2cell_inst(0, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_ts2cell_discseq;
CREATE TABLE tbl_ts2cell_discseq AS
SELECT k, random_ts2cell_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_ts2cell_seq;
CREATE TABLE tbl_ts2cell_seq AS
SELECT k, random_ts2cell_seq(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_ts2cell_seqset;
CREATE TABLE tbl_ts2cell_seqset AS
SELECT k, random_ts2cell_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_ts2cell;
CREATE TABLE tbl_ts2cell(k, temp) AS
(SELECT k, inst FROM tbl_ts2cell_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_ts2cell_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_ts2cell_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_ts2cell_seqset LIMIT size / 4);

------------------------------------------------------------------------------
-- Geodetic spatiotemporal box (operand for the bounding-box operator tests).
-- A ts2cell bounding box is a 2D geodetic STBox at SRID 4326, as the one of a
-- th3index is, so the operand is the 2D geodetic box the H3 fixture builds as
-- tbl_geodstbox; it is named for this family so that the two fixtures load
-- into one database side by side.
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geodstbox_s2cell;
CREATE TABLE tbl_geodstbox_s2cell AS
SELECT k, random_geodstbox(-170, 170, -80, 80, 0, 1000, '2001-01-01',
  '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

------------------------------------------------------------------------------
-- Large temporal S2 cell index table for the index-operator tests. It
-- must be big enough that the planner chooses a GiST/SP-GiST index scan over a
-- sequential scan, otherwise the index tests exercise nothing. The threshold
-- is around 10 000 rows; size * 100 + 100 keeps a small margin above it
-- (10 100 at the default size of 100).
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_ts2cell_big;
CREATE TABLE tbl_ts2cell_big(k, temp) AS
SELECT k, random_ts2cell_seq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS temp
FROM generate_series(1, size * 100 + 100) k;

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_s2cell(100);
/*
SELECT * FROM tbl_s2cell LIMIT 3;
SELECT * FROM tbl_s2cellset LIMIT 3;
SELECT * FROM tbl_ts2cell_inst LIMIT 3;
SELECT * FROM tbl_ts2cell_discseq LIMIT 3;
SELECT * FROM tbl_ts2cell_seq LIMIT 3;
SELECT * FROM tbl_ts2cell_seqset LIMIT 3;
SELECT * FROM tbl_ts2cell LIMIT 3;
*/
