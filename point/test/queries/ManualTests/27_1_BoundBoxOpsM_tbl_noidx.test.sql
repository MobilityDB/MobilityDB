/*****************************************************************************/

DROP INDEX IF EXISTS tbl_tgeompointinst_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompointinst_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpointinst_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpointinst_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeompointi_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompointi_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpointi_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpointi_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeompointseq_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompointseq_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpointseq_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpointseq_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeompoints_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompoints_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpoints_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoints_spgist_idx;

/*****************************************************************************/

drop table if exists test_geoboundboxops;
create table test_geoboundboxops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint,
	spgistidx bigint );

/*****************************************************************************/

select gbox(inst) from tbl_tgeompointinst LIMIT 1000;
select gbox(ti) from tbl_tgeompointi LIMIT 1000;
select gbox(seq) from tbl_tgeompointseq LIMIT 1000;
select gbox(ts) from tbl_tgeompoints LIMIT 1000;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompoint', 'tgeompointinst', count(*) from tbl_geompoint, tbl_tgeompointinst where g && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompoint', 'tgeompointinst', count(*) from tbl_geompoint, tbl_tgeompointinst where g @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompoint', 'tgeompointinst', count(*) from tbl_geompoint, tbl_tgeompointinst where g <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompoint', 'tgeompointinst', count(*) from tbl_geompoint, tbl_tgeompointinst where g ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompoint', 'tgeompointi', count(*) from tbl_geompoint, tbl_tgeompointi where g && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompoint', 'tgeompointi', count(*) from tbl_geompoint, tbl_tgeompointi where g @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompoint', 'tgeompointi', count(*) from tbl_geompoint, tbl_tgeompointi where g <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompoint', 'tgeompointi', count(*) from tbl_geompoint, tbl_tgeompointi where g ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompoint', 'tgeompointseq', count(*) from tbl_geompoint, tbl_tgeompointseq where g && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompoint', 'tgeompointseq', count(*) from tbl_geompoint, tbl_tgeompointseq where g @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompoint', 'tgeompointseq', count(*) from tbl_geompoint, tbl_tgeompointseq where g <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompoint', 'tgeompointseq', count(*) from tbl_geompoint, tbl_tgeompointseq where g ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompoint', 'tgeompoints', count(*) from tbl_geompoint, tbl_tgeompoints where g && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompoint', 'tgeompoints', count(*) from tbl_geompoint, tbl_tgeompoints where g @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompoint', 'tgeompoints', count(*) from tbl_geompoint, tbl_tgeompoints where g <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompoint', 'tgeompoints', count(*) from tbl_geompoint, tbl_tgeompoints where g ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompolygon', 'tgeompointinst', count(*) from tbl_geompolygon, tbl_tgeompointinst where g && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompolygon', 'tgeompointinst', count(*) from tbl_geompolygon, tbl_tgeompointinst where g @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompolygon', 'tgeompointinst', count(*) from tbl_geompolygon, tbl_tgeompointinst where g <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompolygon', 'tgeompointinst', count(*) from tbl_geompolygon, tbl_tgeompointinst where g ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompolygon', 'tgeompointi', count(*) from tbl_geompolygon, tbl_tgeompointi where g && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompolygon', 'tgeompointi', count(*) from tbl_geompolygon, tbl_tgeompointi where g @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompolygon', 'tgeompointi', count(*) from tbl_geompolygon, tbl_tgeompointi where g <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompolygon', 'tgeompointi', count(*) from tbl_geompolygon, tbl_tgeompointi where g ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompolygon', 'tgeompointseq', count(*) from tbl_geompolygon, tbl_tgeompointseq where g && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompolygon', 'tgeompointseq', count(*) from tbl_geompolygon, tbl_tgeompointseq where g @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompolygon', 'tgeompointseq', count(*) from tbl_geompolygon, tbl_tgeompointseq where g <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompolygon', 'tgeompointseq', count(*) from tbl_geompolygon, tbl_tgeompointseq where g ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'geompolygon', 'tgeompoints', count(*) from tbl_geompolygon, tbl_tgeompoints where g && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'geompolygon', 'tgeompoints', count(*) from tbl_geompolygon, tbl_tgeompoints where g @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'geompolygon', 'tgeompoints', count(*) from tbl_geompolygon, tbl_tgeompoints where g <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'geompolygon', 'tgeompoints', count(*) from tbl_geompolygon, tbl_tgeompoints where g ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestamptz', 'tgeompointinst', count(*) from tbl_timestamptz, tbl_tgeompointinst where t && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestamptz', 'tgeompointinst', count(*) from tbl_timestamptz, tbl_tgeompointinst where t @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestamptz', 'tgeompointinst', count(*) from tbl_timestamptz, tbl_tgeompointinst where t <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestamptz', 'tgeompointinst', count(*) from tbl_timestamptz, tbl_tgeompointinst where t ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestamptz', 'tgeompointi', count(*) from tbl_timestamptz, tbl_tgeompointi where t && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestamptz', 'tgeompointi', count(*) from tbl_timestamptz, tbl_tgeompointi where t @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestamptz', 'tgeompointi', count(*) from tbl_timestamptz, tbl_tgeompointi where t <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestamptz', 'tgeompointi', count(*) from tbl_timestamptz, tbl_tgeompointi where t ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestamptz', 'tgeompointseq', count(*) from tbl_timestamptz, tbl_tgeompointseq where t && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestamptz', 'tgeompointseq', count(*) from tbl_timestamptz, tbl_tgeompointseq where t @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestamptz', 'tgeompointseq', count(*) from tbl_timestamptz, tbl_tgeompointseq where t <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestamptz', 'tgeompointseq', count(*) from tbl_timestamptz, tbl_tgeompointseq where t ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestamptz', 'tgeompoints', count(*) from tbl_timestamptz, tbl_tgeompoints where t && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestamptz', 'tgeompoints', count(*) from tbl_timestamptz, tbl_tgeompoints where t @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestamptz', 'tgeompoints', count(*) from tbl_timestamptz, tbl_tgeompoints where t <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestamptz', 'tgeompoints', count(*) from tbl_timestamptz, tbl_tgeompoints where t ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestampset', 'tgeompointinst', count(*) from tbl_timestampset, tbl_tgeompointinst where ts && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestampset', 'tgeompointinst', count(*) from tbl_timestampset, tbl_tgeompointinst where ts @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestampset', 'tgeompointinst', count(*) from tbl_timestampset, tbl_tgeompointinst where ts <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestampset', 'tgeompointinst', count(*) from tbl_timestampset, tbl_tgeompointinst where ts ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestampset', 'tgeompointi', count(*) from tbl_timestampset, tbl_tgeompointi where ts && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestampset', 'tgeompointi', count(*) from tbl_timestampset, tbl_tgeompointi where ts @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestampset', 'tgeompointi', count(*) from tbl_timestampset, tbl_tgeompointi where ts <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestampset', 'tgeompointi', count(*) from tbl_timestampset, tbl_tgeompointi where ts ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestampset', 'tgeompointseq', count(*) from tbl_timestampset, tbl_tgeompointseq where ts && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestampset', 'tgeompointseq', count(*) from tbl_timestampset, tbl_tgeompointseq where ts @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestampset', 'tgeompointseq', count(*) from tbl_timestampset, tbl_tgeompointseq where ts <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestampset', 'tgeompointseq', count(*) from tbl_timestampset, tbl_tgeompointseq where ts ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'timestampset', 'tgeompoints', count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts && t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'timestampset', 'tgeompoints', count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts @> t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'timestampset', 'tgeompoints', count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts <@ t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'timestampset', 'tgeompoints', count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts ~= t2.ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'period', 'tgeompointinst', count(*) from tbl_period, tbl_tgeompointinst where p && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'period', 'tgeompointinst', count(*) from tbl_period, tbl_tgeompointinst where p @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'period', 'tgeompointinst', count(*) from tbl_period, tbl_tgeompointinst where p <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'period', 'tgeompointinst', count(*) from tbl_period, tbl_tgeompointinst where p ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'period', 'tgeompointi', count(*) from tbl_period, tbl_tgeompointi where p && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'period', 'tgeompointi', count(*) from tbl_period, tbl_tgeompointi where p @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'period', 'tgeompointi', count(*) from tbl_period, tbl_tgeompointi where p <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'period', 'tgeompointi', count(*) from tbl_period, tbl_tgeompointi where p ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'period', 'tgeompointseq', count(*) from tbl_period, tbl_tgeompointseq where p && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'period', 'tgeompointseq', count(*) from tbl_period, tbl_tgeompointseq where p @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'period', 'tgeompointseq', count(*) from tbl_period, tbl_tgeompointseq where p <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'period', 'tgeompointseq', count(*) from tbl_period, tbl_tgeompointseq where p ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'period', 'tgeompoints', count(*) from tbl_period, tbl_tgeompoints where p && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'period', 'tgeompoints', count(*) from tbl_period, tbl_tgeompoints where p @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'period', 'tgeompoints', count(*) from tbl_period, tbl_tgeompoints where p <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'period', 'tgeompoints', count(*) from tbl_period, tbl_tgeompoints where p ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'periodset', 'tgeompointinst', count(*) from tbl_periodset, tbl_tgeompointinst where ps && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'periodset', 'tgeompointinst', count(*) from tbl_periodset, tbl_tgeompointinst where ps @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'periodset', 'tgeompointinst', count(*) from tbl_periodset, tbl_tgeompointinst where ps <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'periodset', 'tgeompointinst', count(*) from tbl_periodset, tbl_tgeompointinst where ps ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'periodset', 'tgeompointi', count(*) from tbl_periodset, tbl_tgeompointi where ps && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'periodset', 'tgeompointi', count(*) from tbl_periodset, tbl_tgeompointi where ps @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'periodset', 'tgeompointi', count(*) from tbl_periodset, tbl_tgeompointi where ps <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'periodset', 'tgeompointi', count(*) from tbl_periodset, tbl_tgeompointi where ps ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'periodset', 'tgeompointseq', count(*) from tbl_periodset, tbl_tgeompointseq where ps && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'periodset', 'tgeompointseq', count(*) from tbl_periodset, tbl_tgeompointseq where ps @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'periodset', 'tgeompointseq', count(*) from tbl_periodset, tbl_tgeompointseq where ps <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'periodset', 'tgeompointseq', count(*) from tbl_periodset, tbl_tgeompointseq where ps ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'periodset', 'tgeompoints', count(*) from tbl_periodset, tbl_tgeompoints where ps && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'periodset', 'tgeompoints', count(*) from tbl_periodset, tbl_tgeompoints where ps @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'periodset', 'tgeompoints', count(*) from tbl_periodset, tbl_tgeompoints where ps <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'periodset', 'tgeompoints', count(*) from tbl_periodset, tbl_tgeompoints where ps ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'gbox', 'tgeompointinst', count(*) from tbl_gbox, tbl_tgeompointinst where b && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'gbox', 'tgeompointinst', count(*) from tbl_gbox, tbl_tgeompointinst where b @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'gbox', 'tgeompointinst', count(*) from tbl_gbox, tbl_tgeompointinst where b <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'gbox', 'tgeompointinst', count(*) from tbl_gbox, tbl_tgeompointinst where b ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'gbox', 'tgeompointi', count(*) from tbl_gbox, tbl_tgeompointi where b && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'gbox', 'tgeompointi', count(*) from tbl_gbox, tbl_tgeompointi where b @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'gbox', 'tgeompointi', count(*) from tbl_gbox, tbl_tgeompointi where b <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'gbox', 'tgeompointi', count(*) from tbl_gbox, tbl_tgeompointi where b ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'gbox', 'tgeompointseq', count(*) from tbl_gbox, tbl_tgeompointseq where b && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'gbox', 'tgeompointseq', count(*) from tbl_gbox, tbl_tgeompointseq where b @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'gbox', 'tgeompointseq', count(*) from tbl_gbox, tbl_tgeompointseq where b <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'gbox', 'tgeompointseq', count(*) from tbl_gbox, tbl_tgeompointseq where b ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'gbox', 'tgeompoints', count(*) from tbl_gbox, tbl_tgeompoints where b && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'gbox', 'tgeompoints', count(*) from tbl_gbox, tbl_tgeompoints where b @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'gbox', 'tgeompoints', count(*) from tbl_gbox, tbl_tgeompoints where b <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'gbox', 'tgeompoints', count(*) from tbl_gbox, tbl_tgeompoints where b ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'geompoint', count(*) from tbl_tgeompointinst, tbl_geompoint where inst && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'geompoint', count(*) from tbl_tgeompointinst, tbl_geompoint where inst @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'geompoint', count(*) from tbl_tgeompointinst, tbl_geompoint where inst <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'geompoint', count(*) from tbl_tgeompointinst, tbl_geompoint where inst ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'geompolygon', count(*) from tbl_tgeompointinst, tbl_geompolygon where inst && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'geompolygon', count(*) from tbl_tgeompointinst, tbl_geompolygon where inst @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'geompolygon', count(*) from tbl_tgeompointinst, tbl_geompolygon where inst <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'geompolygon', count(*) from tbl_tgeompointinst, tbl_geompolygon where inst ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'timestamptz', count(*) from tbl_tgeompointinst, tbl_timestamptz where inst && t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'timestamptz', count(*) from tbl_tgeompointinst, tbl_timestamptz where inst @> t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'timestamptz', count(*) from tbl_tgeompointinst, tbl_timestamptz where inst <@ t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'timestamptz', count(*) from tbl_tgeompointinst, tbl_timestamptz where inst ~= t;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'timestampset', count(*) from tbl_tgeompointinst, tbl_timestampset where inst && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'timestampset', count(*) from tbl_tgeompointinst, tbl_timestampset where inst @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'timestampset', count(*) from tbl_tgeompointinst, tbl_timestampset where inst <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'timestampset', count(*) from tbl_tgeompointinst, tbl_timestampset where inst ~= ts;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'period', count(*) from tbl_tgeompointinst, tbl_period where inst && p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'period', count(*) from tbl_tgeompointinst, tbl_period where inst @> p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'period', count(*) from tbl_tgeompointinst, tbl_period where inst <@ p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'period', count(*) from tbl_tgeompointinst, tbl_period where inst ~= p;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'periodset', count(*) from tbl_tgeompointinst, tbl_periodset where inst && ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'periodset', count(*) from tbl_tgeompointinst, tbl_periodset where inst @> ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'periodset', count(*) from tbl_tgeompointinst, tbl_periodset where inst <@ ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'periodset', count(*) from tbl_tgeompointinst, tbl_periodset where inst ~= ps;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'gbox', count(*) from tbl_tgeompointinst, tbl_gbox where inst && b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'gbox', count(*) from tbl_tgeompointinst, tbl_gbox where inst @> b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'gbox', count(*) from tbl_tgeompointinst, tbl_gbox where inst <@ b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'gbox', count(*) from tbl_tgeompointinst, tbl_gbox where inst ~= b;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'tgeompointinst', count(*) from tbl_tgeompointinst i1, tbl_tgeompointinst i2 where i1.inst && i2.inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'tgeompointinst', count(*) from tbl_tgeompointinst i1, tbl_tgeompointinst i2 where i1.inst @> i2.inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'tgeompointinst', count(*) from tbl_tgeompointinst i1, tbl_tgeompointinst i2 where i1.inst <@ i2.inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'tgeompointinst', count(*) from tbl_tgeompointinst i1, tbl_tgeompointinst i2 where i1.inst ~= i2.inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'tgeompointi', count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'tgeompointi', count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'tgeompointi', count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'tgeompointi', count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'tgeompointseq', count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'tgeompointseq', count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'tgeompointseq', count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'tgeompointseq', count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointinst', 'tgeompoints', count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointinst', 'tgeompoints', count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointinst', 'tgeompoints', count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointinst', 'tgeompoints', count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'geompoint', count(*) from tbl_tgeompointi, tbl_geompoint where ti && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'geompoint', count(*) from tbl_tgeompointi, tbl_geompoint where ti @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'geompoint', count(*) from tbl_tgeompointi, tbl_geompoint where ti <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'geompoint', count(*) from tbl_tgeompointi, tbl_geompoint where ti ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'geompolygon', count(*) from tbl_tgeompointi, tbl_geompolygon where ti && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'geompolygon', count(*) from tbl_tgeompointi, tbl_geompolygon where ti @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'geompolygon', count(*) from tbl_tgeompointi, tbl_geompolygon where ti <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'geompolygon', count(*) from tbl_tgeompointi, tbl_geompolygon where ti ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'timestamptz', count(*) from tbl_tgeompointi, tbl_timestamptz where ti && t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'timestamptz', count(*) from tbl_tgeompointi, tbl_timestamptz where ti @> t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'timestamptz', count(*) from tbl_tgeompointi, tbl_timestamptz where ti <@ t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'timestamptz', count(*) from tbl_tgeompointi, tbl_timestamptz where ti ~= t;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'timestampset', count(*) from tbl_tgeompointi, tbl_timestampset where ti && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'timestampset', count(*) from tbl_tgeompointi, tbl_timestampset where ti @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'timestampset', count(*) from tbl_tgeompointi, tbl_timestampset where ti <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'timestampset', count(*) from tbl_tgeompointi, tbl_timestampset where ti ~= ts;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'period', count(*) from tbl_tgeompointi, tbl_period where ti && p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'period', count(*) from tbl_tgeompointi, tbl_period where ti @> p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'period', count(*) from tbl_tgeompointi, tbl_period where ti <@ p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'period', count(*) from tbl_tgeompointi, tbl_period where ti ~= p;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'periodset', count(*) from tbl_tgeompointi, tbl_periodset where ti && ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'periodset', count(*) from tbl_tgeompointi, tbl_periodset where ti @> ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'periodset', count(*) from tbl_tgeompointi, tbl_periodset where ti <@ ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'periodset', count(*) from tbl_tgeompointi, tbl_periodset where ti ~= ps;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'gbox', count(*) from tbl_tgeompointi, tbl_gbox where ti && b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'gbox', count(*) from tbl_tgeompointi, tbl_gbox where ti @> b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'gbox', count(*) from tbl_tgeompointi, tbl_gbox where ti <@ b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'gbox', count(*) from tbl_tgeompointi, tbl_gbox where ti ~= b;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'tgeompointinst', count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'tgeompointinst', count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'tgeompointinst', count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'tgeompointinst', count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'tgeompointi', count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti && t2.ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'tgeompointi', count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti @> t2.ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'tgeompointi', count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti <@ t2.ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'tgeompointi', count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti ~= t2.ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'tgeompointseq', count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'tgeompointseq', count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'tgeompointseq', count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'tgeompointseq', count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointi', 'tgeompoints', count(*) from tbl_tgeompointi, tbl_tgeompoints where ti && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointi', 'tgeompoints', count(*) from tbl_tgeompointi, tbl_tgeompoints where ti @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointi', 'tgeompoints', count(*) from tbl_tgeompointi, tbl_tgeompoints where ti <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointi', 'tgeompoints', count(*) from tbl_tgeompointi, tbl_tgeompoints where ti ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'geompoint', count(*) from tbl_tgeompointseq, tbl_geompoint where seq && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'geompoint', count(*) from tbl_tgeompointseq, tbl_geompoint where seq @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'geompoint', count(*) from tbl_tgeompointseq, tbl_geompoint where seq <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'geompoint', count(*) from tbl_tgeompointseq, tbl_geompoint where seq ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'geompolygon', count(*) from tbl_tgeompointseq, tbl_geompolygon where seq && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'geompolygon', count(*) from tbl_tgeompointseq, tbl_geompolygon where seq @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'geompolygon', count(*) from tbl_tgeompointseq, tbl_geompolygon where seq <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'geompolygon', count(*) from tbl_tgeompointseq, tbl_geompolygon where seq ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'timestamptz', count(*) from tbl_tgeompointseq, tbl_timestamptz where seq && t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'timestamptz', count(*) from tbl_tgeompointseq, tbl_timestamptz where seq @> t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'timestamptz', count(*) from tbl_tgeompointseq, tbl_timestamptz where seq <@ t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'timestamptz', count(*) from tbl_tgeompointseq, tbl_timestamptz where seq ~= t;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'timestampset', count(*) from tbl_tgeompointseq, tbl_timestampset where seq && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'timestampset', count(*) from tbl_tgeompointseq, tbl_timestampset where seq @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'timestampset', count(*) from tbl_tgeompointseq, tbl_timestampset where seq <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'timestampset', count(*) from tbl_tgeompointseq, tbl_timestampset where seq ~= ts;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'period', count(*) from tbl_tgeompointseq, tbl_period where seq && p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'period', count(*) from tbl_tgeompointseq, tbl_period where seq @> p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'period', count(*) from tbl_tgeompointseq, tbl_period where seq <@ p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'period', count(*) from tbl_tgeompointseq, tbl_period where seq ~= p;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'periodset', count(*) from tbl_tgeompointseq, tbl_periodset where seq && ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'periodset', count(*) from tbl_tgeompointseq, tbl_periodset where seq @> ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'periodset', count(*) from tbl_tgeompointseq, tbl_periodset where seq <@ ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'periodset', count(*) from tbl_tgeompointseq, tbl_periodset where seq ~= ps;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'gbox', count(*) from tbl_tgeompointseq, tbl_gbox where seq && b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'gbox', count(*) from tbl_tgeompointseq, tbl_gbox where seq @> b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'gbox', count(*) from tbl_tgeompointseq, tbl_gbox where seq <@ b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'gbox', count(*) from tbl_tgeompointseq, tbl_gbox where seq ~= b;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'tgeompointinst', count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'tgeompointinst', count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'tgeompointinst', count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'tgeompointinst', count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'tgeompointi', count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'tgeompointi', count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'tgeompointi', count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'tgeompointi', count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'tgeompointseq', count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq && s2.seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'tgeompointseq', count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq @> s2.seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'tgeompointseq', count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq <@ s2.seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'tgeompointseq', count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq ~= s2.seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompointseq', 'tgeompoints', count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq && ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompointseq', 'tgeompoints', count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq @> ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompointseq', 'tgeompoints', count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq <@ ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompointseq', 'tgeompoints', count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq ~= ts;

/*****************************************************************************/

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'geompoint', count(*) from tbl_tgeompoints, tbl_geompoint where ts && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'geompoint', count(*) from tbl_tgeompoints, tbl_geompoint where ts @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'geompoint', count(*) from tbl_tgeompoints, tbl_geompoint where ts <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'geompoint', count(*) from tbl_tgeompoints, tbl_geompoint where ts ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'geompolygon', count(*) from tbl_tgeompoints, tbl_geompolygon where ts && g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'geompolygon', count(*) from tbl_tgeompoints, tbl_geompolygon where ts @> g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'geompolygon', count(*) from tbl_tgeompoints, tbl_geompolygon where ts <@ g;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'geompolygon', count(*) from tbl_tgeompoints, tbl_geompolygon where ts ~= g;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'timestamptz', count(*) from tbl_tgeompoints, tbl_timestamptz where ts && t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'timestamptz', count(*) from tbl_tgeompoints, tbl_timestamptz where ts @> t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'timestamptz', count(*) from tbl_tgeompoints, tbl_timestamptz where ts <@ t;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'timestamptz', count(*) from tbl_tgeompoints, tbl_timestamptz where ts ~= t;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'timestampset', count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts && t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'timestampset', count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts @> t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'timestampset', count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts <@ t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'timestampset', count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts ~= t2.ts;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'period', count(*) from tbl_tgeompoints, tbl_period where ts && p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'period', count(*) from tbl_tgeompoints, tbl_period where ts @> p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'period', count(*) from tbl_tgeompoints, tbl_period where ts <@ p;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'period', count(*) from tbl_tgeompoints, tbl_period where ts ~= p;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'periodset', count(*) from tbl_tgeompoints, tbl_periodset where ts && ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'periodset', count(*) from tbl_tgeompoints, tbl_periodset where ts @> ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'periodset', count(*) from tbl_tgeompoints, tbl_periodset where ts <@ ps;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'periodset', count(*) from tbl_tgeompoints, tbl_periodset where ts ~= ps;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'gbox', count(*) from tbl_tgeompoints, tbl_gbox where ts && b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'gbox', count(*) from tbl_tgeompoints, tbl_gbox where ts @> b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'gbox', count(*) from tbl_tgeompoints, tbl_gbox where ts <@ b;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'gbox', count(*) from tbl_tgeompoints, tbl_gbox where ts ~= b;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'tgeompointinst', count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts && inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'tgeompointinst', count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts @> inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'tgeompointinst', count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts <@ inst;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'tgeompointinst', count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts ~= inst;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'tgeompointi', count(*) from tbl_tgeompoints, tbl_tgeompointi where ts && ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'tgeompointi', count(*) from tbl_tgeompoints, tbl_tgeompointi where ts @> ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'tgeompointi', count(*) from tbl_tgeompoints, tbl_tgeompointi where ts <@ ti;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'tgeompointi', count(*) from tbl_tgeompoints, tbl_tgeompointi where ts ~= ti;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'tgeompointseq', count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts && seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'tgeompointseq', count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts @> seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'tgeompointseq', count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts <@ seq;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'tgeompointseq', count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts ~= seq;

insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '&&', 'tgeompoints', 'tgeompoints', count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts && t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '@>', 'tgeompoints', 'tgeompoints', count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts @> t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '<@', 'tgeompoints', 'tgeompoints', count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts <@ t2.ts;
insert into test_geoboundboxops(op, leftarg, rightarg, noidx)
select '~=', 'tgeompoints', 'tgeompoints', count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts ~= t2.ts;

/*****************************************************************************/
