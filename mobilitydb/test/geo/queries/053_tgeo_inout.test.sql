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
-- Input
-------------------------------------------------------------------------------

-- Maximum decimal digits

SELECT asText(tgeometry 'Point(1.123456789 1.123456789 1.123456789)@2000-01-01', 6);
SELECT asText(tgeometry '{Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03}', 6);
SELECT asText(tgeometry '[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03]', 6);
SELECT asText(tgeometry '{[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03],[Point(3.123456789 3.123456789 3.123456789)@2000-01-04, Point(3.123456789 3.123456789 3.123456789)@2000-01-05]}', 6);
SELECT asText(tgeometry 'Interp=Step;[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03]', 6);
SELECT asText(tgeometry 'Interp=Step;{[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03],[Point(3.123456789 3.123456789 3.123456789)@2000-01-04, Point(3.123456789 3.123456789 3.123456789)@2000-01-05]}', 6);

SELECT asEWKT(tgeography 'Point(1.123456789 1.123456789 1.123456789)@2000-01-01', 6);
SELECT asEWKT(tgeography '{Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03}', 6);
SELECT asEWKT(tgeography '[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03]', 6);
SELECT asEWKT(tgeography '{[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03],[Point(3.123456789 3.123456789 3.123456789)@2000-01-04, Point(3.123456789 3.123456789 3.123456789)@2000-01-05]}', 6);
SELECT asEWKT(tgeography 'Interp=Step;[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03]', 6);
SELECT asEWKT(tgeography 'Interp=Step;{[Point(1.123456789 1.123456789 1.123456789)@2000-01-01, Point(2.123456789 2.123456789 2.123456789)@2000-01-02, Point(1.123456789 1.123456789 1.123456789)@2000-01-03],[Point(3.123456789 3.123456789 3.123456789)@2000-01-04, Point(3.123456789 3.123456789 3.123456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '[Point(1 2)@2000-01-01, Point(1 2)@2000-01-02)')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '(Point(1 2)@2000-01-01, Point(1 2)@2000-01-02)')));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

-- Additional attributes
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry 'SRID=4326;Point(1 2 3)@2000-01-01',1,0,2)));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}',1,0,2)));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]',1,0,2)));
SELECT asEWKT(tgeometryFromMFJSON(asMFJSON(tgeometry 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}',1,0,2)));
/* Errors */
SELECT tgeometryFromMFJSON('ABC');
SELECT tgeometryFromMFJSON('{"types":"MovingGeometry","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT tgeometryFromMFJSON('{"type":"XXX","coordinates":[1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT tgeometryFromMFJSON('{"type":"MovingFloat","values":[1],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interp":"Discrete"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"XXX"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","valuesxxx":[[1,1]],"datetimes":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[[1,1]],"datetimess":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[{"type":"XXX","coordinates":[1,1]}],"datetimes":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[{"type":"Point","coordinates":[[1,1]]}],"datetimes":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[{"type":"Point","coordinates":[1,1]}],"datetimes":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[{"type":"Point","coordinates":[1,1]}],"datetimes":["XXXX"],"interpolation":"None"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","values":[{"type":"Point","coordinates":[1,1]}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"Linear"}');
SELECT tgeometryFromMFJSON(
'{"type":"MovingGeometry","sequences":[{"values":{"type":"Point","coordinates":[1,1]},"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolation":"Step"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","sequences":[],"interpolation":"Step"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","sequences":[{"values":[{"type":"XXX","coordinates":[[1,1]]}],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolation":"Step"}');
SELECT tgeometryFromMFJSON('{"type":"MovingGeometry","sequences":[{"values":[{"type":"Point","coordinates":[[1,1]]}],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolation":"Step"}');
SELECT tgeometryFromMFJSON(
'{"type":"MovingGeometry","sequences":[{"values":[{"type":"Point","coordinates":[1,1]}],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":false,"upper_inc":true}],"interpolation":"Step"}');
SELECT tgeometryFromMFJSON(
'{"type":"MovingGeometry","sequences":[{"values":[{"type":"Point","coordinates":[1,1]},{"type":"Point","coordinates":[1,1]}],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true}],"interpolation":"Step"}');

-----------------------------------------------------------------------

SELECT asEWKT(tgeographyFromMFJSON(asMFJSON(tgeography 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeographyFromMFJSON(asMFJSON(tgeography '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeographyFromMFJSON(asMFJSON(tgeography '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeographyFromMFJSON(asMFJSON(tgeography '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

-----------------------------------------------------------------------

SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '[Point(1 2)@2000-01-01, Point(1 2)@2000-01-02)')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '(Point(1 2)@2000-01-01, Point(1 2)@2000-01-02)')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=4326;Point(1 2 3)@2000-01-01')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]')));
SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}')));

select asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'XDR')));
select asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=5676;Point(1 1 1)@2000-01-01', 'XDR')));
/* Errors */
select asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'ABC')));

-------------------------------------------------------------------------------
-- Output
-------------------------------------------------------------------------------

SELECT asText(tgeometry 'Point(1 1)@2000-01-01');
SELECT asText(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asText(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asText(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asText(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT asText(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asText(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asText(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asText(tgeometry 'Point(1 1 1)@2000-01-01');
SELECT asText(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asText(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asText(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asText(tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asText(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asText(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asText(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asText('{}'::tgeometry[]);
SELECT asText(ARRAY[tgeometry 'Point(1 1)@2000-01-01']);
SELECT asText(ARRAY[geometry 'Point(1 1)', 'Point(2 2)']);

SELECT asEWKT(tgeometry 'Point(1 1)@2000-01-01');
SELECT asEWKT(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asEWKT(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asEWKT(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asEWKT(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT asEWKT(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asEWKT(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asEWKT(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asEWKT(tgeometry 'Point(1 1 1)@2000-01-01');
SELECT asEWKT(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asEWKT(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asEWKT(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asEWKT(tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asEWKT(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asEWKT(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asEWKT(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asEWKT('{}'::tgeometry[]);
SELECT asEWKT(ARRAY[tgeometry 'Point(1 1)@2000-01-01']);
SELECT asEWKT(ARRAY[geometry 'Point(1 1)', 'Point(2 2)']);

SELECT asMFJSON(tgeometry 'Point(1 1)@2000-01-01');
SELECT asMFJSON(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT asMFJSON(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT asMFJSON(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT asMFJSON(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT asMFJSON(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT asMFJSON(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT asMFJSON(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asMFJSON(tgeometry 'Point(1 1 1)@2000-01-01');
SELECT asMFJSON(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}');
SELECT asMFJSON(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]');
SELECT asMFJSON(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}');
SELECT asMFJSON(tgeography 'Point(1.5 1.5 1.5)@2000-01-01');
SELECT asMFJSON(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}');
SELECT asMFJSON(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]');
SELECT asMFJSON(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}');

SELECT asMFJSON(tgeometry 'Point(1 1)@2000-01-01', 0, 0, 20);
SELECT asMFJSON(tgeometry 'Point(1 1)@2000-01-01', 0, 0, -1);
SELECT asMFJSON(tgeometry 'SRID=4326;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 3, 0, 2);
SELECT asMFJSON(tgeometry 'SRID=4326;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 4, 0, 2);
SELECT asMFJSON(tgeometry '[Point(1 2 3)@2019-01-01, Point(4 5 6)@2019-01-02]', 1, 0, 2);

-- Additional pretty-print attribute
-- Notice that the Linux and Mac versions of json_c produce slightly different versions of the pretty-print JSON
SELECT asText(tgeometryFromMFJSON(asMFJSON(tgeometry 'POINT(1 1)@2000-01-01', 1, 3, 15)));
SELECT asText(tgeometryFromMFJSON(asMFJSON(tgeometry '{POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02}', 1, 3, 15)));
SELECT asText(tgeometryFromMFJSON(asMFJSON(tgeometry '[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]', 1, 3, 15)));
SELECT asText(tgeometryFromMFJSON(asMFJSON(tgeometry '{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02], [POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}', 1, 3, 15)));

/* Errors */
SELECT asMFJSON(tgeometry 'SRID=123456;Point(50.813810 4.384260)@2019-01-01 18:00:00.15+02', 4, 2);

-------------------------------------------------------------------------------

SELECT asBinary(tgeometry 'Point(1 1)@2000-01-01');
SELECT asBinary(tgeometry 'Point(1 1)@2000-01-01', 'NDR');
SELECT asBinary(tgeometry 'Point(1 1)@2000-01-01', 'XDR');
SELECT asHexEWKB(tgeometry '[Point(1 1)@2000-01-01]');
SELECT asHexEWKB(tgeometry '[Point(1 1)@2000-01-01]', 'NDR');
SELECT asHexEWKB(tgeometry '[Point(1 1)@2000-01-01]', 'XDR');

SELECT asEWKT(tgeometryFromEWKB(asEWKB(tgeometry 'SRID=4326;Point(1 1)@2000-01-01')));
SELECT asHexEWKB(tgeometry 'SRID=4326;Point(1 1)@2000-01-01');
select asHexEWKB(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'NDR');
select asHexEWKB(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'XDR');
/* Errors */
select asEWKB(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'ABCD');

-------------------------------------------------------------------------------

SELECT astext('{}'::geometry[]);

----------------------------------------------------------------------
