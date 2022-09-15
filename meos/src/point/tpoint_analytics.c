  /***********************************************************************
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
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/tsequence.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialrels.h"
#include "point/tpoint_spatialfuncs.h"

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
 *****************************************************************************/

/**
 * Convert the geometry/geography point and the measure into a PostGIS point
 * with an M coordinate
 */
static LWPOINT *
point_measure_to_lwpoint(Datum point, Datum measure)
{
  GSERIALIZED *gs = DatumGetGserializedP(point);
  int32 srid = gserialized_get_srid(gs);
  double d = DatumGetFloat8(measure);
  LWPOINT *result;
  if (FLAGS_GET_Z(gs->gflags))
  {
    const POINT3DZ *point = gserialized_point3dz_p(gs);
    result = lwpoint_make4d(srid, point->x, point->y, point->z, d);
  }
  else
  {
    const POINT2D *point = gserialized_point2d_p(gs);
    result = lwpoint_make3dm(srid, point->x, point->y, d);
  }
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->gflags));
  return result;
}

/**
 * Convert the temporal instant point into a PostGIS trajectory point
 * where the M coordinate encodes the timestamp in Unix epoch
 */
static LWPOINT *
tpointinst_to_lwpoint(const TInstant *inst)
{
  double epoch = ((double) inst->t / 1e6) + DELTA_UNIX_POSTGRES_EPOCH;
  return point_measure_to_lwpoint(tinstant_value(inst), Float8GetDatum(epoch));
}

/*****************************************************************************
 * Convert a temporal point into a LinestringM geometry/geography where the M
 * coordinates values are given by a temporal float.
 *****************************************************************************/

/**
 * Construct a geometry/geography with M measure from the temporal instant
 * point and the temporal float.
 *
 * @param[in] inst Temporal point
 * @param[in] measure Temporal float
 */
static GSERIALIZED *
tpointinst_to_geo_measure(const TInstant *inst, const TInstant *measure)
{
  LWPOINT *point = measure ?
    point_measure_to_lwpoint(tinstant_value(inst), tinstant_value(measure)) :
    tpointinst_to_lwpoint(inst);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  pfree(point);
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal discrete
 * sequence point and the temporal float.
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static GSERIALIZED *
tpointdiscseq_to_geo_measure(const TSequence *seq, const TSequence *measure)
{
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (measure)
    {
      const TInstant *m = tsequence_inst_n(measure, i);
      points[i] = (LWGEOM *) point_measure_to_lwpoint(tinstant_value(inst),
          tinstant_value(m));
    }
    else
      points[i] = (LWGEOM *) tpointinst_to_lwpoint(inst);
  }
  GSERIALIZED *result;
  if (seq->count == 1)
    result = geo_serialize(points[0]);
  else
  {
    LWGEOM *mpoint = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
      points[0]->srid, NULL, (uint32_t) seq->count, points);
    result = geo_serialize(mpoint);
    pfree(mpoint);
  }
  for (int i = 0; i < seq->count; i++)
    lwgeom_free (points[i]);
  pfree(points);
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. The function removes one point if two
 * consecutive points are equal
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 * @pre The temporal point and the measure are synchronized
 */
static LWGEOM *
tpointseq_to_geo_measure1(const TSequence *seq, const TSequence *measure)
{
  LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
  /* Remove two consecutive points if they are equal */
  const TInstant *inst = tsequence_inst_n(seq, 0);
  const TInstant *m = tsequence_inst_n(measure, 0);
  LWPOINT *value1 = point_measure_to_lwpoint(tinstant_value(inst),
    tinstant_value(m));
  points[0] = value1;
  int k = 1;
  for (int i = 1; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    m = tsequence_inst_n(measure, i);
    LWPOINT *value2 = point_measure_to_lwpoint(tinstant_value(inst),
      tinstant_value(m));
    /* Add point only if previous point is diffrent from the current one */
    if (lwpoint_same(value1, value2) != LW_TRUE)
      points[k++] = value2;
    value1 = value2;
  }
  LWGEOM *result;
  if (k == 1)
  {
    result = (LWGEOM *) points[0];
    pfree(points);
  }
  else
  {
    result = MOBDB_FLAGS_GET_LINEAR(seq->flags) ?
      (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, (uint32_t) k,
        (LWGEOM **) points) :
      (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) k, (LWGEOM **) points);
    for (int i = 0; i < k; i++)
      lwpoint_free(points[i]);
    pfree(points);
  }
  return result;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory
 * geometry/geography where the M coordinates encode the timestamps in
 * Unix epoch
 */
static LWGEOM *
tpointseq_to_geo1(const TSequence *seq)
{
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    points[i] = (LWGEOM *) tpointinst_to_lwpoint(inst);
  }
  LWGEOM *result;
  if (seq->count == 1)
  {
    result = points[0];
    pfree(points);
  }
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
      result = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
        (uint32_t) seq->count, points);
    else
      result = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) seq->count, points);
    for (int i = 0; i < seq->count; i++)
      lwpoint_free((LWPOINT *) points[i]);
    pfree(points);
  }
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float.
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static GSERIALIZED *
tpointseq_to_geo_measure(const TSequence *seq, const TSequence *measure)
{
  LWGEOM *lwgeom = measure ?
    tpointseq_to_geo_measure1(seq, measure) : tpointseq_to_geo1(seq);
  GSERIALIZED *result = geo_serialize(lwgeom);
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float.
 *
 * @param[in] ss Temporal point
 * @param[in] measure Temporal float
 */
static GSERIALIZED *
tpointseqset_to_geo_measure(const TSequenceSet *ss, const TSequenceSet *measure)
{
  const TSequence *seq;
  const TSequence *m = NULL;

  /* Instantaneous sequence */
  if (ss->count == 1)
  {
    seq = tsequenceset_seq_n(ss, 0);
    if (measure)
      m = tsequenceset_seq_n(measure, 0);
    return tpointseq_to_geo_measure(seq, m);
  }

  uint8_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    seq = tsequenceset_seq_n(ss, i);
    if (measure)
    {
      m = tsequenceset_seq_n(measure, i);
      geoms[i] = tpointseq_to_geo_measure1(seq, m);
    }
    else
      geoms[i] = tpointseq_to_geo1(seq);
    /* If output type not initialized make geom type as output type */
    if (! colltype)
      colltype = (uint8_t) lwtype_get_collectiontype(geoms[i]->type);
    /* If geom type is not compatible with current output type make output
     * type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[i]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  // TODO add the bounding box instead of ask PostGIS to compute it again
  LWGEOM *coll = (LWGEOM *) lwcollection_construct((uint8_t) colltype,
    geoms[0]->srid, NULL, (uint32_t) ss->count, geoms);
  GSERIALIZED *result = geo_serialize(coll);
  /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  pfree(geoms);
  return result;
}

/*****************************************************************************/

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float.
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tpointseq_to_geo_measure_segmentize1(const TSequence *seq,
  const TSequence *measure, LWGEOM **result)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  const TInstant *m = tsequence_inst_n(measure, 0);
  LWPOINT *points[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = (LWGEOM *) point_measure_to_lwpoint(tinstant_value(inst),
      tinstant_value(m));
    return 1;
  }

  /* General case */
  for (int i = 0; i < seq->count - 1; i++)
  {
    points[0] = point_measure_to_lwpoint(tinstant_value(inst),
      tinstant_value(m));
    inst = tsequence_inst_n(seq, i + 1);
    points[1] = point_measure_to_lwpoint(tinstant_value(inst),
      tinstant_value(m));
    result[i] = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, 2,
      (LWGEOM **) points);
    lwpoint_free(points[0]); lwpoint_free(points[1]);
    m = tsequence_inst_n(measure, i + 1);
  }
  return seq->count - 1;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory
 * geometry/geography where the M coordinates encode the timestamps in
 * Unix epoch
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 *
 * @param[in] seq Temporal point
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tpointseq_to_geo_segmentize1(const TSequence *seq, LWGEOM **result)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  LWPOINT *points[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = (LWGEOM *) tpointinst_to_lwpoint(inst);
    return 1;
  }

  /* General case */
  for (int i = 0; i < seq->count - 1; i++)
  {
    points[0] = tpointinst_to_lwpoint(inst);
    inst = tsequence_inst_n(seq, i + 1);
    points[1] = tpointinst_to_lwpoint(inst);
    result[i] = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, 2,
      (LWGEOM **) points);
    lwpoint_free(points[0]); lwpoint_free(points[1]);
  }
  return seq->count - 1;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float.
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static GSERIALIZED *
tpointseq_to_geo_measure_segmentize(const TSequence *seq,
  const TSequence *measure)
{
  int count = (seq->count == 1) ? 1 : seq->count - 1;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  if (measure)
    tpointseq_to_geo_measure_segmentize1(seq, measure, geoms);
  else
    tpointseq_to_geo_segmentize1(seq, geoms);
  GSERIALIZED *result;
  /* Instantaneous sequence */
  if (seq->count == 1)
    result = geo_serialize(geoms[0]);
  else
  {
    // TODO add the bounding box instead of ask PostGIS to compute it again
    LWGEOM *segcoll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
      geoms[0]->srid, NULL, (uint32_t)(seq->count - 1), geoms);
    result = geo_serialize(segcoll);
  }
  for (int i = 0; i < count; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence set
 * point and the temporal float.
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static GSERIALIZED *
tpointseqset_to_geo_measure_segmentize(const TSequenceSet *ss,
  const TSequenceSet *measure)
{
  const TSequence *seq, *m = NULL;

  /* Instantaneous sequence */
  if (ss->count == 1)
  {
    seq = tsequenceset_seq_n(ss, 0);
    if (measure)
      m = tsequenceset_seq_n(measure, 0);
    return tpointseq_to_geo_measure_segmentize(seq, m);
  }

  uint8_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {

    seq = tsequenceset_seq_n(ss, i);
    if (measure)
    {
      m = tsequenceset_seq_n(measure, i);
      k += tpointseq_to_geo_measure_segmentize1(seq, m, &geoms[k]);
    }
    else
      k += tpointseq_to_geo_segmentize1(seq, &geoms[k]);
    /* Output type not initialized */
    if (! colltype)
      colltype = (uint8_t) lwtype_get_collectiontype(geoms[k - 1]->type);
      /* Input type not compatible with output */
      /* make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
         lwtype_get_collectiontype(geoms[k - 1]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  GSERIALIZED *result;
  // TODO add the bounding box instead of ask PostGIS to compute it again
  LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
    geoms[0]->srid, NULL, (uint32_t) k, geoms);
  result = geo_serialize(coll);
  for (int i = 0; i < k; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Construct a geometry/geography with M measure from the temporal
 * point and
 * - either the temporal float given in the second argument (if any)
 * - or the time information of the temporal point where the M coordinates
 *   encode the timestamps in number of seconds since '1970-01-01'
 * @sqlfunc geoMeasure() if the second argument is not NULL
 * @sqlop @p :: if the second argument is NULL
 */
bool
tpoint_to_geo_measure(const Temporal *tpoint, const Temporal *measure,
  bool segmentize, GSERIALIZED **result)
{
  ensure_tgeo_type(tpoint->temptype);
  Temporal *sync1, *sync2;
  if (measure)
  {
    ensure_tnumber_type(measure->temptype);
    /* Return false if the temporal values do not intersect in time
     * The operation is synchronization without adding crossings */
    if (! intersection_temporal_temporal(tpoint, measure, SYNCHRONIZE_NOCROSS,
        &sync1, &sync2))
      return false;
  }
  else
  {
    sync1 = (Temporal *) tpoint;
    sync2 = NULL;
  }

  ensure_valid_tempsubtype(sync1->subtype);
  if (sync1->subtype == TINSTANT)
    *result = tpointinst_to_geo_measure(
      (TInstant *) sync1, (TInstant *) sync2);
  else if (sync1->subtype == TSEQUENCE)
  {
    if (MOBDB_FLAGS_GET_DISCRETE(sync1->flags))
      *result = tpointdiscseq_to_geo_measure(
        (TSequence *) sync1, (TSequence *) sync2);
    else
      *result = segmentize ?
        tpointseq_to_geo_measure_segmentize(
          (TSequence *) sync1, (TSequence *) sync2) :
        tpointseq_to_geo_measure(
          (TSequence *) sync1, (TSequence *) sync2);
  }
  else /* sync1->subtype == TSEQUENCESET */
    *result = segmentize ?
      tpointseqset_to_geo_measure_segmentize(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2) :
      tpointseqset_to_geo_measure(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2);

  if (measure)
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
 * Converts the PostGIS trajectory geometry/geography where the M coordinates
 * encode the timestamps in Unix epoch into a temporal instant point.
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
  FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint1);
  mobdbType temptype = geodetic ? T_TGEOGPOINT : T_TGEOMPOINT;
  TInstant *result = tinstant_make(PointerGetDatum(gs), temptype, t);
  pfree(gs);
  return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates
 * encode the timestamps in Unix epoch into a temporal instant point.
 */
static TInstant *
geo_to_tpointinst(const GSERIALIZED *geo)
{
  /* Geometry is a POINT */
  LWGEOM *lwgeom = lwgeom_from_gserialized(geo);
  TInstant *result = trajpoint_to_tpointinst((LWPOINT *) lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates
 * encode the timestamps in Unix epoch into a temporal discrete sequence point.
 */
static TSequence *
geo_to_tpointdiscseq(const GSERIALIZED *geo)
{
  /* Geometry is a MULTIPOINT */
  LWGEOM *lwgeom = lwgeom_from_gserialized(geo);
  bool hasz = (bool) FLAGS_GET_Z(geo->gflags);
  /* Verify that is a valid set of trajectory points */
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  double m1 = -1 * DBL_MAX, m2;
  int npoints = lwcoll->ngeoms;
  for (int i = 0; i < npoints; i++)
  {
    LWPOINT *lwpoint = (LWPOINT *) lwcoll->geoms[i];
    if (hasz)
    {
      POINT4D point = getPoint4d(lwpoint->point, 0);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(lwpoint->point, 0);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      lwgeom_free(lwgeom);
      elog(ERROR, "Trajectory must be valid");
    }
    m1 = m2;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (int i = 0; i < npoints; i++)
    instants[i] = trajpoint_to_tpointinst((LWPOINT *) lwcoll->geoms[i]);
  lwgeom_free(lwgeom);

  return tsequence_make_free(instants, npoints, npoints, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates
 * encode the timestamps in Unix epoch into a temporal sequence point.
 */
static TSequence *
geo_to_tpointseq(const GSERIALIZED *geo)
{
  /* Geometry is a LINESTRING */
  bool hasz = (bool) FLAGS_GET_Z(geo->gflags);
  LWGEOM *lwgeom = lwgeom_from_gserialized(geo);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom);
  int npoints = lwline->points->npoints;
  /*
   * Verify that the trajectory is valid.
   * Since calling lwgeom_is_trajectory causes discrepancies with regression
   * tests because of the error message depends on PostGIS version,
   * the verification is made here.
   */
  double m1 = -1 * DBL_MAX, m2;
  for (int i = 0; i < npoints; i++)
  {
    if (hasz)
    {
      POINT4D point = getPoint4d(lwline->points, (uint32_t) i);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(lwline->points, (uint32_t) i);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      lwgeom_free(lwgeom);
      elog(ERROR, "Trajectory must be valid");
    }
    m1 = m2;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (int i = 0; i < npoints; i++)
  {
    /* Return freshly allocated LWPOINT */
    LWPOINT *lwpoint = lwline_get_lwpoint(lwline, (uint32_t) i);
    instants[i] = trajpoint_to_tpointinst(lwpoint);
    lwpoint_free(lwpoint);
  }
  lwgeom_free(lwgeom);
  /* The resulting sequence assumes linear interpolation */
  return tsequence_make_free(instants, npoints, npoints, true, true, LINEAR,
    NORMALIZE);
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates
 * encode the timestamps in Unix epoch into a temporal sequence set point.
 */
static TSequenceSet *
geo_to_tpointseqset(const GSERIALIZED *geo)
{
  /* Geometry is a MULTILINESTRING or a COLLECTION composed of POINT and LINESTRING */
  LWGEOM *lwgeom = lwgeom_from_gserialized(geo);
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  int ngeoms = lwcoll->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    if (lwgeom1->type != POINTTYPE && lwgeom1->type != LINETYPE)
    {
      lwgeom_free(lwgeom);
      elog(ERROR, "Component geometry/geography must be of type Point(Z)M or Linestring(Z)M");
    }
  }

  TSequence **sequences = palloc(sizeof(TSequence *) * ngeoms);
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    GSERIALIZED *gs1 = geo_serialize(lwgeom1);
    if (lwgeom1->type == POINTTYPE)
    {
      TInstant *inst = geo_to_tpointinst(gs1);
      /* The resulting sequence assumes linear interpolation */
      sequences[i] = tinstant_to_tsequence(inst, LINEAR);
      pfree(inst);
    }
    else /* lwgeom1->type == LINETYPE */
      sequences[i] = geo_to_tpointseq(gs1);
    pfree(gs1);
  }
  lwgeom_free(lwgeom);
  /* The resulting sequence set assumes linear interpolation */
  return tsequenceset_make_free(sequences, ngeoms, NORMALIZE_NO);
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
  Temporal *result = NULL; /* Make compiler quiet */
  int geomtype = gserialized_get_type(geo);
  if (geomtype == POINTTYPE)
    result = (Temporal *) geo_to_tpointinst(geo);
  else if (geomtype == MULTIPOINTTYPE)
    result = (Temporal *) geo_to_tpointdiscseq(geo);
  else if (geomtype == LINETYPE)
    result = (Temporal *) geo_to_tpointseq(geo);
  else if (geomtype == MULTILINETYPE || geomtype == COLLECTIONTYPE)
    result = (Temporal *) geo_to_tpointseqset(geo);
  else
    elog(ERROR, "Invalid geometry type for trajectory");
  return result;
}

/***********************************************************************
 * Simple Douglas-Peucker-like value simplification for temporal floats.
 ***********************************************************************/

/**
 * Finds a split when simplifying the temporal sequence point using a
 * spatio-temporal extension of the Douglas-Peucker line simplification
 * algorithm.
 *
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 */
static void
tfloatseq_findsplit(const TSequence *seq, int i1, int i2, int *split,
  double *dist)
{
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  const TInstant *start = tsequence_inst_n(seq, i1);
  const TInstant *end = tsequence_inst_n(seq, i2);
  double value1 = DatumGetFloat8(tinstant_value(start));
  double value2 = DatumGetFloat8(tinstant_value(end));
  double duration2 = (double) (end->t - start->t);
  /* Loop for every instant between i1 and i2 */
  const TInstant *inst1 = start;
  for (int k = i1 + 1; k < i2; k++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, k);
    double value = DatumGetFloat8(tinstant_value(inst2));
    double duration1 = (double) (inst2->t - inst1->t);
    double ratio = duration1 / duration2;
    double value_interp = value1 + (value2 - value1) * ratio;
    double d = fabs(value - value_interp);
    if (d > *dist)
    {
      /* record the maximum */
      *split = k;
      *dist = d;
    }
  }
  return;
}

/**
 * Return a negative or a positive value depending on whether the first number
 * is less than or greater than the second one
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

/***********************************************************************
 * Simple spatio-temporal Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 ***********************************************************************/

#if 0 /* not used */
/**
 * Return the speed of the temporal point in the segment in units per second
 *
 * @param[in] inst1, inst2 Instants defining the segment
 * @param[in] func Distance function (2D, 3D, or geodetic)
 */
static double
tpointinst_speed(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  return datum_point_eq(value1, value2) ? 0 :
    DatumGetFloat8(func(value1, value2)) /
      ((double)(inst2->t - inst1->t) / 1000000);
}
#endif /* not used */

/**
 * Return the 2D distance between the points
 */
static double
dist2d_pt_pt(POINT2D *p1, POINT2D *p2)
{
  return hypot(p2->x - p1->x, p2->y - p1->y);
}

/**
 * Return the 3D distance between the points
 */
static double
dist3d_pt_pt(POINT3DZ *p1, POINT3DZ *p2)
{
  return hypot3d(p2->x - p1->x, p2->y - p1->y, p2->z - p1->z);
}

/**
 * Return the 2D distance between the point the segment
 *
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
 * Return the 3D distance between the point the segment
 *
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
 * Finds a split when simplifying the temporal sequence point using a
 * spatio-temporal extension of the Douglas-Peucker line simplification
 * algorithm.
 *
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[in] synchronized True when using the Synchronized Euclidean Distance
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 */
static void
tpointseq_findsplit(const TSequence *seq, int i1, int i2, bool synchronized,
  int *split, double *dist)
{
  POINT2D p2k, p2_sync, p2a, p2b;
  POINT3DZ p3k, p3_sync, p3a, p3b;
  Datum value;
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  double d = -1;
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  /* Initialization of values wrt instants i1 and i2 */
  const TInstant *start = tsequence_inst_n(seq, i1);
  const TInstant *end = tsequence_inst_n(seq, i2);
  if (hasz)
  {
    p3a = datum_point3dz(tinstant_value(start));
    p3b = datum_point3dz(tinstant_value(end));
  }
  else
  {
    p2a = datum_point2d(tinstant_value(start));
    p2b = datum_point2d(tinstant_value(end));
  }

  /* Loop for every instant between i1 and i2 */
  for (int k = i1 + 1; k < i2; k++)
  {
    double d_tmp;
    const TInstant *inst = tsequence_inst_n(seq, k);
    if (hasz)
    {
      p3k = datum_point3dz(tinstant_value(inst));
      if (synchronized)
      {
        value = tsegment_value_at_timestamp(start, end, linear, inst->t);
        p3_sync = datum_point3dz(value);
        d_tmp = dist3d_pt_pt(&p3k, &p3_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist3d_pt_seg(&p3k, &p3a, &p3b);
    }
    else
    {
      p2k = datum_point2d(tinstant_value(inst));
      if (synchronized)
      {
        value = tsegment_value_at_timestamp(start, end, linear, inst->t);
        p2_sync = datum_point2d(value);
        d_tmp = dist2d_pt_pt(&p2k, &p2_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist2d_pt_seg(&p2k, &p2a, &p2b);
    }
    if (d_tmp > d)
    {
      /* record the maximum */
      d = d_tmp;
      *split = k;
    }
  }
  *dist = d;
  return;
}

/*****************************************************************************/

static TSequence *
tsequence_simplify(const TSequence *seq, double eps_dist, bool synchronized,
  uint32_t minpts)
{
  static size_t stack_size = 256;
  int *stack, *outlist; /* recursion stack */
  int stack_static[stack_size];
  int outlist_static[stack_size];
  int sp = -1; /* recursion stack pointer */
  int i1, split;
  uint32_t outn = 0;
  uint32_t i;
  double dist;

  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  assert(seq->temptype == T_TFLOAT || tgeo_type(seq->temptype));
  /* Do not try to simplify really short things */
  if (seq->count < 3)
    return tsequence_copy(seq);

  /* Only heap allocate book-keeping arrays if necessary */
  if ((unsigned int) seq->count > stack_size)
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
    if (seq->temptype == T_TFLOAT)
      /* There is no synchronized distance for temporal floats */
      tfloatseq_findsplit(seq, i1, stack[sp], &split, &dist);
    else /* tgeo_type(seq->temptype) */
      tpointseq_findsplit(seq, i1, stack[sp], synchronized, &split, &dist);
    bool dosplit = (dist >= 0 &&
      (dist > eps_dist || outn + sp + 1 < minpts));
    if (dosplit)
      stack[++sp] = split;
    else
    {
      outlist[outn++] = stack[sp];
      i1 = stack[sp--];
    }
  }
  while (sp >= 0);

  /* Put list of retained points into order */
  qsort(outlist, outn, sizeof(int), int_cmp);
  /* Create new TSequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * outn);
  for (i = 0; i < outn; i++)
    instants[i] = tsequence_inst_n(seq, outlist[i]);
  TSequence *result = tsequence_make(instants, outn, outn,
    seq->period.lower_inc, seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);

  /* Only free if arrays are on heap */
  if (stack != stack_static)
    pfree(stack);
  if (outlist != outlist_static)
    pfree(outlist);

  return result;
}

/**
 * Simplify the temporal sequence set float/point using a spatio-temporal
 * extension of the Douglas-Peucker line simplification algorithm.
 *
 * @param[in] ss Temporal point
 * @param[in] eps_dist Epsilon speed
 * @param[in] synchronized True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only Douglas-Peucker
 * @param[in] minpts Minimum number of points
 */
static TSequenceSet *
tsequenceset_simplify(const TSequenceSet *ss, double eps_dist,
  bool synchronized, uint32_t minpts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    sequences[i] = tsequence_simplify(seq, eps_dist, synchronized, minpts);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_analytics
 * @brief Simplify the temporal float/point using a spatio-temporal
 * extension of the Douglas-Peucker line simplification algorithm.
 * @sqlfunc simplify()
 */
Temporal *
temporal_simplify(const Temporal *temp, double eps_dist, bool synchronized)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_simplify((TSequence *) temp, eps_dist,
      synchronized, 2);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_simplify((TSequenceSet *) temp,
      eps_dist, synchronized, 2);
  return result;
}

/*****************************************************************************
 * Mapbox Vector Tile functions for temporal points.
 *****************************************************************************/

/**
 * Return a temporal point with consecutive equal points removed.
 * Equality test only on x and y dimensions of input.
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
  instants[0] = tsequence_inst_n(seq, 0);
  const POINT2D *last = datum_point2d_p(tinstant_value(instants[0]));
  int k = 1;
  for (int i = 1; i < seq->count; i++)
  {
    bool last_point = (i == seq->count - 1);
    const TInstant *inst = tsequence_inst_n(seq, i);
    const POINT2D *pt = datum_point2d_p(tinstant_value(inst));

    /* Don't drop points if we are running short of points */
    if (seq->count - i > min_points - k)
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
      if (last_point && k > 1 && tolerance > 0.0 && dsq <= tolsq)
      {
        k--;
      }
    }

    /* Save the point */
    instants[k++] = inst;
    last = pt;
  }
  /* Construct the result */
  TSequence *result = tsequence_make(instants, k, k, seq->period.lower_inc,
    seq->period.upper_inc, MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * Return a temporal point with consecutive equal points removed.
 * Equality test only on x and y dimensions of input.
 */
static TSequenceSet *
tpointseqset_remove_repeated_points(const TSequenceSet *ss, double tolerance,
  int min_points)
{
  const TSequence *seq;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq = tsequenceset_seq_n(ss, 0);
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
    seq = tsequenceset_seq_n(ss, i);
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
 * Return a temporal point with consecutive equal points removed.
 * Equality test only on x and y dimensions of input.
 */
static Temporal *
tpoint_remove_repeated_points(const Temporal *temp, double tolerance,
  int min_points)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
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
 * Affine transform a temporal point (iterator function)
 */
static void
tpointinst_affine1(const TInstant *inst, const AFFINE *a, int srid,
  bool hasz, TInstant **result)
{
  Datum value = tinstant_value(inst);
  double x, y;
  LWPOINT *lwpoint;
  if (hasz)
  {
    POINT3DZ p3d = datum_point3dz(value);
    x = p3d.x;
    y = p3d.y;
    double z = p3d.z;
    p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
    p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
    p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
    lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
  }
  else
  {
    POINT2D p2d = datum_point2d(value);
    x = p2d.x;
    y = p2d.y;
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
 * Affine transform a temporal point.
 */
static TInstant *
tpointinst_affine(const TInstant *inst, const AFFINE *a)
{
  int srid = tpointinst_srid(inst);
  bool hasz = MOBDB_FLAGS_GET_Z(inst->flags);
  TInstant *result;
  tpointinst_affine1(inst, a, srid, hasz, &result);
  return result;
}

/**
 * Affine transform a temporal point.
 */
static TSequence *
tpointseq_affine(const TSequence *seq, const AFFINE *a)
{
  int srid = tpointseq_srid(seq);
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    tpointinst_affine1(inst, a, srid, hasz, &instants[i]);
  }
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * Affine transform a temporal point.
 *
 * @param[in] ss Temporal point
 * @param[in] a Affine transformation
 */
static TSequenceSet *
tpointseqset_affine(const TSequenceSet *ss, const AFFINE *a)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tpointseq_affine(tsequenceset_seq_n(ss, i), a);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * Affine transform a temporal point.
 */
static Temporal *
tpoint_affine(const Temporal *temp, const AFFINE *a)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
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
 * Stick a temporal point to the given grid specification.
 */
static TInstant *
tpointinst_grid(const TInstant *inst, const gridspec *grid)
{
  bool hasz = MOBDB_FLAGS_GET_Z(inst->flags);
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
 * Stick a temporal point to the given grid specification.
 */
static TSequence *
tpointseq_grid(const TSequence *seq, const gridspec *grid, bool filter_pts)
{
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  int srid = tpointseq_srid(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int k = 0;
  for (int i = 0; i < seq->count; i++)
  {
    POINT4D p, prev_p;
    const TInstant *inst = tsequence_inst_n(seq, i);
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
    instants[k++] = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
    lwpoint_free(lwpoint);
    pfree(gs);
    memcpy(&prev_p, &p, sizeof(POINT4D));
  }
  if (filter_pts && k == 1)
  {
    pfree_array((void **) instants, 1);
    return NULL;
  }

  /* Construct the result */
  return tsequence_make_free(instants, k, k, k > 1 ? seq->period.lower_inc : true,
    k > 1 ? seq->period.upper_inc : true, MOBDB_FLAGS_GET_INTERP(seq->flags),
    NORMALIZE);
}

/**
 * Stick a temporal point to the given grid specification.
 */
static TSequenceSet *
tpointseqset_grid(const TSequenceSet *ss, const gridspec *grid, bool filter_pts)
{
  int k = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tpointseq_grid(tsequenceset_seq_n(ss, i), grid, filter_pts);
    if (seq != NULL)
      sequences[k++] = seq;
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Stick a temporal point to the given grid specification.
 *
 * Only the x, y, and possible z dimensions are gridded, the timestamp is
 * kept unmodified. Two consecutive instants falling on the same grid cell
 * are collapsed into one single instant.
 */
static Temporal *
tpoint_grid(const Temporal *temp, const gridspec *grid, bool filter_pts)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
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
 * Transform a temporal point into vector tile coordinate space.
 *
 * @param[in] tpoint Temporal point
 * @param[in] box Geometric bounds of the tile contents without buffer
 * @param[in] extent Tile extent in tile coordinate space
 * @param[in] buffer Buffer distance in tile coordinate space
 * @param[in] clip_geom True if temporal point should be clipped
 */
static Temporal *
tpoint_mvt(const Temporal *tpoint, const STBOX *box, uint32_t extent,
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
  Temporal *tpoint2 = temporal_simplify(tpoint1, res, false);
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
  STBOX clip_box;
  stbox_set(NULL, true, false, false, srid, min, max, min, max, 0, 0,
    &clip_box);
  Temporal *tpoint5 = tpoint_at_stbox1(tpoint4, &clip_box, UPPER_INC);
  pfree(tpoint4);
  if (tpoint5 == NULL)
    return NULL;
  /* We need to grid again the result of the clipping */
  Temporal *result = tpoint_grid(tpoint5, &grid, true);
  pfree(tpoint5);
  return result;
}

/*****************************************************************************/

/**
 * Decouple the points and the timestamps of a temporal point.
 *
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] inst Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpointinst_decouple(const TInstant *inst, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64));
  times[0] = (inst->t / 1e6) + DELTA_UNIX_POSTGRES_EPOCH;
  *timesarr = times;
  *count = 1;
  return DatumGetGserializedP(tinstant_value_copy(inst));
}

/**
 * Decouple the points and the timestamps of a temporal point.
 *
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] seq Temporal point
 * @param[out] times Array of timestamps
 * @note The timestamps are returned in Unix epoch
 */
static LWGEOM *
tpointseq_decouple1(const TSequence *seq, int64 *times)
{
  /* General case */
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    Datum value = tinstant_value(inst);
    GSERIALIZED *gs = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gs);
    times[i] = (inst->t / 1e6) + DELTA_UNIX_POSTGRES_EPOCH;
  }
  LWGEOM *result = lwpointarr_make_trajectory(points, seq->count,
    MOBDB_FLAGS_GET_LINEAR(seq->flags) ? LINEAR : STEPWISE);
  for (int i = 0; i < seq->count; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points);
  return result;
}

/**
 * Decouple the points and the timestamps of a temporal point.
 *
 * @note The function does not remove consecutive points/instants that are equal.
 * @param[in] seq Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpointseq_decouple(const TSequence *seq, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64) * seq->count);
  LWGEOM *lwgeom = tpointseq_decouple1(seq, times);
  GSERIALIZED *result = geo_serialize(lwgeom);
  pfree(lwgeom);
  *timesarr = times;
  *count = seq->count;
  return result;
}

/**
 * Decouple the points and the timestamps of a temporal point.
 *
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
    return tpointseq_decouple(tsequenceset_seq_n(ss, 0), timesarr, count);

  /* General case */
  uint32_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->count);
  int64 *times = palloc(sizeof(int64) * ss->totalcount);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    geoms[i] = tpointseq_decouple1(seq, &times[k]);
    k += seq->count;
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
  /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  pfree(geoms);
  return result;
}

/**
 * Decouple the points and the timestamps of a temporal point.
 *
 * @param[in] temp Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 */
static GSERIALIZED *
tpoint_decouple(const Temporal *temp, int64 **timesarr, int *count)
{
  GSERIALIZED *result;
  ensure_valid_tempsubtype(temp->subtype);
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
tpoint_AsMVTGeom(const Temporal *temp, const STBOX *bounds, int32_t extent,
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
  STBOX box;
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
