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
-------------------------------------------------------------------------------

-- Literal-value MF-JSON shape tests for tpcpatch — companion to
-- 421_tpcpoint_mfjson.test.sql for the tpcpoint side. Every assertion
-- collapses to a scalar so the diff stays diff-able.

\set inst1 'tpcpatch(pcpatch(1, pcpoint(1, 1.0, 2.0, 3.0), pcpoint(1, 4.0, 5.0, 6.0)), ''2024-01-01''::timestamptz)'
\set inst2 'tpcpatch(pcpatch(1, pcpoint(1, 7.0, 8.0, 9.0)), ''2024-01-02''::timestamptz)'

-------------------------------------------------------------------------------
-- type tag — MovingPCPatch on every subtype
-------------------------------------------------------------------------------

SELECT asMFJSON(:inst1)::jsonb ->> 'type' = 'MovingPCPatch';
SELECT asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb ->> 'type' = 'MovingPCPatch';
SELECT asMFJSON(tpcpatchSeqSet(ARRAY[
  tpcpatchSeq(ARRAY[:inst1]),
  tpcpatchSeq(ARRAY[:inst2])]))::jsonb ->> 'type' = 'MovingPCPatch';

-------------------------------------------------------------------------------
-- per-instant value object — pcid, npoints, bounds[4]
-------------------------------------------------------------------------------

-- pcid in each value object matches the input pcid
SELECT (asMFJSON(:inst1)::jsonb -> 'values' -> 0 ->> 'pcid')::int = 1;

-- npoints reflects the actual point count in the source patch
SELECT (asMFJSON(:inst1)::jsonb -> 'values' -> 0 ->> 'npoints')::int = 2;
SELECT (asMFJSON(:inst2)::jsonb -> 'values' -> 0 ->> 'npoints')::int = 1;

-- bounds is a 4-element array following PCBOUNDS layout
-- [xmin, xmax, ymin, ymax]
SELECT jsonb_array_length(asMFJSON(:inst1)::jsonb -> 'values' -> 0 -> 'bounds') = 4;
SELECT (asMFJSON(:inst1)::jsonb -> 'values' -> 0 -> 'bounds') = '[1, 4, 2, 5]'::jsonb;

-- Sequence emits one value object per instant, in input order
SELECT jsonb_array_length(asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb -> 'values') = 2;
SELECT (asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb -> 'values' -> 1 ->> 'npoints')::int = 1;

-------------------------------------------------------------------------------
-- bbox embedding (options=1) — bbox + pcid at the top level
-------------------------------------------------------------------------------

SELECT (asMFJSON(:inst1, options := 1)::jsonb ->> 'pcid')::int = 1;
SELECT asMFJSON(:inst1, options := 1)::jsonb ? 'bbox';
-- bbox is [[xmin, ymin], [xmax, ymax]]
SELECT jsonb_array_length(asMFJSON(:inst1, options := 1)::jsonb -> 'bbox') = 2;

-- bbox + period coexist when both temporal and spatial axes are populated
SELECT asMFJSON(:inst1, options := 1)::jsonb ? 'period';
SELECT asMFJSON(:inst1, options := 1)::jsonb -> 'period' ? 'begin';
SELECT asMFJSON(:inst1, options := 1)::jsonb -> 'period' ? 'end';

-------------------------------------------------------------------------------
-- datetimes / interpolation
-------------------------------------------------------------------------------

-- Single instant has interpolation = None
SELECT asMFJSON(:inst1)::jsonb ->> 'interpolation' = 'None';
-- Sequence has interpolation = Step (tpcpatch's default)
SELECT asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb ->> 'interpolation' = 'Step';
-- datetimes count matches values count
SELECT jsonb_array_length(
  asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb -> 'datetimes') =
  jsonb_array_length(
  asMFJSON(tpcpatchSeq(ARRAY[:inst1, :inst2]))::jsonb -> 'values');

-------------------------------------------------------------------------------
