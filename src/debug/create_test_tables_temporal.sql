DROP FUNCTION IF EXISTS create_test_tables_temporal();
CREATE OR REPLACE FUNCTION create_test_tables_temporal(size int DEFAULT 100) 
RETURNS text AS $$
DECLARE
	perc int;
BEGIN
perc := size * 0.01;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

drop table if exists tbl_bool;
create table tbl_bool as
/* Add perc NULL values */
select k, NULL as b
from generate_series(1, perc) as k union
select k, random_bool()
from generate_series(perc+1, size) as k;

drop table if exists tbl_int;
create table tbl_int as
/* Add perc NULL values */
select k, NULL as i
from generate_series(1, perc) as k union
select k, random_int(1, 100)
from generate_series(perc+1, size) as k;

drop table if exists tbl_float;
create table tbl_float as
/* Add perc NULL values */
select k, NULL as f
from generate_series(1, perc) as k union
select k, random_float(1, 100)
from generate_series(perc+1, size) as k;

drop table if exists tbl_text;
create table tbl_text as
/* Add perc NULL values */
select k, NULL as t
from generate_series(1, perc) as k union
select k, random_text(10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_tbox;
create table tbl_tbox as
/* Add perc NULL values */
select k, NULL as b
from generate_series(1, perc) as k union
select k, random_tbox(0, 100, 0, 100, 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_interval;
create table tbl_interval as
/* Add perc NULL values */
select k, NULL as i
from generate_series(1, perc) as k union
select k, random_minutes(1, 100)
from generate_series(perc+1, size) as k;

drop table if exists tbl_timestamptz;
create table tbl_timestamptz as
/* Add perc NULL values */
select k, NULL as t
from generate_series(1, perc) as k union
select k, random_timestamptz('2001-01-01', '2001-12-31')
from generate_series(perc+1, size) as k;

drop table if exists tbl_intrange;
create table tbl_intrange as
/* Add perc NULL values */
select k, NULL as i
from generate_series(1, perc) as k union
select k, random_intrange(1, 100, 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_floatrange;
create table tbl_floatrange as
/* Add perc NULL values */
select k, NULL as f
from generate_series(1, perc) as k union
select k, random_floatrange(1, 100, 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_tstzrange;
create table tbl_tstzrange as
/* Add perc NULL values */
select k, NULL as r
from generate_series(1, perc) as k union
select k, random_tstzrange('2001-01-01', '2001-12-31', 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_tstzrangearr;
create table tbl_tstzrangearr as
/* Add perc NULL values */
select k, NULL as ra
from generate_series(1, perc) as k union
select k, random_tstzrangearr('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) as k;

-------------------------------------------------------------------------------
-- Time types
-------------------------------------------------------------------------------

drop table if exists tbl_timestampset;
create table tbl_timestampset as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_timestampset('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_period;
create table tbl_period as
/* Add perc NULL values */
select k, NULL as p
from generate_series(1, perc) as k union
select k, random_period('2001-01-01', '2001-12-31', 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_periodset;
create table tbl_periodset as
/* Add perc NULL values */
select k, NULL as ps
from generate_series(1, perc) as k union
select k, random_periodset('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) as k;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

drop table if exists tbl_tboolinst;
create table tbl_tboolinst as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tboolinst('2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tboolinst t1
set inst = (select inst from tbl_tboolinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tboolinst t1
set inst = (select tboolinst(random_bool(), getTimestamp(inst)) 
	from tbl_tboolinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_tintinst;
create table tbl_tintinst as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tintinst(1, 100, '2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tintinst t1
set inst = (select inst from tbl_tintinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tintinst t1
set inst = (select tintinst(random_int(1, 100), getTimestamp(inst)) 
	from tbl_tintinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_tfloatinst;
create table tbl_tfloatinst as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tfloatinst(1, 100, '2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloatinst t1
set inst = (select inst from tbl_tfloatinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloatinst t1
set inst = (select tfloatinst(random_float(1, 100), getTimestamp(inst)) 
	from tbl_tfloatinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_ttextinst;
create table tbl_ttextinst as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_ttextinst('2001-01-01', '2001-12-31', 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttextinst t1
set inst = (select inst from tbl_ttextinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttextinst t1
set inst = (select ttextinst(random_text(10), getTimestamp(inst)) 
	from tbl_ttextinst t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbooli;
create table tbl_tbooli as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tbooli('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tbooli t1
set ti = (select ti from tbl_tbooli t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tbooli t1
set ti = (select ~ ti from tbl_tbooli t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tbooli t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tbooli t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tbooli t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tbooli t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tinti;
create table tbl_tinti as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tinti(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tinti t1
set ti = (select ti from tbl_tinti t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tinti t1
set ti = (select ti + random_int(1, 2) from tbl_tinti t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tinti t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tinti t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tinti t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tinti t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloati;
create table tbl_tfloati as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tfloati(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloati t1
set ti = (select ti from tbl_tfloati t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloati t1
set ti = (select ti + random_int(1, 2) from tbl_tfloati t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloati t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tfloati t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloati t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tfloati t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttexti;
create table tbl_ttexti as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttexti t1
set ti = (select ti from tbl_ttexti t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttexti t1
set ti = (select ti || text 'A' from tbl_ttexti t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttexti t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_ttexti t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttexti t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_ttexti t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tboolseq;
create table tbl_tboolseq as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tboolseq('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tboolseq t1
set seq = (select seq from tbl_tboolseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tboolseq t1
set seq = (select ~ seq from tbl_tboolseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tboolseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tboolseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tboolseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tboolseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tintseq;
create table tbl_tintseq as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tintseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tintseq t1
set seq = (select seq from tbl_tintseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tintseq t1
set seq = (select seq + random_int(1, 2) from tbl_tintseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tintseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tintseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tintseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tintseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloatseq;
create table tbl_tfloatseq as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tfloatseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloatseq t1
set seq = (select seq from tbl_tfloatseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloatseq t1
set seq = (select seq + random_int(1, 2) from tbl_tfloatseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloatseq t1
set seq = (select shift(seq, duration(seq)) from tbl_tfloatseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloatseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_tfloatseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttextseq;
create table tbl_ttextseq as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_ttextseq('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttextseq t1
set seq = (select seq from tbl_ttextseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttextseq t1
set seq = (select seq || text 'A' from tbl_ttextseq t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttextseq t1
set seq = (select shift(seq, duration(seq)) from tbl_ttextseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttextseq t1
set seq = (select shift(seq, date_trunc('minute',duration(seq)/2)) 
	from tbl_ttextseq t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbools;
create table tbl_tbools as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tbools('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tbools t1
set ts = (select ts from tbl_tbools t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tbools t1
set ts = (select ~ ts from tbl_tbools t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tbools t1
set ts = (select shift(ts, duration(ts)) from tbl_tbools t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tbools t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tbools t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tints;
create table tbl_tints as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tints t1
set ts = (select ts from tbl_tints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tints t1
set ts = (select ts + random_int(1, 2) from tbl_tints t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tints t1
set ts = (select shift(ts, duration(ts)) from tbl_tints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tints t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tints t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloats;
create table tbl_tfloats as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tfloats t1
set ts = (select ts from tbl_tfloats t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloats t1
set ts = (select ts + random_int(1, 2) from tbl_tfloats t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloats t1
set ts = (select shift(ts, duration(ts)) from tbl_tfloats t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloats t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_tfloats t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttexts;
create table tbl_ttexts as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_ttexts('2001-01-01', '2001-12-31', 10, 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_ttexts t1
set ts = (select ts from tbl_ttexts t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttexts t1
set ts = (select ts || text 'A' from tbl_ttexts t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttexts t1
set ts = (select shift(ts, duration(ts)) from tbl_ttexts t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttexts t1
set ts = (select shift(ts, date_trunc('minute', duration(ts)/2)) 
	from tbl_ttexts t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbool;
create table tbl_tbool(k, temp) as
(select k, inst from tbl_tboolinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tbooli order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tboolseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tbools order by k limit size / 4);

drop table if exists tbl_tint;
create table tbl_tint(k, temp) as
(select k, inst from tbl_tintinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tinti order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tintseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tints order by k limit size / 4);

drop table if exists tbl_tfloat;
create table tbl_tfloat(k, temp) as
(select k, inst from tbl_tfloatinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tfloati order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tfloatseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tfloats order by k limit size / 4);

drop table if exists tbl_ttext;
create table tbl_ttext(k, temp) as
(select k, inst from tbl_ttextinst order by k limit size / 4) union all
(select k + size / 4, ti from tbl_ttexti order by k limit size / 4) union all
(select k + size / 2, seq from tbl_ttextseq order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_ttexts order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- select create_test_tables_temporal(100)