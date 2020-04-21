-------------------------------------------------------------------------------
set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;

-------------------------------------------------------------------------------
-- Extent aggregate function
-------------------------------------------------------------------------------

select setprecision(extent(inst), 13) from tbl_tgeompointinst;
select setprecision(extent(inst), 13) from tbl_tgeogpointinst;
select setprecision(extent(ti), 13) from tbl_tgeompointi;
select setprecision(extent(ti), 13) from tbl_tgeogpointi;
select setprecision(extent(seq), 13) from tbl_tgeompointseq;
select setprecision(extent(seq), 13) from tbl_tgeogpointseq;
select setprecision(extent(ts), 13) from tbl_tgeompoints;
select setprecision(extent(ts), 13) from tbl_tgeogpoints;
select setprecision(extent(temp), 13) from tbl_tgeompoint;
select setprecision(extent(temp), 13) from tbl_tgeogpoint;

select setprecision(extent(inst), 13) from tbl_tgeompoint3Dinst;
select setprecision(extent(inst), 13) from tbl_tgeogpoint3Dinst;
select setprecision(extent(ti), 13) from tbl_tgeompoint3Di;
select setprecision(extent(ti), 13) from tbl_tgeogpoint3Di;
select setprecision(extent(seq), 13) from tbl_tgeompoint3Dseq;
select setprecision(extent(seq), 13) from tbl_tgeogpoint3Dseq;
select setprecision(extent(ts), 13) from tbl_tgeompoint3Ds;
select setprecision(extent(ts), 13) from tbl_tgeogpoint3Ds;
select setprecision(extent(temp), 13) from tbl_tgeompoint3D;
select setprecision(extent(temp), 13) from tbl_tgeogpoint3D;

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

set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
