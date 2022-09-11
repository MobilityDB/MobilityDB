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

SELECT asText(round(tcentroid(temp), 6)) FROM ( VALUES
  (NULL::tnpoint),
  ('Npoint(1, 0.5)@2000-01-01'),
  ('{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')) t(temp);
SELECT asText(round(tcentroid(temp), 6)) FROM ( VALUES
  (tnpoint 'Npoint(1, 0.5)@2000-01-01'),
  ('{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'),
  (NULL)) t(temp);
/* Errors */
SELECT asText(round(tcentroid(temp), 6)) FROM ( VALUES
  (tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'),
  ('[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')) t(temp);

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tnpoint_inst;
SELECT numInstants(wcount(inst, '1 hour')) FROM tbl_tnpoint_inst;
SELECT numInstants(tcentroid(inst)) FROM tbl_tnpoint_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(wcount(inst, '1 hour')) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tnpoint_discseq;
SELECT numInstants(wcount(ti, '1 hour')) FROM tbl_tnpoint_discseq;
SELECT numInstants(tcentroid(ti)) FROM tbl_tnpoint_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpoint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpoint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tnpoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tnpoint_seq;
SELECT numSequences(wcount(seq, '1 hour')) FROM tbl_tnpoint_seq;
SELECT numSequences(tcentroid(seq)) FROM tbl_tnpoint_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(seq, '1 hour')) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ts)) FROM tbl_tnpoint_seqset;
SELECT numSequences(wcount(ts, '1 hour')) FROM tbl_tnpoint_seqset;
SELECT numSequences(tcentroid(ts)) FROM tbl_tnpoint_seqset;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(ts, '1 hour')) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
