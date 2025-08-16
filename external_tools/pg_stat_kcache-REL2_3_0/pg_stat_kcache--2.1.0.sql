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
    OUT userid      oid,
    OUT dbid        oid,
    OUT reads       bigint,             /* total reads, in bytes */
    OUT writes      bigint,             /* total writes, in bytes */
    OUT user_time   double precision,   /* total user CPU time used */
    OUT system_time double precision,   /* total system CPU time used */
    OUT minflts     bigint,             /* total page reclaims (soft page faults) */
    OUT majflts     bigint,             /* total page faults (hard page faults) */
    OUT nswaps      bigint,             /* total swaps */
    OUT msgsnds     bigint,             /* total IPC messages sent */
    OUT msgrcvs     bigint,             /* total IPC messages received */
    OUT nsignals    bigint,             /* total signals received */
    OUT nvcsws      bigint,             /* total voluntary context switches */
    OUT nivcsws     bigint              /* total involuntary context switches */
)
RETURNS SETOF record
LANGUAGE c COST 1000
AS '$libdir/pg_stat_kcache', 'pg_stat_kcache_2_1';
GRANT ALL ON FUNCTION pg_stat_kcache() TO public;

CREATE FUNCTION pg_stat_kcache_reset()
    RETURNS void
    LANGUAGE c COST 1000
    AS '$libdir/pg_stat_kcache', 'pg_stat_kcache_reset';
REVOKE ALL ON FUNCTION pg_stat_kcache_reset() FROM public;

CREATE VIEW pg_stat_kcache_detail AS
SELECT s.query, d.datname, r.rolname,
       k.user_time,
       k.system_time,
       k.minflts,
       k.majflts,
       k.nswaps,
       k.reads AS reads,
       k.reads/(current_setting('block_size')::integer) AS reads_blks,
       k.writes AS writes,
       k.writes/(current_setting('block_size')::integer) AS writes_blks,
       k.msgsnds,
       k.msgrcvs,
       k.nsignals,
       k.nvcsws,
       k.nivcsws
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
       SUM(user_time) AS user_time,
       SUM(system_time) AS system_time,
       SUM(minflts) AS minflts,
       SUM(majflts) AS majflts,
       SUM(nswaps) AS nswaps,
       SUM(reads) AS reads,
       SUM(reads_blks) AS reads_blks,
       SUM(writes) AS writes,
       SUM(writes_blks) AS writes_blks,
       SUM(msgsnds) AS msgsnds,
       SUM(msgrcvs) AS msgrcvs,
       SUM(nsignals) AS nsignals,
       SUM(nvcsws) AS nvcsws,
       SUM(nivcsws) AS nivcsws
  FROM pg_stat_kcache_detail
 GROUP BY datname;
GRANT SELECT ON pg_stat_kcache TO public;
