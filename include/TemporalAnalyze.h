/*****************************************************************************
 *
 * TemporalAnalyze.h
 *
 * Common Functions for analyze function of temporal types
 *
 * Estimates are based on histograms of lower and upper bounds.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	include/TemporalAnalyze.h
 *
 *****************************************************************************/

#ifndef MOBILITYDB_TEMPANALYZE_COMMON_UTILITIES_H
#define MOBILITYDB_TEMPANALYZE_COMMON_UTILITIES_H

#include <TemporalTypes.h>
#include <TemporalPoint.h>
#include <PostGIS.h>

/**
* The dimensions of temporal types our code can handle.
* We'll use this to determine which part of our types
* should have statistics
*/
#define TEMPORAL_STATISTIC	1
#define TNUMBER_STATISTIC	2
#define TPOINT_STATISTIC	3

/* Extra data for compute_stats function */
typedef struct {
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
	Datum		key; 	 	 	/* This is 'e' from the LC algorithm. */
	int			frequency; 	  /* This is 'f'. */
	int			delta; 	 	  /* And this is 'delta'. */
	int			last_container; /* For de-duplication of array elements. */
} TrackItem;

/* A hash table entry for distinct-elements counts */
typedef struct
{
	int			count; 	 	  /* Count of distinct elements in an array */
	int			frequency; 	  /* Number of arrays seen with this count */
} DECountItem;

extern Datum temporal_analyze_internal(VacAttrStats *stats, int durationType, int temporalType);
/*****************************************************************************
 * Statistics information for Temporal types
 *****************************************************************************/
extern void temporalinst_info(VacAttrStats *stats);
extern void temporal_extra_info(VacAttrStats *stats);
/*****************************************************************************
 * Statistics functions for TemporalInst type
 *****************************************************************************/
extern void compute_timestamptz_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
 	 	 	 	 	 	 	 	 	  int samplerows, double totalrows);
extern void compute_temporalinst_twodim_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
 	 	 	 	 	 	 	 	 	 	 	  int samplerows, double totalrows);
/*****************************************************************************
 * Statistics functions for TemporalI type
 *****************************************************************************/
extern void compute_timestamptz_set_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									  int samplerows, double totalrows);
extern void compute_temporali_twodim_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
											  int samplerows, double totalrows);
/*****************************************************************************
 * Statistics functions for Trajectory types (TemporalSeq and TemporalS)
 *****************************************************************************/
extern void compute_timestamptz_traj_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
 	 	 	 	 	 	 	 	 	 	   int samplerows, double totalrows);
extern void compute_twodim_traj_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
									 int samplerows, double totalrows);
/*****************************************************************************
 * Statistics functions for TPOINT types
 *****************************************************************************/
extern Datum tpoint_analyze_internal(VacAttrStats *stats);
extern void tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
								 int samplerows, double totalrows);
/*****************************************************************************
 * Comparison functions for different data types
 *****************************************************************************/
extern uint32 element_hash_value(const void *key, Size keysize);
extern uint32 element_hash_temporal(const void *key, Size keysize);
extern int element_match(const void *key1, const void *key2, Size keysize);
extern int trackitem_compare_frequencies_desc(const void *e1, const void *e2);
extern int trackitem_compare_element(const void *e1, const void *e2);
extern int countitem_compare_count(const void *e1, const void *e2);
extern int element_compare(const void *key1, const void *key2);
extern uint32 generic_element_hash(const void *key, Size keysize, FmgrInfo * hash);
extern int period_bound_qsort_cmp(const void *a1, const void *a2);
extern int float8_qsort_cmp(const void *a1, const void *a2);
extern int range_bound_qsort_cmp(const void *a1, const void *a2);
extern int compare_scalars(const void *a, const void *b, void *arg);
extern int compare_mcvs(const void *a, const void *b);
/*****************************************************************************
 * Different functions used for 1D, 2D, and 3D types.
 *****************************************************************************/
extern HeapTuple remove_temporaldim(HeapTuple tuple, TupleDesc tupDesc, int attrNum, Oid attrtypid,
 	 	 	 	 	 	 	 	 	bool geom, Datum value);
extern Period* get_bbox_onedim(Datum value, Oid oid);
extern BOX* get_bbox_twodim(Datum value, Oid oid);
extern GBOX* get_bbox_threedim(Datum value, Oid oid);
extern void box_deserialize(BOX *box, RangeBound *lowerdim1, RangeBound *upperdim1,
							PeriodBound *lowerdim2, PeriodBound *upperdim2);
extern void gbox_deserialize(GBOX *box, RangeBound *lowerdim1, RangeBound *upperdim1,
							 RangeBound *lowerdim2, RangeBound *upperdim2,
							 PeriodBound *lowerdim3, PeriodBound *upperdim3);

#endif MOBILITYDB_TEMPANALYZE_COMMON_UTILITIES_H
