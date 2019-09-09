--TBOOL cases
drop table if exists test_analyze_tbools;
create table test_analyze_tbools (
t period,
temp tbool(sequenceset)
);

insert into test_analyze_tbools(temp)
select random_tbools('2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 1000);

update test_analyze_tbools
set t = timespan(temp);


vacuum analyze test_analyze_tbools;


explain analyze select * from test_analyze_tbools where period '[2001-02-01, 2001-04-01]' &<# temp;
explain analyze select * from test_analyze_tbools where temp #>> period '[2001-02-01, 2001-04-01]';
explain analyze select * from test_analyze_tbools where period '[2001-02-01, 2001-04-01]' #&> temp;
explain analyze select * from test_analyze_tbools where temp <<# period '[2001-02-01, 2001-04-01]';
explain analyze select * from test_analyze_tbools where period '[2001-02-01, 2001-04-01]' <<# temp;
explain analyze select * from test_analyze_tbools where temp #>> period '[2001-02-01, 2001-04-01]';

--TBOOL op TBOOL
explain analyze select * from test_analyze_tbools where temp && tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where temp @> tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where temp <@ tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where tbool '[f@2001-02-01, t@2001-03-01]' &<# temp;
explain analyze select * from test_analyze_tbools where temp #>> tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where tbool '{f@2001-02-01, t@2001-03-01}'::tbool #&> temp;
explain analyze select * from test_analyze_tbools where temp <<# tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where tbool '{f@2001-02-01, t@2001-03-01}' <<# temp;
explain analyze select * from test_analyze_tbools where temp #>> tbool '{f@2001-02-01, t@2001-03-01}';
explain analyze select * from test_analyze_tbools where temp #&> tbool '{f@2001-02-01, t@2001-03-01}';

-----------------------------------------------------------------------------
--TINT cases
--TemporalInst
drop table if exists test_analyze_tintinst;
create table test_analyze_tintinst (
value integer,
t timestamp,
temp tint(instant)
);

insert into test_analyze_tintinst(temp)
select random_tintinst(1, 100, '2001-01-01', '2001-12-31') as i
from generate_series(1, 100);

update test_analyze_tintinst
set value = getValue(temp),
t = getTimestamp(temp);

vacuum analyze test_analyze_tintinst;

explain analyze select * from test_analyze_tintinst where temp << 23
explain analyze select * from test_analyze_tintinst where temp >> 23
explain analyze select * from test_analyze_tintinst where temp &< 23
explain analyze select * from test_analyze_tintinst where temp &> 23
explain analyze select * from test_analyze_tintinst where temp #>> period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tintinst where temp <<# period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tintinst where temp &<# period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tintinst where temp #&> period '[2001-01-01, 2001-02-01]';
-----------------------------------------------------------------------------
--TemporalSeq, TemporalS, and Temporal
drop table if exists test_analyze_tints;
create table test_analyze_tints (
value integer[],
t period,
temp tint(sequenceset)
);

insert into test_analyze_tints(temp)
select random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as i
from generate_series(1, 100);

update test_analyze_tints
set value = getValues(temp),
t = timespan(temp);

vacuum analyze test_analyze_tints;

--TINTS op ( int/float or period)
explain analyze select * from test_analyze_tints where temp && 50;
explain analyze select * from test_analyze_tints where 50 && temp;
explain analyze select * from test_analyze_tints where temp <@ 50;
explain analyze select * from test_analyze_tints where 50 <@ temp;
explain analyze select * from test_analyze_tints where temp @> 50
explain analyze select * from test_analyze_tints where temp ~= 50
explain analyze select * from test_analyze_tints where temp << 50
explain analyze select * from test_analyze_tints where 50 << temp;
explain analyze select * from test_analyze_tints where temp >> 50
explain analyze select * from test_analyze_tints where 50 >> temp;
explain analyze select * from test_analyze_tints where temp &< 50;
explain analyze select * from test_analyze_tints where 50 &< temp;
explain analyze select * from test_analyze_tints where temp &> 50
explain analyze select * from test_analyze_tints where 50 &> temp
explain analyze select * from test_analyze_tints where temp #>> period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tints where period '[2001-01-01, 2001-02-01]' #>> temp;
explain analyze select * from test_analyze_tints where temp <<# period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tints where period '[2001-01-01, 2001-02-01]' <<# temp;
explain analyze select * from test_analyze_tints where temp &<# period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tints where period '[2001-01-01, 2001-02-01]' &<# temp;
explain analyze select * from test_analyze_tints where temp #&> period '[2001-01-01, 2001-02-01]';
explain analyze select * from test_analyze_tints where period '[2001-01-01, 2001-02-01]' #&> temp;

--TINTS op TBOX
explain analyze select * from test_analyze_tints where temp && 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox && temp;
explain analyze select * from test_analyze_tints where temp @> 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox @> temp;
explain analyze select * from test_analyze_tints where temp <@ 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox <@ temp;
explain analyze select * from test_analyze_tints where temp << 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox << temp;
explain analyze select * from test_analyze_tints where temp >> 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox >> temp;
explain analyze select * from test_analyze_tints where temp &< 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox &< temp;
explain analyze select * from test_analyze_tints where temp &> 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox &> temp;
explain analyze select * from test_analyze_tints where temp <<# 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox <<# temp;
explain analyze select * from test_analyze_tints where temp #>> 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox #>> temp;
explain analyze select * from test_analyze_tints where temp #&> 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox #&> temp;
explain analyze select * from test_analyze_tints where temp &<# 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox;
explain analyze select * from test_analyze_tints where 'TBOX((44,5.207082e+13),(64,6.17172e+13))'::tbox &<# temp;

--TINTS op TINT
explain analyze select * from test_analyze_tints where temp && tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' && temp;
explain analyze select * from test_analyze_tints where temp @> tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' @> temp;
explain analyze select * from test_analyze_tints where temp <@ tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' <@ temp;
explain analyze select * from test_analyze_tints where temp ~= tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' ~= temp;
explain analyze select * from test_analyze_tints where temp >> tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' >> temp;
explain analyze select * from test_analyze_tints where temp << tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' << temp;
explain analyze select * from test_analyze_tints where temp &< tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' &< temp;
explain analyze select * from test_analyze_tints where temp &> tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' &> temp;
explain analyze select * from test_analyze_tints where temp #>> tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' #>> temp;
explain analyze select * from test_analyze_tints where temp <<# tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' <<# temp;
explain analyze select * from test_analyze_tints where temp #&> tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' #&> temp;
explain analyze select * from test_analyze_tints where temp &<# tint '[54@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tint '[54@2001-04-25 18:24:00+02]' &<# temp;

--TINTS op TFLOAT
explain analyze select * from test_analyze_tints where temp && tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' && temp;
explain analyze select * from test_analyze_tints where temp @> tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' @> temp;
explain analyze select * from test_analyze_tints where temp <@ tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' <@ temp;
explain analyze select * from test_analyze_tints where temp ~= tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' ~= temp;
explain analyze select * from test_analyze_tints where temp >> tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' >> temp;
explain analyze select * from test_analyze_tints where temp << tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' << temp;
explain analyze select * from test_analyze_tints where temp &< tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' &< temp;
explain analyze select * from test_analyze_tints where temp &> tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' &> temp;
explain analyze select * from test_analyze_tints where temp #>> tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' #>> temp;
explain analyze select * from test_analyze_tints where temp <<# tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' <<# temp;
explain analyze select * from test_analyze_tints where temp #&> tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' #&> temp;
explain analyze select * from test_analyze_tints where temp &<# tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]';
explain analyze select * from test_analyze_tints where tfloat '[54.9@2001-01-25 18:24:00+02, 74.9@2001-04-25 18:24:00+02]' &<# temp;


--TINTS op INTRANGE
explain analyze select * from test_analyze_tints where temp && intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' && temp;
explain analyze select * from test_analyze_tints where temp @> intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' @> temp;
explain analyze select * from test_analyze_tints where temp <@ intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' <@ temp;
explain analyze select * from test_analyze_tints where temp << intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' << temp;
explain analyze select * from test_analyze_tints where temp >> intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' >> temp;
explain analyze select * from test_analyze_tints where temp &< intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' &< temp;
explain analyze select * from test_analyze_tints where temp &> intrange '[50, 60]';
explain analyze select * from test_analyze_tints where intrange '[50, 60]' &> temp;
-----------------------------------------------------------------------------
