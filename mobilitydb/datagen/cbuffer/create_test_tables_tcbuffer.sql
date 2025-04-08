/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation FOR any purpose, without fee, and without a written
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
 * @brief Function generating test tables for temporal circular buffers
 * @details These functions use the random generator for these types that are 
 * in the file `random_tpose.sql`. Refer to that file for the meaning of the
 * parameters used in the function calls of this file.
 */

DROP FUNCTION IF EXISTS create_test_tables_cbuffer();
CREATE OR REPLACE FUNCTION create_test_tables_cbuffer(size int DEFAULT 1000)
RETURNS text AS $$
BEGIN

------------------------------------------------------------------------------
-- Static circular buffer
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_cbuffer;
CREATE TABLE tbl_cbuffer AS
SELECT k, random_cbuffer(-100, 100, -100, 100, 0, 10, 5676) AS cb
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_cbufferset;
CREATE TABLE tbl_cbufferset AS
/* Add perc NULL values */
SELECT k, NULL AS s
FROM generate_series(1, perc) AS k UNION
SELECT k, random_cbuffer_set(-100, 100, -100, 100, 0, 10, 1, 10, 5676)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal circular buffer
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tcbuffer_inst;
CREATE TABLE tbl_tcbuffer_inst AS
SELECT k, random_tcbuffer_inst(-100, 100, -100, 100, 0, 10, '2001-01-01', '2001-12-31', 5676) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tcbuffer_discseq;
CREATE TABLE tbl_tcbuffer_discseq AS
SELECT k, random_tcbuffer_discseq(-100, 100, -100, 100, 0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tcbuffer_seq;
CREATE TABLE tbl_tcbuffer_seq AS
SELECT k, random_tcbuffer_contseq(-100, 100, -100, 100, 0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tcbuffer_seqset;
CREATE TABLE tbl_tcbuffer_seqset AS
SELECT k, random_tcbuffer_seqset(-100, 100, -100, 100, 0, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tcbuffer;
CREATE TABLE tbl_tcbuffer(k, temp) AS
(SELECT k, inst FROM tbl_tcbuffer_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, seq FROM tbl_tcbuffer_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tcbuffer_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tcbuffer_seqset LIMIT size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_cbuffer(100);
/*
SELECT * FROM tbl_cbuffer LIMIT 3;
SELECT * FROM tbl_nsegment LIMIT 3;
SELECT * FROM tbl_tcbuffer_inst LIMIT 3;
SELECT * FROM tbl_tcbuffer_discseq LIMIT 3;
SELECT * FROM tbl_tcbuffer_seq LIMIT 3;
SELECT * FROM tbl_tcbuffer_seqset LIMIT 3;
SELECT * FROM tbl_tcbuffer LIMIT 3;
SELECT * FROM tbl_tcbuffer LIMIT 3 OFFSET 25;
SELECT * FROM tbl_tcbuffer LIMIT 3 OFFSET 50;
SELECT * FROM tbl_tcbuffer LIMIT 3 OFFSET 75;
*/
