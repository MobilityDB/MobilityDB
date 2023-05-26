/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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

/*
 * create_test_tables_tnpoint.sql
 * Function generating a set of test tables for temporal network point types.
 *
 * These functions use the random generator for these types that are in the
 * file random_tnpoint.sql. Refer to that file for the meaning of the
 * parameters used in the function calls of this file.
 */

DROP FUNCTION IF EXISTS create_test_tables_npoint();
CREATE OR REPLACE FUNCTION create_test_tables_npoint(size int DEFAULT 1000)
RETURNS text AS $$
BEGIN

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS ways;
CREATE TABLE ways (
  gid bigint PRIMARY KEY,
  the_geom geometry,
  length float
);
INSERT INTO ways(gid, the_geom)
SELECT k, random_geomlinestring(0, size, 0, size,10)
FROM generate_series (0, size) AS k;
UPDATE ways
SET length = st_length(the_geom);

/*
SELECT gid, st_astext(the_geom)
FROM ways
LIMIT 2;
*/

------------------------------------------------------------------------------
-- Static Network Types
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_npoint;
CREATE TABLE tbl_npoint AS
SELECT k, random_npoint(0, size) AS np
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_nsegment;
CREATE TABLE tbl_nsegment AS
SELECT k, random_nsegment(0, size) AS ns
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_npointset;
CREATE TABLE tbl_npointset AS
/* Add perc NULL values */
SELECT k, NULL AS n
FROM generate_series(1, perc) AS k UNION
SELECT k, random_npoint_set(1, 100, 5, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal Network Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tnpoint_inst;
CREATE TABLE tbl_tnpoint_inst AS
SELECT k, random_tnpoint_inst(0, size, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpoint_discseq;
CREATE TABLE tbl_tnpoint_discseq AS
SELECT k, random_tnpoint_discseq(0, size, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpoint_seq;
CREATE TABLE tbl_tnpoint_seq AS
SELECT k, random_tnpoint_seq(0, size, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpoint_seqset;
CREATE TABLE tbl_tnpoint_seqset AS
SELECT k, random_tnpoint_seqset(0, size, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tnpoint;
CREATE TABLE tbl_tnpoint(k, temp) AS
(SELECT k, inst FROM tbl_tnpoint_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, seq FROM tbl_tnpoint_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tnpoint_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tnpoint_seqset LIMIT size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_npoint(100);
/*
SELECT * FROM tbl_npoint LIMIT 3;
SELECT * FROM tbl_nsegment LIMIT 3;
SELECT * FROM tbl_tnpoint_inst LIMIT 3;
SELECT * FROM tbl_tnpoint_discseq LIMIT 3;
SELECT * FROM tbl_tnpoint_seq LIMIT 3;
SELECT * FROM tbl_tnpoint_seqset LIMIT 3;
SELECT * FROM tbl_tnpoint LIMIT 3;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 25;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 50;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 75;
*/
