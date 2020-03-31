DROP FUNCTION IF EXISTS create_test_tables_tpoint_big();
CREATE OR REPLACE FUNCTION create_test_tables_tpoint_big(size int DEFAULT 10000) 
RETURNS text AS $$
DECLARE
	perc int;
BEGIN
perc := size * 0.02;
IF perc < 1 THEN perc := 1; END IF;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

drop table if exists tbl_tgeompointinst_big;
create table tbl_tgeompointinst_big as
select k, random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointinst_big t1
set inst = (select inst from tbl_tgeompointinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointinst_big t1
set inst = (select tgeompointinst(random_geompoint(0, 100, 0, 100), getTimestamp(inst)) 
	from tbl_tgeompointinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeompoint3Dinst_big;
create table tbl_tgeompoint3Dinst_big as
select k, random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Dinst_big t1
set inst = (select inst from tbl_tgeompoint3Dinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Dinst_big t1
set inst = (select tgeompointinst(random_geompoint3D(0, 100, 0, 100, 0, 100), getTimestamp(inst)) 
	from tbl_tgeompoint3Dinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeogpointinst_big;
create table tbl_tgeogpointinst_big as
select k, random_tgeogpointinst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointinst_big t1
set inst = (select inst from tbl_tgeogpointinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointinst_big t1
set inst = (select tgeogpointinst(random_geogpoint(-10, 32, 35, 72), getTimestamp(inst)) 
	from tbl_tgeogpointinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeogpoint3Dinst_big;
create table tbl_tgeogpoint3Dinst_big as
select k, random_tgeogpoint3Dinst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Dinst_big t1
set inst = (select inst from tbl_tgeogpoint3Dinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Dinst_big t1
set inst = (select tgeogpointinst(random_geogpoint3D(-10, 32, 35, 72, 0, 1000), getTimestamp(inst)) 
	from tbl_tgeogpoint3Dinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompointi_big;
create table tbl_tgeompointi_big as
select k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointi_big t1
set ti = (select ti from tbl_tgeompointi_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointi_big t1
set ti = (select setPrecision(ti,6) from tbl_tgeompointi_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompointi_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeompointi_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompointi_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeompointi_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Di_big;
create table tbl_tgeompoint3Di_big as
select k, random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Di_big t1
set ti = (select ti from tbl_tgeompoint3Di_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Di_big t1
set ti = (select setPrecision(ti,3) from tbl_tgeompoint3Di_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Di_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeompoint3Di_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Di_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeompoint3Di_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpointi_big;
create table tbl_tgeogpointi_big as
select k, random_tgeogpointi(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointi_big t1
set ti = (select ti from tbl_tgeogpointi_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointi_big t1
set ti = (select setPrecision(ti,3) from tbl_tgeogpointi_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpointi_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeogpointi_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpointi_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeogpointi_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Di_big;
create table tbl_tgeogpoint3Di_big as
select k, random_tgeogpoint3Di(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Di_big t1
set ti = (select ti from tbl_tgeogpoint3Di_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Di_big t1
set ti = (select setPrecision(ti,3) from tbl_tgeogpoint3Di_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Di_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeogpoint3Di_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Di_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeogpoint3Di_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompointseq_big;
create table tbl_tgeompointseq_big as
select k, random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointseq_big t1
set seq = (select seq from tbl_tgeompointseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointseq_big t1
set seq = (select setPrecision(seq,3) from tbl_tgeompointseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompointseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tgeompointseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompointseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tgeompointseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Dseq_big;
create table tbl_tgeompoint3Dseq_big as
select k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Dseq_big t1
set seq = (select seq from tbl_tgeompoint3Dseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Dseq_big t1
set seq = (select setPrecision(seq,3) from tbl_tgeompoint3Dseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Dseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tgeompoint3Dseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Dseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tgeompoint3Dseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpointseq_big;
create table tbl_tgeogpointseq_big as
select k, random_tgeogpointseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointseq_big t1
set seq = (select seq from tbl_tgeogpointseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointseq_big t1
set seq = (select setPrecision(seq,3) from tbl_tgeogpointseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpointseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tgeogpointseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpointseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tgeogpointseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Dseq_big;
create table tbl_tgeogpoint3Dseq_big as
select k, random_tgeogpoint3Dseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Dseq_big t1
set seq = (select seq from tbl_tgeogpoint3Dseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Dseq_big t1
set seq = (select setPrecision(seq,3) from tbl_tgeogpoint3Dseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Dseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tgeogpoint3Dseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Dseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tgeogpoint3Dseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompoints_big;
create table tbl_tgeompoints_big as
select k, random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeompoints_big t1
set ts = (select ts from tbl_tgeompoints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoints_big t1
set ts = (select setPrecision(ts,3) from tbl_tgeompoints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoints_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tgeompoints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoints_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tgeompoints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Ds_big;
create table tbl_tgeompoint3Ds_big as
select k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeompoint3Ds_big t1
set ts = (select ts from tbl_tgeompoint3Ds_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Ds_big t1
set ts = (select setPrecision(ts,3) from tbl_tgeompoint3Ds_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Ds_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tgeompoint3Ds_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Ds_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tgeompoint3Ds_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoints_big;
create table tbl_tgeogpoints_big as
select k, random_tgeogpoints(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeogpoints_big t1
set ts = (select ts from tbl_tgeogpoints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoints_big t1
set ts = (select setPrecision(ts,3) from tbl_tgeogpoints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoints_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tgeogpoints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoints_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tgeogpoints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Ds_big;
create table tbl_tgeogpoint3Ds_big as
select k, random_tgeogpoint3Ds(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeogpoint3Ds_big t1
set ts = (select ts from tbl_tgeogpoint3Ds_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Ds_big t1
set ts = (select setPrecision(ts,3) from tbl_tgeogpoint3Ds_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Ds_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tgeogpoint3Ds_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Ds_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tgeogpoint3Ds_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompoint_big;
create table tbl_tgeompoint_big(k, temp) as
(select k, inst from tbl_tgeompointinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeompointi_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeompointseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeompoints_big order by k limit size / 4);

drop table if exists tbl_tgeompoint3D_big;
create table tbl_tgeompoint3D_big(k, temp) as
(select k, inst from tbl_tgeompoint3Dinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeompoint3Di_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeompoint3Dseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeompoint3Ds_big order by k limit size / 4);

drop table if exists tbl_tgeogpoint_big;
create table tbl_tgeogpoint_big(k, temp) as
(select k, inst from tbl_tgeogpointinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeogpointi_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeogpointseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeogpoints_big order by k limit size / 4);

drop table if exists tbl_tgeogpoint3D_big;
create table tbl_tgeogpoint3D_big(k, temp) as
(select k, inst from tbl_tgeogpoint3Dinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeogpoint3Di_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeogpoint3Dseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeogpoint3Ds_big order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- select create_test_tables_tpoint_big(10000)
