-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT round(extent(temp), 6) FROM tbl_tgeometry_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeography_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeometry_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeography_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeometry_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeography_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeometry_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeography_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry;
SELECT round(extent(temp), 6) FROM tbl_tgeography;

SELECT round(extent(temp), 6) FROM tbl_tgeometry3d_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeography3d_inst;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3d_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeography3d_discseq;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3d_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeography3d_contseq;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3d_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeography3d_seqset;
SELECT round(extent(temp), 6) FROM tbl_tgeometry3d;
SELECT round(extent(temp), 6) FROM tbl_tgeography3d;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(temp)) FROM tbl_tgeometry_inst;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeometry_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(temp)) FROM tbl_tgeometry_discseq;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeometry_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeometry_contseq;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeometry_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeometry_seqset;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeometry_seqset GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(temp)) FROM tbl_tgeometry3d_inst;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeometry3d_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(temp)) FROM tbl_tgeometry3d_discseq;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeometry3d_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeometry3d_contseq;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeometry3d_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeometry3d_seqset;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeometry3d_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(temp)) FROM tbl_tgeography_inst;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeography_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(temp)) FROM tbl_tgeography_discseq;
SELECT k%10, numInstants(tcount(temp)) FROM tbl_tgeography_discseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeography_contseq;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeography_contseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(temp)) FROM tbl_tgeography_seqset;
SELECT k%10, numSequences(tcount(temp)) FROM tbl_tgeography_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
