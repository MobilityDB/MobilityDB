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
#include <liblwgeom.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

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
 * Make the Period type as a Span type for facilitating the manipulation of
 * the time dimension
 */
typedef Span Period;

/**
 * Structure to represent timestamp sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  int32 count;          /**< Number of TimestampTz elements */
  Period period;        /**< Bounding period */
  TimestampTz elems[1]; /**< Beginning of variable-length data */
} TimestampSet;

/**
 * Structure to represent period sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  int32 count;          /**< Number of Period elements */
  Period period;        /**< Bounding period */
  Period elems[1];      /**< Beginning of variable-length data */
} PeriodSet;

/**
 * Structure to represent temporal boxes
 */
typedef struct
{
  Period      period; /**< time span */
  Span        span;   /**< value span */
  int16       flags;  /**< flags */
} TBOX;

/**
 * Structure to represent spatiotemporal boxes
 */
typedef struct
{
  Period      period; /**< time span */
  double      xmin;   /**< minimum x value */
  double      xmax;   /**< maximum x value */
  double      ymin;   /**< minimum y value */
  double      ymax;   /**< maximum y value */
  double      zmin;   /**< minimum z value */
  double      zmax;   /**< maximum z value */
  int32       srid;   /**< SRID */
  int16       flags;  /**< flags */
} STBOX;

/**
 * @brief Enumeration that defines the interpolation types used in
 * MobilityDB.
 */
typedef enum
{
  INTERP_NONE =    0,
  DISCRETE =       1,
  STEPWISE =       2,
  LINEAR =         3,
} interpType;

/**
 * Structure to represent the common structure of temporal values of
 * any temporal subtype
 */
typedef struct
{
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  /* variable-length data follows */
} Temporal;

/**
 * Structure to represent temporal values of instant subtype
 */
typedef struct
{
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  TimestampTz   t;            /**< Timestamp (8 bytes) */
  Datum         value;        /**< Base value for types passed by value,
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
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  int32         count;        /**< Number of TInstant elements */
  int32         maxcount;     /**< Maximum number of TInstant elements */
  int16         bboxsize;     /**< Size of the bounding box */
  Period        period;       /**< Time span (24 bytes). All bounding boxes
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
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  int32         count;        /**< Number of TSequence elements */
  int32         totalcount;   /**< Total number of TInstant elements in all
                                   composing TSequence elements */
  int32         maxcount;     /**< Maximum number of TSequence elements */
  int16         bboxsize;     /**< Size of the bounding box */
  Period        period;       /**< Time span (24 bytes). All bounding boxes
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

/*****************************************************************************
 * Initialization of the MEOS library
 *****************************************************************************/

extern void meos_initialize(void);
extern void meos_finish(void);

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
extern char *pg_interval_out(Interval *span);
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
extern char *gserialized_as_geojson(const GSERIALIZED *geom, int option, int precision, char *srs);
extern char *gserialized_as_hexewkb(const GSERIALIZED *geom, const char *type);
extern char *gserialized_as_text(const GSERIALIZED *geom, int precision);
extern GSERIALIZED *gserialized_from_ewkb(const bytea *bytea_wkb, int32 srid);
extern GSERIALIZED *gserialized_from_geojson(const char *geojson);
extern GSERIALIZED *gserialized_from_hexewkb(const char *wkt);
extern GSERIALIZED *gserialized_from_text(char *wkt, int srid);
extern GSERIALIZED *gserialized_in(char *input, int32 geom_typmod);
extern char *gserialized_out(const GSERIALIZED *geom);
extern bool gserialized_same(const GSERIALIZED *geom1, const GSERIALIZED *geom2);

/*****************************************************************************
 * Functions for span and time types
 *****************************************************************************/

/* Input/output functions for span and time types */

extern Span *floatspan_in(const char *str);
extern char *floatspan_out(const Span *s, int maxdd);
extern Span *intspan_in(const char *str);
extern char *intspan_out(const Span *s);
extern Period *period_in(const char *str);
extern char *period_out(const Span *s);
extern char *periodset_as_hexwkb(const PeriodSet *ps, uint8_t variant, size_t *size_out);
extern uint8_t *periodset_as_wkb(const PeriodSet *ps, uint8_t variant, size_t *size_out);
extern PeriodSet *periodset_from_hexwkb(const char *hexwkb);
extern PeriodSet *periodset_from_wkb(const uint8_t *wkb, int size);
extern PeriodSet *periodset_in(const char *str);
extern char *periodset_out(const PeriodSet *ps);
extern char *span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out);
extern uint8_t *span_as_wkb(const Span *s, uint8_t variant, size_t *size_out);
extern Span *span_from_hexwkb(const char *hexwkb);
extern Span *span_from_wkb(const uint8_t *wkb, int size);
extern char *span_out(const Span *s, Datum arg);
extern char *timestampset_as_hexwkb(const TimestampSet *ts, uint8_t variant, size_t *size_out);
extern uint8_t *timestampset_as_wkb(const TimestampSet *ts, uint8_t variant, size_t *size_out);
extern TimestampSet *timestampset_from_hexwkb(const char *hexwkb);
extern TimestampSet *timestampset_from_wkb(const uint8_t *wkb, int size);
extern TimestampSet *timestampset_in(const char *str);
extern char *timestampset_out(const TimestampSet *ts);

/*****************************************************************************/

/* Constructor functions for span and time types */

extern Span *floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc);
extern Span *intspan_make(int lower, int upper, bool lower_inc, bool upper_inc);
extern Period *period_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc);
extern PeriodSet *periodset_copy(const PeriodSet *ps);
extern PeriodSet *periodset_make(const Period **periods, int count, bool normalize);
extern PeriodSet *periodset_make_free(Period **periods, int count, bool normalize);
extern Span *span_copy(const Span *s);
extern TimestampSet *timestampset_copy(const TimestampSet *ts);
extern TimestampSet *timestampset_make(const TimestampTz *times, int count);
extern TimestampSet *timestampset_make_free(TimestampTz *times, int count);

/*****************************************************************************/

/* Cast functions for span and time types */

extern Span *float_to_floaspan(double d);
extern Span *int_to_intspan(int i);
extern PeriodSet *period_to_periodset(const Period *period);
extern Period *periodset_to_period(const PeriodSet *ps);
extern Period *timestamp_to_period(TimestampTz t);
extern PeriodSet *timestamp_to_periodset(TimestampTz t);
extern TimestampSet *timestamp_to_timestampset(TimestampTz t);
extern PeriodSet *timestampset_to_periodset(const TimestampSet *ts);

/*****************************************************************************/

/* Accessor functions for span and time types */

extern double floatspan_lower(const Span *s);
extern double floatspan_upper(const Span *s);
extern int intspan_lower(const Span *s);
extern int intspan_upper(const Span *s);
extern Interval *period_duration(const Span *s);
extern TimestampTz period_lower(const Period *p);
extern TimestampTz period_upper(const Period *p);
extern Interval *periodset_duration(const PeriodSet *ps);
extern Period *periodset_end_period(const PeriodSet *ps);
extern TimestampTz periodset_end_timestamp(const PeriodSet *ps);
extern uint32 periodset_hash(const PeriodSet *ps);
extern uint64 periodset_hash_extended(const PeriodSet *ps, uint64 seed);
extern int periodset_mem_size(const PeriodSet *ps);
extern int periodset_num_periods(const PeriodSet *ps);
extern int periodset_num_timestamps(const PeriodSet *ps);
extern Period *periodset_period_n(const PeriodSet *ps, int i);
extern const Period **periodset_periods(const PeriodSet *ps, int *count);
extern Period *periodset_start_period(const PeriodSet *ps);
extern TimestampTz periodset_start_timestamp(const PeriodSet *ps);
extern Interval *periodset_timespan(const PeriodSet *ps);
extern bool periodset_timestamp_n(const PeriodSet *ps, int n, TimestampTz *result);
extern TimestampTz *periodset_timestamps(const PeriodSet *ps, int *count);
extern uint32 span_hash(const Span *s);
extern uint64 span_hash_extended(const Span *s, uint64 seed);
extern bool span_lower_inc(const Span *s);
extern bool span_upper_inc(const Span *s);
extern double span_width(const Span *s);
extern TimestampTz timestampset_end_timestamp(const TimestampSet *ss);
extern uint32 timestampset_hash(const TimestampSet *ss);
extern uint64 timestampset_hash_extended(const TimestampSet *ss, uint64 seed);
extern int timestampset_mem_size(const TimestampSet *ss);
extern int timestampset_num_timestamps(const TimestampSet *ss);
extern TimestampTz timestampset_start_timestamp(const TimestampSet *ss);
extern Interval *timestampset_timespan(const TimestampSet *ss);
extern bool timestampset_timestamp_n(const TimestampSet *ss, int n, TimestampTz *result);
extern TimestampTz *timestampset_timestamps(const TimestampSet *ss);

/*****************************************************************************/

/* Transformation functions for span and time types */

extern PeriodSet *periodset_shift_tscale(const PeriodSet *ps, const Interval *start, const Interval *duration);
extern void span_expand(const Span *s1, Span *s2);
extern void period_shift_tscale(const Interval *start, const Interval *duration, Period *result);
extern TimestampSet *timestampset_shift_tscale(const TimestampSet *ss, const Interval *start, const Interval *duration);

/*****************************************************************************
 * Bounding box functions for span and time types
 *****************************************************************************/

/* Topological functions for span and time types */

extern bool adjacent_floatspan_float(const Span *s, double d);
extern bool adjacent_intspan_int(const Span *s, int i);
extern bool adjacent_period_periodset(const Period *p, const PeriodSet *ps);
extern bool adjacent_period_timestamp(const Period *p, TimestampTz t);
extern bool adjacent_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool adjacent_periodset_period(const PeriodSet *ps, const Period *p);
extern bool adjacent_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool adjacent_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool adjacent_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool adjacent_span_span(const Span *s1, const Span *s2);
extern bool adjacent_timestamp_period(TimestampTz t, const Period *p);
extern bool adjacent_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool adjacent_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool adjacent_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool contained_float_floatspan(double d, const Span *s);
extern bool contained_int_intspan(int i, const Span *s);
extern bool contained_period_periodset(const Period *p, const PeriodSet *ps);
extern bool contained_periodset_period(const PeriodSet *ps, const Period *p);
extern bool contained_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool contained_span_span(const Span *s1, const Span *s2);
extern bool contained_timestamp_period(TimestampTz t, const Period *p);
extern bool contained_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool contained_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool contained_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool contained_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool contained_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool contains_floatspan_float(const Span *s, double d);
extern bool contains_intspan_int(const Span *s, int i);
extern bool contains_period_periodset(const Period *p, const PeriodSet *ps);
extern bool contains_period_timestamp(const Period *p, TimestampTz t);
extern bool contains_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool contains_periodset_period(const PeriodSet *ps, const Period *p);
extern bool contains_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool contains_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool contains_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool contains_span_span(const Span *s1, const Span *s2);
extern bool contains_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool contains_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overlaps_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overlaps_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overlaps_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overlaps_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overlaps_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overlaps_span_span(const Span *s1, const Span *s2);
extern bool overlaps_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overlaps_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overlaps_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
/*****************************************************************************/

/* Position functions for span and time types */

extern bool after_period_periodset(const Period *p, const PeriodSet *ps);
extern bool after_period_timestamp(const Period *p, TimestampTz t);
extern bool after_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool after_periodset_period(const PeriodSet *ps, const Period *p);
extern bool after_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool after_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool after_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool after_timestamp_period(TimestampTz t, const Period *p);
extern bool after_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool after_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool after_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool after_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool after_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool after_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool before_period_periodset(const Period *p, const PeriodSet *ps);
extern bool before_period_timestamp(const Period *p, TimestampTz t);
extern bool before_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool before_periodset_period(const PeriodSet *ps, const Period *p);
extern bool before_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool before_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool before_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool before_timestamp_period(TimestampTz t, const Period *p);
extern bool before_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool before_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool before_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool before_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool before_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool before_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool left_float_floatspan(double d, const Span *s);
extern bool left_floatspan_float(const Span *s, double d);
extern bool left_int_intspan(int i, const Span *s);
extern bool left_intspan_int(const Span *s, int i);
extern bool left_span_span(const Span *s1, const Span *s2);
extern bool overafter_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overafter_period_timestamp(const Period *p, TimestampTz t);
extern bool overafter_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overafter_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overafter_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overafter_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool overafter_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overafter_timestamp_period(TimestampTz t, const Period *p);
extern bool overafter_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool overafter_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool overafter_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overafter_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overafter_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool overafter_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overbefore_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overbefore_period_timestamp(const Period *p, TimestampTz t);
extern bool overbefore_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool overbefore_periodset_period(const PeriodSet *ps, const Period *p);
extern bool overbefore_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool overbefore_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool overbefore_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern bool overbefore_timestamp_period(TimestampTz t, const Period *p);
extern bool overbefore_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool overbefore_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern bool overbefore_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool overbefore_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool overbefore_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern bool overbefore_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool overleft_float_floatspan(double d, const Span *s);
extern bool overleft_floatspan_float(const Span *s, double d);
extern bool overleft_int_intspan(int i, const Span *s);
extern bool overleft_intspan_int(const Span *s, int i);
extern bool overleft_span_span(const Span *s1, const Span *s2);
extern bool overright_float_floatspan(double d, const Span *s);
extern bool overright_floatspan_float(const Span *s, double d);
extern bool overright_int_intspan(int i, const Span *s);
extern bool overright_intspan_int(const Span *s, int i);
extern bool overright_span_span(const Span *s1, const Span *s2);
extern bool right_float_floatspan(double d, const Span *s);
extern bool right_floatspan_float(const Span *s, double d);
extern bool right_int_intspan(int i, const Span *s);
extern bool right_intspan_int(const Span *s, int i);
extern bool right_span_span(const Span *s1, const Span *s2);


/*****************************************************************************/

/* Set functions for span and time types */

extern PeriodSet *intersection_period_periodset(const Period *p, const PeriodSet *ps);
extern bool intersection_period_timestamp(const Period *p, TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_period_timestampset(const Period *ps, const TimestampSet *ts);
extern PeriodSet *intersection_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *intersection_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool intersection_periodset_timestamp(const PeriodSet *ps, TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern Span *intersection_span_span(const Span *s1, const Span *s2);
extern bool intersection_timestamp_period(TimestampTz t, const Period *p, TimestampTz *result);
extern bool intersection_timestamp_periodset(TimestampTz t, const PeriodSet *ps, TimestampTz *result);
extern bool intersection_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result);
extern bool intersection_timestamp_timestampset(TimestampTz t, const TimestampSet *ts, TimestampTz *result);
extern TimestampSet *intersection_timestampset_period(const TimestampSet *ts, const Period *p);
extern TimestampSet *intersection_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern bool intersection_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern PeriodSet *minus_period_period(const Period *p1, const Period *p2);
extern PeriodSet *minus_period_periodset(const Period *p, const PeriodSet *ps);
extern PeriodSet *minus_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *minus_period_timestampset(const Period *p, const TimestampSet *ts);
extern PeriodSet *minus_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *minus_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern PeriodSet *minus_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern PeriodSet *minus_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern Span *minus_span_span(const Span *s1, const Span *s2);
extern bool minus_timestamp_period(TimestampTz t, const Period *p, TimestampTz *result);
extern bool minus_timestamp_periodset(TimestampTz t, const PeriodSet *ps, TimestampTz *result);
extern bool minus_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result);
extern bool minus_timestamp_timestampset(TimestampTz t, const TimestampSet *ts, TimestampTz *result);
extern TimestampSet *minus_timestampset_period(const TimestampSet *ts, const Period *p);
extern TimestampSet *minus_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern TimestampSet *minus_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern TimestampSet *minus_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);
extern PeriodSet *union_period_period(const Period *p1, const Period *p2);
extern PeriodSet *union_period_periodset(const Period *p, const PeriodSet *ps);
extern PeriodSet *union_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *union_period_timestampset(const Period *p, const TimestampSet *ts);
extern PeriodSet *union_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *union_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern PeriodSet *union_periodset_timestamp(PeriodSet *ps, TimestampTz t);
extern PeriodSet *union_periodset_timestampset(PeriodSet *ps, TimestampSet *ts);
extern Span *union_span_span(const Span *s1, const Span *s2, bool strict);
extern PeriodSet *union_timestamp_period(TimestampTz t, const Period *p);
extern PeriodSet *union_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern TimestampSet *union_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern TimestampSet *union_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern PeriodSet *union_timestampset_period(const TimestampSet *ts, const Period *p);
extern PeriodSet *union_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern TimestampSet *union_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t);
extern TimestampSet *union_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);

/*****************************************************************************/

/* Distance functions for span and time types */

extern double distance_floatspan_float(const Span *s, double d);
extern double distance_intspan_int(const Span *s, int i);
extern double distance_period_periodset(const Period *p, const PeriodSet *ps);
extern double distance_period_timestamp(const Period *p, TimestampTz t);
extern double distance_period_timestampset(const Period *p, const TimestampSet *ts);
extern double distance_periodset_period(const PeriodSet *ps, const Period *p);
extern double distance_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2);
extern double distance_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern double distance_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts);
extern double distance_span_span(const Span *s1, const Span *s2);
extern double distance_timestamp_period(TimestampTz t, const Period *p);
extern double distance_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern double distance_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern double distance_timestamp_timestampset(TimestampTz t, const TimestampSet *ts);
extern double distance_timestampset_period(const TimestampSet *ts, const Period *p);
extern double distance_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps);
extern double distance_timestampset_timestamp(const TimestampSet *ts, TimestampTz t);
extern double distance_timestampset_timestampset(const TimestampSet *ts1, const TimestampSet *ts2);

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
extern bool timestampset_eq(const TimestampSet *ss1, const TimestampSet *ss2);
extern bool timestampset_ne(const TimestampSet *ss1, const TimestampSet *ss2);
extern int timestampset_cmp(const TimestampSet *ss1, const TimestampSet *ss2);
extern bool timestampset_lt(const TimestampSet *ss1, const TimestampSet *ss2);
extern bool timestampset_le(const TimestampSet *ss1, const TimestampSet *ss2);
extern bool timestampset_ge(const TimestampSet *ss1, const TimestampSet *ss2);
extern bool timestampset_gt(const TimestampSet *ss1, const TimestampSet *ss2);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */

extern TBOX *tbox_in(const char *str);
extern char *tbox_out(const TBOX *box, int maxdd);
extern TBOX *tbox_from_wkb(const uint8_t *wkb, int size);
extern TBOX *tbox_from_hexwkb(const char *hexwkb);
extern STBOX *stbox_from_wkb(const uint8_t *wkb, int size);
extern STBOX *stbox_from_hexwkb(const char *hexwkb);
extern uint8_t *tbox_as_wkb(const TBOX *box, uint8_t variant, size_t *size_out);
extern char *tbox_as_hexwkb(const TBOX *box, uint8_t variant, size_t *size);
extern uint8_t *stbox_as_wkb(const STBOX *box, uint8_t variant, size_t *size_out);
extern char *stbox_as_hexwkb(const STBOX *box, uint8_t variant, size_t *size);
extern STBOX *stbox_in(const char *str);
extern char *stbox_out(const STBOX *box, int maxdd);

/*****************************************************************************/

/* Constructor functions for box types */

extern TBOX *tbox_make(const Period *p, const Span *s);
extern void tbox_set(const Period *p, const Span *s, TBOX *box);
extern TBOX *tbox_copy(const TBOX *box);
extern STBOX * stbox_make(const Period *p, bool hasx, bool hasz, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
extern void stbox_set(const Period *p, bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, STBOX *box);
extern STBOX *stbox_copy(const STBOX *box);

/*****************************************************************************/

/* Cast functions for box types */

extern TBOX *int_to_tbox(int i);
extern TBOX *float_to_tbox(double d);
extern TBOX *span_to_tbox(const Span *span);
extern TBOX *timestamp_to_tbox(TimestampTz t);
extern TBOX *timestampset_to_tbox(const TimestampSet *ss);
extern TBOX *period_to_tbox(const Period *p);
extern TBOX *periodset_to_tbox(const PeriodSet *ps);
extern TBOX *int_timestamp_to_tbox(int i, TimestampTz t);
extern TBOX *float_timestamp_to_tbox(double d, TimestampTz t);
extern TBOX *int_period_to_tbox(int i, const Period *p);
extern TBOX *float_period_to_tbox(double d, const Period *p);
extern TBOX *span_timestamp_to_tbox(const Span *span, TimestampTz t);
extern TBOX *span_period_to_tbox(const Span *span, const Period *p);
extern Span *tbox_to_floatspan(const TBOX *box);
extern Period *tbox_to_period(const TBOX *box);
extern Period *stbox_to_period(const STBOX *box);
extern TBOX *tnumber_to_tbox(const Temporal *temp);
extern GSERIALIZED *stbox_to_geo(const STBOX *box);
extern STBOX *tpoint_to_stbox(const Temporal *temp);
extern STBOX *geo_to_stbox(const GSERIALIZED *gs);
extern STBOX *timestamp_to_stbox(TimestampTz t);
extern STBOX *timestampset_to_stbox(const TimestampSet *ts);
extern STBOX *period_to_stbox(const Period *p);
extern STBOX *periodset_to_stbox(const PeriodSet *ps);
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
extern int32 stbox_srid(const STBOX *box);

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
extern TSequence *tbooldiscseq_from_base(bool b, const TSequence *is);
extern TSequence *tbooldiscseq_from_base_time(bool b, const TimestampSet *ts);
extern TSequence *tboolseq_from_base(bool b, const TSequence *seq);
extern TSequence *tboolseq_from_base_time(bool b, const Period *p);
extern TSequenceSet *tboolseqset_from_base(bool b, const TSequenceSet *ss);
extern TSequenceSet *tboolseqset_from_base_time(bool b, const PeriodSet *ps);
extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *tfloat_from_base(bool b, const Temporal *temp, interpType interp);
extern TInstant *tfloatinst_make(double d, TimestampTz t);
extern TSequence *tfloatdiscseq_from_base_time(bool b, const TimestampSet *ts);
extern TSequence *tfloatseq_from_base(bool b, const TSequence *seq, interpType interp);
extern TSequence *tfloatseq_from_base_time(bool b, const Period *p, interpType interp);
extern TSequenceSet *tfloatseqset_from_base(bool b, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tfloatseqset_from_base_time(bool b, const PeriodSet *ps, interpType interp);
extern Temporal *tgeogpoint_from_base(const GSERIALIZED *gs, const Temporal *temp, interpType interp);
extern TInstant *tgeogpointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tgeogpointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts);
extern TSequence *tgeogpointseq_from_base(const GSERIALIZED *gs, const TSequence *seq, interpType interp);
extern TSequence *tgeogpointseq_from_base_time(const GSERIALIZED *gs, const Period *p, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_base(const GSERIALIZED *gs, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_base_time(const GSERIALIZED *gs, const PeriodSet *ps, interpType interp);
extern Temporal *tgeompoint_from_base(const GSERIALIZED *gs, const Temporal *temp, interpType interp);
extern TInstant *tgeompointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tgeompointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts);
extern TSequence *tgeompointseq_from_base(const GSERIALIZED *gs, const TSequence *seq, interpType interp);
extern TSequence *tgeompointseq_from_base_time(const GSERIALIZED *gs, const Period *p, interpType interp);
extern TSequenceSet *tgeompointseqset_from_base(const GSERIALIZED *gs, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tgeompointseqset_from_base_time(const GSERIALIZED *gs, const PeriodSet *ps, interpType interp);
extern Temporal *tint_from_base(int i, const Temporal *temp);
extern TInstant *tintinst_make(int i, TimestampTz t);
extern TSequence *tintdiscseq_from_base_time(int i, const TimestampSet *ts);
extern TSequence *tintseq_from_base(int i, const TSequence *seq);
extern TSequence *tintseq_from_base_time(int i, const Period *p);
extern TSequenceSet *tintseqset_from_base(int i, const TSequenceSet *ss);
extern TSequenceSet *tintseqset_from_base_time(int i, const PeriodSet *ps);
extern TSequence *tsequence_make(const TInstant **instants, int count,  int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords,
  const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_make_free(TInstant **instants, int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_make(const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_gaps(const TInstant **instants, int count, interpType interp, float maxdist, Interval *maxt);
extern Temporal *ttext_from_base(const text *txt, const Temporal *temp);
extern TInstant *ttextinst_make(const text *txt, TimestampTz t);
extern TSequence *ttextdiscseq_from_base_time(const text *txt, const TimestampSet *ts);
extern TSequence *ttextseq_from_base(const text *txt, const TSequence *seq);
extern TSequence *ttextseq_from_base_time(const text *txt, const Period *p);
extern TSequenceSet *ttextseqset_from_base(const text *txt, const TSequenceSet *ss);
extern TSequenceSet *ttextseqset_from_base_time(const text *txt, const PeriodSet *ps);

/*****************************************************************************/

/* Cast functions for temporal types */

extern Temporal *tfloat_to_tint(const Temporal *temp);
extern Temporal *tint_to_tfloat(const Temporal *temp);
extern Span *tnumber_to_span(const Temporal *temp);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern bool tbool_end_value(const Temporal *temp);
extern bool tbool_start_value(const Temporal *temp);
extern bool *tbool_values(const Temporal *temp, int *count);
extern Interval *temporal_duration(const Temporal *temp);
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
extern const TInstant *temporal_start_instant(const Temporal *temp);
extern TSequence *temporal_start_sequence(const Temporal *temp);
extern TimestampTz temporal_start_timestamp(const Temporal *temp);
extern char *temporal_subtype(const Temporal *temp);
extern PeriodSet *temporal_time(const Temporal *temp);
extern Interval *temporal_timespan(const Temporal *temp);
extern bool temporal_timestamp_n(const Temporal *temp, int n, TimestampTz *result);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);
extern double tfloat_end_value(const Temporal *temp);
extern double tfloat_max_value(const Temporal *temp);
extern double tfloat_min_value(const Temporal *temp);
extern Span **tfloat_spans(const Temporal *temp, int *count);
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

extern TSequence *tsequence_compact(const TSequence *seq);
extern Temporal *temporal_append_tinstant(Temporal *temp, const TInstant *inst, bool expand);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);
extern Temporal *temporal_shift_tscale(const Temporal *temp, const Interval *shift, const Interval *duration);
extern Temporal *temporal_step_to_linear(const Temporal *temp);
extern Temporal *temporal_to_tinstant(const Temporal *temp);
extern Temporal *temporal_to_tdiscseq(const Temporal *temp);
extern Temporal *temporal_to_tsequence(const Temporal *temp);
extern Temporal *temporal_to_tsequenceset(const Temporal *temp);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *tbool_at_value(const Temporal *temp, bool b);
extern Temporal *tbool_at_values(const Temporal *temp, bool *values, int count);
extern Temporal *tbool_minus_value(const Temporal *temp, bool b);
extern Temporal *tbool_minus_values(const Temporal *temp, bool *values, int count);
extern bool tbool_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, bool *value);
extern Temporal *temporal_at_max(const Temporal *temp);
extern Temporal *temporal_at_min(const Temporal *temp);
extern Temporal *temporal_at_period(const Temporal *temp, const Period *p);
extern Temporal *temporal_at_periodset(const Temporal *temp, const PeriodSet *ps);
extern Temporal *temporal_at_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_timestampset(const Temporal *temp, const TimestampSet *ts);
extern Temporal *temporal_minus_max(const Temporal *temp);
extern Temporal *temporal_minus_min(const Temporal *temp);
extern Temporal *temporal_minus_period(const Temporal *temp, const Period *p);
extern Temporal *temporal_minus_periodset(const Temporal *temp, const PeriodSet *ps);
extern Temporal *temporal_minus_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_minus_timestampset(const Temporal *temp, const TimestampSet *ts);
extern Temporal *tfloat_at_value(const Temporal *temp, double d);
extern Temporal *tfloat_at_values(const Temporal *temp, double *values, int count);
extern Temporal *tfloat_minus_value(const Temporal *temp, double d);
extern Temporal *tfloat_minus_values(const Temporal *temp, double *values, int count);
extern bool tfloat_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, double *value);
extern Temporal *tint_at_value(const Temporal *temp, int i);
extern Temporal *tint_at_values(const Temporal *temp, int *values, int count);
extern Temporal *tint_minus_value(const Temporal *temp, int i);
extern Temporal *tint_minus_values(const Temporal *temp, int *values, int count);
extern bool tint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, int *value);
extern Temporal *tnumber_at_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_at_spans(const Temporal *temp, Span **spans, int count);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *tnumber_minus_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_minus_spans(const Temporal *temp, Span **spans, int count);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *tpoint_at_geometry(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpoint_at_stbox(const Temporal *temp, const STBOX *box);
extern Temporal *tpoint_at_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_at_values(const Temporal *temp, GSERIALIZED **values, int count);
extern Temporal *tpoint_minus_geometry(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBOX *box);
extern Temporal *tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_minus_values(const Temporal *temp, GSERIALIZED **values, int count);
extern bool tpoint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value);
extern Temporal *ttext_at_value(const Temporal *temp, text *txt);
extern Temporal *ttext_at_values(const Temporal *temp, text **values, int count);
extern Temporal *ttext_minus_value(const Temporal *temp, text *txt);
extern Temporal *ttext_minus_values(const Temporal *temp, text **values, int count);
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

/*****************************************************************************/

/* Mathematical functions for temporal types */

extern Temporal *add_float_tfloat(double d, const Temporal *tnumber);
extern Temporal *add_int_tint(int i, const Temporal *tnumber);
extern Temporal *add_tfloat_float(const Temporal *tnumber, double d);
extern Temporal *add_tint_int(const Temporal *tnumber, int i);
extern Temporal *add_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
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
extern Temporal *tnumber_degrees(const Temporal *temp);
extern Temporal *tnumber_derivative(const Temporal *temp);


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
extern bool adjacent_period_temporal(const Period *p, const Temporal *temp);
extern bool adjacent_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool adjacent_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool adjacent_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool adjacent_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool adjacent_temporal_period(const Temporal *temp, const Period *p);
extern bool adjacent_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool adjacent_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool adjacent_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool adjacent_tfloat_float(const Temporal *tnumber, double d);
extern bool adjacent_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool adjacent_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool adjacent_tint_int(const Temporal *tnumber, int i);
extern bool adjacent_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool adjacent_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool adjacent_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool adjacent_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool adjacent_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool adjacent_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contained_float_tfloat(double d, const Temporal *tnumber);
extern bool contained_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contained_int_tint(int i, const Temporal *tnumber);
extern bool contained_period_temporal(const Period *p, const Temporal *temp);
extern bool contained_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool contained_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contained_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool contained_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool contained_temporal_period(const Temporal *temp, const Period *p);
extern bool contained_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contained_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contained_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool contained_tfloat_float(const Temporal *tnumber, double d);
extern bool contained_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contained_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool contained_tint_int(const Temporal *tnumber, int i);
extern bool contained_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contained_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool contained_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool contained_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contained_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool contained_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool contains_bbox_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool contains_float_tfloat(double d, const Temporal *tnumber);
extern bool contains_int_tint(int i, const Temporal *tnumber);
extern bool contains_period_temporal(const Period *p, const Temporal *temp);
extern bool contains_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool contains_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool contains_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool contains_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool contains_temporal_period(const Temporal *temp, const Period *p);
extern bool contains_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contains_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool contains_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool contains_tfloat_float(const Temporal *tnumber, double d);
extern bool contains_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool contains_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool contains_tint_int(const Temporal *tnumber, int i);
extern bool contains_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool contains_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool contains_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool contains_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool contains_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool contains_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool left_float_tfloat(double d, const Temporal *tnumber);
extern bool left_int_tint(int i, const Temporal *tnumber);
extern bool left_tfloat_float(const Temporal *tnumber, double d);
extern bool left_tint_int(const Temporal *tnumber, int i);
extern bool overlaps_float_tfloat(double d, const Temporal *tnumber);
extern bool overlaps_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overlaps_int_tint(int i, const Temporal *tnumber);
extern bool overlaps_period_temporal(const Period *p, const Temporal *temp);
extern bool overlaps_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overlaps_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overlaps_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overlaps_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overlaps_temporal_period(const Temporal *temp, const Period *p);
extern bool overlaps_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overlaps_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overlaps_tfloat_float(const Temporal *tnumber, double d);
extern bool overlaps_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overlaps_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overlaps_tint_int(const Temporal *tnumber, int i);
extern bool overlaps_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overlaps_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overlaps_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overlaps_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overlaps_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
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
extern bool same_period_temporal(const Period *p, const Temporal *temp);
extern bool same_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool same_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool same_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool same_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool same_temporal_period(const Temporal *temp, const Period *p);
extern bool same_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool same_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool same_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool same_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool same_tfloat_float(const Temporal *tnumber, double d);
extern bool same_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool same_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool same_tint_int(const Temporal *tnumber, int i);
extern bool same_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool same_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool same_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool same_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool same_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool same_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool above_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool above_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool above_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool above_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool above_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool after_period_temporal(const Period *p, const Temporal *temp);
extern bool after_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool after_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool after_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool after_temporal_period(const Temporal *temp, const Period *p);
extern bool after_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool after_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool after_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool after_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool after_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool after_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool after_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool after_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool after_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool after_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool back_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool back_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool back_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool back_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool back_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool before_period_temporal(const Period *p, const Temporal *temp);
extern bool before_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool before_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool before_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool before_temporal_period(const Temporal *temp, const Period *p);
extern bool before_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool before_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool before_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool before_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool before_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool before_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool before_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool before_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool before_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool before_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool below_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool below_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool below_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool below_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool below_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool front_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool front_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool front_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool front_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool front_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool left_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool left_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool left_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool left_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool left_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool left_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool left_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool left_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool left_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool left_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overabove_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overabove_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overabove_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overabove_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overabove_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overafter_period_temporal(const Period *p, const Temporal *temp);
extern bool overafter_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overafter_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overafter_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overafter_temporal_period(const Temporal *temp, const Period *p);
extern bool overafter_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overafter_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overafter_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overafter_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overafter_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overafter_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overafter_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overafter_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overback_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overback_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overback_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overback_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overback_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbefore_period_temporal(const Period *p, const Temporal *temp);
extern bool overbefore_periodset_temporal(const PeriodSet *ps, const Temporal *temp);
extern bool overbefore_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overbefore_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overbefore_temporal_period(const Temporal *temp, const Period *p);
extern bool overbefore_temporal_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_temporal_timestamp(const Temporal *temp, TimestampTz t);
extern bool overbefore_temporal_timestampset(const Temporal *temp, const TimestampSet *ts);
extern bool overbefore_timestamp_temporal(TimestampTz t, const Temporal *temp);
extern bool overbefore_timestampset_temporal(const TimestampSet *ts, const Temporal *temp);
extern bool overbefore_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overbefore_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overbefore_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overbefore_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overbelow_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overbelow_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overbelow_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overbelow_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overbelow_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overfront_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overfront_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overfront_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overfront_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overfront_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overleft_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overleft_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overleft_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overleft_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overleft_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overleft_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overleft_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overleft_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overleft_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overleft_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool overright_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool overright_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool overright_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool overright_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool overright_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool overright_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool overright_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool overright_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool overright_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool overright_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);
extern bool right_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint);
extern bool right_span_tnumber(const Span *span, const Temporal *tnumber);
extern bool right_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint);
extern bool right_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber);
extern bool right_tnumber_span(const Temporal *tnumber, const Span *span);
extern bool right_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox);
extern bool right_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2);
extern bool right_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo);
extern bool right_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox);
extern bool right_tpoint_tpoint(const Temporal *tpoint1, const Temporal *tpoint2);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tfloat_float(const Temporal *temp, double d);
extern Temporal *distance_tint_int(const Temporal *temp, int i);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern Temporal *distance_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo);
extern Temporal *distance_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_geo(const STBOX *box, const GSERIALIZED *gs);
extern double nad_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern double nad_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern double nad_tfloat_float(const Temporal *temp, double d);
extern double nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2);
extern int nad_tint_int(const Temporal *temp, int i);
extern int nad_tint_tint(const Temporal *temp1, const Temporal *temp2);
extern double nad_tnumber_tbox(const Temporal *temp, const TBOX *box);
extern double nad_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tpoint_stbox(const Temporal *temp, const STBOX *box);
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
extern Temporal *tpoint_cumulative_length(const Temporal *temp);
extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);
extern bool tpoint_is_simple(const Temporal *temp);
extern double tpoint_length(const Temporal *temp);
extern Temporal *tpoint_speed(const Temporal *temp);
extern int tpoint_srid(const Temporal *temp);
extern STBOX *tpoint_stboxes(const Temporal *temp, int *count);
extern GSERIALIZED *tpoint_trajectory(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern STBOX *geo_expand_spatial(const GSERIALIZED *gs, double d);
extern Temporal *tgeompoint_tgeogpoint(const Temporal *temp, bool oper);
extern STBOX *tpoint_expand_spatial(const Temporal *temp, double d);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);
extern Temporal *tpoint_set_srid(const Temporal *temp, int32 srid);

/*****************************************************************************/

/* Spatial relationship functions for temporal point types */

extern int contains_geo_tpoint(const GSERIALIZED *geo, const Temporal *temp);
extern int disjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int disjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int dwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int dwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist);
extern int intersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int intersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tcontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *tintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr, bool atvalue);
extern int touches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *ttouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);

/*****************************************************************************/

/* Time functions for temporal types */

extern bool temporal_intersects_period(const Temporal *temp, const Period *p);
extern bool temporal_intersects_periodset(const Temporal *temp, const PeriodSet *ps);
extern bool temporal_intersects_timestamp(const Temporal *temp, TimestampTz t);
extern bool temporal_intersects_timestampset(const Temporal *temp, const TimestampSet *ss);

/*****************************************************************************/

/* Local aggregate functions for temporal types */

extern double tnumber_integral(const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);
extern GSERIALIZED *tpoint_twcentroid(const Temporal *temp);

/*****************************************************************************/

/* Tile functions for temporal types */

extern Temporal **temporal_time_split(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **buckets, int *newcount);
extern Temporal **tint_value_split(const Temporal *temp, int start_bucket,
  int size, int count, int **buckets, int *newcount);
extern Temporal **tfloat_value_split(const Temporal *temp, double start_bucket,
  double size, int count, float **buckets, int *newcount);

/*****************************************************************************/

/* Similarity functions for temporal types */

extern double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2);
extern double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count);

/*****************************************************************************/

/* Analytics functions for temporal types */

Temporal *geo_to_tpoint(const GSERIALIZED *geo);
Temporal *temporal_simplify(const Temporal *temp, double eps_dist, bool synchronized);
bool tpoint_AsMVTGeom(const Temporal *temp, const STBOX *bounds, int32_t extent,
  int32_t buffer, bool clip_geom, GSERIALIZED **geom, int64 **timesarr, int *count);
bool tpoint_to_geo_measure(const Temporal *tpoint, const Temporal *measure, bool segmentize, GSERIALIZED **result);

/*****************************************************************************/

#endif
