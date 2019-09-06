/*****************************************************************************
 *
 * temporal_analyze.h
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *-------------------------------------------------------------------------
 */
#ifndef __TEMPORAL_ANALYZE_H__
#define __TEMPORAL_ANALYZE_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <commands/vacuum.h>
#include <utils/rangetypes.h>
#include <parser/parse_oper.h>
#include <statistics/extended_stats_internal.h>

/* 
 * Extra data for compute_stats function 
 * Structure based on the ArrayAnalyzeExtraData from file array_typanalyze.c
 */
typedef struct 
{
	/* Information about array element type */
	Oid type_id;			/* element type's OID */
	Oid eq_opr;				/* default equality operator's OID */
	Oid lt_opr;				/* default less than operator's OID */
	bool typbyval;			/* physical properties of element type */
	int16 typlen;
	char typalign;

	/* Information about the value part of array element */
	Oid value_type_id;		/* element type's OID */
	Oid value_eq_opr;		/* default equality operator's OID */
	Oid value_lt_opr;		/* default less than operator's OID */
	bool value_typbyval;	/* physical properties of element type */
	int16 value_typlen;
	char value_typalign;

	/* Information about the temporal part of array element */
	Oid time_type_id;	/* element type's OID */
	Oid time_eq_opr;	/* default equality operator's OID */
	Oid time_lt_opr;	/* default less than operator's OID */
	bool time_typbyval;	/* physical properties of element type */
	int16 time_typlen;
	char time_typalign;

	/*
	 * Lookup data for element type's comparison and hash functions (these are
	 * in the type's typcache entry, which we expect to remain valid over the
	 * lifespan of the ANALYZE run)
	 */
	FmgrInfo *cmp;
	FmgrInfo *hash;
	FmgrInfo *value_cmp;
	FmgrInfo *value_hash;
	FmgrInfo *time_cmp;
	FmgrInfo *time_hash;

	/* Saved state from std_typanalyze() */
	AnalyzeAttrComputeStatsFunc std_compute_stats;
	void *std_extra_data;
} TemporalAnalyzeExtraData;

/*
 * Extra information used by the default analysis routines
 */
typedef struct
{
	int	    count;		/* # of duplicates */
	int	    first;		/* values[] index of first occurrence */
} ScalarMCVItem;

typedef struct
{
	SortSupport ssup;
	int	   *tupnoLink;
} CompareScalarsContext;

/*****************************************************************************
 * Statistics information for temporal types
 *****************************************************************************/

extern void temporal_extra_info(VacAttrStats *stats);

extern void temporalinst_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									   int samplerows, double totalrows);

extern void temporals_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									int samplerows, double totalrows);

/*****************************************************************************/

extern Datum temporal_analyze(PG_FUNCTION_ARGS);
extern Datum tnumber_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif 

