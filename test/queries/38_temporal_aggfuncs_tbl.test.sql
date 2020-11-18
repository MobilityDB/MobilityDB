/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

-------------------------------------------------------------------------------

SET parallel_tuple_cost=0;
SET parallel_setup_cost=0;
SET force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT extent(inst) FROM tbl_tboolinst;
SELECT extent(inst) FROM tbl_ttextinst;
SELECT extent(inst) FROM tbl_tintinst;
SELECT setprecision(extent(inst), 13) FROM tbl_tfloatinst;

SELECT extent(ti) FROM tbl_tbooli;
SELECT extent(ti) FROM tbl_ttexti;
SELECT extent(ti) FROM tbl_tinti;
SELECT setprecision(extent(ti), 13) FROM tbl_tfloati;

SELECT extent(seq) FROM tbl_tboolseq;
SELECT extent(seq) FROM tbl_ttextseq;
SELECT extent(seq) FROM tbl_tintseq;
SELECT setprecision(extent(seq), 13) FROM tbl_tfloatseq;

SELECT extent(ts) FROM tbl_tbools;
SELECT extent(ts) FROM tbl_ttexts;
SELECT extent(ts) FROM tbl_tints;
SELECT setprecision(extent(ts), 13) FROM tbl_tfloats;

SELECT extent(temp) FROM tbl_tbool;
SELECT extent(temp) FROM tbl_ttext;
SELECT extent(temp) FROM tbl_tint;
SELECT setprecision(extent(temp), 13) FROM tbl_tfloat;

-------------------------------------------------------------------------------
-- TemporalInst aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(inst)) FROM tbl_tboolinst;
SELECT numInstants(tor(inst)) FROM tbl_tboolinst;
SELECT numInstants(tcount(inst)) FROM tbl_tboolinst;

SELECT numInstants(tmin(inst)) FROM tbl_tintinst;
SELECT numInstants(tmax(inst)) FROM tbl_tintinst;
SELECT numInstants(tcount(inst)) FROM tbl_tintinst;
SELECT numInstants(tsum(inst)) FROM tbl_tintinst;
SELECT numInstants(tavg(inst)) FROM tbl_tintinst;

SELECT numInstants(tmin(inst)) FROM tbl_tfloatinst;
SELECT numInstants(tmax(inst)) FROM tbl_tfloatinst;
SELECT numInstants(tcount(inst)) FROM tbl_tfloatinst;
SELECT numInstants(tsum(inst)) FROM tbl_tfloatinst;
SELECT numInstants(tavg(inst)) FROM tbl_tfloatinst;

SELECT numInstants(tmin(inst)) FROM tbl_ttextinst;
SELECT numInstants(tmax(inst)) FROM tbl_ttextinst;
SELECT numInstants(tcount(inst)) FROM tbl_ttextinst;

SELECT k%10, numInstants(tand(inst)) FROM tbl_tboolinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(inst)) FROM tbl_tboolinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tboolinst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_tintinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_tintinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tintinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(inst)) FROM tbl_tintinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(inst)) FROM tbl_tintinst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_tfloatinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_tfloatinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tfloatinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(inst)) FROM tbl_tfloatinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(inst)) FROM tbl_tfloatinst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(inst)) FROM tbl_ttextinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(inst)) FROM tbl_ttextinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_ttextinst GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalI aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(ti)) FROM tbl_tbooli;
SELECT numInstants(tor(ti)) FROM tbl_tbooli;
SELECT numInstants(tcount(ti)) FROM tbl_tbooli;

SELECT numInstants(tmin(ti)) FROM tbl_tinti;
SELECT numInstants(tmax(ti)) FROM tbl_tinti;
SELECT numInstants(tcount(ti)) FROM tbl_tinti;
SELECT numInstants(tsum(ti)) FROM tbl_tinti;
SELECT numInstants(tavg(ti)) FROM tbl_tinti;

SELECT numInstants(tmin(ti)) FROM tbl_tfloati;
SELECT numInstants(tmax(ti)) FROM tbl_tfloati;
SELECT numInstants(tcount(ti)) FROM tbl_tfloati;
SELECT numInstants(tsum(ti)) FROM tbl_tfloati;
SELECT numInstants(tavg(ti)) FROM tbl_tfloati;

SELECT numInstants(tmin(ti)) FROM tbl_ttexti;
SELECT numInstants(tmax(ti)) FROM tbl_ttexti;
SELECT numInstants(tcount(ti)) FROM tbl_ttexti;

SELECT k%10, numInstants(tand(ti)) FROM tbl_tbooli GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(ti)) FROM tbl_tbooli GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tbooli GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_tinti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_tinti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tinti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(ti)) FROM tbl_tinti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(ti)) FROM tbl_tinti GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_tfloati GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_tfloati GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tfloati GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(ti)) FROM tbl_tfloati GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(ti)) FROM tbl_tfloati GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(ti)) FROM tbl_ttexti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(ti)) FROM tbl_ttexti GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_ttexti GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalSeq aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(seq)) FROM tbl_tboolseq;
SELECT numSequences(tor(seq)) FROM tbl_tboolseq;
SELECT numSequences(tcount(seq)) FROM tbl_tboolseq;

SELECT numSequences(tmin(seq)) FROM tbl_tintseq;
SELECT numSequences(tmax(seq)) FROM tbl_tintseq;
SELECT numSequences(tcount(seq)) FROM tbl_tintseq;
SELECT numSequences(tsum(seq)) FROM tbl_tintseq;
SELECT numSequences(tavg(seq)) FROM tbl_tintseq;

SELECT numSequences(tmin(seq)) FROM tbl_tfloatseq;
SELECT numSequences(tmax(seq)) FROM tbl_tfloatseq;
SELECT numSequences(tcount(seq)) FROM tbl_tfloatseq;
SELECT numSequences(tsum(seq)) FROM tbl_tfloatseq;
SELECT numSequences(tavg(seq)) FROM tbl_tfloatseq;

SELECT numSequences(tmin(seq)) FROM tbl_ttextseq;
SELECT numSequences(tmax(seq)) FROM tbl_ttextseq;
SELECT numSequences(tcount(seq)) FROM tbl_ttextseq;

SELECT k%10, numSequences(tand(seq)) FROM tbl_tboolseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(seq)) FROM tbl_tboolseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tboolseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_tintseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_tintseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tintseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(seq)) FROM tbl_tintseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(seq)) FROM tbl_tintseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_tfloatseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_tfloatseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tfloatseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(seq)) FROM tbl_tfloatseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(seq)) FROM tbl_tfloatseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(seq)) FROM tbl_ttextseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(seq)) FROM tbl_ttextseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_ttextseq GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- TemporalS aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(ts)) FROM tbl_tbools;
SELECT numSequences(tor(ts)) FROM tbl_tbools;
SELECT numSequences(tcount(ts)) FROM tbl_tbools;

SELECT numSequences(tmin(ts)) FROM tbl_tints;
SELECT numSequences(tmax(ts)) FROM tbl_tints;
SELECT numSequences(tcount(ts)) FROM tbl_tints;
SELECT numSequences(tsum(ts)) FROM tbl_tints;
SELECT numSequences(tavg(ts)) FROM tbl_tints;

SELECT numSequences(tmin(ts)) FROM tbl_tfloats;
SELECT numSequences(tmax(ts)) FROM tbl_tfloats;
SELECT numSequences(tcount(ts)) FROM tbl_tfloats;
SELECT numSequences(tsum(ts)) FROM tbl_tfloats;
SELECT numSequences(tavg(ts)) FROM tbl_tfloats;

SELECT numSequences(tmin(ts)) FROM tbl_ttexts;
SELECT numSequences(tmax(ts)) FROM tbl_ttexts;
SELECT numSequences(tcount(ts)) FROM tbl_ttexts;

SELECT k%10, numSequences(tand(ts)) FROM tbl_tbools GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(ts)) FROM tbl_tbools GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tbools GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_tints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_tints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(ts)) FROM tbl_tints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(ts)) FROM tbl_tints GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_tfloats GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_tfloats GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tfloats GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(ts)) FROM tbl_tfloats GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(ts)) FROM tbl_tfloats GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(ts)) FROM tbl_ttexts GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(ts)) FROM tbl_ttexts GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_ttexts GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SET parallel_tuple_cost=100;
SET parallel_setup_cost=100;
SET force_parallel_mode=off;

-------------------------------------------------------------------------------

