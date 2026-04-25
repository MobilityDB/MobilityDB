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

-- Table-level tests for tpcbox.

-------------------------------------------------------------------------------
-- Send / receive round-trip
-------------------------------------------------------------------------------

COPY tbl_tpcbox TO '/tmp/tbl_tpcbox' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tpcbox_tmp;
CREATE TABLE tbl_tpcbox_tmp AS TABLE tbl_tpcbox WITH NO DATA;
COPY tbl_tpcbox_tmp FROM '/tmp/tbl_tpcbox' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tpcbox t1, tbl_tpcbox_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tpcbox_tmp;

-------------------------------------------------------------------------------
-- pcid uniformity + dimension flags
-------------------------------------------------------------------------------

SELECT bool_and(pcid(b) = 1) FROM tbl_tpcbox;
SELECT bool_and(hasX(b)) FROM tbl_tpcbox;
SELECT bool_and(hasZ(b)) FROM tbl_tpcbox;
SELECT bool_and(hasT(b)) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- Range checks for the bounds
-------------------------------------------------------------------------------

SELECT bool_and(xmin(b) <= xmax(b)) FROM tbl_tpcbox;
SELECT bool_and(ymin(b) <= ymax(b)) FROM tbl_tpcbox;
SELECT bool_and(zmin(b) <= zmax(b)) FROM tbl_tpcbox;
SELECT bool_and(tmin(b) <= tmax(b)) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- Set operations preserve pcid
-------------------------------------------------------------------------------

SELECT bool_and(pcid(t1.b + t2.b) = 1)
FROM tbl_tpcbox t1, tbl_tpcbox t2;

SELECT COUNT(*)
FROM tbl_tpcbox t1, tbl_tpcbox t2
WHERE (t1.b * t2.b) IS NOT NULL;

-------------------------------------------------------------------------------
-- Topological predicates self-relate sensibly
-------------------------------------------------------------------------------

-- Every box equals itself.
SELECT bool_and(b ~= b) FROM tbl_tpcbox;
-- Every box overlaps itself.
SELECT bool_and(b && b) FROM tbl_tpcbox;
-- Every box contains itself.
SELECT bool_and(b @> b) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcbox t1, tbl_tpcbox t2 WHERE t1.b =  t2.b;
SELECT COUNT(*) FROM tbl_tpcbox t1, tbl_tpcbox t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tpcbox t1, tbl_tpcbox t2 WHERE t1.b <  t2.b;
SELECT COUNT(*) FROM tbl_tpcbox t1, tbl_tpcbox t2 WHERE t1.b <= t2.b;

-------------------------------------------------------------------------------
