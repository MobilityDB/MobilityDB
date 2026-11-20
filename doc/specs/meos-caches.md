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
| Version | v1.1.0 |
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
5. **The time zone a thread names is the zone of the program.** A thread
   naming none takes the zone the program named last, by whichever thread,
   and the zone of the machine while the program names none. Every function
   reading the zone of the thread takes it first (`meos_ensure_timezone`), so
   such a thread follows each later naming from its next such call, whether
   it reads or writes a timestamp or computes with one, as adding an interval
   or truncating to a unit does; a thread naming its own zone keeps it.

6. **A per-thread variable read in a hot path is read once per call.**
   Each reading of a per-thread variable in the standalone library loaded at
   run time, as the bindings load it, is one lookup of the thread
   (`__tls_get_addr`). A function reading one in a loop copies its value into
   a local once and passes it to the functions it calls for each step, as
   the clipping of a temporal point passes its arrays of events, intervals
   and meets to the functions computing the intervals of each segment. The
   address of a per-thread variable is a constant to the compiler, which
   reads it again wherever it is used, inlined copies and specialized clones
   included, so a variable whose address is passed is read through
   `MEOS_TLS_ADDRESS` of `meos/include/meos_tls.h`, a function the compiler
   does not inline, as the broken-down time of `pgtypes/timezone/localtime.c`
   is.

A program using MEOS from several threads therefore calls `meos_initialize`
once for the process, then, on each thread before its first MEOS call,
`meos_initialize_collation`, and `meos_initialize_timezone` where the thread
names a zone of its own, and on each thread before it ends, `meos_finalize`.

## 4. The caches

| Cache | Variables | Holds | Filled by | Emptied when | Freed by |
|---|---|---|---|---|---|
| Time zones | `timezone_cache`, `session_timezone`, `meos_thread_tz_named`, `meos_thread_tz_gen` (`pgtypes/timezone/pgtz.c`) | the time zones loaded from the zone database, the current zone of the thread, whether the thread named it, and the naming of the program it took otherwise | `meos_initialize_timezone`, a time zone named in a value | — | `meos_finalize_timezone` |
| GMT zone | `gmtptr` (`pgtypes/timezone/localtime.c`) | the state of the GMT zone | the first conversion through `pg_gmtime`, as `epoch` reads | — | `meos_finalize_timezone` |
| Time zone directory | `tzdir`, `done_tzdir` (`pgtypes/timezone/pgtz.c`) | the directory of the zone database, where the build states none | the first zone loaded | — | holds no memory |
| Date and time keywords | `datecache`, `deltacache` (`pgtypes/utils/datetime.c`) | the keyword last read in each field of a date, time or interval | parsing a date, time or interval | — | holds no memory |
| Time zone abbreviations | `tzabbrevcache` (`pgtypes/utils/datetime.c`) | the abbreviation last read in each field, and its zone | parsing a time zone abbreviation | the current time zone is set or freed | holds no memory |
| Current time | `cache_ts`, `cache_tm`, `cache_fsec`, `cache_tz`, `cache_timezone` (`pgtypes/utils/datetime.c`) | the current instant broken down under the current time zone | the keywords of the current instant and of `today`, `tomorrow`, `yesterday` | the current time zone is set or freed | holds no memory |
| Default collation | `default_locale` (`pgtypes/utils/pg_locale.c`) | the locale of the default collation, `C.UTF-8` | `meos_initialize_collation` | — | `meos_finalize_collation` |
| Other collations | `CollationCache`, `last_collation_cache_oid`, `last_collation_cache_locale` (`pgtypes/utils/pg_locale.c`) | the locale of each collation other than the default | a collation other than the default, which no function of MEOS passes | — | — |
| Spatial reference systems | `MEOS_PROJ_CACHE`, `MEOS_PROJ_CACHE_PATH` (`meos/src/geo/tspatial_transform_meos.c`, `meos/src/geo/tspatial_transform.c`) | the PROJ transformations and the systems read from `spatial_ref_sys.csv` with their names in GeoJSON and MF-JSON, and the path of that file | a transformation, a geodetic operation, a GeoJSON or MF-JSON value written | the next transformation after `meos_set_spatial_ref_sys_csv` names another file | `meos_finalize_projsrs` |
| PROJ context | `MEOS_PJ_CONTEXT` (`meos/src/temporal/meos.c`) | the PROJ context of the thread | the first transformation | — | `meos_finalize` |
| Spatial relationships | `MEOS_RELATE_CTX_CACHE` (`meos/src/geo/geo_funcs.c`) | the edges of the geometries a relationship is asked about | a spatial relationship | — | `meos_finalize_relate` |
| Ways | `MEOS_WAYS_CACHE`, `MEOS_WAYS_CACHE_PATH`, `MEOS_WAYS_INDEX` (`meos/src/npoint/ways_meos.c`) | the routes read from the ways file, the path of that file, and their index | a network point turned into a geometry | the next network point turned into a geometry after `meos_set_ways_csv` names another file | `meos_finalize_ways` |
| Point cloud schemas | `cache_buf`, `cache_count`, `cache_cap`, `schemas_xml_read` (`meos/src/pointcloud/schema_hook.c`) | the schema of each pcid, registered or read from the schemas document | `meos_pc_schema_register*`, a lookup the document answers | `meos_pc_schema_clear` | `meos_finalize` |
| Format pictures | `DCHCache`, `n_DCHCache`, `DCHCounter`, `NUMCache`, `n_NUMCache`, `NUMCounter` (`pgtypes/utils/formatting.c`) | the parsed date/time and number format pictures, 20 of each | a date/time template, as the `.datetime(template)` method of a JSON path reads | — | `meos_finalize` |
| Default date/time formats | `fmt_txt` (`pgtypes/utils/jsonpath_exec.c`) | the ISO formats the `.datetime()` method of a JSON path tries | the `.datetime()` method of a JSON path | — | `meos_finalize` |
| Regular expressions | `re_array`, `num_res` (`pgtypes/utils/regexp.c`) | the compiled regular expressions, 32, the oldest dropped and freed first | `like_regex` in a JSON path | — | `meos_finalize` |
| Character classes | `pg_ctype_cache_list` (`pgtypes/regex/regc_pg_locale.c`) | the characters of each class a regular expression names, per collation | a regular expression naming a class, as `[[:alpha:]]` | — | `meos_finalize` |
| JSON scratch | `JSON_TO_FREE`, `JSON_TO_FREE_CONTEXT` (`pgtypes/utils/jsonb_util.c`) | the memory a JSON parse leaves to free | a JSON parse | — | `meos_finalize` |
| Random generators | `MEOS_PRNG_INITIALIZED`, `MEOS_GENERATION_RNG`, `MEOS_AGGREGATION_RNG` (`meos/src/temporal/meos.c`) | the generators of the data generator and of the sampling aggregates | the first random draw | — | `meos_finalize` |
| S2 corners | `s2_uv_cache`, `s2_vertex_cache`, `s2_lon_cache` (`meos/src/s2cell/s2cell.c`) | the cube coordinates, the degrees and the longitudes of the corners read last, 1024 of each, by their leaf indices | a corner of an S2 cell read (`s2cell_corner_uv`, `s2cell_corner_lonlat`, `s2cell_corner_lon`) | — | holds no memory |
| S2 outlines | `s2_last_outline`, `s2_last_outline_cell`, `s2_last_geo_outline`, `s2_last_geo_outline_cell` (`meos/src/s2cell/s2cell.c`) | the planar and the geodetic outline of the last S2 cell read | `s2cell_cell_outline_last`, `s2cell_cell_outline_geodetic_last` | — | holds no memory |
| QUADBIN rows | `quadbin_lat_cache` (`meos/src/quadbin/quadbin.c`) | the latitudes of the row boundaries read last, 1024, by the bits of their row | a row boundary of a QUADBIN cell read (`quadbin_row_latitude`) | — | holds no memory |
| H3 outline | `h3_last_outline`, `h3_last_outline_cell` (`meos/src/h3/h3_geo.c`) | the outline of the last H3 cell read | `h3_cell_outline_last` | — | holds no memory |
| H3 poles | `MEOS_H3_POLE_CELL`, `MEOS_H3_POLE_CELL_SET` (`meos/src/h3/th3index_boxops.c`) | the cells holding the two poles at each resolution | the first box of a cell of that resolution (`h3index_pole_cells`) | — | holds no memory |
| H3 box | `MEOS_H3_BOX`, `MEOS_H3_BOX_CELL` (`meos/src/h3/th3index_boxops.c`) | the box of the last H3 cell a temporal cell value is bounded by | `th3index_cell_set_stbox` | — | holds no memory |
| Cell positions | `dggs_vec_cache` (`meos/src/temporal/tcellindex.c`) | the geodetic positions of the degrees read last, 512, by the bits of their degrees | the geodetic walk of a cell (`dggs_lonlat_vector`) | — | holds no memory |

`meos_finalize` calls `meos_finalize_timezone`, `meos_finalize_projsrs`,
`meos_finalize_relate`, `meos_finalize_collation` and, in a build carrying
network points, `meos_finalize_ways`, then frees the caches the table names
`meos_finalize` for. A program may call one of the `meos_finalize_*` functions
alone to free one cache, which the next lookup fills again.

### State that is per thread and is not a cache

The following variables are per thread and keep nothing a later call reads
as a cache:

- **The error state**: the number of the last error (`MEOS_ERR_NO`) and the
  flag a parse saving its errors sets (`MEOS_ERROR_SAVE`), in
  `meos/src/temporal/meos_error.c`.
- **The result of the WKT parser** (`global_parser_result`,
  `postgis/liblwgeom/lwin_wkt.h`), which one parse fills and reads.
- **The arrays of one clipping** (`events`, `intervals`, `periods`,
  `rtree_results`, `meets` in `meos/src/geo/tpoint_geom_clip.c`), created and
  destroyed by the call that fills them.
- **The candidates of a relationship kernel** (`dist_pip_results`,
  `meos/src/geo/tgeo_distance.c`), created and destroyed with the R-tree of
  the geometry context they are read against.
- **The buffers of the time zone and locale code taken from PostgreSQL**:
  the broken-down time `tm` of `pgtypes/timezone/localtime.c`; the zone
  directory `tzdirpath`, the zone `tz` and the name `resultbuf` that the probe
  of the system zone fills in `pgtypes/timezone/findtimezone.c`; the strategy
  `pg_regex_strategy` and locale `pg_regex_locale` of the regular expressions
  of `pgtypes/regex/regc_pg_locale.c`; the locale conventions
  `CurrentLocaleConv`, `CurrentLocaleConvValid`, `CurrentLocaleConvAllocated`
  and `CurrentLCTimeValid`, and the locale names `save_lc_ctype` and
  `iso_lc_messages`, of `pgtypes/utils/pg_locale.c`; and whether the thread
  holds the table of abbreviations (`tznames_held`,
  `pgtypes/timezone/tznames_default.c`).

## 5. State that is per process

The following state is shared by the threads of a process and is not a cache:
it is set once, or it is a table no call changes.

- **Settings**: the allocator (`meos_initialize_allocator`), the error and
  interrupt handlers, the paths of `spatial_ref_sys.csv`, of the ways file and
  of the point cloud schemas document, the date and interval styles, and the
  hooks a host installs to look point cloud schemas up.
- **The default time zone of the system**, probed once for the process, since
  the C library keeps it per process.
- **The time zone the program names**, the last named by any thread, with
  the count of its namings, under its own lock (`meos_program_tz_name`,
  `meos_program_tz_gen` in `pgtypes/timezone/pgtz.c`).
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

## 7. The spatial reference systems in the PostgreSQL extension

The MobilityDB extension asks the same cache of spatial reference systems as
MEOS does, through the same functions: a transformation, a geodetic
operation, and the coordinate reference system that `asMFJSON` names. It
holds no second cache of them. In the extension the cache belongs to the
backend: it lies in a child of the `CacheMemoryContext`, as the ways cache
of `meos/src/npoint/ways.c` does, its systems are read from the table
`spatial_ref_sys`, and a reset of that context destroys its PROJ objects and
forgets it, the next lookup filling it again.

An SRID that the systems read do not hold is refused alike in both builds,
with the message `Cannot find SRID (<srid>) in spatial_ref_sys`.

## 8. Out of scope

The caches of the PostgreSQL extension other than those listed, which live in
the memory contexts of the backend, and the caches of the libraries MEOS
links, such as PROJ's own, which MEOS reaches only through the PROJ context of
each thread.

## 9. References

- `meos/include/meos_tls.h`: the `MEOS_TLS` qualifier.
- `meos/src/temporal/meos.c`: `meos_initialize` and `meos_finalize`.
- `meos/test/threaded_test.c`: the program the MEOS workflow runs under the
  thread sanitizer.
