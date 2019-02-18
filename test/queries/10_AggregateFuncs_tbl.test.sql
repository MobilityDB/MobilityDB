/*****************************************************************************
 * TemporalInst aggregate functions
 *****************************************************************************/

select numInstants(tand(inst)) from tbl_tboolinst;
select numInstants(tor(inst)) from tbl_tboolinst;
select numInstants(tcount(inst)) from tbl_tboolinst;

select numInstants(tmin(inst)) from tbl_tintinst;
select numInstants(tmax(inst)) from tbl_tintinst;
select numInstants(tcount(inst)) from tbl_tintinst;
select numInstants(tsum(inst)) from tbl_tintinst;
select numInstants(tavg(inst)) from tbl_tintinst;

select numInstants(tmin(inst)) from tbl_tfloatinst;
select numInstants(tmax(inst)) from tbl_tfloatinst;
select numInstants(tcount(inst)) from tbl_tfloatinst;
select numInstants(tsum(inst)) from tbl_tfloatinst;
select numInstants(tavg(inst)) from tbl_tfloatinst; 
 
select numInstants(tmin(inst)) from tbl_ttextinst;
select numInstants(tmax(inst)) from tbl_ttextinst;
select numInstants(tcount(inst)) from tbl_ttextinst;

select k%10, numInstants(tand(inst)) from tbl_tboolinst group by k%10 order by k%10;
select k%10, numInstants(tor(inst)) from tbl_tboolinst group by k%10 order by k%10;
select k%10, numInstants(tcount(inst)) from tbl_tboolinst group by k%10 order by k%10;

select k%10, numInstants(tmin(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tmax(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tcount(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tsum(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tavg(inst)) from tbl_tintinst group by k%10 order by k%10;

select k%10, numInstants(tmin(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tmax(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tcount(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tsum(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tavg(inst)) from tbl_tfloatinst group by k%10 order by k%10; 
 
select k%10, numInstants(tmin(inst)) from tbl_ttextinst group by k%10 order by k%10;
select k%10, numInstants(tmax(inst)) from tbl_ttextinst group by k%10 order by k%10;
select k%10, numInstants(tcount(inst)) from tbl_ttextinst group by k%10 order by k%10;

/*****************************************************************************
 * TemporalI aggregate functions
 *****************************************************************************/

select numInstants(tand(ti)) from tbl_tbooli;
select numInstants(tor(ti)) from tbl_tbooli;
select numInstants(tcount(ti)) from tbl_tbooli;

select numInstants(tmin(ti)) from tbl_tinti;
select numInstants(tmax(ti)) from tbl_tinti;
select numInstants(tcount(ti)) from tbl_tinti;
select numInstants(tsum(ti)) from tbl_tinti;
select numInstants(tavg(ti)) from tbl_tinti;

select numInstants(tmin(ti)) from tbl_tfloati;
select numInstants(tmax(ti)) from tbl_tfloati;
select numInstants(tcount(ti)) from tbl_tfloati;
select numInstants(tsum(ti)) from tbl_tfloati;
select numInstants(tavg(ti)) from tbl_tfloati; 

select numInstants(tmin(ti)) from tbl_ttexti;
select numInstants(tmax(ti)) from tbl_ttexti;
select numInstants(tcount(ti)) from tbl_ttexti;

select k%10, numInstants(tand(ti)) from tbl_tbooli group by k%10 order by k%10;
select k%10, numInstants(tor(ti)) from tbl_tbooli group by k%10 order by k%10;
select k%10, numInstants(tcount(ti)) from tbl_tbooli group by k%10 order by k%10;

select k%10, numInstants(tmin(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tmax(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tcount(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tsum(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tavg(ti)) from tbl_tinti group by k%10 order by k%10;

select k%10, numInstants(tmin(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tmax(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tcount(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tsum(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tavg(ti)) from tbl_tfloati group by k%10 order by k%10; 

select k%10, numInstants(tmin(ti)) from tbl_ttexti group by k%10 order by k%10;
select k%10, numInstants(tmax(ti)) from tbl_ttexti group by k%10 order by k%10;
select k%10, numInstants(tcount(ti)) from tbl_ttexti group by k%10 order by k%10;

/*****************************************************************************
 * TemporalSeq aggregate functions
 *****************************************************************************/

select numSequences(tand(seq)) from tbl_tboolseq;
select numSequences(tor(seq)) from tbl_tboolseq;
select numSequences(tcount(seq)) from tbl_tboolseq;

select numSequences(tmin(seq)) from tbl_tintseq;
select numSequences(tmax(seq)) from tbl_tintseq;
select numSequences(tcount(seq)) from tbl_tintseq;
select numSequences(tsum(seq)) from tbl_tintseq;
select numSequences(tavg(seq)) from tbl_tintseq;

select numSequences(tmin(seq)) from tbl_tfloatseq;
select numSequences(tmax(seq)) from tbl_tfloatseq;
select numSequences(tcount(seq)) from tbl_tfloatseq;
select numSequences(tsum(seq)) from tbl_tfloatseq;
select numSequences(tavg(seq)) from tbl_tfloatseq; 

select numSequences(tmin(seq)) from tbl_ttextseq;
select numSequences(tmax(seq)) from tbl_ttextseq;
select numSequences(tcount(seq)) from tbl_ttextseq;

select k%10, numSequences(tand(seq)) from tbl_tboolseq group by k%10 order by k%10;
select k%10, numSequences(tor(seq)) from tbl_tboolseq group by k%10 order by k%10;
select k%10, numSequences(tcount(seq)) from tbl_tboolseq group by k%10 order by k%10;

select k%10, numSequences(tmin(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tmax(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tcount(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tsum(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tavg(seq)) from tbl_tintseq group by k%10 order by k%10;

select k%10, numSequences(tmin(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tmax(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tcount(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tsum(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tavg(seq)) from tbl_tfloatseq group by k%10 order by k%10;

select k%10, numSequences(tmin(seq)) from tbl_ttextseq group by k%10 order by k%10;
select k%10, numSequences(tmax(seq)) from tbl_ttextseq group by k%10 order by k%10;
select k%10, numSequences(tcount(seq)) from tbl_ttextseq group by k%10 order by k%10;

/*****************************************************************************
 * TemporalS aggregate functions
 *****************************************************************************/

select numSequences(tand(ts)) from tbl_tbools;
select numSequences(tor(ts)) from tbl_tbools;
select numSequences(tcount(ts)) from tbl_tbools;

select numSequences(tmin(ts)) from tbl_tints;
select numSequences(tmax(ts)) from tbl_tints;
select numSequences(tcount(ts)) from tbl_tints;
select numSequences(tsum(ts)) from tbl_tints;
select numSequences(tavg(ts)) from tbl_tints;

select numSequences(tmin(ts)) from tbl_tfloats;
select numSequences(tmax(ts)) from tbl_tfloats;
select numSequences(tcount(ts)) from tbl_tfloats;
select numSequences(tsum(ts)) from tbl_tfloats;
select numSequences(tavg(ts)) from tbl_tfloats; 

select numSequences(tmin(ts)) from tbl_ttexts;
select numSequences(tmax(ts)) from tbl_ttexts;
select numSequences(tcount(ts)) from tbl_ttexts;

select k%10, numSequences(tand(ts)) from tbl_tbools group by k%10 order by k%10;
select k%10, numSequences(tor(ts)) from tbl_tbools group by k%10 order by k%10;
select k%10, numSequences(tcount(ts)) from tbl_tbools group by k%10 order by k%10;

select k%10, numSequences(tmin(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tmax(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tcount(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tsum(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tavg(ts)) from tbl_tints group by k%10 order by k%10;

select k%10, numSequences(tmin(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tmax(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tcount(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tsum(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tavg(ts)) from tbl_tfloats group by k%10 order by k%10; 

select k%10, numSequences(tmin(ts)) from tbl_ttexts group by k%10 order by k%10;
select k%10, numSequences(tmax(ts)) from tbl_ttexts group by k%10 order by k%10;
select k%10, numSequences(tcount(ts)) from tbl_ttexts group by k%10 order by k%10;

/*****************************************************************************/

