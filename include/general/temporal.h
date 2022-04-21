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
 * @file temporal.h
 * Basic functions for temporal types of any subtype.
 */

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/lsyscache.h>
#include <utils/rangetypes.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timetypes.h"
#include "general/tbox.h"
#include "point/stbox.h"

#if POSTGRESQL_VERSION_NUMBER < 130000
#ifndef USE_FLOAT4_BYVAL
#error Postgres needs to be configured with USE_FLOAT4_BYVAL
#endif
#endif

#ifndef USE_FLOAT8_BYVAL
#error Postgres needs to be configured with USE_FLOAT8_BYVAL
#endif

/**
 * Floating point precision
 */
#define MOBDB_EPSILON   1.e-05
#define MOBDB_FP_EQ(A, B) (fabs((A)-(B)) <= MOBDB_EPSILON)
#define MOBDB_FP_NE(A, B) (fabs((A)-(B)) > MOBDB_EPSILON)
#define MOBDB_FP_LT(A, B) (((A) + MOBDB_EPSILON) < (B))
#define MOBDB_FP_LE(A, B) (((A) - MOBDB_EPSILON) <= (B))
#define MOBDB_FP_GT(A, B) (((A) - MOBDB_EPSILON) > (B))
#define MOBDB_FP_GE(A, B) (((A) + MOBDB_EPSILON) >= (B))

/**
 * Precision for distance operations
 */
#define DIST_EPSILON    1.0e-05

/** Symbolic constants for lifting */
#define DISCONTINUOUS   true
#define CONTINUOUS      false

/** Symbolic constants for lifting */
#define INVERT          true
#define INVERT_NO       false

/** Symbolic constants for the restriction functions */
#define REST_AT         true
#define REST_MINUS      false

/** Symbolic constants for the restriction functions with boxes */
#define UPPER_INC       true
#define UPPER_EXC       false

/** Symbolic constants for the ever/always functions */
#define EVER            true
#define ALWAYS          false

/** Symbolic constants for the restriction and the aggregation functions */
#define GET_MIN          true
#define GET_MAX          false

/** Symbolic constants for the synchronization and the aggregation functions */
#define CROSSINGS       true
#define CROSSINGS_NO    false

/** Symbolic constants for the synchronization and the aggregation functions */
#define BBOX_TEST       true
#define BBOX_TEST_NO    false

/** Symbolic constants for the make functions */
#define MERGE           true
#define MERGE_NO        false

#define NORMALIZE       true
#define NORMALIZE_NO    false

#define LINEAR          true
#define STEP            false

/** Symbolic constants for spatial relationships */
#define WITH_Z          true
#define NO_Z            false

/* Determine whether reduce the roundoff errors with the range operations
 * by taking the bounds instead of the projected value at the timestamp */
#define RANGE_ROUNDOFF  false

/** Enumeration for the intersection/synchronization functions */
typedef enum
{
  SYNCHRONIZE_NOCROSS,
  SYNCHRONIZE_CROSS,
} SyncMode;

/** Enumeration for the families of temporal types */
typedef enum
{
  TEMPORALTYPE,
  TNUMBERTYPE,
  TPOINTTYPE,
  TNPOINTTYPE,
} TemporalFamily;

/*****************************************************************************
 * Concrete subtype of temporal types
 *****************************************************************************/

/**
 * Enumeration for the concrete subtype of temporal types
 */
#define ANYTEMPSUBTYPE  0
#define INSTANT         1
#define INSTANTSET      2
#define SEQUENCE        3
#define SEQUENCESET     4

#define TYPMOD_GET_SUBTYPE(typmod) ((int16) ((typmod == -1) ? (0) : (typmod & 0x0000000F)))

/**
 * Structure to represent the temporal subtype array
 */
struct tempsubtype_struct
{
  char *subtypeName;   /**< string representing the temporal type */
  int16 subtype;       /**< subtype */
};

#define TEMPSUBTYPE_STRUCT_ARRAY_LEN \
  (sizeof tempsubtype_struct_array/sizeof(struct tempsubtype_struct))
#define TEMPSUBTYPE_MAX_LEN   13

/*****************************************************************************
 * Macros for manipulating the 'flags' element where the less significant
 * bits are GTZXLCB, where
 *   G: coordinates are geodetic
 *   T: has T coordinate,
 *   Z: has Z coordinate
 *   X: has value or X coordinate
 *   L: linear interpolation
 *   C: continuous base type
 *   B: base type passed by value
 * Notice that formally speaking the Linear interpolation flag is only needed
 * for sequence and sequence set subtypes. To facilate the transformation from
 * one subtype to another, the linear flag for instant and instant set is set
 * to the value of the continuous subtype flag.
 *****************************************************************************/

#define MOBDB_FLAG_BYVAL      0x0001
#define MOBDB_FLAG_CONTINUOUS 0x0002
#define MOBDB_FLAG_LINEAR     0x0004
#define MOBDB_FLAG_X          0x0008
#define MOBDB_FLAG_Z          0x0010
#define MOBDB_FLAG_T          0x0020
#define MOBDB_FLAG_GEODETIC   0x0040

/* The following flag is only used for TInstant */
#define MOBDB_FLAGS_GET_BYVAL(flags)      ((bool) (((flags) & MOBDB_FLAG_BYVAL)))
#define MOBDB_FLAGS_GET_CONTINUOUS(flags) ((bool) (((flags) & MOBDB_FLAG_CONTINUOUS)>>1))
#define MOBDB_FLAGS_GET_LINEAR(flags)     ((bool) (((flags) & MOBDB_FLAG_LINEAR)>>2))
#define MOBDB_FLAGS_GET_X(flags)          ((bool) (((flags) & MOBDB_FLAG_X)>>3))
#define MOBDB_FLAGS_GET_Z(flags)          ((bool) (((flags) & MOBDB_FLAG_Z)>>4))
#define MOBDB_FLAGS_GET_T(flags)          ((bool) (((flags) & MOBDB_FLAG_T)>>5))
#define MOBDB_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & MOBDB_FLAG_GEODETIC)>>6))

/* The following flag is only used for TInstant */
#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_BYVAL) : ((flags) & ~MOBDB_FLAG_BYVAL))
#define MOBDB_FLAGS_SET_CONTINUOUS(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_CONTINUOUS) : ((flags) & ~MOBDB_FLAG_CONTINUOUS))
#define MOBDB_FLAGS_SET_LINEAR(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_LINEAR) : ((flags) & ~MOBDB_FLAG_LINEAR))
#define MOBDB_FLAGS_SET_X(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_X) : ((flags) & ~MOBDB_FLAG_X))
#define MOBDB_FLAGS_SET_Z(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_Z) : ((flags) & ~MOBDB_FLAG_Z))
#define MOBDB_FLAGS_SET_T(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_T) : ((flags) & ~MOBDB_FLAG_T))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_GEODETIC) : ((flags) & ~MOBDB_FLAG_GEODETIC))

/*****************************************************************************
 * Definitions for bucketing and tiling
 *****************************************************************************/

/*
 * The default origin is Monday 2000-01-03. We don't use PG epoch since it
 * starts on a saturday. This makes time-buckets by a week more intuitive and
 * aligns it with date_trunc.
 */
#define JAN_3_2000 (2 * USECS_PER_DAY)
#define DEFAULT_TIME_ORIGIN (JAN_3_2000)
#define DEFAULT_FLOATRANGE_ORIGIN (0.0)
#define DEFAULT_INTRANGE_ORIGIN (0)

/*****************************************************************************
 * Definitions for GiST indexes
 *****************************************************************************/

/* Minimum accepted ratio of split */
#define LIMIT_RATIO 0.3

/* Convenience macros for NaN-aware comparisons */
#define FLOAT8_EQ(a,b)  (float8_cmp_internal(a, b) == 0)
#define FLOAT8_LT(a,b)  (float8_cmp_internal(a, b) < 0)
#define FLOAT8_LE(a,b)  (float8_cmp_internal(a, b) <= 0)
#define FLOAT8_GT(a,b)  (float8_cmp_internal(a, b) > 0)
#define FLOAT8_GE(a,b)  (float8_cmp_internal(a, b) >= 0)
#define FLOAT8_MAX(a,b)  (FLOAT8_GT(a, b) ? (a) : (b))
#define FLOAT8_MIN(a,b)  (FLOAT8_LT(a, b) ? (a) : (b))

/*****************************************************************************
 * Additional operator strategy numbers used in the GiST and SP-GiST temporal
 * opclasses with respect to those defined in the file stratnum.h
 *****************************************************************************/

#define RTOverBeforeStrategyNumber    28    /* for &<# */
#define RTBeforeStrategyNumber        29    /* for <<# */
#define RTAfterStrategyNumber         30    /* for #>> */
#define RTOverAfterStrategyNumber     31    /* for #&> */
#define RTOverFrontStrategyNumber     32    /* for &</ */
#define RTFrontStrategyNumber         33    /* for <</ */
#define RTBackStrategyNumber          34    /* for />> */
#define RTOverBackStrategyNumber      35    /* for /&> */

/*****************************************************************************
 * Struct definitions for temporal types
 *****************************************************************************/

/**
 * Structure to represent the common structure of temporal values of
 * any temporal subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  uint8         temptype;     /**< temporal type */
  uint8         subtype;      /**< temporal subtype */
  int16         flags;        /**< flags */
  /* variable-length data follows, if any */
} Temporal;

/**
 * Structure to represent temporal values of instant subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  uint8         temptype;     /**< temporal type */
  uint8         subtype;      /**< temporal subtype */
  int16         flags;        /**< flags */
  TimestampTz   t;            /**< timestamp (8 bytes) */
  /* variable-length data follows */
} TInstant;

/**
 * Structure to represent temporal values of instant set subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  uint8         temptype;     /**< temporal type */
  uint8         subtype;      /**< temporal subtype */
  int16         flags;        /**< flags */
  int32         count;        /**< number of TInstant elements */
  int16         bboxsize;     /**< size of the bounding box */
  /**< beginning of variable-length data */
} TInstantSet;

/**
 * Structure to represent temporal values of sequence subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  uint8         temptype;     /**< temporal type */
  uint8         subtype;      /**< temporal subtype */
  int16         flags;        /**< flags */
  int32         count;        /**< number of TInstant elements */
  int16         bboxsize;     /**< size of the bounding box */
  Period        period;       /**< time span (24 bytes) */
  /**< beginning of variable-length data */
} TSequence;

/**
 * Structure to represent temporal values of sequence set subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  uint8         temptype;     /**< temporal type */
  uint8         subtype;      /**< temporal subtype */
  int16         flags;        /**< flags */
  int32         count;        /**< number of TSequence elements */
  int32         totalcount;   /**< total number of TInstant elements in all TSequence elements */
  int16         bboxsize;     /**< size of the bounding box */
  /**< beginning of variable-length data */
} TSequenceSet;

/**
 * Structure to represent all types of bounding boxes
 */
typedef union bboxunion
{
  Period    p;
  TBOX      b;
  STBOX     g;
} bboxunion;

/**
 * Structure to represent values of the internal type for computing aggregates
 * for temporal number types
 */
typedef struct
{
  double    a;
  double    b;
} double2;

/**
 * Structure to represent values of the internal type for computing aggregates
 * for 2D temporal point types
 */
typedef struct
{
  double    a;
  double    b;
  double    c;
} double3;

/**
 * Structure to represent values of the internal type for computing aggregates
 * for 3D temporal point types
 */
typedef struct
{
  double    a;
  double    b;
  double    c;
  double    d;
} double4;

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

/* Definition of qsort comparator for integers */
typedef int (*qsort_comparator) (const void *a, const void *b);

/* Definition of a variadic function type for temporal lifting */
typedef Datum (*varfunc) (Datum, ...);

/* Definition of a binary function with two or three Datum arguments */
typedef Datum (*datum_func2) (Datum, Datum);
typedef Datum (*datum_func3) (Datum, Datum, Datum);

/*****************************************************************************
 * Struct definitions for GisT indexes copied from PostgreSQL
 *****************************************************************************/

/**
 * Structure to represent information about an entry that can be placed
 * to either group without affecting overlap over selected axis ("common entry").
 */
typedef struct
{
  /* Index of entry in the initial array */
  int      index;
  /* Delta between penalties of entry insertion into different groups */
  double    delta;
} CommonEntry;

/**
 * Structure to represent a projection of bounding box to an axis.
 */
typedef struct
{
  double    lower,
            upper;
} SplitInterval;

/*****************************************************************************
 * fmgr macros temporal types
 *****************************************************************************/

/* doubleN */

#define DatumGetDouble2P(X)       ((double2 *) DatumGetPointer(X))
#define Double2PGetDatum(X)       PointerGetDatum(X)
#define DatumGetDouble3P(X)       ((double3 *) DatumGetPointer(X))
#define Double3PGetDatum(X)       PointerGetDatum(X)
#define DatumGetDouble4P(X)       ((double4 *) DatumGetPointer(X))
#define Double4PGetDatum(X)       PointerGetDatum(X)

/* Temporal types */

#define DatumGetTemporalP(X)       ((Temporal *) PG_DETOAST_DATUM(X))
#define DatumGetTInstantP(X)       ((TInstant *) PG_DETOAST_DATUM(X))
#define DatumGetTInstantSetP(X)    ((TInstantSet *) PG_DETOAST_DATUM(X))
#define DatumGetTSequenceP(X)      ((TSequence *) PG_DETOAST_DATUM(X))
#define DatumGetTSequenceSetP(X)   ((TSequenceSet *) PG_DETOAST_DATUM(X))

#define PG_GETARG_TEMPORAL_P(X)    ((Temporal *) PG_GETARG_VARLENA_P(X))

#define PG_GETARG_ANYDATUM(X) (get_typlen(get_fn_expr_argtype(fcinfo->flinfo, X)) == -1 ? \
  PointerGetDatum(PG_GETARG_VARLENA_P(X)) : PG_GETARG_DATUM(X))

#define DATUM_FREE(value, basetype) \
  do { \
    if (! basetype_byvalue(basetype)) \
      pfree(DatumGetPointer(value)); \
  } while (0)

#define DATUM_FREE_IF_COPY(value, basetype, n) \
  do { \
    if (! basetype_byvalue(basetype) && DatumGetPointer(value) != PG_GETARG_POINTER(n)) \
      pfree(DatumGetPointer(value)); \
  } while (0)

/*
 * This macro is based on PG_FREE_IF_COPY, except that it accepts two pointers.
 * See PG_FREE_IF_COPY comment in src/include/fmgr.h in postgres source code
 * for more details.
 * This macro is the same as POSTGIS_FREE_IF_COPY_P.
 */
#define PG_FREE_IF_COPY_P(ptrsrc, ptrori) \
  do { \
    if ((Pointer) (ptrsrc) != (Pointer) (ptrori)) \
      pfree(ptrsrc); \
  } while (0)

#define PG_DATUM_NEEDS_DETOAST(datum) \
  (VARATT_IS_EXTENDED((datum)) || VARATT_IS_EXTERNAL((datum)) || \
   VARATT_IS_COMPRESSED((datum)))

/*****************************************************************************/

/* Initialization function */

extern void _PG_init(void);

/* Typmod functions */

extern const char *tempsubtype_name(int16 subtype);
extern bool tempsubtype_from_string(const char *str, int16 *subtype);

/* Parameter tests */

extern void ensure_valid_tempsubtype(int16 type);
extern void ensure_valid_tempsubtype_all(int16 type);
extern void ensure_seq_subtypes(int16 subtype);
extern void ensure_tinstarr(const TInstant **instants, int count);
extern int *tsequenceset_make_valid_gaps(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, bool linear, double maxdist, Interval *maxt,
  int *countsplits);
extern void ensure_linear_interpolation(int16 flags);
extern void ensure_common_dimension(int16 flags1, int16 flags2);
extern void ensure_same_temptype(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_same_interpolation(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_increasing_timestamps(const TInstant *inst1,
  const TInstant *inst2, bool strict);
extern void ensure_valid_tinstarr(const TInstant **instants, int count,
  bool merge, int16 subtype);
extern int *ensure_valid_tinstarr_gaps(const TInstant **instants, int count,
  bool merge, int16 subtype, double maxdist, Interval *maxt, int *countsplits);
extern void ensure_valid_tseqarr(const TSequence **sequences, int count);

extern void ensure_positive_datum(Datum size, CachedType basetype);
extern void ensure_valid_duration(const Interval *duration);
extern void ensure_non_empty_array(ArrayType *array);

/* General functions */

extern void *temporal_bbox_ptr(const Temporal *temp);
extern void temporal_bbox(const Temporal *temp, void *box);
extern void temporal_bbox_slice(Datum tempdatum, void *box);
extern Temporal *temporal_copy(const Temporal *temp);
extern bool intersection_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, SyncMode mode, Temporal **inter1, Temporal **inter2);
extern const TInstant *tinstarr_inst_n(const Temporal *temp, int n);

/* Version functions */


/* Input/output functions */

extern char *temporal_to_string(const Temporal *temp,
  char *(*value_out)(Oid, Datum));
extern void temporal_write(const Temporal* temp, StringInfo buf);
extern Temporal* temporal_read(StringInfo buf, CachedType temptype);

/* Constructor functions */

extern Temporal *temporal_from_base(const Temporal *temp, Datum value,
  CachedType basetype, bool linear);

/* Append and merge functions */

extern Temporal *temporal_append_tinstant(const Temporal *temp,
  const Temporal *inst);
extern Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2);
extern Temporal *temporal_merge_array(Temporal **temparr, int count);

/* Cast functions */

extern RangeType *tint_range(const Temporal *temp);
extern RangeType *tfloat_range(const Temporal *temp);
extern Temporal *tint_tfloat(const Temporal *temp);
extern Temporal *tfloat_tint(const Temporal *temp);
extern void temporal_period(const Temporal *temp, Period *p);
extern TBOX *tnumber_to_tbox(Temporal *temp);

/* Transformation functions */

extern Temporal *temporal_tinstant(const Temporal *temp);
extern Temporal *temporal_tinstantset(const Temporal *temp);
extern Temporal *temporal_tsequence(const Temporal *temp);
extern Temporal *temporal_tsequenceset(const Temporal *temp);
extern Temporal *tempstep_templinear(const Temporal *temp);
extern Temporal *temporal_shift_tscale(const Temporal *temp, bool shift,
  bool tscale, Interval *start, Interval *duration);

/* Accessor functions */

extern char *temporal_subtype(const Temporal *temp);
extern char *temporal_interpolation(const Temporal *temp);
extern Datum *temporal_values(const Temporal *temp, int *count);
extern RangeType **tfloat_ranges(const Temporal *temp, int *count);
extern PeriodSet *temporal_time(const Temporal *temp);
extern RangeType *tnumber_range(const Temporal *temp);
extern Datum temporal_start_value(Temporal *temp);
extern Datum temporal_end_value(Temporal *temp);
extern const TInstant *temporal_min_instant(const Temporal *temp);
extern const TInstant *temporal_max_instant(const Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern Datum temporal_max_value(const Temporal *temp);
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
extern const TInstant **temporal_instants(const Temporal *temp,
  int *count);
extern int temporal_num_timestamps(const Temporal *temp);
extern TimestampTz temporal_start_timestamp(const Temporal *temp);
extern TimestampTz temporal_end_timestamp(Temporal *temp);
extern bool temporal_timestamp_n(Temporal *temp, int n, TimestampTz *result);
extern TimestampTz *temporal_timestamps(const Temporal *temp, int *count);

/* Ever/always equal operators */

extern bool temporal_bbox_ev_al_eq(const Temporal *temp, Datum value,
  bool ever);
extern bool temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value,
  bool ever);
extern bool temporal_ever_eq(const Temporal *temp, Datum value);
extern bool temporal_always_eq(const Temporal *temp, Datum value);
extern bool temporal_ever_lt(const Temporal *temp, Datum value);
extern bool temporal_always_lt(const Temporal *temp, Datum value);
extern bool temporal_ever_le(const Temporal *temp, Datum value);
extern bool temporal_always_le(const Temporal *temp, Datum value);

/* Restriction functions */

extern bool temporal_bbox_restrict_value(const Temporal *temp, Datum value);
extern Datum *temporal_bbox_restrict_values(const Temporal *temp,
  const Datum *values, int count, int *newcount);
extern RangeType **tnumber_bbox_restrict_ranges(const Temporal *temp,
  RangeType **ranges, int count, int *newcount);
extern Temporal *temporal_restrict_minmax(const Temporal *temp, bool min,
  bool atfunc);

extern Temporal *temporal_restrict_value(const Temporal *temp,
  Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, Datum *values,
  int count, bool atfunc);
extern Temporal *tnumber_restrict_range(const Temporal *temp,
 RangeType *range, bool atfunc);
extern Temporal *tnumber_restrict_ranges(const Temporal *temp,
  RangeType **ranges, int count, bool atfunc);
extern bool temporal_value_at_timestamp_inc(const Temporal *temp,
  TimestampTz t, Datum *value);
extern bool temporal_value_at_timestamp(const Temporal *temp, TimestampTz t,
  Datum *result);

extern Temporal *temporal_restrict_timestamp(const Temporal *temp,
  TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_timestampset(const Temporal *temp,
  const TimestampSet *ts, bool atfunc);
extern Temporal *temporal_restrict_period(const Temporal *temp,
  const Period *ps, bool atfunc);
extern Temporal *temporal_restrict_periodset(const Temporal *temp,
  const PeriodSet *ps, bool atfunc);
extern Temporal *tnumber_at_tbox(const Temporal *temp, const TBOX *box);
extern Temporal *tnumber_minus_tbox(const Temporal *temp, const TBOX *box);

/* Intersects functions */

extern bool temporal_intersects_timestamp(const Temporal *temp, TimestampTz t);
extern bool temporal_intersects_timestampset(const Temporal *temp,
  const TimestampSet *ts);
extern bool temporal_intersects_period(const Temporal *temp, const Period *p);
extern bool temporal_intersects_periodset(const Temporal *temp,
  const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumber_integral(const Temporal *temp);
extern double tnumber_twavg(const Temporal *temp);

/* Comparison functions */

extern bool temporal_eq(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ne(const Temporal *temp1, const Temporal *temp2);
extern int temporal_cmp(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_lt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_le(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_gt(const Temporal *temp1, const Temporal *temp2);
extern bool temporal_ge(const Temporal *temp1, const Temporal *temp2);

/* Functions for defining hash index */

extern uint32 temporal_hash(const Temporal *temp);

/*****************************************************************************/

#endif
