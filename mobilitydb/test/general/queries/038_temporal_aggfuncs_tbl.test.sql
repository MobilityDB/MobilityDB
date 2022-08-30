-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

SET parallel_tuple_cost=0;
SET parallel_setup_cost=0;
SET force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT extent(inst) FROM tbl_tbool_inst;
SELECT extent(inst) FROM tbl_ttext_inst;
SELECT extent(inst) FROM tbl_tint_inst;
SELECT round(extent(inst), 13) FROM tbl_tfloat_inst;

SELECT extent(ti) FROM tbl_tbool_discseq;
SELECT extent(ti) FROM tbl_ttext_discseq;
SELECT extent(ti) FROM tbl_tint_discseq;
SELECT round(extent(ti), 13) FROM tbl_tfloat_discseq;

SELECT extent(seq) FROM tbl_tbool_seq;
SELECT extent(seq) FROM tbl_ttext_seq;
SELECT extent(seq) FROM tbl_tint_seq;
SELECT round(extent(seq), 13) FROM tbl_tfloat_seq;

SELECT extent(ts) FROM tbl_tbool_seqset;
SELECT extent(ts) FROM tbl_ttext_seqset;
SELECT extent(ts) FROM tbl_tint_seqset;
SELECT round(extent(ts), 13) FROM tbl_tfloat_seqset;

SELECT extent(temp) FROM tbl_tbool;
SELECT extent(temp) FROM tbl_ttext;
SELECT extent(temp) FROM tbl_tint;
SELECT round(extent(temp), 13) FROM tbl_tfloat;

-------------------------------------------------------------------------------
-- TemporalInst aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(inst)) FROM tbl_tbool_inst;
SELECT numInstants(tor(inst)) FROM tbl_tbool_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tbool_inst;

SELECT numInstants(tmin(inst)) FROM tbl_tint_inst;
SELECT numInstants(tmax(inst)) FROM tbl_tint_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tint_inst;
SELECT numInstants(tsum(inst)) FROM tbl_tint_inst;
SELECT numInstants(tavg(inst)) FROM tbl_tint_inst;

SELECT numInstants(tmin(inst)) FROM tbl_tfloat_inst;
SELECT numInstants(tmax(inst)) FROM tbl_tfloat_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tfloat_inst;
SELECT numInstants(tsum(inst)) FROM tbl_tfloat_inst;
SELECT numInstants(tavg(inst)) FROM tbl_tfloat_inst;

SELECT numInstants(tmin(inst)) FROM tbl_ttext_inst;
SELECT numInstants(tmax(inst)) FROM tbl_ttext_inst;
SELECT numInstants(tcount(inst)) FROM tbl_ttext_inst;

SELECT k%10, numInstants(tand(inst)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(inst)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(inst)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(inst)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(inst)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(inst)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalI aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(ti)) FROM tbl_tbool_discseq;
SELECT numInstants(tor(ti)) FROM tbl_tbool_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tbool_discseq;

SELECT numInstants(tmin(ti)) FROM tbl_tint_discseq;
SELECT numInstants(tmax(ti)) FROM tbl_tint_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tint_discseq;
SELECT numInstants(tsum(ti)) FROM tbl_tint_discseq;
SELECT numInstants(tavg(ti)) FROM tbl_tint_discseq;

SELECT numInstants(tmin(ti)) FROM tbl_tfloat_discseq;
SELECT numInstants(tmax(ti)) FROM tbl_tfloat_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tfloat_discseq;
SELECT numInstants(tsum(ti)) FROM tbl_tfloat_discseq;
SELECT numInstants(tavg(ti)) FROM tbl_tfloat_discseq;

SELECT numInstants(tmin(ti)) FROM tbl_ttext_discseq;
SELECT numInstants(tmax(ti)) FROM tbl_ttext_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_ttext_discseq;

SELECT k%10, numInstants(tand(ti)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(ti)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(ti)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(ti)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(ti)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(ti)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalSeq aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(seq)) FROM tbl_tbool_seq;
SELECT numSequences(tor(seq)) FROM tbl_tbool_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tbool_seq;

SELECT numSequences(tmin(seq)) FROM tbl_tint_seq;
SELECT numSequences(tmax(seq)) FROM tbl_tint_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tint_seq;
SELECT numSequences(tsum(seq)) FROM tbl_tint_seq;
SELECT numSequences(tavg(seq)) FROM tbl_tint_seq;

SELECT numSequences(tmin(seq)) FROM tbl_tfloat_seq;
SELECT numSequences(tmax(seq)) FROM tbl_tfloat_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tfloat_seq;
SELECT numSequences(tsum(seq)) FROM tbl_tfloat_seq;
SELECT numSequences(tavg(seq)) FROM tbl_tfloat_seq;

SELECT numSequences(tmin(seq)) FROM tbl_ttext_seq;
SELECT numSequences(tmax(seq)) FROM tbl_ttext_seq;
SELECT numSequences(tcount(seq)) FROM tbl_ttext_seq;

SELECT k%10, numSequences(tand(seq)) FROM tbl_tbool_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(seq)) FROM tbl_tbool_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tbool_seq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_tint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_tint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(seq)) FROM tbl_tint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(seq)) FROM tbl_tint_seq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_tfloat_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_tfloat_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tfloat_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(seq)) FROM tbl_tfloat_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(seq)) FROM tbl_tfloat_seq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_ttext_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_ttext_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_ttext_seq GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalS aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(ts)) FROM tbl_tbool_seqset;
SELECT numSequences(tor(ts)) FROM tbl_tbool_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_tbool_seqset;

SELECT numSequences(tmin(ts)) FROM tbl_tint_seqset;
SELECT numSequences(tmax(ts)) FROM tbl_tint_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_tint_seqset;
SELECT numSequences(tsum(ts)) FROM tbl_tint_seqset;
SELECT numSequences(tavg(ts)) FROM tbl_tint_seqset;

SELECT numSequences(tmin(ts)) FROM tbl_tfloat_seqset;
SELECT numSequences(tmax(ts)) FROM tbl_tfloat_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_tfloat_seqset;
SELECT numSequences(tsum(ts)) FROM tbl_tfloat_seqset;
SELECT numSequences(tavg(ts)) FROM tbl_tfloat_seqset;

SELECT numSequences(tmin(ts)) FROM tbl_ttext_seqset;
SELECT numSequences(tmax(ts)) FROM tbl_ttext_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_ttext_seqset;

SELECT k%10, numSequences(tand(ts)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(ts)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(ts)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(ts)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(ts)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(ts)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SET parallel_tuple_cost=100;
SET parallel_setup_cost=100;
SET force_parallel_mode=off;

-------------------------------------------------------------------------------

