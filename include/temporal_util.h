/*****************************************************************************
 *
 * temporal_util.c
 *    Miscellaneous utility functions for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_UTIL_H__
#define __TEMPORAL_UTIL_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/array.h>
#if MOBDB_PGSQL_VERSION >= 120000
#include <utils/float.h>
#endif
#include <utils/rangetypes.h>

#include "temporal.h"
#include "postgis.h"

/*****************************************************************************/

/* Miscellaneous functions */

extern void _PG_init(void);
extern size_t double_pad(size_t size);
extern bool get_typbyval_fast(Oid type);
extern int get_typlen_fast(Oid type);
extern Datum datum_copy(Datum value, Oid type);
extern double datum_double(Datum d, Oid valuetypid);

/* PostgreSQL call helpers */

extern Datum call_input(Oid type, char *str);
extern char *call_output(Oid type, Datum value);
extern bytea *call_send(Oid type, Datum value);
extern Datum call_recv(Oid type, StringInfo buf);
extern Datum call_function1(PGFunction func, Datum arg1);
extern Datum call_function2(PGFunction func, Datum arg1, Datum arg2);
extern Datum call_function3(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3);
extern Datum call_function4(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3, Datum arg4);

extern Datum CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo, 
  Oid collation, Datum arg1, Datum arg2, Datum arg3, Datum arg4);

/* Array functions */

extern char *stringarr_to_string(char **strings, int count, int outlen, 
  char *prefix, char open, char close);
extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Period **periodarr_extract(ArrayType *array, int *count);
extern RangeType **rangearr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, Oid type);
extern ArrayType *timestamparr_to_array(TimestampTz *times, int count);
extern ArrayType *periodarr_to_array(Period **periods, int count);
extern ArrayType *rangearr_to_array(RangeType **ranges, int count, Oid type,
  bool free);
extern ArrayType *textarr_to_array(text **textarr, int count, bool free);
extern ArrayType *temporalarr_to_array(Temporal **tsequenceset, int count);
extern ArrayType *stboxarr_to_array(STBOX *boxarr, int count);

/* Sort functions */

extern void datumarr_sort(Datum *values, int count, Oid valuetypid);
extern void timestamparr_sort(TimestampTz *times, int count);
extern void periodarr_sort(Period **periods, int count);
extern void rangearr_sort(RangeType **ranges, int count);
extern void tinstantarr_sort(TInstant **instants, int count);
extern void tsequencearr_sort(TSequence **sequences, int count);

/* Remove duplicate functions */

extern int datumarr_remove_duplicates(Datum *values, int count,
  Oid valuetypid);
extern int timestamparr_remove_duplicates(TimestampTz *values, int count);
extern int tinstantarr_remove_duplicates(TInstant **instants, int count);

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

/* Hypothenuse functions */

extern double hypot3d(double x, double y, double z);
extern double hypot4d(double x, double y, double z, double m);

/*****************************************************************************/

#endif
