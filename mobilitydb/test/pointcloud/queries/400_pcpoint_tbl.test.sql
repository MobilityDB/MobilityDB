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

-- Table-level tests for the static pcpoint / pcpatch types and the
-- pcpointset / pcpatchset set types.  Every query collapses to a single
-- scalar (COUNT, MIN, MAX, bool_and, etc.) so the diff is human-readable.

-------------------------------------------------------------------------------
-- Send / receive round-trip: dump as binary, reload, compare row-by-row.
-------------------------------------------------------------------------------

COPY tbl_pcpoint TO '/tmp/tbl_pcpoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_pcpoint_tmp;
CREATE TABLE tbl_pcpoint_tmp AS TABLE tbl_pcpoint WITH NO DATA;
COPY tbl_pcpoint_tmp FROM '/tmp/tbl_pcpoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint_tmp t2 WHERE t1.k = t2.k AND t1.pt <> t2.pt;
DROP TABLE tbl_pcpoint_tmp;

COPY tbl_pcpatch TO '/tmp/tbl_pcpatch' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_pcpatch_tmp;
CREATE TABLE tbl_pcpatch_tmp AS TABLE tbl_pcpatch WITH NO DATA;
COPY tbl_pcpatch_tmp FROM '/tmp/tbl_pcpatch' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_pcpatch t1, tbl_pcpatch_tmp t2 WHERE t1.k = t2.k AND t1.pa <> t2.pa;
DROP TABLE tbl_pcpatch_tmp;

-------------------------------------------------------------------------------
-- pcid uniformity — every row in the datagen tables must use pcid = 1.
-------------------------------------------------------------------------------

SELECT bool_and(pcid(pt) = 1) FROM tbl_pcpoint;
SELECT bool_and(pcid(pa) = 1) FROM tbl_pcpatch;

-------------------------------------------------------------------------------
-- Coordinate accessors range-check.
-------------------------------------------------------------------------------

SELECT bool_and(getX(pt) BETWEEN -100 AND 100) FROM tbl_pcpoint;
SELECT bool_and(getY(pt) BETWEEN -100 AND 100) FROM tbl_pcpoint;
SELECT bool_and(getZ(pt) BETWEEN    0 AND 100) FROM tbl_pcpoint;

-------------------------------------------------------------------------------
-- Comparison operators.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt =  t2.pt;
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt <> t2.pt;
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt <  t2.pt;
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt <= t2.pt;
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt >  t2.pt;
SELECT COUNT(*) FROM tbl_pcpoint t1, tbl_pcpoint t2 WHERE t1.pt >= t2.pt;

-------------------------------------------------------------------------------
-- Set types — basic invariants.
-------------------------------------------------------------------------------

SELECT bool_and(numValues(s) BETWEEN 1 AND 10)
FROM tbl_pcpointset WHERE s IS NOT NULL;
SELECT COUNT(*) FROM tbl_pcpointset WHERE s IS NULL;

SELECT bool_and(numValues(s) BETWEEN 1 AND 10)
FROM tbl_pcpatchset WHERE s IS NOT NULL;
SELECT COUNT(*) FROM tbl_pcpatchset WHERE s IS NULL;

-------------------------------------------------------------------------------
