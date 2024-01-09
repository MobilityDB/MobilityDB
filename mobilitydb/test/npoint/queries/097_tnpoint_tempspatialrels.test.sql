-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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
-- tContains
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tContains(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tContains(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tContains(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tContains(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Additional parameter

SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tContains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tDisjoint(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tDisjoint(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tDisjoint(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tDisjoint(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(NULL::npoint, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', NULL::tnpoint);

SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::geometry);
SELECT tDisjoint(NULL::tnpoint, geometry 'SRID=5676;Point(1 1)');

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

-- Test for NULL inputs since the function is not STRICT
SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::npoint);
SELECT tDisjoint(NULL::tnpoint, npoint 'NPoint(2, 0.3)');

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

-- Additional parameter

SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', true);

SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', true);
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', true);
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', true);
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', true);

SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tDisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', false);

SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tDisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

SELECT tDisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', false);
SELECT tDisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', false);
SELECT tDisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', false);
SELECT tDisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', false);

SELECT tDisjoint(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)');
SELECT tDisjoint(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)', false);
SELECT tDisjoint(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)', true);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tIntersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tIntersects(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tIntersects(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tIntersects(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tIntersects(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(NULL::npoint, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tIntersects(npoint 'NPoint(2, 0.3)', NULL::tnpoint);

SELECT tIntersects(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tIntersects(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tIntersects(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tIntersects(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::geometry);
SELECT tIntersects(NULL::tnpoint, geometry 'SRID=5676;Point(1 1)');

SELECT tIntersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT tIntersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT tIntersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT tIntersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT tIntersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT tIntersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT tIntersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT tIntersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

-- Test for NULL inputs since the function is not STRICT
SELECT tIntersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::npoint);
SELECT tIntersects(NULL::tnpoint, npoint 'NPoint(2, 0.3)');

SELECT tIntersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT tIntersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT tIntersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT tIntersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

SELECT tIntersects(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)');
SELECT tIntersects(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)', false);
SELECT tIntersects(tnpoint 'Npoint(1,0.5)@2000-01-01', npoint 'Npoint(1,0.6)', true);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tTouches(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tTouches(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tTouches(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tTouches(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(NULL::npoint, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tTouches(npoint 'NPoint(2, 0.3)', NULL::tnpoint);

SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT tTouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::geometry);
SELECT tTouches(NULL::tnpoint, geometry 'SRID=5676;Point(1 1)');

SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT tTouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT tTouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT tTouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

-- Test for NULL inputs since the function is not STRICT
SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::npoint);
SELECT tTouches(NULL::tnpoint, npoint 'NPoint(2, 0.3)');

SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT tTouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT tTouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT tTouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

-- Additional parameter

SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tTouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tTouches(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tTouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tTouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tTouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', true);

SELECT tTouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', true);
SELECT tTouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', true);
SELECT tTouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', true);
SELECT tTouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', true);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint, 2);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL);

SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT tDwithin(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::npoint, tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', NULL::tnpoint, 2);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL);

SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::geometry, 2);
SELECT tDwithin(NULL::tnpoint, geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', NULL);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::npoint, 2);
SELECT tDwithin(NULL::tnpoint, npoint 'NPoint(2, 0.3)', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', NULL);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', 2);

-- Test for NULL inputs since the function is not STRICT
SELECT tDwithin(NULL::tnpoint, tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::tnpoint, 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

-- Additional parameter

SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2, true);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2, true);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2, true);
SELECT tDwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2, true);

SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2, true);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2, true);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2, true);
SELECT tDwithin(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2, true);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2, true);

SELECT tDwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tDwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tDwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tDwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', 2, true);

-------------------------------------------------------------------------------
