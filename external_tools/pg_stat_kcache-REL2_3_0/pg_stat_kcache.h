#ifndef PGSK_H_
#define PGSK_H_

#define PG_STAT_KCACHE_COLS_V2_0    7
#define PG_STAT_KCACHE_COLS_V2_1    15
#define PG_STAT_KCACHE_COLS_V2_2    28
#define PG_STAT_KCACHE_COLS_V2_3    29
#define PG_STAT_KCACHE_COLS         29 /* maximum of above */

/* ru_inblock block size is 512 bytes with Linux
 * see http://lkml.indiana.edu/hypermail/linux/kernel/0703.2/0937.html
 */
#define RUSAGE_BLOCK_SIZE	512			/* Size of a block for getrusage() */

/*
 * Current getrusage counters.
 *
 * For platform without getrusage support, we rely on postgres implementation
 * defined in rusagestub.h, which only supports user and system time.
 *
 * Note that the following counters are not maintained on GNU/Linux:
 *   - ru_nswap
 *   - ru_msgsnd
 *   - ru_msgrcv
 *   - ru_nsignals
*/
typedef struct pgskCounters
{
	double			usage;		/* usage factor */
	/* These fields are always used */
	float8			utime;		/* CPU user time */
	float8			stime;		/* CPU system time */
#ifdef HAVE_GETRUSAGE
/* These fields are only used for platform with HAVE_GETRUSAGE defined */
	int64			minflts;	/* page reclaims (soft page faults) */
	int64			majflts;	/* page faults (hard page faults) */
	int64			nswaps;		/* swaps */
	int64			reads;		/* Physical block reads */
	int64			writes;		/* Physical block writes */
	int64			msgsnds;	/* IPC messages sent */
	int64			msgrcvs;	/* IPC messages received */
	int64			nsignals;	/* signals received */
	int64			nvcsws;		/* voluntary context witches */
	int64			nivcsws;	/* unvoluntary context witches */
#endif
} pgskCounters;

typedef enum pgskStoreKind
{
	/*
	 * PGSS_PLAN and PGSS_EXEC must be respectively 0 and 1 as they're used to
	 * reference the underlying values in the arrays in the Counters struct,
	 * and this order is required in pg_stat_statements_internal().
	 */
	PGSK_PLAN = 0,
	PGSK_EXEC,

	PGSK_NUMKIND				/* Must be last value of this enum */
} pgskStoreKind;

/* Hook for extensions to use pgskCounters right after calculation. */
typedef void (*pgsk_counters_hook_type) (
		pgskCounters *counters,
		const char *query_string,
		int level,
		pgskStoreKind kind);
extern PGDLLIMPORT pgsk_counters_hook_type pgsk_counters_hook;

#endif
