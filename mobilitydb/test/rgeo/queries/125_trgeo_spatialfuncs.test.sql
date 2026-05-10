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

-------------------------------------------------------------------------------
-- SRID
-------------------------------------------------------------------------------

SELECT SRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT SRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT SRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT SRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');
SELECT SRID(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/*****************************************************************************/

SELECT asEWKT(setSRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 5676));
SELECT asEWKT(setSRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', 5676));
SELECT asEWKT(setSRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', 5676));
SELECT asEWKT(setSRID(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', 5676));

-------------------------------------------------------------------------------
-- traversedArea
-------------------------------------------------------------------------------

-- No unary union: returns a geometry collection of polygon positions
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(0 0),0.0)@2000-01-01'));
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(0 0),0.0)@2000-01-02}'));
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(0 0),0.0)@2000-01-02]'));
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(0 0),0.0)@2000-01-02],[Pose(Point(0 0),0.0)@2000-01-03, Pose(Point(0 0),0.0)@2000-01-04]}'));

-- With unary union: collapses duplicate positions into a single polygon
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(0 0),0.0)@2000-01-02}', true));
SELECT ST_AsText(traversedArea(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(0 0),0.0)@2000-01-02]', true));

/*****************************************************************************/

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry
-------------------------------------------------------------------------------

-- TInstant: pose at (0,0) -> triangle overlaps bounding box
SELECT asEWKT(atGeometry(trgeometry 'Polygon((0 0,2 0,1 2,0 0));Pose(Point(0 0),0.0)@2000-01-01', 'Polygon((-1 -1,3 -1,3 3,-1 3,-1 -1))'::geometry));
-- Discrete TSequenceSet: first instant inside, second outside query geometry
SELECT asEWKT(atGeometry(trgeometry 'Polygon((0 0,2 0,1 2,0 0));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(10 10),0.0)@2000-01-02}', 'Polygon((-1 -1,3 -1,3 3,-1 3,-1 -1))'::geometry));
-- minusGeometry: complement of the above
SELECT asEWKT(minusGeometry(trgeometry 'Polygon((0 0,2 0,1 2,0 0));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(10 10),0.0)@2000-01-02}', 'Polygon((-1 -1,3 -1,3 3,-1 3,-1 -1))'::geometry));
-- atGeometry returns NULL when no instant matches
SELECT asEWKT(atGeometry(trgeometry 'Polygon((0 0,2 0,1 2,0 0));Pose(Point(10 10),0.0)@2000-01-01', 'Polygon((-1 -1,3 -1,3 3,-1 3,-1 -1))'::geometry));
-- Linear TSequence: not supported, raises error
SELECT asEWKT(atGeometry(trgeometry 'Polygon((0 0,2 0,1 2,0 0));[Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(1 1),0.0)@2000-01-02]', 'Polygon((-1 -1,3 -1,3 3,-1 3,-1 -1))'::geometry));

/*****************************************************************************/

-------------------------------------------------------------------------------
-- atStbox / minusStbox
-------------------------------------------------------------------------------

-- TInstant: pose at (0,0), stbox covers the triangle
SELECT asEWKT(atStbox(trgeometry 'Polygon((0 0,2 0,1 2,0 0));Pose(Point(0 0),0.0)@2000-01-01', 'STBOX XT(((-1,-1),(3,3)),[2000-01-01,2000-01-02])'::stbox));
-- Discrete TSequenceSet: first instant inside box, second outside
SELECT asEWKT(atStbox(trgeometry 'Polygon((0 0,2 0,1 2,0 0));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(10 10),0.0)@2000-01-02}', 'STBOX XT(((-1,-1),(3,3)),[2000-01-01,2000-01-02])'::stbox));
-- minusStbox: complement
SELECT asEWKT(minusStbox(trgeometry 'Polygon((0 0,2 0,1 2,0 0));{Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(10 10),0.0)@2000-01-02}', 'STBOX XT(((-1,-1),(3,3)),[2000-01-01,2000-01-02])'::stbox));
-- Linear TSequence: not supported, raises error
SELECT asEWKT(atStbox(trgeometry 'Polygon((0 0,2 0,1 2,0 0));[Pose(Point(0 0),0.0)@2000-01-01, Pose(Point(1 1),0.0)@2000-01-02]', 'STBOX XT(((-1,-1),(3,3)),[2000-01-01,2000-01-02])'::stbox));

/*****************************************************************************/
