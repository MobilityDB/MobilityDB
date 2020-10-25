-------------------------------------------------------------------------------
-- TemporalInst
-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes'))
FROM tbl_tintinst;

SELECT numSequences(wmax(inst, interval '5 minutes'))
FROM tbl_tintinst;

SELECT numSequences(wsum(inst, interval '5 minutes'))
FROM tbl_tintinst;

SELECT numSequences(wcount(inst, interval '5 minutes'))
FROM tbl_tintinst;

SELECT numSequences(wavg(inst, interval '5 minutes'))
FROM tbl_tintinst;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes'))
FROM tbl_tfloatinst;

SELECT numSequences(wmax(inst, interval '5 minutes'))
FROM tbl_tfloatinst;

SELECT numSequences(wsum(inst, interval '5 minutes'))
FROM tbl_tfloatinst;

SELECT numSequences(wcount(inst, interval '5 minutes'))
FROM tbl_tfloatinst;

SELECT numSequences(wavg(inst, interval '5 minutes'))
FROM tbl_tfloatinst;

-------------------------------------------------------------------------------
-- TemporalI
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes'))
FROM tbl_tinti;

SELECT numSequences(wmax(ti, interval '5 minutes'))
FROM tbl_tinti;

SELECT numSequences(wsum(ti, interval '5 minutes'))
FROM tbl_tinti;

SELECT numSequences(wcount(ti, interval '5 minutes'))
FROM tbl_tinti;

SELECT numSequences(wavg(ti, interval '5 minutes'))
FROM tbl_tinti;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes'))
FROM tbl_tfloati;

SELECT numSequences(wmax(ti, interval '5 minutes'))
FROM tbl_tfloati;

SELECT numSequences(wsum(ti, interval '5 minutes'))
FROM tbl_tfloati;

SELECT numSequences(wcount(ti, interval '5 minutes'))
FROM tbl_tfloati;

SELECT numSequences(wavg(ti, interval '5 minutes'))
FROM tbl_tfloati;

-------------------------------------------------------------------------------
-- TemporalSeq
-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes'))
FROM tbl_tintseq;

SELECT numSequences(wmax(seq, interval '5 minutes'))
FROM tbl_tintseq;

SELECT numSequences(wsum(seq, interval '5 minutes'))
FROM tbl_tintseq;

SELECT numSequences(wcount(seq, interval '5 minutes'))
FROM tbl_tintseq;

SELECT numSequences(wavg(seq, interval '5 minutes'))
FROM tbl_tintseq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes'))
FROM tbl_tfloatseq;

SELECT numSequences(wmax(seq, interval '5 minutes'))
FROM tbl_tfloatseq;

SELECT numSequences(wcount(seq, interval '5 minutes'))
FROM tbl_tfloatseq;

-------------------------------------------------------------------------------
-- TemporalS
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ts, interval '5 minutes'))
FROM tbl_tints;

SELECT numSequences(wmax(ts, interval '5 minutes'))
FROM tbl_tints;

SELECT numSequences(wsum(ts, interval '5 minutes'))
FROM tbl_tints;

SELECT numSequences(wcount(ts, interval '5 minutes'))
FROM tbl_tints;

SELECT numSequences(wavg(ts, interval '5 minutes'))
FROM tbl_tints;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ts, interval '5 minutes'))
FROM tbl_tfloats;

SELECT numSequences(wmax(ts, interval '5 minutes'))
FROM tbl_tfloats;

SELECT numSequences(wcount(ts, interval '5 minutes'))
FROM tbl_tfloats;

-------------------------------------------------------------------------------
