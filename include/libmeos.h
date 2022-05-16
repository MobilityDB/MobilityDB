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
 * @file libmeso.h
 * API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __LIBMEOS_H__
#define __LIBMEOS_H__

/* JSON-C */
#include <json-c/json.h>
/* MobilityDB */
#include "general/span.h"
#include "general/timetypes.h"
#include "general/tbox.h"
#include "general/temporal.h"
#include "point/stbox.h"


/*****************************************************************************
 * Functions for span and time types
 *****************************************************************************/

/* Input/output functions for span and time types */

extern PeriodSet *periodset_in(char *str);
extern char *periodset_out(const PeriodSet *ps);
extern PeriodSet *periodset_recv(StringInfo buf);
extern bytea *periodset_send(const PeriodSet *ps);
extern Span *span_in(char *str, CachedType spantype);
extern char *span_out(const Span *s);
extern Span *span_recv(StringInfo buf);
extern bytea *span_send(const Span *s);
extern TimestampSet *timestampset_in(char *str);
extern char *timestampset_out(const TimestampSet *ts);
extern TimestampSet *timestampset_recv(StringInfo buf);
extern bytea *timestampset_send(const TimestampSet *ts);

/*****************************************************************************/

/* Constructor functions for span and time types */

extern PeriodSet *periodset_make(const Period **periods, int count, bool normalize);
extern PeriodSet *periodset_make_free(Period **periods, int count, bool normalize);
extern PeriodSet *periodset_copy(const PeriodSet *ps);
extern Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, CachedType basetype);
extern void span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc, CachedType basetype, Span *s);
extern Span *span_copy(const Span *s);
extern TimestampSet *timestampset_make(const TimestampTz *times, int count);
extern TimestampSet *timestampset_make_free(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(const TimestampSet *ts);

/*****************************************************************************/

/* Cast functions for span and time types */

extern PeriodSet *timestamp_to_periodset (TimestampTz t);
extern PeriodSet *timestampset_to_periodset(const TimestampSet *ts);
extern PeriodSet *period_to_periodset(const Period *period);
extern Span *elem_to_span(Datum d, CachedType basetype);
extern Period *timestamp_to_period(TimestampTz t);
extern TimestampSet *timestamp_to_timestampset(TimestampTz t);

/*****************************************************************************/

/* Accessor functions for span and time types */

extern const Period *periodset_per_n(const PeriodSet *ps, int index);
extern int periodset_mem_size(const PeriodSet *ps);
extern Interval *periodset_timespan(const PeriodSet *ps);
extern void periodset_set_period(const PeriodSet *ps, Period *p);
extern Interval *periodset_duration(const PeriodSet *ps);
extern int periodset_num_periods(const PeriodSet *ps);
extern Period *periodset_start_period(const PeriodSet *ps);
extern Period *periodset_end_period(const PeriodSet *ps);
extern Period *periodset_period_n(const PeriodSet *ps, int i);
extern const Period **periodset_periods(const PeriodSet *ps);
extern int periodset_num_timestamps(const PeriodSet *ps);
extern TimestampTz periodset_start_timestamp(const PeriodSet *ps);
extern TimestampTz periodset_end_timestamp(const PeriodSet *ps);
extern bool periodset_timestamp_n(const PeriodSet *ps, int n, TimestampTz *result);
extern TimestampTz *periodset_timestamps(const PeriodSet *ps, int *count);
extern uint32 periodset_hash(const PeriodSet *ps);
extern uint64 periodset_hash_extended(const PeriodSet *ps, uint64 seed);
extern Datum span_lower(Span *s);
extern Datum span_upper(Span *s);
extern bool span_lower_inc(Span *s);
extern bool span_upper_inc(Span *s);
extern double span_width(const Span *s);
extern uint32 span_hash(const Span *s);
extern uint64 span_hash_extended(const Span *s, uint64 seed);
extern Interval *period_duration(const Span *s);
extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index);
extern int timestampset_mem_size(const TimestampSet *ts);
extern Interval *timestampset_timespan(const TimestampSet *ts);
extern void timestampset_set_period(const TimestampSet *ts, Period *p);
extern int timestampset_num_timestamps(const TimestampSet *ts);
extern TimestampTz timestampset_start_timestamp(const TimestampSet *ts);
extern TimestampTz timestampset_end_timestamp(const TimestampSet *ts);
extern bool timestampset_timestamp_n(const TimestampSet *ts, int n, TimestampTz *result);
extern TimestampTz *timestampset_timestamps(const TimestampSet *ts);
extern uint32 timestampset_hash(const TimestampSet *ts);
extern uint64 timestampset_hash_extended(const TimestampSet *ts, uint64 seed);

/*****************************************************************************/

/* Transformation functions for span and time types */

extern PeriodSet *periodset_shift_tscale(const PeriodSet *ps, const Interval *start, const Interval *duration);
extern void span_expand(const Span *s1, Span *s2);
extern void period_shift_tscale(const Interval *start, const Interval *duration, Period *result);
extern TimestampSet *timestampset_shift_tscale(const TimestampSet *ts, const Interval *start, const Interval *duration);

/*****************************************************************************
 * Bounding box functions for span and time types
 *****************************************************************************/

/* Topological functions for span and time types */

extern bool contains_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool contains_span_span(const Span *s1, const Span *s2);
extern bool contained_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool contained_span_span(const Span *s1, const Span *s2);
extern bool overlaps_span_span(const Span *s1, const Span *s2);
extern bool adjacent_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool adjacent_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool adjacent_span_span(const Span *s1, const Span *s2);
extern bool contains_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool contains_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool contains_period_timestamp(const Period *p, TimestampTz t);
extern bool contains_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool contains_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool contains_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool contains_periodset_period(const PeriodSet *ps, const Period *p);
extern bool contains_period_periodset(const Period *p, const PeriodSet *ps);
extern bool contains_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool contained_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool contained_timestamp_period(TimestampTz t, const Period *p);
extern bool contained_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool contained_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool contained_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool contained_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool contained_period_periodset(const Period *p, const PeriodSet *ps);
extern bool contained_periodset_period(const PeriodSet *ps, const Period *p);
extern bool contained_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overlaps_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overlaps_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overlaps_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overlaps_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overlaps_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overlaps_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overlaps_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overlaps_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool adjacent_timestamp_period(TimestampTz t, const Period *p);
extern bool adjacent_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool adjacent_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool adjacent_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool adjacent_period_timestamp(const Period *p, TimestampTz t);
extern bool adjacent_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool adjacent_period_periodset(const Period *p, const PeriodSet *ps);
extern bool adjacent_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool adjacent_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool adjacent_periodset_period(const PeriodSet *ps, const Period *p);
extern bool adjacent_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);

/*****************************************************************************/

/* Position functions for span and time types */

extern bool left_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool left_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool left_span_span(const Span *s1, const Span *s2);
extern bool right_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool right_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool right_span_span(const Span *s1, const Span *s2);
extern bool overleft_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overleft_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overleft_span_span(const Span *s1, const Span *s2);
extern bool overright_elem_span(Datum d, CachedType basetype, const Span *s);
extern bool overright_span_elem(const Span *s, Datum d, CachedType basetype);
extern bool overright_span_span(const Span *s1, const Span *s2);
extern bool before_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool before_timestamp_period(TimestampTz t, const Period *p);
extern bool before_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool before_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool before_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool before_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool before_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool before_period_timestamp(const Period *p, TimestampTz t);
extern bool before_period_periodset(const Period *p, const PeriodSet *ps);
extern bool before_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool before_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool before_periodset_period(const PeriodSet *ps, const Period *p);
extern bool before_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool after_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool after_timestamp_period(TimestampTz t, const Period *p);
extern bool after_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool after_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool after_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool after_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool after_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool after_period_timestamp(const Period *p, TimestampTz t);
extern bool after_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool after_period_periodset(const Period *p, const PeriodSet *ps);
extern bool after_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool after_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool after_periodset_period(const PeriodSet *ps, const Period *p);
extern bool after_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overbefore_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool overbefore_timestamp_period(TimestampTz t, const Period *p);
extern bool overbefore_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool overbefore_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool overbefore_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overbefore_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overbefore_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overbefore_period_timestamp(const Period *p, TimestampTz t);
extern bool overbefore_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overbefore_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overbefore_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool overbefore_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overbefore_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overbefore_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overafter_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool overafter_timestamp_period(TimestampTz t, const Period *p);
extern bool overafter_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool overafter_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool overafter_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overafter_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overafter_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overafter_period_timestamp(const Period *p, TimestampTz t);
extern bool overafter_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overafter_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overafter_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool overafter_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overafter_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overafter_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);

/*****************************************************************************/

/* Set functions for span and time types */

extern Span *union_span_span(const Span *s1, const Span *s2, bool strict);
extern bool inter_span_span(const Span *s1, const Span *s2, Span *result);
extern Span *intersection_span_span(const Span *s1, const Span *s2);
extern Span *minus_span_span(const Span *s1, const Span *s2);
extern TimestampSet *union_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern TimestampSet *union_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern PeriodSet *union_timestamp_period(TimestampTz t, const Period *p);
extern PeriodSet *union_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern TimestampSet *union_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t);
extern TimestampSet *union_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern PeriodSet *union_timestampset_period(const TimestampSet *ts, const Period *p);
extern PeriodSet *union_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern PeriodSet *union_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *union_period_timestampset(const Period *p, const TimestampSet *ts);
extern PeriodSet *union_period_period(const Period *p1, const Period *p2);
extern PeriodSet *union_period_periodset(const Period *p, const PeriodSet *ps);
extern PeriodSet *union_periodset_timestamp(PeriodSet *ps, TimestampTz t);
extern PeriodSet *union_periodset_timestampset(PeriodSet *ps, TimestampSet *ts);
extern PeriodSet *union_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *union_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool intersection_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result);
extern bool intersection_timestamp_timestampset(TimestampTz t, const TimestampSet *ts, TimestampTz *result);
extern bool intersection_timestamp_period(TimestampTz t, const Period *p, TimestampTz *result);
extern bool intersection_timestamp_periodset(TimestampTz t, const PeriodSet *ps, TimestampTz *result);
extern bool intersection_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern TimestampSet *intersection_timestampset_period(const TimestampSet *ts, const Period *p);
extern TimestampSet *intersection_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool intersection_period_timestamp(const Period *p, TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_period_timestampset(const Period *ps, const TimestampSet *ts);
extern PeriodSet *intersection_period_periodset(const Period *p, const PeriodSet *ps);
extern bool intersection_periodset_timestamp(const PeriodSet *ps, TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern PeriodSet *intersection_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *intersection_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool minus_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result);
extern bool minus_timestamp_timestampset(TimestampTz t, const TimestampSet *ts, TimestampTz *result);
extern bool minus_timestamp_period(TimestampTz t, const Period *p, TimestampTz *result);
extern bool minus_timestamp_periodset(TimestampTz t, const PeriodSet *ps, TimestampTz *result);
extern TimestampSet *minus_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern TimestampSet *minus_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern TimestampSet *minus_timestampset_period(const TimestampSet *ts, const Period *p);
extern TimestampSet *minus_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern PeriodSet *minus_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *minus_period_timestampset(const Period *p, const TimestampSet *ts);
extern PeriodSet *minus_period_period(const Period *p1, const Period *p2);
extern PeriodSet *minus_period_periodset(const Period *p, const PeriodSet *ps);
extern PeriodSet *minus_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern PeriodSet *minus_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern PeriodSet *minus_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *minus_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);

/*****************************************************************************/

/* Distance functions for span and time types */

extern double distance_elem_elem(Datum l, Datum r, CachedType typel, CachedType typer);
extern double distance_elem_span(Datum d, CachedType basetype, const Span *s);
extern double distance_span_elem(const Span *s, Datum d, CachedType basetype);
extern double distance_span_span(const Span *s1, const Span *s2);
extern double distance_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern double distance_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern double distance_timestamp_period(TimestampTz t, const Period *p);
extern double distance_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern double distance_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern double distance_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern double distance_timestampset_period(const TimestampSet *ts, const Period *p);
extern double distance_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern double distance_period_timestamp(const Period *p, TimestampTz t);
extern double distance_period_timestampset(const Period *p, const TimestampSet *ts);
extern double distance_period_periodset(const Period *p, const PeriodSet *ps);
extern double distance_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern double distance_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern double distance_periodset_period(const PeriodSet *ps, const Period *p);
extern double distance_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);

/*****************************************************************************/

/* Comparison functions for span and time types */

extern bool periodset_eq(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_ne(const PeriodSet *ps1, const PeriodSet *ps2);
extern int periodset_cmp(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_lt(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_le(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_ge(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_gt(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool span_eq(const Span *s1, const Span *s2);
extern bool span_ne(const Span *s1, const Span *s2);
extern int span_cmp(const Span *s1, const Span *s2);
extern bool span_lt(const Span *s1, const Span *s2);
extern bool span_le(const Span *s1, const Span *s2);
extern bool span_ge(const Span *s1, const Span *s2);
extern bool span_gt(const Span *s1, const Span *s2);
extern bool timestampset_eq(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ne(const TimestampSet *ts1, const TimestampSet *ts2);
extern int timestampset_cmp(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_lt(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_le(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ge(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_gt(const TimestampSet *ts1, const TimestampSet *ts2);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */

extern TBOX *tbox_in(char *str);
extern char *tbox_out(const TBOX *box);
extern TBOX *tbox_recv(StringInfo buf);
extern bytea *tbox_send(TBOX *box);
extern STBOX *stbox_in(char *str);
extern char *stbox_out(const STBOX *box);
extern STBOX *stbox_recv(StringInfo buf);
extern bytea *stbox_send(STBOX *box);

/*****************************************************************************/

/* Constructor functions for box types */

extern TBOX *tbox_make(bool hasx, bool hast, double xmin, double xmax, TimestampTz tmin, TimestampTz tmax);
extern void tbox_set(bool hasx, bool hast, double xmin, double xmax, TimestampTz tmin, TimestampTz tmax, TBOX *box);
extern TBOX *tbox_copy(const TBOX *box);
extern STBOX *stbox_make(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, TimestampTz tmin, TimestampTz tmax);
extern void stbox_set(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, TimestampTz tmin, TimestampTz tmax, STBOX *box);
extern STBOX *stbox_copy(const STBOX *box);

/*****************************************************************************/

/* Cast functions for box types */

extern void number_set_tbox(Datum value, CachedType basetype, TBOX *box);
extern void int_set_tbox(int i, TBOX *box);
extern void float_set_tbox(double d, TBOX *box);
extern void span_set_tbox(const Span *span, TBOX *box);
extern void timestamp_set_tbox(TimestampTz t, TBOX *box);
extern void timestampset_set_tbox(const TimestampSet *ts, TBOX *box);
extern void period_set_tbox(const Period *p, TBOX *box);
extern void periodset_set_tbox(const PeriodSet *ps, TBOX *box);
extern TBOX *int_timestamp_to_tbox(int i, TimestampTz t);
extern TBOX *float_timestamp_to_tbox(double d, TimestampTz t);
extern TBOX *int_period_to_tbox(int i, const Period *p);
extern TBOX *float_period_to_tbox(double d, const Period *p);
extern TBOX *span_timestamp_to_tbox(const Span *span, TimestampTz t);
extern TBOX *span_period_to_tbox(const Span *span, const Period *p);
extern Span *tbox_to_intspan(const TBOX *box);
extern Span *tbox_to_floatspan(const TBOX *box);
extern Period *tbox_to_period(const TBOX *box);
extern TBOX *tnumber_to_tbox(Temporal *temp);
extern void stbox_set_gbox(const STBOX *box, GBOX *gbox);
extern void stbox_set_box3d(const STBOX *box, BOX3D *box3d);
extern Datum stbox_to_geometry(const STBOX *box);
extern bool geo_set_stbox(const GSERIALIZED *gs, STBOX *box);
extern void timestamp_set_stbox(TimestampTz t, STBOX *box);
extern void timestampset_set_stbox(const TimestampSet *ts, STBOX *box);
extern void period_set_stbox(const Period *p, STBOX *box);
extern void periodset_set_stbox(const PeriodSet *ps, STBOX *box);
extern STBOX *geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t);
extern STBOX *geo_period_to_stbox(const GSERIALIZED *gs, const Period *p);

/*****************************************************************************/

/* Accessor functions for box types */

extern bool tbox_hasx(const TBOX *box);
extern bool tbox_hast(const TBOX *box);
extern bool tbox_xmin(const TBOX *box, double *result);
extern bool tbox_xmax(const TBOX *box, double *result);
extern bool tbox_tmin(const TBOX *box, TimestampTz *result);
extern bool tbox_tmax(const TBOX *box, TimestampTz *result);
extern bool stbox_hasx(const STBOX *box);
extern bool stbox_hasz(const STBOX *box);
extern bool stbox_hast(const STBOX *box);
extern bool stbox_isgeodetic(const STBOX *box);
extern bool stbox_xmin(const STBOX *box, double *result);
extern bool stbox_xmax(const STBOX *box, double *result);
extern bool stbox_ymin(const STBOX *box, double *result);
extern bool stbox_ymax(const STBOX *box, double *result);
extern bool stbox_zmin(const STBOX *box, double *result);
extern bool stbox_zmax(const STBOX *box, double *result);
extern bool stbox_tmin(const STBOX *box, TimestampTz *result);
extern bool stbox_tmax(const STBOX *box, TimestampTz *result);
extern int32 stbox_get_srid(const STBOX *box);

/*****************************************************************************/

/* Transformation functions for box types */

extern void tbox_expand(const TBOX *box1, TBOX *box2);
extern void tbox_shift_tscale(const Interval *start, const Interval *duration, TBOX *box);
extern TBOX *tbox_expand_value(const TBOX *box, const double d);
extern TBOX *tbox_expand_temporal(const TBOX *box, const Interval *interval);
extern void stbox_expand(const STBOX *box1, STBOX *box2);
extern void stbox_shift_tscale(const Interval *start, const Interval *duration, STBOX *box);
extern STBOX *stbox_set_srid(const STBOX *box, int32 srid);
extern STBOX *stbox_expand_spatial(const STBOX *box, double d);
extern STBOX *stbox_expand_temporal(const STBOX *box, const Interval *interval);

/*****************************************************************************/

/* Topological functions for box types */

extern bool contains_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool contained_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overlaps_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool same_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool adjacent_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool contains_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool adjacent_stbox_stbox(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

/* Position functions for box types */

extern bool left_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overleft_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool right_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overright_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool before_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overbefore_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool after_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overafter_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool left_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overleft_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool right_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overright_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool below_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overbelow_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool above_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overabove_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool front_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overfront_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool back_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overback_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool before_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overbefore_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool after_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overafter_stbox_stbox(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

/* Set functions for box types */

extern TBOX *union_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool inter_tbox_tbox(const TBOX *box1, const TBOX *box2, TBOX *result);
extern TBOX *intersection_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern STBOX *union_stbox_stbox(const STBOX *box1, const STBOX *box2, bool strict);
extern bool inter_stbox_stbox(const STBOX *box1, const STBOX *box2, STBOX *result);
extern STBOX *intersection_stbox_stbox(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

/* Comparison functions for box types */

extern bool tbox_eq(const TBOX *box1, const TBOX *box2);
extern bool tbox_ne(const TBOX *box1, const TBOX *box2);
extern int tbox_cmp(const TBOX *box1, const TBOX *box2);
extern bool tbox_lt(const TBOX *box1, const TBOX *box2);
extern bool tbox_le(const TBOX *box1, const TBOX *box2);
extern bool tbox_ge(const TBOX *box1, const TBOX *box2);
extern bool tbox_gt(const TBOX *box1, const TBOX *box2);
extern bool stbox_eq(const STBOX *box1, const STBOX *box2);
extern bool stbox_ne(const STBOX *box1, const STBOX *box2);
extern int stbox_cmp(const STBOX *box1, const STBOX *box2);
extern bool stbox_lt(const STBOX *box1, const STBOX *box2);
extern bool stbox_le(const STBOX *box1, const STBOX *box2);
extern bool stbox_ge(const STBOX *box1, const STBOX *box2);
extern bool stbox_gt(const STBOX *box1, const STBOX *box2);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Input/output functions for temporal types */

extern Temporal *temporal_in(char *str, CachedType temptype);
extern char *temporal_out(const Temporal *temp);
extern Temporal *temporal_recv(StringInfo buf);
extern void temporal_write(const Temporal *temp, StringInfo buf);
extern bytea *temporal_send(const Temporal *temp);
extern TInstant *tinstant_in(char *str, CachedType temptype);
extern char *tinstant_out(const TInstant *inst);
extern TInstant *tinstant_recv(StringInfo buf, CachedType temptype);
extern void tinstant_write(const TInstant *inst, StringInfo buf);
extern TInstantSet *tinstantset_in(char *str, CachedType temptype);
extern char *tinstantset_out(const TInstantSet *ti);
extern TInstantSet *tinstantset_recv(StringInfo buf, CachedType temptype);
extern TSequence *tsequence_in(char *str, CachedType temptype, bool linear);
extern char *tsequence_out(const TSequence *seq);
extern TSequence *tsequence_recv(StringInfo buf, CachedType temptype);
extern void tsequence_write(const TSequence *seq, StringInfo buf);
extern TSequenceSet *tsequenceset_in(char *str, CachedType temptype, bool linear);
extern char *tsequenceset_out(const TSequenceSet *ts);
extern TSequenceSet *tsequenceset_recv(StringInfo buf, CachedType temptype);
extern void tsequenceset_write(const TSequenceSet *ts, StringInfo buf);
extern TInstant *tpointinst_from_mfjson(json_object *mfjson, int srid, CachedType temptype);
extern TInstantSet *tpointinstset_from_mfjson(json_object *mfjson, int srid, CachedType temptype);
extern TSequence *tpointseq_from_mfjson(json_object *mfjson, int srid, CachedType temptype, bool linear);
extern TSequenceSet *tpointseqset_from_mfjson(json_object *mfjson, int srid, CachedType temptype, bool linear);
extern Temporal *tpoint_from_ewkb(uint8_t *wkb, int size);
extern char *tpoint_as_text(const Temporal *temp);
extern char *tpoint_as_ewkt(const Temporal *temp);
extern char **geoarr_as_text(const Datum *geoarr, int count, bool extended);
extern char **tpointarr_as_text(const Temporal **temparr, int count, bool extended);
extern char *tpointinst_as_mfjson(const TInstant *inst, int precision, const STBOX *bbox, char *srs);
extern char *tpointinstset_as_mfjson(const TInstantSet *ti, int precision, const STBOX *bbox, char *srs);
extern char *tpointseq_as_mfjson(const TSequence *seq, int precision, const STBOX *bbox, char *srs);
extern char *tpointseqset_as_mfjson(const TSequenceSet *ts, int precision, const STBOX *bbox, char *srs);
extern char *tpoint_as_mfjson(const Temporal *temp, int precision, int has_bbox, char *srs);
extern uint8_t *tpoint_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern char *tpoint_as_hexewkb(const Temporal *temp, uint8_t variant, size_t *size);

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *temporal_from_base(Datum value, CachedType temptype, const Temporal *temp, bool linear);
extern TInstant *tinstant_make(Datum value, CachedType temptype, TimestampTz t);
extern TInstant *tinstant_copy(const TInstant *inst);
extern TInstantSet *tinstantset_make(const TInstant **instants, int count, bool merge);
extern TInstantSet *tinstantset_make_free(TInstant **instants, int count, bool merge);
extern TInstantSet *tinstantset_copy(const TInstantSet *ti);
extern TInstantSet *tinstantset_from_base(Datum value, CachedType temptype, const TimestampSet *ts);
extern TSequence *tsequence_make(const TInstant **instants, int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TSequence *tsequence_make_free(TInstant **instants, int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TSequence *tsequence_copy(const TSequence *seq);
extern TSequence *tsequence_from_base(Datum value, CachedType temptype, const Period *p, bool linear);
extern TSequenceSet *tsequenceset_make(const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_gaps(const TInstant **instants, int count, bool linear, float maxdist, Interval *maxt);
extern TSequenceSet *tsequenceset_copy(const TSequenceSet *ts);
extern TSequenceSet *tsequenceset_from_base(Datum value, CachedType temptype, const PeriodSet *ps, bool linear);

/*****************************************************************************/

/* Cast functions for temporal types */

extern Span *tint_to_span(const Temporal *temp);
extern Span *tfloat_to_span(const Temporal *temp);
extern Temporal *tint_to_tfloat(const Temporal *temp);
extern Temporal *tfloat_to_tint(const Temporal *temp);
extern void temporal_set_period(const Temporal *temp, Period *p);
extern TInstant *tintinst_to_tfloatinst(const TInstant *inst);
extern TInstant *tfloatinst_to_tintinst(const TInstant *inst);
extern TInstantSet *tintinstset_to_tfloatinstset(const TInstantSet *ti);
extern TInstantSet *tfloatinstset_to_tintinstset(const TInstantSet *ti);
extern TSequence *tintseq_to_tfloatseq(const TSequence *seq);
extern TSequence *tfloatseq_to_tintseq(const TSequence *seq);
extern Span *tfloatseqset_to_span(const TSequenceSet *ts);
extern TSequenceSet *tintseqset_to_tfloatseqset(const TSequenceSet *ts);
extern TSequenceSet *tfloatseqset_to_tintseqset(const TSequenceSet *ts);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern char *temporal_subtype(const Temporal *temp);
extern char *temporal_interpolation(const Temporal *temp);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern Datum *temporal_values(const Temporal *temp, int *count);
extern Span **tfloat_spans(const Temporal *temp, int *count);
extern PeriodSet *temporal_time(const Temporal *temp);
extern Span *tnumber_span(const Temporal *temp);
extern Datum temporal_start_value(Temporal *temp);
extern Datum temporal_end_value(Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern Datum temporal_max_value(const Temporal *temp);
extern const TInstant *temporal_min_instant(const Temporal *temp);
extern const TInstant *temporal_max_instant(const Temporal *temp);
extern Interval *temporal_timespan(const Temporal *temp);
extern Interval *temporal_duration(const Temporal *temp);
extern int temporal_num_sequences(const Temporal *temp);
extern TSequence *temporal_start_sequence(const Temporal *temp);
extern TSequence *temporal_end_sequence(const Temporal *temp);
extern TSequence *temporal_sequence_n(const Temporal *temp, int i);
extern TSequence **temporal_sequences(const Temporal *temp, int *count);
extern TSequence **temporal_segments(const Temporal *temp, int *count);
extern int temporal_num_instants(const Temporal *temp);
extern const TInstant *temporal_start_instant(const Temporal *temp);
extern const TInstant *temporal_end_instant(const Temporal *temp);
extern const TInstant *temporal_instant_n(Temporal *temp, int n);
extern const TInstant **temporal_instants(const Temporal *temp, int *count);
extern int temporal_num_timestamps(const Temporal *temp);
extern TimestampTz temporal_start_timestamp(const Temporal *temp);
extern TimestampTz temporal_end_timestamp(Temporal *temp);
extern bool temporal_timestamp_n(Temporal *temp, int n, TimestampTz *result);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);
extern uint32 temporal_hash(const Temporal *temp);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern Datum tinstant_value(const TInstant *inst);
extern Datum tinstant_value_copy(const TInstant *inst);
extern Datum *tinstant_values(const TInstant *inst, int *count);
extern Span **tfloatinst_spans(const TInstant *inst, int *count);
extern PeriodSet *tinstant_time(const TInstant *inst);
extern void tinstant_period(const TInstant *inst, Period *p);
extern TSequence **tinstant_sequences(const TInstant *inst, int *count);
extern TimestampTz *tinstant_timestamps(const TInstant *inst, int *count);
extern const TInstant **tinstant_instants(const TInstant *inst, int *count);
extern bool tinstant_value_at_timestamp(const TInstant *inst, TimestampTz t, Datum *result);
extern uint32 tinstant_hash(const TInstant *inst);
extern const TInstant *tinstantset_inst_n(const TInstantSet *ti, int index);
extern void tinstantset_set_bbox(const TInstantSet *ti, void *box);
extern Datum *tinstantset_values(const TInstantSet *ti, int *count);
extern Span **tfloatinstset_spans(const TInstantSet *ti, int *count);
extern PeriodSet *tinstantset_time(const TInstantSet *ti);
extern Datum tinstantset_min_value(const TInstantSet *ti);
extern Datum tinstantset_max_value(const TInstantSet *ti);
extern void tinstantset_period(const TInstantSet *ti, Period *p);
extern Interval *tinstantset_timespan(const TInstantSet *ti);
extern TSequence **tinstantset_sequences(const TInstantSet *ti, int *count);
extern const TInstant **tinstantset_instants(const TInstantSet *ti, int *count);
extern TimestampTz tinstantset_start_timestamp(const TInstantSet *ti);
extern TimestampTz tinstantset_end_timestamp(const TInstantSet *ti);
extern TimestampTz *tinstantset_timestamps(const TInstantSet *ti, int *count);
extern const TInstant *tinstantset_min_instant(const TInstantSet *ti);
extern const TInstant *tinstantset_max_instant(const TInstantSet *ti);
extern uint32 tinstantset_hash(const TInstantSet *ti);
extern const TInstant *tsequence_inst_n(const TSequence *seq, int index);
extern void tsequence_set_bbox(const TSequence *seq, void *box);
extern Datum *tsequence_values(const TSequence *seq, int *count);
extern Span *tfloatseq_span(const TSequence *seq);
extern Span **tfloatseq_spans(const TSequence *seq, int *count);
extern PeriodSet *tsequence_time(const TSequence *seq);
extern const TInstant *tsequence_min_instant(const TSequence *seq);
extern const TInstant *tsequence_max_instant(const TSequence *seq);
extern Datum tsequence_min_value(const TSequence *seq);
extern Datum tsequence_max_value(const TSequence *seq);
extern Interval *tsequence_duration(const TSequence *seq);
extern void tsequence_period(const TSequence *seq, Period *p);
extern TSequence **tsequence_sequences(const TSequence *seq, int *count);
extern TSequence **tsequence_segments(const TSequence *seq, int *count);
extern const TInstant **tsequence_instants(const TSequence *seq, int *count);
extern TimestampTz tsequence_start_timestamp(const TSequence *seq);
extern TimestampTz tsequence_end_timestamp(const TSequence *seq);
extern TimestampTz *tsequence_timestamps(const TSequence *seq, int *count);
extern bool tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result);
extern bool tsequence_value_at_timestamp_inc(const TSequence *seq, TimestampTz t, Datum *result);
extern uint32 tsequence_hash(const TSequence *seq);
extern const TSequence *tsequenceset_seq_n(const TSequenceSet *ts, int index);
extern void tsequenceset_set_bbox(const TSequenceSet *ts, void *box);
extern Datum *tsequenceset_values(const TSequenceSet *ts, int *count);
extern Span **tfloatseqset_spans(const TSequenceSet *ts, int *count);
extern const TInstant *tsequenceset_min_instant(const TSequenceSet *ts);
extern const TInstant *tsequenceset_max_instant(const TSequenceSet *ts);
extern Datum tsequenceset_min_value(const TSequenceSet *ts);
extern Datum tsequenceset_max_value(const TSequenceSet *ts);
extern PeriodSet *tsequenceset_time(const TSequenceSet *ts);
extern Interval *tsequenceset_timespan(const TSequenceSet *ts);
extern Interval *tsequenceset_duration(const TSequenceSet *ts);
extern void tsequenceset_period(const TSequenceSet *ts, Period *p);
extern const TSequence **tsequenceset_sequences_p(const TSequenceSet *ts);
extern TSequence **tsequenceset_sequences(const TSequenceSet *ts, int *count);
extern TSequence **tsequenceset_segments(const TSequenceSet *ts, int *count);
extern int tsequenceset_num_instants(const TSequenceSet *ts);
extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ts, int n);
extern const TInstant **tsequenceset_instants(const TSequenceSet *ts, int *count);
extern TimestampTz tsequenceset_start_timestamp(const TSequenceSet *ts);
extern TimestampTz tsequenceset_end_timestamp(const TSequenceSet *ts);
extern int tsequenceset_num_timestamps(const TSequenceSet *ts);
extern bool tsequenceset_timestamp_n(const TSequenceSet *ts, int n, TimestampTz *result);
extern TimestampTz *tsequenceset_timestamps(const TSequenceSet *ts, int *count);
extern bool tsequenceset_value_at_timestamp(const TSequenceSet *ts, TimestampTz t, Datum *result);
extern bool tsequenceset_value_at_timestamp_inc(const TSequenceSet *ts, TimestampTz t, Datum *result);
extern uint32 tsequenceset_hash(const TSequenceSet *ts);

/*****************************************************************************/

/* Transformation functions for temporal types */

extern Temporal *temporal_append_tinstant(const Temporal *temp, const Temporal *inst);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);
extern Temporal *temporal_to_tinstant(const Temporal *temp);
extern Temporal *temporal_to_tinstantset(const Temporal *temp);
extern Temporal *temporal_to_tsequence(const Temporal *temp);
extern Temporal *temporal_to_tsequenceset(const Temporal *temp);
extern Temporal *temporal_step_to_linear(const Temporal *temp);
extern Temporal *temporal_shift_tscale(const Temporal *temp, bool shift, bool tscale, Interval *start, Interval *duration);
extern TInstant *tinstantset_to_tinstant(const TInstantSet *ti);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ts);
extern TInstant *tinstant_shift(const TInstant *inst, const Interval *interval);
extern Temporal *tinstant_merge(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge_array(const TInstant **instants, int count);
extern TInstantSet *tinstant_to_tinstantset(const TInstant *inst);
extern TInstantSet *tsequence_to_tinstantset(const TSequence *seq);
extern TInstantSet *tsequenceset_to_tinstantset(const TSequenceSet *ts);
extern TInstantSet *tinstantset_shift_tscale(const TInstantSet *ti, const Interval *start, const Interval *duration);
extern TInstantSet *tinstantset_append_tinstant(const TInstantSet *ti, const TInstant *inst);
extern Temporal *tinstantset_merge(const TInstantSet *ti1, const TInstantSet *ti2);
extern Temporal *tinstantset_merge_array(const TInstantSet **instsets, int count);
extern Temporal *tsequence_append_tinstant(const TSequence *seq, const TInstant *inst);
extern Temporal *tsequence_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tsequence_merge_array(const TSequence **sequences, int count);
extern TSequence *tinstant_to_tsequence(const TInstant *inst, bool linear);
extern TSequence *tinstantset_to_tsequence(const TInstantSet *ti, bool linear);
extern TSequence *tsequenceset_to_tsequence(const TSequenceSet *ts);
extern TSequenceSet *tsequence_step_to_linear(const TSequence *seq);
extern TSequence *tsequence_shift_tscale(const TSequence *seq, const Interval *start, const Interval *duration);
extern TSequenceSet *tsequenceset_append_tinstant(const TSequenceSet *ts, const TInstant *inst);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ts1, const TSequenceSet *ts2);
extern TSequenceSet *tsequenceset_merge_array(const TSequenceSet **seqsets, int count);
extern TSequenceSet *tinstant_to_tsequenceset(const TInstant *inst, bool linear);
extern TSequenceSet *tinstantset_to_tsequenceset(const TInstantSet *ti, bool linear);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tsequenceset_step_to_linear(const TSequenceSet *ts);
extern TSequenceSet *tsequenceset_shift_tscale(const TSequenceSet *ts, const Interval *start, const Interval *duration);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, Datum *values, int count, bool atfunc);
extern Temporal *tnumber_restrict_span(const Temporal *temp, Span *span, bool atfunc);
extern Temporal *tnumber_restrict_spans(const Temporal *temp, Span **spans, int count, bool atfunc);
extern bool temporal_value_at_timestamp_inc(const Temporal *temp, TimestampTz t, Datum *value);
extern bool temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, Datum *result);
extern Temporal *temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_timestampset(const Temporal *temp, const TimestampSet *ts, bool atfunc);
extern Temporal *temporal_restrict_period(const Temporal *temp, const Period *ps, bool atfunc);
extern Temporal *temporal_restrict_periodset(const Temporal *temp, const PeriodSet *ps, bool atfunc);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *temporal_at_min(const Temporal *temp);
extern Temporal *temporal_minus_min(const Temporal *temp);
extern Temporal *temporal_at_max(const Temporal *temp);
extern Temporal *temporal_minus_max(const Temporal *temp);
extern Temporal *temporal_at_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_minus_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_timestampset(const Temporal *temp, const TimestampSet *ts);
extern Temporal *temporal_minus_timestampset(const Temporal *temp, const TimestampSet *ts);
extern Temporal *temporal_at_period(const Temporal *temp, const Period *p);
extern Temporal *temporal_minus_period(const Temporal *temp, const Period *p);
extern Temporal *temporal_at_periodset(const Temporal *temp, const PeriodSet *ps);
extern Temporal *temporal_minus_periodset(const Temporal *temp, const PeriodSet *ps);
extern TInstant *tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc);
extern TInstant *tinstant_restrict_values(const TInstant *inst, const Datum *values, int count, bool atfunc);
extern TInstant *tnumberinst_restrict_span(const TInstant *inst, const Span *span, bool atfunc);
extern TInstant *tnumberinst_restrict_spans(const TInstant *inst, Span **normspans, int count, bool atfunc);
extern TInstant *tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc);
extern TInstant *tinstant_restrict_timestampset(const TInstant *inst, const TimestampSet *ts, bool atfunc);
extern TInstant *tinstant_restrict_period(const TInstant *inst, const Period *period, bool atfunc);
extern TInstant *tinstant_restrict_periodset(const TInstant *inst, const PeriodSet *ps, bool atfunc);
extern TInstantSet *tinstantset_restrict_value(const TInstantSet *ti, Datum value, bool atfunc);
extern TInstantSet *tinstantset_restrict_values(const TInstantSet *ti, const Datum *values, int count, bool atfunc);
extern TInstantSet *tnumberinstset_restrict_span(const TInstantSet *ti, const Span *span, bool atfunc);
extern TInstantSet *tnumberinstset_restrict_spans(const TInstantSet *ti, Span **normspans, int count, bool atfunc);
extern TInstantSet *tinstantset_restrict_minmax(const TInstantSet *ti, bool min, bool atfunc);
extern Temporal *tinstantset_restrict_timestamp(const TInstantSet *ti, TimestampTz t, bool atfunc);
extern TInstantSet *tinstantset_restrict_timestampset(const TInstantSet *ti, const TimestampSet *ts, bool atfunc);
extern TInstantSet *tinstantset_restrict_period(const TInstantSet *ti, const Period *period, bool atfunc);
extern TInstantSet *tinstantset_restrict_periodset(const TInstantSet *ti, const PeriodSet *ps, bool atfunc);
extern bool tinstantset_value_at_timestamp(const TInstantSet *ti, TimestampTz t, Datum *result);
extern TSequenceSet *tsequence_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequenceSet *tsequence_restrict_values(const TSequence *seq, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumberseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseq_restrict_spans(const TSequence *seq, Span **normspans, int count, bool atfunc, bool bboxtest);
extern TSequenceSet *tsequence_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TInstant *tsequence_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequenceSet *tsequence_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TInstantSet *tsequence_at_timestampset(const TSequence *seq, const TimestampSet *ts);
extern TSequenceSet *tsequence_minus_timestampset(const TSequence *seq, const TimestampSet *ts);
extern TSequence *tsequence_at_period(const TSequence *seq, const Period *p);
extern TSequenceSet *tsequence_minus_period(const TSequence *seq, const Period *p);
extern TSequenceSet *tsequence_restrict_periodset(const TSequence *seq, const PeriodSet *ps, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ts, Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ts, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_span(const TSequenceSet *ts, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_spans(const TSequenceSet *ts, Span **normspans, int count, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ts, bool min, bool atfunc);
extern Temporal *tsequenceset_restrict_timestamp(const TSequenceSet *ts, TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_timestampset(const TSequenceSet *ts1, const TimestampSet *ts2, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_period(const TSequenceSet *ts, const Period *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_periodset(const TSequenceSet *ts, const PeriodSet *ps, bool atfunc);
extern TInstant *tpointinst_restrict_geometry(const TInstant *inst, Datum geom, bool atfunc);
extern TInstantSet *tpointinstset_restrict_geometry(const TInstantSet *ti, Datum geom, bool atfunc);
extern TSequenceSet *tpointseq_restrict_geometry(const TSequence *seq, Datum geom, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_geometry(const TSequenceSet *ts, Datum geom, const STBOX *box, bool atfunc);
extern Temporal *tpoint_restrict_geometry(const Temporal *temp, const GSERIALIZED *gs, bool atfunc);
extern Temporal *tpoint_restrict_stbox(const Temporal *temp, const STBOX *box, bool atfunc);

/*****************************************************************************/

/* Boolean functions for temporal types */

extern Temporal *tnot_tbool(const Temporal *temp);
extern Temporal *tand_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tand_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tor_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tor_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Mathematical functions for temporal types */

extern Temporal *tnumber_degrees(const Temporal *temp);
extern TSequence *tnumberseq_derivative(const TSequence *seq);
extern TSequenceSet *tnumberseqset_derivative(const TSequenceSet *ts);
extern Temporal *tnumber_derivative(const Temporal *temp);
extern Temporal *add_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern Temporal *add_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern Temporal *add_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *sub_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern Temporal *sub_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern Temporal *sub_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *mult_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern Temporal *mult_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern Temporal *mult_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *div_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern Temporal *div_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern Temporal *div_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);

/*****************************************************************************/

/* Text functions for temporal types */

extern Temporal *textcat_text_ttext(Datum value, Temporal *temp);
extern Temporal *textcat_ttext_text(const Temporal *temp, Datum value);
extern Temporal *textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2);
extern Temporal *ttext_upper(const Temporal *temp);
extern Temporal *ttext_lower(const Temporal *temp);

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/* Topological functions for temporal types */

extern bool contains_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contains_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contains_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool contains_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool contains_period_temporal(const Period *p, const Temporal *temp);
extern bool contains_temporal_period(const Temporal *temp, const Period *p);
extern bool contains_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool contains_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contained_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contained_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contained_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool contained_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool contained_period_temporal(const Period *p, const Temporal *temp);
extern bool contained_temporal_period(const Temporal *temp, const Period *p);
extern bool contained_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool contained_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overlaps_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overlaps_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overlaps_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overlaps_period_temporal(const Period *p, const Temporal *temp);
extern bool overlaps_temporal_period(const Temporal *temp, const Period *p);
extern bool overlaps_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overlaps_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool same_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool same_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool same_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool same_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool same_period_temporal(const Period *p, const Temporal *temp);
extern bool same_temporal_period(const Temporal *temp, const Period *p);
extern bool same_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool same_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool same_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool adjacent_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool adjacent_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool adjacent_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool adjacent_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool adjacent_period_temporal(const Period *p, const Temporal *temp);
extern bool adjacent_temporal_period(const Temporal *temp, const Period *p);
extern bool adjacent_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool adjacent_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contains_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool contains_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool contains_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contains_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contains_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool contains_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool contains_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool contained_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool contained_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool contained_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contained_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contained_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool contained_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool contained_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overlaps_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool overlaps_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool overlaps_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overlaps_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overlaps_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overlaps_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overlaps_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool same_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool same_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool same_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool same_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool same_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool same_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool same_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool adjacent_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool adjacent_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool adjacent_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool adjacent_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool adjacent_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool adjacent_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool adjacent_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern int boxop_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool(*func)(const STBOX *, const STBOX *), bool invert);
extern bool overlaps_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overlaps_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overlaps_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overlaps_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overlaps_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contains_bbox_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contains_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool contains_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contains_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool contains_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contained_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contained_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool contained_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contained_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool contained_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool same_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool same_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool same_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool same_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool same_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool adjacent_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool adjacent_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool adjacent_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool adjacent_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool adjacent_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool before_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overbefore_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool after_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overafter_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool before_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overbefore_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool after_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overafter_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool before_period_temporal(const Period *p, const Temporal *temp);
extern bool overbefore_period_temporal(const Period *p, const Temporal *temp);
extern bool after_period_temporal(const Period *p, const Temporal *temp);
extern bool overafter_period_temporal(const Period *p, const Temporal *temp);
extern bool before_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overbefore_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool after_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overafter_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool before_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overbefore_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool after_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overafter_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool before_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overbefore_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool after_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overafter_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool before_temporal_period(const Temporal *temp, const Period *p);
extern bool overbefore_temporal_period(const Temporal *temp, const Period *p);
extern bool after_temporal_period(const Temporal *temp, const Period *p);
extern bool overafter_temporal_period(const Temporal *temp, const Period *p);
extern bool before_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overbefore_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool after_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overafter_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool before_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool after_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool left_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool overleft_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool right_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool overright_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber);
extern bool left_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overleft_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool right_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overright_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool left_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool overleft_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool right_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool overright_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype);
extern bool left_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overleft_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool right_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overright_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool left_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overleft_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool right_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overright_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool before_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overbefore_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool after_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overafter_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool left_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overleft_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool right_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overright_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool before_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overbefore_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool after_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overafter_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool left_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overleft_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool right_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overright_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool before_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overbefore_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool after_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overafter_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool left_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overleft_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool right_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overright_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool below_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overbelow_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool above_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overabove_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool front_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overfront_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool back_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overback_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool left_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overleft_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool right_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overright_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool below_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overbelow_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool above_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overabove_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool front_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overfront_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool back_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overback_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool left_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overleft_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool right_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overright_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool below_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overbelow_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool above_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overabove_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool front_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overfront_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool back_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overback_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool before_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overbefore_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool after_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overafter_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool left_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overleft_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool right_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overright_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool below_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overbelow_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool above_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overabove_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool front_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overfront_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool back_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overback_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool before_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overbefore_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool after_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overafter_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool left_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overleft_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool right_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overright_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool below_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbelow_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool above_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overabove_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool front_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overfront_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool back_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overback_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool before_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbefore_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool after_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overafter_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value, CachedType valuetype, CachedType restype);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2, CachedType restype);
extern double nad_tnumber_number(const Temporal *temp, Datum value, CachedType basetype);
extern double nad_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern double nad_tnumber_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *distance_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo);
extern Temporal *distance_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_stbox_geo(const STBOX *box, const GSERIALIZED *gs);
extern double nad_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern double nad_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern double nad_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern bool shortestline_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, Datum *result);
extern bool shortestline_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, Datum *line);

/*****************************************************************************/

/* Ever/always functions for temporal types */

extern bool temporal_ever_eq(const Temporal *temp, Datum value);
extern bool temporal_always_eq(const Temporal *temp, Datum value);
extern bool temporal_ever_lt(const Temporal *temp, Datum value);
extern bool temporal_always_lt(const Temporal *temp, Datum value);
extern bool temporal_ever_le(const Temporal *temp, Datum value);
extern bool temporal_always_le(const Temporal *temp, Datum value);
extern bool tinstant_ever_eq(const TInstant *inst, Datum value);
extern bool tinstant_always_eq(const TInstant *inst, Datum value);
extern bool tinstant_ever_lt(const TInstant *inst, Datum value);
extern bool tinstant_ever_le(const TInstant *inst, Datum value);
extern bool tinstant_always_lt(const TInstant *inst, Datum value);
extern bool tinstant_always_le(const TInstant *inst, Datum value);
extern bool tinstantset_ever_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_le(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_le(const TInstantSet *ti, Datum value);
extern bool tsequence_ever_eq(const TSequence *seq, Datum value);
extern bool tsequence_always_eq(const TSequence *seq, Datum value);
extern bool tsequence_ever_lt(const TSequence *seq, Datum value);
extern bool tsequence_ever_le(const TSequence *seq, Datum value);
extern bool tsequence_always_lt(const TSequence *seq, Datum value);
extern bool tsequence_always_le(const TSequence *seq, Datum value);
extern bool tsequenceset_ever_eq(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_always_eq(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_ever_lt(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_ever_le(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_always_lt(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_always_le(const TSequenceSet *ts, Datum value);
extern bool tpointinst_ever_eq(const TInstant *inst, Datum value);
extern bool tpointinstset_ever_eq(const TInstantSet *ti, Datum value);
extern bool tpointseq_ever_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_ever_eq(const TSequenceSet *ts, Datum value);
extern bool tpoint_ever_eq(const Temporal *temp, Datum value);
extern bool tpointinst_always_eq(const TInstant *inst, Datum value);
extern bool tpointinstset_always_eq(const TInstantSet *ti, Datum value);
extern bool tpointseq_always_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_always_eq(const TSequenceSet *ts, Datum value);
extern bool tpoint_always_eq(const Temporal *temp, Datum value);

/*****************************************************************************/

/* Comparison functions for temporal types */

extern bool temporal_eq(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ne(const Temporal *temp1, const Temporal *temp2);
extern int temporal_cmp(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_lt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_le(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ge(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_gt(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *teq_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tne_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *tne_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tlt_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *tlt_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *tlt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tle_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *tle_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *tle_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tgt_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *tgt_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *tgt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tge_base_temporal(Datum base, CachedType basetype, const Temporal *temp);
extern Temporal *tge_temporal_base(const Temporal *temp, Datum base, CachedType basetype);
extern Temporal *tge_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool tinstant_eq(const TInstant *inst1, const TInstant *inst2);
extern int tinstant_cmp(const TInstant *inst1, const TInstant *inst2);
extern bool tinstantset_eq(const TInstantSet *ti1, const TInstantSet *ti2);
extern int tinstantset_cmp(const TInstantSet *ti1, const TInstantSet *ti2);
extern bool tsequence_eq(const TSequence *seq1, const TSequence *seq2);
extern int tsequence_cmp(const TSequence *seq1, const TSequence *seq2);
extern bool tsequenceset_eq(const TSequenceSet *ts1, const TSequenceSet *ts2);
extern int tsequenceset_cmp(const TSequenceSet *ts1, const TSequenceSet *ts2);
extern Temporal *teq_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern Temporal *teq_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern Temporal *tne_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern Temporal *tne_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);

/*****************************************************************************
  Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */

extern STBOX *tpoint_stbox(const Temporal *temp);
extern STBOX *tpointseq_stboxes(const TSequence *seq, int *count);
extern STBOX *tpointseqset_stboxes(const TSequenceSet *ts, int *count);
extern STBOX *tpoint_stboxes(const Temporal *temp, int *count);
extern Datum tpointinstset_trajectory(const TInstantSet *ti);
extern Datum tpointseq_trajectory(const TSequence *seq);
extern Datum tpointseqset_trajectory(const TSequenceSet *ts);
extern Datum tpoint_trajectory(const Temporal *temp);
extern int tpointinst_srid(const TInstant *inst);
extern int tpointinstset_srid(const TInstantSet *ti);
extern int tpointseq_srid(const TSequence *seq);
extern int tpointseqset_srid(const TSequenceSet *ts);
extern int tpoint_srid(const Temporal *temp);
extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);
extern double tpointseq_length(const TSequence *seq);
extern double tpointseqset_length(const TSequenceSet *ts);
extern double tpoint_length(const Temporal *temp);
extern TInstant *tpointinst_cumulative_length(const TInstant *inst);
extern TInstantSet *tpointinstset_cumulative_length(const TInstantSet *ti);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq, double prevlength);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ts);
extern Temporal *tpoint_cumulative_length(const Temporal *temp);
extern TSequence *tpointseq_speed(const TSequence *seq);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ts);
extern Temporal *tpoint_speed(const Temporal *temp);
extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ts);
extern Temporal *tpoint_azimuth(const Temporal *temp);
extern bool bearing_geo_geo(const GSERIALIZED *geo1, const GSERIALIZED *geo2, Datum *result);
extern Temporal *bearing_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool invert);
extern Temporal *bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern bool tpointinstset_is_simple(const TInstantSet *ti);
extern bool tpointseq_is_simple(const TSequence *seq);
extern bool tpointseqset_is_simple(const TSequenceSet *ts);
extern bool tpoint_is_simple(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern STBOX *geo_expand_spatial(const GSERIALIZED *gs, double d);
extern STBOX *tpoint_expand_spatial(const Temporal *temp, double d);
extern TInstant *tpointinst_set_srid(const TInstant *inst, int32 srid);
extern TInstantSet *tpointinstset_set_srid(const TInstantSet *ti, int32 srid);
extern TSequence *tpointseq_set_srid(const TSequence *seq, int32 srid);
extern TSequenceSet *tpointseqset_set_srid(const TSequenceSet *ts, int32 srid);
extern Temporal *tpoint_set_srid(const Temporal *temp, int32 srid);
extern TInstant *tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper);
extern TInstantSet *tgeompointinstset_tgeogpointinstset(const TInstantSet *ti, bool oper);
extern TSequence *tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeompointseqset_tgeogpointseqset(const TSequenceSet *ts, bool oper);
extern Temporal *tgeompoint_tgeogpoint(const Temporal *temp, bool oper);
extern TInstantSet **tpointinstset_make_simple(const TInstantSet *ti, int *count);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ts, int *count);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);

/*****************************************************************************/

/* Spatial relationship functions for temporal point types */

extern int disjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int intersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int touches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int dwithin_tpoint_geo(Temporal *temp, GSERIALIZED *gs, Datum dist);
extern int dwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, Datum dist);
extern int disjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int intersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tcontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr, Datum atvalue);
extern Temporal *ttouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, Datum atvalue);
extern Temporal *tdwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, Datum dist, bool restr, Datum atvalue);
extern Temporal *tdwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, Datum dist, bool restr, Datum atvalue);
extern Temporal *tdisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, Datum atvalue);
extern Temporal *tintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, Datum atvalue);

/*****************************************************************************/

/* Time functions for temporal types */

extern bool temporal_intersects_timestamp(const Temporal *temp, TimestampTz t);
extern bool temporal_intersects_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool temporal_intersects_period(const Temporal *temp, const Period *p);
extern bool temporal_intersects_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool tinstant_intersects_timestamp(const TInstant *inst, TimestampTz t);
extern bool tinstant_intersects_timestampset(const TInstant *inst, const TimestampSet *ts);
extern bool tinstant_intersects_period(const TInstant *inst, const Period *p);
extern bool tinstant_intersects_periodset(const TInstant *inst, const PeriodSet *ps);
extern bool tinstantset_intersects_timestamp(const TInstantSet *ti, TimestampTz t);
extern bool tinstantset_intersects_timestampset(const TInstantSet *ti, const TimestampSet *ts);
extern bool tinstantset_intersects_period(const TInstantSet *ti, const Period *period);
extern bool tinstantset_intersects_periodset(const TInstantSet *ti, const PeriodSet *ps);
extern bool tsequence_intersects_timestamp(const TSequence *seq, TimestampTz t);
extern bool tsequence_intersects_timestampset(const TSequence *seq, const TimestampSet *ts);
extern bool tsequence_intersects_period(const TSequence *seq, const Period *p);
extern bool tsequence_intersects_periodset(const TSequence *seq, const PeriodSet *ps);
extern bool tsequenceset_intersects_timestamp(const TSequenceSet *ts, TimestampTz t);
extern bool tsequenceset_intersects_timestampset(const TSequenceSet *ts, const TimestampSet *ts1);
extern bool tsequenceset_intersects_period(const TSequenceSet *ts, const Period *p);
extern bool tsequenceset_intersects_periodset(const TSequenceSet *ts, const PeriodSet *ps);

/*****************************************************************************/

/* Local aggregate functions for temporal types */

extern double tnumber_integral(const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);
extern double tnumberinstset_twavg(const TInstantSet *ti);
extern double tnumberseq_integral(const TSequence *seq);
extern double tnumberseq_twavg(const TSequence *seq);
extern double tnumberseqset_integral(const TSequenceSet *ts);
extern double tnumberseqset_twavg(const TSequenceSet *ts);
extern Datum tpointinstset_twcentroid(const TInstantSet *ti);
extern Datum tpointseq_twcentroid(const TSequence *seq);
extern Datum tpointseqset_twcentroid(const TSequenceSet *ts);
extern Datum tpoint_twcentroid(const Temporal *temp);

/*****************************************************************************/

/* Multidimensional tiling functions for temporal types */

extern Temporal **temporal_time_split(Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, TimestampTz torigin, int count, TimestampTz **buckets, int *newcount);

/*****************************************************************************/

/* Similarity functions for temporal types */

extern double temporal_frechet_distance(Temporal *temp1, Temporal *temp2);
extern double temporal_dyntimewarp_distance(Temporal *temp1, Temporal *temp2);
extern Match *temporal_frechet_path(Temporal *temp1, Temporal *temp2, int *count);
extern Match *temporal_dyntimewarp_path(Temporal *temp1, Temporal *temp2, int *count);

/*****************************************************************************/

#endif
