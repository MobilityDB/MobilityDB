drop table if exists test_analyze_tbool;
create table test_analyze_tbool (
value bool,
t timestamp,
temp tbool
);

insert into test_analyze_tbool(temp)
select random_tboolinst('2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tbool
set value = getValue(temp),
t = getTimestamp(temp);

select * from test_analyze_tbool limit 3;
------------------------------------------------
create table test_analyze_tint (
value integer,
t timestamp,
temp tint
);

insert into test_analyze_tint(temp)
select random_tintinst(1, 100, '2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tint
set value = getValue(temp),
t = getTimestamp(temp);

select * from  test_analyze_tint limit 3;

------------------------------------------------
drop table if exists test_analyze_tgeompoint;
create table test_analyze_tgeompoint (
value geometry,
t period,
temp tgeompoint
);

insert into test_analyze_tgeompoint(temp)
select random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 100);

insert into test_analyze_tgeompoint(temp)
select random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

insert into test_analyze_tgeompoint(temp)
select random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

insert into test_analyze_tgeompoint(temp)
select random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tgeompoint
set value = getValues(temp),
t = timespan(temp);

------------------------------------------------
drop table if exists test_analyze_tboolinst;
create table test_analyze_tboolinst (
value bool,
t timestamp,
temp tbool(Instant)
);

insert into test_analyze_tboolinst(temp)
select random_tboolinst('2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tboolinst
set value = getValue(temp),
t = getTimestamp(temp);

select * from test_analyze_tboolinst limit 3;
------------------------------------------------
create table test_analyze_tintinst (
value integer,
t timestamp,
temp tint(Instant)
);

insert into test_analyze_tintinst(temp)
select random_tintinst(1, 100, '2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tintinst
set value = getValue(temp),
t = getTimestamp(temp);

select * from  test_analyze_tintinst limit 3;

------------------------------------------------
drop table if exists test_analyze_tgeompointinst;
create table test_analyze_tgeompointinst (
value geometry(point),
t timestamp,
temp tgeompoint(Instant)
);

insert into test_analyze_tgeompointinst(temp)
select random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tgeompointinst
set value = getValue(temp),
t = getTimestamp(temp);
------------------------------------------------
drop table if exists test_analyze_tbooli;
create table test_analyze_tbooli (
value bool[],
t timestamp[],
temp tbool(instantset)
);

insert into test_analyze_tbooli(temp)
select random_tbooli('2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tbooli
set value = getValues(temp),
t = timestamps(temp);
------------------------------------------------
drop table if exists test_analyze_tinti;
create table test_analyze_tinti (
value integer[],
t timestamp[],
temp tint(InstantSet)
);

insert into test_analyze_tinti(temp)
select random_tinti(1, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tinti
set value = getValues(temp),
t = timestamps(temp);

select * from  test_analyze_tinti limit 3;

------------------------------------------------
drop table if exists test_analyze_tgeompointi;
create table test_analyze_tgeompointi (
value geometry,
t timestamp[],
temp tgeompoint(InstantSet)
);

insert into test_analyze_tgeompointi(temp)
select random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tgeompointi
set value = getValues(temp),
t = timestamps(temp);

------------------------------------------------
drop table if exists test_analyze_tboolseq;
create table test_analyze_tboolseq (
value bool[],
t Period,
temp tbool(Sequence)
);

insert into test_analyze_tboolseq(temp)
select random_tboolseq('2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tboolseq
set value = getValues(temp),
t = timespan(temp);

select * from test_analyze_tboolseq limit 3;
------------------------------------------------
drop table if exists test_analyze_tintseq;
create table test_analyze_tintseq (
value integer[],
t period,
temp tint(Sequence)
);

insert into test_analyze_tintseq(temp)
select random_tintseq(1, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tintseq
set value = getValues(temp),
t = timespan(temp);

select * from  test_analyze_tintseq limit 3;
------------------------------------------------
drop table if exists test_analyze_tfloatseq;
create table test_analyze_tfloatseq (
value FloatRange,
t Period,
temp tfloat(Sequence)
);

insert into test_analyze_tfloatseq(temp)
select random_tfloatseq(1, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tfloatseq
set value = valueRange(temp),
t = timespan(temp);

select * from  test_analyze_tfloatseq limit 3;

------------------------------------------------
drop table if exists test_analyze_tgeompointseq;
create table test_analyze_tgeompointseq (
value geometry,
t period,
temp tgeompoint(Sequence)
);

insert into test_analyze_tgeompointseq(temp)
select random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as i
from generate_series(1, 100);

update test_analyze_tgeompointseq
set value = getValues(temp),
t = timespan(temp);

insert into test_analyze_tgeompointseq
select * from  test_analyze_tgeompointseq limit 30;

------------------------------------------------
drop table if exists test_analyze_tbools;
create table test_analyze_tbools (
value bool[],
t Period,
temp tbool(Sequenceset)
);

insert into test_analyze_tbools(temp)
select random_tbools('2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 100);

update test_analyze_tbools
set value = getValues(temp),
t = timespan(temp);

------------------------------------------------
drop table if exists test_analyze_tfloats;

create table test_analyze_tfloats (
value FloatRange[],
t PeriodSet,
temp tfloat
);

insert into test_analyze_tfloats(temp)
select random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 100);

select * from  test_analyze_tfloats where numSequences(temp) > 1;

update test_analyze_tfloats
set value = getValues(temp),
t = getTime(temp);

------------------------------------------------
drop table if exists test_analyze_tgeompoints;
create table test_analyze_tgeompoints (
value geometry,
t period,
temp tgeompoint(SequenceSet)
);

insert into test_analyze_tgeompoints(temp)
select random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 100);

update test_analyze_tgeompoints
set value = getValues(temp),
t = timespan(temp);
------------------------------------------------