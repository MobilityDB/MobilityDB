-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

-- Table-based tests for the quadbinset type, exercised over tbl_quadbinset.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_quadbinset TO '/tmp/tbl_quadbinset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_quadbinset_tmp;
CREATE TABLE tbl_quadbinset_tmp AS TABLE tbl_quadbinset WITH NO DATA;
COPY tbl_quadbinset_tmp FROM '/tmp/tbl_quadbinset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_quadbinset_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numValues(s)) FROM tbl_quadbinset;
SELECT COUNT(*) FROM tbl_quadbinset WHERE memSize(s) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_quadbinset t1, tbl_quadbinset t2 WHERE t1.s <> t2.s;

SELECT MAX(hash(s)) FROM tbl_quadbinset;

SELECT MAX(hashExtended(s, 1)) FROM tbl_quadbinset;

-------------------------------------------------------------------------------
