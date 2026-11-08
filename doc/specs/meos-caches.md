<!--
   ****************************************************************************
    MobilityDB / MEOS Specification — Caches
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->

# Caches — Lifecycle Specification

| | |
|---|---|
| Version | v1.0.0 |
| Scope | The data MEOS keeps between two calls, the thread it belongs to, and what frees it |
| Reference | `meos/src/temporal/meos.c` (`meos_initialize`, `meos_finalize`), `meos/include/meos_tls.h` (`MEOS_TLS`) |

## 1. Scope and intent

MEOS keeps the results of some lookups between two calls: the time zones it
has loaded, the spatial reference systems it has read, the format pictures it
has parsed, and so on. This document lists every such cache, states the
thread each one belongs to and the call that frees it, and states the rules a
program follows to use MEOS from several threads. It is prescriptive: a cache
added to MEOS follows the same rules.

## 2. Notation

*Per thread* means that the variable holding the cache is declared with
`MEOS_TLS`, the thread-local storage qualifier of `meos/include/meos_tls.h`,
so that each thread of the standalone library has its own copy. In the
MobilityDB extension `MEOS_TLS` expands to nothing, since a PostgreSQL backend
is a single-threaded process, and the cache belongs to the backend.

*Freed by* names the call that releases the memory the cache holds and
empties it. Every such call is made by `meos_finalize`.

## 3. The rules

1. **Every cache is per thread.** A thread fills its own caches and reads only
   its own: what one thread loads, parses or registers is invisible to the
   others, and no lock is taken on a lookup.
2. **`meos_finalize` frees every cache of the calling thread.** After it, the
   thread holds no memory MEOS allocated for a cache. A thread that ends
   without calling it leaves its caches allocated, since the storage of a
   thread-local variable is released with the thread but what it points to is
   not.
3. **A cache keyed on a setting is emptied where the setting changes.** The
   time zone lookups are read under the current time zone of the thread, so
   setting the time zone empties them; a later lookup fills them again under the new zone.
4. **Settings and handlers are per process.** The allocator, the error
   handler, the paths of the data files and the date and interval styles are
   set once for the process, and every thread reads them.

A program using MEOS from several threads therefore calls `meos_initialize`
once for the process, then, on each thread before its first MEOS call,
`meos_initialize_timezone` and `meos_initialize_collation`, and on each thread
before it ends, `meos_finalize`.

## 4. The caches

| Cache | Variables | Holds | Filled by | Emptied when | Freed by |
|---|---|---|---|---|---|
| Time zones | `timezone_cache`, `session_timezone` (`pgtypes/timezone/pgtz.c`) | the time zones loaded from the zone database, and the current zone of the thread | `meos_initialize_timezone`, a time zone named in a value | — | `meos_finalize_timezone` |
| GMT zone | `gmtptr` (`pgtypes/timezone/localtime.c`) | the state of the GMT zone | the first conversion through `pg_gmtime`, as `epoch` reads | — | `meos_finalize_timezone` |
| Time zone directory | `tzdir` (`pgtypes/timezone/pgtz.c`) | the directory of the zone database, where the build states none | the first zone loaded | — | holds no memory |
| Date and time keywords | `datecache`, `deltacache` (`pgtypes/utils/datetime.c`) | the keyword last read in each field of a date, time or interval | parsing a date, time or interval | — | holds no memory |
| Time zone abbreviations | `tzabbrevcache` (`pgtypes/utils/datetime.c`) | the abbreviation last read in each field, and its zone | parsing a time zone abbreviation | the current time zone is set or freed | holds no memory |
| Current time | `cache_ts`, `cache_tm`, ... (`pgtypes/utils/datetime.c`) | the current instant broken down under the current time zone | the keywords of the current instant and of `today`, `tomorrow`, `yesterday` | the current time zone is set or freed | holds no memory |
| Default collation | `default_locale` (`pgtypes/utils/pg_locale.c`) | the locale of the default collation, `C.UTF-8` | `meos_initialize_collation` | — | `meos_finalize_collation` |
| Other collations | `CollationCache` (`pgtypes/utils/pg_locale.c`) | the locale of each collation other than the default | a collation other than the default, which no function of MEOS passes | — | — |
| Spatial reference systems | `MEOS_PROJ_CACHE`, `MEOS_PROJ_CACHE_PATH` (`meos/src/geo/tspatial_transform_meos.c`) | the PROJ transformations and the systems read from `spatial_ref_sys.csv`, and the path of that file | a transformation | the next transformation after `meos_set_spatial_ref_sys_csv` names another file | `meos_finalize_projsrs` |
| PROJ context | `MEOS_PJ_CONTEXT` (`meos/src/temporal/meos.c`) | the PROJ context of the thread | the first transformation | — | `meos_finalize` |
| Spatial relationships | `MEOS_RELATE_CTX_CACHE` (`meos/src/geo/geo_funcs.c`) | the edges of the geometries a relationship is asked about | a spatial relationship | — | `meos_finalize_relate` |
| Ways | `MEOS_WAYS_CACHE`, `MEOS_WAYS_CACHE_PATH`, `MEOS_WAYS_INDEX` (`meos/src/npoint/ways_meos.c`) | the routes read from the ways file, the path of that file, and their index | a network point turned into a geometry | the next network point turned into a geometry after `meos_set_ways_csv` names another file | `meos_finalize_ways` |
| Point cloud schemas | `cache_buf` (`meos/src/pointcloud/schema_hook.c`) | the schema of each pcid, registered or read from the schemas document | `meos_pc_schema_register*`, a lookup the document answers | `meos_pc_schema_clear` | `meos_finalize` |
| Format pictures | `DCHCache`, `NUMCache` (`pgtypes/utils/formatting.c`) | the parsed date/time and number format pictures, 20 of each | a date/time template, as the `.datetime(template)` method of a JSON path reads | — | `meos_finalize` |
| Default date/time formats | `fmt_txt` (`pgtypes/utils/jsonpath_exec.c`) | the ISO formats the `.datetime()` method of a JSON path tries | the `.datetime()` method of a JSON path | — | `meos_finalize` |
| Regular expressions | `re_array` (`pgtypes/utils/regexp.c`) | the compiled regular expressions, 32, the oldest dropped and freed first | `like_regex` in a JSON path | — | `meos_finalize` |
| Character classes | `pg_ctype_cache_list` (`pgtypes/regex/regc_pg_locale.c`) | the characters of each class a regular expression names, per collation | a regular expression naming a class, as `[[:alpha:]]` | — | `meos_finalize` |
| JSON scratch | `JSON_TO_FREE` (`pgtypes/utils/jsonb_util.c`) | the memory a JSON parse leaves to free | a JSON parse | — | `meos_finalize` |
| Random generators | `MEOS_GENERATION_RNG`, `MEOS_AGGREGATION_RNG` (`meos/src/temporal/meos.c`) | the generators of the data generator and of the sampling aggregates | the first random draw | — | `meos_finalize` |

`meos_finalize` calls `meos_finalize_timezone`, `meos_finalize_projsrs`,
`meos_finalize_relate`, `meos_finalize_collation` and, in a build carrying
network points, `meos_finalize_ways`, then frees the caches the table names
`meos_finalize` for. A program may call one of the `meos_finalize_*` functions
alone to free one cache, which the next lookup fills again.

## 5. State that is per process

The following state is shared by the threads of a process and is not a cache:
it is set once, or it is a table no call changes.

- **Settings**: the allocator (`meos_initialize_allocator`), the error and
  interrupt handlers, the paths of `spatial_ref_sys.csv`, of the ways file and
  of the point cloud schemas document, the date and interval styles, and the
  hooks a host installs to look point cloud schemas up.
- **The default time zone of the system**, probed once for the process, since
  the C library keeps it per process.
- **The time zone abbreviation table** of `pgtypes/timezone/tznames_default.c`,
  installed by the first thread setting a time zone and released by the last
  thread freeing it, under its own lock.
- **Constant tables and constants computed once**, such as the degree
  constants of `pgtypes/utils/float.c`.

## 6. Point cloud schemas on several threads

Since the schema cache is per thread, a schema registered on one thread is
found on that thread only. A program reading point cloud values on several
threads registers its schemas on each thread, or names the schemas document
with `meos_set_pointcloud_schemas_xml` once for the process, which each thread
then reads into its own cache on its first lookup. Reading a value needs no
schema; only an operation decoding a coordinate looks the schema of its pcid
up.

## 7. Out of scope

The caches of the PostgreSQL extension other than those listed, which live in
the memory contexts of the backend, and the caches of the libraries MEOS
links, such as PROJ's own, which MEOS reaches only through the PROJ context of
each thread.

## 8. References

- `meos/include/meos_tls.h`: the `MEOS_TLS` qualifier.
- `meos/src/temporal/meos.c`: `meos_initialize` and `meos_finalize`.
- `meos/test/threaded_test.c`: the program the MEOS workflow runs under the
  thread sanitizer.
