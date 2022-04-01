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

#include <postgres.h>
#include <catalog/pg_type.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "timetypes.h"
#include "tbox.h"
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
 * bits are GTZXLCBsss, where
 *   G: coordinates are geodetic
 *   T: has T coordinate,
 *   Z: has Z coordinate
 *   X: has value or X coordinate
 *   L: linear interpolation
 *   C: continuous base type
 *   B: base type passed by value
 *   sss: 3 bits for the temporal subtype (values 0 to 4)
 * Notice that formally speaking the Linear interpolation flag is only needed
 * for sequence and sequence set subtypes. To facilate the transformation from
 * one subtype to another, the linear flag for instant and instant set is set
 * to the value of the continuous subtype flag.
 *****************************************************************************/

#define MOBDB_FLAG_BYVAL      0x0008
#define MOBDB_FLAG_CONTINUOUS 0x0010
#define MOBDB_FLAG_LINEAR     0x0020
#define MOBDB_FLAG_X          0x0040
#define MOBDB_FLAG_Z          0x0080
#define MOBDB_FLAG_T          0x0100
#define MOBDB_FLAG_GEODETIC   0x0200

#define MOBDB_FLAGS_GET_SUBTYPE(flags) ((int16) ((flags & 0x0007)))
/* The following flag is only used for TInstant */
#define MOBDB_FLAGS_GET_BYVAL(flags)      ((bool) (((flags) & MOBDB_FLAG_BYVAL)>>3))
#define MOBDB_FLAGS_GET_CONTINUOUS(flags) ((bool) (((flags) & MOBDB_FLAG_CONTINUOUS)>>4))
#define MOBDB_FLAGS_GET_LINEAR(flags)     ((bool) (((flags) & MOBDB_FLAG_LINEAR)>>5))
#define MOBDB_FLAGS_GET_X(flags)          ((bool) (((flags) & MOBDB_FLAG_X)>>6))
#define MOBDB_FLAGS_GET_Z(flags)          ((bool) (((flags) & MOBDB_FLAG_Z)>>7))
#define MOBDB_FLAGS_GET_T(flags)          ((bool) (((flags) & MOBDB_FLAG_T)>>8))
#define MOBDB_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & MOBDB_FLAG_GEODETIC)>>9))

#define MOBDB_FLAGS_SET_SUBTYPE(flags, value) \
  ((flags) = (((flags) & 0xFFF8) | ((value & 0x0007))))
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
  int16         temptype;     /**< temporal type */
  int16         flags;        /**< flags */
  Oid           basetypid;    /**< base type's OID (4 bytes) */
  /* variable-length data follows, if any */
} Temporal;

/**
 * Structure to represent temporal values of instant subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  int16         temptype;     /**< temporal type */
  int16         flags;        /**< flags */
  Oid           basetypid;    /**< base type's OID (4 bytes) */
  TimestampTz   t;            /**< timestamp (8 bytes) */
  /* variable-length data follows */
} TInstant;

/**
 * Structure to represent temporal values of instant set subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  int16         temptype;     /**< temporal type */
  int16         flags;        /**< flags */
  Oid           basetypid;    /**< base type's OID (4 bytes) */
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
  int16         temptype;     /**< temporal type */
  int16         flags;        /**< flags */
  Oid           basetypid;    /**< base type's OID (4 bytes) */
  int32         count;        /**< number of TInstant elements */
  Period        period;       /**< time span (24 bytes) */
  int16         bboxsize;     /**< size of the bounding box */
  /**< beginning of variable-length data */
} TSequence;

/**
 * Structure to represent temporal values of sequence set subtype
 */
typedef struct
{
  int32         vl_len_;      /**< varlena header (do not touch directly!) */
  int16         temptype;     /**< temporal type */
  int16         flags;        /**< flags */
  Oid           basetypid;    /**< base type's OID (4 bytes) */
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

#define DATUM_FREE(value, basetypid) \
  do { \
    if (! base_type_byvalue(basetypid)) \
      pfree(DatumGetPointer(value)); \
  } while (0)

#define DATUM_FREE_IF_COPY(value, basetypid, n) \
  do { \
    if (! base_type_byvalue(basetypid) && DatumGetPointer(value) != PG_GETARG_POINTER(n)) \
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

extern Datum temporal_typmod_in(PG_FUNCTION_ARGS);
extern Datum temporal_typmod_out(PG_FUNCTION_ARGS);
extern Datum temporal_enforce_typmod(PG_FUNCTION_ARGS);

extern const char *tempsubtype_name(int16 subtype);
extern bool tempsubtype_from_string(const char *str, int16 *subtype);

/* Parameter tests */

extern void ensure_valid_tempsubtype(int16 type);
extern void ensure_valid_tempsubtype_all(int16 type);
extern void ensure_seq_subtypes(int16 subtype);
extern void ensure_linear_interpolation(int16 flags);
extern void ensure_common_dimension(int16 flags1, int16 flags2);
extern void ensure_same_base_type(const Temporal *temp1,
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

extern void ensure_positive_datum(Datum size, Oid type);
extern void ensure_valid_duration(const Interval *duration);
extern void ensure_non_empty_array(ArrayType *array);

/* General functions */

extern void *temporal_bbox_ptr(const Temporal *temp);
extern void temporal_bbox(const Temporal *temp, void *box);
extern void temporal_bbox_slice(Datum tempdatum, void *box);
extern Temporal *temporal_copy(const Temporal *temp);
extern bool intersection_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, SyncMode mode, Temporal **inter1, Temporal **inter2);

/* Version functions */

extern Datum mobilitydb_version(PG_FUNCTION_ARGS);
extern Datum mobilitydb_full_version(PG_FUNCTION_ARGS);

/* Input/output functions */

extern Datum temporal_in(PG_FUNCTION_ARGS);
extern Datum temporal_out(PG_FUNCTION_ARGS);
extern Datum temporal_send(PG_FUNCTION_ARGS);
extern Datum temporal_recv(PG_FUNCTION_ARGS);

extern Temporal* temporal_read(StringInfo buf, Oid basetypid);
extern void temporal_write(const Temporal* temp, StringInfo buf);

/* Constructor functions */

extern Datum tinstant_constructor(PG_FUNCTION_ARGS);
extern Datum tinstantset_constructor(PG_FUNCTION_ARGS);
extern Datum tlinearseq_constructor(PG_FUNCTION_ARGS);
extern Datum tstepseq_constructor(PG_FUNCTION_ARGS);
extern Datum tsequenceset_constructor(PG_FUNCTION_ARGS);
extern Datum tstepseqset_constructor_gaps(PG_FUNCTION_ARGS);
extern Datum tlinearseqset_constructor_gaps(PG_FUNCTION_ARGS);

extern Temporal *temporal_from_base(const Temporal *temp, Datum value,
  Oid basetypid, bool linear);

/* Append and merge functions */

extern Datum temporal_append_tinstant(PG_FUNCTION_ARGS);
extern Datum temporal_merge(PG_FUNCTION_ARGS);
extern Datum temporal_merge_array(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tint_to_range(PG_FUNCTION_ARGS);
extern Datum tfloat_to_range(PG_FUNCTION_ARGS);
extern Datum tint_to_tfloat(PG_FUNCTION_ARGS);
extern Datum tfloat_to_tint(PG_FUNCTION_ARGS);
extern Datum temporal_to_period(PG_FUNCTION_ARGS);

extern void temporal_period(const Temporal *temp, Period *p);

/* Transformation functions */

extern Datum temporal_to_tinstant(PG_FUNCTION_ARGS);
extern Datum temporal_to_tinstantset(PG_FUNCTION_ARGS);
extern Datum temporal_to_tsequence(PG_FUNCTION_ARGS);
extern Datum temporal_to_tsequenceset(PG_FUNCTION_ARGS);
extern Datum tstep_to_linear(PG_FUNCTION_ARGS);
extern Datum temporal_shift(PG_FUNCTION_ARGS);
extern Datum temporal_tscale(PG_FUNCTION_ARGS);
extern Datum temporal_shift_tscale(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum temporal_subtype(PG_FUNCTION_ARGS);
extern Datum temporal_interpolation(PG_FUNCTION_ARGS);
extern Datum temporal_mem_size(PG_FUNCTION_ARGS);
extern Datum temporal_get_values(PG_FUNCTION_ARGS);
extern Datum tfloat_get_ranges(PG_FUNCTION_ARGS);
extern Datum tinstant_get_value(PG_FUNCTION_ARGS);
extern Datum temporal_get_time(PG_FUNCTION_ARGS);
extern Datum tinstant_timestamp(PG_FUNCTION_ARGS);
extern Datum tnumber_value_range(PG_FUNCTION_ARGS);
extern Datum temporal_start_value(PG_FUNCTION_ARGS);
extern Datum temporal_end_value(PG_FUNCTION_ARGS);
extern Datum temporal_min_value(PG_FUNCTION_ARGS);
extern Datum temporal_max_value(PG_FUNCTION_ARGS);
extern Datum temporal_timespan(PG_FUNCTION_ARGS);
extern Datum temporal_duration(PG_FUNCTION_ARGS);
extern Datum temporal_num_sequences(PG_FUNCTION_ARGS);
extern Datum temporal_start_sequence(PG_FUNCTION_ARGS);
extern Datum temporal_end_sequence(PG_FUNCTION_ARGS);
extern Datum temporal_sequence_n(PG_FUNCTION_ARGS);
extern Datum temporal_sequences(PG_FUNCTION_ARGS);
extern Datum temporal_segments(PG_FUNCTION_ARGS);
extern Datum temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum temporal_instants(PG_FUNCTION_ARGS);
extern Datum temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum temporal_timestamps(PG_FUNCTION_ARGS);

extern PeriodSet *temporal_get_time_internal(const Temporal *temp);
extern RangeType *tnumber_value_range_internal(const Temporal *temp);
extern TInstant *temporal_start_instant_internal(const Temporal *temp);
extern const TInstant *temporal_min_instant(const Temporal *temp);
extern Datum temporal_min_value_internal(const Temporal *temp);
extern const TInstant *tinstarr_inst_n(const Temporal *temp, int n);
extern const TInstant **temporal_instants_internal(const Temporal *temp,
  int *count);

/* Ever/always equal operators */

extern Datum temporal_ever_eq(PG_FUNCTION_ARGS);
extern Datum temporal_always_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ne(PG_FUNCTION_ARGS);
extern Datum temporal_always_ne(PG_FUNCTION_ARGS);

extern Datum temporal_ever_lt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_le(PG_FUNCTION_ARGS);
extern Datum temporal_ever_gt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ge(PG_FUNCTION_ARGS);
extern Datum temporal_always_lt(PG_FUNCTION_ARGS);
extern Datum temporal_always_le(PG_FUNCTION_ARGS);
extern Datum temporal_always_gt(PG_FUNCTION_ARGS);
extern Datum temporal_always_ge(PG_FUNCTION_ARGS);

extern bool temporal_bbox_ev_al_eq(const Temporal *temp, Datum value,
  bool ever);
extern bool temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value,
  bool ever);
extern bool temporal_ever_eq_internal(const Temporal *temp, Datum value);

/* Restriction functions */

extern Datum temporal_at_value(PG_FUNCTION_ARGS);
extern Datum temporal_minus_value(PG_FUNCTION_ARGS);
extern Datum temporal_at_values(PG_FUNCTION_ARGS);
extern Datum temporal_minus_values(PG_FUNCTION_ARGS);
extern Datum tnumber_at_range(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_range(PG_FUNCTION_ARGS);
extern Datum tnumber_at_ranges(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_ranges(PG_FUNCTION_ARGS);
extern Datum temporal_at_min(PG_FUNCTION_ARGS);
extern Datum temporal_minus_min(PG_FUNCTION_ARGS);
extern Datum temporal_at_max(PG_FUNCTION_ARGS);
extern Datum temporal_minus_max(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_value_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_at_period(PG_FUNCTION_ARGS);
extern Datum temporal_minus_period(PG_FUNCTION_ARGS);
extern Datum temporal_at_periodset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_periodset(PG_FUNCTION_ARGS);
extern Datum tnumber_at_tbox(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_tbox(PG_FUNCTION_ARGS);

extern bool temporal_bbox_restrict_value(const Temporal *temp, Datum value);
extern Datum *temporal_bbox_restrict_values(const Temporal *temp,
  const Datum *values, int count, int *newcount);
extern RangeType **tnumber_bbox_restrict_ranges(const Temporal *temp,
  RangeType **ranges, int count, int *newcount);

extern Temporal *temporal_restrict_value_internal(const Temporal *temp,
  Datum value, bool atfunc);
extern Temporal *tnumber_restrict_range_internal(const Temporal *temp,
 RangeType *range, bool atfunc);
extern Temporal *temporal_restrict_timestamp_internal(const Temporal *temp,
  TimestampTz t, bool atfunc);
extern bool temporal_value_at_timestamp_inc(const Temporal *temp,
  TimestampTz t, Datum *value);
extern Temporal *temporal_restrict_period_internal(const Temporal *temp,
  const Period *ps, bool atfunc);
extern Temporal *temporal_restrict_periodset_internal(const Temporal *temp,
  const PeriodSet *ps, bool atfunc);
extern Temporal *tnumber_at_tbox_internal(const Temporal *temp, const TBOX *box);
extern Temporal *tnumber_minus_tbox_internal(const Temporal *temp, const TBOX *box);

/* Intersects functions */

extern Datum temporal_intersects_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_period(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_periodset(PG_FUNCTION_ARGS);

/* Local aggregate functions */

extern Datum tnumber_integral(PG_FUNCTION_ARGS);
extern Datum tnumber_twavg(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum temporal_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ne(PG_FUNCTION_ARGS);

extern Datum temporal_cmp(PG_FUNCTION_ARGS);
extern Datum temporal_lt(PG_FUNCTION_ARGS);
extern Datum temporal_le(PG_FUNCTION_ARGS);
extern Datum temporal_ge(PG_FUNCTION_ARGS);
extern Datum temporal_gt(PG_FUNCTION_ARGS);

/* Functions for defining hash index */

extern Datum temporal_hash(PG_FUNCTION_ARGS);

extern uint32 temporal_hash_internal(const Temporal *temp);

/*****************************************************************************/

#endif
