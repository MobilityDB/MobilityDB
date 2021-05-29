-------------------------------------------------------------------------------

SELECT asText(tcentroid(temp)) FROM ( VALUES
	(NULL::tnpoint),
	('Npoint(1, 0.5)@2000-01-01'),
	('{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')) t(temp);
SELECT asText(tcentroid(temp)) FROM ( VALUES
	(tnpoint 'Npoint(1, 0.5)@2000-01-01'),
	('{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'),
	(NULL)) t(temp);
/* Errors */
SELECT asText(tcentroid(temp)) FROM ( VALUES
	(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}'),
	('[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')) t(temp);

-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) FROM tbl_tnpoint_inst;
SELECT numInstants(wcount(inst, '1 hour')) FROM tbl_tnpoint_inst;
SELECT numInstants(tcentroid(inst)) FROM tbl_tnpoint_inst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(wcount(inst, '1 hour')) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tnpoint_inst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tnpoint_instset;
SELECT numInstants(wcount(ti, '1 hour')) FROM tbl_tnpoint_instset;
SELECT numInstants(tcentroid(ti)) FROM tbl_tnpoint_instset;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpoint_instset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpoint_instset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tnpoint_instset GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tnpoint_seq;
SELECT numSequences(wcount(seq, '1 hour')) FROM tbl_tnpoint_seq;
SELECT numSequences(tcentroid(seq)) FROM tbl_tnpoint_seq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(seq, '1 hour')) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tnpoint_seq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ts)) FROM tbl_tnpoint_seqset;
SELECT numSequences(wcount(ts, '1 hour')) FROM tbl_tnpoint_seqset;
SELECT numSequences(tcentroid(ts)) FROM tbl_tnpoint_seqset;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(ts, '1 hour'))) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tnpoint_seqset GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
