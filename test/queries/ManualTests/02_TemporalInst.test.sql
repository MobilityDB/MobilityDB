-------------------------------------------------------------------------------
-- Input/output functions
-------------------------------------------------------------------------------

select tboolinst 'TRUE@2012-01-01 08:00:00';
select tboolinst 'FALSE@2012-01-01 08:00:00';

select tintinst '1@2012-01-01 08:00:00';
select tintinst '2@2012-01-01 08:00:00';

select tfloatinst '1@2012-01-01 08:00:00';
select tfloatinst '2@2012-01-01 08:00:00';

select ttextinst 'AAA@2012-01-01 08:00:00';
select ttextinst 'BBB@2012-01-01 08:00:00';

-- Erroneous input
/*
select tboolinst '2@2012-01-01 08:00:00';
select tintinst 'TRUE@2012-01-01 08:00:00';
select tfloatinst 'ABC@2012-01-01 08:00:00';
*/

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

select tboolinst(TRUE, '2012-01-01 08:00:00');
select tboolinst(NULL, '2012-01-01 08:00:00');

select tintinst(1, '2012-01-01 08:00:00');
select tintinst(NULL, '2012-01-01 08:00:00');

select tfloatinst(1, '2012-01-01 08:00:00');
select tfloatinst(NULL, '2012-01-01 08:00:00');

select ttextinst('AAA', '2001-01-01 08:00:00');


select ARRAY[
tboolinst(true, '2012-01-01 08:00:00'),
tboolinst(true, '2012-01-01 08:00:00'),
tboolinst(true, '2012-01-01 08:00:00')
];

select ARRAY[
tintinst(1, '2012-01-01 08:00:00'),
tintinst(2, '2012-01-01 08:00:00'),
tintinst(3, '2012-01-01 08:00:00')
];

select ARRAY[
tfloatinst(1, '2012-01-01 08:00:00'),
tfloatinst(2, '2012-01-01 08:00:00'),
tfloatinst(3, '2012-01-01 08:00:00')
];

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

select tboolinst(TRUE, '2012-01-01 08:00:00')::tboolseq;
select tboolinst(TRUE, '2012-01-01 08:00:00')::tbools;
select tboolinst(TRUE, '2012-01-01 08:00:00')::tbooli;

select tintinst(1, '2012-01-01 08:00:00')::tintseq;
select tintinst(1, '2012-01-01 08:00:00')::tints;
select tintinst(1, '2012-01-01 08:00:00')::tinti;

select tintinst(1, '2012-01-01 08:00:00')::tfloatinst;
select tintinst(1, '2012-01-01 08:00:00')::tfloatseq;
select tintinst(1, '2012-01-01 08:00:00')::tfloats;
select tintinst(1, '2012-01-01 08:00:00')::tfloati;

select tfloatinst(1, '2012-01-01 08:00:00')::tfloatseq;
select tfloatinst(1, '2012-01-01 08:00:00')::tfloats;
select tfloatinst(1, '2012-01-01 08:00:00')::tfloati;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

select getValue(tboolinst(TRUE, '2012-01-01 08:00:00'));
select getValue(tintinst(1, '2012-01-01 08:00:00'));
select getValue(tfloatinst(1, '2012-01-01 08:00:00'));

select getTime(tboolinst(TRUE, '2012-01-01 08:00:00'));
select getTime(tintinst(1, '2012-01-01 08:00:00'));
select getTime(tfloatinst(1, '2012-01-01 08:00:00'));

select everEquals(tboolinst(TRUE, '2012-01-01 08:00:00'), TRUE);
select everEquals(tboolinst(TRUE, '2012-01-01 08:00:00'), FALSE);
select everEquals(tintinst(1, '2012-01-01 08:00:00'), 1);
select everEquals(tintinst(1, '2012-01-01 08:00:00'), 2);
select everEquals(tfloatinst(1, '2012-01-01 08:00:00'), 1);
select everEquals(tfloatinst(1, '2012-01-01 08:00:00'), 2);

select shift(tboolinst(TRUE, '2012-01-01 08:00:00'), interval '5 min');
select shift(tintinst(1, '2012-01-01 08:00:00'), interval '5 min');
select shift(tfloatinst(1, '2012-01-01 08:00:00'), interval '5 min');
select shift(ttextinst('A', '2012-01-01 08:00:00'), interval '5 min');

select atValue(tboolinst(TRUE, '2012-01-01 08:00:00'), TRUE);
select atValue(tboolinst(TRUE, '2012-01-01 08:00:00'), FALSE);
select atValue(tintinst(1, '2012-01-01 08:00:00'), 1);
select atValue(tintinst(1, '2012-01-01 08:00:00'), 2);
select atValue(tfloatinst(1, '2012-01-01 08:00:00'), 1);
select atValue(tfloatinst(1, '2012-01-01 08:00:00'), 2);

select minusValue(tboolinst(TRUE, '2012-01-01 08:00:00'), TRUE);
select minusValue(tboolinst(TRUE, '2012-01-01 08:00:00'), FALSE);
select minusValue(tintinst(1, '2012-01-01 08:00:00'), 1);
select minusValue(tintinst(1, '2012-01-01 08:00:00'), 2);
select minusValue(tfloatinst(1, '2012-01-01 08:00:00'), 1);
select minusValue(tfloatinst(1, '2012-01-01 08:00:00'), 2);

select atValues(tboolinst(TRUE, '2012-01-01 08:00:00'), ARRAY[TRUE,FALSE]);
select atValues(tintinst(1, '2012-01-01 08:00:00'), ARRAY[1,2,3]);
select atValues(tintinst(1, '2012-01-01 08:00:00'), ARRAY[2,3,4]);
select atValues(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[1,2,3]);
select atValues(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[2,3,4]);

select minusValues(tboolinst(TRUE, '2012-01-01 08:00:00'), ARRAY[FALSE]);
select minusValues(tintinst(1, '2012-01-01 08:00:00'), ARRAY[1,2,3]);
select minusValues(tintinst(1, '2012-01-01 08:00:00'), ARRAY[2,3,4]);
select minusValues(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[1,2,3]);
select minusValues(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[2,3,4]);

select atRange(tintinst(1, '2012-01-01 08:00:00'), intrange '[1,3]');
select atRange(tintinst(1, '2012-01-01 08:00:00'), intrange '[2,4]');
select atRange(tfloatinst(1, '2012-01-01 08:00:00'), floatrange '[1,3]');
select atRange(tfloatinst(1, '2012-01-01 08:00:00'), floatrange '[2,4]');

select minusRange(tintinst(1, '2012-01-01 08:00:00'), intrange '[1,3]');
select minusRange(tintinst(1, '2012-01-01 08:00:00'), intrange '(1,3]');
select minusRange(tintinst(1, '2012-01-01 08:00:00'), intrange '[2,4]');
select minusRange(tfloatinst(1, '2012-01-01 08:00:00'), floatrange '[1,3]');
select minusRange(tfloatinst(1, '2012-01-01 08:00:00'), floatrange '[2,4]');

select atRanges(tintinst(1, '2012-01-01 08:00:00'), ARRAY[intrange '[1,3]', '[4,6]']);
select atRanges(tintinst(1, '2012-01-01 08:00:00'), ARRAY[intrange '[2,3]', '[4,6]']);
select atRanges(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[floatrange '[1,3]', '[4,6]']);
select atRanges(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[floatrange '[2,3]', '[4,6]']);

select minusRanges(tintinst(1, '2012-01-01 08:00:00'), ARRAY[intrange '[1,3]', '[4,6]']);
select minusRanges(tintinst(1, '2012-01-01 08:00:00'), ARRAY[intrange '(1,3]', '[4,6]']);
select minusRanges(tintinst(1, '2012-01-01 08:00:00'), ARRAY[intrange '[2,3]', '[4,6]']);
select minusRanges(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[floatrange '[1,3]', '[4,6]']);
select minusRanges(tfloatinst(1, '2012-01-01 08:00:00'), ARRAY[floatrange '[2,3]', '[4,6]']);

select atTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select atTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select atTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select atTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select atTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select atTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');

select minusTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select minusTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select minusTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select minusTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select minusTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select minusTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');

select atTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select atTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select atTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select atTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select atTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select atTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');


select minusTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select minusTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select minusTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select minusTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select minusTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:00:00, 2012-01-01 08:05:00, 2012-01-01 08:10:00}');
select minusTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset '{2012-01-01 08:05:00, 2012-01-01 08:10:00}');

select atPeriod(tboolinst(true, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select atPeriod(tboolinst(true, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select atPeriod(tintinst(1, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select atPeriod(tintinst(1, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select atPeriod(tfloatinst(1, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select atPeriod(tfloatinst(1, '2012-01-01 08:00:00'), 
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));

select minusPeriod(tboolinst(true, '2012-01-01 08:00:00'),
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select minusPeriod(tboolinst(true, '2012-01-01 08:00:00'),
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select minusPeriod(tintinst(1, '2012-01-01 08:00:00'),
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select minusPeriod(tintinst(1, '2012-01-01 08:00:00'),
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select minusPeriod(tfloatinst(1, '2012-01-01 08:00:00'),
	period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select minusPeriod(tfloatinst(1, '2012-01-01 08:00:00'),
	period('2012-01-01 08:01:00','2012-01-01 08:10:00'));

select atPeriodSet(tboolinst(true, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00, 2012-01-01 08:05:00), (2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select atPeriodSet(tboolinst(true, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00), (2012-01-01 08:06:00, 2012-01-01 08:10:00)}');
select atPeriodSet(tintinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00,2012-01-01 08:05:00),	(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select atPeriodSet(tintinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00),(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select atPeriodSet(tfloatinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00,2012-01-01 08:05:00),	(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select atPeriodset(tfloatinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00),(2012-01-01 08:06:00,2012-01-01 08:10:00)}');

select minusPeriodSet(tboolinst(true, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00, 2012-01-01 08:05:00), (2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select minusPeriodSet(tboolinst(true, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00), (2012-01-01 08:06:00, 2012-01-01 08:10:00)}');
select minusPeriodSet(tintinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00,2012-01-01 08:05:00),	(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select minusPeriodSet(tintinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00),(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select minusPeriodSet(tfloatinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:00:00,2012-01-01 08:05:00),	(2012-01-01 08:06:00,2012-01-01 08:10:00)}');
select minusPeriodSet(tfloatinst(1, '2012-01-01 08:00:00'),
	PeriodSet '{ (2012-01-01 08:01:00,2012-01-01 08:05:00),(2012-01-01 08:06:00,2012-01-01 08:10:00)}');

select intersectsTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select intersectsTimestamp(tboolinst(TRUE, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select intersectsTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select intersectsTimestamp(tintinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');
select intersectsTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:00:00');
select intersectsTimestamp(tfloatinst(1, '2012-01-01 08:00:00'),'2012-01-01 08:10:00');

select intersectsTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:00:00', '2012-01-01 08:05:00', '2012-01-01 08:10:00']));
select intersectsTimestampSet(tboolinst(TRUE, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:05:00', '2012-01-01 08:10:00']);
select intersectsTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:00:00', '2012-01-01 08:05:00', '2012-01-01 08:10:00']));
select intersectsTimestampSet(tintinst(1, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:05:00', '2012-01-01 08:10:00']));
select intersectsTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:00:00', '2012-01-01 08:05:00', '2012-01-01 08:10:00']));
select intersectsTimestampSet(tfloatinst(1, '2012-01-01 08:00:00'),
	timestampset(ARRAY[timestamp '2012-01-01 08:05:00', '2012-01-01 08:10:00']));

select intersectsPeriod(tboolinst(true, '2012-01-01 08:00:00'), period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select intersectsPeriod(tboolinst(true, '2012-01-01 08:00:00'), period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select intersectsPeriod(tintinst(1, '2012-01-01 08:00:00'), period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select intersectsPeriod(tintinst(1, '2012-01-01 08:00:00'), period('2012-01-01 08:01:00','2012-01-01 08:10:00'));
select intersectsPeriod(tfloatinst(1, '2012-01-01 08:00:00'), period('2012-01-01 08:00:00','2012-01-01 08:10:00'));
select intersectsPeriod(tfloatinst(1, '2012-01-01 08:00:00'), period('2012-01-01 08:01:00','2012-01-01 08:10:00'));

select intersectsPeriodSet(tboolinst(true, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:00:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));
select intersectsPeriodSet(tboolinst(true, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:01:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));
select intersectsPeriodSet(tintinst(1, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:00:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));
select intersectsPeriodSet(tintinst(1, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:01:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));
select intersectsPeriodSet(tfloatinst(1, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:00:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));
select intersectsPeriodSet(tfloatinst(1, '2012-01-01 08:00:00'), 
	periodset(ARRAY[period('2012-01-01 08:01:00','2012-01-01 08:05:00'), 
	period('2012-01-01 08:06:00','2012-01-01 08:10:00')]));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

select ttextinst('AAA', '2001-01-01 08:00:00') =
	ttextinst('AAA', '2001-01-01 08:00:00');

select ttextinst('AAA', '2001-01-01 08:00:00') <
	ttextinst('AAA', '2001-01-01 08:00:00');

WITH Values AS(
select tboolinst(true, '2012-01-01 08:00:00') AS value union
select tboolinst(false, '2012-01-01 08:00:00') union
select tboolinst(true, '2012-01-01 08:00:00') union
select tboolinst(false, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS(
select tboolinst(true, '2012-01-01 08:00:00') AS value union all
select tboolinst(false, '2012-01-01 08:00:00') union all
select tboolinst(true, '2012-01-01 08:00:00') union all
select tboolinst(false, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS(
select tintinst(1, '2012-01-01 08:00:00') AS value union
select tintinst(1, '2012-01-01 08:00:00') union
select tintinst(2, '2012-01-01 08:00:00') union
select tintinst(3, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS(
select tintinst(1, '2012-01-01 08:00:00') AS value union all
select tintinst(1, '2012-01-01 08:00:00') union all
select tintinst(2, '2012-01-01 08:00:00') union all
select tintinst(3, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS(
select tfloatinst(1, '2012-01-01 08:00:00') AS value union
select tfloatinst(1, '2012-01-01 08:00:00') union
select tfloatinst(2, '2012-01-01 08:00:00') union
select tfloatinst(3, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS(
select tfloatinst(1, '2012-01-01 08:00:00') AS value union all
select tfloatinst(1, '2012-01-01 08:00:00') union all
select tfloatinst(2, '2012-01-01 08:00:00') union all
select tfloatinst(3, '2012-01-01 08:00:00')
)
select *
from Values
order by value;

WITH Values AS (
select ttextinst('AAA', '2012-01-01 08:00:00') AS value union
select ttextinst('BBB', '2012-01-01 08:00:00') union
select ttextinst('AAA', '2012-01-01 08:00:00') union
select ttextinst('BBB', '2012-01-01 08:00:00')
)
select *
from Values
order by value;

------------------------------------------------------------------------------
