pg_stat_kcache
==============

Features
--------

Gathers statistics about real reads and writes done by the filesystem layer.
It is provided in the form of an extension for PostgreSQL >= 9.4., and requires
pg_stat_statements extension to be installed. PostgreSQL 9.4 or more is
required as previous version of provided pg_stat_statements didn't expose the
queryid field.

Installation
============

From PGDG repositories
----------------------

If you installed PostgreSQL from the PGDG repositories (either `APT on
Debian/Ubuntu <https://apt.postgresql.org>` or `YUM on RHEL/Rocky
<https://yum.postgresql.org`), the recommended way to install pg_stat_kcache is
to get it from the same repositories.

For Debian/Ubuntu::

  apt install postgresql-XY-pg-stat-kcache

and RHEL/Rocky::

  yum install pg_stat_kcacheXY

or for PostgreSQL 11 and above::

  yum install pg_stat_kcache_XY

(where XY is your major PostgreSQL version)

Compiling
---------

The module can be built using the standard PGXS infrastructure. For this to
work, the ``pg_config`` program must be available in your $PATH. Instruction to
install follows::

 git clone https://github.com/powa-team/pg_stat_kcache.git
 cd pg_stat_kcache
 make
 make install

PostgreSQL setup
----------------

The extension is now available. But, as it requires some shared memory to hold
its counters, the module must be loaded at PostgreSQL startup. Thus, you must
add the module to ``shared_preload_libraries`` in your ``postgresql.conf``. You
need a server restart to take the change into account.  As this extension
depends on pg_stat_statements, it also need to be added to
``shared_preload_libraries``.

Add the following parameters into you ``postgresql.conf``::

 # postgresql.conf
 shared_preload_libraries = 'pg_stat_statements,pg_stat_kcache'

Once your PostgreSQL cluster is restarted, you can install the extension in
every database where you need to access the statistics::

 mydb=# CREATE EXTENSION pg_stat_kcache;

Configuration
=============

The following GUCs can be configured, in ``postgresql.conf``:

- *pg_stat_kcache.linux_hz* (int, default -1): informs pg_stat_kcache of the
  linux CONFIG_HZ config option. This is used by pg_stat_kcache to compensate
  for sampling errors. The default value is -1, tries to guess it at startup.
- *pg_stat_kcache.track* (enum, default top): controls which statements are
  tracked by pg_stat_kcache. Specify top to track top-level statements (those
  issued directly by clients), all to also track nested statements (such as
  statements invoked within functions), or none to disable statement statistics
  collection.
- *pg_stat_kcache.track_planning* (bool, default off): controls whether
  planning operations and duration are tracked by pg_stat_kcache (requires
  PostgreSQL 13 or above).

Usage
=====

pg_stat_kcache create several objects.

pg_stat_kcache view
-------------------

+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
|       Name       |       Type       |                                                                       Description                                                                       |
+==================+==================+=========================================================================================================================================================+
| datname          | name             | Name of the database                                                                                                                                    |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_user_time   | double precision | User CPU time used planning statements in this database, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)      |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_system_time | double precision | System CPU time used planning  statements in this database, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)   |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_minflts     | bigint           | Number of page reclaims (soft page faults) planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)          |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_majflts     | bigint           | Number of page faults (hard page faults) planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)            |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nswaps      | bigint           | Number of swaps planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)                                     |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads       | bigint           | Number of bytes read by the filesystem layer planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)        |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)    |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes      | bigint           | Number of bytes written by the filesystem layer planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)     |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero) |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgsnds     | bigint           | Number of IPC messages sent planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)                         |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgrcvs     | bigint           | Number of IPC messages received planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)                     |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nsignals    | bigint           | Number of signals received planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)                          |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nvcsws      | bigint           | Number of voluntary context switches planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)                |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nivcsws     | bigint           | Number of involuntary context switches planning  statements in this database (if pg_stat_kcache.track_planning is enabled, otherwise zero)              |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_user_time   | double precision | User CPU time used executing  statements in this database, in seconds and milliseconds                                                                  |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_system_time | double precision | System CPU time used executing  statements in this database, in seconds and milliseconds                                                                |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_minflts     | bigint           | Number of page reclaims (soft page faults) executing statements in this database                                                                        |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_majflts     | bigint           | Number of page faults (hard page faults) executing statements in this database                                                                          |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nswaps      | bigint           | Number of swaps executing statements in this database                                                                                                   |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads       | bigint           | Number of bytes read by the filesystem layer executing statements in this database                                                                      |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer executing statements in this database                                                                  |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes      | bigint           | Number of bytes written by the filesystem layer executing statements in this database                                                                   |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer executing statements in this database                                                               |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgsnds     | bigint           | Number of IPC messages sent executing statements in this database                                                                                       |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgrcvs     | bigint           | Number of IPC messages received executing statements in this database                                                                                   |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nsignals    | bigint           | Number of signals received executing statements in this database                                                                                        |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nvcsws      | bigint           | Number of voluntary context switches executing statements in this database                                                                              |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nivcsws     | bigint           | Number of involuntary context switches executing statements in this database                                                                            |
+------------------+------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+

pg_stat_kcache_detail view
--------------------------

+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
|       Name       |       Type       |                                                               Description                                                                |
+==================+==================+==========================================================================================================================================+
| query            | text             | Query text                                                                                                                               |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| top              | bool             | True if the statement is top-level                                                                                                       |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| datname          | name             | Database name                                                                                                                            |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| rolname          | name             | Role name                                                                                                                                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_user_time   | double precision | User CPU time used planning the statement, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_system_time | double precision | System CPU time used planning the statement, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)   |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_minflts     | bigint           | Number of page reclaims (soft page faults) planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_majflts     | bigint           | Number of page faults (hard page faults) planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)            |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nswaps      | bigint           | Number of swaps planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads       | bigint           | Number of bytes read by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)        |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)    |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes      | bigint           | Number of bytes written by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero) |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgsnds     | bigint           | Number of IPC messages sent planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                         |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgrcvs     | bigint           | Number of IPC messages received planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nsignals    | bigint           | Number of signals received planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nvcsws      | bigint           | Number of voluntary context switches planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nivcsws     | bigint           | Number of involuntary context switches planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)              |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_user_time   | double precision | User CPU time used executing the statement, in seconds and milliseconds                                                                  |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_system_time | double precision | System CPU time used executing the statement, in seconds and milliseconds                                                                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_minflts     | bigint           | Number of page reclaims (soft page faults) executing the statements                                                                      |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_majflts     | bigint           | Number of page faults (hard page faults) executing the statements                                                                        |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nswaps      | bigint           | Number of swaps executing the statements                                                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads       | bigint           | Number of bytes read by the filesystem layer executing the statements                                                                    |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer executing the statements                                                                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes      | bigint           | Number of bytes written by the filesystem layer executing the statements                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer executing the statements                                                             |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgsnds     | bigint           | Number of IPC messages sent executing the statements                                                                                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgrcvs     | bigint           | Number of IPC messages received executing the statements                                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nsignals    | bigint           | Number of signals received executing the statements                                                                                      |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nvcsws      | bigint           | Number of voluntary context switches executing the statements                                                                            |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nivcsws     | bigint           | Number of involuntary context switches executing the statements                                                                          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+

pg_stat_kcache_reset function
-----------------------------

Resets the statistics gathered by pg_stat_kcache. Can be called by superusers::

 pg_stat_kcache_reset()


pg_stat_kcache function
-----------------------

This function is a set-returning functions that dumps the containt of the counters
of the shared memory structure. This function is used by the pg_stat_kcache view.
The function can be called by any user::

 SELECT * FROM pg_stat_kcache();

It provides the following columns:

+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
|       Name       |       Type       |                                                               Description                                                                |
+==================+==================+==========================================================================================================================================+
| queryid          | bigint           | pg_stat_statements' query identifier                                                                                                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| top              | bool             | True if the statement is top-level                                                                                                       |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| userid           | oid              | Database OID                                                                                                                             |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| dbid             | oid              | Database OID                                                                                                                             |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_user_time   | double precision | User CPU time used planning the statement, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_system_time | double precision | System CPU time used planning the statement, in seconds and milliseconds (if pg_stat_kcache.track_planning is enabled, otherwise zero)   |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_minflts     | bigint           | Number of page reclaims (soft page faults) planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_majflts     | bigint           | Number of page faults (hard page faults) planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)            |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nswaps      | bigint           | Number of swaps planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads       | bigint           | Number of bytes read by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)        |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)    |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes      | bigint           | Number of bytes written by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero) |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgsnds     | bigint           | Number of IPC messages sent planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                         |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_msgrcvs     | bigint           | Number of IPC messages received planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nsignals    | bigint           | Number of signals received planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nvcsws      | bigint           | Number of voluntary context switches planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| plan_nivcsws     | bigint           | Number of involuntary context switches planning the statement (if pg_stat_kcache.track_planning is enabled, otherwise zero)              |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_user_time   | double precision | User CPU time used executing the statement, in seconds and milliseconds                                                                  |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_system_time | double precision | System CPU time used executing the statement, in seconds and milliseconds                                                                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_minflts     | bigint           | Number of page reclaims (soft page faults) executing the statements                                                                      |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_majflts     | bigint           | Number of page faults (hard page faults) executing the statements                                                                        |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nswaps      | bigint           | Number of swaps executing the statements                                                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads       | bigint           | Number of bytes read by the filesystem layer executing the statements                                                                    |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_reads_blks  | bigint           | Number of 8K blocks read by the filesystem layer executing the statements                                                                |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes      | bigint           | Number of bytes written by the filesystem layer executing the statements                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_writes_blks | bigint           | Number of 8K blocks written by the filesystem layer executing the statements                                                             |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgsnds     | bigint           | Number of IPC messages sent executing the statements                                                                                     |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_msgrcvs     | bigint           | Number of IPC messages received executing the statements                                                                                 |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nsignals    | bigint           | Number of signals received executing the statements                                                                                      |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nvcsws      | bigint           | Number of voluntary context switches executing the statements                                                                            |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+
| exec_nivcsws     | bigint           | Number of involuntary context switches executing the statements                                                                          |
+------------------+------------------+------------------------------------------------------------------------------------------------------------------------------------------+

Updating the extension
======================

Note that a PostgreSQL restart is required for changes other than than SQL
objects.  Most of the new code will be enabled as soon as the restart is done,
whether or not the extension is updated, as the extension only takes care of
exposing the internal data structure in SQL.

Please also note that when the set-returning function fields are changes, a
PostgreSQL restart is required to load the new version of the extension.  Until
the restart is done, updating the extension will fail with messages similar to:

could not find function "pg_stat_kcache_2_2" in file .../pg_stat_kcache.so

Bugs and limitations
====================

No known bugs.

Tracking planner resources usage requires PostgreSQL 13 or above.

We assume that a kernel block is 512 bytes. This is true for Linux, but may not
be the case for another Unix implementation.

See: http://lkml.indiana.edu/hypermail/linux/kernel/0703.2/0937.html

On platforms without a native getrusage(2), all fields except `user_time` and
`system_time` will be NULL.

On platforms with a native getrusage(2), some of the fields may not be
maintained.  This is a platform dependent behavior, please refer to your
platform getrusage(2) manual page for more details.

If *pg_stat_kcache.track* is all, pg_stat_kcache tracks nested statements.
The max number of nesting level that will be tracked is is limited to 64, in
order to keep implementation simple, but this should be enough for reasonable
use cases.

Even if *pg_stat_kcache.track* is all, pg_stat_kcache view considers only
statistics of top-level statements. So, there is the case which even though
user cpu time used planning a nested statement is high, `plan_user_time` of
pg_stat_kcache view is small. In such a case, user cpu time used planning a
nested statement is counted in `exec_user_time`.

Authors
=======

pg_stat_kcache is an original development from Thomas Reiss, with large
portions of code inspired from pg_stat_plans. Julien Rouhaud also contributed
some parts of the extension.

Thanks goes to Peter Geoghegan for providing much inspiration with
pg_stat_plans so we could write this extension quite straightforward.

License
=======

pg_stat_kcache is free software distributed under the PostgreSQL license.

Copyright (c) 2014-2017, Dalibo
Copyright (c) 2018-2022, The PoWA-team

