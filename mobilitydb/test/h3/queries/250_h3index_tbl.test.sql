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

-- Table-based tests for the static h3index type, exercised over tbl_h3index.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_h3index TO '/tmp/tbl_h3index' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_h3index_tmp;
CREATE TABLE tbl_h3index_tmp AS TABLE tbl_h3index WITH NO DATA;
COPY tbl_h3index_tmp FROM '/tmp/tbl_h3index' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index_tmp t2 WHERE t1.k = t2.k AND t1.h3 <> t2.h3;
DROP TABLE tbl_h3index_tmp;

-------------------------------------------------------------------------------
-- (Hex)WKB round trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE h3indexFromBinary(asBinary(h3)) <> h3;
SELECT COUNT(*) FROM tbl_h3index WHERE h3indexFromHexWKB(asHexWKB(h3)) <> h3;

-------------------------------------------------------------------------------
-- Validity predicates
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE NOT isValidCell(h3);

-------------------------------------------------------------------------------
-- Casts to / from bigint (assignment)
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE (h3::bigint)::h3index <> h3;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 = t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 <> t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 < t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 <= t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 > t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 >= t2.h3;

-------------------------------------------------------------------------------
