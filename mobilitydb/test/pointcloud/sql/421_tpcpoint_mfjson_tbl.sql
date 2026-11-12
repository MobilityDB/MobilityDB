-- MF-JSON output: every row returns a non-empty JSON string with the
-- expected "type" key.

SELECT bool_and(asMFJSON(temp) LIKE '%"type":"MovingPCPoint"%')
FROM tbl_tpcpoint;

SELECT bool_and(asMFJSON(temp) LIKE '%"type":"MovingPCPatch"%')
FROM tbl_tpcpatch;

-- Bbox embedding: with options=1 the output includes a "bbox" key and
-- the matching pcid (= 1 for our datagen schema).
SELECT bool_and(asMFJSON(temp, 1) LIKE '%"pcid":1%' AND
                asMFJSON(temp, 1) LIKE '%"bbox":%')
FROM tbl_tpcpoint;

-- Hand-roll a JSON parse via regex extraction: pull the first
-- coordinate triple out and verify it's three numbers.
SELECT bool_and(
  substring(asMFJSON(inst) FROM '"coordinates":\[\[[^\]]+\]') ~
    '^"coordinates":\[\[-?[0-9.]+,-?[0-9.]+(,-?[0-9.]+)?\]$')
FROM tbl_tpcpoint_inst;

-- tpcpatch instants emit pcid + npoints + bounds.
SELECT bool_and(asMFJSON(inst) LIKE '%"pcid":1%' AND
                asMFJSON(inst) LIKE '%"npoints":%' AND
                asMFJSON(inst) LIKE '%"bounds":[%')
FROM tbl_tpcpatch_inst;

-------------------------------------------------------------------------------
-- MF-JSON round trip: every row reads back equal to itself
SELECT COUNT(*), COUNT(*) FILTER (WHERE tpcpointFromMFJSON(asMFJSON(temp)) = temp)
FROM tbl_tpcpoint;
SELECT COUNT(*), COUNT(*) FILTER (WHERE tpcpatchFromMFJSON(asMFJSON(temp)) = temp)
FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
