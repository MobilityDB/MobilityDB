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
 * @brief Basic functions for temporal types of any subtype.
 */

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
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

/** Symbolic constants for the normalizing spans */
#define SORT            true
#define SORT_NO         false

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
#define TINSTANT        1
#define TSEQUENCE       2
#define TSEQUENCESET    3

/*****************************************************************************
 * Interpolation functions
 *****************************************************************************/

/**
 * Enumeration for the interpolation functions for temporal types
 */
// #define INTERP_NONE     0
// #define DISCRETE        1
// #define STEPWISE        2
// #define LINEAR          3

/*****************************************************************************
 * Macros for manipulating the 'flags' element where the less significant
 * bits are GTZXIICB, where
 *   G: coordinates are geodetic
 *   T: has T coordinate,
 *   Z: has Z coordinate
 *   X: has value or X coordinate
 *   II: interpolation
 *   C: continuous base type
 *   B: base type passed by value
 * Notice that formally speaking the interpolation flags are only needed
 * for sequence and sequence set subtypes.
 *****************************************************************************/

/* The following flag is only used for TInstant */
#define MOBDB_FLAG_BYVAL      0x0001  // 1
#define MOBDB_FLAG_CONTINUOUS 0x0002  // 2
/* The following two interpolation flags are only used for TSequence and TSequenceSet */
#define MOBDB_FLAGS_INTERP    0x000C  // 4 or 8
/* The following two flags are used for both bounding boxes and temporal types */
#define MOBDB_FLAG_X          0x0010  // 16
#define MOBDB_FLAG_Z          0x0020  // 32
#define MOBDB_FLAG_T          0x0040  // 64
#define MOBDB_FLAG_GEODETIC   0x0080  // 128

#define MOBDB_FLAGS_GET_BYVAL(flags)      ((bool) (((flags) & MOBDB_FLAG_BYVAL)))
#define MOBDB_FLAGS_GET_CONTINUOUS(flags) ((bool) (((flags) & MOBDB_FLAG_CONTINUOUS)>>1))
#define MOBDB_FLAGS_GET_X(flags)          ((bool) (((flags) & MOBDB_FLAG_X)>>4))
#define MOBDB_FLAGS_GET_Z(flags)          ((bool) (((flags) & MOBDB_FLAG_Z)>>5))
#define MOBDB_FLAGS_GET_T(flags)          ((bool) (((flags) & MOBDB_FLAG_T)>>6))
#define MOBDB_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & MOBDB_FLAG_GEODETIC)>>7))

#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_BYVAL) : ((flags) & ~MOBDB_FLAG_BYVAL))
#define MOBDB_FLAGS_SET_CONTINUOUS(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_CONTINUOUS) : ((flags) & ~MOBDB_FLAG_CONTINUOUS))
#define MOBDB_FLAGS_SET_X(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_X) : ((flags) & ~MOBDB_FLAG_X))
#define MOBDB_FLAGS_SET_Z(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_Z) : ((flags) & ~MOBDB_FLAG_Z))
#define MOBDB_FLAGS_SET_T(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_T) : ((flags) & ~MOBDB_FLAG_T))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
  ((flags) = (value) ? ((flags) | MOBDB_FLAG_GEODETIC) : ((flags) & ~MOBDB_FLAG_GEODETIC))

#define MOBDB_FLAGS_GET_INTERP(flags) (((flags) & MOBDB_FLAGS_INTERP) >> 2)
#define MOBDB_FLAGS_SET_INTERP(flags, value) ((flags) = (((flags) & ~MOBDB_FLAGS_INTERP) | ((value & 0x0003) << 2)))

#define MOBDB_FLAGS_GET_DISCRETE(flags)   ((bool) (MOBDB_FLAGS_GET_INTERP((flags)) == DISCRETE))
#define MOBDB_FLAGS_GET_STEPWISE(flags)   ((bool) (MOBDB_FLAGS_GET_INTERP((flags)) == STEPWISE))
#define MOBDB_FLAGS_GET_LINEAR(flags)     ((bool) (MOBDB_FLAGS_GET_INTERP((flags)) == LINEAR))

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
enum MOBDB_WKB_TYPE
{
  MOBDB_WKB_T_BOOL =           1,   /**< boolean type */
  MOBDB_WKB_T_DOUBLE2 =        2,   /**< double2 type */
  MOBDB_WKB_T_DOUBLE3 =        3,   /**< double3 type */
  MOBDB_WKB_T_DOUBLE4 =        4,   /**< double4 type */
  MOBDB_WKB_T_FLOAT8 =         5,   /**< float8 type */
  MOBDB_WKB_T_FLOATSPAN =      6,   /**< float8 span type */
  MOBDB_WKB_T_INT4 =           7,   /**< int4 type */
  MOBDB_WKB_T_INTSPAN =        8,   /**< int4 span type */
  MOBDB_WKB_T_INT8 =           9,   /**< int8 type */
  MOBDB_WKB_T_PERIOD =         10,  /**< period type */
  MOBDB_WKB_T_PERIODSET =      11,  /**< period set type */
  MOBDB_WKB_T_STBOX =          12,  /**< spatiotemporal box type */
  MOBDB_WKB_T_TBOOL =          13,  /**< temporal boolean type */
  MOBDB_WKB_T_TBOX =           14,  /**< temporal box type */
  MOBDB_WKB_T_TDOUBLE2 =       15,  /**< temporal double2 type */
  MOBDB_WKB_T_TDOUBLE3 =       16,  /**< temporal double3 type */
  MOBDB_WKB_T_TDOUBLE4 =       17,  /**< temporal double4 type */
  MOBDB_WKB_T_TEXT =           18,  /**< text type */
  MOBDB_WKB_T_TFLOAT =         19,  /**< temporal float type */
  MOBDB_WKB_T_TIMESTAMPSET =   20,  /**< timestamp set type */
  MOBDB_WKB_T_TIMESTAMPTZ =    21,  /**< timestamp with time zone type */
  MOBDB_WKB_T_TINT =           22,  /**< temporal integer type */
  MOBDB_WKB_T_TTEXT =          23,  /**< temporal text type */
  MOBDB_WKB_T_GEOMETRY =       24,  /**< geometry type */
  MOBDB_WKB_T_GEOGRAPHY =      25,  /**< geography type */
  MOBDB_WKB_T_TGEOMPOINT =     26,  /**< temporal geometry point type */
  MOBDB_WKB_T_TGEOGPOINT =     27,  /**< temporal geography point type */
  MOBDB_WKB_T_NPOINT =         28,  /**< network point type */
  MOBDB_WKB_T_NSEGMENT =       29,  /**< network segment type */
  MOBDB_WKB_T_TNPOINT =        30,  /**< temporal network point type */
};

/* Temporal subtype */
enum MOBDB_WKB_TSUBTYPE
{
  MOBDB_WKB_TINSTANT =         1,  /**< temporal instant subtype */
  MOBDB_WKB_TSEQUENCE =        2,  /**< temporal sequence subtype */
  MOBDB_WKB_TSEQUENCESET =     3,  /**< temporal sequence set subtype */
};

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
 * - Temporal types: xxSS where SS correspond to the subtype and x is unused
 */
#define MOBDB_WKB_XFLAG            0x01  // 1
#define MOBDB_WKB_TFLAG            0x02  // 2
#define MOBDB_WKB_INTERPFLAGS      0x0C  // 4 + 8
#define MOBDB_WKB_ZFLAG            0x10  // 16
#define MOBDB_WKB_GEODETICFLAG     0x20  // 32
#define MOBDB_WKB_SRIDFLAG         0x40  // 64

#define MOBDB_WKB_GET_INTERP(flags) (((flags) & MOBDB_WKB_INTERPFLAGS) >> 2)
#define MOBDB_WKB_SET_INTERP(flags, value) ((flags) = (((flags) & ~MOBDB_WKB_INTERPFLAGS) | ((value & 0x0003) << 2)))

// #define MOBDB_WKB_GET_LINEAR(flags)     ((bool) (((flags) & MOBDB_WKB_LINEARFLAG)>>3))

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
 * Additional struct definitions for temporal types
 *****************************************************************************/

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
#define PG_GETARG_TINSTANT_P(X)    ((TInstant *) PG_GETARG_VARLENA_P(X))
#define PG_GETARG_TSEQUENCE_P(X)    ((TSequence *) PG_GETARG_VARLENA_P(X))
#define PG_GETARG_TSEQUENCESET_P(X)    ((TSequenceSet *) PG_GETARG_VARLENA_P(X))

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
extern void ensure_continuous(const Temporal *temp);
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
  bool merge, interpType interp);
extern int *ensure_valid_tinstarr_gaps(const TInstant **instants, int count,
  bool merge, interpType interp, double maxdist, Interval *maxt, int *countsplits);
extern void ensure_valid_tseqarr(const TSequence **sequences, int count);

extern void ensure_positive_datum(Datum size, mobdbType basetype);
extern void ensure_valid_duration(const Interval *duration);

/* General functions */

extern void *temporal_bbox_ptr(const Temporal *temp);
extern void temporal_bbox_slice(Datum tempdatum, void *box);

extern bool intersection_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, SyncMode mode, Temporal **inter1, Temporal **inter2);

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

#endif
