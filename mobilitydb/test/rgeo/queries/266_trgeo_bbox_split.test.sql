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
-- expandSpace
-------------------------------------------------------------------------------

SELECT expandSpace(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0.5 0.5), 0.0)@2001-01-01',
  1.0);

SELECT expandSpace(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03]',
  0.5);

-------------------------------------------------------------------------------
-- spans
-------------------------------------------------------------------------------

SELECT spans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01');

SELECT spans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(1 0),0.0)@2001-01-03]');

SELECT spans(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(1 0),0.0)@2001-01-02],[Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(3 0),0.0)@2001-01-04]}');

-------------------------------------------------------------------------------
-- stboxes
-------------------------------------------------------------------------------

SELECT stboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0), 0.0)@2001-01-01');

SELECT stboxes(trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03]');

-------------------------------------------------------------------------------
-- splitNSpans
-------------------------------------------------------------------------------

SELECT splitNSpans(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(4 0),0.0)@2001-01-05]',
  2);

SELECT splitNSpans(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(1 0),0.0)@2001-01-02],[Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(3 0),0.0)@2001-01-04]}',
  3);

-------------------------------------------------------------------------------
-- splitEachNSpans
-------------------------------------------------------------------------------

SELECT splitEachNSpans(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(4 0),0.0)@2001-01-05]',
  2);

-------------------------------------------------------------------------------
-- splitNStboxes
-------------------------------------------------------------------------------

SELECT splitNStboxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(4 0),0.0)@2001-01-05]',
  2);

-------------------------------------------------------------------------------
-- splitEachNStboxes
-------------------------------------------------------------------------------

SELECT splitEachNStboxes(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0.0)@2001-01-01, Pose(Point(2 0),0.0)@2001-01-03, Pose(Point(4 0),0.0)@2001-01-05]',
  2);

-------------------------------------------------------------------------------
