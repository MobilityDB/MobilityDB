-- Table-level tests for tpcpoint (the four tbl_tpcpoint_* tables — instant,
-- discrete sequence, continuous sequence, sequence set — plus the merged
-- tbl_tpcpoint).

-------------------------------------------------------------------------------
-- Send / receive round-trip
-------------------------------------------------------------------------------

\copy tbl_tpcpoint TO 'tbl_tpcpoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tpcpoint_tmp;
CREATE TABLE tbl_tpcpoint_tmp AS TABLE tbl_tpcpoint WITH NO DATA;
\copy tbl_tpcpoint_tmp FROM 'tbl_tpcpoint' (FORMAT BINARY);
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
  tpcboxZT(-100, -100, 0, 100, 100, 100,
    tstzspan '[2001-01-01, 2001-12-31]', 1)) IS NOT NULL;

-- atTpcbox with a box of another schema raises, as the overlaps of their
-- boxes raises.
SELECT bool_and(atTpcbox(temp,
  tpcbox
    'TPCBOX(ZT(((-100,-100,0),(100,100,100)),[2001-01-01, 2001-12-31]), 999)')
  IS NULL)
FROM tbl_tpcpoint;

-- minusTpcbox raises as atTpcbox does.
SELECT bool_and(minusTpcbox(temp,
  tpcbox
    'TPCBOX(ZT(((-100,-100,0),(100,100,100)),[2001-01-01, 2001-12-31]), 999)')
  IS NOT NULL)
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The cast to tpcbox is the box the extent of each value answers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint WHERE temp::tpcbox IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpcpoint WHERE tpcbox(temp) <> extentTransition(NULL::tpcbox, temp);
