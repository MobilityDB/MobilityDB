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

-- Value-level tests for asMFJSON() on tpcpoint and tpcpatch.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 2.0, 3.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[4.0, 5.0, 6.0]::float[]), ''2024-01-02''::timestamptz)'
\set patch 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set q1 'tpcpatch(:patch, ''2024-01-01''::timestamptz)'

-------------------------------------------------------------------------------
-- tpcpoint MF-JSON: instant + sequence with bbox = options 1.
-------------------------------------------------------------------------------

SELECT asMFJSON(:p1);
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]));
SELECT asMFJSON(:p1, 1);

-------------------------------------------------------------------------------
-- tpcpoint MF-JSON shape — structural assertions (mirror 432 for tpcpatch).
-------------------------------------------------------------------------------

-- type tag — MovingPCPoint on every subtype
SELECT asMFJSON(:p1)::jsonb ->> 'type' = 'MovingPCPoint';
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb ->> 'type' = 'MovingPCPoint';

-- coordinates — one [x, y, z] per instant, in input order
SELECT (asMFJSON(:p1)::jsonb -> 'coordinates' -> 0) = '[1, 2, 3]'::jsonb;
SELECT jsonb_array_length(asMFJSON(:p1)::jsonb -> 'coordinates') = 1;
SELECT jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'coordinates') = 2;
SELECT (asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb
  -> 'coordinates' -> 1) = '[4, 5, 6]'::jsonb;

-- bbox embedding (options=1) — pcid + bbox + period at the top level
SELECT (asMFJSON(:p1, options := 1)::jsonb ->> 'pcid')::int = 1;
SELECT asMFJSON(:p1, options := 1)::jsonb ? 'bbox';
-- bbox is [[xmin, ymin, zmin], [xmax, ymax, zmax]] for 3D schemas
SELECT jsonb_array_length(asMFJSON(:p1, options := 1)::jsonb -> 'bbox') = 2;
SELECT jsonb_array_length(
  asMFJSON(:p1, options := 1)::jsonb -> 'bbox' -> 0) = 3;

-- bbox + period coexist
SELECT asMFJSON(:p1, options := 1)::jsonb ? 'period';
SELECT asMFJSON(:p1, options := 1)::jsonb -> 'period' ? 'begin';
SELECT asMFJSON(:p1, options := 1)::jsonb -> 'period' ? 'end';

-- datetimes / interpolation
SELECT asMFJSON(:p1)::jsonb ->> 'interpolation' = 'None';
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb ->> 'interpolation' = 'Step';
-- datetimes count matches coordinates count
SELECT jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'datetimes') =
  jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'coordinates');

-------------------------------------------------------------------------------
-- tpcpatch MF-JSON: pcid + npoints + bounds in the values array.
-------------------------------------------------------------------------------

SELECT asMFJSON(:q1);
SELECT asMFJSON(:q1, 1);

-------------------------------------------------------------------------------
