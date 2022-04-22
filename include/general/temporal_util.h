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
#include <catalog/pg_type.h>
#include <utils/array.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif
#include <utils/rangetypes.h>
/* PostgreSQL */
#include "general/temporal.h"
#include "point/postgis.h"

/*****************************************************************************/

/* Temporal/base types tests */

extern bool temporal_type(CachedType temptype);
extern void ensure_temporal_type(CachedType temptype);
extern void ensure_temporal_basetype(CachedType basetype);
extern bool temptype_continuous(CachedType temptype);
extern void ensure_temptype_continuous(CachedType temptype);
extern bool basetype_byvalue(CachedType basetype);
extern int16 basetype_length(CachedType basetype);
extern bool talpha_type(CachedType temptype);
extern bool tnumber_type(CachedType temptype);
extern void ensure_tnumber_type(CachedType temptype);
extern bool tnumber_basetype(CachedType basetype);
extern void ensure_tnumber_basetype(CachedType basetype);
extern bool tnumber_rangetype(CachedType rangetype);
extern void ensure_tnumber_rangetype(CachedType rangetype);
extern bool tspatial_type(CachedType temptype);
extern bool tspatial_basetype(CachedType basetype);
extern bool tgeo_basetype(CachedType basetype);
extern bool tgeo_type(CachedType basetype);
extern void ensure_tgeo_type(CachedType basetype);

/* Oid functions */

extern Oid basetype_rangeoid(CachedType basetype);
extern Oid basetype_oid(CachedType basetype);
extern Oid temptype_oid(CachedType temptype);

/* Miscellaneous functions */

extern size_t double_pad(size_t size);
extern Datum datum_copy(Datum value, Oid typid);
extern double datum_double(Datum d, CachedType basetype);
extern double tnumberinst_double(const TInstant *inst);
extern text *cstring2text(const char *cstring);
extern char *text2cstring(const text *textptr);

/* PostgreSQL call helpers */

extern Datum call_input(Oid typid, char *str);
extern char *call_output(Oid typid, Datum value);
extern bytea *call_send(Oid typid, Datum value);
extern Datum call_recv(Oid typid, StringInfo buf);
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

/* Array functions */

extern void pfree_array(void **array, int count);
extern void pfree_datumarr(Datum *array, int count);
extern char *stringarr_to_string(char **strings, int count, int outlen,
  char *prefix, char open, char close);
extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Period **periodarr_extract(ArrayType *array, int *count);
extern RangeType **rangearr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, CachedType type);
extern ArrayType *timestamparr_to_array(const TimestampTz *times, int count);
extern ArrayType *periodarr_to_array(const Period **periods, int count);
extern ArrayType *rangearr_to_array(RangeType **ranges, int count, CachedType type);
extern ArrayType *strarr_to_textarray(char **strarr, int count);
extern ArrayType *temporalarr_to_array(const Temporal **temporal, int count);
extern ArrayType *stboxarr_to_array(STBOX *boxarr, int count);

/* Sort functions */

extern void datumarr_sort(Datum *values, int count, CachedType basetype);
extern void timestamparr_sort(TimestampTz *times, int count);
extern void double2arr_sort(double2 *doubles, int count);
extern void double3arr_sort(double3 *triples, int count);
extern void periodarr_sort(Period **periods, int count);
extern void rangearr_sort(RangeType **ranges, int count);
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

extern bool datum_eq(Datum l, Datum r, CachedType type);
extern bool datum_ne(Datum l, Datum r, CachedType type);
extern bool datum_lt(Datum l, Datum r, CachedType type);
extern bool datum_le(Datum l, Datum r, CachedType type);
extern bool datum_gt(Datum l, Datum r, CachedType type);
extern bool datum_ge(Datum l, Datum r, CachedType type);

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

#endif
