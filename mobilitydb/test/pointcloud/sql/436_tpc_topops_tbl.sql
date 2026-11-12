-- Table-level smoke tests for the bbox topological operators on
-- tpcpoint / tpcpatch.  Each row should self-overlap, self-contain,
-- self-equal under ~=, and not be adjacent to itself; a query box
-- spanning the entire datagen extent should contain every row.

-------------------------------------------------------------------------------
-- Self-relations: every row's bbox is reflexive under &&, @>, <@, ~=.
-------------------------------------------------------------------------------

SELECT bool_and(temp && temp) FROM tbl_tpcpoint;
SELECT bool_and(temp @> temp) FROM tbl_tpcpoint;
SELECT bool_and(temp <@ temp) FROM tbl_tpcpoint;
SELECT bool_and(temp ~= temp) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp -|- temp)) FROM tbl_tpcpoint;

SELECT bool_and(temp && temp) FROM tbl_tpcpatch;
SELECT bool_and(temp @> temp) FROM tbl_tpcpatch;
SELECT bool_and(temp <@ temp) FROM tbl_tpcpatch;
SELECT bool_and(temp ~= temp) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp -|- temp)) FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- All-encompassing query box should contain every row.
-------------------------------------------------------------------------------

SELECT bool_and(tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1) @> temp)
FROM tbl_tpcpoint;
SELECT bool_and(tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1) @> temp)
FROM tbl_tpcpatch;

-- Reverse direction: every row is contained in that big box.
SELECT bool_and(temp <@ tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1)) FROM tbl_tpcpoint;
SELECT bool_and(temp <@ tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1)) FROM tbl_tpcpatch;

-- Time-only: every row's tstzspan is contained in this wide span.
SELECT bool_and(temp <@ tstzspan '[2001-01-01, 2030-01-01]') FROM tbl_tpcpoint;
SELECT bool_and(temp <@ tstzspan '[2001-01-01, 2030-01-01]') FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
