-------------------------------------------------------------------------------
-- Tests of operators that do not involved indexes for time types.
-- File TimeOps.c
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t -|- p;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t -|- ps;

SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts -|- p;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts -|- ps;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p -|- t;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p -|- ts;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p -|- t2.p;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p -|- ps;

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps -|- t;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps -|- ts;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps -|- p;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps -|- t2.ps;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p + t2.p IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts + t IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t + ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts + t2.ts IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps + p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p + ps IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps + t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

/* In SQL timestamptz - timestamptz yields an interval */
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t - ts IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_period WHERE t - p IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_periodset WHERE t - ps IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts - t IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts - t2.ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_period WHERE ts - p IS NOT NULL;
SELECT count(*) FROM tbl_timestampset, tbl_periodset WHERE ts - ps IS NOT NULL;

SELECT count(*) FROM tbl_period, tbl_timestamptz WHERE p - t IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_timestampset WHERE p - ts IS NOT NULL;
SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p - t2.p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p - ps IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_timestamptz WHERE ps - t IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_timestampset WHERE ps - ts IS NOT NULL;
SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps - p IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps - t2.ps IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_period t1, tbl_period t2 WHERE t1.p * t2.p IS NOT NULL;

SELECT count(*) FROM tbl_timestampset, tbl_timestamptz WHERE ts * t IS NOT NULL;
SELECT count(*) FROM tbl_timestamptz, tbl_timestampset WHERE t * ts IS NOT NULL;
SELECT count(*) FROM tbl_timestampset t1, tbl_timestampset t2 WHERE t1.ts * t2.ts IS NOT NULL;

SELECT count(*) FROM tbl_periodset, tbl_period WHERE ps * p IS NOT NULL;
SELECT count(*) FROM tbl_period, tbl_periodset WHERE p * ps IS NOT NULL;
SELECT count(*) FROM tbl_periodset t1, tbl_periodset t2 WHERE t1.ps * t2.ps IS NOT NULL;

-------------------------------------------------------------------------------
