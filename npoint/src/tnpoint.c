/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

#include "tnpoint.h"

#include <assert.h>

#include "temporaltypes.h"
#include "temporal_parser.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tpoint_spatialfuncs.h"
#include "tnpoint_static.h"
#include "tnpoint_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_in);

PGDLLEXPORT Datum
tnpoint_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Oid valuetypid = temporal_valuetypid(temptypid);
  Temporal *result = temporal_parse(&input, valuetypid);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast tnpoint as tgeompoint */

TInstant *
tnpointinst_as_tgeompointinst(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  Datum geom = npoint_as_geom_internal(np);
  TInstant *result = tinstant_make(geom, inst->t, type_oid(T_GEOMETRY));
  pfree(DatumGetPointer(geom));
  return result;
}

TInstantSet *
tnpointi_as_tgeompointi(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tnpointinst_as_tgeompointinst(inst);
  }
  TInstantSet *result = tinstantset_make_free(instants, ti->count, MERGE_NO);
  return result;
}

TSequence *
tnpointseq_as_tgeompointseq(const TSequence *seq)
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

TSequenceSet *
tnpoints_as_tgeompoints(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tnpointseq_as_tgeompointseq(seq);
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ts->count, false);
  return result;
}

Temporal *
tnpoint_as_tgeompoint_internal(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *)tnpointinst_as_tgeompointinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *)tnpointi_as_tgeompointi((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *)tnpointseq_as_tgeompointseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *)tnpoints_as_tgeompoints((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_as_tgeompoint);

PGDLLEXPORT Datum
tnpoint_as_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = tnpoint_as_tgeompoint_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Cast tgeompoint as tnpoint */

TInstant *
tgeompointinst_as_tnpointinst(const TInstant *inst)
{
  Datum geom = tinstant_value(inst);
  npoint *np = geom_as_npoint_internal(geom);
  if (np == NULL)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(np), inst->t,
    type_oid(T_NPOINT));
  pfree(np);
  return result;
}

TInstantSet *
tgeompointi_as_tnpointi(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    TInstant *inst1 = tgeompointinst_as_tnpointinst(inst);
    if (inst1 == NULL)
    {
      for (int j = 0; j < i; j++)
        pfree(instants[j]);
      pfree(instants);
      return NULL;
    }
    instants[i] = inst1;
  }
  TInstantSet *result = tinstantset_make_free(instants, ti->count, MERGE_NO);
  return result;
}

TSequence *
tgeompointseq_as_tnpointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    TInstant *inst1 = tgeompointinst_as_tnpointinst(inst);
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

TSequenceSet *
tgeompoints_as_tnpoints(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    TSequence *seq1 = tgeompointseq_as_tnpointseq(seq);
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

Temporal *
tgeompoint_as_tnpoint_internal(Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *)tgeompointinst_as_tnpointinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *)tgeompointi_as_tnpointi((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *)tgeompointseq_as_tnpointseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *)tgeompoints_as_tnpoints((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_as_tnpoint);

PGDLLEXPORT Datum
tgeompoint_as_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = tgeompoint_as_tnpoint_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/*
 * Route functions for temporal instants and sequences
 */

int64
tnpointinst_route(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  return np->rid;
}

int64
tnpointiseq_route(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  return np->rid;
}

/*
 * Positions functions
 * Return the network segments covered by the moving object
 */

nsegment **
tnpointinst_positions(const TInstant *inst)
{
  nsegment **result = palloc(sizeof(nsegment *));
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  result[0] = nsegment_make(np->rid, np->pos, np->pos);
  return result;
}

nsegment **
tnpointi_positions(const TInstantSet *ti, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * ti->count);
  /* The following function removes duplicate values */
  int count1 = tinstantset_values(values, ti);   
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

nsegment **
tnpointseq_step_positions(const TSequence *seq, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * seq->count);
  /* The following function removes duplicate values */
  int count1 = tsequence_values(values, seq);
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

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

nsegment **
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

nsegment **
tnpoints_linear_positions(const TSequenceSet *ts, int *count)
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

nsegment **
tnpoints_step_positions(const TSequenceSet *ts, int *count)
{
  Datum *values = palloc(sizeof(Datum *) * ts->totalcount);
  /* The following function removes duplicate values */
  int count1 = tsequenceset_values(values, ts);
  nsegment **result = palloc(sizeof(nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    npoint *np = DatumGetNpoint(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  *count = count1;
  return result;
}

nsegment **
tnpoints_positions(const TSequenceSet *ts, int *count)
{
  nsegment **result;
  if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
    result = tnpoints_linear_positions(ts, count);
  else
    result = tnpoints_step_positions(ts, count);
  return result;
}

nsegment **
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
    result = tnpointi_positions((TInstantSet *) temp, count);
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_positions((TSequence *) temp, count);
  else /* temp->subtype == SEQUENCESET */
    result = tnpoints_positions((TSequenceSet *) temp, count);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_positions);

PGDLLEXPORT Datum
tnpoint_positions(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
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

/* Route of a temporal instant */

PG_FUNCTION_INFO_V1(tnpoint_route);

PGDLLEXPORT Datum
tnpoint_route(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
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

/*
 * Routes functions
 * Return the routes covered by the moving object
 */

ArrayType *
tnpointinst_routes(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  ArrayType *result = int64arr_to_array(&np->rid, 1);
  return result;
}

ArrayType *
tnpointi_routes(const TInstantSet *ti)
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

ArrayType *
tnpointseq_routes(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  ArrayType *result = int64arr_to_array(&np->rid, 1);
  return result;
}

ArrayType *
tnpoints_routes(const TSequenceSet *ts)
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

PGDLLEXPORT Datum
tnpoint_routes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tnpointinst_routes((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tnpointi_routes((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_routes((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnpoints_routes((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
