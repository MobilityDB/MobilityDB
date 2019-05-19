/*****************************************************************************
 * typmod
 *****************************************************************************/
 
select tgeompoint(Instant, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(Instant, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(InstantSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Sequence, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(SequenceSet, Point) '{[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02],
	[Point(0 1)@2000-01-03, Point(1 1)@2000-01-04]}';
select tgeompoint(SequenceSet, PointZ) '{[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02],
	[Point(0 1 0)@2000-01-03, Point(1 1 1)@2000-01-04]}';

/*
select tgeompoint(Instant, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(InstantSet, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(InstantSet, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(Sequence, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(Sequence, PointZ) 'Point(0 1)@2000-01-01';
select tgeompoint(SequenceSet, Point) 'Point(0 1)@2000-01-01';
select tgeompoint(SequenceSet, PointZ) 'Point(0 1)@2000-01-01';

select tgeompoint(Instant, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(InstantSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(InstantSet, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Sequence, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(Sequence, PointZ) 'Point(0 1 0)@2000-01-01';
select tgeompoint(SequenceSet, Point) 'Point(0 1 0)@2000-01-01';
select tgeompoint(SequenceSet, PointZ) 'Point(0 1 0)@2000-01-01';

select tgeompoint(Instant, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Instant, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(InstantSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Sequence, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(Sequence, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(SequenceSet, Point) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';
select tgeompoint(SequenceSet, PointZ) '{Point(0 1)@2000-01-01, Point(1 1)@2000-01-02}';

select tgeompoint(Instant, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Instant, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(InstantSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Sequence, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(Sequence, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(SequenceSet, Point) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';
select tgeompoint(SequenceSet, PointZ) '{Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02}';

select tgeompoint(Instant, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Instant, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(InstantSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(InstantSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(Sequence, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(SequenceSet, Point) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';
select tgeompoint(SequenceSet, PointZ) '[Point(0 1)@2000-01-01, Point(1 1)@2000-01-02]';

select tgeompoint(Instant, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Instant, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(InstantSet, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(InstantSet, PointZ) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
select tgeompoint(Sequence, Point) '[Point(0 1 0)@2000-01-01, Point(1 1 1)@2000-01-02]';
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
*/

/*****************************************************************************
 * Instant
 *****************************************************************************/
 
select tgeompoint 'Point(0 0)@2012-01-01 08:00:00';
select tgeompointinst(ST_Point(0,0), timestamptz '2012-01-01 08:00:00');

select tgeompoint 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointseq(ST_Point(0,0), ST_Point(1,1), period '[2012-01-01 08:00:00, 2012-01-01 08:00:05)');

select tgeompoint '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompoints(ARRAY[
	tgeompointseq(ST_Point(0,0), ST_Point(1,1), period '[2012-01-01 08:00:00, 2012-01-01 08:00:05)'),
	tgeompointseq(ST_Point(1,1), ST_Point(0,0), period '[2012-01-01 08:00:05, 2012-01-01 08:00:10)')
]);

select tgeompoint '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi(ARRAY[
	tgeompoint 'Point(0 0)@2012-01-01 08:00:00',
	tgeompoint 'Point(1 1)@2012-01-01 08:05:00'
]);
select tgeompointi(ARRAY[
	tgeompointinst(ST_Point(0,0), timestamptz '2012-01-01 08:00:00'),
	tgeompointinst(ST_Point(1,1), timestamptz '2012-01-01 08:00:05')
]);

/*****************************************************************************/

select astext(tgeompoint 'Point(0 0)@2012-01-01 08:00:00');
select astext(ARRAY[
	tgeompoint 'Point(2 2)@2012-01-01 08:00:00', 
	tgeompoint 'Point(0 0)@2012-01-01 08:05:00'
]);

select astext(tgeompoint 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');
select astext(ARRAY[
	tgeompointseq(ST_Point(0,0), ST_Point(1,1), period '[2012-01-01 08:00:00, 2012-01-01 08:00:05)'),
	tgeompointseq(ST_Point(1,1), ST_Point(0,0), period '[2012-01-01 08:00:05, 2012-01-01 08:00:10)')
]);

select astext(tgeompoint '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
	Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');
select astext(ARRAY[
	tgeompoints(ARRAY[
		tgeompointseq(ST_Point(0,0), ST_Point(1,1), period '[2012-01-01 08:00:00, 2012-01-01 08:00:05)'),
		tgeompointseq(ST_Point(1,1), ST_Point(0,0), period '[2012-01-01 08:00:05, 2012-01-01 08:00:10)')]),
	tgeompoints(ARRAY[
		tgeompointseq(ST_Point(2,2), ST_Point(1,1), period '[2012-01-01 08:00:00, 2012-01-01 08:00:05)'),
		tgeompointseq(ST_Point(3,3), ST_Point(0,0), period '[2012-01-01 08:00:05, 2012-01-01 08:00:10)')])
]);

select astext(tgeompoint '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');
select astext(ARRAY[
	tgeompoint '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}',
	tgeompoint '{Point(2 2)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}'
]);

/*****************************************************************************/

select SRID(tgeompoint 'Point(0 0)@2012-01-01 08:00:00');
select SRID(tgeompoint 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');
select SRID(tgeompoints(ARRAY[
	tgeompoint 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompoint 'Point(1 1)->Point(0 0)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)'
]));
select SRID(tgeompoint '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');
	
/*****************************************************************************/

select ST_AsText(trajectory(tgeompoint 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)'));
select ST_AsText(trajectory(tgeompoints(ARRAY[
	tgeompoint 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompoint 'Point(1 1)->Point(0 0)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)'
])));

/*****************************************************************************
 * TPointInst
 *****************************************************************************/

select ST_AsText(getValue(tgeompoint 'Point(0 0)@2012-01-01 08:00:00'));
select getTime(tgeompoint 'Point(0 0)@2012-01-01 08:00:00');
select everEquals(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', ST_Point(0,0));
select everEquals(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', ST_Point(1,1));
	
select astext(shift(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', interval '5 min'));
	
select astext(atValue(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', ST_Point(0,0)));
select astext(atValue(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', ST_Point(1,1)));
select astext(atTimestamp(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', timestamptz '2012-01-01 08:00:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)@2012-01-01 08:00:00', timestamptz '2012-01-01 08:05:00'));

/*****************************************************************************
 * TPointPer
 *****************************************************************************/

select ST_AsText(getValues(tgeompoint 'Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)'));
select ST_AsText(getValues(tgeompoint 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)'));

select ST_AsText(startValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)'));
select ST_AsText(endValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)'));

select getTime(tgeompoint 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select duration(tgeompoint 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select startTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)');
select endTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)');

select startInstant(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)');
select endInstant(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)');

select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,0));
select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@(2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,0));
select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00]', 
	ST_Point(2,2));
select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(2,2));
select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(1,1));
select everEquals(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,1));

select astext(shift(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	interval '5 min'));

select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,0)));
select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@(2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,0)));
select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00]', 
	ST_Point(2,2)));
select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(2,2)));
select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(1,1)));
select astext(atValue(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 
	ST_Point(0,1)));

SELECT asText(minusValue(tgeompoint '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
geometry 'Point(0 0)'));
--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(0 0)@2012-01-02 00:00:00+01), (POINT(0 0)@2012-01-02 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01)}"


SELECT asText(minusValue(tgeompoint '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
geometry 'Point(3 3)'));
--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01)}"

SELECT asText(minusValue(tgeompoint '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05]',
geometry 'Point(3 3)'));
--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01)}"

SELECT asText(minusValue(tgeompoint '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05]',
geometry 'Point(-1 -1)'));
--"{(POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01]}"


select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:15:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:00:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@(2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:00:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00]', 
	timestamptz '2012-01-01 08:10:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:10:00'));
select astext(atTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:05:00'));
	
select astext(atTimestampSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestampset(ARRAY[timestamptz '2012-01-01 08:00:00', '2012-01-01 08:05:00', '2012-01-01 08:10:00'])));
	
select astext(atPeriod(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	period '[2012-01-01 08:02:00, 2012-01-01 08:05:00]'));

select astext(atPeriodSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	periodSet(ARRAY[period '[2012-01-01 08:02:00, 2012-01-01 08:05:00]', '[2012-01-01 08:06:00, 2012-01-01 08:07:00]'])));
	
select intersectsTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:05:00');
select intersectsTimestamp(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestamptz '2012-01-01 08:10:00');
		
select intersectsTimestampSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestampset(ARRAY[timestamptz '2012-01-01 08:00:00', '2012-01-01 08:05:00', '2012-01-01 08:15:00']));
select intersectsTimestampSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	timestampset(ARRAY[timestamptz '2012-01-01 07:00:00', '2012-01-01 08:15:00']));
	
select intersectsPeriod(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	period '[2012-01-01 08:02:00, 2012-01-01 08:05:00]');
select intersectsPeriod(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	period '[2012-01-01 08:02:00, 2012-01-01 08:15:00]');
select intersectsPeriod(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	period '[2012-01-01 08:10:00, 2012-01-01 08:15:00]');
	
select intersectsPeriodSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	periodSet(ARRAY[period '[2012-01-01 08:02:00, 2012-01-01 08:05:00]', '[2012-01-01 08:06:00, 2012-01-01 08:07:00]']));
select intersectsPeriodSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	periodSet(ARRAY[period '[2012-01-01 08:02:00, 2012-01-01 08:05:00]', '[2012-01-01 08:10:00, 2012-01-01 08:15:00]']));
select intersectsPeriodSet(tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)', 
	periodSet(ARRAY[period '[2012-01-01 07:02:00, 2012-01-01 07:05:00]', '[2012-01-01 08:10:00, 2012-01-01 08:15:00]']));

select st_astext(twCentroid(tgeompoint 'Point(0 0)->Point(2 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)'));

select length(
tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, 
Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]'
);

select cumulativeLength(
tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, 
Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]'
);

select speed(
tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, 
Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]'
) * 3600 * 24;

select astext(atGeometry(
tgeompoints(ARRAY[
tgeompoint 'Point(0 0)->Point(2 0)@[2000-01-01 08:00, 2000-01-01 08:10)',
tgeompoint 'Point(2 0)->Point(0 2)@[2000-01-01 08:10, 2000-01-01 08:20)',
tgeompoint 'Point(0 2)->Point(1 2)@[2000-01-01 08:20, 2000-01-01 08:30)',
tgeompoint 'Point(1 2)->Point(1 0)@[2000-01-01 08:30, 2000-01-01 08:40)',
tgeompoint 'Point(1 0)->Point(4 0)@[2000-01-01 08:40, 2000-01-01 08:50)'
])
,
geometry 'polygon((1 0,1 2,3 2,3 0,1 0))'));

/*****************************************************************************
 * TPointS
 *****************************************************************************/

select st_astext(getValues(
tgeompoints(ARRAY[
tgeompoint 'Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)',
tgeompoint 'Point(0 0)->Point(2 2)@[2012-01-01 08:10:00, 2012-01-01 08:20:00)',
tgeompoint 'Point(2 2)@[2012-01-01 08:20:00, 2012-01-01 08:30:00)',
tgeompoint 'Point(2 2)->Point(0 0)@[2012-01-01 08:30:00, 2012-01-01 08:40:00)',
tgeompoint 'Point(0 0)@[2012-01-01 08:40:00, 2012-01-01 08:50:00]'
])));

---------------------------------------
-- tempcontseq_from_temporalinstarr
---------------------------------------

select astext(tgeompointseq(ARRAY[
tgeompointinst(ST_Point(1,0), '2000-01-01 08:00'),
tgeompointinst(ST_Point(2,0), '2000-01-01 08:05'),
tgeompointinst(ST_Point(3,0), '2000-01-01 08:10'),
tgeompointinst(ST_Point(1,0), '2000-01-01 08:15')
]));

/*
-- ERROR
select astext(tgeompointseq(ARRAY[
tgeompointinst(ST_Point(1,0), '2000-01-01 08:00'),
tgeompointinst(ST_Point(2,0), '2000-01-01 08:05'),
tgeompointinst(ST_Point(3,0), '2000-01-01 08:10'),
tgeompointinst(ST_Point(1,0), '2000-01-01 08:10')
]));
*/
---------------------------------------

select astext(atPeriodSet(
tgeompointseq(st_point(1,1), st_point(4,4), '2012-01-01 08:00:00', '2012-01-01 08:15:00'),
periodset(ARRAY[period('2012-01-01 08:05:00','2012-01-01 08:10:00'), period('2012-01-01 08:11:00','2012-01-01 08:12:00')])));

select astext(atPeriodSet(
tgeompointseq(st_point(1,1), st_point(4,4), '2012-01-01 08:00:00', '2012-01-01 08:15:00'),
periodset(ARRAY[period('2012-01-01 08:05:00','2012-01-01 08:10:00'), period('2012-01-01 08:08:00','2012-01-01 08:12:00')])));

---------------------------------------
-- twCentroid
---------------------------------------

select st_astext(twCentroid(tgeompoints(ARRAY[
tgeompoint 'Point(0 0)->Point(2 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompoint 'Point(2 2)->Point(0 0)@[2001-01-01 08:10:00, 2001-01-01 08:20:00)'
])));
-- "POINT(1 1)"

select st_astext(twCentroid(tgeompoints(ARRAY[
tgeompoint 'Point(0 0)->Point(2 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompoint 'Point(2 2)->Point(0 0)@[2001-01-01 08:10:00, 2001-01-01 08:20:00)',
tgeompoint 'Point(0 0)@[2001-01-01 08:20:00, 2001-01-01 08:40:00)'
])));
-- "POINT(0.5 0.5)"

select st_astext(twCentroid(tgeompoints(ARRAY[
tgeompoint 'Point(0 0)->Point(2 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompoint 'Point(2 2)->Point(0 0)@[2001-01-01 08:10:00, 2001-01-01 08:20:00)',
tgeompoint 'Point(0 0)@[2001-01-01 08:20:00, 2001-01-01 09:00:00)'
])));
-- "POINT(0.333333333333333 0.333333333333333)"

select st_astext(twCentroid(tgeompoints(ARRAY[
tgeompoint 'Point(0 0)->Point(2 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompoint 'Point(2 2)->Point(0 0)@[2001-01-01 08:10:00, 2001-01-01 08:20:00)',
tgeompoint 'Point(0 0)@[2001-01-01 08:20:00, 2001-01-01 10:00:00)'
])));
-- "POINT(0.166666666666667 0.166666666666667)"

---------------------------------------
-- length
---------------------------------------

select length(tgeompoints(ARRAY[
	tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]',
	tgeompoint '[Point(0 0)@2000-01-05, Point(1 0)@2000-01-06, Point(1 1)@2000-01-07, Point(0 0)@2000-01-08]']));

---------------------------------------
-- cumulativeLength
---------------------------------------

select unnest(sequences(cumulativeLength(tgeompoints(ARRAY[
	tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]',
	tgeompoint '[Point(0 0)@2000-01-05, Point(1 0)@2000-01-06, Point(1 1)@2000-01-07, Point(0 0)@2000-01-08]']))));

---------------------------------------
-- speed
---------------------------------------

select unnest(sequences(speed(tgeompoints(ARRAY[
	tgeompoint '[Point(0 0)@2000-01-01, Point(1 0)@2000-01-02, Point(1 1)@2000-01-03, Point(0 0)@2000-01-04]',
	tgeompoint '[Point(0 0)@2000-01-05, Point(1 0)@2000-01-06, Point(1 1)@2000-01-07, Point(0 0)@2000-01-08]']))
	* 3600 * 24));

/*****************************************************************************
 * TPointI
 *****************************************************************************/

select astext(ARRAY[ST_Point(0,0), ST_Point(1,1)]);

select astext(getValues(tgeompointi(ARRAY[
	tgeompointinst(ST_Point(0,0), timestamptz '2012-01-01 08:00:00'),
	tgeompointinst(ST_Point(1,1), timestamptz '2012-01-01 08:00:05')
])));

select astext(getValues(tgeompointi(ARRAY[
tgeompoint 'Point(0 0)@2012-01-01 08:00:00',
tgeompoint 'Point(2 2)@2012-01-01 08:10:00',
tgeompoint 'Point(2 2)@2012-01-01 08:20:00',
tgeompoint 'Point(0 0)@2012-01-01 08:30:00',
tgeompoint 'Point(0 0)@2012-01-01 08:40:00'
])));

/*****************************************************************************/


