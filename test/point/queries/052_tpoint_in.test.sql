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

SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'Interp=Stepwise;[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'Interp=Stepwise;{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'SRID=4326;Point(1 2 3)@2000-01-01',1,2)));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}',1,2)));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]',1,2)));
SELECT asEWKT(tgeompointFromMFJSON(asMFJSON(tgeompoint 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}',1,2)));
/* Errors */
SELECT tgeompointFromMFJSON('ABC');
SELECT tgeompointFromMFJSON('{"types":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"XXX","coordinates":[1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolationss":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["XXX"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete","Linear"]}');

SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":"[1,1]","datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolations":"Discrete"}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,2,3,4],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinatess":[1,1],"datetimes":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimess":"2000-01-01T00:00:00+01","interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinatess":[[1,1]],"datetimes":["2000-01-01T00:00:00+01"],"interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":"[[1,1]]","datetimes":["2000-01-01T00:00:00+01"],"interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[],"datetimes":["2000-01-01T00:00:00+01"],"interpolations":["Discrete"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimess":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":[],"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true,"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"lower_incl":true,"upper_inc":true,"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1],[2,2]],"datetimes":["2000-01-01T00:00:00+01","2000-01-02T00:00:00+01"],"lower_inc":true,"upper_incl":true,"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","sequences":{"coordinates":[[1,1]],"datetimes":["2000-01-01T00:00:00+01"],"lower_inc":true,"upper_inc":true},"interpolations":["Linear"]}'
);
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","sequences":[],"interpolations":["Linear"]}');
SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":"2000-01-01T00:00:00+01","lower_inc":true,"upper_inc":true,"interpolations":["Linear"]}');

SELECT tgeompointFromMFJSON('{"type":"MovingPoint","coordinates":[[1,1]],"datetimes":[],"lower_inc":true,"upper_inc":true,"interpolations":["Linear"]}');

-----------------------------------------------------------------------

SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));
SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint 'Interp=Stepwise;[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeogpointFromMFJSON(asMFJSON(tgeogpoint 'Interp=Stepwise;{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

-----------------------------------------------------------------------

SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'Point(1 2)@2000-01-01')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '{Point(1 2)@2000-01-01, Point(3 4)@2000-01-02}')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02]')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '(Point(1 2)@2000-01-01, Point(3 4)@2000-01-02)')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint '{[Point(1 2)@2000-01-01, Point(3 4)@2000-01-02],[Point(1 2)@2000-01-03, Point(3 4)@2000-01-04]}')));

SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=4326;Point(1 2 3)@2000-01-01')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=4326;{Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02}')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=4326;[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02]')));
SELECT asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=4326;{[Point(1 2 3)@2000-01-01, Point(4 5 6)@2000-01-02],[Point(1 2 3)@2000-01-03, Point(4 5 6)@2000-01-04]}')));

select asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'XDR')));
select asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=5676;Point(1 1 1)@2000-01-01', 'XDR')));
/* Errors */
select asEWKT(tgeompointFromEWKB(asEWKB(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'ABC')));

----------------------------------------------------------------------
