-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT round(extent(inst), 6) FROM tbl_tgeometry_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeography_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeometry_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeography_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeometry_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeography_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeometry_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeography_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry;
SELECT round(extent(temp), 6) FROM tbl_tgeography;

SELECT round(extent(inst), 6) FROM tbl_tgeometry3D_inst;
SELECT round(extent(inst), 6) FROM tbl_tgeography3D_inst;
SELECT round(extent(ti), 6) FROM tbl_tgeometry3D_discseq;
SELECT round(extent(ti), 6) FROM tbl_tgeography3D_discseq;
SELECT round(extent(seq), 6) FROM tbl_tgeometry3D_seq;
SELECT round(extent(seq), 6) FROM tbl_tgeography3D_seq;
SELECT round(extent(ss), 6) FROM tbl_tgeometry3D_seqset;
SELECT round(extent(ss), 6) FROM tbl_tgeography3D_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3D;
SELECT round(extent(temp), 6) FROM tbl_tgeography3D;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeometry_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeometry_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeometry_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeometry_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeometry_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeometry_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeometry_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeometry_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(inst)) FROM tbl_tgeometry3D_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeometry3D_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeometry3D_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeometry3D_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeometry3D_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeometry3D_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeometry3D_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeometry3D_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeography_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeography_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeography_discseq;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeography_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeography_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeography_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ss)) FROM tbl_tgeography_seqset;
SELECT k%10, numSequences(tcount(ss)) FROM tbl_tgeography_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
