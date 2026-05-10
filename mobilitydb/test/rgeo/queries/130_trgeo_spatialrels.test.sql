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
-- eContains
-------------------------------------------------------------------------------

SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 0.1,0.1 0.1,0.1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 0.1,0.1 0.1,0.1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');

/* Errors */
SELECT eContains(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- aContains
-------------------------------------------------------------------------------

SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 0.1,0.1 0.1,0.1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aContains(geometry 'SRID=5676;Polygon((0 0,0 0.1,0.1 0.1,0.1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');

/* Errors */
SELECT aContains(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eCovers, aCovers
-------------------------------------------------------------------------------

SELECT eCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT eCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT eCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT eCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT eCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Point(1 1)');
SELECT eCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Point(1 1)');
SELECT eCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Point(1 1)');

/* Errors */
SELECT eCovers(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/*****************************************************************************/

SELECT aCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT aCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT aCovers(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT aCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT aCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Point(1 1)');
SELECT aCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Point(1 1)');
SELECT aCovers(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Point(1 1)');

/* Errors */
SELECT aCovers(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT eDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT eDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');

SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/* Errors */
SELECT eDisjoint(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- aDisjoint
-------------------------------------------------------------------------------

SELECT aDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT aDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT aDisjoint(geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 0.5,0.5 0.5,0.5 0,0 0))');

SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aDisjoint(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/* Errors */
SELECT aDisjoint(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eIntersects
-------------------------------------------------------------------------------

SELECT eIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT eIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT eIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');

SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/* Errors */
SELECT eIntersects(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- aIntersects
-------------------------------------------------------------------------------

SELECT aIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT aIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT aIntersects(geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 10,10 10,10 0,0 0))');

SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aIntersects(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/* Errors */
SELECT aIntersects(geometry 'SRID=3812;Polygon((0 0,0 10,10 10,10 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------

SELECT eTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT eTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT eTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT eTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT eTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT eTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT eTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT eTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');

/* Errors */
SELECT eTouches(geometry 'SRID=3812;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

/*****************************************************************************/

SELECT aTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT aTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}');
SELECT aTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]');
SELECT aTouches(geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT aTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT aTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT aTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');
SELECT aTouches(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Polygon((0 0,0 1,1 1,1 0,0 0))');

/* Errors */
SELECT aTouches(geometry 'SRID=3812;Polygon((0 0,0 1,1 1,1 0,0 0))', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- eDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);
SELECT eDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', 1.0);
SELECT eDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', 1.0);
SELECT eDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', 1.0);

SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Point(1 1)', 1.0);

SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);
SELECT eDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);

/* Errors */
SELECT eDwithin(geometry 'SRID=3812;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);

-------------------------------------------------------------------------------
-- aDwithin
-------------------------------------------------------------------------------

SELECT aDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);
SELECT aDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', 1.0);
SELECT aDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', 1.0);
SELECT aDwithin(geometry 'SRID=5676;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', 1.0);

SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03]', geometry 'SRID=5676;Point(1 1)', 1.0);
SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.4)@2000-01-01, Pose(Point(1 1),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}', geometry 'SRID=5676;Point(1 1)', 1.0);

SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);
SELECT aDwithin(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.7)@2000-01-03}', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);

/* Errors */
SELECT aDwithin(geometry 'SRID=3812;Point(1 1)', trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1.0);

-------------------------------------------------------------------------------
