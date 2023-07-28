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
 * @brief Spatial functions for temporal network points.
 */

#include "npoint/tnpoint_spatialfuncs.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tsequence.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_boxops.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_tempspatialrels.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

#if 0 /* not used */
/**
 * @brief Ensure that a temporal network point and a STBox have the same SRID
 */
void
ensure_same_srid_tnpoint_stbox(const Temporal *temp, const STBox *box)
{
  if (MEOS_FLAGS_GET_X(box->flags) &&
    tnpoint_srid(temp) != box->srid)
    elog(ERROR, "The temporal network point and the box must be in the same SRID");
  return;
}
#endif /* not used */

/**
 * @brief Ensure that two temporal network point instants have the same route
 * identifier
 */
void
ensure_same_rid_tnpointinst(const TInstant *inst1, const TInstant *inst2)
{
  if (tnpointinst_route(inst1) != tnpointinst_route(inst2))
    elog(ERROR, "All network points composing a temporal sequence must have same route identifier");
  return;
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * @brief Return true if a segment of a temporal network point value intersects
 * a base value at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 */
bool
tnpointsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  Npoint *np1 = DatumGetNpointP(tinstant_value(inst1));
  Npoint *np2 = DatumGetNpointP(tinstant_value(inst2));
  Npoint *np = DatumGetNpointP(value);
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
  if (fabs(fraction) < MEOS_EPSILON || fabs(fraction - 1.0) < MEOS_EPSILON)
    return false;

  if (t != NULL)
  {
    double duration = (double) (inst2->t - inst1->t);
    *t = inst1->t + (long) (duration * fraction);
  }
  return true;
}

/*****************************************************************************
 * Functions for spatial reference systems of temporal network points
 * For temporal points of duration distinct from TInstant the Spatial
 * reference system identifier (SRID) is obtained from the bounding box.
 *****************************************************************************/

/**
 * @brief Return the SRID of a temporal network point of subtype instant.
 */
int
tnpointinst_srid(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  GSERIALIZED *line = route_geom(np->rid);
  int result = gserialized_get_srid(line);
  pfree(line);
  return result;
}

/**
 * @brief Return the SRID of a temporal network point
 */
int
tnpoint_srid(const Temporal *temp)
{
  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tnpointinst_srid((const TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_srid((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_srid((TSequenceSet *) temp);
  return result;
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
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    result[i] = DatumGetNpointP(tinstant_value(inst));
  }
  *count = seq->count;
  return result;
}

/**
 * @brief Return the network points covered by a temporal network point
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
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      result[npoints++] = DatumGetNpointP(tinstant_value(inst));
    }
  }
  *count = npoints;
  return result;
}

/*****************************************************************************
 * Geometric positions (Trajectotry) functions
 * Return the geometric positions covered by a temporal network point
 *****************************************************************************/

/**
 * @brief Return the geometry covered by a temporal network point.
 * @param[in] inst Temporal network point
 */
GSERIALIZED *
tnpointinst_geom(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value(inst));
  return npoint_geom(np);
}

/**
 * @brief Return the geometry covered by a temporal network point.
 * @param[in] seq Temporal network point
 */
GSERIALIZED *
tnpointseq_geom(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return tnpointinst_geom(TSEQUENCE_INST_N(seq, 0));

  GSERIALIZED *result;
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
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
 * @brief Return the geometry covered by a temporal network point.
 * @param[in] ss Temporal network point
 */
GSERIALIZED *
tnpointseqset_geom(const TSequenceSet *ss)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnpointseq_geom(TSEQUENCESET_SEQ_N(ss, 0));

  int count;
  GSERIALIZED *result;
  if (MEOS_FLAGS_GET_LINEAR(ss->flags))
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
 * @brief Return the geometry covered by a temporal network point.
 * @param[in] temp Temporal network point
 */
GSERIALIZED *
tnpoint_geom(const Temporal *temp)
{
  GSERIALIZED *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tnpointinst_geom((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tnpointseq_geom((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnpointseqset_geom((TSequenceSet *) temp);
  return result;
}

/**
 * @brief Compute the trajectory of two instants.
 * @param[in] np1, np2 Network points
 */
static Datum
tnpointseqsegm_trajectory(const Npoint *np1, const Npoint *np2)
{
  assert(np1->rid == np2->rid && np1->pos != np2->pos);
  GSERIALIZED *line = route_geom(np1->rid);
  if ((np1->pos == 0 && np2->pos == 1) || (np2->pos == 0 && np1->pos == 1))
    return PointerGetDatum(line);

  GSERIALIZED *traj;
  if (np1->pos < np2->pos)
    traj = gserialized_line_substring(line, np1->pos, np2->pos);
  else /* np1->pos >= np2->pos */
  {
    GSERIALIZED *traj2 = gserialized_line_substring(line, np2->pos, np1->pos);
    traj = gserialized_reverse(traj2);
    pfree(traj2);
  }
  pfree(line);
  return PointerGetDatum(traj);
}

/*****************************************************************************
 * Geographical equality for network points
 *****************************************************************************/

/**
 * @brief Determines the spatial equality for network points.
 * Two network points may be have different rid but represent the same
 * spatial point at the intersection of the two rids
 */
bool
npoint_same(const Npoint *np1, const Npoint *np2)
{
  /* Same route identifier */
  if (np1->rid == np2->rid)
    return fabs(np1->pos - np2->pos) < MEOS_EPSILON;
  Datum point1 = PointerGetDatum(npoint_geom(np1));
  Datum point2 = PointerGetDatum(npoint_geom(np2));
  bool result = datum_eq(point1, point2, T_GEOMETRY);
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
  Npoint *np1 = DatumGetNpointP(tinstant_value(inst));
  double length = route_length(np1->rid);
  double fraction = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    Npoint *np2 = DatumGetNpointP(tinstant_value(inst));
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
  double result = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    result += tnpointseq_length(seq);
  }
  return result;
}

/**
 * @brief Length traversed by a temporal network point
 */
double
tnpoint_length(const Temporal *temp)
{
  double result = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tnpointseq_length((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnpointseqset_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point.
 * @pre The sequence has linear interpolation
 * @sqlfunc cumulativeLength()
 */
static TSequence *
tnpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tinstant_make(Float8GetDatum(prevlength), T_TFLOAT,
      inst1->t);
    TSequence *result = tinstant_to_tsequence(inst, LINEAR);
    pfree(inst);
    return result;
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  Npoint *np1 = DatumGetNpointP(tinstant_value(inst1));
  double rlength = route_length(np1->rid);
  double length = prevlength;
  instants[0] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Npoint *np2 = DatumGetNpointP(tinstant_value(inst2));
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
    length += DatumGetFloat8(tinstant_value(end));
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @brief Cumulative length traversed by a temporal network point
 */
Temporal *
tnpoint_cumulative_length(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_from_base_temp(Float8GetDatum(0.0), T_TFLOAT, temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnpointseq_cumulative_length((TSequence *) temp, 0);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnpointseqset_cumulative_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * @brief Speed of a temporal network point
 */
static TSequence *
tnpointseq_speed(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Step interpolation */
  if (! MEOS_FLAGS_GET_LINEAR(seq->flags))
  {
    Datum length = Float8GetDatum(0.0);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      instants[i] = tinstant_make(length, T_TFLOAT, inst->t);
    }
  }
  else
  /* Linear interpolation */
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
    Npoint *np1 = DatumGetNpointP(tinstant_value(inst1));
    double rlength = route_length(np1->rid);
    const TInstant *inst2 = NULL; /* make the compiler quiet */
    double speed = 0; /* make the compiler quiet */
    for (int i = 0; i < seq->count - 1; i++)
    {
      inst2 = TSEQUENCE_INST_N(seq, i + 1);
      Npoint *np2 = DatumGetNpointP(tinstant_value(inst2));
      double length = fabs(np2->pos - np1->pos) * rlength;
      speed = length / (((double)(inst2->t) - (double)(inst1->t)) / 1000000);
      instants[i] = tinstant_make(Float8GetDatum(speed), T_TFLOAT, inst1->t);
      inst1 = inst2;
      np1 = np2;
    }
    instants[seq->count-1] = tinstant_make(Float8GetDatum(speed), T_TFLOAT,
      inst2->t);
  }
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, true);
  return result;
}

/**
 * @brief Speed of a temporal network point
 */
static TSequenceSet *
tnpointseqset_speed(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TSequence *seq1 = tnpointseq_speed(seq);
    if (seq1 != NULL)
      sequences[nseqs++] = seq1;
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, STEP);
}

/**
 * @brief Speed of a temporal network point
 */
Temporal *
tnpoint_speed(const Temporal *temp)
{
  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_GET_DISCRETE(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnpointseq_speed((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnpointseqset_speed((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Time-weighed centroid for temporal network points
 *****************************************************************************/

/**
 * @brief Return the time-weighed centroid of a temporal network point
 */
Datum
tnpoint_twcentroid(const Temporal *temp)
{
  Temporal *tgeom = tnpoint_tgeompoint(temp);
  Datum result = PointerGetDatum(tpoint_twcentroid(tgeom));
  pfree(tgeom);
  return result;
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * @brief Temporal azimuth of two temporal network point instants (iteration
 * function)
 */
static TInstant **
tnpointsegm_azimuth_iter(const TInstant *inst1, const TInstant *inst2,
  int *count)
{
  const Npoint *np1 = DatumGetNpointP(tinstant_value(inst1));
  const Npoint *np2 = DatumGetNpointP(tinstant_value(inst2));

  /* Constant segment */
  if (np1->pos == np2->pos)
  {
    *count = 0;
    return NULL;
  }

/* Find all vertices in the segment */
  GSERIALIZED *traj = DatumGetGserializedP(tnpointseqsegm_trajectory(np1, np2));
  int countVertices = gserialized_numpoints_linestring(traj);
  TInstant **result = palloc(sizeof(TInstant *) * countVertices);
  GSERIALIZED *vertex1 = gserialized_pointn_linestring(traj, 1); /* 1-based */
  double azimuth;
  TimestampTz time = inst1->t;
  for (int i = 0; i < countVertices - 1; i++)
  {
    GSERIALIZED *vertex2 = gserialized_pointn_linestring(traj, i + 2); /* 1-based */
    double fraction = gserialized_line_locate_point(traj, vertex2);
    assert(! datum_point_eq(PointerGetDatum(vertex1),
      PointerGetDatum(vertex2)));
    gserialized_azimuth(vertex1, vertex2, &azimuth);
    result[i] = tinstant_make(Float8GetDatum(azimuth), T_TFLOAT, time);
    pfree(vertex1);
    vertex1 = vertex2;
    time =  inst1->t + (long) ((double) (inst2->t - inst1->t) * fraction);
  }
  pfree(traj);
  pfree(vertex1);
  *count = countVertices - 1;
  return result;
}

/**
 * @brief Helper function to make a sequence from a set of instants
 */
static TSequence *
tsequence_assemble_instants(TInstant ***instants, int *countinsts,
  int totalinsts, int from, int to, bool lower_inc, TimestampTz last_time)
{
  TInstant **allinstants = palloc(sizeof(TInstant *) * (totalinsts + 1));
  int ninsts = 0;
  for (int i = from; i < to; i++)
  {
    for (int j = 0; j < countinsts[i]; j++)
      allinstants[ninsts++] = instants[i][j];
    if (instants[i] != NULL)
      pfree(instants[i]);
  }
  /* Add closing instant */
  Datum last_value = tinstant_value(allinstants[ninsts - 1]);
  allinstants[ninsts++] = tinstant_make(last_value, T_TFLOAT, last_time);
  /* Resulting sequence has step interpolation */
  return tsequence_make_free(allinstants, ninsts, lower_inc, true, STEP, true);
}

/**
 * @brief Temporal azimuth of a temporal network point of sequence subtype
 * (iterator function)
 */
static int
tnpointseq_azimuth_iter(const TSequence *seq, TSequence **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  TInstant ***instants = palloc(sizeof(TInstant *) * (seq->count - 1));
  int *countinsts = palloc0(sizeof(int) * (seq->count - 1));
  int totalinsts = 0; /* number of created instants so far */
  int nseqs = 0; /* number of created sequences */
  int m = 0; /* index of the segment from which to assemble instants */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    instants[i] = tnpointsegm_azimuth_iter(inst1, inst2, &countinsts[i]);
    /* If constant segment */
    if (countinsts[i] == 0)
    {
      if (totalinsts != 0)
      {
        /* Assemble all instants created so far */
        result[nseqs++] = tsequence_assemble_instants(instants, countinsts,
          totalinsts, m, i, lower_inc, inst1->t);
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
    result[nseqs++] = tsequence_assemble_instants(instants, countinsts,
      totalinsts, m, seq->count - 1, lower_inc, inst1->t);
  }
  pfree(instants);
  pfree(countinsts);
  return nseqs;
}

/**
 * @brief Temporal azimuth of a temporal network point of sequence subtype
 */
static TSequenceSet *
tnpointseq_azimuth(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * (seq->count - 1));
  int count = tnpointseq_azimuth_iter(seq, sequences);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, count, true);
}

/**
 * @brief Temporal azimuth of a temporal network point of sequence set subtype
 */
static TSequenceSet *
tnpointseqset_azimuth(const TSequenceSet *ss)
{
  if (ss->count == 1)
    return tnpointseq_azimuth(TSEQUENCESET_SEQ_N(ss, 0));

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    int countstep = tnpointseq_azimuth_iter(seq, &sequences[nseqs]);
    nseqs += countstep;
  }
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, STEP);
}

/**
 * @brief Temporal azimuth of a temporal network point
 */
Temporal *
tnpoint_azimuth(const Temporal *temp)
{
  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnpointseq_azimuth((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnpointseqset_azimuth((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Restrict a temporal network point to (the complement of) a geometry
 */
Temporal *
tnpoint_restrict_geom_time(const Temporal *temp, const GSERIALIZED *geo,
  const Span *zspan, const Span *period, bool atfunc)
{
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(geo));
  if (gserialized_is_empty(geo))
  {
    Temporal *result = atfunc ? NULL : temporal_copy(temp);
    if (atfunc)
      return NULL;
    return result;
  }
  ensure_has_not_Z_gs(geo);

  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *resultgeom = tpoint_restrict_geom_time(tempgeom, geo, zspan,
    period, atfunc);
  Temporal *result = NULL;
  if (resultgeom != NULL)
  {
    /* We do not call the function tgeompoint_tnpoint to avoid
     * roundoff errors */
    SpanSet *ps = temporal_time(resultgeom);
    result = temporal_restrict_periodset(temp, ps, REST_AT);
    pfree(resultgeom);
    pfree(ps);
  }
  pfree(tempgeom);
  return result;
}

/*****************************************************************************/
