-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- tcontains
-------------------------------------------------------------------------------

SELECT tcontains(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tcontains(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tcontains(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tcontains(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tcontains(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

-- Additional parameter
SELECT tcontains(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', true);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true);

SELECT tcontains(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', false);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', false);
SELECT tcontains(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', false);

SELECT tcontains(geometry 'Linestring(1 1,2 2)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]');

/* Errors */
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tcontains(geometry 'Point(1 1)', tgeompoint 'SRID=5676;Point(1 1)@2000-01-01');
SELECT tcontains(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');
SELECT tcontains(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tdisjoint
-------------------------------------------------------------------------------

SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tdisjoint(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tdisjoint(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tdisjoint(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tdisjoint(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tdisjoint(tgeompoint '[Point(0 1)@2000-01-01, Point(2 1)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tdisjoint(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-04)', geometry 'Linestring(1 1,2 1)');
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-04)', geometry 'Linestring(0 0,1 1)');

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

-- Additional parameter
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', true);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true);

SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', false);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', false);
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', false);

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', true);
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', true);
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', true);
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', true);

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', false);
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', false);
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', false);
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', false);

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', true);

SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tdisjoint(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tdisjoint(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tdisjoint(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', false);

/* Errors */
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tdisjoint(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tdisjoint(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tdisjoint(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tintersects
-------------------------------------------------------------------------------

SELECT tintersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tintersects(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tintersects(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tintersects(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tintersects(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT tintersects(tgeompoint '[Point(0 1)@2000-01-01, Point(2 1)@2000-01-04]', geometry 'Linestring(1 0,1 1,2 1,2 0)');
SELECT tintersects(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-04)', geometry 'Linestring(1 1,2 1)');
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-04)', geometry 'Linestring(0 0,1 1)');

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]', geometry 'Linestring(1 1,2 2)');
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(4 1)@2000-01-02]', geometry 'Linestring(1 2,1 0,2 0,2 2)');

-- Additional parameter
SELECT tintersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', true);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true);

SELECT tintersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', false);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', false);
SELECT tintersects(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', false);

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', true);
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', true);
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', true);
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', true);

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', false);
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', false);
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', false);
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', false);

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', true);

SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tintersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT tintersects(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', false);

-- Coverage
SELECT tintersects(tgeompoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-03}', tgeompoint 'Point(2 2)@2000-01-02');
SELECT intersects(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint '[Point(2 1)@2000-01-01, Point(1 2)@2000-01-02]');

/* Errors */
SELECT tintersects(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tintersects(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT tintersects(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT tintersects(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- ttouches
-------------------------------------------------------------------------------

SELECT ttouches(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT ttouches(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01');
SELECT ttouches(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT ttouches(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT ttouches(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT ttouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)');
SELECT ttouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)');
SELECT ttouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)');
SELECT ttouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)');

SELECT ttouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty');
SELECT ttouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty');
SELECT ttouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty');
SELECT ttouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty');

SELECT ttouches(geometry 'Linestring(1 1,2 2)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]');

-- Additional parameter
SELECT ttouches(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', true);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', true);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true);

SELECT ttouches(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', false);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', false);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', false);
SELECT ttouches(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', false);

SELECT ttouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', true);
SELECT ttouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', true);
SELECT ttouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', true);
SELECT ttouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', true);

SELECT ttouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', false);
SELECT ttouches(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', false);
SELECT ttouches(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', false);
SELECT ttouches(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', false);

/* Errors */
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT ttouches(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT ttouches(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1)@2000-01-01');
SELECT ttouches(geometry 'Point(1 1)', tgeompoint 'Point(1 1 1)@2000-01-01');

-------------------------------------------------------------------------------
-- tdwithin
-------------------------------------------------------------------------------

SELECT tdwithin(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tdwithin(geometry 'Point empty', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(geometry 'Point empty', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(geometry 'Point empty', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(geometry 'Point empty', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', 2);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty', 2);

--3D
SELECT tdwithin(geometry 'Point(1 1 1)', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(geometry 'Point(1 1 1)', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(geometry 'Point(1 1 1)', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(geometry 'Point(1 1 1)', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tdwithin(geometry 'Point Z empty', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(geometry 'Point Z empty', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(geometry 'Point Z empty', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(geometry 'Point Z empty', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point(1 1 1)', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point(1 1 1)', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point(1 1 1)', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point(1 1 1)', 2);

SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', geometry 'Point Z empty', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', geometry 'Point Z empty', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', geometry 'Point Z empty', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', geometry 'Point Z empty', 2);

-- Coverage
SELECT tdwithin(tgeompoint '(Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', geometry 'Point(0 1)', 1);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)', geometry 'Point(2 3)', 1);
SELECT tdwithin(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02)', geometry 'Point(2 3)', 1);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2);

SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(1 3)@2000-01-03]', geometry 'Point(1 2)', 0);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(1 2)@2000-01-03]', geometry 'Point(1 3)', 0);
SELECT tdwithin(tgeompoint '(Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(1 0)@2000-01-01, Point(2 0)@2000-01-02]', 1);
SELECT tdwithin(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(1 0)@2000-01-01, Point(2 0)@2000-01-02]', 1);
SELECT tdwithin(tgeompoint '[Point(0 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeompoint '[Point(2 0)@2000-01-01, Point(1 0)@2000-01-02]', 1);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(0 0)@2000-01-02]', tgeompoint '[Point(2 0)@2000-01-01, Point(1 1)@2000-01-02]', 1);
SELECT tdwithin(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(0 2)@2000-01-01, Point(1 3)@2000-01-02]', 1);
SELECT tdwithin(tgeompoint '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]', tgeompoint '[Point(4 0)@2000-01-01, Point(3 1)@2000-01-02]', 0);

SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(1 1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', 2);
SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', 2);
SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);
SELECT tdwithin(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03], [Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', 2);

SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(3 3)@
2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]
,[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);

SELECT tdwithin(tgeompoint 'Interp=Stepwise;[Point(2 0)@2000-01-01, Point(2 2)@2000-01-05]',
  tgeompoint 'Interp=Stepwise;[Point(1 0)@2000-01-01, Point(2 0)@2000-01-05]', 1);
SELECT tdwithin(tgeompoint '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-05]',
  tgeompoint 'Interp=Stepwise;[Point(0 1)@2000-01-01, Point(2 0)@2000-01-05]', 1);
SELECT tdwithin(tgeompoint '[Point(1 0)@2000-01-01, Point(1 4)@2000-01-05]',
  tgeompoint 'Interp=Stepwise;[Point(1 2)@2000-01-01, Point(1 3)@2000-01-05]', 1);

-- Mixed 2D/3D
SELECT tdwithin(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);

-- Additional parameter
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2, true);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2, true);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2, true);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2, true);

SELECT tdwithin(geometry 'Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2, false);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 2, false);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 2, false);
SELECT tdwithin(geometry 'Point(1 1)', tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 2, false);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', 2, true);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', 2, true);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', 2, true);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', 2, true);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1)', 2, false);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point(1 1)', 2, false);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point(1 1)', 2, false);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point(1 1)', 2, false);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2, true);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', 2, true);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', 2, true);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', 2, true);

SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2, false);
SELECT tdwithin(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-01', 2, false);
SELECT tdwithin(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-01', 2, false);
SELECT tdwithin(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03], [Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-01', 2, false);

/* Errors */
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)', 2);
SELECT tdwithin(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(geometry 'Linestring(1 1,2 2)', tgeompoint 'Point(1 1)@2000-01-01', 2);
SELECT tdwithin(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)', 2);

-------------------------------------------------------------------------------
