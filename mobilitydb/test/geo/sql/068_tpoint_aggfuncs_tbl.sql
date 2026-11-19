-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT round(extent(temp), 6) FROM tbl_tgeompoint_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint;

SELECT round(extent(temp), 6) FROM tbl_tgeompoint3d_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3d_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3d_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3d_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3d_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3d_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3d_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3d_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3d;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3d;

-------------------------------------------------------------------------------

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint_inst;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint_discseq;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint_contseq;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint_seqset;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint3d_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint3d_inst;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint3d_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint3d_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint3d_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint3d_discseq;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint3d_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint3d_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint3d_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint3d_contseq;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint3d_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint3d_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint3d_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint3d_seqset;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint3d_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint3d_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(temp)) FROM tbl_tgeogpoint_inst;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeogpoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(temp)) FROM tbl_tgeogpoint_discseq;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeogpoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeogpoint_contseq;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeogpoint_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeogpoint_seqset;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeogpoint_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
