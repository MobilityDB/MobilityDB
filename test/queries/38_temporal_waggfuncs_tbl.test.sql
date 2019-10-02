-------------------------------------------------------------------------------
-- TemporalInst
-------------------------------------------------------------------------------

select numSequences(wmin(inst, interval '5 minutes'))
from tbl_tintinst;

select numSequences(wmax(inst, interval '5 minutes'))
from tbl_tintinst;

select numSequences(wsum(inst, interval '5 minutes'))
from tbl_tintinst;

select numSequences(wcount(inst, interval '5 minutes'))
from tbl_tintinst;

select numSequences(wavg(inst, interval '5 minutes'))
from tbl_tintinst;

-------------------------------------------------------------------------------

select numSequences(wmin(inst, interval '5 minutes'))
from tbl_tfloatinst;

select numSequences(wmax(inst, interval '5 minutes'))
from tbl_tfloatinst;

select numSequences(wsum(inst, interval '5 minutes'))
from tbl_tfloatinst;

select numSequences(wcount(inst, interval '5 minutes'))
from tbl_tfloatinst;

select numSequences(wavg(inst, interval '5 minutes'))
from tbl_tfloatinst;

-------------------------------------------------------------------------------
-- TemporalI
-------------------------------------------------------------------------------

select numSequences(wmin(ti, interval '5 minutes'))
from tbl_tinti;

select numSequences(wmax(ti, interval '5 minutes'))
from tbl_tinti;

select numSequences(wsum(ti, interval '5 minutes'))
from tbl_tinti;

select numSequences(wcount(ti, interval '5 minutes'))
from tbl_tinti;

select numSequences(wavg(ti, interval '5 minutes'))
from tbl_tinti;

-------------------------------------------------------------------------------

select numSequences(wmin(ti, interval '5 minutes'))
from tbl_tfloati;

select numSequences(wmax(ti, interval '5 minutes'))
from tbl_tfloati;

select numSequences(wsum(ti, interval '5 minutes'))
from tbl_tfloati;

select numSequences(wcount(ti, interval '5 minutes'))
from tbl_tfloati;

select numSequences(wavg(ti, interval '5 minutes'))
from tbl_tfloati;

-------------------------------------------------------------------------------
-- TemporalSeq
-------------------------------------------------------------------------------

select numSequences(wmin(seq, interval '5 minutes'))
from tbl_tintseq;

select numSequences(wmax(seq, interval '5 minutes'))
from tbl_tintseq;

select numSequences(wsum(seq, interval '5 minutes'))
from tbl_tintseq;

select numSequences(wcount(seq, interval '5 minutes'))
from tbl_tintseq;

select numSequences(wavg(seq, interval '5 minutes'))
from tbl_tintseq;

-------------------------------------------------------------------------------

select numSequences(wmin(seq, interval '5 minutes'))
from tbl_tfloatseq;

select numSequences(wmax(seq, interval '5 minutes'))
from tbl_tfloatseq;

select numSequences(wcount(seq, interval '5 minutes'))
from tbl_tfloatseq;

-------------------------------------------------------------------------------
-- TemporalS
-------------------------------------------------------------------------------

select numSequences(wmin(ts, interval '5 minutes'))
from tbl_tints;

select numSequences(wmax(ts, interval '5 minutes'))
from tbl_tints;

select numSequences(wsum(ts, interval '5 minutes'))
from tbl_tints;

select numSequences(wcount(ts, interval '5 minutes'))
from tbl_tints;

select numSequences(wavg(ts, interval '5 minutes'))
from tbl_tints;

-------------------------------------------------------------------------------

select numSequences(wmin(ts, interval '5 minutes'))
from tbl_tfloats;

select numSequences(wmax(ts, interval '5 minutes'))
from tbl_tfloats;

select numSequences(wcount(ts, interval '5 minutes'))
from tbl_tfloats;

-------------------------------------------------------------------------------
