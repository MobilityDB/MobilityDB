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
-- Regression tests for the tpose tile surface (119_tpose_tile.in.sql).
--
-- Every function converts its pose operand to the temporal geometry point
-- its position resolves to (tpose::tgeompoint) and delegates to the
-- temporal geometry point tile function, so a value keeps the
-- interpolation it carries: a sequence with the default linear
-- interpolation is tiled along its trajectory, and the split functions
-- rebuild the tpose fragment by restricting the original value to the time
-- extent of each tile.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are STRICT
SELECT spaceBoxes(NULL::tpose, 2.0);
SELECT spaceBoxes(tpose 'Pose(Point(1 1), 0.1)@2001-01-01', NULL::float);

-------------------------------------------------------------------------------
-- Boxes: a linear sequence is tiled along its trajectory and agrees with the
-- cast written out by hand
-------------------------------------------------------------------------------

SELECT spaceBoxes(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', 2.0);
SELECT spaceBoxes(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', 2.0) =
  spaceBoxes(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]'::tgeompoint, 2.0) AS agrees_geo;
SELECT spaceBoxes(tpose '{Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03}', 2.0) AS step_control;
SELECT timeBoxes(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', interval '1 day', '2001-01-01');
SELECT spaceTimeBoxes(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', 2.0, interval '1 day',
  geometry 'Point(0 0)', '2001-01-01');

-------------------------------------------------------------------------------
-- Splits: each fragment is a tpose restricted to the time extent of its tile
-------------------------------------------------------------------------------

SELECT (spaceSplit(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', 2.0)).*;
SELECT (spaceTimeSplit(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', 2.0, interval '1 day',
  geometry 'Point(0 0)', '2001-01-01')).*;
SELECT (timeSplit(tpose '[Pose(Point(1 1),0.1)@2001-01-01,
  Pose(Point(3 3),0.1)@2001-01-03]', interval '1 day', '2001-01-01')).*;

-------------------------------------------------------------------------------
