/*-------------------------------------------------------------------------
 *
 * float.c
 *	  Functions for the built-in floating-point types.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/float.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>

/*
 * Configurable GUC parameter
 *
 * If >0, use shortest-decimal format for output; this is both the default and
 * allows for compatibility with clients that explicitly set a value here to
 * get round-trip-accurate results. If 0 or less, then use the old, slow,
 * decimal rounding method.
 */
int			extra_float_digits = 1;

/*
 * We use these out-of-line ereport() calls to report float overflow,
 * underflow, and zero-divide, because following our usual practice of
 * repeating them at each call site would lead to a lot of code bloat.
 *
 * This does mean that you don't get a useful error location indicator.
 */
pg_noinline void
float_overflow_error(void)
{
	elog(ERROR, "value out of range: overflow");
}

pg_noinline void
float_underflow_error(void)
{
	elog(ERROR, "value out of range: underflow");
}

pg_noinline void
float_zero_divide_error(void)
{
	elog(ERROR, "division by zero");
}

/*
 *		float8{eq,ne,lt,le,gt,ge}		- float8/float8 comparison operations
 */
int
float8_cmp_internal(float8 a, float8 b)
{
	if (float8_gt(a, b))
		return 1;
	if (float8_lt(a, b))
		return -1;
	return 0;
}

