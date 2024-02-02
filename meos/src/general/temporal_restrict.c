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
 * @file
 * @brief Restriction functions for temporal values
 */

#include "general/temporal_restrict.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/tinstant.h"
#include "general/temporal_boxops.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_util.h"
#include "general/type_parser.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
  #include "npoint/tnpoint_distance.h"
#endif

/*****************************************************************************
 * Bounding box tests for the restriction functions
 *****************************************************************************/

/**
 * @brief Return true if the bounding box of a temporal value contains a base
 * value
 */
bool
temporal_bbox_restrict_value(const Temporal *temp, Datum value)
{
  assert(temp);

  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    Span span1, span2;
    tnumber_set_span(temp, &span1);
    value_set_span(value, temptype_basetype(temp->temptype), &span2);
    return cont_span_span(&span1, &span2);
  }
  if (tgeo_type(temp->temptype))
  {
    /* Test that the geometry is not empty */
    GSERIALIZED *gs = DatumGetGserializedP(value);
    assert(gserialized_get_type(gs) == POINTTYPE);
    assert(tpoint_srid(temp) == gserialized_get_srid(gs));
    assert(MEOS_FLAGS_GET_Z(temp->flags) == FLAGS_GET_Z(gs->gflags));
    if (gserialized_is_empty(gs))
      return false;
    if (temp->subtype != TINSTANT)
    {
      STBox box1, box2;
      temporal_set_bbox(temp, &box1);
      geo_set_stbox(gs, &box2);
      return contains_stbox_stbox(&box1, &box2);
    }
  }
  return true;
}

/**
 * @brief Return true if the bounding box of the temporal number overlaps the
 * span of base values
 */
bool
tnumber_bbox_restrict_span(const Temporal *temp, const Span *s)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  numspan_set_tbox(s, &box2);
  return overlaps_tbox_tbox(&box1, &box2);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @csqlfn #Temporal_at_value(), #Temporal_minus_value()
 */
Temporal *
temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc)
{
  assert(temp);
  /* Ensure validity of the arguments */
  if (tgeo_type(temp->temptype))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value);
    if (! ensure_point_type(gs) ||
        ! ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs)) ||
        ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;
  }

  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_value(temp, value))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE ||
          MEOS_FLAGS_DISCRETE_INTERP(temp->flags)) ?
        temporal_cp(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_value((TInstant *) temp, value,
        atfunc);
    case TSEQUENCE:
      return (interp == DISCRETE) ?
        (Temporal *) tdiscseq_restrict_value((TSequence *) temp, value, atfunc) :
        (Temporal *) tcontseq_restrict_value((TSequence *) temp, value, atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_value((TSequenceSet *) temp,
        value, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Return true if the bounding boxes of a temporal value and a set
 * overlap
 * @param[in] temp Temporal value
 * @param[in] s Set
 */
bool
temporal_bbox_restrict_set(const Temporal *temp, const Set *s)
{
  assert(temp); assert(s);
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    Span span1, span2;
    tnumber_set_span(temp, &span1);
    set_set_span(s, &span2);
    return over_span_span(&span1, &span2);
  }
  if (tgeo_type(temp->temptype) && temp->subtype != TINSTANT)
  {
    STBox box;
    temporal_set_bbox(temp, &box);
    return contains_stbox_stbox(&box, (STBox *) SET_BBOX_PTR(s));
  }
  return true;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) an array of base
 * values
 * @param[in] temp Temporal value
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_values(), #Temporal_minus_values()
 */
Temporal *
temporal_restrict_values(const Temporal *temp, const Set *s, bool atfunc)
{
  assert(temp); assert(s);
  if (tgeo_type(temp->temptype))
  {
    assert(tpoint_srid(temp) == geoset_srid(s));
    assert(same_spatial_dimensionality(temp->flags, s->flags));
  }

  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_set(temp, s))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE) ? temporal_cp(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_values((TInstant *) temp, s,
        atfunc);
    case TSEQUENCE:
      return (interp == DISCRETE) ?
        (Temporal *) tdiscseq_restrict_values((TSequence *) temp, s, atfunc) :
        (Temporal *) tcontseq_restrict_values((TSequence *) temp, s, atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_values((TSequenceSet *) temp,
        s, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span of base
 * values
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Tnumber_at_span(), #Tnumber_minus_span()
 */
Temporal *
tnumber_restrict_span(const Temporal *temp, const Span *s, bool atfunc)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! tnumber_bbox_restrict_span(temp, s))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype == TSEQUENCE && interp != DISCRETE) ?
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp) :
        temporal_cp(temp);
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tnumberinst_restrict_span((TInstant *) temp,
        s, atfunc);
    case TSEQUENCE:
      return (interp == DISCRETE) ?
        (Temporal *) tnumberdiscseq_restrict_span((TSequence *) temp, s,
          atfunc) :
        (Temporal *) tnumbercontseq_restrict_span((TSequence *) temp, s,
          atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_restrict_span((TSequenceSet *) temp, s,
        atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span set
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Tnumber_at_spanset(), #Tnumber_minus_spanset()
 */
Temporal *
tnumber_restrict_spanset(const Temporal *temp, const SpanSet *ss, bool atfunc)
{
  assert(temp); assert(ss);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  Span s;
  tnumber_set_span(temp, &s);
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! over_span_span(&s, &ss->span))
  {
    if (atfunc)
      return NULL;
    else
    {
      if (temp->subtype == TSEQUENCE && interp != DISCRETE)
        return (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
      else
        return temporal_cp(temp);
    }
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tnumberinst_restrict_spanset((TInstant *) temp, ss,
        atfunc);
    case TSEQUENCE:
      return (interp == DISCRETE) ?
        (Temporal *) tnumberdiscseq_restrict_spanset((TSequence *) temp, ss,
          atfunc) :
        (Temporal *) tnumbercontseq_restrict_spanset((TSequence *) temp, ss,
          atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_restrict_spanset((TSequenceSet *) temp,
        ss, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a minimum base value
 * @param[in] temp Temporal value
 * @param[in] min True if the restriction is wrt min, false for max
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_min(), #Temporal_at_max(), #Temporal_minus_min(),
 *   #Temporal_minus_max()
 */
Temporal *
temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal *) tdiscseq_restrict_minmax((TSequence *) temp, min, atfunc) :
        (Temporal *) tcontseq_restrict_minmax((TSequence *) temp, min, atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_minmax((TSequenceSet *) temp,
        min, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to a timestamp
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_timestamptz(), Temporal_minus_timestamptz()
 */
Temporal *
temporal_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_timestamptz((TInstant *) temp, t,
        atfunc);
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
        return atfunc ?
          (Temporal *) tdiscseq_at_timestamptz((TSequence *) temp, t) :
          (Temporal *) tdiscseq_minus_timestamptz((TSequence *) temp, t);
      else
        return atfunc ?
          (Temporal *) tcontseq_at_timestamptz((TSequence *) temp, t) :
          (Temporal *) tcontseq_minus_timestamptz((TSequence *) temp, t);
    }
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_timestamptz(
        (TSequenceSet *) temp, t, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Return the last argument initialized with the base value of a
 * temporal value at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] result Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
temporal_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  assert(temp); assert(result);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_value_at_timestamptz((TInstant *) temp, t, result);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        tdiscseq_value_at_timestamptz((TSequence *) temp, t, result) :
        tsequence_value_at_timestamptz((TSequence *) temp, t, strict, result);
    default: /* TSEQUENCESET */
      return tsequenceset_value_at_timestamptz((TSequenceSet *) temp, t,
        strict, result);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a timestamp set
 * @param[in] temp Temporal value
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzset(), #Temporal_minus_tstzset()
 */
Temporal *
temporal_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc)
{
  assert(temp); assert(s);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzset((TInstant *) temp, s,
        atfunc);
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
        return (Temporal *) tdiscseq_restrict_tstzset((TSequence *) temp, s,
          atfunc);
      else
        return atfunc ?
          (Temporal *) tcontseq_at_tstzset((TSequence *) temp, s) :
          (Temporal *) tcontseq_minus_tstzset((TSequence *) temp, s);
    }
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_tstzset(
        (TSequenceSet *) temp, s, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspan(), #Temporal_minus_tstzspan()
 */
Temporal *
temporal_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc)
{
  assert(temp); assert(s);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzspan(
        (TInstant *) temp, s, atfunc);
    case TSEQUENCE:
      return tsequence_restrict_tstzspan((TSequence *) temp, s, atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_tstzspan(
        (TSequenceSet *) temp, s, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span set
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 */
Temporal *
temporal_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool atfunc)
{
  assert(temp); assert(ss);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzspanset(
        (TInstant *) temp, ss, atfunc);
    case TSEQUENCE:
      return tsequence_restrict_tstzspanset((TSequence *) temp, ss, atfunc);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_restrict_tstzspanset(
        (TSequenceSet *) temp, ss, atfunc);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal number restricted to a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @csqlfn #Tnumber_at_tbox()
 */
Temporal *
tnumber_at_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_tbox(temp, box))
    return NULL;

  /* Bounding box test */
  TBox box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return NULL;

  /* At least one of MEOS_FLAGS_GET_T and MEOS_FLAGS_GET_X is true */
  Temporal *temp1;
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  if (hast)
    /* Due the bounding box test above, temp1 is never NULL */
    temp1 = temporal_restrict_tstzspan(temp, &box->period, REST_AT);
  else
    temp1 = (Temporal *) temp;

  Temporal *result;
  if (hasx)
  {
    /* Ensure function is called for temporal numbers */
    assert(tnumber_type(temp->temptype));
    result = tnumber_restrict_span(temp1, &box->span, REST_AT);
  }
  else
    result = temp1;
  if (hasx && hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal number restricted to the complement of a temporal
 * box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @note It is not possible to make the difference from each dimension
 * separately, i.e., restrict at the period and then restrict to the span.
 * Therefore, we compute `atTbox` and then compute the complement of the
 * value obtained.
 * @csqlfn #Tnumber_minus_tbox()
 */
Temporal *
tnumber_minus_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_tbox(temp, box))
    return NULL;

  /* Bounding box test */
  TBox box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return temporal_cp(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tnumber_at_tbox(temp, box);
  if (temp1 != NULL)
  {
    SpanSet *ss = temporal_time(temp1);
    result = temporal_restrict_tstzspanset(temp, ss, REST_MINUS);
    pfree(temp1); pfree(ss);
  }
  return result;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a base value
 * @param[in] inst Temporal instant
 * @param[in] value Value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_value(), #Temporal_minus_value()
 */
TInstant *
tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc)
{
  assert(inst);
  if (datum_eq(value, tinstant_val(inst),
      temptype_basetype(inst->temptype)))
    return atfunc ? tinstant_copy(inst) : NULL;
  return atfunc ? NULL : tinstant_copy(inst);
}

/**
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) an array of base values
 * @pre There are no duplicates values in the array
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tinstant_restrict_values_test(const TInstant *inst, const Set *s, bool atfunc)
{
  Datum value = tinstant_val(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  for (int i = 0; i < s->count; i++)
  {
    if (datum_eq(value, SET_VAL_N(s, i), basetype))
      return atfunc ? true : false;
  }
  return atfunc ? false : true;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to an array of base values
 * @param[in] inst Temporal instant
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_values(), #Temporal_minus_values()
 */
TInstant *
tinstant_restrict_values(const TInstant *inst, const Set *s, bool atfunc)
{
  assert(inst);
  if (tinstant_restrict_values_test(inst, s, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @brief Return true if a temporal number instant satisfies the restriction to
 * (the complement of) a span of base values
 * @param[in] inst Temporal number
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal number
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tnumberinst_restrict_span_test(const TInstant *inst, const Span *s,
  bool atfunc)
{
  bool contains = contains_span_value(s, tinstant_val(inst));
  return atfunc ? contains : ! contains;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal number instant to (the complement of) a span of
 * base values
 * @param[in] inst Temporal number
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Tnumber_at_span(), #Tnumber_minus_span()
 */
TInstant *
tnumberinst_restrict_span(const TInstant *inst, const Span *s, bool atfunc)
{
  assert(inst); assert(s);
  if (tnumberinst_restrict_span_test(inst, s, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @brief Return true if a temporal number satisfies the restriction to
 * (the complement of) an array of spans of base values
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tnumberinst_restrict_spanset_test(const TInstant *inst, const SpanSet *ss,
  bool atfunc)
{
  Datum value = tinstant_val(inst);
  for (int i = 0; i < ss->count; i++)
  {
    if (contains_span_value(SPANSET_SP_N(ss, i), value))
      return atfunc ? true : false;
  }
  return atfunc ? false : true;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal number instant to (the complement of) a span set
 * @param[in] inst Temporal instant
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Tnumber_at_spanset(), #Tnumber_minus_spanset()
 */
TInstant *
tnumberinst_restrict_spanset(const TInstant *inst, const SpanSet *ss,
  bool atfunc)
{
  assert(inst); assert(ss);
  if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a timestamptz
 * @param[in] inst Temporal instant
 * @param[in] t Timestamp
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 * @csqlfn #Temporal_at_timestamptz(), #Temporal_minus_timestamptz()
 */
TInstant *
tinstant_restrict_timestamptz(const TInstant *inst, TimestampTz t, bool atfunc)
{
  assert(inst);
  if (inst->t == t)
    return atfunc ? tinstant_copy(inst) : NULL;
  return atfunc ? NULL : tinstant_copy(inst);
}

/**
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) a timestamptz set
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tinstant_restrict_tstzset_test(const TInstant *inst, const Set *s,
  bool atfunc)
{
  for (int i = 0; i < s->count; i++)
    if (inst->t == DatumGetTimestampTz(SET_VAL_N(s, i)))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a timestamptz set
 * @param[in] inst Temporal instant
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzset(), #Temporal_minus_tstzset()
 */
TInstant *
tinstant_restrict_tstzset(const TInstant *inst, const Set *s, bool atfunc)
{
  assert(inst); assert(s);
  if (tinstant_restrict_tstzset_test(inst, s, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a timestamptz span
 * @param[in] inst Temporal instant
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspan(), #Temporal_minus_tstzspan()
 */
TInstant *
tinstant_restrict_tstzspan(const TInstant *inst, const Span *s, bool atfunc)
{
  assert(inst); assert(s);
  bool contains = contains_span_timestamptz(s, inst->t);
  if ((atfunc && ! contains) || (! atfunc && contains))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) a timestamptz set
 * @note This function is called for each composing instant in a temporal
 * discrete sequence
 */
bool
tinstant_restrict_tstzspanset_test(const TInstant *inst, const SpanSet *ss,
  bool atfunc)
{
  for (int i = 0; i < ss->count; i++)
    if (contains_span_timestamptz(SPANSET_SP_N(ss, i), inst->t))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a span set
 * @param[in] inst Temporal instant
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspanset(), #Temporal_minus_tstzspanset()
 */
TInstant *
tinstant_restrict_tstzspanset(const TInstant *inst,const  SpanSet *ss,
  bool atfunc)
{
  assert(inst); assert(ss);
  if (tinstant_restrict_tstzspanset_test(inst, ss, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) a base
 * value
 * @param[in] seq Temporal sequence
 * @param[in] value Base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 */
TSequence *
tdiscseq_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  assert(seq);
  meosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    Datum value1 = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    bool equal = datum_eq(value, value1, basetype);
    if ((atfunc && ! equal) || (! atfunc && equal))
      return NULL;
    return tsequence_copy(seq);
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool equal = datum_eq(value, tinstant_val(inst), basetype);
    if ((atfunc && equal) || (! atfunc && ! equal))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) an array
 * of base values
 * @param[in] seq Temporal sequence
 * @param[in] s Set of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 */
TSequence *
tdiscseq_restrict_values(const TSequence *seq, const Set *s, bool atfunc)
{
  assert(seq); assert(s);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (tinstant_restrict_values_test(TSEQUENCE_INST_N(seq, 0), s, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tinstant_restrict_values_test(inst, s, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a segment of a temporal sequence to (the complement of) a
 * base value
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] interp Interpolation
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 * @return Number of resulting sequences returned
 */
static int
tsegment_restrict_value(const TInstant *inst1, const TInstant *inst2,
  interpType interp, bool lower_inc, bool upper_inc, Datum value, bool atfunc,
  TSequence **result)
{
  assert(inst1->temptype == inst2->temptype);
  assert(interp != DISCRETE);
  Datum value1 = tinstant_val(inst1);
  Datum value2 = tinstant_val(inst2);
  meosType basetype = temptype_basetype(inst1->temptype);
  TInstant *instants[2];
  /* Is the segment constant? */
  bool isconst = datum_eq(value1, value2, basetype);
  /* Does the lower bound belong to the answer? */
  bool lower = atfunc ? datum_eq(value1, value, basetype) :
    datum_ne(value1, value, basetype);
  /* Does the upper bound belong to the answer? */
  bool upper = atfunc ? datum_eq(value2, value, basetype) :
    datum_ne(value2, value, basetype);
  /* For linear interpolation and not constant segment is the value in the
   * interior of the segment? */
  Datum projvalue = 0; /* make compiler quiet */
  TimestampTz t = 0; /* make compiler quiet */
  bool interior = (interp == LINEAR) && ! isconst &&
    tlinearsegm_intersection_value(inst1, inst2, value, basetype, &projvalue, &t);

  /* Overall segment does not belong to the answer */
  if ((isconst && ! lower) ||
    (! isconst && atfunc && (interp == LINEAR) && ((lower && ! lower_inc) ||
      (upper && ! upper_inc) || (! lower && ! upper && ! interior))))
    return 0;

  /* Segment belongs to the answer but bounds may not */
  if ((isconst && lower) ||
    /* Linear interpolation: Test of bounds */
    (! isconst && (interp == LINEAR) && ! atfunc &&
    (! lower || ! upper || ! interior)))
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc && lower, upper_inc && upper, interp, NORMALIZE_NO);
    return 1;
  }

  /* Step interpolation */
  if (interp == STEP)
  {
    int nseqs = 0;
    if (lower)
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, STEP, NORMALIZE_NO);
      pfree(instants[1]);
    }
    if (upper_inc && upper)
      result[nseqs++] = tinstant_to_tsequence(inst2, STEP);
    return nseqs;
  }

  /* Linear interpolation: Test of bounds */
  if (atfunc && ((lower && lower_inc) || (upper && upper_inc)))
  {
    result[0] = tinstant_to_tsequence(lower ? inst1 : inst2, LINEAR);
    return 1;
  }
  /* Interpolation */
  if (atfunc)
  {
    TInstant *inst = tinstant_make_free(projvalue, inst1->temptype, t);
    result[0] = tinstant_to_tsequence(inst, LINEAR);
    pfree(inst);
    return 1;
  }
  else
  {
    /* Due to roundoff errors t may be equal to inst1-> or ins2->t */
    if (t == inst1->t)
    {
      DATUM_FREE(projvalue, basetype);
      if (! lower_inc)
        return 0;

      instants[0] = (TInstant *) inst1;
      instants[1] = (TInstant *) inst2;
      result[0] = tsequence_make((const TInstant **) instants, 2,
        ! lower_inc, upper_inc, LINEAR, NORMALIZE_NO);
      return 1;
    }
    else if (t == inst2->t)
    {
      DATUM_FREE(projvalue, basetype);
      if (! upper_inc)
        return 0;

      instants[0] = (TInstant *) inst1;
      instants[1] = (TInstant *) inst2;
      result[0] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, ! upper_inc, LINEAR, NORMALIZE_NO);
      return 1;
    }
    else
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make_free(projvalue, inst1->temptype, t);
      result[0] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, LINEAR, NORMALIZE_NO);
      instants[0] = instants[1];
      instants[1] = (TInstant *) inst2;
      result[1] = tsequence_make((const TInstant **) instants, 2,
        false, upper_inc, LINEAR, NORMALIZE_NO);
      pfree(instants[0]);
      return 2;
    }
  }
}

/**
 * @brief Restrict a temporal sequence to (the complement of) a base value
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] value Base value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set.
 * For this reason the bounding box and the instantaneous sequence sets are
 * repeated here.
 */
int
tcontseq_restrict_value_iter(const TSequence *seq, Datum value, bool atfunc,
  TSequence **result)
{
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    /* We do not call the function tinstant_restrict_value since this
     * would create a new unnecessary instant that needs to be freed */
    bool equal = datum_eq(tinstant_val(inst1), value,
      temptype_basetype(seq->temptype));
    if ((atfunc && ! equal) || (! atfunc && equal))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Bounding box test */
  if (! temporal_bbox_restrict_value((Temporal *) seq, value))
  {
    if (atfunc)
      return 0;
    /* Minus function */
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Each iteration adds between 0 and 2 sequences */
    nseqs += tsegment_restrict_value(inst1, inst2, interp, lower_inc, upper_inc,
      value, atfunc, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to (the complement of) a base value
 * @param[in] seq Temporal sequence
 * @param[in] value Base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note There is no bounding box or instantaneous test in this function,
 * they are done in the @p atValue and @p minusValue functions since the latter
 * are called for each sequence in a sequence set or for each element in the
 * array for the @p atValues and @p minusValues functions.
 */
TSequenceSet *
tcontseq_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  assert(seq);
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tcontseq_restrict_value_iter(seq, value, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to an array of base values (iterator
 * function)
 * @param[in] seq Temporal sequence
 * @param[in] set Set of base values
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre There are no duplicates values in the array
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_at_values_iter(const TSequence *seq, const Set *set,
  TSequence **result)
{
  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tinstant_restrict_values(inst1, set, REST_AT);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Bounding box test */
  if (! temporal_bbox_restrict_set((Temporal *) seq, set))
    return 0;

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    for (int j = 0; j < set->count; j++)
      /* Each iteration adds between 0 and 2 sequences */
      nseqs += tsegment_restrict_value(inst1, inst2, interp, lower_inc,
        upper_inc, SET_VAL_N(set, j), REST_AT, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  if (nseqs > 1)
    tseqarr_sort(result, nseqs);

  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to (the complement of) an array of base
 * values
 * @param[in] seq Temporal sequence
 * @param[in] s Set of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function #tsequence_at_values_iter since the latter is called
 * for each composing sequence of a temporal sequence set number.
 */
TSequenceSet *
tcontseq_restrict_values(const TSequence *seq, const Set *s, bool atfunc)
{
  assert(seq); assert(s);
  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count *
    s->count * 2);
  int newcount = tsequence_at_values_iter(seq, s, sequences);
  TSequenceSet *atresult = tsequenceset_make_free(sequences, newcount, NORMALIZE);
  if (atfunc)
    return atresult;

  /*
   * MINUS function
   * Compute the complement of the previous value.
   */
  if (newcount == 0)
    return tsequence_to_tsequenceset(seq);

  SpanSet *ps1 = tsequenceset_time(atresult);
  SpanSet *ps2 = minus_span_spanset(&seq->period, ps1);
  TSequenceSet *result = NULL;
  if (ps2 != NULL)
  {
    result = tcontseq_restrict_tstzspanset(seq, ps2, REST_AT);
    pfree(ps2);
  }
  pfree(atresult); pfree(ps1);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal discrete number sequence to (the complement of) a
 * span of base values
 * @param[in] seq Temporal number
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test has been done in the dispatch function.
 */
TSequence *
tnumberdiscseq_restrict_span(const TSequence *seq, const Span *s, bool atfunc)
{
  assert(seq); assert(s);
  assert(temptype_basetype(seq->temptype) == s->basetype);
  /* Instantaneous sequence */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tnumberinst_restrict_span_test(inst, s, atfunc))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @brief Restrict a temporal discrete sequence number to (the complement of) an
 * array of spans of base values
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test has been done in the dispatch function.
 */
TSequence *
tnumberdiscseq_restrict_spanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  assert(seq); assert(ss);
  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE,
      NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a segment of a temporal number to (the complement of) a span
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] interp Interpolation of the segment
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 */
static int
tnumbersegm_restrict_span(const TInstant *inst1, const TInstant *inst2,
  interpType interp, bool lower_inc, bool upper_inc, const Span *s,
  bool atfunc, TSequence **result)
{
  Datum value1 = tinstant_val(inst1);
  Datum value2 = tinstant_val(inst2);
  meosType basetype = temptype_basetype(inst1->temptype);
  meosType spantype = basetype_spantype(basetype);
  TInstant *instants[2];
  bool found;

  /* Constant segment (step or linear interpolation) */
  if (datum_eq(value1, value2, basetype))
  {
    found = contains_span_value(s, value1);
    if ((atfunc && ! found) || (! atfunc && found))
      return 0;
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    return 1;
  }

  /* Step interpolation */
  if (interp == STEP)
  {
    int nseqs = 0;
    found = contains_span_value(s, value1);
    if ((atfunc && found) || (! atfunc && ! found))
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, interp, NORMALIZE_NO);
      pfree(instants[1]);
    }
    found = contains_span_value(s, value2);
    if (upper_inc &&
      ((atfunc && found) || (! atfunc && ! found)))
    {
      result[nseqs++] = tinstant_to_tsequence(inst2, interp);
    }
    return nseqs;
  }

  /* Linear interpolation */

  /* Compute the intersection of the spans */
  Span valuespan, inter;
  bool increasing = DatumGetFloat8(value1) < DatumGetFloat8(value2);
  if (increasing)
    span_set(value1, value2, lower_inc, upper_inc, basetype, spantype,
      &valuespan);
  else
    span_set(value2, value1, upper_inc, lower_inc, basetype, spantype,
      &valuespan);
  found = inter_span_span(&valuespan, s, &inter);
  /* The intersection is empty */
  if (! found)
  {
    if (atfunc)
      return 0;
    /* MINUS */
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    return 1;
  }

  /* Compute the instants of the intersection */
  TInstant *inter1, *inter2;
  bool tofree1 = false, tofree2 = false;
  TimestampTz t1, t2;
  Datum lower, upper;
  bool lower_inc1, upper_inc1;
  if (increasing)
  {
    lower = inter.lower; upper = inter.upper;
    lower_inc1 = inter.lower_inc; upper_inc1 = inter.upper_inc;
  }
  else
  {
    lower = inter.upper; upper = inter.lower;
    lower_inc1 = inter.upper_inc; upper_inc1 = inter.lower_inc;
  }
  tfloatsegm_intersection_value(inst1, inst2, lower, basetype, &t1);
  if (t1 == inst1->t)
    inter1 = (TInstant *) inst1;
  else if (t1 == inst2->t)
    inter1 = (TInstant *) inst2;
  else
  {
    /* To reduce the roundoff errors we project the temporal number to the
     * timestamp instead of taking the bound value */
    inter1 = tsegment_at_timestamptz(inst1, inst2, interp, t1);
    tofree1 = true;
  }
  int j = 1;
  if (! datum_eq(lower, upper, basetype))
  {
    tfloatsegm_intersection_value(inst1, inst2, upper, basetype, &t2);
    if (t2 == inst1->t)
      inter2 = (TInstant *) inst1;
    else if (t2 == inst2->t)
      inter2 = (TInstant *) inst2;
    else
    {
      /* To reduce the roundoff errors we project the temporal number to the
       * timestamp instead of taking the bound value */
      inter2 = tsegment_at_timestamptz(inst1, inst2, interp, t2);
      tofree2 = true;
    }
    j = 2;
  }

  /* Compute the result */
  int nseqs = 0;
  if (atfunc)
  {
    /* We need order the instants */
    if (j > 1 && inter1->t > inter2->t)
    {
      TInstant *swap = inter1;
      inter1 = inter2;
      inter2 = swap;
      tofree1 = ! tofree1;
      tofree2 = ! tofree2;
    }
    instants[0] = inter1;
    if (j > 1)
      instants[1] = inter2;
    result[nseqs++] = tsequence_make((const TInstant **) instants, j,
        lower_inc1, upper_inc1, interp, NORMALIZE_NO);
  }
  else
  {
    /* First segment if any */
    if (j == 1)
      inter2 = inter1;
    if (inter1->t == inst1->t)
    {
      if (lower_inc && ! lower_inc1)
      {
        instants[0] = inter1;
        result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, interp, NORMALIZE_NO);
      }
    }
    else
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = inter1;
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, ! lower_inc1, interp, NORMALIZE_NO);
    }
    /* Second segment if any */
    if (inter2->t < inst2->t)
    {
      instants[0] = (j == 1) ? inter1 : inter2;
      instants[1] = (TInstant *) inst2;
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
          ! upper_inc1, upper_inc, interp, NORMALIZE_NO);
    }
    else
    {
      if (upper_inc && ! upper_inc1)
      {
        instants[0] = (j == 1) ? inter1 : inter2;
        result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, interp, NORMALIZE_NO);
      }
    }
  }
  if (tofree1)
    pfree(inter1);
  if (j > 1 && tofree2)
    pfree(inter2);
  return nseqs;
}

/**
 * @brief Restrict a temporal number to (the complement of) a span (iterator
 * function)
 * @param[in] seq temporal number
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_span_iter(const TSequence *seq, const Span *s,
  bool atfunc, TSequence **result)
{
  /* Bounding box test */
  TBox box1, box2;
  tsequence_set_bbox(seq, &box1);
  numspan_set_tbox(s, &box2);
  if (! overlaps_tbox_tbox(&box1, &box2))
  {
    if (atfunc)
      return 0;
    else
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }
  }

  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    /* The bounding box test above does not distinguish between
     * inclusive/exclusive bounds */
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tnumberinst_restrict_span(inst1, s, atfunc);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    nseqs += tnumbersegm_restrict_span(inst1, inst2, interp, lower_inc,
      upper_inc, s, atfunc, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence number to (the complement of) a span
 * @param[in] seq Temporal number
 * @param[in] s Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note It is supposed that a bounding box test has been done in the dispatch
 * function.
 */
TSequenceSet *
tnumbercontseq_restrict_span(const TSequence *seq, const Span *s, bool atfunc)
{
  assert(seq); assert(s);
  assert(temptype_basetype(seq->temptype) == s->basetype);
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tnumbercontseq_restrict_span_iter(seq, s, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal number to (the complement of) an array of spans
 * of base values (iterator function)
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre The array of spans is normalized
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_spanset_iter(const TSequence *seq, const SpanSet *ss,
  bool atfunc, TSequence **result)
{
  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tnumberinst_restrict_spanset(inst1, ss, atfunc);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (atfunc)
  {
    /* AT function */
    inst1 = TSEQUENCE_INST_N(seq, 0);
    bool lower_inc = seq->period.lower_inc;
    int nseqs = 0;
    for (int i = 1; i < seq->count; i++)
    {
      inst2 = TSEQUENCE_INST_N(seq, i);
      bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
      for (int j = 0; j < ss->count; j++)
        nseqs += tnumbersegm_restrict_span(inst1, inst2, interp, lower_inc,
          upper_inc, SPANSET_SP_N(ss, j), REST_AT, &result[nseqs]);
      inst1 = inst2;
      lower_inc = true;
    }
    if (nseqs > 1)
      tseqarr_sort(result, nseqs);
    return nseqs;
  }
  else
  {
    /*
     * MINUS function
     * Compute first the tnumberseq_at_spans, then compute its complement
     * Notice that in this case due to rounoff errors it may be the case
     * that temp is not equal to merge(atSpans(temp, .),minusSpans(temp, .),
     * since we kept the span values instead of the projected values when
     * computing atSpans
     */
    TSequenceSet *seqset = tnumbercontseq_restrict_spanset(seq, ss, REST_AT);
    if (seqset == NULL)
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }

    SpanSet *ps1 = tsequenceset_time(seqset);
    SpanSet *ps2 = minus_span_spanset(&seq->period, ps1);
    int newcount = 0;
    if (ps2 != NULL)
    {
      newcount = tcontseq_at_tstzspanset1(seq, ps2, result);
      pfree(ps2);
    }
    pfree(seqset); pfree(ps1);
    return newcount;
  }
}

/**
 * @brief Restrict a temporal number to (the complement of) an array of spans
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 */
TSequenceSet *
tnumbercontseq_restrict_spanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  assert(seq); assert(ss);
  assert(temptype_basetype(seq->temptype) == ss->basetype);
  /* General case */
  int maxcount = seq->count * ss->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int newcount = tnumbercontseq_restrict_spanset_iter(seq, ss, atfunc,
    sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) its
 * minimum/maximum base value
 * @param[in] seq Temporal sequence
 * @param[in] min True if restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_min(), #Temporal_at_max(), #Temporal_minus_min(),
 * #Temporal_minus_max()
 */
TSequence *
tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  assert(seq);
  Datum minmax = min ? tsequence_min_val(seq) : tsequence_max_val(seq);
  return tdiscseq_restrict_value(seq, minmax, atfunc);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal continuous sequence to (the complement of) its
 * minimum/maximum base value
 * @param[in] seq Temporal sequence
 * @param[in] min True if restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_min(), #Temporal_at_max(), #Temporal_minus_min(),
 * #Temporal_minus_max()
 */
TSequenceSet *
tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  assert(seq);
  Datum minmax = min ? tsequence_min_val(seq) : tsequence_max_val(seq);
  return tcontseq_restrict_value(seq, minmax, atfunc);
}

/*****************************************************************************/

/**
 * @brief Return the last argument initialized with the value of a temporal
 * discrete sequence at a timestamptz
 * @note In order to be compatible with the corresponding functions for
 * temporal sequences that need to interpolate the value, it is necessary to
 * return a copy of the value.
 */
bool
tdiscseq_value_at_timestamptz(const TSequence *seq, TimestampTz t, Datum *result)
{
  assert(seq); assert(result);
  int loc = tdiscseq_find_timestamptz(seq, t);
  if (loc < 0)
    return false;

  *result = tinstant_value(TSEQUENCE_INST_N(seq, loc));
  return true;
}

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) a
 * timestamptz
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value.
 */
TInstant *
tdiscseq_at_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  /* Bounding box test */
  if (! contains_span_timestamptz(&seq->period, t))
    return NULL;

  /* Instantenous sequence */
  if (seq->count == 1)
    return tinstant_copy(TSEQUENCE_INST_N(seq, 0));

  /* General case */
  const TInstant *inst;
  int loc = tdiscseq_find_timestamptz(seq, t);
  if (loc < 0)
    return NULL;
  inst = TSEQUENCE_INST_N(seq, loc);
  return tinstant_copy(inst);
}

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) a
 * timestamptz
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value.
 */
TSequence *
tdiscseq_minus_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  /* Bounding box test */
  if (! contains_span_timestamptz(&seq->period, t))
    return tsequence_copy(seq);

  /* Instantenous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (inst->t != t)
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) a
 * timestamptz set
 */
TSequence *
tdiscseq_restrict_tstzset(const TSequence *seq, const Set *s, bool atfunc)
{
  assert(seq); assert(s);
  TSequence *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (s->count == 1)
  {
    Temporal *temp = atfunc ?
      (Temporal *) tdiscseq_at_timestamptz(seq,
        DatumGetTimestampTz(SET_VAL_N(s, 0))) :
      (Temporal *) tdiscseq_minus_timestamptz(seq,
        DatumGetTimestampTz(SET_VAL_N(s, 0)));
    if (temp == NULL || ! atfunc)
      return (TSequence *) temp;
    /* Transform the result of tdiscseq_at_timestamp into a sequence */
    result = tinstant_to_tsequence((const TInstant *) temp, DISCRETE);
    pfree(temp);
    return result;
  }

  /* Bounding box test */
  Span p;
  set_set_span(s, &p);
  if (! over_span_span(&seq->period, &p))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tinstant_restrict_tstzset_test(inst, s, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, ninsts = 0;
  while (i < seq->count && j < s->count)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(s, j));
    int cmp = timestamptz_cmp_internal(inst->t, t);
    if (cmp == 0)
    {
      if (atfunc)
        instants[ninsts++] = inst;
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      if (! atfunc)
        instants[ninsts++] = inst;
      i++;
    }
    else
      j++;
  }
  /* For minus copy the instants after the discrete sequence */
  if (! atfunc)
  {
    while (i < seq->count)
      instants[ninsts++] = TSEQUENCE_INST_N(seq, i++);
  }
  result = (ninsts == 0) ? NULL : tsequence_make(instants, ninsts, true, true,
    DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @brief Restrict a temporal discrete sequence to (the complement of) a
 * timestamptz span
 */
TSequence *
tdiscseq_restrict_tstzspan(const TSequence *seq, const Span *s, bool atfunc)
{
  assert(seq); assert(s);
  /* Bounding box test */
  if (! over_span_span(&seq->period, s))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool contains = contains_span_timestamptz(s, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @brief Restrict a discrete temporal sequence to (the complement of) a
 * timestamptz span set
 */
TSequence *
tdiscseq_restrict_tstzspanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  assert(seq); assert(ss);
  const TInstant *inst;

  /* Singleton span set */
  if (ss->count == 1)
    return tdiscseq_restrict_tstzspan(seq, SPANSET_SP_N(ss, 0), atfunc);

  /* Bounding box test */
  if (! over_span_span(&seq->period, &ss->span))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tinstant_restrict_tstzspanset_test(inst, ss, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    bool contains = contains_spanset_timestamptz(ss, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict the segment of a temporal sequence to a timestamptz
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] interp Interpolation of the segment
 * @param[in] t Timestamp
 * @pre The timestamp t satisfies `inst1->t <= t <= inst2->t`
 * @note The function creates a new value that must be freed
 */
TInstant *
tsegment_at_timestamptz(const TInstant *inst1, const TInstant *inst2,
  interpType interp, TimestampTz t)
{
  Datum value = tsegment_value_at_timestamptz(inst1, inst2, interp, t);
  return tinstant_make_free(value, inst1->temptype, t);
}

/**
 * @brief Restrict a temporal continuous sequence to a timestamptz
 */
TInstant *
tcontseq_at_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  /* Bounding box test */
  if (! contains_span_timestamptz(&seq->period, t))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tinstant_copy(TSEQUENCE_INST_N(seq, 0));

  /* General case */
  int n = tcontseq_find_timestamptz(seq, t);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, n);
  if (t == inst1->t)
    return tinstant_copy(inst1);
  else
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, n + 1);
    return tsegment_at_timestamptz(inst1, inst2,
      MEOS_FLAGS_GET_INTERP(seq->flags), t);
  }
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to a timestamptz
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @csqlfn #Temporal_at_timestamptz() */
TInstant *
tsequence_at_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return tdiscseq_at_timestamptz(seq, t);
  else
    return tcontseq_at_timestamptz(seq, t);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tcontseq_minus_timestamp_iter(const TSequence *seq, TimestampTz t,
  TSequence **result)
{
  /* Bounding box test */
  if (! contains_span_timestamptz(&seq->period, t))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  const TInstant *inst1, *inst2;
  inst1 = TSEQUENCE_INST_N(seq, 0);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  int i, nseqs = 0;
  int n = tcontseq_find_timestamptz(seq, t);
  /* Compute the first sequence until t */
  if (n != 0 || inst1->t < t)
  {
    for (i = 0; i < n; i++)
      instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i);
    inst1 = TSEQUENCE_INST_N(seq, n);
    inst2 = TSEQUENCE_INST_N(seq, n + 1);
    if (inst1->t == t)
    {
      if (interp == LINEAR)
      {
        instants[n] = (TInstant *) inst1;
        result[nseqs++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, interp, NORMALIZE_NO);
      }
      else
      {
        instants[n] = tinstant_make(tinstant_val(instants[n - 1]),
          inst1->temptype, t);
        result[nseqs++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, interp, NORMALIZE_NO);
        pfree(instants[n]);
      }
    }
    else
    {
      /* inst1->t < t */
      instants[n] = (TInstant *) inst1;
      instants[n + 1] = (interp == LINEAR) ?
        tsegment_at_timestamptz(inst1, inst2, interp, t) :
        tinstant_make(tinstant_val(inst1), inst1->temptype, t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, n + 2,
        seq->period.lower_inc, false, interp, NORMALIZE_NO);
      pfree(instants[n + 1]);
    }
  }
  /* Compute the second sequence after t */
  inst1 = TSEQUENCE_INST_N(seq, n);
  inst2 = TSEQUENCE_INST_N(seq, n + 1);
  if (t < inst2->t)
  {
    instants[0] = tsegment_at_timestamptz(inst1, inst2, interp, t);
    for (i = 1; i < seq->count - n; i++)
      instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i + n);
    result[nseqs++] = tsequence_make((const TInstant **) instants,
      seq->count - n, false, seq->period.upper_inc, interp, NORMALIZE_NO);
    pfree(instants[0]);
  }
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 */
TSequenceSet *
tcontseq_minus_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  TSequence *sequences[2];
  int count = tcontseq_minus_timestamp_iter(seq, t, sequences);
  if (count == 0)
    return NULL;
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(sequences[i]);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to a timestamptz set
 */
TSequence *
tcontseq_at_tstzset(const TSequence *seq, const Set *s)
{
  assert(seq); assert(s);
  TInstant *inst;

  /* Singleton timestamp set */
  if (s->count == 1)
  {
    inst = tsequence_at_timestamptz(seq,
      DatumGetTimestampTz(SET_VAL_N(s, 0)));
    if (inst == NULL)
      return NULL;
    TSequence *result = tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
    pfree(inst);
    return result;
  }

  /* Bounding box test */
  Span p;
  set_set_span(s, &p);
  if (! over_span_span(&seq->period, &p))
    return NULL;

  inst = (TInstant *) TSEQUENCE_INST_N(seq, 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (! contains_set_value(s, TimestampTzGetDatum(inst->t)))
      return NULL;
    return tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
  }

  /* General case */
  TimestampTz t = Max(DatumGetTimestampTz(seq->period.lower),
    DatumGetTimestampTz(SET_VAL_N(s, 0)));
  int loc;
  set_find_value(s, TimestampTzGetDatum(t), &loc);
  TInstant **instants = palloc(sizeof(TInstant *) * (s->count - loc));
  int ninsts = 0;
  for (int i = loc; i < s->count; i++)
  {
    t = DatumGetTimestampTz(SET_VAL_N(s, i));
    inst = tcontseq_at_timestamptz(seq, t);
    if (inst != NULL)
      instants[ninsts++] = inst;
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz set
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] s Tstzset
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @note This function is called for each sequence of a temporal sequence set
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_tstzset_iter(const TSequence *seq, const Set *s,
  TSequence **result)
{
  /* Singleton timestamp set */
  if (s->count == 1)
    return tcontseq_minus_timestamp_iter(seq,
      DatumGetTimestampTz(SET_VAL_N(s, 0)), result);

  /* Bounding box test */
  Span p;
  set_set_span(s, &p);
  if (! over_span_span(&seq->period, &p))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (contains_set_value(s,
          TimestampTzGetDatum(TSEQUENCE_INST_N(seq, 0)->t)))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant **tofree = palloc(sizeof(TInstant *) * Min(s->count, seq->count));
  bool lower_inc = seq->period.lower_inc;
  int i = 0,    /* current instant of the argument sequence */
    j = 0,      /* current timestamp of the argument timestamp set */
    nseqs = 0,  /* current number of new sequences */
    ninsts = 0, /* number of instants in the currently constructed sequence */
    nfree = 0;  /* number of instants to free */
  while (i < seq->count && j < s->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(s, j));
    Datum value;
    if (inst->t < t)
    {
      instants[ninsts++] = (TInstant *) inst;
      i++; /* advance instants */
    }
    else if (inst->t == t)
    {
      /* Close the current sequence */
      if (ninsts > 0)
      {
        if (interp == LINEAR)
          instants[ninsts++] = (TInstant *) inst;
        else /* interp == STEP */
        {
          /* Take the value of the previous instant */
          value = tinstant_val(instants[ninsts - 1]);
          instants[ninsts] = tinstant_make(value, inst->temptype, inst->t);
          tofree[nfree++] = instants[ninsts++];
        }
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, false, interp, NORMALIZE_NO);
        ninsts = 0;
      }
      /* If it is not the last instant start a new sequence */
      if (i < seq->count - 1)
      {
        instants[ninsts++] = (TInstant *) inst;
        lower_inc = false;
      }
      i++; /* advance instants */
      j++; /* advance timestamps */
    }
    else /* inst->t > t */
    {
      if (ninsts > 0)
      {
        /* Close the current sequence */
        if (interp == LINEAR)
          /* Interpolate */
          value = tsegment_value_at_timestamptz(instants[ninsts - 1], inst,
            LINEAR, t);
        else
          /* Take the value of the previous instant */
          value = tinstant_val(instants[ninsts - 1]);
        instants[ninsts] = tinstant_make(value, inst->temptype, t);
        tofree[nfree] = instants[ninsts++];
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, false, interp, NORMALIZE_NO);
        /* Restart a new sequence */
        instants[0] = tofree[nfree++];
        ninsts = 1;
        lower_inc = false;
      }
      j++; /* advance timestamps */
    }
  }
  /* Compute the sequence after the timestamp set */
  if (i < seq->count)
  {
    for (j = i; j < seq->count; j++)
      instants[ninsts++] = (TInstant *) TSEQUENCE_INST_N(seq, j);
  }
  if (ninsts > 0)
  {
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      lower_inc, seq->period.upper_inc, interp, NORMALIZE_NO);
  }
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz set
 */
TSequenceSet *
tcontseq_minus_tstzset(const TSequence *seq, const Set *s)
{
  assert(seq); assert(s);
  TSequence **sequences = palloc0(sizeof(TSequence *) * (s->count + 1));
  int count = tcontseq_minus_tstzset_iter(seq, s, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a continuous temporal sequence to a timestamptz span
 */
TSequence *
tcontseq_at_tstzspan(const TSequence *seq, const Span *s)
{
  assert(seq); assert(s);
  /* Bounding box test */
  Span inter;
  if (! inter_span_span(&seq->period, s, &inter))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tsequence_copy(seq);

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result;
  /* Intersecting period is instantaneous */
  if (inter.lower == inter.upper)
  {
    TInstant *inst = tcontseq_at_timestamptz(seq, inter.lower);
    result = tinstant_to_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  const TInstant *inst1, *inst2;
  int n = tcontseq_find_timestamptz(seq, inter.lower);
  /* If the lower bound of the intersecting period is exclusive */
  if (n == -1)
    n = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * (seq->count - n));
  /* Compute the value at the beginning of the intersecting period */
  inst1 = TSEQUENCE_INST_N(seq, n);
  inst2 = TSEQUENCE_INST_N(seq, n + 1);
  instants[0] = tsegment_at_timestamptz(inst1, inst2, interp, inter.lower);
  int ninsts = 1;
  for (int i = n + 2; i < seq->count; i++)
  {
    /* If the end of the intersecting period is between inst1 and inst2 */
    if (inst1->t <= DatumGetTimestampTz(inter.upper) &&
        DatumGetTimestampTz(inter.upper) <= inst2->t)
      break;

    inst1 = inst2;
    inst2 = TSEQUENCE_INST_N(seq, i);
    /* If the intersecting period contains inst1 */
    if (DatumGetTimestampTz(inter.lower) <= inst1->t &&
        inst1->t <= DatumGetTimestampTz(inter.upper))
      instants[ninsts++] = (TInstant *) inst1;
  }
  /* The last two values of sequences with step interpolation and
   * exclusive upper bound must be equal */
  if (interp == LINEAR || inter.upper_inc)
    instants[ninsts++] = tsegment_at_timestamptz(inst1, inst2, interp,
      inter.upper);
  else
  {
    Datum value = tinstant_val(instants[ninsts - 1]);
    instants[ninsts++] = tinstant_make(value, seq->temptype, inter.upper);
  }
  /* Since by definition the sequence is normalized it is not necessary to
   * normalize the projection of the sequence to the period */
  result = tsequence_make((const TInstant **) instants, ninsts,
    inter.lower_inc, inter.upper_inc, interp, NORMALIZE_NO);

  pfree(instants[0]); pfree(instants[ninsts - 1]); pfree(instants);

  return result;
}

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz span
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] s Span
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_tstzspan_iter(const TSequence *seq, const Span *s,
  TSequence **result)
{
  /* Bounding box test */
  if (! over_span_span(&seq->period, s))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  SpanSet *ss = minus_span_span(&seq->period, s);
  if (ss == NULL)
    return 0;
  for (int i = 0; i < ss->count; i++)
    result[i] = tcontseq_at_tstzspan(seq, SPANSET_SP_N(ss, i));
  int count = ss->count;
  pfree(ss);
  return count;
}

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz span
 */
TSequenceSet *
tcontseq_minus_tstzspan(const TSequence *seq, const Span *s)
{
  assert(seq); assert(s);
  TSequence *sequences[2];
  int count = tcontseq_minus_tstzspan_iter(seq, s, sequences);
  if (count == 0)
    return NULL;
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(sequences[i]);
  return result;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a timestamptz span
 * @param[in] seq Temporal sequence
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspan(), #Temporal_minus_tstzspan()
 */
Temporal *
tsequence_restrict_tstzspan(const TSequence *seq, const Span *s, bool atfunc)
{
  assert(seq); assert(s);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return (Temporal *) tdiscseq_restrict_tstzspan(seq, s, atfunc);
  else
    return atfunc ?
      (Temporal *) tcontseq_at_tstzspan(seq, s) :
      (Temporal *) tcontseq_minus_tstzspan(seq, s);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to a timestamptz span set
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is NOT called for each sequence of a temporal sequence
 * set but is called when computing tpointseq minus geometry
 */
int
tcontseq_at_tstzspanset1(const TSequence *seq, const SpanSet *ss,
  TSequence **result)
{
  /* Singleton span set */
  if (ss->count == 1)
  {
    result[0] = tcontseq_at_tstzspan(seq, SPANSET_SP_N(ss, 0));
    return 1;
  }

  /* Bounding box test */
  if (! over_span_span(&seq->period, &ss->span))
    return 0;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (! contains_spanset_timestamptz(ss, TSEQUENCE_INST_N(seq, 0)->t))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  int loc;
  /* The second argument in the following call should be a Datum */
  spanset_find_value(ss, seq->period.lower, &loc);
  int nseqs = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    TSequence *seq1 = tcontseq_at_tstzspan(seq, s);
    if (seq1 != NULL)
      result[nseqs++] = seq1;
    if (DatumGetTimestampTz(seq->period.upper) < DatumGetTimestampTz(s->upper))
      break;
  }
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to the complement of a timestamptz span
 * set (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the output array
 * @note This function is called for each sequence of a temporal sequence set.
 * To avoid roundoff errors in the loop we must use (1) compute the
 * complement of the span set and (2) compute the "at" function
 */
int
tcontseq_minus_tstzspanset_iter(const TSequence *seq, const SpanSet *ss,
  TSequence **result)
{
  /* Singleton span set */
  if (ss->count == 1)
    return tcontseq_minus_tstzspan_iter(seq, SPANSET_SP_N(ss, 0), result);

  /* The sequence can be split at most into (count + 1) sequences
   *    |----------------------|
   *        |---| |---| |---|
   */

  /* Compute the complement of the span set */
  SpanSet *ss1 = minus_span_spanset(&seq->period, ss);
  if (! ss1)
    return 0;
  int nseqs = 0;
  for (int i = 0; i < ss1->count; i++)
    result[nseqs++] = tcontseq_at_tstzspan(seq, SPANSET_SP_N(ss1, i));
  pfree(ss1);
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to (the complement of) a timestamptz
 * span set
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 */
TSequenceSet *
tcontseq_restrict_tstzspanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  assert(seq); assert(ss);
  /* Bounding box test */
  if (! over_span_span(&seq->period, &ss->span))
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (contains_spanset_timestamptz(ss, TSEQUENCE_INST_N(seq, 0)->t))
      return atfunc ? tsequence_to_tsequenceset(seq) : NULL;
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);
  }

  /* General case */
  int count = atfunc ? ss->count : ss->count + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int count1 = atfunc ? tcontseq_at_tstzspanset1(seq, ss, sequences) :
    tcontseq_minus_tstzspanset_iter(seq, ss, sequences);
  return tsequenceset_make_free(sequences, count1, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) a timestamptz
 * span set
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 */
Temporal *
tsequence_restrict_tstzspanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  return MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
      (Temporal *) tdiscseq_restrict_tstzspanset(seq, ss, atfunc) :
      (Temporal *) tcontseq_restrict_tstzspanset(seq, ss, atfunc);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) a base value
 * @param[in] ss Temporal sequence set
 * @param[in] value Value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 * @csqlfn #Temporal_at_value(), #Temporal_minus_value()
 */
TSequenceSet *
tsequenceset_restrict_value(const TSequenceSet *ss, Datum value, bool atfunc)
{
  assert(ss);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tcontseq_restrict_value(TSEQUENCESET_SEQ_N(ss, 0), value, atfunc);

  /* General case */
  int count = ss->totalcount;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tcontseq_restrict_value_iter(TSEQUENCESET_SEQ_N(ss, i), value,
      atfunc, &sequences[nseqs]);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) an array of
 * base values
 * @param[in] ss Temporal sequence set
 * @param[in] s Set of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 * @csqlfn #Temporal_at_values(), #Temporal_minus_values()
 */
TSequenceSet *
tsequenceset_restrict_values(const TSequenceSet *ss, const Set *s,
  bool atfunc)
{
  assert(ss); assert(s);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tcontseq_restrict_values(TSEQUENCESET_SEQ_N(ss, 0), s, atfunc);

  /* General case
   * Compute the AT function */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount *
     s->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tsequence_at_values_iter(TSEQUENCESET_SEQ_N(ss, i), s,
      &sequences[nseqs]);
  TSequenceSet *atresult = tsequenceset_make_free(sequences, nseqs, NORMALIZE);
  if (atfunc)
    return atresult;

  /*
   * MINUS function
   * Compute the complement of the previous value.
   */
  if (nseqs == 0)
    return tsequenceset_copy(ss);

  SpanSet *ps1 = tsequenceset_time(ss);
  SpanSet *ps2 = tsequenceset_time(atresult);
  SpanSet *ps = minus_spanset_spanset(ps1, ps2);
  TSequenceSet *result = NULL;
  if (ps != NULL)
  {
    result = tsequenceset_restrict_tstzspanset(ss, ps, REST_AT);
    pfree(ps);
  }
  pfree(atresult); pfree(ps1); pfree(ps2);
  return result;
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal number to a span of base values
 * @param[in] ss Temporal sequence set
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note It is supposed that a bounding box test has been done in the dispatch
 * function.
 * @csqlfn #Tnumber_at_span(), #Tnumber_minus_span()
 */
TSequenceSet *
tnumberseqset_restrict_span(const TSequenceSet *ss, const Span *s,
  bool atfunc)
{
  assert(ss); assert(s);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumbercontseq_restrict_span(TSEQUENCESET_SEQ_N(ss, 0), s, atfunc);

  /* General case */
  int count = ss->totalcount;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tnumbercontseq_restrict_span_iter(TSEQUENCESET_SEQ_N(ss, i), s,
      atfunc, &sequences[nseqs]);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal number to (the complement of) an array of
 * spans of base values
 * @param[in] ss Temporal sequence set
 * @param[in] sps Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal number value
 * @csqlfn #Tnumber_at_spanset(), #Tnumber_minus_spanset()
 */
TSequenceSet *
tnumberseqset_restrict_spanset(const TSequenceSet *ss, const SpanSet *sps,
  bool atfunc)
{
  assert(ss); assert(sps);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumbercontseq_restrict_spanset(TSEQUENCESET_SEQ_N(ss, 0),
      sps, atfunc);

  /* General case */
  int maxcount = ss->totalcount * sps->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tnumbercontseq_restrict_spanset_iter(TSEQUENCESET_SEQ_N(ss, i),
      sps, atfunc, &sequences[nseqs]);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) its
 * minimum/maximum base value
 * @param[in] ss Temporal sequence set
 * @param[in] min True if restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_min(), #Temporal_at_max(), #Temporal_minus_min(),
 * #Temporal_minus_max()
 */
TSequenceSet *
tsequenceset_restrict_minmax(const TSequenceSet *ss, bool min, bool atfunc)
{
  assert(ss);
  Datum minmax = min ? tsequenceset_min_val(ss) : tsequenceset_max_val(ss);
  return tsequenceset_restrict_value(ss, minmax, atfunc);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) a timestamptz
 * @param[in] ss Temporal sequence set
 * @param[in] t Timestamp
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_timestamptz(), #Temporal_minus_timestamptz()
 */
Temporal *
tsequenceset_restrict_timestamptz(const TSequenceSet *ss, TimestampTz t,
  bool atfunc)
{
  assert(ss);
  /* Bounding box test */
  if (! contains_span_timestamptz(&ss->period, t))
    return atfunc ? NULL : (Temporal *) tsequenceset_copy(ss);

  /* Singleton sequence set */
  if (ss->count == 1)
    return atfunc ?
      (Temporal *) tcontseq_at_timestamptz(TSEQUENCESET_SEQ_N(ss, 0), t) :
      (Temporal *) tcontseq_minus_timestamptz(TSEQUENCESET_SEQ_N(ss, 0), t);

  /* General case */
  const TSequence *seq;
  if (atfunc)
  {
    int loc;
    if (! tsequenceset_find_timestamptz(ss, t, &loc))
      return NULL;
    seq = TSEQUENCESET_SEQ_N(ss, loc);
    return (Temporal *) tsequence_at_timestamptz(seq, t);
  }
  else
  {
    /* At most one composing sequence can be split into two */
    TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count + 1));
    int i, nseqs = 0;
    for (i = 0; i < ss->count; i++)
    {
      seq = TSEQUENCESET_SEQ_N(ss, i);
      nseqs += tcontseq_minus_timestamp_iter(seq, t, &sequences[nseqs]);
      if (t < DatumGetTimestampTz(seq->period.upper))
      {
        i++;
        break;
      }
    }
    /* Copy the remaining sequences if went out of the for loop with the break */
    for (int j = i; j < ss->count; j++)
      sequences[nseqs++] = tsequence_copy(TSEQUENCESET_SEQ_N(ss, j));
    /* nseqs is never equal to 0 since in that case it is a singleton sequence
       set and it has been dealt by tcontseq_minus_timestamp above */
    assert(nseqs > 0);
    return (Temporal *) tsequenceset_make_free(sequences, nseqs, NORMALIZE_NO);
  }
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) a timestamptz
 * set
 * @param[in] ss Temporal sequence set
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzset(), #Temporal_minus_tstzset()
 */
Temporal *
tsequenceset_restrict_tstzset(const TSequenceSet *ss, const Set *s,
  bool atfunc)
{
  assert(ss); assert(s);
  /* Singleton timestamp set */
  if (s->count == 1)
  {
    Temporal *temp = tsequenceset_restrict_timestamptz(ss,
      DatumGetTimestampTz(SET_VAL_N(s, 0)), atfunc);
    if (atfunc && temp != NULL)
    {
      Temporal *result = (Temporal *) tinstant_to_tsequence(
        (const TInstant *) temp, DISCRETE);
      pfree(temp);
      return result;
    }
    return temp;
  }

  /* Bounding box test */
  Span s1;
  set_set_span(s, &s1);
  if (! over_span_span(&ss->period, &s1))
    return atfunc ? NULL : (Temporal *) tsequenceset_copy(ss);

  /* Singleton sequence set */
  if (ss->count == 1)
    return atfunc ?
      (Temporal *) tcontseq_at_tstzset(TSEQUENCESET_SEQ_N(ss, 0), s) :
      (Temporal *) tcontseq_minus_tstzset(TSEQUENCESET_SEQ_N(ss, 0), s);

  /* General case */
  const TSequence *seq;
  if (atfunc)
  {
    TInstant **instants = palloc(sizeof(TInstant *) * s->count);
    int count = 0;
    int i = 0, j = 0;
    while (i < s->count && j < ss->count)
    {
      seq = TSEQUENCESET_SEQ_N(ss, j);
      TimestampTz t = DatumGetTimestampTz(SET_VAL_N(s, i));
      if (contains_span_timestamptz(&seq->period, t))
      {
        instants[count++] = tsequence_at_timestamptz(seq, t);
        i++;
      }
      else
      {
        if (t <= DatumGetTimestampTz(seq->period.lower))
          i++;
        if (t >= DatumGetTimestampTz(seq->period.upper))
          j++;
      }
    }
    return (Temporal *) tsequence_make_free(instants, count, true, true,
      DISCRETE, NORMALIZE_NO);
  }
  else
  {
    /* For the minus case each timestamp will split at most one
     * composing sequence into two */
    TSequence **sequences = palloc(sizeof(TSequence *) *
      (ss->count + s->count + 1));
    int nseqs = 0;
    for (int i = 0; i < ss->count; i++)
    {
      seq = TSEQUENCESET_SEQ_N(ss, i);
      nseqs += tcontseq_minus_tstzset_iter(seq, s, &sequences[nseqs]);

    }
    return (Temporal *) tsequenceset_make_free(sequences, nseqs, NORMALIZE);
  }
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) a timestamptz
 * span
 * @param[in] ss Temporal sequence set
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspan(), #Temporal_minus_tstzspan()
 */
TSequenceSet *
tsequenceset_restrict_tstzspan(const TSequenceSet *ss, const Span *s,
  bool atfunc)
{
  /* Bounding box test */
  if (! over_span_span(&ss->period, s))
    return atfunc ? NULL : tsequenceset_copy(ss);

  TSequence *seq;
  TSequenceSet *result;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    if (atfunc)
    {
      seq = tcontseq_at_tstzspan(TSEQUENCESET_SEQ_N(ss, 0), s);
      result = tsequence_to_tsequenceset(seq);
      pfree(seq);
      return result;
    }
    else
      return tcontseq_minus_tstzspan(TSEQUENCESET_SEQ_N(ss, 0), s);
  }

  /* General case */
  if (atfunc)
  {
    /* AT */
    int loc;
    tsequenceset_find_timestamptz(ss, DatumGetTimestampTz(s->lower), &loc);
    /* We are sure that loc < ss->count due to the bounding period test above */
    TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count - loc));
    TSequence *tofree[2];
    int nseqs = 0, nfree = 0;
    for (int i = loc; i < ss->count; i++)
    {
      seq = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
      if (cont_span_span(s, &seq->period))
        sequences[nseqs++] = seq;
      else if (over_span_span(s, &seq->period))
      {
        TSequence *newseq = tcontseq_at_tstzspan(seq, s);
        sequences[nseqs++] = tofree[nfree++] = newseq;
      }
      int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(s->upper),
        DatumGetTimestampTz(seq->period.upper));
      if (cmp < 0 || (cmp == 0 && seq->period.upper_inc))
        break;
    }
    if (nseqs == 0)
    {
      pfree(sequences);
      return NULL;
    }
    /* Since both the tsequenceset and the period are normalized it is not
     * necessary to normalize the result of the projection */
    result = tsequenceset_make((const TSequence **) sequences, nseqs,
      NORMALIZE_NO);
    for (int i = 0; i < nfree; i++)
      pfree(tofree[i]);
    pfree(sequences);
    return result;
  }
  else
  {
    /* MINUS */
    SpanSet *ps = tsequenceset_time(ss);
    SpanSet *resultps = minus_spanset_span(ps, s);
    result = NULL;
    if (resultps != NULL)
    {
      result = tsequenceset_restrict_tstzspanset(ss, resultps, REST_AT);
      pfree(resultps);
    }
    pfree(ps);
    return result;
  }
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal sequence set to (the complement of) a timestamptz
 * span set
 * @param[in] ss Temporal sequence set
 * @param[in] ps Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_tstzspanset(), #Temporal_minus_tstzspanset()
 */
TSequenceSet *
tsequenceset_restrict_tstzspanset(const TSequenceSet *ss, const SpanSet *ps,
  bool atfunc)
{
  assert(ss); assert(ps);
  /* Singleton span set */
  if (ps->count == 1)
    return tsequenceset_restrict_tstzspan(ss, SPANSET_SP_N(ps, 0), atfunc);

  /* Bounding box test */
  if (! over_span_span(&ss->period, &ps->span))
    return atfunc ? NULL : tsequenceset_copy(ss);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tcontseq_restrict_tstzspanset(TSEQUENCESET_SEQ_N(ss, 0), ps, atfunc);

  /* General case */
  TSequence **sequences;
  int i = 0, j = 0, nseqs = 0;
  if (atfunc)
  {
    TimestampTz t = Max(DatumGetTimestampTz(ss->period.lower),
      DatumGetTimestampTz(ps->span.lower));
    tsequenceset_find_timestamptz(ss, t, &i);
    spanset_find_value(ps, DatumGetTimestampTz(t), &j);
    sequences = palloc(sizeof(TSequence *) * (ss->count + ps->count - i - j));
  }
  else
    sequences = palloc(sizeof(TSequence *) * (ss->count + ps->count));
  while (i < ss->count && j < ps->count)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    const Span *s = SPANSET_SP_N(ps, j);
    /* The sequence and the period do not overlap */
    if (lf_span_span(&seq->period, s))
    {
      if (! atfunc)
        /* Copy the sequence */
        sequences[nseqs++] = tsequence_copy(seq);
      i++;
    }
    else if (over_span_span(&seq->period, s))
    {
      if (atfunc)
      {
        /* Compute the restriction of the sequence and the period */
        TSequence *seq1 = tcontseq_at_tstzspan(seq, s);
        if (seq1 != NULL)
          sequences[nseqs++] = seq1;
        int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq->period.upper),
          DatumGetTimestampTz(s->upper));
        if (cmp == 0 && seq->period.upper_inc == s->upper_inc)
        {
          i++; j++;
        }
        else if (cmp < 0 ||
          (cmp == 0 && ! seq->period.upper_inc && s->upper_inc))
          i++;
        else
          j++;
      }
      else
      {
        /* Compute the difference of the sequence and the FULL periodset.
         * Notice that we cannot compute the difference with the
         * current period without replicating the functionality in
         * #tcontseq_minus_tstzspanset_iter */
        nseqs += tcontseq_minus_tstzspanset_iter(seq, ps, &sequences[nseqs]);
        i++;
      }
    }
    else
      j++;
  }
  if (! atfunc)
  {
    /* For minus copy the sequences after the span set */
    while (i < ss->count)
      sequences[nseqs++] = tsequence_copy(TSEQUENCESET_SEQ_N(ss, i++));
  }
  /* It is necessary to normalize despite the fact that both the tsequenceset
  * and the periodset are normalized */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/
