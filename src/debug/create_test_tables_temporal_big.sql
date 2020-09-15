DROP FUNCTION IF EXISTS create_test_tables_temporal_big();
CREATE OR REPLACE FUNCTION create_test_tables_temporal_big(size int DEFAULT 100) 
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.01;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_timestampset_big;
CREATE TABLE tbl_timestampset_big AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_timestampset('2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_period_big;
CREATE TABLE tbl_period_big AS
/* Add perc NULL values */
SELECT k, NULL AS p
FROM generate_series(1, perc) AS k UNION
SELECT k, random_period('2001-01-01', '2001-12-31', 10)
FROM generate_series(perc+1, size) AS k;

DROP TABLE IF EXISTS tbl_periodset_big;
CREATE TABLE tbl_periodset_big AS
/* Add perc NULL values */
SELECT k, NULL AS ps
FROM generate_series(1, perc) AS k UNION
SELECT k, random_periodset('2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) AS k;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tboolinst_big;
CREATE TABLE tbl_tboolinst_big AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tboolinst('2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tboolinst_big t1
SET inst = (SELECT inst FROM tbl_tboolinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tboolinst_big t1
SET inst = (SELECT tboolinst(random_bool(), getTimestamp(inst)) 
  FROM tbl_tboolinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_tintinst_big;
CREATE TABLE tbl_tintinst_big AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tintinst(1, 100, '2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tintinst_big t1
SET inst = (SELECT inst FROM tbl_tintinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tintinst_big t1
SET inst = (SELECT tintinst(random_int(1, 100), getTimestamp(inst)) 
  FROM tbl_tintinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_tfloatinst_big;
CREATE TABLE tbl_tfloatinst_big AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloatinst(1, 100, '2001-01-01', '2001-12-31')
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloatinst_big t1
SET inst = (SELECT inst FROM tbl_tfloatinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloatinst_big t1
SET inst = (SELECT tfloatinst(random_float(1, 100), getTimestamp(inst)) 
  FROM tbl_tfloatinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

DROP TABLE IF EXISTS tbl_ttextinst_big;
CREATE TABLE tbl_ttextinst_big AS
/* Add perc NULL values */
SELECT k, NULL AS inst
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttextinst('2001-01-01', '2001-12-31', 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttextinst_big t1
SET inst = (SELECT inst FROM tbl_ttextinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttextinst_big t1
SET inst = (SELECT ttextinst(random_text(10), getTimestamp(inst)) 
  FROM tbl_ttextinst_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbooli_big;
CREATE TABLE tbl_tbooli_big AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tbooli('2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tbooli_big t1
SET ti = (SELECT ti FROM tbl_tbooli_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tbooli_big t1
SET ti = (SELECT ~ ti FROM tbl_tbooli_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tbooli_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
  FROM tbl_tbooli_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tbooli_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
  FROM tbl_tbooli_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tinti_big;
CREATE TABLE tbl_tinti_big AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tinti(1, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tinti_big t1
SET ti = (SELECT ti FROM tbl_tinti_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tinti_big t1
SET ti = (SELECT ti + random_int(1, 2) FROM tbl_tinti_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tinti_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
  FROM tbl_tinti_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tinti_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
  FROM tbl_tinti_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloati_big;
CREATE TABLE tbl_tfloati_big AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloati(1, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloati_big t1
SET ti = (SELECT ti FROM tbl_tfloati_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloati_big t1
SET ti = (SELECT ti + random_int(1, 2) FROM tbl_tfloati_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloati_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
  FROM tbl_tfloati_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloati_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
  FROM tbl_tfloati_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttexti_big;
CREATE TABLE tbl_ttexti_big AS
/* Add perc NULL values */
SELECT k, NULL AS ti
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttexti_big t1
SET ti = (SELECT ti FROM tbl_ttexti_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttexti_big t1
SET ti = (SELECT ti || text 'A' FROM tbl_ttexti_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttexti_big t1
SET ti = (SELECT shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
  FROM tbl_ttexti_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttexti_big t1
SET ti = (SELECT shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
  FROM tbl_ttexti_big t2 WHERE t2.k = t1.k+2)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tboolseq_big;
CREATE TABLE tbl_tboolseq_big AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tboolseq('2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tboolseq_big t1
SET seq = (SELECT seq FROM tbl_tboolseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tboolseq_big t1
SET seq = (SELECT ~ seq FROM tbl_tboolseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tboolseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tboolseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tboolseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
  FROM tbl_tboolseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tintseq_big;
CREATE TABLE tbl_tintseq_big AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tintseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tintseq_big t1
SET seq = (SELECT seq FROM tbl_tintseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tintseq_big t1
SET seq = (SELECT seq + random_int(1, 2) FROM tbl_tintseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tintseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tintseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tintseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
  FROM tbl_tintseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloatseq_big;
CREATE TABLE tbl_tfloatseq_big AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloatseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_tfloatseq_big t1
SET seq = (SELECT seq FROM tbl_tfloatseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloatseq_big t1
SET seq = (SELECT seq + random_int(1, 2) FROM tbl_tfloatseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloatseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_tfloatseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloatseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
  FROM tbl_tfloatseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttextseq_big;
CREATE TABLE tbl_ttextseq_big AS
/* Add perc NULL values */
SELECT k, NULL AS seq
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttextseq('2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(perc+1, size) k;
/* Add perc duplicates */
UPDATE tbl_ttextseq_big t1
SET seq = (SELECT seq FROM tbl_ttextseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttextseq_big t1
SET seq = (SELECT seq || text 'A' FROM tbl_ttextseq_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttextseq_big t1
SET seq = (SELECT shift(seq, timespan(seq)) FROM tbl_ttextseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttextseq_big t1
SET seq = (SELECT shift(seq, date_trunc('minute',timespan(seq)/2)) 
  FROM tbl_ttextseq_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbools_big;
CREATE TABLE tbl_tbools_big AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tbools('2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tbools_big t1
SET ts = (SELECT ts FROM tbl_tbools_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tbools_big t1
SET ts = (SELECT ~ ts FROM tbl_tbools_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tbools_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tbools_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tbools_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
  FROM tbl_tbools_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tints_big;
CREATE TABLE tbl_tints_big AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tints_big t1
SET ts = (SELECT ts FROM tbl_tints_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tints_big t1
SET ts = (SELECT ts + random_int(1, 2) FROM tbl_tints_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tints_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tints_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tints_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
  FROM tbl_tints_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_tfloats_big;
CREATE TABLE tbl_tfloats_big AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_tfloats_big t1
SET ts = (SELECT ts FROM tbl_tfloats_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_tfloats_big t1
SET ts = (SELECT ts + random_int(1, 2) FROM tbl_tfloats_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_tfloats_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_tfloats_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_tfloats_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
  FROM tbl_tfloats_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

DROP TABLE IF EXISTS tbl_ttexts_big;
CREATE TABLE tbl_ttexts_big AS
/* Add perc NULL values */
SELECT k, NULL AS ts
FROM generate_series(1, perc) AS k UNION
SELECT k, random_ttexts('2001-01-01', '2001-12-31', 10, 10, 10, 10)
FROM generate_series(perc+1, size) AS k;
/* Add perc duplicates */
UPDATE tbl_ttexts_big t1
SET ts = (SELECT ts FROM tbl_ttexts_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
UPDATE tbl_ttexts_big t1
SET ts = (SELECT ts || text 'A' FROM tbl_ttexts_big t2 WHERE t2.k = t1.k+perc)
WHERE k in (SELECT i FROM generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
UPDATE tbl_ttexts_big t1
SET ts = (SELECT shift(ts, timespan(ts)) FROM tbl_ttexts_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
UPDATE tbl_ttexts_big t1
SET ts = (SELECT shift(ts, date_trunc('minute', timespan(ts)/2)) 
  FROM tbl_ttexts_big t2 WHERE t2.k = t1.k+perc)
WHERE t1.k in (SELECT i FROM generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tbool_big;
CREATE TABLE tbl_tbool_big(k, temp) AS
(SELECT k, inst FROM tbl_tboolinst_big order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tbooli_big order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tboolseq_big order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tbools_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tint_big;
CREATE TABLE tbl_tint_big(k, temp) AS
(SELECT k, inst FROM tbl_tinttinst_big order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tinti_big order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tintseq_big order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tints_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_tfloat_big;
CREATE TABLE tbl_tfloat_big(k, temp) AS
(SELECT k, inst FROM tbl_tfloatinst_big order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_tfloati_big order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_tfloatseq_big order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_tfloats_big order by k limit size / 4);

DROP TABLE IF EXISTS tbl_ttext_big;
CREATE TABLE tbl_ttext_big(k, temp) AS
(SELECT k, inst FROM tbl_ttextinst_big order by k limit size / 4) UNION all
(SELECT k + size / 4, ti FROM tbl_ttexti_big order by k limit size / 4) UNION all
(SELECT k + size / 2, seq FROM tbl_ttextseq_big order by k limit size / 4) UNION all
(SELECT k + size / 4 * 3, ts FROM tbl_ttexts_big order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_temporal_big(10000)