/***********************************************************************
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
 * @brief Analytic functions for temporal points and temporal floats.
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom_internal.h>
#include <lwgeodetic_tree.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/tsequence.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"

/* Timestamps in PostgreSQL are encoded as MICROseconds since '2000-01-01'
 * while Unix epoch are encoded as MILLIseconds since '1970-01-01'.
 * Therefore the value used for conversions is computed as follows
 * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
 * which results in 946684800 */
#define DELTA_UNIX_POSTGRES_EPOCH 946684800

/*****************************************************************************
 * Convert a temporal point into a PostGIS geometry/geography where the M
 * coordinates are
 * - either given in the second parameter
 * - or encode the timestamps of the temporal point in Unix epoch
 *
 * NOTICE that the original subtype is lost in the translation since when
 * converting back and forth a temporal point and a geometry/geography,
 * the minimal subtype is used. Therefore,
 * - an instantaneous sequence converted back and forth will result into an
 *   instant
 * - a singleton sequence set converted back and forth will result into a
 *   sequence
 * This does not affect equality since in MobilityDB equality of temporal types
 * does not take into account the subtype but the temporal semantics. However,
 * this may be an issue when the column of a table is restricted to a given
 * temporal subtype using a type modifier or typmod. We refer to the MobilityDB
 * manual for understanding how to restrict columns of tables using typmod.
 *
 * NOTICE that the step interpolation is lost in the translation. Therefore,
 * when converting back and forth a temporal sequence (set) with step
 * interpolation to a geometry/geography the result will be a temporal
 * sequence with step interpolation.

 * NOTICE also that the temporal bounds are lost in the translation.
 * By default, the temporal bounds are set to true when converting back from a
 * geometry/geography to a temporal point.

 * THEREFORE, the equivalence
 * temp == (temp::geometry/geography)::tgeompoint/tgeogpoint
 * is true ONLY IF all temporal bounds are true and for temporal points with
 * linear interpolation
 *****************************************************************************/

/**
 * @brief Convert a geometry/geography point and a measure into a PostGIS point
 * with an M coordinate
 */
static LWGEOM *
point_meas_to_lwpoint(Datum point, Datum meas)
{
  GSERIALIZED *gs = DatumGetGserializedP(point);
  int32 srid = gserialized_get_srid(gs);
  int hasz = FLAGS_GET_Z(gs->gflags);
  int geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  double d = DatumGetFloat8(meas);
  LWPOINT *lwresult;
  if (hasz)
  {
    const POINT3DZ *pt = GSERIALIZED_POINT3DZ_P(gs);
    lwresult = lwpoint_make4d(srid, pt->x, pt->y, pt->z, d);
  }
  else
  {
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    lwresult = lwpoint_make3dm(srid, pt->x, pt->y, d);
  }
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  return (LWGEOM *) lwresult;
}

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * instant point and either the temporal float or the timestamp of the temporal
 * point (iterator function)
 * @param[in] inst Temporal point
 * @param[in] meas Temporal float (may be null)
 * @pre The temporal point and the measure are synchronized
 */
static LWGEOM *
tpointinst_to_geo_meas_iter(const TInstant *inst, const TInstant *meas)
{
  Datum m;
  if (meas)
    m = tinstant_value(meas);
  else
  {
    double epoch = ((double) inst->t / 1e6) + DELTA_UNIX_POSTGRES_EPOCH;
    m = Float8GetDatum(epoch);
  }
  return point_meas_to_lwpoint(tinstant_value(inst), m);
}

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * instant point and either the temporal float or the timestamp of the temporal
 * point.
 * @param[in] inst Temporal point
 * @param[in] meas Temporal float (may be null)
 * @pre The temporal point and the measure are synchronized
 */
static GSERIALIZED *
tpointinst_to_geo_meas(const TInstant *inst, const TInstant *meas)
{
  LWGEOM *lwresult = tpointinst_to_geo_meas_iter(inst, meas);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * discrete sequence point and either the temporal float or the timestamps of
 * the temporal point.
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float (may be null)
 * @pre The temporal point and the measure are synchronized
 */
static GSERIALIZED *
tpointseq_disc_to_geo_meas(const TSequence *seq, const TSequence *meas)
{
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const TInstant *m = meas ? TSEQUENCE_INST_N(meas, i) : NULL;
    points[i] = tpointinst_to_geo_meas_iter(inst, m);
  }
  GSERIALIZED *result;
  if (seq->count == 1)
  {
    result = geo_serialize(points[0]);
    lwgeom_free(points[0]); pfree(points);
  }
  else
  {
    LWGEOM *lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, srid,
      NULL, (uint32_t) seq->count, points);
    FLAGS_SET_Z(lwresult->flags, hasz);
    FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
    result = geo_serialize(lwresult);
    lwgeom_free(lwresult);
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * sequence point and either the temporal float or the timestamps of the
 * temporal point.
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float (may be null)
 * @pre The temporal point and the measure are synchronized
 * @note The function does not add a point if is equal to the previous one.
 */
static GSERIALIZED *
tpointseq_cont_to_geo_meas(const TSequence *seq, const TSequence *meas)
{
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  LWGEOM **points = palloc(sizeof(LWPOINT *) * seq->count);
  /* Keep the first point */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const TInstant *m = meas ? TSEQUENCE_INST_N(meas, 0) : NULL;
  LWGEOM *value1 = tpointinst_to_geo_meas_iter(inst, m);
  points[0] = value1;
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    m = meas ? TSEQUENCE_INST_N(meas, i) : NULL;
    LWGEOM *value2 = tpointinst_to_geo_meas_iter(inst, m);
    /* Do not add a point if it is equal to the previous one */
    if (lwpoint_same((LWPOINT *) value1, (LWPOINT *) value2) != LW_TRUE)
    {
      points[npoints++] = value2;
      value1 = value2;
    }
    else
      lwgeom_free(value2);
  }
  LWGEOM *lwresult;
  if (npoints == 1)
  {
    lwresult = points[0];
    pfree(points);
  }
  else
  {
    if (linear)
    {
      lwresult = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) npoints,
        points);
      for (int i = 0; i < npoints; i++)
        lwgeom_free(points[i]);
      pfree(points);
    }
    else
    {
      lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, srid, NULL,
        (uint32_t) npoints, points);
    }
  }
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * sequence set point and either the temporal float or the timestamps of the
 * temporal point.
 * @param[in] ss Temporal point
 * @param[in] meas Temporal float (may be null)
 * @pre The temporal point and the measure are synchronized
 * @note This function has a similar algorithm as #tpointseqset_trajectory
 */
static GSERIALIZED *
tpointseqset_to_geo_meas(const TSequenceSet *ss, const TSequenceSet *meas)
{
  const TSequence *seq1, *seq2;

  /* Instantaneous sequence */
  if (ss->count == 1)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, 0) : NULL;
    return tpointseq_cont_to_geo_meas(seq1, seq2);
  }

  int32 srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  bool linear = MEOS_FLAGS_GET_LINEAR(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->count);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointseq_to_geo_meas accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, i);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, i) : NULL;
    /* Keep the first point */
    const TInstant *inst = TSEQUENCE_INST_N(seq1, 0);
    const TInstant *m = meas ? TSEQUENCE_INST_N(seq2, 0) : NULL;
    LWGEOM *value1 = tpointinst_to_geo_meas_iter(inst, m);
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    points[npoints] = value1;
    int k = 1;
    for (int j = 1; j < seq1->count; j++)
    {
      inst = TSEQUENCE_INST_N(seq1, j);
      m = meas ? TSEQUENCE_INST_N(seq2, j) : NULL;
      LWGEOM *value2 = tpointinst_to_geo_meas_iter(inst, m);
      /* Do not add a point if it is equal to the previous one */
      if (lwpoint_same((LWPOINT *) value1, (LWPOINT *) value2) != LW_TRUE)
      {
        points[npoints + k++] = value2;
        value1 = value2;
      }
      else
        lwgeom_free(value2);
    }
    if (linear && k > 1)
    {
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) k,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      for (int j = 0; j < k; j++)
        lwgeom_free(points[npoints + j]);
    }
    else
      npoints += k;
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  pfree(points); pfree(lines);
  return result;
}

/*****************************************************************************/

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * sequence point and either the temporal float or the timestamps of the
 * temporal point. The result is a (Multi)Point when there are only
 * instantaneous sequences or a (Multi)linestring when each composing
 * linestring corresponds to a segment of a sequence of the temporal point.
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float (may be null)
 */
static GSERIALIZED *
tpointseq_cont_to_geo_meas_segm(const TSequence *seq, const TSequence *meas)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const TInstant *m = meas ? TSEQUENCE_INST_N(meas, 0) : NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    /* Result is a point */
    return tpointinst_to_geo_meas(inst, m);

  /* General case */
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * (seq->count - 1));
  LWGEOM *points[2];
  points[0] = tpointinst_to_geo_meas_iter(inst, m);
  for (int i = 0; i < seq->count - 1; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i + 1);
    m = meas ? TSEQUENCE_INST_N(meas, i + 1) : NULL;
    points[1] = tpointinst_to_geo_meas_iter(inst, m);
    lines[i] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2, points);
    FLAGS_SET_Z(lines[i]->flags, hasz);
    FLAGS_SET_GEODETIC(lines[i]->flags, geodetic);
    lwgeom_free(points[0]);
    points[0] = points[1];
  }
  lwgeom_free(points[0]);
  LWGEOM *lwresult;
  if (seq->count == 2)
  {
    /* Result is a linestring */
    lwresult = lines[0];
    pfree(lines);
  }
  else
  {
    /* Result is a multilinestring */
    lwresult = (LWGEOM *) lwcollection_construct(MULTILINETYPE, srid, NULL,
      (uint32_t) seq->count - 1, lines);
    FLAGS_SET_Z(lwresult->flags, hasz);
    FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  }
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Construct a geometry/geography with M measure from the temporal
 * sequence set point and either the temporal float or the timestamps of the
 * temporal point. The result is a (Multi)Point when there are only
 * instantaneous sequences or a (Multi)linestring when each composing
 * linestring corresponds to a segment of a sequence of the temporal point.
 * @param[in] ss Temporal point
 * @param[in] meas Temporal float (may be null)
 */
static GSERIALIZED *
tpointseqset_to_geo_meas_segm(const TSequenceSet *ss, const TSequenceSet *meas)
{
  const TSequence *seq1, *seq2;

  /* Instantaneous sequence */
  if (ss->count == 1)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, 0) : NULL;
    return tpointseq_cont_to_geo_meas_segm(seq1, seq2);
  }

  int32 srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointseq_to_geo_meas_segm accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, i);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, i) : NULL;
    /* Keep the first point */
    const TInstant *inst = TSEQUENCE_INST_N(seq1, 0);
    const TInstant *m = meas ? TSEQUENCE_INST_N(seq2, 0) : NULL;
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    points[npoints] = tpointinst_to_geo_meas_iter(inst, m);
    if (seq1->count == 1)
    {
      /* Add a point for the current sequence */
      npoints++;
      continue;
    }
    /* Add lines for each segment of the current sequence */
    for (int j = 1; j < seq1->count; j++)
    {
      inst = TSEQUENCE_INST_N(seq1, j);
      m = meas ? TSEQUENCE_INST_N(seq2, j) : NULL;
      points[npoints + 1] = tpointinst_to_geo_meas_iter(inst, m);
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      lwgeom_free(points[npoints]);
      points[npoints] = points[npoints + 1];
    }
    lwgeom_free(points[npoints]);
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  pfree(points); pfree(lines);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Construct a geometry/geography with M measure from the temporal
 * point and the arguments. The latter can be
 * - either the temporal float given in the second argument (if any)
 * - or the time information of the temporal point where the M coordinates
 *   encode the timestamps in number of seconds since '1970-01-01'
 * @param[in] tpoint Temporal point
 * @param[in] meas Temporal float (may be null)
 * @param[in] segmentize When true, in the general case the resulting geometry
 * will be a MultiLineString composed one Linestring per segment of the
 * temporal sequence (set)
 * @param[out] result Resulting geometry array
 * @sqlfunc geoMeasure() when the second argument is not NULL
 * @sqlop @p :: when the second argument is NULL
 */
bool
tpoint_to_geo_meas(const Temporal *tpoint, const Temporal *meas,
  bool segmentize, GSERIALIZED **result)
{
  assert(tgeo_type(tpoint->temptype));
  Temporal *sync1, *sync2;
  if (meas)
  {
    assert(tnumber_type(meas->temptype));
    /* Return false if the temporal values do not intersect in time
     * The operation is synchronization without adding crossings */
    if (! intersection_temporal_temporal(tpoint, meas, SYNCHRONIZE_NOCROSS,
        &sync1, &sync2))
      return false;
  }
  else
  {
    sync1 = (Temporal *) tpoint;
    sync2 = NULL;
  }

  assert(temptype_subtype(sync1->subtype));
  if (sync1->subtype == TINSTANT)
    *result = tpointinst_to_geo_meas((TInstant *) sync1, (TInstant *) sync2);
  else if (sync1->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(sync1->flags))
      *result = tpointseq_disc_to_geo_meas((TSequence *) sync1,
        (TSequence *) sync2);
    else
      *result = segmentize ?
        tpointseq_cont_to_geo_meas_segm(
          (TSequence *) sync1, (TSequence *) sync2) :
        tpointseq_cont_to_geo_meas(
          (TSequence *) sync1, (TSequence *) sync2);
  }
  else /* sync1->subtype == TSEQUENCESET */
    *result = segmentize ?
      tpointseqset_to_geo_meas_segm(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2) :
      tpointseqset_to_geo_meas(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2);

  if (meas)
  {
    pfree(sync1); pfree(sync2);
  }
  return true;
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the
 * timestamps in Unix epoch into a temporal point.
 *****************************************************************************/

/**
 * @brief Convert the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal instant point.
 */
static TInstant *
trajpoint_to_tpointinst(LWPOINT *lwpoint)
{
  bool hasz = (bool) FLAGS_GET_Z(lwpoint->flags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(lwpoint->flags);
  LWPOINT *lwpoint1;
  TimestampTz t;
  if (hasz)
  {
    POINT4D point = getPoint4d(lwpoint->point, 0);
    t = (TimestampTz) ((point.m - DELTA_UNIX_POSTGRES_EPOCH) * 1e6);
    lwpoint1 = lwpoint_make3dz(lwpoint->srid, point.x, point.y, point.z);
  }
  else
  {
    POINT3DM point = getPoint3dm(lwpoint->point, 0);
    t = (TimestampTz) ((point.m - DELTA_UNIX_POSTGRES_EPOCH) * 1e6);
    lwpoint1 = lwpoint_make2d(lwpoint->srid, point.x, point.y);
  }
  FLAGS_SET_Z(lwpoint1->flags, hasz);
  FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint1);
  meosType temptype = geodetic ? T_TGEOGPOINT : T_TGEOMPOINT;
  TInstant *result = tinstant_make(PointerGetDatum(gs), temptype, t);
  lwpoint_free(lwpoint1);
  pfree(gs);
  return result;
}

/**
 * @brief Convert the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal instant point.
 */
static TInstant *
geo_to_tpointinst(const LWGEOM *lwgeom)
{
  /* Geometry is a POINT */
  return trajpoint_to_tpointinst((LWPOINT *) lwgeom);
}

/**
 * @brief Ensure that a PostGIS trajectory has increasing timestamps.
 * @note The verification is made in this function since calling the PostGIS
 * function lwgeom_is_trajectory causes discrepancies with regression tests
 * due to the error message that varies across PostGIS versions.
 */
static void
ensure_valid_trajectory(const LWGEOM *lwgeom, bool hasz, bool discrete)
{
  assert(lwgeom->type != MULTIPOINTTYPE || lwgeom->type != MULTILINETYPE);
  LWCOLLECTION *lwcoll = NULL;
  LWLINE *lwline = NULL;
  uint32_t npoints;
  if (discrete)
  {
    lwcoll = lwgeom_as_lwcollection(lwgeom);
    npoints = lwcoll->ngeoms;
  }
  else
  {
    lwline = lwgeom_as_lwline(lwgeom);
    npoints = lwline->points->npoints;
  }
  double m1 = -1 * DBL_MAX, m2;
  for (uint32_t i = 0; i < npoints; i++)
  {
    const POINTARRAY *pa = discrete ?
      ((LWPOINT *) lwcoll->geoms[i])->point : lwline->points;
    uint32_t where = discrete ? 0 : i;
    if (hasz)
    {
      POINT4D point = getPoint4d(pa, where);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(pa, where);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      elog(ERROR, "Trajectory must be valid");
    }
    m1 = m2;
  }
  return;
}

/**
 * @brief Convert the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal discrete
 * sequence point.
 */
static TSequence *
geo_to_tpointseq_disc(const LWGEOM *lwgeom, bool hasz)
{
  /* Verify that the trajectory is valid */
  ensure_valid_trajectory(lwgeom, hasz, true);
  /* Geometry is a MULTIPOINT */
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  uint32_t npoints = lwcoll->ngeoms;
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
    instants[i] = trajpoint_to_tpointinst((LWPOINT *) lwcoll->geoms[i]);
  return tsequence_make_free(instants, npoints, true, true, DISCRETE,
    NORMALIZE);
}

/**
 * @brief Convert the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal sequence
 * point.
 * @note Notice that it is not possible to encode step interpolation in
 * PostGIS and thus sequences obtained will be either discrete or linear.
 */
static TSequence *
geo_to_tpointseq_linear(const LWGEOM *lwgeom, bool hasz, bool geodetic)
{
  /* Verify that the trajectory is valid */
  ensure_valid_trajectory(lwgeom, hasz, false);
  /* Geometry is a LINESTRING */
  LWLINE *lwline = lwgeom_as_lwline(lwgeom);
  uint32_t npoints = lwline->points->npoints;
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
  {
    /* Return freshly allocated LWPOINT */
    LWPOINT *lwpoint = lwline_get_lwpoint(lwline, i);
    /* Function lwline_get_lwpoint lose the geodetic flag if any */
    FLAGS_SET_Z(lwpoint->flags, hasz);
    FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
    instants[i] = trajpoint_to_tpointinst(lwpoint);
    lwpoint_free(lwpoint);
  }
  /* The resulting sequence assumes linear interpolation */
  return tsequence_make_free(instants, npoints, true, true, LINEAR, NORMALIZE);
}

/**
 * @brief Convert the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal sequence
 * set point.
 * @note With respect to functions #geo_to_tpointseq_disc and
 * #geo_to_tpointseq_linear there is no validation of the trajectory since
 * it is more elaborated to be done. Nevertheless, erroneous geometries where
 * the timestamps are not increasing will be detected by the constructor of
 * the sequence set.
 */
static TSequenceSet *
geo_to_tpointseqset(const LWGEOM *lwgeom, bool hasz, bool geodetic)
{
  /* Geometry is a MULTILINESTRING or a COLLECTION composed of (MULTI)POINT and
   * (MULTI)LINESTRING */
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  int ngeoms = lwcoll->ngeoms;
  int totalgeoms = 0;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    if (lwgeom1->type != POINTTYPE && lwgeom1->type != MULTIPOINTTYPE &&
        lwgeom1->type != LINETYPE && lwgeom1->type != MULTILINETYPE)
    {
      elog(ERROR, "Component geometry/geography must be of type (Multi)Point(Z)M or (Multi)Linestring(Z)M");
    }
    if (lwgeom1->type == POINTTYPE || lwgeom1->type == LINETYPE)
      totalgeoms++;
    else /* lwgeom1->type == MULTIPOINTTYPE || lwgeom1->type == MULTILINETYPE */
      totalgeoms += lwgeom_as_lwcollection(lwgeom1)->ngeoms;
  }

  TSequence **sequences = palloc(sizeof(TSequence *) * totalgeoms);
  int nseqs = 0;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    if (lwgeom1->type == POINTTYPE)
    {
      TInstant *inst1 = geo_to_tpointinst(lwgeom1);
      /* The resulting sequence assumes linear interpolation */
      sequences[nseqs++] = tinstant_to_tsequence(inst1, LINEAR);
      pfree(inst1);
    }
    else if (lwgeom1->type == LINETYPE)
      sequences[nseqs++] = geo_to_tpointseq_linear(lwgeom1, hasz, geodetic);
    else /* lwgeom1->type == MULTIPOINTTYPE || lwgeom1->type == MULTILINETYPE */
    {
      LWCOLLECTION *lwcoll1 = lwgeom_as_lwcollection(lwgeom1);
      int ngeoms1 = lwcoll1->ngeoms;
      for (int j = 0; j < ngeoms1; j++)
      {
        LWGEOM *lwgeom2 = lwcoll1->geoms[j];
        if (lwgeom2->type == POINTTYPE)
        {
          TInstant *inst2 = geo_to_tpointinst(lwgeom2);
          /* The resulting sequence assumes linear interpolation */
          sequences[nseqs++] = tinstant_to_tsequence(inst2, LINEAR);
          pfree(inst2);
        }
        else /* lwgeom2->type == LINETYPE */
          sequences[nseqs++] = geo_to_tpointseq_linear(lwgeom2, hasz, geodetic);
      }
    }
  }
  /* It is necessary to sort the sequences */
  tseqarr_sort(sequences, nseqs);
  /* The resulting sequence set assumes linear interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Converts the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal point.
 * @sqlfunc tgeompoint(), tgeogpoint()
 * @sqlop @p ::
 */
Temporal *
geo_to_tpoint(const GSERIALIZED *geo)
{
  ensure_non_empty(geo);
  ensure_has_M_gs(geo);
  bool hasz = (bool) FLAGS_GET_Z(geo->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(geo->gflags);
  LWGEOM *lwgeom = lwgeom_from_gserialized(geo);
  Temporal *result = NULL; /* Make compiler quiet */
  if (lwgeom->type == POINTTYPE)
    result = (Temporal *) geo_to_tpointinst(lwgeom);
  else if (lwgeom->type == MULTIPOINTTYPE)
    result = (Temporal *) geo_to_tpointseq_disc(lwgeom, hasz);
  else if (lwgeom->type == LINETYPE)
    result = (Temporal *) geo_to_tpointseq_linear(lwgeom, hasz, geodetic);
  else if (lwgeom->type == MULTILINETYPE || lwgeom->type == COLLECTIONTYPE)
    result = (Temporal *) geo_to_tpointseqset(lwgeom, hasz, geodetic);
  else
    elog(ERROR, "Invalid geometry type for trajectory");
  lwgeom_free(lwgeom);
  return result;
}

/***********************************************************************
 * Minimum distance simplification for temporal floats and points.
 * Inspired from Moving Pandas function MinDistanceGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 ***********************************************************************/

/**
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain distance apart.
 * @param[in] seq Temporal value
 * @param[in] dist Minimum distance
 */
TSequence *
tsequence_simplify_min_dist(const TSequence *seq, double dist)
{
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  /* Add first instant to the output sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = inst1;
  int ninsts = 1;
  bool last = false;
  /* Loop for every instant */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    double d = tinstant_distance(inst1, inst2, func);
    if (d > dist)
    {
      /* Add instant to output sequence */
      instants[ninsts++] = inst2;
      inst1 = inst2;
      if (i == seq->count - 1)
        last = true;
    }
  }
  if (seq->count > 1 && ! last)
    instants[ninsts++] = TSEQUENCE_INST_N(seq, seq->count - 1);
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain distance apart.
 * @param[in] ss Temporal value
 * @param[in] dist Distance
 */
TSequenceSet *
tsequenceset_simplify_min_dist(const TSequenceSet *ss, double dist)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tsequence_simplify_min_dist(seq, dist);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain distance apart.
 *
 * This function is inspired from the Moving Pandas function MinDistanceGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @sqlfunc minDistSimplify()
 */
Temporal *
temporal_simplify_min_dist(const Temporal *temp, double dist)
{
  ensure_positive_datum(Float8GetDatum(dist), T_FLOAT8);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_simplify_min_dist((TSequence *) temp, dist);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_simplify_min_dist((TSequenceSet *) temp,
      dist);
  return result;
}

/***********************************************************************
 * Minimum time delta simplification for temporal floats and points.
 * Inspired from Moving Pandas function MinTimeDeltaGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 ***********************************************************************/

/**
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain time interval apart.
 * @param[in] seq Temporal value
 * @param[in] mint Minimum time interval
 */
TSequence *
tsequence_simplify_min_tdelta(const TSequence *seq, const Interval *mint)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  /* Add first instant to the output sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = inst1;
  int ninsts = 1;
  bool last = false;
  /* Loop for every instant */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Interval *duration = pg_timestamp_mi(inst2->t, inst1->t);
    if (pg_interval_cmp(duration, mint) > 0)
    {
      /* Add instant to output sequence */
      instants[ninsts++] = inst2;
      inst1 = inst2;
      if (i == seq->count - 1)
        last = true;
    }
    pfree(duration);
  }
  if (seq->count > 1 && ! last)
    instants[ninsts++] = TSEQUENCE_INST_N(seq, seq->count - 1);
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain time interval apart.
 * @param[in] ss Temporal value
 * @param[in] mint Minimum time interval
 */
TSequenceSet *
tsequenceset_simplify_min_tdelta(const TSequenceSet *ss, const Interval *mint)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tsequence_simplify_min_tdelta(seq, mint);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Simplify the temporal sequence float/point ensuring that consecutive
 * values are at least a certain time interval apart.
 *
 * This function is inspired from the Moving Pandas function MinTimeDeltaGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 * @param[in] temp Temporal value
 * @param[in] mint Minimum time interval
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @sqlfunc minTimeDeltaSimplify()
 */
Temporal *
temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint)
{
  ensure_valid_duration(mint);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_simplify_min_tdelta((TSequence *) temp,
      mint);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_simplify_min_tdelta((TSequenceSet *) temp,
      mint);
  return result;
}

/***********************************************************************
 * Simple Douglas-Peucker-like value simplification for temporal floats.
 ***********************************************************************/

/**
 * @brief Find a split when simplifying the temporal sequence float using the
 * Douglas-Peucker line simplification algorithm.
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 * @note For temporal floats only the Synchronized Distance is used
 */
static void
tfloatseq_findsplit(const TSequence *seq, int i1, int i2, int *split,
  double *dist)
{
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  const TInstant *start = TSEQUENCE_INST_N(seq, i1);
  const TInstant *end = TSEQUENCE_INST_N(seq, i2);
  double startval = DatumGetFloat8(tinstant_value(start));
  double endval = DatumGetFloat8(tinstant_value(end));
  double duration2 = (double) (end->t - start->t);
  /* Loop for every instant between i1 and i2 */
  for (int idx = i1 + 1; idx < i2; idx++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, idx);
    double value = DatumGetFloat8(tinstant_value(inst));
    /*
     * The following is equivalent to
     * tsegment_value_at_timestamp(start, end, LINEAR, inst->t);
     */
    double duration1 = (double) (inst->t - start->t);
    double ratio = duration1 / duration2;
    double value_interp = startval + (endval - startval) * ratio;
    double d = fabs(value - value_interp);
    if (d > *dist)
    {
      /* Record the maximum */
      *split = idx;
      *dist = d;
    }
  }
  return;
}

/***********************************************************************
 * Simple spatio-temporal Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 ***********************************************************************/

/**
 * @brief Return the 2D distance between the points
 */
static double
dist2d_pt_pt(POINT2D *p1, POINT2D *p2)
{
  return hypot(p2->x - p1->x, p2->y - p1->y);
}

/**
 * @brief Return the 3D distance between the points
 */
static double
dist3d_pt_pt(POINT3DZ *p1, POINT3DZ *p2)
{
  return hypot3d(p2->x - p1->x, p2->y - p1->y, p2->z - p1->z);
}

/**
 * @brief Return the 2D distance between the point the segment
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @see http://geomalgorithms.com/a02-_lines.html
 * @note Derived from the PostGIS function lw_dist2d_pt_seg in
 * file measures.c
 */
static double
dist2d_pt_seg(POINT2D *p, POINT2D *A, POINT2D *B)
{
  POINT2D c;
  double r;
  /* If start==end, then use pt distance */
  if (A->x == B->x && A->y == B->y)
    return dist2d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
      ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist2d_pt_pt(p, A);
  if (r > 1)  /* If the second vertex B is closest to the point p */
    return dist2d_pt_pt(p, B);

  /* else if the point p is closer to some point between a and b
  then we find that point and send it to dist2d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);

  return dist2d_pt_pt(p, &c);
}

/**
 * @brief Return the 3D distance between the point the segment
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html
 */
static double
dist3d_pt_seg(POINT3DZ *p, POINT3DZ *A, POINT3DZ *B)
{
  POINT3DZ c;
  double r;
  /* If start==end, then use pt distance */
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && FP_EQUALS(A->z, B->z))
    return dist3d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) +
        (p->z-A->z) * (B->z-A->z) ) /
      ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) +
        (B->z-A->z) * (B->z-A->z) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist3d_pt_pt(p, A);
  if (r > 1) /* If the second vertex B is closest to the point p */
    return dist3d_pt_pt(p, B);

  /* If the point p is closer to some point between a and b, then we find that
     point and send it to dist3d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);
  c.z = A->z + r * (B->z - A->z);

  return dist3d_pt_pt(p, &c);
}

/**
 * @brief Find a split when simplifying the temporal sequence point using the
 * Douglas-Peucker line simplification algorithm.
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[in] syncdist True when using the Synchronized Euclidean Distance
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 */
static void
tpointseq_findsplit(const TSequence *seq, int i1, int i2, bool syncdist,
  int *split, double *dist)
{
  POINT2D *p2k, *p2_sync, *p2a, *p2b;
  POINT3DZ *p3k, *p3_sync, *p3a, *p3b;
  Datum value;
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  double d = -1;
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  /* Initialization of values wrt instants i1 and i2 */
  const TInstant *start = TSEQUENCE_INST_N(seq, i1);
  const TInstant *end = TSEQUENCE_INST_N(seq, i2);
  if (hasz)
  {
    p3a = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value(start));
    p3b = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value(end));
  }
  else
  {
    p2a = (POINT2D *) DATUM_POINT2D_P(tinstant_value(start));
    p2b = (POINT2D *) DATUM_POINT2D_P(tinstant_value(end));
  }

  /* Loop for every instant between i1 and i2 */
  for (int idx = i1 + 1; idx < i2; idx++)
  {
    double d_tmp;
    const TInstant *inst = TSEQUENCE_INST_N(seq, idx);
    if (hasz)
    {
      p3k = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value(inst));
      if (syncdist)
      {
        value = tsegment_value_at_timestamp(start, end, linear, inst->t);
        p3_sync = (POINT3DZ *) DATUM_POINT3DZ_P(value);
        d_tmp = dist3d_pt_pt(p3k, p3_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist3d_pt_seg(p3k, p3a, p3b);
    }
    else
    {
      p2k = (POINT2D *) DATUM_POINT2D_P(tinstant_value(inst));
      if (syncdist)
      {
        value = tsegment_value_at_timestamp(start, end, linear, inst->t);
        p2_sync = (POINT2D *) DATUM_POINT2D_P(value);
        d_tmp = dist2d_pt_pt(p2k, p2_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist2d_pt_seg(p2k, p2a, p2b);
    }
    if (d_tmp > d)
    {
      /* record the maximum */
      d = d_tmp;
      *split = idx;
    }
  }
  *dist = d;
  return;
}

/*****************************************************************************/

/**
 * @brief Simplify the temporal sequence float/point using a single-pass
 * implementation of the Douglas-Peucker line simplification algorithm that
 * checks whether the provided distance threshold is exceeded.
 * @param[in] seq Temporal value
 * @param[in] dist Minimum distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
TSequence *
tsequence_simplify_max_dist(const TSequence *seq, double dist, bool syncdist,
  uint32_t minpts)
{
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *prev = NULL;
  const TInstant *cur = NULL;
  uint32_t start = 0, /* Lower index for finding the split */
           ninsts = 0;     /* Number of instants in the result */
  int split;          /* Index of the split */
  double d;           /* Distance */
  for (int i = 0; i < seq->count; i++)
  {
    cur = TSEQUENCE_INST_N(seq, i);
    if (prev == NULL)
    {
      instants[ninsts++] = cur;
      prev = cur;
      continue;
    }
    /* For temporal floats only Synchronized Distance is used */
    if (seq->temptype == T_TFLOAT)
      tfloatseq_findsplit(seq, start, i, &split, &d);
    else /* tgeo_type(seq->temptype) */
      tpointseq_findsplit(seq, start, i, syncdist, &split, &d);
    bool dosplit = (d >= 0 && (d > dist || start + i + 1 < minpts));
    if (dosplit)
    {
      prev = cur;
      instants[ninsts++] = TSEQUENCE_INST_N(seq, split);
      start = split;
      continue;
    }
  }
  if (instants[ninsts - 1] != cur)
    instants[ninsts++] = cur;
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Simplify the temporal sequence set float/point using a single-pass
 * Douglas-Peucker line simplification algorithm.
 * @param[in] ss Temporal point
 * @param[in] dist Distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
static TSequenceSet *
tsequenceset_simplify_max_dist(const TSequenceSet *ss, double dist,
  bool syncdist, uint32_t minpts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tsequence_simplify_max_dist(seq, dist, syncdist, minpts);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Simplify the temporal float/point using a single-pass Douglas-Peucker
 * line simplification algorithm.
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @param[in] syncdist True when the Synchronized Distance is used, false when
 * the spatial-only distance is used. Only used for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @sqlfunc maxDistSimplify()
 */
Temporal *
temporal_simplify_max_dist(const Temporal *temp, double dist, bool syncdist)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_simplify_max_dist((TSequence *) temp, dist,
      syncdist, 2);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_simplify_max_dist((TSequenceSet *) temp,
      dist, syncdist, 2);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a negative or a positive value depending on whether the first
 * number is less than or greater than the second one
 */
static int
int_cmp(const void *a, const void *b)
{
  /* casting pointer types */
  const int *ia = (const int *) a;
  const int *ib = (const int *) b;
  /* returns negative if b > a and positive if a > b */
  return *ia - *ib;
}

#define DP_STACK_SIZE 256

/**
 * @brief Simplify the temporal sequence set float/point using the
 * Douglas-Peucker line simplification algorithm.
 */
static TSequence *
tsequence_simplify_dp(const TSequence *seq, double dist, bool syncdist,
  uint32_t minpts)
{
  int *stack, *outlist; /* recursion stack */
  int stack_static[DP_STACK_SIZE];
  int outlist_static[DP_STACK_SIZE];
  int sp = -1; /* recursion stack pointer */
  int i1, split;
  uint32_t outn = 0;
  uint32_t i;
  double d;

  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  assert(seq->temptype == T_TFLOAT || tgeo_type(seq->temptype));
  /* Do not try to simplify really short things */
  if (seq->count < 3)
    return tsequence_copy(seq);

  /* Only heap allocate book-keeping arrays if necessary */
  if ((unsigned int) seq->count > DP_STACK_SIZE)
  {
    stack = palloc(sizeof(int) * seq->count);
    outlist = palloc(sizeof(int) * seq->count);
  }
  else
  {
    stack = stack_static;
    outlist = outlist_static;
  }

  i1 = 0;
  stack[++sp] = seq->count - 1;
  /* Add first point to output list */
  outlist[outn++] = 0;
  do
  {
    /* For temporal floats only Synchronized Distance is used */
    if (seq->temptype == T_TFLOAT)
      tfloatseq_findsplit(seq, i1, stack[sp], &split, &d);
    else /* tgeo_type(seq->temptype) */
      tpointseq_findsplit(seq, i1, stack[sp], syncdist, &split, &d);
    bool dosplit = (d >= 0 && (d > dist || outn + sp + 1 < minpts));
    if (dosplit)
      stack[++sp] = split;
    else
    {
      outlist[outn++] = stack[sp];
      i1 = stack[sp--];
    }
  }
  while (sp >= 0);

  /* Order the list of points kept */
  qsort(outlist, outn, sizeof(int), int_cmp);
  /* Create a new temporal sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * outn);
  for (i = 0; i < outn; i++)
    instants[i] = TSEQUENCE_INST_N(seq, outlist[i]);
  TSequence *result = tsequence_make(instants, outn, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);

  /* Free memory only if arrays are on the heap */
  if (stack != stack_static)
    pfree(stack);
  if (outlist != outlist_static)
    pfree(outlist);

  return result;
}

/**
 * @brief Simplify the temporal sequence set float/point using the
 * Douglas-Peucker line simplification algorithm.
 * @param[in] ss Temporal point
 * @param[in] dist Distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
static TSequenceSet *
tsequenceset_simplify_dp(const TSequenceSet *ss, double dist, bool syncdist,
  uint32_t minpts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tsequence_simplify_dp(seq, dist, syncdist, minpts);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Simplify the temporal float/point using the Douglas-Peucker line
 * simplification algorithm.
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @param[in] syncdist True when the Synchronized Distance is used, false when
 * the spatial-only distance is used. Only used for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @sqlfunc DouglasPeuckerSimplify()
 */
Temporal *
temporal_simplify_dp(const Temporal *temp, double dist, bool syncdist)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_simplify_dp((TSequence *) temp, dist,
      syncdist, 2);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_simplify_dp((TSequenceSet *) temp,
      dist, syncdist, 2);
  return result;
}

/*****************************************************************************
 * Mapbox Vector Tile functions for temporal points.
 *****************************************************************************/

/**
 * @brief Return a temporal point with consecutive equal points removed.
 * @note The equality test is done only on x and y dimensions of input.
 */
static TSequence *
tpointseq_remove_repeated_points(const TSequence *seq, double tolerance,
  int min_points)
{
  /* No-op on short inputs */
  if (seq->count <= min_points)
    return tsequence_copy(seq);

  double tolsq = tolerance * tolerance;
  double dsq = FLT_MAX;

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *last = DATUM_POINT2D_P(tinstant_value(instants[0]));
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    bool last_point = (i == seq->count - 1);
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const POINT2D *pt = DATUM_POINT2D_P(tinstant_value(inst));

    /* Don't drop points if we are running short of points */
    if (seq->count - i > min_points - npoints)
    {
      if (tolerance > 0.0)
      {
        /* Only drop points that are within our tolerance */
        dsq = distance2d_sqr_pt_pt(last, pt);
        /* Allow any point but the last one to be dropped */
        if (! last_point && dsq <= tolsq)
          continue;
      }
      else
      {
        /* At tolerance zero, only skip exact dupes */
        if (FP_EQUALS(pt->x, last->x) && FP_EQUALS(pt->y, last->y))
          continue;
      }

      /* Got to last point, and it's not very different from
       * the point that preceded it. We want to keep the last
       * point, not the second-to-last one, so we pull our write
       * index back one value */
      if (last_point && npoints > 1 && tolerance > 0.0 && dsq <= tolsq)
      {
        npoints--;
      }
    }

    /* Save the point */
    instants[npoints++] = inst;
    last = pt;
  }
  /* Construct the result */
  TSequence *result = tsequence_make(instants, npoints, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal point with consecutive equal points removed.
 * @note The equality test is done only on x and y dimensions of input.
 */
static TSequenceSet *
tpointseqset_remove_repeated_points(const TSequenceSet *ss, double tolerance,
  int min_points)
{
  const TSequence *seq;
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq = TSEQUENCESET_SEQ_N(ss, 0);
    TSequence *seq1 = tpointseq_remove_repeated_points(seq, tolerance,
      min_points);
    TSequenceSet *result = tsequence_to_tsequenceset(seq1);
    pfree(seq1);
    return result;
  }

  /* No-op on short inputs */
  if (ss->totalcount <= min_points)
    return tsequenceset_copy(ss);

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int npoints = 0;
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Don't drop sequences if we are running short of points */
    if (ss->totalcount - npoints > min_points)
    {
      /* Minimum number of points set to 2 */
      sequences[i] = tpointseq_remove_repeated_points(seq, tolerance, 2);
      npoints += sequences[i]->count;
    }
    else
    {
      /* Save the sequence */
      sequences[i] = tsequence_copy(seq);
    }
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return a temporal point with consecutive equal points removed.
 * @note The equality test is done only on x and y dimensions of input.
 */
static Temporal *
tpoint_remove_repeated_points(const Temporal *temp, double tolerance,
  int min_points)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_remove_repeated_points(
      (TSequence *) temp, tolerance, min_points);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_remove_repeated_points(
      (TSequenceSet *) temp, tolerance, min_points);
  return result;
}

/*****************************************************************************
 * Affine functions
 *****************************************************************************/

/**
 * @brief Affine transform a temporal point (iterator function)
 */
static void
tpointinst_affine_iter(const TInstant *inst, const AFFINE *a, int srid,
  bool hasz, TInstant **result)
{
  Datum value = tinstant_value(inst);
  double x, y;
  LWPOINT *lwpoint;
  if (hasz)
  {
    const POINT3DZ *pt = DATUM_POINT3DZ_P(value);
    POINT3DZ p3d;
    x = pt->x;
    y = pt->y;
    double z = pt->z;
    p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
    p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
    p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
    lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(value);
    POINT2D p2d;
    x = pt->x;
    y = pt->y;
    p2d.x = a->afac * x + a->bfac * y + a->xoff;
    p2d.y = a->dfac * x + a->efac * y + a->yoff;
    lwpoint = lwpoint_make2d(srid, p2d.x, p2d.y);
  }
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  *result = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
  lwpoint_free(lwpoint);
  pfree(gs);
  return;
}

/**
 * @brief Affine transform a temporal point.
 */
static TInstant *
tpointinst_affine(const TInstant *inst, const AFFINE *a)
{
  int srid = tpointinst_srid(inst);
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags);
  TInstant *result;
  tpointinst_affine_iter(inst, a, srid, hasz, &result);
  return result;
}

/**
 * @brief Affine transform a temporal point.
 */
static TSequence *
tpointseq_affine(const TSequence *seq, const AFFINE *a)
{
  int srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    tpointinst_affine_iter(inst, a, srid, hasz, &instants[i]);
  }
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Affine transform a temporal point.
 * @param[in] ss Temporal point
 * @param[in] a Affine transformation
 */
static TSequenceSet *
tpointseqset_affine(const TSequenceSet *ss, const AFFINE *a)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tpointseq_affine(TSEQUENCESET_SEQ_N(ss, i), a);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Affine transform a temporal point.
 */
static Temporal *
tpoint_affine(const Temporal *temp, const AFFINE *a)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_affine((TInstant *) temp, a);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_affine((TSequence *) temp, a);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_affine((TSequenceSet *) temp, a);
  return result;
}

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

static void
point_grid(Datum value, bool hasz, const gridspec *grid, POINT4D *p)
{
  /* Read and round point */
  datum_point4d(value, p);
  if (grid->xsize > 0)
    p->x = rint((p->x - grid->ipx) / grid->xsize) * grid->xsize + grid->ipx;
  if (grid->ysize > 0)
    p->y = rint((p->y - grid->ipy) / grid->ysize) * grid->ysize + grid->ipy;
  if (hasz && grid->zsize > 0)
    p->z = rint((p->z - grid->ipz) / grid->zsize) * grid->zsize + grid->ipz;
}

/**
 * @brief Stick a temporal point to the given grid specification.
 */
static TInstant *
tpointinst_grid(const TInstant *inst, const gridspec *grid)
{
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags);
  if (grid->xsize == 0 && grid->ysize == 0 && (hasz ? grid->zsize == 0 : 1))
    return tinstant_copy(inst);

  int srid = tpointinst_srid(inst);
  Datum value = tinstant_value(inst);
  POINT4D p;
  point_grid(value, hasz, grid, &p);
  /* Write rounded values into the next instant */
  LWPOINT *lwpoint = hasz ?
    lwpoint_make3dz(srid, p.x, p.y, p.z) : lwpoint_make2d(srid, p.x, p.y);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  lwpoint_free(lwpoint);
  /* Construct the result */
  TInstant *result = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
  /* We cannot lwpoint_free(lwpoint) */
  pfree(gs);
  return result;
}

/**
 * @brief Stick a temporal point to the given grid specification.
 */
static TSequence *
tpointseq_grid(const TSequence *seq, const gridspec *grid, bool filter_pts)
{
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  int srid = tpointseq_srid(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  for (int i = 0; i < seq->count; i++)
  {
    POINT4D p, prev_p = { 0 }; /* make compiler quiet */
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Datum value = tinstant_value(inst);
    point_grid(value, hasz, grid, &p);
    /* Skip duplicates */
    if (i > 1 && prev_p.x == p.x && prev_p.y == p.y &&
      (hasz ? prev_p.z == p.z : 1))
      continue;

    /* Write rounded values into the next instant */
    LWPOINT *lwpoint = hasz ?
      lwpoint_make3dz(srid, p.x, p.y, p.z) : lwpoint_make2d(srid, p.x, p.y);
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    instants[ninsts++] = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
    lwpoint_free(lwpoint);
    pfree(gs);
    memcpy(&prev_p, &p, sizeof(POINT4D));
  }
  /* We are sure that ninsts > 0 */
  if (filter_pts && ninsts == 1)
  {
    pfree_array((void **) instants, 1);
    return NULL;
  }

  /* Construct the result */
  return tsequence_make_free(instants, ninsts, ninsts > 1 ?
    seq->period.lower_inc : true, ninsts > 1 ? seq->period.upper_inc : true,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Stick a temporal point to the given grid specification.
 */
static TSequenceSet *
tpointseqset_grid(const TSequenceSet *ss, const gridspec *grid, bool filter_pts)
{
  int nseqs = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tpointseq_grid(TSEQUENCESET_SEQ_N(ss, i), grid, filter_pts);
    if (seq != NULL)
      sequences[nseqs++] = seq;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Stick a temporal point to the given grid specification.
 *
 * Only the x, y, and possible z dimensions are gridded, the timestamp is
 * kept unmodified. Two consecutive instants falling on the same grid cell
 * are collapsed into one single instant.
 */
static Temporal *
tpoint_grid(const Temporal *temp, const gridspec *grid, bool filter_pts)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_grid((TInstant *) temp, grid);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_grid((TSequence *) temp, grid, filter_pts);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_grid((TSequenceSet *) temp, grid,
      filter_pts);
  return result;
}

/*****************************************************************************/

/**
 * @brief Transform a temporal point into vector tile coordinate space.
 * @param[in] tpoint Temporal point
 * @param[in] box Geometric bounds of the tile contents without buffer
 * @param[in] extent Tile extent in tile coordinate space
 * @param[in] buffer Buffer distance in tile coordinate space
 * @param[in] clip_geom True if temporal point should be clipped
 */
static Temporal *
tpoint_mvt(const Temporal *tpoint, const STBox *box, uint32_t extent,
  uint32_t buffer, bool clip_geom)
{
  AFFINE affine = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gridspec grid = {0, 0, 0, 0, 1, 1, 0, 0};
  double width = box->xmax - box->xmin;
  double height = box->ymax - box->ymin;
  double resx, resy, res, fx, fy;

  resx = width / extent;
  resy = height / extent;
  res = (resx < resy ? resx : resy) / 2;
  fx = extent / width;
  fy = -(extent / height);

  /* Remove all non-essential points (under the output resolution) */
  Temporal *tpoint1 = tpoint_remove_repeated_points(tpoint, res, 2);

  /* Euclidean (not synchronized) distance, i.e., parameter set to false */
  Temporal *tpoint2 = temporal_simplify_dp(tpoint1, res, false);
  pfree(tpoint1);

  /* Transform to tile coordinate space */
  affine.afac = fx;
  affine.efac = fy;
  affine.ifac = 1;
  affine.xoff = -box->xmin * fx;
  affine.yoff = -box->ymax * fy;
  Temporal *tpoint3 = tpoint_affine(tpoint2, &affine);
  pfree(tpoint2);

  /* Snap to integer precision, removing duplicate and single points */
  Temporal *tpoint4 = tpoint_grid(tpoint3, &grid, true);
  pfree(tpoint3);
  if (tpoint4 == NULL || ! clip_geom)
    return tpoint4;

  /* Clip temporal point taking into account the buffer */
  double max = (double) extent + (double) buffer;
  double min = -(double) buffer;
  int srid = tpoint_srid(tpoint);
  STBox clip_box;
  stbox_set(true, false, false, srid, min, max, min, max, 0, 0, NULL,
    &clip_box);
  Temporal *tpoint5 = tpoint_restrict_stbox(tpoint4, &clip_box, false,
    REST_AT);
  pfree(tpoint4);
  if (tpoint5 == NULL)
    return NULL;
  /* We need to grid again the result of the clipping */
  Temporal *result = tpoint_grid(tpoint5, &grid, true);
  pfree(tpoint5);
  return result;
}

/*****************************************************************************
 * Decouple the points and the timestamps of a temporal point.
 * With respect to the trajectory functions, e.g., #tpoint_trajectory,
 * the resulting geometry is not optimized in order to maintain the
 * composing points of the geometry and the associated timestamps synchronized
 *****************************************************************************/

/**
 * @brief Decouple the points and the timestamps of a temporal point.
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] inst Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpointinst_decouple(const TInstant *inst, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64));
  times[0] = (inst->t / 1000000) + DELTA_UNIX_POSTGRES_EPOCH;
  *timesarr = times;
  *count = 1;
  return DatumGetGserializedP(tinstant_value_copy(inst));
}

/**
 * @brief Decouple the points and the timestamps of a temporal point
 * (iterator function).
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] seq Temporal point
 * @param[out] times Array of timestamps
 * @note The timestamps are returned in Unix epoch
 */
static LWGEOM *
tpointseq_decouple_iter(const TSequence *seq, int64 *times)
{
  /* General case */
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Datum value = tinstant_value(inst);
    GSERIALIZED *gs = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gs);
    times[i] = (inst->t / 1000000) + DELTA_UNIX_POSTGRES_EPOCH;
  }
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  LWGEOM *result = lwpointarr_make_trajectory(points, seq->count, interp);
  if (interp == LINEAR)
  {
    for (int i = 0; i < seq->count; i++)
      lwpoint_free((LWPOINT *) points[i]);
    pfree(points);
  }
  return result;
}

/**
 * @brief Decouple the points and the timestamps of a temporal point.
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] seq Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpointseq_decouple(const TSequence *seq, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64) * seq->count);
  LWGEOM *lwgeom = tpointseq_decouple_iter(seq, times);
  GSERIALIZED *result = geo_serialize(lwgeom);
  pfree(lwgeom);
  *timesarr = times;
  *count = seq->count;
  return result;
}

/**
 * @brief Decouple the points and the timestamps of a temporal point.
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] ss Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpointseqset_decouple(const TSequenceSet *ss, int64 **timesarr, int *count)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_decouple(TSEQUENCESET_SEQ_N(ss, 0), timesarr, count);

  /* General case */
  uint32_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->count);
  int64 *times = palloc(sizeof(int64) * ss->totalcount);
  int ntimes = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    geoms[i] = tpointseq_decouple_iter(seq, &times[ntimes]);
    ntimes += seq->count;
    /* If output type not initialized make geom type as output type */
    if (! colltype)
      colltype = lwtype_get_collectiontype(geoms[i]->type);
    /* If geom type is not compatible with current output type
     * make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[i]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  LWGEOM *coll = (LWGEOM *) lwcollection_construct((uint8_t) colltype,
    geoms[0]->srid, NULL, (uint32_t) ss->count, geoms);
  GSERIALIZED *result = geo_serialize(coll);
  *timesarr = times;
  *count = ss->totalcount;
  /* We cannot lwgeom_free(geoms[i]) or pfree(geoms) */
  lwgeom_free(coll);
  return result;
}

/**
 * @brief Decouple the points and the timestamps of a temporal point.
 * @param[in] temp Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpoint_decouple(const Temporal *temp, int64 **timesarr, int *count)
{
  GSERIALIZED *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tpointinst_decouple((TInstant *) temp, timesarr, count);
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_decouple((TSequence *) temp, timesarr, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_decouple((TSequenceSet *) temp, timesarr, count);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Transform the temporal point to Mapbox Vector Tile format
 * @sqlfunc AsMVTGeom()
 */
bool
tpoint_AsMVTGeom(const Temporal *temp, const STBox *bounds, int32_t extent,
  int32_t buffer, bool clip_geom, GSERIALIZED **geom, int64 **timesarr,
  int *count)
{
  if (bounds->xmax - bounds->xmin <= 0 || bounds->ymax - bounds->ymin <= 0)
    elog(ERROR, "%s: Geometric bounds are too small", __func__);
  if (extent <= 0)
    elog(ERROR, "%s: Extent must be greater than 0", __func__);

  /* Contrary to what is done in PostGIS we do not use the following filter
   * to enable the visualization of temporal points with instant subtype.
   * PostGIS filtering adapted to MobilityDB would be as follows.

  / * Bounding box test to drop geometries smaller than the resolution * /
  STBox box;
  temporal_set_bbox(temp, &box);
  double tpoint_width = box.xmax - box.xmin;
  double tpoint_height = box.ymax - box.ymin;
  / * We use half of the square height and width as limit: We use this
   * and not area so it works properly with lines * /
  double bounds_width = ((bounds->xmax - bounds->xmin) / extent) / 2.0;
  double bounds_height = ((bounds->ymax - bounds->ymin) / extent) / 2.0;
  if (tpoint_width < bounds_width && tpoint_height < bounds_height)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_NULL();
  }
  */

  Temporal *temp1 = tpoint_mvt(temp, bounds, extent, buffer, clip_geom);
  if (temp1 == NULL)
    return false;

  /* Decouple the geometry and the timestamps */
  *geom = tpoint_decouple(temp1, timesarr, count);

  pfree(temp1);
  return true;
}

/*****************************************************************************/
