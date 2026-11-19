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
 * @brief Function generating a set of test tables for geometry/geography types
 * and temporal point types
 * @details These functions use the random generator for these types that are
 * in the
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

DROP TABLE IF EXISTS tbl_stbox3d;
CREATE TABLE tbl_stbox3d AS
SELECT k, random_stbox3d(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geodstbox;
CREATE TABLE tbl_geodstbox AS
SELECT k, random_geodstbox(10, 32, 35, 72, 0, 3000, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geodstbox3d;
CREATE TABLE tbl_geodstbox3d AS
SELECT k, random_geodstbox3d(10, 32, 35, 72, 0, 3000, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geom_point;
CREATE TABLE tbl_geom_point AS
SELECT 1 AS k, geometry 'point empty' AS g UNION
SELECT k, random_geom_point(0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_point3d;
CREATE TABLE tbl_geom_point3d AS
SELECT 1 AS k, geometry 'pointZ empty' AS g UNION
SELECT k, random_geom_point3d(0, 100, 0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_point;
CREATE TABLE tbl_geog_point AS
SELECT 1 AS k, geography 'point empty' AS g UNION
SELECT k, random_geog_point(-10, 32, 35, 72)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_point3d;
CREATE TABLE tbl_geog_point3d AS
SELECT 1 AS k, geography 'pointZ empty' AS g UNION
SELECT k, random_geog_point3d(-10, 32, 35, 72, 0, 1000)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_linestring;
CREATE TABLE tbl_geom_linestring AS
SELECT 1 AS k, geometry 'linestring empty' AS g UNION
SELECT k, random_geom_linestring(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_linestring3d;
CREATE TABLE tbl_geom_linestring3d AS
SELECT 1 AS k, geometry 'linestring Z empty' AS g UNION
SELECT k, random_geom_linestring3d(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_linestring;
CREATE TABLE tbl_geog_linestring AS
SELECT 1 AS k, geography 'linestring empty' AS g UNION
SELECT k, random_geog_linestring(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_linestring3d;
CREATE TABLE tbl_geog_linestring3d AS
SELECT 1 AS k, geography 'linestring Z empty' AS g UNION
SELECT k, random_geog_linestring3d(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_polygon;
CREATE TABLE tbl_geom_polygon AS
SELECT 1 AS k, geometry 'polygon empty' AS g UNION
SELECT k, random_geom_polygon(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_polygon3d;
CREATE TABLE tbl_geom_polygon3d AS
SELECT 1 AS k, geometry 'polygon Z empty' AS g UNION
SELECT k, random_geom_polygon3d(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_polygon;
CREATE TABLE tbl_geog_polygon AS
SELECT 1 AS k, geography 'polygon empty' AS g UNION
SELECT k, random_geog_polygon(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_polygon3d;
CREATE TABLE tbl_geog_polygon3d AS
SELECT 1 AS k, geography 'polygon Z empty' AS g UNION
SELECT k, random_geog_polygon3d(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipoint;
CREATE TABLE tbl_geom_multipoint AS
SELECT 1 AS k, geometry 'multipoint empty' AS g UNION
SELECT k, random_geom_multipoint(0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipoint3d;
CREATE TABLE tbl_geom_multipoint3d AS
SELECT 1 AS k, geometry 'multipoint Z empty' AS g UNION
SELECT k, random_geom_multipoint3d(0, 100, 0, 100, 0, 100, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipoint;
CREATE TABLE tbl_geog_multipoint AS
SELECT 1 AS k, geography 'multipoint empty' AS g UNION
SELECT k, random_geog_multipoint(-10, 32, 35, 72, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipoint3d;
CREATE TABLE tbl_geog_multipoint3d AS
SELECT 1 AS k, geography 'multipoint Z empty' AS g UNION
SELECT k, random_geog_multipoint3d(-10, 32, 35, 72, 0, 1000, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multilinestring;
CREATE TABLE tbl_geom_multilinestring AS
SELECT 1 AS k, geometry 'multilinestring empty' AS g UNION
SELECT k, random_geom_multilinestring(0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multilinestring3d;
CREATE TABLE tbl_geom_multilinestring3d AS
SELECT 1 AS k, geometry 'multilinestring Z empty' AS g UNION
SELECT k, random_geom_multilinestring3d(0, 100, 0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multilinestring;
CREATE TABLE tbl_geog_multilinestring AS
SELECT 1 AS k, geography 'multilinestring empty' AS g UNION
SELECT k, random_geog_multilinestring(-10, 32, 35, 72, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multilinestring3d;
CREATE TABLE tbl_geog_multilinestring3d AS
SELECT 1 AS k, geography 'multilinestring Z empty' AS g UNION
SELECT k, random_geog_multilinestring3d(-10, 32, 35, 72, 0, 1000, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipolygon;
CREATE TABLE tbl_geom_multipolygon AS
SELECT 1 AS k, geometry 'multipolygon empty' AS g UNION
SELECT k, random_geom_multipolygon(0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_multipolygon3d;
CREATE TABLE tbl_geom_multipolygon3d AS
SELECT 1 AS k, geometry 'multipolygon Z empty' AS g UNION
SELECT k, random_geom_multipolygon3d(0, 100, 0, 100, 0, 100, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipolygon;
CREATE TABLE tbl_geog_multipolygon AS
SELECT 1 AS k, geography 'multipolygon empty' AS g UNION
SELECT k, random_geog_multipolygon(-10, 32, 35, 72, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geog_multipolygon3d;
CREATE TABLE tbl_geog_multipolygon3d AS
SELECT 1 AS k, geography 'multipolygon Z empty' AS g UNION
SELECT k, random_geog_multipolygon3d(-10, 32, 35, 72, 0, 1000, 10, 5, 10, 5, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_circularstring;
CREATE TABLE tbl_geom_circularstring AS
SELECT 1 AS k, geometry 'circularstring empty' AS g UNION
SELECT k, random_geom_circularstring(0, 100, 0, 100, 10, 1, 5)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_compoundcurve;
CREATE TABLE tbl_geom_compoundcurve AS
SELECT 1 AS k, geometry 'compoundcurve empty' AS g UNION
SELECT k, random_geom_compoundcurve(0, 100, 0, 100, 10, 1, 5)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom_curvepolygon;
CREATE TABLE tbl_geom_curvepolygon AS
SELECT 1 AS k, geometry 'curvepolygon empty' AS g UNION
SELECT k, random_geom_curvepolygon(0, 100, 0, 100, 10, 2, 5)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geom;
CREATE TABLE tbl_geom (
  k serial PRIMARY KEY,
  g geometry);
INSERT INTO tbl_geom(g)
(SELECT g FROM tbl_geom_point ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_linestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_polygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipolygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_circularstring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_compoundcurve ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_curvepolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geom3d;
CREATE TABLE tbl_geom3d (
  k serial PRIMARY KEY,
  g geometry);
INSERT INTO tbl_geom3d(g)
(SELECT g FROM tbl_geom_point3d ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_linestring3d ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geom_polygon3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipoint3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multilinestring3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geom_multipolygon3d ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geog;
CREATE TABLE tbl_geog (
  k serial PRIMARY KEY,
  g geography);
INSERT INTO tbl_geog(g)
(SELECT g FROM tbl_geog_point ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_linestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_polygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geog3d;
CREATE TABLE tbl_geog3d (
  k serial PRIMARY KEY,
  g geography);
INSERT INTO tbl_geog3d(g)
(SELECT g FROM tbl_geog_point3d ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_linestring3d ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geog_polygon3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipoint3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multilinestring3d ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geog_multipolygon3d ORDER BY k LIMIT (size * 0.2));

------------------------------------------------------------------------------
-- Geo Set Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_geomset;
CREATE TABLE tbl_geomset AS
/* Add perc NULL values */
SELECT k, NULL AS g
FROM generate_series(1, perc) AS k UNION
SELECT k, random_geom_point_set(-100, 100, -100, 100, 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_geogset;
CREATE TABLE tbl_geogset AS
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
SELECT k, random_tgeompoint_inst(0, 100, 0, 100, '2001-01-01', '2001-12-31') AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_inst t1
SET temp = (SELECT temp FROM tbl_tgeompoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_inst t1
SET temp = (SELECT tgeompoint(random_geom_point(0, 100, 0, 100), getTimestamp(temp))
  FROM tbl_tgeompoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_inst;
CREATE TABLE tbl_tgeompoint3d_inst AS
SELECT k, random_tgeompoint3d_inst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_inst t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_inst t1
SET temp = (SELECT tgeompoint(random_geom_point3d(0, 100, 0, 100, 0, 100), getTimestamp(temp))
  FROM tbl_tgeompoint3d_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_inst;
CREATE TABLE tbl_tgeogpoint_inst AS
SELECT k, random_tgeogpoint_inst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_inst t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_inst t1
SET temp = (SELECT tgeogpoint(random_geog_point(-10, 32, 35, 72), getTimestamp(temp))
  FROM tbl_tgeogpoint_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_inst;
CREATE TABLE tbl_tgeogpoint3d_inst AS
SELECT k, random_tgeogpoint3d_inst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_inst t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_inst t1
SET temp = (SELECT tgeogpoint(random_geog_point3d(-10, 32, 35, 72, 0, 1000), getTimestamp(temp))
  FROM tbl_tgeogpoint3d_inst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_discseq;
CREATE TABLE tbl_tgeompoint_discseq AS
SELECT k, random_tgeompoint_discseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_discseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_discseq t1
SET temp = (SELECT round(temp,6) FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_discseq t1
SET temp = (SELECT shiftTime(temp, endTimestamp(temp)-startTimestamp(temp))
  FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_discseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',(endTimestamp(temp)-startTimestamp(temp))/2))
  FROM tbl_tgeompoint_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_discseq;
CREATE TABLE tbl_tgeompoint3d_discseq AS
SELECT k, random_tgeompoint3d_discseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_discseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_discseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3d_discseq t1
SET temp = (SELECT shiftTime(temp, endTimestamp(temp)-startTimestamp(temp))
  FROM tbl_tgeompoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3d_discseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',(endTimestamp(temp)-startTimestamp(temp))/2))
  FROM tbl_tgeompoint3d_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_discseq;
CREATE TABLE tbl_tgeogpoint_discseq AS
SELECT k, random_tgeogpoint_discseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_discseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_discseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_discseq t1
SET temp = (SELECT shiftTime(temp, endTimestamp(temp)-startTimestamp(temp))
  FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_discseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',(endTimestamp(temp)-startTimestamp(temp))/2))
  FROM tbl_tgeogpoint_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_discseq;
CREATE TABLE tbl_tgeogpoint3d_discseq AS
SELECT k, random_tgeogpoint3d_discseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_discseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_discseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3d_discseq t1
SET temp = (SELECT shiftTime(temp, endTimestamp(temp)-startTimestamp(temp))
  FROM tbl_tgeogpoint3d_discseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3d_discseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',(endTimestamp(temp)-startTimestamp(temp))/2))
  FROM tbl_tgeogpoint3d_discseq t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_contseq;
CREATE TABLE tbl_tgeompoint_contseq AS
SELECT k, random_tgeompoint_contseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_contseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',duration(temp, true)/2))
  FROM tbl_tgeompoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_contseq;
CREATE TABLE tbl_tgeompoint3d_contseq AS
SELECT k, random_tgeompoint3d_contseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_contseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3d_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3d_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',duration(temp, true)/2))
  FROM tbl_tgeompoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_contseq;
CREATE TABLE tbl_tgeogpoint_contseq AS
SELECT k, random_tgeogpoint_contseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_contseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute',duration(temp, true)/2))
  FROM tbl_tgeogpoint_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_contseq;
CREATE TABLE tbl_tgeogpoint3d_contseq AS
SELECT k, random_tgeogpoint3d_contseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_contseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3d_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3d_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint3d_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_seqset;
CREATE TABLE tbl_tgeompoint_seqset AS
SELECT k, random_tgeompoint_seqset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_seqset t1
SET temp = (SELECT temp FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_seqset;
CREATE TABLE tbl_tgeompoint3d_seqset AS
SELECT k, random_tgeompoint3d_seqset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_seqset t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3d_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3d_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_seqset;
CREATE TABLE tbl_tgeogpoint_seqset AS
SELECT k, random_tgeogpoint_seqset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_seqset t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_seqset;
CREATE TABLE tbl_tgeogpoint3d_seqset AS
SELECT k, random_tgeogpoint3d_seqset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_seqset t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3d_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3d_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint3d_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint;
CREATE TABLE tbl_tgeompoint(k, temp) AS
(SELECT k, temp FROM tbl_tgeompoint_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, temp FROM tbl_tgeompoint_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, temp FROM tbl_tgeompoint_contseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, temp FROM tbl_tgeompoint_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeompoint3d;
CREATE TABLE tbl_tgeompoint3d(k, temp) AS
(SELECT k, temp FROM tbl_tgeompoint3d_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, temp FROM tbl_tgeompoint3d_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, temp FROM tbl_tgeompoint3d_contseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, temp FROM tbl_tgeompoint3d_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint;
CREATE TABLE tbl_tgeogpoint(k, temp) AS
(SELECT k, temp FROM tbl_tgeogpoint_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, temp FROM tbl_tgeogpoint_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, temp FROM tbl_tgeogpoint_contseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, temp FROM tbl_tgeogpoint_seqset ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint3d;
CREATE TABLE tbl_tgeogpoint3d(k, temp) AS
(SELECT k, temp FROM tbl_tgeogpoint3d_inst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, temp FROM tbl_tgeogpoint3d_discseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, temp FROM tbl_tgeogpoint3d_contseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, temp FROM tbl_tgeogpoint3d_seqset ORDER BY k LIMIT size / 4);

------------------------------------------------------------------------------
-- Temporal Point Types with Step Interpolation
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_step_contseq;
CREATE TABLE tbl_tgeompoint_step_contseq AS
SELECT k, random_tgeompoint_contseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_step_contseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_step_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_step_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_step_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_step_contseq;
CREATE TABLE tbl_tgeompoint3d_step_contseq AS
SELECT k, random_tgeompoint3d_contseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_step_contseq t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_step_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3d_step_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3d_step_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_step_contseq;
CREATE TABLE tbl_tgeogpoint_step_contseq AS
SELECT k, random_tgeogpoint_contseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_step_contseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_step_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_step_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_step_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_step_contseq;
CREATE TABLE tbl_tgeogpoint3d_step_contseq AS
SELECT k, random_tgeogpoint3d_contseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_step_contseq t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_step_contseq t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3d_step_contseq t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3d_step_contseq t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint3d_step_contseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_step_seqset;
CREATE TABLE tbl_tgeompoint_step_seqset AS
SELECT k, random_tgeompoint_seqset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint_step_seqset t1
SET temp = (SELECT temp FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint_step_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint_step_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint_step_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3d_step_seqset;
CREATE TABLE tbl_tgeompoint3d_step_seqset AS
SELECT k, random_tgeompoint3d_seqset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3d_step_seqset t1
SET temp = (SELECT temp FROM tbl_tgeompoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3d_step_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeompoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3d_step_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeompoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3d_step_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeompoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint_step_seqset;
CREATE TABLE tbl_tgeogpoint_step_seqset AS
SELECT k, random_tgeogpoint_seqset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint_step_seqset t1
SET temp = (SELECT temp FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint_step_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint_step_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint_step_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3d_step_seqset;
CREATE TABLE tbl_tgeogpoint3d_step_seqset AS
SELECT k, random_tgeogpoint3d_seqset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10, linear:=false) AS temp
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3d_step_seqset t1
SET temp = (SELECT temp FROM tbl_tgeogpoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3d_step_seqset t1
SET temp = (SELECT round(temp,3) FROM tbl_tgeogpoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3d_step_seqset t1
SET temp = (SELECT shiftTime(temp, duration(temp, true)) FROM tbl_tgeogpoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3d_step_seqset t1
SET temp = (SELECT shiftTime(temp, date_trunc('minute', duration(temp, true)/2))
  FROM tbl_tgeogpoint3d_step_seqset t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

------------------------------------------------------------------------------
-- The tables of the index tests: big enough that the planner chooses a
-- GiST/SP-GiST index scan over a sequential scan, otherwise the index tests
-- exercise nothing. The threshold is around 10 000 rows; size * 100 + 100
-- keeps a small margin above it (10 100 at the default size of 100). The four
-- quarters of a temporal table are the four subtypes, as in its table above,
-- each made by the call that makes the table of that subtype.
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint3d_big;
CREATE TABLE tbl_tgeompoint3d_big(k, temp) AS
SELECT k, CASE (k - 1) * 4 / (size * 100 + 100)
  WHEN 0 THEN random_tgeompoint3d_inst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31')
  WHEN 1 THEN random_tgeompoint3d_discseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10)
  WHEN 2 THEN random_tgeompoint3d_contseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10)
  ELSE random_tgeompoint3d_seqset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10)
  END AS temp
FROM generate_series(1, size * 100 + 100) k;

DROP TABLE IF EXISTS tbl_tgeogpoint3d_big;
CREATE TABLE tbl_tgeogpoint3d_big(k, temp) AS
SELECT k, CASE (k - 1) * 4 / (size * 100 + 100)
  WHEN 0 THEN random_tgeogpoint3d_inst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31')
  WHEN 1 THEN random_tgeogpoint3d_discseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10)
  WHEN 2 THEN random_tgeogpoint3d_contseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10)
  ELSE random_tgeogpoint3d_seqset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10)
  END AS temp
FROM generate_series(1, size * 100 + 100) k;

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_tpoint(100);
