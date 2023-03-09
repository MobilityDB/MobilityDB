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
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_H__
#define __MEOS_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL */
@POSTGRES_DEFS@
/* PostGIS */
@POSTGIS_DEFS@

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/**
 * Structure to represent sets of values
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 settype;        /**< set type */
  uint8 basetype;       /**< span basetype */
  int16 flags;          /**< flags */
  int32 count;          /**< Number of elements */
  int32 maxcount;       /**< Maximum number of elements */
  int32 bboxsize;       /**< Size of the bouding box */
} Set;

/**
 * Structure to represent spans (a.k.a. ranges)
 */
typedef struct
{
  uint8 spantype;       /**< span type */
  uint8 basetype;       /**< span basetype */
  bool lower_inc;       /**< lower bound is inclusive (vs exclusive) */
  bool upper_inc;       /**< upper bound is inclusive (vs exclusive) */
  Datum lower;          /**< lower bound value */
  Datum upper;          /**< upper bound value */
} Span;

/**
 * Structure to represent span sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 spansettype;    /**< span set type */
  uint8 spantype;       /**< span type */
  uint8 basetype;       /**< span basetype */
  int32 count;          /**< Number of Span elements */
  Span span;            /**< Bounding span */
  Span elems[1];        /**< Beginning of variable-length data */
} SpanSet;

/**
 * Structure to represent temporal boxes
 */
typedef struct
{
  Span period;          /**< time span */
  Span span;            /**< value span */
  int16 flags;          /**< flags */
} TBox;

/**
 * Structure to represent spatiotemporal boxes
 */
typedef struct
{
  Span period;          /**< time span */
  double xmin;          /**< minimum x value */
  double xmax;          /**< maximum x value */
  double ymin;          /**< minimum y value */
  double ymax;          /**< maximum y value */
  double zmin;          /**< minimum z value */
  double zmax;          /**< maximum z value */
  int32  srid;          /**< SRID */
  int16  flags;         /**< flags */
} STBox;

/**
 * @brief Enumeration that defines the interpolation types used in
 * MobilityDB.
 */
typedef enum
{
  INTERP_NONE =    0,
  DISCRETE =       1,
  STEP =           2,
  LINEAR =         3,
} interpType;

/**
 * Structure to represent the common structure of temporal values of
 * any temporal subtype
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 temptype;       /**< Temporal type */
  uint8 subtype;        /**< Temporal subtype */
  int16 flags;          /**< Flags */
  /* variable-length data follows */
} Temporal;

/**
 * Structure to represent temporal values of instant subtype
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 temptype;       /**< Temporal type */
  uint8 subtype;        /**< Temporal subtype */
  int16 flags;          /**< Flags */
  TimestampTz t;        /**< Timestamp (8 bytes) */
  Datum value;          /**< Base value for types passed by value,
                             first 8 bytes of the base value for values
                             passed by reference. The extra bytes
                             needed are added upon creation. */
  /* variable-length data follows */
} TInstant;

/**
 * Structure to represent temporal values of instant set or sequence subtype
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 temptype;       /**< Temporal type */
  uint8 subtype;        /**< Temporal subtype */
  int16 flags;          /**< Flags */
  int32 count;          /**< Number of TInstant elements */
  int32 maxcount;       /**< Maximum number of TInstant elements */
  int16 bboxsize;       /**< Size of the bounding box */
  Span period;          /**< Time span (24 bytes). All bounding boxes
                             start with a period so actually it is also
                             the begining of the bounding box. The extra
                             bytes needed are added upon creation. */
  /* variable-length data follows */
} TSequence;

#define TSEQUENCE_BBOX_PTR(seq)      ((void *)(&(seq)->period))

/**
 * Structure to represent temporal values of sequence set subtype
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 temptype;       /**< Temporal type */
  uint8 subtype;        /**< Temporal subtype */
  int16 flags;          /**< Flags */
  int32 count;          /**< Number of TSequence elements */
  int32 totalcount;     /**< Total number of TInstant elements in all
                             composing TSequence elements */
  int32 maxcount;       /**< Maximum number of TSequence elements */
  int16 bboxsize;       /**< Size of the bounding box */
  Span period;          /**< Time span (24 bytes). All bounding boxes
                             start with a period so actually it is also
                             the begining of the bounding box. The extra
                             bytes needed are added upon creation. */
  /* variable-length data follows */
} TSequenceSet;

#define TSEQUENCESET_BBOX_PTR(ss)      ((void *)(&(ss)->period))

/**
 * Struct for storing a similarity match
 */
typedef struct
{
  int i;
  int j;
} Match;

/*****************************************************************************/

/**
 * Structure to represent skiplist elements
 */

#define SKIPLIST_MAXLEVEL 32  /**< maximum possible is 47 with current RNG */

typedef struct
{
  void *value;
  int height;
  int next[SKIPLIST_MAXLEVEL];
} SkipListElem;

/**
 * Structure to represent skiplists that keep the current state of an aggregation
 */
typedef struct
{
  int capacity;
  int next;
  int length;
  int *freed;
  int freecount;
  int freecap;
  int tail;
  void *extra;
  size_t extrasize;
  SkipListElem *elems;
} SkipList;

/*****************************************************************************
 * Initialization of the MEOS library
 *****************************************************************************/

extern void meos_initialize(const char *tz_str);
extern void meos_finalize(void);

/*****************************************************************************
 * Functions for input/output PostgreSQL time types
 *****************************************************************************/

extern bool bool_in(const char *in_str);
extern char *bool_out(bool b);
extern DateADT pg_date_in(const char *str);
extern char *pg_date_out(DateADT date);
extern int pg_interval_cmp(const Interval *interval1, const Interval *interval2);
extern Interval *pg_interval_in(const char *str, int32 typmod);
extern Interval *pg_interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, double secs);
extern Interval *pg_interval_mul(const Interval *span, double factor);
extern char *pg_interval_out(const Interval *span);
extern Interval *pg_interval_pl(const Interval *span1, const Interval *span2);
extern TimeADT pg_time_in(const char *str, int32 typmod);
extern char *pg_time_out(TimeADT time);
extern Timestamp pg_timestamp_in(const char *str, int32 typmod);
extern Interval *pg_timestamp_mi(TimestampTz dt1, TimestampTz dt2);
extern TimestampTz pg_timestamp_mi_interval(TimestampTz timestamp, const Interval *span);
extern char *pg_timestamp_out(Timestamp dt);
extern TimestampTz pg_timestamp_pl_interval(TimestampTz timestamp, const Interval *span);
extern TimestampTz pg_timestamptz_in(const char *str, int32 typmod);
extern char *pg_timestamptz_out(TimestampTz dt);

/*****************************************************************************
 * Functions for input/output and manipulation of PostGIS types
 *****************************************************************************/

extern bytea *gserialized_as_ewkb(const GSERIALIZED *geom, char *type);
extern char *gserialized_as_ewkt(const GSERIALIZED *geom, int precision);
extern char *gserialized_as_geojson(const GSERIALIZED *geom, int option, int precision, char *srs);
extern char *gserialized_as_hexewkb(const GSERIALIZED *geom, const char *type);
extern char *gserialized_as_text(const GSERIALIZED *geom, int precision);
extern GSERIALIZED *gserialized_from_ewkb(const bytea *bytea_wkb, int32 srid);
extern GSERIALIZED *gserialized_from_geojson(const char *geojson);
extern GSERIALIZED *gserialized_from_hexewkb(const char *wkt);
extern GSERIALIZED *gserialized_from_text(char *wkt, int srid);
extern GSERIALIZED *gserialized_in(char *input, int32 geom_typmod);
extern char *gserialized_out(const GSERIALIZED *geom);
extern bool pgis_gserialized_same(const GSERIALIZED *geom1, const GSERIALIZED *geom2);

/*****************************************************************************
 * Functions for set and span types
 *****************************************************************************/

/* Input/output functions for set and span types */

extern Set *bigintset_in(const char *str);
extern Span *bigintspan_in(const char *str);
extern char *bigintspan_out(const Span *s);
extern SpanSet *bigintspanset_in(const char *str);
extern char *bigintspanset_outbigintspanset_out(const SpanSet *ss);
extern Set *floatset_in(const char *str);
extern Span *floatspan_in(const char *str);
extern char *floatspan_out(const Span *s, int maxdd);
extern SpanSet *floatspanset_in(const char *str);
extern char *floatspanset_out(const SpanSet *ss, int maxdd);
extern char *geoset_as_text(const Set *set, int maxdd);
extern char *geoset_as_ewkt(const Set *set, int maxdd);
extern Set *intset_in(const char *str);
extern Span *intspan_in(const char *str);
extern char *intspan_out(const Span *s);
extern SpanSet *intspanset_in(const char *str);
extern char *intspanset_out(const SpanSet *ss);
extern uint8_t *set_as_wkb(const Set *s, uint8_t variant, size_t *size_out);
extern Set *set_from_hexwkb(const char *hexwkb);
extern Set *set_from_wkb(const uint8_t *wkb, int size);
extern char *set_out(const Set *s, int maxdd);
extern Span *period_in(const char *str);
extern char *period_out(const Span *s);
extern SpanSet *periodset_in(const char *str);
extern char *periodset_out(const SpanSet *ss);
extern uint8_t *span_as_wkb(const Span *s, uint8_t variant, size_t *size_out);
extern Span *span_from_hexwkb(const char *hexwkb);
extern Span *span_from_wkb(const uint8_t *wkb, int size);
extern char *span_out(const Span *s, int maxdd);
extern uint8_t *spanset_as_wkb(const SpanSet *ss, uint8_t variant, size_t *size_out);
extern SpanSet *spanset_from_hexwkb(const char *hexwkb);
extern SpanSet *spanset_from_wkb(const uint8_t *wkb, int size);
extern char *spanset_out(const SpanSet *ss, int maxdd);
extern Set *tstzset_in(const char *str);

/*****************************************************************************/

/* Constructor functions for set and span types */

extern Span *bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc);
extern Span *floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc);
extern Span *intspan_make(int lower, int upper, bool lower_inc, bool upper_inc);
extern Set *set_copy(const Set *ts);
extern Span *tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc);
extern Span *span_copy(const Span *s);
extern SpanSet *spanset_copy(const SpanSet *ps);
extern SpanSet *spanset_make(const Span **spans, int count, bool normalize);
extern SpanSet *spanset_make_free(Span **spans, int count, bool normalize);

/*****************************************************************************/

/* Cast functions for set and span types */

extern Set *bigint_to_bigintset(int64 i);
extern Span *bigint_to_intspan(int i);
extern Span *float_to_floaspan(double d);
extern Set *float_to_floatset(double d);
extern Set *int_to_intset(int i);
extern Span *int_to_intspan(int i);
extern void set_set_span(const Set *os, Span *s);
extern Span *set_to_span(const Set *s);
extern void spatialset_set_stbox(const Set *set, STBox *box);
extern STBox *spatialset_to_stbox(const Set *s);
extern SpanSet *set_to_spanset(const Set *s);
extern SpanSet *span_to_spanset(const Span *s);
extern Span *timestamp_to_period(TimestampTz t);
extern SpanSet *timestamp_to_periodset(TimestampTz t);
extern Set *timestamp_to_tstzset(TimestampTz t);
extern SpanSet *tstzset_to_periodset(const Set *ts);

/*****************************************************************************/

/* Accessor functions for set and span types */

extern int64 bigintset_end_value(const Set *s);
extern int bigintset_num_values(const Set *s);
extern int64 bigintset_start_value(const Set *s);
extern bool bigintset_value_n(const Set *s, int n, int64 *result);
extern int64 *bigintset_values(const Set *s);
extern int bigintspan_lower(const Span *s);
extern int bigintspan_upper(const Span *s);
extern int bigintspanset_lower(const SpanSet *ss);
extern int bigintspanset_upper(const SpanSet *ss);
extern double floatset_end_value(const Set *s);
extern int floatset_num_values(const Set *s);
extern double floatset_start_value(const Set *s);
extern bool floatset_value_n(const Set *s, int n, double *result);
extern double *floatset_values(const Set *s);
extern double floatspan_lower(const Span *s);
extern double floatspan_upper(const Span *s);
extern double floatspanset_lower(const SpanSet *ss);
extern double floatspanset_upper(const SpanSet *ss);
extern int intset_end_value(const Set *s);
extern int intset_num_values(const Set *s);
extern int intset_start_value(const Set *s);
extern bool intset_value_n(const Set *s, int n, int *result);
extern int *intset_values(const Set *s);
extern int intspan_lower(const Span *s);
extern int intspan_upper(const Span *s);
extern int intspanset_lower(const SpanSet *ss);
extern int intspanset_upper(const SpanSet *ss);
extern Datum set_end_value(const Set *s);
extern uint32 set_hash(const Set *s);
extern uint64 set_hash_extended(const Set *s, uint64 seed);
extern int set_mem_size(const Set *s);
extern int set_num_values(const Set *s);
extern Datum set_start_value(const Set *s);
extern bool set_value_n(const Set *s, int n, Datum *result);
extern Datum *set_values(const Set *s);
extern Interval *period_duration(const Span *s);
extern TimestampTz period_lower(const Span *p);
extern TimestampTz period_upper(const Span *p);
extern Interval *periodset_duration(const SpanSet *ps, bool boundspan);
extern TimestampTz periodset_end_timestamp(const SpanSet *ps);
extern TimestampTz periodset_lower(const SpanSet *ps);
extern int periodset_num_timestamps(const SpanSet *ps);
extern TimestampTz periodset_start_timestamp(const SpanSet *ps);
extern bool periodset_timestamp_n(const SpanSet *ps, int n, TimestampTz *result);
extern TimestampTz *periodset_timestamps(const SpanSet *ps, int *count);
extern TimestampTz periodset_upper(const SpanSet *ps);
extern uint32 span_hash(const Span *s);
extern uint64 span_hash_extended(const Span *s, Datum seed);
extern bool span_lower_inc(const Span *s);
extern bool span_upper_inc(const Span *s);
extern double span_width(const Span *s);
extern Span *spanset_end_span(const SpanSet *ss);
extern uint32 spanset_hash(const SpanSet *ps);
extern uint64 spanset_hash_extended(const SpanSet *ps, uint64 seed);
extern bool spanset_lower_inc(const SpanSet *ss);
extern int spanset_mem_size(const SpanSet *ss);
extern int spanset_num_spans(const SpanSet *ss);
extern Span *spanset_span_n(const SpanSet *ss, int i);
extern const Span **spanset_spans(const SpanSet *ss);
extern Span *spanset_start_span(const SpanSet *ss);
extern bool spanset_upper_inc(const SpanSet *ss);
extern double spanset_width(const SpanSet *ss);
extern TimestampTz tstzset_end_timestamp(const Set *ts);
extern int tstzset_num_timestamps(const Set *ts);
extern TimestampTz tstzset_start_timestamp(const Set *ts);
extern bool tstzset_timestamp_n(const Set *ts, int n, TimestampTz *result);
extern TimestampTz *tstzset_timestamps(const Set *ts);
extern int geoset_srid(const Set *set);

/*****************************************************************************/

/* Transformation functions for set and span types */

extern void bigintspan_set_floatspan(const Span *s1, Span *s2);
extern void floatspan_set_bigintspan(const Span *s1, Span *s2);
extern void floatspan_set_intspan(const Span *s1, Span *s2);
extern void intspan_set_floatspan(const Span *s1, Span *s2);
extern void numspan_set_floatspan(const Span *s1, Span *s2);
extern Span *period_tprecision(const Span *s, const Interval *duration, TimestampTz torigin);
extern SpanSet *periodset_tprecision(const SpanSet *ss, const Interval *duration, TimestampTz torigin);
extern void period_shift_tscale(Span *p, const Interval *shift, const Interval *duration);
extern SpanSet *periodset_shift_tscale(const SpanSet *ps, const Interval *shift, const Interval *duration);
extern Set *set_shift(const Set *s, Datum shift);
extern void span_expand(const Span *s1, Span *s2);
extern TimestampTz timestamp_tprecision(TimestampTz t, const Interval *duration, TimestampTz torigin);
extern Set *tstzset_shift_tscale(const Set *ts, const Interval *shift, const Interval *duration);

/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Topological functions for set and span types */

extern bool adjacent_bigintspan_bigint(const Span *s, int64 i);
extern bool adjacent_bigintspanset_bigint(const SpanSet *ss, int64 i);
extern bool adjacent_floatspan_float(const Span *s, double d);
extern bool adjacent_intspan_int(const Span *s, int i);
extern bool adjacent_set_span(const Set *os, const Span *s);
extern bool adjacent_set_spanset(const Set *s, const SpanSet *ss);
extern bool adjacent_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern bool adjacent_span_set(const Span *s, const Set *os);
extern bool adjacent_span_span(const Span *s1, const Span *s2);
extern bool adjacent_span_spanset(const Span *s, const SpanSet *ss);
extern bool adjacent_spanset_set(const SpanSet *ss, const Set *s);
extern bool adjacent_spanset_span(const SpanSet *ss, const Span *s);
extern bool adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_bigint_bigintset(int64 i, const Set *s);
extern bool contained_bigint_bigintspan(int64 i, const Span *s);
extern bool contained_bigint_bigintspanset(int64 i, const SpanSet *ss);
extern bool contained_float_floatset(double d, const Set *s);
extern bool contained_float_floatspan(double d, const Span *s);
extern bool contained_float_floatspanset(double d, const SpanSet *ss);
extern bool contained_int_intset(int i, const Set *s);
extern bool contained_int_intspanset (int i, const SpanSet *ss);
extern bool contained_int_intspan(int i, const Span *s);
extern bool contained_set_set(const Set *s1, const Set *s2);
extern bool contained_set_span(const Set *os, const Span *s);
extern bool contained_set_spanset(const Set *s, const SpanSet *ss);
extern bool contained_span_span(const Span *s1, const Span *s2);
extern bool contained_span_spanset(const Span *s, const SpanSet *ss);
extern bool contained_spanset_span(const SpanSet *ss, const Span *s);
extern bool contained_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_timestamp_period(TimestampTz t, const Span *p);
extern bool contained_timestamp_timestampset(TimestampTz t, const Set *ts);
extern bool contained_timestampset_timestampset(const Set *ts1, const Set *ts2);
extern bool contains_floatspan_float(const Span *s, double d);
extern bool contains_intspan_int(const Span *s, int i);
extern bool contains_set_set(const Set *s1, const Set *s2);
extern bool contains_period_timestamp(const Span *p, TimestampTz t);
extern bool contains_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern bool contains_span_set(const Span *s, const Set *os);
extern bool contains_span_span(const Span *s1, const Span *s2);
extern bool contains_span_spanset(const Span *s, const SpanSet *ss);
extern bool contains_spanset_set(const SpanSet *ss, const Set *s);
extern bool contains_spanset_span(const SpanSet *ss, const Span *s);
extern bool contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contains_timestampset_timestamp(const Set *ts, TimestampTz t);
extern bool contains_timestampset_timestampset(const Set *ts1, const Set *ts2);
extern bool overlaps_set_set(const Set *s1, const Set *s2);
extern bool overlaps_span_set(const Span *s, const Set *os);
extern bool overlaps_span_span(const Span *s1, const Span *s2);
extern bool overlaps_spanset_set(const SpanSet *ss, const Set *s);
extern bool overlaps_spanset_span(const SpanSet *ss, const Span *s);
extern bool overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool overlaps_timestampset_period(const Set *ts, const Span *p);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool after_timestamp_timestampset(TimestampTz t, const Set *ts);
extern bool before_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern bool before_timestamp_timestampset(TimestampTz t, const Set *ts);
extern bool left_float_floatspan(double d, const Span *s);
extern bool left_floatspan_float(const Span *s, double d);
extern bool left_int_intspan(int i, const Span *s);
extern bool left_intspan_int(const Span *s, int i);
extern bool left_set_set(const Set *s1, const Set *s2);
extern bool left_set_span(const Set *os, const Span *s);
extern bool left_set_spanset(const Set *s, const SpanSet *ss);
extern bool left_span_set(const Span *s, const Set *os);
extern bool left_span_span(const Span *s1, const Span *s2);
extern bool left_span_spanset(const Span *s, const SpanSet *ss);
extern bool left_spanset_set(const SpanSet *ss, const Set *s);
extern bool left_spanset_span(const SpanSet *ss, const Span *s);
extern bool left_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool overafter_period_timestamp(const Span *p, TimestampTz t);
extern bool overafter_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern bool overafter_timestamp_period(TimestampTz t, const Span *p);
extern bool overafter_timestamp_periodset(TimestampTz t, const SpanSet *ps);
extern bool overafter_timestamp_timestampset(TimestampTz t, const Set *ts);
extern bool overbefore_period_timestamp(const Span *p, TimestampTz t);
extern bool overbefore_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern bool overbefore_timestamp_period(TimestampTz t, const Span *p);
extern bool overbefore_timestamp_periodset(TimestampTz t, const SpanSet *ps);
extern bool overbefore_timestamp_timestampset(TimestampTz t, const Set *ts);
extern bool overleft_float_floatspan(double d, const Span *s);
extern bool overleft_floatspan_float(const Span *s, double d);
extern bool overleft_int_intspan(int i, const Span *s);
extern bool overleft_intspan_int(const Span *s, int i);
extern bool overleft_set_set(const Set *s1, const Set *s2);
extern bool overleft_set_span(const Set *os, const Span *s);
extern bool overleft_set_spanset(const Set *s, const SpanSet *ss);
extern bool overleft_span_set(const Span *s, const Set *os);
extern bool overleft_span_span(const Span *s1, const Span *s2);
extern bool overleft_span_spanset(const Span *s, const SpanSet *ss);
extern bool overleft_spanset_set(const SpanSet *ss, const Set *s);
extern bool overleft_spanset_span(const SpanSet *ss, const Span *s);
extern bool overleft_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool overright_float_floatspan(double d, const Span *s);
extern bool overright_floatspan_float(const Span *s, double d);
extern bool overright_int_intspan(int i, const Span *s);
extern bool overright_intspan_int(const Span *s, int i);
extern bool overright_set_set(const Set *s1, const Set *s2);
extern bool overright_set_span(const Set *os, const Span *s);
extern bool overright_set_spanset(const Set *s, const SpanSet *ss);
extern bool overright_span_set(const Span *s, const Set *os);
extern bool overright_span_span(const Span *s1, const Span *s2);
extern bool overright_span_spanset(const Span *s, const SpanSet *ss);
extern bool overright_spanset_set(const SpanSet *ss, const Set *s);
extern bool overright_spanset_span(const SpanSet *ss, const Span *s);
extern bool overright_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool right_float_floatspan(double d, const Span *s);
extern bool right_floatspan_float(const Span *s, double d);
extern bool right_int_intspan(int i, const Span *s);
extern bool right_intspan_int(const Span *s, int i);
extern bool right_set_set(const Set *s1, const Set *s2);
extern bool right_set_span(const Set *os, const Span *s);
extern bool right_set_spanset(const Set *s, const SpanSet *ss);
extern bool right_span_set(const Span *s, const Set *os);
extern bool right_span_span(const Span *s1, const Span *s2);
extern bool right_span_spanset(const Span *s, const SpanSet *ss);
extern bool right_spanset_set(const SpanSet *ss, const Set *s);
extern bool right_spanset_span(const SpanSet *ss, const Span *s);
extern bool right_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************/

/* Set functions for set and span types */

extern Span *bbox_union_span_span(const Span *s1, const Span *s2);
extern Set *intersection_set_set(const Set *s1, const Set *s2);
extern bool intersection_period_timestamp(const Span *p, TimestampTz t, TimestampTz *result);
extern bool intersection_periodset_timestamp(const SpanSet *ps, TimestampTz t, TimestampTz *result);
extern Set *intersection_span_set(const Span *s, const Set *os);
extern Span *intersection_span_span(const Span *s1, const Span *s2);
extern Set *intersection_spanset_set(const SpanSet *ss, const Set *s);
extern SpanSet *intersection_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool intersection_timestamp_period(TimestampTz t, const Span *p, TimestampTz *result);
extern Set *intersection_timestampset_period(const Set *ts, const Span *p);
extern Set *intersection_timestampset_periodset(const Set *ts, const SpanSet *ps);
extern bool intersection_timestampset_timestamp(const Set *ts, const TimestampTz t, TimestampTz *result);
extern Set *intersection_timestampset_timestampset(const Set *ts1, const Set *ts2);
extern Set *minus_set_set(const Set *s1, const Set *s2);
extern Set *minus_set_span(const Set *os, const Span *s);
extern Set *minus_set_spanset(const Set *s, const SpanSet *ss);
extern SpanSet *minus_period_timestamp(const Span *p, TimestampTz t);
extern SpanSet *minus_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern SpanSet *minus_span_set(const Span *s, const Set *os);
extern SpanSet *minus_span_span(const Span *s1, const Span *s2);
extern SpanSet *minus_span_spanset(const Span *s, const SpanSet *ss);
extern SpanSet *minus_spanset_set(const SpanSet *ss, const Set *s);
extern SpanSet *minus_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool minus_timestamp_period(TimestampTz t, const Span *p, TimestampTz *result);
extern bool minus_timestamp_periodset(TimestampTz t, const SpanSet *ps, TimestampTz *result);
extern Set *minus_timestampset_timestamp(const Set *ts, TimestampTz t);
extern Set *minus_timestampset_timestampset(const Set *ts1, const Set *ts2);
extern Set *union_set_set(const Set *s1, const Set *s2);
extern SpanSet *union_period_timestamp(const Span *p, TimestampTz t);
extern SpanSet *union_period_timestampset(const Span *p, const Set *ts);
extern SpanSet *union_periodset_timestamp(SpanSet *ps, TimestampTz t);
extern SpanSet *union_span_set(const Span *s, const Set *os);
extern SpanSet *union_span_span(const Span *s1, const Span *s2);
extern SpanSet *union_spanset_set(const SpanSet *ss, const Set *s);
extern SpanSet *union_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern SpanSet *union_timestamp_period(TimestampTz t, const Span *p);
extern Set *union_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern Set *union_timestamp_timestampset(TimestampTz t, const Set *ts);
extern SpanSet *union_timestampset_period(const Set *ts, const Span *p);
extern Set *union_timestampset_timestamp(const Set *ts, const TimestampTz t);

/*****************************************************************************/

/* Distance functions for set and span types */

extern double distance_floatspan_float(const Span *s, double d);
extern double distance_intspan_int(const Span *s, int i);
extern double distance_set_set(const Set *s1, const Set *s2);
extern double distance_period_periodset(const Span *p, const SpanSet *ps);
extern double distance_period_timestamp(const Span *p, TimestampTz t);
extern double distance_period_timestampset(const Span *p, const Set *ts);
extern double distance_periodset_period(const SpanSet *ps, const Span *p);
extern double distance_periodset_periodset(const SpanSet *ps1, const SpanSet *ps2);
extern double distance_periodset_timestamp(const SpanSet *ps, TimestampTz t);
extern double distance_periodset_timestampset(const SpanSet *ps, const Set *ts);
extern double distance_span_set(const Span *s, const Set *os);
extern double distance_span_span(const Span *s1, const Span *s2);
extern double distance_spanset_span(const SpanSet *ss, const Span *s);
extern double distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern double distance_timestamp_period(TimestampTz t, const Span *p);
extern double distance_timestamp_periodset(TimestampTz t, const SpanSet *ps);
extern double distance_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern double distance_timestamp_timestampset(TimestampTz t, const Set *ts);
extern double distance_timestampset_period(const Set *ts, const Span *p);
extern double distance_timestampset_periodset(const Set *ts, const SpanSet *ps);
extern double distance_timestampset_timestamp(const Set *ts, TimestampTz t);
extern double distance_timestampset_timestampset(const Set *ts1, const Set *ts2);

/*****************************************************************************/

/* Aggregate functions for set and span types */

extern Span *bigint_extent_transfn(Span *s, int64 i);
extern Set *bigint_union_transfn(Set *state, int64 i);
extern Span *int_extent_transfn(Span *s, int i);
extern Set *int_union_transfn(Set *state, int i);
extern Span *float_extent_transfn(Span *s, double d);
extern Set *float_union_transfn(Set *state, double d);
extern SkipList *period_tcount_transfn(SkipList *state, const Span *p);
extern SkipList *periodset_tcount_transfn(SkipList *state, const SpanSet *ps);
extern Set *set_union_finalfn(Set *state);
extern Span *span_extent_transfn(Span *s1, const Span *s2);
extern Span *spanset_extent_transfn(Span *s, const SpanSet *ss);
extern Set *text_union_transfn(Set *state, const text *txt);
extern Span *timestamp_extent_transfn(Span *p, TimestampTz t);
extern SkipList *timestamp_tcount_transfn(SkipList *state, TimestampTz t);
extern Set *timestamp_union_transfn(Set *state, TimestampTz t);
extern Span *set_extent_transfn(Span *span, const Set *set);
extern SkipList *tstzset_tcount_transfn(SkipList *state, const Set *ts);

/*****************************************************************************/

/* Comparison functions for set and span types */

extern int bigintset_cmp(const Set *s1, const Set *s2);
extern bool bigintset_eq(const Set *s1, const Set *s2);
extern bool bigintset_ge(const Set *s1, const Set *s2);
extern bool bigintset_gt(const Set *s1, const Set *s2);
extern bool bigintset_le(const Set *s1, const Set *s2);
extern bool bigintset_lt(const Set *s1, const Set *s2);
extern bool bigintset_ne(const Set *s1, const Set *s2);
extern int floatset_cmp(const Set *s1, const Set *s2);
extern bool floatset_eq(const Set *s1, const Set *s2);
extern bool floatset_ge(const Set *s1, const Set *s2);
extern bool floatset_gt(const Set *s1, const Set *s2);
extern bool floatset_le(const Set *s1, const Set *s2);
extern bool floatset_lt(const Set *s1, const Set *s2);
extern bool floatset_ne(const Set *s1, const Set *s2);
extern int intset_cmp(const Set *s1, const Set *s2);
extern bool intset_eq(const Set *s1, const Set *s2);
extern bool intset_ge(const Set *s1, const Set *s2);
extern bool intset_gt(const Set *s1, const Set *s2);
extern bool intset_le(const Set *s1, const Set *s2);
extern bool intset_lt(const Set *s1, const Set *s2);
extern bool intset_ne(const Set *s1, const Set *s2);
extern int set_cmp(const Set *s1, const Set *s2);
extern bool set_eq(const Set *s1, const Set *s2);
extern bool set_ge(const Set *s1, const Set *s2);
extern bool set_gt(const Set *s1, const Set *s2);
extern bool set_le(const Set *s1, const Set *s2);
extern bool set_lt(const Set *s1, const Set *s2);
extern bool set_ne(const Set *s1, const Set *s2);
extern int span_cmp(const Span *s1, const Span *s2);
extern bool span_eq(const Span *s1, const Span *s2);
extern bool span_ge(const Span *s1, const Span *s2);
extern bool span_gt(const Span *s1, const Span *s2);
extern bool span_le(const Span *s1, const Span *s2);
extern bool span_lt(const Span *s1, const Span *s2);
extern bool span_ne(const Span *s1, const Span *s2);
extern int spanset_cmp(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_eq(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_ge(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_gt(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_le(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_lt(const SpanSet *ss1, const SpanSet *ss2);
extern bool spanset_ne(const SpanSet *ss1, const SpanSet *ss2);
extern int tstzset_cmp(const Set *ts1, const Set *ts2);
extern bool tstzset_eq(const Set *ts1, const Set *ts2);
extern bool tstzset_ge(const Set *ts1, const Set *ts2);
extern bool tstzset_gt(const Set *ts1, const Set *ts2);
extern bool tstzset_le(const Set *ts1, const Set *ts2);
extern bool tstzset_lt(const Set *ts1, const Set *ts2);
extern bool tstzset_ne(const Set *ts1, const Set *ts2);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */

extern TBox *tbox_in(const char *str);
extern char *tbox_out(const TBox *box, int maxdd);
extern TBox *tbox_from_wkb(const uint8_t *wkb, int size);
extern TBox *tbox_from_hexwkb(const char *hexwkb);
extern STBox *stbox_from_wkb(const uint8_t *wkb, int size);
extern STBox *stbox_from_hexwkb(const char *hexwkb);
extern uint8_t *tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out);
extern char *tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size);
extern uint8_t *stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out);
extern char *stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size);
extern STBox *stbox_in(const char *str);
extern char *stbox_out(const STBox *box, int maxdd);

/*****************************************************************************/

/* Constructor functions for box types */

extern TBox *tbox_make(const Span *p, const Span *s);
extern void tbox_set(const Span *p, const Span *s, TBox *box);
extern TBox *tbox_copy(const TBox *box);
extern STBox * stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *p);
extern void stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, const Span *p, STBox *box);
extern STBox *stbox_copy(const STBox *box);

/*****************************************************************************/

/* Cast functions for box types */

extern TBox *int_to_tbox(int i);
extern TBox *float_to_tbox(double d);
extern TBox *span_to_tbox(const Span *span);
extern TBox *timestamp_to_tbox(TimestampTz t);
extern TBox *tstzset_to_tbox(const Set *ss);
extern TBox *period_to_tbox(const Span *p);
extern TBox *periodset_to_tbox(const SpanSet *ps);
extern TBox *int_timestamp_to_tbox(int i, TimestampTz t);
extern TBox *float_timestamp_to_tbox(double d, TimestampTz t);
extern TBox *int_period_to_tbox(int i, const Span *p);
extern TBox *float_period_to_tbox(double d, const Span *p);
extern TBox *span_timestamp_to_tbox(const Span *span, TimestampTz t);
extern TBox *span_period_to_tbox(const Span *span, const Span *p);
extern Span *tbox_to_floatspan(const TBox *box);
extern Span *tbox_to_period(const TBox *box);
extern Span *stbox_to_period(const STBox *box);
extern TBox *tnumber_to_tbox(const Temporal *temp);
extern GSERIALIZED *stbox_to_geo(const STBox *box);
extern STBox *tpoint_to_stbox(const Temporal *temp);
extern STBox *geo_to_stbox(const GSERIALIZED *gs);
extern STBox *timestamp_to_stbox(TimestampTz t);
extern STBox *tstzset_to_stbox(const Set *ts);
extern STBox *period_to_stbox(const Span *p);
extern STBox *periodset_to_stbox(const SpanSet *ps);
extern STBox *geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t);
extern STBox *geo_period_to_stbox(const GSERIALIZED *gs, const Span *p);

/*****************************************************************************/

/* Accessor functions for box types */

extern bool tbox_hasx(const TBox *box);
extern bool tbox_hast(const TBox *box);
extern bool tbox_xmin(const TBox *box, double *result);
extern bool tbox_xmax(const TBox *box, double *result);
extern bool tbox_tmin(const TBox *box, TimestampTz *result);
extern bool tbox_tmax(const TBox *box, TimestampTz *result);
extern bool stbox_hasx(const STBox *box);
extern bool stbox_hasz(const STBox *box);
extern bool stbox_hast(const STBox *box);
extern bool stbox_isgeodetic(const STBox *box);
extern bool stbox_xmin(const STBox *box, double *result);
extern bool stbox_xmax(const STBox *box, double *result);
extern bool stbox_ymin(const STBox *box, double *result);
extern bool stbox_ymax(const STBox *box, double *result);
extern bool stbox_zmin(const STBox *box, double *result);
extern bool stbox_zmax(const STBox *box, double *result);
extern bool stbox_tmin(const STBox *box, TimestampTz *result);
extern bool stbox_tmax(const STBox *box, TimestampTz *result);
extern int32 stbox_srid(const STBox *box);

/*****************************************************************************/

/* Transformation functions for box types */

extern void tbox_expand(const TBox *box1, TBox *box2);
extern void tbox_shift_tscale(TBox *box, const Interval *start, const Interval *duration);
extern TBox *tbox_expand_value(const TBox *box, const double d);
extern TBox *tbox_expand_time(const TBox *box, const Interval *interval);
extern void stbox_expand(const STBox *box1, STBox *box2);
extern void stbox_shift_tscale(STBox *box, const Interval *start, const Interval *duration);
extern STBox *stbox_set_srid(const STBox *box, int32 srid);
extern STBox *stbox_expand_space(const STBox *box, double d);
extern STBox *stbox_expand_time(const STBox *box, const Interval *interval);

/*****************************************************************************/

/* Topological functions for box types */

extern bool contains_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool contained_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overlaps_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool same_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool adjacent_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool contains_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool contained_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overlaps_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool same_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool adjacent_stbox_stbox(const STBox *box1, const STBox *box2);

/*****************************************************************************/

/* Position functions for box types */

extern bool left_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overleft_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool right_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overright_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool before_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overbefore_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool after_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overafter_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool left_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overleft_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool right_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overright_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool below_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overbelow_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool above_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overabove_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool front_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overfront_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool back_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overback_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool before_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overbefore_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool after_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overafter_stbox_stbox(const STBox *box1, const STBox *box2);

/*****************************************************************************/

/* Set functions for box types */

extern TBox *union_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool inter_tbox_tbox(const TBox *box1, const TBox *box2, TBox *result);
extern TBox *intersection_tbox_tbox(const TBox *box1, const TBox *box2);
extern STBox *union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict);
extern bool inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result);
extern STBox *intersection_stbox_stbox(const STBox *box1, const STBox *box2);

/*****************************************************************************/

/* Comparison functions for box types */

extern bool tbox_eq(const TBox *box1, const TBox *box2);
extern bool tbox_ne(const TBox *box1, const TBox *box2);
extern int tbox_cmp(const TBox *box1, const TBox *box2);
extern bool tbox_lt(const TBox *box1, const TBox *box2);
extern bool tbox_le(const TBox *box1, const TBox *box2);
extern bool tbox_ge(const TBox *box1, const TBox *box2);
extern bool tbox_gt(const TBox *box1, const TBox *box2);
extern bool stbox_eq(const STBox *box1, const STBox *box2);
extern bool stbox_ne(const STBox *box1, const STBox *box2);
extern int stbox_cmp(const STBox *box1, const STBox *box2);
extern bool stbox_lt(const STBox *box1, const STBox *box2);
extern bool stbox_le(const STBox *box1, const STBox *box2);
extern bool stbox_ge(const STBox *box1, const STBox *box2);
extern bool stbox_gt(const STBox *box1, const STBox *box2);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Utility functions for temporal types */

extern text *cstring2text(const char *cstring);
extern char *text2cstring(const text *textptr);

/* Input/output functions for temporal types */

extern Temporal *tbool_in(const char *str);
extern char *tbool_out(const Temporal *temp);
extern char *temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern char *temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags, int precision, char *srs);
extern uint8_t *temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern Temporal *temporal_from_hexwkb(const char *hexwkb);
extern Temporal *temporal_from_mfjson(const char *mfjson);
extern Temporal *temporal_from_wkb(const uint8_t *wkb, int size);
extern Temporal *tfloat_in(const char *str);
extern char *tfloat_out(const Temporal *temp, int maxdd);
extern Temporal *tgeogpoint_in(const char *str);
extern Temporal *tgeompoint_in(const char *str);
extern Temporal *tint_in(const char *str);
extern char *tint_out(const Temporal *temp);
extern char *tpoint_as_ewkt(const Temporal *temp, int maxdd);
extern char *tpoint_as_text(const Temporal *temp, int maxdd);
extern char *tpoint_out(const Temporal *temp, int maxdd);
extern Temporal *ttext_in(const char *str);
extern char *ttext_out(const Temporal *temp);

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *tbool_from_base(bool b, const Temporal *temp);
extern TInstant *tboolinst_make(bool b, TimestampTz t);
extern TSequence *tbooldiscseq_from_base_time(bool b, const Set *ts);
extern TSequence *tboolseq_from_base(bool b, const TSequence *seq);
extern TSequence *tboolseq_from_base_time(bool b, const Span *p);
extern TSequenceSet *tboolseqset_from_base(bool b, const TSequenceSet *ss);
extern TSequenceSet *tboolseqset_from_base_time(bool b, const SpanSet *ps);
extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *tfloat_from_base(double d, const Temporal *temp, interpType interp);
extern TInstant *tfloatinst_make(double d, TimestampTz t);
extern TSequence *tfloatdiscseq_from_base_time(double d, const Set *ts);
extern TSequence *tfloatseq_from_base(double d, const TSequence *seq, interpType interp);
extern TSequence *tfloatseq_from_base_time(double d, const Span *p, interpType interp);
extern TSequenceSet *tfloatseqset_from_base(double d, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tfloatseqset_from_base_time(double d, const SpanSet *ps, interpType interp);
extern Temporal *tgeogpoint_from_base(const GSERIALIZED *gs, const Temporal *temp, interpType interp);
extern TInstant *tgeogpointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tgeogpointdiscseq_from_base_time(const GSERIALIZED *gs, const Set *ts);
extern TSequence *tgeogpointseq_from_base(const GSERIALIZED *gs, const TSequence *seq, interpType interp);
extern TSequence *tgeogpointseq_from_base_time(const GSERIALIZED *gs, const Span *p, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_base(const GSERIALIZED *gs, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_base_time(const GSERIALIZED *gs, const SpanSet *ps, interpType interp);
extern Temporal *tgeompoint_from_base(const GSERIALIZED *gs, const Temporal *temp, interpType interp);
extern TInstant *tgeompointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tgeompointdiscseq_from_base_time(const GSERIALIZED *gs, const Set *ts);
extern TSequence *tgeompointseq_from_base(const GSERIALIZED *gs, const TSequence *seq, interpType interp);
extern TSequence *tgeompointseq_from_base_time(const GSERIALIZED *gs, const Span *p, interpType interp);
extern TSequenceSet *tgeompointseqset_from_base(const GSERIALIZED *gs, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tgeompointseqset_from_base_time(const GSERIALIZED *gs, const SpanSet *ps, interpType interp);
extern Temporal *tint_from_base(int i, const Temporal *temp);
extern TInstant *tintinst_make(int i, TimestampTz t);
extern TSequence *tintdiscseq_from_base_time(int i, const Set *ts);
extern TSequence *tintseq_from_base(int i, const TSequence *seq);
extern TSequence *tintseq_from_base_time(int i, const Span *p);
extern TSequenceSet *tintseqset_from_base(int i, const TSequenceSet *ss);
extern TSequenceSet *tintseqset_from_base_time(int i, const SpanSet *ps);
extern TSequence *tsequence_make(const TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_make_exp(const TInstant **instants, int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords,
  const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_make_free(TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_make(const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_exp(const TSequence **sequences, int count, int maxcount, bool normalize);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_gaps(const TInstant **instants, int count, interpType interp, float maxdist, Interval *maxt);
extern Temporal *ttext_from_base(const text *txt, const Temporal *temp);
extern TInstant *ttextinst_make(const text *txt, TimestampTz t);
extern TSequence *ttextdiscseq_from_base_time(const text *txt, const Set *ts);
extern TSequence *ttextseq_from_base(const text *txt, const TSequence *seq);
extern TSequence *ttextseq_from_base_time(const text *txt, const Span *p);
extern TSequenceSet *ttextseqset_from_base(const text *txt, const TSequenceSet *ss);
extern TSequenceSet *ttextseqset_from_base_time(const text *txt, const SpanSet *ps);

/*****************************************************************************/

/* Cast functions for temporal types */

extern Temporal *tfloat_to_tint(const Temporal *temp);
extern Temporal *tint_to_tfloat(const Temporal *temp);
extern Span *tnumber_to_span(const Temporal *temp);
extern Span *temporal_to_period(const Temporal *temp);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern bool tbool_end_value(const Temporal *temp);
extern bool tbool_start_value(const Temporal *temp);
extern bool *tbool_values(const Temporal *temp, int *count);
extern Interval *temporal_duration(const Temporal *temp, bool boundspan);
extern const TInstant *temporal_end_instant(const Temporal *temp);
extern TSequence *temporal_end_sequence(const Temporal *temp);
extern TimestampTz temporal_end_timestamp(const Temporal *temp);
extern uint32 temporal_hash(const Temporal *temp);
extern const TInstant *temporal_instant_n(const Temporal *temp, int n);
extern const TInstant **temporal_instants(const Temporal *temp, int *count);
extern char *temporal_interpolation(const Temporal *temp);
extern const TInstant *temporal_max_instant(const Temporal *temp);
extern const TInstant *temporal_min_instant(const Temporal *temp);
extern int temporal_num_instants(const Temporal *temp);
extern int temporal_num_sequences(const Temporal *temp);
extern int temporal_num_timestamps(const Temporal *temp);
extern TSequence **temporal_segments(const Temporal *temp, int *count);
extern TSequence *temporal_sequence_n(const Temporal *temp, int i);
extern TSequence **temporal_sequences(const Temporal *temp, int *count);
extern size_t temporal_memory_size(const Temporal *temp);
extern const TInstant *temporal_start_instant(const Temporal *temp);
extern TSequence *temporal_start_sequence(const Temporal *temp);
extern TimestampTz temporal_start_timestamp(const Temporal *temp);
extern char *temporal_subtype(const Temporal *temp);
extern SpanSet *temporal_time(const Temporal *temp);
extern bool temporal_timestamp_n(const Temporal *temp, int n, TimestampTz *result);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);
extern double tfloat_end_value(const Temporal *temp);
extern double tfloat_max_value(const Temporal *temp);
extern double tfloat_min_value(const Temporal *temp);
extern SpanSet *tfloat_spanset(const Temporal *temp);
extern double tfloat_start_value(const Temporal *temp);
extern double *tfloat_values(const Temporal *temp, int *count);
extern int tint_end_value(const Temporal *temp);
extern int tint_max_value(const Temporal *temp);
extern int tint_min_value(const Temporal *temp);
extern int tint_start_value(const Temporal *temp);
extern int *tint_values(const Temporal *temp, int *count);
extern GSERIALIZED *tpoint_end_value(const Temporal *temp);
extern GSERIALIZED *tpoint_start_value(const Temporal *temp);
extern GSERIALIZED **tpoint_values(const Temporal *temp, int *count);
extern text *ttext_end_value(const Temporal *temp);
extern text *ttext_max_value(const Temporal *temp);
extern text *ttext_min_value(const Temporal *temp);
extern text *ttext_start_value(const Temporal *temp);
extern text **ttext_values(const Temporal *temp, int *count);

/*****************************************************************************/

/* Transformation functions for temporal types */

extern Temporal *temporal_append_tinstant(Temporal *temp, const TInstant *inst, bool expand);
extern Temporal *temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);
extern Temporal *temporal_shift(const Temporal *temp, const Interval *shift);
extern Temporal *temporal_shift_tscale(const Temporal *temp, const Interval *shift, const Interval *duration);
extern Temporal *temporal_step_to_linear(const Temporal *temp);
extern Temporal *temporal_to_tinstant(const Temporal *temp);
extern Temporal *temporal_to_tdiscseq(const Temporal *temp);
extern Temporal *temporal_to_tcontseq(const Temporal *temp);
extern Temporal *temporal_to_tsequenceset(const Temporal *temp);
extern Temporal *temporal_tscale(const Temporal *temp, const Interval *duration);
extern Temporal *temporal_tprecision(const Temporal *temp, const Interval *duration, TimestampTz origin);
extern Temporal *temporal_tsample(const Temporal *temp, const Interval *duration, TimestampTz origin);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *tbool_at_value(const Temporal *temp, bool b);
extern Temporal *tbool_minus_value(const Temporal *temp, bool b);
extern bool tbool_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, bool *value);
extern Temporal *temporal_at_max(const Temporal *temp);
extern Temporal *temporal_at_min(const Temporal *temp);
extern Temporal *temporal_at_period(const Temporal *temp, const Span *p);
extern Temporal *temporal_at_periodset(const Temporal *temp, const SpanSet *ps);
extern Temporal *temporal_at_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_timestampset(const Temporal *temp, const Set *ts);
extern Temporal *temporal_minus_max(const Temporal *temp);
extern Temporal *temporal_minus_min(const Temporal *temp);
extern Temporal *temporal_minus_period(const Temporal *temp, const Span *p);
extern Temporal *temporal_minus_periodset(const Temporal *temp, const SpanSet *ps);
extern Temporal *temporal_minus_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_minus_timestampset(const Temporal *temp, const Set *ts);
extern Temporal *tfloat_at_value(const Temporal *temp, double d);
extern Temporal *tfloat_at_values(const Temporal *temp, const Set *set);
extern Temporal *tfloat_minus_value(const Temporal *temp, double d);
extern Temporal *tfloat_minus_values(const Temporal *temp, const Set *set);
extern bool tfloat_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, double *value);
extern Temporal *tint_at_value(const Temporal *temp, int i);
extern Temporal *tint_at_values(const Temporal *temp, const Set *set);
extern Temporal *tint_minus_value(const Temporal *temp, int i);
extern Temporal *tint_minus_values(const Temporal *temp, const Set *set);
extern bool tint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, int *value);
extern Temporal *tnumber_at_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_at_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBox *box);
extern Temporal *tnumber_minus_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_minus_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBox *box);
extern Temporal *tpoint_at_geometry(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpoint_at_stbox(const Temporal *temp, const STBox *box);
extern Temporal *tpoint_at_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_at_values(const Temporal *temp, const Set *set);
extern Temporal *tpoint_minus_geometry(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBox *box);
extern Temporal *tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_minus_values(const Temporal *temp, const Set *set);
extern bool tpoint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value);
extern TSequence *tsequence_at_period(const TSequence *seq, const Span *p);
extern Temporal *ttext_at_value(const Temporal *temp, text *txt);
extern Temporal *ttext_at_values(const Temporal *temp, const Set *set);
extern Temporal *ttext_minus_value(const Temporal *temp, text *txt);
extern Temporal *ttext_minus_values(const Temporal *temp, const Set *set);
extern bool ttext_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, text **value);

/*****************************************************************************/

/* Boolean functions for temporal types */

extern Temporal *tand_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tand_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tnot_tbool(const Temporal *temp);
extern Temporal *tor_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tor_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2);
extern SpanSet *tbool_when_true(const Temporal *temp);

/*****************************************************************************/

/* Mathematical functions for temporal types */

extern Temporal *add_float_tfloat(double d, const Temporal *tnumber);
extern Temporal *add_int_tint(int i, const Temporal *tnumber);
extern Temporal *add_tfloat_float(const Temporal *tnumber, double d);
extern Temporal *add_tint_int(const Temporal *tnumber, int i);
extern Temporal *add_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern double float_degrees(double value, bool normalize);
extern Temporal *div_float_tfloat(double d, const Temporal *tnumber);
extern Temporal *div_int_tint(int i, const Temporal *tnumber);
extern Temporal *div_tfloat_float(const Temporal *tnumber, double d);
extern Temporal *div_tint_int(const Temporal *tnumber, int i);
extern Temporal *div_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *mult_float_tfloat(double d, const Temporal *tnumber);
extern Temporal *mult_int_tint(int i, const Temporal *tnumber);
extern Temporal *mult_tfloat_float(const Temporal *tnumber, double d);
extern Temporal *mult_tint_int(const Temporal *tnumber, int i);
extern Temporal *mult_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *sub_float_tfloat(double d, const Temporal *tnumber);
extern Temporal *sub_int_tint(int i, const Temporal *tnumber);
extern Temporal *sub_tfloat_float(const Temporal *tnumber, double d);
extern Temporal *sub_tint_int(const Temporal *tnumber, int i);
extern Temporal *sub_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern Temporal *tfloat_degrees(const Temporal *temp, bool normalize);
extern Temporal *tfloat_radians(const Temporal *temp);
extern Temporal *tfloat_derivative(const Temporal *temp);
extern Temporal *tnumber_abs(const Temporal *temp);
extern Temporal *tnumber_delta_value(const Temporal *temp);

/*****************************************************************************/

/* Text functions for temporal types */

extern Temporal *textcat_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *textcat_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2);
extern Temporal *ttext_upper(const Temporal *temp);
extern Temporal *ttext_lower(const Temporal *temp);

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/* Topological functions for temporal types */

extern bool adjacent_float_tfloat(double d, const Temporal *tnumber);
extern bool adjacent_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool adjacent_int_tint(int i, const Temporal *tnumber);
extern bool adjacent_period_temporal(const Span *p, const Temporal *temp);
extern bool adjacent_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool adjacent_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool adjacent_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool adjacent_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool adjacent_temporal_period(const Temporal *temp, const Span *p);
extern bool adjacent_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool adjacent_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool adjacent_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool adjacent_tfloat_float(const Temporal *tnumber, double d);
extern bool adjacent_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool adjacent_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool adjacent_tint_int(const Temporal *tnumber, int i);
extern bool adjacent_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool adjacent_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool adjacent_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool adjacent_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool adjacent_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool adjacent_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contained_float_tfloat(double d, const Temporal *tnumber);
extern bool contained_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contained_int_tint(int i, const Temporal *tnumber);
extern bool contained_period_temporal(const Span *p, const Temporal *temp);
extern bool contained_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool contained_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contained_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool contained_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool contained_temporal_period(const Temporal *temp, const Span *p);
extern bool contained_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contained_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contained_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool contained_tfloat_float(const Temporal *tnumber, double d);
extern bool contained_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contained_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool contained_tint_int(const Temporal *tnumber, int i);
extern bool contained_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contained_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool contained_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool contained_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contained_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool contained_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contains_bbox_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contains_float_tfloat(double d, const Temporal *tnumber);
extern bool contains_int_tint(int i, const Temporal *tnumber);
extern bool contains_period_temporal(const Span *p, const Temporal *temp);
extern bool contains_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool contains_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contains_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool contains_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool contains_temporal_period(const Temporal *temp, const Span *p);
extern bool contains_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contains_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contains_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool contains_tfloat_float(const Temporal *tnumber, double d);
extern bool contains_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contains_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool contains_tint_int(const Temporal *tnumber, int i);
extern bool contains_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contains_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool contains_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool contains_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contains_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool contains_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool left_float_tfloat(double d, const Temporal *tnumber);
extern bool left_int_tint(int i, const Temporal *tnumber);
extern bool left_tfloat_float(const Temporal *tnumber, double d);
extern bool left_tint_int(const Temporal *tnumber, int i);
extern bool overlaps_float_tfloat(double d, const Temporal *tnumber);
extern bool overlaps_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overlaps_int_tint(int i, const Temporal *tnumber);
extern bool overlaps_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overlaps_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overlaps_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool overlaps_temporal_period(const Temporal *temp, const Span *p);
extern bool overlaps_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overlaps_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool overlaps_tfloat_float(const Temporal *tnumber, double d);
extern bool overlaps_tint_int(const Temporal *tnumber, int i);
extern bool overlaps_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overlaps_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool overlaps_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overlaps_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overlaps_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overlaps_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overleft_float_tfloat(double d, const Temporal *tnumber);
extern bool overleft_int_tint(int i, const Temporal *tnumber);
extern bool overleft_tfloat_float(const Temporal *tnumber, double d);
extern bool overleft_tint_int(const Temporal *tnumber, int i);
extern bool overright_float_tfloat(double d, const Temporal *tnumber);
extern bool overright_int_tint(int i, const Temporal *tnumber);
extern bool overright_tfloat_float(const Temporal *tnumber, double d);
extern bool overright_tint_int(const Temporal *tnumber, int i);
extern bool right_float_tfloat(double d, const Temporal *tnumber);
extern bool right_int_tint(int i, const Temporal *tnumber);
extern bool right_tfloat_float(const Temporal *tnumber, double d);
extern bool right_tint_int(const Temporal *tnumber, int i);
extern bool same_float_tfloat(double d, const Temporal *tnumber);
extern bool same_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool same_int_tint(int i, const Temporal *tnumber);
extern bool same_period_temporal(const Span *p, const Temporal *temp);
extern bool same_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool same_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool same_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool same_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool same_temporal_period(const Temporal *temp, const Span *p);
extern bool same_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool same_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool same_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool same_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool same_tfloat_float(const Temporal *tnumber, double d);
extern bool same_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool same_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool same_tint_int(const Temporal *tnumber, int i);
extern bool same_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool same_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool same_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool same_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool same_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool same_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool above_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool above_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool above_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool above_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool above_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool after_period_temporal(const Span *p, const Temporal *temp);
extern bool after_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool after_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool after_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool after_temporal_period(const Temporal *temp, const Span *p);
extern bool after_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool after_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool after_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool after_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool after_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool after_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool after_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool after_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool after_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool after_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool back_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool back_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool back_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool back_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool back_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool before_period_temporal(const Span *p, const Temporal *temp);
extern bool before_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool before_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool before_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool before_temporal_period(const Temporal *temp, const Span *p);
extern bool before_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool before_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool before_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool before_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool before_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool before_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool before_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool before_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool before_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool before_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool below_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool below_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool below_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool below_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool below_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool front_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool front_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool front_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool front_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool front_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool left_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool left_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool left_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool left_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool left_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool left_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool left_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool left_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool left_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool left_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overabove_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overabove_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overabove_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overabove_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overabove_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overafter_period_temporal(const Span *p, const Temporal *temp);
extern bool overafter_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool overafter_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overafter_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool overafter_temporal_period(const Temporal *temp, const Span *p);
extern bool overafter_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overafter_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool overafter_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overafter_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool overafter_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool overafter_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overafter_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overafter_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overback_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overback_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overback_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overback_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overback_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbefore_period_temporal(const Span *p, const Temporal *temp);
extern bool overbefore_periodset_temporal(const SpanSet *ps, const Temporal *temp);
extern bool overbefore_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overbefore_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool overbefore_temporal_period(const Temporal *temp, const Span *p);
extern bool overbefore_temporal_periodset(const Temporal *temp, const SpanSet *ps);
extern bool overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overbefore_temporal_timestampset(const Temporal *temp, const Set *ts);
extern bool overbefore_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overbefore_timestampset_temporal(const Set *ts, const Temporal *temp);
extern bool overbefore_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool overbefore_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overbefore_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overbefore_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbelow_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overbelow_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overbelow_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overbelow_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overbelow_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overfront_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overfront_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overfront_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overfront_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overfront_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overleft_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overleft_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overleft_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overleft_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool overleft_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overleft_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool overleft_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overleft_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overleft_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overleft_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overright_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overright_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overright_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool overright_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool overright_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overright_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool overright_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overright_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overright_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool overright_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool right_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool right_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool right_stbox_tpoint(const STBox *stbox, const Temporal *tpoint);
extern bool right_tbox_tnumber(const TBox *tbox, const Temporal *tnumber);
extern bool right_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool right_tnumber_tbox(const Temporal *tnumber, const TBox *tbox);
extern bool right_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool right_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool right_tpoint_stbox(const Temporal *tpoint, const STBox *stbox);
extern bool right_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tfloat_float(const Temporal *temp, double d);
extern Temporal *distance_tint_int(const Temporal *temp, int i);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern Temporal *distance_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo);
extern Temporal *distance_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_geo(const STBox *box, const GSERIALIZED *gs);
extern double nad_stbox_stbox(const STBox *box1, const STBox *box2);
extern double nad_tbox_tbox(const TBox *box1, const TBox *box2);
extern double nad_tfloat_float(const Temporal *temp, double d);
extern double nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2);
extern int nad_tint_int(const Temporal *temp, int i);
extern int nad_tint_tint(const Temporal *temp1, const Temporal *temp2);
extern double nad_tnumber_tbox(const Temporal *temp, const TBox *box);
extern double nad_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tpoint_stbox(const Temporal *temp, const STBox *box);
extern double nad_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern bool shortestline_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, GSERIALIZED **result);
extern bool shortestline_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, GSERIALIZED **result);

/*****************************************************************************/

/* Ever/always functions for temporal types */

extern bool tbool_always_eq(const Temporal *temp, bool b);
extern bool tbool_ever_eq(const Temporal *temp, bool b);
extern bool tfloat_always_eq(const Temporal *temp, double d);
extern bool tfloat_always_le(const Temporal *temp, double d);
extern bool tfloat_always_lt(const Temporal *temp, double d);
extern bool tfloat_ever_eq(const Temporal *temp, double d);
extern bool tfloat_ever_le(const Temporal *temp, double d);
extern bool tfloat_ever_lt(const Temporal *temp, double d);
extern bool tgeogpoint_always_eq(const Temporal *temp, GSERIALIZED *gs);;
extern bool tgeogpoint_ever_eq(const Temporal *temp, GSERIALIZED *gs);;
extern bool tgeompoint_always_eq(const Temporal *temp, GSERIALIZED *gs);
extern bool tgeompoint_ever_eq(const Temporal *temp, GSERIALIZED *gs);;
extern bool tint_always_eq(const Temporal *temp, int i);
extern bool tint_always_le(const Temporal *temp, int i);
extern bool tint_always_lt(const Temporal *temp, int i);
extern bool tint_ever_eq(const Temporal *temp, int i);
extern bool tint_ever_le(const Temporal *temp, int i);
extern bool tint_ever_lt(const Temporal *temp, int i);
extern bool ttext_always_eq(const Temporal *temp, text *txt);
extern bool ttext_always_le(const Temporal *temp, text *txt);
extern bool ttext_always_lt(const Temporal *temp, text *txt);
extern bool ttext_ever_eq(const Temporal *temp, text *txt);
extern bool ttext_ever_le(const Temporal *temp, text *txt);
extern bool ttext_ever_lt(const Temporal *temp, text *txt);

/*****************************************************************************/

/* Comparison functions for temporal types */

extern int temporal_cmp(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_eq(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ge(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_gt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_le(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_lt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ne(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_bool_tbool(bool b, const Temporal *temp);
extern Temporal *teq_float_tfloat(double d, const Temporal *temp);
extern Temporal *teq_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern Temporal *teq_int_tint(int i, const Temporal *temp);
extern Temporal *teq_point_tgeogpoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_point_tgeompoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_tbool_bool(const Temporal *temp, bool b);
extern Temporal *teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *teq_tfloat_float(const Temporal *temp, double d);
extern Temporal *teq_tgeogpoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *teq_tgeompoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *teq_tint_int(const Temporal *temp, int i);
extern Temporal *teq_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern Temporal *teq_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *tge_float_tfloat(double d, const Temporal *temp);
extern Temporal *tge_int_tint(int i, const Temporal *temp);
extern Temporal *tge_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tge_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tge_tfloat_float(const Temporal *temp, double d);
extern Temporal *tge_tint_int(const Temporal *temp, int i);
extern Temporal *tge_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *tgt_float_tfloat(double d, const Temporal *temp);
extern Temporal *tgt_int_tint(int i, const Temporal *temp);
extern Temporal *tgt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tgt_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tgt_tfloat_float(const Temporal *temp, double d);
extern Temporal *tgt_tint_int(const Temporal *temp, int i);
extern Temporal *tgt_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *tle_float_tfloat(double d, const Temporal *temp);
extern Temporal *tle_int_tint(int i, const Temporal *temp);
extern Temporal *tle_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tle_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tle_tfloat_float(const Temporal *temp, double d);
extern Temporal *tle_tint_int(const Temporal *temp, int i);
extern Temporal *tle_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *tlt_float_tfloat(double d, const Temporal *temp);
extern Temporal *tlt_int_tint(int i, const Temporal *temp);
extern Temporal *tlt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tlt_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tlt_tfloat_float(const Temporal *temp, double d);
extern Temporal *tlt_tint_int(const Temporal *temp, int i);
extern Temporal *tlt_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *tne_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tne_float_tfloat(double d, const Temporal *temp);
extern Temporal *tne_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern Temporal *tne_int_tint(int i, const Temporal *temp);
extern Temporal *tne_point_tgeogpoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_point_tgeompoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tne_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tne_tfloat_float(const Temporal *temp, double d);
extern Temporal *tne_tgeogpoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_tgeompoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_tint_int(const Temporal *temp, int i);
extern Temporal *tne_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern Temporal *tne_ttext_text(const Temporal *temp, const text *txt);

/*****************************************************************************
  Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */

extern bool bearing_point_point(const GSERIALIZED *geo1, const GSERIALIZED *geo2, double *result);
extern Temporal *bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert);
extern Temporal *bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tpoint_azimuth(const Temporal *temp);
extern GSERIALIZED *tpoint_convex_hull(const Temporal *temp);
extern Temporal *tpoint_cumulative_length(const Temporal *temp);
extern bool tpoint_direction(const Temporal *temp, double *result);
extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);
extern bool tpoint_is_simple(const Temporal *temp);
extern double tpoint_length(const Temporal *temp);
extern Temporal *tpoint_speed(const Temporal *temp);
extern int tpoint_srid(const Temporal *temp);
extern STBox *tpoint_stboxes(const Temporal *temp, int *count);
extern GSERIALIZED *tpoint_trajectory(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern STBox *geo_expand_space(const GSERIALIZED *gs, double d);
extern Temporal *tgeompoint_tgeogpoint(const Temporal *temp, bool oper);
extern STBox *tpoint_expand_space(const Temporal *temp, double d);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);
extern Temporal *tpoint_set_srid(const Temporal *temp, int32 srid);

/*****************************************************************************/

/* Spatial relationship functions for temporal point types */

extern int econtains_geo_tpoint(const GSERIALIZED *geo, const Temporal *temp);
extern int edisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tcontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *tintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, bool atvalue);
extern Temporal *ttouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);

/*****************************************************************************/

/* Modification functions for temporal types */

extern Temporal *temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect);
extern Temporal *temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect);
extern Temporal *temporal_delete_timestamp(const Temporal *temp, TimestampTz t, bool connect);
extern Temporal *temporal_delete_timestampset(const Temporal *temp, const Set *ts, bool connect);
extern Temporal *temporal_delete_period(const Temporal *temp, const Span *p, bool connect);
extern Temporal *temporal_delete_periodset(const Temporal *temp, const SpanSet *ps, bool connect);
extern TSequenceSet *temporal_stops(const Temporal *temp, double mindist, const Interval *minduration);

/*****************************************************************************/

/* Local and temporal aggregate functions for temporal types */

extern SkipList *tbool_tand_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tbool_tor_transfn(SkipList *state, const Temporal *temp);
extern Span *temporal_extent_transfn(Span *p, const Temporal *temp);
extern Temporal *temporal_tagg_finalfn(SkipList *state);
extern SkipList *temporal_tcount_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tmin_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tsum_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_tmin_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_tsum_transfn(SkipList *state, const Temporal *temp);
extern double tnumber_integral(const Temporal *temp);
extern TBox *tnumber_extent_transfn(TBox *box, const Temporal *temp);
extern Temporal *tnumber_tavg_finalfn(SkipList *state);
extern SkipList *tnumber_tavg_transfn(SkipList *state, const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);
extern STBox *tpoint_extent_transfn(STBox *box, const Temporal *temp);
extern GSERIALIZED *tpoint_twcentroid(const Temporal *temp);
extern SkipList *ttext_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *ttext_tmin_transfn(SkipList *state, const Temporal *temp);

/*****************************************************************************/

/* Tile functions for temporal types */

extern int int_bucket(int value, int size, int origin);
extern double float_bucket(double value, double size, double origin);
extern TimestampTz timestamptz_bucket(TimestampTz timestamp, const Interval *duration, TimestampTz origin);

extern Span *intspan_bucket_list(const Span *bounds, int size, int origin, int *newcount);
extern Span *floatspan_bucket_list(const Span *bounds, double size, double origin, int *newcount);
extern Span *period_bucket_list(const Span *bounds, const Interval *duration, TimestampTz origin, int *newcount);

extern TBox *tbox_tile_list(const TBox *bounds, double xsize, const Interval *duration, double xorigin, TimestampTz torigin, int *rows, int *columns);

extern Temporal **tint_value_split(Temporal *temp, int size, int origin, int *newcount);
extern Temporal **tfloat_value_split(Temporal *temp, double size, double origin, int *newcount);
extern Temporal **temporal_time_split(Temporal *temp, Interval *duration, TimestampTz torigin, int *newcount);
extern Temporal **tint_value_time_split(Temporal *temp, int size, int vorigin, Interval *duration, TimestampTz torigin, int *newcount);
extern Temporal **tfloat_value_time_split(Temporal *temp, double size, double vorigin, Interval *duration, TimestampTz torigin, int *newcount);

extern STBox *stbox_tile_list(STBox *bounds, double size, const Interval *duration, GSERIALIZED *sorigin, TimestampTz torigin, int **cellcount);

/*****************************************************************************/

/* Similarity functions for temporal types */

extern double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2);
extern double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern double temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Analytics functions for temporal types */

Temporal *geo_to_tpoint(const GSERIALIZED *geo);
Temporal *temporal_simplify_min_dist(const Temporal *temp, double dist);
Temporal *temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint);
Temporal *temporal_simplify_dp(const Temporal *temp, double eps_dist, bool synchronized);
Temporal *temporal_simplify_max_dist(const Temporal *temp, double eps_dist, bool synchronized);
bool tpoint_AsMVTGeom(const Temporal *temp, const STBox *bounds, int32_t extent,
  int32_t buffer, bool clip_geom, GSERIALIZED **geom, int64 **timesarr, int *count);
bool tpoint_to_geo_measure(const Temporal *tpoint, const Temporal *measure, bool segmentize, GSERIALIZED **result);

/*****************************************************************************/

#endif
