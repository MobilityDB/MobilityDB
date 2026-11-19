-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT extent(temp) FROM tbl_tbool_inst;
SELECT extent(temp) FROM tbl_ttext_inst;
SELECT extent(temp) FROM tbl_tint_inst;
SELECT round(extent(temp), 6) FROM tbl_tfloat_inst;

SELECT extent(temp) FROM tbl_tbool_discseq;
SELECT extent(temp) FROM tbl_ttext_discseq;
SELECT extent(temp) FROM tbl_tint_discseq;
SELECT round(extent(temp), 6) FROM tbl_tfloat_discseq;

SELECT extent(temp) FROM tbl_tbool_contseq;
SELECT extent(temp) FROM tbl_ttext_contseq;
SELECT extent(temp) FROM tbl_tint_contseq;
SELECT round(extent(temp), 6) FROM tbl_tfloat_contseq;

SELECT extent(temp) FROM tbl_tbool_seqset;
SELECT extent(temp) FROM tbl_ttext_seqset;
SELECT extent(temp) FROM tbl_tint_seqset;
SELECT round(extent(temp), 6) FROM tbl_tfloat_seqset;

SELECT extent(temp) FROM tbl_tbool;
SELECT extent(temp) FROM tbl_ttext;
SELECT extent(temp) FROM tbl_tint;
SELECT round(extent(temp), 6) FROM tbl_tfloat;

-------------------------------------------------------------------------------
-- TemporalInst aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(temp)) FROM tbl_tbool_inst;
SELECT numInstants(tor(temp)) FROM tbl_tbool_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tbool_inst;

SELECT numInstants(tmin(temp)) FROM tbl_tint_inst;
SELECT numInstants(tmax(temp)) FROM tbl_tint_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tint_inst;
SELECT numInstants(tsum(temp)) FROM tbl_tint_inst;
SELECT numInstants(tavg(temp)) FROM tbl_tint_inst;

SELECT numInstants(tmin(temp)) FROM tbl_tfloat_inst;
SELECT numInstants(tmax(temp)) FROM tbl_tfloat_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tfloat_inst;
SELECT numInstants(tsum(temp)) FROM tbl_tfloat_inst;
SELECT numInstants(tavg(temp)) FROM tbl_tfloat_inst;

SELECT numInstants(tmin(temp)) FROM tbl_ttext_inst;
SELECT numInstants(tmax(temp)) FROM tbl_ttext_inst;
SELECT numInstants(tcount(temp)) FROM tbl_ttext_inst;

SELECT k%10, numInstants(tand(temp)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(temp)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tbool_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(temp)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(temp)) FROM tbl_tint_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(temp)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(temp)) FROM tbl_tfloat_inst GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_ttext_inst GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- Temporal discrete sequence aggregate functions
-------------------------------------------------------------------------------

SELECT numInstants(tand(temp)) FROM tbl_tbool_discseq;
SELECT numInstants(tor(temp)) FROM tbl_tbool_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tbool_discseq;

SELECT numInstants(tmin(temp)) FROM tbl_tint_discseq;
SELECT numInstants(tmax(temp)) FROM tbl_tint_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tint_discseq;
SELECT numInstants(tsum(temp)) FROM tbl_tint_discseq;
SELECT numInstants(tavg(temp)) FROM tbl_tint_discseq;

SELECT numInstants(tmin(temp)) FROM tbl_tfloat_discseq;
SELECT numInstants(tmax(temp)) FROM tbl_tfloat_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tfloat_discseq;
SELECT numInstants(tsum(temp)) FROM tbl_tfloat_discseq;
SELECT numInstants(tavg(temp)) FROM tbl_tfloat_discseq;

SELECT numInstants(tmin(temp)) FROM tbl_ttext_discseq;
SELECT numInstants(tmax(temp)) FROM tbl_ttext_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_ttext_discseq;

SELECT k%10, numInstants(tand(temp)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tor(temp)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tbool_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(temp)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(temp)) FROM tbl_tint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tsum(temp)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tavg(temp)) FROM tbl_tfloat_discseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numInstants(tmin(temp)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tmax(temp)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_ttext_discseq GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- Temporal continuous sequence aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(temp)) FROM tbl_tbool_contseq;
SELECT numSequences(tor(temp)) FROM tbl_tbool_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tbool_contseq;

SELECT numSequences(tmin(temp)) FROM tbl_tint_contseq;
SELECT numSequences(tmax(temp)) FROM tbl_tint_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tint_contseq;
SELECT numSequences(tsum(temp)) FROM tbl_tint_contseq;
SELECT numSequences(tavg(temp)) FROM tbl_tint_contseq;

SELECT numSequences(tmin(temp)) FROM tbl_tfloat_contseq;
SELECT numSequences(tmax(temp)) FROM tbl_tfloat_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tfloat_contseq;
SELECT numSequences(tsum(temp)) FROM tbl_tfloat_contseq;
SELECT numSequences(tavg(temp)) FROM tbl_tfloat_contseq;

SELECT numSequences(tmin(temp)) FROM tbl_ttext_contseq;
SELECT numSequences(tmax(temp)) FROM tbl_ttext_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_ttext_contseq;

SELECT k%10, numSequences(tand(temp)) FROM tbl_tbool_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(temp)) FROM tbl_tbool_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tbool_contseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_tint_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_tint_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tint_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(temp)) FROM tbl_tint_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(temp)) FROM tbl_tint_contseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_tfloat_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_tfloat_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tfloat_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(temp)) FROM tbl_tfloat_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(temp)) FROM tbl_tfloat_contseq GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_ttext_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_ttext_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_ttext_contseq GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
-- Temporal sequence set aggregate functions
-------------------------------------------------------------------------------

SELECT numSequences(tand(temp)) FROM tbl_tbool_seqset;
SELECT numSequences(tor(temp)) FROM tbl_tbool_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tbool_seqset;

SELECT numSequences(tmin(temp)) FROM tbl_tint_seqset;
SELECT numSequences(tmax(temp)) FROM tbl_tint_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tint_seqset;
SELECT numSequences(tsum(temp)) FROM tbl_tint_seqset;
SELECT numSequences(tavg(temp)) FROM tbl_tint_seqset;

SELECT numSequences(tmin(temp)) FROM tbl_tfloat_seqset;
SELECT numSequences(tmax(temp)) FROM tbl_tfloat_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tfloat_seqset;
SELECT numSequences(tsum(temp)) FROM tbl_tfloat_seqset;
SELECT numSequences(tavg(temp)) FROM tbl_tfloat_seqset;

SELECT numSequences(tmin(temp)) FROM tbl_ttext_seqset;
SELECT numSequences(tmax(temp)) FROM tbl_ttext_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_ttext_seqset;

SELECT k%10, numSequences(tand(temp)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tor(temp)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tbool_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(temp)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(temp)) FROM tbl_tint_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tsum(temp)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tavg(temp)) FROM tbl_tfloat_seqset GROUP BY k%10 ORDER BY k%10;

SELECT k%10, numSequences(tmin(temp)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tmax(temp)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_ttext_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT DISTINCT ON (getTimestamp(temp)) temp
  FROM tbl_tint_inst
  ORDER BY getTimestamp(temp) )
SELECT numInstants(appendInstant(temp)) FROM temp;

-------------------------------------------------------------------------------


-------------------------------------------------------------------------------

  