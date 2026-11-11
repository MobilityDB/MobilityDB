-- Ever/always and temporal comparison operators for tpcpatch. Values are built
-- with the ergonomic pcpatch constructor because pgPointCloud's pcpatch_in
-- accepts hex-WKB only, so a literal is not readable here. Equality on a
-- pcpatch is byte equality of the serialized patch, so the result is
-- deterministic. Patches: A holds (1,1,1) and (2,2,2), B holds (5,5,5) and
-- (6,6,6).

\set pA 'pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))'
\set pB 'pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))'
\set iA 'tpcpatch(:pA, ''2001-01-01''::timestamptz)'
\set iB 'tpcpatch(:pB, ''2001-01-02''::timestamptz)'
\set seqAB 'tpcpatchSeq(ARRAY[:iA, :iB])'
\set seqAA 'tpcpatchSeq(ARRAY[:iA, tpcpatch(:pA, ''2001-01-02''::timestamptz)])'

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

SELECT tpcpatch(PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])]), '2024-01-01'::timestamptz) ?&& tpcbox 'TPCBOX(XT(((0,0),(5,5)),[2024-01-01, 2024-01-02]), 1)';
SELECT tpcbox 'TPCBOX(XT(((3,3),(5,5)),[2024-01-01, 2024-01-02]), 1)' ?&& tpcpatch(PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])]), '2024-01-01'::timestamptz);
SELECT tpcpatch(PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])]), '2024-01-01'::timestamptz) %&& tpcbox 'TPCBOX(X((0,0),(5,5)), 1)';
SELECT tpcpatch(PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])]), '2024-01-01'::timestamptz) ?&& tpcbox 'TPCBOX(T([2024-01-02, 2024-01-03]), 1)';
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2
WHERE t1.temp ?&& tpcbox(t2.temp) <> (atTpcbox(t1.temp, tpcbox(t2.temp)) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2
WHERE t1.temp %&& tpcbox(t2.temp) <> (minusTpcbox(t1.temp, tpcbox(t2.temp)) IS NULL);
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp ?&& tpcbox(t2.temp);
SELECT COUNT(*) FROM tbl_tpcpatch t1, tbl_tpcpatch t2 WHERE t1.temp %&& tpcbox(t2.temp);
/* Errors */
SELECT tpcpatch(PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])]), '2024-01-01'::timestamptz) ?&& tpcbox 'TPCBOX(X((0,0),(5,5)), 999)';
