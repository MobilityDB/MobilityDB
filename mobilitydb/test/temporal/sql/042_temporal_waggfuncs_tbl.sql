-------------------------------------------------------------------------------
-- Temporal instant
-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wmax(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wsum(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wcount(inst, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wavg(inst, interval '5 minutes')) FROM tbl_tint_inst;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wmax(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wsum(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wcount(inst, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wavg(inst, interval '5 minutes')) FROM tbl_tfloat_inst;

-------------------------------------------------------------------------------
-- Temporal discrete sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wmax(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wsum(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wcount(ti, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wavg(ti, interval '5 minutes')) FROM tbl_tint_discseq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wmax(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wsum(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wcount(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wavg(ti, interval '5 minutes')) FROM tbl_tfloat_discseq;

-------------------------------------------------------------------------------
-- Temporal continuous sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wmax(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wsum(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wcount(seq, interval '5 minutes')) FROM tbl_tint_seq;
SELECT numSequences(wavg(seq, interval '5 minutes')) FROM tbl_tint_seq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(seq, interval '5 minutes')) FROM tbl_tfloat_seq;
SELECT numSequences(wmax(seq, interval '5 minutes')) FROM tbl_tfloat_seq;
SELECT numSequences(wcount(seq, interval '5 minutes')) FROM tbl_tfloat_seq;

-------------------------------------------------------------------------------
-- Temporal sequence sets
-------------------------------------------------------------------------------

SELECT numSequences(wmin(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wmax(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wsum(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wcount(ss, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wavg(ss, interval '5 minutes')) FROM tbl_tint_seqset;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wmax(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wcount(ss, interval '5 minutes')) FROM tbl_tfloat_seqset;

-------------------------------------------------------------------------------
