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

/* JSON-C */
#include <json-c/json.h>
/* PostgreSQL */
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h" /* For meosType */

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
 * Macros for manipulating the 'flags' element where the less significant
 * bits are GTZXIICB, where
 *   G: coordinates are geodetic
 *   T: has T coordinate,
 *   Z: has Z coordinate
 *   X: has value or X coordinate
 *   II: interpolation, whose values are
 *   - 00: INTERP_NONE (undetermined) for TInstant
 *   - 01: DISCRETE
 *   - 10: STEP
 *   - 11: LINEAR
 *   C: continuous base type / Ordered collection
 *   B: base type passed by value
 * Notice that formally speaking the interpolation flags are only needed
 * for sequence and sequence set subtypes.
 *****************************************************************************/

/* The following flag is only used for Collection and TInstant */
#define MEOS_FLAG_BYVAL      0x0001  // 1
/* The following flag is only used for Collection */
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

#define MEOS_FLAGS_GET_BYVAL(flags)      ((bool) (((flags) & MEOS_FLAG_BYVAL)))
#define MEOS_FLAGS_GET_ORDERED(flags)    ((bool) (((flags) & MEOS_FLAG_ORDERED)>>1))
#define MEOS_FLAGS_GET_CONTINUOUS(flags) ((bool) (((flags) & MEOS_FLAG_CONTINUOUS)>>1))
#define MEOS_FLAGS_GET_X(flags)          ((bool) (((flags) & MEOS_FLAG_X)>>4))
#define MEOS_FLAGS_GET_Z(flags)          ((bool) (((flags) & MEOS_FLAG_Z)>>5))
#define MEOS_FLAGS_GET_T(flags)          ((bool) (((flags) & MEOS_FLAG_T)>>6))
#define MEOS_FLAGS_GET_GEODETIC(flags)   ((bool) (((flags) & MEOS_FLAG_GEODETIC)>>7))

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

#define MEOS_FLAGS_GET_INTERP(flags) (((flags) & MEOS_FLAGS_INTERP) >> 2)
#define MEOS_FLAGS_SET_INTERP(flags, value) ((flags) = (((flags) & ~MEOS_FLAGS_INTERP) | ((value & 0x0003) << 2)))

#define MEOS_FLAGS_GET_DISCRETE(flags)   ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == DISCRETE))
#define MEOS_FLAGS_GET_STEP(flags)       ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == STEP))
#define MEOS_FLAGS_GET_LINEAR(flags)     ((bool) (MEOS_FLAGS_GET_INTERP((flags)) == LINEAR))

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#if MEOS
#define TimestampTzGetDatum(X) Int64GetDatum(X)
#define DatumGetTimestampTz(X)((TimestampTz) DatumGetInt64(X))
#endif /* MEOS */

/*****************************************************************************
 * Functions for set and span types
 *****************************************************************************/

/* Macros for speeding up access to component values of sets*/

#ifdef DEBUG_BUILD
extern void *SET_BBOX_PTR(const Set *s);
extern size_t *SET_OFFSETS_PTR(const Set *s);
extern Datum SET_VAL_N(const Set *s, int index);
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
#endif

/*****************************************************************************/

/* Generic type functions */

extern uint32 datum_hash(Datum d, meosType basetype);
extern uint64 datum_hash_extended(Datum d, meosType basetype, uint64 seed);

/*****************************************************************************/

/* Input/output functions for set and span types */

extern Set *set_in(const char *str, meosType basetype);
extern char *set_out(const Set *s, int maxdd);
extern Span *span_in(const char *str, meosType spantype);
extern char *span_out(const Span *s, int maxdd);
extern SpanSet *spanset_in(const char *str, meosType spantype);
extern char *spanset_out(const SpanSet *ss, int maxdd);

/*****************************************************************************/

/* Constructor functions for set and span types */

extern Set *set_make(const Datum *values, int count, meosType basetype, bool ordered);
extern Set *set_make_exp(const Datum *values, int count, int maxcount, meosType basetype, bool ordered);
extern Set *set_make_free(Datum *values, int count, meosType basetype, bool ordered);
extern Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype);
extern void span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype, Span *s);
extern SpanSet *spanset_make_free(Span *spans, int count, bool normalize);

/*****************************************************************************/

/* Cast functions for set and span types */

extern Set *value_to_set(Datum d, meosType basetype);
extern Span *value_to_span(Datum d, meosType basetype);
extern SpanSet *value_to_spanset(Datum d, meosType basetype);

/*****************************************************************************/

/* Accessor functions for set and span types */

extern Datum set_end_value(const Set *s);
extern int set_mem_size(const Set *s);
extern void set_set_span(const Set *os, Span *s);
extern Datum set_start_value(const Set *s);
extern bool set_value_n(const Set *s, int n, Datum *result);
extern Datum *set_values(const Set *s);
extern int spanset_mem_size(const SpanSet *ss);
extern void spatialset_set_stbox(const Set *set, STBox *box);
extern void value_set_span(Datum d, meosType basetype, Span *s);

/*****************************************************************************/

/* Transformation functions for set and span types */

extern void floatspan_set_intspan(const Span *s1, Span *s2);
extern void floatspan_set_numspan(const Span *s1, Span *s2, meosType basetype);
extern void intspan_set_floatspan(const Span *s1, Span *s2);
extern void numspan_set_floatspan(const Span *s1, Span *s2);
extern Set *set_shift(const Set *s, Datum shift);
extern void span_shift(Span *s, Datum value);
extern void spanset_shift(SpanSet *s, Datum value);

/*****************************************************************************/

/* Aggregate functions for set and span types */

extern Span *spanbase_extent_transfn(Span *s, Datum d, meosType basetype);
extern Set *value_union_transfn(Set *state, Datum d, meosType basetype);

/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Topological functions for set and span types */

extern bool adjacent_span_value(const Span *s, Datum d, meosType basetype);
extern bool adjacent_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool contains_span_value(const Span *s, Datum d, meosType basetype);
extern bool contains_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool contains_set_value(const Set *s, Datum d, meosType basetype);
extern bool contains_set_set(const Set *s1, const Set *s2);
extern bool contained_value_span(Datum d, meosType basetype, const Span *s);
extern bool contained_value_set(Datum d, meosType basetype, const Set *s);
extern bool contained_set_set(const Set *s1, const Set *s2);
extern bool contained_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overlaps_value_span(Datum d, meosType basetype, const Span *s);
extern bool overlaps_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overlaps_span_value(const Span *s, Datum d, meosType basetype);
extern bool overlaps_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overlaps_set_set(const Set *s1, const Set *s2);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool left_set_set(const Set *s1, const Set *s2);
extern bool left_set_value(const Set *s, Datum d, meosType basetype);
extern bool left_span_value(const Span *s, Datum d, meosType basetype);
extern bool left_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool left_value_set(Datum d, meosType basetype, const Set *s);
extern bool left_value_span(Datum d, meosType basetype, const Span *s);
extern bool left_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool right_value_set(Datum d, meosType basetype, const Set *s);
extern bool right_set_value(const Set *s, Datum d, meosType basetype);
extern bool right_set_set(const Set *s1, const Set *s2);
extern bool right_value_span(Datum d, meosType basetype, const Span *s);
extern bool right_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool right_span_value(const Span *s, Datum d, meosType basetype);
extern bool right_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overleft_value_set(Datum d, meosType basetype, const Set *s);
extern bool overleft_set_value(const Set *s, Datum d, meosType basetype);
extern bool overleft_set_set(const Set *s1, const Set *s2);
extern bool overleft_value_span(Datum d, meosType basetype, const Span *s);
extern bool overleft_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overleft_span_value(const Span *s, Datum d, meosType basetype);
extern bool overleft_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overright_value_set(Datum d, meosType basetype, const Set *s);
extern bool overright_set_value(const Set *s, Datum d, meosType basetype);
extern bool overright_set_set(const Set *s1, const Set *s2);
extern bool overright_value_span(Datum d, meosType basetype, const Span *s);
extern bool overright_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overright_span_value(const Span *s, Datum d, meosType basetype);
extern bool overright_spanset_value(const SpanSet *ss, Datum d, meosType basetype);

/*****************************************************************************/

/* Set functions for set and span types */

extern bool inter_span_span(const Span *s1, const Span *s2, Span *result);
extern bool intersection_set_value(const Set *s, Datum d, meosType basetype, Datum *result);
extern bool intersection_span_value(const Span *s, Datum d, meosType basetype, Datum *result);
extern bool intersection_spanset_value(const SpanSet *ss, Datum d, meosType basetype, Datum *result);
extern Set *minus_set_value(const Set *s, Datum d, meosType basetype);
extern SpanSet *minus_span_value(const Span *s, Datum d, meosType basetype);
extern SpanSet *minus_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool minus_value_set(Datum d, meosType basetype, const Set *s, Datum *result);
extern bool minus_value_span(Datum d, meosType basetype, const Span *s, Datum *result);
extern bool minus_value_spanset(Datum d, meosType basetype, const SpanSet *ss, Datum *result);
extern Set *union_set_value(const Set *s, const Datum d, meosType basetype);
extern SpanSet *union_span_value(const Span *s, Datum v, meosType basetype);
extern SpanSet *union_spanset_value(const SpanSet *ss, Datum d, meosType basetype);

/*****************************************************************************/

/* Distance functions for set and span types */

extern double distance_value_value(Datum l, Datum r, meosType typel, meosType typer);
extern double distance_span_value(const Span *s, Datum d, meosType basetype);
extern double distance_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern double distance_value_set(Datum d, meosType basetype, const Set *s);
extern double distance_set_value(const Set *s, Datum d, meosType basetype);
extern double distance_set_set(const Set *s1, const Set *s2);

/*****************************************************************************/

/* Hash functions for set and span types */

extern uint32 datum_hash(Datum d, meosType basetype);
extern uint64 datum_hash_extended(Datum d, meosType basetype, uint64 seed);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Constructor functions for box types */

extern TBox *number_period_to_tbox(Datum d, meosType basetype, const Span *p);
extern TBox *number_timestamp_to_tbox(Datum d, meosType basetype, TimestampTz t);
extern void stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *p, STBox *box);
extern void tbox_set(const Span *s, const Span *p, TBox *box);

/*****************************************************************************/

/* Cast functions for box types */

extern void float_set_tbox(double d, TBox *box);
extern bool geo_set_stbox(const GSERIALIZED *gs, STBox *box);
extern void geoarr_set_stbox(const Datum *values, int count, STBox *box);
extern void int_set_tbox(int i, TBox *box);
extern void number_set_tbox(Datum d, meosType basetype, TBox *box);
extern void numset_set_tbox(const Set *s, TBox *box);
extern void numspan_set_tbox(const Span *span, TBox *box);
extern void numspanset_set_tbox(const SpanSet *ss, TBox *box);
extern void period_set_stbox(const Span *p, STBox *box);
extern void period_set_tbox(const Span *p, TBox *box);
extern void periodset_set_stbox(const SpanSet *ps, STBox *box);
extern void periodset_set_tbox(const SpanSet *ps, TBox *box);
extern void stbox_set_box3d(const STBox *box, BOX3D *box3d);
extern void stbox_set_gbox(const STBox *box, GBOX *gbox);
extern void timestamp_set_stbox(TimestampTz t, STBox *box);
extern void timestamp_set_tbox(TimestampTz t, TBox *box);
extern void timestampset_set_stbox(const Set *ts, STBox *box);
extern void timestampset_set_tbox(const Set *ts, TBox *box);

/*****************************************************************************/

/* Set functions for box types */

extern bool inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result);
extern bool inter_tbox_tbox(const TBox *box1, const TBox *box2, TBox *result);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Macros for speeding up access to components of temporal sequences (sets)*/

#ifdef DEBUG_BUILD
extern const TInstant *TSEQUENCE_INST_N(const TSequence *seq, int index);
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

/*****************************************************************************/

/* Input/output functions for temporal types */

extern char **geoarr_as_text(const Datum *geoarr, int count, int maxdd, bool extended);
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
extern char **temporalarr_out(const Temporal **temparr, int count, int maxdd);
extern char *tfloatinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision);
extern TInstant *tfloatinst_from_mfjson(json_object *mfjson);
extern TInstant *tfloatinst_in(const char *str);
extern char *tfloatseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision);
extern TSequence *tfloatseq_from_mfjson(json_object *mfjson, interpType interp);
extern TSequence *tfloatseq_in(const char *str, interpType interp);
extern char *tfloatseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision);
extern TSequenceSet *tfloatseqset_from_mfjson(json_object *mfjson, interpType interp);
extern TSequenceSet *tfloatseqset_in(const char *str);
extern char *tgeogpointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern TInstant *tgeogpointinst_from_mfjson(json_object *mfjson, int srid);
extern TInstant *tgeogpointinst_in(const char *str);
extern char *tgeogpointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern TSequence *tgeogpointseq_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequence *tgeogpointseq_in(const char *str, interpType interp);
extern char *tgeogpointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TSequenceSet *tgeogpointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequenceSet *tgeogpointseqset_in(const char *str);
extern char *tgeompointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern TInstant *tgeompointinst_from_mfjson(json_object *mfjson, int srid);
extern TInstant *tgeompointinst_in(const char *str);
extern char *tgeompointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern TSequence *tgeompointseq_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequence *tgeompointseq_in(const char *str, interpType interp);
extern char *tgeompointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TSequenceSet *tgeompointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequenceSet *tgeompointseqset_in(const char *str);
extern char *tinstant_as_mfjson(const TInstant *inst, int precision, bool with_bbox, char *srs);
extern TInstant *tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype);
extern TInstant *tinstant_in(const char *str, meosType temptype);
extern char *tinstant_out(const TInstant *inst, int maxdd);
extern TSequence *tdiscseq_in(const char *str, meosType temptype);
extern char *tintinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *tintinst_from_mfjson(json_object *mfjson);
extern TInstant *tintinst_in(const char *str);
extern char *tintseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *tintseq_from_mfjson(json_object *mfjson);
extern TSequence *tintseq_in(const char *str, interpType interp);
extern char *tintseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *tintseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tintseqset_in(const char *str);
extern char **tpointarr_as_text(const Temporal **temparr, int count, int maxdd, bool extended);
extern char *tsequence_as_mfjson(const TSequence *seq, int precision, bool with_bbox, char *srs);
extern TSequence *tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype, interpType interp);
extern TSequence *tsequence_in(const char *str, meosType temptype, interpType interp);
extern char *tsequence_out(const TSequence *seq, int maxdd);
extern char *tsequenceset_as_mfjson(const TSequenceSet *ss, int precision, bool with_bbox, char *srs);
extern TSequenceSet *tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype, interpType interp);
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

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *temporal_from_base_temp(Datum value, meosType temptype, const Temporal *temp);
extern TInstant *tinstant_copy(const TInstant *inst);
extern TInstant *tinstant_make(Datum value, meosType temptype, TimestampTz t);
extern TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords, const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *tsequence_from_base_timestampset(Datum value, meosType temptype, const Set *ss);
extern TSequence *tsequence_compact(const TSequence *seq);
extern void tsequence_restart(TSequence *seq, int last);
extern TSequence *tsequence_subseq(const TSequence *seq, int from, int to, bool lower_inc, bool upper_inc);
extern TSequence *tsequence_copy(const TSequence *seq);
extern TSequence *tsequence_from_base_temp(Datum value, meosType temptype, const TSequence *seq);
extern TSequence *tsequence_from_base_period(Datum value, meosType temptype, const Span *p, interpType interp);
extern TSequence *tsequence_make_free(TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequenceSet *tsequenceset_compact(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);
extern void tsequenceset_restart(TSequenceSet *ss, int last);
extern TSequenceSet *tsequenceset_copy(const TSequenceSet *ss);
extern TSequenceSet *tseqsetarr_to_tseqset(TSequenceSet **seqsets, int count, int totalseqs);
extern TSequenceSet *tsequenceset_from_base_temp(Datum value, meosType temptype, const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_from_base_periodset(Datum value, meosType temptype, const SpanSet *ps, interpType interp);

/*****************************************************************************/

/* Cast functions for temporal types */

extern void temporal_set_period(const Temporal *temp, Span *p);
extern TInstant *tfloatinst_to_tintinst(const TInstant *inst);
extern TSequence *tfloatseq_to_tintseq(const TSequence *seq);
extern TSequenceSet *tfloatseqset_to_tintseqset(const TSequenceSet *ss);
extern void tinstant_set_period(const TInstant *inst, Span *p);
extern TInstant *tintinst_to_tfloatinst(const TInstant *inst);
extern TSequence *tintseq_to_tfloatseq(const TSequence *seq);
extern TSequenceSet *tintseqset_to_tfloatseqset(const TSequenceSet *ss);
extern void tsequence_set_period(const TSequence *seq, Span *p);
extern void tsequenceset_set_period(const TSequenceSet *ss, Span *p);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern Datum temporal_end_value(const Temporal *temp);
extern Datum temporal_max_value(const Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern void tnumber_set_span(const Temporal *temp, Span *span);
extern Datum temporal_start_value(const Temporal *temp);
extern SpanSet *tnumberinst_valuespans(const TInstant *inst);
extern SpanSet *tnumberseq_valuespans(const TSequence *seq);
extern SpanSet *tnumberseqset_valuespans(const TSequenceSet *ss);
extern uint32 tinstant_hash(const TInstant *inst);
extern const TInstant **tinstant_instants(const TInstant *inst, int *count);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern SpanSet *tinstant_time(const TInstant *inst);
extern TimestampTz *tinstant_timestamps(const TInstant *inst, int *count);
extern Datum tinstant_value(const TInstant *inst);
extern bool tinstant_value_at_timestamp(const TInstant *inst, TimestampTz t, Datum *result);
extern Datum tinstant_value_copy(const TInstant *inst);
extern Datum *tinstant_values(const TInstant *inst, int *count);
extern uint32 tdiscseq_hash(const TSequence *seq);
extern bool tdiscseq_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result);
extern Interval *tsequence_duration(const TSequence *seq);
extern TimestampTz tsequence_end_timestamp(const TSequence *seq);
extern uint32 tsequence_hash(const TSequence *seq);
extern const TInstant **tsequence_instants(const TSequence *seq);
extern const TInstant *tsequence_max_instant(const TSequence *seq);
extern Datum tsequence_max_value(const TSequence *seq);
extern const TInstant *tsequence_min_instant(const TSequence *seq);
extern Datum tsequence_min_value(const TSequence *seq);
extern TSequence **tsequence_segments(const TSequence *seq, int *count);
extern TSequence **tsequence_sequences(const TSequence *seq, int *count);
extern void tsequence_set_bbox(const TSequence *seq, void *box);
extern void tsequence_expand_bbox(TSequence *seq, const TInstant *inst);
extern void tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq);
extern TimestampTz tsequence_start_timestamp(const TSequence *seq);
extern SpanSet *tsequence_time(const TSequence *seq);
extern TimestampTz *tsequence_timestamps(const TSequence *seq, int *count);
extern bool tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequence_values(const TSequence *seq, int *count);
extern Interval *tsequenceset_duration(const TSequenceSet *ss, bool boundspan);
extern TimestampTz tsequenceset_end_timestamp(const TSequenceSet *ss);
extern uint32 tsequenceset_hash(const TSequenceSet *ss);
extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
extern const TInstant **tsequenceset_instants(const TSequenceSet *ss);
extern const TInstant *tsequenceset_max_instant(const TSequenceSet *ss);
extern Datum tsequenceset_max_value(const TSequenceSet *ss);
extern const TInstant *tsequenceset_min_instant(const TSequenceSet *ss);
extern Datum tsequenceset_min_value(const TSequenceSet *ss);
extern int tsequenceset_num_instants(const TSequenceSet *ss);
extern int tsequenceset_num_timestamps(const TSequenceSet *ss);
extern TSequence **tsequenceset_segments(const TSequenceSet *ss, int *count);
extern TSequence **tsequenceset_sequences(const TSequenceSet *ss);
extern const TSequence **tsequenceset_sequences_p(const TSequenceSet *ss);
extern void tsequenceset_set_bbox(const TSequenceSet *ss, void *box);
extern TimestampTz tsequenceset_start_timestamp(const TSequenceSet *ss);
extern SpanSet *tsequenceset_time(const TSequenceSet *ss);
extern Interval *tsequenceset_timespan(const TSequenceSet *ss);
extern bool tsequenceset_timestamp_n(const TSequenceSet *ss, int n, TimestampTz *result);
extern TimestampTz *tsequenceset_timestamps(const TSequenceSet *ss, int *count);
extern bool tsequenceset_value_at_timestamp(const TSequenceSet *ss, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequenceset_values(const TSequenceSet *ss, int *count);

// RENAME
// extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
// extern const TInstant *tsequenceset_start_instant(const TSequenceSet *ss);
// extern const TInstant *tsequenceset_end_instant(const TSequenceSet *ss);

/*****************************************************************************/

/* Transformation functions for temporal types */

extern Temporal *tinstant_merge(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge_array(const TInstant **instants, int count);
extern TInstant *tinstant_shift(const TInstant *inst, const Interval *interval);
extern TSequence *tinstant_to_tsequence(const TInstant *inst, interpType interp);
extern TSequenceSet *tinstant_to_tsequenceset(const TInstant *inst, interpType interp);
extern Temporal *tdiscseq_set_interp(const TSequence *seq, interpType interp);
extern TSequence *tcontseq_to_discrete(const TSequence *seq);
extern Temporal *tcontseq_to_linear(const TSequence *seq);
extern TSequence *tcontseq_to_step(const TSequence *seq);
extern Temporal *tdiscseq_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tdiscseq_merge_array(const TSequence **sequences, int count);
extern TSequence *tdiscseq_to_tsequence(const TSequence *seq, interpType interp);
extern TSequenceSet *tdiscseq_to_tsequenceset(const TSequence *seq, interpType interp);
extern Temporal *tsequence_append_tinstant(TSequence *seq, const TInstant *inst, double maxdist, const Interval *maxt, bool expand);
extern Temporal *tsequence_append_tsequence(TSequence *seq1, const TSequence *seq2, bool expand);
extern Temporal *tsequence_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tsequence_merge_array(const TSequence **sequences, int count);
extern Temporal *tsequence_set_interp(const TSequence *seq, interpType interp);
extern TSequence *tsequence_shift_tscale(const TSequence *seq, const Interval *start, const Interval *duration);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TSequence *tsequence_to_tdiscseq(const TSequence *seq);
extern TSequence *tsequence_to_tcontseq(const TSequence *seq, interpType interp);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tsequenceset_append_tinstant(TSequenceSet *ss, const TInstant *inst, double maxdist, const Interval *maxt, bool expand);
extern TSequenceSet *tsequenceset_append_tsequence(TSequenceSet *ss, const TSequence *seq, bool expand);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern TSequenceSet *tsequenceset_merge_array(const TSequenceSet **seqsets, int count);
extern Temporal *tsequenceset_set_interp(const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tsequenceset_shift_tscale(const TSequenceSet *ss, const Interval *start, const Interval *duration);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ss);
extern TSequence *tsequenceset_to_discrete(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_to_step(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_to_linear(const TSequenceSet *ss);
extern TSequence *tsequenceset_to_tsequence(const TSequenceSet *ss);
extern TSequenceSet *tstepseq_to_linear(const TSequence *seq);
extern int tstepseq_to_linear_iter(const TSequence *seq, TSequence **result);
extern TSequenceSet *tstepseqset_to_linear(const TSequenceSet *ss);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern TSequence *tcontseq_at_period(const TSequence *seq, const Span *p);
extern TInstant *tcontseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tcontseq_at_timestampset(const TSequence *seq, const Set *ts);
extern TSequenceSet *tcontseq_minus_period(const TSequence *seq, const Span *p);
extern TSequenceSet *tcontseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequenceSet *tcontseq_minus_timestampset(const TSequence *seq, const Set *ts);
extern TSequenceSet *tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TSequenceSet *tcontseq_restrict_periodset(const TSequence *seq, const SpanSet *ps, bool atfunc);
extern TSequenceSet *tcontseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequenceSet *tcontseq_restrict_values(const TSequence *seq, const Set *set, bool atfunc);
extern TInstant *tdiscseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TSequence *tdiscseq_restrict_period(const TSequence *seq, const Span *period, bool atfunc);
extern TSequence *tdiscseq_restrict_periodset(const TSequence *seq, const SpanSet *ps, bool atfunc);
extern TSequence *tdiscseq_restrict_timestampset(const TSequence *seq, const Set *ts, bool atfunc);
extern TSequence *tdiscseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequence *tdiscseq_restrict_values(const TSequence *seq, const Set *set, bool atfunc);
extern bool temporal_bbox_restrict_set(const Temporal *temp, const Set *set);
extern Temporal *temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc);
extern Temporal *temporal_restrict_period(const Temporal *temp, const Span *p, bool atfunc);
extern Temporal *temporal_restrict_periodset(const Temporal *temp, const SpanSet *ps, bool atfunc);
extern Temporal *temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_timestampset(const Temporal *temp, const Set *ts, bool atfunc);
extern Temporal *temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, const Set *set, bool atfunc);
extern bool temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, Datum *result);
extern TInstant *tinstant_restrict_period(const TInstant *inst, const Span *period, bool atfunc);
extern TInstant *tinstant_restrict_periodset(const TInstant *inst, const SpanSet *ps, bool atfunc);
extern TInstant *tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc);
extern TInstant *tinstant_restrict_timestampset(const TInstant *inst, const Set *ts, bool atfunc);
extern TInstant *tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc);
extern TInstant *tinstant_restrict_values(const TInstant *inst, const Set *set, bool atfunc);
extern Temporal *tnumber_restrict_span(const Temporal *temp, const Span *span, bool atfunc);
extern Temporal *tnumber_restrict_spanset(const Temporal *temp, const SpanSet *ss, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_spanset(const TSequence *seq, const SpanSet *ss, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_spanset(const TSequence *seq, const SpanSet *ss, bool atfunc);
extern TInstant *tnumberinst_restrict_span(const TInstant *inst, const Span *span, bool atfunc);
extern TInstant *tnumberinst_restrict_spanset(const TInstant *inst, const SpanSet *ss, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_span(const TSequenceSet *ss, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_spanset(const TSequenceSet *ss, const SpanSet *spanset, bool atfunc);
extern Temporal *tpoint_restrict_geom_time(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern Temporal *tpoint_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc, bool atfunc);
extern TInstant *tpointinst_restrict_geom_time(const TInstant *inst, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern TInstant *tpointinst_restrict_stbox(const TInstant *inst, const STBox *box, bool border_inc, bool atfunc);
extern TSequence *tpointseq_disc_restrict_geom_time(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern TSequence *tpointseq_disc_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern TSequenceSet *tpointseq_linear_restrict_geom_time(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern TSequenceSet *tpointseq_linear_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern Temporal *tpointseq_restrict_geom_time(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern Temporal *tpointseq_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern TSequenceSet *tpointseq_step_restrict_geom_time(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern TSequenceSet *tpointseq_step_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_geom_time(const TSequenceSet *ss, const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_stbox(const TSequenceSet *ss, const STBox *box, bool border_inc, bool atfunc);
extern TSequence *tsequence_at_period(const TSequence *seq, const Span *p);
extern TInstant *tsequence_at_timestamp(const TSequence *seq, TimestampTz t);
extern Temporal *tsequence_restrict_period(const TSequence *seq, const Span *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ss, bool min, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_period(const TSequenceSet *ss, const Span *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_periodset(const TSequenceSet *ss, const SpanSet *ps, bool atfunc);
extern Temporal *tsequenceset_restrict_timestamp(const TSequenceSet *ss, TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_timestampset(const TSequenceSet *ss, const Set *ts, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ss, Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ss, const Set *set, bool atfunc);

/*****************************************************************************/

/* Mathematical functions for temporal types */

extern TSequence *tnumberseq_derivative(const TSequence *seq);
extern TSequenceSet *tnumberseqset_derivative(const TSequenceSet *ss);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value, meosType valuetype, meosType restype);
extern double nad_tnumber_number(const Temporal *temp, Datum value, meosType basetype);

/*****************************************************************************/

/* Ever/always functions for temporal types */

extern bool temporal_always_eq(const Temporal *temp, Datum value);
extern bool temporal_always_le(const Temporal *temp, Datum value);
extern bool temporal_always_lt(const Temporal *temp, Datum value);
extern bool temporal_ever_eq(const Temporal *temp, Datum value);
extern bool temporal_ever_le(const Temporal *temp, Datum value);
extern bool temporal_ever_lt(const Temporal *temp, Datum value);
extern bool tinstant_always_eq(const TInstant *inst, Datum value);
extern bool tinstant_always_le(const TInstant *inst, Datum value);
extern bool tinstant_always_lt(const TInstant *inst, Datum value);
extern bool tinstant_ever_eq(const TInstant *inst, Datum value);
extern bool tinstant_ever_le(const TInstant *inst, Datum value);
extern bool tinstant_ever_lt(const TInstant *inst, Datum value);
extern bool tpoint_always_eq(const Temporal *temp, Datum value);
extern bool tpoint_ever_eq(const Temporal *temp, Datum value);
extern bool tpointinst_always_eq(const TInstant *inst, Datum value);
extern bool tpointinst_ever_eq(const TInstant *inst, Datum value);
extern bool tpointseq_always_eq(const TSequence *seq, Datum value);
extern bool tpointseq_ever_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_always_eq(const TSequenceSet *ss, Datum value);
extern bool tpointseqset_ever_eq(const TSequenceSet *ss, Datum value);
extern bool tsequence_always_eq(const TSequence *seq, Datum value);
extern bool tsequence_always_le(const TSequence *seq, Datum value);
extern bool tsequence_always_lt(const TSequence *seq, Datum value);
extern bool tsequence_ever_eq(const TSequence *seq, Datum value);
extern bool tsequence_ever_le(const TSequence *seq, Datum value);
extern bool tsequence_ever_lt(const TSequence *seq, Datum value);
extern bool tsequenceset_always_eq(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_always_le(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_always_lt(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_eq(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_le(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_lt(const TSequenceSet *ss, Datum value);

/*****************************************************************************/

/* Comparison functions for temporal types */

extern int tinstant_cmp(const TInstant *inst1, const TInstant *inst2);
extern bool tinstant_eq(const TInstant *inst1, const TInstant *inst2);
extern int tsequence_cmp(const TSequence *seq1, const TSequence *seq2);
extern bool tsequence_eq(const TSequence *seq1, const TSequence *seq2);
extern int tsequenceset_cmp(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern bool tsequenceset_eq(const TSequenceSet *ss1, const TSequenceSet *ss2);

/*****************************************************************************
 * Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */

extern int tpointinst_srid(const TInstant *inst);
extern GSERIALIZED *tpointseq_cont_trajectory(const TSequence *seq);
extern GSERIALIZED *tpointseq_disc_trajectory(const TSequence *seq);
extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq, double prevlength);
extern bool tpointseq_is_simple(const TSequence *seq);
extern double tpointseq_length(const TSequence *seq);
extern TSequence *tpointseq_speed(const TSequence *seq);
extern int tpointseq_srid(const TSequence *seq);
extern STBox *tpointseq_stboxes(const TSequence *seq, int *count);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ss);
extern bool tpointseqset_is_simple(const TSequenceSet *ss);
extern double tpointseqset_length(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ss);
extern int tpointseqset_srid(const TSequenceSet *ss);
extern STBox *tpointseqset_stboxes(const TSequenceSet *ss, int *count);
extern GSERIALIZED *tpointseqset_trajectory(const TSequenceSet *ss);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern TInstant *tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper);
extern TSequence *tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeompointseqset_tgeogpointseqset(const TSequenceSet *ss, bool oper);
extern TInstant *tpointinst_set_srid(const TInstant *inst, int32 srid);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern TSequence *tpointseq_set_srid(const TSequence *seq, int32 srid);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ss, int *count);
extern TSequenceSet *tpointseqset_set_srid(const TSequenceSet *ss, int32 srid);

/*****************************************************************************/

/* Spatial relationship functions for temporal point types */


/*****************************************************************************/

/* Modification functions for temporal types */

extern Temporal *tcontseq_insert(const TSequence *seq1, const TSequence *seq2);
// extern Temporal *tcontseq_update(const TSequence *seq1, const TSequence *seq2);
extern TSequenceSet *tsequenceset_insert(const TSequenceSet *ss1, const TSequenceSet *ss2);
// extern TSequenceSet *tsequenceset_update(const TSequenceSet *ss1, const TSequenceSet *ss2);

extern TSequence *tcontseq_delete_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tcontseq_delete_timestampset(const TSequence *seq, const Set *ts);
extern TSequence *tcontseq_delete_period(const TSequence *seq, const Span *p);
extern TSequence *tcontseq_delete_periodset(const TSequence *seq, const SpanSet *ps);
extern TSequenceSet *tsequenceset_delete_timestamp(const TSequenceSet *ss, TimestampTz t);
extern TSequenceSet *tsequenceset_delete_timestampset(const TSequenceSet *ss, const Set *ts);
extern TSequenceSet *tsequenceset_delete_period(const TSequenceSet *ss, const Span *p);
extern TSequenceSet *tsequenceset_delete_periodset(const TSequenceSet *ss, const SpanSet *ps);

/*****************************************************************************/

/* Local aggregate functions for temporal types */

extern double tnumberseq_integral(const TSequence *seq);
extern double tnumbercontseq_twavg(const TSequence *seq);
extern double tnumberdiscseq_twavg(const TSequence *seq);
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

/* Multidimensional tiling functions for temporal types */

extern Temporal **temporal_time_split1(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **buckets, int *newcount);
extern Temporal **tnumber_value_split1(const Temporal *temp, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount);

/*****************************************************************************/

/* Similarity functions for temporal types */


/*****************************************************************************/

#endif
