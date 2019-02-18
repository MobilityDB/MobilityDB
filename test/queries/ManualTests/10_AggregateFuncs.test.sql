/*****************************************************************************
 * TemporalInst aggregate functions
 *****************************************************************************/

select tand(unnest) as y from unnest(ARRAY[
	tboolinst 'true@2010-01-01 08:00:00',
	tboolinst 'false@2010-01-02 08:00:00',
	tboolinst 'true@2010-01-03 08:00:00',
	tboolinst 'false@2010-01-04 08:00:00',
	tboolinst 'true@2010-01-04 08:00:00'
]);

select tor(unnest) as y from unnest(ARRAY[
	tboolinst 'true@2010-01-01 08:00:00',
	tboolinst 'false@2010-01-02 08:00:00',
	tboolinst 'true@2010-01-03 08:00:00',
	tboolinst 'false@2010-01-04 08:00:00',
	tboolinst 'true@2010-01-04 08:00:00'
]);

WITH Values AS (
select tboolinst(true, '2012-01-01 08:00:00') as value
union all
select tboolinst(false, '2012-01-01 08:00:00')
union all
select tboolinst(false, '2012-01-01 08:10:00')
union all
select tboolinst(false, '2012-01-01 08:15:00')
)
select tand(value)
from Values;

WITH Values AS (
select tboolinst(true, '2012-01-01 08:00:00') as value
union all
select tboolinst(true, '2012-01-01 08:00:00')
union all
select tboolinst(false, '2012-01-01 08:10:00')
union all
select tboolinst(false, '2012-01-01 08:15:00')
)
select tor(value)
from Values;

WITH Values AS (
select tboolinst(true, '2012-01-01 08:00:00') as value
union all
select tboolinst(false, '2012-01-01 08:00:00')
union all
select tboolinst(false, '2012-01-01 08:10:00')
union all
select tboolinst(false, '2012-01-01 08:15:00')
)
select tand(value)
from Values;

WITH Values AS (
select tboolinst(true, '2012-01-01 08:00:00') as value
union all
select tboolinst(false, '2012-01-01 08:00:00')
union all
select tboolinst(false, '2012-01-01 08:10:00')
union all
select tboolinst(false, '2012-01-01 08:15:00')
)
select tor(value)
from Values;
 
/*****************************************************************************/
 
select min(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '5@2010-01-04 08:00:00'
]);

select max(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '5@2010-01-04 08:00:00'
]);

select sum(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '5@2010-01-04 08:00:00'
]);

select count(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '5@2010-01-04 08:00:00'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '5@2010-01-04 08:00:00'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintinst '0@2010-01-01 08:00:00',
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '4@2010-01-04 08:00:00'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00',
	tintinst '3@2010-01-03 08:00:00',
	tintinst '4@2010-01-03 08:00:00',
	tintinst '4@2010-01-04 08:00:00',
	tintinst '4@2010-01-04 08:00:00'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintinst '0@2010-01-04 08:00:00',
	tintinst '1@2010-01-04 08:00:00',
	tintinst '2@2010-01-01 08:00:00',
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintinst '1@2010-01-04 08:00:00',
	tintinst '2@2010-01-01 08:00:00',
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00',
	tintinst '5@2010-01-03 08:00:00'
]);
 
select avg(unnest) as y from unnest(ARRAY[
	tfloatinst '1@2010-01-04 08:00:00',
	tfloatinst '2@2010-01-01 08:00:00',
	tfloatinst '3@2010-01-02 08:00:00',
	tfloatinst '4@2010-01-03 08:00:00',
	tfloatinst '2@2010-01-04 08:00:00'
]);
 
/*****************************************************************************
 * TemporalSeq aggregate functions
 *****************************************************************************/

select tand(unnest) as y from unnest(ARRAY[
	tboolseq 'true@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tboolseq 'false@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tboolseq 'true@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tboolseq 'false@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tboolseq 'true@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select tor(unnest) as y from unnest(ARRAY[
	tboolseq 'true@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tboolseq 'false@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tboolseq 'true@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tboolseq 'false@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tboolseq 'true@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);


WITH Values AS (
select tboolseq(true, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00]') AS value union
select tboolseq(false, period '[2012-01-01 08:05:00, 2012-01-01 08:15:00]') union
select tboolseq(true, period '[2012-01-01 08:10:00, 2012-01-01 08:20:00]') union
select tboolseq(true, period '[2012-01-01 09:00:00, 2012-01-01 09:10:00]') union
select tboolseq(false, period '[2012-01-01 09:05:00, 2012-01-01 09:15:00]') union
select tboolseq(true, period '[2012-01-01 09:10:00, 2012-01-01 09:20:00]')
)
select tand(value)
from Values;


/*****************************************************************************/

select min(unnest) as y from unnest(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select max(unnest) as y from unnest(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select sum(unnest) as y from unnest(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select count(unnest) as y from unnest(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'
]);

select avg(unnest) as y from unnest(ARRAY[
	tfloatseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tfloatseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tfloatseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tfloatseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintseq '5@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)',
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)'
]);

select avg(unnest) as y from unnest(ARRAY[
	tintseq '4@[2010-01-04 08:00:00, 2010-01-06 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-05 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '1@[2010-01-01 08:00:00, 2010-01-03 08:00:00)'
]);

/*****************************************************************************
 * TemporalS aggregate functions
 *****************************************************************************/

select tand(unnest) as y from unnest(ARRAY[
tbools(ARRAY[
	tboolseq 'true@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tboolseq 'false@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tbools(ARRAY[
	tboolseq 'true@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tboolseq 'false@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select tand(unnest) as y from unnest(ARRAY[
tbools(ARRAY[
	tboolseq 'true@[2010-01-01 08:00:00, 2010-01-02 08:10:00)',
	tboolseq 'false@[2010-01-02 08:10:00, 2010-01-03 08:20:00)']),
tbools(ARRAY[
	tboolseq 'true@[2010-01-02 08:05:00, 2010-01-04 08:15:00)',
	tboolseq 'false@[2010-01-04 08:15:00, 2010-01-05 08:25:00)'])
]);

select tor(unnest) as y from unnest(ARRAY[
tbools(ARRAY[
	tboolseq 'true@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tboolseq 'false@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tbools(ARRAY[
	tboolseq 'true@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tboolseq 'false@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

/*****************************************************************************/

select min(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tints(ARRAY[
	tintseq '3@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select min(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)']),
tints(ARRAY[
	tintseq '1@[2010-01-03 20:00:00, 2010-01-04 20:00:00)'])
]);

select min(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)']),
tints(ARRAY[
	tintseq '1@[2010-01-03 20:00:00, 2010-01-04 08:00:00)'])
]);

select min(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)']),
tints(ARRAY[
	tintseq '1@[2010-01-03 20:00:00, 2010-01-04 20:00:00)',
	tintseq '0@[2010-01-04 20:00:00, 2010-01-05 20:00:00)'])
]);

select max(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tints(ARRAY[
	tintseq '3@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select sum(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tints(ARRAY[
	tintseq '3@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select count(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tints(ARRAY[
	tintseq '3@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select avg(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)']),
tints(ARRAY[
	tintseq '3@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)'])
]);

select avg(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '3@[2010-01-03 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-05 08:00:00)']),
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-02 08:00:00)',
	tintseq '2@[2010-01-02 08:00:00, 2010-01-03 08:00:00)'])
]);

select avg(unnest) as y from unnest(ARRAY[
tints(ARRAY[
	tintseq '2@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tintseq '4@[2010-01-04 08:00:00, 2010-01-06 08:00:00)']),
tints(ARRAY[
	tintseq '1@[2010-01-01 08:00:00, 2010-01-03 08:00:00)',
	tintseq '3@[2010-01-03 08:00:00, 2010-01-05 08:00:00)'])
]);

select avg(unnest) as y from unnest(ARRAY[
tfloats(ARRAY[
	tfloatseq '1@[2010-01-01 08:00:00, 2010-01-03 08:00:00)',
	tfloatseq '3@[2010-01-03 08:00:00, 2010-01-05 08:00:00)']),
tfloats(ARRAY[
	tfloatseq '2@[2010-01-02 08:00:00, 2010-01-04 08:00:00)',
	tfloatseq '4@[2010-01-04 08:00:00, 2010-01-06 08:00:00)'])
]);

/*****************************************************************************
 * TemporalI aggregate functions
 *****************************************************************************/

select tand(unnest) as y from unnest(ARRAY[
tbooli(ARRAY[
	tboolinst 'true@2010-01-01 08:00:00',
	tboolinst 'false@2010-01-02 08:00:00']),
tbooli(ARRAY[
	tboolinst 'true@2010-01-02 08:00:00',
	tboolinst 'false@2010-01-03 08:00:00'])
]);

select tor(unnest) as y from unnest(ARRAY[
tbooli(ARRAY[
	tboolinst 'true@2010-01-01 08:00:00',
	tboolinst 'false@2010-01-02 08:00:00']),
tbooli(ARRAY[
	tboolinst 'true@2010-01-02 08:00:00',
	tboolinst 'false@2010-01-03 08:00:00'])
]);

/*****************************************************************************/

select min(unnest) as y from unnest(ARRAY[
tinti(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00']),
tinti(ARRAY[
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'])
]);

select max(unnest) as y from unnest(ARRAY[
tinti(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00']),
tinti(ARRAY[
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'])
]);

select sum(unnest) as y from unnest(ARRAY[
tinti(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00']),
tinti(ARRAY[
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'])
]);

select count(unnest) as y from unnest(ARRAY[
tinti(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00']),
tinti(ARRAY[
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'])
]);

select avg(unnest) as y from unnest(ARRAY[
tinti(ARRAY[
	tintinst '1@2010-01-01 08:00:00',
	tintinst '2@2010-01-02 08:00:00']),
tinti(ARRAY[
	tintinst '3@2010-01-02 08:00:00',
	tintinst '4@2010-01-03 08:00:00'])
]);

select avg(unnest) as y from unnest(ARRAY[
tfloati(ARRAY[
	tfloatinst '1@2010-01-01 08:00:00',
	tfloatinst '2@2010-01-03 08:00:00']),
tfloati(ARRAY[
	tfloatinst '3@2010-01-02 08:00:00',
	tfloatinst '4@2010-01-04 08:00:00'])
]);

select avg(unnest) as y from unnest(ARRAY[
tfloati(ARRAY[
	tfloatinst '3@2010-01-02 08:00:00',
	tfloatinst '4@2010-01-03 08:00:00']),
tfloati(ARRAY[
	tfloatinst '1@2010-01-01 08:00:00',
	tfloatinst '2@2010-01-02 08:00:00'])
]);

/*****************************************************************************/
-- temporalinst_aggregation_transfn

with values(v) as (
select tfloatinst '1@2000-01-01 08:00' union all
select tfloatinst '2@2000-01-01 08:05' union all
select tfloatinst '3@2000-01-01 08:05' union all
select tfloatinst '4@2000-01-01 08:10' 
)
select sum(v) from values;

with values(v) as (
select tfloatinst '4@2000-01-01 08:10' union all
select tfloatinst '3@2000-01-01 08:05' union all
select tfloatinst '2@2000-01-01 08:05' union all
select tfloatinst '1@2000-01-01 08:00' 
)
select sum(v) from values;

with values(v) as (
select tfloatinst '2@2000-01-01 08:05' union all
select tfloatinst '3@2000-01-01 08:05' union all
select tfloatinst '1@2000-01-01 08:00' union all
select tfloatinst '4@2000-01-01 08:10' 
)
select sum(v) from values;

with values(v) as (
select tfloatinst '4@2000-01-01 08:10' union all
select tfloatinst '1@2000-01-01 08:00' union all
select tfloatinst '2@2000-01-01 08:05' union all
select tfloatinst '3@2000-01-01 08:05'
)
select sum(v) from values;

/*****************************************************************************/
-- temporali_aggregation_transfn

with values(v) as (
select tfloati(ARRAY[
  tfloatinst '1@2000-01-01 08:00',
  tfloatinst '3@2000-01-01 08:10',
  tfloatinst '5@2000-01-01 08:20',
  tfloatinst '7@2000-01-01 08:30']) 
union all
select tfloati(ARRAY[
  tfloatinst '2@2000-01-01 08:05',
  tfloatinst '4@2000-01-01 08:15',
  tfloatinst '6@2000-01-01 08:25']) 
)
select sum(v) from values;

with values(v) as (
select tfloati(ARRAY[
  tfloatinst '1@2000-01-01 08:00',
  tfloatinst '3@2000-01-01 08:10',
  tfloatinst '5@2000-01-01 08:20',
  tfloatinst '7@2000-01-01 08:30']) 
union all
select tfloati(ARRAY[
  tfloatinst '2@2000-01-01 08:05',
  tfloatinst '4@2000-01-01 08:20',
  tfloatinst '6@2000-01-01 08:25']) 
)
select sum(v) from values;

with values(v) as (
select tfloati(ARRAY[
  tfloatinst '1@2000-01-01 08:00',
  tfloatinst '3@2000-01-01 08:10',
  tfloatinst '5@2000-01-01 08:20',
  tfloatinst '7@2000-01-01 08:30']) 
union all
select tfloati(ARRAY[
  tfloatinst '2@2000-01-01 08:35',
  tfloatinst '4@2000-01-01 08:40',
  tfloatinst '6@2000-01-01 08:45']) 
)
select sum(v) from values;

with values(v) as (
select tfloati(ARRAY[
  tfloatinst '1@2000-01-01 08:20',
  tfloatinst '3@2000-01-01 08:30',
  tfloatinst '5@2000-01-01 08:40',
  tfloatinst '7@2000-01-01 08:50']) 
union all
select tfloati(ARRAY[
  tfloatinst '2@2000-01-01 08:00',
  tfloatinst '4@2000-01-01 08:05',
  tfloatinst '6@2000-01-01 08:10']) 
)
select sum(v) from values;

/*****************************************************************************/
/*****************************************************************************/

WITH Values AS (
select tfloatinst(1, '2012-01-01 08:00:00') as value
union
select tfloatinst(2, '2012-01-01 08:00:00')
union
select tfloatinst(4, '2012-01-01 08:10:00')
union
select tfloatinst(5, '2012-01-01 08:15:00')
)
select min(value)
from Values;

WITH Values AS (
select tfloatinst(1, '2012-01-01 08:15:00') as value
union
select tfloatinst(2, '2012-01-01 08:10:00')
union
select tfloatinst(4, '2012-01-01 08:00:00')
union
select tfloatinst(5, '2012-01-01 08:00:00')
)
select min(value)
from Values;

/*****************************************************************************/

-- Constant segments

-- Equal
WITH seq as (
select 1 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;
-- Disjoint, ts1 left to ts2
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:10:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;
-- Disjoint, ts2 left to ts1
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:10:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;
-- ts1 contains ts2, intervals to the left and to the right
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:10:00, 2012-01-01 08:15:00)') as s
)
select min(s)
from seq;
-- ts1 contains ts2, interval to the left
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:10:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;
-- ts1 contains ts2, interval to the right
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;
-- ts2 contains ts1, intervals to the left and to the right
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:10:00, 2012-01-01 08:15:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;
-- ts2 contains ts1, interval to the left
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:10:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;
-- ts2 contains ts1, interval to the right
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;
-- ts1 overlaps to the left ts2
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:10:00, 2012-01-01 08:30:00)') as s
)
select min(s)
from seq;
-- ts2 overlaps to the left ts1
WITH seq as (
select 1 as id, tfloatseq(100, 100, period '[2012-01-01 08:10:00, 2012-01-01 08:30:00)') as s
union
select 2 as id, tfloatseq( 10,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:20:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq( 10, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq(130, 10,  period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq( 50,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

-- Increasing and decreasing segments
WITH seq as (
select 1 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq( 10, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq(130,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq(100, 10,  period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq( 10, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

WITH seq as (
select 1 as id, tfloatseq(100, 130, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
union
select 2 as id, tfloatseq( 50,  10, period '[2012-01-01 08:00:00, 2012-01-01 08:10:00)') as s
)
select min(s)
from seq;

/*****************************************************************************/

