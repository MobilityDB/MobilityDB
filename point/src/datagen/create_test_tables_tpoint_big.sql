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

DROP FUNCTION IF EXISTS create_test_tables_tpoint_big();
CREATE OR REPLACE FUNCTION create_test_tables_tpoint_big(size int DEFAULT 10000)
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.02;
IF perc < 1 THEN perc := 1; END IF;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointinst_big;
CREATE TABLE tbl_tgeompointinst_big AS
SELECT k, random_tgeompoint_inst(0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointinst_big t1
SET inst = (SELECT inst FROM tbl_tgeompointinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointinst_big t1
SET inst = (SELECT tgeompoint_inst(random_geompoint(0, 100, 0, 100), getTimestamp(inst))
  FROM tbl_tgeompointinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dinst_big;
CREATE TABLE tbl_tgeompoint3Dinst_big AS
SELECT k, random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dinst_big t1
SET inst = (SELECT inst FROM tbl_tgeompoint3Dinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dinst_big t1
SET inst = (SELECT tgeompoint_inst(random_geompoint3D(0, 100, 0, 100, 0, 100), getTimestamp(inst))
  FROM tbl_tgeompoint3Dinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointinst_big;
CREATE TABLE tbl_tgeogpointinst_big AS
SELECT k, random_tgeogpoint_inst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointinst_big t1
SET inst = (SELECT inst FROM tbl_tgeogpointinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointinst_big t1
SET inst = (SELECT tgeogpoint_inst(random_geogpoint(-10, 32, 35, 72), getTimestamp(inst))
  FROM tbl_tgeogpointinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dinst_big;
CREATE TABLE tbl_tgeogpoint3Dinst_big AS
SELECT k, random_tgeogpoint3Dinst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dinst_big t1
SET inst = (SELECT inst FROM tbl_tgeogpoint3Dinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dinst_big t1
SET inst = (SELECT tgeogpoint_inst(random_geogpoint3D(-10, 32, 35, 72, 0, 1000), getTimestamp(inst))
  FROM tbl_tgeogpoint3Dinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointinstset_big;
CREATE TABLE tbl_tgeompointinstset_big AS
SELECT k, random_tgeompoint_instset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointinstset_big t1
SET ti = (SELECT ti FROM tbl_tgeompointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointinstset_big t1
SET ti = (SELECT setPrecision(ti,6) FROM tbl_tgeompointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompointinstset_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeompointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompointinstset_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeompointinstset_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dinstset_big;
CREATE TABLE tbl_tgeompoint3Dinstset_big AS
SELECT k, random_tgeompoint3Dinstset(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dinstset_big t1
SET ti = (SELECT ti FROM tbl_tgeompoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dinstset_big t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeompoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Dinstset_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeompoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Dinstset_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeompoint3Dinstset_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointinstset_big;
CREATE TABLE tbl_tgeogpointinstset_big AS
SELECT k, random_tgeogpoint_instset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointinstset_big t1
SET ti = (SELECT ti FROM tbl_tgeogpointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointinstset_big t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeogpointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpointinstset_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeogpointinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpointinstset_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeogpointinstset_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dinstset_big;
CREATE TABLE tbl_tgeogpoint3Dinstset_big AS
SELECT k, random_tgeogpoint3Dinstset(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dinstset_big t1
SET ti = (SELECT ti FROM tbl_tgeogpoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dinstset_big t1
SET ti = (SELECT setPrecision(ti,3) FROM tbl_tgeogpoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Dinstset_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti))
  FROM tbl_tgeogpoint3Dinstset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Dinstset_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2))
  FROM tbl_tgeogpoint3Dinstset_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointseq_big;
CREATE TABLE tbl_tgeompointseq_big AS
SELECT k, random_tgeompoint_seq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompointseq_big t1
SET seq = (SELECT seq FROM tbl_tgeompointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointseq_big t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeompointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompointseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeompointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompointseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tgeompointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dseq_big;
CREATE TABLE tbl_tgeompoint3Dseq_big AS
SELECT k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dseq_big t1
SET seq = (SELECT seq FROM tbl_tgeompoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dseq_big t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeompoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Dseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeompoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Dseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tgeompoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointseq_big;
CREATE TABLE tbl_tgeogpointseq_big AS
SELECT k, random_tgeogpoint_seq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointseq_big t1
SET seq = (SELECT seq FROM tbl_tgeogpointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointseq_big t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeogpointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpointseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeogpointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpointseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tgeogpointseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dseq_big;
CREATE TABLE tbl_tgeogpoint3Dseq_big AS
SELECT k, random_tgeogpoint3Dseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10) AS seq
FROM generate_series(1, size) k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dseq_big t1
SET seq = (SELECT seq FROM tbl_tgeogpoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dseq_big t1
SET seq = (SELECT setPrecision(seq,3) FROM tbl_tgeogpoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Dseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tgeogpoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Dseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2))
  FROM tbl_tgeogpoint3Dseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointseqset_big;
CREATE TABLE tbl_tgeompointseqset_big AS
SELECT k, random_tgeompoint_seqset(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompointseqset_big t1
SET ts = (SELECT ts FROM tbl_tgeompointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompointseqset_big t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeompointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompointseqset_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeompointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompointseqset_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tgeompointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeompoint3Dseqset_big;
CREATE TABLE tbl_tgeompoint3Dseqset_big AS
SELECT k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeompoint3Dseqset_big t1
SET ts = (SELECT ts FROM tbl_tgeompoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeompoint3Dseqset_big t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeompoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeompoint3Dseqset_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeompoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeompoint3Dseqset_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tgeompoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpointseqset_big;
CREATE TABLE tbl_tgeogpointseqset_big AS
SELECT k, random_tgeogpoint_seqset(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpointseqset_big t1
SET ts = (SELECT ts FROM tbl_tgeogpointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpointseqset_big t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeogpointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpointseqset_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeogpointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpointseqset_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tgeogpointseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

DROP TABLE IF EXISTS tbl_tgeogpoint3Dseqset_big;
CREATE TABLE tbl_tgeogpoint3Dseqset_big AS
SELECT k, random_tgeogpoint3Ds(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tgeogpoint3Dseqset_big t1
SET ts = (SELECT ts FROM tbl_tgeogpoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tgeogpoint3Dseqset_big t1
SET ts = (SELECT setPrecision(ts,3) FROM tbl_tgeogpoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE k IN (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tgeogpoint3Dseqset_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tgeogpoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tgeogpoint3Dseqset_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2))
  FROM tbl_tgeogpoint3Dseqset_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k IN (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompoint_big;
CREATE TABLE tbl_tgeompoint_big(k, temp) AS
(SELECT k, inst FROM tbl_tgeompointinst_big order by k limit size / 4) union all
(SELECT k + size / 4, ti FROM tbl_tgeompointinstset_big order by k limit size / 4) union all
(SELECT k + size / 2, seq FROM tbl_tgeompointseq_big order by k limit size / 4) union all
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompointseqset_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tgeompoint3D_big;
CREATE TABLE tbl_tgeompoint3D_big(k, temp) AS
(SELECT k, inst FROM tbl_tgeompoint3Dinst_big order by k limit size / 4) union all
(SELECT k + size / 4, ti FROM tbl_tgeompoint3Dinstset_big order by k limit size / 4) union all
(SELECT k + size / 2, seq FROM tbl_tgeompoint3Dseq_big order by k limit size / 4) union all
(SELECT k + size / 4 * 3, ts FROM tbl_tgeompoint3Dseqset_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint_big;
CREATE TABLE tbl_tgeogpoint_big(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpointinst_big order by k limit size / 4) union all
(SELECT k + size / 4, ti FROM tbl_tgeogpointinstset_big order by k limit size / 4) union all
(SELECT k + size / 2, seq FROM tbl_tgeogpointseq_big order by k limit size / 4) union all
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpointseqset_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tgeogpoint3D_big;
CREATE TABLE tbl_tgeogpoint3D_big(k, temp) AS
(SELECT k, inst FROM tbl_tgeogpoint3Dinst_big order by k limit size / 4) union all
(SELECT k + size / 4, ti FROM tbl_tgeogpoint3Dinstset_big order by k limit size / 4) union all
(SELECT k + size / 2, seq FROM tbl_tgeogpoint3Dseq_big order by k limit size / 4) union all
(SELECT k + size / 4 * 3, ts FROM tbl_tgeogpoint3Dseqset_big order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_tpoint_big(10000);

