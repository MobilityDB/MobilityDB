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

-- Maximum decimal digits

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2000-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2000-01-03}', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2000-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2000-01-03]', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2000-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2000-01-03],[Pose(Point(3.123456789 3.123456789),0.5)@2000-01-04, Pose(Point(3.123456789 3.123456789),0.5)@2000-01-05]}', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2000-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2000-01-03]', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1.123456789 1.123456789),0.5)@2000-01-01, Pose(Point(2.123456789 2.123456789),0.5)@2000-01-02, Pose(Point(1.123456789 1.123456789),0.5)@2000-01-03],[Pose(Point(3.123456789 3.123456789),0.5)@2000-01-04, Pose(Point(3.123456789 3.123456789),0.5)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02}')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02)')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02)')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02],[Pose(Point(1 2),0.5)@2000-01-03, Pose(Point(3 4),0.5)@2000-01-04]}')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02],[Pose(Point(1 2),0.5)@2000-01-03, Pose(Point(3 4),0.5)@2000-01-04]}')));

-- Additional attributes
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02}',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]',1,0,2)));
SELECT asEWKT(trgeometryFromMFJSON(asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02],[Pose(Point(1 2),0.5)@2000-01-03, Pose(Point(3 4),0.5)@2000-01-04]}',1,0,2)));
/* Errors */
SELECT trgeometryFromMFJSON('ABC');
SELECT trgeometryFromMFJSON('{"types":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"XXX","coordinates":[1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingFloat","values":[1],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interp":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolation":"XXX"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimess":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":"2000-01-01T00:00:00+01","interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":["XXX"],"interpolation":"None"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinatess":[1,1],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"Discrete"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"Linear"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","sequences":{"coordinates":[[1,1]],"datetimes":["2000-01-01T00:00:00+01"],"lowerInc":true,"upperInc":true},"interpolation":"Linear"}');
SELECT trgeometryFromMFJSON('{"type":"MovingPoint","sequences":[],"interpolation":"Linear"}');

-----------------------------------------------------------------------

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02}')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02)')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));(Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02)')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02],[Pose(Point(1 2),0.5)@2000-01-03, Pose(Point(3 4),0.5)@2000-01-04]}')));

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02}')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02]')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 2),0.5)@2000-01-01, Pose(Point(3 4),0.5)@2000-01-02],[Pose(Point(1 2),0.5)@2000-01-03, Pose(Point(3 4),0.5)@2000-01-04]}')));

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'XDR')));
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'XDR')));
/* Errors */
SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'ABC')));

-------------------------------------------------------------------------------
-- Output
-------------------------------------------------------------------------------

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03}');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03]');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT asText('{}'::trgeometry[]);
SELECT asText(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01']);

SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03}');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03]');
SELECT asEWKT(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT asEWKT('{}'::trgeometry[]);
SELECT asEWKT(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01']);

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03}');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03]');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03}');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03]');
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03],[Pose(Point(3 3),0.5)@2000-01-04, Pose(Point(3 3),0.5)@2000-01-05]}');

SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 0, 0, 20);
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 0, 0, -1);
SELECT asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 3, 0, 2);
SELECT asMFJSON(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 4, 0, 2);
SELECT asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 2),0.5)@2019-01-01, Pose(Point(3 4),0.5)@2019-01-02]', 1, 0, 2);

-- Additional pretty-print attribute
-- Notice that the Linux and Mac versions of json_c produce slightly different versions of the pretty-print JSON
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02}', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02]', 1, 3, 15)));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02], [Pose(Point(3 3),0.5)@2000-01-03, Pose(Point(3 3),0.5)@2000-01-04]}', 1, 3, 15)));

/* Errors */
SELECT asMFJSON(trgeometry 'SRID=123456;Polygon((1 1,2 2,3 1,1 1));Pose(Point(50.813810 4.384260),0.5)@2019-01-01 18:00:00.15+02', 4, 2);

-------------------------------------------------------------------------------

SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'NDR');
SELECT asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'XDR');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01]');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01]', 'NDR');
SELECT asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0.5)@2000-01-01]', 'XDR');

SELECT asEWKT(trgeometryFromEWKB(asEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01')));
SELECT asHexEWKB(trgeometry 'SRID=4326;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01');
SELECT asHexEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'NDR');
SELECT asHexEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'XDR');
/* Errors */
SELECT asEWKB(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0.5)@2000-01-01', 'ABCD');

-------------------------------------------------------------------------------

SELECT astext('{}'::geometry[]);

----------------------------------------------------------------------
