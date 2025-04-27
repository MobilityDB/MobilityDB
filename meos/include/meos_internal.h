/*****************************************************************************
 *
 * This MobilityDB code seq provided under The PostgreSQL License.
 * Copyright(c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License(GPLv2 or later).
 * Copyright(c) 2001-2023, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement seq hereby granted, provided that the above copyright notice and
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
 * @brief Internal API of the Mobility Engine Open Source(MEOS) library.
 */

#ifndef __MEOS_INTERNAL_H__
#define __MEOS_INTERNAL_H__

/* C */
#include <stddef.h>
/* JSON-C */
#include <json-c/json.h>
/* GSL */
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
/* PROJ */
#include <proj.h>
/* PostgreSQL */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include "general/meos_catalog.h" /* For meosType */

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that a pointer is not null
 */
#ifdef MEOS
  #define VALIDATE_NOT_NULL(ptr, ret) \
    do { if (! ensure_not_null((void *) (ptr))) return (ret); } while (0)
#else
  #define VALIDATE_NOT_NULL(ptr, ret) \
    do { assert(ptr); } while (0)
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Macro for ensuring that a set is an integer set
 */
#if MEOS
  #define VALIDATE_INTSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_INTSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_INTSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_INTSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a set is a big integer set
 */
#if MEOS
  #define VALIDATE_BIGINTSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_BIGINTSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_BIGINTSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_BIGINTSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a set is a float set
 */
#if MEOS
  #define VALIDATE_FLOATSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_FLOATSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_FLOATSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_FLOATSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a set is a text set
 */
#if MEOS
  #define VALIDATE_TEXTSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_TEXTSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TEXTSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_TEXTSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a set is a date set
 */
#if MEOS
  #define VALIDATE_DATESET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_DATESET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_DATESET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_DATESET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a set is a timestamptz set
 */
#if MEOS
  #define VALIDATE_TSTZSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_TSTZSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TSTZSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_TSTZSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the span is a number span
 */
#ifdef MEOS
  #define VALIDATE_NUMSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_numset_type((set)->settype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_NUMSET(set, ret) \
    do { \
      assert(set); \
      assert(numset_type((set)->settype)); \
    } while (0)
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Macro for ensuring that a span is an integer span
 */
#if MEOS
  #define VALIDATE_INTSPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_span_isof_type((span), T_INTSPAN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_INTSPAN(span, ret) \
    do { \
      assert(span); \
      assert((span)->spantype == T_INTSPAN); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span is a big integer span
 */
#if MEOS
  #define VALIDATE_BIGINTSPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_span_isof_type((span), T_BIGINTSPAN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_BIGINTSPAN(span, ret) \
    do { \
      assert(span); \
      assert((span)->spantype == T_BIGINTSPAN); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span is a float span
 */
#if MEOS
  #define VALIDATE_FLOATSPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_span_isof_type((span), T_FLOATSPAN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_FLOATSPAN(span, ret) \
    do { \
      assert(span); \
      assert((span)->spantype == T_FLOATSPAN); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span is a date span
 */
#if MEOS
  #define VALIDATE_DATESPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_span_isof_type((span), T_DATESPAN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_DATESPAN(span, ret) \
    do { \
      assert(span); \
      assert((span)->spantype == T_DATESPAN); \
    } while (0)
#endif /* MEOS */


/**
 * @brief Macro for ensuring that the span is a timestamptz span
 */
#ifdef MEOS
  #define VALIDATE_TSTZSPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_span_isof_type((span), T_TSTZSPAN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TSTZSPAN(span, ret) \
    do { \
      assert(span); \
      assert((span)->spantype == T_TSTZSPAN); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the span is a number span
 */
#ifdef MEOS
  #define VALIDATE_NUMSPAN(span, ret) \
    do { \
          if (! ensure_not_null((void *) (span)) || \
              ! ensure_numspan_type((span)->spantype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_NUMSPAN(span, ret) \
    do { \
      assert(span); \
      assert(numspan_type((span)->spantype)); \
    } while (0)
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Macro for ensuring that a span set is an integer span set
 */
#if MEOS
  #define VALIDATE_INTSPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_spanset_isof_type((ss), T_INTSPANSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_INTSPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert((ss)->spansettype == T_INTSPANSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span set is a big integer span set
 */
#if MEOS
  #define VALIDATE_BIGINTSPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_spanset_isof_type((ss), T_BIGINTSPANSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_BIGINTSPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert((ss)->spansettype == T_BIGINTSPANSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span set is a float span set
 */
#if MEOS
  #define VALIDATE_FLOATSPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_spanset_isof_type((ss), T_FLOATSPANSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_FLOATSPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert((ss)->spansettype == T_FLOATSPANSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that a span set is a date span set
 */
#if MEOS
  #define VALIDATE_DATESPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_spanset_isof_type((ss), T_DATESPANSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_DATESPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert((ss)->spansettype == T_DATESPANSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the span set is a timestamptz span set
 */
#ifdef MEOS
  #define VALIDATE_TSTZSPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_spanset_isof_type(ss, T_TSTZSPANSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TSTZSPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert((ss)->spansettype == T_TSTZSPANSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the span set is a number span set
 */
#ifdef MEOS
  #define VALIDATE_NUMSPANSET(ss, ret) \
    do { \
          if (! ensure_not_null((void *) (ss)) || \
              ! ensure_numspanset_type((ss)->spansettype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_NUMSPANSET(ss, ret) \
    do { \
      assert(ss); \
      assert(numspanset_type((ss)->spansettype)); \
    } while (0)
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Macro for ensuring that the temporal value is a temporal Boolean
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TBOOL(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TBOOL) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TBOOL(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TBOOL); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value is a temporal integer
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TINT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TINT(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TINT); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value is a temporal float
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TFLOAT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TFLOAT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TFLOAT(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TFLOAT); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value is a temporal text
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TTEXT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TTEXT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TTEXT(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TTEXT); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value is a temporal number
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TNUMBER(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_tnumber_type(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TNUMBER(temp, ret) \
    do { \
      assert(temp); \
      assert(tnumber_type(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/*****************************************************************************
 * Internal function accessing the Gnu Scientic Library (GSL)
 *****************************************************************************/

extern gsl_rng *gsl_get_generation_rng(void);
extern gsl_rng *gsl_get_aggregation_rng(void);

/*****************************************************************************
 * Internal function accessing the PROJ library
 *****************************************************************************/

extern PJ_CONTEXT *proj_get_context(void);

/*****************************************************************************
 * Direct access to a single point in the GSERIALIZED struct
 *****************************************************************************/

/*
 * Obtain a geometry/geography point from the GSERIALIZED WITHOUT creating
 * the corresponding LWGEOM. These functions constitute a **SERIOUS**
 * break of encapsulation but it is the only way to achieve reasonable
 * performance when manipulating mobility data.
 * The datum_* functions suppose that the GSERIALIZED has been already
 * detoasted. This is typically the case when the datum is within a Temporal*
 * that has been already detoasted with PG_GETARG_TEMPORAL*
 * The first variant (e.g. datum_point2d) is slower than the second (e.g.
 * datum_point2d_p) since the point is passed by value and thus the bytes
 * are copied. The second version is declared const because you aren't allowed
 * to modify the values, only read them.
 */

/**
 * @brief Macro for accessing the GSERIALIZED value of a temporal point.
 * @pre It is assumed that the geometry/geography IS NOT TOASTED
 */
#define DatumGetGserializedP(X)      ((GSERIALIZED *) DatumGetPointer(X))
#define GserializedPGetDatum(X)      PointerGetDatum(X)

/**
 * @brief Definition for the internal aspects of  the GSERIALIZED struct
 */
// #define LWFLAG_EXTFLAGS    0x20
#define LWFLAG_VERSBIT2    0x80

// #define FLAGS_GET_EXTFLAGS(flags)     (((flags) & LWFLAG_EXTFLAGS)>>5)
#define FLAGS_GET_VERSBIT2(flags)     (((flags) & LWFLAG_VERSBIT2)>>7)

#define GS_POINT_PTR(gs)    ( (uint8_t *) ((gs)->data) + 8 + \
  FLAGS_GET_BBOX((gs)->gflags) * FLAGS_NDIMS_BOX((gs)->gflags) * 8 + \
  FLAGS_GET_VERSBIT2((gs)->gflags) * 8 )

/**
 * @brief Return a pointer to a 2D/3DZ point from the datum/GSERIALIZED
 */
#define DATUM_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR(DatumGetGserializedP(gs)) )
#define DATUM_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR(DatumGetGserializedP(gs)) )

#define GSERIALIZED_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR((gs)) )
#define GSERIALIZED_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR((gs)) )

/*****************************************************************************
 * Macros for manipulating the 'flags' element where the less significant
 * bits are MGTZXIICB, where
 *   M: (GEOM) the reference geometry is stored
 *   G: coordinates are geodetic
 *   T: has T coordinate,
 *   Z: has Z coordinate
 *   X: has value or X coordinate
 *   II: interpolation, whose values are
 *   - 00: INTERP_NONE (undetermined) for TInstant
 *   - 01: DISCRETE
 *   - 10: STEP
 *   - 11: LINEAR
 *   C: continuous base type / Ordered set
 *   B: base type passed by value
 * Notice that the interpolation flags are only needed for sequence and
 * sequence set subtypes.
 *****************************************************************************/

/* The following flag is only used for Set and TInstant */
#define MEOS_FLAG_BYVAL      0x0001  // 1
/* The following flag is only used for Set */
#define MEOS_FLAG_ORDERED    0x0002  // 2
/* The following flag is only used for Temporal */
#define MEOS_FLAG_CONTINUOUS 0x0002  // 2
/* The following two interpolation flags are only used for TSequence and TSequenceSet */
#define MEOS_FLAGS_INTERP    0x000C  // 4 / 8
/* The following two flags are used for both bounding boxes and temporal types */
#define MEOS_FLAG_X          0x0010  // 16
#define MEOS_FLAG_Z          0x0020  // 32
#define MEOS_FLAG_T          0x0040  // 64
#define MEOS_FLAG_GEODETIC   0x0080  // 128
#define MEOS_FLAG_GEOM       0x0100  // 256

#define MEOS_FLAGS_GET_BYVAL(flags)      ((bool) (((flags) & MEOS_FLAG_BYVAL)))
#define MEOS_FLAGS_GET_ORDERED(flags)    ((bool) (((flags) & MEOS_FLAG_ORDERED)>>1))
#define MEOS_FLAGS_GET_CONTINUOUS(flags) ((bool) (((flags) & MEOS_FLAG_CONTINUOUS)>>1))
#define MEOS_FLAGS_GET_X(flags)          ((bool) (((flags) & MEOS_FLAG_X)>>4))
#define MEOS_FLAGS_GET_Z(flags)          ((bool) (((flags) & MEOS_FLAG_Z)>>5))
#define MEOS_FLAGS_GET_T(flags)          ((bool) (((flags) & MEOS_FLAG_T)>>6))
#define MEOS_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & MEOS_FLAG_GEODETIC)>>7))
#define MEOS_FLAGS_GET_GEOM(flags)       ((bool) (((flags) & MEOS_FLAG_GEOM)>>8))

#define MEOS_FLAGS_BYREF(flags)          ((bool) (((flags) & ! MEOS_FLAG_BYVAL)))

#define MEOS_FLAGS_SET_BYVAL(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_BYVAL) : ((flags) & ~MEOS_FLAG_BYVAL))
#define MEOS_FLAGS_SET_ORDERED(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_ORDERED) : ((flags) & ~MEOS_FLAG_ORDERED))
#define MEOS_FLAGS_SET_CONTINUOUS(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_CONTINUOUS) : ((flags) & ~MEOS_FLAG_CONTINUOUS))
#define MEOS_FLAGS_SET_X(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_X) : ((flags) & ~MEOS_FLAG_X))
#define MEOS_FLAGS_SET_Z(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_Z) : ((flags) & ~MEOS_FLAG_Z))
#define MEOS_FLAGS_SET_T(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_T) : ((flags) & ~MEOS_FLAG_T))
#define MEOS_FLAGS_SET_GEODETIC(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_GEODETIC) : ((flags) & ~MEOS_FLAG_GEODETIC))
#define MEOS_FLAGS_SET_GEOM(flags, value) \
  ((flags) = (value) ? ((flags) | MEOS_FLAG_GEOM) : ((flags) & ~MEOS_FLAG_GEOM))

#define MEOS_FLAGS_GET_INTERP(flags) (((flags) & MEOS_FLAGS_INTERP) >> 2)
#define MEOS_FLAGS_SET_INTERP(flags, value) ((flags) = (((flags) & ~MEOS_FLAGS_INTERP) | ((value & 0x0003) << 2)))

#define MEOS_FLAGS_DISCRETE_INTERP(flags)   ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == DISCRETE))
#define MEOS_FLAGS_STEP_INTERP(flags)       ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == STEP))
#define MEOS_FLAGS_LINEAR_INTERP(flags)     ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == LINEAR))
#define MEOS_FLAGS_STEP_LINEAR_INTERP(flags)  \
  ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == STEP || MEOS_FLAGS_GET_INTERP((flags)) == LINEAR))

/*****************************************************************************
 * Generic type functions
 *****************************************************************************/

#if MEOS
#define TimestampTzGetDatum(X) Int64GetDatum(X)
#define DatumGetTimestampTz(X)((TimestampTz) DatumGetInt64(X))
#endif /* MEOS */

extern Datum datum_floor(Datum d);
extern Datum datum_ceil(Datum d);
extern Datum datum_degrees(Datum d, Datum normalize);
extern Datum datum_radians(Datum d);
extern uint32 datum_hash(Datum d, meosType basetype);
extern uint64 datum_hash_extended(Datum d, meosType basetype, uint64 seed);

extern Datum datum_float_round(Datum value, Datum size);
extern Datum datum_geo_round(Datum value, Datum size);
extern GSERIALIZED *point_round(const GSERIALIZED *gs, int maxdd);
extern void floatspan_round_set(const Span *s, int maxdd, Span *result);

/*****************************************************************************
 *  Macros for speeding up access to component values
 *****************************************************************************/

/* Macros for speeding up access to component values of sets and span sets */

#ifdef DEBUG_BUILD
extern void *SET_BBOX_PTR(const Set *s);
extern size_t *SET_OFFSETS_PTR(const Set *s);
extern Datum SET_VAL_N(const Set *s, int index);
extern const Span *SPANSET_SP_N(const SpanSet *ss, int index);
#else
/**
 * @brief Return a pointer to the bounding box of a set (if any)
 */
#define SET_BBOX_PTR(s) ( (void *)( \
  ((char *) (s)) + DOUBLE_PAD(sizeof(Set)) ) )

/**
 * @brief Return a pointer to the offsets array of a set
 */
#define SET_OFFSETS_PTR(s) ( (size_t *)( \
  ((char *) (s)) + DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD((s)->bboxsize) ) )

/**
 * @brief Return the n-th value of a set
 * @pre The argument @p index is less than the number of values in the set
 */
#define SET_VAL_N(s, index) ( (Datum) ( \
  MEOS_FLAGS_GET_BYVAL((s)->flags) ? (SET_OFFSETS_PTR(s))[index] : \
  PointerGetDatum( ((char *) (s)) + DOUBLE_PAD(sizeof(Set)) + \
    DOUBLE_PAD((s)->bboxsize) + (sizeof(size_t) * (s)->maxcount) + \
    (SET_OFFSETS_PTR(s))[index] ) ) )

/**
 * @brief Return the n-th span of a span set.
 * @pre The argument @p index is less than the number of spans in the span set
 * @note This is the macro equivalent to #spanset_span_n.
 * This function does not verify that the index is is in the correct bounds
 */
#define SPANSET_SP_N(ss, index) (const Span *) &((ss)->elems[(index)])
#endif

/*****************************************************************************/

/* Macros for speeding up access to components of temporal sequences (sets)*/

#ifdef DEBUG_BUILD
extern size_t *TSEQUENCE_OFFSETS_PTR(const TSequence *seq);
extern const TInstant *TSEQUENCE_INST_N(const TSequence *seq, int index);
extern size_t *TSEQUENCESET_OFFSETS_PTR(const TSequenceSet *ss);
extern const TSequence *TSEQUENCESET_SEQ_N(const TSequenceSet *ss, int index);
#else
/**
 * @brief Return a pointer to the offsets array of a temporal sequence
 * @note The period component of the bbox is already declared in the struct
 */
#define TSEQUENCE_OFFSETS_PTR(seq) ( (size_t *)( \
  ((char *) &((seq)->period)) + (seq)->bboxsize ) )

/**
 * @brief Return the n-th instant of a temporal sequence.
 * @note The period component of the bbox is already declared in the struct
 * @pre The argument @p index is less than the number of instants in the
 * sequence
 */
#define TSEQUENCE_INST_N(seq, index) ( (const TInstant *)( \
  ((char *) &((seq)->period)) + (seq)->bboxsize + \
  (sizeof(size_t) * (seq)->maxcount) + (TSEQUENCE_OFFSETS_PTR(seq))[index] ) )

/**
 * @brief Return a pointer to the offsets array of a temporal sequence set
 * @note The period component of the bbox is already declared in the struct
 */
#define TSEQUENCESET_OFFSETS_PTR(ss) ( (size_t *)( \
  ((char *) &((ss)->period)) + (ss)->bboxsize ) )

/**
 * @brief Return the n-th sequence of a temporal sequence set
 * @note The period component of the bbox is already declared in the struct
 * @pre The argument @p index is less than the number of sequences in the
 * sequence set
 */
#define TSEQUENCESET_SEQ_N(ss, index) ( (const TSequence *)( \
  ((char *) &((ss)->period)) + (ss)->bboxsize + \
  (sizeof(size_t) * (ss)->maxcount) + (TSEQUENCESET_OFFSETS_PTR(ss))[index] ) )
#endif /* DEBUG_BUILD */

/*****************************************************************************
 * Functions for set and span types
 *****************************************************************************/

/* Input and output functions for set and span types */

extern Set *set_in(const char *str, meosType basetype);
extern char *set_out(const Set *s, int maxdd);
extern Span *span_in(const char *str, meosType spantype);
extern char *span_out(const Span *s, int maxdd);
extern SpanSet *spanset_in(const char *str, meosType spantype);
extern char *spanset_out(const SpanSet *ss, int maxdd);

/*****************************************************************************/

/* Constructor functions for set and span types */

extern Set *set_make(const Datum *values, int count, meosType basetype, bool order);
extern Set *set_make_exp(const Datum *values, int count, int maxcount, meosType basetype, bool order);
extern Set *set_make_free(Datum *values, int count, meosType basetype, bool order);
extern Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype);
extern void span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype, meosType spantype, Span *s);
extern SpanSet *spanset_make_exp(Span *spans, int count, int maxcount, bool normalize, bool order);
extern SpanSet *spanset_make_free(Span *spans, int count, bool normalize, bool order);

/*****************************************************************************/

/* Conversion functions for set and span types */

extern void value_set_span(Datum value, meosType basetype, Span *s);
extern Set *value_set(Datum d, meosType basetype);
extern Span *value_span(Datum d, meosType basetype);
extern SpanSet *value_spanset(Datum d, meosType basetype);

/*****************************************************************************/

/* Accessor functions for set and span types */

extern Datum numspan_width(const Span *s);
extern Datum numspanset_width(const SpanSet *ss, bool boundspan);
extern Datum set_end_value(const Set *s);
extern int set_mem_size(const Set *s);
extern void set_set_subspan(const Set *s, int minidx, int maxidx, Span *result);
extern void set_set_span(const Set *s, Span *result);
extern Datum set_start_value(const Set *s);
extern bool set_value_n(const Set *s, int n, Datum *result);
extern Datum *set_vals(const Set *s);
extern Datum *set_values(const Set *s);
extern Datum spanset_lower(const SpanSet *ss);
extern int spanset_mem_size(const SpanSet *ss);
extern const Span **spanset_sps(const SpanSet *ss);
extern Datum spanset_upper(const SpanSet *ss);

/*****************************************************************************/

/* Transformation functions for set and span types */

extern void datespan_set_tstzspan(const Span *s1, Span *s2);
extern void floatspan_set_intspan(const Span *s1, Span *s2);
extern void intspan_set_floatspan(const Span *s1, Span *s2);
extern Set *numset_shift_scale(const Set *s, Datum shift, Datum width, bool hasshift, bool haswidth);
extern Span *numspan_shift_scale(const Span *s, Datum shift, Datum width, bool hasshift, bool haswidth);
extern SpanSet *numspanset_shift_scale(const SpanSet *ss, Datum shift, Datum width, bool hasshift, bool haswidth);
extern Set *set_compact(const Set *s);
extern void span_expand(const Span *s1, Span *s2);
extern SpanSet *spanset_compact(const SpanSet *ss);
extern Set *textcat_textset_text_int(const Set *s, const text *txt, bool invert);
extern void tstzspan_set_datespan(const Span *s1, Span *s2);

/*****************************************************************************
 * Comparison functions for set and span types
 *****************************************************************************/


/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Topological functions for set and span types */

extern bool adjacent_span_value(const Span *s, Datum value);
extern bool adjacent_spanset_value(const SpanSet *ss, Datum value);
extern bool adjacent_value_spanset(Datum value, const SpanSet *ss);
extern bool contained_value_set(Datum value, const Set *s);
extern bool contained_value_span(Datum value, const Span *s);
extern bool contained_value_spanset(Datum value, const SpanSet *ss);
extern bool contains_set_value(const Set *s, Datum value);
extern bool contains_span_value(const Span *s, Datum value);
extern bool contains_spanset_value(const SpanSet *ss, Datum value);
extern bool ovadj_span_span(const Span *s1, const Span *s2);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool left_set_value(const Set *s, Datum value);
extern bool left_span_value(const Span *s, Datum value);
extern bool left_spanset_value(const SpanSet *ss, Datum value);
extern bool left_value_set(Datum value, const Set *s);
extern bool left_value_span(Datum value, const Span *s);
extern bool left_value_spanset(Datum value, const SpanSet *ss);
extern bool lfnadj_span_span(const Span *s1, const Span *s2);
extern bool overleft_set_value(const Set *s, Datum value);
extern bool overleft_span_value(const Span *s, Datum value);
extern bool overleft_spanset_value(const SpanSet *ss, Datum value);
extern bool overleft_value_set(Datum value, const Set *s);
extern bool overleft_value_span(Datum value, const Span *s);
extern bool overleft_value_spanset(Datum value, const SpanSet *ss);
extern bool overright_set_value(const Set *s, Datum value);
extern bool overright_span_value(const Span *s, Datum value);
extern bool overright_spanset_value(const SpanSet *ss, Datum value);
extern bool overright_value_set(Datum value, const Set *s);
extern bool overright_value_span(Datum value, const Span *s);
extern bool overright_value_spanset(Datum value, const SpanSet *ss);
extern bool right_value_set(Datum value, const Set *s);
extern bool right_set_value(const Set *s, Datum value);
extern bool right_value_span(Datum value, const Span *s);
extern bool right_value_spanset(Datum value, const SpanSet *ss);
extern bool right_span_value(const Span *s, Datum value);
extern bool right_spanset_value(const SpanSet *ss, Datum value);

/*****************************************************************************/

/* Set functions for set and span types */

extern void bbox_union_span_span(const Span *s1, const Span *s2, Span *result);
extern bool inter_span_span(const Span *s1, const Span *s2, Span *result);
extern Set *intersection_set_value(const Set *s, Datum value);
extern Span *intersection_span_value(const Span *s, Datum value);
extern SpanSet *intersection_spanset_value(const SpanSet *ss, Datum value);
extern Set *intersection_value_set(Datum value, const Set *s);
extern Span *intersection_value_span(Datum value, const Span *s);
extern SpanSet *intersection_value_spanset(Datum value, const SpanSet *ss);
extern int mi_span_span(const Span *s1, const Span *s2, Span *result);
extern Set *minus_set_value(const Set *s, Datum value);
extern SpanSet *minus_span_value(const Span *s, Datum value);
extern SpanSet *minus_spanset_value(const SpanSet *ss, Datum value);
extern Set *minus_value_set(Datum value, const Set *s);
extern SpanSet *minus_value_span(Datum value, const Span *s);
extern SpanSet *minus_value_spanset(Datum value, const SpanSet *ss);
extern Span *super_union_span_span(const Span *s1, const Span *s2);
extern Set *union_set_value(const Set *s, const Datum value);
extern SpanSet *union_span_value(const Span *s, Datum value);
extern SpanSet *union_spanset_value(const SpanSet *ss, Datum value);
extern Set *union_value_set(const Datum value, const Set *s);
extern SpanSet *union_value_span(Datum value, const Span *s);
extern SpanSet *union_value_spanset(Datum value, const SpanSet *ss);

/*****************************************************************************/

/* Distance functions for set and span types */

extern Datum distance_set_set(const Set *s1, const Set *s2);
extern Datum distance_set_value(const Set *s, Datum value);
extern Datum distance_span_span(const Span *s1, const Span *s2);
extern Datum distance_span_value(const Span *s, Datum value);
extern Datum distance_spanset_span(const SpanSet *ss, const Span *s);
extern Datum distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2);
extern Datum distance_spanset_value(const SpanSet *ss, Datum value);
extern Datum distance_value_value(Datum l, Datum r, meosType basetype);

/*****************************************************************************/

/* Aggregate functions for set and span types */

extern Span *spanbase_extent_transfn(Span *state, Datum value, meosType basetype);
extern Set *value_union_transfn(Set *state, Datum value, meosType basetype);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Constructor functions for box types */

extern TBox *number_tstzspan_to_tbox(Datum d, meosType basetype, const Span *s);
extern TBox *number_timestamptz_to_tbox(Datum d, meosType basetype, TimestampTz t);
extern void stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *s, STBox *box);
extern void tbox_set(const Span *s, const Span *p, TBox *box);

/*****************************************************************************/

/* Conversion functions for box types */

extern STBox *box3d_stbox(const BOX3D *box);
extern STBox *gbox_stbox(const GBOX *box);
extern void float_set_tbox(double d, TBox *box);
extern void gbox_set_stbox(const GBOX *box, int32_t srid, STBox *result);
extern bool geo_set_stbox(const GSERIALIZED *gs, STBox *box);
extern void geoarr_set_stbox(const Datum *values, int count, STBox *box);
extern void int_set_tbox(int i, TBox *box);
extern void number_set_tbox(Datum d, meosType basetype, TBox *box);
extern TBox *number_tbox(Datum value, meosType basetype);
extern void numset_set_tbox(const Set *s, TBox *box);
extern void numspan_set_tbox(const Span *span, TBox *box);
extern void numspanset_set_tbox(const SpanSet *ss, TBox *box);
extern bool spatial_set_stbox(Datum d, meosType basetype, STBox *box);
extern void spatialset_set_stbox(const Set *set, STBox *box);
extern void stbox_set_box3d(const STBox *box, BOX3D *box3d);
extern void stbox_set_gbox(const STBox *box, GBOX *gbox);
extern void timestamptz_set_stbox(TimestampTz t, STBox *box);
extern void timestamptz_set_tbox(TimestampTz t, TBox *box);
extern void tstzset_set_stbox(const Set *s, STBox *box);
extern void tstzset_set_tbox(const Set *s, TBox *box);
extern void tstzspan_set_stbox(const Span *s, STBox *box);
extern void tstzspan_set_tbox(const Span *s, TBox *box);
extern void tstzspanset_set_stbox(const SpanSet *ss, STBox *box);
extern void tstzspanset_set_tbox(const SpanSet *ss, TBox *box);

/*****************************************************************************/

/* Transformation functions for box types */

extern TBox *tbox_shift_scale_value(const TBox *box, Datum shift, Datum width, bool hasshift, bool haswidth);
extern void stbox_expand(const STBox *box1, STBox *box2);
extern void tbox_expand(const TBox *box1, TBox *box2);

/*****************************************************************************/

/* Set functions for box types */

extern bool inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result);
extern bool inter_tbox_tbox(const TBox *box1, const TBox *box2, TBox *result);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Input and output functions for temporal types */

extern char *tboolinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *tboolinst_from_mfjson(json_object *mfjson);
extern TInstant *tboolinst_in(const char *str);
extern char *tboolseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *tboolseq_from_mfjson(json_object *mfjson);
extern TSequence *tboolseq_in(const char *str, interpType interp);
extern char *tboolseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *tboolseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tboolseqset_in(const char *str);
extern Temporal *temporal_in(const char *str, meosType temptype);
extern char *temporal_out(const Temporal *temp, int maxdd);
extern char **temparr_out(const Temporal **temparr, int count, int maxdd);
extern char *tfloatinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision);
extern TInstant *tfloatinst_from_mfjson(json_object *mfjson);
extern TInstant *tfloatinst_in(const char *str);
extern char *tfloatseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision);
extern TSequence *tfloatseq_from_mfjson(json_object *mfjson, interpType interp);
extern TSequence *tfloatseq_in(const char *str, interpType interp);
extern char *tfloatseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision);
extern TSequenceSet *tfloatseqset_from_mfjson(json_object *mfjson, interpType interp);
extern TSequenceSet *tfloatseqset_in(const char *str);
extern char *tgeoinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern TInstant *tgeogpointinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeogpointinst_in(const char *str);
extern TSequence *tgeogpointseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeogpointseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeogpointseqset_in(const char *str);
extern TInstant *tgeompointinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeompointinst_in(const char *str);
extern TSequence *tgeompointseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeompointseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeompointseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeompointseqset_in(const char *str);
extern TInstant *tgeographyinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeographyinst_in(const char *str);
extern TSequence *tgeographyseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeographyseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeographyseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeographyseqset_in(const char *str);
extern TInstant *tgeometryinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeometryinst_in(const char *str);
extern TSequence *tgeometryseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeometryseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeometryseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeometryseqset_in(const char *str);
extern char *tgeoseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern char *tgeoseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TInstant *tinstant_from_mfjson(json_object *mfjson, bool isgeo, int32_t srid, meosType temptype);
extern TInstant *tinstant_in(const char *str, meosType temptype);
extern char *tinstant_out(const TInstant *inst, int maxdd);
extern char *tintinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *tintinst_from_mfjson(json_object *mfjson);
extern TInstant *tintinst_in(const char *str);
extern char *tintseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *tintseq_from_mfjson(json_object *mfjson);
extern TSequence *tintseq_in(const char *str, interpType interp);
extern char *tintseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *tintseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tintseqset_in(const char *str);
extern char *tpointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern char *tpointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern char *tpointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TSequence *tsequence_from_mfjson(json_object *mfjson, bool isgeo, int32_t srid, meosType temptype, interpType interp);
extern TSequence *tsequence_in(const char *str, meosType temptype, interpType interp);
extern char *tsequence_out(const TSequence *seq, int maxdd);
extern TSequenceSet *tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int32_t srid, meosType temptype, interpType interp);
extern TSequenceSet *tsequenceset_in(const char *str, meosType temptype, interpType interp);
extern char *tsequenceset_out(const TSequenceSet *ss, int maxdd);
extern char *ttextinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *ttextinst_from_mfjson(json_object *mfjson);
extern TInstant *ttextinst_in(const char *str);
extern char *ttextseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *ttextseq_from_mfjson(json_object *mfjson);
extern TSequence *ttextseq_in(const char *str, interpType interp);
extern char *ttextseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *ttextseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *ttextseqset_in(const char *str);
extern Temporal *temporal_from_mfjson(const char *mfjson, meosType temptype);

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *temporal_from_base_temp(Datum value, meosType temptype, const Temporal *temp);
extern TInstant *tinstant_copy(const TInstant *inst);
extern TInstant *tinstant_make(Datum value, meosType temptype, TimestampTz t);
extern TInstant *tinstant_make_free(Datum value, meosType temptype, TimestampTz t);
extern TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords, const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_copy(const TSequence *seq);
extern TSequence *tsequence_from_base_tstzset(Datum value, meosType temptype, const Set *ss);
extern TSequence *tsequence_from_base_tstzspan(Datum value, meosType temptype, const Span *s, interpType interp);
extern TSequence *tsequence_make_exp(const TInstant **instants, int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_make_free(TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_copy(const TSequenceSet *ss);
extern TSequenceSet *tseqsetarr_to_tseqset(TSequenceSet **seqsets, int count, int totalseqs);
extern TSequenceSet *tsequenceset_from_base_tstzspanset(Datum value, meosType temptype, const SpanSet *ss, interpType interp);
extern TSequenceSet *tsequenceset_make_exp(const TSequence **sequences, int count, int maxcount, bool normalize);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);

/*****************************************************************************/

/* Conversion functions for temporal types */

extern void temporal_set_tstzspan(const Temporal *temp, Span *s);
extern void tinstant_set_tstzspan(const TInstant *inst, Span *s);
extern void tnumber_set_tbox(const Temporal *temp, TBox *box);
extern Span *tnumber_span(const Temporal *temp);
extern void tnumberinst_set_tbox(const TInstant *inst, TBox *box);
extern void tnumberseq_set_tbox(const TSequence *seq, TBox *box);
extern void tnumberseqset_set_tbox(const TSequenceSet *ss, TBox *box);
extern void tsequence_set_tstzspan(const TSequence *seq, Span *s);
extern void tsequenceset_set_tstzspan(const TSequenceSet *ss, Span *s);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern const TInstant *temporal_end_inst(const Temporal *temp);
extern Datum temporal_end_value(const Temporal *temp);
extern const TInstant *temporal_inst_n(const Temporal *temp, int n);
extern const TInstant **temporal_instants_p(const Temporal *temp, int *count);
extern Datum temporal_max_value(const Temporal *temp);
extern size_t temporal_mem_size(const Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern const TSequence **temporal_sequences_p(const Temporal *temp, int *count);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern const TInstant *temporal_start_inst(const Temporal *temp);
extern Datum temporal_start_value(const Temporal *temp);
extern Datum *temporal_values_p(const Temporal *temp, int *count);
extern bool temporal_value_n(const Temporal *temp, int n, Datum *result);
extern Datum *temporal_values(const Temporal *temp, int *count);
extern uint32 tinstant_hash(const TInstant *inst);
extern const TInstant **tinstant_insts(const TInstant *inst, int *count);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern SpanSet *tinstant_time(const TInstant *inst);
extern TimestampTz *tinstant_timestamps(const TInstant *inst, int *count);
extern Datum tinstant_value_p(const TInstant *inst);
extern Datum tinstant_value(const TInstant *inst);
extern bool tinstant_value_at_timestamptz(const TInstant *inst, TimestampTz t, Datum *result);
extern Datum *tinstant_values_p(const TInstant *inst, int *count);
extern void tnumber_set_span(const Temporal *temp, Span *span);
extern SpanSet *tnumberinst_valuespans(const TInstant *inst);
extern SpanSet *tnumberseq_valuespans(const TSequence *seq);
extern SpanSet *tnumberseqset_valuespans(const TSequenceSet *ss);
extern Interval *tsequence_duration(const TSequence *seq);
extern TimestampTz tsequence_end_timestamptz(const TSequence *seq);
extern uint32 tsequence_hash(const TSequence *seq);
extern const TInstant **tsequence_insts_p(const TSequence *seq);
extern const TInstant *tsequence_max_inst(const TSequence *seq);
extern Datum tsequence_max_val(const TSequence *seq);
extern const TInstant *tsequence_min_inst(const TSequence *seq);
extern Datum tsequence_min_val(const TSequence *seq);
extern TSequence **tsequence_segments(const TSequence *seq, int *count);
extern const TSequence **tsequence_seqs(const TSequence *seq, int *count);
extern TimestampTz tsequence_start_timestamptz(const TSequence *seq);
extern SpanSet *tsequence_time(const TSequence *seq);
extern TimestampTz *tsequence_timestamps(const TSequence *seq, int *count);
extern bool tsequence_value_at_timestamptz(const TSequence *seq, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequence_values_p(const TSequence *seq, int *count);
extern Interval *tsequenceset_duration(const TSequenceSet *ss, bool boundspan);
extern TimestampTz tsequenceset_end_timestamptz(const TSequenceSet *ss);
extern uint32 tsequenceset_hash(const TSequenceSet *ss);
extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
extern const TInstant **tsequenceset_insts_p(const TSequenceSet *ss);
extern const TInstant *tsequenceset_max_inst(const TSequenceSet *ss);
extern Datum tsequenceset_max_val(const TSequenceSet *ss);
extern const TInstant *tsequenceset_min_inst(const TSequenceSet *ss);
extern Datum tsequenceset_min_val(const TSequenceSet *ss);
extern int tsequenceset_num_instants(const TSequenceSet *ss);
extern int tsequenceset_num_timestamps(const TSequenceSet *ss);
extern TSequence **tsequenceset_segments(const TSequenceSet *ss, int *count);
extern const TSequence **tsequenceset_sequences_p(const TSequenceSet *ss);
extern TimestampTz tsequenceset_start_timestamptz(const TSequenceSet *ss);
extern SpanSet *tsequenceset_time(const TSequenceSet *ss);
extern bool tsequenceset_timestamptz_n(const TSequenceSet *ss, int n, TimestampTz *result);
extern TimestampTz *tsequenceset_timestamps(const TSequenceSet *ss, int *count);
extern bool tsequenceset_value_at_timestamptz(const TSequenceSet *ss, TimestampTz t, bool strict, Datum *result);
extern bool tsequenceset_value_n(const TSequenceSet *ss, int n, Datum *result);
extern Datum *tsequenceset_values_p(const TSequenceSet *ss, int *count);

/*****************************************************************************/

/* Transformation functions for temporal types */

extern void temporal_restart(Temporal *temp, int count);
extern TSequence *temporal_tsequence(const Temporal *temp, interpType interp);
extern TSequenceSet *temporal_tsequenceset(const Temporal *temp, interpType interp);
extern TInstant *tinstant_shift_time(const TInstant *inst, const Interval *interv);
extern TSequence *tinstant_to_tsequence(const TInstant *inst, interpType interp);
extern TSequence *tinstant_to_tsequence_free(TInstant *inst, interpType interp);
extern TSequenceSet *tinstant_to_tsequenceset(const TInstant *inst, interpType interp);
extern Temporal *tnumber_shift_scale_value(const Temporal *temp, Datum shift, Datum width, bool hasshift, bool haswidth);
extern TInstant *tnumberinst_shift_value(const TInstant *inst, Datum shift);
extern TSequence *tnumberseq_shift_scale_value(const TSequence *seq, Datum shift, Datum width, bool hasshift, bool haswidth);
extern TSequenceSet *tnumberseqset_shift_scale_value(const TSequenceSet *ss, Datum start, Datum width, bool hasshift, bool haswidth);
extern void tsequence_restart(TSequence *seq, int count);
extern Temporal *tsequence_set_interp(const TSequence *seq, interpType interp);
extern TSequence *tsequence_shift_scale_time(const TSequence *seq, const Interval *shift, const Interval *duration);
extern TSequence *tsequence_subseq(const TSequence *seq, int from, int to, bool lower_inc, bool upper_inc);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tsequence_to_tsequenceset_free(TSequence *seq);
extern TSequenceSet *tsequence_to_tsequenceset_interp(const TSequence *seq, interpType interp);
extern void tsequenceset_restart(TSequenceSet *ss, int count);
extern Temporal *tsequenceset_set_interp(const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tsequenceset_shift_scale_time(const TSequenceSet *ss, const Interval *start, const Interval *duration);
extern TSequence *tsequenceset_to_discrete(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_to_linear(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_to_step(const TSequenceSet *ss);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ss);
extern TSequence *tsequenceset_to_tsequence(const TSequenceSet *ss);

/*****************************************************************************/

/* Modification functions for temporal types */

extern Temporal *tinstant_merge(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge_array(const TInstant **instants, int count);
extern Temporal *tsequence_append_tinstant(TSequence *seq, const TInstant *inst, double maxdist, const Interval *maxt, bool expand);
extern Temporal *tsequence_append_tsequence(const TSequence *seq1, const TSequence *seq2, bool expand);
extern Temporal *tsequence_delete_timestamptz(const TSequence *seq, TimestampTz t, bool connect);
extern Temporal *tsequence_delete_tstzset(const TSequence *seq, const Set *s, bool connect);
extern Temporal *tsequence_delete_tstzspan(const TSequence *seq, const Span *s, bool connect);
extern Temporal *tsequence_delete_tstzspanset(const TSequence *seq, const SpanSet *ss, bool connect);
extern Temporal *tsequence_insert(const TSequence *seq1, const TSequence *seq2, bool connect);
extern Temporal *tsequence_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tsequence_merge_array(const TSequence **sequences, int count);
extern TSequenceSet *tsequenceset_append_tinstant(TSequenceSet *ss, const TInstant *inst, double maxdist, const Interval *maxt, bool expand);
extern TSequenceSet *tsequenceset_append_tsequence(TSequenceSet *ss, const TSequence *seq, bool expand);
extern TSequenceSet *tsequenceset_delete_timestamptz(const TSequenceSet *ss, TimestampTz t);
extern TSequenceSet *tsequenceset_delete_tstzset(const TSequenceSet *ss, const Set *s);
extern TSequenceSet *tsequenceset_delete_tstzspan(const TSequenceSet *ss, const Span *s);
extern TSequenceSet *tsequenceset_delete_tstzspanset(const TSequenceSet *ss, const SpanSet *ps);
extern TSequenceSet *tsequenceset_insert(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern TSequenceSet *tsequenceset_merge_array(const TSequenceSet **seqsets, int count);

/*****************************************************************************/

/* Bounding box functions for temporal types */

extern void tspatial_set_stbox(const Temporal *temp, STBox *box);
extern void tgeoinst_set_stbox(const TInstant *inst, STBox *box);
extern void tspatialseq_set_stbox(const TSequence *seq, STBox *box);
extern void tspatialseqset_set_stbox(const TSequenceSet *ss, STBox *box);
extern void tsequence_expand_bbox(TSequence *seq, const TInstant *inst);
extern void tsequence_set_bbox(const TSequence *seq, void *box);
extern void tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq);
extern void tsequenceset_set_bbox(const TSequenceSet *ss, void *box);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern TSequence *tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TSequenceSet *tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern bool temporal_bbox_restrict_set(const Temporal *temp, const Set *set);
extern Temporal *temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc);
extern Temporal *temporal_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc);
extern Temporal *temporal_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc);
extern Temporal *temporal_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss, bool atfunc);
extern Temporal *temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, const Set *set, bool atfunc);
extern bool temporal_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, Datum *result);
extern TInstant *tinstant_restrict_tstzspan(const TInstant *inst, const Span *period, bool atfunc);
extern TInstant *tinstant_restrict_tstzspanset(const TInstant *inst, const SpanSet *ss, bool atfunc);
extern TInstant *tinstant_restrict_timestamptz(const TInstant *inst, TimestampTz t, bool atfunc);
extern TInstant *tinstant_restrict_tstzset(const TInstant *inst, const Set *s, bool atfunc);
extern TInstant *tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc);
extern TInstant *tinstant_restrict_values(const TInstant *inst, const Set *set, bool atfunc);
extern Temporal *tgeo_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern Temporal *tgeo_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc, bool atfunc);
extern TInstant *tgeoinst_restrict_geom(const TInstant *inst, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern TInstant *tgeoinst_restrict_stbox(const TInstant *inst, const STBox *box, bool border_inc, bool atfunc);
extern Temporal *tgeoseq_restrict_geom(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern Temporal *tgeotseq_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern TSequenceSet *tgeoseqset_restrict_geom(const TSequenceSet *ss, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern TSequenceSet *tgeoseqset_restrict_stbox(const TSequenceSet *ss, const STBox *box, bool border_inc, bool atfunc);
extern Temporal *tnumber_restrict_span(const Temporal *temp, const Span *span, bool atfunc);
extern Temporal *tnumber_restrict_spanset(const Temporal *temp, const SpanSet *ss, bool atfunc);
extern TInstant *tnumberinst_restrict_span(const TInstant *inst, const Span *span, bool atfunc);
extern TInstant *tnumberinst_restrict_spanset(const TInstant *inst, const SpanSet *ss, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_span(const TSequenceSet *ss, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_spanset(const TSequenceSet *ss, const SpanSet *spanset, bool atfunc);
extern TInstant *tsequence_at_timestamptz(const TSequence *seq, TimestampTz t);
extern Temporal *tsequence_restrict_tstzspan(const TSequence *seq, const Span *s, bool atfunc);
extern Temporal *tsequence_restrict_tstzspanset(const TSequence *seq, const SpanSet *ss, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ss, bool min, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_tstzspan(const TSequenceSet *ss, const Span *s, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_tstzspanset(const TSequenceSet *ss, const SpanSet *ps, bool atfunc);
extern Temporal *tsequenceset_restrict_timestamptz(const TSequenceSet *ss, TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_tstzset(const TSequenceSet *ss, const Set *s, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ss, Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ss, const Set *s, bool atfunc);

/*****************************************************************************/

/* Traditional comparison functions for temporal types */

extern int tinstant_cmp(const TInstant *inst1, const TInstant *inst2);
extern bool tinstant_eq(const TInstant *inst1, const TInstant *inst2);
extern int tsequence_cmp(const TSequence *seq1, const TSequence *seq2);
extern bool tsequence_eq(const TSequence *seq1, const TSequence *seq2);
extern int tsequenceset_cmp(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern bool tsequenceset_eq(const TSequenceSet *ss1, const TSequenceSet *ss2);

/*****************************************************************************/

/* Ever/always functions for temporal types */

extern int always_eq_base_temporal(Datum value, const Temporal *temp);
extern int always_eq_temporal_base(const Temporal *temp, Datum value);
extern int always_ne_base_temporal(Datum value, const Temporal *temp);
extern int always_ne_temporal_base(const Temporal *temp, Datum value);
extern int always_ge_base_temporal(Datum value, const Temporal *temp);
extern int always_ge_temporal_base(const Temporal *temp, Datum value);
extern int always_gt_base_temporal(Datum value, const Temporal *temp);
extern int always_gt_temporal_base(const Temporal *temp, Datum value);
extern int always_le_base_temporal(Datum value, const Temporal *temp);
extern int always_le_temporal_base(const Temporal *temp, Datum value);
extern int always_lt_base_temporal(Datum value, const Temporal *temp);
extern int always_lt_temporal_base(const Temporal *temp, Datum value);
extern int ever_eq_base_temporal(Datum value, const Temporal *temp);
extern int ever_eq_temporal_base(const Temporal *temp, Datum value);
extern int ever_ne_base_temporal(Datum value, const Temporal *temp);
extern int ever_ne_temporal_base(const Temporal *temp, Datum value);
extern int ever_ge_base_temporal(Datum value, const Temporal *temp);
extern int ever_ge_temporal_base(const Temporal *temp, Datum value);
extern int ever_gt_base_temporal(Datum value, const Temporal *temp);
extern int ever_gt_temporal_base(const Temporal *temp, Datum value);
extern int ever_le_base_temporal(Datum value, const Temporal *temp);
extern int ever_le_temporal_base(const Temporal *temp, Datum value);
extern int ever_lt_base_temporal(Datum value, const Temporal *temp);
extern int ever_lt_temporal_base(const Temporal *temp, Datum value);

/*****************************************************************************/

/* Mathematical functions for temporal types */

extern TSequence *tfloatseq_derivative(const TSequence *seq);
extern TSequenceSet *tfloatseqset_derivative(const TSequenceSet *ss);
extern TInstant *tnumberinst_abs(const TInstant *inst);
extern TSequence *tnumberseq_abs(const TSequence *seq);
extern TSequence *tnumberseq_angular_difference(const TSequence *seq);
extern TSequence *tnumberseq_delta_value(const TSequence *seq);
extern TSequenceSet *tnumberseqset_abs(const TSequenceSet *ss);
extern TSequence *tnumberseqset_angular_difference(const TSequenceSet *ss);
extern TSequenceSet *tnumberseqset_delta_value(const TSequenceSet *ss);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value);
extern Datum nad_tbox_tbox(const TBox *box1, const TBox *box2);
extern Datum nad_tnumber_number(const Temporal *temp, Datum value);
extern Datum nad_tnumber_tbox(const Temporal *temp, const TBox *box);
extern Datum nad_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern int32_t spatial_srid(Datum d, meosType basetype);
extern bool spatial_has_z(Datum d, meosType basetype);
extern bool spatial_is_geodetic(Datum d, meosType basetype);
extern bool spatial_set_srid(Datum d, meosType basetype, int32_t srid);
extern int tspatialinst_srid(const TInstant *inst);
extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq, double prevlength);
extern bool tpointseq_is_simple(const TSequence *seq);
extern double tpointseq_length(const TSequence *seq);
extern GSERIALIZED *tpointseq_linear_trajectory(const TSequence *seq, bool unary_union);
extern TSequence *tpointseq_speed(const TSequence *seq);
extern STBox *tgeoseq_stboxes(const TSequence *seq, int *count);
extern STBox *tpointseq_split_n_stboxes(const TSequence *seq, int max_count, int *count);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ss);
extern bool tpointseqset_is_simple(const TSequenceSet *ss);
extern double tpointseqset_length(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ss);
extern STBox *tgeoseqset_stboxes(const TSequenceSet *ss, int *count);
extern STBox *tpointseqset_split_n_stboxes(const TSequenceSet *ss, int max_count, int *count);
extern GSERIALIZED *tpointseqset_trajectory(const TSequenceSet *ss);
extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern TInstant *tgeominst_tgeoginst(const TInstant *inst, bool oper);
extern TSequence *tgeomseq_tgeogseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeomseqset_tgeogseqset(const TSequenceSet *ss, bool oper);
extern Temporal *tgeom_tgeog(const Temporal *temp, bool oper);
extern Temporal *tgeo_tpoint(const Temporal *temp, bool oper);
extern Temporal *tgeompoint_tnpoint(const Temporal *temp);
extern Temporal *tnpoint_tgeompoint(const Temporal *temp);
extern void tspatialinst_set_srid(TInstant *inst, int32 srid);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern void tpointseq_set_srid(TSequence *seq, int32 srid);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ss, int *count);
extern void tpointseqset_set_srid(TSequenceSet *ss, int32 srid);

/*****************************************************************************/

/* Local aggregate functions for temporal types */

extern double tnumberseq_integral(const TSequence *seq);
extern double tnumberseq_twavg(const TSequence *seq);
extern double tnumberseqset_integral(const TSequenceSet *ss);
extern double tnumberseqset_twavg(const TSequenceSet *ss);
extern GSERIALIZED *tpointseq_twcentroid(const TSequence *seq);
extern GSERIALIZED *tpointseqset_twcentroid(const TSequenceSet *ss);

/*****************************************************************************/

/* Compact functions for final append aggregate */

extern Temporal *temporal_compact(const Temporal *temp);
extern TSequence *tsequence_compact(const TSequence *seq);
extern TSequenceSet *tsequenceset_compact(const TSequenceSet *ss);

/*****************************************************************************/

/* Aggregate functions for temporal types */

extern void skiplist_free(SkipList *list);
extern Temporal *temporal_app_tinst_transfn(Temporal *state, const TInstant *inst, interpType interp, double maxdist, const Interval *maxt);
extern Temporal *temporal_app_tseq_transfn(Temporal *state, const TSequence *seq);

/*****************************************************************************/

/* Tile functions for span and temporal types */

extern Span *numspanset_spans(const SpanSet *ss, Datum vsize, Datum vorigin, int *count);
extern Span *spanset_time_spans(const SpanSet *ss, const Interval *duration, Datum torigin, int *count);
extern Span *spanset_value_spans(const SpanSet *ss, Datum vsize, Datum vorigin, int *count);
extern Span *timespanset_spans(const SpanSet *ss, const Interval *duration, Datum torigin, int *count);
extern Span *tnumber_value_spans(const Temporal *temp, Datum size, Datum origin, int *count);
extern TBox *tnumber_value_boxes(const Temporal *temp, Datum vsize, Datum vorigin, int *count);
extern TBox *tnumber_time_boxes(const Temporal *temp, const Interval *duration, TimestampTz torigin, int *count);
extern TBox *tnumber_value_time_boxes(const Temporal *temp, Datum vsize, const Interval *duration, Datum vorigin, TimestampTz torigin, int *count);
extern Temporal **tnumber_value_split(const Temporal *temp, Datum vsize, Datum vorigin, Datum **bins, int *count);
extern TBox *tbox_get_value_time_tile(Datum value, TimestampTz t, Datum vsize, const Interval *duration, Datum vorigin, TimestampTz torigin, meosType basetype, meosType spantype);
extern Temporal **tnumber_value_time_split(const Temporal *temp, Datum size, const Interval *duration, Datum vorigin, TimestampTz torigin, Datum **value_bins, TimestampTz **time_bins, int *count);

/*****************************************************************************/

/* Similarity functions for temporal types */


/*****************************************************************************/

#endif /* __MEOS_INTERNAL_H__ */
