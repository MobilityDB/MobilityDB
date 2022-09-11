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

SELECT round(extent(inst), 6) FROM tbl_tgeompoint_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeogpoint_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeompoint_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeogpoint_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeompoint_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeogpoint_seq;
SELECT round(extent(ts), 6) FROM tbl_tgeompoint_seqset;
SELECT round(extent(ts), 6) FROM tbl_tgeogpoint_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint;

SELECT round(extent(inst), 6) FROM tbl_tgeompoint3D_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeogpoint3D_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeompoint3D_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeogpoint3D_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeompoint3D_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeogpoint3D_seq;
SELECT round(extent(ts), 6) FROM tbl_tgeompoint3D_seqset;
SELECT round(extent(ts), 6) FROM tbl_tgeogpoint3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompoint_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompoint_inst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompoint_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompoint_discseq;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompoint_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompoint_seq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ts)) FROM tbl_tgeompoint_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_tgeompoint_seqset;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompoint3D_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompoint3D_inst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompoint3D_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompoint3D_discseq;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompoint3D_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompoint3D_seq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompoint3D_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompoint3D_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ts)) FROM tbl_tgeompoint3D_seqset;
SELECT numSequences(tcount(ts)) FROM tbl_tgeompoint3D_seqset;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeogpoint_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeogpoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeogpoint_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeogpoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeogpoint_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeogpoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ts)) FROM tbl_tgeogpoint_seqset;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeogpoint_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SET parallel_tuple_cost=100;
SET parallel_setup_cost=100;
SET force_parallel_mode=off;

-------------------------------------------------------------------------------
