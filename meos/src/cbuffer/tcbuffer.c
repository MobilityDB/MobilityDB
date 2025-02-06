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
 * @brief Basic functions for temporal circular buffers
 */

#include "cbuffer/tcbuffer.h"

/* C */
#include <assert.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Return a cbuffer interpolated from a cbuffer segment with respect to 
 * the fraction of its total length
 * @param[in] start,end Circular buffers defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the interpolated buffer must be located
 */
Datum
cbuffersegm_interpolate_point(Datum start, Datum end, long double ratio)
{
  Cbuffer *cbuf1 = DatumGetCbufferP(start);
  Cbuffer *cbuf2 = DatumGetCbufferP(end);
  Datum d1 = PointerGetDatum(&cbuf1->point);
  Datum d2 = PointerGetDatum(&cbuf2->point);
  GSERIALIZED *point = 
    DatumGetGserializedP(geosegm_interpolate_point(d1, d2, ratio));
  double radius = cbuf1->radius + 
    (double) ((long double)(cbuf2->radius - cbuf1->radius) * ratio);
  Cbuffer *result = cbuffer_make(point, radius);
  return PointerGetDatum(result);
}
  
/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after synchronization done in function 
 * #tcbuffer_constructor
 */
TInstant *
tcbufferinst_constructor(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst1->temptype == T_TGEOMPOINT);
  assert(inst2); assert(inst2->temptype == T_TFLOAT);
  assert(inst1->t == inst2->t);
  Cbuffer *cbuf = cbuffer_make(DatumGetGserializedP(tinstant_val(inst1)), 
    DatumGetFloat8(tinstant_val(inst2)));
  return tinstant_make_free(PointerGetDatum(cbuf), T_TCBUFFER, inst1->t);
}

/**
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after synchronization done in function 
 * #tcbuffer_constructor
 */
TSequence *
tcbufferseq_constructor(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq1->temptype == T_TGEOMPOINT);
  assert(seq2); assert(seq2->temptype == T_TFLOAT);
  assert(seq1->count == seq2->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  for (int i = 0; i < seq1->count; i++)
    instants[i] = tcbufferinst_constructor(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i));
  return tsequence_make_free(instants, seq1->count, seq1->period.lower_inc, 
    seq1->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @note This function is called after synchronization done in function 
 * #tcbuffer_constructor
 */
TSequenceSet *
tcbufferseqset_constructor(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss1->temptype == T_TGEOMPOINT);
  assert(ss2); assert(ss2->temptype == T_TFLOAT);
  assert(ss1->count == ss2->count);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
  for (int i = 0; i < ss1->count; i++)
    sequences[i] = tcbufferseq_constructor(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i));
  return tsequenceset_make_free(sequences, ss1->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal circular buffer from a temporal point and a 
 * temporal float
 * @csqlfn #Tcbuffer_constructor()
 */
Temporal *
tcbuffer_constructor(const Temporal *tpoint, const Temporal *tfloat)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) tpoint) || 
      ! ensure_temporal_isof_type(tpoint, T_TGEOMPOINT) ||
      ! ensure_not_null((void *) tfloat) || 
      ! ensure_temporal_isof_type(tfloat, T_TFLOAT))
    return NULL;

  Temporal *sync1, *sync2;
  /* Return false if the temporal values do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(tpoint, tfloat, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return false;

  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_constructor((TInstant *) sync1, 
        (TInstant *) sync2);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_constructor((TSequence *) sync1,
        (TSequence *) sync2);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_constructor((TSequenceSet *) sync1, 
        (TSequenceSet *) sync2);
  }
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return a temporal circular buffer converted to a temporal geometry point
 */
TInstant *
tcbufferinst_tgeompointinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  const GSERIALIZED *point = cbuffer_point(DatumGetCbufferP(tinstant_val(inst)));
  return tinstant_make(PointerGetDatum(point), T_TGEOMPOINT, inst->t);
}

/**
 * @brief Return a temporal circular buffer converted to a temporal geometry point
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
 * @brief Return a temporal circular buffer converted to a temporal geometry point
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
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal circular buffer converted to a temporal geometry point
 */
Temporal *
tcbuffer_tgeompoint(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TCBUFFER);
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

#if MEOS
/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal circular buffer transformed to a temporal geometry
 * point
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tgeompoint()
 */
Temporal *
tcbuffer_to_tgeompoint(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    return NULL;
  return tcbuffer_tgeompoint(temp);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return a temporal circular buffer converted to a temporal float
 */
TInstant *
tcbufferinst_tfloatinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  double radius = cbuffer_radius(DatumGetCbufferP(tinstant_val(inst)));
  return tinstant_make(Float8GetDatum(radius), T_TFLOAT, inst->t);
}

/**
 * @brief Return a temporal circular buffer converted to a temporal float
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
 * @brief Return a temporal circular buffer converted to a temporal float
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
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal circular buffer converted to a temporal float
 */
Temporal *
tcbuffer_tfloat(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TCBUFFER);
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

#if MEOS
/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal circular buffer transformed to a temporal float
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tfloat()
 */
Temporal *
tcbuffer_to_tfloat(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    return NULL;
  return tcbuffer_tfloat(temp);
}
#endif /* MEOS */

/*****************************************************************************/

// /**
 // * @brief Return a temporal geometry point converted to a temporal circular buffer
 // */
// TInstant *
// tgeompointinst_tcbufferinst(const TInstant *inst)
// {
  // assert(inst); assert(inst->temptype == T_TGEOMPOINT);
  // Cbuffer *np = geom_cbuffer(DatumGetGserializedP(tinstant_val(inst)));
  // if (np == NULL)
    // return NULL;
  // return tinstant_make_free(PointerGetDatum(np), T_TCBUFFER, inst->t);
// }

// /**
 // * @brief Return a temporal geometry point converted to a temporal circular buffer
 // */
// TSequence *
// tgeompointseq_tcbufferseq(const TSequence *seq)
// {
  // assert(seq); assert(seq->temptype == T_TGEOMPOINT);
  // TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  // for (int i = 0; i < seq->count; i++)
  // {
    // TInstant *inst = tgeompointinst_tcbufferinst(TSEQUENCE_INST_N(seq, i));
    // if (inst == NULL)
    // {
      // pfree_array((void **) instants, i);
      // return NULL;
    // }
    // instants[i] = inst;
  // }
  // return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    // seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
// }

// /**
 // * @brief Return a temporal geometry point converted to a temporal circular buffer
 // */
// TSequenceSet *
// tgeompointseqset_tcbufferseqset(const TSequenceSet *ss)
// {
  // assert(ss); assert(ss->temptype == T_TGEOMPOINT);
  // TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  // for (int i = 0; i < ss->count; i++)
  // {
    // TSequence *seq = tgeompointseq_tcbufferseq(TSEQUENCESET_SEQ_N(ss, i));
    // if (seq == NULL)
    // {
      // pfree_array((void **) sequences, i);
      // return NULL;
    // }
    // sequences[i] = seq;
  // }
  // return tsequenceset_make_free(sequences, ss->count, true);
// }

// /**
 // * @ingroup meos_temporal_spatial_transf
 // * @brief Return a temporal geometry point converted to a temporal circular buffer
 // * @csqlfn #Tgeompoint_to_tcbuffer()
 // */
// Temporal *
// tgeompoint_tcbuffer(const Temporal *temp)
// {
  // assert(temp); assert(temp->temptype == T_TGEOMPOINT);
  // int32_t srid_tpoint = tpoint_srid(temp);
  // int32_t srid_ways = get_srid_ways();
  // if (! ensure_same_srid(srid_tpoint, srid_ways))
    // return NULL;

  // assert(temptype_subtype(temp->subtype));
  // switch (temp->subtype)
  // {
    // case TINSTANT:
      // return (Temporal *) tgeompointinst_tcbufferinst((TInstant *) temp);
    // case TSEQUENCE:
      // return (Temporal *) tgeompointseq_tcbufferseq((TSequence *) temp);
    // default: /* TSEQUENCESET */
      // return (Temporal *) tgeompointseqset_tcbufferseqset((TSequenceSet *) temp);
  // }
// }

// #if MEOS
// /**
 // * @ingroup meos_temporal_spatial_transf
 // * @brief Return a temporal geometry point transformed to a temporal network
 // * point
 // * @param[in] temp Temporal point
 // * @csqlfn #Tgeompoint_to_tcbuffer()
 // */
// Temporal *
// tgeompoint_to_tcbuffer(const Temporal *temp)
// {
  // if (! ensure_not_null((void *) temp) ||
      // ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    // return NULL;
  // return tgeompoint_tcbuffer(temp);
// }
// #endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/*****************************************************************************/

/**
 * @brief Return the points of a temporal circular buffer
 */
Set *
tcbufferinst_points(const TInstant *inst)
{
  Cbuffer *cbuf = DatumGetCbufferP(tinstant_val(inst));
  Datum value = PointerGetDatum(&cbuf->point);
  return set_make_exp(&value, 1, 1, T_GEOMETRY, ORDER_NO);
}

/**
 * @brief Return the points of a temporal circular buffer
 */
Set *
tcbufferseq_points(const TSequence *seq)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Cbuffer *cbuf = DatumGetCbufferP(tinstant_val(TSEQUENCE_INST_N(seq, i)));
    values[i] = PointerGetDatum(&cbuf->point);
  }
  datumarr_sort(values, seq->count, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, seq->count, T_GEOMETRY);
  return set_make_free(values, count, T_GEOMETRY, ORDER_NO);
}

/**
 * @brief Return the points of a temporal circular buffer
 */
Set *
tcbufferseqset_points(const TSequenceSet *ss)
{
  Datum *values = palloc(sizeof(int64) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, i), 0);
    Cbuffer *cbuf = DatumGetCbufferP(tinstant_val(inst));
    values[i] = cbuf->point;
  }
  datumarr_sort(values, ss->count, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, ss->count, T_GEOMETRY);
  return set_make_free(values, count, T_GEOMETRY, ORDER_NO);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of points of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
Set *
tcbuffer_points(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tcbufferinst_points((TInstant *) temp);
    case TSEQUENCE:
      return tcbufferseq_points((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tcbufferseqset_points((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

