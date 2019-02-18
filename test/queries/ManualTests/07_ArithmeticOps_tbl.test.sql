/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

select count(*) from tbl_tintinst where 1 + inst is not null;
select count(*) from tbl_tfloatinst where 1 + inst is not null;

select count(*) from tbl_tinti where 1 + ti is not null;
select count(*) from tbl_tfloati where 1 + ti is not null;

select count(*) from tbl_tintseq where 1 + seq is not null;
select count(*) from tbl_tfloatseq where 1 + seq is not null;

select count(*) from tbl_tints where 1 + ts is not null;
select count(*) from tbl_tfloats where 1 + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where 1.0 + inst is not null;
select count(*) from tbl_tfloatinst where 1.0 + inst is not null;

select count(*) from tbl_tinti where 1.0 + ti is not null;
select count(*) from tbl_tfloati where 1.0 + ti is not null;

select count(*) from tbl_tintseq where 1.0 + seq is not null;
select count(*) from tbl_tfloatseq where 1.0 + seq is not null;

select count(*) from tbl_tints where 1.0 + ts is not null;
select count(*) from tbl_tfloats where 1.0 + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where inst + 1 is not null;
select count(*) from tbl_tintinst where inst + 1.0 is not null;

select count(*) from tbl_tintinst i1, tbl_tintinst i2 where i1.inst + i2.inst is not null;
select count(*) from tbl_tintinst i1, tbl_tfloatinst i2 where i1.inst + i2.inst is not null;

select count(*) from tbl_tintinst, tbl_tinti where inst + ti is not null;
select count(*) from tbl_tintinst, tbl_tfloati where inst + ti is not null;

select count(*) from tbl_tintinst, tbl_tintseq where inst + seq is not null;
select count(*) from tbl_tintinst, tbl_tfloatseq where inst + seq is not null;

select count(*) from tbl_tintinst, tbl_tints where inst + ts is not null;
select count(*) from tbl_tintinst, tbl_tfloats where inst + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatinst where inst + 1 is not null;
select count(*) from tbl_tfloatinst where inst + 1.0 is not null;

select count(*) from tbl_tfloatinst i1, tbl_tintinst i2 where i1.inst + i2.inst is not null;
select count(*) from tbl_tfloatinst i1, tbl_tfloatinst i2 where i1.inst + i2.inst is not null;

select count(*) from tbl_tfloatinst, tbl_tinti where inst + ti is not null;
select count(*) from tbl_tfloatinst, tbl_tfloati where inst + ti is not null;

select count(*) from tbl_tfloatinst, tbl_tintseq where inst + seq is not null;
select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst + seq is not null;

select count(*) from tbl_tfloatinst, tbl_tints where inst + ts is not null;
select count(*) from tbl_tfloatinst, tbl_tfloats where inst + ts is not null;


/*****************************************************************************/

select count(*) from tbl_tinti where ti + 1 is not null;
select count(*) from tbl_tinti where ti + 1.0 is not null;

select count(*) from tbl_tinti, tbl_tintinst where ti + inst is not null;
select count(*) from tbl_tinti, tbl_tfloatinst where ti + inst is not null;

select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti + t2.ti is not null;
select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti + t2.ti is not null;

select count(*) from tbl_tinti, tbl_tintseq where ti + seq is not null;
select count(*) from tbl_tinti, tbl_tfloatseq where ti + seq is not null;

select count(*) from tbl_tinti, tbl_tints where ti + ts is not null;
select count(*) from tbl_tinti, tbl_tfloats where ti + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloati where ti + 1 is not null;
select count(*) from tbl_tfloati where ti + 1.0 is not null;

select count(*) from tbl_tfloati, tbl_tintinst where ti + inst is not null;
select count(*) from tbl_tfloati, tbl_tfloatinst where ti + inst is not null;

select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti + t2.ti is not null;
select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti + t2.ti is not null;

select count(*) from tbl_tfloati, tbl_tintseq where ti + seq is not null;
select count(*) from tbl_tfloati, tbl_tfloatseq where ti + seq is not null;

select count(*) from tbl_tfloati, tbl_tints where ti + ts is not null;
select count(*) from tbl_tfloati, tbl_tfloats where ti + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintseq where seq + 1 is not null;
select count(*) from tbl_tintseq where seq + 1.0 is not null;

select count(*) from tbl_tintseq, tbl_tintinst where seq + inst is not null;
select count(*) from tbl_tintseq, tbl_tfloatinst where seq + inst is not null;

select count(*) from tbl_tintseq, tbl_tinti where seq + ti is not null;
select count(*) from tbl_tintseq, tbl_tfloati where seq + ti is not null;

select count(*) from tbl_tintseq s1, tbl_tintseq s2 where s1.seq + s2.seq is not null;
select count(*) from tbl_tintseq s1, tbl_tfloatseq s2 where s1.seq + s2.seq is not null;

select count(*) from tbl_tintseq, tbl_tints where seq + ts is not null;
select count(*) from tbl_tintseq, tbl_tfloats where seq + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatseq where seq + 1 is not null;
select count(*) from tbl_tfloatseq where seq + 1.0 is not null;

select count(*) from tbl_tfloatseq, tbl_tintinst where seq + inst is not null;
select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq + inst is not null;

select count(*) from tbl_tfloatseq, tbl_tinti where seq + ti is not null;
select count(*) from tbl_tfloatseq, tbl_tfloati where seq + ti is not null;

select count(*) from tbl_tfloatseq s1, tbl_tintseq s2 where s1.seq + s2.seq is not null;
select count(*) from tbl_tfloatseq s1, tbl_tfloatseq s2 where s1.seq + s2.seq is not null;

select count(*) from tbl_tfloatseq, tbl_tints where seq + ts is not null;
select count(*) from tbl_tfloatseq, tbl_tfloats where seq + ts is not null;

/*****************************************************************************/

select count(*) from tbl_tints where ts + 1 is not null;
select count(*) from tbl_tints where ts + 1.0 is not null;

select count(*) from tbl_tints, tbl_tintinst where ts + inst is not null;
select count(*) from tbl_tints, tbl_tfloatinst where ts + inst is not null;

select count(*) from tbl_tints, tbl_tinti where ts + ti is not null;
select count(*) from tbl_tints, tbl_tfloati where ts + ti is not null;

select count(*) from tbl_tints, tbl_tintseq where ts + seq is not null;
select count(*) from tbl_tints, tbl_tfloatseq where ts + seq is not null;

select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts + t2.ts is not null;
select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts + t2.ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloats where ts + 1 is not null;
select count(*) from tbl_tfloats where ts + 1.0 is not null;

select count(*) from tbl_tfloats, tbl_tintinst where ts + inst is not null;
select count(*) from tbl_tfloats, tbl_tfloatinst where ts + inst is not null;

select count(*) from tbl_tfloats, tbl_tinti where ts + ti is not null;
select count(*) from tbl_tfloats, tbl_tfloati where ts + ti is not null;

select count(*) from tbl_tfloats, tbl_tintseq where ts + seq is not null;
select count(*) from tbl_tfloats, tbl_tfloatseq where ts + seq is not null;

select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts + t2.ts is not null;
select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts + t2.ts is not null;

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

select count(*) from tbl_tintinst where 1 - inst is not null;
select count(*) from tbl_tfloatinst where 1 - inst is not null;

select count(*) from tbl_tinti where 1 - ti is not null;
select count(*) from tbl_tfloati where 1 - ti is not null;

select count(*) from tbl_tintseq where 1 - seq is not null;
select count(*) from tbl_tfloatseq where 1 - seq is not null;

select count(*) from tbl_tints where 1 - ts is not null;
select count(*) from tbl_tfloats where 1 - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where 1.0 - inst is not null;
select count(*) from tbl_tfloatinst where 1.0 - inst is not null;

select count(*) from tbl_tinti where 1.0 - ti is not null;
select count(*) from tbl_tfloati where 1.0 - ti is not null;

select count(*) from tbl_tintseq where 1.0 - seq is not null;
select count(*) from tbl_tfloatseq where 1.0 - seq is not null;

select count(*) from tbl_tints where 1.0 - ts is not null;
select count(*) from tbl_tfloats where 1.0 - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where inst - 1 is not null;
select count(*) from tbl_tintinst where inst - 1.0 is not null;

select count(*) from tbl_tintinst i1, tbl_tintinst i2 where i1.inst - i2.inst is not null;
select count(*) from tbl_tintinst i1, tbl_tfloatinst i2 where i1.inst - i2.inst is not null;

select count(*) from tbl_tintinst, tbl_tinti where inst - ti is not null;
select count(*) from tbl_tintinst, tbl_tfloati where inst - ti is not null;

select count(*) from tbl_tintinst, tbl_tintseq where inst - seq is not null;
select count(*) from tbl_tintinst, tbl_tfloatseq where inst - seq is not null;

select count(*) from tbl_tintinst, tbl_tints where inst - ts is not null;
select count(*) from tbl_tintinst, tbl_tfloats where inst - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatinst where inst - 1 is not null;
select count(*) from tbl_tfloatinst where inst - 1.0 is not null;

select count(*) from tbl_tfloatinst i1, tbl_tintinst i2 where i1.inst - i2.inst is not null;
select count(*) from tbl_tfloatinst i1, tbl_tfloatinst i2 where i1.inst - i2.inst is not null;

select count(*) from tbl_tfloatinst, tbl_tinti where inst - ti is not null;
select count(*) from tbl_tfloatinst, tbl_tfloati where inst - ti is not null;

select count(*) from tbl_tfloatinst, tbl_tintseq where inst - seq is not null;
select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst - seq is not null;

select count(*) from tbl_tfloatinst, tbl_tints where inst - ts is not null;
select count(*) from tbl_tfloatinst, tbl_tfloats where inst - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tinti where ti - 1 is not null;
select count(*) from tbl_tinti where ti - 1.0 is not null;

select count(*) from tbl_tinti, tbl_tintinst where ti - inst is not null;
select count(*) from tbl_tinti, tbl_tfloatinst where ti - inst is not null;

select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti - t2.ti is not null;
select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti - t2.ti is not null;

select count(*) from tbl_tinti, tbl_tintseq where ti - seq is not null;
select count(*) from tbl_tinti, tbl_tfloatseq where ti - seq is not null;

select count(*) from tbl_tinti, tbl_tints where ti - ts is not null;
select count(*) from tbl_tinti, tbl_tfloats where ti - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloati where ti - 1 is not null;
select count(*) from tbl_tfloati where ti - 1.0 is not null;

select count(*) from tbl_tfloati, tbl_tintinst where ti - inst is not null;
select count(*) from tbl_tfloati, tbl_tfloatinst where ti - inst is not null;

select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti - t2.ti is not null;
select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti - t2.ti is not null;

select count(*) from tbl_tfloati, tbl_tintseq where ti - seq is not null;
select count(*) from tbl_tfloati, tbl_tfloatseq where ti - seq is not null;

select count(*) from tbl_tfloati, tbl_tints where ti - ts is not null;
select count(*) from tbl_tfloati, tbl_tfloats where ti - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintseq where seq - 1 is not null;
select count(*) from tbl_tintseq where seq - 1.0 is not null;

select count(*) from tbl_tintseq, tbl_tintinst where seq - inst is not null;
select count(*) from tbl_tintseq, tbl_tfloatinst where seq - inst is not null;

select count(*) from tbl_tintseq, tbl_tinti where seq - ti is not null;
select count(*) from tbl_tintseq, tbl_tfloati where seq - ti is not null;

select count(*) from tbl_tintseq s1, tbl_tintseq s2 where s1.seq - s2.seq is not null;
select count(*) from tbl_tintseq s1, tbl_tfloatseq s2 where s1.seq - s2.seq is not null;

select count(*) from tbl_tintseq, tbl_tints where seq - ts is not null;
select count(*) from tbl_tintseq, tbl_tfloats where seq - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatseq where seq - 1 is not null;
select count(*) from tbl_tfloatseq where seq - 1.0 is not null;

select count(*) from tbl_tfloatseq, tbl_tintinst where seq - inst is not null;
select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq - inst is not null;

select count(*) from tbl_tfloatseq, tbl_tinti where seq - ti is not null;
select count(*) from tbl_tfloatseq, tbl_tfloati where seq - ti is not null;

select count(*) from tbl_tfloatseq s1, tbl_tintseq s2 where s1.seq - s2.seq is not null;
select count(*) from tbl_tfloatseq s1, tbl_tfloatseq s2 where s1.seq - s2.seq is not null;

select count(*) from tbl_tfloatseq, tbl_tints where seq - ts is not null;
select count(*) from tbl_tfloatseq, tbl_tfloats where seq - ts is not null;

/*****************************************************************************/

select count(*) from tbl_tints where ts - 1 is not null;
select count(*) from tbl_tints where ts - 1.0 is not null;

select count(*) from tbl_tints, tbl_tintinst where ts - inst is not null;
select count(*) from tbl_tints, tbl_tfloatinst where ts - inst is not null;

select count(*) from tbl_tints, tbl_tinti where ts - ti is not null;
select count(*) from tbl_tints, tbl_tfloati where ts - ti is not null;

select count(*) from tbl_tints, tbl_tintseq where ts - seq is not null;
select count(*) from tbl_tints, tbl_tfloatseq where ts - seq is not null;

select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts - t2.ts is not null;
select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts - t2.ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloats where ts - 1 is not null;
select count(*) from tbl_tfloats where ts - 1.0 is not null;

select count(*) from tbl_tfloats, tbl_tintinst where ts - inst is not null;
select count(*) from tbl_tfloats, tbl_tfloatinst where ts - inst is not null;

select count(*) from tbl_tfloats, tbl_tinti where ts - ti is not null;
select count(*) from tbl_tfloats, tbl_tfloati where ts - ti is not null;

select count(*) from tbl_tfloats, tbl_tintseq where ts - seq is not null;
select count(*) from tbl_tfloats, tbl_tfloatseq where ts - seq is not null;

select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts - t2.ts is not null;
select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts - t2.ts is not null;

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

select count(*) from tbl_tintinst where 1 * inst is not null;
select count(*) from tbl_tfloatinst where 1 * inst is not null;

select count(*) from tbl_tinti where 1 * ti is not null;
select count(*) from tbl_tfloati where 1 * ti is not null;

select count(*) from tbl_tintseq where 1 * seq is not null;
select count(*) from tbl_tfloatseq where 1 * seq is not null;

select count(*) from tbl_tints where 1 * ts is not null;
select count(*) from tbl_tfloats where 1 * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where 1.0 * inst is not null;
select count(*) from tbl_tfloatinst where 1.0 * inst is not null;

select count(*) from tbl_tinti where 1.0 * ti is not null;
select count(*) from tbl_tfloati where 1.0 * ti is not null;

select count(*) from tbl_tintseq where 1.0 * seq is not null;
select count(*) from tbl_tfloatseq where 1.0 * seq is not null;

select count(*) from tbl_tints where 1.0 * ts is not null;
select count(*) from tbl_tfloats where 1.0 * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where inst * 1 is not null;
select count(*) from tbl_tintinst where inst * 1.0 is not null;

select count(*) from tbl_tintinst i1, tbl_tintinst i2 where i1.inst * i2.inst is not null;
select count(*) from tbl_tintinst i1, tbl_tfloatinst i2 where i1.inst * i2.inst is not null;

select count(*) from tbl_tintinst, tbl_tinti where inst * ti is not null;
select count(*) from tbl_tintinst, tbl_tfloati where inst * ti is not null;

select count(*) from tbl_tintinst, tbl_tintseq where inst * seq is not null;
select count(*) from tbl_tintinst, tbl_tfloatseq where inst * seq is not null;

select count(*) from tbl_tintinst, tbl_tints where inst * ts is not null;
select count(*) from tbl_tintinst, tbl_tfloats where inst * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatinst where inst * 1 is not null;
select count(*) from tbl_tfloatinst where inst * 1.0 is not null;

select count(*) from tbl_tfloatinst i1, tbl_tintinst i2 where i1.inst * i2.inst is not null;
select count(*) from tbl_tfloatinst i1, tbl_tfloatinst i2 where i1.inst * i2.inst is not null;

select count(*) from tbl_tfloatinst, tbl_tinti where inst * ti is not null;
select count(*) from tbl_tfloatinst, tbl_tfloati where inst * ti is not null;

select count(*) from tbl_tfloatinst, tbl_tintseq where inst * seq is not null;
select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst * seq is not null;

select count(*) from tbl_tfloatinst, tbl_tints where inst * ts is not null;
select count(*) from tbl_tfloatinst, tbl_tfloats where inst * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tinti where ti * 1 is not null;
select count(*) from tbl_tinti where ti * 1.0 is not null;

select count(*) from tbl_tinti, tbl_tintinst where ti * inst is not null;
select count(*) from tbl_tinti, tbl_tfloatinst where ti * inst is not null;

select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti * t2.ti is not null;
select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti * t2.ti is not null;

select count(*) from tbl_tinti, tbl_tintseq where ti * seq is not null;
select count(*) from tbl_tinti, tbl_tfloatseq where ti * seq is not null;

select count(*) from tbl_tinti, tbl_tints where ti * ts is not null;
select count(*) from tbl_tinti, tbl_tfloats where ti * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloati where ti * 1 is not null;
select count(*) from tbl_tfloati where ti * 1.0 is not null;

select count(*) from tbl_tfloati, tbl_tintinst where ti * inst is not null;
select count(*) from tbl_tfloati, tbl_tfloatinst where ti * inst is not null;

select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti * t2.ti is not null;
select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti * t2.ti is not null;

select count(*) from tbl_tfloati, tbl_tintseq where ti * seq is not null;
select count(*) from tbl_tfloati, tbl_tfloatseq where ti * seq is not null;

select count(*) from tbl_tfloati, tbl_tints where ti * ts is not null;
select count(*) from tbl_tfloati, tbl_tfloats where ti * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintseq where seq * 1 is not null;
select count(*) from tbl_tintseq where seq * 1.0 is not null;

select count(*) from tbl_tintseq, tbl_tintinst where seq * inst is not null;
select count(*) from tbl_tintseq, tbl_tfloatinst where seq * inst is not null;

select count(*) from tbl_tintseq, tbl_tinti where seq * ti is not null;
select count(*) from tbl_tintseq, tbl_tfloati where seq * ti is not null;

select count(*) from tbl_tintseq s1, tbl_tintseq s2 where s1.seq * s2.seq is not null;
select count(*) from tbl_tintseq s1, tbl_tfloatseq s2 where s1.seq * s2.seq is not null;

select count(*) from tbl_tintseq, tbl_tints where seq * ts is not null;
select count(*) from tbl_tintseq, tbl_tfloats where seq * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatseq where seq * 1 is not null;
select count(*) from tbl_tfloatseq where seq * 1.0 is not null;

select count(*) from tbl_tfloatseq, tbl_tintinst where seq * inst is not null;
select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq * inst is not null;

select count(*) from tbl_tfloatseq, tbl_tinti where seq * ti is not null;
select count(*) from tbl_tfloatseq, tbl_tfloati where seq * ti is not null;

select count(*) from tbl_tfloatseq s1, tbl_tintseq s2 where s1.seq * s2.seq is not null;
select count(*) from tbl_tfloatseq s1, tbl_tfloatseq s2 where s1.seq * s2.seq is not null;

select count(*) from tbl_tfloatseq, tbl_tints where seq * ts is not null;
select count(*) from tbl_tfloatseq, tbl_tfloats where seq * ts is not null;

/*****************************************************************************/

select count(*) from tbl_tints where ts * 1 is not null;
select count(*) from tbl_tints where ts * 1.0 is not null;

select count(*) from tbl_tints, tbl_tintinst where ts * inst is not null;
select count(*) from tbl_tints, tbl_tfloatinst where ts * inst is not null;

select count(*) from tbl_tints, tbl_tinti where ts * ti is not null;
select count(*) from tbl_tints, tbl_tfloati where ts * ti is not null;

select count(*) from tbl_tints, tbl_tintseq where ts * seq is not null;
select count(*) from tbl_tints, tbl_tfloatseq where ts * seq is not null;

select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts * t2.ts is not null;
select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts * t2.ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloats where ts * 1 is not null;
select count(*) from tbl_tfloats where ts * 1.0 is not null;

select count(*) from tbl_tfloats, tbl_tintinst where ts * inst is not null;
select count(*) from tbl_tfloats, tbl_tfloatinst where ts * inst is not null;

select count(*) from tbl_tfloats, tbl_tinti where ts * ti is not null;
select count(*) from tbl_tfloats, tbl_tfloati where ts * ti is not null;

select count(*) from tbl_tfloats, tbl_tintseq where ts * seq is not null;
select count(*) from tbl_tfloats, tbl_tfloatseq where ts * seq is not null;

select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts * t2.ts is not null;
select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts * t2.ts is not null;

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

select count(*) from tbl_tintinst where 1 / inst is not null;
select count(*) from tbl_tfloatinst where 1 / inst is not null;

select count(*) from tbl_tinti where 1 / ti is not null;
select count(*) from tbl_tfloati where 1 / ti is not null;

select count(*) from tbl_tintseq where 1 / seq is not null;
select count(*) from tbl_tfloatseq where 1 / seq is not null;

select count(*) from tbl_tints where 1 / ts is not null;
select count(*) from tbl_tfloats where 1 / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where 1.0 / inst is not null;
select count(*) from tbl_tfloatinst where 1.0 / inst is not null;

select count(*) from tbl_tinti where 1.0 / ti is not null;
select count(*) from tbl_tfloati where 1.0 / ti is not null;

select count(*) from tbl_tintseq where 1.0 / seq is not null;
select count(*) from tbl_tfloatseq where 1.0 / seq is not null;

select count(*) from tbl_tints where 1.0 / ts is not null;
select count(*) from tbl_tfloats where 1.0 / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintinst where inst / 1 is not null;
select count(*) from tbl_tintinst where inst / 1.0 is not null;

select count(*) from tbl_tintinst i1, tbl_tintinst i2 where i1.inst / i2.inst is not null;
select count(*) from tbl_tintinst i1, tbl_tfloatinst i2 where i1.inst / i2.inst is not null;

select count(*) from tbl_tintinst, tbl_tinti where inst / ti is not null;
select count(*) from tbl_tintinst, tbl_tfloati where inst / ti is not null;

select count(*) from tbl_tintinst, tbl_tintseq where inst / seq is not null;
select count(*) from tbl_tintinst, tbl_tfloatseq where inst / seq is not null;

select count(*) from tbl_tintinst, tbl_tints where inst / ts is not null;
select count(*) from tbl_tintinst, tbl_tfloats where inst / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatinst where inst / 1 is not null;
select count(*) from tbl_tfloatinst where inst / 1.0 is not null;

select count(*) from tbl_tfloatinst i1, tbl_tintinst i2 where i1.inst / i2.inst is not null;
select count(*) from tbl_tfloatinst i1, tbl_tfloatinst i2 where i1.inst / i2.inst is not null;

select count(*) from tbl_tfloatinst, tbl_tinti where inst / ti is not null;
select count(*) from tbl_tfloatinst, tbl_tfloati where inst / ti is not null;

select count(*) from tbl_tfloatinst, tbl_tintseq where inst / seq is not null;
select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst / seq is not null;

select count(*) from tbl_tfloatinst, tbl_tints where inst / ts is not null;
select count(*) from tbl_tfloatinst, tbl_tfloats where inst / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tinti where ti / 1 is not null;
select count(*) from tbl_tinti where ti / 1.0 is not null;

select count(*) from tbl_tinti, tbl_tintinst where ti / inst is not null;
select count(*) from tbl_tinti, tbl_tfloatinst where ti / inst is not null;

select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti / t2.ti is not null;
select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti / t2.ti is not null;

select count(*) from tbl_tinti, tbl_tintseq where ti / seq is not null;
select count(*) from tbl_tinti, tbl_tfloatseq where ti / seq is not null;

select count(*) from tbl_tinti, tbl_tints where ti / ts is not null;
select count(*) from tbl_tinti, tbl_tfloats where ti / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloati where ti / 1 is not null;
select count(*) from tbl_tfloati where ti / 1.0 is not null;

select count(*) from tbl_tfloati, tbl_tintinst where ti / inst is not null;
select count(*) from tbl_tfloati, tbl_tfloatinst where ti / inst is not null;

select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti / t2.ti is not null;
select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti / t2.ti is not null;

select count(*) from tbl_tfloati, tbl_tintseq where ti / seq is not null;
select count(*) from tbl_tfloati, tbl_tfloatseq where ti / seq is not null;

select count(*) from tbl_tfloati, tbl_tints where ti / ts is not null;
select count(*) from tbl_tfloati, tbl_tfloats where ti / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tintseq where seq / 1 is not null;
select count(*) from tbl_tintseq where seq / 1.0 is not null;

select count(*) from tbl_tintseq, tbl_tintinst where seq / inst is not null;
select count(*) from tbl_tintseq, tbl_tfloatinst where seq / inst is not null;

select count(*) from tbl_tintseq, tbl_tinti where seq / ti is not null;
select count(*) from tbl_tintseq, tbl_tfloati where seq / ti is not null;

select count(*) from tbl_tintseq s1, tbl_tintseq s2 where s1.seq / s2.seq is not null;
select count(*) from tbl_tintseq s1, tbl_tfloatseq s2 where s1.seq / s2.seq is not null;

select count(*) from tbl_tintseq, tbl_tints where seq / ts is not null;
select count(*) from tbl_tintseq, tbl_tfloats where seq / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloatseq where seq / 1 is not null;
select count(*) from tbl_tfloatseq where seq / 1.0 is not null;

select count(*) from tbl_tfloatseq, tbl_tintinst where seq / inst is not null;
select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq / inst is not null;

select count(*) from tbl_tfloatseq, tbl_tinti where seq / ti is not null;
select count(*) from tbl_tfloatseq, tbl_tfloati where seq / ti is not null;

select count(*) from tbl_tfloatseq s1, tbl_tintseq s2 where s1.seq / s2.seq is not null;
select count(*) from tbl_tfloatseq s1, tbl_tfloatseq s2 where s1.seq / s2.seq is not null;

select count(*) from tbl_tfloatseq, tbl_tints where seq / ts is not null;
select count(*) from tbl_tfloatseq, tbl_tfloats where seq / ts is not null;

/*****************************************************************************/

select count(*) from tbl_tints where ts / 1 is not null;
select count(*) from tbl_tints where ts / 1.0 is not null;

select count(*) from tbl_tints, tbl_tintinst where ts / inst is not null;
select count(*) from tbl_tints, tbl_tfloatinst where ts / inst is not null;

select count(*) from tbl_tints, tbl_tinti where ts / ti is not null;
select count(*) from tbl_tints, tbl_tfloati where ts / ti is not null;

select count(*) from tbl_tints, tbl_tintseq where ts / seq is not null;
select count(*) from tbl_tints, tbl_tfloatseq where ts / seq is not null;

select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts / t2.ts is not null;
select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts / t2.ts is not null;

/*****************************************************************************/

select count(*) from tbl_tfloats where ts / 1 is not null;
select count(*) from tbl_tfloats where ts / 1.0 is not null;

select count(*) from tbl_tfloats, tbl_tintinst where ts / inst is not null;
select count(*) from tbl_tfloats, tbl_tfloatinst where ts / inst is not null;

select count(*) from tbl_tfloats, tbl_tinti where ts / ti is not null;
select count(*) from tbl_tfloats, tbl_tfloati where ts / ti is not null;

select count(*) from tbl_tfloats, tbl_tintseq where ts / seq is not null;
select count(*) from tbl_tfloats, tbl_tfloatseq where ts / seq is not null;

select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts / t2.ts is not null;
select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts / t2.ts is not null;

/*****************************************************************************/
