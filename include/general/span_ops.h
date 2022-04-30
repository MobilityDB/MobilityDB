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

#ifndef __SPAN_OPS_H__
#define __SPAN_OPS_H__

/* MobilityDB */
#include "general/temp_catalog.h"
#include "general/span.h"

/*****************************************************************************/

/* Miscellaneous */

extern uint32_t time_max_header_size(void);
extern bool time_type(CachedType timetype);
extern void ensure_time_type(CachedType timetype);

/* contains? */

extern bool contains_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool contains_span_span(const Span *s1, const Span *s2);

/* contained? */

extern bool contained_span_span(const Span *s1, const Span *s2);

/* overlaps? */

extern bool overlaps_span_span(const Span *s1, const Span *s2);

/* left */

extern bool left_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool left_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool left_span_span(const Span *s1, const Span *s2);

/* right */

extern bool right_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool right_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool right_span_span(const Span *s1, const Span *s2);

/* overleft */

extern bool overleft_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overleft_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overleft_span_span(const Span *s1, const Span *s2);

/* overright */

extern bool overright_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overright_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overright_span_span(const Span *s1, const Span *s2);

/* adjacent */

extern bool adjacent_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool adjacent_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool adjacent_span_span(const Span *s1, const Span *s2);

/* union */

extern Span *union_span_span(const Span *s1, const Span *s2, bool strict);

/* intersection */

extern Span *intersection_span_span(const Span *s1, const Span *s2);

/* minus */

extern Span *minus_span_span(const Span *s1, const Span *s2);

/* Distance returning a float in seconds for use with indexes in
 * nearest neighbor searches */

extern double distance_elem_elem(Datum d1, Datum d2, CachedType basetype1,
  CachedType basetype2);
extern double distance_span_elem(const Span *s,  Datum d, CachedType basetype);
extern double distance_span_span(const Span *s1, const Span *s2);

/*****************************************************************************/

#endif
