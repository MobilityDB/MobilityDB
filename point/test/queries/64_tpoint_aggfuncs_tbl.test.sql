﻿-------------------------------------------------------------------------------
SET parallel_tuple_cost=0;
SET parallel_setup_cost=0;
SET force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

SELECT setprecision(extent(inst), 13) FROM tbl_tgeompointinst;
SELECT setprecision(extent(inst), 13) FROM tbl_tgeogpointinst;
SELECT setprecision(extent(ti), 13) FROM tbl_tgeompointi;
SELECT setprecision(extent(ti), 13) FROM tbl_tgeogpointi;
SELECT setprecision(extent(seq), 13) FROM tbl_tgeompointseq;
SELECT setprecision(extent(seq), 13) FROM tbl_tgeogpointseq;
SELECT setprecision(extent(ts), 13) FROM tbl_tgeompoints;
SELECT setprecision(extent(ts), 13) FROM tbl_tgeogpoints;
SELECT setprecision(extent(temp), 13) FROM tbl_tgeompoint;
SELECT setprecision(extent(temp), 13) FROM tbl_tgeogpoint;

SELECT setprecision(extent(inst), 13) FROM tbl_tgeompoint3Dinst;
SELECT setprecision(extent(inst), 13) FROM tbl_tgeogpoint3Dinst;
SELECT setprecision(extent(ti), 13) FROM tbl_tgeompoint3Di;
SELECT setprecision(extent(ti), 13) FROM tbl_tgeogpoint3Di;
SELECT setprecision(extent(seq), 13) FROM tbl_tgeompoint3Dseq;
SELECT setprecision(extent(seq), 13) FROM tbl_tgeogpoint3Dseq;
SELECT setprecision(extent(ts), 13) FROM tbl_tgeompoint3Ds;
SELECT setprecision(extent(ts), 13) FROM tbl_tgeogpoint3Ds;
SELECT setprecision(extent(temp), 13) FROM tbl_tgeompoint3D;
SELECT setprecision(extent(temp), 13) FROM tbl_tgeogpoint3D;

-------------------------------------------------------------------------------

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompointinst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompointinst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompointinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompointinst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompointi;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompointi;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompointi GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompointi GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompointseq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompointseq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompointseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompointseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ts)) FROM tbl_tgeompoints;
SELECT numSequences(tcount(ts)) FROM tbl_tgeompoints;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tgeompoints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeompoints GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(inst)) FROM tbl_tgeompoint3Dinst;
SELECT numInstants(tcount(inst)) FROM tbl_tgeompoint3Dinst;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tgeompoint3Dinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeompoint3Dinst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcentroid(ti)) FROM tbl_tgeompoint3Di;
SELECT numInstants(tcount(ti)) FROM tbl_tgeompoint3Di;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tgeompoint3Di GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeompoint3Di GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(seq)) FROM tbl_tgeompoint3Dseq;
SELECT numSequences(tcount(seq)) FROM tbl_tgeompoint3Dseq;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tgeompoint3Dseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeompoint3Dseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcentroid(ts)) FROM tbl_tgeompoint3Ds;
SELECT numSequences(tcount(ts)) FROM tbl_tgeompoint3Ds;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tgeompoint3Ds GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeompoint3Ds GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tgeogpointinst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tgeogpointinst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tgeogpointi;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tgeogpointi GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tgeogpointseq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tgeogpointseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ts)) FROM tbl_tgeogpoints;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tgeogpoints GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------

SET parallel_tuple_cost=100;
SET parallel_setup_cost=100;
SET force_parallel_mode=off;

-------------------------------------------------------------------------------
