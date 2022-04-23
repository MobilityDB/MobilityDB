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
 * @file time_ops.h
 * Operators for time types.
 */

#ifndef __TIME_OPS_H__
#define __TIME_OPS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/span.h"
#include "general/timetypes.h"

/*****************************************************************************/

/**
 * Enumeration for the relative position of a given element into a skiplist
 */
typedef enum
{
  BEFORE,
  DURING,
  AFTER
} RelativePos;

/*****************************************************************************/

/* Miscellaneous */

extern uint32_t time_max_header_size(void);
extern bool time_type(CachedType timetype);
extern void ensure_time_type(CachedType timetype);

/* Functions for aggregations */

extern RelativePos pos_elem_elem(Datum d1, Datum d2);
extern RelativePos pos_span_elem(const Span *s, Datum d);

/* contains? */

extern bool contains_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool contains_span_span(const Span *s1, const Span *s2);

/* contained? */

extern bool contained_span_span(const Span *s1, const Span *s2);

/* overlaps? */

extern bool overlaps_span_span(const Span *s1, const Span *s2);

/* before */

extern bool before_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool before_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool before_span_span(const Span *s1, const Span *s2);

/* after */

extern bool after_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool after_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool after_span_span(const Span *s1, const Span *s2);

/* overbefore */

extern bool overbefore_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overbefore_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overbefore_span_span(const Span *s1, const Span *s2);

/* overafter */

extern bool overafter_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overafter_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overafter_span_span(const Span *s1, const Span *s2);

/* adjacent */

extern bool adjacent_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool adjacent_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool adjacent_span_span(const Span *s1, const Span *s2);

/* union */

extern Span *union_elem_span(Datum d, CachedType basetype, const Span *s);
extern Span *union_span_elem(const Span *s, Datum d, CachedType basetype);
extern Span *union_span_span(const Span *s1, const Span *s2);

/* intersection */

// extern bool intersection_elem_elem(Datum d1, Datum d2, Datum *result);
extern bool intersection_span_elem(const Span *s, Datum d, CachedType basetype,
  Datum *result);
extern Span *intersection_span_span(const Span *s1, const Span *s2);

/* minus */

extern bool minus_elem_elem(Datum d1, Datum d2, Datum *result);
extern Span *minus_span_elem(const Span *s, Datum d, CachedType basetype);
extern Span *minus_span_span(const Span *s1, const Span *s2);

/* Distance returning a float in seconds for use with indexes in
 * nearest neighbor searches */

extern double distance_elem_elem(Datum d1, Datum d2, CachedType basetype1,
  CachedType basetype2);
extern double distance_span_elem(const Span *s,  Datum d, CachedType basetype);
extern double distance_span_span(const Span *s1, const Span *s2);

/*****************************************************************************/

#endif
