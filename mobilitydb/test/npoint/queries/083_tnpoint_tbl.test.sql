﻿-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

--------------------------------------------------------------------------------
-- Send/receive functions
--------------------------------------------------------------------------------

COPY tbl_tnpoint TO '/tmp/tbl_tnpoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tnpoint_tmp;
CREATE TABLE tbl_tnpoint_tmp AS TABLE tbl_tnpoint WITH NO DATA;
COPY tbl_tnpoint_tmp FROM '/tmp/tbl_tnpoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tnpoint_tmp;

-------------------------------------------------------------------------------
--  Constructors
-------------------------------------------------------------------------------

SELECT MAX(getPosition(startValue(tnpoint(t1.np, t2.t)))) FROM tbl_npoint t1, tbl_timestamptz t2;

WITH test(temp) as (
SELECT tnpoint_discseq(array_agg(t.inst ORDER BY getTimestamp(t.inst))) FROM tbl_tnpoint_inst t GROUP BY k%10 )
SELECT MAX(getPosition(startValue(temp))) FROM test;

WITH test(temp) as (
SELECT tnpoint_contseq(array_agg(t.inst ORDER BY getTimestamp(t.inst))) FROM tbl_tnpoint_inst t GROUP BY route(t.inst) )
SELECT MAX(getPosition(startValue(temp))) FROM test;

WITH test(temp) as (
SELECT tnpoint_seqset(array_agg(t.seq ORDER BY startTimestamp(t.seq))) FROM tbl_tnpoint_seq t GROUP BY k%10 )
SELECT MAX(getPosition(startValue(temp))) FROM test;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tnpointinst_test;
CREATE TABLE tbl_tnpointinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tnpoint_seq;
WITH temp AS (
  SELECT numSequences(tnpoint_seqset_gaps(array_agg(inst ORDER BY getTime(inst)), true, 5.0, '5 minutes'::interval))
  FROM tbl_tnpointinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tnpointinst_test;

-------------------------------------------------------------------------------
--  Transformation functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(inst)) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(tnpoint_discseq(inst)) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(tnpoint_contseq(inst)) FROM tbl_tnpoint_inst;
SELECT DISTINCT tempSubtype(tnpoint_seqset(inst)) FROM tbl_tnpoint_inst;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(ti)) FROM tbl_tnpoint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tnpoint_discseq(ti)) FROM tbl_tnpoint_discseq;
SELECT DISTINCT tempSubtype(tnpoint_contseq(ti)) FROM tbl_tnpoint_discseq WHERE numInstants(ti) = 1;
SELECT DISTINCT tempSubtype(tnpoint_seqset(ti)) FROM tbl_tnpoint_discseq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(seq)) FROM tbl_tnpoint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tnpoint_discseq(seq)) FROM tbl_tnpoint_seq WHERE numInstants(seq) = 1;
SELECT DISTINCT tempSubtype(tnpoint_contseq(seq)) FROM tbl_tnpoint_seq;
SELECT DISTINCT tempSubtype(tnpoint_seqset(seq)) FROM tbl_tnpoint_seq;

-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(tnpoint(ts)) FROM tbl_tnpoint_seqset WHERE numInstants(ts) = 1;
SELECT DISTINCT tempSubtype(tnpoint_discseq(ts)) FROM tbl_tnpoint_seqset WHERE duration(ts) = '00:00:00';
SELECT DISTINCT tempSubtype(tnpoint_contseq(ts)) FROM tbl_tnpoint_seqset WHERE numSequences(ts) = 1;
SELECT DISTINCT tempSubtype(tnpoint_seqset(ts)) FROM tbl_tnpoint_seqset;

-------------------------------------------------------------------------------
--  Append functions
-------------------------------------------------------------------------------

SELECT MAX(numInstants(appendInstant(temp, shift(endInstant(temp), '5 min')))) FROM tbl_tnpoint;

-------------------------------------------------------------------------------
--  Cast functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint WHERE temp::tgeompoint IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint WHERE round(temp, 7) = round((temp::tgeompoint)::tnpoint, 7);

-------------------------------------------------------------------------------
--  Accessor functions
-------------------------------------------------------------------------------

SELECT DISTINCT tempSubtype(temp) FROM tbl_tnpoint ORDER BY 1;

SELECT MAX(memorySize(temp)) FROM tbl_tnpoint;

/*
SELECT stbox(temp) FROM tbl_tnpoint;
*/

SELECT getValue(inst) FROM tbl_tnpoint_inst ORDER BY getValue(inst) LIMIT 1;

SELECT MAX(array_length(getValues(temp), 1)) FROM tbl_tnpoint;

SELECT MAX(array_length(positions(temp), 1)) FROM tbl_tnpoint;

SELECT MAX(route(inst)) FROM tbl_tnpoint_inst;

SELECT MAX(numValues(routes(temp))) FROM tbl_tnpoint;

SELECT MAX(timespan(getTime(temp))) FROM tbl_tnpoint;

SELECT MAX(getTimestamp(inst)) FROM tbl_tnpoint_inst;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp ?= t2.np;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE t1.temp %= t2.np;

SELECT COUNT(*) FROM tbl_tnpoint_inst t1, tbl_npoint t2 WHERE ever_eq(t1.inst, t2.np);

SELECT COUNT(*) FROM tbl_tnpoint_inst t1, tbl_npoint t2 WHERE always_eq(t1.inst, t2.np);

SELECT MAX(startTimestamp(shift(t1.temp, t2.i))) FROM tbl_tnpoint t1, tbl_interval t2;

SELECT DISTINCT MAX(getPosition(startValue(temp))) FROM tbl_tnpoint;

SELECT DISTINCT MAX(getPosition(endValue(temp))) FROM tbl_tnpoint;

SELECT MAX(timespan(temp)) FROM tbl_tnpoint;

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

SELECT MAX(numSequences(ts)) FROM tbl_tnpoint_seqset;

SELECT MAX(timespan(startSequence(ts))) FROM tbl_tnpoint_seqset;

SELECT MAX(timespan(endSequence(ts))) FROM tbl_tnpoint_seqset;

SELECT MAX(timespan(sequenceN(ts, numSequences(ts)))) FROM tbl_tnpoint_seqset;

SELECT MAX(array_length(sequences(ts),1)) FROM tbl_tnpoint_seqset;

-------------------------------------------------------------------------------
--  Restriction functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint
WHERE atValue(temp, np) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_npoint
WHERE minusValue(temp, np) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint,
( SELECT array_agg(np) AS valuearr FROM tbl_npoint) tmp
WHERE atValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint,
( SELECT array_agg(np) AS valuearr FROM tbl_npoint) tmp
WHERE minusValues(temp, valuearr) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE atTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE minusTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE valueAtTimestamp(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset
WHERE atTime(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset
WHERE minusTime(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_period
WHERE atTime(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_period
WHERE minusTime(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset
WHERE atTime(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset
WHERE minusTime(temp, ps) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestamptz
WHERE overlapsTime(temp, t) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_timestampset
WHERE overlapsTime(temp, ts) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_period
WHERE overlapsTime(temp, p) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tnpoint, tbl_periodset
WHERE overlapsTime(temp, ps) IS NOT NULL;

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
SELECT MAX(tnpoint_hash(temp)) FROM tbl_tnpoint;

-------------------------------------------------------------------------------
-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tnpoint_rtree_idx ON tbl_tnpoint USING gist(temp);

-- EXPLAIN ANALYZE
-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?= 'NPoint(1, 0.1)';

-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp %= 'NPoint(1, 0.1)';

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, timestamptz '2001-06-01');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, timestampset '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, period '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, periodset '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tnpoint_rtree_idx;

-------------------------------------------------------------------------------
-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tnpoint_quadtree_idx ON tbl_tnpoint USING spgist(temp);

-- EXPLAIN ANALYZE
-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp ?= 'NPoint(1, 0.1)';

-- SELECT COUNT(*) FROM tbl_tnpoint WHERE temp %= 'NPoint(1, 0.1)';

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, timestamptz '2001-06-01');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, timestampset '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, period '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tnpoint WHERE overlapsTime(temp, periodset '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tnpoint_quadtree_idx;

-------------------------------------------------------------------------------
