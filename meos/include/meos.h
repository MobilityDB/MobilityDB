/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @file
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 */

#ifndef __MEOS_H__
#define __MEOS_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL */
#if MEOS
#include "postgres_int_defs.h"
#else
#include <postgres.h>
#include <utils/date.h>
#include <utils/timestamp.h>
#endif

/*****************************************************************************
 * Toolchain dependent definitions
 *****************************************************************************/

#ifdef _MSC_VER
/*
 * Under MSVC, functions exported by a loadable module must be marked
 * "dllexport".  Other compilers don't need that.
 * Borrowed from PostgreSQL file win32.h
 */
#define PGDLLEXPORT __declspec (dllexport)
/*
 * Avoids warning C4996: 'strdup': The POSIX name for this item is deprecated.
 */
#define strdup _strdup
#endif

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/**
 * @brief Align to double
 */
#define DOUBLE_PAD(size) ( (size) + ((size) % 8 ? (8 - (size) % 8) : 0 ) )

/**
 * Structure to represent sets of values
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 settype;        /**< Set type */
  uint8 basetype;       /**< Span basetype */
  int16 flags;          /**< Flags */
  int32 count;          /**< Number of elements */
  int32 maxcount;       /**< Maximum number of elements */
  int16 bboxsize;       /**< Size of the bouding box */
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
  char padding[4];      /**< Not used */
  Datum lower;          /**< lower bound value */
  Datum upper;          /**< upper bound value */
} Span;

/**
 * Structure to represent span sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 spansettype;    /**< Span set type */
  uint8 spantype;       /**< Span type */
  uint8 basetype;       /**< Span basetype */
  char padding;         /**< Not used */
  int32 count;          /**< Number of elements */
  int32 maxcount;       /**< Maximum number of elements */
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
  double ymin;          /**< minimum y value */
  double zmin;          /**< minimum z value */
  double xmax;          /**< maximum x value */
  double ymax;          /**< maximum y value */
  double zmax;          /**< maximum z value */
  int32_t srid;         /**< SRID */
  int16 flags;          /**< flags */
} STBox;

/**
 * @brief Enumeration that defines the temporal subtypes used in MEOS
 */
typedef enum
{
  ANYTEMPSUBTYPE =   0,  /**< Any temporal subtype */
  TINSTANT =         1,  /**< Temporal instant subtype */
  TSEQUENCE =        2,  /**< Temporal sequence subtype */
  TSEQUENCESET =     3,  /**< Temporal sequence set subtype */
} tempSubtype;

/**
 * @brief Enumeration that defines the interpolation types used in MEOS
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
 * Structure to represent temporal values of sequence subtype
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
  char padding[6];      /**< Not used */
  Span period;          /**< Time span (24 bytes). All bounding boxes start
                             with a period so actually it is also the begining
                             of the bounding box. The extra bytes needed for
                             the bounding box are added upon creation. */
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
  int16 padding;        /**< Not used */
  Span period;          /**< Time span (24 bytes). All bounding boxes start
                             with a period so actually it is also the begining
                             of the bounding box. The extra bytes needed for
                             the bounding box are added upon creation. */
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
 * Structure for skiplists
 */
typedef struct SkipList SkipList;

/*****************************************************************************/

/**
 * Structure for the in-memory Rtree index
 */
typedef struct RTree RTree;

/* RTree functions */

extern RTree *rtree_create_intspan();
extern RTree *rtree_create_bigintspan();
extern RTree *rtree_create_floatspan();
extern RTree *rtree_create_datespan();
extern RTree *rtree_create_tstzspan();
extern RTree *rtree_create_tbox();
extern RTree *rtree_create_stbox();
extern void rtree_free(RTree *rtree);
extern void rtree_insert(RTree *rtree, void *box, int64 id);
extern int *rtree_search(const RTree *rtree,const void *query, int *count);

/*****************************************************************************
 * Error codes
 *****************************************************************************/

typedef enum
{
  MEOS_SUCCESS                   = 0,  // Successful operation

  MEOS_ERR_INTERNAL_ERROR        = 1,  // Unspecified internal error
  MEOS_ERR_INTERNAL_TYPE_ERROR   = 2,  // Internal type error
  MEOS_ERR_VALUE_OUT_OF_RANGE    = 3,  // Internal out of range error
  MEOS_ERR_DIVISION_BY_ZERO      = 4,  // Internal division by zero error
  MEOS_ERR_MEMORY_ALLOC_ERROR    = 5,  // Internal malloc error
  MEOS_ERR_AGGREGATION_ERROR     = 6,  // Internal aggregation error
  MEOS_ERR_DIRECTORY_ERROR       = 7,  // Internal directory error
  MEOS_ERR_FILE_ERROR            = 8,  // Internal file error

  MEOS_ERR_INVALID_ARG           = 10, // Invalid argument
  MEOS_ERR_INVALID_ARG_TYPE      = 11, // Invalid argument type
  MEOS_ERR_INVALID_ARG_VALUE     = 12, // Invalid argument value
  MEOS_ERR_FEATURE_NOT_SUPPORTED = 13, // Feature not currently supported

  MEOS_ERR_MFJSON_INPUT          = 20, // MFJSON input error
  MEOS_ERR_MFJSON_OUTPUT         = 21, // MFJSON output error
  MEOS_ERR_TEXT_INPUT            = 22, // Text input error
  MEOS_ERR_TEXT_OUTPUT           = 23, // Text output error
  MEOS_ERR_WKB_INPUT             = 24, // WKB input error
  MEOS_ERR_WKB_OUTPUT            = 25, // WKB output error
  MEOS_ERR_GEOJSON_INPUT         = 26, // GEOJSON input error
  MEOS_ERR_GEOJSON_OUTPUT        = 27, // GEOJSON output error

} errorCode;

extern void meos_error(int errlevel, int errcode, const char *format, ...);

/* Set or read error level */

extern int meos_errno(void);
extern int meos_errno_set(int err);
extern int meos_errno_restore(int err);
extern int meos_errno_reset(void);

/*****************************************************************************
 * Initialization of the MEOS library
 *****************************************************************************/

/* Definition of error handler function */
typedef void (*error_handler_fn)(int, int, const char *);

extern void meos_initialize_timezone(const char *name);
extern void meos_initialize_error_handler(error_handler_fn err_handler);
extern void meos_finalize_timezone(void);
extern void meos_finalize_projsrs(void);
extern void meos_finalize_ways(void);

extern bool meos_set_datestyle(const char *newval, void *extra);
extern bool meos_set_intervalstyle(const char *newval, int extra);
extern char *meos_get_datestyle(void);
extern char *meos_get_intervalstyle(void);

extern void meos_set_spatial_ref_sys_csv(const char* path);

extern void meos_initialize(void);
extern void meos_finalize(void);

/******************************************************************************
 * Functions for base and time types
 ******************************************************************************/

extern DateADT add_date_int(DateADT d, int32 days);
extern Interval *add_interval_interval(const Interval *interv1, const Interval *interv2);
extern TimestampTz add_timestamptz_interval(TimestampTz t, const Interval *interv);
extern bool bool_in(const char *str);
extern char *bool_out(bool b);
extern text *cstring2text(const char *str);
extern Timestamp date_to_timestamp(DateADT dateVal);
extern TimestampTz date_to_timestamptz(DateADT d);
extern double float_exp(double d);
extern double float_ln(double d);
extern double float_log10(double d);
extern char *float8_out(double d, int maxdd);
extern double float_round(double d, int maxdd);
extern int int32_cmp(int32 l, int32 r);
extern int int64_cmp(int64 l, int64 r);
extern Interval *interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, double secs);
extern int minus_date_date(DateADT d1, DateADT d2);
extern DateADT minus_date_int(DateADT d, int32 days);
extern TimestampTz minus_timestamptz_interval(TimestampTz t, const Interval *interv);
extern Interval *minus_timestamptz_timestamptz(TimestampTz t1, TimestampTz t2);
extern Interval *mul_interval_double(const Interval *interv, double factor);
extern DateADT pg_date_in(const char *str);
extern char *pg_date_out(DateADT d);
extern int pg_interval_cmp(const Interval *interv1, const Interval *interv2);
extern Interval *pg_interval_in(const char *str, int32 typmod);
extern char *pg_interval_out(const Interval *interv);
extern Timestamp pg_timestamp_in(const char *str, int32 typmod);
extern char *pg_timestamp_out(Timestamp t);
extern TimestampTz pg_timestamptz_in(const char *str, int32 typmod);
extern char *pg_timestamptz_out(TimestampTz t);
extern char *text2cstring(const text *txt);
extern int text_cmp(const text *txt1, const text *txt2);
extern text *text_copy(const text *txt);
extern text *text_in(const char *str);
extern text *text_initcap(const text *txt);
extern text *text_lower(const text *txt);
extern char *text_out(const text *txt);
extern text *text_upper(const text *txt);
extern text *textcat_text_text(const text *txt1, const text *txt2);
extern TimestampTz timestamptz_shift(TimestampTz t, const Interval *interv);
extern DateADT timestamp_to_date(Timestamp t);
extern DateADT timestamptz_to_date(TimestampTz t);

/*============================================================================
 * Functions for set and span types
  ===========================================================================*/

/*****************************************************************************
 * Input/output functions for set and span types
 *****************************************************************************/

extern Set *bigintset_in(const char *str);
extern char *bigintset_out(const Set *set);
extern Span *bigintspan_expand(const Span *s, int64 value);
extern Span *bigintspan_in(const char *str);
extern char *bigintspan_out(const Span *s);
extern SpanSet *bigintspanset_in(const char *str);
extern char *bigintspanset_out(const SpanSet *ss);
extern Set *dateset_in(const char *str);
extern char *dateset_out(const Set *s);
extern Span *datespan_in(const char *str);
extern char *datespan_out(const Span *s);
extern SpanSet *datespanset_in(const char *str);
extern char *datespanset_out(const SpanSet *ss);
extern Set *floatset_in(const char *str);
extern char *floatset_out(const Set *set, int maxdd);
extern Span *floatspan_expand(const Span *s, double value);
extern Span *floatspan_in(const char *str);
extern char *floatspan_out(const Span *s, int maxdd);
extern SpanSet *floatspanset_in(const char *str);
extern char *floatspanset_out(const SpanSet *ss, int maxdd);
extern Set *intset_in(const char *str);
extern char *intset_out(const Set *set);
extern Span *intspan_expand(const Span *s, int32 value);
extern Span *intspan_in(const char *str);
extern char *intspan_out(const Span *s);
extern SpanSet *intspanset_in(const char *str);
extern char *intspanset_out(const SpanSet *ss);
extern char *set_as_hexwkb(const Set *s, uint8_t variant, size_t *size_out);
extern uint8_t *set_as_wkb(const Set *s, uint8_t variant, size_t *size_out);
extern Set *set_from_hexwkb(const char *hexwkb);
extern Set *set_from_wkb(const uint8_t *wkb, size_t size);
extern char *span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out);
extern uint8_t *span_as_wkb(const Span *s, uint8_t variant, size_t *size_out);
extern Span *span_from_hexwkb(const char *hexwkb);
extern Span *span_from_wkb(const uint8_t *wkb, size_t size);
extern char *spanset_as_hexwkb(const SpanSet *ss, uint8_t variant, size_t *size_out);
extern uint8_t *spanset_as_wkb(const SpanSet *ss, uint8_t variant, size_t *size_out);
extern SpanSet *spanset_from_hexwkb(const char *hexwkb);
extern SpanSet *spanset_from_wkb(const uint8_t *wkb, size_t size);
extern Set *textset_in(const char *str);
extern char *textset_out(const Set *set);
extern Set *tstzset_in(const char *str);
extern char *tstzset_out(const Set *set);
extern Span *tstzspan_in(const char *str);
extern char *tstzspan_out(const Span *s);
extern SpanSet *tstzspanset_in(const char *str);
extern char *tstzspanset_out(const SpanSet *ss);

/*****************************************************************************
 * Constructor functions for set and span types
 *****************************************************************************/

extern Set *bigintset_make(const int64 *values, int count);
extern Span *bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc);
extern Set *dateset_make(const DateADT *values, int count);
extern Span *datespan_make(DateADT lower, DateADT upper, bool lower_inc, bool upper_inc);
extern Set *floatset_make(const double *values, int count);
extern Span *floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc);
extern Set *intset_make(const int *values, int count);
extern Span *intspan_make(int lower, int upper, bool lower_inc, bool upper_inc);
extern Set *set_copy(const Set *s);
extern Span *span_copy(const Span *s);
extern SpanSet *spanset_copy(const SpanSet *ss);
extern SpanSet *spanset_make(Span *spans, int count);
extern Set *textset_make(text **values, int count);
extern Set *tstzset_make(const TimestampTz *values, int count);
extern Span *tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc);

/*****************************************************************************
 * Conversion functions for set and span types
 *****************************************************************************/

extern Set *bigint_to_set(int64 i);
extern Span *bigint_to_span(int i);
extern SpanSet *bigint_to_spanset(int i);
extern Set *date_to_set(DateADT d);
extern Span *date_to_span(DateADT d);
extern SpanSet *date_to_spanset(DateADT d);
extern Set *dateset_to_tstzset(const Set *s);
extern Span *datespan_to_tstzspan(const Span *s);
extern SpanSet *datespanset_to_tstzspanset(const SpanSet *ss);
extern Set *float_to_set(double d);
extern Span *float_to_span(double d);
extern SpanSet *float_to_spanset(double d);
extern Set *floatset_to_intset(const Set *s);
extern Span *floatspan_to_intspan(const Span *s);
extern SpanSet *floatspanset_to_intspanset(const SpanSet *ss);
extern Set *int_to_set(int i);
extern Span *int_to_span(int i);
extern SpanSet *int_to_spanset(int i);
extern Set *intset_to_floatset(const Set *s);
extern Span *intspan_to_floatspan(const Span *s);
extern SpanSet *intspanset_to_floatspanset(const SpanSet *ss);
extern Span *set_to_span(const Set *s);
extern SpanSet *set_to_spanset(const Set *s);
extern SpanSet *span_to_spanset(const Span *s);
extern Set *text_to_set(const text *txt);
extern Set *timestamptz_to_set(TimestampTz t);
extern Span *timestamptz_to_span(TimestampTz t);
extern SpanSet *timestamptz_to_spanset(TimestampTz t);
extern Set *tstzset_to_dateset(const Set *s);
extern Span *tstzspan_to_datespan(const Span *s);
extern SpanSet *tstzspanset_to_datespanset(const SpanSet *ss);

/*****************************************************************************
 * Accessor functions for set and span types
 *****************************************************************************/

extern int64 bigintset_end_value(const Set *s);
extern int64 bigintset_start_value(const Set *s);
extern bool bigintset_value_n(const Set *s, int n, int64 *result);
extern int64 *bigintset_values(const Set *s);
extern int64 bigintspan_lower(const Span *s);
extern int64 bigintspan_upper(const Span *s);
extern int64 bigintspan_width(const Span *s);
extern int64 bigintspanset_lower(const SpanSet *ss);
extern int64 bigintspanset_upper(const SpanSet *ss);
extern int64 bigintspanset_width(const SpanSet *ss, bool boundspan);
extern DateADT dateset_end_value(const Set *s);
extern DateADT dateset_start_value(const Set *s);
extern bool dateset_value_n(const Set *s, int n, DateADT *result);
extern DateADT *dateset_values(const Set *s);
extern Interval *datespan_duration(const Span *s);
extern DateADT datespan_lower(const Span *s);
extern DateADT datespan_upper(const Span *s);
extern bool datespanset_date_n(const SpanSet *ss, int n, DateADT *result);
extern Set *datespanset_dates(const SpanSet *ss);
extern Interval *datespanset_duration(const SpanSet *ss, bool boundspan);
extern DateADT datespanset_end_date(const SpanSet *ss);
extern int datespanset_num_dates(const SpanSet *ss);
extern DateADT datespanset_start_date(const SpanSet *ss);
extern double floatset_end_value(const Set *s);
extern double floatset_start_value(const Set *s);
extern bool floatset_value_n(const Set *s, int n, double *result);
extern double *floatset_values(const Set *s);
extern double floatspan_lower(const Span *s);
extern double floatspan_upper(const Span *s);
extern double floatspan_width(const Span *s);
extern double floatspanset_lower(const SpanSet *ss);
extern double floatspanset_upper(const SpanSet *ss);
extern double floatspanset_width(const SpanSet *ss, bool boundspan);
extern int intset_end_value(const Set *s);
extern int intset_start_value(const Set *s);
extern bool intset_value_n(const Set *s, int n, int *result);
extern int *intset_values(const Set *s);
extern int intspan_lower(const Span *s);
extern int intspan_upper(const Span *s);
extern int intspan_width(const Span *s);
extern int intspanset_lower(const SpanSet *ss);
extern int intspanset_upper(const SpanSet *ss);
extern int intspanset_width(const SpanSet *ss, bool boundspan);
extern uint32 set_hash(const Set *s);
extern uint64 set_hash_extended(const Set *s, uint64 seed);
extern int set_num_values(const Set *s);
extern uint32 span_hash(const Span *s);
extern uint64 span_hash_extended(const Span *s, uint64 seed);
extern bool span_lower_inc(const Span *s);
extern bool span_upper_inc(const Span *s);
extern Span *spanset_end_span(const SpanSet *ss);
extern uint32 spanset_hash(const SpanSet *ss);
extern uint64 spanset_hash_extended(const SpanSet *ss, uint64 seed);
extern bool spanset_lower_inc(const SpanSet *ss);
extern int spanset_num_spans(const SpanSet *ss);
extern Span *spanset_span(const SpanSet *ss);
extern Span *spanset_span_n(const SpanSet *ss, int i);
extern Span **spanset_spanarr(const SpanSet *ss);
extern Span *spanset_start_span(const SpanSet *ss);
extern bool spanset_upper_inc(const SpanSet *ss);
extern text *textset_end_value(const Set *s);
extern text *textset_start_value(const Set *s);
extern bool textset_value_n(const Set *s, int n, text **result);
extern text **textset_values(const Set *s);
extern TimestampTz tstzset_end_value(const Set *s);
extern TimestampTz tstzset_start_value(const Set *s);
extern bool tstzset_value_n(const Set *s, int n, TimestampTz *result);
extern TimestampTz *tstzset_values(const Set *s);
extern Interval *tstzspan_duration(const Span *s);
extern TimestampTz tstzspan_lower(const Span *s);
extern TimestampTz tstzspan_upper(const Span *s);
extern Interval *tstzspanset_duration(const SpanSet *ss, bool boundspan);
extern TimestampTz tstzspanset_end_timestamptz(const SpanSet *ss);
extern TimestampTz tstzspanset_lower(const SpanSet *ss);
extern int tstzspanset_num_timestamps(const SpanSet *ss);
extern TimestampTz tstzspanset_start_timestamptz(const SpanSet *ss);
extern Set *tstzspanset_timestamps(const SpanSet *ss);
extern bool tstzspanset_timestamptz_n(const SpanSet *ss, int n, TimestampTz *result);
extern TimestampTz tstzspanset_upper(const SpanSet *ss);

/*****************************************************************************
 * Transformation functions for set and span types
 *****************************************************************************/

extern Set *bigintset_shift_scale(const Set *s, int64 shift, int64 width, bool hasshift, bool haswidth);
extern Span *bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift, bool haswidth);
extern SpanSet *bigintspanset_shift_scale(const SpanSet *ss, int64 shift, int64 width, bool hasshift, bool haswidth);
extern Set *dateset_shift_scale(const Set *s, int shift, int width, bool hasshift, bool haswidth);
extern Span *datespan_shift_scale(const Span *s, int shift, int width, bool hasshift, bool haswidth);
extern SpanSet *datespanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift, bool haswidth);
extern Set *floatset_ceil(const Set *s);
extern Set *floatset_degrees(const Set *s, bool normalize);
extern Set *floatset_floor(const Set *s);
extern Set *floatset_radians(const Set *s);
extern Set *floatset_shift_scale(const Set *s, double shift, double width, bool hasshift, bool haswidth);
extern Span *floatspan_ceil(const Span *s);
extern Span *floatspan_degrees(const Span *s, bool normalize);
extern Span *floatspan_floor(const Span *s);
extern Span *floatspan_radians(const Span *s);
extern Span *floatspan_round(const Span *s, int maxdd);
extern Span *floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift, bool haswidth);
extern SpanSet *floatspanset_ceil(const SpanSet *ss);
extern SpanSet *floatspanset_floor(const SpanSet *ss);
extern SpanSet *floatspanset_degrees(const SpanSet *ss, bool normalize);
extern SpanSet *floatspanset_radians(const SpanSet *ss);
extern SpanSet *floatspanset_round(const SpanSet *ss, int maxdd);
extern SpanSet *floatspanset_shift_scale(const SpanSet *ss, double shift, double width, bool hasshift, bool haswidth);
extern Set *intset_shift_scale(const Set *s, int shift, int width, bool hasshift, bool haswidth);
extern Span *intspan_shift_scale(const Span *s, int shift, int width, bool hasshift, bool haswidth);
extern SpanSet *intspanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift, bool haswidth);
extern Span *tstzspan_expand(const Span *s, const Interval *interv);
extern Set *set_round(const Set *s, int maxdd);
extern Set *textcat_text_textset(const text *txt, const Set *s);
extern Set *textcat_textset_text(const Set *s, const text *txt);
extern Set *textset_initcap(const Set *s);
extern Set *textset_lower(const Set *s);
extern Set *textset_upper(const Set *s);
extern TimestampTz timestamptz_tprecision(TimestampTz t, const Interval *duration, TimestampTz torigin);
extern Set *tstzset_shift_scale(const Set *s, const Interval *shift, const Interval *duration);
extern Set *tstzset_tprecision(const Set *s, const Interval *duration, TimestampTz torigin);
extern Span *tstzspan_shift_scale(const Span *s, const Interval *shift, const Interval *duration);
extern Span *tstzspan_tprecision(const Span *s, const Interval *duration, TimestampTz torigin);
extern SpanSet *tstzspanset_shift_scale(const SpanSet *ss, const Interval *shift, const Interval *duration);
extern SpanSet *tstzspanset_tprecision(const SpanSet *ss, const Interval *duration, TimestampTz torigin);

/*****************************************************************************
 * Comparison functions for set and span types
 *****************************************************************************/

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

/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Split functions */

extern Span *set_spans(const Set *s);
extern Span *set_split_each_n_spans(const Set *s, int elems_per_span, int *count);
extern Span *set_split_n_spans(const Set *s, int span_count, int *count);
extern Span *spanset_spans(const SpanSet *ss);
extern Span *spanset_split_each_n_spans(const SpanSet *ss, int elems_per_span, int *count);
extern Span *spanset_split_n_spans(const SpanSet *ss, int span_count, int *count);

/* Topological functions */

extern bool adjacent_span_bigint(const Span *s, int64 i);
extern bool adjacent_span_date(const Span *s, DateADT d);
extern bool adjacent_span_float(const Span *s, double d);
extern bool adjacent_span_int(const Span *s, int i);
extern bool adjacent_span_span(const Span *s1, const Span *s2);
extern bool adjacent_span_spanset(const Span *s, const SpanSet *ss);
extern bool adjacent_span_timestamptz(const Span *s, TimestampTz t);
extern bool adjacent_spanset_bigint(const SpanSet *ss, int64 i);
extern bool adjacent_spanset_date(const SpanSet *ss, DateADT d);
extern bool adjacent_spanset_float(const SpanSet *ss, double d);
extern bool adjacent_spanset_int(const SpanSet *ss, int i);
extern bool adjacent_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool adjacent_spanset_span(const SpanSet *ss, const Span *s);
extern bool adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_bigint_set(int64 i, const Set *s);
extern bool contained_bigint_span(int64 i, const Span *s);
extern bool contained_bigint_spanset(int64 i, const SpanSet *ss);
extern bool contained_date_set(DateADT d, const Set *s);
extern bool contained_date_span(DateADT d, const Span *s);
extern bool contained_date_spanset(DateADT d, const SpanSet *ss);
extern bool contained_float_set(double d, const Set *s);
extern bool contained_float_span(double d, const Span *s);
extern bool contained_float_spanset(double d, const SpanSet *ss);
extern bool contained_int_set(int i, const Set *s);
extern bool contained_int_span(int i, const Span *s);
extern bool contained_int_spanset(int i, const SpanSet *ss);
extern bool contained_set_set(const Set *s1, const Set *s2);
extern bool contained_span_span(const Span *s1, const Span *s2);
extern bool contained_span_spanset(const Span *s, const SpanSet *ss);
extern bool contained_spanset_span(const SpanSet *ss, const Span *s);
extern bool contained_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_text_set(const text *txt, const Set *s);
extern bool contained_timestamptz_set(TimestampTz t, const Set *s);
extern bool contained_timestamptz_span(TimestampTz t, const Span *s);
extern bool contained_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool contains_set_bigint(const Set *s, int64 i);
extern bool contains_set_date(const Set *s, DateADT d);
extern bool contains_set_float(const Set *s, double d);
extern bool contains_set_int(const Set *s, int i);
extern bool contains_set_set(const Set *s1, const Set *s2);
extern bool contains_set_text(const Set *s, text *t);
extern bool contains_set_timestamptz(const Set *s, TimestampTz t);
extern bool contains_span_bigint(const Span *s, int64 i);
extern bool contains_span_date(const Span *s, DateADT d);
extern bool contains_span_float(const Span *s, double d);
extern bool contains_span_int(const Span *s, int i);
extern bool contains_span_span(const Span *s1, const Span *s2);
extern bool contains_span_spanset(const Span *s, const SpanSet *ss);
extern bool contains_span_timestamptz(const Span *s, TimestampTz t);
extern bool contains_spanset_bigint(const SpanSet *ss, int64 i);
extern bool contains_spanset_date(const SpanSet *ss, DateADT d);
extern bool contains_spanset_float(const SpanSet *ss, double d);
extern bool contains_spanset_int(const SpanSet *ss, int i);
extern bool contains_spanset_span(const SpanSet *ss, const Span *s);
extern bool contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contains_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool overlaps_set_set(const Set *s1, const Set *s2);
extern bool overlaps_span_span(const Span *s1, const Span *s2);
extern bool overlaps_span_spanset(const Span *s, const SpanSet *ss);
extern bool overlaps_spanset_span(const SpanSet *ss, const Span *s);
extern bool overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool after_date_set(DateADT d, const Set *s);
extern bool after_date_span(DateADT d, const Span *s);
extern bool after_date_spanset(DateADT d, const SpanSet *ss);
extern bool after_set_date(const Set *s, DateADT d);
extern bool after_set_timestamptz(const Set *s, TimestampTz t);
extern bool after_span_date(const Span *s, DateADT d);
extern bool after_span_timestamptz(const Span *s, TimestampTz t);
extern bool after_spanset_date(const SpanSet *ss, DateADT d);
extern bool after_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool after_timestamptz_set(TimestampTz t, const Set *s);
extern bool after_timestamptz_span(TimestampTz t, const Span *s);
extern bool after_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool before_date_set(DateADT d, const Set *s);
extern bool before_date_span(DateADT d, const Span *s);
extern bool before_date_spanset(DateADT d, const SpanSet *ss);
extern bool before_set_date(const Set *s, DateADT d);
extern bool before_set_timestamptz(const Set *s, TimestampTz t);
extern bool before_span_date(const Span *s, DateADT d);
extern bool before_span_timestamptz(const Span *s, TimestampTz t);
extern bool before_spanset_date(const SpanSet *ss, DateADT d);
extern bool before_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool before_timestamptz_set(TimestampTz t, const Set *s);
extern bool before_timestamptz_span(TimestampTz t, const Span *s);
extern bool before_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool left_bigint_set(int64 i, const Set *s);
extern bool left_bigint_span(int64 i, const Span *s);
extern bool left_bigint_spanset(int64 i, const SpanSet *ss);
extern bool left_float_set(double d, const Set *s);
extern bool left_float_span(double d, const Span *s);
extern bool left_float_spanset(double d, const SpanSet *ss);
extern bool left_int_set(int i, const Set *s);
extern bool left_int_span(int i, const Span *s);
extern bool left_int_spanset(int i, const SpanSet *ss);
extern bool left_set_bigint(const Set *s, int64 i);
extern bool left_set_float(const Set *s, double d);
extern bool left_set_int(const Set *s, int i);
extern bool left_set_set(const Set *s1, const Set *s2);
extern bool left_set_text(const Set *s, text *txt);
extern bool left_span_bigint(const Span *s, int64 i);
extern bool left_span_float(const Span *s, double d);
extern bool left_span_int(const Span *s, int i);
extern bool left_span_span(const Span *s1, const Span *s2);
extern bool left_span_spanset(const Span *s, const SpanSet *ss);
extern bool left_spanset_bigint(const SpanSet *ss, int64 i);
extern bool left_spanset_float(const SpanSet *ss, double d);
extern bool left_spanset_int(const SpanSet *ss, int i);
extern bool left_spanset_span(const SpanSet *ss, const Span *s);
extern bool left_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool left_text_set(const text *txt, const Set *s);
extern bool overafter_date_set(DateADT d, const Set *s);
extern bool overafter_date_span(DateADT d, const Span *s);
extern bool overafter_date_spanset(DateADT d, const SpanSet *ss);
extern bool overafter_set_date(const Set *s, DateADT d);
extern bool overafter_set_timestamptz(const Set *s, TimestampTz t);
extern bool overafter_span_date(const Span *s, DateADT d);
extern bool overafter_span_timestamptz(const Span *s, TimestampTz t);
extern bool overafter_spanset_date(const SpanSet *ss, DateADT d);
extern bool overafter_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool overafter_timestamptz_set(TimestampTz t, const Set *s);
extern bool overafter_timestamptz_span(TimestampTz t, const Span *s);
extern bool overafter_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool overbefore_date_set(DateADT d, const Set *s);
extern bool overbefore_date_span(DateADT d, const Span *s);
extern bool overbefore_date_spanset(DateADT d, const SpanSet *ss);
extern bool overbefore_set_date(const Set *s, DateADT d);
extern bool overbefore_set_timestamptz(const Set *s, TimestampTz t);
extern bool overbefore_span_date(const Span *s, DateADT d);
extern bool overbefore_span_timestamptz(const Span *s, TimestampTz t);
extern bool overbefore_spanset_date(const SpanSet *ss, DateADT d);
extern bool overbefore_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool overbefore_timestamptz_set(TimestampTz t, const Set *s);
extern bool overbefore_timestamptz_span(TimestampTz t, const Span *s);
extern bool overbefore_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool overleft_bigint_set(int64 i, const Set *s);
extern bool overleft_bigint_span(int64 i, const Span *s);
extern bool overleft_bigint_spanset(int64 i, const SpanSet *ss);
extern bool overleft_float_set(double d, const Set *s);
extern bool overleft_float_span(double d, const Span *s);
extern bool overleft_float_spanset(double d, const SpanSet *ss);
extern bool overleft_int_set(int i, const Set *s);
extern bool overleft_int_span(int i, const Span *s);
extern bool overleft_int_spanset(int i, const SpanSet *ss);
extern bool overleft_set_bigint(const Set *s, int64 i);
extern bool overleft_set_float(const Set *s, double d);
extern bool overleft_set_int(const Set *s, int i);
extern bool overleft_set_set(const Set *s1, const Set *s2);
extern bool overleft_set_text(const Set *s, text *txt);
extern bool overleft_span_bigint(const Span *s, int64 i);
extern bool overleft_span_float(const Span *s, double d);
extern bool overleft_span_int(const Span *s, int i);
extern bool overleft_span_span(const Span *s1, const Span *s2);
extern bool overleft_span_spanset(const Span *s, const SpanSet *ss);
extern bool overleft_spanset_bigint(const SpanSet *ss, int64 i);
extern bool overleft_spanset_float(const SpanSet *ss, double d);
extern bool overleft_spanset_int(const SpanSet *ss, int i);
extern bool overleft_spanset_span(const SpanSet *ss, const Span *s);
extern bool overleft_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool overleft_text_set(const text *txt, const Set *s);
extern bool overright_bigint_set(int64 i, const Set *s);
extern bool overright_bigint_span(int64 i, const Span *s);
extern bool overright_bigint_spanset(int64 i, const SpanSet *ss);
extern bool overright_float_set(double d, const Set *s);
extern bool overright_float_span(double d, const Span *s);
extern bool overright_float_spanset(double d, const SpanSet *ss);
extern bool overright_int_set(int i, const Set *s);
extern bool overright_int_span(int i, const Span *s);
extern bool overright_int_spanset(int i, const SpanSet *ss);
extern bool overright_set_bigint(const Set *s, int64 i);
extern bool overright_set_float(const Set *s, double d);
extern bool overright_set_int(const Set *s, int i);
extern bool overright_set_set(const Set *s1, const Set *s2);
extern bool overright_set_text(const Set *s, text *txt);
extern bool overright_span_bigint(const Span *s, int64 i);
extern bool overright_span_float(const Span *s, double d);
extern bool overright_span_int(const Span *s, int i);
extern bool overright_span_span(const Span *s1, const Span *s2);
extern bool overright_span_spanset(const Span *s, const SpanSet *ss);
extern bool overright_spanset_bigint(const SpanSet *ss, int64 i);
extern bool overright_spanset_float(const SpanSet *ss, double d);
extern bool overright_spanset_int(const SpanSet *ss, int i);
extern bool overright_spanset_span(const SpanSet *ss, const Span *s);
extern bool overright_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool overright_text_set(const text *txt, const Set *s);
extern bool right_bigint_set(int64 i, const Set *s);
extern bool right_bigint_span(int64 i, const Span *s);
extern bool right_bigint_spanset(int64 i, const SpanSet *ss);
extern bool right_float_set(double d, const Set *s);
extern bool right_float_span(double d, const Span *s);
extern bool right_float_spanset(double d, const SpanSet *ss);
extern bool right_int_set(int i, const Set *s);
extern bool right_int_span(int i, const Span *s);
extern bool right_int_spanset(int i, const SpanSet *ss);
extern bool right_set_bigint(const Set *s, int64 i);
extern bool right_set_float(const Set *s, double d);
extern bool right_set_int(const Set *s, int i);
extern bool right_set_set(const Set *s1, const Set *s2);
extern bool right_set_text(const Set *s, text *txt);
extern bool right_span_bigint(const Span *s, int64 i);
extern bool right_span_float(const Span *s, double d);
extern bool right_span_int(const Span *s, int i);
extern bool right_span_span(const Span *s1, const Span *s2);
extern bool right_span_spanset(const Span *s, const SpanSet *ss);
extern bool right_spanset_bigint(const SpanSet *ss, int64 i);
extern bool right_spanset_float(const SpanSet *ss, double d);
extern bool right_spanset_int(const SpanSet *ss, int i);
extern bool right_spanset_span(const SpanSet *ss, const Span *s);
extern bool right_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool right_text_set(const text *txt, const Set *s);

/*****************************************************************************
 * Set functions for set and span types
 *****************************************************************************/

extern Set *intersection_bigint_set(int64 i, const Set *s);
extern Set *intersection_date_set(DateADT d, const Set *s);
extern Set *intersection_float_set(double d, const Set *s);
extern Set *intersection_int_set(int i, const Set *s);
extern Set *intersection_set_bigint(const Set *s, int64 i);
extern Set *intersection_set_date(const Set *s, DateADT d);
extern Set *intersection_set_float(const Set *s, double d);
extern Set *intersection_set_int(const Set *s, int i);
extern Set *intersection_set_set(const Set *s1, const Set *s2);
extern Set *intersection_set_text(const Set *s, const text *txt);
extern Set *intersection_set_timestamptz(const Set *s, TimestampTz t);
extern Span *intersection_span_bigint(const Span *s, int64 i);
extern Span *intersection_span_date(const Span *s, DateADT d);
extern Span *intersection_span_float(const Span *s, double d);
extern Span *intersection_span_int(const Span *s, int i);
extern Span *intersection_span_span(const Span *s1, const Span *s2);
extern SpanSet *intersection_span_spanset(const Span *s, const SpanSet *ss);
extern Span *intersection_span_timestamptz(const Span *s, TimestampTz t);
extern SpanSet *intersection_spanset_bigint(const SpanSet *ss, int64 i);
extern SpanSet *intersection_spanset_date(const SpanSet *ss, DateADT d);
extern SpanSet *intersection_spanset_float(const SpanSet *ss, double d);
extern SpanSet *intersection_spanset_int(const SpanSet *ss, int i);
extern SpanSet *intersection_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern SpanSet *intersection_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern Set *intersection_text_set(const text *txt, const Set *s);
extern Set *intersection_timestamptz_set(TimestampTz t, const Set *s);
extern Set *minus_bigint_set(int64 i, const Set *s);
extern SpanSet *minus_bigint_span(int64 i, const Span *s);
extern SpanSet *minus_bigint_spanset(int64 i, const SpanSet *ss);
extern Set *minus_date_set(DateADT d, const Set *s);
extern SpanSet *minus_date_span(DateADT d, const Span *s);
extern SpanSet *minus_date_spanset(DateADT d, const SpanSet *ss);
extern Set *minus_float_set(double d, const Set *s);
extern SpanSet *minus_float_span(double d, const Span *s);
extern SpanSet *minus_float_spanset(double d, const SpanSet *ss);
extern Set *minus_int_set(int i, const Set *s);
extern SpanSet *minus_int_span(int i, const Span *s);
extern SpanSet *minus_int_spanset(int i, const SpanSet *ss);
extern Set *minus_set_bigint(const Set *s, int64 i);
extern Set *minus_set_date(const Set *s, DateADT d);
extern Set *minus_set_float(const Set *s, double d);
extern Set *minus_set_int(const Set *s, int i);
extern Set *minus_set_set(const Set *s1, const Set *s2);
extern Set *minus_set_text(const Set *s, const text *txt);
extern Set *minus_set_timestamptz(const Set *s, TimestampTz t);
extern SpanSet *minus_span_bigint(const Span *s, int64 i);
extern SpanSet *minus_span_date(const Span *s, DateADT d);
extern SpanSet *minus_span_float(const Span *s, double d);
extern SpanSet *minus_span_int(const Span *s, int i);
extern SpanSet *minus_span_span(const Span *s1, const Span *s2);
extern SpanSet *minus_span_spanset(const Span *s, const SpanSet *ss);
extern SpanSet *minus_span_timestamptz(const Span *s, TimestampTz t);
extern SpanSet *minus_spanset_bigint(const SpanSet *ss, int64 i);
extern SpanSet *minus_spanset_date(const SpanSet *ss, DateADT d);
extern SpanSet *minus_spanset_float(const SpanSet *ss, double d);
extern SpanSet *minus_spanset_int(const SpanSet *ss, int i);
extern SpanSet *minus_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern SpanSet *minus_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern Set *minus_text_set(const text *txt, const Set *s);
extern Set *minus_timestamptz_set(TimestampTz t, const Set *s);
extern SpanSet *minus_timestamptz_span(TimestampTz t, const Span *s);
extern SpanSet *minus_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern Set *union_bigint_set(int64 i, const Set *s);
extern SpanSet *union_bigint_span(const Span *s, int64 i);
extern SpanSet *union_bigint_spanset(int64 i, SpanSet *ss);
extern Set *union_date_set(DateADT d, const Set *s);
extern SpanSet *union_date_span(const Span *s, DateADT d);
extern SpanSet *union_date_spanset(DateADT d, SpanSet *ss);
extern Set *union_float_set(double d, const Set *s);
extern SpanSet *union_float_span(const Span *s, double d);
extern SpanSet *union_float_spanset(double d, SpanSet *ss);
extern Set *union_int_set(int i, const Set *s);
extern SpanSet *union_int_span(int i, const Span *s);
extern SpanSet *union_int_spanset(int i, SpanSet *ss);
extern Set *union_set_bigint(const Set *s, int64 i);
extern Set *union_set_date(const Set *s, DateADT d);
extern Set *union_set_float(const Set *s, double d);
extern Set *union_set_int(const Set *s, int i);
extern Set *union_set_set(const Set *s1, const Set *s2);
extern Set *union_set_text(const Set *s, const text *txt);
extern Set *union_set_timestamptz(const Set *s, TimestampTz t);
extern SpanSet *union_span_bigint(const Span *s, int64 i);
extern SpanSet *union_span_date(const Span *s, DateADT d);
extern SpanSet *union_span_float(const Span *s, double d);
extern SpanSet *union_span_int(const Span *s, int i);
extern SpanSet *union_span_span(const Span *s1, const Span *s2);
extern SpanSet *union_span_spanset(const Span *s, const SpanSet *ss);
extern SpanSet *union_span_timestamptz(const Span *s, TimestampTz t);
extern SpanSet *union_spanset_bigint(const SpanSet *ss, int64 i);
extern SpanSet *union_spanset_date(const SpanSet *ss, DateADT d);
extern SpanSet *union_spanset_float(const SpanSet *ss, double d);
extern SpanSet *union_spanset_int(const SpanSet *ss, int i);
extern SpanSet *union_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern SpanSet *union_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern Set *union_text_set(const text *txt, const Set *s);
extern Set *union_timestamptz_set(TimestampTz t, const Set *s);
extern SpanSet *union_timestamptz_span(TimestampTz t, const Span *s);
extern SpanSet *union_timestamptz_spanset(TimestampTz t, SpanSet *ss);

/*****************************************************************************
 * Distance functions for set and span types
 *****************************************************************************/

extern int64 distance_bigintset_bigintset(const Set *s1, const Set *s2);
extern int64 distance_bigintspan_bigintspan(const Span *s1, const Span *s2);
extern int64 distance_bigintspanset_bigintspan(const SpanSet *ss, const Span *s);
extern int64 distance_bigintspanset_bigintspanset(const SpanSet *ss1, const SpanSet *ss2);
extern int distance_dateset_dateset(const Set *s1, const Set *s2);
extern int distance_datespan_datespan(const Span *s1, const Span *s2);
extern int distance_datespanset_datespan(const SpanSet *ss, const Span *s);
extern int distance_datespanset_datespanset(const SpanSet *ss1, const SpanSet *ss2);
extern double distance_floatset_floatset(const Set *s1, const Set *s2);
extern double distance_floatspan_floatspan(const Span *s1, const Span *s2);
extern double distance_floatspanset_floatspan(const SpanSet *ss, const Span *s);
extern double distance_floatspanset_floatspanset(const SpanSet *ss1, const SpanSet *ss2);
extern int distance_intset_intset(const Set *s1, const Set *s2);
extern int distance_intspan_intspan(const Span *s1, const Span *s2);
extern int distance_intspanset_intspan(const SpanSet *ss, const Span *s);
extern int distance_intspanset_intspanset(const SpanSet *ss1, const SpanSet *ss2);
extern int64 distance_set_bigint(const Set *s, int64 i);
extern int distance_set_date(const Set *s, DateADT d);
extern double distance_set_float(const Set *s, double d);
extern int distance_set_int(const Set *s, int i);
extern double distance_set_timestamptz(const Set *s, TimestampTz t);
extern int64 distance_span_bigint(const Span *s, int64 i);
extern int distance_span_date(const Span *s, DateADT d);
extern double distance_span_float(const Span *s, double d);
extern int distance_span_int(const Span *s, int i);
extern double distance_span_timestamptz(const Span *s, TimestampTz t);
extern int64 distance_spanset_bigint(const SpanSet *ss, int64 i);
extern int distance_spanset_date(const SpanSet *ss, DateADT d);
extern double distance_spanset_float(const SpanSet *ss, double d);
extern int distance_spanset_int(const SpanSet *ss, int i);
extern double distance_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern double distance_tstzset_tstzset(const Set *s1, const Set *s2);
extern double distance_tstzspan_tstzspan(const Span *s1, const Span *s2);
extern double distance_tstzspanset_tstzspan(const SpanSet *ss, const Span *s);
extern double distance_tstzspanset_tstzspanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************
 * Aggregate functions for set and span types
 *****************************************************************************/

extern Span *bigint_extent_transfn(Span *state, int64 i);
extern Set *bigint_union_transfn(Set *state, int64 i);
extern Span *date_extent_transfn(Span *state, DateADT d);
extern Set *date_union_transfn(Set *state, DateADT d);
extern Span *float_extent_transfn(Span *state, double d);
extern Set *float_union_transfn(Set *state, double d);
extern Span *int_extent_transfn(Span *state, int i);
extern Set *int_union_transfn(Set *state, int32 i);
extern Span *set_extent_transfn(Span *state, const Set *s);
extern Set *set_union_finalfn(Set *state);
extern Set *set_union_transfn(Set *state, Set *s);
extern Span *span_extent_transfn(Span *state, const Span *s);
extern SpanSet *span_union_transfn(SpanSet *state, const Span *s);
extern Span *spanset_extent_transfn(Span *state, const SpanSet *ss);
extern SpanSet *spanset_union_finalfn(SpanSet *state);
extern SpanSet *spanset_union_transfn(SpanSet *state, const SpanSet *ss);
extern Set *text_union_transfn(Set *state, const text *txt);
extern Span *timestamptz_extent_transfn(Span *state, TimestampTz t);
extern Set *timestamptz_union_transfn(Set *state, TimestampTz t);

/*****************************************************************************
 * Bin functions for span and spanset types
 *****************************************************************************/

extern int64 bigint_get_bin(int64 value, int64 vsize, int64 vorigin);
extern Span *bigintspan_bins(const Span *s, int64 vsize, int64 vorigin, int *count);
extern Span *bigintspanset_bins(const SpanSet *ss, int64 vsize, int64 vorigin, int *count);
extern DateADT date_get_bin(DateADT d, const Interval *duration, DateADT torigin);
extern Span *datespan_bins(const Span *s, const Interval *duration, DateADT torigin, int *count);
extern Span *datespanset_bins(const SpanSet *ss, const Interval *duration, DateADT torigin, int *count);
extern double float_get_bin(double value, double vsize, double vorigin);
extern Span *floatspan_bins(const Span *s, double vsize, double vorigin, int *count);
extern Span *floatspanset_bins(const SpanSet *ss, double vsize, double vorigin, int *count);
extern int int_get_bin(int value, int vsize, int vorigin);
extern Span *intspan_bins(const Span *s, int vsize, int vorigin, int *count);
extern Span *intspanset_bins(const SpanSet *ss, int vsize, int vorigin, int *count);
extern TimestampTz timestamptz_get_bin(TimestampTz t, const Interval *duration, TimestampTz torigin);
extern Span *tstzspan_bins(const Span *s, const Interval *duration, TimestampTz origin, int *count);
extern Span *tstzspanset_bins(const SpanSet *ss, const Interval *duration, TimestampTz torigin, int *count);

/*===========================================================================*
 * Functions for temporal boxes
 *===========================================================================*/

/*****************************************************************************
 * Input and output functions for box types
 *****************************************************************************/

extern char *tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size);
extern uint8_t *tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out);
extern TBox *tbox_from_hexwkb(const char *hexwkb);
extern TBox *tbox_from_wkb(const uint8_t *wkb, size_t size);
extern TBox *tbox_in(const char *str);
extern char *tbox_out(const TBox *box, int maxdd);

/*****************************************************************************
 * Constructor functions for box types
 *****************************************************************************/

extern TBox *float_timestamptz_to_tbox(double d, TimestampTz t);
extern TBox *float_tstzspan_to_tbox(double d, const Span *s);
extern TBox *int_timestamptz_to_tbox(int i, TimestampTz t);
extern TBox *int_tstzspan_to_tbox(int i, const Span *s);
extern TBox *numspan_tstzspan_to_tbox(const Span *span, const Span *s);
extern TBox *numspan_timestamptz_to_tbox(const Span *span, TimestampTz t);
extern TBox *tbox_copy(const TBox *box);
extern TBox *tbox_make(const Span *s, const Span *p);

/*****************************************************************************
 * Conversion functions for box types
 *****************************************************************************/

extern TBox *float_to_tbox(double d);
extern TBox *int_to_tbox(int i);
extern TBox *set_to_tbox(const Set *s);
extern TBox *span_to_tbox(const Span *s);
extern TBox *spanset_to_tbox(const SpanSet *ss);
extern Span *tbox_to_intspan(const TBox *box);
extern Span *tbox_to_floatspan(const TBox *box);
extern Span *tbox_to_tstzspan(const TBox *box);
extern TBox *timestamptz_to_tbox(TimestampTz t);

/*****************************************************************************
 * Accessor functions for box types
 *****************************************************************************/

extern uint32 tbox_hash(const TBox *box);
extern uint64 tbox_hash_extended(const TBox *box, uint64 seed);
extern bool tbox_hast(const TBox *box);
extern bool tbox_hasx(const TBox *box);
extern bool tbox_tmax(const TBox *box, TimestampTz *result);
extern bool tbox_tmax_inc(const TBox *box, bool *result);
extern bool tbox_tmin(const TBox *box, TimestampTz *result);
extern bool tbox_tmin_inc(const TBox *box, bool *result);
extern bool tbox_xmax(const TBox *box, double *result);
extern bool tbox_xmax_inc(const TBox *box, bool *result);
extern bool tbox_xmin(const TBox *box, double *result);
extern bool tbox_xmin_inc(const TBox *box, bool *result);
extern bool tboxfloat_xmax(const TBox *box, double *result);
extern bool tboxfloat_xmin(const TBox *box, double *result);
extern bool tboxint_xmax(const TBox *box, int *result);
extern bool tboxint_xmin(const TBox *box, int *result);

/*****************************************************************************
 * Transformation functions for box types
 *****************************************************************************/

extern TBox *tbox_expand_time(const TBox *box, const Interval *interv);
extern TBox *tbox_round(const TBox *box, int maxdd);
extern TBox *tbox_shift_scale_time(const TBox *box, const Interval *shift, const Interval *duration);
extern TBox *tfloatbox_expand(const TBox *box, double d);
extern TBox *tfloatbox_shift_scale(const TBox *box, double shift, double width, bool hasshift, bool haswidth);
extern TBox *tintbox_expand(const TBox *box, int i);
extern TBox *tintbox_shift_scale(const TBox *box, int shift, int width, bool hasshift, bool haswidth);

/*****************************************************************************
 * Set functions for box types
 *****************************************************************************/

extern TBox *union_tbox_tbox(const TBox *box1, const TBox *box2, bool strict);
extern TBox *intersection_tbox_tbox(const TBox *box1, const TBox *box2);

/*****************************************************************************
 * Bounding box functions for box types
 *****************************************************************************/

/* Topological functions for box types */

extern bool adjacent_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool contained_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool contains_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overlaps_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool same_tbox_tbox(const TBox *box1, const TBox *box2);

/*****************************************************************************/

/* Position functions for box types */

extern bool after_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool before_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool left_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overafter_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overbefore_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overleft_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool overright_tbox_tbox(const TBox *box1, const TBox *box2);
extern bool right_tbox_tbox(const TBox *box1, const TBox *box2);

/*****************************************************************************
 * Comparison functions for box types
 *****************************************************************************/

extern int tbox_cmp(const TBox *box1, const TBox *box2);
extern bool tbox_eq(const TBox *box1, const TBox *box2);
extern bool tbox_ge(const TBox *box1, const TBox *box2);
extern bool tbox_gt(const TBox *box1, const TBox *box2);
extern bool tbox_le(const TBox *box1, const TBox *box2);
extern bool tbox_lt(const TBox *box1, const TBox *box2);
extern bool tbox_ne(const TBox *box1, const TBox *box2);

/*===========================================================================*
 * Functions for temporal types
 *===========================================================================*/

/*****************************************************************************
 * Input and output functions for temporal types
 *****************************************************************************/

extern Temporal *tbool_from_mfjson(const char *str);
extern Temporal *tbool_in(const char *str);
extern char *tbool_out(const Temporal *temp);
extern char *temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern char *temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags, int precision, const char *srs);
extern uint8_t *temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern Temporal *temporal_from_hexwkb(const char *hexwkb);
extern Temporal *temporal_from_wkb(const uint8_t *wkb, size_t size);
extern Temporal *tfloat_from_mfjson(const char *str);
extern Temporal *tfloat_in(const char *str);
extern char *tfloat_out(const Temporal *temp, int maxdd);
extern Temporal *tint_from_mfjson(const char *str);
extern Temporal *tint_in(const char *str);
extern char *tint_out(const Temporal *temp);
extern Temporal *ttext_from_mfjson(const char *str);
extern Temporal *ttext_in(const char *str);
extern char *ttext_out(const Temporal *temp);

/*****************************************************************************
 * Constructor functions for temporal types
 *****************************************************************************/

extern Temporal *tbool_from_base_temp(bool b, const Temporal *temp);
extern TInstant *tboolinst_make(bool b, TimestampTz t);
extern TSequence *tboolseq_from_base_tstzset(bool b, const Set *s);
extern TSequence *tboolseq_from_base_tstzspan(bool b, const Span *s);
extern TSequenceSet *tboolseqset_from_base_tstzspanset(bool b, const SpanSet *ss);
extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *tfloat_from_base_temp(double d, const Temporal *temp);
extern TInstant *tfloatinst_make(double d, TimestampTz t);
extern TSequence *tfloatseq_from_base_tstzset(double d, const Set *s);
extern TSequence *tfloatseq_from_base_tstzspan(double d, const Span *s, interpType interp);
extern TSequenceSet *tfloatseqset_from_base_tstzspanset(double d, const SpanSet *ss, interpType interp);
extern Temporal *tint_from_base_temp(int i, const Temporal *temp);
extern TInstant *tintinst_make(int i, TimestampTz t);
extern TSequence *tintseq_from_base_tstzset(int i, const Set *s);
extern TSequence *tintseq_from_base_tstzspan(int i, const Span *s);
extern TSequenceSet *tintseqset_from_base_tstzspanset(int i, const SpanSet *ss);
extern TSequence *tsequence_make(TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_make(TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_gaps(TInstant **instants, int count, interpType interp, const Interval *maxt, double maxdist);
extern Temporal *ttext_from_base_temp(const text *txt, const Temporal *temp);
extern TInstant *ttextinst_make(const text *txt, TimestampTz t);
extern TSequence *ttextseq_from_base_tstzset(const text *txt, const Set *s);
extern TSequence *ttextseq_from_base_tstzspan(const text *txt, const Span *s);
extern TSequenceSet *ttextseqset_from_base_tstzspanset(const text *txt, const SpanSet *ss);

/*****************************************************************************
 * Conversion functions for temporal types
 *****************************************************************************/

extern Temporal *tbool_to_tint(const Temporal *temp);
extern Span *temporal_to_tstzspan(const Temporal *temp);
extern Temporal *tfloat_to_tint(const Temporal *temp);
extern Temporal *tint_to_tfloat(const Temporal *temp);
extern Span *tnumber_to_span(const Temporal *temp);
extern TBox *tnumber_to_tbox (const Temporal *temp);

/*****************************************************************************
 * Accessor functions for temporal types
 *****************************************************************************/

extern bool tbool_end_value(const Temporal *temp);
extern bool tbool_start_value(const Temporal *temp);
extern bool tbool_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, bool *value);
extern bool tbool_value_n(const Temporal *temp, int n, bool *result);
extern bool *tbool_values(const Temporal *temp, int *count);
extern Interval *temporal_duration(const Temporal *temp, bool boundspan);
extern TInstant *temporal_end_instant(const Temporal *temp);
extern TSequence *temporal_end_sequence(const Temporal *temp);
extern TimestampTz temporal_end_timestamptz(const Temporal *temp);
extern uint32 temporal_hash(const Temporal *temp);
extern TInstant *temporal_instant_n(const Temporal *temp, int n);
extern TInstant **temporal_instants(const Temporal *temp, int *count);
extern const char *temporal_interp(const Temporal *temp);
extern bool temporal_lower_inc(const Temporal *temp);
extern TInstant *temporal_max_instant(const Temporal *temp);
extern TInstant *temporal_min_instant(const Temporal *temp);
extern int temporal_num_instants(const Temporal *temp);
extern int temporal_num_sequences(const Temporal *temp);
extern int temporal_num_timestamps(const Temporal *temp);
extern TSequence **temporal_segments(const Temporal *temp, int *count);
extern TSequence *temporal_sequence_n(const Temporal *temp, int i);
extern TSequence **temporal_sequences(const Temporal *temp, int *count);
extern TInstant *temporal_start_instant(const Temporal *temp);
extern TSequence *temporal_start_sequence(const Temporal *temp);
extern TimestampTz temporal_start_timestamptz(const Temporal *temp);
extern TSequenceSet *temporal_stops(const Temporal *temp, double maxdist, const Interval *minduration);
extern const char *temporal_subtype(const Temporal *temp);
extern SpanSet *temporal_time(const Temporal *temp);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);
extern bool temporal_timestamptz_n(const Temporal *temp, int n, TimestampTz *result);
extern bool temporal_upper_inc(const Temporal *temp);
extern double tfloat_avg_value(const Temporal *temp);
extern double tfloat_end_value(const Temporal *temp);
extern double tfloat_min_value(const Temporal *temp);
extern double tfloat_max_value(const Temporal *temp);
extern double tfloat_start_value(const Temporal *temp);
extern bool tfloat_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, double *value);
extern bool tfloat_value_n(const Temporal *temp, int n, double *result);
extern double *tfloat_values(const Temporal *temp, int *count);
extern int tint_end_value(const Temporal *temp);
extern int tint_max_value(const Temporal *temp);
extern int tint_min_value(const Temporal *temp);
extern int tint_start_value(const Temporal *temp);
extern bool tint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, int *value);
extern bool tint_value_n(const Temporal *temp, int n, int *result);
extern int *tint_values(const Temporal *temp, int *count);
extern double tnumber_avg_value(const Temporal *temp);
extern double tnumber_integral(const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);
extern SpanSet *tnumber_valuespans(const Temporal *temp);
extern text *ttext_end_value(const Temporal *temp);
extern text *ttext_max_value(const Temporal *temp);
extern text *ttext_min_value(const Temporal *temp);
extern text *ttext_start_value(const Temporal *temp);
extern bool ttext_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, text **value);
extern bool ttext_value_n(const Temporal *temp, int n, text **result);
extern text **ttext_values(const Temporal *temp, int *count);

/*****************************************************************************
 * Transformation functions for temporal types
 *****************************************************************************/

extern double float_degrees(double value, bool normalize);
extern Temporal **temparr_round(Temporal **temp, int count, int maxdd);
extern Temporal *temporal_round(const Temporal *temp, int maxdd);
extern Temporal *temporal_scale_time(const Temporal *temp, const Interval *duration);
extern Temporal *temporal_set_interp(const Temporal *temp, interpType interp);
extern Temporal *temporal_shift_scale_time(const Temporal *temp, const Interval *shift, const Interval *duration);
extern Temporal *temporal_shift_time(const Temporal *temp, const Interval *shift);
extern TInstant *temporal_to_tinstant(const Temporal *temp);
extern TSequence *temporal_to_tsequence(const Temporal *temp, interpType interp);
extern TSequenceSet *temporal_to_tsequenceset(const Temporal *temp, interpType interp);
extern Temporal *tfloat_ceil(const Temporal *temp);
extern Temporal *tfloat_degrees(const Temporal *temp, bool normalize);
extern Temporal *tfloat_floor(const Temporal *temp);
extern Temporal *tfloat_radians(const Temporal *temp);
extern Temporal *tfloat_scale_value(const Temporal *temp, double width);
extern Temporal *tfloat_shift_scale_value(const Temporal *temp, double shift, double width);
extern Temporal *tfloat_shift_value(const Temporal *temp, double shift);
extern Temporal *tint_scale_value(const Temporal *temp, int width);
extern Temporal *tint_shift_scale_value(const Temporal *temp, int shift, int width);
extern Temporal *tint_shift_value(const Temporal *temp, int shift);

/*****************************************************************************
 * Modification functions for temporal types
 *****************************************************************************/

extern Temporal *temporal_append_tinstant(Temporal *temp, const TInstant *inst, interpType interp, double maxdist, const Interval *maxt, bool expand);
extern Temporal *temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand);
extern Temporal *temporal_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect);
extern Temporal *temporal_delete_tstzset(const Temporal *temp, const Set *s, bool connect);
extern Temporal *temporal_delete_tstzspan(const Temporal *temp, const Span *s, bool connect);
extern Temporal *temporal_delete_tstzspanset(const Temporal *temp, const SpanSet *ss, bool connect);
extern Temporal *temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);
extern Temporal *temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect);

/*****************************************************************************
 * Restriction functions for temporal types
 *****************************************************************************/

extern Temporal *tbool_at_value(const Temporal *temp, bool b);
extern Temporal *tbool_minus_value(const Temporal *temp, bool b);
extern Temporal *temporal_at_max(const Temporal *temp);
extern Temporal *temporal_at_min(const Temporal *temp);
extern Temporal *temporal_at_timestamptz(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_tstzset(const Temporal *temp, const Set *s);
extern Temporal *temporal_at_tstzspan(const Temporal *temp, const Span *s);
extern Temporal *temporal_at_tstzspanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *temporal_at_values(const Temporal *temp, const Set *set);
extern Temporal *temporal_minus_max(const Temporal *temp);
extern Temporal *temporal_minus_min(const Temporal *temp);
extern Temporal *temporal_minus_timestamptz(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_minus_tstzset(const Temporal *temp, const Set *s);
extern Temporal *temporal_minus_tstzspan(const Temporal *temp, const Span *s);
extern Temporal *temporal_minus_tstzspanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *temporal_minus_values(const Temporal *temp, const Set *set);
extern Temporal *tfloat_at_value(const Temporal *temp, double d);
extern Temporal *tfloat_minus_value(const Temporal *temp, double d);
extern Temporal *tint_at_value(const Temporal *temp, int i);
extern Temporal *tint_minus_value(const Temporal *temp, int i);
extern Temporal *tnumber_at_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_at_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBox *box);
extern Temporal *tnumber_minus_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_minus_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBox *box);
extern Temporal *ttext_at_value(const Temporal *temp, text *txt);
extern Temporal *ttext_minus_value(const Temporal *temp, text *txt);

/*****************************************************************************
 * Comparison functions for temporal types
 *****************************************************************************/

/* Traditional comparison functions for temporal types */

extern int temporal_cmp(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_eq(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ge(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_gt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_le(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_lt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ne(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Ever and always comparison functions for temporal types */

extern int always_eq_bool_tbool(bool b, const Temporal *temp);
extern int always_eq_float_tfloat(double d, const Temporal *temp);
extern int always_eq_int_tint(int i, const Temporal *temp);
extern int always_eq_tbool_bool(const Temporal *temp, bool b);
extern int always_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_eq_text_ttext(const text *txt, const Temporal *temp);
extern int always_eq_tfloat_float(const Temporal *temp, double d);
extern int always_eq_tint_int(const Temporal *temp, int i);
extern int always_eq_ttext_text(const Temporal *temp, const text *txt);
extern int always_ge_float_tfloat(double d, const Temporal *temp);
extern int always_ge_int_tint(int i, const Temporal *temp);
extern int always_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_ge_text_ttext(const text *txt, const Temporal *temp);
extern int always_ge_tfloat_float(const Temporal *temp, double d);
extern int always_ge_tint_int(const Temporal *temp, int i);
extern int always_ge_ttext_text(const Temporal *temp, const text *txt);
extern int always_gt_float_tfloat(double d, const Temporal *temp);
extern int always_gt_int_tint(int i, const Temporal *temp);
extern int always_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_gt_text_ttext(const text *txt, const Temporal *temp);
extern int always_gt_tfloat_float(const Temporal *temp, double d);
extern int always_gt_tint_int(const Temporal *temp, int i);
extern int always_gt_ttext_text(const Temporal *temp, const text *txt);
extern int always_le_float_tfloat(double d, const Temporal *temp);
extern int always_le_int_tint(int i, const Temporal *temp);
extern int always_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_le_text_ttext(const text *txt, const Temporal *temp);
extern int always_le_tfloat_float(const Temporal *temp, double d);
extern int always_le_tint_int(const Temporal *temp, int i);
extern int always_le_ttext_text(const Temporal *temp, const text *txt);
extern int always_lt_float_tfloat(double d, const Temporal *temp);
extern int always_lt_int_tint(int i, const Temporal *temp);
extern int always_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_lt_text_ttext(const text *txt, const Temporal *temp);
extern int always_lt_tfloat_float(const Temporal *temp, double d);
extern int always_lt_tint_int(const Temporal *temp, int i);
extern int always_lt_ttext_text(const Temporal *temp, const text *txt);
extern int always_ne_bool_tbool(bool b, const Temporal *temp);
extern int always_ne_float_tfloat(double d, const Temporal *temp);
extern int always_ne_int_tint(int i, const Temporal *temp);
extern int always_ne_tbool_bool(const Temporal *temp, bool b);
extern int always_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_text_ttext(const text *txt, const Temporal *temp);
extern int always_ne_tfloat_float(const Temporal *temp, double d);
extern int always_ne_tint_int(const Temporal *temp, int i);
extern int always_ne_ttext_text(const Temporal *temp, const text *txt);
extern int ever_eq_bool_tbool(bool b, const Temporal *temp);
extern int ever_eq_float_tfloat(double d, const Temporal *temp);
extern int ever_eq_int_tint(int i, const Temporal *temp);
extern int ever_eq_tbool_bool(const Temporal *temp, bool b);
extern int ever_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_text_ttext(const text *txt, const Temporal *temp);
extern int ever_eq_tfloat_float(const Temporal *temp, double d);
extern int ever_eq_tint_int(const Temporal *temp, int i);
extern int ever_eq_ttext_text(const Temporal *temp, const text *txt);
extern int ever_ge_float_tfloat(double d, const Temporal *temp);
extern int ever_ge_int_tint(int i, const Temporal *temp);
extern int ever_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_ge_text_ttext(const text *txt, const Temporal *temp);
extern int ever_ge_tfloat_float(const Temporal *temp, double d);
extern int ever_ge_tint_int(const Temporal *temp, int i);
extern int ever_ge_ttext_text(const Temporal *temp, const text *txt);
extern int ever_gt_float_tfloat(double d, const Temporal *temp);
extern int ever_gt_int_tint(int i, const Temporal *temp);
extern int ever_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_gt_text_ttext(const text *txt, const Temporal *temp);
extern int ever_gt_tfloat_float(const Temporal *temp, double d);
extern int ever_gt_tint_int(const Temporal *temp, int i);
extern int ever_gt_ttext_text(const Temporal *temp, const text *txt);
extern int ever_le_float_tfloat(double d, const Temporal *temp);
extern int ever_le_int_tint(int i, const Temporal *temp);
extern int ever_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_le_text_ttext(const text *txt, const Temporal *temp);
extern int ever_le_tfloat_float(const Temporal *temp, double d);
extern int ever_le_tint_int(const Temporal *temp, int i);
extern int ever_le_ttext_text(const Temporal *temp, const text *txt);
extern int ever_lt_float_tfloat(double d, const Temporal *temp);
extern int ever_lt_int_tint(int i, const Temporal *temp);
extern int ever_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_lt_text_ttext(const text *txt, const Temporal *temp);
extern int ever_lt_tfloat_float(const Temporal *temp, double d);
extern int ever_lt_tint_int(const Temporal *temp, int i);
extern int ever_lt_ttext_text(const Temporal *temp, const text *txt);
extern int ever_ne_bool_tbool(bool b, const Temporal *temp);
extern int ever_ne_float_tfloat(double d, const Temporal *temp);
extern int ever_ne_int_tint(int i, const Temporal *temp);
extern int ever_ne_tbool_bool(const Temporal *temp, bool b);
extern int ever_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_text_ttext(const text *txt, const Temporal *temp);
extern int ever_ne_tfloat_float(const Temporal *temp, double d);
extern int ever_ne_tint_int(const Temporal *temp, int i);
extern int ever_ne_ttext_text(const Temporal *temp, const text *txt);

/*****************************************************************************/

/* Temporal comparison functions for temporal types */

extern Temporal *teq_bool_tbool(bool b, const Temporal *temp);
extern Temporal *teq_float_tfloat(double d, const Temporal *temp);
extern Temporal *teq_int_tint(int i, const Temporal *temp);
extern Temporal *teq_tbool_bool(const Temporal *temp, bool b);
extern Temporal *teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *teq_tfloat_float(const Temporal *temp, double d);
extern Temporal *teq_tint_int(const Temporal *temp, int i);
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
extern Temporal *tne_int_tint(int i, const Temporal *temp);
extern Temporal *tne_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tne_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tne_tfloat_float(const Temporal *temp, double d);
extern Temporal *tne_tint_int(const Temporal *temp, int i);
extern Temporal *tne_ttext_text(const Temporal *temp, const text *txt);

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/* Split functions */

extern Span *temporal_spans(const Temporal *temp, int *count);
extern Span *temporal_split_each_n_spans(const Temporal *temp, int elem_count, int *count);
extern Span *temporal_split_n_spans(const Temporal *temp, int span_count, int *count);
extern TBox *tnumber_split_each_n_tboxes(const Temporal *temp, int elem_count, int *count);
extern TBox *tnumber_split_n_tboxes(const Temporal *temp, int box_count, int *count);
extern TBox *tnumber_tboxes(const Temporal *temp, int *count);

/* Topological functions for temporal types */

extern bool adjacent_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool adjacent_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool adjacent_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool adjacent_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool adjacent_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool adjacent_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool adjacent_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool contained_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool contained_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contained_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool contained_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool contained_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool contained_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool contained_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool contains_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool contains_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool contains_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool contains_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool contains_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool contains_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool contains_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool overlaps_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool overlaps_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool overlaps_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool overlaps_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool overlaps_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool same_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool same_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool same_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool same_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool same_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool same_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool same_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool same_tstzspan_temporal(const Span *s, const Temporal *temp);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool after_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool after_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool after_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool after_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool after_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool after_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool before_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool before_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool before_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool before_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool before_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool before_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool left_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool left_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool left_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool left_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool left_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool overafter_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool overafter_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool overbefore_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool overbefore_temporal_tstzspan(const Temporal *temp, const Span *s);
extern bool overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool overbefore_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_tstzspan_temporal(const Span *s, const Temporal *temp);
extern bool overleft_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool overleft_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool overleft_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool overleft_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool overleft_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool overright_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool overright_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool overright_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool overright_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool overright_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern bool right_numspan_tnumber(const Span *s, const Temporal *temp);
extern bool right_tbox_tnumber(const TBox *box, const Temporal *temp);
extern bool right_tnumber_numspan(const Temporal *temp, const Span *s);
extern bool right_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool right_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Boolean functions for temporal types
 *****************************************************************************/

extern Temporal *tand_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tand_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2);
extern SpanSet *tbool_when_true(const Temporal *temp);
extern Temporal *tnot_tbool(const Temporal *temp);
extern Temporal *tor_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tor_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Mathematical functions for temporal types
 *****************************************************************************/

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
extern Temporal *temporal_derivative(const Temporal *temp);
extern Temporal *tfloat_exp(const Temporal *temp);
extern Temporal *tfloat_ln(const Temporal *temp);
extern Temporal *tfloat_log10(const Temporal *temp);
extern Temporal *tnumber_abs(const Temporal *temp);
extern double float_angular_difference(double degrees1, double degrees2);
extern Temporal *tnumber_angular_difference(const Temporal *temp);
extern Temporal *tnumber_delta_value(const Temporal *temp);

/*****************************************************************************
 * Text functions for temporal types
 *****************************************************************************/

extern Temporal *textcat_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *textcat_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2);
extern Temporal *ttext_initcap(const Temporal *temp);
extern Temporal *ttext_upper(const Temporal *temp);
extern Temporal *ttext_lower(const Temporal *temp);

/*****************************************************************************
 * Distance functions for temporal types
 *****************************************************************************/

extern Temporal *tdistance_tfloat_float(const Temporal *temp, double d);
extern Temporal *tdistance_tint_int(const Temporal *temp, int i);
extern Temporal *tdistance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern double nad_tboxfloat_tboxfloat(const TBox *box1, const TBox *box2);
extern int nad_tboxint_tboxint(const TBox *box1, const TBox *box2);
extern double nad_tfloat_float(const Temporal *temp, double d);
extern double nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2);
extern double nad_tfloat_tbox(const Temporal *temp, const TBox *box);
extern int nad_tint_int(const Temporal *temp, int i);
extern int nad_tint_tbox(const Temporal *temp, const TBox *box);
extern int nad_tint_tint(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Aggregate functions for temporal types
 *****************************************************************************/

extern SkipList *tbool_tand_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tbool_tor_transfn(SkipList *state, const Temporal *temp);
extern Span *temporal_extent_transfn(Span *s, const Temporal *temp);
extern Temporal *temporal_tagg_finalfn(SkipList *state);
extern SkipList *temporal_tcount_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tmin_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_tsum_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tfloat_wmax_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *tfloat_wmin_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *tfloat_wsum_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *timestamptz_tcount_transfn(SkipList *state, TimestampTz t);
extern SkipList *tint_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_tmin_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_tsum_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tint_wmax_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *tint_wmin_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *tint_wsum_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern TBox *tnumber_extent_transfn(TBox *box, const Temporal *temp);
extern Temporal *tnumber_tavg_finalfn(SkipList *state);
extern SkipList *tnumber_tavg_transfn(SkipList *state, const Temporal *temp);
extern SkipList *tnumber_wavg_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
extern SkipList *tstzset_tcount_transfn(SkipList *state, const Set *s);
extern SkipList *tstzspan_tcount_transfn(SkipList *state, const Span *s);
extern SkipList *tstzspanset_tcount_transfn(SkipList *state, const SpanSet *ss);
extern SkipList *ttext_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *ttext_tmin_transfn(SkipList *state, const Temporal *temp);

/*****************************************************************************
 * Analytics functions for temporal types
 *****************************************************************************/

/* Simplification functions for temporal types */

extern Temporal *temporal_simplify_dp(const Temporal *temp, double eps_dist, bool synchronized);
extern Temporal *temporal_simplify_max_dist(const Temporal *temp, double eps_dist, bool synchronized);
extern Temporal *temporal_simplify_min_dist(const Temporal *temp, double dist);
extern Temporal *temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint);

/*****************************************************************************/

/* Reduction functions for temporal types */

extern Temporal *temporal_tprecision(const Temporal *temp, const Interval *duration, TimestampTz origin);
extern Temporal *temporal_tsample(const Temporal *temp, const Interval *duration, TimestampTz origin, interpType interp);

/*****************************************************************************/

/* Similarity functions for temporal types */

extern double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern double temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Tile functions for temporal types */

extern Span *temporal_time_bins(const Temporal *temp, const Interval *duration, TimestampTz origin, int *count);
extern Temporal **temporal_time_split(const Temporal *temp, const Interval *duration, TimestampTz torigin, TimestampTz **time_bins, int *count);
extern TBox *tfloat_time_boxes(const Temporal *temp, const Interval *duration, TimestampTz torigin, int *count);
extern Span *tfloat_value_bins(const Temporal *temp, double vsize, double vorigin, int *count);
extern TBox *tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin, int *count);
extern Temporal **tfloat_value_split(const Temporal *temp, double size, double origin, double **bins, int *count);
extern TBox *tfloat_value_time_boxes(const Temporal *temp, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, int *count);
extern Temporal **tfloat_value_time_split(const Temporal *temp, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, double **value_bins, TimestampTz **time_bins, int *count);
extern TBox *tfloatbox_time_tiles(const TBox *box, const Interval *duration, TimestampTz torigin, int *count);
extern TBox *tfloatbox_value_tiles(const TBox *box, double vsize, double vorigin, int *count);
extern TBox *tfloatbox_value_time_tiles(const TBox *box, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, int *count);
extern TBox *tint_time_boxes(const Temporal *temp, const Interval *duration, TimestampTz torigin, int *count);
extern Span *tint_value_bins(const Temporal *temp, int vsize, int vorigin, int *count);
extern TBox *tint_value_boxes(const Temporal *temp, int vsize, int vorigin, int *count);
extern Temporal **tint_value_split(const Temporal *temp, int vsize, int vorigin, int **bins, int *count);
extern TBox *tint_value_time_boxes(const Temporal *temp, int vsize, const Interval *duration, int vorigin, TimestampTz torigin, int *count);
extern Temporal **tint_value_time_split(const Temporal *temp, int size, const Interval *duration, int vorigin, TimestampTz torigin, int **value_bins, TimestampTz **time_bins, int *count);
extern TBox *tintbox_time_tiles(const TBox *box, const Interval *duration, TimestampTz torigin, int *count);
extern TBox *tintbox_value_tiles(const TBox *box, int xsize, int xorigin, int *count);
extern TBox *tintbox_value_time_tiles(const TBox *box, int xsize, const Interval *duration, int xorigin, TimestampTz torigin, int *count);

/*****************************************************************************/

#endif
