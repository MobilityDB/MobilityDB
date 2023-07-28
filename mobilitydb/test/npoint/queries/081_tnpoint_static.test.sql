-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

SELECT npoint 'npoint(1,0.5)';
SELECT npoint ' npoint   (   1   ,	0.5   )   ';
/* Errors */
SELECT npoint 'point(1,0.5)';
SELECT npoint 'npoint 1,0.5)';
SELECT npoint 'npoint(1,0.5';
SELECT npoint 'npoint(1 0.5)';
SELECT npoint 'npoint(1000,0.5)';
SELECT npoint 'npoint(1,1.5)';
SELECT npoint 'npoint(1,0.5)xxx';

SELECT nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment '  nsegment  (  1  ,  0.5  ,  0.7 ) ';
/* Errors */
SELECT nsegment 'Nsegment 1, 1.1)';
SELECT nsegment(1, 1.1);
SELECT nsegment 'segment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.5,0.7';
SELECT nsegment 'nsegment(1 0.5 0.7)';
SELECT nsegment 'nsegment(1000,0.5,0.7)';
SELECT nsegment 'nsegment(1,1.5,0.7)';
SELECT nsegment 'nsegment(1,0.5,0.7)xxx';
SELECT nsegment 'NSegment(1, 1, 1.5)';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT npoint(1, 0.5);
/* Errors */
SELECT npoint(1000,0.5);
SELECT npoint(1,1.5);

SELECT nsegment(1, 0.2, 0.6);
SELECT nsegment(1);
SELECT nsegment(1, 0.2);
SELECT nsegment(npoint(1, 0.5));
/* Errors */
SELECT nsegment(1000,0.5,0.7);
SELECT nsegment(1,1.5,0.7);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT route(npoint 'npoint(1,0.5)');
SELECT getPosition(npoint 'npoint(1,0.5)');
SELECT srid(npoint 'npoint(1,0.5)');

SELECT route(nsegment 'nsegment(1,0.5,0.7)');
SELECT startPosition(nsegment 'nsegment(1,0.5,0.7)');
SELECT endPosition(nsegment 'nsegment(1,0.5,0.7)');
SELECT srid(nsegment 'nsegment(1,0.5,0.7)');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT round(npoint 'NPoint(1, 0.123456789)', 6);
SELECT round(nsegment 'NSegment(1, 0.123456789, 0.223456789)', 6);

-------------------------------------------------------------------------------
-- Cast functions between network and space
-------------------------------------------------------------------------------

SELECT ST_AsText(round(npoint 'npoint(1,0.2)'::geometry, 6));

SELECT ST_AsText(round(nsegment 'nsegment(1,0.5,0.7)'::geometry, 6));

SELECT round((npoint 'npoint(1,0.2)'::geometry)::npoint, 6);

SELECT round((nsegment 'nsegment(1,0.5,0.7)'::geometry)::nsegment, 6);
SELECT round((nsegment 'nsegment(1,0.5,0.5)'::geometry)::nsegment, 6);

SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::npoint;

SELECT geometry 'SRID=5676;LINESTRING(83.2832009065896 86.0903322231025,69.0807154867798 81.2081503681839,13.625699095428 97.5346013903618)'::nsegment;
-- NULL
SELECT geometry 'SRID=5676;LINESTRING(416.346567736997 528.335344322874,610.455019399524 528.508247341961,476.989195102204 642.550969672973)'::nsegment;
/* Errors */
SELECT geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'::nsegment;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(2,0.5)';

SELECT nsegment_cmp(nsegment 'nsegment(1,0.3,0.5)', nsegment 'nsegment(1,0.3,0.4)');

SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(2,0.3,0.5)';

-------------------------------------------------------------------------------/
