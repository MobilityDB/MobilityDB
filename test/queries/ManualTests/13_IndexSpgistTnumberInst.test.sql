/*****************************************************************************/

drop table if exists tbl_tintinst;
create table tbl_tintinst as
select i, random_tintinst(1, 100, '2001-01-01', '2001-12-31') as inst
from generate_series(1,10000) i;

select * from tbl_tintinst limit 10;

drop index if exists tbl_tintinst_spgist_idx; 

create index tbl_tintinst_spgist_idx on tbl_tintinst using spgist(inst);

/*****************************************************************************/

drop table if exists tbl_tfloatinst;
create table tbl_tfloatinst as
select inst.i, tfloatinst(inst) as inst
from tbl_tintinst inst;

select * from tbl_tfloatinst limit 10

drop index if exists tbl_tfloatinst_spgist_idx; 

create index tbl_tfloatinst_spgist_idx on tbl_tfloatinst using spgist(inst)

/*****************************************************************************/

select count(*) from tbl_tintinst
where getValue(inst) = 5
-- 98 

explain
select count(*) from tbl_tintinst
where inst <@ 5
-- 98 

select count(*) from tbl_tintinst
where inst <@ 5.0
-- 98 

explain
select count(*) from tbl_tintinst
where getValue(inst) <@ intrange '[2,5]'
-- 410

explain
select count(*) from tbl_tintinst
where inst <@ intrange '[2,5]'
-- 410 

select count(*) from tbl_tintinst
where inst <@ floatrange '[2,5]'
-- 410 

select getTime(inst) from tbl_tintinst group by getTime(inst) having count(*) > 1

select count(*) from tbl_tintinst
where getTime(inst) = timestamptz '2001-06-05 13:14:33'
-- 2

explain
select count(*) from tbl_tintinst
where inst <@ timestamptz '2001-06-05 13:14:33'
-- 2 

select count(*) from tbl_tintinst
where getTime(inst) <@ period('2001-03-01', '2001-07-31')
-- 4094

explain
select count(*) from tbl_tintinst
where inst <@ period('2001-03-01', '2001-07-31')
-- 4094

explain analyze
select count(*) from tbl_tintinst
where inst <@ box(tintper(5, '2001-03-01', '2001-07-31'))
-- 42

explain
select count(*) from tbl_tintinst
where inst <@ tintper(5, '2001-03-01', '2001-07-31')
-- 42

explain
select count(*) from tbl_tintinst
where inst <@ tfloatper(5, 5, '2001-03-01', '2001-07-31')
-- 42

explain
select count(*) from tbl_tintinst
where inst <@ tfloatper(5, 7, '2001-03-01', '2001-07-31')
-- 125

explain
select count(*) from tbl_tintinst
where inst <@ tintp(ARRAY[
tintper(5, '2001-03-01', '2001-05-01'),
tintper(7, '2001-05-01', '2001-07-31', true, true)])
-- 125

explain
select count(*) from tbl_tintinst
where inst <@ tfloatp(ARRAY[
tfloatper(5, 6, '2001-03-01', '2001-05-01'),
tfloatper(6, 7, '2001-05-01', '2001-07-31', true, true)])
-- 125

explain analyze
select count(*) from tbl_tintinst
where inst <@ tinti(ARRAY[
tintinst(5, '2001-03-01'),
tintinst(7, '2001-07-31')])
-- 125

explain analyze
select count(*) from tbl_tintinst
where inst <@ tfloati(ARRAY[
tfloatinst(5, '2001-03-01'),
tfloatinst(7, '2001-07-31')])
-- 125

/*****************************************************************************/

select count(*) from tbl_tfloatinst
where getValue(inst) = 5
-- 98 

explain
select count(*) from tbl_tfloatinst
where inst <@ 5
-- 98 

select count(*) from tbl_tfloatinst
where inst <@ 5.0
-- 98 

explain
select count(*) from tbl_tfloatinst
where getValue(inst) <@ floatrange '[2,5]'
-- 410

explain
select count(*) from tbl_tfloatinst
where inst <@ intrange '[2,5]'
-- 410 

select count(*) from tbl_tfloatinst
where inst <@ floatrange '[2,5]'
-- 410 

select getTime(inst) from tbl_tfloatinst group by getTime(inst) having count(*) > 1

select count(*) from tbl_tfloatinst
where getTime(inst) = timestamptz '2001-06-05 13:14:33'
-- 2

explain
select count(*) from tbl_tfloatinst
where inst <@ timestamptz '2001-06-05 13:14:33'
-- 2 

select count(*) from tbl_tfloatinst
where getTime(inst) <@ period('2001-03-01', '2001-07-31')
-- 4094

explain
select count(*) from tbl_tfloatinst
where inst <@ period('2001-03-01', '2001-07-31')
-- 4094

explain analyze
select count(*) from tbl_tfloatinst
where inst <@ box(tintper(5, '2001-03-01', '2001-07-31'))
-- 42

explain
select count(*) from tbl_tfloatinst
where inst <@ tintper(5, '2001-03-01', '2001-07-31')
-- 42

explain
select count(*) from tbl_tfloatinst
where inst <@ tfloatper(5, 5, '2001-03-01', '2001-07-31')
-- 42

explain
select count(*) from tbl_tfloatinst
where inst <@ tfloatper(5, 7, '2001-03-01', '2001-07-31')
-- 125

explain
select count(*) from tbl_tfloatinst
where inst <@ tintp(ARRAY[
tintper(5, '2001-03-01', '2001-05-01'),
tintper(7, '2001-05-01', '2001-07-31', true, true)])
-- 125

explain
select count(*) from tbl_tfloatinst
where inst <@ tfloatp(ARRAY[
tfloatper(5, 6, '2001-03-01', '2001-05-01'),
tfloatper(6, 7, '2001-05-01', '2001-07-31', true, true)])
-- 125

explain analyze
select count(*) from tbl_tfloatinst
where inst <@ tinti(ARRAY[
tintinst(5, '2001-03-01'),
tintinst(7, '2001-07-31')])
-- 125

explain analyze
select count(*) from tbl_tfloatinst
where inst <@ tfloati(ARRAY[
tfloatinst(5, '2001-03-01'),
tfloatinst(7, '2001-07-31')])
-- 125

/*****************************************************************************/

@>

/*****************************************************************************/

&&

/*****************************************************************************/

~=

/*****************************************************************************/

<<

/*****************************************************************************/

&<

/*****************************************************************************/

>>

/*****************************************************************************/

&>

/*****************************************************************************/

<<|

/*****************************************************************************/

&<|

/*****************************************************************************/

|>>

/*****************************************************************************/

|&>

/*****************************************************************************/

<<#

/*****************************************************************************/

&<#

/*****************************************************************************/

#>>

/*****************************************************************************/

#&>

/*****************************************************************************/
