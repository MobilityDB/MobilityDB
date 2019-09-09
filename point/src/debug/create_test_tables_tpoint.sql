DROP FUNCTION IF EXISTS create_test_tables_tpoint();
CREATE OR REPLACE FUNCTION create_test_tables_tpoint(size int DEFAULT 100) 
RETURNS text AS $$
DECLARE
	perc int;
BEGIN
perc := size * 0.02;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Geo types
-- In the following tables, geography points are restricted to the bounding  
-- box covering approximately continental Europe, that is, "BOX(-10 32,35 72)"
-------------------------------------------------------------------------------

drop table if exists tbl_stbox;
create table tbl_stbox as
select k, random_stbox(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as b
from generate_series(1, size) k;

drop table if exists tbl_geompoint;
create table tbl_geompoint as
select 1 as k, geometry 'point empty' as g union
select k, random_geompoint(0, 100, 0, 100)
from generate_series(2, size) k;

drop table if exists tbl_geompoint3D;
create table tbl_geompoint3D as
select 1 as k, geometry 'pointZ empty' as g union
select k, random_geompoint3D(0, 100, 0, 100, 0, 100)
from generate_series(2, size) k;

drop table if exists tbl_geogpoint;
create table tbl_geogpoint as
select 1 as k, geography 'point empty' as g union
select k, random_geogpoint(-10, 32, 35, 72)
from generate_series(2, size) k;

drop table if exists tbl_geogpoint3D;
create table tbl_geogpoint3D as
select 1 as k, geography 'pointZ empty' as g union
select k, random_geogpoint3D(-10, 32, 35, 72, 0, 1000)
from generate_series(2, size) k;

drop table if exists tbl_geomlinestring;
create table tbl_geomlinestring as
select 1 as k, geometry 'linestring empty' as g union
select k, random_geomlinestring(0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geomlinestring3D;
create table tbl_geomlinestring3D as
select 1 as k, geometry 'linestring Z empty' as g union
select k, random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geoglinestring;
create table tbl_geoglinestring as
select 1 as k, geography 'linestring empty' as g union
select k, random_geoglinestring(-10, 32, 35, 72, 10)
from generate_series(2, size) k;

drop table if exists tbl_geoglinestring3D;
create table tbl_geoglinestring3D as
select 1 as k, geography 'linestring Z empty' as g union
select k, random_geoglinestring3D(-10, 32, 35, 72, 0, 1000, 10) 
from generate_series(2, size) k;

drop table if exists tbl_geompolygon;
create table tbl_geompolygon as
select 1 as k, geometry 'polygon empty' as g union
select k, random_geompolygon(0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geompolygon3D;
create table tbl_geompolygon3D as
select 1 as k, geometry 'polygon Z empty' as g union
select k, random_geompolygon3D(0, 100, 0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogpolygon;
create table tbl_geogpolygon as
select 1 as k, geography 'polygon empty' as g union
select k, random_geogpolygon(-10, 32, 35, 72, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogpolygon3D;
create table tbl_geogpolygon3D as
select 1 as k, geography 'polygon Z empty' as g union
select k, random_geogpolygon3D(-10, 32, 35, 72, 0, 1000, 10)
from generate_series(2, size) k;

drop table if exists tbl_geommultipoint;
create table tbl_geommultipoint as
select 1 as k, geometry 'multipoint empty' as g union
select k, random_geommultipoint(0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geommultipoint3D;
create table tbl_geommultipoint3D as
select 1 as k, geometry 'multipoint Z empty' as g union
select k, random_geommultipoint3D(0, 100, 0, 100, 0, 100, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultipoint;
create table tbl_geogmultipoint as
select 1 as k, geography 'multipoint empty' as g union
select k, random_geogmultipoint(-10, 32, 35, 72, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultipoint3D;
create table tbl_geogmultipoint3D as
select 1 as k, geography 'multipoint Z empty' as g union
select k, random_geogmultipoint3D(-10, 32, 35, 72, 10, 0, 1000)
from generate_series(2, size) k;

drop table if exists tbl_geommultilinestring;
create table tbl_geommultilinestring as
select 1 as k, geometry 'multilinestring empty' as g union
select k, random_geommultilinestring(0, 100, 0, 100, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geommultilinestring3D;
create table tbl_geommultilinestring3D as
select 1 as k, geometry 'multilinestring Z empty' as g union
select k, random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultilinestring;
create table tbl_geogmultilinestring as
select 1 as k, geography 'multilinestring empty' as g union
select k, random_geogmultilinestring(-10, 32, 35, 72, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultilinestring3D;
create table tbl_geogmultilinestring3D as
select 1 as k, geography 'multilinestring Z empty' as g union
select k, random_geogmultilinestring3D(-10, 32, 35, 72, 0, 1000, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geommultipolygon;
create table tbl_geommultipolygon as
select 1 as k, geometry 'multipolygon empty' as g union
select k, random_geommultipolygon(0, 100, 0, 100, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geommultipolygon3D;
create table tbl_geommultipolygon3D as
select 1 as k, geometry 'multipolygon Z empty' as g union
select k, random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultipolygon;
create table tbl_geogmultipolygon as
select 1 as k, geography 'multipolygon empty' as g union
select k, random_geogmultipolygon(-10, 32, 35, 72, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geogmultipolygon3D;
create table tbl_geogmultipolygon3D as
select 1 as k, geography 'multipolygon Z empty' as g union
select k, random_geogmultipolygon3D(-10, 32, 35, 72, 0, 1000, 10, 10)
from generate_series(2, size) k;

drop table if exists tbl_geometry;
create table tbl_geometry (
	k serial primary key,
	g geometry);
insert into tbl_geometry(g)
(select g from tbl_geompoint order by k limit (size * 0.1)) union all
(select g from tbl_geomlinestring order by k limit (size * 0.1)) union all
(select g from tbl_geompolygon order by k limit (size * 0.2)) union all
(select g from tbl_geommultipoint order by k limit (size * 0.2)) union all
(select g from tbl_geommultilinestring order by k limit (size * 0.2)) union all
(select g from tbl_geommultipolygon order by k limit (size * 0.2));

drop table if exists tbl_geometry3D;
create table tbl_geometry3D (
	k serial primary key,
	g geometry);
insert into tbl_geometry3D(g)
(select g from tbl_geompoint3D order by k limit (size * 0.1)) union all
(select g from tbl_geomlinestring3D order by k limit (size * 0.1)) union all
(select g from tbl_geompolygon3D order by k limit (size * 0.2)) union all
(select g from tbl_geommultipoint3D order by k limit (size * 0.2)) union all
(select g from tbl_geommultilinestring3D order by k limit (size * 0.2)) union all
(select g from tbl_geommultipolygon3D order by k limit (size * 0.2));

drop table if exists tbl_geography;
create table tbl_geography (
	k serial primary key,
	g geography);
insert into tbl_geography(g)
(select g from tbl_geogpoint order by k limit (size * 0.1)) union all
(select g from tbl_geoglinestring order by k limit (size * 0.1)) union all
(select g from tbl_geogpolygon order by k limit (size * 0.2)) union all
(select g from tbl_geogmultipoint order by k limit (size * 0.2)) union all
(select g from tbl_geogmultilinestring order by k limit (size * 0.2)) union all
(select g from tbl_geogmultipolygon order by k limit (size * 0.2));

drop table if exists tbl_geography3D;
create table tbl_geography3D (
	k serial primary key,
	g geography);
insert into tbl_geography3D(g)
(select g from tbl_geogpoint3D order by k limit (size * 0.1)) union all
(select g from tbl_geoglinestring3D order by k limit (size * 0.1)) union all
(select g from tbl_geogpolygon3D order by k limit (size * 0.2)) union all
(select g from tbl_geogmultipoint3D order by k limit (size * 0.2)) union all
(select g from tbl_geogmultilinestring3D order by k limit (size * 0.2)) union all
(select g from tbl_geogmultipolygon3D order by k limit (size * 0.2));

------------------------------------------------------------------------------
-- Temporal Point Types
------------------------------------------------------------------------------

drop table if exists tbl_tgeompointinst;
create table tbl_tgeompointinst as
select k, random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointinst t1
set inst = (select inst from tbl_tgeompointinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointinst t1
set inst = (select tgeompointinst(random_geompoint(0, 100, 0, 100), getTimestamp(inst)) 
	from tbl_tgeompointinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeompoint3Dinst;
create table tbl_tgeompoint3Dinst as
select k, random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Dinst t1
set inst = (select inst from tbl_tgeompoint3Dinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Dinst t1
set inst = (select tgeompointinst(random_geompoint3D(0, 100, 0, 100, 0, 100), getTimestamp(inst)) 
	from tbl_tgeompoint3Dinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeogpointinst;
create table tbl_tgeogpointinst as
select k, random_tgeogpointinst(-10, 32, 35, 72, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointinst t1
set inst = (select inst from tbl_tgeogpointinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointinst t1
set inst = (select tgeogpointinst(random_geogpoint(-10, 32, 35, 72), getTimestamp(inst)) 
	from tbl_tgeogpointinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

drop table if exists tbl_tgeogpoint3Dinst;
create table tbl_tgeogpoint3Dinst as
select k, random_tgeogpoint3Dinst(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31') as inst
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Dinst t1
set inst = (select inst from tbl_tgeogpoint3Dinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Dinst t1
set inst = (select tgeogpointinst(random_geogpoint3D(-10, 32, 35, 72, 0, 1000), getTimestamp(inst)) 
	from tbl_tgeogpoint3Dinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompointi;
create table tbl_tgeompointi as
select k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointi t1
set ti = (select ti from tbl_tgeompointi t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointi t1
set ti = (select setPrecision(ti,6) from tbl_tgeompointi t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompointi t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeompointi t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompointi t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeompointi t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Di;
create table tbl_tgeompoint3Di as
select k, random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Di t1
set ti = (select ti from tbl_tgeompoint3Di t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Di t1
set ti = (select setPrecision(ti,3) from tbl_tgeompoint3Di t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Di t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeompoint3Di t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Di t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeompoint3Di t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpointi;
create table tbl_tgeogpointi as
select k, random_tgeogpointi(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointi t1
set ti = (select ti from tbl_tgeogpointi t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointi t1
set ti = (select setPrecision(ti,3) from tbl_tgeogpointi t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpointi t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeogpointi t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpointi t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeogpointi t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Di;
create table tbl_tgeogpoint3Di as
select k, random_tgeogpoint3Di(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) as ti
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Di t1
set ti = (select ti from tbl_tgeogpoint3Di t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Di t1
set ti = (select setPrecision(ti,3) from tbl_tgeogpoint3Di t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Di t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tgeogpoint3Di t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Di t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tgeogpoint3Di t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompointseq;
create table tbl_tgeompointseq as
select k, random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompointseq t1
set seq = (select seq from tbl_tgeompointseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompointseq t1
set seq = (select setPrecision(seq,3) from tbl_tgeompointseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompointseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tgeompointseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompointseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tgeompointseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Dseq;
create table tbl_tgeompoint3Dseq as
select k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeompoint3Dseq t1
set seq = (select seq from tbl_tgeompoint3Dseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Dseq t1
set seq = (select setPrecision(seq,3) from tbl_tgeompoint3Dseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Dseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tgeompoint3Dseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Dseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tgeompoint3Dseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpointseq;
create table tbl_tgeogpointseq as
select k, random_tgeogpointseq(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpointseq t1
set seq = (select seq from tbl_tgeogpointseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpointseq t1
set seq = (select setPrecision(seq,3) from tbl_tgeogpointseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpointseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tgeogpointseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpointseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tgeogpointseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Dseq;
create table tbl_tgeogpoint3Dseq as
select k, random_tgeogpoint3Dseq(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10) as seq
from generate_series(1, size) k;
/* Add perc duplicates */
update tbl_tgeogpoint3Dseq t1
set seq = (select seq from tbl_tgeogpoint3Dseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Dseq t1
set seq = (select setPrecision(seq,3) from tbl_tgeogpoint3Dseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Dseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tgeogpoint3Dseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Dseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tgeogpoint3Dseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompoints;
create table tbl_tgeompoints as
select k, random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeompoints t1
set ts = (select ts from tbl_tgeompoints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoints t1
set ts = (select setPrecision(ts,3) from tbl_tgeompoints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoints t1
set ts = (select shift(ts, duration(ts)) from tbl_tgeompoints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoints t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tgeompoints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeompoint3Ds;
create table tbl_tgeompoint3Ds as
select k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeompoint3Ds t1
set ts = (select ts from tbl_tgeompoint3Ds t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeompoint3Ds t1
set ts = (select setPrecision(ts,3) from tbl_tgeompoint3Ds t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeompoint3Ds t1
set ts = (select shift(ts, duration(ts)) from tbl_tgeompoint3Ds t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeompoint3Ds t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tgeompoint3Ds t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoints;
create table tbl_tgeogpoints as
select k, random_tgeogpoints(-10, 32, 35, 72, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeogpoints t1
set ts = (select ts from tbl_tgeogpoints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoints t1
set ts = (select setPrecision(ts,3) from tbl_tgeogpoints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoints t1
set ts = (select shift(ts, duration(ts)) from tbl_tgeogpoints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoints t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tgeogpoints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

drop table if exists tbl_tgeogpoint3Ds;
create table tbl_tgeogpoint3Ds as
select k, random_tgeogpoint3Ds(-10, 32, 35, 72, 0, 1000, '2001-01-01', '2001-12-31', 10, 10, 10) as ts
from generate_series(1, size) as k;
/* Add perc duplicates */
update tbl_tgeogpoint3Ds t1
set ts = (select ts from tbl_tgeogpoint3Ds t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1, perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tgeogpoint3Ds t1
set ts = (select setPrecision(ts,3) from tbl_tgeogpoint3Ds t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples that meet */
update tbl_tgeogpoint3Ds t1
set ts = (select shift(ts, duration(ts)) from tbl_tgeogpoint3Ds t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that overlap */
update tbl_tgeogpoint3Ds t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tgeogpoint3Ds t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tgeompoint;
create table tbl_tgeompoint(k, temp) as
(select k, inst from tbl_tgeompointinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeompointi order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeompointseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeompoints order by k limit size / 4);

drop table if exists tbl_tgeompoint3D;
create table tbl_tgeompoint3D(k, temp) as
(select k, inst from tbl_tgeompoint3Dinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeompoint3Di order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeompoint3Dseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeompoint3Ds order by k limit size / 4);

drop table if exists tbl_tgeogpoint;
create table tbl_tgeogpoint(k, temp) as
(select k, inst from tbl_tgeogpointinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeogpointi order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeogpointseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeogpoints order by k limit size / 4);

drop table if exists tbl_tgeogpoint3D;
create table tbl_tgeogpoint3D(k, temp) as
(select k, inst from tbl_tgeogpoint3Dinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tgeogpoint3Di order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tgeogpoint3Dseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tgeogpoint3Ds order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- select create_test_tables_tpoint(100)
