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
#include "postgres_int_defs.h"
/* PostGIS */
#include <liblwgeom.h>

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
 * @brief Enumeration that defines the spatial relationships for which a call
 * to GEOS is made.
 */
typedef enum
{
  INTERSECTS =     0,
  CONTAINS =       1,
  TOUCHES =        2,
} spatialRel;

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
 * Error codes
 *****************************************************************************/

typedef enum
{
  MEOS_SUCCESS                  = 0,  // Successful operation

  MEOS_ERR_INTERNAL_ERROR       = 1,  // Unspecified internal error
  MEOS_ERR_INTERNAL_TYPE_ERROR  = 2,  // Internal type error
  MEOS_ERR_VALUE_OUT_OF_RANGE   = 3,  // Internal out of range error
  MEOS_ERR_DIVISION_BY_ZERO     = 4,  // Internal division by zero error
  MEOS_ERR_MEMORY_ALLOC_ERROR   = 5,  // Internal malloc error
  MEOS_ERR_AGGREGATION_ERROR    = 6,  // Internal aggregation error
  MEOS_ERR_DIRECTORY_ERROR      = 7,  // Internal directory error
  MEOS_ERR_FILE_ERROR           = 8,  // Internal file error

  MEOS_ERR_INVALID_ARG          = 10, // Invalid argument
  MEOS_ERR_INVALID_ARG_TYPE     = 11, // Invalid argument type
  MEOS_ERR_INVALID_ARG_VALUE    = 12, // Invalid argument value

  MEOS_ERR_MFJSON_INPUT         = 20, // MFJSON input error
  MEOS_ERR_MFJSON_OUTPUT        = 21, // MFJSON output error
  MEOS_ERR_TEXT_INPUT           = 22, // Text input error
  MEOS_ERR_TEXT_OUTPUT          = 23, // Text output error
  MEOS_ERR_WKB_INPUT            = 24, // WKB input error
  MEOS_ERR_WKB_OUTPUT           = 25, // WKB output error
  MEOS_ERR_GEOJSON_INPUT        = 26, // GEOJSON input error
  MEOS_ERR_GEOJSON_OUTPUT       = 27, // GEOJSON output error

} errorCode;

extern void meos_error(int errlevel, int errcode, char *format, ...);

/* Set or read error level */

extern int meos_errno(void);
extern int meos_errno_set(int err);
extern int meos_errno_restore(int err);
extern int meos_errno_reset(void);

/*****************************************************************************
 * Initialization of the MEOS library
 *****************************************************************************/

/* Definition of error handler function */
typedef void (*error_handler_fn)(int, int, char *);

extern void meos_initialize_timezone(const char *name);
extern void meos_initialize_error_handler(error_handler_fn err_handler);
extern void meos_finalize_timezone(void);

extern void meos_initialize(const char *tz_str, error_handler_fn err_handler);
extern void meos_finalize(void);

/*****************************************************************************
 * Functions for PostgreSQL types
 *****************************************************************************/

extern bool bool_in(const char *in_str);
extern char *bool_out(bool b);
extern text *cstring2text(const char *cstring);
extern DateADT pg_date_in(const char *str);
extern char *pg_date_out(DateADT date);
extern int pg_interval_cmp(const Interval *interval1, const Interval *interval2);
extern Interval *pg_interval_in(const char *str, int32 typmod);
extern Interval *pg_interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, double secs);
extern Interval *pg_interval_mul(const Interval *span, double factor);
extern char *pg_interval_out(const Interval *span);
extern text *pg_interval_to_char(Interval *it, text *fmt);
extern Interval *pg_interval_pl(const Interval *span1, const Interval *span2);
extern TimeADT pg_time_in(const char *str, int32 typmod);
extern char *pg_time_out(TimeADT time);
extern Timestamp pg_timestamp_in(const char *str, int32 typmod);
extern Interval *pg_timestamp_mi(TimestampTz dt1, TimestampTz dt2);
extern TimestampTz pg_timestamp_mi_interval(TimestampTz timestamp, const Interval *span);
extern char *pg_timestamp_out(Timestamp dt);
extern TimestampTz pg_timestamp_pl_interval(TimestampTz timestamp, const Interval *span);
extern text *pg_timestamp_to_char(Timestamp dt, text *fmt);
extern TimestampTz pg_timestamptz_in(const char *str, int32 typmod);
extern char *pg_timestamptz_out(TimestampTz dt);
extern text *pg_timestamptz_to_char(TimestampTz dt, text *fmt);
extern DateADT pg_to_date(text *date_txt, text *fmt);
extern TimestampTz pg_to_timestamp(text *date_txt, text *fmt);
extern char *text2cstring(const text *textptr);
extern char *text_out(const text *txt);

/*****************************************************************************
 * Functions for PostGIS types
 *****************************************************************************/

extern GSERIALIZED *geography_from_hexewkb(const char *wkt);
extern GSERIALIZED *geography_from_text(char *wkt, int srid);
extern GSERIALIZED *geometry_from_hexewkb(const char *wkt);
extern GSERIALIZED *geometry_from_text(char *wkt, int srid);
extern bytea *gserialized_as_ewkb(const GSERIALIZED *gs, char *type);
extern char *gserialized_as_ewkt(const GSERIALIZED *gs, int precision);
extern char *gserialized_as_geojson(const GSERIALIZED *gs, int option, int precision, char *srs);
extern char *gserialized_as_hexewkb(const GSERIALIZED *gs, const char *type);
extern char *gserialized_as_text(const GSERIALIZED *gs, int precision);
extern GSERIALIZED *gserialized_from_ewkb(const bytea *bytea_wkb, int32 srid);
extern GSERIALIZED *gserialized_from_geojson(const char *geojson);
extern char *gserialized_out(const GSERIALIZED *gs);
extern GSERIALIZED *pgis_geography_in(char *input, int32 geom_typmod);
extern GSERIALIZED *pgis_geometry_in(char *input, int32 geom_typmod);
extern bool pgis_gserialized_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2);

/*****************************************************************************
 * Functions for set and span types
 *****************************************************************************/

/* Input/output functions for set and span types */

extern Set *bigintset_in(const char *str);
extern char *bigintset_out(const Set *set);
extern Span *bigintspan_in(const char *str);
extern char *bigintspan_out(const Span *s);
extern SpanSet *bigintspanset_in(const char *str);
extern char *bigintspanset_out(const SpanSet *ss);
extern Set *floatset_in(const char *str);
extern char *floatset_out(const Set *set, int maxdd);
extern Span *floatspan_in(const char *str);
extern char *floatspan_out(const Span *s, int maxdd);
extern SpanSet *floatspanset_in(const char *str);
extern char *floatspanset_out(const SpanSet *ss, int maxdd);
extern Set *geogset_in(const char *str);
extern Set *geomset_in(const char *str);
extern char *geoset_as_ewkt(const Set *set, int maxdd);
extern char *geoset_as_text(const Set *set, int maxdd);
extern char *geoset_out(const Set *set, int maxdd);
extern Set *intset_in(const char *str);
extern char *intset_out(const Set *set);
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

/*****************************************************************************/

/* Constructor functions for set and span types */

extern Set *bigintset_make(const int64 *values, int count);
extern Span *bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc);
extern Set *floatset_make(const double *values, int count);
extern Span *floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc);
extern Set *geoset_make(const GSERIALIZED **values, int count);
extern Set *intset_make(const int *values, int count);
extern Span *intspan_make(int lower, int upper, bool lower_inc, bool upper_inc);
extern Set *set_copy(const Set *s);
extern Span *span_copy(const Span *s);
extern SpanSet *spanset_copy(const SpanSet *ps);
extern SpanSet *spanset_make(Span *spans, int count, bool normalize);
extern Set *textset_make(const text **values, int count);
extern Set *tstzset_make(const TimestampTz *values, int count);
extern Span *tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc);

/*****************************************************************************/

/* Conversion functions for set and span types */

extern Set *bigint_to_set(int64 i);
extern Span *bigint_to_span(int i);
extern SpanSet *bigint_to_spanset(int i);
extern Span *date_to_tstzspan(DateADT d);
extern Set *float_to_set(double d);
extern Span *float_to_span(double d);
extern SpanSet *float_to_spanset(double d);
extern Set *geo_to_set(GSERIALIZED *gs);
extern Set *int_to_set(int i);
extern Span *int_to_span(int i);
extern SpanSet *int_to_spanset(int i);
extern SpanSet *set_to_spanset(const Set *s);
extern SpanSet *span_to_spanset(const Span *s);
extern Set *text_to_set(text *txt);
extern Set *timestamptz_to_set(TimestampTz t);
extern Span *timestamptz_to_span(TimestampTz t);
extern SpanSet *timestamptz_to_spanset(TimestampTz t);

/*****************************************************************************/

/* Accessor functions for set and span types */

extern int64 bigintset_end_value(const Set *s);
extern int64 bigintset_start_value(const Set *s);
extern bool bigintset_value_n(const Set *s, int n, int64 *result);
extern int64 *bigintset_values(const Set *s);
extern int bigintspan_lower(const Span *s);
extern int bigintspan_upper(const Span *s);
extern int bigintspanset_lower(const SpanSet *ss);
extern int bigintspanset_upper(const SpanSet *ss);
extern double floatset_end_value(const Set *s);
extern double floatset_start_value(const Set *s);
extern bool floatset_value_n(const Set *s, int n, double *result);
extern double *floatset_values(const Set *s);
extern double floatspan_lower(const Span *s);
extern double floatspan_upper(const Span *s);
extern double floatspanset_lower(const SpanSet *ss);
extern double floatspanset_upper(const SpanSet *ss);
extern GSERIALIZED *geoset_end_value(const Set *s);
extern int geoset_srid(const Set *set);
extern GSERIALIZED *geoset_start_value(const Set *s);
extern bool geoset_value_n(const Set *s, int n, GSERIALIZED **result);
extern GSERIALIZED **geoset_values(const Set *s);
extern int intset_end_value(const Set *s);
extern int intset_start_value(const Set *s);
extern bool intset_value_n(const Set *s, int n, int *result);
extern int *intset_values(const Set *s);
extern int intspan_lower(const Span *s);
extern int intspan_upper(const Span *s);
extern int intspanset_lower(const SpanSet *ss);
extern int intspanset_upper(const SpanSet *ss);
extern uint32 set_hash(const Set *s);
extern uint64 set_hash_extended(const Set *s, uint64 seed);
extern int set_num_values(const Set *s);
extern Span *set_span(const Set *s);
extern uint32 span_hash(const Span *s);
extern uint64 span_hash_extended(const Span *s, uint64 seed);
extern bool span_lower_inc(const Span *s);
extern bool span_upper_inc(const Span *s);
extern double span_width(const Span *s);
extern Span *spanset_end_span(const SpanSet *ss);
extern uint32 spanset_hash(const SpanSet *ps);
extern uint64 spanset_hash_extended(const SpanSet *ps, uint64 seed);
extern bool spanset_lower_inc(const SpanSet *ss);
extern int spanset_num_spans(const SpanSet *ss);
extern Span *spanset_span(const SpanSet *ss);
extern Span *spanset_span_n(const SpanSet *ss, int i);
extern const Span **spanset_spans(const SpanSet *ss);
extern Span *spanset_start_span(const SpanSet *ss);
extern bool spanset_upper_inc(const SpanSet *ss);
extern double spanset_width(const SpanSet *ss, bool boundspan);
extern STBox *spatialset_stbox(const Set *s);
extern text *textset_end_value(const Set *s);
extern text *textset_start_value(const Set *s);
extern bool textset_value_n(const Set *s, int n, text **result);
extern text **textset_values(const Set *s);
extern TimestampTz tstzset_end_value(const Set *ts);
extern TimestampTz tstzset_start_value(const Set *ts);
extern bool tstzset_value_n(const Set *ts, int n, TimestampTz *result);
extern TimestampTz *tstzset_values(const Set *ts);
extern Interval *tstzspan_duration(const Span *s);
extern TimestampTz tstzspan_lower(const Span *p);
extern TimestampTz tstzspan_upper(const Span *p);
extern Interval *tstzspanset_duration(const SpanSet *ps, bool boundspan);
extern TimestampTz tstzspanset_end_timestamp(const SpanSet *ps);
extern TimestampTz tstzspanset_lower(const SpanSet *ps);
extern int tstzspanset_num_timestamps(const SpanSet *ps);
extern TimestampTz tstzspanset_start_timestamp(const SpanSet *ps);
extern bool tstzspanset_timestamp_n(const SpanSet *ps, int n, TimestampTz *result);
extern TimestampTz *tstzspanset_timestamps(const SpanSet *ps, int *count);
extern TimestampTz tstzspanset_upper(const SpanSet *ps);

/*****************************************************************************/

/* Transformation functions for set and span types */

extern Set *bigintset_shift_scale(const Set *s, int64 shift, int64 width, bool hasshift, bool haswidth);
extern Span *bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift, bool haswidth);
extern SpanSet *bigintspanset_shift_scale(const SpanSet *ss, int64 shift, int64 width, bool hasshift, bool haswidth);
extern Set *floatset_round(const Set *s, int maxdd);
extern Set *floatset_shift_scale(const Set *s, double shift, double width, bool hasshift, bool haswidth);
extern Span *floatspan_intspan(const Span *s);
extern Span *floatspan_round(const Span *s, int maxdd);
extern Span *floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift, bool haswidth);
extern SpanSet *floatspanset_intspanset(const SpanSet *ss);
extern SpanSet *floatspanset_round(const SpanSet *ss, int maxdd);
extern SpanSet *floatspanset_shift_scale(const SpanSet *ss, double shift, double width, bool hasshift, bool haswidth);
extern Set *geoset_round(const Set *s, int maxdd);
extern Set *intset_shift_scale(const Set *s, int shift, int width, bool hasshift, bool haswidth);
extern Span *intspan_floatspan(const Span *s);
extern Span *intspan_shift_scale(const Span *s, int shift, int width, bool hasshift, bool haswidth);
extern SpanSet *intspanset_floatspanset(const SpanSet *ss);
extern SpanSet *intspanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift, bool haswidth);
extern Set *textset_lower(const Set *s);
extern Set *textset_upper(const Set *s);
extern TimestampTz timestamptz_tprecision(TimestampTz t, const Interval *duration, TimestampTz torigin);
extern Set *tstzset_shift_scale(const Set *ts, const Interval *shift, const Interval *duration);
extern Span *tstzspan_shift_scale(const Span *p, const Interval *shift, const Interval *duration);
extern Span *tstzspan_tprecision(const Span *s, const Interval *duration, TimestampTz torigin);
extern SpanSet *tstzspanset_shift_scale(const SpanSet *ss, const Interval *shift, const Interval *duration);
extern SpanSet *tstzspanset_tprecision(const SpanSet *ss, const Interval *duration, TimestampTz torigin);

/*****************************************************************************/

/* Set functions for set and span types */

extern bool intersection_set_bigint(const Set *s, int64 i, int64 *result);
extern bool intersection_set_float(const Set *s, double d, double *result);
extern bool intersection_set_geo(const Set *s, const GSERIALIZED *gs, GSERIALIZED **result);
extern bool intersection_set_int(const Set *s, int i, int *result);
extern Set *intersection_set_set(const Set *s1, const Set *s2);
extern bool intersection_set_text(const Set *s, const text *txt, text **result);
extern bool intersection_set_timestamptz(const Set *s, TimestampTz t, TimestampTz *result);
extern bool intersection_span_bigint(const Span *s, int64 i, int64 *result);
extern bool intersection_span_float(const Span *s, double d, double *result);
extern bool intersection_span_int(const Span *s, int i, int *result);
extern bool intersection_span_timestamptz(const Span *s, TimestampTz t, TimestampTz *result);
extern Span *intersection_span_span(const Span *s1, const Span *s2);
extern bool intersection_spanset_bigint(const SpanSet *ss, int64 i, int64 *result);
extern bool intersection_spanset_float(const SpanSet *ss, double d, double *result);
extern bool intersection_spanset_int(const SpanSet *ss, int i, int *result);
extern SpanSet *intersection_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool intersection_spanset_timestamptz(const SpanSet *ss, TimestampTz t, TimestampTz *result);
extern bool minus_bigint_set(int64 i, const Set *s, int64 *result);
extern bool minus_bigint_span(int64 i, const Span *s, int64 *result);
extern bool minus_bigint_spanset(int64 i, const SpanSet *ss, int64 *result);
extern bool minus_float_set(double d, const Set *s, double *result);
extern bool minus_float_span(double d, const Span *s, double *result);
extern bool minus_float_spanset(double d, const SpanSet *ss, double *result);
extern bool minus_geo_set(const GSERIALIZED *gs, const Set *s, GSERIALIZED **result);
extern bool minus_int_set(int i, const Set *s, int *result);
extern bool minus_int_span(int i, const Span *s, int *result);
extern bool minus_int_spanset(int i, const SpanSet *ss, int *result);
extern Set *minus_set_bigint(const Set *s, int64 i);
extern Set *minus_set_int(const Set *s, int i);
extern Set *minus_set_float(const Set *s, double d);
extern Set *minus_set_geo(const Set *s, const GSERIALIZED *gs);
extern Set *minus_set_set(const Set *s1, const Set *s2);
extern Set *minus_set_text(const Set *s, const text *txt);
extern Set *minus_set_timestamptz(const Set *s, TimestampTz t);
extern SpanSet *minus_span_bigint(const Span *s, int64 i);
extern SpanSet *minus_span_float(const Span *s, double d);
extern SpanSet *minus_span_int(const Span *s, int i);
extern SpanSet *minus_span_span(const Span *s1, const Span *s2);
extern SpanSet *minus_span_spanset(const Span *s, const SpanSet *ss);
extern SpanSet *minus_span_timestamptz(const Span *s, TimestampTz t);
extern SpanSet *minus_spanset_bigint(const SpanSet *ss, int64 i);
extern SpanSet *minus_spanset_float(const SpanSet *ss, double d);
extern SpanSet *minus_spanset_int(const SpanSet *ss, int i);
extern SpanSet *minus_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern SpanSet *minus_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool minus_text_set(const text *txt, const Set *s, text **result);
extern bool minus_timestamptz_span(TimestampTz t, const Span *s, TimestampTz *result);
extern bool minus_timestamptz_spanset(TimestampTz t, const SpanSet *ss, TimestampTz *result);
extern bool minus_timestamptz_set(TimestampTz t, const Set *s, TimestampTz *result);
extern Set *union_set_bigint(const Set *s, int64 i);
extern Set *union_set_float(const Set *s, double d);
extern Set *union_set_geo(const Set *s, const GSERIALIZED *gs);
extern Set *union_set_int(const Set *s, int i);
extern Set *union_set_set(const Set *s1, const Set *s2);
extern Set *union_set_text(const Set *s, const text *txt);
extern Set *union_set_timestamptz(const Set *s, const TimestampTz t);
extern SpanSet *union_span_bigint(const Span *s, int64 i);
extern SpanSet *union_span_float(const Span *s, double d);
extern SpanSet *union_span_int(const Span *s, int i);
extern SpanSet *union_span_span(const Span *s1, const Span *s2);
extern SpanSet *union_span_timestamptz(const Span *s, TimestampTz t);
extern SpanSet *union_spanset_bigint(const SpanSet *ss, int64 i);
extern SpanSet *union_spanset_float(const SpanSet *ss, double d);
extern SpanSet *union_spanset_int(const SpanSet *ss, int i);
extern SpanSet *union_spanset_timestamptz(SpanSet *ss, TimestampTz t);
extern SpanSet *union_spanset_span(const SpanSet *ss, const Span *s);
extern SpanSet *union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Topological functions for set and span types */

extern bool adjacent_span_bigint(const Span *s, int64 i);
extern bool adjacent_span_float(const Span *s, double d);
extern bool adjacent_span_int(const Span *s, int i);
extern bool adjacent_span_span(const Span *s1, const Span *s2);
extern bool adjacent_span_timestamptz(const Span *p, TimestampTz t);
extern bool adjacent_spanset_bigint(const SpanSet *ss, int64 i);
extern bool adjacent_spanset_float(const SpanSet *ss, double d);
extern bool adjacent_spanset_int(const SpanSet *ss, int i);
extern bool adjacent_spanset_timestamptz(const SpanSet *ps, TimestampTz t);
extern bool adjacent_spanset_span(const SpanSet *ss, const Span *s);
extern bool adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_bigint_set(int64 i, const Set *s);
extern bool contained_bigint_span(int64 i, const Span *s);
extern bool contained_bigint_spanset(int64 i, const SpanSet *ss);
extern bool contained_float_set(double d, const Set *s);
extern bool contained_float_span(double d, const Span *s);
extern bool contained_float_spanset(double d, const SpanSet *ss);
extern bool contained_geo_set(GSERIALIZED *gs, const Set *s);
extern bool contained_int_set(int i, const Set *s);
extern bool contained_int_span(int i, const Span *s);
extern bool contained_int_spanset(int i, const SpanSet *ss);
extern bool contained_set_set(const Set *s1, const Set *s2);
extern bool contained_span_span(const Span *s1, const Span *s2);
extern bool contained_span_spanset(const Span *s, const SpanSet *ss);
extern bool contained_spanset_span(const SpanSet *ss, const Span *s);
extern bool contained_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contained_text_set(text *txt, const Set *s);
extern bool contained_timestamptz_span(TimestampTz t, const Span *p);
extern bool contained_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool contained_timestamptz_set(TimestampTz t, const Set *ts);
extern bool contains_set_bigint(const Set *s, int64 i);
extern bool contains_set_float(const Set *s, double d);
extern bool contains_set_geo(const Set *s, GSERIALIZED *gs);
extern bool contains_set_int(const Set *s, int i);
extern bool contains_set_set(const Set *s1, const Set *s2);
extern bool contains_set_text(const Set *s, text *t);
extern bool contains_set_timestamptz(const Set *s, TimestampTz t);
extern bool contains_span_bigint(const Span *s, int64 i);
extern bool contains_span_float(const Span *s, double d);
extern bool contains_span_int(const Span *s, int i);
extern bool contains_span_span(const Span *s1, const Span *s2);
extern bool contains_span_spanset(const Span *s, const SpanSet *ss);
extern bool contains_span_timestamptz(const Span *p, TimestampTz t);
extern bool contains_spanset_bigint(const SpanSet *ss, int64 i);
extern bool contains_spanset_float(const SpanSet *ss, double d);
extern bool contains_spanset_int(const SpanSet *ss, int i);
extern bool contains_spanset_span(const SpanSet *ss, const Span *s);
extern bool contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern bool contains_spanset_timestamptz(const SpanSet *ps, TimestampTz t);
extern bool overlaps_set_set(const Set *s1, const Set *s2);
extern bool overlaps_span_span(const Span *s1, const Span *s2);
extern bool overlaps_spanset_span(const SpanSet *ss, const Span *s);
extern bool overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool after_set_timestamptz(const Set *s, TimestampTz t);
extern bool after_span_timestamptz(const Span *s, TimestampTz t);
extern bool after_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool after_timestamptz_set(TimestampTz t, const Set *ts);
extern bool after_timestamptz_span(TimestampTz t, const Span *s);
extern bool after_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool before_set_timestamptz(const Set *s, TimestampTz t);
extern bool before_span_timestamptz(const Span *s, TimestampTz t);
extern bool before_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool before_timestamptz_set(TimestampTz t, const Set *ts);
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
extern bool left_text_set(text *txt, const Set *s);
extern bool overafter_set_timestamptz(const Set *s, TimestampTz t);
extern bool overafter_span_timestamptz(const Span *s, TimestampTz t);
extern bool overafter_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool overafter_timestamptz_set(TimestampTz t, const Set *ts);
extern bool overafter_timestamptz_span(TimestampTz t, const Span *s);
extern bool overafter_timestamptz_spanset(TimestampTz t, const SpanSet *ss);
extern bool overbefore_set_timestamptz(const Set *s, TimestampTz t);
extern bool overbefore_span_timestamptz(const Span *s, TimestampTz t);
extern bool overbefore_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern bool overbefore_timestamptz_set(TimestampTz t, const Set *ts);
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
extern bool overleft_text_set(text *txt, const Set *s);
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
extern bool overright_text_set(text *txt, const Set *s);
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
extern bool right_text_set(text *txt, const Set *s);

/*****************************************************************************/

/* Distance functions for set and span types */

extern double distance_set_bigint(const Set *s, int64 i);
extern double distance_set_float(const Set *s, double d);
extern double distance_set_int(const Set *s, int i);
extern double distance_set_set(const Set *s1, const Set *s2);
extern double distance_set_timestamptz(const Set *s, TimestampTz t);
extern double distance_span_bigint(const Span *s, int64 i);
extern double distance_span_float(const Span *s, double d);
extern double distance_span_int(const Span *s, int i);
extern double distance_span_span(const Span *s1, const Span *s2);
extern double distance_span_timestamptz(const Span *s, TimestampTz t);
extern double distance_spanset_bigint(const SpanSet *ss, int64 i);
extern double distance_spanset_float(const SpanSet *ss, double d);
extern double distance_spanset_int(const SpanSet *ss, int i);
extern double distance_spanset_timestamptz(const SpanSet *ss, TimestampTz t);
extern double distance_spanset_span(const SpanSet *ss, const Span *s);
extern double distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);

/*****************************************************************************/

/* Comparison functions for set and span types */

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

/*****************************************************************************/

/* Aggregate functions for set and span types */

extern Span *bigint_extent_transfn(Span *s, int64 i);
extern Set *bigint_union_transfn(Set *state, int64 i);
extern Span *float_extent_transfn(Span *s, double d);
extern Set *float_union_transfn(Set *state, double d);
extern Span *int_extent_transfn(Span *s, int i);
extern Set *int_union_transfn(Set *state, int i);
extern Span *set_extent_transfn(Span *span, const Set *set);
extern Set *set_union_finalfn(Set *state);
extern Set *set_union_transfn(Set *state, Set *set);
extern Span *span_extent_transfn(Span *s1, const Span *s2);
extern SpanSet *span_union_transfn(SpanSet *state, const Span *span);
extern Span *spanset_extent_transfn(Span *s, const SpanSet *ss);
extern SpanSet *spanset_union_finalfn(SpanSet *state);
extern SpanSet *spanset_union_transfn(SpanSet *state, const SpanSet *ss);
extern Set *text_union_transfn(Set *state, const text *txt);
extern Span *timestamptz_extent_transfn(Span *p, TimestampTz t);
extern SkipList *timestamptz_tcount_transfn(SkipList *state, TimestampTz t);
extern Set *timestamptz_union_transfn(Set *state, TimestampTz t);
extern SkipList *tstzset_tcount_transfn(SkipList *state, const Set *ts);
extern SkipList *tstzspan_tcount_transfn(SkipList *state, const Span *p);
extern SkipList *tstzspanset_tcount_transfn(SkipList *state, const SpanSet *ps);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */

extern TBox *tbox_in(const char *str);
extern char *tbox_out(const TBox *box, int maxdd);
extern TBox *tbox_from_wkb(const uint8_t *wkb, size_t size);
extern TBox *tbox_from_hexwkb(const char *hexwkb);
extern STBox *stbox_from_wkb(const uint8_t *wkb, size_t size);
extern STBox *stbox_from_hexwkb(const char *hexwkb);
extern uint8_t *tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out);
extern char *tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size);
extern uint8_t *stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out);
extern char *stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size);
extern STBox *stbox_in(const char *str);
extern char *stbox_out(const STBox *box, int maxdd);

/*****************************************************************************/

/* Constructor functions for box types */

extern TBox *float_tstzspan_to_tbox(double d, const Span *p);
extern TBox *float_timestamptz_to_tbox(double d, TimestampTz t);
extern STBox *geo_tstzspan_to_stbox(const GSERIALIZED *gs, const Span *p);
extern STBox *geo_timestamptz_to_stbox(const GSERIALIZED *gs, TimestampTz t);
extern TBox *int_tstzspan_to_tbox(int i, const Span *p);
extern TBox *int_timestamptz_to_tbox(int i, TimestampTz t);
extern TBox *numspan_tstzspan_to_tbox(const Span *span, const Span *p);
extern TBox *numspan_timestamptz_to_tbox(const Span *span, TimestampTz t);
extern STBox *stbox_copy(const STBox *box);
extern STBox *stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *p);
extern TBox *tbox_copy(const TBox *box);
extern TBox *tbox_make(const Span *s, const Span *p);

/*****************************************************************************/

/* Conversion functions for box types */

extern STBox *box3d_to_stbox(const BOX3D *box);
extern STBox *gbox_to_stbox(const GBOX *box);
extern TBox *float_to_tbox(double d);
extern STBox *geo_to_stbox(const GSERIALIZED *gs);
extern TBox *int_to_tbox(int i);
extern TBox *numset_to_tbox(const Set *s);
extern TBox *numspan_to_tbox(const Span *s);
extern TBox *numspanset_to_tbox(const SpanSet *ss);
extern STBox *tstzspan_to_stbox(const Span *p);
extern TBox *tstzspan_to_tbox(const Span *p);
extern STBox *tstzspanset_to_stbox(const SpanSet *ps);
extern TBox *tstzspanset_to_tbox(const SpanSet *ps);
extern GBOX *stbox_to_gbox(const STBox *box);
extern BOX3D *stbox_to_box3d(const STBox *box);
extern GSERIALIZED *stbox_to_geo(const STBox *box);
extern Span *stbox_to_tstzspan(const STBox *box);
extern Span *tbox_to_floatspan(const TBox *box);
extern Span *tbox_to_tstzspan(const TBox *box);
extern STBox *timestamptz_to_stbox(TimestampTz t);
extern TBox *timestamptz_to_tbox(TimestampTz t);
extern STBox *tstzset_to_stbox(const Set *ts);
extern TBox *tstzset_to_tbox(const Set *ss);
extern TBox *tnumber_to_tbox(const Temporal *temp);
extern STBox *tpoint_to_stbox(const Temporal *temp);

/*****************************************************************************/

/* Accessor functions for box types */

extern bool stbox_hast(const STBox *box);
extern bool stbox_hasx(const STBox *box);
extern bool stbox_hasz(const STBox *box);
extern bool stbox_isgeodetic(const STBox *box);
extern int32 stbox_srid(const STBox *box);
extern bool stbox_tmax(const STBox *box, TimestampTz *result);
extern bool stbox_tmax_inc(const STBox *box, bool *result);
extern bool stbox_tmin(const STBox *box, TimestampTz *result);
extern bool stbox_tmin_inc(const STBox *box, bool *result);
extern bool stbox_xmax(const STBox *box, double *result);
extern bool stbox_xmin(const STBox *box, double *result);
extern bool stbox_ymax(const STBox *box, double *result);
extern bool stbox_ymin(const STBox *box, double *result);
extern bool stbox_zmax(const STBox *box, double *result);
extern bool stbox_zmin(const STBox *box, double *result);
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

/*****************************************************************************/

/* Transformation functions for box types */

extern STBox *stbox_expand_space(const STBox *box, double d);
extern STBox *stbox_expand_time(const STBox *box, const Interval *interval);
extern STBox *stbox_get_space(const STBox *box);
extern STBox *stbox_round(const STBox *box, int maxdd);
extern STBox *stbox_set_srid(const STBox *box, int32 srid);
extern STBox *stbox_shift_scale_time(const STBox *box, const Interval *shift, const Interval *duration);
extern TBox *tbox_expand_time(const TBox *box, const Interval *interval);
extern TBox *tbox_expand_value(const TBox *box, const double d);
extern TBox *tbox_round(const TBox *box, int maxdd);
extern TBox *tbox_shift_scale_float(const TBox *box, double shift, double width, bool hasshift, bool haswidth);
extern TBox *tbox_shift_scale_int(const TBox *box, int shift, int width, bool hasshift, bool haswidth);
extern TBox *tbox_shift_scale_time(const TBox *box, const Interval *shift, const Interval *duration);

/*****************************************************************************/

/* Set functions for box types */

extern TBox *union_tbox_tbox(const TBox *box1, const TBox *box2, bool strict);
extern bool inter_tbox_tbox(const TBox *box1, const TBox *box2, TBox *result);
extern TBox *intersection_tbox_tbox(const TBox *box1, const TBox *box2);
extern STBox *union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict);
extern bool inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result);
extern STBox *intersection_stbox_stbox(const STBox *box1, const STBox *box2);

/*****************************************************************************
 * Bounding box functions for box types
 *****************************************************************************/

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

/* Split functions */

extern STBox *stbox_quad_split(const STBox *box, int *count);

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

/* Input/output functions for temporal types */

extern Temporal *tbool_in(const char *str);
extern char *tbool_out(const Temporal *temp);
extern char *temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern char *temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags, int precision, char *srs);
extern uint8_t *temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out);
extern Temporal *temporal_from_hexwkb(const char *hexwkb);
extern Temporal *temporal_from_mfjson(const char *mfjson);
extern Temporal *temporal_from_wkb(const uint8_t *wkb, size_t size);
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

extern Temporal *tbool_from_base_temp(bool b, const Temporal *temp);
extern TInstant *tboolinst_make(bool b, TimestampTz t);
extern TSequence *tboolseq_from_base_tstzspan(bool b, const Span *p);
extern TSequence *tboolseq_from_base_tstzset(bool b, const Set *ts);
extern TSequenceSet *tboolseqset_from_base_tstzspanset(bool b, const SpanSet *ps);
extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *tfloat_from_base_temp(double d, const Temporal *temp);
extern TInstant *tfloatinst_make(double d, TimestampTz t);
extern TSequence *tfloatseq_from_base_tstzspan(double d, const Span *p, interpType interp);
extern TSequence *tfloatseq_from_base_tstzset(double d, const Set *ts);
extern TSequenceSet *tfloatseqset_from_base_tstzspanset(double d, const SpanSet *ps, interpType interp);
extern Temporal *tint_from_base_temp(int i, const Temporal *temp);
extern TInstant *tintinst_make(int i, TimestampTz t);
extern TSequence *tintseq_from_base_tstzspan(int i, const Span *p);
extern TSequence *tintseq_from_base_tstzset(int i, const Set *ts);
extern TSequenceSet *tintseqset_from_base_tstzspanset(int i, const SpanSet *ps);
extern Temporal *tpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp);
extern TInstant *tpointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tpointseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *p, interpType interp);
extern TSequence *tpointseq_from_base_tstzset(const GSERIALIZED *gs, const Set *ts);
extern TSequenceSet *tpointseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ps, interpType interp);
extern TSequence *tsequence_make(const TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_make(const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_gaps(const TInstant **instants, int count, interpType interp, Interval *maxt, double maxdist);
extern Temporal *ttext_from_base_temp(const text *txt, const Temporal *temp);
extern TInstant *ttextinst_make(const text *txt, TimestampTz t);
extern TSequence *ttextseq_from_base_tstzspan(const text *txt, const Span *p);
extern TSequence *ttextseq_from_base_tstzset(const text *txt, const Set *ts);
extern TSequenceSet *ttextseqset_from_base_tstzspanset(const text *txt, const SpanSet *ps);

/*****************************************************************************/

/* Conversion functions for temporal types */

extern Span *temporal_to_tstzspan(const Temporal *temp);
extern Temporal *tfloat_to_tint(const Temporal *temp);
extern Temporal *tint_to_tfloat(const Temporal *temp);
extern Span *tnumber_to_span(const Temporal *temp);

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
extern char *temporal_interp(const Temporal *temp);
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
extern TSequenceSet *temporal_stops(const Temporal *temp, double maxdist, const Interval *minduration);
extern char *temporal_subtype(const Temporal *temp);
extern SpanSet *temporal_time(const Temporal *temp);
extern bool temporal_timestamp_n(const Temporal *temp, int n, TimestampTz *result);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);
extern double tfloat_end_value(const Temporal *temp);
extern double tfloat_max_value(const Temporal *temp);
extern double tfloat_min_value(const Temporal *temp);
extern double tfloat_start_value(const Temporal *temp);
extern double *tfloat_values(const Temporal *temp, int *count);
extern int tint_end_value(const Temporal *temp);
extern int tint_max_value(const Temporal *temp);
extern int tint_min_value(const Temporal *temp);
extern int tint_start_value(const Temporal *temp);
extern int *tint_values(const Temporal *temp, int *count);
extern SpanSet *tnumber_valuespans(const Temporal *temp);
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

extern Temporal *temporal_scale_time(const Temporal *temp, const Interval *duration);
extern Temporal *temporal_set_interp(const Temporal *temp, interpType interp);
extern Temporal *temporal_shift_scale_time(const Temporal *temp, const Interval *shift, const Interval *duration);
extern Temporal *temporal_shift_time(const Temporal *temp, const Interval *shift);
extern Temporal *temporal_to_tinstant(const Temporal *temp);
extern Temporal *temporal_to_tsequence(const Temporal *temp, interpType interp);
extern Temporal *temporal_to_tsequenceset(const Temporal *temp, interpType interp);
extern Temporal *tfloat_scale_value(const Temporal *temp, double width);
extern Temporal *tfloat_shift_scale_value(const Temporal *temp, double shift, double width);
extern Temporal *tfloat_shift_value(const Temporal *temp, double shift);
extern Temporal *tint_scale_value(const Temporal *temp, int width);
extern Temporal *tint_shift_scale_value(const Temporal *temp, int shift, int width);
extern Temporal *tint_shift_value(const Temporal *temp, int shift);

/*****************************************************************************/

/* Modification functions for temporal types */

extern Temporal *temporal_append_tinstant(Temporal *temp, const TInstant *inst, double maxdist, Interval *maxt, bool expand);
extern Temporal *temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand);
extern Temporal *temporal_delete_tstzspan(const Temporal *temp, const Span *p, bool connect);
extern Temporal *temporal_delete_tstzspanset(const Temporal *temp, const SpanSet *ps, bool connect);
extern Temporal *temporal_delete_timestamp(const Temporal *temp, TimestampTz t, bool connect);
extern Temporal *temporal_delete_tstzset(const Temporal *temp, const Set *ts, bool connect);
extern Temporal *temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);
extern Temporal *temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *tbool_at_value(const Temporal *temp, bool b);
extern Temporal *tbool_minus_value(const Temporal *temp, bool b);
extern bool tbool_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, bool *value);
extern Temporal *temporal_at_max(const Temporal *temp);
extern Temporal *temporal_at_min(const Temporal *temp);
extern Temporal *temporal_at_tstzspan(const Temporal *temp, const Span *p);
extern Temporal *temporal_at_tstzspanset(const Temporal *temp, const SpanSet *ps);
extern Temporal *temporal_at_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_tstzset(const Temporal *temp, const Set *ts);
extern Temporal *temporal_at_values(const Temporal *temp, const Set *set);
extern Temporal *temporal_minus_max(const Temporal *temp);
extern Temporal *temporal_minus_min(const Temporal *temp);
extern Temporal *temporal_minus_tstzspan(const Temporal *temp, const Span *p);
extern Temporal *temporal_minus_tstzspanset(const Temporal *temp, const SpanSet *ps);
extern Temporal *temporal_minus_timestamp(const Temporal *temp, TimestampTz t);
extern Temporal *temporal_minus_tstzset(const Temporal *temp, const Set *ts);
extern Temporal *temporal_minus_values(const Temporal *temp, const Set *set);
extern Temporal *tfloat_at_value(const Temporal *temp, double d);
extern Temporal *tfloat_minus_value(const Temporal *temp, double d);
extern bool tfloat_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, double *value);
extern Temporal *tint_at_value(const Temporal *temp, int i);
extern Temporal *tint_minus_value(const Temporal *temp, int i);
extern bool tint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, int *value);
extern Temporal *tnumber_at_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_at_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBox *box);
extern Temporal *tnumber_minus_span(const Temporal *temp, const Span *span);
extern Temporal *tnumber_minus_spanset(const Temporal *temp, const SpanSet *ss);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBox *box);
extern Temporal *tpoint_at_geom_time(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, const Span *period);
extern Temporal *tpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpoint_at_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_minus_geom_time(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, const Span *period);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs);
extern bool tpoint_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value);
extern Temporal *ttext_at_value(const Temporal *temp, text *txt);
extern Temporal *ttext_minus_value(const Temporal *temp, text *txt);
extern bool ttext_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, text **value);

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

/* Ever/always comparison functions for temporal types */

extern bool tbool_always_eq(const Temporal *temp, bool b);
extern bool tbool_ever_eq(const Temporal *temp, bool b);
extern bool tfloat_always_eq(const Temporal *temp, double d);
extern bool tfloat_always_le(const Temporal *temp, double d);
extern bool tfloat_always_lt(const Temporal *temp, double d);
extern bool tfloat_ever_eq(const Temporal *temp, double d);
extern bool tfloat_ever_le(const Temporal *temp, double d);
extern bool tfloat_ever_lt(const Temporal *temp, double d);
extern bool tint_always_eq(const Temporal *temp, int i);
extern bool tint_always_le(const Temporal *temp, int i);
extern bool tint_always_lt(const Temporal *temp, int i);
extern bool tint_ever_eq(const Temporal *temp, int i);
extern bool tint_ever_le(const Temporal *temp, int i);
extern bool tint_ever_lt(const Temporal *temp, int i);
extern bool tpoint_always_eq(const Temporal *temp, const GSERIALIZED *gs);;
extern bool tpoint_ever_eq(const Temporal *temp, const GSERIALIZED *gs);;
extern bool ttext_always_eq(const Temporal *temp, text *txt);
extern bool ttext_always_le(const Temporal *temp, text *txt);
extern bool ttext_always_lt(const Temporal *temp, text *txt);
extern bool ttext_ever_eq(const Temporal *temp, text *txt);
extern bool ttext_ever_le(const Temporal *temp, text *txt);
extern bool ttext_ever_lt(const Temporal *temp, text *txt);

/*****************************************************************************/

/* Temporal comparison functions for temporal types */

extern Temporal *teq_bool_tbool(bool b, const Temporal *temp);
extern Temporal *teq_float_tfloat(double d, const Temporal *temp);
extern Temporal *teq_int_tint(int i, const Temporal *temp);
extern Temporal *teq_point_tpoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_tbool_bool(const Temporal *temp, bool b);
extern Temporal *teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *teq_tfloat_float(const Temporal *temp, double d);
extern Temporal *teq_tpoint_point(const Temporal *temp, const GSERIALIZED *gs);
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
extern Temporal *tne_point_tpoint(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tne_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *tne_tfloat_float(const Temporal *temp, double d);
extern Temporal *tne_tpoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_tint_int(const Temporal *temp, int i);
extern Temporal *tne_ttext_text(const Temporal *temp, const text *txt);

/*****************************************************************************/

/* Boolean functions for temporal types */

extern Temporal *tand_bool_tbool(bool b, const Temporal *temp);
extern Temporal *tand_tbool_bool(const Temporal *temp, bool b);
extern Temporal *tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2);
extern SpanSet *tbool_when_true(const Temporal *temp);
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
extern double float_degrees(double value, bool normalize);
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
extern Temporal *tfloat_round(const Temporal *temp, int maxdd);
extern Temporal **tfloatarr_round(const Temporal **temp, int count, int maxdd);
extern Temporal *tfloat_degrees(const Temporal *temp, bool normalize);
extern Temporal *tfloat_derivative(const Temporal *temp);
extern Temporal *tfloat_radians(const Temporal *temp);
extern Temporal *tnumber_abs(const Temporal *temp);
extern Temporal *tnumber_angular_difference(const Temporal *temp);
extern Temporal *tnumber_delta_value(const Temporal *temp);

/*****************************************************************************/

/* Text functions for temporal types */

extern Temporal *textcat_text_ttext(const text *txt, const Temporal *temp);
extern Temporal *textcat_ttext_text(const Temporal *temp, const text *txt);
extern Temporal *textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2);
extern Temporal *ttext_upper(const Temporal *temp);
extern Temporal *ttext_lower(const Temporal *temp);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tfloat_float(const Temporal *temp, double d);
extern Temporal *distance_tint_int(const Temporal *temp, int i);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);
extern Temporal *distance_tpoint_point(const Temporal *temp, const GSERIALIZED *gs);
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

/*****************************************************************************
  Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */

extern bool bearing_point_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result);
extern Temporal *bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert);
extern Temporal *bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tpoint_angular_difference(const Temporal *temp);
extern Temporal *tpoint_azimuth(const Temporal *temp);
extern GSERIALIZED *tpoint_convex_hull(const Temporal *temp);
extern Temporal *tpoint_cumulative_length(const Temporal *temp);
extern bool tpoint_direction(const Temporal *temp, double *result);
extern Temporal *tpoint_get_x(const Temporal *temp);
extern Temporal *tpoint_get_y(const Temporal *temp);
extern Temporal *tpoint_get_z(const Temporal *temp);
extern bool tpoint_is_simple(const Temporal *temp);
extern double tpoint_length(const Temporal *temp);
extern Temporal *tpoint_speed(const Temporal *temp);
extern int tpoint_srid(const Temporal *temp);
extern STBox *tpoint_stboxes(const Temporal *temp, int *count);
extern GSERIALIZED *tpoint_trajectory(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern STBox *geo_expand_space(const GSERIALIZED *gs, double d);
Temporal *geo_to_tpoint(const GSERIALIZED *gs);
extern Temporal *tgeogpoint_to_tgeompoint(const Temporal *temp);
extern Temporal *tgeompoint_to_tgeogpoint(const Temporal *temp);
bool tpoint_AsMVTGeom(const Temporal *temp, const STBox *bounds, int32_t extent, int32_t buffer, bool clip_geom, GSERIALIZED **gsarr, int64 **timesarr, int *count);
extern STBox *tpoint_expand_space(const Temporal *temp, double d);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);
extern Temporal *tpoint_round(const Temporal *temp, int maxdd);
extern Temporal **tpointarr_round(const Temporal **temp, int count, int maxdd);
extern Temporal *tpoint_set_srid(const Temporal *temp, int32 srid);
bool tpoint_to_geo_meas(const Temporal *tpoint, const Temporal *measure, bool segmentize, GSERIALIZED **result);

/*****************************************************************************/

/* Ever spatial relationship functions for temporal point types */

extern int econtains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp);
extern int edisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);

/*****************************************************************************/

/* Temporal spatial relationship functions for temporal point types */

extern Temporal *tcontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *tintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *ttouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);

/*****************************************************************************/

/* Aggregate functions for temporal types */

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
extern TBox *tnumber_extent_transfn(TBox *box, const Temporal *temp);
extern double tnumber_integral(const Temporal *temp);
extern Temporal *tnumber_tavg_finalfn(SkipList *state);
extern SkipList *tnumber_tavg_transfn(SkipList *state, const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);
extern STBox *tpoint_extent_transfn(STBox *box, const Temporal *temp);
extern Temporal *tpoint_tcentroid_finalfn(SkipList *state);
extern SkipList *tpoint_tcentroid_transfn(SkipList *state, Temporal *temp);
extern GSERIALIZED *tpoint_twcentroid(const Temporal *temp);
extern SkipList *ttext_tmax_transfn(SkipList *state, const Temporal *temp);
extern SkipList *ttext_tmin_transfn(SkipList *state, const Temporal *temp);

/*****************************************************************************
 * Analytics functions for temporal types
 *****************************************************************************/

/* Simplification functions for temporal types */

Temporal *temporal_simplify_min_dist(const Temporal *temp, double dist);
Temporal *temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint);
Temporal *temporal_simplify_dp(const Temporal *temp, double eps_dist, bool synchronized);
Temporal *temporal_simplify_max_dist(const Temporal *temp, double eps_dist, bool synchronized);

/*****************************************************************************/

/* Reduction functions for temporal types */

extern Temporal *temporal_tprecision(const Temporal *temp, const Interval *duration, TimestampTz origin);
extern Temporal *temporal_tsample(const Temporal *temp, const Interval *duration, TimestampTz origin);

/*****************************************************************************/

/* Similarity functions for temporal types */

extern double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2);
extern Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count);
extern double temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Tile functions for temporal types */

extern double float_bucket(double value, double size, double origin);
extern Span *floatspan_bucket_list(const Span *bounds, double size, double origin, int *count);
extern int int_bucket(int value, int size, int origin);
extern Span *intspan_bucket_list(const Span *bounds, int size, int origin, int *count);
extern Span *tstzspan_bucket_list(const Span *bounds, const Interval *duration, TimestampTz origin, int *count);
extern STBox *stbox_tile_list(const STBox *bounds, double xsize, double ysize, double zsize, const Interval *duration, GSERIALIZED *sorigin, TimestampTz torigin, int *count);
extern TBox *tintbox_tile_list(const TBox *box, int xsize, const Interval *duration, int xorigin, TimestampTz torigin, int *count);
extern TBox *tfloatbox_tile_list(const TBox *box, double xsize, const Interval *duration, double xorigin, TimestampTz torigin, int *count);
extern Temporal **temporal_time_split(Temporal *temp, Interval *duration, TimestampTz torigin, TimestampTz **time_buckets, int *count);
extern Temporal **tfloat_value_split(Temporal *temp, double size, double origin, double **value_buckets, int *count);
extern Temporal **tfloat_value_time_split(Temporal *temp, double size, Interval *duration, double vorigin, TimestampTz torigin, double **value_buckets, TimestampTz **time_buckets, int *count);
extern TimestampTz timestamptz_bucket(TimestampTz timestamp, const Interval *duration, TimestampTz origin);
extern Temporal **tint_value_split(Temporal *temp, int size, int origin, int **value_buckets, int *count);
extern Temporal **tint_value_time_split(Temporal *temp, int size, Interval *duration, int vorigin, TimestampTz torigin, int **value_buckets, TimestampTz **time_buckets, int *count);
extern Temporal **tpoint_space_split(Temporal *temp, float xsize, float ysize, float zsize, GSERIALIZED *sorigin, bool bitmatrix, GSERIALIZED ***space_buckets, int *count);
extern Temporal **tpoint_space_time_split(Temporal *temp, float xsize, float ysize, float zsize, Interval *duration, GSERIALIZED *sorigin, TimestampTz torigin, bool bitmatrix, GSERIALIZED ***space_buckets, TimestampTz **time_buckets, int *count);


/*****************************************************************************/

#endif
