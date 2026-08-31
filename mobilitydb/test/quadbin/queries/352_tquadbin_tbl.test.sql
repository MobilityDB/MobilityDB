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

-- Table-based tests for the tquadbin type, exercised over tbl_tquadbin.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_tquadbin TO '/tmp/tbl_tquadbin' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tquadbin_tmp;
CREATE TABLE tbl_tquadbin_tmp AS TABLE tbl_tquadbin WITH NO DATA;
COPY tbl_tquadbin_tmp FROM '/tmp/tbl_tquadbin' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tquadbin_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numInstants(temp)) FROM tbl_tquadbin;
SELECT COUNT(*) FROM tbl_tquadbin WHERE startValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE endValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE getTime(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE duration(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE startInstant(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE endInstant(temp) IS NOT NULL;
SELECT SUM(numTimestamps(temp)) FROM tbl_tquadbin;
SELECT COUNT(*) FROM tbl_tquadbin WHERE memSize(temp) > 0;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin WHERE atTime(temp, getTime(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tquadbin WHERE minusTime(temp, getTime(temp)) IS NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE t1.temp <> t2.temp;

-------------------------------------------------------------------------------
