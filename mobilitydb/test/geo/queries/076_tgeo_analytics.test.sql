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
-- Affine transforms

-- Rotate a 3D temporal point 180 degrees about the z axis.  Note this is long-hand for doing rotate();
SELECT asEWKT(affine(temp, 
  cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), 0, 0, 0, 1, 0, 0, 0)) 
  AS using_affine, asEWKT(rotate(temp, pi())) AS using_rotate
FROM (SELECT tgeometry '[POINT(1 2 3)@2001-01-01, POINT(1 4 3)@2001-01-02]' AS temp) AS t;

-- Rotate a 3D temporal point 180 degrees in both the x and z axis
SELECT asEWKT(affine(temp, 
  cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), -sin(pi()), 0, sin(pi()), cos(pi()), 0, 0, 0))
FROM (SELECT tgeometry '[Point(1 2 3)@2001-01-01, Point(1 4 3)@2001-01-02]' AS temp) AS t;

-- Rotate a temporal point 180 degrees
SELECT asEWKT(rotate(tgeometry '[POINT(50 160)@2001-01-01, POINT(50 50)@2001-01-02, POINT(100 50)@2001-01-03]', pi()), 6);
-- Rotate 30 degrees counter-clockwise at x=50, y=160
SELECT asEWKT(rotate(tgeometry '[POINT(50 160)@2001-01-01, POINT(50 50)@2001-01-02, POINT(100 50)@2001-01-03]', pi()/6, 50, 160), 6);
-- Rotate 60 degrees clockwise from centroid
SELECT asEWKT(rotate(temp, -pi()/3, ST_Centroid(traversedArea(temp))), 6)
FROM (SELECT tgeometry '[POINT(50 160)@2001-01-01, POINT(50 50)@2001-01-02, POINT(100 50)@2001-01-03]' AS temp) AS t;

SELECT asEWKT(scale(tgeometry '[Point(1 2 3)@2001-01-01, Point(1 1 1)@2001-01-02]', geometry 'POINT(0.5 0.75 0.8)'));
SELECT asEWKT(scale(tgeometry '[Point(1 2 3)@2001-01-01, Point(1 1 1)@2001-01-02]', 0.5, 0.75, 0.8));
SELECT asEWKT(scale(tgeometry '[Point(1 2 3)@2001-01-01, Point(1 1 1)@2001-01-02]', 0.5, 0.75));
SELECT asEWKT(scale(tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]', geometry 'POINT(2 2)', geometry 'POINT(1 1)'));

-- Empty geometry
SELECT asEWKT(scale(tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]', geometry 'POINT Empty'));
SELECT asEWKT(scale(tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]', geometry 'POINT(1 1)', geometry 'POINT Empty'));
-- Coverage
SELECT asEWKT(scale(tgeometry 'Point(1 1)@2001-01-01', geometry 'POINT(1 1)'));
SELECT asEWKT(scale(tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02],[Point(3 3)@2001-01-03]}', geometry 'POINT(1 1)'));

/* Errors */
SELECT asEWKT(scale(tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]', geometry 'Linestring(1 1,2 2)'));
SELECT asEWKT(scale(tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]', geometry 'POINT(1 1)', geometry 'Linestring(1 1,2 2)'));

-------------------------------------------------------------------------------
