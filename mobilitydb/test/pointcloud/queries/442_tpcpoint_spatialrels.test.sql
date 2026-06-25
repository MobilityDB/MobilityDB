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

-- Value-level tests for the spatial-relationship functions on
-- tpcpoint — eIntersects / aIntersects / eDisjoint / aDisjoint /
-- eDwithin / aDwithin.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), ''2024-01-02''::timestamptz)'
\set inst1 ':p1'
\set seq 'tpcpointSeq(ARRAY[:p1, :p2])'

-------------------------------------------------------------------------------
-- eIntersects
-------------------------------------------------------------------------------

SELECT eIntersects(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT eIntersects(geometry 'SRID=0;POINT(0 0)', :p1);
SELECT eIntersects(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT eIntersects(:seq, geometry 'SRID=0;POINT(0 0)');
SELECT eIntersects(:p1, :p1);

-------------------------------------------------------------------------------
-- aIntersects
-------------------------------------------------------------------------------

SELECT aIntersects(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT aIntersects(:seq, geometry 'SRID=0;POINT(0 0)');

-------------------------------------------------------------------------------
-- eDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT eDisjoint(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT eDisjoint(:seq, geometry 'SRID=0;POINT(99 99)');

-------------------------------------------------------------------------------
-- aDisjoint
-------------------------------------------------------------------------------

SELECT aDisjoint(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT aDisjoint(:seq, geometry 'SRID=0;POINT(99 99)');

-------------------------------------------------------------------------------
-- eDwithin / aDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(:p1, geometry 'SRID=0;POINT(1 1)', 5.0);
SELECT eDwithin(:p1, geometry 'SRID=0;POINT(99 99)', 5.0);
SELECT aDwithin(:p1, geometry 'SRID=0;POINT(0 0)', 1.0);
SELECT eDwithin(:p1, :p1, 1e-9);
SELECT eDwithin(:seq, geometry 'SRID=0;POINT(0 0)', 1.0);

-------------------------------------------------------------------------------
