/*****************************************************************************
 *
 * time_analyze.h
 *	  Functions for gathering statistics from time type columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_ANALYZE_H__
#define __TIME_ANALYZE_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum period_analyze(PG_FUNCTION_ARGS);
extern Datum timestampset_analyze(PG_FUNCTION_ARGS);
extern Datum periodset_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
