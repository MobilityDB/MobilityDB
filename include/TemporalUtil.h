/*****************************************************************************
 *
 * TemporalUtil.c
 *	  Miscellaneous utility functions for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALUTIL_H__
#define __TEMPORALUTIL_H__

#include <postgres.h>

/*****************************************************************************/

/* Miscellaneous functions */

extern void _PG_init(void);
extern void debugstr(char *msg);
extern size_t double_pad(size_t size);
extern bool type_byval_fast(Oid type);
extern int get_typlen_fast(Oid type);
extern Datum datum_copy(Datum value, Oid type);
extern double datum_double(Datum d, Oid valuetypid);

/* Assertion tests */

extern void temporal_duration_is_valid(int16 type);
extern void numrange_type_oid(Oid type);
extern void base_type_oid(Oid valuetypid);
extern void base_type_all_oid(Oid valuetypid);
extern void continuous_base_type_oid(Oid valuetypid);
extern void continuous_base_type_all_oid(Oid valuetypid);
extern void numeric_base_type_oid(Oid type);
extern void point_base_type_oid(Oid type);

/* PostgreSQL call helpers */

extern Datum call_input(Oid type, char *str);
extern char *call_output(Oid type, Datum value);
extern bytea *call_send(Oid type, Datum value);
extern Datum call_recv(Oid type, StringInfo buf);
extern Datum call_function1(PGFunction func, Datum op);
extern Datum call_function2(PGFunction func, Datum lop, Datum rop);
extern Datum call_function3(PGFunction func, Datum lop, Datum rop, Datum param);
extern Datum call_function4(PGFunction func, Datum lop, Datum rop, Datum param1, Datum param2);

/* Array functions */

extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Period **periodarr_extract(ArrayType *array, int *count);
extern RangeType **rangearr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, Oid type);
extern ArrayType *timestamparr_to_array(TimestampTz *times, int count);
extern ArrayType *periodarr_to_array(Period **periods, int count);
extern ArrayType *rangearr_to_array(RangeType **ranges, int count, Oid type);
extern ArrayType *textarr_to_array(text **textarr, int count);
extern ArrayType *temporalarr_to_array(Temporal **temporals, int count);
 
/* Sort functions */

extern void datum_sort(Datum *values, int count, Oid valuetypid);
extern void timestamp_sort(TimestampTz *values, int count);
extern void periodarr_sort(Period **periods, int count);
extern void rangearr_sort(RangeType **ranges, int count);
extern void temporalinstarr_sort(TemporalInst **instants, int count);
extern void temporalseqarr_sort(TemporalSeq **sequences, int count);

/* Remove duplicate functions */

extern int datum_remove_duplicates(Datum *values, int count, Oid valuetypid);
extern int timestamp_remove_duplicates(TimestampTz *values, int count);

/* Text functions */

extern int text_cmp(text *arg1, text *arg2, Oid collid);

/* Comparison functions on datums */

extern bool datum_eq(Datum l, Datum r, Oid type);
extern bool datum_ne(Datum l, Datum r, Oid type);
extern bool datum_lt(Datum l, Datum r, Oid type);
extern bool datum_le(Datum l, Datum r, Oid type);
extern bool datum_gt(Datum l, Datum r, Oid type);
extern bool datum_ge(Datum l, Datum r, Oid type);

extern bool datum_eq2(Datum l, Datum r, Oid typel, Oid typer); 
extern bool datum_ne2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_lt2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_le2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_gt2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_ge2(Datum l, Datum r, Oid typel, Oid typer);

extern Datum datum2_eq2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_ne2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_lt2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_le2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_gt2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_ge2(Datum l, Datum r, Oid typel, Oid typer);

/*****************************************************************************/

#endif
