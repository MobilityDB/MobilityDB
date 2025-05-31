/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Basic functions for temporal circular buffers
 */

/* C */
#include <assert.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

// TODO REMOVE WHEN THE TWO FUNCTIONS BELOW HAVE BEEN WRITTEN !!!
extern int cbuffersegm_distance_turnpt(const Cbuffer *start1,
  const Cbuffer *end1, const Cbuffer *start2, const Cbuffer *end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2);

/**
 * @brief Return 1 if a segment of a temporal circular buffer and a circular
 * buffer intersect during the period defined by the timestamps output in the
 * last arguments
 * @param[in] start,end Temporal instants defining the segment
 * @param[in] value Value to locate
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 
 * @return Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp.
 */
int
tcbuffersegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  /* While waiting for this function we cheat and call the function below */
  int result = cbuffersegm_distance_turnpt(DatumGetCbufferP(start),
    DatumGetCbufferP(end), DatumGetCbufferP(value), DatumGetCbufferP(value),
    lower, upper, t1, t2);
  /* The above temporary function sometimes provides inverted timestamps 
   * We patch it for the moment */
  if (*t1 > *t2)
  {
    TimestampTz temp = *t2;
    *t2 = *t1;
    *t1 = temp;
  }
  return result;
}

/**
 * @brief Return 1 if the segments of two temporal circular buffers intersect
 * during the period defined by the timestamps output in the last arguments
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 
 * @return Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
int
tcbuffersegm_intersection(Datum start1, Datum end1, Datum start2, Datum end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  /* While waiting for this function we cheat and call the function below */
  return cbuffersegm_distance_turnpt(DatumGetCbufferP(start1),
    DatumGetCbufferP(end1), DatumGetCbufferP(start2), DatumGetCbufferP(end2),
    lower, upper, t1, t2);
}

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * valid for operations
 * @param[in] temp Temporal value
 * @param[in] cb Value
 */
bool
ensure_valid_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(cb, false);
  if (! ensure_same_srid(tspatial_srid(temp), cbuffer_srid(cb)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 */
bool
ensure_valid_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a
 * spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 */
bool
ensure_valid_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) || 
      ! ensure_same_srid(tspatial_srid(temp), box->srid))
    return false;
  return true;
}

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * valid for operations
 * @param[in] temp1,temp2 Temporal value
 */
bool
ensure_valid_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp1, false); VALIDATE_TCBUFFER(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return false;
  return true;
}

/*****************************************************************************
 * Input/output
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_cbuffer_inout
 * @brief Return a temporal circular buffer from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tcbuffer_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TCBUFFER);
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer instant from its Well-Known Text 
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tcbufferinst_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer sequence from its Well-Known Text 
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tcbufferseq_in(const char *str, interpType interp UNUSED)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tcbufferseqset_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after the synchronization done in function 
 * #tcbuffer_make
 */
TInstant *
tcbufferinst_make(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst1->temptype == T_TGEOMPOINT);
  assert(inst2); assert(inst2->temptype == T_TFLOAT);
  assert(inst1->t == inst2->t);
  Cbuffer *cb = cbuffer_make(DatumGetGserializedP(tinstant_value_p(inst1)), 
    DatumGetFloat8(tinstant_value_p(inst2)));
  return tinstant_make_free(PointerGetDatum(cb), T_TCBUFFER, inst1->t);
}

/**
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after the synchronization done in function 
 * #tcbuffer_make
 */
TSequence *
tcbufferseq_make(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq1->temptype == T_TGEOMPOINT);
  assert(seq2); assert(seq2->temptype == T_TFLOAT);
  assert(seq1->count == seq2->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  for (int i = 0; i < seq1->count; i++)
    instants[i] = tcbufferinst_make(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i));
  return tsequence_make_free(instants, seq1->count, seq1->period.lower_inc, 
    seq1->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after the synchronization done in function 
 * #tcbuffer_make
 */
TSequenceSet *
tcbufferseqset_make(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss1->temptype == T_TGEOMPOINT);
  assert(ss2); assert(ss2->temptype == T_TFLOAT);
  assert(ss1->count == ss2->count);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
  for (int i = 0; i < ss1->count; i++)
    sequences[i] = tcbufferseq_make(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i));
  return tsequenceset_make_free(sequences, ss1->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @csqlfn #Tcbuffer_constructor()
 */
Temporal *
tcbuffer_make(const Temporal *tpoint, const Temporal *tfloat)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(tpoint, NULL); VALIDATE_TFLOAT(tfloat, NULL);

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal values do not intersect in time
   * The operation performed is synchronization without adding crossings */
  if (! intersection_temporal_temporal(tpoint, tfloat, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  assert(temptype_subtype(sync1->subtype));
  Temporal *result;
  switch (sync1->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tcbufferinst_make((TInstant *) sync1, 
        (TInstant *) sync2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tcbufferseq_make((TSequence *) sync1,
        (TSequence *) sync2);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tcbufferseqset_make((TSequenceSet *) sync1, 
        (TSequenceSet *) sync2);
  }
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return a temporal geometry point constructed from the points of a 
 * temporal circular buffer
 */
TInstant *
tcbufferinst_tgeompointinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  const GSERIALIZED *point = cbuffer_point_p(
    DatumGetCbufferP(tinstant_value_p(inst)));
  return tinstant_make(PointerGetDatum(point), T_TGEOMPOINT, inst->t);
}

/**
 * @brief Return a temporal geometry point constructed from the points of a 
 * temporal circular buffer
 */
TSequence *
tcbufferseq_tgeompointseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TCBUFFER);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tcbufferinst_tgeompointinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc, 
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal geometry point constructed from the points of a 
 * temporal circular buffer
 */
TSequenceSet *
tcbufferseqset_tgeompointseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TCBUFFER);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_tgeompointseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Return a temporal geometry point constructed from the points of a 
 * temporal circular buffer
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tgeompoint()
 */
Temporal *
tcbuffer_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_tgeompointinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_tgeompointseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_tgeompointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TInstant *
tcbufferinst_tfloatinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  double radius = cbuffer_radius(DatumGetCbufferP(tinstant_value_p(inst)));
  return tinstant_make(Float8GetDatum(radius), T_TFLOAT, inst->t);
}

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TSequence *
tcbufferseq_tfloatseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TCBUFFER);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tcbufferinst_tfloatinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc, 
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TSequenceSet *
tcbufferseqset_tfloatseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TCBUFFER);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_tfloatseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tfloat()
 */
Temporal *
tcbuffer_to_tfloat(const Temporal *temp)
{
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_tfloatinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_tfloatseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_tfloatseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Convert a temporal geometry point into a temporal circular buffer
 * with a zero radius
 */
TInstant *
tgeompointinst_tcbufferinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TGEOMPOINT);
  Cbuffer *cb = cbuffer_make(DatumGetGserializedP(tinstant_value_p(inst)), 0.0);
  if (cb == NULL)
    return NULL;
  return tinstant_make_free(PointerGetDatum(cb), T_TCBUFFER, inst->t);
}

/**
 * @brief Convert a temporal geometry point into a temporal circular buffer
 * with a zero radius
 */
TSequence *
tgeompointseq_tcbufferseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TGEOMPOINT);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tgeompointinst_tcbufferinst(TSEQUENCE_INST_N(seq, i));
    if (inst == NULL)
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
    instants[i] = inst;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Convert a temporal geometry point into a temporal circular buffer
 * with a zero radius
 */
TSequenceSet *
tgeompointseqset_tcbufferseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TGEOMPOINT);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tgeompointseq_tcbufferseq(TSEQUENCESET_SEQ_N(ss, i));
    if (seq == NULL)
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
    sequences[i] = seq;
  }
  return tsequenceset_make_free(sequences, ss->count, true);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Return a temporal geometry point transformed to a temporal circular
 * buffer with a zero radius
 * @param[in] temp Temporal point
 * @csqlfn #Tgeompoint_to_tcbuffer()
 */
Temporal *
tgeompoint_to_tcbuffer(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeompointinst_tcbufferinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tgeompointseq_tcbufferseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeompointseqset_tcbufferseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the start value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
Cbuffer *
tcbuffer_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  return DatumGetCbufferP(temporal_start_value(temp));
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the end value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
Cbuffer *
tcbuffer_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  return DatumGetCbufferP(temporal_end_value(temp));
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the n-th value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tcbuffer_value_n(const Temporal *temp, int n, Cbuffer **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetCbufferP(dresult);
  return true;
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of copies of base values of a temporal circular buffer
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Cbuffer **
tcbuffer_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(count, false);

  Datum *datumarr = temporal_values_p(temp, count);
  Cbuffer **result = palloc(sizeof(Cbuffer *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = cbuffer_copy(DatumGetCbufferP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
Set *
tcbufferinst_members(const TInstant *inst, bool point)
{
  Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  Datum value = point ? 
    PointerGetDatum(&cb->point) : Float8GetDatum(cb->radius);
  return set_make_exp(&value, 1, 1, point ? T_GEOMETRY : T_TFLOAT, ORDER_NO);
}

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
Set *
tcbufferseq_members(const TSequence *seq, bool point)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Cbuffer *cb = DatumGetCbufferP(
      tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    values[i] = point ? 
      PointerGetDatum(&cb->point) : Float8GetDatum(cb->radius);
  }
  datumarr_sort(values, seq->count, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, seq->count, T_GEOMETRY);
  /* Free the duplicate values that have been found */
  for (int i = count; i < seq->count; i++)
    pfree(DatumGetPointer(values[i]));
  return set_make_free(values, count, T_GEOMETRY, ORDER_NO);
}

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
Set *
tcbufferseqset_members(const TSequenceSet *ss, bool point)
{
  Datum *values = palloc(sizeof(Datum) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, j)));
      values[i] = point ? 
        PointerGetDatum(&cb->point) : Float8GetDatum(cb->radius);
    }
  }
  meosType basetype = point ? T_GEOMETRY : T_TFLOAT;
  datumarr_sort(values, ss->count, basetype);
  int count = datumarr_remove_duplicates(values, ss->count, basetype);
  /* Free the duplicate values that have been found */
  for (int i = count; i < ss->count; i++)
    pfree(DatumGetPointer(values[i]));
  return set_make_free(values, count, basetype, ORDER_NO);
}

/**
 * @ingroup meos_internal_cbuffer_accessor
 * @brief Return the points or radii or radius of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
Set *
tcbuffer_members(const Temporal *temp, bool point)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tcbufferinst_members((TInstant *) temp, point);
    case TSEQUENCE:
      return tcbufferseq_members((TSequence *) temp, point);
    default: /* TSEQUENCESET */
      return tcbufferseqset_members((TSequenceSet *) temp, point);
  }
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of points or radius of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
inline Set *
tcbuffer_points(const Temporal *temp)
{
  return tcbuffer_members(temp, true);
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of radii of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
inline Set *
tcbuffer_radius(const Temporal *temp)
{
  return tcbuffer_members(temp, false);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the value of a temporal circular buffer at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tcbuffer_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Cbuffer **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(value, false);
  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetCbufferP(res);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return a temporal circular buffer with the radius expanded by a
 * distance
 * @param[in] temp Temporal value
 * @param[in] dist Distance
 * @csqlfn #Tcbuffer_expand()
 */
Temporal *
tcbuffer_expand(const Temporal *temp, double dist)
{
  assert(temp); assert(temp->temptype == T_TCBUFFER);
  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  Temporal *tfloat = tcbuffer_to_tfloat(temp);
  Temporal *tfloat_exp = arithop_tnumber_number(tfloat, Float8GetDatum(dist),
    ADD, &datum_add, INVERT_NO);
  Temporal *result = tcbuffer_make(tpoint, tfloat_exp);
  pfree(tpoint); pfree(tfloat); pfree(tfloat_exp);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tcbuffer_restrict_cbuffer(const Temporal *temp, const Cbuffer *cb, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return temporal_restrict_value(temp, PointerGetDatum(cb), atfunc);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tcbuffer_at_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcbuffer_restrict_cbuffer(temp, cb, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a 
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tcbuffer_minus_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcbuffer_restrict_cbuffer(temp, cb, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_at_stbox()
 */
Temporal *
tcbuffer_restrict_stbox(const Temporal *temp, const STBox *box,
  bool border_inc UNUSED, bool atfunc)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(box, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_stbox(temp, box))
    return NULL;

  /* Bounding box test */
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  if (! overlaps_stbox_stbox(&box1, box))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  Temporal *tfloat = tcbuffer_to_tfloat(temp);
  Temporal *tpoint_rest = tgeo_restrict_stbox(tpoint, box, NULL, atfunc);
  if (! tpoint_rest)
    return NULL;
  Temporal *result = tcbuffer_make(tpoint_rest, tfloat);
  pfree(tpoint); pfree(tfloat); pfree(tpoint_rest);
  return result;
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_at_stbox()
 */
Temporal *
tcbuffer_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tcbuffer_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a 
 * geometry
 * @param[in] temp Temporal value
 * @param[in] box Value
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_minus_stbox()
 */
Temporal *
tcbuffer_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tcbuffer_restrict_stbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Tcbuffer_at_geom()
 */
Temporal *
tcbuffer_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, bool atfunc)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  Temporal *tfloat = tcbuffer_to_tfloat(temp);
  Temporal *tpoint_rest = tgeo_restrict_geom(tpoint, gs, NULL, atfunc);
  Temporal *result = NULL;
  if (tpoint_rest)
    result = tcbuffer_make(tpoint_rest, tfloat);
  pfree(tpoint); pfree(tfloat); pfree(tpoint_rest);
  return result;
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Tcbuffer_at_geom()
 */
Temporal *
tcbuffer_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs))
    return NULL;
  return tcbuffer_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a 
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Tcbuffer_minus_geom()
 */
Temporal *
tcbuffer_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  if (! ensure_valid_tcbuffer_geo(temp, gs))
    return NULL;
  return tcbuffer_restrict_geom(temp, gs, REST_MINUS);
}

/*****************************************************************************/

