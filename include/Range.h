/*****************************************************************************
 *
 * Range.h
 *	  Extension of operators for range types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __RANGE_H__
#define __RANGE_H__

#include <postgres.h>

/*****************************************************************************/

extern const char *range_to_string(RangeType *range);
extern Datum lower_datum(RangeType *range);
extern Datum upper_datum(RangeType *range);
extern bool lower_inc(RangeType *range);
extern bool upper_inc(RangeType *range);
extern RangeType *range_make(Datum from, Datum to, bool lower_inc, bool upper_inc, Oid subtypid);
extern RangeType **rangearr_normalize(RangeType **ranges, int *count);

extern Datum intrange_canonical(PG_FUNCTION_ARGS);

extern Datum range_left_elem(PG_FUNCTION_ARGS);
extern Datum range_overleft_elem(PG_FUNCTION_ARGS);
extern Datum range_right_elem(PG_FUNCTION_ARGS);
extern Datum range_overright_elem(PG_FUNCTION_ARGS);
extern Datum range_adjacent_elem(PG_FUNCTION_ARGS);

extern bool range_left_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_overleft_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_right_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_overright_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_adjacent_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);

extern Datum elem_left_range(PG_FUNCTION_ARGS);
extern Datum elem_overleft_range(PG_FUNCTION_ARGS);
extern Datum elem_right_range(PG_FUNCTION_ARGS);
extern Datum elem_overright_range(PG_FUNCTION_ARGS);
extern Datum elem_adjacent_range(PG_FUNCTION_ARGS);

extern bool elem_left_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_overleft_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_right_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_overright_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_adjacent_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);

/*****************************************************************************/

#endif
