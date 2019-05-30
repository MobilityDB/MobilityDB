
-------------------------------------------------------------------------------
-- Test all operators without having collected statistics

SELECT count(*) FROM tbl_timestampset WHERE ts < timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p < period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps < periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p <= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <= periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts > timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p > period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps > periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts >= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p >= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps >= periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestampset WHERE ts @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestampset WHERE ts && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps -|- periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

analyze tbl_period;
analyze tbl_periodset;
analyze tbl_timestampset;

-------------------------------------------------------------------------------
-- Test all operators after having collected statistics

SELECT count(*) FROM tbl_timestampset WHERE ts < timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p < period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps < periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p <= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <= periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts > timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p > period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps > periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts >= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p >= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps >= periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestampset WHERE ts @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestampset WHERE ts && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestamptz WHERE t #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_timestamptz WHERE t -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestamptz WHERE t -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_timestampset WHERE ts -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_timestampset WHERE ts -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_period WHERE p -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_period WHERE p -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_period WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_period WHERE p -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT count(*) FROM tbl_periodset WHERE ps -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_periodset WHERE ps -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_periodset WHERE ps -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_periodset WHERE ps -|- periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

--select period_statistics_validate();
--vacuum analyse tbl_period;
--vacuum analyse tbl_periodset;
--vacuum analyse tbl_timestampset;
--SELECT count(*) FROM execution_stats WHERE abs(PlanRows-ActualRows) > 10
-- STATISTICS COLLECTION FUNCTIONS

DROP FUNCTION IF EXISTS period_statistics_validate;
CREATE OR REPLACE FUNCTION period_statistics_validate() 
RETURNS char(10) AS $$
DECLARE
	Query char(5);
	PlanRows bigint;
	ActualRows bigint;
	QFilter  varchar;
	RowsRemovedbyFilter bigint;
	J json;
	StartTime timestamp;
	RandTimestamp timestamptz;
	RandPeriod period;
	RandTimestampSet timestampset;
	RandPeriodset periodset;
	k int;	
BEGIN

CREATE TABLE IF NOT EXISTS execution_stats 
(Query char(5), 
StartTime timestamp, 
QFilter varchar, 
PlanRows bigint, 
ActualRows bigint, 
RowsRemovedByFilter bigint, 
J json);

TRUNCATE TABLE execution_stats;

SET log_error_verbosity to terse;
k:= 0;

-----------------------------------------------
---- OPERATOR @>-------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p @> RandTimeStamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p @> RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p @> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE RandPeriod @> p
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;



k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p @> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE RandPeriodset @> p 
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

----------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset 
	WHERE ps @> RandTimeStamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset 
	WHERE ps @> RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps @> RandTimeStamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps @> RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps @> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE RandPeriod @> ps
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps @> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE RandPeriodset @> ps
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


-----------------------------------------------
---- OPERATOR <@-------------------------------
-----------------------------------------------


k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE RandTimeStamp <@ ps
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE RandTimeStamp <@ p
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <@ RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <@ RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <@ RandPeriodSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;



k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p <@ RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p <@ RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <@ RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <@ RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR &&-------------------------------
-----------------------------------------------


k:= k+1;
FOR i IN 1..100 LOOP
	RandTimeStampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset 
	WHERE ps && RandTimeStampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset 
	WHERE ps && RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps && RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p && RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p && RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p && RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps && RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps && RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps && RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


-----------------------------------------------
---- OPERATOR <<#------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE RandTimestamp <<# p
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE RandTimestamp <<# ps
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset
	WHERE RandTimestamp <<# ps
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <<# RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps <<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p <<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p <<# RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p <<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p <<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100); 
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <<# RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps <<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;



-----------------------------------------------
---- OPERATOR #>>------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #>> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #>> RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #>> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #>> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #>> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p #>> RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #>> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #>> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #>> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #>> RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #>> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #>> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;




-----------------------------------------------
---- OPERATOR &<#------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps &<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps &<# RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps &<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps &<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p &<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p &<# RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p &<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p &<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps &<# RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampSet:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps &<# RandTimestampSet
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps &<# RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps &<# RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


-----------------------------------------------
---- OPERATOR #&>------------------------------
-----------------------------------------------


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #&> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #&> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #&> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #&> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p #&> RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #&> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	Randtimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset
	WHERE ps #&> RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps #&> RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #&> RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #&> RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period 
	WHERE p #&> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps #&> RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


-----------------------------------------------
---- OPERATOR -|-  ----------------------------
-----------------------------------------------


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps -|- RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_timestampset
	WHERE ps -|- RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p -|- RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	Randtimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p -|- RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p -|- RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_period
	WHERE p -|- RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	Randtimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset
	WHERE ps -|- RandTimestamp
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandTimestampset:= random_timestampset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset
	WHERE ps -|- RandTimestampset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps -|- RandPeriod
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
	RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
	EXPLAIN (ANALYZE, FORMAT JSON)
	SELECT * 
	FROM tbl_periodset 
	WHERE ps -|- RandPeriodset
	INTO J;

	StartTime := clock_timestamp();
	PlanRows:= (J->0->'Plan'->>'Plan Rows')::bigint;
	ActualRows:=  (J->0->'Plan'->>'Actual Rows')::bigint;
	QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
	RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: bigint;

	Query:= 'Q' || k;		
	INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


RETURN 'THE END'; 
END;
$$ LANGUAGE 'plpgsql';

