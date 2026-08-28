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

-- The temporal S2 cell index type: input and output, the constructors, the
-- accessors, the restrictions, the conversions to and from tbigint, and the
-- bounding box.
--
-- A cell literal here is a token the type accepts; `47c3c3` is the level-10
-- cell over Brussels and `54b5c9` a level-10 cell over San Francisco, so the
-- two lie on different cube faces and a value spanning them spans the globe.

-------------------------------------------------------------------------------
-- Input and output
-------------------------------------------------------------------------------

SELECT asText(ts2cell '47c3c3@2000-01-01');
SELECT asText(ts2cell '{47c3c3@2000-01-01, 54b5c9@2000-01-02}');
SELECT asText(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT asText(ts2cell '{[47c3c3@2000-01-01, 54b5c9@2000-01-02],[47c3c3@2000-01-03]}');

-------------------------------------------------------------------------------
-- Interpolation
--
-- A cell index is discrete, so a sequence is step interpolated and asking for
-- a linear one is refused.
-------------------------------------------------------------------------------

SELECT interp(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT interp(ts2cell '{47c3c3@2000-01-01, 54b5c9@2000-01-02}');
SELECT asText(ts2cell 'Interp=Step;[47c3c3@2000-01-01, 54b5c9@2000-01-02]');

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(ts2cell(s2cell '47c3c3', timestamptz '2000-01-01'));
SELECT asText(ts2cellInst(ts2cell '[47c3c3@2000-01-01]'));
SELECT asText(ts2cellSeq(ARRAY[ts2cell '47c3c3@2000-01-01', ts2cell '54b5c9@2000-01-02']));
SELECT asText(ts2cell(s2cell '47c3c3', tstzspan '[2000-01-01, 2000-01-02]'));

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT tempSubtype(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT tempBasetype(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT startValue(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT endValue(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT getValues(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT numInstants(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');
SELECT valueAtTimestamp(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]', '2000-01-01 12:00');
SELECT duration(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]');

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT asText(atValue(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]', s2cell '47c3c3'));
SELECT asText(minusValue(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]', s2cell '47c3c3'));
SELECT asText(atTime(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]', tstzspan '[2000-01-01, 2000-01-01 12:00]'));

-------------------------------------------------------------------------------
-- Conversions
--
-- The int64 payload survives the round trip through tbigint, and the cast is
-- ASSIGNMENT so it must be spelled out.
-------------------------------------------------------------------------------

SELECT asText(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]'::tbigint);
SELECT asText(ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]'::tbigint::ts2cell);
SELECT ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]'::tbigint::ts2cell
  = ts2cell '[47c3c3@2000-01-01, 54b5c9@2000-01-02]';

-------------------------------------------------------------------------------
-- Comparisons
--
-- A ts2cell is a spatiotemporal type, so `temporal_cmp` reads the bounding
-- box before the cell identifier and only falls through to the identifier
-- where the boxes are equal. Two cells on different cube faces are therefore
-- ordered by their extents, which need not agree with the order the static
-- s2cell comparison gives them, and the tquadbin sibling behaves the same
-- way.
-------------------------------------------------------------------------------

SELECT ts2cell '[47c3c3@2000-01-01]' = ts2cell '[47c3c3@2000-01-01]';
SELECT ts2cell '[47c3c3@2000-01-01]' <> ts2cell '[54b5c9@2000-01-01]';
SELECT cmp(ts2cell '[47c3c3@2000-01-01]', ts2cell '[47c3c3@2000-01-01]');
SELECT s2cell '47c3c3' < s2cell '54b5c9';
SELECT cmp(ts2cell '[47c3c3@2000-01-01]', ts2cell '[54b5c9@2000-01-01]');

-------------------------------------------------------------------------------
