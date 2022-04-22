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
 * @brief Basic functions for temporal network points.
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


/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TInstant *
tnpointinst_tgeompointinst(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  Datum geom = npoint_geom(np);
  TInstant *result = tinstant_make(geom, inst->t, T_TGEOMPOINT);
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TInstantSet *
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
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TSequence *
tnpointseq_tgeompointseq(const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  Npoint *np = DatumGetNpointP(tinstant_value(tsequence_inst_n(seq, 0)));
  Datum line = route_geom(np->rid);
  /* We are sure line is not empty */
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int srid = gserialized_get_srid(gs);
  LWLINE *lwline = (LWLINE *)lwgeom_from_gserialized(gs);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    np = DatumGetNpointP(tinstant_value(inst));
    POINTARRAY* opa = lwline_interpolate_points(lwline, np->pos, 0);
    LWGEOM *lwpoint;
    assert(opa->npoints <= 1);
    lwpoint = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
    Datum point = PointerGetDatum(geo_serialize(lwpoint));
    instants[i] = tinstant_make(point, inst->t, T_TGEOMPOINT);
    pfree(DatumGetPointer(point));
  }
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
  pfree(DatumGetPointer(line));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point.
 */
TSequenceSet *
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
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point.
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

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TInstant *
tgeompointinst_tnpointinst(const TInstant *inst)
{
  Datum geom = tinstant_value(inst);
  Npoint *np = geom_npoint(geom);
  if (np == NULL)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(np), inst->t, T_TNPOINT);
  pfree(np);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TInstantSet *
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
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TSequence *
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
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point.
 */
TSequenceSet *
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
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point.
 */
Temporal *
tgeompoint_tnpoint(const Temporal *temp)
{
  int32_t srid_tpoint = tpoint_srid(temp);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_tpoint, srid_ways);
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tgeompointinst_tnpointinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tgeompointinstset_tnpointinstset((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tgeompointseq_tnpointseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tgeompointseqset_tnpointseqset((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Set the precision of the fraction of the temporal network point to the
 * number of decimal places.
 */
Temporal *
tnpoint_round(const Temporal *temp, Datum size)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_npoint_round;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointinstset_positions(const TInstantSet *ti, int *count)
{
  int count1;
  /* The following function removes duplicate values */
  Datum *values = tinstantset_values(ti, &count1);
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
 * Return the network segments covered by the temporal network point.
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
 * Return the network segments covered by the temporal network point.
 */
Nsegment *
tnpointseq_linear_positions(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  int64 rid = np->rid;
  double minPos = np->pos, maxPos = np->pos;
  for (int i = 1; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    np = DatumGetNpointP(tinstant_value(inst));
    minPos = Min(minPos, np->pos);
    maxPos = Max(maxPos, np->pos);
  }
  return nsegment_make(rid, minPos, maxPos);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseq_positions(const TSequence *seq, int *count)
{
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
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
 * Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_linear_positions(const TSequenceSet *ts, int *count)
{
  Nsegment **segments = palloc(sizeof(Nsegment *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    segments[i] = tnpointseq_linear_positions(seq);
  }
  Nsegment **result = segments;
  int count1 = ts->count;
  if (count1 > 1)
    result = nsegmentarr_normalize(segments, &count1);
  *count = count1;
  return result;
}

/**
 * Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_step_positions(const TSequenceSet *ts, int *count)
{
  /* The following function removes duplicate values */
  int newcount;
  Datum *values = tsequenceset_values(ts, &newcount);
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpointseqset_positions(const TSequenceSet *ts, int *count)
{
  Nsegment **result;
  if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
    result = tnpointseqset_linear_positions(ts, count);
  else
    result = tnpointseqset_step_positions(ts, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the network segments covered by the temporal network point.
 */
Nsegment **
tnpoint_positions(const Temporal *temp, int *count)
{
  Nsegment **result;
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

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the route of the temporal network point.
 */
int64
tnpointinst_route(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  return np->rid;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the route of a temporal network point.
 */
int64
tnpoint_route(const Temporal *temp)
{
  if (temp->subtype != INSTANT && temp->subtype != SEQUENCE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal instant or a temporal sequence")));

  const TInstant *inst = (temp->subtype == INSTANT) ?
    (const TInstant *) temp : tsequence_inst_n((const TSequence *) temp, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  return np->rid;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of routes of a temporal network point
 */
int64 *
tnpointinst_routes(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  int64 *result = palloc(sizeof(int64));
  result[0]= np->rid;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of routes of a temporal network point
 */
int64 *
tnpointinstset_routes(const TInstantSet *ti)
{
  int64 *result = palloc(sizeof(int64) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    Npoint *np = DatumGetNpointP(tinstant_value(inst));
    result[i] = np->rid;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of routes of a temporal network point
 */
int64 *
tnpointseq_routes(const TSequence *seq)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  int64 *result = palloc(sizeof(int64));
  result[0]= np->rid;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of routes of a temporal network point
 */
int64 *
tnpointseqset_routes(const TSequenceSet *ts)
{
  int64 *result = palloc(sizeof(int64) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tsequence_inst_n(seq, 0);
    Npoint *np = DatumGetNpointP(tinstant_value(inst));
    result[i] = np->rid;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of routes of a temporal network point
 */
int64 *
tnpoint_routes(const Temporal *temp, int *count)
{
  int64 *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tnpointinst_routes((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = tnpointinstset_routes((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = tnpointseq_routes((TSequence *) temp);
    *count = 1;
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = tnpointseqset_routes((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->count;
  }
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Convert a C array of int64 values into a PostgreSQL array
 */
ArrayType *
int64arr_array(const int64 *int64arr, int count)
{
  return construct_array((Datum *)int64arr, count, INT8OID, 8, true, 'd');
}

#if 0 /* not used */
/**
 * Convert a C array of network point values into a PostgreSQL array
 */
ArrayType *
npointarr_array(Npoint **npointarr, int count)
{
  return construct_array((Datum *)npointarr, count, type_oid(T_NPOINT),
    sizeof(Npoint), false, 'd');
}
#endif

/**
 * Convert a C array of network segment values into a PostgreSQL array
 */
ArrayType *
nsegmentarr_array(Nsegment **nsegmentarr, int count)
{
  return construct_array((Datum *)nsegmentarr, count, type_oid(T_NSEGMENT),
    sizeof(Nsegment), false, 'd');
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_in);
/**
 * Input function for temporal network points
 */
PGDLLEXPORT Datum
Tnpoint_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_parse(&input, oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_to_tgeompoint);
/**
 * Cast a temporal network point as a temporal geometric point
 */
PGDLLEXPORT Datum
Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tgeompoint_to_tnpoint);
/**
 * Cast a temporal geometric point as a temporal network point
 */
PGDLLEXPORT Datum
Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tnpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_round);
/**
 * @brief Set the precision of the fraction of the temporal network point to the
 * number of decimal places.
 */
PGDLLEXPORT Datum
Tnpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tnpoint_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_positions);
/**
 * Return the network segments covered by the temporal network point
 */
PGDLLEXPORT Datum
Tnpoint_positions(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Nsegment **segments = tnpoint_positions(temp, &count);
  ArrayType *result = nsegmentarr_array(segments, count);
  pfree_array((void **) segments, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnpoint_route);
/**
 * Return the route of a temporal network point
 */
PGDLLEXPORT Datum
Tnpoint_route(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 result = tnpoint_route(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT64(result);
}

PG_FUNCTION_INFO_V1(Tnpoint_routes);
/**
 * Return the array of routes of a temporal network point
 */
PGDLLEXPORT Datum
Tnpoint_routes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  int64 *routes = tnpoint_routes(temp, &count);
  ArrayType *result = int64arr_array(routes, count);
  pfree(routes);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
