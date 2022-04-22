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
 * @file tpoint_tempspatialrels.c
 * @brief Temporal spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal
 * Boolean.
 *
 * The following relationships are supported for a temporal geometry point
 * and a geometry: `tcontains`, `tdisjoint`, `tintersects`, `ttouches`, and
 * `tdwithin`.
 *
 * The following relationships are supported for two temporal geometry points:
 * `tdwithin`.
 *
 * The following relationships are supported for two temporal geography points:
 * `tdisjoint`, `tintersects`, `tdwithin`.
 *
 * tintersects and tdisjoint for a temporal point and a geometry allow a fast
 * implementation by (1) using bounding box tests, and (2) splitting temporal
 * sequence points into an array of simple (that is, not self-intersecting)
 * fragments and the answer is computed for each fragment without any
 * additional call to PostGIS.
 *
 * The implementation of tcontains and ttouches involving a temporal point
 * and a geometry is derived from the above by computing the boundary of the
 * geometry and
 * (1) tcontains(geo, tpoint) = tintersects(geo, tpoint) &
 *     ~ tintersects(st_boundary(geo), tpoint)
 *     where & and ~ are the temporal boolean operators and and not
 * (2) ttouches(geo, tpoint) = tintersects(st_boundary(geo), tpoint)
 *
 * Notice also that twithin has a custom implementation as follows
 * - In the case of a temporal point and a geometry we (1) call PostGIS to
 *   compute a buffer of the geometry and the distance parameter d, and
 *   (2) compute the result from tpointseq_at_geometry(seq, geo_buffer)
 * - In the case of two temporal points we need to compute the instants
 *   at which two temporal sequences have a distance d between each other,
 *   which amounts to solve the equation distance(seg1(t), seg2(t)) = d.
 */

#include "point/tpoint_tempspatialrels.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/tbool_boolops.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"

/*****************************************************************************
 * Generic functions for computing the temporal spatial relationships
 * with arbitrary geometries
 *****************************************************************************/

/*
 * Examples of values returned by PostGIS for the intersection
 * of a line and an arbitrary geometry

select st_astext(st_intersection(
geometry 'linestring(0 1,2 1)',
geometry 'polygon((0 0,1 1,2 0,0 0))'))
-- "POINT(1 1)"

select st_astext(st_intersection(
geometry 'linestring(0 1,4 1)',
geometry 'polygon((0 0,1 1,2 0.5,3 1,4 0,0 0))'))
-- "MULTIPOINT(1 1,3 1)"

select st_astext(st_intersection(
geometry 'linestring(0 1,2 1)',
geometry 'polygon((1 0,2 0,2 1,1 1,1 0))'))
-- "LINESTRING(1 1,2 1)"

select st_astext(st_intersection(
geometry 'linestring(0 2,5 2)',
geometry 'polygon((1 0,1 3,2 3,2 1,3 1,3 3,4 3,4 0,1 0))'))
-- "MULTILINESTRING((1 2,2 2),(3 2,4 2))"

select st_astext(st_intersection(
geometry 'linestring(0 1,4 1)',
geometry 'polygon((0 0,1 1,2 0.5,3 1,4 1,4 0,0 0))'))
-- "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(3 1,4 1))"
*/

/*****************************************************************************
 * tintersects and tdisjoint functions for temporal geometry points
 * The functions follow a similar approach as atGeometry functions to minimize
 * the number of calls to PostGIS in order to speed up the computation
 *****************************************************************************/

/**
 * @brief Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * @param[in] inst Temporal point
 * @param[in] geom Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be called
 */
TInstant *
tinterrel_tpointinst_geom(const TInstant *inst, Datum geom, bool tinter,
  Datum (*func)(Datum, Datum))
{
  /* Result depends on whether we are computing tintersects or tdisjoint */
  bool result = DatumGetBool(func(tinstant_value(inst), geom));
  /* For disjoint we need to invert the result */
  if (! tinter)
    result = ! result;
  return tinstant_make(BoolGetDatum(result), inst->t, T_TBOOL);
}

/**
 * @brief Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * @param[in] ti Temporal point
 * @param[in] geom Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be called
 */
TInstantSet *
tinterrel_tpointinstset_geom(const TInstantSet *ti, Datum geom, bool tinter,
  Datum (*func)(Datum, Datum))
{
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    bool result = DatumGetBool(func(tinstant_value(inst), geom));
    /* For disjoint we need to invert the result */
    if (! tinter)
      result = ! result;
    instants[i] = tinstant_make(BoolGetDatum(result), inst->t, T_TBOOL);
  }
  TInstantSet *result = tinstantset_make(instants, ti->count, MERGE_NO);
  pfree_array((void **) instants, ti->count);
  return result;
}

/**
 * Evaluates tintersects/tdisjoint for a temporal sequence point with step
 * interpolation and a geometry
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be called
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple, that is, non self-intersecting
 */
static TSequence **
tinterrel_tpointseq_step_geom(const TSequence *seq, Datum geom, bool tinter,
  Datum (*func)(Datum, Datum), int *count)
{
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);
  bool lower_inc1 = seq->period.lower_inc;
  int k = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst1 = tsequence_inst_n(seq, i);
    const TInstant *inst2 = (i < seq->count - 1) ?
      tsequence_inst_n(seq, i + 1) : NULL;
    /* If last instant exclusive upper bound */
    if (inst2 == NULL && ! seq->period.upper_inc)
      break;
    Datum datum_res = func(tinstant_value(inst1), geom);
    /* Result depends on whether we are computing tintersects or tdisjoint */
    if (! tinter)
      datum_res = BoolGetDatum(! DatumGetBool(datum_res));
    TInstant *instants[2];
    instants[0] = tinstant_make(datum_res, inst1->t, T_TBOOL);
    int l = 1;
    bool upper_inc1 = false;
    if (inst2 != NULL)
      instants[l++] = tinstant_make(datum_res, inst2->t, T_TBOOL);
    else
      upper_inc1 = true;
    result[k++] = tsequence_make((const TInstant **) instants, l,
      lower_inc1, upper_inc1, STEP, NORMALIZE_NO);
    pfree(instants[0]);
    if (inst2 != NULL)
      pfree(instants[1]);
    lower_inc1 = true;
    inst1 = inst2;
  }
  *count = k;
  return result;
}

/**
 * Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple, that is, non self-intersecting
 */
static TSequence **
tinterrel_tpointseq_simple_geom(const TSequence *seq, Datum geom, const STBOX *box,
  bool tinter, int *count)
{
  /* The temporal sequence has at least 2 instants since
   * (1) the instantaneous full sequence test is done in the calling function
   * (2) the simple components of a non self-intersecting sequence have at least
   *     two instants */
  assert(seq->count > 1);
  TSequence **result;
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Datum datum_yes = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum datum_no = tinter ? BoolGetDatum(false) : BoolGetDatum(true);

  /* Bounding box test */
  STBOX *box1 = tsequence_bbox_ptr(seq);
  if (! overlaps_stbox_stbox(box1, box))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base(datum_no, T_TBOOL,
      &seq->period, STEP);
    *count = 1;
    return result;
  }

  Datum traj = tpointseq_trajectory(seq);
  Datum inter = geom_intersection2d(traj, geom);
  GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
  if (gserialized_is_empty(gsinter))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base(datum_no, T_TBOOL,
      &seq->period, STEP);
    pfree(DatumGetPointer(inter));
    *count = 1;
    return result;
  }

  const TInstant *start = tsequence_inst_n(seq, 0);
  const TInstant *end = tsequence_inst_n(seq, seq->count - 1);
  /* If the trajectory is a point the result is true due to the
   * non-empty intersection test above */
  if (seq->count == 2 &&
    datum_point_eq(tinstant_value(start), tinstant_value(end)))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base(datum_yes, T_TBOOL,
      &seq->period, STEP);
    PG_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
    pfree(DatumGetPointer(inter));
    *count = 1;
    return result;
  }

  /* Get the periods at which the temporal point intersects the geometry */
  int countper;
  Period **periods = tpointseq_interperiods(seq, gsinter, &countper);
  if (countper == 0)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base(datum_no, T_TBOOL,
      &seq->period, STEP);
    pfree(DatumGetPointer(gsinter));
    *count = 1;
    return result;
  }
  PeriodSet *ps;
  if (countper == 1)
    ps = minus_period_period(&seq->period, periods[0]);
  else
  {
    /* It is necessary to sort the periods */
    periodarr_sort(periods, countper);
    PeriodSet *ps1 = periodset_make((const Period **) periods, countper, NORMALIZE);
    ps = minus_period_periodset(&seq->period, ps1);
    pfree(ps1);
  }
  int newcount = countper;
  if (ps != NULL)
    newcount += ps->count;
  result = palloc(sizeof(TSequence *) * newcount);
  for (int i = 0; i < countper; i++)
    result[i] = tsequence_from_base(datum_yes, T_TBOOL,
      periods[i], STEP);
  if (ps != NULL)
  {
    for (int i = 0; i < ps->count; i++)
    {
      const Period *p = periodset_per_n(ps, i);
      result[i + countper] = tsequence_from_base(datum_no, T_TBOOL,
        p, STEP);
    }
    tseqarr_sort(result, newcount);
    pfree(ps);
  }
  pfree_array((void **) periods, countper);
  *count = newcount;
  return result;
}

/**
 * Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * The function splits the temporal point in an array of temporal point
 * sequences that are simple (that is, not self-intersecting) and loops
 * for each piece.
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used for instantaneous sequences
 * @param[out] count Number of elements in the output array
 */
static TSequence **
tinterrel_tpointseq_geom1(const TSequence *seq, Datum geom, const STBOX *box,
  bool tinter, Datum (*func)(Datum, Datum), int *count)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tinterrel_tpointinst_geom(tsequence_inst_n(seq, 0),
      geom, tinter, func);
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tinstant_tsequence(inst, STEP);
    pfree(inst);
    *count = 1;
    return result;
  }

  /* Step interpolation */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
    return tinterrel_tpointseq_step_geom(seq, geom, tinter, func, count);

  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int newcount;
  TSequence **simpleseqs = tpointseq_make_simple(seq, &newcount);
  TSequence ***sequences = palloc(sizeof(TSequence *) * newcount);
  /* palloc0 used due to initialize the counters to 0 */
  int *countseqs = palloc0(sizeof(int) * newcount);
  int totalcount = 0;
  for (int i = 0; i < newcount; i++)
  {
    sequences[i] = tinterrel_tpointseq_simple_geom(simpleseqs[i], geom, box,
      tinter, &countseqs[i]);
    totalcount += countseqs[i];
  }
  *count = totalcount;
  return tseqarr2_to_tseqarr(sequences, countseqs,
    newcount, totalcount);
}

/**
 * @brief Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * The function splits the temporal point in an array of temporal point
 * sequences that are simple (that is, not self-intersecting) and loops
 * for each piece.
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] func PostGIS function to be used for instantaneous sequences
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 */
TSequenceSet *
tinterrel_tpointseq_geom(const TSequence *seq, Datum geom, const STBOX *box,
  bool tinter, Datum (*func)(Datum, Datum))
{
  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int count;
  TSequence **sequences = tinterrel_tpointseq_geom1(seq, geom, box, tinter,
    func, &count);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * @param[in] ts Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used for instantaneous sequences
 */
TSequenceSet *
tinterrel_tpointseqset_geom(const TSequenceSet *ts, Datum geom,
  const STBOX *box, bool tinter, Datum (*func)(Datum, Datum))
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tinterrel_tpointseq_geom(tsequenceset_seq_n(ts, 0), geom, box,
      tinter, func);

  TSequence ***sequences = palloc(sizeof(TSequence *) * ts->count);
  /* palloc0 used to initizalize the counters to 0 */
  int *countseqs = palloc0(sizeof(int) * ts->count);
  int totalcount = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tinterrel_tpointseq_geom1(seq, geom, box, tinter, func,
        &countseqs[i]);
    totalcount += countseqs[i];
  }
  TSequence **allseqs = tseqarr2_to_tseqarr(sequences, countseqs, ts->count,
    totalcount);
  return tsequenceset_make_free(allseqs, totalcount, NORMALIZE);
}

/**
 * @brief Evaluates tintersects/tdisjoint for a temporal point and a geometry.
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] restr True when the atValue function is applied to the result
 * @param[in] atvalue Value to be use for the atValue function
 * @pre The geometry is NOT empty. This should be ensured by the calling
 * function
 * @note 3D is not supported because there is no 3D intersection function
 * provided by PostGIS
 */
Temporal *
tinterrel_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool tinter,
  bool restr, Datum atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z(temp->flags); ensure_has_not_Z_gs(gs);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Datum datum_no = tinter ? BoolGetDatum(false) : BoolGetDatum(true);

  /* Bounding box test */
  STBOX box1, box2;
  temporal_bbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_stbox(gs, &box2);
  if (!overlaps_stbox_stbox(&box1, &box2))
    return temporal_from_base(temp, datum_no, T_TBOOL, STEP);

  /* 3D only if both arguments are 3D */
  Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) &&
    FLAGS_GET_Z(GS_FLAGS(gs)) ? &geom_intersects3d : &geom_intersects2d;

  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinterrel_tpointinst_geom((TInstant *) temp,
      PointerGetDatum(gs), tinter, func);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinterrel_tpointinstset_geom((TInstantSet *) temp,
      PointerGetDatum(gs), tinter, func);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tinterrel_tpointseq_geom((TSequence *) temp,
      PointerGetDatum(gs), &box2, tinter, func);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tinterrel_tpointseqset_geom((TSequenceSet *) temp,
      PointerGetDatum(gs), &box2, tinter, func);
  /* Restrict the result to the Boolean value in the third argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  return result;
}

/*****************************************************************************
 * Functions to compute the tdwithin relationship between temporal sequences.
 * This requires to determine the instants t1 and t2 at which two temporal
 * sequences have a distance d between each other. This amounts to solve the
 * equation
 *     distance(seg1(t), seg2(t)) = d
 * The function assumes that the two segments are synchronized,
 * that they are not instants, and that they are not both constant.
 *
 * Possible cases
 *
 * Parallel (a == 0) within distance

SELECT tdwithin(
tgeompoint '[POINT(0 1)@2000-01-01, POINT(1 2)@2000-01-02]',
tgeompoint '[POINT(0 0)@2000-01-01, POINT(1 1)@2000-01-02]', 1)
-- "{[t@2000-01-01, t@2000-01-02]}"

  * Parallel (a == 0) but not within distance

SELECT tdwithin(
tgeompoint '[POINT(0 2)@2000-01-01, POINT(1 3)@2000-01-02]',
tgeompoint '[POINT(0 0)@2000-01-01, POINT(1 1)@2000-01-02]', 1)
-- "{[f@2000-01-01, f@2000-01-02]}"

 * No solution (root < 0)

SELECT tdwithin(
tgeompoint '[POINT(2 3)@2000-01-01, POINT(3 4)@2000-01-03]',
tgeompoint '[POINT(4 4)@2000-01-01, POINT(6 2)@2000-01-03]', 1)
-- "{[f@2000-01-01, f@2000-01-03]}"

 * One solution (root == 0)
   - solution within segment

SELECT tdwithin(
tgeompoint '[POINT(2 2)@2000-01-01, POINT(1 1)@2000-01-03]',
tgeompoint '[POINT(3 1)@2000-01-01, POINT(2 2)@2000-01-03]', 1)
-- "{[f@2000-01-01, t@2000-01-02], (f@2000-01-02, f@2000-01-03]}"

   - solution outside to segment

SELECT tdwithin(
tgeompoint '[POINT(3 3)@2000-01-01, POINT(2 2)@2000-01-03]',
tgeompoint '[POINT(4 0)@2000-01-01, POINT(3 1)@2000-01-03]', 1)
-- "{[f@2000-01-01, f@2000-01-03]}"

 * Two solutions (root > 0)
 - segments contains solution period

SELECT tdwithin(
tgeompoint '[POINT(1 1)@2000-01-01, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(1 3)@2000-01-01, POINT(5 3)@2000-01-05]', 1)
-- "{[f@2000-01-01, t@2000-01-02, t@2000-01-04], (f@2000-01-04, f@2000-01-05]}"

  - solution period contains segment

SELECT tdwithin(
tgeompoint '[POINT(2.5 2.5)@2000-01-02 12:00, POINT(3.5 3.5)@2000-01-05 12:00]',
tgeompoint '[POINT(2.5 3.0)@2000-01-02 12:00, POINT(3.5 3.0)@2000-01-03 12:00]', 1)
-- "{[t@2000-01-02 12:00:00+00, t@2000-01-03 12:00:00+00]}"

  - solution period overlaps to the left segment

SELECT tdwithin(
tgeompoint '[POINT(3 3)@2000-01-03, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(3 3)@2000-01-03, POINT(5 3)@2000-01-05]', 1)
-- "{[t@2000-01-03, f@2000-01-04, f@2000-01-05]}"

  - solution period overlaps to the right segment

SELECT tdwithin(
tgeompoint '[POINT(1 1)@2000-01-01, POINT(3 3)@2000-01-03]',
tgeompoint '[POINT(1 3)@2000-01-01, POINT(3 3)@2000-01-03]', 1)
-- "{[f@2000-01-01, t@2000-01-02, t@2000-01-03]}"

  - solution period intersects at an instant with the segment

SELECT tdwithin(
tgeompoint '[POINT(4 4)@2000-01-04, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(4 3)@2000-01-04, POINT(5 3)@2000-01-05]', 1)
-- "{[t@2000-01-04], (f@2000-01-04, f@2000-01-05]}"

 *****************************************************************************/

/**
 * Return the timestamps at which the segments of the two temporal points
 * are within the given distance
 *
 * @param[in] sv1,ev1 Points defining the first segment
 * @param[in] sv2,ev2 Points defining the second segment
 * @param[in] lower,upper Timestamps associated to the segments
 * @param[in] dist Distance
 * @param[in] hasz True for 3D segments
 * @param[in] func Distance function (2D or 3D)
 * @param[out] t1,t2 Resulting timestamps
 * @result Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
int
tdwithin_tpointsegm_tpointsegm(Datum sv1, Datum ev1, Datum sv2, Datum ev2,
  TimestampTz lower, TimestampTz upper, double dist, bool hasz,
  datum_func3 func, TimestampTz *t1, TimestampTz *t2)
{
  /* To reduce problems related to floating point arithmetic, lower and upper
   * are shifted, respectively, to 0 and 1 before computing the solutions
   * of the quadratic equation */
  double duration = upper - lower;
  long double a, b, c;
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = datum_point3dz_p(sv1);
    const POINT3DZ *p2 = datum_point3dz_p(ev1);
    const POINT3DZ *p3 = datum_point3dz_p(sv2);
    const POINT3DZ *p4 = datum_point3dz_p(ev2);

    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2
    * z(t) = a3 * t + c3 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    double a3 = (p2->z - p1->z);
    double c3 = p1->z;

    /* per2 functions
     * x(t) = a4 * t + c4
     * y(t) = a5 * t + c5
     * z(t) = a6 * t + c6 */
    double a4 = (p4->x - p3->x);
    double c4 = p3->x;
    double a5 = (p4->y - p3->y);
    double c5 = p3->y;
    double a6 = (p4->z - p3->z);
    double c6 = p3->z;

    /* compute the distance function */
    double a_x = (a1 - a4) * (a1 - a4);
    double a_y = (a2 - a5) * (a2 - a5);
    double a_z = (a3 - a6) * (a3 - a6);
    double b_x = 2 * (a1 - a4) * (c1 - c4);
    double b_y = 2 * (a2 - a5) * (c2 - c5);
    double b_z = 2 * (a3 - a6) * (c3 - c6);
    double c_x = (c1 - c4) * (c1 - c4);
    double c_y = (c2 - c5) * (c2 - c5);
    double c_z = (c3 - c6) * (c3 - c6);
    /* distance function = dist */
    a = a_x + a_y + a_z;
    b = b_x + b_y + b_z;
    c = c_x + c_y + c_z - (dist * dist);
  }
  else /* 2D */
  {
    const POINT2D *p1 = datum_point2d_p(sv1);
    const POINT2D *p2 = datum_point2d_p(ev1);
    const POINT2D *p3 = datum_point2d_p(sv2);
    const POINT2D *p4 = datum_point2d_p(ev2);
    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    /* per2 functions
     * x(t) = a3 * t + c3
     * y(t) = a4 * t + c4 */
    double a3 = (p4->x - p3->x);
    double c3 = p3->x;
    double a4 = (p4->y - p3->y);
    double c4 = p3->y;
    /* compute the distance function */
    double a_x = (a1 - a3) * (a1 - a3);
    double a_y = (a2 - a4) * (a2 - a4);
    double b_x = 2 * (a1 - a3) * (c1 - c3);
    double b_y = 2 * (a2 - a4) * (c2 - c4);
    double c_x = (c1 - c3) * (c1 - c3);
    double c_y = (c2 - c4) * (c2 - c4);
    /* distance function = dist */
    a = a_x + a_y;
    b = b_x + b_y;
    c = c_x + c_y - (dist * dist);
  }
  /* They are parallel, moving in the same direction at the same speed */
  if (a == 0)
  {
    if (!func(sv1, sv2, Float8GetDatum(dist)))
      return 0;
    *t1 = lower;
    *t2 = upper;
    return 2;
  }
  /* Solving the quadratic equation for distance = dist */
  long double discriminant = b * b - 4 * a * c;

  /* One solution */
  if (discriminant == 0)
  {
    long double t5 = (-1 * b) / (2 * a);
    if (t5 < 0.0 || t5 > 1.0)
      return 0;
    *t1 = *t2 = lower + (TimestampTz) (t5 * duration);
    return 1;
  }
  /* No solution */
  if (discriminant < 0)
    return 0;
  else
  /* At most two solutions depending on whether they are within the time interval */
  {
    /* Apply a mixture of quadratic formula and Viète formula to improve precision */
    long double t5, t6;
    if (b >= 0)
    {
      t5 = (-1 * b - sqrtl(discriminant)) / (2 * a);
      t6 = (2 * c ) / (-1 * b - sqrtl(discriminant));
    }
    else
    {
      t5 = (2 * c ) / (-1 * b + sqrtl(discriminant));
      t6 = (-1 * b + sqrtl(discriminant)) / (2 * a);
    }

    /* If the two intervals do not intersect */
    if (0.0 > t6 || t5 > 1.0)
      return 0;
    /* Compute the intersection of the two intervals */
    long double t7 = Max(0.0, t5);
    long double t8 = Min(1.0, t6);
    if (fabsl(t7 - t8) < MOBDB_EPSILON)
    {
      *t1 = *t2 = lower + (TimestampTz) (t7 * duration);
      return 1;
    }
    else
    {
      *t1 = lower + (TimestampTz) (t7 * duration);
      *t2 = lower + (TimestampTz) (t8 * duration);
      return 2;
    }
  }
}

/**
 * Return the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @result Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tpointseq_tpointseq2(const TSequence *seq1, const TSequence *seq2,
  Datum dist, datum_func3 func, TSequence **result)
{
  const TInstant *start1 = tsequence_inst_n(seq1, 0);
  const TInstant *start2 = tsequence_inst_n(seq2, 0);
  if (seq1->count == 1)
  {
    TInstant *inst = tinstant_make(func(tinstant_value(start1),
      tinstant_value(start2), dist), start1->t, T_TBOOL);
    result[0] = tinstant_tsequence(inst, STEP);
    pfree(inst);
    return 1;
  }

  int k = 0;
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(seq1->flags);
  Datum sv1 = tinstant_value(start1);
  Datum sv2 = tinstant_value(start2);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  const Datum datum_true = BoolGetDatum(true);
  const Datum datum_false = BoolGetDatum(false);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, lower, T_TBOOL);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  double dist_d = DatumGetFloat8(dist);
  for (int i = 1; i < seq1->count; i++)
  {
    /* Each iteration of the for loop adds between one and three sequences */
    const TInstant *end1 = tsequence_inst_n(seq1, i);
    const TInstant *end2 = tsequence_inst_n(seq2, i);
    Datum ev1 = tinstant_value(end1);
    Datum ev2 = tinstant_value(end2);
    TimestampTz upper = end1->t;
    bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

    /* Both segments are constant or have step interpolation */
    if ((datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2)) ||
      (! linear1 && ! linear2))
    {
      Datum value = func(sv1, sv2, dist);
      tinstant_set(instants[0], value, lower);
      if (! linear1 && ! linear2 && upper_inc)
      {
        Datum value1 = func(ev1, ev2, dist);
        tinstant_set(instants[1], value1, upper);
      }
      else
        tinstant_set(instants[1], value, upper);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin function is true */
      TimestampTz t1, t2;
      Datum sev1 = linear1 ? ev1 : sv1;
      Datum sev2 = linear2 ? ev2 : sv2;
      int solutions = tdwithin_tpointsegm_tpointsegm(sv1, sev1, sv2, sev2,
        lower, upper, dist_d, hasz, func, &t1, &t2);

      /* <  F  > */
      bool upper_inc1 = linear1 && linear2 && upper_inc;
      if (solutions == 0 ||
      (solutions == 1 && ((t1 == lower && !lower_inc) ||
        (t1 == upper && !upper_inc))))
      {
        tinstant_set(instants[0], datum_false, lower);
        tinstant_set(instants[1], datum_false, upper);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, upper_inc1, STEP, NORMALIZE_NO);
      }
      /*
       *  <  T  >               2 solutions, lower == t1, upper == t2
       *  [T](  F  )            1 solution, lower == t1 (t1 == t2)
       *  [T  T](  F  )         2 solutions, lower == t1, upper != t2
       *  (  F  )[T]            1 solution && upper == t1, (t1 == t2)
       *  (  F  )[T](  F  )     1 solution, lower != t1 (t1 == t2)
       *  (  F  )[T  T]         2 solutions, lower != t1, upper == t2
       *  (  F  )[T  T](  F  )  2 solutions, lower != t1, upper != t2
       */
      else
      {
        int j = 0;
        if (t1 != lower)
          tinstant_set(instants[j++], datum_false, lower);
        tinstant_set(instants[j++], datum_true, t1);
        if (solutions == 2 && t1 != t2)
          tinstant_set(instants[j++], datum_true, t2);
        result[k++] = tsequence_make((const TInstant **) instants, j, lower_inc,
          (t2 != upper) ? true : upper_inc1, STEP, NORMALIZE_NO);
        if (t2 != upper)
        {
          tinstant_set(instants[0], datum_false, t2);
          tinstant_set(instants[1], datum_false, upper);
          result[k++] = tsequence_make((const TInstant **) instants, 2, false,
            upper_inc1, STEP, NORMALIZE_NO);
        }
      }
      /* Add extra final point if only one segment is linear */
      if (upper_inc && (! linear1 || ! linear2))
      {
        Datum value = func(ev1, ev2, dist);
        tinstant_set(instants[0], value, upper);
        result[k++] = tinstant_tsequence(instants[0], STEP);
      }
    }
    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return k;
}

/**
 * Return the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseq_tpointseq(const TSequence *seq1, const TSequence *seq2,
  Datum dist, datum_func3 func)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq1->count * 4);
  int count = tdwithin_tpointseq_tpointseq2(seq1, seq2, dist, func, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[in] ts1,ts2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseqset_tpointseqset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, Datum dist, datum_func3 func)
{
  /* Singleton sequence set */
  if (ts1->count == 1)
    return tdwithin_tpointseq_tpointseq(tsequenceset_seq_n(ts1, 0),
      tsequenceset_seq_n(ts2, 0), dist, func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ts1->totalcount * 4);
  int k = 0;
  for (int i = 0; i < ts1->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, i);
    k += tdwithin_tpointseq_tpointseq2(seq1, seq2, dist, func, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************/

/**
 * Return the timestamps at which a temporal point segment and a point
 * are within the given distance
 *
 * @param[in] start,end Points defining the segment
 * @param[in] point Point
 * @param[in] lower,upper Timestamps associated to the segment
 * @param[in] dist Distance
 * @param[in] hasz True for 3D segments
 * @param[out] t1,t2 Resulting timestamps
 * @result Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
static int
tdwithin_tpointsegm_point(Datum start, Datum end, Datum point,
  TimestampTz lower, TimestampTz upper, double dist, bool hasz,
  TimestampTz *t1, TimestampTz *t2)
{
  /* To reduce problems related to floating point arithmetic, lower and upper
   * are shifted, respectively, to 0 and 1 before computing the solutions
   * of the quadratic equation */
  double duration = upper - lower;
  long double a, b, c;
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = datum_point3dz_p(start);
    const POINT3DZ *p2 = datum_point3dz_p(end);
    const POINT3DZ *p3 = datum_point3dz_p(point);

    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2
    * z(t) = a3 * t + c3 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    double a3 = (p2->z - p1->z);
    double c3 = p1->z;

    /* per2 functions
     * x(t) = c4
     * y(t) = c5
     * z(t) = c6 */
    double c4 = p3->x;
    double c5 = p3->y;
    double c6 = p3->z;

    /* compute the distance function */
    double a_x = a1 * a1;
    double a_y = a2 * a2;
    double a_z = a3 * a3;
    double b_x = 2 * a1 * (c1 - c4);
    double b_y = 2 * a2 * (c2 - c5);
    double b_z = 2 * a3 * (c3 - c6);
    double c_x = (c1 - c4) * (c1 - c4);
    double c_y = (c2 - c5) * (c2 - c5);
    double c_z = (c3 - c6) * (c3 - c6);
    /* distance function = dist */
    a = a_x + a_y + a_z;
    b = b_x + b_y + b_z;
    c = c_x + c_y + c_z - (dist * dist);
  }
  else /* 2D */
  {
    const POINT2D *p1 = datum_point2d_p(start);
    const POINT2D *p2 = datum_point2d_p(end);
    const POINT2D *p3 = datum_point2d_p(point);
    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    /* per2 functions
     * x(t) = c3
     * y(t) = c4 */
    double c3 = p3->x;
    double c4 = p3->y;
    /* compute the distance function */
    double a_x = a1 * a1;
    double a_y = a2 * a2;
    double b_x = 2 * a1 * (c1 - c3);
    double b_y = 2 * a2 * (c2 - c4);
    double c_x = (c1 - c3) * (c1 - c3);
    double c_y = (c2 - c4) * (c2 - c4);
    /* distance function = dist */
    a = a_x + a_y;
    b = b_x + b_y;
    c = c_x + c_y - (dist * dist);
  }
  /* Solving the quadratic equation for distance = dist */
  assert(a != 0);
  long double discriminant = b * b - 4 * a * c;

  /* One solution */
  if (discriminant == 0)
  {
    long double t5 = (-1 * b) / (2 * a);
    if (t5 < 0.0 || t5 > 1.0)
      return 0;
    *t1 = *t2 = lower + (TimestampTz) (t5 * duration);
    return 1;
  }
  /* No solution */
  if (discriminant < 0)
    return 0;
  else
  /* At most two solutions depending on whether they are within the time interval */
  {
    /* Apply a mixture of quadratic formula and Viète formula to improve precision */
    long double t5, t6;
    if (b >= 0)
    {
      t5 = (-1 * b - sqrtl(discriminant)) / (2 * a);
      t6 = (2 * c ) / (-1 * b - sqrtl(discriminant));
    }
    else
    {
      t5 = (2 * c ) / (-1 * b + sqrtl(discriminant));
      t6 = (-1 * b + sqrtl(discriminant)) / (2 * a);
    }

    /* If the two intervals do not intersect */
    if (0.0 > t6 || t5 > 1.0)
      return 0;
    /* Compute the intersection of the two intervals */
    long double t7 = Max(0.0, t5);
    long double t8 = Min(1.0, t6);
    if (fabsl(t7 - t8) < MOBDB_EPSILON)
    {
      *t1 = *t2 = lower + (TimestampTz) (t7 * duration);
      return 1;
    }
    else
    {
      *t1 = lower + (TimestampTz) (t7 * duration);
      *t2 = lower + (TimestampTz) (t8 * duration);
      return 2;
    }
  }
}

/**
 * Return the timestamps at which a temporal point and a point are
 * within the given distance
 *
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @result Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tpointseq_point1(const TSequence *seq, Datum point, Datum dist,
  datum_func3 func, TSequence **result)
{
  const TInstant *start = tsequence_inst_n(seq, 0);
  Datum sv = tinstant_value(start);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(func(sv, point, dist), start->t, T_TBOOL);
    result[0] = tinstant_tsequence(inst, STEP);
    pfree(inst);
    return 1;
  }

  int k = 0;
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  TimestampTz lower = start->t;
  bool lower_inc = seq->period.lower_inc;
  const Datum datum_true = BoolGetDatum(true);
  const Datum datum_false = BoolGetDatum(false);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, lower, T_TBOOL);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  double dist_d = DatumGetFloat8(dist);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the for loop adds between one and three sequences */
    const TInstant *end = tsequence_inst_n(seq, i);
    Datum ev = tinstant_value(end);
    TimestampTz upper = end->t;
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;

    /* Segment is constant or has step interpolation */
    if (datum_point_eq(sv, ev) || ! linear)
    {
      Datum value = func(sv, point, dist);
      tinstant_set(instants[0], value, lower);
      if (! linear && upper_inc)
      {
        Datum value1 = func(ev, point, dist);
        tinstant_set(instants[1], value1, upper);
      }
      else
        tinstant_set(instants[1], value, upper);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      int solutions = tdwithin_tpointsegm_point(sv, ev, point,
        lower, upper, dist_d, hasz, &t1, &t2);

      /* <  F  > */
      bool upper_inc1 = linear && upper_inc;
      if (solutions == 0 ||
      (solutions == 1 && ((t1 == lower && !lower_inc) ||
        (t1 == upper && !upper_inc))))
      {
        tinstant_set(instants[0], datum_false, lower);
        tinstant_set(instants[1], datum_false, upper);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, upper_inc1, STEP, NORMALIZE_NO);
      }
      /*
       *  <  T  >               2 solutions, lower == t1, upper == t2
       *  [T](  F  )            1 solution, lower == t1 (t1 == t2)
       *  [T  T](  F  )         2 solutions, lower == t1, upper != t2
       *  (  F  )[T]            1 solution && upper == t1, (t1 == t2)
       *  (  F  )[T](  F  )     1 solution, lower != t1 (t1 == t2)
       *  (  F  )[T  T]         2 solutions, lower != t1, upper == t2
       *  (  F  )[T  T](  F  )  2 solutions, lower != t1, upper != t2
       */
      else
      {
        int j = 0;
        if (t1 != lower)
          tinstant_set(instants[j++], datum_false, lower);
        tinstant_set(instants[j++], datum_true, t1);
        if (solutions == 2 && t1 != t2)
          tinstant_set(instants[j++], datum_true, t2);
        result[k++] = tsequence_make((const TInstant **) instants, j, lower_inc,
          (t2 != upper) ? true : upper_inc1, STEP, NORMALIZE_NO);
        if (t2 != upper)
        {
          tinstant_set(instants[0], datum_false, t2);
          tinstant_set(instants[1], datum_false, upper);
          result[k++] = tsequence_make((const TInstant **) instants, 2, false,
            upper_inc1, STEP, NORMALIZE_NO);
        }
      }
    }
    start = end;
    sv = ev;
    lower = upper;
    lower_inc = true;
  }
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return k;
}

/**
 * Return the timestamps at which the a temporal point and a point are
 * within the given distance
 *
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseq_point(const TSequence *seq, Datum point, Datum dist,
  datum_func3 func)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * 4);
  int count = tdwithin_tpointseq_point1(seq, point, dist, func, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return the timestamps at which a temporal point and a point are
 * within the given distance
 *
 * @param[in] ts Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 */
static TSequenceSet *
tdwithin_tpointseqset_point(const TSequenceSet *ts, Datum point, Datum dist,
  datum_func3 func)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tdwithin_tpointseq_point(tsequenceset_seq_n(ts, 0), point, dist,
      func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount * 4);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tdwithin_tpointseq_point1(seq, point, dist, func, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return the temporal contains relationship between the geometry and
 * the temporal point
 */
Temporal *
tcontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  Datum atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  Temporal *inter = tinterrel_tpoint_geo(temp, gs, TINTERSECTS, restr,
    atvalue);
  Datum bound = call_function1(boundary, PointerGetDatum(gs));
  GSERIALIZED *gsbound = (GSERIALIZED *) PG_DETOAST_DATUM(bound);
  Temporal *result;
  if (! gserialized_is_empty(gsbound))
  {
    Temporal *inter_bound = tinterrel_tpoint_geo(temp, gsbound,
      TINTERSECTS, restr, atvalue);
    Temporal *not_inter_bound = tnot_tbool(inter_bound);
    result = boolop_tbool_tbool(inter, not_inter_bound, &datum_and);
    pfree(inter);
    PG_FREE_IF_COPY_P(gsbound, DatumGetPointer(bound));
    pfree(DatumGetPointer(bound));
    pfree(inter_bound);
    pfree(not_inter_bound);
  }
  else
    result = inter;
  /* Restrict the result to the Boolean value in the third argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  return result;
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return the temporal touches relationship between the geometry and the
 * temporal point (internal version)
 */
Temporal *
ttouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, Datum atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z(temp->flags); ensure_has_not_Z_gs(gs);
  Datum bound = call_function1(boundary, PointerGetDatum(gs));
  GSERIALIZED *gsbound = (GSERIALIZED *) PG_DETOAST_DATUM(bound);
  Temporal *result;
  if (! gserialized_is_empty(gsbound))
  {
    result = tinterrel_tpoint_geo(temp, gsbound, TINTERSECTS, restr,
      atvalue);
    PG_FREE_IF_COPY_P(gsbound, DatumGetPointer(bound));
    pfree(DatumGetPointer(bound));
  }
  else
    result = temporal_from_base(temp, BoolGetDatum(false), T_TBOOL, STEP);
  /* Restrict the result to the Boolean value in the third argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  return result;
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return a temporal Boolean that states whether the temporal point and
 * the geometry are within the given distance.
 */
Temporal *
tdwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, Datum dist,
  bool restr, Datum atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  datum_func3 func =
    /* 3D only if both arguments are 3D */
    MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(GS_FLAGS(gs)) ?
    &geom_dwithin3d : &geom_dwithin2d;
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = dist;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = INVERT_NO;
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant_base((TInstant *) temp,
      PointerGetDatum(gs), &lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset_base((TInstantSet *) temp,
      PointerGetDatum(gs), &lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tdwithin_tpointseq_point((TSequence *) temp,
      PointerGetDatum(gs), dist, func);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tdwithin_tpointseqset_point((TSequenceSet *) temp,
      PointerGetDatum(gs), dist, func);
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return a temporal Boolean that states whether the temporal points
 * are within the given distance.
 */
Temporal *
tdwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  Datum dist, bool restr, Datum atvalue)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  Temporal *sync1, *sync2;
  /* Return false if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  datum_func3 func = get_dwithin_fn(temp1->flags, temp2->flags);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = dist;
  lfinfo.restype = T_TBOOL;
  Temporal *result;
  ensure_valid_tempsubtype(sync1->subtype);
  if (sync1->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant_tinstant(
      (TInstant *) sync1, (TInstant *) sync2, &lfinfo);
  else if (sync1->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset_tinstantset(
      (TInstantSet *) sync1, (TInstantSet *) sync2, &lfinfo);
  else if (sync1->subtype == SEQUENCE)
    result = (Temporal *) tdwithin_tpointseq_tpointseq(
      (TSequence *) sync1, (TSequence *) sync2, dist, func);
  else /* sync1->subtype == SEQUENCESET */
    result = (Temporal *) tdwithin_tpointseqset_tpointseqset(
      (TSequenceSet *) sync1, (TSequenceSet *) sync2, dist, func);
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Generic functions for computing the temporal spatial relationships
 * with arbitrary geometries
 *****************************************************************************/

/**
 * Return the temporal disjoint/intersection relationship between the geometry
 * and the temporal point
 */
static Datum
tinterrel_geo_tpoint_ext(FunctionCallInfo fcinfo, bool tinter)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Return the temporal disjoint/intersection relationship between the geometry
 * and the temporal point
 */
static Datum
tinterrel_tpoint_geo_ext(FunctionCallInfo fcinfo, bool tinter)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, tinter, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tcontains_geo_tpoint);
/**
 * Return the temporal contains relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
Tcontains_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tcontains_geo_tpoint(gs, temp, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tdisjoint_geo_tpoint);
/**
 * Return the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
Tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_geo_tpoint_ext(fcinfo, TDISJOINT);
}

PG_FUNCTION_INFO_V1(Tdisjoint_tpoint_geo);
/**
 * Return the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
Tdisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  return tinterrel_tpoint_geo_ext(fcinfo, TDISJOINT);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tintersects_geo_tpoint);
/**
 * Return the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
Tintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_geo_tpoint_ext(fcinfo, TINTERSECTS);
}

PG_FUNCTION_INFO_V1(Tintersects_tpoint_geo);
/**
 * Return the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
Tintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  return tinterrel_tpoint_geo_ext(fcinfo, TINTERSECTS);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Ttouches_geo_tpoint);
/**
 * Return the temporal touches relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
Ttouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Ttouches_tpoint_geo);
/**
 * Return the temporal touches relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
Ttouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tdwithin_geo_tpoint);
/**
 * Return a temporal Boolean that states whether the geometry and the
 * temporal point are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdwithin_tpoint_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdwithin_tpoint_geo);
/**
 * Return a temporal Boolean that states whether the temporal point and
 * the geometry are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdwithin_tpoint_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tdwithin_tpoint_tpoint);
/**
 * Return a temporal Boolean that states whether the temporal points
 * are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdwithin_tpoint_tpoint(temp1, temp2, dist,
    restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
