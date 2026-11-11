-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- Simplification functions
--
-- A temporal pose simplifies the trajectory of its points and keeps its
-- instants at the timestamps that simplification keeps. Each block checks the
-- result against that composition written out by hand.
-------------------------------------------------------------------------------

\set seq 'tpose ''[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(2 2), 0.2)@2001-01-02, Pose(Point(3 1), 0.3)@2001-01-03, Pose(Point(4 4), 0.4)@2001-01-04]'''

SELECT minDistSimplify(NULL::tpose, 1.0);
SELECT minTimeDeltaSimplify(NULL::tpose, interval '1 day');

SELECT asText(minDistSimplify(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 1.0));
SELECT asText(minDistSimplify(tpose '{Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(2 2), 0.2)@2001-01-02, Pose(Point(1 1), 0.3)@2001-01-03}', 1.0));
SELECT asText(minDistSimplify(:seq, 2.0));
SELECT asText(minTimeDeltaSimplify(:seq, interval '2 days'));
SELECT asText(maxDistSimplify(:seq, 1.0));
SELECT asText(maxDistSimplify(:seq, 1.0, false));
SELECT asText(douglasPeuckerSimplify(:seq, 1.0));
SELECT asText(douglasPeuckerSimplify(:seq, 1.0, false));

SELECT minDistSimplify(:seq, 2.0) = deleteTime(:seq,
  set(timestamps(:seq)) - set(timestamps(minDistSimplify(:seq::tgeompoint, 2.0))),
  true) AS agrees_mindist;
SELECT minTimeDeltaSimplify(:seq, interval '2 days') = deleteTime(:seq,
  set(timestamps(:seq)) -
  set(timestamps(minTimeDeltaSimplify(:seq::tgeompoint, interval '2 days'))),
  true) AS agrees_mintdelta;
SELECT maxDistSimplify(:seq, 1.0) = deleteTime(:seq,
  set(timestamps(:seq)) - set(timestamps(maxDistSimplify(:seq::tgeompoint, 1.0))),
  true) AS agrees_maxdist;
SELECT douglasPeuckerSimplify(:seq, 1.0, false) = deleteTime(:seq,
  set(timestamps(:seq)) -
  set(timestamps(douglasPeuckerSimplify(:seq::tgeompoint, 1.0, false))),
  true) AS agrees_dp;

-------------------------------------------------------------------------------
-- Simplification keeps the interpolation and the sequence segmentation of the
-- value it simplifies
-------------------------------------------------------------------------------

SELECT interp(minDistSimplify(tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', 2.0));
SELECT interp(minTimeDeltaSimplify(tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', interval '2 days'));
SELECT interp(maxDistSimplify(tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', 2.0));
SELECT interp(douglasPeuckerSimplify(tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', 2.0));

-- A step value stays step and comes back unchanged
SELECT asText(minDistSimplify(tpose 'Interp=Step;[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', 2.0));

-- When nothing is dropped the value comes back unchanged
SELECT minDistSimplify(tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]', 0.5) = tpose '[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03]';

-- The gap of a sequence set survives
SELECT numSequences(minDistSimplify(tpose '{[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03], [Pose(Point(9 0), 0.4)@2001-01-05, Pose(Point(20 0), 0.5)@2001-01-06]}', 2.0));
SELECT asText(minDistSimplify(tpose '{[Pose(Point(0 0), 0.1)@2001-01-01, Pose(Point(1 0), 0.2)@2001-01-02, Pose(Point(4 0), 0.3)@2001-01-03], [Pose(Point(9 0), 0.4)@2001-01-05, Pose(Point(20 0), 0.5)@2001-01-06]}', 2.0));

-- The trajectory of a pose is simplified in the plane, so a geodetic pose
-- raises an error
SELECT minDistSimplify(tpose '[GeodPose(Point(1 1),0.1)@2001-01-01, GeodPose(Point(2 2),0.2)@2001-01-02]', 1.0);

-------------------------------------------------------------------------------
