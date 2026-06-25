-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- Ever/always and temporal comparison operators for tquadbin.
-- Cell equality is int64 bit-equality (the underlying uint64 payload), so the
-- result is deterministic. Cells: A = '480fffffffffffff' (z0 world cell),
-- B = '48427fffffffffff' (tile 3,5,4), C = '48a6227affffffff' (res 10).

-------------------------------------------------------------------------------
-- eEq / ?=
-------------------------------------------------------------------------------
SELECT quadbin '480fffffffffffff' ?= tquadbin '480fffffffffffff@2001-01-01';
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '480fffffffffffff';
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= tquadbin '480fffffffffffff@2001-01-01';
-- mid-trajectory match (ever-equal)
SELECT tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-05]' ?= quadbin '48427fffffffffff';
-- no match
SELECT tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '48427fffffffffff';

-------------------------------------------------------------------------------
-- aEq / %=
-------------------------------------------------------------------------------
SELECT tquadbin '480fffffffffffff@2001-01-01' %= quadbin '480fffffffffffff';
-- mixed trajectory — not always
SELECT tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}' %= quadbin '480fffffffffffff';
-- two identical trajectories — always equal
SELECT tquadbin '[480fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02]' %= tquadbin '[480fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02]';

-------------------------------------------------------------------------------
-- eNe / ?<> and aNe / %<>
-------------------------------------------------------------------------------
SELECT tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}' ?<> quadbin '480fffffffffffff';
SELECT tquadbin '480fffffffffffff@2001-01-01' %<> quadbin '48427fffffffffff';
-- a trajectory can't be both ever-equal AND always-not-equal
SELECT NOT (
  (tquadbin '480fffffffffffff@2001-01-01' ?= quadbin '480fffffffffffff')
  AND
  (tquadbin '480fffffffffffff@2001-01-01' %<> quadbin '480fffffffffffff'));

-------------------------------------------------------------------------------
-- temporal_teq / #= and temporal_tne / #<> (tbool result)
-------------------------------------------------------------------------------
SELECT asText(quadbin '480fffffffffffff' #= tquadbin '480fffffffffffff@2001-01-01');
SELECT asText(tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02]' #= quadbin '480fffffffffffff');
SELECT asText(tquadbin '480fffffffffffff@2001-01-01' #<> tquadbin '48427fffffffffff@2001-01-01');

-------------------------------------------------------------------------------
