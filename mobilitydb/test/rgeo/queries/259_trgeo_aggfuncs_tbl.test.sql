-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- tcount — 2D
-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_trgeometry2d_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_trgeometry2d_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(seq)) FROM tbl_trgeometry2d_discseq;
SELECT k%10, numInstants(tcount(seq)) FROM tbl_trgeometry2d_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_trgeometry2d_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_trgeometry2d_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_trgeometry2d_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_trgeometry2d_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- tcount — 3D
-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_trgeometry3d_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_trgeometry3d_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(seq)) FROM tbl_trgeometry3d_discseq;
SELECT k%10, numInstants(tcount(seq)) FROM tbl_trgeometry3d_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_trgeometry3d_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_trgeometry3d_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_trgeometry3d_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_trgeometry3d_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- extent — 2D
-------------------------------------------------------------------------------

SELECT round(extent(inst), 6) FROM tbl_trgeometry2d_inst;
SELECT round(extent(seq), 6)  FROM tbl_trgeometry2d_discseq;
SELECT round(extent(seq), 6)  FROM tbl_trgeometry2d_seq;
SELECT round(extent(ss), 6)   FROM tbl_trgeometry2d_seqset;

-------------------------------------------------------------------------------
-- extent — 3D
-------------------------------------------------------------------------------

SELECT round(extent(inst), 6) FROM tbl_trgeometry3d_inst;
SELECT round(extent(seq), 6)  FROM tbl_trgeometry3d_discseq;
SELECT round(extent(seq), 6)  FROM tbl_trgeometry3d_seq;
SELECT round(extent(ss), 6)   FROM tbl_trgeometry3d_seqset;

-------------------------------------------------------------------------------
