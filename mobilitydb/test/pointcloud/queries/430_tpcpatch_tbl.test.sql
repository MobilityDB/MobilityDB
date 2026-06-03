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

-- Table-level tests for tpcpatch.

-------------------------------------------------------------------------------
-- Send / receive round-trip
-------------------------------------------------------------------------------

COPY tbl_tpcpatch TO '/tmp/tbl_tpcpatch' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tpcpatch_tmp;
CREATE TABLE tbl_tpcpatch_tmp AS TABLE tbl_tpcpatch WITH NO DATA;
COPY tbl_tpcpatch_tmp FROM '/tmp/tbl_tpcpatch' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tpcpatch_tmp;

-- asBinary / tpcpatchFromBinary round-trip — exercises the WKB
-- encoder + decoder directly (independent of COPY BINARY plumbing).
SELECT COUNT(*) FROM tbl_tpcpatch
  WHERE tpcpatchFromBinary(asBinary(temp)) <> temp;

-- asHexWKB / tpcpatchFromHexWKB round-trip.
-- Note: MFJSON is intentionally output-only for tpcpatch (the JSON
-- form is a per-instant summary that drops the per-point payload),
-- so no MFJSON round-trip test.
SELECT COUNT(*) FROM tbl_tpcpatch
  WHERE tpcpatchFromHexWKB(asHexWKB(temp)) <> temp;

-------------------------------------------------------------------------------
-- pcid uniformity
-------------------------------------------------------------------------------

SELECT bool_and(pcid(inst) = 1) FROM tbl_tpcpatch_inst;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpatch_discseq;
SELECT bool_and(pcid(seq)  = 1) FROM tbl_tpcpatch_seq;
SELECT bool_and(pcid(ss)   = 1) FROM tbl_tpcpatch_seqset;

-------------------------------------------------------------------------------
-- Subtype invariants
-------------------------------------------------------------------------------

SELECT bool_and(numInstants(inst) = 1) FROM tbl_tpcpatch_inst;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpatch_discseq;
SELECT bool_and(numInstants(seq) BETWEEN 1 AND 10) FROM tbl_tpcpatch_seq;
SELECT bool_and(numInstants(ss)  BETWEEN 1 AND 100) FROM tbl_tpcpatch_seqset;

-------------------------------------------------------------------------------
-- Per-instant patch metadata
-------------------------------------------------------------------------------

SELECT bool_and(startNumPoints(inst) BETWEEN 1 AND 10) FROM tbl_tpcpatch_inst;
SELECT bool_and(endNumPoints(inst)   BETWEEN 1 AND 10) FROM tbl_tpcpatch_inst;

-------------------------------------------------------------------------------
-- numPoints — total across every instant — and points(...) SRF
--   numPoints(inst) is just startNumPoints(inst) when inst is a TInstant.
--   For multi-instant subtypes the total must equal sum of per-instant
--   counts, and the row count of points(...) must equal numPoints(...).
-------------------------------------------------------------------------------

SELECT bool_and(numPoints(inst) = startNumPoints(inst)) FROM tbl_tpcpatch_inst;
SELECT bool_and(numPoints(temp) >= 1) FROM tbl_tpcpatch;
SELECT bool_and(numPoints(temp) =
  (SELECT COUNT(*) FROM points(temp))) FROM tbl_tpcpatch;

-- The number of distinct timestamps emitted by points(...) equals the
-- value's instant count. Avoids comparing against getTime(), which
-- excludes the boundary timestamp of an upper_inc=false sequence.
SELECT bool_and(numInstants(temp) =
  (SELECT COUNT(DISTINCT t) FROM points(temp))) FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp =  t2.temp;
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp <  t2.temp;

-------------------------------------------------------------------------------
-- Time-restriction round-trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpatch
WHERE atTime(temp, tstzspan '[2001-01-01, 2002-01-01]') IS NOT NULL;

-------------------------------------------------------------------------------
-- atTpcbox / minusTpcbox — patch-level PCBOUNDS overlap
-------------------------------------------------------------------------------

-- atTpcbox over the full datagen extent leaves every row populated.
SELECT COUNT(*) FROM tbl_tpcpatch WHERE atTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 1, 0)) IS NOT NULL;

-- atTpcbox with a pcid mismatch yields NULL (empty) for every row.
SELECT bool_and(atTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 999, 0)) IS NULL)
FROM tbl_tpcpatch;

-- minusTpcbox is the complement.
SELECT bool_and(minusTpcbox(temp,
  tpcbox_zt(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 999, 0)) IS NOT NULL)
FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- Per-instant restrictions (fine-grained) — atTpcboxFine / minusTpcboxFine,
-- atGeometry / minusGeometry. Run across the full corpus to surface any
-- rebuild-side leak or empty-instant pruning bug.
-------------------------------------------------------------------------------

-- A box that fully contains the datagen extent: every input row stays
-- populated and atTpcboxFine + minusTpcboxFine partition the patch's
-- points cleanly.
SELECT COUNT(*) FROM tbl_tpcpatch
WHERE atTpcboxFine(temp,
  tpcbox_zt(-1000, -1000, -1000, 1000, 1000, 1000,
    tstzspan '[2001-01-01, 2002-01-01]', 1, 0)) IS NOT NULL;

-- For every row, fine-restrict point counts at most equal the un-restricted
-- count.
SELECT bool_and(numPoints(atTpcboxFine(temp,
  tpcbox_zt(-1000, -1000, -1000, 1000, 1000, 1000,
    tstzspan '[2001-01-01, 2002-01-01]', 1, 0))) <= numPoints(temp))
FROM tbl_tpcpatch
WHERE atTpcboxFine(temp,
  tpcbox_zt(-1000, -1000, -1000, 1000, 1000, 1000,
    tstzspan '[2001-01-01, 2002-01-01]', 1, 0)) IS NOT NULL;

-- atGeometry over a generously-sized polygon keeps every row.
SELECT COUNT(*) FROM tbl_tpcpatch
WHERE atGeometry(temp,
  geometry 'SRID=0;POLYGON((-1000 -1000, 1000 -1000, 1000 1000, -1000 1000, -1000 -1000))'
) IS NOT NULL;

-- atGeometry shrinks the working set monotonically: surviving point
-- counts cannot exceed the original. Limited to instants because the
-- atGeometry/minusGeometry partition contract is well-defined only on
-- continuous interpolation; discrete sequences in tbl_tpcpatch reject
-- the operation.
SELECT bool_and(numPoints(atGeometry(temp, g.poly)) <= numPoints(temp))
FROM tbl_tpcpatch_inst,
     LATERAL (SELECT inst AS temp) t,
     (SELECT geometry 'SRID=0;POLYGON((-50 -50, 50 -50, 50 50, -50 50, -50 -50))' AS poly) g
WHERE atGeometry(t.temp, g.poly) IS NOT NULL;

-------------------------------------------------------------------------------
-- Spatial relationships (per-point) — eIntersects(tpcpatch, geometry)
-------------------------------------------------------------------------------

-- A polygon strictly outside the datagen extent must not intersect anything.
SELECT bool_and(NOT eIntersects(temp,
  geometry 'SRID=0;POLYGON((1000000 1000000, 1000000 1000001, 1000001 1000001, 1000001 1000000, 1000000 1000000))'))
FROM tbl_tpcpatch;

-- A polygon containing the entire datagen extent must intersect every row.
SELECT bool_and(eIntersects(temp,
  geometry 'SRID=0;POLYGON((-10000 -10000, 10000 -10000, 10000 10000, -10000 10000, -10000 -10000))'))
FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- points(tpcpatch) SRF — emits one row per (timestamp, pcpoint) pair.
-- Total emitted rows must equal sum(numPoints) over the corpus.
-------------------------------------------------------------------------------

SELECT
  (SELECT COUNT(*) FROM tbl_tpcpatch t, LATERAL points(t.temp))
  =
  (SELECT COALESCE(SUM(numPoints(temp)), 0) FROM tbl_tpcpatch);

-------------------------------------------------------------------------------
