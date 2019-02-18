/*****************************************************************************
 * Temporal npoint to gbox
 *****************************************************************************/

SELECT gbox(inst) FROM tbl_tnpointinst;
SELECT gbox(ti) FROM tbl_tnpointi;
SELECT gbox(seq) FROM tbl_tnpointseq;
SELECT gbox(ts) FROM tbl_tnpoints;

/*****************************************************************************
 * Expand
 *****************************************************************************/

SELECT expandSpatial(inst, 10) FROM tbl_tnpointinst;
SELECT expandTemporal(inst, '1 day'::interval) FROM tbl_tnpointinst;

SELECT expandSpatial(ti, 10) FROM tbl_tnpointi;
SELECT expandTemporal(ti, '1 day'::interval) FROM tbl_tnpointi;

SELECT expandSpatial(seq, 10) FROM tbl_tnpointseq;
SELECT expandTemporal(seq, '1 day'::interval) FROM tbl_tnpointseq;

SELECT expandSpatial(ts, 10) FROM tbl_tnpoints;
SELECT expandTemporal(ts, '1 day'::interval) FROM tbl_tnpoints;

/*****************************************************************************
 * Geometry op Temporal npoint
 *****************************************************************************/

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g && t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g @> t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g <@ t2.inst;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointinst t2 WHERE t1.g ~= t2.inst;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g && t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g @> t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g <@ t2.ti;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointi t2 WHERE t1.g ~= t2.ti;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g && t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g @> t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g <@ t2.seq;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpointseq t2 WHERE t1.g ~= t2.seq;

SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g && t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g @> t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g <@ t2.ts;
SELECT count(*) FROM geompoint_tbl t1, tbl_tnpoints t2 WHERE t1.g ~= t2.ts;

/*****************************************************************************
 * Timestamptz op Temporal npoint
 *****************************************************************************/

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t && t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t @> t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t <@ t2.inst;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointinst t2 WHERE t1.t ~= t2.inst;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t && t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t @> t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t <@ t2.ti;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointi t2 WHERE t1.t ~= t2.ti;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t && t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t @> t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t <@ t2.seq;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpointseq t2 WHERE t1.t ~= t2.seq;

SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t && t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t @> t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t <@ t2.ts;
SELECT count(*) FROM timestamptz_tbl t1, tbl_tnpoints t2 WHERE t1.t ~= t2.ts;

/*****************************************************************************
 * Timestampset op Temporal npoint
 *****************************************************************************/

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts && t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts @> t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts <@ t2.inst;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointinst t2 WHERE t1.ts ~= t2.inst;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts && t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts @> t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts <@ t2.ti;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointi t2 WHERE t1.ts ~= t2.ti;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts && t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts @> t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts <@ t2.seq;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpointseq t2 WHERE t1.ts ~= t2.seq;

SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts && t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts @> t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts <@ t2.ts;
SELECT count(*) FROM timestampset_tbl t1, tbl_tnpoints t2 WHERE t1.ts ~= t2.ts;

/*****************************************************************************
 * Period op Temporal npoint
 *****************************************************************************/

SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p && t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p @> t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p <@ t2.inst;
SELECT count(*) FROM period_tbl t1, tbl_tnpointinst t2 WHERE t1.p ~= t2.inst;

SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p && t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p @> t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p <@ t2.ti;
SELECT count(*) FROM period_tbl t1, tbl_tnpointi t2 WHERE t1.p ~= t2.ti;

SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p && t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p @> t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p <@ t2.seq;
SELECT count(*) FROM period_tbl t1, tbl_tnpointseq t2 WHERE t1.p ~= t2.seq;

SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p && t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p @> t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p <@ t2.ts;
SELECT count(*) FROM period_tbl t1, tbl_tnpoints t2 WHERE t1.p ~= t2.ts;

/*****************************************************************************
 * periodset op Temporal npoint
 *****************************************************************************/

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps && t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps @> t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps <@ t2.inst;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointinst t2 WHERE t1.ps ~= t2.inst;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps && t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps @> t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps <@ t2.ti;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointi t2 WHERE t1.ps ~= t2.ti;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps && t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps @> t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps <@ t2.seq;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpointseq t2 WHERE t1.ps ~= t2.seq;

SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps && t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps @> t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps <@ t2.ts;
SELECT count(*) FROM periodset_tbl t1, tbl_tnpoints t2 WHERE t1.ps ~= t2.ts;

/*****************************************************************************
 * TNpointInst op <type>
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst && t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst @> t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst <@ t2.g;
SELECT count(*) FROM tbl_tnpointinst t1, geompoint_tbl t2 WHERE t1.inst ~= t2.g;

SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst && t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst @> t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst <@ t2.t;
SELECT count(*) FROM tbl_tnpointinst t1, timestamptz_tbl t2 WHERE t1.inst ~= t2.t;

SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst && t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst @> t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst <@ t2.p;
SELECT count(*) FROM tbl_tnpointinst t1, period_tbl t2 WHERE t1.inst ~= t2.p;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst && t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst @> t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst <@ t2.inst;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointinst t2 WHERE t1.inst ~= t2.inst;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst && t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst @> t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst <@ t2.ti;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointi t2 WHERE t1.inst ~= t2.ti;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst && t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst @> t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst <@ t2.seq;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpointseq t2 WHERE t1.inst ~= t2.seq;

SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst && t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst @> t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst <@ t2.ts;
SELECT count(*) FROM tbl_tnpointinst t1, tbl_tnpoints t2 WHERE t1.inst ~= t2.ts;

/*****************************************************************************
 * tNpointI op <type>
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti && t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti @> t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti <@ t2.g;
SELECT count(*) FROM tbl_tnpointi t1, geompoint_tbl t2 WHERE t1.ti ~= t2.g;

SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti && t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti @> t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti <@ t2.t;
SELECT count(*) FROM tbl_tnpointi t1, timestamptz_tbl t2 WHERE t1.ti ~= t2.t;

SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti && t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti @> t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti <@ t2.p;
SELECT count(*) FROM tbl_tnpointi t1, period_tbl t2 WHERE t1.ti ~= t2.p;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti && t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti @> t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti <@ t2.inst;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointinst t2 WHERE t1.ti ~= t2.inst;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti && t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti @> t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti <@ t2.ti;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointi t2 WHERE t1.ti ~= t2.ti;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti && t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti @> t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti <@ t2.seq;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpointseq t2 WHERE t1.ti ~= t2.seq;

SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti && t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti @> t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti <@ t2.ts;
SELECT count(*) FROM tbl_tnpointi t1, tbl_tnpoints t2 WHERE t1.ti ~= t2.ts;

/*****************************************************************************
 * TNpointSeq op <type>
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq && t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq @> t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq <@ t2.g;
SELECT count(*) FROM tbl_tnpointseq t1, geompoint_tbl t2 WHERE t1.seq ~= t2.g;

SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq && t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq @> t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq <@ t2.t;
SELECT count(*) FROM tbl_tnpointseq t1, timestamptz_tbl t2 WHERE t1.seq ~= t2.t;

SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq && t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq @> t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq <@ t2.p;
SELECT count(*) FROM tbl_tnpointseq t1, period_tbl t2 WHERE t1.seq ~= t2.p;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq && t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq @> t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq <@ t2.inst;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointinst t2 WHERE t1.seq ~= t2.inst;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq && t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq @> t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq <@ t2.ti;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointi t2 WHERE t1.seq ~= t2.ti;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq && t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq @> t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq <@ t2.seq;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpointseq t2 WHERE t1.seq ~= t2.seq;

SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq && t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq @> t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq <@ t2.ts;
SELECT count(*) FROM tbl_tnpointseq t1, tbl_tnpoints t2 WHERE t1.seq ~= t2.ts;

/*****************************************************************************
 * TNpointS op <type>
 *****************************************************************************/

SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts && t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts @> t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts <@ t2.g;
SELECT count(*) FROM tbl_tnpoints t1, geompoint_tbl t2 WHERE t1.ts ~= t2.g;

SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts && t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts @> t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts <@ t2.t;
SELECT count(*) FROM tbl_tnpoints t1, timestamptz_tbl t2 WHERE t1.ts ~= t2.t;

SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts && t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts @> t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts <@ t2.p;
SELECT count(*) FROM tbl_tnpoints t1, period_tbl t2 WHERE t1.ts ~= t2.p;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts && t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts @> t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts <@ t2.inst;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointinst t2 WHERE t1.ts ~= t2.inst;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts && t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts @> t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts <@ t2.ti;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointi t2 WHERE t1.ts ~= t2.ti;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts && t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts @> t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts <@ t2.seq;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpointseq t2 WHERE t1.ts ~= t2.seq;

SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts && t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts @> t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts <@ t2.ts;
SELECT count(*) FROM tbl_tnpoints t1, tbl_tnpoints t2 WHERE t1.ts ~= t2.ts;

/*****************************************************************************/