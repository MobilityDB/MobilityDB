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

-- Temporal spatial relationships for temporal pose chains. Every one of them
-- converts its operands to a temporal geometry and re-delegates, so a chain
-- answers exactly as the pose it composes to does.

\set square 'geometry ''Polygon((0 0,0 2,2 2,2 0,0 0))'''
\set inside 'tposechain ''PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'''
\set corner 'tposechain ''PoseChain(Pose(Point(0 0), 0.1))@2001-01-01'''
\set moving 'tposechain ''{PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03}'''

-------------------------------------------------------------------------------
-- tIntersects and tDisjoint are negations of each other
-------------------------------------------------------------------------------

SELECT asText(tIntersects(:moving, :square));
SELECT asText(tIntersects(:square, :moving));
SELECT asText(tDisjoint(:moving, :square));
SELECT asText(tDisjoint(:square, :moving));

-------------------------------------------------------------------------------
-- tContains and tCovers
-------------------------------------------------------------------------------

SELECT asText(tContains(:square, :moving));
SELECT asText(tCovers(:square, :moving));
SELECT asText(tContains(:square, :corner));
SELECT asText(tCovers(:square, :corner));

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT asText(tTouches(:corner, :square));
SELECT asText(tTouches(:square, :corner));
SELECT asText(tTouches(:inside, :square));

-------------------------------------------------------------------------------
-- tDwithin, including the two-chain direction
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, tposechain 'PoseChain(Pose(Point(1 2), 0.1))@2001-01-01', 2.0));
SELECT asText(tDwithin(:inside, tposechain 'PoseChain(Pose(Point(1 9), 0.1))@2001-01-01', 2.0));

-------------------------------------------------------------------------------
