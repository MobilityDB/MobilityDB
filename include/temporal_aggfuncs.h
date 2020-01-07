/*****************************************************************************
 *
 * temporal_aggfuncs.h
 *	  Temporal aggregate functions
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_AGGFUNCS_H__
#define __TEMPORAL_AGGFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

typedef struct AggregateState
{
	int 		size;
	void		*extra;
	size_t		extrasize;
	Temporal 	*values[];
} AggregateState;

/*****************************************************************************/

/* SkipList - Internal type for computing aggregates */

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_INITIAL_CAPACITY 1024
#define SKIPLIST_GROW 2
#define SKIPLIST_INITIAL_FREELIST 32

typedef struct
{
	Temporal *value;
	int height;
	int next[SKIPLIST_MAXLEVEL];
} Elem;

typedef struct
{
	int capacity;
	int next;
	int length;
	int *freed;
	int freecount;
	int freecap;
	int tail;
	void *extra;
	size_t extrasize;
	Elem *elems;
} SkipList;

/*****************************************************************************/

extern Datum datum_min_int32(Datum l, Datum r);
extern Datum datum_max_int32(Datum l, Datum r);
extern Datum datum_sum_int32(Datum l, Datum r);
extern Datum datum_min_float8(Datum l, Datum r);
extern Datum datum_max_float8(Datum l, Datum r);
extern Datum datum_sum_float8(Datum l, Datum r);
extern Datum datum_min_text(Datum l, Datum r);
extern Datum datum_max_text(Datum l, Datum r);
extern Datum datum_sum_double2(Datum l, Datum r);
extern Datum datum_sum_double3(Datum l, Datum r);
extern Datum datum_sum_double4(Datum l, Datum r);

extern Temporal *skiplist_headval(SkipList *list);
extern Temporal **skiplist_values(SkipList *list);
extern SkipList *skiplist_make(FunctionCallInfo fcinfo, Temporal **values, 
	int count);
extern void skiplist_splice(FunctionCallInfo fcinfo, SkipList *list, 
	Temporal **values, int count, Datum (*func)(Datum, Datum), bool crossings);
extern void aggstate_set_extra(FunctionCallInfo fcinfo, SkipList *state, 
	void *data, size_t size);

extern SkipList *temporalseq_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
	TemporalSeq *seq, Datum (*func)(Datum, Datum), bool interpoint);
extern SkipList *temporal_tagg_combinefn(FunctionCallInfo fcinfo, SkipList *state1,
	SkipList *state2, Datum (*func)(Datum, Datum), bool crossings);

/*****************************************************************************/

extern Datum temporal_extent_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_extent_combinefn(PG_FUNCTION_ARGS);
extern Datum tnumber_extent_transfn(PG_FUNCTION_ARGS);
extern Datum tnumber_extent_combinefn(PG_FUNCTION_ARGS);

extern Datum tbool_tand_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tand_combinefn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tagg_finalfn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_finalfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_combinefn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
