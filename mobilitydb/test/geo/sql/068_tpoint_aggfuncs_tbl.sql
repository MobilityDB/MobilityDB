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

SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D;

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

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint3D_inst;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint3D_inst;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(temp)) FROM tbl_tgeompoint3D_discseq;
SELECT numInstants(tcount(temp)) FROM tbl_tgeompoint3D_discseq;
SELECT k%10, numInstants(tcentroid(temp)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint3D_contseq;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint3D_contseq;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint3D_contseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint3D_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(temp)) FROM tbl_tgeompoint3D_seqset;
SELECT numSequences(tcount(temp)) FROM tbl_tgeompoint3D_seqset;
SELECT k%10, numSequences(tcentroid(temp)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;

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
