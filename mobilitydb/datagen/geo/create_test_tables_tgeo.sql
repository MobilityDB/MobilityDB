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

/*
 * Function generating a set of test tables for temporal geo types.
 *
 * These functions use the random generator for these types that are in the
 * file random_geo.sql. Refer to that file for the meaning of the
 * parameters used in the function calls of this file.
 */

DROP FUNCTION IF EXISTS create_test_tables_geo();
CREATE OR REPLACE FUNCTION create_test_tables_geo(size int DEFAULT 1000)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.02;
IF perc < 1 THEN perc := 1; END IF;

------------------------------------------------------------------------------
-- Static geo
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geometry;
CREATE TABLE tbl_geometry AS
SELECT k, random_geometry(-100, 100, -100, 100, 3812) AS geo
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geometry3D;
CREATE TABLE tbl_geometry3D AS
SELECT k, random_geometry3D(-100, 100, -100, 100, -100, 100, 3812) AS geo
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geometryset;
CREATE TABLE tbl_geometryset AS
SELECT k, random_geometry_set(-100, 100, -100, 100, 1, 10, 3812)
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_geometry3Dset;
CREATE TABLE tbl_geometry3Dset AS
SELECT k, random_geometry3D_set(-100, 100, -100, 100, -100, 100, 1, 10, 3812)
FROM generate_series(1, size) AS k;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geography;
CREATE TABLE tbl_geography AS
SELECT k, random_geography(-180, 180, -90, 90, 7844) AS geo
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geography3D;
CREATE TABLE tbl_geography3D AS
SELECT k, random_geography3D(-180, 180, -90, 90, 0, 100, 7844) AS geo
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geographyset;
CREATE TABLE tbl_geographyset AS
SELECT k, random_geography_set(-180, 180, -90, 90, 1, 10, 7844)
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_geography3Dset;
CREATE TABLE tbl_geography3Dset AS
SELECT k, random_geography3D_set(-180, 180, -90, 90, 0, 100, 1, 10, 7844)
FROM generate_series(1, size) AS k;

------------------------------------------------------------------------------
-- Temporal geo
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeometry_inst;
CREATE TABLE tbl_tgeometry_inst AS
SELECT k, random_tgeometry_inst(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 3812) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry3D_inst;
CREATE TABLE tbl_tgeometry3D_inst AS
SELECT k, random_tgeometry3D_inst(-100, 100, -100, 100, -100, 100, '2001-01-01', '2001-12-31', 3812) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry_discseq;
CREATE TABLE tbl_tgeometry_discseq AS
SELECT k, random_tgeometry_discseq(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 3812) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry3D_discseq;
CREATE TABLE tbl_tgeometry3D_discseq AS
SELECT k, random_tgeometry3D_discseq(-100, 100, -100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 3812) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry_seq;
CREATE TABLE tbl_tgeometry_seq AS
SELECT k, random_tgeometry_stepseq(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 3812) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry3D_seq;
CREATE TABLE tbl_tgeometry3D_seq AS
SELECT k, random_tgeometry3D_stepseq(-100, 100, -100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 3812) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeometry_seqset;
CREATE TABLE tbl_tgeometry_seqset AS
SELECT k, random_tgeometry_seqset(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tgeometry3D_seqset;
CREATE TABLE tbl_tgeometry3D_seqset AS
SELECT k, random_tgeometry3D_seqset(-100, 100, -100, 100, -100, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tgeometry;
CREATE TABLE tbl_tgeometry(k, temp) AS
(SELECT k, inst FROM tbl_tgeometry_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeometry_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeometry_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tgeometry_seqset LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeometry3D;
CREATE TABLE tbl_tgeometry3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeometry3D_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeometry3D_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeometry3D_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tgeometry3D_seqset LIMIT size / 4);

------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeography_inst;
CREATE TABLE tbl_tgeography_inst AS
SELECT k, random_tgeography_inst(-180, 180, -90, 90, '2001-01-01', '2001-12-31', 7844) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography3D_inst;
CREATE TABLE tbl_tgeography3D_inst AS
SELECT k, random_tgeography3D_inst(-180, 180, -90, 90, 0, 100, '2001-01-01', '2001-12-31', 7844) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography_discseq;
CREATE TABLE tbl_tgeography_discseq AS
SELECT k, random_tgeography_discseq(-180, 180, -90, 90, '2001-01-01', '2001-12-31', 10, 1, 10, 7844) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography3D_discseq;
CREATE TABLE tbl_tgeography3D_discseq AS
SELECT k, random_tgeography3D_discseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 7844) AS ti
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography_seq;
CREATE TABLE tbl_tgeography_seq AS
SELECT k, random_tgeography_stepseq(-180, 180, -90, 90, '2001-01-01', '2001-12-31', 10, 1, 10, 7844) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography3D_seq;
CREATE TABLE tbl_tgeography3D_seq AS
SELECT k, random_tgeography3D_stepseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 7844) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tgeography_seqset;
CREATE TABLE tbl_tgeography_seqset AS
SELECT k, random_tgeography_seqset(-180, 180, -90, 90, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 7844) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tgeography3D_seqset;
CREATE TABLE tbl_tgeography3D_seqset AS
SELECT k, random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 7844) AS ss
FROM generate_series(1, size) AS k;

DROP TABLE IF EXISTS tbl_tgeography;
CREATE TABLE tbl_tgeography(k, temp) AS
(SELECT k, inst FROM tbl_tgeography_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeography_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeography_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tgeography_seqset LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeography3D;
CREATE TABLE tbl_tgeography3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeography3D_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeography3D_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeography3D_seq LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss FROM tbl_tgeography3D_seqset LIMIT size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_geo(100);
/*
SELECT * FROM tbl_geometry LIMIT 3;
SELECT * FROM tbl_geometryset LIMIT 3;
SELECT * FROM tbl_tgeometry_inst LIMIT 3;
SELECT * FROM tbl_tgeometry_discseq LIMIT 3;
SELECT * FROM tbl_tgeometry_seq LIMIT 3;
SELECT * FROM tbl_tgeometry_seqset LIMIT 3;
SELECT * FROM tbl_tgeometry LIMIT 3;
SELECT * FROM tbl_tgeometry LIMIT 3 OFFSET 25;
SELECT * FROM tbl_tgeometry LIMIT 3 OFFSET 50;
SELECT * FROM tbl_tgeometry LIMIT 3 OFFSET 75;
*/
