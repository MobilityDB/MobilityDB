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
 * @brief Basic functions for temporal network points
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return a temporal network point converted to a temporal geometry point
 */
TInstant *
tnpointinst_tgeompointinst(const TInstant *inst)
{
  GSERIALIZED *geom = npoint_geom(DatumGetNpointP(tinstant_val(inst)));
  return tinstant_make_free(PointerGetDatum(geom), T_TGEOMPOINT, inst->t);
}

/**
 * @brief Return a temporal network point converted to a temporal geometry point
 */
TSequence *
tnpointdiscseq_tgeompointdiscseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tnpointinst_tgeompointinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Return a temporal network point converted to a temporal geometry point
 */
TSequence *
tnpointcontseq_tgeompointcontseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const Npoint *np = DatumGetNpointP(tinstant_val(inst));
  /* We are sure line is not empty */
  GSERIALIZED *line = route_geom(np->rid);
  int srid = gserialized_get_srid(line);
  LWLINE *lwline = (LWLINE *) lwgeom_from_gserialized(line);
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    np = DatumGetNpointP(tinstant_val(inst));
    POINTARRAY *opa = lwline_interpolate_points(lwline, np->pos, 0);
    assert(opa->npoints <= 1);
    LWGEOM *lwpoint = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
    Datum point = PointerGetDatum(geo_serialize(lwpoint));
    lwgeom_free(lwpoint);
    instants[i] = tinstant_make_free(point, T_TGEOMPOINT, inst->t);
  }

  pfree(line); lwline_free(lwline);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal network point converted to a temporal geometry point
 */
TSequenceSet *
tnpointseqset_tgeompointseqset(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tnpointcontseq_tgeompointcontseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @brief Return a temporal network point converted to a temporal geometry point
 */
Temporal *
tnpoint_tgeompoint(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tnpointinst_tgeompointinst((TInstant *) temp);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal *) tnpointdiscseq_tgeompointdiscseq((TSequence *) temp) :
        (Temporal *) tnpointcontseq_tgeompointcontseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnpointseqset_tgeompointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal geometry point converted to a temporal network point
 */
TInstant *
tgeompointinst_tnpointinst(const TInstant *inst)
{
  Npoint *np = geom_npoint(DatumGetGserializedP(tinstant_val(inst)));
  if (np == NULL)
    return NULL;
  return tinstant_make_free(PointerGetDatum(np), T_TNPOINT, inst->t);
}

/**
 * @brief Return a temporal geometry point converted to a temporal network point
 */
TSequence *
tgeompointseq_tnpointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tgeompointinst_tnpointinst(TSEQUENCE_INST_N(seq, i));
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
 * @brief Return a temporal geometry point converted to a temporal network point
 */
TSequenceSet *
tgeompointseqset_tnpointseqset(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tgeompointseq_tnpointseq(TSEQUENCESET_SEQ_N(ss, i));
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
 * @brief Return a temporal geometry point converted to a temporal network point
 */
Temporal *
tgeompoint_tnpoint(const Temporal *temp)
{
  int32_t srid_tpoint = tpoint_srid(temp);
  int32_t srid_ways = get_srid_ways();
  if (! ensure_same_srid(srid_tpoint, srid_ways))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeompointinst_tnpointinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tgeompointseq_tnpointseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeompointseqset_tnpointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointinst_positions(const TInstant *inst)
{
  Nsegment **result = palloc(sizeof(Nsegment *));
  Npoint *np = DatumGetNpointP(tinstant_val(inst));
  result[0] = nsegment_make(np->rid, np->pos, np->pos);
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 * @note The function is used for both discrete and step interpolation
 */
Nsegment **
tnpointseq_step_positions(const TSequence *seq, int *count)
{
  int count1;
  /* The following function removes duplicate values */
  Datum *values = tsequence_vals(seq, &count1);
  Nsegment **result = palloc(sizeof(Nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    Npoint *np = DatumGetNpointP(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  pfree(values);
  *count = count1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment *
tnpointseq_linear_positions(const TSequence *seq)
{
  Npoint *np = DatumGetNpointP(tinstant_val(TSEQUENCE_INST_N(seq, 0)));
  int64 rid = np->rid;
  double minPos = np->pos, maxPos = np->pos;
  for (int i = 1; i < seq->count; i++)
  {
    np = DatumGetNpointP(tinstant_val(TSEQUENCE_INST_N(seq, i)));
    minPos = Min(minPos, np->pos);
    maxPos = Max(maxPos, np->pos);
  }
  return nsegment_make(rid, minPos, maxPos);
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseq_positions(const TSequence *seq, int *count)
{
  if (! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    return tnpointseq_step_positions(seq, count);

  Nsegment **result = palloc(sizeof(Nsegment *));
  result[0] = tnpointseq_linear_positions(seq);
  *count = 1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_linear_positions(const TSequenceSet *ss, int *count)
{
  Nsegment **segments = palloc(sizeof(Nsegment *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    segments[i] = tnpointseq_linear_positions(TSEQUENCESET_SEQ_N(ss, i));
  Nsegment **result = segments;
  int count1 = ss->count;
  if (count1 > 1)
    result = nsegmentarr_normalize(segments, &count1);
  *count = count1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_step_positions(const TSequenceSet *ss, int *count)
{
  /* The following function removes duplicate values */
  int newcount;
  Datum *values = tsequenceset_vals(ss, &newcount);
  Nsegment **result = palloc(sizeof(Nsegment *) * newcount);
  for (int i = 0; i < newcount; i++)
  {
    Npoint *np = DatumGetNpointP(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  pfree(values);
  *count = newcount;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_positions(const TSequenceSet *ss, int *count)
{
  return (MEOS_FLAGS_LINEAR_INTERP(ss->flags)) ?
    tnpointseqset_linear_positions(ss, count) :
    tnpointseqset_step_positions(ss, count);
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpoint_positions(const Temporal *temp, int *count)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnpointinst_positions((TInstant *) temp);
    case TSEQUENCE:
      return tnpointseq_positions((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tnpointseqset_positions((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return the route of the temporal network point
 */
int64
tnpointinst_route(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_val(inst));
  return np->rid;
}

/**
 * @brief Return the single route of a temporal network point
 * @return On error return @p INT_MAX
 */
int64
tnpoint_route(const Temporal *temp)
{
  if ( temp->subtype != TINSTANT && MEOS_FLAGS_DISCRETE_INTERP(temp->flags) )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Input must be a temporal instant or a temporal sequence with continuous interpolation");
    return INT_MAX;
  }
  const TInstant *inst = (temp->subtype == TINSTANT) ?
    (const TInstant *) temp : TSEQUENCE_INST_N((const TSequence *) temp, 0);
  Npoint *np = DatumGetNpointP(tinstant_val(inst));
  return np->rid;
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointinst_routes(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_val(inst));
  Datum value = Int64GetDatum(np->rid);
  return set_make_exp(&value, 1, 1, T_INT8, ORDERED);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointdiscseq_routes(const TSequence *seq)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Npoint *np = DatumGetNpointP(tinstant_val(TSEQUENCE_INST_N(seq, i)));
    values[i] = Int64GetDatum(np->rid);
  }
  datumarr_sort(values, seq->count, T_INT8);
  int count = datumarr_remove_duplicates(values, seq->count, T_INT8);
  return set_make_free(values, count, T_INT8, ORDERED);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointcontseq_routes(const TSequence *seq)
{
  const Npoint *np = DatumGetNpointP(tinstant_val(TSEQUENCE_INST_N(seq, 0)));
  Datum value = Int64GetDatum(np->rid);
  return set_make_exp(&value, 1, 1, T_INT8, ORDERED);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointseqset_routes(const TSequenceSet *ss)
{
  Datum *values = palloc(sizeof(int64) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, i), 0);
    Npoint *np = DatumGetNpointP(tinstant_val(inst));
    values[i] = np->rid;
  }
  datumarr_sort(values, ss->count, T_INT8);
  int count = datumarr_remove_duplicates(values, ss->count, T_INT8);
  return set_make_free(values, count, T_INT8, ORDERED);
}

/**
 * @brief Return the array of routes of a temporal network point
 */
Set *
tnpoint_routes(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnpointinst_routes((TInstant *) temp);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        tnpointdiscseq_routes((TSequence *) temp) :
        tnpointcontseq_routes((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnpointseqset_routes((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

