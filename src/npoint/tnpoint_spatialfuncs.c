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
 * @file tnpoint_spatialfuncs.c
 * Geospatial functions for temporal network points.
 */

#include "npoint/tnpoint_spatialfuncs.h"

#include <assert.h>
#include <float.h>

#include "general/periodset.h"
#include "general/timeops.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_boxops.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_distance.h"
#include "npoint/tnpoint_tempspatialrels.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the temporal network point and the STBOX have the same SRID
 */
void
ensure_same_srid_tnpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(box->flags) &&
    tnpoint_srid_internal(temp) != box->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal network point and the box must be in the same SRID")));
}

/**
 * Ensure that the temporal network point instants have the same route identifier
 */
void
ensure_same_rid_tnpointinst(const TInstant *inst1, const TInstant *inst2)
{
  if (tnpointinst_route(inst1) != tnpointinst_route(inst2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("All network points composing a temporal sequence must have same route identifier")));
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * Returns true if the segment of the temporal network point value intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 */
bool
tnpointsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  npoint *np1 = DatumGetNpoint(tinstant_value(inst1));
  npoint *np2 = DatumGetNpoint(tinstant_value(inst2));
  npoint *np = DatumGetNpoint(value);
  double min = Min(np1->pos, np2->pos);
  double max = Max(np1->pos, np2->pos);
  /* if value is to the left or to the right of the range */
  if ((np->rid != np1->rid) ||
    (np->pos < np1->pos && np->pos < np2->pos) ||
    (np->pos > np1->pos && np->pos > np2->pos))
  // if (np->rid != np1->rid || (np->pos < min && np->pos > max))
    return false;

  double range = (max - min);
  double partial = (np->pos - min);
  double fraction = np1->pos < np2->pos ? partial / range : 1 - partial / range;
  if (fabs(fraction) < MOBDB_EPSILON || fabs(fraction - 1.0) < MOBDB_EPSILON)
    return false;

  if (t != NULL)
  {
    double duration = (inst2->t - inst1->t);
    *t = inst1->t + (long) (duration * fraction);
  }
  return true;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/* Spatial reference system identifier (SRID) of a temporal network point.
 * For temporal points of duration distinct from INSTANT the SRID is
 * obtained from the bounding box. */

/**
 * Returns the SRID of a temporal network point of sutbype instant
 */
int
tnpointinst_srid(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  Datum line = route_geom(np->rid);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int result = gserialized_get_srid(gs);
  pfree(DatumGetPointer(line));
  return result;
}

/**
 * Returns the SRID of a temporal network point (dispatch function)
 */
int
tnpoint_srid_internal(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->basetypid != type_oid(T_NPOINT))
    elog(ERROR, "unknown npoint base type: %d", temp->basetypid);
  if (temp->subtype == INSTANT)
    result = tnpointinst_srid((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tpointinstset_srid((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tpointseq_srid((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tpointseqset_srid((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_srid);
/**
 * Returns the SRID of a temporal network point
 */
PGDLLEXPORT Datum
tnpoint_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int result = tnpoint_srid_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * NPoints Functions
 * Return the network points covered by the temporal npoint
 *****************************************************************************/

/**
 * Return the network points covered by the temporal network point
 *
 * @param[in] ti Temporal network point
 * @param[out] count Number of elements of the output array
 * @note Only the particular cases returning points are covered
 */
npoint **
tnpointinstset_npoints(const TInstantSet *ti, int *count)
{
  npoint **result = palloc(sizeof(npoint *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    npoint *np = DatumGetNpoint(tinstant_value(tinstantset_inst_n(ti, i)));
    result[i] = np;
  }
  *count = ti->count;
  return result;
}

/**
 * Return the network points covered by the temporal network point
 *
 * @param[in] seq Temporal network point
 * @param[out] count Number of elements of the output array
 * @note Only the particular cases returning points are covered
 */
npoint **
tnpointseq_step_npoints(const TSequence *seq, int *count)
{
  npoint **result = palloc(sizeof(npoint *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    npoint *np = DatumGetNpoint(tinstant_value(tsequence_inst_n(seq, i)));
    result[i] = np;
  }
  *count = seq->count;
  return result;
}

/**
 * Return the network points covered by the temporal network point
 *
 * @param[in] ts Temporal network point
 * @param[out] count Number of elements of the output array
 * @note Only the particular cases returning points are covered
 */
npoint **
tnpointseqset_step_npoints(const TSequenceSet *ts, int *count)
{
  npoint **result = palloc(sizeof(npoint *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
    {
      npoint *np = DatumGetNpoint(tinstant_value(tsequence_inst_n(seq, j)));
      result[k++] = np;
    }
  }
  *count = k;
  return result;
}

/*****************************************************************************
 * Geometric positions (Trajectotry) functions
 * Return the geometric positions covered by the temporal npoint
 *****************************************************************************/

/**
 * Return the geometry covered by the temporal network point
 *
 * @param[in] inst Temporal network point
 */
Datum
tnpointinst_geom(const TInstant *inst)
{
  npoint *np = DatumGetNpoint(tinstant_value(inst));
  return npoint_as_geom_internal(np);
}

/**
 * Return the geometry covered by the temporal network point
 *
 * @param[in] ti Temporal network point
 */
Datum
tnpointinstset_geom(const TInstantSet *ti)
{
  /* Instantaneous sequence */
  if (ti->count == 1)
    return tnpointinst_geom(tinstantset_inst_n(ti, 0));

  int count;
  /* The following function does not remove duplicate values */
  npoint **points = tnpointinstset_npoints(ti, &count);
  Datum result = npointarr_to_geom_internal(points, count);
  pfree(points);
  return result;
}

/**
 * Return the geometry covered by the temporal network point
 *
 * @param[in] seq Temporal network point
 */
Datum
tnpointseq_geom(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return tnpointinst_geom(tsequence_inst_n(seq, 0));

  Datum result;
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    nsegment *segment = tnpointseq_linear_positions(seq);
    result = nsegment_as_geom_internal(segment);
    pfree(segment);
  }
  else
  {
    int count;
    /* The following function does not remove duplicate values */
    npoint **points = tnpointseq_step_npoints(seq, &count);
    result = npointarr_to_geom_internal(points, count);
    pfree(points);
  }
  return result;
}

/**
 * Return the geometry covered by the temporal network point
 *
 * @param[in] ts Temporal network point
 */
Datum
tnpointseqset_geom(const TSequenceSet *ts)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tnpointseq_geom(tsequenceset_seq_n(ts, 0));

  int count;
  Datum result;
  if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
  {
    nsegment **segments = tnpointseqset_positions(ts, &count);
    result = nsegmentarr_to_geom_internal(segments, count);
    pfree_array((void **) segments, count);
  }
  else
  {
    npoint **points = tnpointseqset_step_npoints(ts, &count);
    result = npointarr_to_geom_internal(points, count);
    pfree(points);
  }
  return result;
}

/**
 * Return the geometry covered by the temporal network point
 * (dispatch function)
 *
 * @param[in] temp Temporal network point
 */
Datum
tnpoint_geom(const Temporal *temp)
{
  Datum result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tnpointinst_geom((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tnpointinstset_geom((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_geom((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnpointseqset_geom((TSequenceSet *) temp);
  return result;
}

/**
 * Compute the trajectory of two instants.
 *
 * @param[in] inst1, inst2 Temporal network point instants
 */
Datum
tnpointseqsegm_trajectory(const TInstant *inst1, const TInstant *inst2)
{
  npoint *np1 = DatumGetNpoint(tinstant_value(inst1));
  npoint *np2 = DatumGetNpoint(tinstant_value(inst2));
  assert(np1->rid == np2->rid);

  if (np1->pos == np2->pos)
    return npoint_as_geom_internal(np1);

  Datum line = route_geom(np1->rid);
  if ((np1->pos == 0 && np2->pos == 1) ||
    (np2->pos == 0 && np1->pos == 1))
    return line;

  Datum traj;
  if (np1->pos < np2->pos)
    traj = call_function3(LWGEOM_line_substring, line,
      Float8GetDatum(np1->pos), Float8GetDatum(np2->pos));
  else /* np1->pos < np2->pos */
  {
    Datum traj2 = call_function3(LWGEOM_line_substring, line,
      Float8GetDatum(np2->pos), Float8GetDatum(np1->pos));
    traj = call_function1(LWGEOM_reverse, traj2);
    pfree(DatumGetPointer(traj2));
  }
  pfree(DatumGetPointer(line));
  return traj;
}

PG_FUNCTION_INFO_V1(tnpoint_trajectory);
/**
 * Return the geometry covered by the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = tnpoint_geom(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Geographical equality for network points
 *****************************************************************************/

/**
 * Determines the spatial equality for network points.
 * Two network points may be have different rid but represent the same
 * spatial point at the intersection of the two rids
 */
bool
npoint_same_internal(const npoint *np1, const npoint *np2)
{
  /* Same route identifier */
  if (np1->rid == np2->rid)
    return fabs(np1->pos - np2->pos) < MOBDB_EPSILON;
  Datum point1 = npoint_as_geom_internal(np1);
  Datum point2 = npoint_as_geom_internal(np2);
  bool result = datum_eq(point1, point2, type_oid(T_GEOMETRY));
  pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
  return result;
}

PG_FUNCTION_INFO_V1(npoint_same);
/**
 * Determines the spatial equality for network points
 */
PGDLLEXPORT Datum
npoint_same(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  PG_RETURN_BOOL(npoint_same_internal(np1, np2));
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/**
 * Length traversed by the temporal network point
 */
static double
tnpointseq_length(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  const TInstant *inst = tsequence_inst_n(seq, 0);
  npoint *np1 = DatumGetNpoint(tinstant_value(inst));
  double length = route_length(np1->rid);
  double fraction = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    npoint *np2 = DatumGetNpoint(tinstant_value(inst));
    fraction += fabs(np2->pos - np1->pos);
    np1 = np2;
  }
  return length * fraction;
}

/**
 * Length traversed by the temporal network point
 */
static double
tnpointseqset_length(const TSequenceSet *ts)
{
  double result = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result += tnpointseq_length(seq);
  }
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_length);
/**
 * Length traversed by the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double result = 0.0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET ||
    (temp->subtype == SEQUENCE && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
    (temp->subtype == SEQUENCESET && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
    ;
  else if (temp->subtype == SEQUENCE)
    result = tnpointseq_length((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnpointseqset_length((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/**
 * Cumulative length traversed by the temporal npoint
 */
static TInstant *
tnpointinst_set_zero(const TInstant *inst)
{
  return tinstant_make(Float8GetDatum(0.0), inst->t, FLOAT8OID);
}

/**
 * Cumulative length traversed by the temporal npoint
 */
static TInstantSet *
tnpointinstset_set_zero(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  Datum zero = Float8GetDatum(0.0);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tinstant_make(zero, inst->t, FLOAT8OID);
  }
  TInstantSet *result = tinstantset_make((const TInstant **) instants,
    ti->count, MERGE_NO);
  for (int i = 1; i < ti->count; i++)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * Cumulative length traversed by the temporal npoint
 */
static TSequence *
tnpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  const TInstant *inst1;
  TInstant *inst;
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    inst = tinstant_make(Float8GetDatum(prevlength), inst1->t, FLOAT8OID);
    TSequence *result = tsequence_make((const TInstant **) &inst, 1,
      true, true, true, false);
    pfree(inst);
    return result;
  }

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Stepwise interpolation */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    Datum length = Float8GetDatum(0.0);
    for (int i = 0; i < seq->count; i++)
    {
      inst1 = tsequence_inst_n(seq, i);
      instants[i] = tinstant_make(length, inst1->t, FLOAT8OID);
    }
  }
  else
  /* Linear interpolation */
  {
    inst1 = tsequence_inst_n(seq, 0);
    npoint *np1 = DatumGetNpoint(tinstant_value(inst1));
    double rlength = route_length(np1->rid);
    double length = prevlength;
    instants[0] = tinstant_make(Float8GetDatum(length), inst1->t, FLOAT8OID);
    for (int i = 1; i < seq->count; i++)
    {
      const TInstant *inst2 = tsequence_inst_n(seq, i);
      npoint *np2 = DatumGetNpoint(tinstant_value(inst2));
      length += fabs(np2->pos - np1->pos) * rlength;
      instants[i] = tinstant_make(Float8GetDatum(length), inst2->t,
        FLOAT8OID);
      np1 = np2;
    }
  }
  TSequence *result = tsequence_make((const TInstant **) instants,
    seq->count, seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), false);

  for (int i = 1; i < seq->count; i++)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * Cumulative length traversed by the temporal npoint
 */
static TSequenceSet *
tnpointseqset_cumulative_length(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  double length = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tnpointseq_cumulative_length(seq, length);
    const TInstant *end = tsequence_inst_n(sequences[i], seq->count - 1);
    length += DatumGetFloat8(tinstant_value(end));
  }
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    ts->count, false);

  for (int i = 1; i < ts->count; i++)
    pfree(sequences[i]);
  pfree(sequences);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_cumulative_length);
/**
 * Cumulative length traversed by the temporal npoint
 */
PGDLLEXPORT Datum
tnpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tnpointinst_set_zero((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tnpointinstset_set_zero((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tnpointseq_cumulative_length((TSequence *) temp, 0);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tnpointseqset_cumulative_length((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * Speed of the temporal network point
 */
static TSequence *
tnpointseq_speed(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Stepwise interpolation */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    Datum length = Float8GetDatum(0.0);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = tsequence_inst_n(seq, i);
      instants[i] = tinstant_make(length, inst->t, FLOAT8OID);
    }
  }
  else
  /* Linear interpolation */
  {
    const TInstant *inst1 = tsequence_inst_n(seq, 0);
    npoint *np1 = DatumGetNpoint(tinstant_value(inst1));
    double rlength = route_length(np1->rid);
    const TInstant *inst2 = NULL; /* make the compiler quiet */
    double speed = 0; /* make the compiler quiet */
    for (int i = 0; i < seq->count - 1; i++)
    {
      inst2 = tsequence_inst_n(seq, i + 1);
      npoint *np2 = DatumGetNpoint(tinstant_value(inst2));
      double length = fabs(np2->pos - np1->pos) * rlength;
      speed = length / (((double)(inst2->t) - (double)(inst1->t)) / 1000000);
      instants[i] = tinstant_make(Float8GetDatum(speed),
        inst1->t, FLOAT8OID);
      inst1 = inst2;
      np1 = np2;
    }
    instants[seq->count-1] = tinstant_make(Float8GetDatum(speed),
      inst2->t, FLOAT8OID);
  }
  /* The resulting sequence has stepwise interpolation */
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, true);
  return result;
}

/**
 * Speed of the temporal network point
 */
static TSequenceSet *
tnpointseqset_speed(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    TSequence *seq1 = tnpointseq_speed(seq);
    if (seq1 != NULL)
      sequences[k++] = seq1;
  }
  if (k == 0)
  {
    pfree(sequences);
    return NULL;
  }
  /* The resulting sequence set has stepwise interpolation */
  TSequenceSet *result = tsequenceset_make_free(sequences, k, STEP);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_speed);
/**
 * Speed of the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tnpointinst_set_zero((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tnpointinstset_set_zero((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tnpointseq_speed((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tnpointseqset_speed((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal network points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_twcentroid);
/**
 * Time-weighed centroid of the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *tgeom = tnpoint_as_tgeompoint_internal(temp);
  Datum result = tpoint_twcentroid_internal(tgeom);
  pfree(tgeom);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * Temporal azimuth of two temporal network point instants
 */
static TInstant **
tnpointseq_azimuth1(const TInstant *inst1, const TInstant *inst2,
  int *count)
{
  npoint *np1 = DatumGetNpoint(tinstant_value(inst1));
  npoint *np2 = DatumGetNpoint(tinstant_value(inst2));

  /* Constant segment */
  if (np1->pos == np2->pos)
  {
    *count = 0;
    return NULL;
  }

  /* Find all vertices in the segment */
  Datum traj = tnpointseqsegm_trajectory(inst1, inst2);
  int countVertices = DatumGetInt32(call_function1(
    LWGEOM_numpoints_linestring, traj));
  TInstant **result = palloc(sizeof(TInstant *) * countVertices);
  Datum vertex1 = call_function2(LWGEOM_pointn_linestring, traj,
    Int32GetDatum(1)); /* 1-based */
  Datum azimuth;
  TimestampTz time = inst1->t;
  for (int i = 0; i < countVertices - 1; i++)
  {
    Datum vertex2 = call_function2(LWGEOM_pointn_linestring, traj,
      Int32GetDatum(i + 2)); /* 1-based */
    double fraction = DatumGetFloat8(call_function2(
      LWGEOM_line_locate_point, traj, vertex2));
    azimuth = call_function2(LWGEOM_azimuth, vertex1, vertex2);
    result[i] = tinstant_make(azimuth, time, FLOAT8OID);
    pfree(DatumGetPointer(vertex1));
    vertex1 = vertex2;
    time =  inst1->t + (long) ((double) (inst2->t - inst1->t) * fraction);
  }
  pfree(DatumGetPointer(traj));
  pfree(DatumGetPointer(vertex1));
  *count = countVertices - 1;
  return result;
}

/**
 * Temporal azimuth of the temporal network point of sequence subtype
 */
static int
tnpointseq_azimuth2(const TSequence *seq, TSequence **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  TInstant ***instants = palloc(sizeof(TInstant *) * (seq->count - 1));
  int *countinsts = palloc0(sizeof(int) * (seq->count - 1));
  int totalinsts = 0; /* number of created instants so far */
  int l = 0; /* number of created sequences */
  int m = 0; /* index of the segment from which to assemble instants */
  Datum last_value;
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    instants[i] = tnpointseq_azimuth1(inst1, inst2, &countinsts[i]);
    /* If constant segment */
    if (countinsts[i] == 0)
    {
      /* Assemble all instants created so far */
      if (totalinsts != 0)
      {
        TInstant **allinstants = palloc(sizeof(TInstant *) * (totalinsts + 1));
        int n = 0;
        for (int j = m; j < i; j++)
        {
          for (int k = 0; k < countinsts[j]; k++)
            allinstants[n++] = instants[j][k];
          if (instants[j] != NULL)
            pfree(instants[j]);
        }
        /* Add closing instant */
        last_value = tinstant_value(allinstants[n - 1]);
        allinstants[n++] = tinstant_make(last_value, inst1->t, FLOAT8OID);
        /* Resulting sequence has stepwise interpolation */
        result[l++] = tsequence_make_free(allinstants, n, lower_inc, true,
          STEP, true);
        /* Indicate that we have consommed all instants created so far */
        m = i;
        totalinsts = 0;
      }
    }
    else
    {
      totalinsts += countinsts[i];
    }
    inst1 = inst2;
    lower_inc = true;
  }
  if (totalinsts != 0)
  {
    /* Assemble all instants created so far */
    TInstant **allinstants = palloc(sizeof(TInstant *) * (totalinsts + 1));
    int n = 0;
    for (int j = m; j < seq->count - 1; j++)
    {
      for (int k = 0; k < countinsts[j]; k++)
        allinstants[n++] = instants[j][k];
      if (instants[j] != NULL)
        pfree(instants[j]);
    }
    /* Add closing instant */
    last_value = tinstant_value(allinstants[n - 1]);
    allinstants[n++] = tinstant_make(last_value, inst1->t, FLOAT8OID);
    /* Resulting sequence has stepwise interpolation */
    result[l++] = tsequence_make((const TInstant **) allinstants, n,
      lower_inc, true, false, true);
    pfree_array((void **) allinstants, n);
  }
  pfree(instants);
  pfree(countinsts);
  return l;
}

/**
 * Temporal azimuth of the temporal network point of sequence subtype
 */
static TSequenceSet *
tnpointseq_azimuth(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * (seq->count - 1));
  int count = tnpointseq_azimuth2(seq, sequences);
  if (count == 0)
  {
    pfree(sequences);
    return NULL;
  }

  /* Resulting sequence set has stepwise interpolation */
  TSequenceSet *result = tsequenceset_make_free(sequences, count, true);
  return result;
}

static TSequenceSet *
tnpointseqset_azimuth(const TSequenceSet *ts)
{
  if (ts->count == 1)
    return tnpointseq_azimuth(tsequenceset_seq_n(ts, 0));

  TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    int countstep = tnpointseq_azimuth2(seq, &sequences[k]);
    k += countstep;
  }
  if (k == 0)
    return NULL;

  /* Resulting sequence set has stepwise interpolation */
  TSequenceSet *result = tsequenceset_make_free(sequences, k, STEP);
  return result;
}

PG_FUNCTION_INFO_V1(tnpoint_azimuth);
/**
 * Temporal azimuth of the temporal network point
 */
PGDLLEXPORT Datum
tnpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET ||
    (temp->subtype == SEQUENCE && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
    (temp->subtype == SEQUENCESET && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
    ;
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tnpointseq_azimuth((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tnpointseqset_azimuth((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * Restrict a temporal network point to (the complement of) a geometry
 */
static Datum
tnpoint_restrict_geometry(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid(tnpoint_srid_internal(temp), gserialized_get_srid(gs));
  if (gserialized_is_empty(gs))
  {
    Temporal *result = atfunc ? NULL : temporal_copy(temp);
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    if (atfunc)
      PG_RETURN_NULL();
    PG_RETURN_POINTER(result);
  }
  ensure_has_not_Z_gs(gs);

  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *geomresult = tpoint_restrict_geometry_internal(geomtemp,
    PointerGetDatum(gs), atfunc);
  Temporal *result = NULL;
  if (geomresult != NULL)
  {
    /* We do not do call the function tgeompoint_as_tnpoint to avoid
     * roundoff errors */
    PeriodSet *ps = temporal_get_time_internal(geomresult);
    result = temporal_restrict_periodset_internal(temp, ps, REST_AT);
    pfree(geomresult);
    pfree(ps);
  }
  pfree(geomtemp);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnpoint_at_geometry);
/**
 * Restricts the temporal point to the geometry
 */
PGDLLEXPORT Datum
tnpoint_at_geometry(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geometry(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(tnpoint_minus_geometry);
/**
 * Restrict the temporal point to the complement of the geometry
 */
PGDLLEXPORT Datum
tnpoint_minus_geometry(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geometry(fcinfo, REST_MINUS);
}

/*****************************************************************************/
