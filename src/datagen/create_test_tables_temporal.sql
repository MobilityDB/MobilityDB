/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * create_test_tables_temporal.sql
 * Function generating a set of test tables for some PostgreSQL data types 
 * and for temporal data types.
 *
 * These functions use the random generator for these types that are in the
 * file random_temporal.sql. Refer to that file for the meaning of the 
 * parameters used in the function calls of this file.
 */
 
DROP FUNCTION IF EXISTS create_test_tables_temporal();
CREATE OR REPLACE FUNCTION create_test_tables_temporal(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.01;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_bool;
CREATE TABLE tbl_bool AS
/* Add perc NULL values */
SELECT k, NULL AS b
FROM generate_series(1, perc) AS k UNION
SELECT k, random_bool()
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_int;
CREATE TABLE tbl_int AS
/* Add perc NULL values */
SELECT k, NULL AS i
FROM generate_series(1, perc) AS k UNION
SELECT k, random_int(1, 100)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_float;
CREATE TABLE tbl_float AS
/* Add perc NULL values */
SELECT k, NULL AS f
FROM generate_series(1, perc) AS k UNION
SELECT k, random_float(1, 100)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_text;
CREATE TABLE tbl_text AS
/* Add perc NULL values */
SELECT k, NULL AS t
FROM generate_series(1, perc) AS k UNION
SELECT k, random_text(10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_tbox;
CREATE TABLE tbl_tbox AS
/* Add perc NULL values */
SELECT k, NULL AS b
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tbox(0, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_interval;
CREATE TABLE tbl_interval AS
/* Add perc NULL values */
SELECT k, NULL AS i
FROM generate_series(1, perc) AS k UNION
SELECT k, random_minutes(1, 100)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_timestamptz;
CREATE TABLE tbl_timestamptz AS
/* Add perc NULL values */
SELECT k, NULL AS t
FROM generate_series(1, perc) AS k UNION
SELECT k, random_timestamptz('2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_intrange;
CREATE TABLE tbl_intrange AS
/* Add perc NULL values */
SELECT k, NULL AS i
FROM generate_series(1, perc) AS k UNION
SELECT k, random_intrange(1, 100, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_floatrange;
CREATE TABLE tbl_floatrange AS
/* Add perc NULL values */
SELECT k, NULL AS f
FROM generate_series(1, perc) AS k UNION
SELECT k, random_floatrange(1, 100, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_tstzrange;
CREATE TABLE tbl_tstzrange AS
/* Add perc NULL values */
SELECT k, NULL AS r
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tstzrange('2001-01-01', '2001-12-31', 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_tstzrangearr;
CREATE TABLE tbl_tstzrangearr AS
/* Add perc NULL values */
SELECT k, NULL AS ra
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tstzrange_array('2001-01-01', '2001-12-31', 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

-------------------------------------------------------------------------------
-- Time types
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_timestampset;
CREATE TABLE tbl_timestampset AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_timestampset('2001-01-01', '2001-12-31', 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_period;
CREATE TABLE tbl_period AS
/* Add perc NULL values */
SELECT k, NULL AS p
FROM generate_series(1, perc) AS k UNION
SELECT k, random_period('2001-01-01', '2001-12-31', 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_periodset;
CREATE TABLE tbl_periodset AS
/* Add perc NULL values */
SELECT k, NULL AS ps
FROM generate_series(1, perc) AS k UNION
SELECT k, random_periodset('2001-01-01', '2001-12-31', 10, 5, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tboolinst;
CREATE TABLE tbl_tboolinst AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tboolinst('2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tboolinst t1
SET inst = (SELECT inst FROM tbl_tboolinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tboolinst t1
SET inst = (SELECT tboolinst(random_bool(), getTimestamp(inst))
  FROM tbl_tboolinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_tintinst;
CREATE TABLE tbl_tintinst AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tintinst(1, 100, '2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tintinst t1
SET inst = (SELECT inst FROM tbl_tintinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tintinst t1
SET inst = (SELECT tintinst(random_int(1, 100), getTimestamp(inst))
  FROM tbl_tintinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_tfloatinst;
CREATE TABLE tbl_tfloatinst AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloatinst(1, 100, '2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloatinst t1
SET inst = (SELECT inst FROM tbl_tfloatinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloatinst t1
SET inst = (SELECT tfloatinst(random_float(1, 100), getTimestamp(inst))
  FROM tbl_tfloatinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_ttextinst;
CREATE TABLE tbl_ttextinst AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttextinst('2001-01-01', '2001-12-31', 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttextinst t1
SET inst = (SELECT inst FROM tbl_ttextinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttextinst t1
SET inst = (SELECT ttextinst(random_text(10), getTimestamp(inst))
  FROM tbl_ttextinst t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbooli;
CREATE TABLE tbl_tbooli AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tbooli('2001-01-01', '2001-12-31', 10, 5, 10) AS ti
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tbooli t1
SET ti = (SELECT ti FROM tbl_tbooli t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tbooli t1
SET ti = (SELECT ~ ti FROM tbl_tbooli t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tbooli t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tbooli t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tbooli t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tbooli t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tinti;
CREATE TABLE tbl_tinti AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tinti(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tinti t1
SET ti = (SELECT ti FROM tbl_tinti t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tinti t1
SET ti = (SELECT ti + random_int(1, 2) FROM tbl_tinti t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tinti t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tinti t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tinti t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tinti t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloati;
CREATE TABLE tbl_tfloati AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloati(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloati t1
SET ti = (SELECT ti FROM tbl_tfloati t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloati t1
SET ti = (SELECT ti + random_int(1, 2) FROM tbl_tfloati t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloati t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tfloati t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloati t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tfloati t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttexti;
CREATE TABLE tbl_ttexti AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttexti t1
SET ti = (SELECT ti FROM tbl_ttexti t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttexti t1
SET ti = (SELECT ti || text 'A' FROM tbl_ttexti t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttexti t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_ttexti t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttexti t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_ttexti t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tboolseq;
CREATE TABLE tbl_tboolseq AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tboolseq('2001-01-01', '2001-12-31', 10, 5, 10) AS seq
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tboolseq t1
SET seq = (SELECT seq FROM tbl_tboolseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tboolseq t1
SET seq = (SELECT ~ seq FROM tbl_tboolseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tboolseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tboolseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tboolseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tboolseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tintseq;
CREATE TABLE tbl_tintseq AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tintseq(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tintseq t1
SET seq = (SELECT seq FROM tbl_tintseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tintseq t1
SET seq = (SELECT seq + random_int(1, 2) FROM tbl_tintseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tintseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tintseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tintseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tintseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloatseq;
CREATE TABLE tbl_tfloatseq AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloatseq(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloatseq t1
SET seq = (SELECT seq FROM tbl_tfloatseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloatseq t1
SET seq = (SELECT seq + random_int(1, 2) FROM tbl_tfloatseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloatseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tfloatseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloatseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tfloatseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttextseq;
CREATE TABLE tbl_ttextseq AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttextseq('2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttextseq t1
SET seq = (SELECT seq FROM tbl_ttextseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttextseq t1
SET seq = (SELECT seq || text 'A' FROM tbl_ttextseq t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttextseq t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_ttextseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttextseq t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_ttextseq t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbools;
CREATE TABLE tbl_tbools AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tbools('2001-01-01', '2001-12-31', 10, 5, 10, 5, 10) AS ts
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tbools t1
SET ts = (SELECT ts FROM tbl_tbools t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tbools t1
SET ts = (SELECT ~ ts FROM tbl_tbools t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tbools t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tbools t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tbools t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tbools t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tints;
CREATE TABLE tbl_tints AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tints t1
SET ts = (SELECT ts FROM tbl_tints t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tints t1
SET ts = (SELECT ts + random_int(1, 2) FROM tbl_tints t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tints t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tints t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tints t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloats;
CREATE TABLE tbl_tfloats AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tfloats t1
SET ts = (SELECT ts FROM tbl_tfloats t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloats t1
SET ts = (SELECT ts + random_int(1, 2) FROM tbl_tfloats t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloats t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tfloats t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloats t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tfloats t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttexts;
CREATE TABLE tbl_ttexts AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttexts('2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_ttexts t1
SET ts = (SELECT ts FROM tbl_ttexts t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttexts t1
SET ts = (SELECT ts || text 'A' FROM tbl_ttexts t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttexts t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_ttexts t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttexts t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_ttexts t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbool;
CREATE TABLE tbl_tbool(k, temp) AS
(SELECT k, inst FROM tbl_tboolinst order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tbooli order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tboolseq order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tbools order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tint;
CREATE TABLE tbl_tint(k, temp) AS
(SELECT k, inst FROM tbl_tintinst order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tinti order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tintseq order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tints order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tfloat;
CREATE TABLE tbl_tfloat(k, temp) AS
(SELECT k, inst FROM tbl_tfloatinst order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tfloati order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tfloatseq order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tfloats order by k limit size / 4);

DROP TABLE IF EXISTS tbl_ttext;
CREATE TABLE tbl_ttext(k, temp) AS
(SELECT k, inst FROM tbl_ttextinst order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_ttexti order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_ttextseq order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_ttexts order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_temporal(100);

