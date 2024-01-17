/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
#include <meos.h>
#include "general/doublen.h"
#include "general/meos_catalog.h"

/* To avoid including pg_collation_d */
#define DEFAULT_COLLATION_OID 100
#define C_COLLATION_OID 950
#define POSIX_COLLATION_OID 951

#ifndef FMGR_H
  /* To avoid including fmgr.h However this implies that the text values must
   * be ALWAYS detoasted */
  #define DatumGetTextP(X)      ((text *) DatumGetPointer(X)) // PG_DETOAST_DATUM(X))
#endif /* FMGR_H */

/**
 * Floating point precision
 */
#define MEOS_EPSILON   1.0e-06
#define MEOS_FP_EQ(A, B) (fabs((A)-(B)) <= MEOS_EPSILON)
#define MEOS_FP_NE(A, B) (fabs((A)-(B)) > MEOS_EPSILON)
#define MEOS_FP_LT(A, B) (((A) + MEOS_EPSILON) < (B))
#define MEOS_FP_LE(A, B) (((A) - MEOS_EPSILON) <= (B))
#define MEOS_FP_GT(A, B) (((A) - MEOS_EPSILON) > (B))
#define MEOS_FP_GE(A, B) (((A) + MEOS_EPSILON) >= (B))

/**
 * Precision for distance operations
 */
#define DIST_EPSILON    1.0e-06

/** Symbolic constants for lifting */
#define DISCONTINUOUS   true
#define CONTINUOUS      false

/** Symbolic constants for sets and for normalizing spans */
#define ORDERED         true
#define ORDERED_NO      false

/** Symbolic constants for the output of string elements */
#define QUOTES          true
#define QUOTES_NO       false

/** Symbolic constants for the output of string elements */
#define SPACES          true
#define SPACES_NO       false

/** Symbolic constants for lifting */
#define INVERT          true
#define INVERT_NO       false

/** Symbolic constants for the restriction functions */
#define REST_AT         true
#define REST_MINUS      false

/** Symbolic constants for the restriction functions with boxes */
#define BORDER_INC       true
#define BORDER_EXC       false

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

/** Symbolic constants for spatial relationships */
#define WITH_Z          true
#define NO_Z            false

/** Symbolic constants for the span selectivity functions */
#define VALUE_SEL       true
#define TIME_SEL        false

/** Symbolic constants for the restriction functions */
#define UPPER_EXC       true
#define TIME_SEL        false

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

/** Enumeration for the set operations of span and temporal types */
typedef enum
{
  UNION,
  INTER,
  MINUS
} SetOper;

/*****************************************************************************
 * Well-Known Binary (WKB)
 *****************************************************************************/

/* Data type size */
#define MEOS_WKB_BYTE_SIZE        1
#define MEOS_WKB_INT2_SIZE        2
#define MEOS_WKB_INT4_SIZE        4
#define MEOS_WKB_INT8_SIZE        8
#define MEOS_WKB_DOUBLE_SIZE      8
#define MEOS_WKB_DATE_SIZE        4
#define MEOS_WKB_TIMESTAMP_SIZE   8

/* Temporal subtype */
enum MEOS_WKB_TSUBTYPE
{
  MEOS_WKB_TINSTANT =         1,  /**< temporal instant subtype */
  MEOS_WKB_TSEQUENCE =        2,  /**< temporal sequence subtype */
  MEOS_WKB_TSEQUENCESET =     3,  /**< temporal sequence set subtype */
};

/* Span bounds */
#define MEOS_WKB_LOWER_INC        0x01
#define MEOS_WKB_UPPER_INC        0x02

/* Machine endianness */
#define XDR                        0  /* big endian */
#define NDR                        1  /* little endian */

/* Variation flags
 * The first byte of the variation flag depends on the type we are sending
 * - Set types: xxxO where O states whether the set is ordered or not
 * - Box types: xxTX where X and T state whether the corresponding dimensions
 *   are present
 * - Temporal types: xxSS where SS correspond to the subtype
 * and x are unused bits
 */
#define MEOS_WKB_ORDERED          0x01  // 1
#define MEOS_WKB_XFLAG            0x01  // 1
#define MEOS_WKB_TFLAG            0x02  // 2
#define MEOS_WKB_INTERPFLAGS      0x0C  // 4 + 8
#define MEOS_WKB_ZFLAG            0x10  // 16
#define MEOS_WKB_GEODETICFLAG     0x20  // 32
#define MEOS_WKB_SRIDFLAG         0x40  // 64

#define MEOS_WKB_GET_INTERP(flags) (((flags) & MEOS_WKB_INTERPFLAGS) >> 2)
#define MEOS_WKB_SET_INTERP(flags, value) ((flags) = (((flags) & ~MEOS_WKB_INTERPFLAGS) | ((value & 0x0003) << 2)))

// #define MEOS_WKB_GET_LINEAR(flags)     ((bool) (((flags) & MEOS_WKB_LINEARFLAG)>>3))

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
#define DEFAULT_BIGINTSPAN_ORIGIN (0)

/*****************************************************************************
 * Additional struct definitions for temporal types
 *****************************************************************************/

/**
 * Structure to represent all types of bounding boxes
 */
typedef union bboxunion
{
  Span p;      /**< Span */
  TBox b;      /**< Temporal box */
  STBox g;     /**< Spatiotemporal box */
} bboxunion;

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

/* Definition of output function */
typedef char *(*outfunc)(Datum value, meosType type, int maxdd);

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

#define PG_GETARG_TEMPORAL_P(X)      ((Temporal *) PG_GETARG_VARLENA_P(X))
#define PG_GETARG_TINSTANT_P(X)      ((TInstant *) PG_GETARG_VARLENA_P(X))
#define PG_GETARG_TSEQUENCE_P(X)     ((TSequence *) PG_GETARG_VARLENA_P(X))
#define PG_GETARG_TSEQUENCESET_P(X)  ((TSequenceSet *) PG_GETARG_VARLENA_P(X))

#define PG_RETURN_TEMPORAL_P(X)      PG_RETURN_POINTER(X)
#define PG_RETURN_TINSTANT_P(X)      PG_RETURN_POINTER(X)
#define PG_RETURN_TSEQUENCE_P(X)     PG_RETURN_POINTER(X)
#define PG_RETURN_TSEQUENCESET_P(X)  PG_RETURN_POINTER(X)

#define TemporalPGetDatum(X)         PointerGetDatum(X)
#define TInstantPGetDatum(X)         PointerGetDatum(X)
#define TSequencePGetDatum(X)        PointerGetDatum(X)
#define TSequenceSetPGetDatum(X)     PointerGetDatum(X)

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

/*****************************************************************************/

/* Parameter tests */

extern bool ensure_not_null(void *ptr);
extern bool ensure_one_not_null(void *ptr1, void *ptr2);
extern bool ensure_one_true(bool hasshift, bool haswidth);
extern bool ensure_valid_interp(meosType temptype, interpType interp);
extern bool ensure_continuous(const Temporal *temp);
extern bool ensure_same_interp(const Temporal *temp1, const Temporal *temp2);
extern bool ensure_same_continuous_interp(int16 flags1, int16 flags2);
extern bool ensure_linear_interp(int16 flags);
extern bool ensure_nonlinear_interp(int16 flags);
extern bool ensure_common_dimension(int16 flags1, int16 flags2);
extern bool ensure_temporal_isof_type(const Temporal *temp, meosType type);
extern bool ensure_temporal_isof_basetype(const Temporal *temp,
  meosType basetype);
extern bool ensure_temporal_isof_subtype(const Temporal *temp,
  tempSubtype type);
extern bool ensure_same_temporal_type(const Temporal *temp1,
  const Temporal *temp2);

extern bool ensure_valid_tnumber_span(const Temporal *temp, const Span *s);
extern bool ensure_valid_tnumber_spanset(const Temporal *temp,
  const SpanSet *ss);
extern bool ensure_valid_tnumber_tbox(const Temporal *temp, const TBox *box);
extern bool ensure_not_negative(int i);
extern bool ensure_positive(int i);
extern bool ensure_less_equal(int i, int j);
extern bool not_negative_datum(Datum size, meosType basetype);
extern bool ensure_not_negative_datum(Datum size, meosType basetype);
extern bool positive_datum(Datum size, meosType basetype);
extern bool ensure_positive_datum(Datum size, meosType basetype);
extern bool valid_duration(const Interval *duration);
extern bool ensure_valid_duration(const Interval *duration);

/* General functions */

extern void *temporal_bbox_ptr(const Temporal *temp);

extern bool intersection_temporal_temporal(const Temporal *temp1,
const Temporal *temp2, SyncMode mode, Temporal **inter1, Temporal **inter2);

/* Version functions */

extern char *mobilitydb_version(void);
extern char *mobilitydb_full_version(void);

/* Ever/always equal operators */

extern bool ea_eq_bbox_temp_base(const Temporal *temp, Datum value, bool ever);
extern bool ea_lt_bbox_temp_base(const Temporal *temp, Datum value, bool ever);

/* Restriction functions */

extern bool temporal_bbox_restrict_value(const Temporal *temp, Datum value);
extern Datum *temporal_bbox_restrict_values(const Temporal *temp,
  const Datum *values, int count, int *newcount);
extern bool temporal_bbox_restrict_set(const Temporal *temp, const Set *set);
extern Temporal *temporal_restrict_minmax(const Temporal *temp, bool min,
  bool atfunc);

/*****************************************************************************/

#endif
