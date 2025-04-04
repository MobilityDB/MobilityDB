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

-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT round(extent(inst), 6) FROM tbl_tgeometry_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeography_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeometry_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeography_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeometry_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeography_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeometry_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeography_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry;
SELECT round(extent(temp), 6) FROM tbl_tgeography;

SELECT round(extent(inst), 6) FROM tbl_tgeometry3D_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeography3D_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeometry3D_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeography3D_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeometry3D_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeography3D_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeometry3D_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeography3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3D;
SELECT round(extent(temp), 6) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeometry_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeometry_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeometry_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeometry_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeometry_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeometry_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeometry_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeometry_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(inst)) FROM tbl_tgeometry3D_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeometry3D_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeometry3D_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeometry3D_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeometry3D_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeometry3D_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeometry3D_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeometry3D_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeography_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeography_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeography_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeography_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeography_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeography_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeography_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeography_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
