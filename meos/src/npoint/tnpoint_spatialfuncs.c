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
 * @brief Spatial functions for temporal network points
 */

#include "npoint/tnpoint_spatialfuncs.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that two temporal network point instants have the same route
 * identifier
 */
bool
ensure_same_rid_tnpointinst(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2);
  if (tnpointinst_route(inst1) != tnpointinst_route(inst2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "All network points composing a temporal sequence must have same route identifier");
    return false;
  }
  return true;
}

/*****************************************************************************
 * Interpolation functions required by tsequence.c that must be implemented
 * for each base type that supports linear interpolation
 *****************************************************************************/

/**
 * @brief Return a network point interpolated from a network point segment
 * with respect to a fraction of its total length
 * @param[in] start,end Network points defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the interpolated network point is located
 */
Npoint *
npointsegm_interpolate(const Npoint *start, const Npoint *end,
  long double ratio)
{
  assert(ratio >= 0.0 && ratio <= 1.0);
  double pos = start->pos + 
    (double) ((long double)(end->pos - start->pos) * ratio);
  Npoint *result = npoint_make(start->rid, pos);
  return result;
}

/**
 * @brief Return true if a segment of a temporal network point value intersects
 * a base value at the timestamp
 * @param[in] start,end Temporal instants defining the segment
 * @param[in] value Base value
 */
long double
npointsegm_locate(const Npoint *start, const Npoint *end, const Npoint *value)
{
  /* This function is called for temporal sequences and thus the three values
   * have the same road identifier */
  assert(start->rid == end->rid); assert(start->rid == value->rid);
  double min = Min(start->pos, end->pos);
  double max = Max(start->pos, end->pos);
  /* If value is to the left or to the right of the range */
  if ((value->rid != start->rid) ||
    (value->pos < start->pos && value->pos < end->pos) ||
    (value->pos > start->pos && value->pos > end->pos))
  // if (value->rid != start->rid || (value->pos < min && value->pos > max))
    return -1.0;

  double range = (max - min);
  double partial = (value->pos - min);
  double fraction = start->pos < end->pos ? partial / range : 1 - partial / range;
  if (fabs(fraction) < MEOS_EPSILON || fabs(fraction - 1.0) < MEOS_EPSILON)
    return -1.0;
  return fraction;
}

/*****************************************************************************
 * NPoints Functions
 * Return the network points covered by a temporal network point
 *****************************************************************************/

/**
 * @brief Return the network points covered by a temporal network point
 * @param[in] seq Temporal network point
 * @param[out] count Number of elements of the output array
 * @note Only the particular cases returning points are covered
 */
static Npoint **
tnpointseq_discstep_npoints(const TSequence *seq, int *count)
{
  Npoint **result = palloc(sizeof(Npoint *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = DatumGetNpointP(tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
  *count = seq->count;
  return result;
}

/**
 * @brief Return the pointers to the network points covered by a temporal
 * network point
 * @param[in] ss Temporal network point
 * @param[out] count Number of elements of the output array
 * @note Only the particular cases returning points are covered
 */
static Npoint **
tnpointseqset_step_npoints(const TSequenceSet *ss, int *count)
{
  Npoint **result = palloc(sizeof(Npoint *) * ss->totalcount);
  int npoints = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
      result[npoints++] = DatumGetNpointP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, j)));
  }
  *count = npoints;
  return result;
}

/*****************************************************************************
 * Geometric positions (Trajectotry) functions
 * Return the geometric positions covered by a temporal network point
 *****************************************************************************/

/**
 * @brief Return the trajectory of a temporal network point
 * @param[in] inst Temporal network point
 */
GSERIALIZED *
tnpointinst_trajectory(const TInstant *inst)
{
  const Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  return npoint_geom(np);
}

/**
 * @brief Return the trajectory a temporal network point
 * @param[in] seq Temporal network point
 */
GSERIALIZED *
tnpointseq_trajectory(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return tnpointinst_trajectory(TSEQUENCE_INST_N(seq, 0));

  GSERIALIZED *result;
  if (MEOS_FLAGS_LINEAR_INTERP(seq->flags))
  {
    Nsegment *segment = tnpointseq_linear_positions(seq);
    result = nsegment_geom(segment);
    pfree(segment);
  }
  else
  {
    int count;
    /* The following function does not remove duplicate values */
    Npoint **points = tnpointseq_discstep_npoints(seq, &count);
    result = npointarr_geom(points, count);
    pfree(points);
  }
  return result;
}

/**
 * @brief Return the trajectory of a temporal network point
 * @param[in] ss Temporal network point
 */
GSERIALIZED *
tnpointseqset_trajectory(const TSequenceSet *ss)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnpointseq_trajectory(TSEQUENCESET_SEQ_N(ss, 0));

  int count;
  GSERIALIZED *result;
  if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
  {
    Nsegment **segments = tnpointseqset_positions(ss, &count);
    result = nsegmentarr_geom(segments, count);
    pfree_array((void **) segments, count);
  }
  else
  {
    Npoint **points = tnpointseqset_step_npoints(ss, &count);
    result = npointarr_geom(points, count);
    pfree(points);
  }
  return result;
}

/**
 * @ingroup meos_npoint_spatial_transf
 * @brief Return the geometry covered by a temporal network point
 * @param[in] temp Temporal network point
 * @csqlfn #Tnpoint_trajectory()
 */
GSERIALIZED *
tnpoint_trajectory(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnpointinst_trajectory((TInstant *) temp);
    case TSEQUENCE:
      return tnpointseq_trajectory((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnpointseqset_trajectory((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Approximate equality for network points
 *****************************************************************************/

/**
 * @ingroup meos_npoint_comp
 * @brief Return true if two network points are approximately equal with
 * respect to an epsilon value
 * @details Two network points may be have different route identifier but
 * represent the same spatial point at the intersection of the two route
 * identifiers
 */
bool
npoint_same(const Npoint *np1, const Npoint *np2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np1, NULL); VALIDATE_NOT_NULL(np2, NULL);

  /* Equal route identifier and same position */
  if (np1->rid == np2->rid && fabs(np1->pos - np2->pos) > MEOS_EPSILON)
    return false;
  /* Same point */
  Datum point1 = PointerGetDatum(npoint_geom(np1));
  Datum point2 = PointerGetDatum(npoint_geom(np2));
  bool result = datum_point_same(point1, point2);
  pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
  return result;
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/**
 * @brief Length traversed by a temporal network point
 */
double
tnpointseq_length(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const Npoint *np1 = DatumGetNpointP(tinstant_value_p(inst));
  double length = route_length(np1->rid);
  double fraction = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    const Npoint *np2 = DatumGetNpointP(tinstant_value_p(inst));
    fraction += fabs(np2->pos - np1->pos);
    np1 = np2;
  }
  return length * fraction;
}

/**
 * @brief Length traversed by a temporal network point
 */
double
tnpointseqset_length(const TSequenceSet *ss)
{
  double result = 0.0;
  for (int i = 0; i < ss->count; i++)
    result += tnpointseq_length(TSEQUENCESET_SEQ_N(ss, i));
  return result;
}

/**
 * @ingroup meos_npoint_spatial_accessor
 * @brief Length traversed by a temporal network point
 * @param[in] temp Temporal point
 * @csqlfn #Tnpoint_length()
 */
double
tnpoint_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, -1.0);

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return 0.0;
  else if (temp->subtype == TSEQUENCE)
    return tnpointseq_length((TSequence *) temp);
  else /* TSEQUENCESET */
    return tnpointseqset_length((TSequenceSet *) temp);
}

/*****************************************************************************/

/**
 * @brief Return the cumulative length traversed by a temporal point
 * @pre The sequence has linear interpolation
 */
static TSequence *
tnpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  assert(seq); assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(Float8GetDatum(prevlength), T_TFLOAT,
      TSEQUENCE_INST_N(seq, 0)->t);
    return tinstant_to_tsequence_free(inst, LINEAR);
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Npoint *np1 = DatumGetNpointP(tinstant_value_p(inst1));
  double rlength = route_length(np1->rid);
  double length = prevlength;
  instants[0] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Npoint *np2 = DatumGetNpointP(tinstant_value_p(inst2));
    length += fabs(np2->pos - np1->pos) * rlength;
    instants[i] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst2->t);
    np1 = np2;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @brief Cumulative length traversed by a temporal network point
 */
static TSequenceSet *
tnpointseqset_cumulative_length(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  double length = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tnpointseq_cumulative_length(seq, length);
    const TInstant *end = TSEQUENCE_INST_N(sequences[i], seq->count - 1);
    length += DatumGetFloat8(tinstant_value_p(end));
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_npoint_spatial_accessor
 * @brief Cumulative length traversed by a temporal network point
 * @param[in] temp Temporal point
 * @csqlfn #Tnpoint_cumulative_length()
 */
Temporal *
tnpoint_cumulative_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return temporal_from_base_temp(Float8GetDatum(0.0), T_TFLOAT, temp);
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) tnpointseq_cumulative_length((TSequence *) temp, 0);
  else /* TSEQUENCESET */
    return (Temporal *) tnpointseqset_cumulative_length((TSequenceSet *) temp);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * @brief Speed of a temporal network point
 * @csqlfn #Tnpoint_speed()
 */
static TSequence *
tnpointseq_speed(const TSequence *seq)
{
  assert(seq); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Npoint *np1 = DatumGetNpointP(tinstant_value_p(inst1));
  double rlength = route_length(np1->rid);
  const TInstant *inst2 = NULL; /* make the compiler quiet */
  double speed = 0; /* make the compiler quiet */
  for (int i = 0; i < seq->count - 1; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i + 1);
    Npoint *np2 = DatumGetNpointP(tinstant_value_p(inst2));
    double length = fabs(np2->pos - np1->pos) * rlength;
    speed = length / (((double)(inst2->t) - (double)(inst1->t)) / 1000000);
    instants[i] = tinstant_make(Float8GetDatum(speed), T_TFLOAT, inst1->t);
    inst1 = inst2;
    np1 = np2;
  }
  instants[seq->count-1] = tinstant_make(Float8GetDatum(speed), T_TFLOAT,
    inst2->t);
  /* The resulting sequence has step interpolation */
  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, true);
}

/**
 * @brief Speed of a temporal network point
 */
static TSequenceSet *
tnpointseqset_speed(const TSequenceSet *ss)
{
  assert(ss); assert(tspatial_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tnpointseq_speed(TSEQUENCESET_SEQ_N(ss, i));
    if (seq)
      sequences[nseqs++] = seq;
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, STEP);
}

/**
 * @ingroup meos_npoint_spatial_accessor
 * @brief Speed of a temporal network point
 * @param[in] temp Temporal point
 * @csqlfn #Tnpoint_speed()
 */
Temporal *
tnpoint_speed(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);
  if (! ensure_linear_interp(temp->flags))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tnpointseq_speed((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnpointseqset_speed((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Time-weighed centroid for temporal network points
 *****************************************************************************/

/**
 * @ingroup meos_npoint_spatial_accessor
 * @brief Return the time-weighed centroid of a temporal network point
 * @param[in] temp Temporal point
 * @csqlfn #Tnpoint_twcentroid()
 */
GSERIALIZED *
tnpoint_twcentroid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  GSERIALIZED *result = tpoint_twcentroid(tpoint);
  pfree(tpoint);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_npoint_restrict
 * @brief Return a temporal network point restricted to (the complement of) a
 * geometry
 */
Temporal *
tnpoint_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_has_not_Z_geo(gs))
    return NULL;

  /* Empty geometry */
  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *res = tgeo_restrict_geom(tpoint, gs, zspan, atfunc);
  Temporal *result = NULL;
  if (res)
  {
    /* We do not call the function tgeompoint_tnpoint to avoid
     * roundoff errors */
    SpanSet *ss = temporal_time(res);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(res);
    pfree(ss);
  }
  pfree(tpoint);
  return result;
}

#if MEOS
/**
 * @ingroup meos_npoint_restrict
 * @brief Return a temporal network point restricted to a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] zspan Span of values to restrict the Z dimension
 * @csqlfn #Tnpoint_at_geom()
 */
inline Temporal *
tnpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
  return tnpoint_restrict_geom(temp, gs, zspan, REST_AT);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Return a temporal point restricted to (the complement of) a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] zspan Span of values to restrict the Z dimension
 * @csqlfn #Tnpoint_minus_geom()
 */
inline Temporal *
tnpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
  return tnpoint_restrict_geom(temp, gs, zspan, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_npoint_restrict
 * @brief Return a temporal network point restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is at, false for minus
 */
Temporal *
tnpoint_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(box, NULL);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *res = tgeo_restrict_stbox(tpoint, box, border_inc, atfunc);
  Temporal *result = NULL;
  if (res)
  {
    result = tgeompoint_tnpoint(res);
    pfree(res);
  }
  return result;
}

#if MEOS
/**
 * @ingroup meos_npoint_restrict
 * @brief Return a temporal network point restricted to a geometry
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @sqlfn #Tnpoint_at_stbox()
 */
inline Temporal *
tnpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tnpoint_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Return a temporal point restricted to (the complement of) a geometry
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @sqlfn #Tnpoint_minus_stbox()
 */
inline Temporal *
tnpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tnpoint_restrict_stbox(temp, box, border_inc, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/
