-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

-- Table-level tests for tpcpatch.

-- Note: WKB-output dispatch for tpcpatch is not hooked up yet, so
-- COPY BINARY round-trip (which the other temporal types exercise)
-- is not yet possible.

-------------------------------------------------------------------------------
-- pcid uniformity
-------------------------------------------------------------------------------

SELECT bool_and(pcid(inst) = 1) FROM tbl_tpcpatch_inst;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpatch_discseq;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpatch_seq;
SELECT bool_and(pcid(ss)   = 1) FROM tbl_tpcpatch_seqset;

-------------------------------------------------------------------------------
-- Subtype invariants
-------------------------------------------------------------------------------

SELECT bool_and(numInstants(inst) = 1) FROM tbl_tpcpatch_inst;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpatch_discseq;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpatch_seq;
SELECT bool_and(numInstants(ss)  BETWEEN 1 AND 100) FROM tbl_tpcpatch_seqset;

-------------------------------------------------------------------------------
-- Per-instant patch metadata
-------------------------------------------------------------------------------

SELECT bool_and(startNumPoints(inst) BETWEEN 1 AND 10) FROM tbl_tpcpatch_inst;

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp =  t2.temp;
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp <  t2.temp;

-------------------------------------------------------------------------------
-- Time-restriction round-trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpatch
WHERE atTime(temp, tstzspan '[2001-01-01, 2002-01-01]') IS NOT NULL;

-------------------------------------------------------------------------------
