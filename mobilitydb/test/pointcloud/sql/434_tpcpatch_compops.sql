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
