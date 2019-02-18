/*****************************************************************************
 * geometry op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g << t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g &< t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g >> t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g &> t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g <<| t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g &<| t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g |>> t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g |&> t2.inst;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g << t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g &< t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g >> t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g &> t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g <<| t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g &<| t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g |>> t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g |&> t2.ti;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g << t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g &< t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g >> t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g &> t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g <<| t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g &<| t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g |>> t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g |&> t2.seq;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g << t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g &< t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g >> t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g &> t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g <<| t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g &<| t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g |>> t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g |&> t2.ts;

/*****************************************************************************
 * timestamptz op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t <<# t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t #>> t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t &<# t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t #&> t2.inst;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t <<# t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t #>> t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t &<# t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t #&> t2.ti;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t <<# t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t #>> t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t &<# t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t #&> t2.seq;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t <<# t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t #>> t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t &<# t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t #&> t2.ts;

/*****************************************************************************
 * timestampset op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts <<# t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts #>> t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts &<# t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts #&> t2.inst;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts <<# t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts #>> t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts &<# t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts #&> t2.ti;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts <<# t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts #>> t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts &<# t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts #&> t2.seq;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts <<# t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts #>> t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts &<# t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts #&> t2.ts;

/*****************************************************************************
 * period op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p <<# t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p #>> t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p &<# t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p #&> t2.inst;

SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p <<# t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p #>> t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p &<# t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p #&> t2.ti;

SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p <<# t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p #>> t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p &<# t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p #&> t2.seq;

SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p <<# t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p #>> t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p &<# t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p #&> t2.ts;

/*****************************************************************************
 * periodset op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps <<# t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps #>> t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps &<# t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps #&> t2.inst;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps <<# t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps #>> t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps &<# t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps #&> t2.ti;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps <<# t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps #>> t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps &<# t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps #&> t2.seq;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps <<# t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps #>> t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps &<# t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps #&> t2.ts;

/*****************************************************************************
 * tnpointinst op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst << t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst >> t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst &< t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst &> t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst <<| t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst |>> t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst &<| t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst |&> t2.g;

SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst <<# t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst #>> t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst &<# t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst #&> t2.t;

SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst <<# t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst #>> t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst &<# t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst #&> t2.p;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst << t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst >> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst &< t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst &> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst <<| t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst |>> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst &<| t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst |&> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst <<# t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst #>> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst &<# t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst #&> t2.inst;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst << t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst >> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst &< t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst &> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst <<| t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst |>> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst &<| t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst |&> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst <<# t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst #>> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst &<# t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst #&> t2.ti;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst << t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst >> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst &< t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst &> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst <<| t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst |>> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst &<| t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst |&> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst <<# t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst #>> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst &<# t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst #&> t2.seq;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst << t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst >> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst &< t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst &> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst <<| t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst |>> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst &<| t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst |&> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst <<# t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst #>> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst &<# t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst #&> t2.ts;

/*****************************************************************************
 * tnpointi op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti << t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti >> t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti &< t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti &> t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti <<| t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti |>> t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti &<| t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti |&> t2.g;

SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti <<# t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti #>> t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti &<# t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti #&> t2.t;

SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti <<# t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti #>> t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti &<# t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti #&> t2.p;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti << t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti >> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti &< t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti &> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti <<| t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti |>> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti &<| t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti |&> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti <<# t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti #>> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti &<# t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti #&> t2.inst;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti << t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti >> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti &< t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti &> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti <<| t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti |>> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti &<| t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti |&> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti <<# t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti #>> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti &<# t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti #&> t2.ti;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti << t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti >> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti &< t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti &> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti <<| t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti |>> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti &<| t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti |&> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti <<# t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti #>> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti &<# t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti #&> t2.seq;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti << t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti >> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti &< t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti &> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti <<| t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti |>> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti &<| t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti |&> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti <<# t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti #>> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti &<# t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti #&> t2.ts;

/*****************************************************************************
 * tnpointseq op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq << t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq >> t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq &< t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq &> t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq <<| t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq |>> t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq &<| t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq |&> t2.g;

SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq <<# t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq #>> t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq &<# t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq #&> t2.t;

SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq <<# t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq #>> t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq &<# t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq #&> t2.p;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq << t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq >> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq &< t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq &> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq <<| t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq |>> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq &<| t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq |&> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq <<# t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq #>> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq &<# t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq #&> t2.inst;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq << t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq >> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq &< t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq &> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq <<| t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq |>> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq &<| t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq |&> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq <<# t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq #>> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq &<# t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq #&> t2.ti;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq << t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq >> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq &< t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq &> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq <<| t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq |>> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq &<| t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq |&> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq <<# t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq #>> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq &<# t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq #&> t2.seq;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq << t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq >> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq &< t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq &> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq <<| t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq |>> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq &<| t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq |&> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq <<# t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq #>> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq &<# t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq #&> t2.ts;

/*****************************************************************************
 * tnpoints op temporal npoint
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts << t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts >> t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts &< t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts &> t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts <<| t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts |>> t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts &<| t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts |&> t2.g;

SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts <<# t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts #>> t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts &<# t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts #&> t2.t;

SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts <<# t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts #>> t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts &<# t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts #&> t2.p;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts << t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts >> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts &< t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts &> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts <<| t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts |>> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts &<| t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts |&> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts <<# t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts #>> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts &<# t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts #&> t2.inst;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts << t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts >> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts &< t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts &> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts <<| t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts |>> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts &<| t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts |&> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts <<# t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts #>> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts &<# t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts #&> t2.ti;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts << t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts >> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts &< t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts &> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts <<| t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts |>> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts &<| t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts |&> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts <<# t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts #>> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts &<# t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts #&> t2.seq;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts << t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts >> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts &< t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts &> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts <<| t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts |>> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts &<| t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts |&> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts <<# t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts #>> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts &<# t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts #&> t2.ts;

/*****************************************************************************/
