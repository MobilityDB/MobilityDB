-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

-- Temporal spatial relationships for tpcpoint. Every one of them converts its
-- operands to the temporal geometry point the schema-aware projection gives
-- and re-delegates, the three predicates its Z-carrying values can answer.
-- That projection promotes step to linear, so tgeompoint — which carries
-- linear interpolation, unlike tgeometry — is what a sequence can reach.

\set square 'geometry ''SRID=0;Polygon((0 0,0 2,2 2,2 0,0 0))'''
\set inside 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz)'
\set corner 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2001-01-01''::timestamptz)'
\set far 'tpcpoint(PC_MakePoint(1, ARRAY[9.0, 9.0, 0.0]::float[]), ''2001-01-02''::timestamptz)'

-------------------------------------------------------------------------------
-- tIntersects and tDisjoint are negations of each other
-------------------------------------------------------------------------------

SELECT asText(tIntersects(:inside, :square));
SELECT asText(tIntersects(:square, :inside));
SELECT asText(tDisjoint(:inside, :square));
SELECT asText(tDisjoint(:far, :square));

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, :inside, 0.0));

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A sequence, whose projection carries linear interpolation
-------------------------------------------------------------------------------

\set seq 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 0.0]::float[]), ''2001-01-02''::timestamptz)])'

SELECT asText(tIntersects(:seq, :square));
SELECT asText(tIntersects(:square, :seq));
SELECT asText(tDisjoint(:seq, :square));
SELECT asText(tDwithin(:seq, geometry 'SRID=0;Point(1 1)', 1.0));

-- The answer is the one the projection itself gives
SELECT asText(tIntersects(:seq, :square)) =
  asText(tIntersects(:seq::tgeompoint, :square)) AS ti_agrees;

-------------------------------------------------------------------------------
