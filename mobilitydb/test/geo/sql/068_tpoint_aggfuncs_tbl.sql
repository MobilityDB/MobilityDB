-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT round(extent(inst), 6) FROM tbl_tgeompoint_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeogpoint_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeompoint_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeogpoint_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeompoint_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeogpoint_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeompoint_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeogpoint_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint;

SELECT round(extent(inst), 6) FROM tbl_tgeompoint3D_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeogpoint3D_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeompoint3D_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeogpoint3D_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeompoint3D_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeogpoint3D_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeompoint3D_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeogpoint3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeompoint3D;
SELECT round(extent(temp), 6) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompoint_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompoint_inst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompoint_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompoint_discseq;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompoint_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompoint_seq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ss)) FROM tbl_tgeompoint_seqset;
SELECT numSequences(tcount(ss)) FROM tbl_tgeompoint_seqset;
SELECT k%10, numSequences(tcentroid(ss)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeompoint_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompoint3D_inst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompoint3D_inst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompoint3D_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompoint3D_discseq;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompoint3D_discseq;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompoint3D_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompoint3D_seq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompoint3D_seq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompoint3D_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompoint3D_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ss)) FROM tbl_tgeompoint3D_seqset;
SELECT numSequences(tcount(ss)) FROM tbl_tgeompoint3D_seqset;
SELECT k%10, numSequences(tcentroid(ss)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeompoint3D_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeogpoint_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeogpoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeogpoint_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeogpoint_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeogpoint_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeogpoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeogpoint_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeogpoint_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
