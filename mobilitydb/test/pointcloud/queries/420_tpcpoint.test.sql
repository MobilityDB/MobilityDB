-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

-- Value-level tests for tpcpoint — constructors, accessors, casts,
-- comparisons. Uses sample values built inline from PC_MakePoint
-- because pgPointCloud's pcpoint_in only accepts hex-WKB (the text
-- form is "not yet implemented" upstream), making constructor-based
-- expressions the readable way to write literal-value tests.

\set inst1 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set inst2 'tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[]), ''2024-01-02''::timestamptz)'
\set inst3 'tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 3.0]::float[]), ''2024-01-03''::timestamptz)'
-- Same value as inst1 at a later timestamp.
\set inst1rep 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), ''2024-01-03''::timestamptz)'

-------------------------------------------------------------------------------
-- Ergonomic pcpoint constructors — same value as the verbose form.
-------------------------------------------------------------------------------

-- 3D form (pcid 1 is registered as a 3D schema in the test fixture).
SELECT pcpoint(1, 1.0, 1.0, 1.0)::text =
  PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])::text;
-- inline use inside tpcpoint
SELECT tpcpoint(pcpoint(1, 1.0, 1.0, 1.0), '2024-01-01'::timestamptz)::text =
  (:inst1)::text;

-------------------------------------------------------------------------------
-- Text output
-------------------------------------------------------------------------------

-- asText produces the same string as the type's own output function, and the
-- cast to text goes through it.
SELECT asText(:inst1) = format('%s', :inst1);
SELECT (:inst1)::text = asText(:inst1);
-- The array form maps over the elements.
SELECT array_length(asText(ARRAY[:inst1, :inst2]), 1);
-- The array form and the element-wise form agree: the representation must
-- not depend on pgpointcloud's uninitialized struct-tail padding.
SELECT asText(ARRAY[:inst1, :inst2]) = ARRAY[asText(:inst1), asText(:inst2)];

-------------------------------------------------------------------------------
-- pcid + subtype size
-------------------------------------------------------------------------------

SELECT pcid(:inst1);
SELECT tempBasetype(:inst1);
SELECT getValue(:inst1);
SELECT getTimestamp(:inst1);
SELECT numInstants(:inst1);
SELECT numInstants(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT numInstants(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3], 'discrete'));

-------------------------------------------------------------------------------
-- Per-dimension projection (X / Y / Z)
-------------------------------------------------------------------------------

SELECT startValue(getX(:inst1));
SELECT startValue(getY(:inst1));
SELECT startValue(getZ(:inst1));

-------------------------------------------------------------------------------
-- Time accessors on a sequence
-------------------------------------------------------------------------------

SELECT startTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT endTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT pcid(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT lowerInc(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT upperInc(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT timestamps(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT numSequences(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT startSequence(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT endSequence(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT sequenceN(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), 1);
SELECT sequences(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT segments(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));

-------------------------------------------------------------------------------
-- Cast to tgeompoint preserves XYZ + timestamps
-------------------------------------------------------------------------------

SELECT numInstants((:inst1)::tgeompoint);
SELECT numInstants((tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]))::tgeompoint);

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT (:inst1) = (:inst1);
SELECT (:inst1) = (:inst2);
SELECT (:inst1) <> (:inst2);
SELECT (:inst1) < (:inst2);

-------------------------------------------------------------------------------
-- Transformations
-------------------------------------------------------------------------------

SELECT tpcpointInst(:inst1) IS NOT NULL;
SELECT tpcpointSeq(:inst1) IS NOT NULL;
SELECT tpcpointSeqSet(:inst1) IS NOT NULL;
SELECT setInterp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), 'step') IS NOT NULL;
SELECT shiftTime(:inst1, interval '1 day') IS NOT NULL;
SELECT scaleTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '2 days')
  IS NOT NULL;
SELECT shiftScaleTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  interval '1 day', interval '2 days') IS NOT NULL;
SELECT tsample(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '1 day')
  IS NOT NULL;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

-- Value restrictions: present value → at non-null / minus non-null;
-- value set → at / minus non-null; absent value → at NULL.
SELECT atValue(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])) IS NOT NULL;
SELECT minusValue(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])) IS NOT NULL;
SELECT atValues(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  set(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]),
    PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])) IS NOT NULL;
SELECT minusValues(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  set(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])])) IS NOT NULL;
SELECT atValue(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  PC_MakePoint(1, ARRAY[9.0, 9.0, 9.0]::float[])) IS NULL;

SELECT atTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspan '[2024-01-02, 2024-01-03]') IS NOT NULL;
SELECT atTpcbox(:inst2, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NOT NULL;
SELECT minusTpcbox(:inst2, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NULL;

-- Pcid-mismatch identity: at → NULL, minus → unchanged.
SELECT atTpcbox(:inst1, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 999, 0)) IS NULL;

-- Restriction to the instants before / after a timestamp. The strict flag
-- is true by default and excludes the instant at the timestamp itself, which
-- on a sequence shows up as the inclusivity of the resulting time span.
SELECT timeSpan(beforeTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02'));
SELECT timeSpan(beforeTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02', false));
SELECT timeSpan(afterTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02'));
SELECT timeSpan(afterTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02', false));
-- An instant is dropped by the strict form at its own timestamp and kept by
-- the non-strict one.
SELECT beforeTimestamp(:inst2, timestamptz '2024-01-02') IS NULL;
SELECT afterTimestamp(:inst2, timestamptz '2024-01-02') IS NULL;
SELECT numInstants(beforeTimestamp(:inst2, timestamptz '2024-01-02', false));
SELECT numInstants(afterTimestamp(:inst2, timestamptz '2024-01-02', false));

-------------------------------------------------------------------------------
-- Modifications
-- insert/update take temporal values that share the same value at their
-- common timestamp; append adds an instant/sequence after the end.
-------------------------------------------------------------------------------

SELECT insert(tpcpointSeq(ARRAY[:inst1, :inst2]),
  tpcpointSeq(ARRAY[:inst2, :inst3])) IS NOT NULL;
SELECT insert(tpcpointSeq(ARRAY[:inst1, :inst2]),
  tpcpointSeq(ARRAY[:inst2, :inst3]), false) IS NOT NULL;
SELECT update(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tpcpointSeq(ARRAY[:inst2])) IS NOT NULL;
SELECT update(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tpcpointSeq(ARRAY[:inst2, :inst3])) IS NOT NULL;

SELECT deleteTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02') IS NOT NULL;
SELECT deleteTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzset '{2024-01-02}') IS NOT NULL;
SELECT deleteTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspan '[2024-01-02, 2024-01-02]') IS NOT NULL;
SELECT deleteTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspanset '{[2024-01-02, 2024-01-02]}') IS NOT NULL;

SELECT appendInstant(tpcpointSeq(ARRAY[:inst1, :inst2]), :inst3) IS NOT NULL;
SELECT appendSequence(tpcpointSeq(ARRAY[:inst1]),
  tpcpointSeq(ARRAY[:inst2, :inst3])) IS NOT NULL;

-------------------------------------------------------------------------------
-- Unnest
-- One row per distinct value, with the span set on which the temporal value
-- takes it. A value occurring twice yields one row whose span set has two
-- spans, so the row count is the number of DISTINCT values.
-------------------------------------------------------------------------------

SELECT count(*) FROM unnest(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT count(*) FROM unnest(tpcpointSeq(ARRAY[:inst1, :inst2, :inst1rep]));
SELECT numSpans((u).time) FROM
  unnest(tpcpointSeq(ARRAY[:inst1, :inst2, :inst1rep])) u
  WHERE (u).value::text = (pcpoint(1, 1.0, 1.0, 1.0))::text;
-- Every span set is contained in the time span of the temporal value.
SELECT bool_and((u).time <@ timeSpan(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3])))
  FROM unnest(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3])) u;

-------------------------------------------------------------------------------
-- Multidimensional tiling
-- timeSplit cuts a temporal value into the fragments falling in each time bin.
-------------------------------------------------------------------------------

SELECT count(*) FROM
  timeSplit(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '1 day');
SELECT count(*) FROM
  timeSplit(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '1 week');
-- The bins start at the origin, so shifting it shifts the fragments.
SELECT count(*) FROM timeSplit(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  interval '2 days', timestamptz '2024-01-01');
-- Each fragment starts at or after the bin it is labelled with, and stays
-- inside the time span of the value it was cut from.
SELECT bool_and(time <= startTimestamp(temp))
  FROM timeSplit(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '1 day');
SELECT bool_and(timeSpan(temp) <@
    timeSpan(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3])))
  FROM timeSplit(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]), interval '1 day');

-------------------------------------------------------------------------------
