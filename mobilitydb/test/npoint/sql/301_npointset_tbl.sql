-------------------------------------------------------------------------------
-- Tests for the set data type
-- File set.c
-------------------------------------------------------------------------------

-- Send/receive functions

\copy tbl_npointset TO 'tbl_npointset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_npointset_tmp;
CREATE TEMP TABLE tbl_npointset_tmp AS TABLE tbl_npointset WITH NO DATA;
\copy tbl_npointset_tmp FROM 'tbl_npointset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset_tmp t2 WHERE t1.k = t2.k AND t1.n <> t2.n;
DROP TABLE tbl_npointset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromText(asText(n)) <> n;
SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromEWKT(asText(n)) <> n;

SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromBinary(asBinary(n)) <> n;
SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromEWKB(asEWKB(n)) <> n;
SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromHexWKB(asHexWKB(n)) <> n;
SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromHexWKB(asHexEWKB(n)) <> n;
SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromHexEWKB(asHexEWKB(n)) <> n;

-------------------------------------------------------------------------------
-- Constructor

SELECT numValues(set(array_agg(DISTINCT np ORDER BY np))) FROM tbl_npoint WHERE np IS NOT NULL;

-------------------------------------------------------------------------------
-- Cast

SELECT round(npointset '{"Npoint(1,0.5)", "Npoint(2,0.5)", "Npoint(3,0.5)"}'::stbox, 6);

SELECT COUNT(*) FROM tbl_npoint WHERE np::npointset IS NOT NULL;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(getPosition(startValue(round(n, 3)))) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(n)) FROM tbl_npointset;

SELECT MIN(numValues(n)) FROM tbl_npointset;
SELECT MIN(getPosition(startValue(n))) FROM tbl_npointset;
SELECT MIN(getPosition(endValue(n))) FROM tbl_npointset;
SELECT MIN(getPosition(valueN(n, 1))) FROM tbl_npointset;
SELECT MIN(array_length(getValues(n), 1)) FROM tbl_npointset;

SELECT MIN(numValues(routes(n))) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(setUnion(np)) FROM tbl_npoint;

WITH test1(k, n) AS (
  SELECT k, unnest(n) FROM tbl_npointset ),
test2 (k, n) AS (
  SELECT k, setUnion(n) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_npointset t2 WHERE t1.k = t2.k AND t1.n <> t2.n;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE cmp(t1.n, t2.n) = -1;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n = t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n <> t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n < t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n <= t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n > t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n >= t2.n;

SELECT MAX(hash(n)) FROM tbl_npointset;

SELECT MAX(hashExtended(n, 1)) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(setUnion(np)) FROM tbl_npoint;

-------------------------------------------------------------------------------
-- The value-and-set directions of the difference and the intersection return
-- a set, which is what the functions behind them build
-------------------------------------------------------------------------------

SELECT setMinus(npoint 'Npoint(1,0.5)', npointset '{"Npoint(2,0.5)"}');
SELECT setIntersection(npoint 'Npoint(1,0.5)', npointset '{"Npoint(1,0.5)", "Npoint(2,0.5)"}');
SELECT setIntersection(npointset '{"Npoint(1,0.5)", "Npoint(2,0.5)"}', npoint 'Npoint(1,0.5)');

-------------------------------------------------------------------------------
