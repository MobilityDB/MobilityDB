/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @file temporal_util.c
 * Miscellaneous utility functions for temporal types.
 */

#ifndef __TEMPORAL_UTIL_H__
#define __TEMPORAL_UTIL_H__

/* PostgreSQL */
#include <postgres.h>
/* PostgreSQL */
#include "general/temporal.h"
#include "general/span.h"

/*****************************************************************************/

/* Miscellaneous functions */

extern size_t double_pad(size_t size);
extern Datum datum_copy(Datum value, Oid typid);
extern double datum_double(Datum d, CachedType basetype);
extern text *cstring2text(const char *cstring);
extern char *text2cstring(const text *textptr);

/* Input/output functions */

extern Datum basetype_input(CachedType type, char *str);
extern char *basetype_output(CachedType type, Datum value);

/* Array functions */

extern void pfree_array(void **array, int count);
extern void pfree_datumarr(Datum *array, int count);
extern char *stringarr_to_string(char **strings, int count, int outlen,
  char *prefix, char open, char close);

/* Sort functions */

extern void datumarr_sort(Datum *values, int count, CachedType basetype);
extern void timestamparr_sort(TimestampTz *times, int count);
extern void double2arr_sort(double2 *doubles, int count);
extern void double3arr_sort(double3 *triples, int count);
extern void spanarr_sort(Span **spans, int count);
extern void tinstarr_sort(TInstant **instants, int count);
extern void tseqarr_sort(TSequence **sequences, int count);

/* Remove duplicate functions */

extern int datumarr_remove_duplicates(Datum *values, int count,
  CachedType basetype);
extern int timestamparr_remove_duplicates(TimestampTz *values, int count);
extern int tinstarr_remove_duplicates(const TInstant **instants, int count);

/* Text functions */

extern int text_cmp(text *arg1, text *arg2, Oid collid);

/* Arithmetic functions */

extern Datum datum_add(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum_sub(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum_mult(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum_div(Datum l, Datum r, CachedType typel, CachedType typer);

/* Comparison functions on datums */

extern int datum_cmp(Datum l, Datum r, CachedType type);
extern bool datum_eq(Datum l, Datum r, CachedType type);
extern bool datum_ne(Datum l, Datum r, CachedType type);
extern bool datum_lt(Datum l, Datum r, CachedType type);
extern bool datum_le(Datum l, Datum r, CachedType type);
extern bool datum_gt(Datum l, Datum r, CachedType type);
extern bool datum_ge(Datum l, Datum r, CachedType type);

extern int datum_cmp2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_eq2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_ne2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_lt2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_le2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_gt2(Datum l, Datum r, CachedType typel, CachedType typer);
extern bool datum_ge2(Datum l, Datum r, CachedType typel, CachedType typer);

extern Datum datum2_eq2(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum2_ne2(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum2_lt2(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum2_le2(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum2_gt2(Datum l, Datum r, CachedType typel, CachedType typer);
extern Datum datum2_ge2(Datum l, Datum r, CachedType typel, CachedType typer);

/* Hypothenuse functions */

extern double hypot3d(double x, double y, double z);
extern double hypot4d(double x, double y, double z, double m);

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

#include <utils/array.h>
#include <utils/rangetypes.h>

/* PostgreSQL call helpers */

extern Datum call_input(Oid typid, char *str);
extern char *call_output(Oid typid, Datum value);
extern Datum call_recv(Oid typid, StringInfo buf);
extern bytea *call_send(Oid typid, Datum value);

extern Datum call_function1(PGFunction func, Datum arg1);
extern Datum call_function2(PGFunction func, Datum arg1, Datum arg2);
extern Datum call_function3(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3);
extern Datum call_function4(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3, Datum arg4);

extern Datum CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo,
  Oid collid, Datum arg1, Datum arg2, Datum arg3, Datum arg4);

extern Datum CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo,
    Oid collid, Datum arg1, Datum arg2, Datum arg3, Datum arg4);

/* Input/output functions */

extern Datum basetype_recv(CachedType type, StringInfo buf);
extern bytea *basetype_send(CachedType type, Datum value);

/* Range functions */

extern RangeType *range_make(Datum from, Datum to, bool lower_inc,
  bool upper_inc, CachedType basetype);

/* Array functions */

extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Period **periodarr_extract(ArrayType *array, int *count);
extern Span **spanarr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, CachedType type);
extern ArrayType *timestamparr_to_array(const TimestampTz *times, int count);
extern ArrayType *periodarr_to_array(const Period **periods, int count);
extern ArrayType *spanarr_to_array(Span **spans, int count);
extern ArrayType *strarr_to_textarray(char **strarr, int count);
extern ArrayType *temporalarr_to_array(const Temporal **temporal, int count);
extern ArrayType *stboxarr_to_array(STBOX *boxarr, int count);

#endif /* #if ! MEOS */

/*****************************************************************************/

#endif
