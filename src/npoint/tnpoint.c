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
 * @file tnpoint.c
 * Basic functions for temporal network points.
 */

#include "npoint/tnpoint.h"

/* PostgreSQL */
#include <assert.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_parser.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_in);
/**
 * Input function for temporal network points
 */
PGDLLEXPORT Datum
tnpoint_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Oid basetypid = temporal_basetypid(temptypid);
  Temporal *result = temporal_parse(&input, basetypid);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast a temporal network point as a temporal geometric point
 */
static TInstant *
tnpointinst_tgeompointinst(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  Datum geom = npoint_geom(np);
  TInstant *result = tinstant_make(geom, inst->t, type_oid(T_GEOMETRY));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * Cast a temporal network point as a temporal geometric point
 */
static TInstantSet *
tnpointinstset_tgeompointinstset(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tnpointinst_tgeompointinst(inst);
  }
  TInstantSet *result = tinstantset_make_free(instants, ti->count, MERGE_NO);
  return result;
}

/**
 * Cast a temporal network point as a temporal geometric point
 */
static TSequence *
tnpointseq_tgeompointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  npoint *np = DatumGetNpoint(tinstant_value(tsequence_inst_n(seq, 0)));
  Datum line = route_geom(np->rid);
  /* We are sure line is not empty */
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int srid = gserialized_get_srid(gs);
  LWLINE *lwline = (LWLINE *)lwgeom_from_gserialized(gs);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    np = DatumGetNpoint(tinstant_value(inst));
    POINTARRAY* opa = lwline_interpolate_points(lwline, np->pos, 0);
    LWGEOM *lwpoint;
    assert(opa->npoints <= 1);
    lwpoint = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
    Datum point = PointerGetDatum(geo_serialize(lwpoint));
    instants[i] = tinstant_make(point, inst->t, type_oid(T_GEOMETRY));
    pfree(DatumGetPointer(point));
  }
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
  pfree(DatumGetPointer(line));
  return result;
}

/**
 * Cast a temporal network point as a temporal geometric point
 */
static TSequenceSet *
tnpointseqset_tgeompointseqset(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tnpointseq_tgeompointseq(seq);
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ts->count, false);
  return result;
}

/**
 * Cast a temporal network point as a temporal geometric point
 * (dispatch function)
 */
Temporal *
tnpoint_tgeompoint(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *)tnpointinst_tgeompointinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *)tnpointinstset_tgeompointinstset((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *)tnpointseq_tgeompointseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *)tnpointseqset_tgeompointseqset((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_to_tgeompoint);
/**
 * Cast a temporal network point as a temporal geometric point
 */
PGDLLEXPORT Datum
tnpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Cast a temporal geometric point as a temporal network point
 */
static TInstant *
tgeompointinst_tnpointinst(const TInstant *inst)
{
  Datum geom = tinstant_value(inst);
  npoint *np = geom_npoint(geom);
  if (np == NULL)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(np), inst->t,
    type_oid(T_NPOINT));
  pfree(np);
  return result;
}

/**
 * Cast a temporal geometric point as a temporal network point
 */
static TInstantSet *
tgeompointinstset_tnpointinstset(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    TInstant *inst1 = tgeompointinst_tnpointinst(inst);
    if (inst1 == NULL)
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
    instants[i] = inst1;
  }
  TInstantSet *result = tinstantset_make_free(instants, ti->count, MERGE_NO);
  return result;
}

/**
 * Cast a temporal geometric point as a temporal network point
 */
static TSequence *
tgeompointseq_tnpointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    TInstant *inst1 = tgeompointinst_tnpointinst(inst);
    if (inst1 == NULL)
    {
      for (int j = 0; j < i; j++)
        pfree(instants[j]);
      pfree(instants);
      return NULL;
    }
    instants[i] = inst1;
  }
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
  return result;
}

/**
 * Cast a temporal geometric point as a temporal network point
 */
static TSequenceSet *
tgeompointseqset_tnpointseqset(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    TSequence *seq1 = tgeompointseq_tnpointseq(seq);
    if (seq1 == NULL)
    {
      for (int j = 0; j < i; j++)
        pfree(sequences[j]);
      pfree(sequences);
      return NULL;
    }
    sequences[i] = seq1;
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ts->count, true);
  return result;
}

/**
 * Cast a temporal geometric point as a temporal network point
 * (dispatch function)
 */
static Temporal *
tgeompoint_tnpoint(Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *)tgeompointinst_tnpointinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *)tgeompointinstset_tnpointinstset((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *)tgeompointseq_tnpointseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *)tgeompointseqset_tnpointseqset((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_to_tnpoint);
/**
 * Cast a temporal geometric point as a temporal network point
 */
PGDLLEXPORT Datum
tgeompoint_to_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32_t srid_tpoint = tpoint_srid_internal(temp);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_tpoint, srid_ways);
  Temporal *result = tgeompoint_tnpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_round);
/**
 * Set the precision of the fraction of the temporal network point to the
 * number of decimal places
 */
PGDLLEXPORT Datum
tnpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &npoint_round_internal;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.restypid = temp->basetypid;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Return the route of the temporal network point
 */
int64
tnpointinst_route(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  return np->rid;
}

/**
 * Return the route of the temporal network point
 */
int64
tnpointseq_route(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  return np->rid;
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointinst_positions(const TInstant *inst)
{
  nsegment **result = palloc(sizeof(nsegment *));
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  result[0] = nsegment_make(np->rid, np->pos, np->pos);
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointinstset_positions(const TInstantSet *ti, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * ti->count);
  /* The following function removes duplicate values */
  int count1 = tinstantset_values(ti, values);
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointseq_step_positions(const TSequence *seq, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * seq->count);
  /* The following function removes duplicate values */
  int count1 = tsequence_values(seq, values);
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 */
nsegment *
tnpointseq_linear_positions(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  int64 rid = np->rid;
  double minPos = np->pos, maxPos = np->pos;
  for (int i = 1; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    np = DatumGetNpoint(tinstant_value(inst));
    minPos = Min(minPos, np->pos);
    maxPos = Max(maxPos, np->pos);
  }
  return nsegment_make(rid, minPos, maxPos);
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointseq_positions(const TSequence *seq, int *count)
{
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    nsegment **result = palloc(sizeof(nsegment *));
    result[0] = tnpointseq_linear_positions(seq);
    *count = 1;
    return result;
  }
  else
    return tnpointseq_step_positions(seq, count);
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointseqset_linear_positions(const TSequenceSet *ts, int *count)
{
  nsegment **segments = palloc(sizeof(nsegment *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    segments[i] = tnpointseq_linear_positions(seq);
  }
  nsegment **result = segments;
  int count1 = ts->count;
  if (count1 > 1)
    result = nsegmentarr_normalize(segments, &count1);
  *count = count1;
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 */
static nsegment **
tnpointseqset_step_positions(const TSequenceSet *ts, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * ts->totalcount);
  /* The following function removes duplicate values */
  int count1 = tsequenceset_values(ts, values);
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 */
nsegment **
tnpointseqset_positions(const TSequenceSet *ts, int *count)
{
  nsegment **result;
  if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
    result = tnpointseqset_linear_positions(ts, count);
  else
    result = tnpointseqset_step_positions(ts, count);
  return result;
}

/**
 * Return the network segments covered by the temporal network point
 * (dispatch function)
 */
static nsegment **
tnpoint_positions_internal(const Temporal *temp, int *count)
{
  nsegment **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tnpointinst_positions((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
    result = tnpointinstset_positions((TInstantSet *) temp, count);
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_positions((TSequence *) temp, count);
  else /* temp->subtype == SEQUENCESET */
    result = tnpointseqset_positions((TSequenceSet *) temp, count);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_positions);
/**
 * Return the network segments covered by the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_positions(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  nsegment **segments = tnpoint_positions_internal(temp, &count);
  ArrayType *result = nsegmentarr_to_array(segments, count);
  for (int i = 0; i < count; i++)
    pfree(segments[i]);
  pfree(segments);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_route);
/**
 * Return the route of a temporal network point
 */
PGDLLEXPORT Datum
tnpoint_route(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  if (temp->subtype != INSTANT && temp->subtype != SEQUENCE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal instant or a temporal sequence")));

  const TInstant *inst = (temp->subtype == INSTANT) ?
    (TInstant *) temp : tsequence_inst_n((TSequence *) temp, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  int64 result = np->rid;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT64(result);
}

/**
 * Return the array of routes of a temporal network point
 */
static ArrayType *
tnpointinst_routes(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  ArrayType *result = int64arr_to_array(&np->rid, 1);
  return result;
}

/**
 * Return the array of routes of a temporal network point
 */
static ArrayType *
tnpointinstset_routes(const TInstantSet *ti)
{
  int64 *routes = palloc(sizeof(int64) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    npoint *np = DatumGetNpoint(tinstant_value(inst));
    routes[i] = np->rid;
  }
  ArrayType *result = int64arr_to_array(routes, ti->count);
  pfree(routes);
  return result;
}

/**
 * Return the array of routes of a temporal network point
 */
static ArrayType *
tnpointseq_routes(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  ArrayType *result = int64arr_to_array(&np->rid, 1);
  return result;
}

/**
 * Return the array of routes of a temporal network point
 */
static ArrayType *
tnpointseqset_routes(const TSequenceSet *ts)
{
  int64 *routes = palloc(sizeof(int64) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tsequence_inst_n(seq, 0);
    npoint *np = DatumGetNpoint(tinstant_value(inst));
    routes[i] = np->rid;
  }
  ArrayType *result = int64arr_to_array(routes, ts->count);
  pfree(routes);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_routes);
/**
 * Return the array of routes of a temporal network point
 */
PGDLLEXPORT Datum
tnpoint_routes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ArrayType *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tnpointinst_routes((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tnpointinstset_routes((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_routes((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnpointseqset_routes((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
