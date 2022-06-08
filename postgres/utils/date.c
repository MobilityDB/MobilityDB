/*-------------------------------------------------------------------------
 *
 * date.c
 *	  implements DATE and TIME data types specified in SQL standard
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/date.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>

// #include "access/xact.h"
#include "common/hashfn.h"
// #include "libpq/pqformat.h"
// #include "miscadmin.h"
// #include "nodes/supportnodes.h"
// #include "parser/scansup.h"
// #include "utils/array.h"
// #include "utils/builtins.h"
#include "utils/date.h"
#include "utils/datetime.h"
// #include "utils/numeric.h"
// #include "utils/sortsupport.h"

/*
 * gcc's -ffast-math switch breaks routines that expect exact results from
 * expressions like timeval / SECS_PER_HOUR, where timeval is double.
 */
#ifdef __FAST_MATH__
#error -ffast-math is known to break this code
#endif

/* time_overflows()
 * Check to see if a broken-down time-of-day is out of range.
 */
bool
time_overflows(int hour, int min, int sec, fsec_t fsec)
{
	/* Range-check the fields individually. */
	if (hour < 0 || hour > HOURS_PER_DAY ||
		min < 0 || min >= MINS_PER_HOUR ||
		sec < 0 || sec > SECS_PER_MINUTE ||
		fsec < 0 || fsec > USECS_PER_SEC)
		return true;

	/*
	 * Because we allow, eg, hour = 24 or sec = 60, we must check separately
	 * that the total time value doesn't exceed 24:00:00.
	 */
	if ((((((hour * MINS_PER_HOUR + min) * SECS_PER_MINUTE)
		   + sec) * USECS_PER_SEC) + fsec) > USECS_PER_DAY)
		return true;

	return false;
}

