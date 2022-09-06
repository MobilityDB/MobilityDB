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
-- Input/output functions
-------------------------------------------------------------------------------

-- Temporal instant

SELECT asText(tgeompoint 'Point(1 1)@2012-01-01 08:00:00');
SELECT asText(tgeompoint '  Point(2 2)@2012-01-01 08:00:00  ');
SELECT asText(tgeogpoint 'Point(1 1)@2012-01-01 08:00:00');
SELECT asText(tgeogpoint '  Point(2 2) @ 2012-01-01 08:00:00  ');
/* Errors */
SELECT tgeompoint 'TRUE@2012-01-01 08:00:00';
SELECT tgeogpoint 'ABC@2012-01-01 08:00:00';
SELECT tgeompoint 'Point empty@2012-01-01 08:00:00';
SELECT tgeogpoint 'Point empty@2012-01-01 08:00:00';
SELECT tgeompoint 'Point(1 1)@2000-01-01 00:00:00+01 ,';
SELECT tgeogpoint 'Point(1 1)@2000-01-01 00:00:00+01 ,';
SELECT tgeogpoint 'Point M(1 1 1)@2000-01-01 00:00:00+01';

-------------------------------------------------------------------------------

-- Temporal instant set

SELECT asText(tgeompoint ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ');
SELECT asText(tgeompoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}');
SELECT asText(tgeogpoint ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ');
SELECT asText(tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}');
/* Errors */
SELECT tgeompoint '{Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeompoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeompoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00} xxx';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00} xxx';

-------------------------------------------------------------------------------

-- Temporal sequence

SELECT asText(tgeompoint ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ');
SELECT asText(tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]');
SELECT asText(tgeogpoint ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ');
SELECT asText(tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]');
SELECT asText(tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(3 3 3)@2001-01-03]');
/* Errors */
SELECT tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00] xxx';
SELECT tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00] xxx';

-------------------------------------------------------------------------------

-- Temporal sequence set

SELECT asText(tgeompoint '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ');
SELECT asText(tgeompoint '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}');

SELECT asText(tgeogpoint '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ');
SELECT asText(tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}');

/* Errors */
SELECT tgeompoint '{[Point(1 1)@2001-01-01 08:00:00, Point(2 2)@2001-01-01 08:05:00, Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00, Point empty@2001-01-01 09:05:00, Point(1 1)@2001-01-01 09:06:00]}';
SELECT tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00, Point(2 2)@2001-01-01 08:05:00, Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00, Point empty@2001-01-01 09:05:00, Point(1 1)@2001-01-01 09:06:00]}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]}';
SELECT tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeompoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]} xxx';
SELECT tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]} xxx';

-------------------------------------------------------------------------------
-- SRID
-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint 'SRID=4326;[Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
SELECT asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
SELECT asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, SRID=4326;Point(0 1)@2000-01-02]');

SELECT asewkt(tgeompoint 'SRID=4326;{[Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
SELECT asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
SELECT asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=4326;Point(0 1)@2000-01-02]}');

/* Errors */
SELECT tgeompoint '{SRID=5676;Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02}';
SELECT tgeompoint 'SRID=5676;{Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02}';
SELECT tgeompoint '[SRID=5676;Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeompoint 'SRID=5676;[Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeompoint '{[SRID=5676;Point(0 1)@2000-01-01], [SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeompoint 'SRID=5676;{[Point(0 1)@2000-01-01], [SRID=3812;Point(0 1)@2000-01-02]}';
SELECT tgeompoint 'SRID=5676;{Point(1 1)@2001-01-01 08:00:00,SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeompoint 'SRID=5676;[Point(1 1)@2001-01-01 08:00:00,SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeompoint 'SRID=5676;{[Point(1 1)@2001-01-01 08:00:00],[SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';

SELECT tgeogpoint 'SRID=5676;Point(1 1)@2001-01-01';
SELECT tgeogpoint '[SRID=7844;Point(0 1)@2000-01-01, SRID=4269;Point(0 1)@2000-01-02]';
SELECT tgeogpoint 'SRID=7844;{Point(1 1)@2001-01-01 08:00:00,SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeogpoint 'SRID=7844;[Point(1 1)@2001-01-01 08:00:00,SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint 'SRID=7844;{[Point(1 1)@2001-01-01 08:00:00],[SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';

-------------------------------------------------------------------------------
-- typmod
-------------------------------------------------------------------------------

SELECT format_type(oid, -1) FROM (SELECT oid FROM pg_type WHERE typname = 'tgeompoint') t;
SELECT format_type(oid, tgeompoint_typmod_in(ARRAY[cstring 'Instant','PointZ','5676']))
FROM (SELECT oid FROM pg_type WHERE typname = 'tgeompoint') t;
/* Errors */
SELECT tgeompoint_typmod_in(ARRAY[cstring 'Instant', NULL,'5676']);
SELECT tgeompoint_typmod_in(ARRAY[[cstring 'Instant'],[cstring 'PointZ'],[cstring '5676']]);
SELECT asewkt(tgeompoint('') 'Point(0 1)@2000-01-01');

SELECT asewkt(tgeompoint(Instant) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant) 'Point(0 1 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Point, 4326) 'SRID=4326;Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(PointZ, 4326) 'SRID=4326;Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, Point, 4326) 'SRID=4326;Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, PointZ, 4326) 'SRID=4326;Point(0 1 0)@2000-01-01');

SELECT asewkt(tgeompoint(Sequence) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence) '{Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Point, 4326) 'SRID=4326;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(PointZ, 4326) 'SRID=4326;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) 'SRID=4326;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) 'SRID=4326;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');

SELECT asewkt(tgeompoint(Sequence) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence) '[Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Point, 4326) 'SRID=4326;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(PointZ, 4326) 'SRID=4326;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) 'SRID=4326;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) 'SRID=4326;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');

SELECT asewkt(tgeompoint(SequenceSet) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Point, 4326) 'SRID=4326;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(PointZ, 4326) 'SRID=4326;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, Point, 4326) 'SRID=4326;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ, 4326) 'SRID=4326;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

SELECT asewkt(tgeogpoint(Instant,PointZ,4326) 'SRID=4326;Point(0 0 0)@2000-01-01');
SELECT asewkt(tgeogpoint(PointZ,4326) 'SRID=4326;Point(0 0 0)@2000-01-01');
SELECT asewkt(tgeompoint(Point) 'Point(0 0)@2000-01-01');
SELECT asewkt(tgeogpoint(PointZ) 'Point(0 0 0)@2000-01-01');

/* Errors */
SELECT tgeompoint(Instant,PointZ,5676,1234) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instan,PointZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instant,PointZZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instant,Point,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instant,Polygon,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(PointZZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Polygon,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instant,PointZZ) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Instant,Polygon) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(PointZZ) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(Polygon) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeompoint(1, 2) '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}';
/* Errors */
SELECT asewkt(tgeompoint(Instant, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, Point, 4326) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, Point, 4326) 'SRID=5434;Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(SequenceSet, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) 'Point(0 1)@2000-01-01');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, PointZ, 4326) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Instant, PointZ, 4326) 'SRID=5434;Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(SequenceSet, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) 'SRID=5434;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) 'SRID=5434;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point, 4326) 'SRID=5434;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(Sequence, PointZ, 4326) 'SRID=5434;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, Point, 4326) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, Point, 4326) 'SRID=5434;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
/* Errors */
SELECT asewkt(tgeompoint(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ, 4326) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeompoint(SequenceSet, PointZ, 4326) 'SRID=5434;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

-------------------------------------------------------------------------------/

SELECT asewkt(tgeogpoint(Instant, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(Instant, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

/* Errors */
SELECT asewkt(tgeogpoint(Instant, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(SequenceSet, Point) 'Point(0 1)@2000-01-01');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) 'Point(0 1)@2000-01-01');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(SequenceSet, Point) 'Point(0 1 0)@2000-01-01');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
/* Errors */
SELECT asewkt(tgeogpoint(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asewkt(tgeogpoint(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'));
SELECT asewkt(tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'));
-- NULL
SELECT asewkt(tgeompoint_inst(NULL, timestamptz '2012-01-01 08:00:00'));
SELECT asewkt(tgeogpoint_inst(NULL, timestamptz '2012-01-01 08:00:00'));
/* Errors */
SELECT asewkt(tgeompoint_inst(geometry 'point empty', timestamptz '2000-01-01'));
SELECT asewkt(tgeogpoint_inst(geography 'point empty', timestamptz '2000-01-01'));


SELECT asewkt(tgeompoint_discseq(ST_Point(1,1), timestampset '{2012-01-01, 2012-01-02, 2012-01-03}'));
SELECT asewkt(tgeogpoint_discseq(ST_Point(1,1), timestampset '{2012-01-01, 2012-01-02, 2012-01-03}'));
-- NULL
SELECT asewkt(tgeompoint_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}'));
SELECT asewkt(tgeompoint_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}'));

SELECT asewkt(tgeompoint_seq(ST_Point(1,1), period '[2012-01-01, 2012-01-03]'));
SELECT asewkt(tgeogpoint_seq(ST_Point(1,1), period '[2012-01-01, 2012-01-03]'));
SELECT asewkt(tgeompoint_seq(ST_Point(1,1), period '[2012-01-01, 2012-01-03]', false));
SELECT asewkt(tgeogpoint_seq(ST_Point(1,1), period '[2012-01-01, 2012-01-03]', false));
-- NULL
SELECT asewkt(tgeompoint_seq(NULL, period '[2012-01-01, 2012-01-03]'));
SELECT asewkt(tgeogpoint_seq(NULL, period '[2012-01-01, 2012-01-03]'));

SELECT asewkt(tgeompoint_seqset(ST_Point(1,1), periodset '{[2012-01-01, 2012-01-03]}'));
SELECT asewkt(tgeogpoint_seqset(ST_Point(1,1), periodset '{[2012-01-01, 2012-01-03]}'));
SELECT asewkt(tgeompoint_seqset(ST_Point(1,1), periodset '{[2012-01-01, 2012-01-03]}', false));
SELECT asewkt(tgeogpoint_seqset(ST_Point(1,1), periodset '{[2012-01-01, 2012-01-03]}', false));
-- NULL
SELECT asewkt(tgeompoint_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}'));
SELECT asewkt(tgeompoint_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}'));


-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointinst_test;
CREATE TABLE tbl_tgeompointinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tgeompoint_seq;
WITH temp AS (
  SELECT numSequences(tgeompoint_seqset_gaps(array_agg(inst ORDER BY getTime(inst)), true, 5.0, '5 minutes'::interval))
  FROM tbl_tgeompointinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tgeompointinst_test;

DROP TABLE IF EXISTS tbl_tgeogpointinst_test;
CREATE TABLE tbl_tgeogpointinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tgeogpoint_seq;
WITH temp AS (
  SELECT numSequences(tgeogpoint_seqset_gaps(array_agg(inst ORDER BY getTime(inst)), true, 5.0, '5 minutes'::interval))
  FROM tbl_tgeogpointinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tgeogpointinst_test;

-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint_discseq(ARRAY[
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeompoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));
SELECT asewkt(tgeogpoint_discseq(ARRAY[
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeogpoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));

/* Errors */
SELECT tgeompoint_discseq(ARRAY[tgeompoint 'SRID=5676;Point(1 1)@2001-01-01', 'SRID=4326;Point(2 2)@2001-01-02']);
SELECT tgeompoint_discseq(ARRAY[tgeompoint 'Point(1 1)@2001-01-01', 'Point(2 2 2)@2001-01-02']);

-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint_seq(ARRAY[
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeompoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));
SELECT asewkt(tgeogpoint_seq(ARRAY[
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeogpoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));

/* Errors */
SELECT tgeompoint_seq(ARRAY[tgeompoint 'SRID=5676;Point(1 1)@2001-01-01', 'SRID=4326;Point(2 2)@2001-01-02']);
SELECT tgeompoint_seq(ARRAY[tgeompoint 'Point(1 1)@2001-01-01', 'Point(2 2 2)@2001-01-02']);

-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint_seqset(ARRAY[
tgeompoint_seq(ARRAY[
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeompoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]),
tgeompoint_seq(ARRAY[
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 09:00:00'),
tgeompoint_inst(ST_Point(2,2), timestamptz '2012-01-01 09:10:00'),
tgeompoint_inst(ST_Point(1,1), timestamptz '2012-01-01 09:20:00')
])]));
SELECT asewkt(tgeogpoint_seqset(ARRAY[
tgeogpoint_seq(ARRAY[
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeogpoint_inst(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeogpoint_inst(ST_Point(3,3), timestamptz '2012-01-01 08:20:00')
]),
tgeogpoint_seq(ARRAY[
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 09:00:00'),
tgeogpoint_inst(ST_Point(2,2), timestamptz '2012-01-01 09:10:00'),
tgeogpoint_inst(ST_Point(1,1), timestamptz '2012-01-01 09:20:00')
])]));

/* Errors */
SELECT tgeompoint_seqset(ARRAY[tgeompoint '[SRID=5676;Point(1 1)@2001-01-01]', '[SRID=4326;Point(2 2)@2001-01-02]']);
SELECT tgeompoint_seqset(ARRAY[tgeompoint '[Point(1 1)@2001-01-01]', '[Point(2 2 2)@2001-01-02]']);

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT asewkt(tgeogpoint(tgeompoint 'Point(1 1)@2001-01-01'));
SELECT asewkt(tgeogpoint(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02}'));
SELECT asewkt(tgeogpoint(tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02]'));
SELECT asewkt(tgeogpoint(tgeompoint '{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02], [Point(2 2)@2001-01-03, Point(2 2)@2001-01-04]}'));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asewkt(tgeompoint_inst(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(tgeompoint_discseq(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(tgeompoint_discseq(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(tgeompoint_seq(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(tgeompoint_seq(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(tgeompoint_seqset(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(tgeompoint_seqset(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(tgeompoint_seqset(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(tgeompoint_seqset(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
/* Errors */
SELECT asewkt(tgeompoint_inst(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(tgeompoint_inst(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(tgeompoint_inst(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(tgeompoint_discseq(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(tgeompoint_discseq(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(tgeompoint_seq(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asewkt(tgeogpoint_inst(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(tgeogpoint_discseq(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(tgeogpoint_discseq(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(tgeogpoint_seq(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(tgeogpoint_seq(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(tgeogpoint_seqset(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(tgeogpoint_seqset(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(tgeogpoint_seqset(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(tgeogpoint_seqset(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asewkt(tgeogpoint_inst(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(tgeogpoint_inst(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(tgeogpoint_inst(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT asewkt(tgeogpoint_discseq(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(tgeogpoint_discseq(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT asewkt(tgeogpoint_seq(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

-------------------------------------------------------------------------------

SELECT asText(toLinear(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01]'));
SELECT asText(toLinear(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(2 2)@2000-01-04]'));
SELECT asText(toLinear(tgeompoint 'Interp=Stepwise;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, Point(2 2)@2000-01-04], [Point(3 3)@2000-01-05, Point(4 4)@2000-01-06]}'));

-------------------------------------------------------------------------------

SELECT asText(appendInstant(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-02'));
SELECT asText(appendInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint 'Point(1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint 'Point(1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeompoint 'Point(1 1)@2000-01-06'));
SELECT asText(appendInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5)@2000-01-02'));
SELECT asText(appendInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5)@2000-01-06'));

SELECT asText(appendInstant(tgeompoint 'Point(1 1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-02'));
SELECT asText(appendInstant(tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeompoint 'Point(1 1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(1 1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeompoint 'Interp=Stepwise;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeompoint 'Point(1 1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(1 1 1)@2000-01-06'));
SELECT asText(appendInstant(tgeompoint 'Interp=Stepwise;{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeompoint 'Point(1 1 1)@2000-01-06'));
SELECT asText(appendInstant(tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-02'));
SELECT asText(appendInstant(tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-06'));

SELECT asText(appendInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Point(3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', 'Point(3 3)@2000-01-04'));
/* Errors */
SELECT asText(appendInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeompoint 'Point(3 3)@2000-01-02'));
SELECT asText(appendInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeompoint 'Point(3 3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeompoint 'SRID=5676;Point(3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Point(3 3)@2000-01-02'));
SELECT asText(appendInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'Point(3 3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeompoint 'SRID=5676;Point(3 3)@2000-01-03'));

-------------------------------------------------------------------------------

SELECT asText(merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-02'));
SELECT asText(merge(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]'));
SELECT asText(merge(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]'));
SELECT asText(merge(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}'));
SELECT asText(merge(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}'));

SELECT asText(merge(tgeogpoint 'Point(1.5 1.5)@2000-01-01', tgeogpoint 'Point(1.5 1.5)@2000-01-02'));
SELECT asText(merge(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(1.5 1.5)@2000-01-03, Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05}'));
SELECT asText(merge(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeogpoint '{Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05}'));
SELECT asText(merge(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(1.5 1.5)@2000-01-03, Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]'));
SELECT asText(merge(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeogpoint '[Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]'));
SELECT asText(merge(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(1.5 1.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]}', tgeogpoint '{[Point(1.5 1.5)@2000-01-05, Point(2.5 2.5)@2000-01-06, Point(1.5 1.5)@2000-01-07],[Point(1.5 1.5)@2000-01-08, Point(1.5 1.5)@2000-01-09]}'));
SELECT asText(merge(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(1.5 1.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]}', tgeogpoint '{[Point(2.5 2.5)@2000-01-06, Point(1.5 1.5)@2000-01-07],[Point(1.5 1.5)@2000-01-08, Point(1.5 1.5)@2000-01-09]}'));

SELECT asText(merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asText(merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-01'));

/* Errors */
SELECT merge(tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', tgeompoint 'Point(1 1)@2000-01-02');
SELECT merge(tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(1 1 1)@2000-01-02');
SELECT merge(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04}');
SELECT merge(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}');
SELECT merge(tgeompoint 'SRID=5676;{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}');
SELECT merge(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeompoint '{Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05}');
SELECT merge(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]');
SELECT merge(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]');
SELECT merge(tgeompoint 'SRID=5676;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]');
SELECT merge(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeompoint '[Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05]');
SELECT merge(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-04, Point(2 2)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}');
SELECT merge(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(2 2)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}');
SELECT merge(tgeompoint 'SRID=5676;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(1 1)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}');
SELECT merge(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeompoint '{[Point(1 1 1)@2000-01-05, Point(1 1 1)@2000-01-06],[Point(1 1 1)@2000-01-08, Point(1 1 1)@2000-01-09]}');

-------------------------------------------------------------------------------

SELECT asText(merge(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-02']));
SELECT asText(merge(ARRAY[tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', '{Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}']));
SELECT asText(merge(ARRAY[tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', '{Point(3 3)@2000-01-03, Point(4 4)@2000-01-04}']));
SELECT asText(merge(ARRAY[tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]']));
SELECT asText(merge(ARRAY[tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]']));
SELECT asText(merge(ARRAY[tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}',
  '{[Point(4 4)@2000-01-04, Point(5 5)@2000-01-05], [Point(6 6)@2000-01-06, Point(7 7)@2000-01-07]}']));
SELECT asText(merge(ARRAY[tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', '{[Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]}']));
SELECT asText(merge(ARRAY [tgeompoint 'Point(1 1)@2000-01-01', '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']));
SELECT asText(merge(ARRAY [tgeompoint 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-01']));
SELECT asText(merge(ARRAY [tgeompoint 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-01']));

/* Errors */
SELECT merge(ARRAY [tgeompoint 'SRID=5676;Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-02']);
SELECT merge(ARRAY [tgeompoint 'Point(1 1)@2000-01-01', 'Point(1 1 1)@2000-01-02']);
SELECT merge(ARRAY [tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04}']);
SELECT merge(ARRAY [tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']);
SELECT merge(ARRAY [tgeompoint 'SRID=5676;{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']);
SELECT merge(ARRAY [tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05}']);
SELECT merge(ARRAY [tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]']);
SELECT merge(ARRAY [tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]']);
SELECT merge(ARRAY [tgeompoint 'SRID=5676;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]']);
SELECT merge(ARRAY [tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05]']);
SELECT merge(ARRAY [tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(2 2)@2000-01-04, Point(2 2)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']);
SELECT merge(ARRAY [tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(2 2)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']);
SELECT merge(ARRAY [tgeompoint 'SRID=5676;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(1 1)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']);
SELECT merge(ARRAY [tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(1 1 1)@2000-01-05, Point(1 1 1)@2000-01-06],[Point(1 1 1)@2000-01-08, Point(1 1 1)@2000-01-09]}']);

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tempSubtype(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tempSubtype(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tempSubtype(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT tempSubtype(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tempSubtype(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tempSubtype(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tempSubtype(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT memSize(tgeompoint 'Point(1 1)@2000-01-01') > 0;
SELECT memSize(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}') > 0;
SELECT memSize(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]') > 0;
SELECT memSize(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}') > 0;
SELECT memSize(tgeogpoint 'Point(1.5 1.5)@2000-01-01') > 0;
SELECT memSize(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}') > 0;
SELECT memSize(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]') > 0;
SELECT memSize(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}') > 0;

SELECT stbox(tgeompoint 'Point(1 1)@2000-01-01');
SELECT round(stbox(tgeogpoint 'Point(1.5 1.5)@2000-01-01'), 13);

SELECT st_asewkt(getValue(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT st_asewkt(getValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
/* Errors */
SELECT st_asewkt(getValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT st_asewkt(getValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_asewkt(getValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT st_asewkt(getValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT st_asewkt(getValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT st_asewkt(getValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT st_asewkt(getValues(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT st_asewkt(getValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT st_asewkt(getValues(tgeompoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}'));
SELECT st_asewkt(getValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_asewkt(getValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT st_asewkt(getValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT st_asewkt(getValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT st_asewkt(getValues(tgeogpoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}'));
SELECT st_asewkt(getValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT st_asewkt(getValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT st_asewkt(startValue(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT st_asewkt(startValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT st_asewkt(startValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_asewkt(startValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT st_asewkt(startValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT st_asewkt(startValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT st_asewkt(startValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT st_asewkt(startValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT st_asewkt(endValue(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT st_asewkt(endValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT st_asewkt(endValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT st_asewkt(endValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT st_asewkt(endValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT st_asewkt(endValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT st_asewkt(endValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT st_asewkt(endValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT getTimestamp(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
/* Errors */
SELECT getTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT getTime(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getTime(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getTime(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getTime(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getTime(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getTime(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getTime(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getTime(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT period(tgeompoint 'Point(1 1)@2000-01-01');
SELECT period(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT period(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT period(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT period(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT period(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT period(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT period(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT timespan(tgeompoint 'Point(1 1)@2000-01-01');
SELECT timespan(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT timespan(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT timespan(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT timespan(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT timespan(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT timespan(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT timespan(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT duration(tgeompoint 'Point(1 1)@2000-01-01');
SELECT duration(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT duration(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT duration(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT duration(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT duration(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT duration(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT duration(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT numSequences(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numSequences(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numSequences(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numSequences(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT numSequences(tgeompoint 'Point(1 1)@2000-01-01');
SELECT numSequences(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numSequences(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT numSequences(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');

SELECT asewkt(startSequence(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(startSequence(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(startSequence(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(startSequence(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asewkt(startSequence(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(startSequence(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(startSequence(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(startSequence(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));

SELECT asewkt(endSequence(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(endSequence(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(endSequence(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(endSequence(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asewkt(endSequence(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(endSequence(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(endSequence(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(endSequence(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));

SELECT asewkt(sequenceN(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1));
SELECT asewkt(sequenceN(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1));
SELECT asewkt(sequenceN(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1));
SELECT asewkt(sequenceN(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1));
/* Errors */
SELECT asewkt(sequenceN(tgeompoint 'Point(1 1)@2000-01-01', 1));
SELECT asewkt(sequenceN(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1));
SELECT asewkt(sequenceN(tgeogpoint 'Point(1.5 1.5)@2000-01-01', 1));
SELECT asewkt(sequenceN(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1));

SELECT asewkt(sequences(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(sequences(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(sequences(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(sequences(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(sequences(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(sequences(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(sequences(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(sequences(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT numInstants(tgeompoint 'Point(1 1)@2000-01-01');
SELECT numInstants(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numInstants(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numInstants(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numInstants(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT numInstants(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numInstants(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numInstants(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asewkt(startInstant(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(startInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(startInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(startInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(startInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(startInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(startInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(startInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asewkt(endInstant(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(endInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(endInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(endInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(endInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(endInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(endInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(endInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asewkt(instantN(tgeompoint 'Point(1 1)@2000-01-01', 1));
SELECT asewkt(instantN(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1));
SELECT asewkt(instantN(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1));
SELECT asewkt(instantN(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1));
SELECT asewkt(instantN(tgeogpoint 'Point(1.5 1.5)@2000-01-01', 1));
SELECT asewkt(instantN(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1));
SELECT asewkt(instantN(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1));
SELECT asewkt(instantN(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1));

SELECT asewkt(instants(tgeompoint 'Point(1 1)@2000-01-01'));
SELECT asewkt(instants(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asewkt(instants(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asewkt(instants(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asewkt(instants(tgeogpoint 'Point(1.5 1.5)@2000-01-01'));
SELECT asewkt(instants(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asewkt(instants(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asewkt(instants(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT numTimestamps(tgeompoint 'Point(1 1)@2000-01-01');
SELECT numTimestamps(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numTimestamps(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numTimestamps(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numTimestamps(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT numTimestamps(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numTimestamps(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numTimestamps(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT startTimestamp(tgeompoint 'Point(1 1)@2000-01-01');
SELECT startTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT startTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT startTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT startTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT startTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT startTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT startTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT endTimestamp(tgeompoint 'Point(1 1)@2000-01-01');
SELECT endTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT endTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT endTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT endTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT endTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT endTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT endTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT timestampN(tgeompoint 'Point(1 1)@2000-01-01', 1);
SELECT timestampN(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1);
SELECT timestampN(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1);
SELECT timestampN(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);
SELECT timestampN(tgeogpoint 'Point(1.5 1.5)@2000-01-01', 1);
SELECT timestampN(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1);
SELECT timestampN(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1);
SELECT timestampN(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1);

SELECT timestamps(tgeompoint 'Point(1 1)@2000-01-01');
SELECT timestamps(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT timestamps(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT timestamps(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT timestamps(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT timestamps(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT timestamps(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT timestamps(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Shift and tscale functions
-------------------------------------------------------------------------------

SELECT asewkt(shift(tgeompoint 'Point(1 1)@2000-01-01', '5 min'));
SELECT asewkt(shift(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '5 min'));
SELECT asewkt(shift(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '5 min'));
SELECT asewkt(shift(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '5 min'));
SELECT asewkt(shift(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '5 min'));
SELECT asewkt(shift(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '5 min'));
SELECT asewkt(shift(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '5 min'));
SELECT asewkt(shift(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '5 min'));

SELECT asewkt(tscale(tgeompoint 'Point(1 1)@2000-01-01', '1 day'));
SELECT asewkt(tscale(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '1 day'));
SELECT asewkt(tscale(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '1 day'));
SELECT asewkt(tscale(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '1 day'));
SELECT asewkt(tscale(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '1 day'));
SELECT asewkt(tscale(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '1 day'));
SELECT asewkt(tscale(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '1 day'));
SELECT asewkt(tscale(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '1 day'));

SELECT asewkt(shiftTscale(tgeompoint 'Point(1 1)@2000-01-01', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '1 day', '1 day'));
SELECT asewkt(shiftTscale(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '1 day', '1 day'));

/* Errors */
SELECT asewkt(tscale(tgeompoint 'Point(1 1)@2000-01-01', '0'));
SELECT asewkt(tscale(tgeompoint 'Point(1 1)@2000-01-01', '-1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tgeompoint 'Point(1 1)@2000-01-01' ?= ST_Point(1,1);
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ?= ST_Point(1,1);
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}' ?= ST_Point(2,2);
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ?= ST_Point(1,1);
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ?= ST_Point(1,1);
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ?= ST_Point(1.5,1.5);
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ?= ST_Point(1.5,1.5);
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ?= ST_Point(1.5,1.5);
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ?= ST_Point(1.5,1.5);

SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}' ?= 'Point(1.5 1.5)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]' ?= 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= 'Point(2 2)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= 'Point(1.5 1.5)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]}' ?= 'Point(0 0)';
SELECT tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= 'Point(1.5 1.5)';

SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' ?= 'Point(2 2 2)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ?= geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ?= geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ?= geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ?= geometry 'Point empty';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ?= geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ?= geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ?= geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ?= geography 'Point empty';
SELECT tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= geography 'POINT(1.49988573656168 1.5000570914792)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ?<> geometry 'Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ?<> geometry 'Point empty';

/* Errors */
SELECT tgeompoint 'Point(1 1)@2000-01-01' ?= geometry 'Linestring(1 1,2 2)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ?= geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ?= geometry 'Point(1 1 1)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' ?= geography 'Linestring(1 1,2 2)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' ?= geography 'SRID=4283;Point(1 1)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' ?= geography 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' %= ST_Point(1,1);
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' %= ST_Point(1,1);
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' %= ST_Point(1,1);
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' %= ST_Point(1,1);
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' %= ST_Point(1.5,1.5);
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' %= ST_Point(1.5,1.5);
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' %= ST_Point(1.5,1.5);
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' %= ST_Point(1.5,1.5);

SELECT tgeompoint 'Point(1 1)@2000-01-01' %= geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' %= geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' %= geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' %= geometry 'Point empty';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' %= geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' %= geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' %= geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' %= geography 'Point empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' %<> geometry 'Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' %<> geometry 'Point empty';

/* Errors */
SELECT tgeompoint 'Point(1 1)@2000-01-01' %= geometry 'Linestring(1 1,2 2)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' %= geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' %= geometry 'Point(1 1 1)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' %= geography 'Linestring(1 1,2 2)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' %= geography 'SRID=4283;Point(1 1)';
SELECT tgeogpoint 'Point(1 1)@2000-01-01' %= geography 'Point(1 1 1)';

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT asText(atValue(tgeompoint 'Point(1 1)@2000-01-01', ST_Point(1,1)));
SELECT asText(atValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1)));
SELECT asText(atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1)));
SELECT asText(atValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1)));
SELECT asText(atValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5)));
SELECT asText(atValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5)));
SELECT asText(atValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5)));
SELECT asText(atValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5)));

SELECT asText(atValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty'));
SELECT asText(atValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty'));
SELECT asText(atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty'));
SELECT asText(atValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty'));
SELECT asText(atValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Point empty'));
SELECT asText(atValue(tgeogpoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geography 'Point empty'));
SELECT asText(atValue(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geography 'Point empty'));
SELECT asText(atValue(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geography 'Point empty'));

/* Roundoff errors */
SELECT asText(atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-16, 1.0 - 1e-16)));
SELECT asText(atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-17, 1.0 - 1e-17)));
SELECT asText(atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 + 1e-16, 1.0 + 1e-16)));

/* Errors */
SELECT atValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)');
SELECT atValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT atValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1 1)');
SELECT atValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Linestring(1 1,2 2)');
SELECT atValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT atValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Point(1 1 1)');

SELECT asText(minusValue(tgeompoint 'Point(1 1)@2000-01-01', ST_Point(1,1)));
SELECT asText(minusValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1)));
SELECT asText(minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1)));
SELECT asText(minusValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1)));
SELECT asText(minusValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5)));
SELECT asText(minusValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5)));
SELECT asText(minusValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5)));
SELECT asText(minusValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5)));

SELECT asText(minusValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point empty'));
SELECT asText(minusValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty'));
SELECT asText(minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty'));
SELECT asText(minusValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty'));
SELECT asText(minusValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Point empty'));
SELECT asText(minusValue(tgeogpoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geography 'Point empty'));
SELECT asText(minusValue(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geography 'Point empty'));
SELECT asText(minusValue(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geography 'Point empty'));

/* Roundoff errors */
SELECT asText(minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-16, 1.0 - 1e-16)));
SELECT asText(minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-17, 1.0 - 1e-17)));
SELECT asText(minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 + 1e-16, 1.0 + 1e-16)));

/* Errors */
SELECT minusValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)');
SELECT minusValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT minusValue(tgeompoint 'Point(1 1)@2000-01-01', geometry 'Point(1 1 1)');
SELECT minusValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Linestring(1 1,2 2)');
SELECT minusValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT minusValue(tgeogpoint 'Point(1 1)@2000-01-01', geography 'Point(1 1 1)');

SELECT asText(atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[ST_Point(1,1)]));
SELECT asText(atValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[ST_Point(1,1)]));
SELECT asText(atValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[ST_Point(1,1)]));
SELECT asText(atValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[ST_Point(1,1)]));
SELECT asText(atValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(atValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(atValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(atValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[ST_Point(1.5,1.5)]));

SELECT asText(atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Point empty']));
SELECT asText(atValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[geometry 'Point empty']));
SELECT asText(atValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[geometry 'Point empty']));
SELECT asText(atValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[geometry 'Point empty']));
SELECT asText(atValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[geography 'Point empty']));
SELECT asText(atValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[geography 'Point empty']));
SELECT asText(atValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[geography 'Point empty']));
SELECT asText(atValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[geography 'Point empty']));

--NULL
SELECT atValues(tgeompoint '{Point(1 1)@2000-01-01}', '{}'::geometry[]);
SELECT atValues(tgeogpoint '{Point(1 1)@2000-01-01}', '{}'::geography[]);

/* Errors */
SELECT atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Linestring(1 1,2 2)']);
SELECT atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'SRID=5676;Point(1 1)']);
SELECT atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Point(1 1 1)']);
SELECT atValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'Linestring(1 1,2 2)']);
SELECT atValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'SRID=4283;Point(1 1)']);
SELECT atValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'Point(1 1 1)']);

SELECT asText(minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[ST_Point(1,1)]));
SELECT asText(minusValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[ST_Point(1,1)]));
SELECT asText(minusValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[ST_Point(1,1)]));
SELECT asText(minusValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[ST_Point(1,1)]));
SELECT asText(minusValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(minusValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(minusValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[ST_Point(1.5,1.5)]));
SELECT asText(minusValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[ST_Point(1.5,1.5)]));

SELECT asText(minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Point empty']));
SELECT asText(minusValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[geometry 'Point empty']));
SELECT asText(minusValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[geometry 'Point empty']));
SELECT asText(minusValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[geometry 'Point empty']));
SELECT asText(minusValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[geography 'Point empty']));
SELECT asText(minusValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[geography 'Point empty']));
SELECT asText(minusValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[geography 'Point empty']));
SELECT asText(minusValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[geography 'Point empty']));

SELECT asText(minusValues(tgeompoint '{Point(1 1)@2000-01-01}', '{}'::geometry[]));
SELECT asText(minusValues(tgeogpoint '{Point(1 1)@2000-01-01}', '{}'::geography[]));

/* Errors */
SELECT minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Linestring(1 1,2 2)']);
SELECT minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'SRID=5676;Point(1 1)']);
SELECT minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[geometry 'Point(1 1 1)']);
SELECT minusValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'Linestring(1 1,2 2)']);
SELECT minusValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'SRID=4283;Point(1 1)']);
SELECT minusValues(tgeogpoint 'Point(1 1)@2000-01-01', ARRAY[geography 'Point(1 1 1)']);

SELECT asText(atTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT st_astext(valueAtTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(minusTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(atTimestampSet(tgeompoint 'Point(1 1)@2000-01-01', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestampset '{2000-01-01}'));
SELECT asText(atTimestampSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestampset '{2000-01-01}'));

SELECT asText(minusTimestampSet(tgeompoint 'Point(1 1)@2000-01-01', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestampset '{2000-01-01}'));
SELECT asText(minusTimestampSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestampset '{2000-01-01}'));

SELECT asText(atPeriod(tgeompoint 'Point(1 1)@2000-01-01', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeogpoint 'Point(1.5 1.5)@2000-01-01', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', period '[2000-01-01,2000-01-02]'));
SELECT asText(atPeriod(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', period '[2000-01-01,2000-01-02]'));

SELECT asText(minusPeriod(tgeompoint 'Point(1 1)@2000-01-01', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeogpoint 'Point(1.5 1.5)@2000-01-01', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', period '[2000-01-01,2000-01-02]'));
SELECT asText(minusPeriod(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', period '[2000-01-01,2000-01-02]'));

SELECT asText(atPeriodSet(tgeompoint 'Point(1 1)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atPeriodSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}'));

SELECT asText(minusPeriodSet(tgeompoint 'Point(1 1)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusPeriodSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}'));

SELECT intersectsTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01');

SELECT intersectsTimestampSet(tgeompoint 'Point(1 1)@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestampset '{2000-01-01}');

SELECT intersectsPeriod(tgeompoint 'Point(1 1)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeogpoint 'Point(1.5 1.5)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT intersectsPeriodSet(tgeompoint 'Point(1 1)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tgeompoint 'Point(1 1)@2000-01-01' = tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' = tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' = tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' = tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' < tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' < tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' < tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' < tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' > tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' > tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' > tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' > tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' >= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' >= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' >= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' >= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' = tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' = tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' = tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' = tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

-- PostGIS changed the function of the function lwgeom_hash from version 3

SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

-------------------------------------------------------------------------------

SELECT tgeompoint_hash(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tgeompoint_hash(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompoint_hash(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompoint_hash(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT tgeogpoint_hash(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tgeogpoint_hash(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tgeogpoint_hash(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeogpoint_hash(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

------------------------------------------------------------------------------
