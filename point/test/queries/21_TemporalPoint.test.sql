/*****************************************************************************
 * SRID
 *****************************************************************************/

select asewkt(tgeompoint 'SRID=4326;[Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
select asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, Point(0 1)@2000-01-02]');
select asewkt(tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, SRID=4326;Point(0 1)@2000-01-02]');

select asewkt(tgeompoint 'SRID=4326;{[Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
select asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [Point(0 1)@2000-01-02]}');
select asewkt(tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=4326;Point(0 1)@2000-01-02]}');

/* Error
select tgeompoint '{SRID=4326;Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02}';
select tgeompoint 'SRID=4326;{Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02}';

select tgeompoint '[SRID=4326;Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02]';
select tgeompoint 'SRID=4326;[Point(0 1)@2000-01-01, SRID=5434;Point(0 1)@2000-01-02]';

select tgeompoint '{[SRID=4326;Point(0 1)@2000-01-01], [SRID=5434;Point(0 1)@2000-01-02]';
select tgeompoint 'SRID=4326;{[Point(0 1)@2000-01-01], [SRID=5434;Point(0 1)@2000-01-02]}';
*/

/*****************************************************************************
 * typmod
 *****************************************************************************/

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

/* Error
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
*/
/*****************************************************************************/
 
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

/* Error
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
*/
/*****************************************************************************/

