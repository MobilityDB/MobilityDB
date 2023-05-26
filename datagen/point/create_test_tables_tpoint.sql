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
 * create_test_tables_tpoint.sql
 * Function generating a set of test tables for geometry/geography types
 * and temporal point types.
 *
 * These functions use the random generator for these types that are in the
 * file random_tpoint.sql. Refer to that file for the meaning of the
 * parameters used in the function calls of this file.
 */

DROP FUNCTION IF EXISTS create_test_tables_tpoint();
CREATE OR REPLACE FUNCTION create_test_tables_tpoint(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.02;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Geo types
-- In the following tables, geography points are restricted to the bounding
-- box covering approximately continental Europe, that is, "BOX(-10 32,35 72)"
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_stbox;
CREATE TABLE tbl_stbox AS
SELECT k, random_stbox(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_stbox3D;
CREATE TABLE tbl_stbox3D AS
SELECT k, random_stbox3D(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geodstbox;
CREATE TABLE tbl_geodstbox AS
SELECT k, random_geodstbox(10, 32, 35, 72, 0, 3000, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geodstbox3D;
CREATE TABLE tbl_geodstbox3D AS
SELECT k, random_geodstbox3D(10, 32, 35, 72, 0, 3000, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geom_point;
CREATE TABLE tbl_geom_point AS
SELECT 1 AS k, geometry 'point empty' AS g UNION
SELECT k, random_geom_point(0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_point3D;
CREATE TABLE tbl_geom_point3D AS
SELECT 1 AS k, geometry 'pointZ empty' AS g UNION
SELECT k, random_geom_point3D(0, 100, 0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_point;
CREATE TABLE tbl_geog_point AS
SELECT 1 AS k, geography 'point empty' AS g UNION
SELECT k, random_geog_point(-10, 32, 35, 72)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_point3D;
CREATE TABLE tbl_geog_point3D AS
SELECT 1 AS k, geography 'pointZ empty' AS g UNION
SELECT k, random_geog_point3D(-10, 32, 35, 72, 0, 1000)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_linestring;
CREATE TABLE tbl_geom_linestring AS
SELECT 1 AS k, geometry 'linestring empty' AS g UNION
SELECT k, random_geom_linestring(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_linestring3D;
CREATE TABLE tbl_geom_linestring3D AS
SELECT 1 AS k, geometry 'linestring Z empty' AS g UNION
SELECT k, random_geom_linestring3D(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_linestring;
CREATE TABLE tbl_geog_linestring AS
SELECT 1 AS k, geography 'linestring empty' AS g UNION
SELECT k, random_geog_linestring(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_linestring3D;
CREATE TABLE tbl_geog_linestring3D AS
SELECT 1 AS k, geography 'linestring Z empty' AS g UNION
SELECT k, random_geog_linestring3D(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_polygon;
CREATE TABLE tbl_geom_polygon AS
SELECT 1 AS k, geometry 'polygon empty' AS g UNION
SELECT k, random_geom_polygon(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_polygon3D;
CREATE TABLE tbl_geom_polygon3D AS
SELECT 1 AS k, geometry 'polygon Z empty' AS g UNION
SELECT k, random_geom_polygon3D(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_polygon;
CREATE TABLE tbl_geog_polygon AS
SELECT 1 AS k, geography 'polygon empty' AS g UNION
SELECT k, random_geog_polygon(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_polygon3D;
CREATE TABLE tbl_geog_polygon3D AS
SELECT 1 AS k, geography 'polygon Z empty' AS g UNION
SELECT k, random_geog_polygon3D(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipoint;
CREATE TABLE tbl_geom_multipoint AS
SELECT 1 AS k, geometry 'multipoint empty' AS g UNION
SELECT k, random_geom_multipoint(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipoint3D;
CREATE TABLE tbl_geom_multipoint3D AS
SELECT 1 AS k, geometry 'multipoint Z empty' AS g UNION
SELECT k, random_geom_multipoint3D(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipoint;
CREATE TABLE tbl_geog_multipoint AS
SELECT 1 AS k, geography 'multipoint empty' AS g UNION
SELECT k, random_geog_multipoint(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipoint3D;
CREATE TABLE tbl_geog_multipoint3D AS
SELECT 1 AS k, geography 'multipoint Z empty' AS g UNION
SELECT k, random_geog_multipoint3D(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multilinestring;
CREATE TABLE tbl_geom_multilinestring AS
SELECT 1 AS k, geometry 'multilinestring empty' AS g UNION
SELECT k, random_geom_multilinestring(0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multilinestring3D;
CREATE TABLE tbl_geom_multilinestring3D AS
SELECT 1 AS k, geometry 'multilinestring Z empty' AS g UNION
SELECT k, random_geom_multilinestring3D(0, 100, 0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multilinestring;
CREATE TABLE tbl_geog_multilinestring AS
SELECT 1 AS k, geography 'multilinestring empty' AS g UNION
SELECT k, random_geog_multilinestring(-10, 32, 35, 72, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multilinestring3D;
CREATE TABLE tbl_geog_multilinestring3D AS
SELECT 1 AS k, geography 'multilinestring Z empty' AS g UNION
SELECT k, random_geog_multilinestring3D(-10, 32, 35, 72, 0, 1000, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipolygon;
CREATE TABLE tbl_geom_multipolygon AS
SELECT 1 AS k, geometry 'multipolygon empty' AS g UNION
SELECT k, random_geom_multipolygon(0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipolygon3D;
CREATE TABLE tbl_geom_multipolygon3D AS
SELECT 1 AS k, geometry 'multipolygon Z empty' AS g UNION
SELECT k, random_geom_multipolygon3D(0, 100, 0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipolygon;
CREATE TABLE tbl_geog_multipolygon AS
SELECT 1 AS k, geography 'multipolygon empty' AS g UNION
SELECT k, random_geog_multipolygon(-10, 32, 35, 72, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipolygon3D;
CREATE TABLE tbl_geog_multipolygon3D AS
SELECT 1 AS k, geography 'multipolygon Z empty' AS g UNION
SELECT k, random_geog_multipolygon3D(-10, 32, 35, 72, 0, 1000, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geometry;
CREATE TABLE tbl_geometry (
  k serial PRIMARY KEY,
  g geometry);
INSERT INTO tbl_geometry(g)
(SELECT g FROM tbl_geom_point ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_linestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_polygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geometry3D;
CREATE TABLE tbl_geometry3D (
  k serial PRIMARY KEY,
  g geometry);
INSERT INTO tbl_geometry3D(g)
(SELECT g FROM tbl_geom_point3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_linestring3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_polygon3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipoint3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multilinestring3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipolygon3D ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geography;
CREATE TABLE tbl_geography (
  k serial PRIMARY KEY,
  g geography);
INSERT INTO tbl_geography(g)
(SELECT g FROM tbl_geog_point ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_linestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_polygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geography3D;
CREATE TABLE tbl_geography3D (
  k serial PRIMARY KEY,
  g geography);
INSERT INTO tbl_geography3D(g)
(SELECT g FROM tbl_geog_point3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_linestring3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_polygon3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipoint3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multilinestring3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipolygon3D ORDER BY k LIMIT (size * 0.2));

------------------------------------------------------------------------------
-- Set Point Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geompointset;
CREATE TABLE tbl_geompointset AS
/* Add perc NULL values */
SELECT k, NULL AS g
FROM generate_series(1, perc) AS k UNION
SELECT k, random_geom_point_set(-100, 100, -100, 100, 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_geogpointset;
CREATE TABLE tbl_geogpointset AS
/* Add perc NULL values */
SELECT k, NULL AS g
FROM generate_series(1, perc) AS k UNION
SELECT k, random_geog_point_set(0, 80, 0, 80, 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal Point Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_inst;
CREATE TABLE tbl_tgeompoint_inst AS
SELECT k, random_tgeompoint_inst(0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_inst t1
SET inst = (SELECT inst FROM tbl_tgeompoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_inst t1
SET inst = (SELECT tgeompoint_inst(random_geom_point(0, 100, 0, 100), getTimestamp(inst))
  FROM tbl_tgeompoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_inst;
CREATE TABLE tbl_tgeompoint3D_inst AS
SELECT k, random_tgeompoint3D_inst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_inst t1
SET inst = (SELECT inst FROM tbl_tgeompoint3D_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_inst t1
SET inst = (SELECT tgeompoint_inst(random_geom_point3D(0, 100, 0, 100, 0, 100), getTimestamp(inst))
  FROM tbl_tgeompoint3D_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_inst;
CREATE TABLE tbl_tgeogpoint_inst AS
SELECT k, random_tgeogpoint_inst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_inst t1
SET inst = (SELECT inst FROM tbl_tgeogpoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_inst t1
SET inst = (SELECT tgeogpoint_inst(random_geog_point(-10, 32, 35, 72), getTimestamp(inst))
  FROM tbl_tgeogpoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_inst;
CREATE TABLE tbl_tgeogpoint3D_inst AS
SELECT k, random_tgeogpoint3D_inst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_inst t1
SET inst = (SELECT inst FROM tbl_tgeogpoint3D_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_inst t1
SET inst = (SELECT tgeogpoint_inst(random_geog_point3D(-10, 32, 35, 72, 0, 1000), getTimestamp(inst))
  FROM tbl_tgeogpoint3D_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_discseq;
CREATE TABLE tbl_tgeompoint_discseq AS
SELECT k, random_tgeompoint_discseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_discseq t1
SET ti = (SELECT ti FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_discseq t1
SET ti = (SELECT round(ti,6) FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_discseq t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_discseq t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_discseq;
CREATE TABLE tbl_tgeompoint3D_discseq AS
SELECT k, random_tgeompoint3D_discseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_discseq t1
SET ti = (SELECT ti FROM tbl_tgeompoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_discseq t1
SET ti = (SELECT round(ti,3) FROM tbl_tgeompoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3D_discseq t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeompoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3D_discseq t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeompoint3D_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_discseq;
CREATE TABLE tbl_tgeogpoint_discseq AS
SELECT k, random_tgeogpoint_discseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_discseq t1
SET ti = (SELECT ti FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_discseq t1
SET ti = (SELECT round(ti,3) FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_discseq t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_discseq t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_discseq;
CREATE TABLE tbl_tgeogpoint3D_discseq AS
SELECT k, random_tgeogpoint3D_discseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_discseq t1
SET ti = (SELECT ti FROM tbl_tgeogpoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_discseq t1
SET ti = (SELECT round(ti,3) FROM tbl_tgeogpoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3D_discseq t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeogpoint3D_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3D_discseq t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeogpoint3D_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_seq;
CREATE TABLE tbl_tgeompoint_seq AS
SELECT k, random_tgeompoint_seq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_seq t1
SET seq = (SELECT seq FROM tbl_tgeompoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeompoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeompoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute',duration(seq, true)/2))
  FROM tbl_tgeompoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_seq;
CREATE TABLE tbl_tgeompoint3D_seq AS
SELECT k, random_tgeompoint3D_seq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_seq t1
SET seq = (SELECT seq FROM tbl_tgeompoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeompoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3D_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeompoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3D_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute',duration(seq, true)/2))
  FROM tbl_tgeompoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_seq;
CREATE TABLE tbl_tgeogpoint_seq AS
SELECT k, random_tgeogpoint_seq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_seq t1
SET seq = (SELECT seq FROM tbl_tgeogpoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeogpoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeogpoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute',duration(seq, true)/2))
  FROM tbl_tgeogpoint_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_seq;
CREATE TABLE tbl_tgeogpoint3D_seq AS
SELECT k, random_tgeogpoint3D_seq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_seq t1
SET seq = (SELECT seq FROM tbl_tgeogpoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeogpoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3D_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeogpoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3D_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute', duration(seq, true)/2))
  FROM tbl_tgeogpoint3D_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_seqset;
CREATE TABLE tbl_tgeompoint_seqset AS
SELECT k, random_tgeompoint_seqset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_seqset t1
SET ts = (SELECT ts FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_seqset;
CREATE TABLE tbl_tgeompoint3D_seqset AS
SELECT k, random_tgeompoint3D_seqset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_seqset t1
SET ts = (SELECT ts FROM tbl_tgeompoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeompoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3D_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeompoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3D_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeompoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_seqset;
CREATE TABLE tbl_tgeogpoint_seqset AS
SELECT k, random_tgeogpoint_seqset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_seqset t1
SET ts = (SELECT ts FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_seqset;
CREATE TABLE tbl_tgeogpoint3D_seqset AS
SELECT k, random_tgeogpoint3D_seqset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_seqset t1
SET ts = (SELECT ts FROM tbl_tgeogpoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeogpoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3D_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeogpoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3D_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeogpoint3D_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint;
CREATE TABLE tbl_tgeompoint(k, temp) AS
(SELECT k, inst FROM tbl_tgeompoint_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeompoint_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeompoint_seq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompoint_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeompoint3D;
CREATE TABLE tbl_tgeompoint3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeompoint3D_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeompoint3D_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeompoint3D_seq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompoint3D_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint;
CREATE TABLE tbl_tgeogpoint(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpoint_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeogpoint_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeogpoint_seq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpoint_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint3D;
CREATE TABLE tbl_tgeogpoint3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpoint3D_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeogpoint3D_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeogpoint3D_seq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpoint3D_seqset ORDER BY k LIMIT size / 4);

------------------------------------------------------------------------------
-- Temporal Point Types with Step Interpolation
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_step_seq;
CREATE TABLE tbl_tgeompoint_step_seq AS
SELECT k, random_tgeompoint_seq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_step_seq t1
SET seq = (SELECT seq FROM tbl_tgeompoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_step_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeompoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_step_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeompoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_step_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute', duration(seq, true)/2))
  FROM tbl_tgeompoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_step_seq;
CREATE TABLE tbl_tgeompoint3D_step_seq AS
SELECT k, random_tgeompoint3D_seq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_step_seq t1
SET seq = (SELECT seq FROM tbl_tgeompoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_step_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeompoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3D_step_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeompoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3D_step_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute', duration(seq, true)/2))
  FROM tbl_tgeompoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_step_seq;
CREATE TABLE tbl_tgeogpoint_step_seq AS
SELECT k, random_tgeogpoint_seq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_step_seq t1
SET seq = (SELECT seq FROM tbl_tgeogpoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_step_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeogpoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_step_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeogpoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_step_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute', duration(seq, true)/2))
  FROM tbl_tgeogpoint_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_step_seq;
CREATE TABLE tbl_tgeogpoint3D_step_seq AS
SELECT k, random_tgeogpoint3D_seq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_step_seq t1
SET seq = (SELECT seq FROM tbl_tgeogpoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_step_seq t1
SET seq = (SELECT round(seq,3) FROM tbl_tgeogpoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3D_step_seq t1
SET seq = (SELECT shift(seq, duration(seq, true)) FROM tbl_tgeogpoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3D_step_seq t1
SET seq = (SELECT shift(seq, date_trunc('minute', duration(seq, true)/2))
  FROM tbl_tgeogpoint3D_step_seq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_step_seqset;
CREATE TABLE tbl_tgeompoint_step_seqset AS
SELECT k, random_tgeompoint_seqset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_step_seqset t1
SET ts = (SELECT ts FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_step_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_step_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_step_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3D_step_seqset;
CREATE TABLE tbl_tgeompoint3D_step_seqset AS
SELECT k, random_tgeompoint3D_seqset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3D_step_seqset t1
SET ts = (SELECT ts FROM tbl_tgeompoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3D_step_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeompoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3D_step_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeompoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3D_step_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeompoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_step_seqset;
CREATE TABLE tbl_tgeogpoint_step_seqset AS
SELECT k, random_tgeogpoint_seqset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_step_seqset t1
SET ts = (SELECT ts FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_step_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_step_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_step_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_step_seqset;
CREATE TABLE tbl_tgeogpoint3D_step_seqset AS
SELECT k, random_tgeogpoint3D_seqset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3D_step_seqset t1
SET ts = (SELECT ts FROM tbl_tgeogpoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3D_step_seqset t1
SET ts = (SELECT round(ts,3) FROM tbl_tgeogpoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3D_step_seqset t1
SET ts = (SELECT shift(ts, duration(ts, true)) FROM tbl_tgeogpoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3D_step_seqset t1
SET ts = (SELECT shift(ts, date_trunc('minute', duration(ts, true)/2))
  FROM tbl_tgeogpoint3D_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_tpoint(100);
