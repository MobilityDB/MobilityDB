-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Tests for the geo set data type
-- File set.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_geomset TO '/tmp/tbl_geomset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_geomset_tmp;
CREATE TABLE tbl_geomset_tmp AS TABLE tbl_geomset WITH NO DATA;
COPY tbl_geomset_tmp FROM '/tmp/tbl_geomset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset_tmp t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
DROP TABLE tbl_geomset_tmp;

COPY tbl_geogset TO '/tmp/tbl_geogset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_geogset_tmp;
CREATE TABLE tbl_geogset_tmp AS TABLE tbl_geogset WITH NO DATA;
COPY tbl_geogset_tmp FROM '/tmp/tbl_geogset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset_tmp t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
DROP TABLE tbl_geogset_tmp;

-- Input/output from/to WKT, EWKT, WKB, and HexWKB

SELECT MAX(length(asText(g))) FROM tbl_geomset;
SELECT MAX(length(asText(g))) FROM tbl_geogset;
SELECT MAX(length(asEWKT(g))) FROM tbl_geomset;
SELECT MAX(length(asEWKT(g))) FROM tbl_geogset;

SELECT COUNT(*) FROM tbl_geomset WHERE geomsetFromBinary(asBinary(g)) <> g;
SELECT COUNT(*) FROM tbl_geogset WHERE geogsetFromBinary(asBinary(g)) <> g;

SELECT COUNT(*) FROM tbl_geomset WHERE geomsetFromHexWKB(asHexWKB(g)) <> g;
SELECT COUNT(*) FROM tbl_geogset WHERE geogsetFromHexWKB(asHexWKB(g)) <> g;

SELECT asText(geomsetFromHexWKB(asHexWKB(geomset '{"Point(1 1 1)"}')));

-------------------------------------------------------------------------------
-- Constructor

SELECT memSize(set_union(g)) FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g);
SELECT memSize(set_union(g)) FROM tbl_geog_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry);

-------------------------------------------------------------------------------
-- Cast

SELECT MAX(memSize(set(g))) FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g);
SELECT MAX(memSize(set(g))) FROM tbl_geog_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g::geometry);

-- Coverage of periodset_stbox_slice
DROP TABLE IF EXISTS test;
CREATE TABLE test(ps) AS
WITH test(ps) AS (
  SELECT span(day, day + interval '1 hour')
  FROM generate_series(timestamptz '2000-01-01', timestamptz '2000-04-01', '1 day') AS day )
SELECT spanset(array_agg(ps)) FROM test;
SELECT ps::stbox FROM test;
DROP TABLE test;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(ST_X(startValue(round(g, 6)))) FROM tbl_geomset;
SELECT MIN(ST_X(startValue(round(g, 6))::geometry)) FROM tbl_geogset;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(g)) FROM tbl_geomset;
SELECT MIN(numValues(g)) FROM tbl_geomset;
SELECT MIN(ST_X(startValue(g))) FROM tbl_geomset;
SELECT MIN(ST_X(endValue(g))) FROM tbl_geomset;
SELECT MIN(ST_X(valueN(g, 1))) FROM tbl_geomset;
SELECT MIN(array_length(getValues(g), 1)) FROM tbl_geomset;

SELECT MAX(memSize(g)) FROM tbl_geogset;
SELECT MIN(numValues(g)) FROM tbl_geogset;
SELECT MIN(ST_X(startValue(g)::geometry)) FROM tbl_geogset;
SELECT MIN(ST_X(endValue(g)::geometry)) FROM tbl_geogset;
SELECT MIN(ST_X(valueN(g, 1)::geometry)) FROM tbl_geogset;
SELECT MIN(array_length(getValues(g), 1)) FROM tbl_geogset;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(set_union(g)) FROM tbl_geom_point3D WHERE NOT ST_IsEmpty(g);
SELECT numValues(set_union(g)) FROM tbl_geog_point3D WHERE NOT ST_IsEmpty(g::geometry);

WITH test1(k, g) AS (
  SELECT k, unnest(g) FROM tbl_geomset ),
test2 (k, g) AS (
  SELECT k, set_union(g) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_geomset t2 WHERE t1.k = t2.k AND t1.g <> t2.g;
WITH test1(k, g) AS (
  SELECT k, unnest(g) FROM tbl_geogset ),
test2 (k, g) AS (
  SELECT k, set_union(g) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_geogset t2 WHERE t1.k = t2.k AND t1.g <> t2.g;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE set_cmp(t1.g, t2.g) = -1;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g = t2.g;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g <> t2.g;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g < t2.g;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g <= t2.g;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g > t2.g;
SELECT COUNT(*) FROM tbl_geomset t1, tbl_geomset t2 WHERE t1.g >= t2.g;

SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE set_cmp(t1.g, t2.g) = -1;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g = t2.g;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g <> t2.g;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g < t2.g;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g <= t2.g;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g > t2.g;
SELECT COUNT(*) FROM tbl_geogset t1, tbl_geogset t2 WHERE t1.g >= t2.g;

SELECT MAX(set_hash(g)) FROM tbl_geomset;
SELECT MAX(set_hash(g)) FROM tbl_geogset;

-- PostGIS currently does not provide an extended hash function
-- SELECT MAX(set_hash_extended(g, 1)) FROM tbl_geomset;
-- SELECT MAX(set_hash_extended(g, 1)) FROM tbl_geogset;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(set_union(g)) FROM tbl_geom_point WHERE NOT ST_IsEmpty(g);

-------------------------------------------------------------------------------
