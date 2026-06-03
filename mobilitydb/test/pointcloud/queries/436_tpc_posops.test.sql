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

-- Value-level tests for the directional position operators on
-- tpcpoint / tpcpatch.

\set p_low  'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set p_high 'tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), ''2024-01-15''::timestamptz)'
\set box_left  'tpcbox_zt(0, 0, 0, 5, 5, 5,    tstzspan ''[2024-01-01, 2024-01-15]'', 1, 0)'
\set box_right 'tpcbox_zt(20, 20, 20, 30, 30, 30, tstzspan ''[2025-01-01, 2025-01-15]'', 1, 0)'

-------------------------------------------------------------------------------
-- X axis: <<, >>, &<, &>
-------------------------------------------------------------------------------

SELECT (:p_low) << (:box_right);
SELECT (:box_right) >> (:p_low);
SELECT (:p_high) << (:box_left);
SELECT (:p_low) &< (:box_right);
SELECT (:p_low) &> (:box_left);

-------------------------------------------------------------------------------
-- Y axis: <<|, |>>, &<|, |&>
-------------------------------------------------------------------------------

SELECT (:p_low) <<| (:box_right);
SELECT (:box_right) |>> (:p_low);
SELECT (:p_low) &<| (:box_right);
SELECT (:p_low) |&> (:box_left);

-------------------------------------------------------------------------------
-- Z axis: <</, />>, &</, /&>
-------------------------------------------------------------------------------

SELECT (:p_low) <</ (:box_right);
SELECT (:box_right) />> (:p_low);
SELECT (:p_low) &</ (:box_right);
SELECT (:p_low) /&> (:box_left);

-------------------------------------------------------------------------------
-- Time axis: <<#, #>>, &<#, #&>  (also tstzspan)
-------------------------------------------------------------------------------

SELECT (:p_low) <<# (:box_right);
SELECT (:box_right) #>> (:p_low);
SELECT (:p_low) <<# tstzspan '[2099-01-01, 2099-12-31]';
SELECT tstzspan '[2099-01-01, 2099-12-31]' #>> (:p_low);
SELECT (:p_low) &<# tstzspan '[2099-01-01, 2099-12-31]';

-------------------------------------------------------------------------------
-- Reflexivity check: nothing is strictly to one side of itself.
-------------------------------------------------------------------------------

SELECT NOT ((:p_low) << (:p_low));
SELECT NOT ((:p_low) >> (:p_low));
SELECT NOT ((:p_low) <<| (:p_low));
SELECT NOT ((:p_low) |>> (:p_low));
SELECT NOT ((:p_low) <<# (:p_low));
SELECT NOT ((:p_low) #>> (:p_low));

-------------------------------------------------------------------------------
