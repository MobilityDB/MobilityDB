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

/**
* The dimensions of temporal types our code can handle.
* We'll use this to determine which part of our types
* should have statistics
*/
#define TEMPORAL_STATISTIC	1
#define TNUMBER_STATISTIC	2
#define TPOINT_STATISTIC	3

/* Extra data for compute_stats function */
typedef struct 
{
	/* Information about array element type */
	Oid type_id;			/* element type's OID */
	Oid eq_opr;				/* default equality operator's OID */
	bool typbyval;			/* physical properties of element type */
	int16 typlen;
	char typalign;

	/* Information about the value part of array element */
	Oid value_type_id;		/* element type's OID */
	Oid value_eq_opr;		/* default equality operator's OID */
	bool value_typbyval;	/* physical properties of element type */
	int16 value_typlen;
	char value_typalign;

	/* Information about the temporal part of array element */
	Oid temporal_type_id;	/* element type's OID */
	Oid temporal_eq_opr;	/* default equality operator's OID */
	bool temporal_typbyval;	/* physical properties of element type */
	int16 temporal_typlen;
	char temporal_typalign;

	/*
	 * Lookup data for element type's comparison and hash functions (these are
	 * in the type's typcache entry, which we expect to remain valid over the
	 * lifespan of the ANALYZE run)
	 */
	FmgrInfo *cmp;
	FmgrInfo *hash;
	FmgrInfo *value_cmp;
	FmgrInfo *value_hash;
	FmgrInfo *temporal_cmp;
	FmgrInfo *temporal_hash;

	/* Saved state from std_typanalyze() */
	AnalyzeAttrComputeStatsFunc std_compute_stats;
	void *std_extra_data;
} TemporalArrayAnalyzeExtraData;

/* A hash table entry for the Lossy Counting algorithm */
typedef struct
{
	Datum		key;			/* This is 'e' from the LC algorithm. */
	int			frequency;		/* This is 'f'. */
	int			delta;			/* And this is 'delta'. */
	int			last_container;	/* For de-duplication of array elements. */
} TrackItem;

/* A hash table entry for distinct-elements counts */
typedef struct
{
	int			count; 	 	  /* Count of distinct elements in an array */
	int			frequency; 	  /* Number of arrays seen with this count */
} DECountItem;

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
 * Statistics information for Temporal types
 *****************************************************************************/

extern void temporal_info(VacAttrStats *stats);
extern void temporal_extra_info(VacAttrStats *stats, int durationType);

/*****************************************************************************
 * Statistics functions for Temporal* type
 *****************************************************************************/

extern void temporalinst_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									   int samplerows, double totalrows);

extern void temporali_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									int samplerows, double totalrows);

extern void temporals_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									int samplerows, double totalrows);

/*****************************************************************************/

extern Datum temporal_analyze(PG_FUNCTION_ARGS);
extern Datum tnumber_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif 

