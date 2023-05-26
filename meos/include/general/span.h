/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Basic routines for spans (a.k.a. ranges) composed of two `Datum`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#ifndef __SPAN_H__
#define __SPAN_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

/*****************************************************************************/

/**
 * Internal representation of either bound of a span (not what's on disk)
 */
typedef struct
{
  Datum val;            /**< bound value */
  bool inclusive;       /**< bound is inclusive (vs exclusive) */
  bool lower;           /**< this is the lower (vs upper) bound */
  uint8 spantype;       /**< span type */
  uint8 basetype;       /**< span basetype */
} SpanBound;

/*
 * fmgr macros for span types
 */

#define DatumGetSpanP(X)           ((Span *) DatumGetPointer(X))
#define SpanPGetDatum(X)           PointerGetDatum(X)
#define PG_GETARG_SPAN_P(X)        DatumGetSpanP(PG_GETARG_DATUM(X))
#define PG_RETURN_SPAN_P(X)        PG_RETURN_POINTER(X)

#if MEOS
  #define DatumGetSpanSetP(X)      ((SpanSet *) DatumGetPointer(X))
#else
  #define DatumGetSpanSetP(X)      ((SpanSet *) PG_DETOAST_DATUM(X))
#endif /* MEOS */
#define SpanSetPGetDatum(X)        PointerGetDatum(X)
#define PG_GETARG_SPANSET_P(X)     ((SpanSet *) PG_GETARG_VARLENA_P(X))
#define PG_RETURN_SPANSET_P(X)     PG_RETURN_POINTER(X)

/*****************************************************************************/

/* General functions */

extern void span_deserialize(const Span *s, SpanBound *lower,
  SpanBound *upper);
extern Span *span_serialize(SpanBound *lower, SpanBound *upper);
extern int span_bound_cmp(const SpanBound *b1, const SpanBound *b2);
extern int span_bound_qsort_cmp(const void *a1, const void *a2);
extern int span_lower_cmp(const Span *a, const Span *b);
extern int span_upper_cmp(const Span *a, const Span *b);
extern Span *spanarr_normalize(Span *spans, int count, bool sort,
  int *newcount);
extern void span_bounds(const Span *s, double *xmin, double *xmax);
extern void lower_upper_shift_tscale(const Interval *shift,
  const Interval *duration, TimestampTz *lower, TimestampTz *upper);
extern void period_delta_scale(Span *p, TimestampTz origin, TimestampTz delta,
  double scale);
extern void period_shift_tscale1(Span *p, const Interval *shift,
  const Interval *duration, TimestampTz *delta, double *scale);

extern size_t span_to_wkb_size(const Span *s);
extern uint8_t *span_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant);

extern int minus_span_span_iter(const Span *s1, const Span *s2, Span *result);
extern int minus_span_value_iter(const Span *s, Datum d, meosType basetype,
  Span *result);

/*****************************************************************************/

#endif /* __SPAN_H__ */
