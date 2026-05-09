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

-- Bounding box decomposition, motion metrics, and tprecision/tsample for
-- trgeometry.

-- A unit square translating from (0,0) to (4,0) with no rotation.
-- stboxes: one box per segment for a linear sequence → 1 box
SELECT round(unnest(stboxes(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]')), 6);

-- stboxes: a sequence set with two sequences → 2 boxes (one per sequence)
SELECT COUNT(*) FROM unnest(stboxes(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));{[Pose(Point(0 0),0)@2026-01-01, Pose(Point(2 0),0)@2026-01-01 12:00], [Pose(Point(4 0),0)@2026-01-02, Pose(Point(6 0),0)@2026-01-03]}'));

-- expandSpace
SELECT round(expandSpace(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]', 1.0), 6);

-- spans
SELECT spans(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]');

-- splitNSpans
SELECT splitNSpans(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-03]', 2);

-- splitEachNSpans (table test)
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE splitEachNSpans(temp, 2) IS NOT NULL;

-- splitNStboxes
SELECT COUNT(*) FROM unnest(splitNStboxes(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-03]', 2));

-- splitEachNStboxes (table test)
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE splitEachNStboxes(temp, 2) IS NOT NULL;

-- length
SELECT round(length(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]')::numeric, 6);

-- cumulativeLength: check it returns a tfloat
SELECT numInstants(cumulativeLength(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]'));

-- speed: check it returns a tfloat
SELECT numInstants(speed(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]'));

-- twCentroid
SELECT ST_AsText(round(twCentroid(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-02]'), 6));

-- tprecision
SELECT asText(tprecision(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-03]',
  interval '1 day'));

-- tsample
SELECT asText(tsample(
  trgeometry 'Polygon((0 0, 1 0, 1 1, 0 1, 0 0));[Pose(Point(0 0),0)@2026-01-01, Pose(Point(4 0),0)@2026-01-03]',
  interval '1 day'));

-- Table tests
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE spans(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE stboxes(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE length(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d WHERE CASE WHEN interp(temp) = 'Linear' THEN speed(temp) IS NOT NULL ELSE FALSE END;

-------------------------------------------------------------------------------
