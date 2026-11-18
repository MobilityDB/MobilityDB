-------------------------------------------------------------------------------
-- Temporal instant
-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tint_inst;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tint_inst;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tfloat_inst;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tfloat_inst;

-------------------------------------------------------------------------------
-- Temporal discrete sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tint_discseq;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tint_discseq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tfloat_discseq;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tfloat_discseq;

-------------------------------------------------------------------------------
-- Temporal continuous sequence
-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tint_contseq;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tint_contseq;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tint_contseq;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tint_contseq;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tint_contseq;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tfloat_contseq;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tfloat_contseq;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tfloat_contseq;

-------------------------------------------------------------------------------
-- Temporal sequence sets
-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wsum(temp, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tint_seqset;
SELECT numSequences(wavg(temp, interval '5 minutes')) FROM tbl_tint_seqset;

-------------------------------------------------------------------------------

SELECT numSequences(wmin(temp, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wmax(temp, interval '5 minutes')) FROM tbl_tfloat_seqset;
SELECT numSequences(wcount(temp, interval '5 minutes')) FROM tbl_tfloat_seqset;

-------------------------------------------------------------------------------
