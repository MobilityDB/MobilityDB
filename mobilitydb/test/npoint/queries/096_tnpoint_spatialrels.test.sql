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
-- contains
-------------------------------------------------------------------------------

SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT contains(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-- Coverage
SELECT disjoint(tnpoint 'Interp=Stepwise;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02]', tnpoint '[Npoint(1, 0.4)@2000-01-01, Npoint(1, 0.2)@2000-01-02]');

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
-- NULL
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02', 2);

-------------------------------------------------------------------------------
