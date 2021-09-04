/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * Temporal spatial relationships for temporal points.
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

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "general/period.h"
#include "general/periodset.h"
#include "general/timeops.h"
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
 * tintersects and tisjoint functions for temporal geometry points
 * The functions follow a similar approach as atGeometry functions to minimize
 * the number of calls to PostGIS in order to speed up the computation
 *****************************************************************************/

/**
 * Evaluates tintersects/tdisjoint for a temporal point and a geometry
 *
 * @param[in] inst Temporal point
 * @param[in] geom Geometry
 * @param[in] func PostGIS function to be called
 */
static TInstant *
tinterrel_tpointinst_geom(const TInstant *inst, Datum geom,
  Datum (*func)(Datum, Datum))
{
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Datum datum_res = func(tinstant_value(inst), geom);
  return tinstant_make(datum_res, inst->t, BOOLOID);
}

/**
 * Evaluates tintersects/tdisjoint for a temporal point and a geometry
 *
 * @param[in] ti Temporal point
 * @param[in] geom Geometry
 * @param[in] func PostGIS function to be called
 */
static TInstantSet *
tinterrel_tpointinstset_geom(const TInstantSet *ti, Datum geom,
  Datum (*func)(Datum, Datum))
{
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    Datum datum_res = func(tinstant_value(inst), geom);
    instants[i] = tinstant_make(datum_res, inst->t, BOOLOID);
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
 * @param[in] func PostGIS function to be called
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple, that is, non self-intersecting
 */
static TSequence **
tinterrel_tpointseq_step_geom(const TSequence *seq, Datum geom,
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
    TInstant *instants[2];
    instants[0] = tinstant_make(datum_res, inst1->t, BOOLOID);
    int l = 1;
    bool upper_inc1 = false;
    if (inst2 != NULL)
      instants[l++] = tinstant_make(datum_res, inst2->t, BOOLOID);
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
 * Evaluates tintersects/tdisjoint for a temporal sequence point and a geometry
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter Whether we compute tintersects or tdisjoint
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
  if (! overlaps_stbox_stbox_internal(box1, box))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_internal(datum_no, BOOLOID,
      &seq->period, STEP);
    *count = 1;
    return result;
  }

  Datum traj = tpointseq_trajectory(seq);
#if POSTGIS_VERSION_NUMBER < 30000
  Datum inter = call_function2(intersection, traj, geom);
#else
  Datum inter = call_function2(ST_Intersection, traj, geom);
#endif
  GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
  if (gserialized_is_empty(gsinter))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_internal(datum_no, BOOLOID,
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
    result[0] = tsequence_from_base_internal(datum_yes, BOOLOID,
      &seq->period, STEP);
    POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
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
    result[0] = tsequence_from_base_internal(datum_no, BOOLOID,
      &seq->period, STEP);
    pfree(DatumGetPointer(gsinter));
    *count = 1;
    return result;
  }
  PeriodSet *ps;
  if (countper == 1)
    ps = minus_period_period_internal(&seq->period, periods[0]);
  else
  {
    /* It is necessary to sort the periods */
    periodarr_sort(periods, countper);
    PeriodSet *ps1 = periodset_make((const Period **) periods, countper, NORMALIZE);
    ps = minus_period_periodset_internal(&seq->period, ps1);
    pfree(ps1);
  }
  int newcount = countper;
  if (ps != NULL)
    newcount += ps->count;
  result = palloc(sizeof(TSequence *) * newcount);
  for (int i = 0; i < countper; i++)
    result[i] = tsequence_from_base_internal(datum_yes, BOOLOID,
      periods[i], STEP);
  if (ps != NULL)
  {
    for (int i = 0; i < ps->count; i++)
    {
      const Period *p = periodset_per_n(ps, i);
      result[i + countper] = tsequence_from_base_internal(datum_no, BOOLOID,
        p, STEP);
    }
    tsequencearr_sort(result, newcount);
    pfree(ps);
  }
  pfree_array((void **) periods, countper);
  *count = newcount;
  return result;
}

/**
 * Evaluates tintersects/tdisjoint for a temporal sequence point and a geometry
 *
 * The function splits the temporal point in an array of temporal point
 * sequences that are simple (that is, not self-intersecting) and loops
 * for each piece.
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter Whether we compute tintersects or tdisjoint
 * @param[in] func PostGIS function to be used for instantaneous sequences
 * @param[out] count Number of elements in the output array
 */
TSequence **
tinterrel_tpointseq_geom1(const TSequence *seq, Datum geom, const STBOX *box,
  bool tinter, Datum (*func)(Datum, Datum), int *count)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tinterrel_tpointinst_geom(tsequence_inst_n(seq, 0),
      geom, func);
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tinstant_to_tsequence(inst, STEP);
    pfree(inst);
    *count = 1;
    return result;
  }

  /* Step interpolation */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
    return tinterrel_tpointseq_step_geom(seq, geom, func, count);

  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int newcount;
  TSequence **simpleseqs = tpointseq_make_simple1(seq, &newcount);
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
  return tsequencearr2_to_tsequencearr(sequences, countseqs,
    newcount, totalcount);
}

/**
 * Evaluates tintersects/tdisjoint for a temporal sequence point and a geometry
 *
 * The function splits the temporal point in an array of temporal point
 * sequences that are simple (that is, not self-intersecting) and loops
 * for each piece.
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] func PostGIS function to be used for instantaneous sequences
 * @param[in] tinter Whether we compute tintersects or tdisjoint
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
 * Evaluates tintersects/tdisjoint for a temporal sequence set point and a geometry
 *
 * @param[in] ts Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter Whether we compute tintersects or tdisjoint
 * @param[in] func PostGIS function to be used for instantaneous sequences
 */
static TSequenceSet *
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
  TSequence **allseqs = tsequencearr2_to_tsequencearr(sequences,
    countseqs, ts->count, totalcount);
  return tsequenceset_make_free(allseqs, totalcount, NORMALIZE);
}

/**
 * Evaluates tintersects/tdisjoint for a temporal point and a geometry
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] tinter Whether we compute tintersects or tdisjoint
 */
Temporal *
tinterrel_tpoint_geo(const Temporal *temp, GSERIALIZED *gs, bool tinter)
{
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Datum datum_no = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (gserialized_is_empty(gs))
    return temporal_from_base(temp, datum_no, BOOLOID, STEP);

  /* Bounding box test */
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  /* Non-empty geometries have a bounding box */
  geo_to_stbox_internal(&box2, gs);
  if (!overlaps_stbox_stbox_internal(&box1, &box2))
    return temporal_from_base(temp, datum_no, BOOLOID, STEP);

  /* 3D only if both arguments are 3D */
  Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) &&
#if POSTGIS_VERSION_NUMBER < 30000
    FLAGS_GET_Z(gs->flags) ? &geom_intersects3d : &geom_intersects2d;
#else
    FLAGS_GET_Z(gs->gflags) ? &geom_intersects3d : &geom_intersects2d;
#endif

  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinterrel_tpointinst_geom((TInstant *) temp,
      PointerGetDatum(gs), func);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinterrel_tpointinstset_geom((TInstantSet *) temp,
      PointerGetDatum(gs), func);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tinterrel_tpointseq_geom((TSequence *) temp,
      PointerGetDatum(gs), &box2, tinter, func);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tinterrel_tpointseqset_geom((TSequenceSet *) temp,
      PointerGetDatum(gs), &box2, tinter, func);
  return result;
}

/*****************************************************************************
 * Functions to compute the tdwithin relationship between a temporal sequence
 * and a geometry. The functions use the st_dwithin function from PostGIS
 * only for instantaneous sequences.
 * These functions are not available for geographies since it is based on the
 * function atGeometry.
 *****************************************************************************/

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence set point and the geometry are within the given distance
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] dist Distance
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tdwithin_tpointseq_geo1(const TSequence *seq, Datum geo, Datum dist, int *count)
{
  TSequence **result;
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    Datum value = tinstant_value(tsequence_inst_n(seq, 0));
    Datum dwithin = geom_dwithin2d(value, geo, dist);
    TInstant *inst = tinstant_make(dwithin, seq->period.lower, BOOLOID);
    result[0] = tinstant_to_tsequence(inst, STEP);
    pfree(inst);
    *count = 1;
    return result;
  }

  /* Restrict to the buffered geometry */
  Datum geo_buffer =
#if POSTGIS_VERSION_NUMBER < 30000
    call_function2(buffer, geo, dist);
#else
    call_function3(buffer, geo, dist, CStringGetTextDatum(""));
#endif
  int count1;
  TSequence **atbuffer = tpointseq_at_geometry(seq, geo_buffer, &count1);
  Datum datum_true = BoolGetDatum(true);
  Datum datum_false = BoolGetDatum(false);
  /* We create two temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed. We can do that since the Boolean
   * base type is of fixed size. */
  TInstant *instants[2];
  instants[0] = tinstant_make(datum_false, seq->period.lower, BOOLOID);
  instants[1] = tinstant_make(datum_false, seq->period.upper, BOOLOID);
  if (atbuffer == NULL)
  {
    result = palloc(sizeof(TSequence *));
    /*  The two instant values created above are the ones needed here */
    result[0] = tsequence_make((const TInstant **) instants, 2,
      seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE_NO);
    pfree(instants[0]); pfree(instants[1]);
    *count = 1;
    return result;
  }

  /* Get the periods during which the value is true */
  Period **periods = palloc(sizeof(Period *) * count1);
  for (int i = 0; i < count1; i++)
    periods[i] = &atbuffer[i]->period;
  /* The period set must be normalized */
  PeriodSet *ps = periodset_make((const Period **) periods, count1, NORMALIZE);
  pfree_array((void **) atbuffer, count1);
  pfree(periods);
  /* Get the periods during which the value is false */
  PeriodSet *minus = minus_period_periodset_internal(&seq->period, ps);
  if (minus == NULL)
  {
    result = palloc(sizeof(TSequence *));
    tinstant_set(instants[0], datum_true, seq->period.lower);
    tinstant_set(instants[1], datum_true, seq->period.upper);
    result[0] = tsequence_make((const TInstant **) instants, 2,
      seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE_NO);
    pfree(instants[0]); pfree(instants[1]);
    *count = 1;
    return result;
  }

  /* The original sequence will be split into ps->count + minus->count sequences
    seq     |------------------------|
                t     t       t
    ps        |---| |---|  |-----|
             f     f     f         f
    minus   |-|   |-|   |--|     |---|
  */
  *count = ps->count + minus->count;
  result = palloc(sizeof(TSequence *) * *count);
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(minus, 0);
  bool truevalue = period_cmp_internal(p1, p2) < 0;
  int j = 0, k = 0;
  for (int i = 0; i < *count; i++)
  {
    int l = 0;
    if (truevalue)
    {
      p1 = periodset_per_n(ps, j);
      tinstant_set(instants[l++], datum_true, p1->lower);
      if (p1->lower != p1->upper)
        tinstant_set(instants[l++], datum_true, p1->upper);
      result[i] = tsequence_make((const TInstant **) instants, l,
        p1->lower_inc, p1->upper_inc, STEP, NORMALIZE_NO);
      j++;
    }
    else
    {
      p2 = periodset_per_n(minus, k);
      tinstant_set(instants[l++], datum_false, p2->lower);
      if (p2->lower != p2->upper)
        tinstant_set(instants[l++], datum_false, p2->upper);
      result[i] = tsequence_make((const TInstant **) instants, l,
        p2->lower_inc, p2->upper_inc, STEP, NORMALIZE_NO);
      k++;
    }
    truevalue = ! truevalue;
  }
  pfree(instants[0]); pfree(instants[1]);
  pfree(ps); pfree(minus);
  return result;
}

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence point and the geometry are within the given distance
 */
static TSequenceSet *
tdwithin_tpointseq_geo(const TSequence *seq, Datum geo, Datum dist)
{
  int count;
  TSequence **sequences = tdwithin_tpointseq_geo1(seq, geo, dist, &count);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence set point and the geometry are within the given distance
 */
static TSequenceSet *
tdwithin_tpointseqset_geo(TSequenceSet *ts, Datum geo, Datum dist)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tdwithin_tpointseq_geo(tsequenceset_seq_n(ts, 0), geo, dist);

  TSequence ***sequences = palloc(sizeof(TSequence *) * ts->count);
  int *countseqs = palloc0(sizeof(int) * ts->count);
  int totalseqs = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tdwithin_tpointseq_geo1(seq, geo, dist, &countseqs[i]);
    totalseqs += countseqs[i];
  }
  TSequence **allsequences = tsequencearr2_to_tsequencearr(sequences,
    countseqs, ts->count, totalseqs);
  return tsequenceset_make_free(allsequences, totalseqs, NORMALIZE);
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
 * Returns the timestamps at which the segments of the two temporal points
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
static int
tdwithin_tpointseq_tpointseq1(Datum sv1, Datum ev1, Datum sv2, Datum ev2,
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
    const POINT3DZ *p1 = datum_get_point3dz_p(sv1);
    const POINT3DZ *p2 = datum_get_point3dz_p(ev1);
    const POINT3DZ *p3 = datum_get_point3dz_p(sv2);
    const POINT3DZ *p4 = datum_get_point3dz_p(ev2);

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
    const POINT2D *p1 = datum_get_point2d_p(sv1);
    const POINT2D *p2 = datum_get_point2d_p(ev1);
    const POINT2D *p3 = datum_get_point2d_p(sv2);
    const POINT2D *p4 = datum_get_point2d_p(ev2);
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
 * Returns the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @result Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tpointseq_tpointseq2(TSequence **result, const TSequence *seq1,
  const TSequence *seq2, Datum dist, datum_func3 func)
{
  const TInstant *start1 = tsequence_inst_n(seq1, 0);
  const TInstant *start2 = tsequence_inst_n(seq2, 0);
  if (seq1->count == 1)
  {
    TInstant *inst = tinstant_make(func(tinstant_value(start1),
      tinstant_value(start2), dist), start1->t, BOOLOID);
    result[0] = tinstant_to_tsequence(inst, STEP);
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
  instants[0] = tinstant_make(datum_true, lower, BOOLOID);
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
      int solutions = tdwithin_tpointseq_tpointseq1(sv1, sev1, sv2, sev2,
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
        result[k++] = tinstant_to_tsequence(instants[0], STEP);
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
 * Returns the timestamps at which the segments of two temporal points are
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
  int count = tdwithin_tpointseq_tpointseq2(sequences, seq1, seq2, dist, func);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns the timestamps at which the segments of two temporal points are
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
    k += tdwithin_tpointseq_tpointseq2(&sequences[k], seq1, seq2, dist,
      func);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

/**
 * Returns the temporal contains relationship between the geometry and the
 * temporal point (internal function)
 */
Temporal *
tcontains_geo_tpoint_internal(GSERIALIZED *gs, Temporal *temp)
{
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  Temporal *inter = tinterrel_tpoint_geo(temp, gs, TINTERSECTS);
  Datum bound = call_function1(boundary, PointerGetDatum(gs));
  GSERIALIZED *gsbound = (GSERIALIZED *) PG_DETOAST_DATUM(bound);
  Temporal *result;
  if (! gserialized_is_empty(gsbound))
  {
    Temporal *inter_bound = tinterrel_tpoint_geo(temp, gsbound, TINTERSECTS);
    Temporal *not_inter_bound = tnot_tbool_internal(inter_bound);
    result = boolop_tbool_tbool(inter, not_inter_bound, &datum_and);
    pfree(inter);
    pfree(DatumGetPointer(bound));
    pfree(inter_bound);
    pfree(not_inter_bound);
  }
  else
    result = inter;
  return result;
}

PG_FUNCTION_INFO_V1(tcontains_geo_tpoint);
/**
 * Returns the temporal contains relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tcontains_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = tcontains_geo_tpoint_internal(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdisjoint_geo_tpoint);
/**
 * Returns the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, TDISJOINT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tpoint_geo);
/**
 * Returns the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
tdisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, TDISJOINT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tintersects_geo_tpoint);
/**
 * Returns the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
tintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, TINTERSECTS);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tpoint_geo);
/**
 * Returns the temporal intersects relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
tintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, TINTERSECTS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

/**
 * Returns the temporal touches relationship between the geometry and the
 * temporal point (internal version)
 */
Temporal *
ttouches_tpoint_geo_internal(Temporal *temp, GSERIALIZED *gs)
{
  Datum bound = call_function1(boundary, PointerGetDatum(gs));
  GSERIALIZED *gsbound = (GSERIALIZED *) PG_DETOAST_DATUM(bound);
  Temporal *result;
  if (! gserialized_is_empty(gsbound))
  {
    result = tinterrel_tpoint_geo(temp, gsbound, TINTERSECTS);
    POSTGIS_FREE_IF_COPY_P(gsbound, DatumGetPointer(bound));
    pfree(DatumGetPointer(bound));
  }
  else
    result = temporal_from_base(temp, BoolGetDatum(false), BOOLOID, STEP);
  return result;
}

PG_FUNCTION_INFO_V1(ttouches_geo_tpoint);
/**
 * Returns the temporal touches relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
ttouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  Temporal *result = ttouches_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttouches_tpoint_geo);
/**
 * Returns the temporal touches relationship between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
ttouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  Temporal *result = ttouches_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/

/**
 * Returns a temporal Boolean that states whether the temporal point and
 * the geometry are within the given distance (dispatch function)
 */
Temporal *
tdwithin_tpoint_geo_internal(const Temporal *temp, GSERIALIZED *gs, Datum dist)
{
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  LiftedFunctionInfo lfinfo;
  /* 3D only if both arguments are 3D */
#if POSTGIS_VERSION_NUMBER < 30000
  lfinfo.func = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags) ?
#else
  lfinfo.func = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags) ?
#endif
    (varfunc) &geom_dwithin3d : (varfunc) &geom_dwithin2d;
  lfinfo.numparam = 3;
  lfinfo.restypid = BOOLOID;
  lfinfo.invert = INVERT_NO;
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant_base((TInstant *) temp,
      PointerGetDatum(gs), temp->basetypid, dist, lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset_base((TInstantSet *) temp,
      PointerGetDatum(gs), temp->basetypid, dist, lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tdwithin_tpointseq_geo((TSequence *) temp,
        PointerGetDatum(gs), dist);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tdwithin_tpointseqset_geo((TSequenceSet *) temp,
        PointerGetDatum(gs), dist);
  return result;
}

PG_FUNCTION_INFO_V1(tdwithin_geo_tpoint);
/**
 * Returns a temporal Boolean that states whether the geometry and the
 * temporal point are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_geo);
/**
 * Returns a temporal Boolean that states whether the temporal point and
 * the geometry are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum dist = PG_GETARG_DATUM(2);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns a temporal Boolean that states whether the temporal points
 * are within the given distance (internal function)
 */
Temporal *
tdwithin_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2,
  Datum dist)
{
  Temporal *sync1, *sync2;
  /* Return false if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  datum_func3 func = get_dwithin_fn(temp1->flags, temp2->flags);
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 3;
  lfinfo.restypid = BOOLOID;
  Temporal *result;
  ensure_valid_tempsubtype(sync1->subtype);
  if (sync1->subtype == INSTANT)
    result = (Temporal *) sync_tfunc_tinstant_tinstant(
      (TInstant *) sync1, (TInstant *) sync2, dist, lfinfo);
  else if (sync1->subtype == INSTANTSET)
    result = (Temporal *) sync_tfunc_tinstantset_tinstantset(
      (TInstantSet *) sync1, (TInstantSet *) sync2, dist, lfinfo);
  else if (sync1->subtype == SEQUENCE)
    result = (Temporal *) tdwithin_tpointseq_tpointseq(
      (TSequence *) sync1, (TSequence *) sync2, dist, func);
  else /* sync1->subtype == SEQUENCESET */
    result = (Temporal *) tdwithin_tpointseqset_tpointseqset(
      (TSequenceSet *) sync1, (TSequenceSet *) sync2, dist, func);

  pfree(sync1); pfree(sync2);
  return result;
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_tpoint);
/**
 * Returns a temporal Boolean that states whether the temporal points
 * are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  ensure_same_srid(tpoint_srid_internal(temp1), tpoint_srid_internal(temp2));
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdwithin_tpoint_tpoint_internal(temp1, temp2, dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
