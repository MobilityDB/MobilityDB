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
-- Input/output functions
-------------------------------------------------------------------------------

-- Temporal instant

SELECT asText(tgeometry 'Point(1 1)@2012-01-01 08:00:00');
SELECT asText(tgeometry '  Point(2 2)@2012-01-01 08:00:00  ');
SELECT asText(tgeography 'Point(1 1)@2012-01-01 08:00:00');
SELECT asText(tgeography '  Point(2 2) @ 2012-01-01 08:00:00  ');
SELECT tgeography 'Point M(1 1 1)@2000-01-01 00:00:00+01';
/* Errors */
SELECT tgeometry 'TRUE@2012-01-01 08:00:00';
SELECT tgeography 'ABC@2012-01-01 08:00:00';
SELECT tgeometry 'Point empty@2012-01-01 08:00:00';
SELECT tgeography 'Point empty@2012-01-01 08:00:00';
SELECT tgeometry 'Point(1 1)@2000-01-01 00:00:00+01 ,';
SELECT tgeography 'Point(1 1)@2000-01-01 00:00:00+01 ,';
SELECT tgeography 'SRID=5676;[Point(1 1)@2000-01-01 00:00:00+01]';

-------------------------------------------------------------------------------

-- Temporal instant set

SELECT asText(tgeometry ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ');
SELECT asText(tgeometry '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}');
SELECT asText(tgeography ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ');
SELECT asText(tgeography '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}');
/* Errors */
SELECT tgeometry '{Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeography '{Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeometry '{Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeography '{Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeometry '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00} xxx';
SELECT tgeography '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00} xxx';

-------------------------------------------------------------------------------

-- Temporal sequence

SELECT asText(tgeometry ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ');
SELECT asText(tgeometry '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]');
SELECT asText(tgeography ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ');
SELECT asText(tgeography '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]');
SELECT asText(tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(3 3 3)@2001-01-03]');
/* Errors */
SELECT tgeometry '[Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography '[Point(1 1)@2001-01-01 08:00:00,Point empty@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeometry '[Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography '[Point(1 1)@2001-01-01 08:00:00,Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeometry '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeography '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeometry '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00] xxx';
SELECT tgeography '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00] xxx';

-------------------------------------------------------------------------------

-- Temporal sequence set

SELECT asText(tgeometry '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ');
SELECT asText(tgeometry '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}');

SELECT asText(tgeography '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ');
SELECT asText(tgeography '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}');

/* Errors */
SELECT tgeometry '{[Point(1 1)@2001-01-01 08:00:00, Point(2 2)@2001-01-01 08:05:00, Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00, Point empty@2001-01-01 09:05:00, Point(1 1)@2001-01-01 09:06:00]}';
SELECT tgeography '{[Point(1 1)@2001-01-01 08:00:00, Point(2 2)@2001-01-01 08:05:00, Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00, Point empty@2001-01-01 09:05:00, Point(1 1)@2001-01-01 09:06:00]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]}';
SELECT tgeography '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeometry '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]} xxx';
SELECT tgeography '{[Point(1 1)@2001-01-01 08:00:00],[Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]} xxx';

-------------------------------------------------------------------------------
-- SRID
-------------------------------------------------------------------------------

SELECT asEWKT(tgeometry 'SRID=4326;[Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
SELECT asEWKT(tgeometry '[SRID=4326;Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
SELECT asEWKT(tgeometry '[SRID=4326;Point(0 1)@2000-01-01, SRID=4326;Point(0 1)@2000-01-02]');

SELECT asEWKT(tgeometry 'SRID=4326;{[Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
SELECT asEWKT(tgeometry '{[SRID=4326;Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
SELECT asEWKT(tgeometry '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=4326;Point(0 1)@2000-01-02]}');

/* Errors */
SELECT tgeometry '{SRID=5676;Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02}';
SELECT tgeometry 'SRID=5676;{Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02}';
SELECT tgeometry '[SRID=5676;Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeometry 'SRID=5676;[Point(0 1)@2000-01-01, SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeometry '{[SRID=5676;Point(0 1)@2000-01-01], [SRID=3812;Point(0 1)@2000-01-02]';
SELECT tgeometry 'SRID=5676;{[Point(0 1)@2000-01-01], [SRID=3812;Point(0 1)@2000-01-02]}';
SELECT tgeometry 'SRID=5676;{Point(1 1)@2001-01-01 08:00:00,SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeometry 'SRID=5676;[Point(1 1)@2001-01-01 08:00:00,SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeometry 'SRID=5676;{[Point(1 1)@2001-01-01 08:00:00],[SRID=3812;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';

SELECT tgeography 'SRID=5676;Point(1 1)@2001-01-01';
SELECT tgeography '[SRID=7844;Point(0 1)@2000-01-01, SRID=4269;Point(0 1)@2000-01-02]';
SELECT tgeography 'SRID=7844;{Point(1 1)@2001-01-01 08:00:00,SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeography 'SRID=7844;[Point(1 1)@2001-01-01 08:00:00,SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeography 'SRID=7844;{[Point(1 1)@2001-01-01 08:00:00],[SRID=4269;Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';

-------------------------------------------------------------------------------
-- typmod
-------------------------------------------------------------------------------

SELECT format_type(oid, -1) FROM (SELECT oid FROM pg_type WHERE typname = 'tgeometry') t;
SELECT format_type(oid, tgeometry_typmod_in(ARRAY[cstring 'Instant','PointZ','5676']))
FROM (SELECT oid FROM pg_type WHERE typname = 'tgeometry') t;
/* Errors */
SELECT tgeometry_typmod_in(ARRAY[cstring 'Instant', NULL,'5676']);
SELECT tgeometry_typmod_in(ARRAY[[cstring 'Instant'],[cstring 'PointZ'],[cstring '5676']]);
SELECT asEWKT(tgeometry('') 'Point(0 1)@2000-01-01');

SELECT asEWKT(tgeometry(Instant) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant) 'Point(0 1 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Point, 4326) 'SRID=4326;Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(PointZ, 4326) 'SRID=4326;Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, Point, 4326) 'SRID=4326;Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, PointZ, 4326) 'SRID=4326;Point(0 1 0)@2000-01-01');

SELECT asEWKT(tgeometry(Sequence) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence) '{Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Point, 4326) 'SRID=4326;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(PointZ, 4326) 'SRID=4326;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) 'SRID=4326;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) 'SRID=4326;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');

SELECT asEWKT(tgeometry(Sequence) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence) '[Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Point, 4326) 'SRID=4326;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(PointZ, 4326) 'SRID=4326;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) 'SRID=4326;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) 'SRID=4326;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');

SELECT asEWKT(tgeometry(SequenceSet) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Point, 4326) 'SRID=4326;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(PointZ, 4326) 'SRID=4326;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, Point, 4326) 'SRID=4326;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02], [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ, 4326) 'SRID=4326;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02], [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

SELECT asEWKT(tgeography(Instant,PointZ,4326) 'SRID=4326;Point(0 0 0)@2000-01-01');
SELECT asEWKT(tgeography(PointZ,4326) 'SRID=4326;Point(0 0 0)@2000-01-01');
SELECT asEWKT(tgeometry(Point) 'Point(0 0)@2000-01-01');
SELECT asEWKT(tgeography(PointZ) 'Point(0 0 0)@2000-01-01');

/* Errors */
SELECT tgeometry(Instant,PointZ,5676,1234) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instan,PointZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instant,PointZZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instant,Point,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instant,Polygon,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(PointZZ,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Polygon,5676) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instant,PointZZ) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Instant,Polygon) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(PointZZ) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(Polygon) 'SRID=5676;Point(0 0 0)@2000-01-01';
SELECT tgeometry(1, 2) '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}';
/* Errors */
SELECT asEWKT(tgeometry(Instant, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, Point, 4326) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, Point, 4326) 'SRID=5434;Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(SequenceSet, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) 'Point(0 1)@2000-01-01');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, PointZ, 4326) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Instant, PointZ, 4326) 'SRID=5434;Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(SequenceSet, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) 'SRID=5434;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) 'SRID=5434;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point, 4326) 'SRID=5434;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(Sequence, PointZ, 4326) 'SRID=5434;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, Point, 4326) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, Point, 4326) 'SRID=5434;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
/* Errors */
SELECT asEWKT(tgeometry(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ, 4326) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeometry(SequenceSet, PointZ, 4326) 'SRID=5434;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

-----------------------------------------------------------------------------/

SELECT asEWKT(tgeography(Instant, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(Instant, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

/* Errors */
SELECT asEWKT(tgeography(Instant, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, PointZ) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(SequenceSet, Point) 'Point(0 1)@2000-01-01');
SELECT asEWKT(tgeography(SequenceSet, PointZ) 'Point(0 1)@2000-01-01');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(Sequence, PointZ) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(SequenceSet, Point) 'Point(0 1 0)@2000-01-01');
SELECT asEWKT(tgeography(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
  [Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}');
/* Errors */
SELECT asEWKT(tgeography(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');
SELECT asEWKT(tgeography(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
  [Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}');

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

SELECT asEWKT(tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'));
SELECT asEWKT(tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'));
-- NULL
SELECT asEWKT(tgeometry(NULL, timestamptz '2012-01-01 08:00:00'));
SELECT asEWKT(tgeography(NULL, timestamptz '2012-01-01 08:00:00'));
/* Errors */
SELECT asEWKT(tgeometry(geometry 'point empty', timestamptz '2000-01-01'));
SELECT asEWKT(tgeography(geography 'point empty', timestamptz '2000-01-01'));


SELECT asEWKT(tgeometry(ST_Point(1,1), tstzset '{2012-01-01, 2012-01-02, 2012-01-03}'));
SELECT asEWKT(tgeography(ST_Point(1,1), tstzset '{2012-01-01, 2012-01-02, 2012-01-03}'));
-- NULL
SELECT asEWKT(tgeometry(NULL, tstzset '{2012-01-01, 2012-01-02, 2012-01-03}'));
SELECT asEWKT(tgeometry(NULL, tstzset '{2012-01-01, 2012-01-02, 2012-01-03}'));

SELECT asEWKT(tgeometry(ST_Point(1,1), tstzspan '[2012-01-01, 2012-01-03]'));
SELECT asEWKT(tgeography(ST_Point(1,1), tstzspan '[2012-01-01, 2012-01-03]'));
SELECT asEWKT(tgeometry(ST_Point(1,1), tstzspan '[2012-01-01, 2012-01-03]', 'step'));
SELECT asEWKT(tgeography(ST_Point(1,1), tstzspan '[2012-01-01, 2012-01-03]', 'step'));
-- NULL
SELECT asEWKT(tgeometry(NULL, tstzspan '[2012-01-01, 2012-01-03]'));
SELECT asEWKT(tgeography(NULL, tstzspan '[2012-01-01, 2012-01-03]'));

SELECT asEWKT(tgeometry(ST_Point(1,1), tstzspanset '{[2012-01-01, 2012-01-03]}'));
SELECT asEWKT(tgeography(ST_Point(1,1), tstzspanset '{[2012-01-01, 2012-01-03]}'));
SELECT asEWKT(tgeometry(ST_Point(1,1), tstzspanset '{[2012-01-01, 2012-01-03]}', 'step'));
SELECT asEWKT(tgeography(ST_Point(1,1), tstzspanset '{[2012-01-01, 2012-01-03]}', 'step'));
-- NULL
SELECT asEWKT(tgeometry(NULL, tstzspanset '{[2012-01-01, 2012-01-03]}'));
SELECT asEWKT(tgeometry(NULL, tstzspanset '{[2012-01-01, 2012-01-03]}'));


-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeometryinst_test;
CREATE TABLE tbl_tgeometryinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tgeometry_seq;
WITH temp AS (
  SELECT numSequences(tgeometrySeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0))
  FROM tbl_tgeometryinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tgeometryinst_test;

DROP TABLE IF EXISTS tbl_tgeographyinst_test;
CREATE TABLE tbl_tgeographyinst_test AS SELECT k, unnest(instants(seq)) AS inst FROM tbl_tgeography_seq;
WITH temp AS (
  SELECT numSequences(tgeographySeqSetGaps(array_agg(inst ORDER BY getTime(inst)), '5 minutes'::interval, 5.0))
  FROM tbl_tgeographyinst_test GROUP BY k )
SELECT MAX(numSequences) FROM temp;
DROP TABLE tbl_tgeographyinst_test;

-------------------------------------------------------------------------------

SELECT asEWKT(tgeometrySeq(ARRAY[
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeometry(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
], 'discrete'));
SELECT asEWKT(tgeographySeq(ARRAY[
tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeography(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
], 'discrete'));

/* Errors */
SELECT tgeometrySeq(ARRAY[tgeometry 'SRID=5676;Point(1 1)@2001-01-01', 'SRID=4326;Point(2 2)@2001-01-02'], 'discrete');
SELECT tgeometrySeq(ARRAY[tgeometry 'Point(1 1)@2001-01-01', 'Point(2 2 2)@2001-01-02'], 'discrete');

-------------------------------------------------------------------------------

SELECT asEWKT(tgeometrySeq(ARRAY[
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeometry(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));
SELECT asEWKT(tgeographySeq(ARRAY[
tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeography(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]));

/* Errors */
SELECT tgeometrySeq(ARRAY[tgeometry 'SRID=5676;Point(1 1)@2001-01-01', 'SRID=4326;Point(2 2)@2001-01-02']);
SELECT tgeometrySeq(ARRAY[tgeometry 'Point(1 1)@2001-01-01', 'Point(2 2 2)@2001-01-02']);

-------------------------------------------------------------------------------

SELECT asEWKT(tgeometrySeqSet(ARRAY[
tgeometrySeq(ARRAY[
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeometry(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 08:20:00')
]),
tgeometrySeq(ARRAY[
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 09:00:00'),
tgeometry(ST_Point(2,2), timestamptz '2012-01-01 09:10:00'),
tgeometry(ST_Point(1,1), timestamptz '2012-01-01 09:20:00')
])]));
SELECT asEWKT(tgeographySeqSet(ARRAY[
tgeographySeq(ARRAY[
tgeography(ST_Point(1,1), timestamptz '2012-01-01 08:00:00'),
tgeography(ST_Point(2,2), timestamptz '2012-01-01 08:10:00'),
tgeography(ST_Point(3,3), timestamptz '2012-01-01 08:20:00')
]),
tgeographySeq(ARRAY[
tgeography(ST_Point(1,1), timestamptz '2012-01-01 09:00:00'),
tgeography(ST_Point(2,2), timestamptz '2012-01-01 09:10:00'),
tgeography(ST_Point(1,1), timestamptz '2012-01-01 09:20:00')
])]));

/* Errors */
SELECT tgeometrySeqSet(ARRAY[tgeometry '[SRID=5676;Point(1 1)@2001-01-01]', '[SRID=4326;Point(2 2)@2001-01-02]']);
SELECT tgeometrySeqSet(ARRAY[tgeometry '[Point(1 1)@2001-01-01]', '[Point(2 2 2)@2001-01-02]']);

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT asEWKT(tgeography(tgeometry 'Point(1 1)@2001-01-01'));
SELECT asEWKT(tgeography(tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02}'));
SELECT asEWKT(tgeography(tgeometry '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02]'));
SELECT asEWKT(tgeography(tgeometry '{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02], [Point(2 2)@2001-01-03, Point(2 2)@2001-01-04]}'));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asEWKT(tgeometryInst(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(setInterp(tgeometry 'Point(1 1)@2000-01-01', 'discrete'));
SELECT asEWKT(setInterp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 'discrete'));
SELECT asEWKT(tgeometrySeq(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(tgeometrySeq(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(tgeometrySeqSet(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(tgeometrySeqSet(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(tgeometrySeqSet(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(tgeometrySeqSet(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
/* Errors */
SELECT asEWKT(tgeometryInst(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(tgeometryInst(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(tgeometryInst(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(setInterp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 'discrete'));
SELECT asEWKT(setInterp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 'discrete'));
SELECT asEWKT(tgeometrySeq(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));

SELECT asEWKT(tgeographyInst(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(setInterp(tgeography 'Point(1.5 1.5)@2000-01-01', 'discrete'));
SELECT asEWKT(setInterp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 'discrete'));
SELECT asEWKT(tgeographySeq(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(tgeographySeq(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(tgeographySeqSet(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(tgeographySeqSet(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(tgeographySeqSet(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(tgeographySeqSet(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asEWKT(tgeographyInst(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(tgeographyInst(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(tgeographyInst(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
SELECT asEWKT(setInterp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 'discrete'));
SELECT asEWKT(setInterp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 'discrete'));
SELECT asEWKT(tgeographySeq(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(appendInstant(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-02'));
SELECT asText(appendInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry 'Point(1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry 'Point(1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tgeometry 'Point(1 1)@2000-01-06'));
SELECT asText(appendInstant(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography 'Point(1.5 1.5)@2000-01-02'));
SELECT asText(appendInstant(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography 'Point(1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography 'Point(1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tgeography 'Point(1.5 1.5)@2000-01-06'));

SELECT asText(appendInstant(tgeometry 'Point(1 1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-02'));
SELECT asText(appendInstant(tgeometry '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}', tgeometry 'Point(1 1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeometry '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]', tgeometry 'Point(1 1 1)@2000-01-04'));
SELECT asText(appendInstant(tgeometry '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}', tgeometry 'Point(1 1 1)@2000-01-06'));
SELECT asText(appendInstant(tgeography 'Point(1.5 1.5 1.5)@2000-01-01', tgeography 'Point(1.5 1.5 1.5)@2000-01-02'));
SELECT asText(appendInstant(tgeography '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}', tgeography 'Point(1.5 1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeography '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]', tgeography 'Point(1.5 1.5 1.5)@2000-01-04'));
SELECT asText(appendInstant(tgeography '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}', tgeography 'Point(1.5 1.5 1.5)@2000-01-06'));

SELECT asText(appendInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry 'Point(3 3)@2000-01-03'));
/* Errors */
SELECT asText(appendInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeometry 'Point(3 3)@2000-01-02'));
SELECT asText(appendInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeometry 'Point(3 3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', tgeometry 'SRID=5676;Point(3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry 'Point(3 3)@2000-01-02'));
SELECT asText(appendInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry 'Point(3 3 3)@2000-01-03'));
SELECT asText(appendInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', tgeometry 'SRID=5676;Point(3 3)@2000-01-03'));

-------------------------------------------------------------------------------

SELECT asText(merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-02'));
SELECT asText(merge(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]'));
SELECT asText(merge(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]'));
SELECT asText(merge(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}'));
SELECT asText(merge(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}'));

SELECT asText(merge(tgeography 'Point(1.5 1.5)@2000-01-01', tgeography 'Point(1.5 1.5)@2000-01-02'));
SELECT asText(merge(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography '{Point(1.5 1.5)@2000-01-03, Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05}'));
SELECT asText(merge(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tgeography '{Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05}'));
SELECT asText(merge(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography '[Point(1.5 1.5)@2000-01-03, Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]'));
SELECT asText(merge(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tgeography '[Point(2.5 2.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]'));
SELECT asText(merge(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(1.5 1.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]}', tgeography '{[Point(1.5 1.5)@2000-01-05, Point(2.5 2.5)@2000-01-06, Point(1.5 1.5)@2000-01-07],[Point(1.5 1.5)@2000-01-08, Point(1.5 1.5)@2000-01-09]}'));
SELECT asText(merge(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(1.5 1.5)@2000-01-04, Point(1.5 1.5)@2000-01-05]}', tgeography '{[Point(2.5 2.5)@2000-01-06, Point(1.5 1.5)@2000-01-07],[Point(1.5 1.5)@2000-01-08, Point(1.5 1.5)@2000-01-09]}'));

SELECT asText(merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01'));
SELECT asText(merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-01'));

-- Different values at the same timestamp
SELECT asText(merge(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04}'));
SELECT asText(merge(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]'));
SELECT asText(merge(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}'));
SELECT asText(merge(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}'));

/* Errors */
SELECT merge(tgeometry 'SRID=5676;Point(1 1)@2000-01-01', tgeometry 'Point(1 1)@2000-01-02');
SELECT merge(tgeometry 'Point(1 1)@2000-01-01', tgeometry 'Point(1 1 1)@2000-01-02');
SELECT merge(tgeometry 'SRID=5676;{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}');
SELECT merge(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tgeometry '{Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05}');
SELECT merge(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]');

SELECT merge(tgeometry 'SRID=5676;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]');
SELECT merge(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tgeometry '[Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05]');
SELECT merge(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(2 2)@2000-01-04, Point(2 2)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}');

SELECT merge(tgeometry 'SRID=5676;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(1 1)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}');
SELECT merge(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', tgeometry '{[Point(1 1 1)@2000-01-05, Point(1 1 1)@2000-01-06],[Point(1 1 1)@2000-01-08, Point(1 1 1)@2000-01-09]}');

-------------------------------------------------------------------------------

SELECT asText(merge(ARRAY[tgeometry 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-02']));
SELECT asText(merge(ARRAY[tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', '{Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}']));
SELECT asText(merge(ARRAY[tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}', '{Point(3 3)@2000-01-03, Point(4 4)@2000-01-04}']));
SELECT asText(merge(ARRAY[tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]']));
SELECT asText(merge(ARRAY[tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]']));
SELECT asText(merge(ARRAY[tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}',
  '{[Point(4 4)@2000-01-04, Point(5 5)@2000-01-05], [Point(6 6)@2000-01-06, Point(7 7)@2000-01-07]}']));
SELECT asText(merge(ARRAY[tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]}', '{[Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]}']));
SELECT asText(merge(ARRAY [tgeometry 'Point(1 1)@2000-01-01', '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']));
SELECT asText(merge(ARRAY [tgeometry 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-01']));
SELECT asText(merge(ARRAY [tgeometry 'Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-01']));

-- Different values at the same timestamp
SELECT asText(merge(ARRAY [tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(2 2)@2000-01-05, Point(2 2)@2000-01-06, Point(1 1)@2000-01-07],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']));
SELECT asText(merge(ARRAY [tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04}']));
SELECT asText(merge(ARRAY [tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']));
SELECT asText(merge(ARRAY [tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(2 2)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]']));

/* Errors */
SELECT merge(ARRAY [tgeometry 'SRID=5676;Point(1 1)@2000-01-01', 'Point(1 1)@2000-01-02']);
SELECT merge(ARRAY [tgeometry 'Point(1 1)@2000-01-01', 'Point(1 1 1)@2000-01-02']);
SELECT merge(ARRAY [tgeometry 'SRID=5676;{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05}']);
SELECT merge(ARRAY [tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '{Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05}']);
SELECT merge(ARRAY [tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(2 2)@2000-01-02, Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]']);
SELECT merge(ARRAY [tgeometry 'SRID=5676;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(1 1)@2000-01-03, Point(2 2)@2000-01-04, Point(1 1)@2000-01-05]']);
SELECT merge(ARRAY [tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '[Point(1 1 1)@2000-01-03, Point(2 2 2)@2000-01-04, Point(1 1 1)@2000-01-05]']);
SELECT merge(ARRAY [tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(2 2)@2000-01-04, Point(2 2)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']);
SELECT merge(ARRAY [tgeometry 'SRID=5676;{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(1 1)@2000-01-05, Point(1 1)@2000-01-06],[Point(1 1)@2000-01-08, Point(1 1)@2000-01-09]}']);
SELECT merge(ARRAY [tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(1 1)@2000-01-04, Point(1 1)@2000-01-05]}', '{[Point(1 1 1)@2000-01-05, Point(1 1 1)@2000-01-06],[Point(1 1 1)@2000-01-08, Point(1 1 1)@2000-01-09]}']);

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tgeometry 'Point(1 1)@2000-01-01');
SELECT tempSubtype(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tempSubtype(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tempSubtype(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT tempSubtype(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT tempSubtype(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tempSubtype(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tempSubtype(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT memSize(tgeometry 'Point(1 1)@2000-01-01') > 0;
SELECT memSize(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}') > 0;
SELECT memSize(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]') > 0;
SELECT memSize(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}') > 0;
SELECT memSize(tgeography 'Point(1.5 1.5)@2000-01-01') > 0;
SELECT memSize(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}') > 0;
SELECT memSize(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]') > 0;
SELECT memSize(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}') > 0;

SELECT stbox(tgeometry 'Point(1 1)@2000-01-01');
SELECT round(stbox(tgeography 'Point(1.5 1.5)@2000-01-01'), 13);

SELECT ST_AsEWKT(getValue(tgeometry 'Point(1 1)@2000-01-01'));
SELECT ST_AsEWKT(getValue(tgeography 'Point(1.5 1.5)@2000-01-01'));
/* Errors */
SELECT ST_AsEWKT(getValue(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsEWKT(getValue(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsEWKT(getValue(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsEWKT(getValue(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsEWKT(getValue(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsEWKT(getValue(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asEWKT(valueSet(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(valueSet(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(valueSet(tgeometry '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}'));
SELECT asEWKT(valueSet(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(valueSet(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(valueSet(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(valueSet(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(valueSet(tgeography '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}'));
SELECT asEWKT(valueSet(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(valueSet(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsEWKT(startValue(tgeometry 'Point(1 1)@2000-01-01'));
SELECT ST_AsEWKT(startValue(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsEWKT(startValue(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsEWKT(startValue(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsEWKT(startValue(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsEWKT(startValue(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsEWKT(startValue(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsEWKT(startValue(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsEWKT(endValue(tgeometry 'Point(1 1)@2000-01-01'));
SELECT ST_AsEWKT(endValue(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT ST_AsEWKT(endValue(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT ST_AsEWKT(endValue(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT ST_AsEWKT(endValue(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT ST_AsEWKT(endValue(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT ST_AsEWKT(endValue(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT ST_AsEWKT(endValue(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT ST_AsEWKT(valueN(tgeometry 'Point(1 1)@2000-01-01', 1));
SELECT ST_AsEWKT(valueN(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1));
SELECT ST_AsEWKT(valueN(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1));
SELECT ST_AsEWKT(valueN(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1));
SELECT ST_AsEWKT(valueN(tgeography 'Point(1.5 1.5)@2000-01-01', 1));
SELECT ST_AsEWKT(valueN(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1));
SELECT ST_AsEWKT(valueN(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1));
SELECT ST_AsEWKT(valueN(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1));

SELECT getTimestamp(tgeometry 'Point(1 1)@2000-01-01');
SELECT getTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01');
/* Errors */
SELECT getTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT getTime(tgeometry 'Point(1 1)@2000-01-01');
SELECT getTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getTime(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT getTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT timeSpan(tgeometry 'Point(1 1)@2000-01-01');
SELECT timeSpan(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT timeSpan(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT timeSpan(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT timeSpan(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT timeSpan(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT timeSpan(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT timeSpan(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT duration(tgeometry 'Point(1 1)@2000-01-01', true);
SELECT duration(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', true);
SELECT duration(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', true);
SELECT duration(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', true);
SELECT duration(tgeography 'Point(1.5 1.5)@2000-01-01', true);
SELECT duration(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', true);
SELECT duration(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', true);
SELECT duration(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', true);

SELECT duration(tgeometry 'Point(1 1)@2000-01-01');
SELECT duration(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT duration(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT duration(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT duration(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT duration(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT duration(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT duration(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT numSequences(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numSequences(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numSequences(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numSequences(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT numSequences(tgeometry 'Point(1 1)@2000-01-01');
SELECT numSequences(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numSequences(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT numSequences(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');

SELECT asEWKT(startSequence(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(startSequence(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(startSequence(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(startSequence(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asEWKT(startSequence(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(startSequence(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(startSequence(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(startSequence(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));

SELECT asEWKT(endSequence(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(endSequence(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(endSequence(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(endSequence(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asEWKT(endSequence(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(endSequence(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(endSequence(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(endSequence(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));

SELECT asEWKT(sequenceN(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1));
SELECT asEWKT(sequenceN(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1));
SELECT asEWKT(sequenceN(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1));
SELECT asEWKT(sequenceN(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1));
/* Errors */
SELECT asEWKT(sequenceN(tgeometry 'Point(1 1)@2000-01-01', 1));
SELECT asEWKT(sequenceN(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1));
SELECT asEWKT(sequenceN(tgeography 'Point(1.5 1.5)@2000-01-01', 1));
SELECT asEWKT(sequenceN(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1));

SELECT asEWKT(sequences(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(sequences(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(sequences(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(sequences(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));
/* Errors */
SELECT asEWKT(sequences(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(sequences(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(sequences(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(sequences(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));

SELECT numInstants(tgeometry 'Point(1 1)@2000-01-01');
SELECT numInstants(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numInstants(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numInstants(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numInstants(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT numInstants(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numInstants(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numInstants(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT asEWKT(startInstant(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(startInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(startInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(startInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(startInstant(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(startInstant(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(startInstant(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(startInstant(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asEWKT(endInstant(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(endInstant(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(endInstant(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(endInstant(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(endInstant(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(endInstant(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(endInstant(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(endInstant(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT asEWKT(instantN(tgeometry 'Point(1 1)@2000-01-01', 1));
SELECT asEWKT(instantN(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1));
SELECT asEWKT(instantN(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1));
SELECT asEWKT(instantN(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1));
SELECT asEWKT(instantN(tgeography 'Point(1.5 1.5)@2000-01-01', 1));
SELECT asEWKT(instantN(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1));
SELECT asEWKT(instantN(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1));
SELECT asEWKT(instantN(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1));

SELECT asEWKT(instants(tgeometry 'Point(1 1)@2000-01-01'));
SELECT asEWKT(instants(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'));
SELECT asEWKT(instants(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'));
SELECT asEWKT(instants(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'));
SELECT asEWKT(instants(tgeography 'Point(1.5 1.5)@2000-01-01'));
SELECT asEWKT(instants(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}'));
SELECT asEWKT(instants(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]'));
SELECT asEWKT(instants(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}'));

SELECT numTimestamps(tgeometry 'Point(1 1)@2000-01-01');
SELECT numTimestamps(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numTimestamps(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numTimestamps(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numTimestamps(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT numTimestamps(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numTimestamps(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numTimestamps(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT startTimestamp(tgeometry 'Point(1 1)@2000-01-01');
SELECT startTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT startTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT startTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT startTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT startTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT startTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT startTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT endTimestamp(tgeometry 'Point(1 1)@2000-01-01');
SELECT endTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT endTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT endTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT endTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT endTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT endTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT endTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT timestampN(tgeometry 'Point(1 1)@2000-01-01', 1);
SELECT timestampN(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1);
SELECT timestampN(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1);
SELECT timestampN(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);
SELECT timestampN(tgeography 'Point(1.5 1.5)@2000-01-01', 1);
SELECT timestampN(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1);
SELECT timestampN(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1);
SELECT timestampN(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1);

SELECT timestamps(tgeometry 'Point(1 1)@2000-01-01');
SELECT timestamps(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT timestamps(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT timestamps(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT timestamps(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT timestamps(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT timestamps(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT timestamps(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT asEWKT(shiftTime(tgeometry 'Point(1 1)@2000-01-01', '5 min'));
SELECT asEWKT(shiftTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '5 min'));
SELECT asEWKT(shiftTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '5 min'));
SELECT asEWKT(shiftTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '5 min'));
SELECT asEWKT(shiftTime(tgeography 'Point(1.5 1.5)@2000-01-01', '5 min'));
SELECT asEWKT(shiftTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '5 min'));
SELECT asEWKT(shiftTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '5 min'));
SELECT asEWKT(shiftTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '5 min'));

SELECT asEWKT(scaleTime(tgeometry 'Point(1 1)@2000-01-01', '1 day'));
SELECT asEWKT(scaleTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '1 day'));
SELECT asEWKT(scaleTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '1 day'));
SELECT asEWKT(scaleTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '1 day'));
SELECT asEWKT(scaleTime(tgeography 'Point(1.5 1.5)@2000-01-01', '1 day'));
SELECT asEWKT(scaleTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '1 day'));
SELECT asEWKT(scaleTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '1 day'));
SELECT asEWKT(scaleTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '1 day'));

SELECT asEWKT(shiftScaleTime(tgeometry 'Point(1 1)@2000-01-01', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeography 'Point(1.5 1.5)@2000-01-01', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '1 day', '1 day'));
SELECT asEWKT(shiftScaleTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '1 day', '1 day'));

/* Errors */
SELECT asEWKT(scaleTime(tgeometry 'Point(1 1)@2000-01-01', '0'));
SELECT asEWKT(scaleTime(tgeometry 'Point(1 1)@2000-01-01', '-1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2000-01-01' ?= ST_Point(1,1);
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ?= ST_Point(1,1);
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(1 1)@2000-01-02}' ?= ST_Point(2,2);
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ?= ST_Point(1,1);
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ?= ST_Point(1,1);
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' ?= ST_Point(1.5,1.5);
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ?= ST_Point(1.5,1.5);
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ?= ST_Point(1.5,1.5);
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ?= ST_Point(1.5,1.5);

SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}' ?= geometry 'Point(1.5 1.5)';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(1 1)@2000-01-02]' ?= geometry 'Point(1 1)';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= geometry 'Point(2 2)';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= geometry 'Point(1.5 1.5)';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02],[Point(2 2)@2000-01-03, Point(1 1)@2000-01-04]}' ?= geometry 'Point(0 0)';

SELECT tgeometry '[Point(1 1 1)@2000-01-01, Point(3 3 3)@2000-01-03]' ?= geometry 'Point(2 2 2)';

SELECT tgeometry 'Point(1 1)@2000-01-01' ?= geometry 'Point empty';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ?= geometry 'Point empty';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ?= geometry 'Point empty';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ?= geometry 'Point empty';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' ?= geography 'Point empty';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ?= geography 'Point empty';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ?= geography 'Point empty';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ?= geography 'Point empty';
SELECT tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]' ?= geography 'POINT(1.49988573656168 1.5000570914792)';

SELECT tgeometry 'Point(1 1)@2000-01-01' ?<> geometry 'Point(1 1)';
SELECT tgeometry 'Point(1 1)@2000-01-01' ?<> geometry 'Point empty';

SELECT tgeometry 'Point(1 1)@2000-01-01' ?= geometry 'Linestring(1 1,2 2)';
SELECT tgeography 'Point(1 1)@2000-01-01' ?= geography 'Linestring(1 1,2 2)';

/* Errors */
SELECT tgeometry 'Point(1 1)@2000-01-01' ?= geometry 'SRID=5676;Point(1 1)';
SELECT tgeometry 'Point(1 1)@2000-01-01' ?= geometry 'Point(1 1 1)';
SELECT tgeography 'Point(1 1)@2000-01-01' ?= geography 'SRID=4283;Point(1 1)';
SELECT tgeography 'Point(1 1)@2000-01-01' ?= geography 'Point(1 1 1)';

SELECT tgeometry 'Point(1 1)@2000-01-01' %= ST_Point(1,1);
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' %= ST_Point(1,1);
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' %= ST_Point(1,1);
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' %= ST_Point(1,1);
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' %= ST_Point(1.5,1.5);
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' %= ST_Point(1.5,1.5);
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' %= ST_Point(1.5,1.5);
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' %= ST_Point(1.5,1.5);

SELECT tgeometry 'Point(1 1)@2000-01-01' %= geometry 'Point empty';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' %= geometry 'Point empty';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' %= geometry 'Point empty';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' %= geometry 'Point empty';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' %= geography 'Point empty';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' %= geography 'Point empty';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' %= geography 'Point empty';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' %= geography 'Point empty';

SELECT tgeometry 'Point(1 1)@2000-01-01' %<> geometry 'Point(1 1)';
SELECT tgeometry 'Point(1 1)@2000-01-01' %<> geometry 'Point empty';

SELECT tgeometry 'Point(1 1)@2000-01-01' %= geometry 'Linestring(1 1,2 2)';
SELECT tgeography 'Point(1 1)@2000-01-01' %= geography 'Linestring(1 1,2 2)';

/* Errors */
SELECT tgeometry 'Point(1 1)@2000-01-01' %= geometry 'SRID=5676;Point(1 1)';
SELECT tgeometry 'Point(1 1)@2000-01-01' %= geometry 'Point(1 1 1)';
SELECT tgeography 'Point(1 1)@2000-01-01' %= geography 'SRID=4283;Point(1 1)';
SELECT tgeography 'Point(1 1)@2000-01-01' %= geography 'Point(1 1 1)';

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT asText(atValues(tgeometry 'Point(1 1)@2000-01-01', ST_Point(1,1)));
SELECT asText(atValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1)));
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1)));
SELECT asText(atValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1)));
SELECT asText(atValues(tgeography 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5)));
SELECT asText(atValues(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5)));
SELECT asText(atValues(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5)));
SELECT asText(atValues(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5)));

SELECT asText(atValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty'));
SELECT asText(atValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty'));
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty'));
SELECT asText(atValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty'));
SELECT asText(atValues(tgeography 'Point(1 1)@2000-01-01', geography 'Point empty'));
SELECT asText(atValues(tgeography '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geography 'Point empty'));
SELECT asText(atValues(tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geography 'Point empty'));
SELECT asText(atValues(tgeography '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geography 'Point empty'));

/* Roundoff errors */
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-16, 1.0 - 1e-16)));
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-17, 1.0 - 1e-17)));
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 + 1e-16, 1.0 + 1e-16)));

SELECT asText(atValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)'));
SELECT asText(atValues(tgeography 'Point(1 1)@2000-01-01', geography 'Linestring(1 1,2 2)'));

/* Errors */
SELECT atValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT atValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1 1)');
SELECT atValues(tgeography 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT atValues(tgeography 'Point(1 1)@2000-01-01', geography 'Point(1 1 1)');

SELECT asText(minusValues(tgeometry 'Point(1 1)@2000-01-01', ST_Point(1,1)));
SELECT asText(minusValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1)));
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1)));
SELECT asText(minusValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1)));
SELECT asText(minusValues(tgeography 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5)));
SELECT asText(minusValues(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5)));
SELECT asText(minusValues(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5)));
SELECT asText(minusValues(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5)));

SELECT asText(minusValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point empty'));
SELECT asText(minusValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geometry 'Point empty'));
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geometry 'Point empty'));
SELECT asText(minusValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geometry 'Point empty'));
SELECT asText(minusValues(tgeography 'Point(1 1)@2000-01-01', geography 'Point empty'));
SELECT asText(minusValues(tgeography '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geography 'Point empty'));
SELECT asText(minusValues(tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geography 'Point empty'));
SELECT asText(minusValues(tgeography '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geography 'Point empty'));

/* Roundoff errors */
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-16, 1.0 - 1e-16)));
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 - 1e-17, 1.0 - 1e-17)));
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]', ST_MakePoint(1.0 + 1e-16, 1.0 + 1e-16)));

SELECT asText(minusValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Linestring(1 1,2 2)'));
SELECT asText(minusValues(tgeography 'Point(1 1)@2000-01-01', geography 'Linestring(1 1,2 2)'));

/* Errors */
SELECT minusValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'SRID=5676;Point(1 1)');
SELECT minusValues(tgeometry 'Point(1 1)@2000-01-01', geometry 'Point(1 1 1)');
SELECT minusValues(tgeography 'Point(1 1)@2000-01-01', geography 'SRID=4283;Point(1 1)');
SELECT minusValues(tgeography 'Point(1 1)@2000-01-01', geography 'Point(1 1 1)');

SELECT asText(atValues(tgeometry 'Point(1 1)@2000-01-01', geomset '{"Point(1 1)"}'));
SELECT asText(atValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geomset '{"Point(1 1)"}'));
SELECT asText(atValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geomset '{"Point(1 1)"}'));
SELECT asText(atValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geomset '{"Point(1 1)"}'));
SELECT asText(atValues(tgeography 'Point(1.5 1.5)@2000-01-01', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(atValues(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(atValues(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(atValues(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geogset '{"Point(1.5 1.5)"}'));

SELECT asText(atValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'Linestring(1 1,2 2)')));
SELECT asText(atValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'Linestring(1 1,2 2)')));

/* Errors */
SELECT atValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'SRID=5676;Point(1 1)'));
SELECT atValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'Point(1 1 1)'));
SELECT atValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'SRID=4283;Point(1 1)'));
SELECT atValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'Point(1 1 1)'));

SELECT asText(minusValues(tgeometry 'Point(1 1)@2000-01-01', geomset '{"Point(1 1)"}'));
SELECT asText(minusValues(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', geomset '{"Point(1 1)"}'));
SELECT asText(minusValues(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', geomset '{"Point(1 1)"}'));
SELECT asText(minusValues(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', geomset '{"Point(1 1)"}'));
SELECT asText(minusValues(tgeography 'Point(1.5 1.5)@2000-01-01', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(minusValues(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(minusValues(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', geogset '{"Point(1.5 1.5)"}'));
SELECT asText(minusValues(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', geogset '{"Point(1.5 1.5)"}'));

SELECT asText(minusValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'Linestring(1 1,2 2)')));
SELECT asText(minusValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'Linestring(1 1,2 2)')));

/* Errors */
SELECT minusValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'SRID=5676;Point(1 1)'));
SELECT minusValues(tgeometry 'Point(1 1)@2000-01-01', set(geometry 'Point(1 1 1)'));
SELECT minusValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'SRID=4283;Point(1 1)'));
SELECT minusValues(tgeography 'Point(1 1)@2000-01-01', set(geography 'Point(1 1 1)'));

SELECT asText(atTime(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT st_astext(valueAtTimestamp(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT st_astext(valueAtTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(minusTime(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(atTime(tgeometry 'Point(1 1)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(atTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(minusTime(tgeometry 'Point(1 1)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(atTime(tgeometry 'Point(1 1)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(atTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));

SELECT asText(minusTime(tgeometry 'Point(1 1)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(minusTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));

SELECT asText(atTime(tgeometry 'Point(1 1)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(atTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));

SELECT asText(minusTime(tgeometry 'Point(1 1)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(minusTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));

-------------------------------------------------------------------------------

SELECT asText(beforeTimestamp(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(beforeTimestamp(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01', false));

SELECT asText(afterTimestamp(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(afterTimestamp(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01', false));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(deleteTime(tgeometry 'Point(1 1)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeography 'Point(1.5 1.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(deleteTime(tgeometry 'Point(1 1)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(deleteTime(tgeometry 'Point(1 1)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspan '[2000-01-01,2000-01-02]'));
SELECT asText(deleteTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspan '[2000-01-01,2000-01-02]'));

SELECT asText(deleteTime(tgeometry 'Point(1 1)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeography 'Point(1.5 1.5)@2000-01-01', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', tstzspanset '{[2000-01-01,2000-01-02]}'));
SELECT asText(deleteTime(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', tstzspanset '{[2000-01-01,2000-01-02]}'));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2000-01-01' = tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' = tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' = tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' = tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' = tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' = tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' = tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeometry 'Point(1 1)@2000-01-01' <> tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' <> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' <> tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' <> tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <> tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <> tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <> tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeometry 'Point(1 1)@2000-01-01' < tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' < tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' < tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' < tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' < tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' < tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' < tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeometry 'Point(1 1)@2000-01-01' <= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' <= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' <= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' <= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeometry 'Point(1 1)@2000-01-01' > tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' > tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' > tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' > tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' > tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' > tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' > tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeometry 'Point(1 1)@2000-01-01' >= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeometry 'Point(1 1)@2000-01-01';
SELECT tgeometry 'Point(1 1)@2000-01-01' >= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeometry 'Point(1 1)@2000-01-01' >= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeometry 'Point(1 1)@2000-01-01' >= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' >= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' >= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' >= tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeography 'Point(1.5 1.5)@2000-01-01' = tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' = tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' = tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' = tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' = tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' = tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' = tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeography 'Point(1.5 1.5)@2000-01-01' <> tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeography 'Point(1.5 1.5)@2000-01-01';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' <> tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' <> tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeography 'Point(1.5 1.5)@2000-01-01' <> tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <> tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <> tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <> tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

-- PostGIS changed the function of the function lwgeom_hash from version 3

SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' < tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' < tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' < tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' < tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' <= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' <= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' <= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' <= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' > tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' > tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' > tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' > tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' >= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeography 'Point(1.5 1.5)@2000-01-01' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' >= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' >= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' IS NOT NULL;
SELECT 1 WHERE tgeography 'Point(1.5 1.5)@2000-01-01' >= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;
SELECT 1 WHERE tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' IS NOT NULL;

-------------------------------------------------------------------------------

SELECT temporal_hash(tgeometry 'Point(1 1)@2000-01-01');
SELECT temporal_hash(tgeometry '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT temporal_hash(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT temporal_hash(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT temporal_hash(tgeography 'Point(1.5 1.5)@2000-01-01');
SELECT temporal_hash(tgeography '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT temporal_hash(tgeography '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT temporal_hash(tgeography '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

------------------------------------------------------------------------------
