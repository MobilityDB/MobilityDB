/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

------------------------------------------------------------------------------
-- Temporal Network Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tnpointinst;
CREATE TABLE tbl_tnpointinst AS
SELECT k, random_tnpointinst(0, size, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpointi;
CREATE TABLE tbl_tnpointi AS
SELECT k, random_tnpointi(0, size, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpointseq;
CREATE TABLE tbl_tnpointseq AS
SELECT k, random_tnpointseq(0, size, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tnpoints;
CREATE TABLE tbl_tnpoints AS
SELECT k, random_tnpoints(0, size, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tnpoint;
CREATE TABLE tbl_tnpoint(k, temp) AS
(SELECT k, inst FROM tbl_tnpointinst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tnpointi LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tnpointseq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tnpoints LIMIT size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_npoint(100);
/*
SELECT * FROM tbl_npoint LIMIT 3;
SELECT * FROM tbl_nsegment LIMIT 3;
SELECT * FROM tbl_tnpointinst LIMIT 3;
SELECT * FROM tbl_tnpointi LIMIT 3;
SELECT * FROM tbl_tnpointseq LIMIT 3;
SELECT * FROM tbl_tnpoints LIMIT 3;
SELECT * FROM tbl_tnpoint LIMIT 3;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 25;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 50;
SELECT * FROM tbl_tnpoint LIMIT 3 OFFSET 75;
*/
