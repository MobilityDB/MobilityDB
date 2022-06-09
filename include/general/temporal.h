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
/* MobilityDB */
#include "general/span.h"
#include "general/temporal_catalog.h"
#include "general/timetypes.h"
#include "general/tbox.h"
#include "point/stbox.h"

/* To avoid including builtins.h */
extern text *cstring_to_text(const char *s);
extern char *text_to_cstring(const text *t);

/* To avoid including pg_collation_d */
#define DEFAULT_COLLATION_OID 100
#define C_COLLATION_OID 950
#define POSIX_COLLATION_OID 951

#if MEOS
  /* To avoid including fmgr.h However this implies that the text values must
   * be ALWAYS detoasted */
  #define DatumGetTextP(X)			((text *) DatumGetPointer(X)) // PG_DETOAST_DATUM(X))
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

/* Determine whether reduce the roundoff errors with the span operations
 * by taking the bounds instead of the projected value at the timestamp */
#define SPAN_ROUNDOFF  false

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
 * Well-Known Binary (WKB)
 *****************************************************************************/

/* Data type size */
#define MOBDB_WKB_TIMESTAMP_SIZE   8
#define MOBDB_WKB_DOUBLE_SIZE      8
#define MOBDB_WKB_INT2_SIZE        2
#define MOBDB_WKB_INT4_SIZE        4
#define MOBDB_WKB_INT8_SIZE        8
#define MOBDB_WKB_BYTE_SIZE        1

/* MobilityDB Types */
#define MOBDB_WKB_T_BOOL           1   /**< boolean type */
#define MOBDB_WKB_T_DOUBLE2        2   /**< double2 type */
#define MOBDB_WKB_T_DOUBLE3        3   /**< double3 type */
#define MOBDB_WKB_T_DOUBLE4        4   /**< double4 type */
#define MOBDB_WKB_T_FLOAT8         5   /**< float8 type */
#define MOBDB_WKB_T_FLOATSPAN      6   /**< float8 span type */
#define MOBDB_WKB_T_INT4           7   /**< int4 type */
#define MOBDB_WKB_T_INTSPAN        8   /**< int4 span type */
#define MOBDB_WKB_T_INT8           9   /**< int8 type */
#define MOBDB_WKB_T_PERIOD         10  /**< period type */
#define MOBDB_WKB_T_PERIODSET      11  /**< period set type */
#define MOBDB_WKB_T_STBOX          12  /**< spatiotemporal box type */
#define MOBDB_WKB_T_TBOOL          13  /**< temporal boolean type */
#define MOBDB_WKB_T_TBOX           14  /**< temporal box type */
#define MOBDB_WKB_T_TDOUBLE2       15  /**< temporal double2 type */
#define MOBDB_WKB_T_TDOUBLE3       16  /**< temporal double3 type */
#define MOBDB_WKB_T_TDOUBLE4       17  /**< temporal double4 type */
#define MOBDB_WKB_T_TEXT           18  /**< text type */
#define MOBDB_WKB_T_TFLOAT         19  /**< temporal float type */
#define MOBDB_WKB_T_TIMESTAMPSET   20  /**< timestamp set type */
#define MOBDB_WKB_T_TIMESTAMPTZ    21  /**< timestamp with time zone type */
#define MOBDB_WKB_T_TINT           22  /**< temporal integer type */
#define MOBDB_WKB_T_TTEXT          23  /**< temporal text type */
#define MOBDB_WKB_T_GEOMETRY       24  /**< geometry type */
#define MOBDB_WKB_T_GEOGRAPHY      25  /**< geography type */
#define MOBDB_WKB_T_TGEOMPOINT     26  /**< temporal geometry point type */
#define MOBDB_WKB_T_TGEOGPOINT     27  /**< temporal geography point type */
#define MOBDB_WKB_T_NPOINT         28  /**< network point type */
#define MOBDB_WKB_T_NSEGMENT       29  /**< network segment type */
#define MOBDB_WKB_T_TNPOINT        30  /**< temporal network point type */

/* Temporal subtype */
#define MOBDB_WKB_INSTANT          1
#define MOBDB_WKB_INSTANTSET       2
#define MOBDB_WKB_SEQUENCE         3
#define MOBDB_WKB_SEQUENCESET      4

/* Period bounds */
#define MOBDB_WKB_LOWER_INC        0x01
#define MOBDB_WKB_UPPER_INC        0x02

/* Machine endianness */
#define XDR                        0  /* big endian */
#define NDR                        1  /* little endian */

/* Variation flags
 * The first byte of the variation flag depends on the type we are sending
 * - Box types: xxTX where X and T state whether the corresponding dimensions
 *   are present and x are unused
 * - Temporal types: xSSS where SSS correspond to the subtype and x is unused
 */
#define MOBDB_WKB_XFLAG            0x01
#define MOBDB_WKB_TFLAG            0x02
#define MOBDB_WKB_ZFLAG            0x10
#define MOBDB_WKB_GEODETICFLAG     0x20
#define MOBDB_WKB_SRIDFLAG         0x40
#define MOBDB_WKB_LINEAR_INTERP    0x80

/*****************************************************************************
 * Definitions for bucketing and tiling
 *****************************************************************************/

/*
 * The default origin is Monday 2000-01-03. We don't use PG epoch since it
 * starts on a Saturday. This makes time-buckets by a week more intuitive and
 * aligns it with date_trunc.
 */
#define JAN_3_2000 (2 * USECS_PER_DAY)
#define DEFAULT_TIME_ORIGIN (JAN_3_2000)
#define DEFAULT_FLOATSPAN_ORIGIN (0.0)
#define DEFAULT_INTSPAN_ORIGIN (0)

/*****************************************************************************
 * Struct definitions for temporal types
 *****************************************************************************/

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
  /* variable-length data follows, if any */
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
  /* variable-length data follows */
} TInstant;

/**
 * Structure to represent temporal values of instant set subtype
 */
typedef struct
{
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  int32         count;        /**< Number of TInstant elements */
  int16         bboxsize;     /**< Size of the bounding box */
  /**< beginning of variable-length data */
} TInstantSet;

/**
 * Structure to represent temporal values of sequence subtype
 */
typedef struct
{
  int32         vl_len_;      /**< Varlena header (do not touch directly!) */
  uint8         temptype;     /**< Temporal type */
  uint8         subtype;      /**< Temporal subtype */
  int16         flags;        /**< Flags */
  int32         count;        /**< Number of TInstant elements */
  int16         bboxsize;     /**< Size of the bounding box */
  Period        period;       /**< Time span (24 bytes) */
  /**< beginning of variable-length data */
} TSequence;

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
  int32         totalcount;   /**< Total number of TInstant elements in all TSequence elements */
  int16         bboxsize;     /**< Size of the bounding box */
  /**< beginning of variable-length data */
} TSequenceSet;

/**
 * Structure to represent all types of bounding boxes
 */
typedef union bboxunion
{
  Period    p;      /**< Period */
  TBOX      b;      /**< Temporal box */
  STBOX     g;      /**< Spatiotemporal box */
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

/**
 * Struct for storing a similarity match
 */
typedef struct
{
  int i;
  int j;
} Match;

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

#if MEOS
  #define DatumGetTemporalP(X)       ((Temporal *) DatumGetPointer(X))
#else
  #define DatumGetTemporalP(X)       ((Temporal *) PG_DETOAST_DATUM(X))
#endif /* MEOS */

#define PG_GETARG_TEMPORAL_P(X)    ((Temporal *) PG_GETARG_VARLENA_P(X))

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

/**
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

/**
 * @brief Macro for accessing the GSERIALIZED value of a temporal point.
 * @pre It is assumed that the geometry/geography IS NOT TOASTED
 */
#define DatumGetGserializedP(X)      ((GSERIALIZED *) DatumGetPointer(X))

/*****************************************************************************/

/* Parameter tests */

extern void ensure_valid_tempsubtype(int16 type);
extern void ensure_valid_tempsubtype_all(int16 type);
extern void ensure_seq_subtypes(int16 subtype);
extern void ensure_tinstarr(const TInstant **instants, int count);
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

/* General functions */

extern void *temporal_bbox_ptr(const Temporal *temp);
extern void temporal_bbox_slice(Datum tempdatum, void *box);

extern bool intersection_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, SyncMode mode, Temporal **inter1, Temporal **inter2);
extern const TInstant *tinstarr_inst_n(const Temporal *temp, int n);

/* Version functions */

extern char *mobilitydb_version(void);
extern char *mobilitydb_full_version(void);

/* Ever/always equal operators */

extern bool temporal_bbox_ev_al_eq(const Temporal *temp, Datum value,
  bool ever);
extern bool temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value,
  bool ever);

/* Restriction functions */

extern bool temporal_bbox_restrict_value(const Temporal *temp, Datum value);
extern Datum *temporal_bbox_restrict_values(const Temporal *temp,
  const Datum *values, int count, int *newcount);
extern Span **tnumber_bbox_restrict_spans(const Temporal *temp,
  Span **spans, int count, int *newcount);
extern Temporal *temporal_restrict_minmax(const Temporal *temp, bool min,
  bool atfunc);

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

#include <c.h>
#include <utils/palloc.h>
#include <utils/elog.h>
// #include <catalog/pg_type.h>
#include <utils/array.h>
#include <utils/lsyscache.h>
#include <catalog/pg_type_d.h> /* for TIMESTAMPTZOID and similar */
#include "point/postgis.h"

// #if POSTGRESQL_VERSION_NUMBER < 130000
// #if USE_FLOAT4_BYVAL
// #error Postgres needs to be configured with USE_FLOAT4_BYVAL
// #endif
// #endif

// #if USE_FLOAT8_BYVAL
// #error Postgres needs to be configured with USE_FLOAT8_BYVAL
// #endif

/* To avoid including fmgrprotos.h */
extern Datum numeric_float8(PG_FUNCTION_ARGS);
extern Datum numeric_round(PG_FUNCTION_ARGS);
extern Datum float8_numeric(PG_FUNCTION_ARGS);

#define PG_GETARG_ANYDATUM(X) (get_typlen(get_fn_expr_argtype(fcinfo->flinfo, X)) == -1 ? \
  PointerGetDatum(PG_GETARG_VARLENA_P(X)) : PG_GETARG_DATUM(X))

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
 * Definitions for GiST indexes
 *****************************************************************************/

/* Minimum accepted ratio of split */
#define LIMIT_RATIO 0.3

#if POSTGRESQL_VERSION_NUMBER < 120000
extern int float8_cmp_internal(float8 a, float8 b);
extern double get_float8_infinity(void);
#endif

/* Convenience macros for NaN-aware comparisons */
#define FLOAT8_EQ(a,b)   (float8_cmp_internal(a, b) == 0)
#define FLOAT8_LT(a,b)   (float8_cmp_internal(a, b) < 0)
#define FLOAT8_LE(a,b)   (float8_cmp_internal(a, b) <= 0)
#define FLOAT8_GT(a,b)   (float8_cmp_internal(a, b) > 0)
#define FLOAT8_GE(a,b)   (float8_cmp_internal(a, b) >= 0)
#define FLOAT8_MAX(a,b)  (FLOAT8_GT(a, b) ? (a) : (b))
#define FLOAT8_MIN(a,b)  (FLOAT8_LT(a, b) ? (a) : (b))

/*****************************************************************************
 * Typmod definitions
 *****************************************************************************/

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

/* Initialization function */

extern void _PG_init(void);

/* Typmod functions */

extern const char *tempsubtype_name(int16 subtype);
extern bool tempsubtype_from_string(const char *str, int16 *subtype);

/* Send/receive functions */

extern Temporal *temporal_recv(StringInfo buf);
extern void temporal_write(const Temporal *temp, StringInfo buf);

/* Parameter tests */

extern void ensure_non_empty_array(ArrayType *array);

#endif /* #if ! MEOS */

/*****************************************************************************/

#endif
