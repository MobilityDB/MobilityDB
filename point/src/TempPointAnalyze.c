/*****************************************************************************
 *
 * TempPointAnalyze.c
 *	  Functions for gathering statistics from temporal columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	point/src/TempPointAnalyze.c
 *
 *****************************************************************************/
#include <TemporalTypes.h>
#include <TemporalAnalyze.h>
/*****************************************************************************/


PG_FUNCTION_INFO_V1(tpoint_analyze);
Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
