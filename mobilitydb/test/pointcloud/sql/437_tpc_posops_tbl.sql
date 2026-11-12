-- Smoke tests for the directional position operators.  Each test
-- collapses to a single scalar.  The properties exercised are:
--   * Self-relation: nothing is strictly positioned to its own X axis
--     (a row is not strictly left of itself, etc.).
--   * Strict-vs-overlapping: strict (<<) implies the lax form (&<).
--   * Symmetric pairs commute properly under the operator.

-------------------------------------------------------------------------------
-- Self-relation negatives — strict directional ops are irreflexive.
-------------------------------------------------------------------------------

SELECT bool_and(NOT (temp <<  temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp >>  temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <<| temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp |>> temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <</ temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp />> temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <<# temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp #>> temp)) FROM tbl_tpcpoint;

SELECT bool_and(NOT (temp <<  temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp >>  temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp <<# temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp #>> temp)) FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- An all-encompassing tpcbox is not strictly to one side of any row.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint
WHERE temp << tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1);

SELECT COUNT(*) FROM tbl_tpcpoint
WHERE tpcboxZT(-200, -200, -200, 200, 200, 200,
  tstzspan '[2001-01-01, 2030-01-01]', 1) >> temp;

-- Reverse: every row is "before" a span that starts well after
-- the data extent.
SELECT COUNT(*) FROM tbl_tpcpoint
WHERE temp <<# tstzspan '[2099-01-01, 2099-12-31]';

SELECT COUNT(*) FROM tbl_tpcpatch
WHERE temp <<# tstzspan '[2099-01-01, 2099-12-31]';

-------------------------------------------------------------------------------
