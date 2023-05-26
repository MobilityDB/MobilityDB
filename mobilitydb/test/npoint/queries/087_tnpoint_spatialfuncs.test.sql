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

SELECT srid(tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT srid(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT srid(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT srid(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT srid(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT srid(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT ST_AsText(round(trajectory(tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(round(trajectory(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(trajectory(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6))) AS t(dp);
SELECT ST_AsText(round(trajectory(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(trajectory(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(round(trajectory(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.2)@2000-01-03]'), 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(trajectory(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.2)@2000-01-03]'), 6))) AS t(dp);
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT ST_AsText(round(trajectory(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.2)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT array_agg(ST_AsText((dp).geom)) FROM (SELECT ST_DumpPoints(round(trajectory(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.2)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6))) AS t(dp);

SELECT ST_AsText(round(trajectory(tnpoint '{Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.2)@2000-01-02}'), 6));
SELECT ST_AsText(round(trajectory(tnpoint '{[Npoint(1, 0.0)@2000-01-01, Npoint(1, 1.0)@2000-01-02], [Npoint(2, 0.1)@2000-01-03, Npoint(2, 0.5)@2000-01-04]}'), 6));
SELECT ST_AsText(round(trajectory(tnpoint '{[npoint(2, 0.2)@2000-01-01, npoint(2, 0.3)@2000-01-02],[npoint(1, 0.0)@2000-01-03, npoint(1, 1.0)@2000-01-04]}'), 6));

SELECT round(atGeometry(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(atGeometry(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(atGeometry(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(atGeometry(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(atGeometry(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(atGeometry(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);

SELECT atGeometry(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon empty');
SELECT atGeometry(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon empty');
SELECT atGeometry(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT atGeometry(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');
SELECT atGeometry(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT atGeometry(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');

-- Test for NULL inputs since the function is not STRICT
SELECT atGeometryTime(NULL::tnpoint, geometry 'SRID=5676;Point(1 1)', tstzspan '[2000-01-02, 2000-01-02]');
SELECT atGeometryTime(tnpoint 'Npoint(1, 0.5)@2000-01-01', NULL::geometry, tstzspan '[2000-01-02, 2000-01-02]');
SELECT atGeometryTime(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Point(1 1)', NULL::tstzspan);

SELECT round(atGeometryTime(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(atGeometryTime(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(atGeometryTime(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(atGeometryTime(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(atGeometryTime(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(atGeometryTime(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);

SELECT round(minusGeometry(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(minusGeometry(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(minusGeometry(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(minusGeometry(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(minusGeometry(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);
SELECT round(minusGeometry(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6);

SELECT minusGeometry(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon empty');
SELECT minusGeometry(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon empty');
SELECT minusGeometry(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT minusGeometry(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');
SELECT minusGeometry(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT minusGeometry(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');

SELECT round(minusGeometryTime(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(minusGeometryTime(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(minusGeometryTime(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(minusGeometryTime(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(minusGeometryTime(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);
SELECT round(minusGeometryTime(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tstzspan '[2000-01-02, 2000-01-02]'), 6);

SELECT equals(npoint(1, 0.5), npoint(1, 0.50000001));
SELECT equals(npoint 'Npoint(1, 1)', npoint 'Npoint(2, 1)');
-- TODO
-- SELECT equals(npoint(1, 0.7744007411523213), npoint(2, 0.6952992297355585));

SELECT round(length(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(length(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(length(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT round(cumulativeLength(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);
SELECT round(cumulativeLength(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(cumulativeLength(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);

SELECT round(speed(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(speed(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);
SELECT round(speed(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(speed(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);

SELECT ST_AsText(round(twCentroid(tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(twCentroid(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(twCentroid(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(twCentroid(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(twCentroid(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(twCentroid(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]}'), 6));

SELECT round(azimuth(tnpoint '[Npoint(1, 0.0)@2000-01-01, Npoint(1, 1.0)@2000-01-02]'), 6);
SELECT round(azimuth(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(azimuth(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);
SELECT round(azimuth(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(azimuth(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6);

SELECT round(azimuth(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6);
SELECT round(azimuth(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03]}'), 6);
SELECT round(azimuth(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.2)@2000-01-02], [Npoint(1, 0.5)@2000-01-03,Npoint(1, 0.5)@2000-01-04]}'), 6);

-------------------------------------------------------------------------------

SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))');

SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');

SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01', npoint 'Npoint(2, 0.5)');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', npoint 'Npoint(2, 0.5)');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)');
SELECT nearestApproachInstant(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)');

SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachInstant(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))')::numeric, 6);

SELECT nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachDistance(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty');
SELECT nearestApproachDistance(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty');

SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT nearestApproachDistance(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01', npoint 'Npoint(2, 0.5)')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', npoint 'Npoint(2, 0.5)')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)')::numeric, 6);

SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Npoint(1, 0.5)@2000-01-01')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')::numeric, 6);
SELECT round(nearestApproachDistance(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}')::numeric, 6);

-------------------------------------------------------------------------------

SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01', geometry 'SRID=5676;Polygon empty'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', geometry 'SRID=5676;Polygon empty'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', geometry 'SRID=5676;Polygon empty'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', geometry 'SRID=5676;Polygon empty'), 6));

SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon((50 50,50 100,100 100,100 50,50 50))', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(geometry 'SRID=5676;Polygon empty', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01', npoint 'Npoint(2, 0.5)'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', npoint 'Npoint(2, 0.5)'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', npoint 'Npoint(2, 0.5)'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', npoint 'Npoint(2, 0.5)'), 6));

SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(npoint 'Npoint(2, 0.5)', tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint 'Npoint(1, 0.5)@2000-01-01',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Npoint(1, 0.5)@2000-01-01'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]'), 6));
SELECT ST_AsText(round(shortestLine(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  tnpoint 'Interp=Step;{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}'), 6));

-------------------------------------------------------------------------------
