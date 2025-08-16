-- This program is open source, licensed under the PostgreSQL License.
-- For license terms, see the LICENSE file.
--
-- Copyright (c) 2014-2017, Dalibo
-- Copyright (c) 2018-2024, The PoWA-team

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_stat_kcache" to load this file. \quit

SET client_encoding = 'UTF8';

CREATE FUNCTION pg_stat_kcache(
    OUT queryid bigint,
    OUT top bool,
    OUT userid      oid,
    OUT dbid        oid,
    /* planning time */
    OUT plan_reads       bigint,             /* total reads, in bytes */
    OUT plan_writes      bigint,             /* total writes, in bytes */
    OUT plan_user_time   double precision,   /* total user CPU time used */
    OUT plan_system_time double precision,   /* total system CPU time used */
    OUT plan_minflts     bigint,             /* total page reclaims (soft page faults) */
    OUT plan_majflts     bigint,             /* total page faults (hard page faults) */
    OUT plan_nswaps      bigint,             /* total swaps */
    OUT plan_msgsnds     bigint,             /* total IPC messages sent */
    OUT plan_msgrcvs     bigint,             /* total IPC messages received */
    OUT plan_nsignals    bigint,             /* total signals received */
    OUT plan_nvcsws      bigint,             /* total voluntary context switches */
    OUT plan_nivcsws     bigint,             /* total involuntary context switches */
    /* execution time */
    OUT exec_reads       bigint,             /* total reads, in bytes */
    OUT exec_writes      bigint,             /* total writes, in bytes */
    OUT exec_user_time   double precision,   /* total user CPU time used */
    OUT exec_system_time double precision,   /* total system CPU time used */
    OUT exec_minflts     bigint,             /* total page reclaims (soft page faults) */
    OUT exec_majflts     bigint,             /* total page faults (hard page faults) */
    OUT exec_nswaps      bigint,             /* total swaps */
    OUT exec_msgsnds     bigint,             /* total IPC messages sent */
    OUT exec_msgrcvs     bigint,             /* total IPC messages received */
    OUT exec_nsignals    bigint,             /* total signals received */
    OUT exec_nvcsws      bigint,             /* total voluntary context switches */
    OUT exec_nivcsws     bigint,             /* total involuntary context switches */
    /* metadata */
    OUT stats_since     timestamptz         /* entry creation time */
)
RETURNS SETOF record
LANGUAGE c COST 1000
AS '$libdir/pg_stat_kcache', 'pg_stat_kcache_2_3';
GRANT ALL ON FUNCTION pg_stat_kcache() TO public;

CREATE FUNCTION pg_stat_kcache_reset()
    RETURNS void
    LANGUAGE c COST 1000
    AS '$libdir/pg_stat_kcache', 'pg_stat_kcache_reset';
REVOKE ALL ON FUNCTION pg_stat_kcache_reset() FROM public;

CREATE VIEW pg_stat_kcache_detail AS
SELECT s.query, k.top, d.datname, r.rolname,
       k.plan_user_time,
       k.plan_system_time,
       k.plan_minflts,
       k.plan_majflts,
       k.plan_nswaps,
       k.plan_reads AS plan_reads,
       k.plan_reads/(current_setting('block_size')::integer) AS plan_reads_blks,
       k.plan_writes AS plan_writes,
       k.plan_writes/(current_setting('block_size')::integer) AS plan_writes_blks,
       k.plan_msgsnds,
       k.plan_msgrcvs,
       k.plan_nsignals,
       k.plan_nvcsws,
       k.plan_nivcsws,
       k.exec_user_time,
       k.exec_system_time,
       k.exec_minflts,
       k.exec_majflts,
       k.exec_nswaps,
       k.exec_reads AS exec_reads,
       k.exec_reads/(current_setting('block_size')::integer) AS exec_reads_blks,
       k.exec_writes AS exec_writes,
       k.exec_writes/(current_setting('block_size')::integer) AS exec_writes_blks,
       k.exec_msgsnds,
       k.exec_msgrcvs,
       k.exec_nsignals,
       k.exec_nvcsws,
       k.exec_nivcsws,
       k.stats_since
  FROM pg_stat_kcache() k
  JOIN pg_stat_statements s
    ON k.queryid = s.queryid AND k.dbid = s.dbid AND k.userid = s.userid
  JOIN pg_database d
    ON  d.oid = s.dbid
  JOIN pg_roles r
    ON r.oid = s.userid;
GRANT SELECT ON pg_stat_kcache_detail TO public;

CREATE VIEW pg_stat_kcache AS
SELECT datname,
       SUM(plan_user_time) AS plan_user_time,
       SUM(plan_system_time) AS plan_system_time,
       SUM(plan_minflts) AS plan_minflts,
       SUM(plan_majflts) AS plan_majflts,
       SUM(plan_nswaps) AS plan_nswaps,
       SUM(plan_reads) AS plan_reads,
       SUM(plan_reads_blks) AS plan_reads_blks,
       SUM(plan_writes) AS plan_writes,
       SUM(plan_writes_blks) AS plan_writes_blks,
       SUM(plan_msgsnds) AS plan_msgsnds,
       SUM(plan_msgrcvs) AS plan_msgrcvs,
       SUM(plan_nsignals) AS plan_nsignals,
       SUM(plan_nvcsws) AS plan_nvcsws,
       SUM(plan_nivcsws) AS plan_nivcsws,
       SUM(exec_user_time) AS exec_user_time,
       SUM(exec_system_time) AS exec_system_time,
       SUM(exec_minflts) AS exec_minflts,
       SUM(exec_majflts) AS exec_majflts,
       SUM(exec_nswaps) AS exec_nswaps,
       SUM(exec_reads) AS exec_reads,
       SUM(exec_reads_blks) AS exec_reads_blks,
       SUM(exec_writes) AS exec_writes,
       SUM(exec_writes_blks) AS exec_writes_blks,
       SUM(exec_msgsnds) AS exec_msgsnds,
       SUM(exec_msgrcvs) AS exec_msgrcvs,
       SUM(exec_nsignals) AS exec_nsignals,
       SUM(exec_nvcsws) AS exec_nvcsws,
       SUM(exec_nivcsws) AS exec_nivcsws,
       MIN(stats_since) AS stats_since
  FROM pg_stat_kcache_detail
  WHERE top IS TRUE
  GROUP BY datname;
GRANT SELECT ON pg_stat_kcache TO public;
