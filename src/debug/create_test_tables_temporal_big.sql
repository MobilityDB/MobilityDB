DROP FUNCTION IF EXISTS create_test_tables_temporal_big();
CREATE OR REPLACE FUNCTION create_test_tables_temporal_big(size int DEFAULT 100) 
RETURNS text AS $$
DECLARE
	perc int;
BEGIN
perc := size * 0.01;
IF perc < 1 THEN perc := 1; END IF;

-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

drop table if exists tbl_timestampset_big;
create table tbl_timestampset_big as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_timestampset('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_period_big;
create table tbl_period_big as
/* Add perc NULL values */
select k, NULL as p
from generate_series(1, perc) as k union
select k, random_period('2001-01-01', '2001-12-31', 10)
from generate_series(perc+1, size) as k;

drop table if exists tbl_periodset_big;
create table tbl_periodset_big as
/* Add perc NULL values */
select k, NULL as ps
from generate_series(1, perc) as k union
select k, random_periodset('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) as k;

------------------------------------------------------------------------------
-- Temporal Types
------------------------------------------------------------------------------

drop table if exists tbl_tboolinst_big;
create table tbl_tboolinst_big as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tboolinst('2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tboolinst_big t1
set inst = (select inst from tbl_tboolinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tboolinst_big t1
set inst = (select tboolinst(random_bool(), getTimestamp(inst)) 
	from tbl_tboolinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_tintinst_big;
create table tbl_tintinst_big as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tintinst(1, 100, '2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tintinst_big t1
set inst = (select inst from tbl_tintinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tintinst_big t1
set inst = (select tintinst(random_int(1, 100), getTimestamp(inst)) 
	from tbl_tintinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_tfloatinst_big;
create table tbl_tfloatinst_big as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_tfloatinst(1, 100, '2001-01-01', '2001-12-31')
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloatinst_big t1
set inst = (select inst from tbl_tfloatinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloatinst_big t1
set inst = (select tfloatinst(random_float(1, 100), getTimestamp(inst)) 
	from tbl_tfloatinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

drop table if exists tbl_ttextinst_big;
create table tbl_ttextinst_big as
/* Add perc NULL values */
select k, NULL as inst
from generate_series(1, perc) as k union
select k, random_ttextinst('2001-01-01', '2001-12-31', 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttextinst_big t1
set inst = (select inst from tbl_ttextinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttextinst_big t1
set inst = (select ttextinst(random_text(10), getTimestamp(inst)) 
	from tbl_ttextinst_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbooli_big;
create table tbl_tbooli_big as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tbooli('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tbooli_big t1
set ti = (select ti from tbl_tbooli_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tbooli_big t1
set ti = (select ~ ti from tbl_tbooli_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tbooli_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tbooli_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tbooli_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tbooli_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tinti_big;
create table tbl_tinti_big as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tinti(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tinti_big t1
set ti = (select ti from tbl_tinti_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tinti_big t1
set ti = (select ti + random_int(1, 2) from tbl_tinti_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tinti_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tinti_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tinti_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tinti_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloati_big;
create table tbl_tfloati_big as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_tfloati(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloati_big t1
set ti = (select ti from tbl_tfloati_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloati_big t1
set ti = (select ti + random_int(1, 2) from tbl_tfloati_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloati_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_tfloati_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloati_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_tfloati_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttexti_big;
create table tbl_ttexti_big as
/* Add perc NULL values */
select k, NULL as ti
from generate_series(1, perc) as k union
select k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttexti_big t1
set ti = (select ti from tbl_ttexti_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttexti_big t1
set ti = (select ti || text 'A' from tbl_ttexti_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttexti_big t1
set ti = (select shift(ti, endTimestamp(ti)-startTimestamp(ti)) 
	from tbl_ttexti_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttexti_big t1
set ti = (select shift(ti, date_trunc('minute',(endTimestamp(ti)-startTimestamp(ti))/2)) 
	from tbl_ttexti_big t2 where t2.k = t1.k+2)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tboolseq_big;
create table tbl_tboolseq_big as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tboolseq('2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tboolseq_big t1
set seq = (select seq from tbl_tboolseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tboolseq_big t1
set seq = (select ~ seq from tbl_tboolseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tboolseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tboolseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tboolseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tboolseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tintseq_big;
create table tbl_tintseq_big as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tintseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tintseq_big t1
set seq = (select seq from tbl_tintseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tintseq_big t1
set seq = (select seq + random_int(1, 2) from tbl_tintseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tintseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tintseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tintseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tintseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloatseq_big;
create table tbl_tfloatseq_big as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_tfloatseq(1, 100, '2001-01-01', '2001-12-31', 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_tfloatseq_big t1
set seq = (select seq from tbl_tfloatseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloatseq_big t1
set seq = (select seq + random_int(1, 2) from tbl_tfloatseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloatseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_tfloatseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloatseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_tfloatseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttextseq_big;
create table tbl_ttextseq_big as
/* Add perc NULL values */
select k, NULL as seq
from generate_series(1, perc) as k union
select k, random_ttextseq('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) k;
/* Add perc duplicates */
update tbl_ttextseq_big t1
set seq = (select seq from tbl_ttextseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttextseq_big t1
set seq = (select seq || text 'A' from tbl_ttextseq_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttextseq_big t1
set seq = (select shift(seq, timespan(seq)) from tbl_ttextseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttextseq_big t1
set seq = (select shift(seq, date_trunc('minute',timespan(seq)/2)) 
	from tbl_ttextseq_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbools_big;
create table tbl_tbools_big as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tbools('2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tbools_big t1
set ts = (select ts from tbl_tbools_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tbools_big t1
set ts = (select ~ ts from tbl_tbools_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tbools_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tbools_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tbools_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tbools_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tints_big;
create table tbl_tints_big as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tints_big t1
set ts = (select ts from tbl_tints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tints_big t1
set ts = (select ts + random_int(1, 2) from tbl_tints_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tints_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tints_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tints_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_tfloats_big;
create table tbl_tfloats_big as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_tfloats_big t1
set ts = (select ts from tbl_tfloats_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_tfloats_big t1
set ts = (select ts + random_int(1, 2) from tbl_tfloats_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_tfloats_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_tfloats_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_tfloats_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_tfloats_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

drop table if exists tbl_ttexts_big;
create table tbl_ttexts_big as
/* Add perc NULL values */
select k, NULL as ts
from generate_series(1, perc) as k union
select k, random_ttexts('2001-01-01', '2001-12-31', 10, 10, 10, 10)
from generate_series(perc+1, size) as k;
/* Add perc duplicates */
update tbl_ttexts_big t1
set ts = (select ts from tbl_ttexts_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 2*perc, 3*perc) i);
/* Add perc tuples with the same timestamp */
update tbl_ttexts_big t1
set ts = (select ts || text 'A' from tbl_ttexts_big t2 where t2.k = t1.k+perc)
where k in (select i from generate_series(1 + 4*perc, 5*perc) i);
/* Add perc tuples that meet */
update tbl_ttexts_big t1
set ts = (select shift(ts, timespan(ts)) from tbl_ttexts_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 6*perc, 7*perc) i);
/* Add perc tuples that overlap */
update tbl_ttexts_big t1
set ts = (select shift(ts, date_trunc('minute', timespan(ts)/2)) 
	from tbl_ttexts_big t2 where t2.k = t1.k+perc)
where t1.k in (select i from generate_series(1 + 8*perc, 9*perc) i);

-------------------------------------------------------------------------------

drop table if exists tbl_tbool_big;
create table tbl_tbool_big(k, temp) as
(select k, inst from tbl_tboolinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tbooli_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tboolseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tbools_big order by k limit size / 4);

drop table if exists tbl_tint_big;
create table tbl_tint_big(k, temp) as
(select k, inst from tbl_tinttinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tinti_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tintseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tints_big order by k limit size / 4);

drop table if exists tbl_tfloat_big;
create table tbl_tfloat_big(k, temp) as
(select k, inst from tbl_tfloatinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_tfloati_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_tfloatseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_tfloats_big order by k limit size / 4);

drop table if exists tbl_ttext_big;
create table tbl_ttext_big(k, temp) as
(select k, inst from tbl_ttextinst_big order by k limit size / 4) union all
(select k + size / 4, ti from tbl_ttexti_big order by k limit size / 4) union all
(select k + size / 2, seq from tbl_ttextseq_big order by k limit size / 4) union all
(select k + size / 4 * 3, ts from tbl_ttexts_big order by k limit size / 4);

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- select create_test_tables_temporal_big(10000)