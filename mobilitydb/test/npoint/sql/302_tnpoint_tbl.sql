--------------------------------------------------------------------------------
-- Send/receive functions
--------------------------------------------------------------------------------

-- This test creates its indexes on copies of the tables of the setup scripts,
-- which hide those tables in its session: every test reads them, and an
-- index on them would change the plans of the other tests.
CREATE TEMP TABLE tbl_tnpoint AS TABLE tbl_tnpoint;
ANALYZE tbl_tnpoint;

\copy tbl_tnpoint TO 'tbl_tnpoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tnpoint_tmp;
CREATE TEMP TABLE tbl_tnpoint_tmp AS TABLE tbl_tnpoint WITH NO DATA;
\copy tbl_tnpoint_tmp FROM 'tbl_tnpoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tnpoint_tmp;

-------------------------------------------------------------------------------
--  Constructors
-------------------------------------------------------------------------------

SELECT MAX(getPosition(startValue(tnpoint(t1.np, t2.t)))) FROM tbl_npoint t1, tbl_timestamptz t2;

WITH test(temp) AS (
SELECT tnpointSeq(array_agg(t.temp ORDER BY getTimestamp(t.temp)), 'discrete') FROM tbl_tnpoint_inst t GROUP BY k%10 )
SELECT MAX(getPosition(startValue(temp))) FROM test;

WITH test(temp) AS (
SELECT tnpointSeq(array_agg(t.temp ORDER BY getTimestamp(t.temp))) FROM tbl_tnpoint_inst t GROUP BY route(t.temp) )
SELECT MAX(getPosition(startValue(temp))) FROM test;

WITH test(temp) AS (
SELECT tnpointSeqSet(array_agg(t.temp ORDER BY startTimestamp(t.temp))) FROM tbl_tnpoint_contseq t GROUP BY k%10 )
SELECT MAX(getPosition(startValue(temp))) FROM test;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tnpointinst_test;
CREATE TEMP TABLE tbl_tnpointinst_test AS SELECT k, unnest(instants(temp)) AS inst FROM tbl_tnpoint_contseq;
WITH temp AS (
  SELECT numSequences(tnpointSeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0))
  FROM tbl_tnpointinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tnpointinst_test;

-------------------------------------------------------------------------------
--  Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(temp)) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(tnpointSeq(temp)) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(tnpointSeqSet(temp)) FROM tbl_tnpoint_inst;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(temp)) FROM tbl_tnpoint_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tnpoint_discseq;
SELECT DISTINCT tempSubtype(tnpointSeq(temp)) FROM tbl_tnpoint_discseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tnpointSeqSet(temp)) FROM tbl_tnpoint_discseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(temp)) FROM tbl_tnpoint_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tnpoint_contseq WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(tnpointSeq(temp)) FROM tbl_tnpoint_contseq;
SELECT DISTINCT tempSubtype(tnpointSeqSet(temp)) FROM tbl_tnpoint_contseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(temp)) FROM tbl_tnpoint_seqset WHERE numInstants(temp) = 1;
SELECT DISTINCT tempSubtype(setInterp(temp, 'discrete')) FROM tbl_tnpoint_seqset WHERE duration(temp) = '00:00:00';
SELECT DISTINCT tempSubtype(tnpointSeq(temp)) FROM tbl_tnpoint_seqset WHERE numSequences(temp) = 1;
SELECT DISTINCT tempSubtype(tnpointSeqSet(temp)) FROM tbl_tnpoint_seqset;

-------------------------------------------------------------------------------
--  Append functions
-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tnpoint 'Npoint(1, 0.1)@2001-01-01' UNION
  SELECT tnpoint 'Npoint(1, 0.2)@2001-01-02' UNION
  SELECT tnpoint 'Npoint(1, 0.4)@2001-01-04' UNION
  SELECT tnpoint 'Npoint(1, 0.5)@2001-01-05' UNION
  SELECT tnpoint 'Npoint(1, 0.7)@2001-01-07' )
SELECT appendInstant(inst, NULL, 1, NULL ORDER BY inst) FROM temp;

SELECT MAX(numInstants(appendInstant(temp, shiftTime(endInstant(temp), '5 min')))) FROM tbl_tnpoint;

-------------------------------------------------------------------------------
--  Cast functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint WHERE temp::tgeompoint IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint WHERE round(temp, 7) = round((temp::tgeompoint)::tnpoint, 7);

-------------------------------------------------------------------------------
--  Accessor functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tnpoint ORDER BY 1;

SELECT MAX(memSize(temp)) FROM tbl_tnpoint;

/*
SELECT stbox(temp) FROM tbl_tnpoint;
*/

SELECT getValue(temp) FROM tbl_tnpoint_inst ORDER BY getValue(temp) LIMIT 1;

SELECT MAX(numValues(getValues(temp))) FROM tbl_tnpoint;

SELECT MAX(array_length(positions(temp), 1)) FROM tbl_tnpoint;

SELECT MAX(route(temp)) FROM tbl_tnpoint_inst;

SELECT MAX(numValues(routes(temp))) FROM tbl_tnpoint;

SELECT MAX(duration(getTime(temp))) FROM tbl_tnpoint;

SELECT MAX(getTimestamp(temp)) FROM tbl_tnpoint_inst;


SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp ?= t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE t1.np ?= t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ?= t2.temp;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp %= t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE t1.np %= t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp %= t2.temp;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp ?<> t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE t1.np ?<> t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp ?<> t2.temp;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp %<> t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE t1.np %<> t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp %<> t2.temp;

SELECT COUNT(*) FROM tbl_tnpoint_inst t1, tbl_npoint t2 WHERE eEqual(t1.temp, t2.np);
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint_inst t2 WHERE eEqual(t1.np, t2.temp);

SELECT COUNT(*) FROM tbl_tnpoint_inst t1, tbl_npoint t2 WHERE aEqual(t1.temp, t2.np);
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint_inst t2 WHERE aEqual(t1.np, t2.temp);

SELECT MAX(startTimestamp(shiftTime(t1.temp, t2.i))) FROM tbl_tnpoint t1, tbl_interval t2;

SELECT DISTINCT MAX(getPosition(startValue(temp))) FROM tbl_tnpoint;

SELECT DISTINCT MAX(getPosition(endValue(temp))) FROM tbl_tnpoint;

SELECT MAX(duration(temp)) FROM tbl_tnpoint;

SELECT MAX(numInstants(temp)) FROM tbl_tnpoint;

SELECT MAX(Route(startInstant(temp))) FROM tbl_tnpoint;

SELECT MAX(Route(endInstant(temp))) FROM tbl_tnpoint;

SELECT MAX(Route(instantN(temp, 1))) FROM tbl_tnpoint;

SELECT MAX(array_length(instants(temp),1)) FROM tbl_tnpoint;

SELECT MAX(numTimestamps(temp)) FROM tbl_tnpoint;

SELECT MAX(startTimestamp(temp)) FROM tbl_tnpoint;

SELECT MAX(endTimestamp(temp)) FROM tbl_tnpoint;

SELECT MAX(timestampN(temp,1)) FROM tbl_tnpoint;

SELECT MAX(array_length(timestamps(temp),1)) FROM tbl_tnpoint;

SELECT MAX(numSequences(temp)) FROM tbl_tnpoint_seqset;

SELECT MAX(duration(startSequence(temp))) FROM tbl_tnpoint_seqset;

SELECT MAX(duration(endSequence(temp))) FROM tbl_tnpoint_seqset;

SELECT MAX(duration(sequenceN(temp, numSequences(temp)))) FROM tbl_tnpoint_seqset;

SELECT MAX(array_length(sequences(temp),1)) FROM tbl_tnpoint_seqset;

-------------------------------------------------------------------------------
--  Restriction functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint
WHERE atValue(temp, np) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint
WHERE minusValue(temp, np) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint,
( SELECT setUnion(np) AS s FROM tbl_npoint) tmp
WHERE atValues(temp, s) IS NOT NULL;
-- Restricting to a set answers what restricting to its elements one by one
-- answers, so the two counts are the same number
SELECT COUNT(*) FROM tbl_tnpoint t
WHERE EXISTS (SELECT 1 FROM tbl_npoint n WHERE atValue(t.temp, n.np) IS NOT NULL);

SELECT COUNT(*) FROM tbl_tnpoint,
( SELECT setUnion(np) AS s FROM tbl_npoint) tmp
WHERE minusValues(temp, s) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE minusTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset
WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzset
WHERE minusTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan
WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspan
WHERE minusTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset
WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_tstzspanset
WHERE minusTime(temp, t) IS NOT NULL;

-------------------------------------------------------------------------------
-- Stops function
-------------------------------------------------------------------------------

SELECT MAX(numInstants(stops(temp, 50.0))) FROM tbl_tnpoint_contseq;
SELECT MAX(numInstants(stops(temp, 50.0, '5 min'))) FROM tbl_tnpoint_contseq;
-- The path of a network point follows its route between two instants, so a
-- stop holds every point of the path within the distance, not only its instants
SELECT COUNT(*) FROM (SELECT unnest(sequences(stops(temp, 50.0))) AS st
  FROM tbl_tnpoint_contseq) s
WHERE ST_MaxDistance(trajectory(st::tgeompoint), trajectory(st::tgeompoint)) > 50.0;

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

-- Update calls the insert function after calling the minusTime function
-- SELECT SUM(numInstants(update(t1.temp, t2.temp))) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.k < t2.k;

-------------------------------------------------------------------------------
--  Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp <> t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp < t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp <= t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp > t2.temp;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2
WHERE t1.temp >= t2.temp;

-------------------------------------------------------------------------------
--  Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

-- This test currently shows different result on github
SELECT MAX(hash(temp)) FROM tbl_tnpoint;

-------------------------------------------------------------------------------
-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING gist(temp);

-- EXPLAIN ANALYZE
-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?= 'NPoint(1, 0.1)';

-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp %= 'NPoint(1, 0.1)';

DROP INDEX tbl_tnpoint_rtree_idx;

-------------------------------------------------------------------------------
-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING spgist(temp);

-- EXPLAIN ANALYZE
-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?= 'NPoint(1, 0.1)';

-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp %= 'NPoint(1, 0.1)';

DROP INDEX tbl_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------
