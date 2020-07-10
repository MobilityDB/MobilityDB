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
SELECT k, random_stbox(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geodstbox;
CREATE TABLE tbl_geodstbox AS
SELECT k, random_geodstbox(10, 32, 35, 72, 0, 3000, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_geompoint;
CREATE TABLE tbl_geompoint AS
SELECT 1 AS k, geometry 'point empty' AS g UNION
SELECT k, random_geompoint(0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geompoint3D;
CREATE TABLE tbl_geompoint3D AS
SELECT 1 AS k, geometry 'pointZ empty' AS g UNION
SELECT k, random_geompoint3D(0, 100, 0, 100, 0, 100)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogpoint;
CREATE TABLE tbl_geogpoint AS
SELECT 1 AS k, geography 'point empty' AS g UNION
SELECT k, random_geogpoint(-10, 32, 35, 72)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogpoint3D;
CREATE TABLE tbl_geogpoint3D AS
SELECT 1 AS k, geography 'pointZ empty' AS g UNION
SELECT k, random_geogpoint3D(-10, 32, 35, 72, 0, 1000)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geomlinestring;
CREATE TABLE tbl_geomlinestring AS
SELECT 1 AS k, geometry 'linestring empty' AS g UNION
SELECT k, random_geomlinestring(0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geomlinestring3D;
CREATE TABLE tbl_geomlinestring3D AS
SELECT 1 AS k, geometry 'linestring Z empty' AS g UNION
SELECT k, random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geoglinestring;
CREATE TABLE tbl_geoglinestring AS
SELECT 1 AS k, geography 'linestring empty' AS g UNION
SELECT k, random_geoglinestring(-10, 32, 35, 72, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geoglinestring3D;
CREATE TABLE tbl_geoglinestring3D AS
SELECT 1 AS k, geography 'linestring Z empty' AS g UNION
SELECT k, random_geoglinestring3D(-10, 32, 35, 72, 0, 1000, 10) 
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geompolygon;
CREATE TABLE tbl_geompolygon AS
SELECT 1 AS k, geometry 'polygon empty' AS g UNION
SELECT k, random_geompolygon(0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geompolygon3D;
CREATE TABLE tbl_geompolygon3D AS
SELECT 1 AS k, geometry 'polygon Z empty' AS g UNION
SELECT k, random_geompolygon3D(0, 100, 0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogpolygon;
CREATE TABLE tbl_geogpolygon AS
SELECT 1 AS k, geography 'polygon empty' AS g UNION
SELECT k, random_geogpolygon(-10, 32, 35, 72, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogpolygon3D;
CREATE TABLE tbl_geogpolygon3D AS
SELECT 1 AS k, geography 'polygon Z empty' AS g UNION
SELECT k, random_geogpolygon3D(-10, 32, 35, 72, 0, 1000, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultipoint;
CREATE TABLE tbl_geommultipoint AS
SELECT 1 AS k, geometry 'multipoint empty' AS g UNION
SELECT k, random_geommultipoint(0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultipoint3D;
CREATE TABLE tbl_geommultipoint3D AS
SELECT 1 AS k, geometry 'multipoint Z empty' AS g UNION
SELECT k, random_geommultipoint3D(0, 100, 0, 100, 0, 100, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultipoint;
CREATE TABLE tbl_geogmultipoint AS
SELECT 1 AS k, geography 'multipoint empty' AS g UNION
SELECT k, random_geogmultipoint(-10, 32, 35, 72, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultipoint3D;
CREATE TABLE tbl_geogmultipoint3D AS
SELECT 1 AS k, geography 'multipoint Z empty' AS g UNION
SELECT k, random_geogmultipoint3D(-10, 32, 35, 72, 10, 0, 1000)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultilinestring;
CREATE TABLE tbl_geommultilinestring AS
SELECT 1 AS k, geometry 'multilinestring empty' AS g UNION
SELECT k, random_geommultilinestring(0, 100, 0, 100, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultilinestring3D;
CREATE TABLE tbl_geommultilinestring3D AS
SELECT 1 AS k, geometry 'multilinestring Z empty' AS g UNION
SELECT k, random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultilinestring;
CREATE TABLE tbl_geogmultilinestring AS
SELECT 1 AS k, geography 'multilinestring empty' AS g UNION
SELECT k, random_geogmultilinestring(-10, 32, 35, 72, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultilinestring3D;
CREATE TABLE tbl_geogmultilinestring3D AS
SELECT 1 AS k, geography 'multilinestring Z empty' AS g UNION
SELECT k, random_geogmultilinestring3D(-10, 32, 35, 72, 0, 1000, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultipolygon;
CREATE TABLE tbl_geommultipolygon AS
SELECT 1 AS k, geometry 'multipolygon empty' AS g UNION
SELECT k, random_geommultipolygon(0, 100, 0, 100, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geommultipolygon3D;
CREATE TABLE tbl_geommultipolygon3D AS
SELECT 1 AS k, geometry 'multipolygon Z empty' AS g UNION
SELECT k, random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultipolygon;
CREATE TABLE tbl_geogmultipolygon AS
SELECT 1 AS k, geography 'multipolygon empty' AS g UNION
SELECT k, random_geogmultipolygon(-10, 32, 35, 72, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geogmultipolygon3D;
CREATE TABLE tbl_geogmultipolygon3D AS
SELECT 1 AS k, geography 'multipolygon Z empty' AS g UNION
SELECT k, random_geogmultipolygon3D(-10, 32, 35, 72, 0, 1000, 10, 10)
FROM generate_series(2, size) k;

DROP TABLE IF EXISTS tbl_geometry;
CREATE TABLE tbl_geometry (
	k serial PRIMARY KEY,
	g geometry);
INSERT INTO tbl_geometry(g)
(SELECT g FROM tbl_geompoint ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geomlinestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geompolygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultipolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geometry3D;
CREATE TABLE tbl_geometry3D (
	k serial PRIMARY KEY,
	g geometry);
INSERT INTO tbl_geometry3D(g)
(SELECT g FROM tbl_geompoint3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geomlinestring3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geompolygon3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultipoint3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultilinestring3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geommultipolygon3D ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geography;
CREATE TABLE tbl_geography (
	k serial PRIMARY KEY,
	g geography);
INSERT INTO tbl_geography(g)
(SELECT g FROM tbl_geogpoint ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geoglinestring ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geogpolygon ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultipoint ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultilinestring ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultipolygon ORDER BY k LIMIT (size * 0.2));

DROP TABLE IF EXISTS tbl_geography3D;
CREATE TABLE tbl_geography3D (
	k serial PRIMARY KEY,
	g geography);
INSERT INTO tbl_geography3D(g)
(SELECT g FROM tbl_geogpoint3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geoglinestring3D ORDER BY k LIMIT (size * 0.1)) UNION ALL
(SELECT g FROM tbl_geogpolygon3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultipoint3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultilinestring3D ORDER BY k LIMIT (size * 0.2)) UNION ALL
(SELECT g FROM tbl_geogmultipolygon3D ORDER BY k LIMIT (size * 0.2));

------------------------------------------------------------------------------
-- Temporal Point Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointinst;
CREATE TABLE tbl_tgeompointinst AS
SELECT k, random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointinst t1
SET inst = (SELECT inst FROM tbl_tgeompointinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointinst t1
SET inst = (SELECT tgeompointinst(random_geompoint(0, 100, 0, 100), getTimestamp(inst)) 
	FROM tbl_tgeompointinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dinst;
CREATE TABLE tbl_tgeompoint3Dinst AS
SELECT k, random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dinst t1
SET inst = (SELECT inst FROM tbl_tgeompoint3Dinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dinst t1
SET inst = (SELECT tgeompointinst(random_geompoint3D(0, 100, 0, 100, 0, 100), getTimestamp(inst)) 
	FROM tbl_tgeompoint3Dinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointinst;
CREATE TABLE tbl_tgeogpointinst AS
SELECT k, random_tgeogpointinst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointinst t1
SET inst = (SELECT inst FROM tbl_tgeogpointinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointinst t1
SET inst = (SELECT tgeogpointinst(random_geogpoint(-10, 32, 35, 72), getTimestamp(inst)) 
	FROM tbl_tgeogpointinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dinst;
CREATE TABLE tbl_tgeogpoint3Dinst AS
SELECT k, random_tgeogpoint3Dinst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dinst t1
SET inst = (SELECT inst FROM tbl_tgeogpoint3Dinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dinst t1
SET inst = (SELECT tgeogpointinst(random_geogpoint3D(-10, 32, 35, 72, 0, 1000), getTimestamp(inst)) 
	FROM tbl_tgeogpoint3Dinst t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointi;
CREATE TABLE tbl_tgeompointi AS
SELECT k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointi t1
SET ti = (SELECT ti FROM tbl_tgeompointi t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointi t1
SET ti = (SELECT setPrecision(ti,6) FROM tbl_tgeompointi t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompointi t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	FROM tbl_tgeompointi t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompointi t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	FROM tbl_tgeompointi t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Di;
CREATE TABLE tbl_tgeompoint3Di AS
SELECT k, random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Di t1
SET ti = (SELECT ti FROM tbl_tgeompoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Di t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeompoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Di t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	FROM tbl_tgeompoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Di t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	FROM tbl_tgeompoint3Di t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointi;
CREATE TABLE tbl_tgeogpointi AS
SELECT k, random_tgeogpointi(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointi t1
SET ti = (SELECT ti FROM tbl_tgeogpointi t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointi t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeogpointi t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpointi t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	FROM tbl_tgeogpointi t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpointi t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	FROM tbl_tgeogpointi t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Di;
CREATE TABLE tbl_tgeogpoint3Di AS
SELECT k, random_tgeogpoint3Di(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Di t1
SET ti = (SELECT ti FROM tbl_tgeogpoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Di t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeogpoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Di t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	FROM tbl_tgeogpoint3Di t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Di t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	FROM tbl_tgeogpoint3Di t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointseq;
CREATE TABLE tbl_tgeompointseq AS
SELECT k, random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointseq t1
SET seq = (SELECT seq FROM tbl_tgeompointseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointseq t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeompointseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompointseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeompointseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompointseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
	FROM tbl_tgeompointseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dseq;
CREATE TABLE tbl_tgeompoint3Dseq AS
SELECT k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dseq t1
SET seq = (SELECT seq FROM tbl_tgeompoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dseq t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeompoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Dseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeompoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Dseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
	FROM tbl_tgeompoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointseq;
CREATE TABLE tbl_tgeogpointseq AS
SELECT k, random_tgeogpointseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointseq t1
SET seq = (SELECT seq FROM tbl_tgeogpointseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointseq t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeogpointseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpointseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeogpointseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpointseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
	FROM tbl_tgeogpointseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dseq;
CREATE TABLE tbl_tgeogpoint3Dseq AS
SELECT k, random_tgeogpoint3Dseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dseq t1
SET seq = (SELECT seq FROM tbl_tgeogpoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dseq t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeogpoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Dseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeogpoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Dseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
	FROM tbl_tgeogpoint3Dseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoints;
CREATE TABLE tbl_tgeompoints AS
SELECT k, random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoints t1
SET ts = (SELECT ts FROM tbl_tgeompoints t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoints t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeompoints t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoints t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeompoints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoints t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
	FROM tbl_tgeompoints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Ds;
CREATE TABLE tbl_tgeompoint3Ds AS
SELECT k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Ds t1
SET ts = (SELECT ts FROM tbl_tgeompoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Ds t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeompoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Ds t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeompoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Ds t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
	FROM tbl_tgeompoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoints;
CREATE TABLE tbl_tgeogpoints AS
SELECT k, random_tgeogpoints(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoints t1
SET ts = (SELECT ts FROM tbl_tgeogpoints t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoints t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeogpoints t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoints t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeogpoints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoints t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
	FROM tbl_tgeogpoints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Ds;
CREATE TABLE tbl_tgeogpoint3Ds AS
SELECT k, random_tgeogpoint3Ds(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Ds t1
SET ts = (SELECT ts FROM tbl_tgeogpoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Ds t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeogpoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Ds t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeogpoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Ds t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
	FROM tbl_tgeogpoint3Ds t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint;
CREATE TABLE tbl_tgeompoint(k, temp) AS
(SELECT k, inst FROM tbl_tgeompointinst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeompointi ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeompointseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompoints ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeompoint3D;
CREATE TABLE tbl_tgeompoint3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeompoint3Dinst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeompoint3Di ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeompoint3Dseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompoint3Ds ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint;
CREATE TABLE tbl_tgeogpoint(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpointinst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeogpointi ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeogpointseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpoints ORDER BY k LIMIT size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint3D;
CREATE TABLE tbl_tgeogpoint3D(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpoint3Dinst ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4, ti FROM tbl_tgeogpoint3Di ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 2, seq FROM tbl_tgeogpoint3Dseq ORDER BY k LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpoint3Ds ORDER BY k LIMIT size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_tpoint(100)
