/*---------------
 * pg_stat_kcache
 *
 * Provides basic statistics about real I/O done by the filesystem layer.
 * This way, calculating a real hit-ratio is doable.  Also provides basis
 * statistics about CPU usage.
 *
 * Large portions of code freely inspired by pg_stat_plans. Thanks to Peter
 * Geoghegan for this great extension.
 *
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *
 */

#include "postgres.h"

#include <unistd.h>

/*
 * pg16 removed the configure probe for getrusage. Simply define it for all
 * platforms except Windows to keep existing code backward compatible.
 */
#if PG_VERSION_NUM >= 160000
#ifndef WIN32
#define HAVE_GETRUSAGE
#endif		/* HAVE_GETRUSAGE */
#endif		/* pg16+ */

#if PG_VERSION_NUM < 160000
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif		/* HAVE_SYS_RESOURCE_H */

#ifndef HAVE_GETRUSAGE
#include "rusagestub.h"
#endif		/* !HAVE_GETRUSAGE */
#endif		/* pg16- */

#include "access/hash.h"
#if PG_VERSION_NUM >= 90600
#include "access/parallel.h"
#endif
#include "executor/executor.h"
#include "funcapi.h"
#include "miscadmin.h"
#if PG_VERSION_NUM >= 130000
#include "optimizer/planner.h"
#endif
#include "pgstat.h"
#if PG_VERSION_NUM >= 90600
#include "postmaster/autovacuum.h"
#endif
#if PG_VERSION_NUM >= 120000
#include "replication/walsender.h"
#endif
#include "storage/fd.h"
#include "storage/ipc.h"
#include "storage/spin.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#if PG_VERSION_NUM >= 160000
#include "utils/pg_rusage.h"
#endif
#include "utils/timestamp.h"

#include "pg_stat_kcache.h"

PG_MODULE_MAGIC;

#if PG_VERSION_NUM >= 90300
#define PGSK_DUMP_FILE		"pg_stat/pg_stat_kcache.stat"
#else
#define PGSK_DUMP_FILE		"global/pg_stat_kcache.stat"
#endif

/* In PostgreSQL 11, queryid becomes a uint64 internally.
 */
#if PG_VERSION_NUM >= 110000
typedef uint64 pgsk_queryid;
#else
typedef uint32 pgsk_queryid;
#endif

#define USAGE_INCREASE			(1.0)
#define USAGE_DECREASE_FACTOR	(0.99)	/* decreased every pgsk_entry_dealloc */
#define USAGE_DEALLOC_PERCENT	5		/* free this % of entries at once */
#define USAGE_INIT				(1.0)	/* including initial planning */

#define TIMEVAL_DIFF(start, end) ((double) end.tv_sec + (double) end.tv_usec / 1000000.0) \
	- ((double) start.tv_sec + (double) start.tv_usec / 1000000.0)

#if PG_VERSION_NUM < 170000
#define MyProcNumber MyBackendId
#define ParallelLeaderProcNumber ParallelLeaderBackendId
#endif

#if PG_VERSION_NUM < 140000
#define ParallelLeaderBackendId ParallelMasterBackendId
#endif

#define PGSK_MAX_NESTED_LEVEL		64

/*
 * Extension version number, for supporting older extension versions' objects
 */
typedef enum pgskVersion
{
	PGSK_V2_0 = 0,
	PGSK_V2_1,
	PGSK_V2_2,
	PGSK_V2_3
} pgskVersion;

/* Magic number identifying the stats file format */
static const uint32 PGSK_FILE_HEADER = 0x20240914;

static struct	rusage exec_rusage_start[PGSK_MAX_NESTED_LEVEL];
#if PG_VERSION_NUM >= 130000
static struct	rusage plan_rusage_start[PGSK_MAX_NESTED_LEVEL];
#endif

static int	pgsk_max = 0;	/* max #queries to store. pg_stat_statements.max is used */

/*
 * Hashtable key that defines the identity of a hashtable entry.  We use the
 * same hash as pg_stat_statements
 */
typedef struct pgskHashKey
{
	Oid			userid;			/* user OID */
	Oid			dbid;			/* database OID */
	pgsk_queryid		queryid;		/* query identifier */
	bool		top;		/* whether statement is top level */
} pgskHashKey;

/*
 * Statistics per database
 */
typedef struct pgskEntry
{
	pgskHashKey		key;		/* hash key of entry - MUST BE FIRST */
	pgskCounters	counters[PGSK_NUMKIND];	/* statistics for this query */
	slock_t			mutex;		/* protects the counters only */
	TimestampTz		stats_since; /* timestamp of entry allocation */
} pgskEntry;

/*
 * Global shared state
 */
typedef struct pgskSharedState
{
	LWLock	   *lock;					/* protects hashtable search/modification */
#if PG_VERSION_NUM >= 90600
	pgsk_queryid	queryids[FLEXIBLE_ARRAY_MEMBER]; /* queryid info for
														parallel leaders */
#endif
} pgskSharedState;

/*---- Local variables ----*/

/* Current nesting depth of planner/ExecutorRun/ProcessUtility calls */
static int	nesting_level = 0;


/* saved hook address in case of unload */
#if PG_VERSION_NUM >= 150000
static shmem_request_hook_type prev_shmem_request_hook = NULL;
#endif
static shmem_startup_hook_type prev_shmem_startup_hook = NULL;
#if PG_VERSION_NUM >= 130000
static planner_hook_type prev_planner_hook = NULL;
#endif
static ExecutorStart_hook_type prev_ExecutorStart = NULL;
static ExecutorRun_hook_type prev_ExecutorRun = NULL;
static ExecutorFinish_hook_type prev_ExecutorFinish = NULL;
static ExecutorEnd_hook_type prev_ExecutorEnd = NULL;

/* Links to shared memory state */
static pgskSharedState *pgsk = NULL;
static HTAB *pgsk_hash = NULL;

/*---- HOOK variables ----*/

pgsk_counters_hook_type pgsk_counters_hook = NULL;

/*---- GUC variables ----*/

typedef enum
{
	PGSK_TRACK_NONE,			/* track no statements */
	PGSK_TRACK_TOP,				/* only top level statements */
	PGSK_TRACK_ALL				/* all statements, including nested ones */
}			PGSKTrackLevel;

static const struct config_enum_entry pgs_track_options[] =
{
	{"none", PGSK_TRACK_NONE, false},
	{"top", PGSK_TRACK_TOP, false},
	{"all", PGSK_TRACK_ALL, false},
	{NULL, 0, false}
};

static int	pgsk_track = PGSK_TRACK_TOP;	/* tracking level */
#if PG_VERSION_NUM >= 130000
static bool pgsk_track_planning = false;	/* whether to track planning duration */
#endif

#define pgsk_enabled(level) \
	((pgsk_track == PGSK_TRACK_ALL && (level) < PGSK_MAX_NESTED_LEVEL) || \
	(pgsk_track == PGSK_TRACK_TOP && (level) == 0))

/*--- Functions --- */

void	_PG_init(void);

extern PGDLLEXPORT Datum	pg_stat_kcache_reset(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum	pg_stat_kcache(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum	pg_stat_kcache_2_1(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum	pg_stat_kcache_2_2(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum	pg_stat_kcache_2_3(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(pg_stat_kcache_reset);
PG_FUNCTION_INFO_V1(pg_stat_kcache);
PG_FUNCTION_INFO_V1(pg_stat_kcache_2_1);
PG_FUNCTION_INFO_V1(pg_stat_kcache_2_2);
PG_FUNCTION_INFO_V1(pg_stat_kcache_2_3);

static void pg_stat_kcache_internal(FunctionCallInfo fcinfo, pgskVersion
		api_version);

static void pgsk_setmax(void);
static Size pgsk_memsize(void);

#if PG_VERSION_NUM >= 150000
static void pgsk_shmem_request(void);
#endif
static void pgsk_shmem_startup(void);
static void pgsk_shmem_shutdown(int code, Datum arg);
#if PG_VERSION_NUM >= 130000
static PlannedStmt *pgsk_planner(Query *parse,
								 const char *query_string,
								 int cursorOptions,
								 ParamListInfo boundParams);
#endif
static void pgsk_ExecutorStart(QueryDesc *queryDesc, int eflags);
static void pgsk_ExecutorRun(QueryDesc *queryDesc,
				 ScanDirection direction,
#if PG_VERSION_NUM >= 90600
				 uint64 count
#else
				 long count
#endif
#if PG_VERSION_NUM >= 100000
				 , bool execute_once
#endif
);
static void pgsk_ExecutorFinish(QueryDesc *queryDesc);
static void pgsk_ExecutorEnd(QueryDesc *queryDesc);
static pgskEntry *pgsk_entry_alloc(pgskHashKey *key);
static void pgsk_entry_dealloc(void);
static void pgsk_entry_reset(void);
static void pgsk_entry_store(pgsk_queryid queryId, pgskStoreKind kind,
							 pgskCounters counters);
static uint32 pgsk_hash_fn(const void *key, Size keysize);
static int	pgsk_match_fn(const void *key1, const void *key2, Size keysize);


static bool pgsk_assign_linux_hz_check_hook(int *newval, void **extra, GucSource source);
static void pgsk_compute_counters(pgskCounters *counters,
								  struct rusage *rusage_start,
								  struct rusage *rusage_end,
								  QueryDesc *queryDesc);
#if PG_VERSION_NUM >= 90600
static Size pgsk_queryids_array_size(void);
static void pgsk_set_queryid(pgsk_queryid queryid);
#endif

static int	pgsk_linux_hz = 0;

void
_PG_init(void)
{
	if (!process_shared_preload_libraries_in_progress)
	{
		elog(ERROR, "This module can only be loaded via shared_preload_libraries");
		return;
	}

	DefineCustomIntVariable("pg_stat_kcache.linux_hz",
				"Inform pg_stat_kcache of the linux CONFIG_HZ config option",
				"This is used by pg_stat_kcache to compensate for sampling errors "
				"in getrusage due to the kernel adhering to its ticks. The default value, -1, "
				"tries to guess it at startup. ",
							&pgsk_linux_hz,
							-1,
							-1,
							INT_MAX,
							PGC_USERSET,
							0,
							pgsk_assign_linux_hz_check_hook,
							NULL,
							NULL);

	DefineCustomEnumVariable("pg_stat_kcache.track",
							 "Selects which statements are tracked by pg_stat_kcache.",
							 NULL,
							 &pgsk_track,
							 PGSK_TRACK_TOP,
							 pgs_track_options,
							 PGC_SUSET,
							 0,
							 NULL,
							 NULL,
							 NULL);

#if PG_VERSION_NUM >= 130000
	DefineCustomBoolVariable("pg_stat_kcache.track_planning",
							 "Selects whether planning duration is tracked by pg_stat_cache.",
							 NULL,
							 &pgsk_track_planning,
							 false,
							 PGC_SUSET,
							 0,
							 NULL,
							 NULL,
							 NULL);
#endif

	EmitWarningsOnPlaceholders("pg_stat_kcache");

	/* set pgsk_max if needed */
	pgsk_setmax();
	/*
	 * If you change code here, don't forget to also report the modifications
	 * in pgsk_shmem_request() for pg15 and later.
	 */
#if PG_VERSION_NUM < 150000
	RequestAddinShmemSpace(pgsk_memsize());
#if PG_VERSION_NUM >= 90600
	RequestNamedLWLockTranche("pg_stat_kcache", 1);
#else
	RequestAddinLWLocks(1);
#endif		/* pg 9.6+ */
#endif		/* pg 15- */

	/* Install hook */
#if PG_VERSION_NUM >= 150000
	prev_shmem_request_hook = shmem_request_hook;
	shmem_request_hook = pgsk_shmem_request;
#endif
	prev_shmem_startup_hook = shmem_startup_hook;
	shmem_startup_hook = pgsk_shmem_startup;
#if PG_VERSION_NUM >= 130000
	prev_planner_hook = planner_hook;
	planner_hook = pgsk_planner;
#endif
	prev_ExecutorStart = ExecutorStart_hook;
	ExecutorStart_hook = pgsk_ExecutorStart;
	prev_ExecutorRun = ExecutorRun_hook;
	ExecutorRun_hook = pgsk_ExecutorRun;
	prev_ExecutorFinish = ExecutorFinish_hook;
	ExecutorFinish_hook = pgsk_ExecutorFinish;
	prev_ExecutorEnd = ExecutorEnd_hook;
	ExecutorEnd_hook = pgsk_ExecutorEnd;
}

static bool
pgsk_assign_linux_hz_check_hook(int *newval, void **extra, GucSource source)
{
	int			val = *newval;
	struct rusage myrusage;
	struct timeval previous_value;

	/* In that case we try to guess it */
	if (val == -1)
	{
		elog(LOG, "Auto detecting pg_stat_kcache.linux_hz parameter...");
		getrusage(RUSAGE_SELF, &myrusage);
		previous_value = myrusage.ru_utime;
		while (myrusage.ru_utime.tv_usec == previous_value.tv_usec &&
			   myrusage.ru_utime.tv_sec == previous_value.tv_sec)
		{
			getrusage(RUSAGE_SELF, &myrusage);
		}
		*newval = (int) (1 / ((myrusage.ru_utime.tv_sec - previous_value.tv_sec) +
		   (myrusage.ru_utime.tv_usec - previous_value.tv_usec) / 1000000.));
		elog(LOG, "pg_stat_kcache.linux_hz is set to %d", *newval);
	}
	return true;
}

static void
pgsk_compute_counters(pgskCounters *counters,
					  struct rusage *rusage_start,
					  struct rusage *rusage_end,
					  QueryDesc *queryDesc)
{
		/* Compute CPU time delta */
		counters->utime = TIMEVAL_DIFF(rusage_start->ru_utime, rusage_end->ru_utime);
		counters->stime = TIMEVAL_DIFF(rusage_start->ru_stime, rusage_end->ru_stime);

		if (queryDesc && queryDesc->totaltime)
		{
			/* Make sure stats accumulation is done */
			InstrEndLoop(queryDesc->totaltime);

			/*
			 * We only consider values greater than 3 * linux tick, otherwise the
			 * bias is too big
			 */
			if (queryDesc->totaltime->total < (3. / pgsk_linux_hz))
			{
				counters->stime = 0;
				counters->utime = queryDesc->totaltime->total;
			}
		}

#ifdef HAVE_GETRUSAGE
		/* Compute the rest of the counters */
		counters->minflts = rusage_end->ru_minflt - rusage_start->ru_minflt;
		counters->majflts = rusage_end->ru_majflt - rusage_start->ru_majflt;
		counters->nswaps = rusage_end->ru_nswap - rusage_start->ru_nswap;
		counters->reads = rusage_end->ru_inblock - rusage_start->ru_inblock;
		counters->writes = rusage_end->ru_oublock - rusage_start->ru_oublock;
		counters->msgsnds = rusage_end->ru_msgsnd - rusage_start->ru_msgsnd;
		counters->msgrcvs = rusage_end->ru_msgrcv - rusage_start->ru_msgrcv;
		counters->nsignals = rusage_end->ru_nsignals - rusage_start->ru_nsignals;
		counters->nvcsws = rusage_end->ru_nvcsw - rusage_start->ru_nvcsw;
		counters->nivcsws = rusage_end->ru_nivcsw - rusage_start->ru_nivcsw;
#endif
}

#if PG_VERSION_NUM >= 90600
static void
pgsk_set_queryid(pgsk_queryid queryid)
{
	/* Only the leader knows the queryid. */
	Assert(!IsParallelWorker());

	pgsk->queryids[MyProcNumber] = queryid;
}
#endif

#if PG_VERSION_NUM >= 150000
/*
 * Request additional shared memory resources.
 *
 * If you change code here, don't forget to also report the modifications in
 * _PG_init() for pg14 and below.
 */
static void
pgsk_shmem_request(void)
{
	if (prev_shmem_request_hook)
		prev_shmem_request_hook();

	RequestAddinShmemSpace(pgsk_memsize());
	RequestNamedLWLockTranche("pg_stat_kcache", 1);
}
#endif

static void
pgsk_shmem_startup(void)
{
	bool		found;
	HASHCTL		info;
	FILE		*file;
	int			i;
	uint32		header;
	int32		num;
	pgskEntry	*buffer = NULL;

	if (prev_shmem_startup_hook)
		prev_shmem_startup_hook();

	/* reset in case this is a restart within the postmaster */
	pgsk = NULL;

	/* Create or attach to the shared memory state */
	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);

	/* global access lock */
	pgsk = ShmemInitStruct("pg_stat_kcache",
					(sizeof(pgskSharedState)
#if PG_VERSION_NUM >= 90600
					 + pgsk_queryids_array_size()
#endif
					),
					&found);

	if (!found)
	{
		/* First time through ... */
#if PG_VERSION_NUM >= 90600
		pgsk->lock = &(GetNamedLWLockTranche("pg_stat_kcache"))->lock;
#else
		pgsk->lock = LWLockAssign();
#endif
	}

	/* set pgsk_max if needed */
	pgsk_setmax();

	memset(&info, 0, sizeof(info));
	info.keysize = sizeof(pgskHashKey);
	info.entrysize = sizeof(pgskEntry);
	info.hash = pgsk_hash_fn;
	info.match = pgsk_match_fn;

	/* allocate stats shared memory hash */
	pgsk_hash = ShmemInitHash("pg_stat_kcache hash",
							  pgsk_max, pgsk_max,
							  &info,
							  HASH_ELEM | HASH_FUNCTION | HASH_COMPARE);

	LWLockRelease(AddinShmemInitLock);

	if (!IsUnderPostmaster)
		on_shmem_exit(pgsk_shmem_shutdown, (Datum) 0);

	/*
	 * Done if some other process already completed our initialization.
	 */
	if (found)
		return;

	/* Load stat file, don't care about locking */
	file = AllocateFile(PGSK_DUMP_FILE, PG_BINARY_R);
	if (file == NULL)
	{
		if (errno == ENOENT)
			return;			/* ignore not-found error */
		goto error;
	}

	/* check is header is valid */
	if (fread(&header, sizeof(uint32), 1, file) != 1 ||
		header != PGSK_FILE_HEADER)
		goto error;

	/* get number of entries */
	if (fread(&num, sizeof(int32), 1, file) != 1)
		goto error;

	for (i = 0; i < num ; i++)
	{
		pgskEntry	temp;
		pgskEntry  *entry;

		if (fread(&temp, sizeof(pgskEntry), 1, file) != 1)
			goto error;

		/* make the hashtable entry (discards old entries if too many) */
		entry = pgsk_entry_alloc(&temp.key);

		/* copy in the actual stats */
		entry->counters[0] = temp.counters[0];
		entry->counters[1] = temp.counters[1];
		entry->stats_since = temp.stats_since;
		/* don't initialize spinlock, already done */
	}

	FreeFile(file);

	/*
	 * Remove the file so it's not included in backups/replication slaves,
	 * etc. A new file will be written on next shutdown.
	 */
	unlink(PGSK_DUMP_FILE);

	return;

error:
	ereport(LOG,
			(errcode_for_file_access(),
			 errmsg("could not read pg_stat_kcache file \"%s\": %m",
					PGSK_DUMP_FILE)));
	if (buffer)
		pfree(buffer);
	if (file)
		FreeFile(file);
	/* delete bogus file, don't care of errors in this case */
	unlink(PGSK_DUMP_FILE);
}

/*
 * shmem_shutdown hook: dump statistics into file.
 *
 */
static void
pgsk_shmem_shutdown(int code, Datum arg)
{
	FILE	*file;
	HASH_SEQ_STATUS hash_seq;
	int32	num_entries;
	pgskEntry	*entry;

	/* Don't try to dump during a crash. */
	if (code)
		return;

	if (!pgsk)
		return;

	file = AllocateFile(PGSK_DUMP_FILE ".tmp", PG_BINARY_W);
	if (file == NULL)
		goto error;

	if (fwrite(&PGSK_FILE_HEADER, sizeof(uint32), 1, file) != 1)
		goto error;

	num_entries = hash_get_num_entries(pgsk_hash);

	if (fwrite(&num_entries, sizeof(int32), 1, file) != 1)
		goto error;

	hash_seq_init(&hash_seq, pgsk_hash);
	while ((entry = hash_seq_search(&hash_seq)) != NULL)
	{
		if (fwrite(entry, sizeof(pgskEntry), 1, file) != 1)
		{
			/* note: we assume hash_seq_term won't change errno */
			hash_seq_term(&hash_seq);
			goto error;
		}
	}

	if (FreeFile(file))
	{
		file = NULL;
		goto error;
	}

	/*
	 * Rename file inplace
	 */
	if (rename(PGSK_DUMP_FILE ".tmp", PGSK_DUMP_FILE) != 0)
		ereport(LOG,
				(errcode_for_file_access(),
				 errmsg("could not rename pg_stat_kcache file \"%s\": %m",
						PGSK_DUMP_FILE ".tmp")));

	return;

error:
	ereport(LOG,
			(errcode_for_file_access(),
			 errmsg("could not read pg_stat_kcache file \"%s\": %m",
					PGSK_DUMP_FILE)));

	if (file)
		FreeFile(file);
	unlink(PGSK_DUMP_FILE);
}

/*
 * Retrieve pg_stat_statement.max GUC value and store it into pgsk_max, since
 * we want to store the same number of entries as pg_stat_statements. Don't do
 * anything if pgsk_max is already set.
 */
static void pgsk_setmax(void)
{
	const char *pgss_max;
	const char *name = "pg_stat_statements.max";

	if (pgsk_max != 0)
		return;

	pgss_max = GetConfigOption(name, true, false);

	/*
	 * Retrieving pg_stat_statements.max can fail if pgss is loaded after pgsk
	 * in shared_preload_libraries.  Hint user in case this happens.
	 */
	if (!pgss_max)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("unrecognized configuration parameter \"%s\"",
						name),
				 errhint("make sure pg_stat_statements is loaded,\n"
					 "and make sure pg_stat_kcache is present after pg_stat_statements"
					 " in the shared_preload_libraries setting")));

	pgsk_max = atoi(pgss_max);
}

static Size pgsk_memsize(void)
{
	Size	size;

	Assert(pgsk_max != 0);

	size = MAXALIGN(sizeof(pgskSharedState));
	size = add_size(size, hash_estimate_size(pgsk_max, sizeof(pgskEntry)));
#if PG_VERSION_NUM >= 90600
	size = add_size(size, MAXALIGN(pgsk_queryids_array_size()));
#endif

	return size;
}

#if PG_VERSION_NUM >= 90600
static Size
pgsk_queryids_array_size(void)
{
	/*
	 * queryid isn't pushed to parallel workers.  We store them in shared mem
	 * for each query, identified by their BackendId.  It therefore needs room
	 * for all possible backends, plus autovacuum launcher and workers, plus bg
	 * workers and an extra one since BackendId numerotation starts at 1.
	 * Starting with pg12, wal senders aren't part of MaxConnections anymore,
	 * so they need to be accounted for.
	 */
#if PG_VERSION_NUM >= 150000
	Assert (MaxBackends > 0);
	return (sizeof(pgsk_queryid) * (MaxBackends + 1));
#else
	return (sizeof(pgsk_queryid) * (MaxConnections + autovacuum_max_workers + 1
							+ max_worker_processes
#if PG_VERSION_NUM >= 120000
							+ max_wal_senders
#endif		/* pg12+ */
							+ 1));
#endif		/* pg15- */
}
#endif


/*
 * support functions
 */

static void
pgsk_entry_store(pgsk_queryid queryId, pgskStoreKind kind,
				 pgskCounters counters)
{
	volatile pgskEntry *e;

	pgskHashKey key;
	pgskEntry  *entry;

	/* Safety check... */
	if (!pgsk || !pgsk_hash)
		return;

	/* Set up key for hashtable search */
	key.userid = GetUserId();
	key.dbid = MyDatabaseId;
	key.queryid = queryId;
	key.top = (nesting_level == 0);

	/* Lookup the hash table entry with shared lock. */
	LWLockAcquire(pgsk->lock, LW_SHARED);

	entry = (pgskEntry *) hash_search(pgsk_hash, &key, HASH_FIND, NULL);

	/* Create new entry, if not present */
	if (!entry)
	{
		/* Need exclusive lock to make a new hashtable entry - promote */
		LWLockRelease(pgsk->lock);
		LWLockAcquire(pgsk->lock, LW_EXCLUSIVE);

		/* OK to create a new hashtable entry */
		entry = pgsk_entry_alloc(&key);
	}

	/*
	 * Grab the spinlock while updating the counters (see comment about
	 * locking rules at the head of the file)
	 */
	e = (volatile pgskEntry *) entry;

	SpinLockAcquire(&e->mutex);

	e->counters[0].usage += USAGE_INCREASE;

	e->counters[kind].utime += counters.utime;
	e->counters[kind].stime += counters.stime;
#ifdef HAVE_GETRUSAGE
	e->counters[kind].minflts += counters.minflts;
	e->counters[kind].majflts += counters.majflts;
	e->counters[kind].nswaps += counters.nswaps;
	e->counters[kind].reads += counters.reads;
	e->counters[kind].writes += counters.writes;
	e->counters[kind].msgsnds += counters.msgsnds;
	e->counters[kind].msgrcvs += counters.msgrcvs;
	e->counters[kind].nsignals += counters.nsignals;
	e->counters[kind].nvcsws += counters.nvcsws;
	e->counters[kind].nivcsws += counters.nivcsws;
#endif

	SpinLockRelease(&e->mutex);

	LWLockRelease(pgsk->lock);
}

/*
 * Allocate a new hashtable entry.
 * caller must hold an exclusive lock on pgsk->lock
 */
static pgskEntry *pgsk_entry_alloc(pgskHashKey *key)
{
	pgskEntry  *entry;
	bool		found;

	/* Make space if needed */
	while (hash_get_num_entries(pgsk_hash) >= pgsk_max)
		pgsk_entry_dealloc();

	/* Find or create an entry with desired hash code */
	entry = (pgskEntry *) hash_search(pgsk_hash, key, HASH_ENTER, &found);

	if (!found)
	{
		/* New entry, initialize it */

		/* reset the statistics */
		memset(&entry->counters, 0, sizeof(pgskCounters) * PGSK_NUMKIND);
		/* set the appropriate initial usage count */
		entry->counters[0].usage = USAGE_INIT;
		/* re-initialize the mutex each time ... we assume no one using it */
		SpinLockInit(&entry->mutex);
		entry->stats_since = GetCurrentTimestamp();
	}

	return entry;
}

/*
 * qsort comparator for sorting into increasing usage order
 */
static int
entry_cmp(const void *lhs, const void *rhs)
{
	double		l_usage = (*(pgskEntry *const *) lhs)->counters[0].usage;
	double		r_usage = (*(pgskEntry *const *) rhs)->counters[0].usage;

	if (l_usage < r_usage)
		return -1;
	else if (l_usage > r_usage)
		return +1;
	else
		return 0;
}

/*
 * Deallocate least used entries.
 * Caller must hold an exclusive lock on pgsk->lock.
 */
static void
pgsk_entry_dealloc(void)
{
	HASH_SEQ_STATUS hash_seq;
	pgskEntry **entries;
	pgskEntry  *entry;
	int			nvictims;
	int			i;

	/*
	 * Sort entries by usage and deallocate USAGE_DEALLOC_PERCENT of them.
	 * While we're scanning the table, apply the decay factor to the usage
	 * values.
	 */
	entries = palloc(hash_get_num_entries(pgsk_hash) * sizeof(pgskEntry *));

	i = 0;
	hash_seq_init(&hash_seq, pgsk_hash);
	while ((entry = hash_seq_search(&hash_seq)) != NULL)
	{
		entries[i++] = entry;
		entry->counters[0].usage *= USAGE_DECREASE_FACTOR;
	}

	qsort(entries, i, sizeof(pgskEntry *), entry_cmp);

	nvictims = Max(10, i * USAGE_DEALLOC_PERCENT / 100);
	nvictims = Min(nvictims, i);

	for (i = 0; i < nvictims; i++)
	{
		hash_search(pgsk_hash, &entries[i]->key, HASH_REMOVE, NULL);
	}

	pfree(entries);
}

static void pgsk_entry_reset(void)
{
	HASH_SEQ_STATUS hash_seq;
	pgskEntry  *entry;

	LWLockAcquire(pgsk->lock, LW_EXCLUSIVE);

	hash_seq_init(&hash_seq, pgsk_hash);
	while ((entry = hash_seq_search(&hash_seq)) != NULL)
	{
		hash_search(pgsk_hash, &entry->key, HASH_REMOVE, NULL);
	}

	LWLockRelease(pgsk->lock);
}

/*
 * Hooks
 */

#if PG_VERSION_NUM >= 130000
/*
 * Planner hook: forward to regular planner, but measure planning time
 * if needed.
 */
static PlannedStmt *
pgsk_planner(Query *parse,
			 const char *query_string,
			 int cursorOptions,
			 ParamListInfo boundParams)
{
	PlannedStmt *result;

	/* We can't process the query if no queryid has been computed. */
	if (pgsk_enabled(nesting_level)
		&& pgsk_track_planning
		&& parse->queryId != UINT64CONST(0))
	{
		struct rusage *rusage_start = &plan_rusage_start[nesting_level];
		struct rusage rusage_end;
		pgskCounters counters;

		/* capture kernel usage stats in rusage_start */
		getrusage(RUSAGE_SELF, rusage_start);

		nesting_level++;
		PG_TRY();
		{
			if (prev_planner_hook)
				result = prev_planner_hook(parse, query_string, cursorOptions,
										   boundParams);
			else
				result = standard_planner(parse, query_string, cursorOptions,
										  boundParams);
			nesting_level--;
		}
		PG_CATCH();
		{
			nesting_level--;
			PG_RE_THROW();
		}
		PG_END_TRY();

		/* capture kernel usage stats in rusage_end */
		getrusage(RUSAGE_SELF, &rusage_end);

		pgsk_compute_counters(&counters, rusage_start, &rusage_end, NULL);

		/* store current number of block reads and writes */
		pgsk_entry_store(parse->queryId, PGSK_PLAN, counters);

		if (pgsk_counters_hook)
		    pgsk_counters_hook(&counters,
							   query_string,
							   nesting_level,
							   PGSK_PLAN);
	}
	else
	{
		/*
		 * Even though we're not tracking planning for this statement, we
		 * must still increment the nesting level, to ensure that functions
		 * evaluated during planning are not seen as top-level calls.
		 */
		nesting_level++;
		PG_TRY();
		{
			if (prev_planner_hook)
				result = prev_planner_hook(parse, query_string, cursorOptions,
										   boundParams);
			else
				result = standard_planner(parse, query_string, cursorOptions,
										  boundParams);
			nesting_level--;
		}
		PG_CATCH();
		{
			nesting_level--;
			PG_RE_THROW();
		}
		PG_END_TRY();
	}

	return result;
}
#endif

static void
pgsk_ExecutorStart (QueryDesc *queryDesc, int eflags)
{
	if (pgsk_enabled(nesting_level))
	{
		struct rusage *rusage_start = &exec_rusage_start[nesting_level];

		/* capture kernel usage stats in rusage_start */
		getrusage(RUSAGE_SELF, rusage_start);

#if PG_VERSION_NUM >= 90600
		/* Save the queryid so parallel worker can retrieve it */
		if (!IsParallelWorker())
		{
			pgsk_set_queryid(queryDesc->plannedstmt->queryId);
		}
#endif
	}

	/* give control back to PostgreSQL */
	if (prev_ExecutorStart)
		prev_ExecutorStart(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}

/*
 * ExecutorRun hook: all we need do is track nesting depth
 */
static void
pgsk_ExecutorRun(QueryDesc *queryDesc,
				 ScanDirection direction,
#if PG_VERSION_NUM >= 90600
				 uint64 count
#else
				 long count
#endif
#if PG_VERSION_NUM >= 100000
				 ,bool execute_once
#endif
)
{
	nesting_level++;
	PG_TRY();
	{
		if (prev_ExecutorRun)
#if PG_VERSION_NUM >= 100000
			prev_ExecutorRun(queryDesc, direction, count, execute_once);
#else
			prev_ExecutorRun(queryDesc, direction, count);
#endif
		else
#if PG_VERSION_NUM >= 100000
			standard_ExecutorRun(queryDesc, direction, count, execute_once);
#else
			standard_ExecutorRun(queryDesc, direction, count);
#endif
		nesting_level--;
	}
	PG_CATCH();
	{
		nesting_level--;
		PG_RE_THROW();
	}
	PG_END_TRY();
}

/*
 * ExecutorFinish hook: all we need do is track nesting depth
 */
static void
pgsk_ExecutorFinish(QueryDesc *queryDesc)
{
	nesting_level++;
	PG_TRY();
	{
		if (prev_ExecutorFinish)
			prev_ExecutorFinish(queryDesc);
		else
			standard_ExecutorFinish(queryDesc);
		nesting_level--;
	}
	PG_CATCH();
	{
		nesting_level--;
		PG_RE_THROW();
	}
	PG_END_TRY();
}

static void
pgsk_ExecutorEnd (QueryDesc *queryDesc)
{
	pgsk_queryid queryId;
	struct rusage rusage_end;
	pgskCounters counters;

	if (pgsk_enabled(nesting_level))
	{
		struct rusage *rusage_start = &exec_rusage_start[nesting_level];

		/* capture kernel usage stats in rusage_end */
		getrusage(RUSAGE_SELF, &rusage_end);

#if PG_VERSION_NUM >= 90600
		if (IsParallelWorker())
			queryId = pgsk->queryids[ParallelLeaderProcNumber];
		else
#endif
		queryId = queryDesc->plannedstmt->queryId;

		pgsk_compute_counters(&counters, rusage_start, &rusage_end, queryDesc);

		/* store current number of block reads and writes */
		pgsk_entry_store(queryId, PGSK_EXEC, counters);

		if (pgsk_counters_hook)
		    pgsk_counters_hook(&counters,
							   (const char *)queryDesc->sourceText,
							   nesting_level,
							   PGSK_EXEC);
	}

	/* give control back to PostgreSQL */
	if (prev_ExecutorEnd)
		prev_ExecutorEnd(queryDesc);
	else
		standard_ExecutorEnd(queryDesc);
}

/*
 * Calculate hash value for a key
 */
static uint32
pgsk_hash_fn(const void *key, Size keysize)
{
	const pgskHashKey *k = (const pgskHashKey *) key;

	return hash_uint32((uint32) k->userid) ^
		hash_uint32((uint32) k->dbid) ^
		hash_uint32((uint32) k->queryid) ^
		hash_uint32((uint32) k->top);
}

/*
 * Compare two keys - zero means match
 */
static int
pgsk_match_fn(const void *key1, const void *key2, Size keysize)
{
	const pgskHashKey *k1 = (const pgskHashKey *) key1;
	const pgskHashKey *k2 = (const pgskHashKey *) key2;

	if (k1->userid == k2->userid &&
		k1->dbid == k2->dbid &&
		k1->queryid == k2->queryid &&
		k1->top == k2->top)
		return 0;
	else
		return 1;
}

/*
 * Reset statistics.
 */
PGDLLEXPORT Datum
pg_stat_kcache_reset(PG_FUNCTION_ARGS)
{
	if (!pgsk)
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("pg_stat_kcache must be loaded via shared_preload_libraries")));

	pgsk_entry_reset();
	PG_RETURN_VOID();
}

PGDLLEXPORT Datum
pg_stat_kcache(PG_FUNCTION_ARGS)
{
	pg_stat_kcache_internal(fcinfo, PGSK_V2_0);

	return (Datum) 0;
}

PGDLLEXPORT Datum
pg_stat_kcache_2_1(PG_FUNCTION_ARGS)
{
	pg_stat_kcache_internal(fcinfo, PGSK_V2_1);

	return (Datum) 0;
}

PGDLLEXPORT Datum
pg_stat_kcache_2_2(PG_FUNCTION_ARGS)
{
	pg_stat_kcache_internal(fcinfo, PGSK_V2_2);

	return (Datum) 0;
}

PGDLLEXPORT Datum
pg_stat_kcache_2_3(PG_FUNCTION_ARGS)
{
	pg_stat_kcache_internal(fcinfo, PGSK_V2_3);

	return (Datum) 0;
}

static void
pg_stat_kcache_internal(FunctionCallInfo fcinfo, pgskVersion api_version)
{
	ReturnSetInfo	*rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	MemoryContext	per_query_ctx;
	MemoryContext	oldcontext;
	TupleDesc		tupdesc;
	Tuplestorestate	*tupstore;
	HASH_SEQ_STATUS hash_seq;
	pgskEntry		*entry;


	if (!pgsk)
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("pg_stat_kcache must be loaded via shared_preload_libraries")));
	/* check to see if caller supports us returning a tuplestore */
	if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("set-valued function called in context that cannot accept a set")));
	if (!(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("materialize mode required, but it is not " \
							"allowed in this context")));

	/* Switch into long-lived context to construct returned data structures */
	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);

	/* Build a tuple descriptor for our result type */
	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		elog(ERROR, "return type must be a row type");

	tupstore = tuplestore_begin_heap(true, false, work_mem);
	rsinfo->returnMode = SFRM_Materialize;
	rsinfo->setResult = tupstore;
	rsinfo->setDesc = tupdesc;

	MemoryContextSwitchTo(oldcontext);

	LWLockAcquire(pgsk->lock, LW_SHARED);

	hash_seq_init(&hash_seq, pgsk_hash);
	while ((entry = hash_seq_search(&hash_seq)) != NULL)
	{
		Datum			values[PG_STAT_KCACHE_COLS];
		bool			nulls[PG_STAT_KCACHE_COLS];
		volatile pgskCounters   *tmp;
		int				i = 0;
		int				kind, min_kind = 0;
#ifdef HAVE_GETRUSAGE
		int64			reads, writes;
#endif
		TimestampTz		stats_since;

		memset(values, 0, sizeof(values));
		memset(nulls, 0, sizeof(nulls));

		values[i++] = Int64GetDatum(entry->key.queryid);
		if (api_version >= PGSK_V2_2)
			values[i++] = BoolGetDatum(entry->key.top);
		values[i++] = ObjectIdGetDatum(entry->key.userid);
		values[i++] = ObjectIdGetDatum(entry->key.dbid);

		/* planning time (kind == 0) is added in v2.2 */
		if (api_version < PGSK_V2_2)
			min_kind = 1;

		/* copy counters to a local variable to keep locking time short */
		{
			volatile pgskEntry *e = (volatile pgskEntry *) entry;

			SpinLockAcquire(&e->mutex);
			tmp = e->counters;
			stats_since = e->stats_since;
			SpinLockRelease(&e->mutex);
		}

		for (kind = min_kind; kind < PGSK_NUMKIND; kind++)
		{
#ifdef HAVE_GETRUSAGE
			reads = tmp[kind].reads * RUSAGE_BLOCK_SIZE;
			writes = tmp[kind].writes * RUSAGE_BLOCK_SIZE;
			values[i++] = Int64GetDatumFast(reads);
			values[i++] = Int64GetDatumFast(writes);
#else
			nulls[i++] = true; /* reads */
			nulls[i++] = true; /* writes */
#endif
			values[i++] = Float8GetDatumFast(tmp[kind].utime);
			values[i++] = Float8GetDatumFast(tmp[kind].stime);
			if (api_version >= PGSK_V2_1)
			{
#ifdef HAVE_GETRUSAGE
				values[i++] = Int64GetDatumFast(tmp[kind].minflts);
				values[i++] = Int64GetDatumFast(tmp[kind].majflts);
				values[i++] = Int64GetDatumFast(tmp[kind].nswaps);
				values[i++] = Int64GetDatumFast(tmp[kind].msgsnds);
				values[i++] = Int64GetDatumFast(tmp[kind].msgrcvs);
				values[i++] = Int64GetDatumFast(tmp[kind].nsignals);
				values[i++] = Int64GetDatumFast(tmp[kind].nvcsws);
				values[i++] = Int64GetDatumFast(tmp[kind].nivcsws);
#else
				nulls[i++] = true; /* minflts */
				nulls[i++] = true; /* majflts */
				nulls[i++] = true; /* nswaps */
				nulls[i++] = true; /* msgsnds */
				nulls[i++] = true; /* msgrcvs */
				nulls[i++] = true; /* nsignals */
				nulls[i++] = true; /* nvcsws */
				nulls[i++] = true; /* nivcsws */
#endif
			}
		}
		if (api_version >= PGSK_V2_3)
			values[i++] = TimestampTzGetDatum(stats_since);

		Assert(i == (api_version == PGSK_V2_0 ? PG_STAT_KCACHE_COLS_V2_0 :
					 api_version == PGSK_V2_1 ? PG_STAT_KCACHE_COLS_V2_1 :
					 api_version == PGSK_V2_2 ? PG_STAT_KCACHE_COLS_V2_2 :
					 api_version == PGSK_V2_3 ? PG_STAT_KCACHE_COLS_V2_3 :
					 -1 /* fail if you forget to update this assert */ ));

		tuplestore_putvalues(tupstore, tupdesc, values, nulls);
	}

	LWLockRelease(pgsk->lock);
}
