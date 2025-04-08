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

SELECT asText(cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT asText(cbuffer ' cbuffer   (  Point  ( 1  1  ) ,	0.5   )   ');
/* Errors */
SELECT cbuffer 'point(1,0.5)';
SELECT cbuffer 'cbuffer 1,0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5';
SELECT cbuffer 'Cbuffer(Point(1 1) 0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1)000,0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),-1.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)xxx';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(cbuffer('Point(1 1)', 0.5));
SELECT asText(cbuffer(ST_Point(1,1), 0.5));
/* Errors */
SELECT cbuffer(1000,0.5);
SELECT cbuffer('Linestring(1 1,2 2)',1.5);
SELECT cbuffer('Point Z(1 1 1)',1.5);
SELECT cbuffer('Point M(1 1 1)',1.5);
SELECT cbuffer(geography 'Point(1 1)',1.5);
SELECT cbuffer('Point(1 1)',-1.5);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsText(point(cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT radius(cbuffer 'Cbuffer(Point(1 1),0.5)');
SELECT srid(cbuffer 'Cbuffer(SRID=5676;Point(1 1),0.5)');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(round(cbuffer 'Cbuffer(Point(1.123456789 1.123456789), 0.123456789)', 6));

-------------------------------------------------------------------------------
-- Cast functions 
-------------------------------------------------------------------------------

SELECT ST_AsText(round(cbuffer 'Cbuffer(Point(1 1),0.2)'::geometry, 6));

SELECT asText(round((cbuffer 'Cbuffer(Point(1 1),0.2)'::geometry)::cbuffer, 6));

-- SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::cbuffer;

-- NULL

-- /* Errors */

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

-- true
SELECT cbuffer 'Cbuffer(Point(1.000001 1),0.5)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1.000001),0.5)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5000001)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';
-- false
SELECT cbuffer 'Cbuffer(Point(1.00001 1),0.5)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1.00001),0.5)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.500001)' ~= cbuffer 'Cbuffer(Point(1 1),0.5)';

-------------------------------------------------------------------------------

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' = cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' != cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' < cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' <= cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' > cbuffer 'Cbuffer(Point(2 2),0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(1 1),0.5)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(1 1),0.7)';
SELECT cbuffer 'Cbuffer(Point(1 1),0.5)' >= cbuffer 'Cbuffer(Point(2 2),0.5)';

-------------------------------------------------------------------------------/
