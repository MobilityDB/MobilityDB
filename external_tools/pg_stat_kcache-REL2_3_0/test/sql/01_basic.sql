CREATE EXTENSION pg_stat_statements;
CREATE EXTENSION pg_stat_kcache;

-- first make sure that catcache is loaded to avoid physical reads
SELECT count(*) >= 0 FROM pg_stat_kcache;
SELECT pg_stat_kcache_reset();

-- dummy query
SELECT 1 AS dummy;

SELECT count(*) FROM pg_stat_kcache WHERE datname = current_database();

SELECT count(*) FROM pg_stat_kcache_detail WHERE datname = current_database() AND (query = 'SELECT $1 AS dummy' OR query = 'SELECT ? AS dummy;');

SELECT exec_reads, exec_reads_blks, exec_writes, exec_writes_blks
FROM pg_stat_kcache_detail
WHERE datname = current_database()
AND (query = 'SELECT $1 AS dummy' OR query = 'SELECT ? AS dummy;');

-- dummy table
CREATE TABLE test AS SELECT i FROM generate_series(1, 1000) i;

-- dummy query again
SELECT count(*) FROM test;

SELECT exec_user_time + exec_system_time > 0 AS cpu_time_ok
FROM pg_stat_kcache_detail
WHERE datname = current_database()
AND query LIKE 'SELECT count(*) FROM test%';

-- dummy nested query
SET pg_stat_statements.track = 'all';
SET pg_stat_statements.track_planning = TRUE;
SET pg_stat_kcache.track = 'all';
SET pg_stat_kcache.track_planning = TRUE;
SELECT pg_stat_kcache_reset();

CREATE OR REPLACE FUNCTION plpgsql_nested()
  RETURNS void AS $$
BEGIN
  PERFORM i::text as str from generate_series(1, 100) as i;
  PERFORM md5(i::text) as str from generate_series(1, 100) as i;
END
$$ LANGUAGE plpgsql;

SELECT plpgsql_nested();

SELECT COUNT(*) FROM pg_stat_kcache_detail WHERE top IS FALSE;
