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

SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tcontains(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tcontains(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tcontains(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tcontains(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-- Additional parameter

SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tcontains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

-------------------------------------------------------------------------------
-- tdisjoint
-------------------------------------------------------------------------------

SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tdisjoint(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tdisjoint(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tdisjoint(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tdisjoint(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

-- Additional parameter

SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', true);
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', true);

SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', true);
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', true);
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', true);
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', true);

SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tdisjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', false);
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', false);

SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', false);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', false);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', false);
SELECT tdisjoint(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', false);

SELECT tdisjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', false);
SELECT tdisjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', false);
SELECT tdisjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', false);
SELECT tdisjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', false);

-------------------------------------------------------------------------------
-- tintersects
-------------------------------------------------------------------------------

SELECT tintersects(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tintersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tintersects(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tintersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tintersects(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tintersects(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tintersects(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tintersects(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tintersects(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tintersects(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT tintersects(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT tintersects(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT tintersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT tintersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT tintersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT tintersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT tintersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT tintersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT tintersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT tintersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT tintersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT tintersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT tintersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT tintersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

-------------------------------------------------------------------------------
-- ttouches
-------------------------------------------------------------------------------

SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT ttouches(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT ttouches(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT ttouches(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT ttouches(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT ttouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT ttouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT ttouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT ttouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT ttouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT ttouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT ttouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT ttouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT ttouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)');
SELECT ttouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)');
SELECT ttouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)');
SELECT ttouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)');

-- Additional parameter

SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT ttouches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', true);
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', true);
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', true);
SELECT ttouches(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', true);

SELECT ttouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', true);
SELECT ttouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', true);
SELECT ttouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', true);
SELECT ttouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', true);

SELECT ttouches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', true);
SELECT ttouches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', true);
SELECT ttouches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', true);
SELECT ttouches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', true);

-------------------------------------------------------------------------------
-- tdwithin
-------------------------------------------------------------------------------

SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT tdwithin(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty', 2);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', 2);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

-- Additional parameter

SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2, true);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2, true);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2, true);
SELECT tdwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2, true);

SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2, true);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2, true);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2, true);
SELECT tdwithin(npoint 'NPoint(2, 0.3)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2, true);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2, true);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2, true);

SELECT tdwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tdwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tdwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(2, 0.3)', 2, true);
SELECT tdwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(2, 0.3)', 2, true);

-------------------------------------------------------------------------------
