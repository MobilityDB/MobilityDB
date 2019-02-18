/******************************************************************************/

/* tgeompointinst */
--left
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst << p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst << p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst << b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst << t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst << ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst << seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst << ts;

--overleft
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst &< p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst &< p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst &< b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst &< t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst &< ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst &< seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst &< ts;

--right
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst >> p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst >> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst >> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst >> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst >> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst >> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst >> ts;

--overright
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst &> p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst &> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst &> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst &> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst &> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst &> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst &> ts;

--below
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst <<| p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst <<| p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst <<| b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst <<| t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst <<| ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst <<| seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst <<| ts;

--overbelow
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst &<| p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst &<| p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst &<| b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst &<| t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst &<| ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst &<| seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst &<| ts;

--above
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst |>> p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst |>> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst |>> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst |>> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst |>> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst |>> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst |>> ts;

--overabove
select count(*) from tbl_tgeompointinst, tbl_geompoint where inst |&> p;
select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst |&> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst |&> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst |&> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst |&> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst |&> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst |&> ts;

--before
select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst <<# t;
select count(*) from tbl_tgeompointinst, tbl_period where inst <<# p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst <<# b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst <<# t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst <<# ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst <<# seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst <<# ts;

--overbefore
select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst &<# t;
select count(*) from tbl_tgeompointinst, tbl_period where inst &<# p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst &<# b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst &<# t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst &<# ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst &<# seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst &<# ts;

--after
select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst #>> t;
select count(*) from tbl_tgeompointinst, tbl_period where inst #>> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst #>> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst #>> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst #>> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst #>> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst #>> ts;

--overafter
select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst #&> t;
select count(*) from tbl_tgeompointinst, tbl_period where inst #&> p;
select count(*) from tbl_tgeompointinst, tbl_gbox where inst #&> b;
select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst #&> t2.inst;
select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst #&> ti;
select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst #&> seq;
select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst #&> ts;

/******************************************************************************/
/* tgeompointi */

--left
select count(*) from tbl_tgeompointi, tbl_geompoint where ti << p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti << p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti << b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti << inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti << per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti << tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti << t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti << seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti << ts;

--overleft
select count(*) from tbl_tgeompointi, tbl_geompoint where ti &< p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti &< p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti &< b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti &< inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti &< per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti &< tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti &< t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti &< seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti &< ts;

--right
select count(*) from tbl_tgeompointi, tbl_geompoint where ti >> p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti >> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti >> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti >> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti >> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti >> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti >> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti >> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti >> ts;

--overright
select count(*) from tbl_tgeompointi, tbl_geompoint where ti &> p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti &> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti &> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti &> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti &> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti &> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti &> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti &> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti &> ts;

--below
select count(*) from tbl_tgeompointi, tbl_geompoint where ti <<| p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti <<| p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti <<| b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti <<| inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti <<| per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti <<| tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti <<| t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti <<| seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti <<| ts;

--overbelow
select count(*) from tbl_tgeompointi, tbl_geompoint where ti &<| p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti &<| p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti &<| b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti &<| inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti &<| per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti &<| tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti &<| t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti &<| seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti &<| ts;

--above
select count(*) from tbl_tgeompointi, tbl_geompoint where ti |>> p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti |>> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti |>> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti |>> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti |>> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti |>> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti |>> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti |>> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti |>> ts;

--overabove
select count(*) from tbl_tgeompointi, tbl_geompoint where ti |&> p;
select count(*) from tbl_tgeompointi, tbl_geompolygon where ti |&> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti |&> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti |&> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti |&> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti |&> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti |&> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti |&> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti |&> ts;

--before
select count(*) from tbl_tgeompointi, tbl_timestamptz where ti <<# t;
select count(*) from tbl_tgeompointi, tbl_period where ti <<# p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti <<# b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti <<# inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti <<# per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti <<# tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti <<# t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti <<# seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti <<# ts;

--overbefore
select count(*) from tbl_tgeompointi, tbl_timestamptz where ti &<# t;
select count(*) from tbl_tgeompointi, tbl_period where ti &<# p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti &<# b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti &<# inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti &<# per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti &<# tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti &<# t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti &<# seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti &<# ts;

--after
select count(*) from tbl_tgeompointi, tbl_timestamptz where ti #>> t;
select count(*) from tbl_tgeompointi, tbl_period where ti #>> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti #>> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti #>> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti #>> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti #>> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti #>> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti #>> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti #>> ts;

--overafter
select count(*) from tbl_tgeompointi, tbl_timestamptz where ti #&> t;
select count(*) from tbl_tgeompointi, tbl_period where ti #&> p;
select count(*) from tbl_tgeompointi, tbl_gbox where ti #&> b;
select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti #&> inst;
select count(*) from tbl_tgeompointi, tbl_tgeompointper where ti #&> per;
select count(*) from tbl_tgeompointi, tbl_tgeompointp where ti #&> tp;
select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti #&> t2.ti;
select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti #&> seq;
select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti #&> ts;

/******************************************************************************/
/* tgeompointseq */

--left
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq << p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq << p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq << b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq << inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq << ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq << t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq << ts;

--overleft
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq &< p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq &< p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq &< b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq &< inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq &< ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq &< t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq &< ts;

--right
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq >> p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq >> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq >> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq >> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq >> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq >> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq >> ts;

--overright
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq &> p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq &> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq &> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq &> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq &> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq &> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq &> ts;

--below
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq <<| p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq <<| p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq <<| b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq <<| inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq <<| ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq <<| t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq <<| ts;

--overbelow
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq &<| p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq &<| p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq &<| b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq &<| inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq &<| ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq &<| t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq &<| ts;

--above
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq |>> p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq |>> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq |>> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq |>> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq |>> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq |>> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq |>> ts;

--overabove
select count(*) from tbl_tgeompointseq, tbl_geompoint where seq |&> p;
select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq |&> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq |&> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq |&> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq |&> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq |&> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq |&> ts;

--before
select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq <<# t;
select count(*) from tbl_tgeompointseq, tbl_period where seq <<# p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq <<# b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq <<# inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq <<# ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq <<# t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq <<# ts;

--overbefore
select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq &<# t;
select count(*) from tbl_tgeompointseq, tbl_period where seq &<# p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq &<# b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq &<# inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq &<# ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq &<# t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq &<# ts;

--after
select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq #>> t;
select count(*) from tbl_tgeompointseq, tbl_period where seq #>> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq #>> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq #>> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq #>> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq #>> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq #>> ts;

--overafter
select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq #&> t;
select count(*) from tbl_tgeompointseq, tbl_period where seq #&> p;
select count(*) from tbl_tgeompointseq, tbl_gbox where seq #&> b;
select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq #&> inst;
select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq #&> ti;
select count(*) from tbl_tgeompointseq t1, tbl_tgeompointseq t2 where t1.seq #&> t2.seq;
select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq #&> ts;

/******************************************************************************/
/* tgeompoints */

--left
select count(*) from tbl_tgeompoints, tbl_geompoint where ts << p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts << p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts << b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts << inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts << ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts << seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts << t2.ts;

--overleft
select count(*) from tbl_tgeompoints, tbl_geompoint where ts &< p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts &< p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts &< b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts &< inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts &< ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts &< seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts &< t2.ts;

--right
select count(*) from tbl_tgeompoints, tbl_geompoint where ts >> p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts >> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts >> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts >> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts >> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts >> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts >> t2.ts;

--overright
select count(*) from tbl_tgeompoints, tbl_geompoint where ts &> p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts &> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts &> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts &> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts &> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts &> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts &> t2.ts;

--below
select count(*) from tbl_tgeompoints, tbl_geompoint where ts <<| p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts <<| p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts <<| b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts <<| inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts <<| ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts <<| seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts <<| t2.ts;

--overbelow
select count(*) from tbl_tgeompoints, tbl_geompoint where ts &<| p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts &<| p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts &<| b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts &<| inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts &<| ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts &<| seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts &<| t2.ts;

--above
select count(*) from tbl_tgeompoints, tbl_geompoint where ts |>> p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts |>> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts |>> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts |>> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts |>> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts |>> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts |>> t2.ts;

--overabove
select count(*) from tbl_tgeompoints, tbl_geompoint where ts |&> p;
select count(*) from tbl_tgeompoints, tbl_geompolygon where ts |&> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts |&> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts |&> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts |&> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts |&> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts |&> t2.ts;

--before
select count(*) from tbl_tgeompoints, tbl_timestamptz where ts <<# t;
select count(*) from tbl_tgeompoints, tbl_period where ts <<# p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts <<# b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts <<# inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts <<# ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts <<# seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts <<# t2.ts;

--overbefore
select count(*) from tbl_tgeompoints, tbl_timestamptz where ts &<# t;
select count(*) from tbl_tgeompoints, tbl_period where ts &<# p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts &<# b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts &<# inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts &<# ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts &<# seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts &<# t2.ts;

--after
select count(*) from tbl_tgeompoints, tbl_timestamptz where ts #>> t;
select count(*) from tbl_tgeompoints, tbl_period where ts #>> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts #>> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts #>> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts #>> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts #>> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts #>> t2.ts;

--overafter
select count(*) from tbl_tgeompoints, tbl_timestamptz where ts #&> t;
select count(*) from tbl_tgeompoints, tbl_period where ts #&> p;
select count(*) from tbl_tgeompoints, tbl_gbox where ts #&> b;
select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts #&> inst;
select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts #&> ti;
select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts #&> seq;
select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts #&> t2.ts;

/******************************************************************************/
