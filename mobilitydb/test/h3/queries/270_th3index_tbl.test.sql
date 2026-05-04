-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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
-- Build per-flavor th3index tables from the existing tbl_tbigint_* fixture
-- tables. The cast is a zero-cost binary coercion since the two types share
-- the same on-disk layout.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index;
DROP TABLE IF EXISTS tbl_th3index_inst;
DROP TABLE IF EXISTS tbl_th3index_discseq;
DROP TABLE IF EXISTS tbl_th3index_seq;
DROP TABLE IF EXISTS tbl_th3index_seqset;

CREATE TABLE tbl_th3index AS
  SELECT k, temp::th3index AS temp FROM tbl_tbigint;
CREATE TABLE tbl_th3index_inst AS
  SELECT k, inst::th3index AS inst FROM tbl_tbigint_inst;
CREATE TABLE tbl_th3index_discseq AS
  SELECT k, ti::th3index AS ti FROM tbl_tbigint_discseq;
CREATE TABLE tbl_th3index_seq AS
  SELECT k, seq::th3index AS seq FROM tbl_tbigint_seq;
CREATE TABLE tbl_th3index_seqset AS
  SELECT k, ss::th3index AS ss FROM tbl_tbigint_seqset;

-------------------------------------------------------------------------------
-- Cardinality matches (cast preserves all rows including NULLs)
-------------------------------------------------------------------------------

SELECT (SELECT COUNT(*) FROM tbl_th3index)         = (SELECT COUNT(*) FROM tbl_tbigint);
SELECT (SELECT COUNT(*) FROM tbl_th3index_inst)    = (SELECT COUNT(*) FROM tbl_tbigint_inst);
SELECT (SELECT COUNT(*) FROM tbl_th3index_discseq) = (SELECT COUNT(*) FROM tbl_tbigint_discseq);
SELECT (SELECT COUNT(*) FROM tbl_th3index_seq)     = (SELECT COUNT(*) FROM tbl_tbigint_seq);
SELECT (SELECT COUNT(*) FROM tbl_th3index_seqset)  = (SELECT COUNT(*) FROM tbl_tbigint_seqset);

-------------------------------------------------------------------------------
-- Round-trip cast preserves value byte-for-byte
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbigint t1, tbl_th3index t2
  WHERE t1.k = t2.k AND t1.temp IS NOT NULL
  AND (t2.temp::tbigint) <> t1.temp;
SELECT COUNT(*) FROM tbl_tbigint_inst t1, tbl_th3index_inst t2
  WHERE t1.k = t2.k AND t1.inst IS NOT NULL
  AND (t2.inst::tbigint) <> t1.inst;
SELECT COUNT(*) FROM tbl_tbigint_discseq t1, tbl_th3index_discseq t2
  WHERE t1.k = t2.k AND t1.ti IS NOT NULL
  AND (t2.ti::tbigint) <> t1.ti;
SELECT COUNT(*) FROM tbl_tbigint_seq t1, tbl_th3index_seq t2
  WHERE t1.k = t2.k AND t1.seq IS NOT NULL
  AND (t2.seq::tbigint) <> t1.seq;
SELECT COUNT(*) FROM tbl_tbigint_seqset t1, tbl_th3index_seqset t2
  WHERE t1.k = t2.k AND t1.ss IS NOT NULL
  AND (t2.ss::tbigint) <> t1.ss;

-------------------------------------------------------------------------------
-- Send/receive: COPY BINARY round-trip preserves every row
-------------------------------------------------------------------------------

COPY tbl_th3index_inst    TO '/tmp/tbl_th3index_inst'    (FORMAT BINARY);
COPY tbl_th3index_discseq TO '/tmp/tbl_th3index_discseq' (FORMAT BINARY);
COPY tbl_th3index_seq     TO '/tmp/tbl_th3index_seq'     (FORMAT BINARY);
COPY tbl_th3index_seqset  TO '/tmp/tbl_th3index_seqset'  (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_th3index_inst_tmp;
DROP TABLE IF EXISTS tbl_th3index_discseq_tmp;
DROP TABLE IF EXISTS tbl_th3index_seq_tmp;
DROP TABLE IF EXISTS tbl_th3index_seqset_tmp;

CREATE TABLE tbl_th3index_inst_tmp    AS TABLE tbl_th3index_inst    WITH NO DATA;
CREATE TABLE tbl_th3index_discseq_tmp AS TABLE tbl_th3index_discseq WITH NO DATA;
CREATE TABLE tbl_th3index_seq_tmp     AS TABLE tbl_th3index_seq     WITH NO DATA;
CREATE TABLE tbl_th3index_seqset_tmp  AS TABLE tbl_th3index_seqset  WITH NO DATA;

COPY tbl_th3index_inst_tmp    FROM '/tmp/tbl_th3index_inst'    (FORMAT BINARY);
COPY tbl_th3index_discseq_tmp FROM '/tmp/tbl_th3index_discseq' (FORMAT BINARY);
COPY tbl_th3index_seq_tmp     FROM '/tmp/tbl_th3index_seq'     (FORMAT BINARY);
COPY tbl_th3index_seqset_tmp  FROM '/tmp/tbl_th3index_seqset'  (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_th3index_inst t1, tbl_th3index_inst_tmp t2
  WHERE t1.k = t2.k AND t1.inst IS NOT NULL AND t1.inst::tbigint <> t2.inst::tbigint;
SELECT COUNT(*) FROM tbl_th3index_discseq t1, tbl_th3index_discseq_tmp t2
  WHERE t1.k = t2.k AND t1.ti IS NOT NULL AND t1.ti::tbigint <> t2.ti::tbigint;
SELECT COUNT(*) FROM tbl_th3index_seq t1, tbl_th3index_seq_tmp t2
  WHERE t1.k = t2.k AND t1.seq IS NOT NULL AND t1.seq::tbigint <> t2.seq::tbigint;
SELECT COUNT(*) FROM tbl_th3index_seqset t1, tbl_th3index_seqset_tmp t2
  WHERE t1.k = t2.k AND t1.ss IS NOT NULL AND t1.ss::tbigint <> t2.ss::tbigint;

DROP TABLE tbl_th3index_inst_tmp;
DROP TABLE tbl_th3index_discseq_tmp;
DROP TABLE tbl_th3index_seq_tmp;
DROP TABLE tbl_th3index_seqset_tmp;

-------------------------------------------------------------------------------
-- Random generators parse-check
--
-- The random_th3index_* wrappers live in the mobilitydb_datagen extension,
-- which is not loaded by the test fixture. We cannot call them here — the
-- data-producing test of those wrappers is the fixture regeneration job.
-- This file stops at verifying the tbigint<->th3index layer.
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Cleanup
-------------------------------------------------------------------------------

DROP TABLE tbl_th3index;
DROP TABLE tbl_th3index_inst;
DROP TABLE tbl_th3index_discseq;
DROP TABLE tbl_th3index_seq;
DROP TABLE tbl_th3index_seqset;

-------------------------------------------------------------------------------
