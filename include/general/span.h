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
 * @file span.h
 * Basic routines for spans (a.k.a. ranges) composed of two `Datum` values and
 * two Boolean values stating whether the bounds are inclusive or not.
 */

#ifndef __SPAN_H__
#define __SPAN_H__

/* PostgreSQL */
#include <postgres.h>
#include <lib/stringinfo.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/temporal_catalog.h"

/*****************************************************************************/

/**
 * Structure to represent spans (a.k.a. ranges)
 */
typedef struct
{
  Datum lower;          /**< lower bound value */
  Datum upper;          /**< upper bound value */
  bool lower_inc;       /**< lower bound is inclusive (vs exclusive) */
  bool upper_inc;       /**< upper bound is inclusive (vs exclusive) */
  uint8 spantype;       /**< span type */
  uint8 basetype;       /**< span basetype */
} Span;

/**
 * Make the Period type as a specialized Span type for faster manipulation
 * of the time dimension
 */
typedef Span Period;

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
#define PG_GETARG_SPAN_P(X)        DatumGetSpanP(PG_GETARG_POINTER(X))
#define PG_RETURN_SPAN_P(X)        PG_RETURN_POINTER(X)

/*****************************************************************************/

/* General functions */

extern void span_deserialize(const Span *s, SpanBound *lower,
  SpanBound *upper);
extern Span *span_serialize(SpanBound *lower, SpanBound *upper);
extern int span_bound_cmp(const SpanBound *b1, const SpanBound *b2);
extern int span_bound_qsort_cmp(const void *a1, const void *a2);
extern int span_lower_cmp(const Span *a, const Span *b);
extern int span_upper_cmp(const Span *a, const Span *b);
extern Span **spanarr_normalize(Span **spans, int count, int *newcount);
extern void span_bounds(const Span *s, double *xmin, double *xmax);

/* Input/output functions */

extern void span_write(const Span *s, StringInfo buf);

/* Transformation functions */

extern Span *floatspan_round(Span *span, Datum size);

/*****************************************************************************/

#endif
