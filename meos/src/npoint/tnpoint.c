/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Basic functions for temporal network points.
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/


/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TInstant *
tnpointinst_tgeompointinst(const TInstant *inst)
{
  const Npoint *np = DatumGetNpointP(tinstant_value(inst));
  GSERIALIZED *geom = npoint_geom(np);
  TInstant *result = tinstant_make(PointerGetDatum(geom), T_TGEOMPOINT,
    inst->t);
  pfree(geom);
  return result;
}

/**
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TSequence *
tnpointdiscseq_tgeompointdiscseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    instants[i] = tnpointinst_tgeompointinst(inst);
  }
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TSequence *
tnpointcontseq_tgeompointcontseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const Npoint *np = DatumGetNpointP(tinstant_value(inst));
  /* We are sure line is not empty */
  GSERIALIZED *line = route_geom(np->rid);
  int srid = gserialized_get_srid(line);
  LWLINE *lwline = (LWLINE *) lwgeom_from_gserialized(line);
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    np = DatumGetNpointP(tinstant_value(inst));
    POINTARRAY *opa = lwline_interpolate_points(lwline, np->pos, 0);
    LWGEOM *lwpoint;
    assert(opa->npoints <= 1);
    lwpoint = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
    Datum point = PointerGetDatum(geo_serialize(lwpoint));
    instants[i] = tinstant_make(point, T_TGEOMPOINT, inst->t);
    lwpoint_free((LWPOINT *) lwpoint);
    pfree(DatumGetPointer(point));
  }

  pfree(line);
  lwline_free(lwline);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TSequenceSet *
tnpointseqset_tgeompointseqset(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tnpointcontseq_tgeompointcontseq(seq);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @brief Cast a temporal network point as a temporal geometric point.
 */
Temporal *
tnpoint_tgeompoint(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tnpointinst_tgeompointinst((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal *) tnpointdiscseq_tgeompointdiscseq((TSequence *) temp) :
      (Temporal *) tnpointcontseq_tgeompointcontseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnpointseqset_tgeompointseqset((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TInstant *
tgeompointinst_tnpointinst(const TInstant *inst)
{
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  Npoint *np = geom_npoint(gs);
  if (np == NULL)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(np), T_TNPOINT, inst->t);
  pfree(np);
  return result;
}

/**
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TSequence *
tgeompointseq_tnpointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TInstant *inst1 = tgeompointinst_tnpointinst(inst);
    if (inst1 == NULL)
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
    instants[i] = inst1;
  }
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  return result;
}

/**
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TSequenceSet *
tgeompointseqset_tnpointseqset(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TSequence *seq1 = tgeompointseq_tnpointseq(seq);
    if (seq1 == NULL)
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
    sequences[i] = seq1;
  }
  return tsequenceset_make_free(sequences, ss->count, true);
}

/**
 * @brief Cast a temporal geometric point as a temporal network point.
 */
Temporal *
tgeompoint_tnpoint(const Temporal *temp)
{
  int32_t srid_tpoint = tpoint_srid(temp);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_tpoint, srid_ways);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tgeompointinst_tnpointinst((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tgeompointseq_tnpointseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tgeompointseqset_tnpointseqset((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointinst_positions(const TInstant *inst)
{
  Nsegment **result = palloc(sizeof(Nsegment *));
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  result[0] = nsegment_make(np->rid, np->pos, np->pos);
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point.
 * @note The function is used for both discrete and step interpolation
 */
Nsegment **
tnpointseq_step_positions(const TSequence *seq, int *count)
{
  int count1;
  /* The following function removes duplicate values */
  Datum *values = tsequence_values(seq, &count1);
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
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment *
tnpointseq_linear_positions(const TSequence *seq)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  int64 rid = np->rid;
  double minPos = np->pos, maxPos = np->pos;
  for (int i = 1; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    np = DatumGetNpointP(tinstant_value(inst));
    minPos = Min(minPos, np->pos);
    maxPos = Max(maxPos, np->pos);
  }
  return nsegment_make(rid, minPos, maxPos);
}

/**
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseq_positions(const TSequence *seq, int *count)
{
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
  {
    Nsegment **result = palloc(sizeof(Nsegment *));
    result[0] = tnpointseq_linear_positions(seq);
    *count = 1;
    return result;
  }
  else
    return tnpointseq_step_positions(seq, count);
}

/**
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_linear_positions(const TSequenceSet *ss, int *count)
{
  Nsegment **segments = palloc(sizeof(Nsegment *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    segments[i] = tnpointseq_linear_positions(seq);
  }
  Nsegment **result = segments;
  int count1 = ss->count;
  if (count1 > 1)
    result = nsegmentarr_normalize(segments, &count1);
  *count = count1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_step_positions(const TSequenceSet *ss, int *count)
{
  /* The following function removes duplicate values */
  int newcount;
  Datum *values = tsequenceset_values(ss, &newcount);
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
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_positions(const TSequenceSet *ss, int *count)
{
  Nsegment **result;
  result = (MEOS_FLAGS_GET_LINEAR(ss->flags)) ?
    tnpointseqset_linear_positions(ss, count) :
    tnpointseqset_step_positions(ss, count);
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpoint_positions(const Temporal *temp, int *count)
{
  Nsegment **result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    result = tnpointinst_positions((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
    result = tnpointseq_positions((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tnpointseqset_positions((TSequenceSet *) temp, count);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the route of the temporal network point.
 */
int64
tnpointinst_route(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  return np->rid;
}

/**
 * @brief Return the single route of a temporal network point.
 */
int64
tnpoint_route(const Temporal *temp)
{
  if ( temp->subtype != TINSTANT && MEOS_FLAGS_GET_DISCRETE(temp->flags) )
    elog(ERROR, "Input must be a temporal instant or a temporal sequence with continuous interpolation");

  const TInstant *inst = (temp->subtype == TINSTANT) ?
    (const TInstant *) temp : TSEQUENCE_INST_N((const TSequence *) temp, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  return np->rid;
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointinst_routes(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  Datum value = Int64GetDatum(np->rid);
  return set_make(&value, 1, T_INT8, ORDERED);
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
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Npoint *np = DatumGetNpointP(tinstant_value(inst));
    values[i] = Int64GetDatum(np->rid);
  }
  datumarr_sort(values, seq->count, T_INT8);
  int count = datumarr_remove_duplicates(values, seq->count, T_INT8);
  Set *result = set_make(values, count, T_INT8, ORDERED);
  pfree(values);
  return result;
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointcontseq_routes(const TSequence *seq)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  Datum value = Int64GetDatum(np->rid);
  return set_make(&value, 1, T_INT8, ORDERED);
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    Npoint *np = DatumGetNpointP(tinstant_value(inst));
    values[i] = np->rid;
  }
  datumarr_sort(values, ss->count, T_INT8);
  int count = datumarr_remove_duplicates(values, ss->count, T_INT8);
  Set *result = set_make(values, count, T_INT8, ORDERED);
  pfree(values);
  return result;
}

/**
 * @brief Return the array of routes of a temporal network point
 */
Set *
tnpoint_routes(const Temporal *temp)
{
  Set *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tnpointinst_routes((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      tnpointdiscseq_routes((TSequence *) temp) :
      tnpointcontseq_routes((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnpointseqset_routes((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/
