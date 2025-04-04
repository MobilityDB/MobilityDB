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
-- Input
-------------------------------------------------------------------------------

SELECT asText(pose 'Pose(Point(1 1),0.5)');
SELECT asText(pose ' pose   (  Point  ( 1  1  ) ,	0.5   )   ');
/* Errors */
SELECT pose 'point(1,0.5)';
SELECT pose 'pose 1,0.5)';
SELECT pose 'Pose(Point(1 1),0.5';
SELECT pose 'Pose(Point(1 1) 0.5)';
SELECT pose 'Pose(Point(1 1)000,0.5)';
SELECT pose 'Pose(Point(1 1),-1.5)';
SELECT pose 'Pose(Point(1 1),0.5)xxx';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(pose('Point(1 1)', 0.5));
SELECT asText(pose(ST_Point(1,1), 0.5));
/* Errors */
SELECT pose('Linestring(1 1,2 2)',1.5);
SELECT pose('Point Z(1 1 1)',1.5);
SELECT pose('Point M(1 1 1)',1.5);
SELECT pose(geography 'Point(1 1)',1.5);
SELECT pose('Point(1 1)',-1.5);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsText(point(pose 'Pose(Point(1 1),0.5)'));
SELECT rotation(pose 'Pose(Point(1 1),0.5)');
SELECT srid(pose 'Pose(SRID=5676;Point(1 1),0.5)');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(round(pose 'Pose(Point(1.123456789 1.123456789), 0.123456789)', 6));

-------------------------------------------------------------------------------
-- Cast functions 
-------------------------------------------------------------------------------

SELECT ST_AsText(round(pose 'Pose(Point(1 1),0.2)'::geometry, 6));

-- SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::pose;

-- NULL

-- /* Errors */

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

-- true
SELECT pose 'Pose(Point(1.000001 1),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1.000001),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5000001)' ~= pose 'Pose(Point(1 1),0.5)';
-- false
SELECT pose 'Pose(Point(1.00001 1),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1.00001),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.500001)' ~= pose 'Pose(Point(1 1),0.5)';

-------------------------------------------------------------------------------

SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(2 2),0.5)';

-------------------------------------------------------------------------------/
