/*****************************************************************************
 *
 * temporal.h
 *  Basic functions for temporal types of any duration.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "timetypes.h"
#include "tbox.h"
#include "stbox.h"

#if MOBDB_PGSQL_VERSION < 130000
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
#define EPSILON         1.0E-05
/**
 * Precision for distance operations
 */
#define DIST_EPSILON    1.0E-05

/** Symbolic constants for lifting */
#define DISCONTINUOUS   true
#define CONTINUOUS      false

/** Symbolic constants for lifting */
#define INVERT          true
#define INVERT_NO       false

/** Symbolic constants for the restriction functions */
#define REST_AT         true
#define REST_MINUS      false

/** Symbolic constants for the ever/always functions */
#define EVER            true
#define ALWAYS          false

/** Symbolic constants for the restriction and the aggregation functions */
#define MIN             true
#define MAX             false

/** Symbolic constants for the synchronization and the aggregation functions */
#define CROSSINGS       true
#define CROSSINGS_NO    false

/** Symbolic constants for the synchronization and the aggregation functions */
#define BBOX_TEST       true
#define BBOX_TEST_NO    false

/** Symbolic constants for the make functions */
#define NORMALIZE       true
#define NORMALIZE_NO    false

#define LINEAR          true
#define STEP            false

/** Symbolic constants for spatial relationships */
#define WITH_Z          true
#define NO_Z            false

/** Enumeration for the intersection/synchronization functions */

typedef enum
{
  INTERSECT,
  SYNCHRONIZE,
  SYNCHRONIZE_CROSS,
} TIntersection;

/** Enumeration for the arithmetic functions */

typedef enum
{
  ADD,
  SUB,
  MULT,
  DIV,
} TArithmetic;

/*****************************************************************************
 * Compatibility with older versions of PostgreSQL
 *****************************************************************************/

#define MOBDB_VERSION_STR "MobilityDB 1.0"
#ifndef MOBDB_PGSQL_VERSION
#error Unconfigured source code -- please run cmake
#endif
#ifndef MOBDB_PGSQL_VERSION_STR
#error Unconfigured source code -- please run cmake
#endif

#define MOBDB_POSTGIS_VERSION 25
#define MOBDB_POSTGIS_VERSION_STR "PostGIS 2.5"

/*****************************************************************************
 * Duration of temporal types
 *****************************************************************************/

/**
 * Enumeration for the duration of temporal types
 */
typedef enum
{
  ANYDURATION,
  INSTANT,
  INSTANTSET,
  SEQUENCE,
  SEQUENCESET,
} TDuration;

#define TYPMOD_GET_DURATION(typmod) ((TDuration) ((typmod == -1) ? (0) : (typmod & 0x0000000F)))

/**
 * Structure to represent the duration array
 */
struct tduration_struct
{
  char *durationName;    /**< string representing the duration */
  TDuration duration;    /**< duration */
};

#define TDURATION_STRUCT_ARRAY_LEN \
  (sizeof tduration_struct_array/sizeof(struct tduration_struct))
#define TDURATION_MAX_LEN   13

/*****************************************************************************
 * Macros for manipulating the 'flags' element
 * GTZXBL
 *****************************************************************************/

#define MOBDB_FLAGS_GET_LINEAR(flags)     ((bool) ((flags) & 0x01))
/* The following flag is only used for TInstant */
#define MOBDB_FLAGS_GET_BYVAL(flags)     ((bool) (((flags) & 0x02)>>1))
#define MOBDB_FLAGS_GET_X(flags)      ((bool) (((flags) & 0x04)>>2))
#define MOBDB_FLAGS_GET_Z(flags)       ((bool) (((flags) & 0x08)>>3))
#define MOBDB_FLAGS_GET_T(flags)       ((bool) (((flags) & 0x10)>>4))
#define MOBDB_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & 0x20)>>5))

#define MOBDB_FLAGS_SET_LINEAR(flags, value) \
  ((flags) = (value) ? ((flags) | 0x01) : ((flags) & 0xFE))
/* The following flag is only used for TInstant */
#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
  ((flags) = (value) ? ((flags) | 0x02) : ((flags) & 0xFD))
#define MOBDB_FLAGS_SET_X(flags, value) \
  ((flags) = (value) ? ((flags) | 0x04) : ((flags) & 0xFB))
#define MOBDB_FLAGS_SET_Z(flags, value) \
  ((flags) = (value) ? ((flags) | 0x08) : ((flags) & 0xF7))
#define MOBDB_FLAGS_SET_T(flags, value) \
  ((flags) = (value) ? ((flags) | 0x10) : ((flags) & 0xEF))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
  ((flags) = (value) ? ((flags) | 0x20) : ((flags) & 0xDF))

/*****************************************************************************
 * Macros for GiST indexes
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
 * any duration
 */
typedef struct
{
  int32    vl_len_;        /**< varlena header (do not touch directly!) */
  TDuration   duration;    /**< duration */
  int16    flags;          /**< flags */
  Oid     valuetypid;      /**< base type's OID (4 bytes) */
  /* variable-length data follows, if any */
} Temporal;

/**
 * Structure to represent temporal values of instant duration
 */
typedef struct
{
  int32    vl_len_;        /**< varlena header (do not touch directly!) */
  TDuration   duration;    /**< duration */
  int16    flags;          /**< flags */
  Oid     valuetypid;      /**< base type's OID (4 bytes) */
  TimestampTz t;           /**< timestamp (8 bytes) */
  /* variable-length data follows */
} TInstant;

/**
 * Structure to represent temporal values of instant set duration
 */
typedef struct
{
  int32    vl_len_;        /**< varlena header (do not touch directly!) */
  TDuration   duration;    /**< duration */
  int16    flags;          /**< flags */
  Oid     valuetypid;      /**< base type's OID (4 bytes) */
  int32     count;         /**< number of TInstant elements */
  size_t    offsets[1];    /**< beginning of variable-length data */
} TInstantSet;

/**
 * Structure to represent temporal values of sequence duration
 */
typedef struct
{
  int32    vl_len_;        /**< varlena header (do not touch directly!) */
  TDuration   duration;    /**< duration */
  int16    flags;          /**< flags */
  Oid     valuetypid;      /**< base type's OID (4 bytes) */
  int32     count;         /**< number of TInstant elements */
  Period     period;       /**< time span (24 bytes) */
  size_t    offsets[1];    /**< beginning of variable-length data */
} TSequence;

/**
 * Structure to represent temporal values of sequence set duration
 */
typedef struct
{
  int32       vl_len_;        /**< varlena header (do not touch directly!) */
  TDuration   duration;       /**< duration */
  int16       flags;          /**< flags */
  Oid         valuetypid;     /**< base type's OID (4 bytes) */
  int32       count;          /**< number of TSequence elements */
  int32       totalcount;     /**< total number of TInstant elements in all TSequence elements */
  size_t      offsets[1];     /**< beginning of variable-length data */
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
typedef Datum (*varfunc)  (Datum, ...);

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

#define DatumGetDouble2P(X)    ((double2 *) DatumGetPointer(X))
#define Double2PGetDatum(X)    PointerGetDatum(X)
#define DatumGetDouble3P(X)    ((double3 *) DatumGetPointer(X))
#define Double3PGetDatum(X)    PointerGetDatum(X)
#define DatumGetDouble4P(X)    ((double4 *) DatumGetPointer(X))
#define Double4PGetDatum(X)    PointerGetDatum(X)

/* Temporal types */

#define DatumGetTemporal(X)      ((Temporal *) PG_DETOAST_DATUM(X))
#define DatumGetTInstant(X)    ((TInstant *) PG_DETOAST_DATUM(X))
#define DatumGetTInstantSet(X)    ((TInstantSet *) PG_DETOAST_DATUM(X))
#define DatumGetTSequence(X)    ((TSequence *) PG_DETOAST_DATUM(X))
#define DatumGetTSequenceSet(X)    ((TSequenceSet *) PG_DETOAST_DATUM(X))

#define PG_GETARG_TEMPORAL(i)    ((Temporal *) PG_GETARG_VARLENA_P(i))

#define PG_GETARG_ANYDATUM(i) (get_typlen(get_fn_expr_argtype(fcinfo->flinfo, i)) == -1 ? \
  PointerGetDatum(PG_GETARG_VARLENA_P(i)) : PG_GETARG_DATUM(i))

#define DATUM_FREE(value, valuetypid) \
  do { \
    if (! get_typbyval_fast(valuetypid)) \
      pfree(DatumGetPointer(value)); \
  } while (0)

#define DATUM_FREE_IF_COPY(value, valuetypid, n) \
  do { \
    if (! get_typbyval_fast(valuetypid) && DatumGetPointer(value) != PG_GETARG_POINTER(n)) \
      pfree(DatumGetPointer(value)); \
  } while (0)

/*
 * Define POSTGIS_FREE_IF_COPY_P if POSTGIS is not loaded.
 * This macro is based on PG_FREE_IF_COPY, except that it accepts two pointers.
 * See PG_FREE_IF_COPY comment in src/include/fmgr.h in postgres source code
 * for more details.
 */
#ifndef POSTGIS_FREE_IF_COPY_P
#define POSTGIS_FREE_IF_COPY_P(ptrsrc, ptrori) \
  do { \
    if ((Pointer) (ptrsrc) != (Pointer) (ptrori)) \
      pfree(ptrsrc); \
  } while (0)
#endif

/*****************************************************************************/

/* Utility functions */

extern TInstant *tsequenceset_find_timestamp_excl(const TSequenceSet *ts, TimestampTz t);
extern TInstant *tsequence_find_timestamp_excl(const TSequence *seq, TimestampTz t);

extern Temporal *temporal_copy(const Temporal *temp);
extern Temporal *pg_getarg_temporal(const Temporal *temp);
extern bool intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  TIntersection mode, Temporal **inter1, Temporal **inter2);
extern bool linear_interpolation(Oid type);

extern const char *tduration_name(TDuration duration);
extern bool tduration_from_string(const char *str, TDuration *duration);

/* Catalog functions */

extern Oid temporal_valuetypid(Oid temptypid);

/* Oid functions */

extern Oid range_oid_from_base(Oid type);
extern Oid temporal_oid_from_base(Oid type);
extern Oid base_oid_from_temporal(Oid type);

/* Trajectory functions */

extern bool type_has_precomputed_trajectory(Oid type);

/* Parameter tests */

extern bool talpha_base_type(Oid type);
extern bool tnumber_base_type(Oid type);
extern bool tnumber_range_type(Oid type);
extern bool tgeo_base_type(Oid type);
extern bool temporal_type(Oid type);
extern bool tnumber_type(Oid type);
extern bool tgeo_type(Oid type);

extern void ensure_talpha_base_type(Oid type);
extern void ensure_tnumber_base_type(Oid type);
extern void ensure_tnumber_range_type(Oid type);
extern void ensure_tgeo_base_type(Oid type);
extern void ensure_temporal_base_type(Oid type);
extern void ensure_temporal_base_type_all(Oid type);

extern void ensure_valid_duration(TDuration type);
extern void ensure_valid_duration_all(TDuration type);
extern void ensure_sequences_duration(TDuration duration);
extern void ensure_non_empty_array(ArrayType *array);
extern void ensure_linear_interpolation(Oid type);
extern void ensure_linear_interpolation_all(Oid type);

extern void ensure_same_duration(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_same_base_type(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_same_interpolation(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_increasing_timestamps(const TInstant *inst1,
  const TInstant *inst2, bool strict);
extern void ensure_same_overlapping_value(const TInstant *inst1,
  const TInstant *inst2);
extern void ensure_valid_tinstantarr(TInstant **instants, int count);
extern void ensure_valid_tsequencearr(TSequence **sequences, int count);

/* Input/output functions */

extern Datum temporal_in(PG_FUNCTION_ARGS);
extern Datum temporal_out(PG_FUNCTION_ARGS);
extern Datum temporal_send(PG_FUNCTION_ARGS);
extern Datum temporal_recv(PG_FUNCTION_ARGS);
extern Temporal* temporal_read(StringInfo buf, Oid valuetypid);
extern void temporal_write(Temporal* temp, StringInfo buf);

/* Constructor functions */

extern Datum tinstant_constructor(PG_FUNCTION_ARGS);
extern Datum tinstantset_constructor(PG_FUNCTION_ARGS);
extern Datum tlinearseq_constructor(PG_FUNCTION_ARGS);
extern Datum tstepseq_constructor(PG_FUNCTION_ARGS);
extern Datum tsequenceset_constructor(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tint_to_tfloat(PG_FUNCTION_ARGS);
extern Datum temporal_to_period(PG_FUNCTION_ARGS);

extern Temporal *tint_to_tfloat_internal(Temporal *temp);

/* Accessor functions */

extern Datum temporal_duration(PG_FUNCTION_ARGS);
extern Datum temporal_interpolation(PG_FUNCTION_ARGS);
extern Datum temporal_mem_size(PG_FUNCTION_ARGS);
extern Datum temporal_get_values(PG_FUNCTION_ARGS);
extern Datum temporal_get_time(PG_FUNCTION_ARGS);
extern Datum tinstant_get_value(PG_FUNCTION_ARGS);
extern Datum tnumber_to_tbox(PG_FUNCTION_ARGS);
extern Datum tnumber_value_range(PG_FUNCTION_ARGS);
extern Datum temporal_start_value(PG_FUNCTION_ARGS);
extern Datum temporal_end_value(PG_FUNCTION_ARGS);
extern Datum temporal_min_value(PG_FUNCTION_ARGS);
extern Datum temporal_max_value(PG_FUNCTION_ARGS);
extern Datum temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum temporal_instants(PG_FUNCTION_ARGS);
extern Datum temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum temporal_shift(PG_FUNCTION_ARGS);

extern PeriodSet *temporal_get_time_internal(const Temporal *temp);
extern Datum tfloat_ranges(const Temporal *temp);
extern TInstant *temporal_min_instant(const Temporal *temp);
extern Datum temporal_min_value_internal(const Temporal *temp);
extern TInstant *temporal_end_instant_internal(const Temporal *temp);
extern TimestampTz temporal_start_timestamp_internal(const Temporal *temp);
extern RangeType *tnumber_value_range_internal(const Temporal *temp);

/* Ever/always equal operators */

extern Datum temporal_ever_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ne(PG_FUNCTION_ARGS);
extern Datum temporal_ever_lt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_le(PG_FUNCTION_ARGS);
extern Datum temporal_ever_gt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ge(PG_FUNCTION_ARGS);

extern Datum temporal_always_eq(PG_FUNCTION_ARGS);
extern Datum temporal_always_ne(PG_FUNCTION_ARGS);
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
extern Datum temporal_intersects_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_period(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_periodset(PG_FUNCTION_ARGS);

extern bool temporal_value_at_timestamp_inc(const Temporal *temp,
  TimestampTz t, Datum *value);

extern bool temporal_bbox_restrict_value(const Temporal *temp, Datum value);
extern Datum *temporal_bbox_restrict_values(const Temporal *temp,
  const Datum *values, int count, int *newcount);
extern RangeType **tnumber_bbox_restrict_ranges(const Temporal *temp,
  RangeType **ranges, int count, int *newcount);

extern Temporal *temporal_restrict_value_internal(const Temporal *temp,
  Datum value, bool atfunc);
extern Temporal *temporal_restrict_values_internal(const Temporal *temp,
  Datum *values, int count, bool atfunc);
extern Temporal *tnumber_restrict_range_internal(const Temporal *temp,
  RangeType *range, bool atfunc);
extern Temporal *temporal_restrict_timestamp_internal(const Temporal *temp,
  TimestampTz t, bool atfunc);
extern Temporal *temporal_at_period_internal(const Temporal *temp,
  const Period *ps);
extern Temporal *temporal_minus_period_internal(const Temporal *temp,
  const Period *ps);
extern Temporal *temporal_restrict_periodset_internal(const Temporal *temp,
  const PeriodSet *ps, bool atfunc);

extern void temporal_period(Period *p, const Temporal *temp);
extern char *temporal_to_string(const Temporal *temp,
  char *(*value_out)(Oid, Datum));
extern void *temporal_bbox_ptr(const Temporal *temp);
extern void temporal_bbox(void *box, const Temporal *temp);

/* Comparison functions */

extern Datum temporal_lt(PG_FUNCTION_ARGS);
extern Datum temporal_le(PG_FUNCTION_ARGS);
extern Datum temporal_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ge(PG_FUNCTION_ARGS);
extern Datum temporal_gt(PG_FUNCTION_ARGS);
extern Datum temporal_cmp(PG_FUNCTION_ARGS);
extern Datum temporal_hash(PG_FUNCTION_ARGS);

extern uint32 temporal_hash_internal(const Temporal *temp);

/*****************************************************************************/

#endif
