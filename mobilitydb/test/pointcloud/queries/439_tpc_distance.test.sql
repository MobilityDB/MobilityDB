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

-- Value-level tests for the |=| (nearest-approach distance) operator.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[3.0, 4.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set box_at_origin 'tpcbox_zt(0, 0, 0, 0, 0, 0, tstzspan ''[2024-01-01, 2024-01-02]'', 1, 0)'
\set box_far       'tpcbox_zt(3, 4, 0, 3, 4, 0, tstzspan ''[2024-01-01, 2024-01-02]'', 1, 0)'

-- Self-distance is zero.
SELECT (:p1) |=| (:p1);

-- 3-4-5 right triangle in the xy plane.
SELECT (:p1) |=| (:p2);
SELECT (:p2) |=| (:p1);

-- Box-to-box at the same coords.
SELECT (:box_at_origin) |=| (:box_at_origin);

-- Distance from a point to a box at the same location is zero.
SELECT (:p1) |=| (:box_at_origin);

-- Disjoint time spans yield infinity.
SELECT (tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]),
                 '2024-01-01'::timestamptz)) |=|
       (tpcbox_zt(0, 0, 0, 0, 0, 0,
                  tstzspan '[2099-01-01, 2099-01-02]', 1, 0)) > 1e10;

-- Pcid mismatch yields infinity.
SELECT (:p1) |=| (tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2024-01-01, 2024-01-02]', 999, 0)) > 1e10;

-------------------------------------------------------------------------------
