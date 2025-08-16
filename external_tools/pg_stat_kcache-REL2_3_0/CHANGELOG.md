## 2.3.0 (2024-09-17)

**New features**:

  - Track entry creation timestamp (Julien Rouhaud, thanks to Andrei Zubkov for
    the feature request)

**Bugfix**:

  - Fix some issues with tracking nesting level (Julien Rouhaud)
  - Fix the pgsk_counters_hook declaration (Julien Rouhaud)

**Miscellaneous**:

  - Add postgres 17 compatibility (Julien Rouhaud)

## 2.2.3 (2024-01-24)

**Miscellaneous**:

  - Improve performance for workloads with a high number of clients (Julien
    Rouhaud, thanks to Vitaliy Kukharik for the report)

## 2.2.2 (2023-08-01)

**Miscellaneous**:

  - Add postgres 16 compatibility (Julien Rouhaud, Christoph Berg, Japin Li)
  - Add CI (Christoph Berg)
  - Recommend installation from PGDG repository rather than compiling the
    extension (Julien Rouhaud)

## 2.2.1 (2022-05-16)

**Bugfix**:

  - Fix memory allocation on postgres 12 and above (Julien Rouhaud)
**New features**:

  - Expose some structures and add a counters hook so other extensions can
    extend this extension's metrics (Sviatoslav Ermilin)

**Miscellaneous**:

  - Various improvements to the debian packaging (Christoph Berg)
  - Add compatibility with postgres 15 (Julien Rouhaud)
  - Improve extension update documentation (Julien Rouhaud)

## 2.2.0 (2020-12-10)

**New features**:

  - Add pg_stat_kcache.track option, similar to pg_stat_statements (Julien
    Rouhaud and github user mikecaat)
  - Add pg_stat_kcache.track_planning, similar to pg_stat_statements, to
    track usge during planning, and maintain 2 sets of counters, depending on
    whether it was during planning or execution.  Requires PostgreSQL 13 or
    above (Julien Rouhaud, github user mikecaat)
  - Add a new "top" column, and accumulate resource usage in different entries
    depending on whether queries are executed at top level or at nested
    statements. (github user mikecaat and Julien Rouhaud)

## 2.1.3 (2020-07-16)

**Bugfix**:

  - Fix memory corruption introduced in v2.1.2 that can cause a PostgreSQL
    crash (Julien Rouhaud, thanks to github user tbe, Nikolay Samokhvalov and
    Andreas Seltenreich for reporting the issue and the extensive checking)

## 2.1.2 (2020-07-08)

**Bugfix**:

  - Accumulate counters for parallel workers too (Julien Rouhaud, thanks to
    Atsushi Torikoshi for the report)

## 2.1.1 (2018-07-28)

**Bugfix**:

  - Fix usage increase, used to keep the most frequent entries in memory
    (Julien Rouhaud)

**Miscellaneous**:

  - Allow PG_CONFIG value to be found on command-line (edechaux)
  - Warn users about incorrect GUC (Julien Rouhaud)
  - Add debian packaging (Julien Rouhaud)

## 2.1.0 (2018-07-17)

**NOTE**: This version requires a change to the on-disk format.  After
installing the new version restarting PostgreSQL, any previously accumulated
statistics will be reset.

  - Add support for architecture that don't provide getrusage(2), such as
    windows.  Only user time and system time will be available on such
    platforms (Julien Rouhaud).
  - Expose more fields of getrusage(2).  Depending on the platform, some of
    these fields are not maintained (Julien Rouhaud).
  - Add a workaround for sampling problems with getrusage(), new parameter
    pg_stat_kcache.linux_hz is added.  By default, this parameter is discovered
    at server startup (Ronan Dunklau).
  - Add compatibility with PostgreSQL 11 (Thomas Reiss)
  - Fix issue when concurrently created entries for the same user, db and
    queryid could lost some execution counters (Mael Rimbault)
  - Do not install docs anymore (Ronan Dunklau)

## 2.0.3 (2016-10-03)
  - Add PG 9.6 compatibility
  - Fix issues in shared memory estimation, which could prevent starting
    postgres or reduce the amount of possible locks (thanks to Jean-Sébastien
    BACQ for the report)
  - Add hint of possible reasons pgss.max could not be retrieved, which could
    prevent starting postgres

## 2.0.2 (2015-03-17)

  - Fix another bug with 32 bits builds (thanks to Alain Delorme for reporting it)

## 2.0.1 (2015-03-16)

  - Fix a bug with 32 bits builds (thanks to Alain Delorme for reporting it)

## 2.0 (2015-01-30)

  - Handle stats per database, user, query

## 1.0 (2014-02-26)

  - Initial release
