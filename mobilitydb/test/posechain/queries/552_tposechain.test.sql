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
-------------------------------------------------------------------------------
-- Tests for the temporal pose chain type
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input/output
-------------------------------------------------------------------------------

SELECT asEWKT(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01');
SELECT asEWKT(tposechain '{PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02}');
SELECT asEWKT(tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]');
SELECT asEWKT(tposechain '{[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02], [PoseChain(Pose(Point(2 2), 0))@2001-01-03]}');

-------------------------------------------------------------------------------
-- Errors
-- Every value of a temporal pose chain holds the same number of links: a chain
-- that gains a joint is a different structure, not a later value of the same
-- one.
-------------------------------------------------------------------------------

SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))@2001-01-02]';
SELECT tposechain '{PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))@2001-01-02}';

-------------------------------------------------------------------------------
-- Comparison against a chain of another length
-- A chain that holds another number of links is a different value, so a
-- comparison against it answers false rather than refusing the question.
-------------------------------------------------------------------------------

SELECT tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01' ?= posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]' ?= posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]' %= posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))';
SELECT posechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0))' ?= tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]';
-- The same chain still compares equal where the lengths agree
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]' ?= posechain 'PoseChain(Pose(Point(0 0), 0))';

-------------------------------------------------------------------------------
-- Interpolation
-- Every link interpolates as a pose does, linearly in position and along the
-- shortest arc in rotation.
-------------------------------------------------------------------------------

-- A two-link arm whose shoulder turns a quarter turn: halfway through, the
-- shoulder is at an eighth of a turn and the elbow has not moved
SELECT asEWKT(round(valueAtTimestamp(
  appendInstant(
    tposechain(posechain(ARRAY[pose(ST_Point(0,0), 0), pose(ST_Point(10,0), 0)]),
      timestamptz '2001-01-01'),
    tposechain(posechain(ARRAY[pose(ST_Point(0,0), pi()/2), pose(ST_Point(10,0), 0)]),
      timestamptz '2001-01-03')),
  timestamptz '2001-01-02'), 6));

-------------------------------------------------------------------------------
-- Conversions
-- Composing the chain at every instant answers where its innermost frame sits
-------------------------------------------------------------------------------

SELECT asEWKT(round(tpose(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01'), 6));
SELECT asEWKT(round((tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01')::tpose, 6));

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT numPoses(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01');
SELECT tempSubtype(tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01');
SELECT interp(tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]');
SELECT asEWKT(getValue(tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01'));
SELECT numInstants(tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]');
SELECT timeSpan(tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]');

-------------------------------------------------------------------------------
-- Spatial reference system
-- Only the outer link names a frame, so only the outer link is transformed
-------------------------------------------------------------------------------

SELECT SRID(tposechain 'SRID=3812;PoseChain(Pose(Point(1 2), 0))@2001-01-01');
SELECT asEWKT(setSRID(tposechain 'PoseChain(Pose(Point(1 2), 0))@2001-01-01', 3812));
SELECT asEWKT(round(transform(tposechain 'SRID=4326;PoseChain(Pose(Point(4.35 50.85), 1), Pose(Point(10 0), 0))@2001-01-01', 3812), 6));
SELECT asEWKT(round(transformPipeline(tposechain 'SRID=4326;PoseChain(Pose(Point(4.35 50.85), 1))@2001-01-01', 'urn:ogc:def:coordinateOperation:EPSG::16031', 4326), 6));

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT asEWKT(atTime(tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-03]', timestamptz '2001-01-02'));
SELECT asEWKT(atValue(tposechain '{PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02}', posechain 'PoseChain(Pose(Point(1 1), 0))'));

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01' = tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01';
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01' <> tposechain 'PoseChain(Pose(Point(1 1), 0))@2001-01-01';

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Bounding box
-- The box of a chain covers the composed position of every prefix, so an
-- instant carries the reach of the whole chain and not only of its last link
-------------------------------------------------------------------------------

SELECT stbox(tposechain 'PoseChain(Pose(Point(1 2), 0), Pose(Point(3 4), 0))@2001-01-01');
SELECT stbox(tposechain '{PoseChain(Pose(Point(1 2), 0))@2001-01-01, PoseChain(Pose(Point(5 6), 0))@2001-01-02}');
SELECT stbox(tposechain '[PoseChain(Pose(Point(1 2), 0))@2001-01-01, PoseChain(Pose(Point(5 6), 0))@2001-01-02]');

-------------------------------------------------------------------------------
