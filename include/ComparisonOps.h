/*****************************************************************************
 *
 * ComparisonOps.h
 *	  Temporal comparison operators (=, <>, <, >, <=, >=).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __COMPARISONOPS_H__
#define __COMPARISONOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum teq_base_temporal(PG_FUNCTION_ARGS);
extern Datum teq_temporal_base(PG_FUNCTION_ARGS);
extern Datum teq_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tne_base_temporal(PG_FUNCTION_ARGS);
extern Datum tne_temporal_base(PG_FUNCTION_ARGS);
extern Datum tne_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tlt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tle_base_temporal(PG_FUNCTION_ARGS);
extern Datum tle_temporal_base(PG_FUNCTION_ARGS);
extern Datum tle_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tgt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tge_base_temporal(PG_FUNCTION_ARGS);
extern Datum tge_temporal_base(PG_FUNCTION_ARGS);
extern Datum tge_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
