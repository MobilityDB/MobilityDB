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
-- Temporal instant
-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wmax(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wsum(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wcount(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wavg(inst, interval '5 minutes')) FROM tbl_tint_inst;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wmax(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wsum(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wcount(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wavg(inst, interval '5 minutes')) FROM tbl_tfloat_inst;

-------------------------------------------------------------------------------
-- Temporal discrete sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wmax(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wsum(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wcount(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wavg(ti, interval '5 minutes')) FROM tbl_tint_discseq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wmax(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wsum(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wcount(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wavg(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;

-------------------------------------------------------------------------------
-- Temporal continuous sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wmax(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wsum(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wcount(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wavg(seq, interval '5 minutes')) FROM tbl_tint_seq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes')) FROM tbl_tfloat_seq;
SELECT numSequences(wmax(seq, interval '5 minutes')) FROM tbl_tfloat_seq;
SELECT numSequences(wcount(seq, interval '5 minutes')) FROM tbl_tfloat_seq;

-------------------------------------------------------------------------------
-- Temporal sequence sets
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wmax(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wsum(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wcount(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wavg(ss, interval '5 minutes')) FROM tbl_tint_seqset;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wmax(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wcount(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;

-------------------------------------------------------------------------------
