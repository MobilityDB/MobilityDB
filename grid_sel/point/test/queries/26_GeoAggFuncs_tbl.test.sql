﻿-------------------------------------------------------------------------------

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