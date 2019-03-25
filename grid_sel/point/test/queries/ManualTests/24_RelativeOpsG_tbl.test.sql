-- CREATE FUNCTION testRelativeOpsG() RETURNS void AS $$
-- BEGIN

/*****************************************************************************/

select count(*) from tbl_timestamptz, tbl_tgeogpointinst where t <<# inst;
select count(*) from tbl_timestamptz, tbl_tgeogpointinst where t #>> inst;
select count(*) from tbl_timestamptz, tbl_tgeogpointinst where t &<# inst;
select count(*) from tbl_timestamptz, tbl_tgeogpointinst where t #&> inst;

select count(*) from tbl_timestamptz, tbl_tgeogpointper where t <<# per;
select count(*) from tbl_timestamptz, tbl_tgeogpointper where t #>> per;
select count(*) from tbl_timestamptz, tbl_tgeogpointper where t &<# per;
select count(*) from tbl_timestamptz, tbl_tgeogpointper where t #&> per;

select count(*) from tbl_timestamptz, tbl_tgeogpointp where t <<# tp;
select count(*) from tbl_timestamptz, tbl_tgeogpointp where t #>> tp;
select count(*) from tbl_timestamptz, tbl_tgeogpointp where t &<# tp;
select count(*) from tbl_timestamptz, tbl_tgeogpointp where t #&> tp;

select count(*) from tbl_timestamptz, tbl_tgeogpointi where t <<# ti;
select count(*) from tbl_timestamptz, tbl_tgeogpointi where t #>> ti;
select count(*) from tbl_timestamptz, tbl_tgeogpointi where t &<# ti;
select count(*) from tbl_timestamptz, tbl_tgeogpointi where t #&> ti;

select count(*) from tbl_timestamptz, tbl_tgeogpointseq where t <<# seq;
select count(*) from tbl_timestamptz, tbl_tgeogpointseq where t #>> seq;
select count(*) from tbl_timestamptz, tbl_tgeogpointseq where t &<# seq;
select count(*) from tbl_timestamptz, tbl_tgeogpointseq where t #&> seq;

select count(*) from tbl_timestamptz, tbl_tgeogpoints where t <<# ts;
select count(*) from tbl_timestamptz, tbl_tgeogpoints where t #>> ts;
select count(*) from tbl_timestamptz, tbl_tgeogpoints where t &<# ts;
select count(*) from tbl_timestamptz, tbl_tgeogpoints where t #&> ts;

/*****************************************************************************/

select count(*) from tbl_period, tbl_tgeogpointinst where p <<# inst;
select count(*) from tbl_period, tbl_tgeogpointinst where p #>> inst;
select count(*) from tbl_period, tbl_tgeogpointinst where p &<# inst;
select count(*) from tbl_period, tbl_tgeogpointinst where p #&> inst;

select count(*) from tbl_period, tbl_tgeogpointper where p <<# per;
select count(*) from tbl_period, tbl_tgeogpointper where p #>> per;
select count(*) from tbl_period, tbl_tgeogpointper where p &<# per;
select count(*) from tbl_period, tbl_tgeogpointper where p #&> per;

select count(*) from tbl_period, tbl_tgeogpointp where p <<# tp;
select count(*) from tbl_period, tbl_tgeogpointp where p #>> tp;
select count(*) from tbl_period, tbl_tgeogpointp where p &<# tp;
select count(*) from tbl_period, tbl_tgeogpointp where p #&> tp;

select count(*) from tbl_period, tbl_tgeogpointi where p <<# ti;
select count(*) from tbl_period, tbl_tgeogpointi where p #>> ti;
select count(*) from tbl_period, tbl_tgeogpointi where p &<# ti;
select count(*) from tbl_period, tbl_tgeogpointi where p #&> ti;

select count(*) from tbl_period, tbl_tgeogpointseq where p <<# seq;
select count(*) from tbl_period, tbl_tgeogpointseq where p #>> seq;
select count(*) from tbl_period, tbl_tgeogpointseq where p &<# seq;
select count(*) from tbl_period, tbl_tgeogpointseq where p #&> seq;

select count(*) from tbl_period, tbl_tgeogpoints where p <<# ts;
select count(*) from tbl_period, tbl_tgeogpoints where p #>> ts;
select count(*) from tbl_period, tbl_tgeogpoints where p &<# ts;
select count(*) from tbl_period, tbl_tgeogpoints where p #&> ts;

/*****************************************************************************/

select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst <<# t;
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst #>> t;
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst &<# t;
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst #&> t;

select count(*) from tbl_tgeogpointinst, tbl_period where inst <<# p;
select count(*) from tbl_tgeogpointinst, tbl_period where inst #>> p;
select count(*) from tbl_tgeogpointinst, tbl_period where inst &<# p;
select count(*) from tbl_tgeogpointinst, tbl_period where inst #&> p;

select count(*) from tbl_tgeogpointinst i1, tbl_tgeogpointinst i2 where i1.inst <<# i2.inst;
select count(*) from tbl_tgeogpointinst i1, tbl_tgeogpointinst i2 where i1.inst #>> i2.inst;
select count(*) from tbl_tgeogpointinst i1, tbl_tgeogpointinst i2 where i1.inst &<# i2.inst;
select count(*) from tbl_tgeogpointinst i1, tbl_tgeogpointinst i2 where i1.inst #&> i2.inst;

select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst <<# per;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst #>> per;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst &<# per;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst #&> per;

select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst <<# tp;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst #>> tp;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst &<# tp;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst #&> tp;

select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst <<# ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst #>> ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst &<# ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst #&> ti;

select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst <<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst #>> seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst &<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst #&> seq;

select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst <<# ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst #>> ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst &<# ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst #&> ts;

/*****************************************************************************/

select count(*) from tbl_tgeogpointper, tbl_timestamptz where per <<# t;
select count(*) from tbl_tgeogpointper, tbl_timestamptz where per #>> t;
select count(*) from tbl_tgeogpointper, tbl_timestamptz where per &<# t;
select count(*) from tbl_tgeogpointper, tbl_timestamptz where per #&> t;

select count(*) from tbl_tgeogpointper, tbl_period where per <<# p;
select count(*) from tbl_tgeogpointper, tbl_period where per #>> p;
select count(*) from tbl_tgeogpointper, tbl_period where per &<# p;
select count(*) from tbl_tgeogpointper, tbl_period where per #&> p;

select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where per <<# inst;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where per #>> inst;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where per &<# inst;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where per #&> inst;

select count(*) from tbl_tgeogpointper s1, tbl_tgeogpointper s2 where s1.per <<# s2.per;
select count(*) from tbl_tgeogpointper s1, tbl_tgeogpointper s2 where s1.per #>> s2.per;
select count(*) from tbl_tgeogpointper s1, tbl_tgeogpointper s2 where s1.per &<# s2.per;
select count(*) from tbl_tgeogpointper s1, tbl_tgeogpointper s2 where s1.per #&> s2.per;

select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where per <<# tp;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where per #>> tp;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where per &<# tp;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where per #&> tp;

select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where per <<# ti;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where per #>> ti;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where per &<# ti;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where per #&> ti;

/*****************************************************************************/

select count(*) from tbl_tgeogpointp, tbl_timestamptz where tp <<# t;
select count(*) from tbl_tgeogpointp, tbl_timestamptz where tp #>> t;
select count(*) from tbl_tgeogpointp, tbl_timestamptz where tp &<# t;
select count(*) from tbl_tgeogpointp, tbl_timestamptz where tp #&> t;

select count(*) from tbl_tgeogpointp, tbl_period where tp <<# p;
select count(*) from tbl_tgeogpointp, tbl_period where tp #>> p;
select count(*) from tbl_tgeogpointp, tbl_period where tp &<# p;
select count(*) from tbl_tgeogpointp, tbl_period where tp #&> p;

select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where tp <<# inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where tp #>> inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where tp &<# inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where tp #&> inst;

select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where tp <<# per;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where tp #>> per;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where tp &<# per;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where tp #&> per;

select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.tp <<# t2.tp;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.tp #>> t2.tp;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.tp &<# t2.tp;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.tp #&> t2.tp;

select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where tp <<# ti;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where tp #>> ti;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where tp &<# ti;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where tp #&> ti;

/*****************************************************************************/

select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti <<# t;
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti #>> t;
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti &<# t;
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti #&> t;

select count(*) from tbl_tgeogpointi, tbl_period where ti <<# p;
select count(*) from tbl_tgeogpointi, tbl_period where ti #>> p;
select count(*) from tbl_tgeogpointi, tbl_period where ti &<# p;
select count(*) from tbl_tgeogpointi, tbl_period where ti #&> p;

select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti <<# inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti #>> inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti &<# inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti #&> inst;

select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti <<# per;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti #>> per;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti &<# per;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti #&> per;

select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti <<# tp;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti #>> tp;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti &<# tp;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti #&> tp;

select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti <<# t2.ti;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti #>> t2.ti;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti &<# t2.ti;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti #&> t2.ti;

select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti <<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti #>> seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti &<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti #&> seq;

select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti <<# ts;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti #>> ts;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti &<# ts;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti #&> ts;

/*****************************************************************************/

select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq <<# t;
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq #>> t;
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq &<# t;
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq #&> t;

select count(*) from tbl_tgeogpointseq, tbl_period where seq <<# p;
select count(*) from tbl_tgeogpointseq, tbl_period where seq #>> p;
select count(*) from tbl_tgeogpointseq, tbl_period where seq &<# p;
select count(*) from tbl_tgeogpointseq, tbl_period where seq #&> p;

select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq <<# inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq #>> inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq &<# inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq #&> inst;

select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq <<# ti;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq #>> ti;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq &<# ti;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq #&> ti;

select count(*) from tbl_tgeogpointseq s1, tbl_tgeogpointseq s2 where s1.seq <<# s2.seq;
select count(*) from tbl_tgeogpointseq s1, tbl_tgeogpointseq s2 where s1.seq #>> s2.seq;
select count(*) from tbl_tgeogpointseq s1, tbl_tgeogpointseq s2 where s1.seq &<# s2.seq;
select count(*) from tbl_tgeogpointseq s1, tbl_tgeogpointseq s2 where s1.seq #&> s2.seq;

select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq <<# ts;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq #>> ts;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq &<# ts;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq #&> ts;

/*****************************************************************************/

select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts <<# t;
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts #>> t;
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts &<# t;
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts #&> t;

select count(*) from tbl_tgeogpoints, tbl_period where ts <<# p;
select count(*) from tbl_tgeogpoints, tbl_period where ts #>> p;
select count(*) from tbl_tgeogpoints, tbl_period where ts &<# p;
select count(*) from tbl_tgeogpoints, tbl_period where ts #&> p;

select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts <<# inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts #>> inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts &<# inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts #&> inst;

select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts <<# ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts #>> ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts &<# ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts #&> ti;

select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts <<# seq;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts #>> seq;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts &<# seq;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts #&> seq;

select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts <<# t2.ts;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts #>> t2.ts;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts &<# t2.ts;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts #&> t2.ts;

/*****************************************************************************/
-- END;
-- $$ LANGUAGE 'plpgsql';

-- select count(*)testRelativeOpsGeomM() 