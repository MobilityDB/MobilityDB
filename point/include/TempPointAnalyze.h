/*****************************************************************************
 *
 * TempPointAnalyze.h
 *	  Functions for gathering statistics from temporal point columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPPOINTANALYZE_H__
#define __TEMPPOINTANALYZE_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tpoint_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
