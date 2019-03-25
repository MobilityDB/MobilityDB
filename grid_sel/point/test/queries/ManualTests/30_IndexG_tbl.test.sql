/******************************************************************************/
/* tgeogpointinst */

--before
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst <<# t;
select count(*) from tbl_tgeogpointinst, tbl_period where inst <<# p;
select count(*) from tbl_tgeogpointinst, tbl_gbox where inst <<# b;
select count(*) from tbl_tgeogpointinst t1, tbl_tgeogpointinst t2 where t1.inst <<# t2.inst;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst <<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst <<# ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst <<# ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst <<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst <<# ts;

--overbefore
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst &<# t;
select count(*) from tbl_tgeogpointinst, tbl_period where inst &<# p;
select count(*) from tbl_tgeogpointinst, tbl_gbox where inst &<# b;
select count(*) from tbl_tgeogpointinst t1, tbl_tgeogpointinst t2 where t1.inst &<# t2.inst;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst &<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst &<# ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst &<# ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst &<# seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst &<# ts;

--after
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst #>> t;
select count(*) from tbl_tgeogpointinst, tbl_period where inst #>> p;
select count(*) from tbl_tgeogpointinst, tbl_gbox where inst #>> b;
select count(*) from tbl_tgeogpointinst t1, tbl_tgeogpointinst t2 where t1.inst #>> t2.inst;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst #>> seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst #>> ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst #>> ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst #>> seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst #>> ts;

--overafter
select count(*) from tbl_tgeogpointinst, tbl_timestamptz where inst #&> t;
select count(*) from tbl_tgeogpointinst, tbl_period where inst #&> p;
select count(*) from tbl_tgeogpointinst, tbl_gbox where inst #&> b;
select count(*) from tbl_tgeogpointinst t1, tbl_tgeogpointinst t2 where t1.inst #&> t2.inst;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointper where inst #&> seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointp where inst #&> ts;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointi where inst #&> ti;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpointseq where inst #&> seq;
select count(*) from tbl_tgeogpointinst, tbl_tgeogpoints where inst #&> ts;

/******************************************************************************/
/* tgeogpointper */

--before
select count(*) from tbl_tgeogpointper, tbl_timestamptz where seq <<# t;
select count(*) from tbl_tgeogpointper, tbl_period where seq <<# p;
select count(*) from tbl_tgeogpointper, tbl_gbox where seq <<# b;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where seq <<# inst;
select count(*) from tbl_tgeogpointper t1, tbl_tgeogpointper t2 where t1.seq <<# t2.seq;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where seq <<# ts;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where seq <<# ti;

--overbefore
select count(*) from tbl_tgeogpointper, tbl_timestamptz where seq &<# t;
select count(*) from tbl_tgeogpointper, tbl_period where seq &<# p;
select count(*) from tbl_tgeogpointper, tbl_gbox where seq &<# b;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where seq &<# inst;
select count(*) from tbl_tgeogpointper t1, tbl_tgeogpointper t2 where t1.seq &<# t2.seq;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where seq &<# ts;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where seq &<# ti;

--after
select count(*) from tbl_tgeogpointper, tbl_timestamptz where seq #>> t;
select count(*) from tbl_tgeogpointper, tbl_period where seq #>> p;
select count(*) from tbl_tgeogpointper, tbl_gbox where seq #>> b;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where seq #>> inst;
select count(*) from tbl_tgeogpointper t1, tbl_tgeogpointper t2 where t1.seq #>> t2.seq;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where seq #>> ts;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where seq #>> ti;

--overafter
select count(*) from tbl_tgeogpointper, tbl_timestamptz where seq #&> t;
select count(*) from tbl_tgeogpointper, tbl_period where seq #&> p;
select count(*) from tbl_tgeogpointper, tbl_gbox where seq #&> b;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointinst where seq #&> inst;
select count(*) from tbl_tgeogpointper t1, tbl_tgeogpointper t2 where t1.seq #&> t2.seq;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointp where seq #&> ts;
select count(*) from tbl_tgeogpointper, tbl_tgeogpointi where seq #&> ti;

/******************************************************************************/
/* tgeogpointp */

--before
select count(*) from tbl_tgeogpointp, tbl_timestamptz where ts <<# t;
select count(*) from tbl_tgeogpointp, tbl_period where ts <<# p;
select count(*) from tbl_tgeogpointp, tbl_gbox where ts <<# b;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where ts <<# inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where ts <<# seq;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.ts <<# t2.ts;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where ts <<# ti;

--overbefore
select count(*) from tbl_tgeogpointp, tbl_timestamptz where ts &<# t;
select count(*) from tbl_tgeogpointp, tbl_period where ts &<# p;
select count(*) from tbl_tgeogpointp, tbl_gbox where ts &<# b;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where ts &<# inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where ts &<# seq;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.ts &<# t2.ts;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where ts &<# ti;

--after
select count(*) from tbl_tgeogpointp, tbl_timestamptz where ts #>> t;
select count(*) from tbl_tgeogpointp, tbl_period where ts #>> p;
select count(*) from tbl_tgeogpointp, tbl_gbox where ts #>> b;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where ts #>> inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where ts #>> seq;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.ts #>> t2.ts;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where ts #>> ti;

--overafter
select count(*) from tbl_tgeogpointp, tbl_timestamptz where ts #&> t;
select count(*) from tbl_tgeogpointp, tbl_period where ts #&> p;
select count(*) from tbl_tgeogpointp, tbl_gbox where ts #&> b;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointinst where ts #&> inst;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointper where ts #&> seq;
select count(*) from tbl_tgeogpointp t1, tbl_tgeogpointp t2 where t1.ts #&> t2.ts;
select count(*) from tbl_tgeogpointp, tbl_tgeogpointi where ts #&> ti;

/******************************************************************************/
/* tgeogpointi */

--before
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti <<# t;
select count(*) from tbl_tgeogpointi, tbl_period where ti <<# p;
select count(*) from tbl_tgeogpointi, tbl_gbox where ti <<# b;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti <<# inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti <<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti <<# ts;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti <<# t2.ti;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti <<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti <<# ts;

--overbefore
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti &<# t;
select count(*) from tbl_tgeogpointi, tbl_period where ti &<# p;
select count(*) from tbl_tgeogpointi, tbl_gbox where ti &<# b;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti &<# inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti &<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti &<# ts;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti &<# t2.ti;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti &<# seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti &<# ts;

--after
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti #>> t;
select count(*) from tbl_tgeogpointi, tbl_period where ti #>> p;
select count(*) from tbl_tgeogpointi, tbl_gbox where ti #>> b;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti #>> inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti #>> seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti #>> ts;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti #>> t2.ti;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti #>> seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti #>> ts;

--overafter
select count(*) from tbl_tgeogpointi, tbl_timestamptz where ti #&> t;
select count(*) from tbl_tgeogpointi, tbl_period where ti #&> p;
select count(*) from tbl_tgeogpointi, tbl_gbox where ti #&> b;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointinst where ti #&> inst;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointper where ti #&> seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointp where ti #&> ts;
select count(*) from tbl_tgeogpointi t1, tbl_tgeogpointi t2 where t1.ti #&> t2.ti;
select count(*) from tbl_tgeogpointi, tbl_tgeogpointseq where ti #&> seq;
select count(*) from tbl_tgeogpointi, tbl_tgeogpoints where ti #&> ts;

/******************************************************************************/
/* tgeogpointseq */

--before
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq <<# t;
select count(*) from tbl_tgeogpointseq, tbl_period where seq <<# p;
select count(*) from tbl_tgeogpointseq, tbl_gbox where seq <<# b;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq <<# inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq <<# ti;
select count(*) from tbl_tgeogpointseq t1, tbl_tgeogpointseq t2 where t1.seq <<# t2.seq;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq <<# ts;

--overbefore
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq &<# t;
select count(*) from tbl_tgeogpointseq, tbl_period where seq &<# p;
select count(*) from tbl_tgeogpointseq, tbl_gbox where seq &<# b;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq &<# inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq &<# ti;
select count(*) from tbl_tgeogpointseq t1, tbl_tgeogpointseq t2 where t1.seq &<# t2.seq;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq &<# ts;

--after
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq #>> t;
select count(*) from tbl_tgeogpointseq, tbl_period where seq #>> p;
select count(*) from tbl_tgeogpointseq, tbl_gbox where seq #>> b;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq #>> inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq #>> ti;
select count(*) from tbl_tgeogpointseq t1, tbl_tgeogpointseq t2 where t1.seq #>> t2.seq;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq #>> ts;

--overafter
select count(*) from tbl_tgeogpointseq, tbl_timestamptz where seq #&> t;
select count(*) from tbl_tgeogpointseq, tbl_period where seq #&> p;
select count(*) from tbl_tgeogpointseq, tbl_gbox where seq #&> b;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointinst where seq #&> inst;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpointi where seq #&> ti;
select count(*) from tbl_tgeogpointseq t1, tbl_tgeogpointseq t2 where t1.seq #&> t2.seq;
select count(*) from tbl_tgeogpointseq, tbl_tgeogpoints where seq #&> ts;

/******************************************************************************/
/* tgeogpoints */

--before
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts <<# t;
select count(*) from tbl_tgeogpoints, tbl_period where ts <<# p;
select count(*) from tbl_tgeogpoints, tbl_gbox where ts <<# b;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts <<# inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts <<# ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts <<# seq;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts <<# t2.ts;

--overbefore
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts &<# t;
select count(*) from tbl_tgeogpoints, tbl_period where ts &<# p;
select count(*) from tbl_tgeogpoints, tbl_gbox where ts &<# b;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts &<# inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts &<# ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts &<# seq;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts &<# t2.ts;

--after
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts #>> t;
select count(*) from tbl_tgeogpoints, tbl_period where ts #>> p;
select count(*) from tbl_tgeogpoints, tbl_gbox where ts #>> b;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts #>> inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts #>> ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts #>> seq;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts #>> t2.ts;

--overafter
select count(*) from tbl_tgeogpoints, tbl_timestamptz where ts #&> t;
select count(*) from tbl_tgeogpoints, tbl_period where ts #&> p;
select count(*) from tbl_tgeogpoints, tbl_gbox where ts #&> b;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointinst where ts #&> inst;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointi where ts #&> ti;
select count(*) from tbl_tgeogpoints, tbl_tgeogpointseq where ts #&> seq;
select count(*) from tbl_tgeogpoints t1, tbl_tgeogpoints t2 where t1.ts #&> t2.ts;

/******************************************************************************/
