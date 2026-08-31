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

-- Temporal spatial relationships for temporal pose chains. Every one of them
-- converts its operands to the temporal geometry point the chain composes to
-- and re-delegates, so a chain answers exactly as that pose does. The family
-- declares the matrix its target declares: tContains and tCovers take the
-- geometry first only, and tTouches has no direction between two chains,
-- because a moving point neither contains nor covers a geometry and two
-- moving points do not touch.

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
-- two chains do not touch, so the direction between them is not declared

-------------------------------------------------------------------------------
-- tDwithin, including the two-chain direction
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, tposechain 'PoseChain(Pose(Point(1 2), 0.1))@2001-01-01', 2.0));
SELECT asText(tDwithin(:inside, tposechain 'PoseChain(Pose(Point(1 9), 0.1))@2001-01-01', 2.0));

-------------------------------------------------------------------------------
-- Linear interpolation, which the tgeompoint target carries
-------------------------------------------------------------------------------

\set lin 'tposechain ''[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(3 3), 0.1))@2001-01-03]'''

SELECT asText(tIntersects(:lin, :square));
SELECT asText(tIntersects(:square, :lin));
SELECT asText(tDisjoint(:lin, :square));
SELECT asText(tDwithin(:lin, geometry 'Point(1 1)', 1.0));

-- The answer is the one the chain's own pose gives, turning points and all
SELECT asText(tIntersects(:lin, :square)) =
  asText(tIntersects(:lin::tpose::tgeompoint, :square)) AS ti_agrees;

-- A moving point neither contains nor covers a geometry, so the direction
-- taking the chain first is not declared
SELECT to_regprocedure('tContains(tposechain, geometry)') IS NULL AS tcontains_absent,
       to_regprocedure('tCovers(tposechain, geometry)') IS NULL AS tcovers_absent,
       to_regprocedure('tTouches(tposechain, tposechain)') IS NULL AS ttouches_absent,
       to_regprocedure('tContains(geometry, tposechain)') IS NOT NULL AS tcontains_present;

-------------------------------------------------------------------------------
