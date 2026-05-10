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

-- Table-level tests for tpcpoint (the four tbl_tpcpoint_* tables — instant,
-- discrete sequence, continuous sequence, sequence set — plus the merged
-- tbl_tpcpoint).

-------------------------------------------------------------------------------
-- Send / receive round-trip
-------------------------------------------------------------------------------

COPY tbl_tpcpoint TO '/tmp/tbl_tpcpoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tpcpoint_tmp;
CREATE TABLE tbl_tpcpoint_tmp AS TABLE tbl_tpcpoint WITH NO DATA;
COPY tbl_tpcpoint_tmp FROM '/tmp/tbl_tpcpoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tpcpoint_tmp;

-- asBinary / tpcpointFromBinary round-trip — exercises the WKB
-- encoder + decoder directly (independent of COPY BINARY plumbing).
SELECT COUNT(*) FROM tbl_tpcpoint
  WHERE tpcpointFromBinary(asBinary(temp)) <> temp;

-- asHexWKB / tpcpointFromHexWKB round-trip.
-- Note: MFJSON is intentionally output-only for tpcpoint (the JSON
-- form does not carry the schema), so no MFJSON round-trip test.
SELECT COUNT(*) FROM tbl_tpcpoint
  WHERE tpcpointFromHexWKB(asHexWKB(temp)) <> temp;

-------------------------------------------------------------------------------
-- pcid uniformity across each subtype
-------------------------------------------------------------------------------

SELECT bool_and(pcid(inst) = 1) FROM tbl_tpcpoint_inst;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpoint_discseq;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpoint_seq;
SELECT bool_and(pcid(ss)   = 1) FROM tbl_tpcpoint_seqset;

-------------------------------------------------------------------------------
-- Subtype invariants
-------------------------------------------------------------------------------

SELECT bool_and(numInstants(inst) = 1) FROM tbl_tpcpoint_inst;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpoint_discseq;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpoint_seq;
SELECT bool_and(numInstants(ss)  BETWEEN 1 AND 100) FROM tbl_tpcpoint_seqset;

-------------------------------------------------------------------------------
-- Per-dimension projections — every dimension must be a tfloat that
-- never escapes the datagen range.
-------------------------------------------------------------------------------

SELECT bool_and(startValue(getX(inst)) BETWEEN -100 AND 100) FROM tbl_tpcpoint_inst;
SELECT bool_and(startValue(getY(inst)) BETWEEN -100 AND 100) FROM tbl_tpcpoint_inst;
SELECT bool_and(startValue(getZ(inst)) BETWEEN    0 AND 100) FROM tbl_tpcpoint_inst;

-------------------------------------------------------------------------------
-- Cast tpcpoint → tgeompoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint_inst WHERE inst::tgeompoint IS NOT NULL;
SELECT bool_and(numInstants(inst::tgeompoint) = numInstants(inst))
FROM tbl_tpcpoint_inst;

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2 WHERE t1.temp =  t2.temp;
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2 WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2 WHERE t1.temp <  t2.temp;

-------------------------------------------------------------------------------
-- Time-restriction round-trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint
WHERE atTime(temp, tstzspan '[2001-01-01, 2002-01-01]') IS NOT NULL;

-------------------------------------------------------------------------------
-- atTpcbox / minusTpcbox — restrict by spatiotemporal bounding box
-------------------------------------------------------------------------------

-- atTpcbox over the full datagen extent leaves every row populated.
SELECT COUNT(*) FROM tbl_tpcpoint WHERE atTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 1, 0)) IS NOT NULL;

-- atTpcbox with a pcid mismatch yields NULL (empty) for every row.
SELECT bool_and(atTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 999, 0)) IS NULL)
FROM tbl_tpcpoint;

-- minusTpcbox is the complement.
SELECT bool_and(minusTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 999, 0)) IS NOT NULL)
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
