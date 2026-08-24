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

-- Temporal spatial relationships for tpcpatch. A patch reaches the geometry
-- engine as the multipoint of the positions its points occupy, and answers the
-- three predicates its Z-carrying values can answer.

\set square 'geometry ''SRID=0;Polygon((0 0,0 2,2 2,2 0,0 0))'''
\set inside 'tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1)), ''2001-01-01''::timestamptz)'
\set straddling 'tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1), pcpoint(1, 9, 9, 9)), ''2001-01-01''::timestamptz)'
\set far 'tpcpatch(pcpatch(1, pcpoint(1, 9, 9, 9)), ''2001-01-02''::timestamptz)'

-------------------------------------------------------------------------------
-- The geometry a patch reaches the engine as
-------------------------------------------------------------------------------

SELECT ST_AsText(geometry(pcpatch(1, pcpoint(1, 1, 1, 1), pcpoint(1, 2, 2, 2))));
SELECT ST_AsText(pcpatch(1, pcpoint(1, 1, 1, 1))::geometry);
SELECT asText(:straddling::tgeometry);

-------------------------------------------------------------------------------
-- tIntersects and tDisjoint are negations of each other
-------------------------------------------------------------------------------

SELECT asText(tIntersects(:inside, :square));
SELECT asText(tIntersects(:square, :inside));
SELECT asText(tIntersects(:far, :square));
SELECT asText(tDisjoint(:inside, :square));
SELECT asText(tDisjoint(:far, :square));

-- One point of the patch inside the square is enough for the whole patch to
-- intersect it
SELECT asText(tIntersects(:straddling, :square));
SELECT asText(tDisjoint(:straddling, :square));

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 2)', 2.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, :inside, 0.0));

-------------------------------------------------------------------------------
-- A sequence answers instant by instant
-------------------------------------------------------------------------------

SELECT asText(tIntersects(tpcpatchSeq(ARRAY[:inside, :far]), :square));

-------------------------------------------------------------------------------
