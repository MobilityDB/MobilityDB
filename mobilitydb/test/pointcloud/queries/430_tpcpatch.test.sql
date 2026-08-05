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
-------------------------------------------------------------------------------

-- Value-level tests for tpcpatch — same constructor-function pattern
-- as 420_tpcpoint.test.sql.

\set patch1 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set patch2 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[5.0, 5.0, 5.0]::float[]), PC_MakePoint(1, ARRAY[6.0, 6.0, 6.0]::float[])])'

\set inst1 'tpcpatch(:patch1, ''2024-01-01''::timestamptz)'
\set inst2 'tpcpatch(:patch2, ''2024-01-02''::timestamptz)'
\set inst3 'tpcpatch(:patch1, ''2024-01-03''::timestamptz)'
\set inst4 'tpcpatch(:patch2, ''2024-01-04''::timestamptz)'

-------------------------------------------------------------------------------
-- Ergonomic pcpatch constructor — same value as the verbose form.
-------------------------------------------------------------------------------

SELECT pcpatch(1, pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))::text =
  (:patch1)::text;
-- inline use inside tpcpatch + numPoints round-trip
SELECT numPoints(tpcpatch(
  pcpatch(1, pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
  '2024-01-01'::timestamptz)) = 2;

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
-- pcid + per-instant point counts
-------------------------------------------------------------------------------

SELECT pcid(:inst1);
SELECT startNumPoints(:inst1);
SELECT endNumPoints(:inst1);
SELECT numInstants(:inst1);
SELECT numInstants(tpcpatchSeq(ARRAY[:inst1, :inst2]));

-------------------------------------------------------------------------------
-- Inherited Temporal<T> accessors
-------------------------------------------------------------------------------

SELECT tempBasetype(:inst1);
SELECT getTimestamp(:inst1);
SELECT numPoints(tpcpatch(getValue(:inst1), '2024-01-05'::timestamptz));
SELECT lowerInc(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT upperInc(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT timestamps(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT numSequences(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT numInstants(startSequence(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3])));
SELECT numInstants(endSequence(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3])));
SELECT numInstants(sequenceN(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]), 1));
SELECT array_length(sequences(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3])), 1);
SELECT array_length(segments(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3])), 1);

-------------------------------------------------------------------------------
-- numPoints(tpcpatch) — total points across every instant
-------------------------------------------------------------------------------

SELECT numPoints(:inst1);                                  -- 2 (one instant, two points)
SELECT numPoints(tpcpatchSeq(ARRAY[:inst1, :inst2]));      -- 4 (two instants, two points each)
SELECT numPoints(tpcpatchSeqSet(ARRAY[                     -- 8 (a 2-instant seq + two singletons, non-overlapping)
  tpcpatchSeq(ARRAY[:inst1, :inst2]),
  tpcpatchSeq(ARRAY[:inst3]),
  tpcpatchSeq(ARRAY[:inst4])]));

-------------------------------------------------------------------------------
-- points(tpcpatch) — SRF: (timestamp, pcpoint) per point in each instant
-------------------------------------------------------------------------------

-- Row count = total number of points (matches numPoints).
SELECT COUNT(*) FROM points(:inst1);                       -- 2
SELECT COUNT(*) FROM points(tpcpatchSeq(ARRAY[:inst1, :inst2])); -- 4

-- All emitted timestamps belong to the input's timestamps.
SELECT bool_and(t IN ('2024-01-01'::timestamptz, '2024-01-02'::timestamptz))
FROM points(tpcpatchSeq(ARRAY[:inst1, :inst2]));

-- Per-instant grouping count matches per-instant numPoints.
SELECT bool_and(c = 2)
FROM (SELECT t, COUNT(*) c FROM points(tpcpatchSeq(ARRAY[:inst1, :inst2]))
      GROUP BY t) x;

-------------------------------------------------------------------------------
-- Time accessors
-------------------------------------------------------------------------------

SELECT startTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2]));
SELECT endTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2]));
SELECT pcid(tpcpatchSeq(ARRAY[:inst1, :inst2]));

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

SELECT tpcpatchInst(:inst1) IS NOT NULL;
SELECT tpcpatchSeq(:inst1) IS NOT NULL;
SELECT tpcpatchSeqSet(:inst1) IS NOT NULL;
SELECT setInterp(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]), 'step')
  IS NOT NULL;
SELECT shiftTime(:inst1, interval '1 day') IS NOT NULL;
SELECT scaleTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  interval '2 days') IS NOT NULL;
SELECT shiftScaleTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  interval '1 day', interval '2 days') IS NOT NULL;
SELECT tsample(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  interval '1 day') IS NOT NULL;

-------------------------------------------------------------------------------
-- Restrictions — at/minusValue(s), at/minusTime, at/minusTpcbox.
-------------------------------------------------------------------------------

-- Value restrictions: present value → at / minus non-null; value set →
-- at / minus non-null; absent value → at NULL.
SELECT atValue(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]), :patch1)
  IS NOT NULL;
SELECT minusValue(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]), :patch1)
  IS NOT NULL;
SELECT atValues(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  set(ARRAY[:patch1, :patch2])) IS NOT NULL;
SELECT minusValues(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  set(ARRAY[:patch1])) IS NOT NULL;
SELECT atValue(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3, :inst4]),
  PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[9.0, 9.0, 9.0]::float[])])) IS NULL;

SELECT atTime(tpcpatchSeq(ARRAY[:inst1, :inst2]),
  tstzspan '[2024-01-02, 2024-01-03]') IS NOT NULL;
SELECT atTpcbox(:inst1, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NOT NULL;
SELECT minusTpcbox(:inst1, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NULL;
SELECT atTpcbox(:inst1, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 999, 0)) IS NULL;

-- Restriction to the instants before / after a timestamp. The strict flag
-- is true by default and excludes the instant at the timestamp itself, which
-- on a sequence shows up as the inclusivity of the resulting time span.
SELECT timeSpan(beforeTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02'));
SELECT timeSpan(beforeTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02', false));
SELECT timeSpan(afterTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02'));
SELECT timeSpan(afterTimestamp(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02', false));
-- An instant is dropped by the strict form at its own timestamp and kept by
-- the non-strict one.
SELECT beforeTimestamp(:inst2, timestamptz '2024-01-02') IS NULL;
SELECT afterTimestamp(:inst2, timestamptz '2024-01-02') IS NULL;
SELECT numInstants(beforeTimestamp(:inst2, timestamptz '2024-01-02', false));
SELECT numInstants(afterTimestamp(:inst2, timestamptz '2024-01-02', false));

-------------------------------------------------------------------------------
-- Per-point restrictions — atTpcboxFine / minusTpcboxFine.
-- inst1 has points at (1,1,1) and (2,2,2). A box covering [0,3]^3
-- keeps both; [0,1]^3 keeps only the first; [10,20]^3 keeps neither
-- and the at-restriction returns NULL.
-------------------------------------------------------------------------------

SELECT numInstants(atTpcboxFine(:inst1, tpcbox_zt(0, 0, 0, 3, 3, 3,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)));
SELECT startNumPoints(atTpcboxFine(:inst1, tpcbox_zt(0, 0, 0, 3, 3, 3,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)));
SELECT startNumPoints(atTpcboxFine(:inst1, tpcbox_zt(0, 0, 0, 1, 1, 1,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)));
SELECT atTpcboxFine(:inst1, tpcbox_zt(10, 10, 10, 20, 20, 20,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NULL;

-- minus is the complement.
SELECT minusTpcboxFine(:inst1, tpcbox_zt(0, 0, 0, 3, 3, 3,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NULL;
SELECT startNumPoints(minusTpcboxFine(:inst1, tpcbox_zt(0, 0, 0, 1, 1, 1,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)));

-------------------------------------------------------------------------------
-- Per-point restrictions by geometry — atGeometry / minusGeometry.
-------------------------------------------------------------------------------

SELECT startNumPoints(atGeometry(:inst1,
  geometry 'Polygon((0 0, 0 3, 3 3, 3 0, 0 0))'));
SELECT atGeometry(:inst1,
  geometry 'Polygon((10 10, 10 20, 20 20, 20 10, 10 10))') IS NULL;
SELECT startNumPoints(minusGeometry(:inst1,
  geometry 'Polygon((0 0, 0 1.5, 1.5 1.5, 1.5 0, 0 0))'));

-------------------------------------------------------------------------------
-- Spatial relationships — eIntersects(tpcpatch, geometry).
-------------------------------------------------------------------------------

SELECT eIntersects(:inst1,
  geometry 'Polygon((0 0, 0 3, 3 3, 3 0, 0 0))');
SELECT eIntersects(:inst1,
  geometry 'Polygon((10 10, 10 20, 20 20, 20 10, 10 10))');

-------------------------------------------------------------------------------
-- points(tpcpatch) SRF — emits one row per (timestamp, pcpoint) pair.
-------------------------------------------------------------------------------

SELECT count(*) FROM points(:inst1);
SELECT count(*) FROM points(tpcpatchSeq(ARRAY[:inst1, :inst2]));
SELECT count(DISTINCT t)
  FROM points(tpcpatchSeq(ARRAY[:inst1, :inst2]));

-------------------------------------------------------------------------------
-- Modifications
-- insert/update take temporal values that share the same value at their
-- common timestamp; append adds an instant/sequence after the end.
-------------------------------------------------------------------------------

SELECT insert(tpcpatchSeq(ARRAY[:inst1, :inst2]),
  tpcpatchSeq(ARRAY[:inst2, :inst3])) IS NOT NULL;
SELECT insert(tpcpatchSeq(ARRAY[:inst1, :inst2]),
  tpcpatchSeq(ARRAY[:inst2, :inst3]), false) IS NOT NULL;
SELECT update(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  tpcpatchSeq(ARRAY[:inst2])) IS NOT NULL;
SELECT update(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  tpcpatchSeq(ARRAY[:inst2, :inst3])) IS NOT NULL;

SELECT deleteTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  timestamptz '2024-01-02') IS NOT NULL;
SELECT deleteTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzset '{2024-01-02}') IS NOT NULL;
SELECT deleteTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspan '[2024-01-02, 2024-01-02]') IS NOT NULL;
SELECT deleteTime(tpcpatchSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspanset '{[2024-01-02, 2024-01-02]}') IS NOT NULL;

SELECT appendInstant(tpcpatchSeq(ARRAY[:inst1, :inst2]), :inst3) IS NOT NULL;
SELECT appendSequence(tpcpatchSeq(ARRAY[:inst1, :inst2]),
  tpcpatchSeq(ARRAY[:inst3])) IS NOT NULL;

-------------------------------------------------------------------------------
