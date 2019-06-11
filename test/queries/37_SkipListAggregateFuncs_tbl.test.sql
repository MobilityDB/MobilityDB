-------------------------------------------------------------------------------

set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- TemporalInst aggregate functions
-------------------------------------------------------------------------------

select numInstants(tand2(inst)) from tbl_tboolinst;
select numInstants(tor2(inst)) from tbl_tboolinst;
select numInstants(tcount2(inst)) from tbl_tboolinst;

select numInstants(tmin2(inst)) from tbl_tintinst;
select numInstants(tmax2(inst)) from tbl_tintinst;
select numInstants(tcount2(inst)) from tbl_tintinst;
select numInstants(tsum2(inst)) from tbl_tintinst;
select numInstants(tavg2(inst)) from tbl_tintinst;

select numInstants(tmin2(inst)) from tbl_tfloatinst;
select numInstants(tmax2(inst)) from tbl_tfloatinst;
select numInstants(tcount2(inst)) from tbl_tfloatinst;
select numInstants(tsum2(inst)) from tbl_tfloatinst;
select numInstants(tavg2(inst)) from tbl_tfloatinst; 
 
select numInstants(tmin2(inst)) from tbl_ttextinst;
select numInstants(tmax2(inst)) from tbl_ttextinst;
select numInstants(tcount2(inst)) from tbl_ttextinst;

select k%10, numInstants(tand2(inst)) from tbl_tboolinst group by k%10 order by k%10;
select k%10, numInstants(tor2(inst)) from tbl_tboolinst group by k%10 order by k%10;
select k%10, numInstants(tcount2(inst)) from tbl_tboolinst group by k%10 order by k%10;

select k%10, numInstants(tmin2(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tmax2(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tcount2(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tsum2(inst)) from tbl_tintinst group by k%10 order by k%10;
select k%10, numInstants(tavg2(inst)) from tbl_tintinst group by k%10 order by k%10;

select k%10, numInstants(tmin2(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tmax2(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tcount2(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tsum2(inst)) from tbl_tfloatinst group by k%10 order by k%10;
select k%10, numInstants(tavg2(inst)) from tbl_tfloatinst group by k%10 order by k%10; 
 
select k%10, numInstants(tmin2(inst)) from tbl_ttextinst group by k%10 order by k%10;
select k%10, numInstants(tmax2(inst)) from tbl_ttextinst group by k%10 order by k%10;
select k%10, numInstants(tcount2(inst)) from tbl_ttextinst group by k%10 order by k%10;

-------------------------------------------------------------------------------
-- TemporalI aggregate functions
-------------------------------------------------------------------------------

select numInstants(tand2(ti)) from tbl_tbooli;
select numInstants(tor2(ti)) from tbl_tbooli;
select numInstants(tcount2(ti)) from tbl_tbooli;

select numInstants(tmin2(ti)) from tbl_tinti;
select numInstants(tmax2(ti)) from tbl_tinti;
select numInstants(tcount2(ti)) from tbl_tinti;
select numInstants(tsum2(ti)) from tbl_tinti;
select numInstants(tavg2(ti)) from tbl_tinti;

select numInstants(tmin2(ti)) from tbl_tfloati;
select numInstants(tmax2(ti)) from tbl_tfloati;
select numInstants(tcount2(ti)) from tbl_tfloati;
select numInstants(tsum2(ti)) from tbl_tfloati;
select numInstants(tavg2(ti)) from tbl_tfloati; 

select numInstants(tmin2(ti)) from tbl_ttexti;
select numInstants(tmax2(ti)) from tbl_ttexti;
select numInstants(tcount2(ti)) from tbl_ttexti;

select k%10, numInstants(tand2(ti)) from tbl_tbooli group by k%10 order by k%10;
select k%10, numInstants(tor2(ti)) from tbl_tbooli group by k%10 order by k%10;
select k%10, numInstants(tcount2(ti)) from tbl_tbooli group by k%10 order by k%10;

select k%10, numInstants(tmin2(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tmax2(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tcount2(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tsum2(ti)) from tbl_tinti group by k%10 order by k%10;
select k%10, numInstants(tavg2(ti)) from tbl_tinti group by k%10 order by k%10;

select k%10, numInstants(tmin2(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tmax2(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tcount2(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tsum2(ti)) from tbl_tfloati group by k%10 order by k%10;
select k%10, numInstants(tavg2(ti)) from tbl_tfloati group by k%10 order by k%10; 

select k%10, numInstants(tmin2(ti)) from tbl_ttexti group by k%10 order by k%10;
select k%10, numInstants(tmax2(ti)) from tbl_ttexti group by k%10 order by k%10;
select k%10, numInstants(tcount2(ti)) from tbl_ttexti group by k%10 order by k%10;

-------------------------------------------------------------------------------
-- TemporalSeq aggregate functions
-------------------------------------------------------------------------------

select numSequences(tand2(seq)) from tbl_tboolseq;
select numSequences(tor2(seq)) from tbl_tboolseq;
select numSequences(tcount2(seq)) from tbl_tboolseq;

select numSequences(tmin2(seq)) from tbl_tintseq;
select numSequences(tmax2(seq)) from tbl_tintseq;
select numSequences(tcount2(seq)) from tbl_tintseq;
select numSequences(tsum2(seq)) from tbl_tintseq;
select numSequences(tavg2(seq)) from tbl_tintseq;

select numSequences(tmin2(seq)) from tbl_tfloatseq;
select numSequences(tmax2(seq)) from tbl_tfloatseq;
select numSequences(tcount2(seq)) from tbl_tfloatseq;
select numSequences(tsum2(seq)) from tbl_tfloatseq;
select numSequences(tavg2(seq)) from tbl_tfloatseq; 

select numSequences(tmin2(seq)) from tbl_ttextseq;
select numSequences(tmax2(seq)) from tbl_ttextseq;
select numSequences(tcount2(seq)) from tbl_ttextseq;

select k%10, numSequences(tand2(seq)) from tbl_tboolseq group by k%10 order by k%10;
select k%10, numSequences(tor2(seq)) from tbl_tboolseq group by k%10 order by k%10;
select k%10, numSequences(tcount2(seq)) from tbl_tboolseq group by k%10 order by k%10;

select k%10, numSequences(tmin2(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tmax2(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tcount2(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tsum2(seq)) from tbl_tintseq group by k%10 order by k%10;
select k%10, numSequences(tavg2(seq)) from tbl_tintseq group by k%10 order by k%10;

select k%10, numSequences(tmin2(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tmax2(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tcount2(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tsum2(seq)) from tbl_tfloatseq group by k%10 order by k%10;
select k%10, numSequences(tavg2(seq)) from tbl_tfloatseq group by k%10 order by k%10;

select k%10, numSequences(tmin2(seq)) from tbl_ttextseq group by k%10 order by k%10;
select k%10, numSequences(tmax2(seq)) from tbl_ttextseq group by k%10 order by k%10;
select k%10, numSequences(tcount2(seq)) from tbl_ttextseq group by k%10 order by k%10;

-------------------------------------------------------------------------------
-- TemporalS aggregate functions
-------------------------------------------------------------------------------

select numSequences(tand2(ts)) from tbl_tbools;
select numSequences(tor2(ts)) from tbl_tbools;
select numSequences(tcount2(ts)) from tbl_tbools;

select numSequences(tmin2(ts)) from tbl_tints;
select numSequences(tmax2(ts)) from tbl_tints;
select numSequences(tcount2(ts)) from tbl_tints;
select numSequences(tsum2(ts)) from tbl_tints;
select numSequences(tavg2(ts)) from tbl_tints;

select numSequences(tmin2(ts)) from tbl_tfloats;
select numSequences(tmax2(ts)) from tbl_tfloats;
select numSequences(tcount2(ts)) from tbl_tfloats;
select numSequences(tsum2(ts)) from tbl_tfloats;
select numSequences(tavg2(ts)) from tbl_tfloats; 

select numSequences(tmin2(ts)) from tbl_ttexts;
select numSequences(tmax2(ts)) from tbl_ttexts;
select numSequences(tcount2(ts)) from tbl_ttexts;

select k%10, numSequences(tand2(ts)) from tbl_tbools group by k%10 order by k%10;
select k%10, numSequences(tor2(ts)) from tbl_tbools group by k%10 order by k%10;
select k%10, numSequences(tcount2(ts)) from tbl_tbools group by k%10 order by k%10;

select k%10, numSequences(tmin2(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tmax2(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tcount2(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tsum2(ts)) from tbl_tints group by k%10 order by k%10;
select k%10, numSequences(tavg2(ts)) from tbl_tints group by k%10 order by k%10;

select k%10, numSequences(tmin2(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tmax2(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tcount2(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tsum2(ts)) from tbl_tfloats group by k%10 order by k%10;
select k%10, numSequences(tavg2(ts)) from tbl_tfloats group by k%10 order by k%10; 

select k%10, numSequences(tmin2(ts)) from tbl_ttexts group by k%10 order by k%10;
select k%10, numSequences(tmax2(ts)) from tbl_ttexts group by k%10 order by k%10;
select k%10, numSequences(tcount2(ts)) from tbl_ttexts group by k%10 order by k%10;

-------------------------------------------------------------------------------

set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------

