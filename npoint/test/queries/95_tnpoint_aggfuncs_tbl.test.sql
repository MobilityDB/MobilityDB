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

SELECT numInstants(tcount(inst)) FROM tbl_tnpointinst;
SELECT numInstants(wcount(inst, '1 hour')) FROM tbl_tnpointinst;
SELECT numInstants(tcentroid(inst)) FROM tbl_tnpointinst;
SELECT k%10, numInstants(tcount(inst)) FROM tbl_tnpointinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(wcount(inst, '1 hour')) FROM tbl_tnpointinst GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(inst)) FROM tbl_tnpointinst GROUP BY k%10 ORDER BY k%10;

SELECT numInstants(tcount(ti)) FROM tbl_tnpointi;
SELECT numInstants(wcount(ti, '1 hour')) FROM tbl_tnpointi;
SELECT numInstants(tcentroid(ti)) FROM tbl_tnpointi;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpointi GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcount(ti)) FROM tbl_tnpointi GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numInstants(tcentroid(ti)) FROM tbl_tnpointi GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(seq)) FROM tbl_tnpointseq;
SELECT numSequences(wcount(seq, '1 hour')) FROM tbl_tnpointseq;
SELECT numSequences(tcentroid(seq)) FROM tbl_tnpointseq;
SELECT k%10, numSequences(tcount(seq)) FROM tbl_tnpointseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(seq, '1 hour')) FROM tbl_tnpointseq GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(seq)) FROM tbl_tnpointseq GROUP BY k%10 ORDER BY k%10;

SELECT numSequences(tcount(ts)) FROM tbl_tnpoints;
SELECT numSequences(wcount(ts, '1 hour')) FROM tbl_tnpoints;
SELECT numSequences(tcentroid(ts)) FROM tbl_tnpoints;
SELECT k%10, numSequences(tcount(ts)) FROM tbl_tnpoints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(wcount(ts, '1 hour'))) FROM tbl_tnpoints GROUP BY k%10 ORDER BY k%10;
SELECT k%10, numSequences(tcentroid(ts)) FROM tbl_tnpoints GROUP BY k%10 ORDER BY k%10;

-------------------------------------------------------------------------------
