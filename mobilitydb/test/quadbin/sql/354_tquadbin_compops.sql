-- Ever/always and temporal comparison operators for tquadbin.
-- Cell equality is int64 bit-equality (the underlying uint64 payload), so the
-- result is deterministic. Cells: A = '480fffffffffffff' (z0 world cell),
-- B = '48427fffffffffff' (tile 3,5,4), C = '48a6227affffffff' (res 10).

-------------------------------------------------------------------------------
-- eEqual / ?=
-------------------------------------------------------------------------------
SELECT quadbin '480fffffffffffff' ?= tquadbin '480fffffffffffff@2001-01-01';
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '480fffffffffffff';
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= tquadbin '480fffffffffffff@2001-01-01';
-- mid-trajectory match (ever-equal)
SELECT tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-05]' ?= quadbin '48427fffffffffff';
-- no match
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '48427fffffffffff';

-------------------------------------------------------------------------------
-- aEqual / %=
-------------------------------------------------------------------------------
SELECT tquadbin '480fffffffffffff@2001-01-01' %= quadbin '480fffffffffffff';
-- mixed trajectory — not always
SELECT tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}' %= quadbin '480fffffffffffff';
-- two identical trajectories — always equal
SELECT tquadbin '[480fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02]' %= tquadbin '[480fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02]';

-------------------------------------------------------------------------------
-- eNotEqual / ?<> and aNotEqual / %<>
-------------------------------------------------------------------------------
SELECT tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}' ?<> quadbin '480fffffffffffff';
SELECT tquadbin '480fffffffffffff@2001-01-01' %<> quadbin '48427fffffffffff';
-- a trajectory can't be both ever-equal AND always-not-equal
SELECT NOT (
  (tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '480fffffffffffff')
  AND
  (tquadbin '480fffffffffffff@2001-01-01' %<> quadbin '480fffffffffffff'));

-------------------------------------------------------------------------------
-- tEqual / #= and tNotEqual / #<> (tbool result)
-------------------------------------------------------------------------------
SELECT asText(quadbin '480fffffffffffff' #= tquadbin '480fffffffffffff@2001-01-01');
SELECT asText(tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02]' #= quadbin '480fffffffffffff');
SELECT asText(tquadbin '480fffffffffffff@2001-01-01' #<> tquadbin '48427fffffffffff@2001-01-01');

-------------------------------------------------------------------------------
