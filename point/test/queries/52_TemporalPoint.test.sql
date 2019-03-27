-------------------------------------------------------------------------------
-- Input/output functions
-------------------------------------------------------------------------------

-- Temporal instant

SELECT tgeompoint 'Point(1 1)@2012-01-01 08:00:00';
SELECT tgeompoint 'Point(2 2)@2012-01-01 08:00:00';
SELECT tgeogpoint 'Point(1 1)@2012-01-01 08:00:00';
SELECT tgeogpoint 'Point(2 2)@2012-01-01 08:00:00';
/* Errors */
SELECT tgeompoint 'TRUE@2012-01-01 08:00:00';
SELECT tgeogpoint 'ABC@2012-01-01 08:00:00';

-------------------------------------------------------------------------------

-- Temporal instant set

SELECT tgeompoint ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ';
SELECT tgeompoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';
SELECT tgeogpoint ' { Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 } ';
SELECT tgeogpoint '{Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00}';

-------------------------------------------------------------------------------

-- Temporal sequence

SELECT tgeompoint ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ';
SELECT tgeompoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
SELECT tgeogpoint ' [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ] ';
SELECT tgeogpoint '[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00]';
/* Error */

-------------------------------------------------------------------------------

-- Temporal sequence set

SELECT tgeompoint '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ';
SELECT tgeompoint '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}';

SELECT tgeogpoint '  { [ Point(1 1)@2001-01-01 08:00:00 , Point(2 2)@2001-01-01 08:05:00 , Point(3 3)@2001-01-01 08:06:00 ],
 [ Point(1 1)@2001-01-01 09:00:00 , Point(2 2)@2001-01-01 09:05:00 , Point(1 1)@2001-01-01 09:06:00 ] } ';
SELECT tgeogpoint '{[Point(1 1)@2001-01-01 08:00:00,Point(2 2)@2001-01-01 08:05:00,Point(3 3)@2001-01-01 08:06:00],
 [Point(1 1)@2001-01-01 09:00:00,Point(2 2)@2001-01-01 09:05:00,Point(1 1)@2001-01-01 09:06:00]}';

-------------------------------------------------------------------------------
-- SRID
-------------------------------------------------------------------------------

select asewkt(tgeompoint 'SRID=4326;[Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
select asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
select asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, SRID=4326;Point(0 1)@2000-01-02]');

select asewkt(tgeompoint 'SRID=4326;{[Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
select asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
select asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=4326;Point(0 1)@2000-01-02]}');

/* Errors */
select tgeompoint '{SRID=4326;Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02}';
select tgeompoint 'SRID=4326;{Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02}';
select tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02]';
select tgeompoint 'SRID=4326;[Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02]';
select tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=5434;Point(0 1)@2000-01-02]';
select tgeompoint 'SRID=4326;{[Point(0 1)@2000-01-01], [SRID=5434;Point(0 1)@2000-01-02]}';

-------------------------------------------------------------------------------
-- typmod
-------------------------------------------------------------------------------

select tgeompoint 'Point(0 1)@2000-01-01';
select tgeompoint 'Point(0 1 1)@2000-01-01';
select tgeompoint(Instant) 'Point(0 1)@2000-01-01';
select tgeompoint(Instant) 'Point(0 1 1)@2000-01-01';
select tgeompoint(Instant, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(Instant, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Instant, Point, 4326) 'SRID=4326;Point(0 1)@2000-01-01';
select tgeompoint(Instant, PointZ, 4326) 'SRID=4326;Point(0 1 0)@2000-01-01';

select tgeompoint '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint '{Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet) '{Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point, 4326) 'SRID=4326;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ, 4326) 'SRID=4326;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';

select tgeompoint '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint '[Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence) '[Point(0 1 1)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, Point, 4326) 'SRID=4326;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ, 4326) 'SRID=4326;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';

select tgeompoint '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, Point, 4326) 'SRID=4326;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ, 4326) 'SRID=4326;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';

/* Errors */
select tgeompoint(Instant, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(Instant, Point, 4326) 'Point(0 1)@2000-01-01';
select tgeompoint(Instant, Point, 4326) 'SRID=5434;Point(0 1)@2000-01-01';
select tgeompoint(InstantSet, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(InstantSet, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(Sequence, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(Sequence, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(SequenceSet, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(SequenceSet, PointZ) 'Point(0 1)@2000-01-01';

select tgeompoint(Instant, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Instant, PointZ, 4326) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Instant, PointZ, 4326) 'SRID=5434;Point(0 1 0)@2000-01-01';
select tgeompoint(InstantSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(InstantSet, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Sequence, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(SequenceSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01';

select tgeompoint(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point, 4326) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point, 4326) 'SRID=5434;{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';

select tgeompoint(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ, 4326) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ, 4326) 'SRID=5434;{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';

select tgeompoint(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(InstantSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(InstantSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, Point, 4326) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, Point, 4326) 'SRID=5434;[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';

select tgeompoint(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(InstantSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(InstantSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ, 4326) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ, 4326) 'SRID=5434;[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';

select tgeompoint(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(InstantSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(InstantSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, Point, 4326) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, Point, 4326) 'SRID=5434;{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';

select tgeompoint(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(InstantSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(InstantSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ, 4326) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ, 4326) 'SRID=5434;{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';

-------------------------------------------------------------------------------/
 
select tgeogpoint(Instant, Point) 'Point(0 1)@2000-01-01';
select tgeogpoint(Instant, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(InstantSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(InstantSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';

/* Errors */
select tgeogpoint(Instant, PointZ) 'Point(0 1)@2000-01-01';
select tgeogpoint(InstantSet, Point) 'Point(0 1)@2000-01-01';
select tgeogpoint(InstantSet, PointZ) 'Point(0 1)@2000-01-01';
select tgeogpoint(Sequence, Point) 'Point(0 1)@2000-01-01';
select tgeogpoint(Sequence, PointZ) 'Point(0 1)@2000-01-01';
select tgeogpoint(SequenceSet, Point) 'Point(0 1)@2000-01-01';
select tgeogpoint(SequenceSet, PointZ) 'Point(0 1)@2000-01-01';

select tgeogpoint(Instant, Point) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(InstantSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(InstantSet, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(Sequence, Point) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(SequenceSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeogpoint(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01';

select tgeogpoint(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(InstantSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeogpoint(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';

select tgeogpoint(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(InstantSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeogpoint(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';

select tgeogpoint(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(InstantSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(InstantSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeogpoint(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';

select tgeogpoint(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(InstantSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(InstantSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(SequenceSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeogpoint(SequenceSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';

select tgeogpoint(Instant, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(Instant, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(InstantSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(InstantSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(Sequence, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(Sequence, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeogpoint(SequenceSet, PointZ) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';

select tgeogpoint(Instant, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(Instant, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(InstantSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(InstantSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(Sequence, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(Sequence, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';
select tgeogpoint(SequenceSet, Point) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

SELECT tgeompointinst(ST_Point(1,1), '2012-01-01 08:00:00');
SELECT tgeompointinst(NULL, '2012-01-01 08:00:00');
SELECT tgeogpointinst(ST_Point(1,1), '2012-01-01 08:00:00');
SELECT tgeogpointinst(NULL, '2012-01-01 08:00:00');

-------------------------------------------------------------------------------

SELECT tgeompointi(ARRAY[
tgeompointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeompointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeompointinst(ST_Point(1,1), '2012-01-01 08:20:00')
]);
SELECT tgeogpointi(ARRAY[
tgeogpointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeogpointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeogpointinst(ST_Point(1,1), '2012-01-01 08:20:00')
]);

-------------------------------------------------------------------------------

SELECT tgeompointseq(ARRAY[
tgeompointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeompointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeompointinst(ST_Point(1,1), '2012-01-01 08:20:00')
]);
SELECT tgeogpointseq(ARRAY[
tgeogpointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeogpointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeogpointinst(ST_Point(1,1), '2012-01-01 08:20:00')
]);

-------------------------------------------------------------------------------

SELECT tgeompoints(ARRAY[
tgeompointseq(ARRAY[
tgeompointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeompointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeompointinst(ST_Point(1,1), '2012-01-01 08:20:00')
]),
tgeompointseq(ARRAY[
tgeompointinst(ST_Point(1,1), '2012-01-01 09:00:00'),
tgeompointinst(ST_Point(2,2), '2012-01-01 09:10:00'),
tgeompointinst(ST_Point(1,1), '2012-01-01 09:20:00')
])]);
SELECT tgeogpoints(ARRAY[
tgeogpointseq(ARRAY[
tgeogpointinst(ST_Point(1,1), '2012-01-01 08:00:00'),
tgeogpointinst(ST_Point(2,2), '2012-01-01 08:10:00'),
tgeogpointinst(ST_Point(3,3), '2012-01-01 08:20:00')
]),
tgeogpointseq(ARRAY[
tgeogpointinst(ST_Point(1,1), '2012-01-01 09:00:00'),
tgeogpointinst(ST_Point(2,2), '2012-01-01 09:10:00'),
tgeogpointinst(ST_Point(1,1), '2012-01-01 09:20:00')
])]);

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT tgeogpoint(tgeompoint 'Point(1 1)@2001-01-01');
SELECT tgeogpoint(tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02}');
SELECT tgeogpoint(tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02]');
SELECT tgeogpoint(tgeompoint '{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02], [Point(2 2)@2001-01-03, Point(2 2)@2001-01-04]}');

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT tgeompointinst(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tgeompointi(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tgeompointi(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompointseq(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tgeompointseq(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompoints(tgeompoint 'Point(1 1)@2000-01-01');
SELECT tgeompoints(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompoints(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompoints(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
/* Errors */
SELECT tgeompointinst(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompointinst(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompointinst(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT tgeompointi(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT tgeompointi(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT tgeompointseq(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT tgeompointseq(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');

SELECT tgeogpointinst(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tgeogpointi(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tgeogpointi(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tgeogpointseq(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tgeogpointseq(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeogpoints(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT tgeogpoints(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tgeogpoints(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeogpoints(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT tgeogpointinst(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tgeogpointinst(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeogpointinst(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT tgeogpointi(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT tgeogpointi(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
SELECT tgeogpointseq(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT tgeogpointseq(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT temporalType(tgeompoint 'Point(1 1)@2000-01-01');
SELECT temporalType(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT temporalType(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT temporalType(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT temporalType(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT temporalType(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT temporalType(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT temporalType(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT memSize(tgeompoint 'Point(1 1)@2000-01-01');
SELECT memSize(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT memSize(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT memSize(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT memSize(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT memSize(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT memSize(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT memSize(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT gbox(tgeompoint 'Point(1 1)@2000-01-01');
SELECT gbox(tgeogpoint 'Point(1.5 1.5)@2000-01-01');

SELECT getValue(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
/* Errors */
SELECT getValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT getValues(tgeompoint 'Point(1 1)@2000-01-01');
SELECT getValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT getValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT getValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT getValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT getValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT getValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT getValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT startValue(tgeompoint 'Point(1 1)@2000-01-01');
SELECT startValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT startValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT startValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT startValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT startValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT startValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT startValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT endValue(tgeompoint 'Point(1 1)@2000-01-01');
SELECT endValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT endValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT endValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT endValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT endValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT endValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT endValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

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

SELECT numSequences(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numSequences(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT numSequences(tgeompoint 'Point(1 1)@2000-01-01');
SELECT numSequences(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numSequences(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numSequences(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT numSequences(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numSequences(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');

SELECT startSequence(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT startSequence(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT startSequence(tgeompoint 'Point(1 1)@2000-01-01');
SELECT startSequence(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT startSequence(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT startSequence(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT startSequence(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT startSequence(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');

SELECT endSequence(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT endSequence(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT endSequence(tgeompoint 'Point(1 1)@2000-01-01');
SELECT endSequence(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT endSequence(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT endSequence(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT endSequence(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT endSequence(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');

SELECT sequenceN(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);
SELECT sequenceN(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1);
/* Errors */
SELECT sequenceN(tgeompoint 'Point(1 1)@2000-01-01', 1);
SELECT sequenceN(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1);
SELECT sequenceN(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1);
SELECT sequenceN(tgeogpoint 'Point(1.5 1.5)@2000-01-01', 1);
SELECT sequenceN(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1);
SELECT sequenceN(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1);

SELECT sequences(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT sequences(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');
/* Errors */
SELECT sequences(tgeompoint 'Point(1 1)@2000-01-01');
SELECT sequences(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT sequences(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT sequences(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT sequences(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT sequences(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');

SELECT numInstants(tgeompoint 'Point(1 1)@2000-01-01');
SELECT numInstants(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT numInstants(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT numInstants(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT numInstants(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT numInstants(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT numInstants(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT numInstants(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT startInstant(tgeompoint 'Point(1 1)@2000-01-01');
SELECT startInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT startInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT startInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT startInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT startInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT startInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT startInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT endInstant(tgeompoint 'Point(1 1)@2000-01-01');
SELECT endInstant(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT endInstant(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT endInstant(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT endInstant(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT endInstant(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT endInstant(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT endInstant(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

SELECT instantN(tgeompoint 'Point(1 1)@2000-01-01', 1);
SELECT instantN(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', 1);
SELECT instantN(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', 1);
SELECT instantN(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', 1);
SELECT instantN(tgeogpoint 'Point(1.5 1.5)@2000-01-01', 1);
SELECT instantN(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', 1);
SELECT instantN(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', 1);
SELECT instantN(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', 1);

SELECT instants(tgeompoint 'Point(1 1)@2000-01-01');
SELECT instants(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}');
SELECT instants(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT instants(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
SELECT instants(tgeogpoint 'Point(1.5 1.5)@2000-01-01');
SELECT instants(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}');
SELECT instants(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]');
SELECT instants(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}');

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

SELECT tgeompoint 'Point(1 1)@2000-01-01' &= ST_Point(1,1);
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' &= ST_Point(1,1);
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' &= ST_Point(1,1);
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' &= ST_Point(1,1);
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' &= ST_Point(1.5,1.5);
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' &= ST_Point(1.5,1.5);
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' &= ST_Point(1.5,1.5);
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' &= ST_Point(1.5,1.5);

SELECT tgeompoint 'Point(1 1)@2000-01-01' @= ST_Point(1,1);
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @= ST_Point(1,1);
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @= ST_Point(1,1);
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @= ST_Point(1,1);
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @= ST_Point(1.5,1.5);
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @= ST_Point(1.5,1.5);
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @= ST_Point(1.5,1.5);
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @= ST_Point(1.5,1.5);
 
SELECT shift(tgeompoint 'Point(1 1)@2000-01-01', '5 min');
SELECT shift(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', '5 min');
SELECT shift(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', '5 min');
SELECT shift(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', '5 min');
SELECT shift(tgeogpoint 'Point(1.5 1.5)@2000-01-01', '5 min');
SELECT shift(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', '5 min');
SELECT shift(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', '5 min');
SELECT shift(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', '5 min');

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT atValue(tgeompoint 'Point(1 1)@2000-01-01', ST_Point(1,1));
SELECT atValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1));
SELECT atValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1));
SELECT atValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1));
SELECT atValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5));
SELECT atValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5));
SELECT atValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5));
SELECT atValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5));

SELECT minusValue(tgeompoint 'Point(1 1)@2000-01-01', ST_Point(1,1));
SELECT minusValue(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ST_Point(1,1));
SELECT minusValue(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ST_Point(1,1));
SELECT minusValue(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ST_Point(1,1));
SELECT minusValue(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ST_Point(1.5,1.5));
SELECT minusValue(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ST_Point(1.5,1.5));
SELECT minusValue(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ST_Point(1.5,1.5));
SELECT minusValue(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ST_Point(1.5,1.5));

SELECT atValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[ST_Point(1,1)]);
SELECT atValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[ST_Point(1,1)]);
SELECT atValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[ST_Point(1,1)]);
SELECT atValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[ST_Point(1,1)]);
SELECT atValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[ST_Point(1.5,1.5)]);
SELECT atValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[ST_Point(1.5,1.5)]);
SELECT atValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[ST_Point(1.5,1.5)]);
SELECT atValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[ST_Point(1.5,1.5)]);

SELECT minusValues(tgeompoint 'Point(1 1)@2000-01-01', ARRAY[ST_Point(1,1)]);
SELECT minusValues(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', ARRAY[ST_Point(1,1)]);
SELECT minusValues(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', ARRAY[ST_Point(1,1)]);
SELECT minusValues(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', ARRAY[ST_Point(1,1)]);
SELECT minusValues(tgeogpoint 'Point(1.5 1.5)@2000-01-01', ARRAY[ST_Point(1.5,1.5)]);
SELECT minusValues(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', ARRAY[ST_Point(1.5,1.5)]);
SELECT minusValues(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', ARRAY[ST_Point(1.5,1.5)]);
SELECT minusValues(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', ARRAY[ST_Point(1.5,1.5)]);

SELECT atTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamp '2000-01-01');
SELECT atTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamp '2000-01-01');

SELECT valueAtTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamp '2000-01-01');

SELECT minusTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamp '2000-01-01');
SELECT minusTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamp '2000-01-01');

SELECT atTimestampSet(tgeompoint 'Point(1 1)@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestampset '{2000-01-01}');

SELECT minusTimestampSet(tgeompoint 'Point(1 1)@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestampset '{2000-01-01}');

SELECT atPeriod(tgeompoint 'Point(1 1)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeogpoint 'Point(1.5 1.5)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT minusPeriod(tgeompoint 'Point(1 1)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeogpoint 'Point(1.5 1.5)@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT atPeriodSet(tgeompoint 'Point(1 1)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT minusPeriodSet(tgeompoint 'Point(1 1)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeogpoint 'Point(1.5 1.5)@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT intersectsTimestamp(tgeompoint 'Point(1 1)@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint 'Point(1.5 1.5)@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}', timestamp '2000-01-01');

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

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' < tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' > tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' >= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

------------------------------------------------------------------------------
