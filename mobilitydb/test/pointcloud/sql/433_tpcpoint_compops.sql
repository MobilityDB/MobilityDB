-- Ever/always and temporal comparison operators for tpcpoint. Values are built
-- with the ergonomic pcpoint constructor because pgPointCloud's pcpoint_in
-- accepts hex-WKB only, so a literal is not readable here. Equality on a
-- pcpoint is byte equality of the serialized point, so the result is
-- deterministic. Points: A = (1,1,1), B = (2,2,2).

\set pA 'pcpoint(1, 1.0, 1.0, 1.0)'
\set pB 'pcpoint(1, 2.0, 2.0, 2.0)'
\set iA 'tpcpoint(:pA, ''2001-01-01''::timestamptz)'
\set iB 'tpcpoint(:pB, ''2001-01-02''::timestamptz)'
\set seqAB 'tpcpointSeq(ARRAY[:iA, :iB])'
\set seqAA 'tpcpointSeq(ARRAY[:iA, tpcpoint(:pA, ''2001-01-02''::timestamptz)])'

-------------------------------------------------------------------------------
-- eEqual / ?=
-------------------------------------------------------------------------------

SELECT (:pA) ?= (:iA);
SELECT (:iA) ?= (:pA);
SELECT (:iA) ?= (:iA);
-- mid-trajectory match (ever equal)
SELECT (:seqAB) ?= (:pB);
-- no match
SELECT (:iA) ?= (:pB);

-------------------------------------------------------------------------------
-- aEqual / %=
-------------------------------------------------------------------------------

SELECT (:iA) %= (:pA);
-- mixed trajectory, not always equal
SELECT (:seqAB) %= (:pA);
-- constant trajectory, always equal
SELECT (:seqAA) %= (:pA);
SELECT (:seqAA) %= (:seqAA);

-------------------------------------------------------------------------------
-- eNotEqual / ?<> and aNotEqual / %<>
-------------------------------------------------------------------------------

SELECT (:pA) ?<> (:seqAB);
SELECT (:seqAB) ?<> (:pA);
SELECT (:iA) %<> (:pB);
SELECT (:seqAB) ?<> (:seqAA);
-- a value cannot be both ever equal and always different
SELECT NOT ((:iA) ?= (:pA) AND (:iA) %<> (:pA));

-------------------------------------------------------------------------------
-- tEqual / #= and tNotEqual / #<>
-------------------------------------------------------------------------------

SELECT asText((:pA) #= (:iA));
SELECT asText((:iA) #= (:pA));
SELECT asText((:seqAB) #= (:pA));
SELECT asText((:iA) #<> (:pB));
SELECT asText((:seqAB) #<> (:seqAA));

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a point cloud box

SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) ?&& tpcbox 'TPCBOX(XT(((8,0),(10,2)),[2024-01-01 00:00, 2024-01-01 10:00]), 1)';
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) && tpcbox 'TPCBOX(XT(((8,0),(10,2)),[2024-01-01 00:00, 2024-01-01 10:00]), 1)';
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) ?&& tpcbox 'TPCBOX(XT(((4,4),(6,6)),[2024-01-01 04:00, 2024-01-01 06:00]), 1)';
SELECT tpcbox 'TPCBOX(XT(((4,4),(6,6)),[2024-01-01 04:00, 2024-01-01 06:00]), 1)' ?&& tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]);
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) %&& tpcbox 'TPCBOX(X((0,0),(10,10)), 1)';
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) %&& tpcbox 'TPCBOX(X((0,0),(9,9)), 1)';
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) ?&& tpcbox 'TPCBOX(T([2024-01-02, 2024-01-03]), 1)';
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2
WHERE t1.temp ?&& tpcbox(t2.temp) <> (atTpcbox(t1.temp, tpcbox(t2.temp)) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2
WHERE t1.temp %&& tpcbox(t2.temp) <> (minusTpcbox(t1.temp, tpcbox(t2.temp)) IS NULL);
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2 WHERE t1.temp ?&& tpcbox(t2.temp);
SELECT COUNT(*) FROM tbl_tpcpoint t1, tbl_tpcpoint t2 WHERE t1.temp %&& tpcbox(t2.temp);
/* Errors */
SELECT tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), '2024-01-01 00:00'::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), '2024-01-01 10:00'::timestamptz)]) ?&& tpcbox 'TPCBOX(X((0,0),(10,10)), 999)';
