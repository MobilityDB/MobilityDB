/*****************************************************************************
 *
 * temporal_aggfuncs.h
 *	  Temporal aggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
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

extern AggregateState *aggstate_make(FunctionCallInfo fcinfo, int size, Temporal **values);
extern void aggstate_set_extra(FunctionCallInfo fcinfo, AggregateState* state, void* data, size_t size);
extern void aggstate_move_extra(AggregateState* dest, AggregateState* src);

extern AggregateState *temporalinst_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalInst *inst, Datum (*operator)(Datum, Datum));
extern AggregateState *temporalinst_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*operator)(Datum, Datum));
extern AggregateState *temporalseq_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	TemporalSeq *seq, Datum (*operator)(Datum, Datum), bool interpoint);
extern AggregateState *temporalseq_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*operator)(Datum, Datum), bool interpoint);
	
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
