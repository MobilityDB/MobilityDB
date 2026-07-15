/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Spatiotemporal relationships for temporal circular buffers
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean.
 *
 * The following relationships are supported: `tcontains`, `tcovers`,
 * `tdisjoint`, `tintersects`, `ttouches`, and `tdwithin`.
 */

#include "geo/tgeo_tempspatialrels.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/tbool_ops.h"
#include "temporal/temporal_compops.h"
#include "temporal/tinstant.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_boxops.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"
#include "cbuffer/tcbuffer_spatialrels.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry satisfy a spatial relationship
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] invert True if the arguments should be inverted
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool invert, datum_func2 func)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments (the 2D/planar constraint is
   * centralized in ensure_valid_tcbuffer_geo) */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Cbuffer *cb = geom_to_cbuffer(gs);
  if (! cb)
    return NULL;

  Temporal *result = tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
    (Datum) NULL, (varfunc) func, 0, invert);
  pfree(cb);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers satisfy a spatial relationship
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;
  return tspatialrel_tspatial_tspatial(temp1, temp2, (Datum) NULL,
    (varfunc) func, 0, INVERT_NO);
}

/*****************************************************************************
 * `tintersects` and `tdisjoint` functions with a geometry
 * Note that these functions with a cbuffer are provided for all spatial types
 *****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer instant and a geometry intersect or are disjoint
 * @param[in] inst Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
Temporal *
tinterrel_tcbufferinst_geom(const TInstant *inst, const GSERIALIZED *gs,
  bool tinter)
{
  assert(inst); assert(gs); assert(! gserialized_is_empty(gs));
  assert(inst->temptype == T_TCBUFFER);
  GSERIALIZED *trav = tcbufferinst_traversed_area(inst);
  GSERIALIZED *inter = geom_intersection2d_coll(trav, gs);
  pfree(trav);
  Datum datum_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  /* If there is no intersection */
  if (! inter)
    return (Temporal *) tinstant_make(datum_false, T_TBOOL, inst->t);
  pfree(inter);
  return (Temporal *) tinstant_make(datum_true, T_TBOOL, inst->t);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer sequence with discrete interpolation and a geometry intersect or are
 * disjoint
 * @param[in] seq Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
static Temporal *
tinterrel_tcbufferseq_disc_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool tinter)
{
  assert(seq); assert(gs); assert(! gserialized_is_empty(gs));
  assert(seq->count > 1); assert(seq->temptype == T_TCBUFFER);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* Compute the intersection of the traversed area of a temporal circular
   * buffer and the geometry */
  GSERIALIZED *trav = tcbufferseq_traversed_area(seq, false);
  GSERIALIZED *inter = geom_intersection2d_coll(trav, gs);
  pfree(trav);
  /* If there is no intersection */
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! inter)
    return (Temporal *) tsequence_from_base_temp(datum_false, T_TBOOL, seq);
  /* Get the points composing the intersection */
  int npoints;
  GSERIALIZED **points = geo_pointarr(inter, &npoints);
  pfree(inter);
  Set *s = NULL;
  /* Iterate for the points composing the intersection */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    GSERIALIZED *circle = tcbufferinst_traversed_area(inst);
    /* Loop for each point in the intersection */
    bool found = false;
    for (int j = 0; j < npoints; j++)
    {
      found = geom_intersects2d(circle, points[j]);
      if (found)
        break;
    }
    /* If intersection found */
    if (found)
    {
      if (! s)
        /* Initialize the set for the first time */
        s = value_set(TimestampTzGetDatum(inst->t), T_TIMESTAMPTZ);
      else
      {
        /* Compute the union of the set and the newly found timestamp */
        Set *s1 = union_set_value(s, TimestampTzGetDatum(inst->t));
        pfree(s);
        s = s1;
      }
    }
    pfree(circle);
  }
  pfree_array((void *) points, npoints);
  /* Compute the result */
  Datum bool_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum bool_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  /* If no individual instant intersects */
  if (! s)
    return (Temporal *) tsequence_from_base_temp(bool_false, T_TBOOL, seq);
  TSequence *res_true = tsequence_from_base_tstzset(bool_true, T_TBOOL, s);
  int count;
  TimestampTz *times = tsequence_timestamps(seq, &count);
  Datum *datumarr = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    datumarr[i] = TimestampTzGetDatum(times[i]);
  pfree(times);
  Set *s_time = set_make_free(datumarr, count, T_TIMESTAMPTZ, ORDER);
  Set *s_minus = minus_set_set(s_time, s);
  /* If the result is true for the whole sequence set `s_minus` is empty */
  Temporal *result;
  if (s_minus)
  {
    TSequence *res_false = tsequence_from_base_tstzset(bool_false, T_TBOOL,
      s_minus);
    result = tsequence_merge(res_true, res_false);
    pfree(res_true); pfree(s_minus); pfree(res_false);
  }
  else
    result = (Temporal *) res_true;
  pfree(s); pfree(s_time); 
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer sequence with step interpolation and a geometry intersect or are
 * disjoint
 * @param[in] seq Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
static Temporal *
tinterrel_tcbufferseq_step_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool tinter)
{
  assert(seq); assert(gs); assert(! gserialized_is_empty(gs));
  assert(seq->count > 1); assert(seq->temptype == T_TCBUFFER);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);
  /* Compute the intersection of the traversed area of a temporal circular
   * buffer and the geometry */
  GSERIALIZED *trav = tcbufferseq_traversed_area(seq, false);
  GSERIALIZED *inter = geom_intersection2d_coll(trav, gs);
  pfree(trav);
  /* If there is no intersection */
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! inter)
    return (Temporal *) tsequence_from_base_temp(datum_false, T_TBOOL, seq);
  /* Get the points composing the intersection */
  int npoints;
  GSERIALIZED **points = geo_pointarr(inter, &npoints);
  pfree(inter);
  SpanSet *ss = NULL;
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const TInstant *next = (i < seq->count - 1) ?
      TSEQUENCE_INST_N(seq, i + 1) : inst;
    TimestampTz mint = DT_NOEND, maxt = DT_NOBEGIN;
    bool upper_inc = (i == seq->count - 1) ? false : seq->period.upper_inc;
    GSERIALIZED *circle = tcbufferinst_traversed_area(inst);
    /* Loop for each point in the intersection */
    for (int j = 0; j < npoints; j++)
    {
      if (geom_intersects2d(circle, points[j]))
      {
        if (timestamptz_cmp_internal(inst->t, mint) < 0)
          mint = inst->t;
        if (timestamptz_cmp_internal(next->t, maxt) > 0)
          maxt = next->t;
      }
    }
    /* If intersection found */
    if (mint != DT_NOEND && maxt != DT_NOBEGIN)
    {
      /* If mint and maxt are equal they cannot be at an exclusive bound */
      if (mint != maxt || ( ! (mint == inst->t && ! lower_inc) &&
           ! (mint == next->t && ! upper_inc) ) )
      {
        bool lower_inc1, upper_inc1;
        if (mint == maxt)
        {
          lower_inc1 = upper_inc1 = true;
        }
        else
        {
          lower_inc1 = (mint == inst->t) ? lower_inc : true;
          upper_inc1 = (maxt == next->t) ? upper_inc : true;
        }
        Span *s = span_make(TimestampTzGetDatum(mint),
          TimestampTzGetDatum(maxt), lower_inc1, upper_inc1, T_TIMESTAMPTZ);
        if (! ss)
          /* Initialize the spanset for the first time */
          ss = span_to_spanset(s);
        else
        {
          /* Compute the union of the spanset and the newly found span bounds */
          SpanSet *ss1 = union_spanset_span(ss, s);
          pfree(ss);
          ss = ss1;
        }
        pfree(s);
      }
    }
    pfree(circle);
  }
  pfree_array((void *) points, npoints);
  /* If there is no intersection */
  if (! ss)
    return NULL;
  Datum bool_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum bool_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  TSequenceSet *res_true = tsequenceset_from_base_tstzspanset(bool_true,
    T_TBOOL, ss, STEP);
  SpanSet *ss_time = tsequence_time(seq);
  SpanSet *ss_minus = minus_spanset_spanset(ss_time, ss);
  /* If the result is true for the whole sequence, `ss_minus` is empty */
  TSequenceSet *result;
  if (ss_minus)
  {
    TSequenceSet *res_false = tsequenceset_from_base_tstzspanset(bool_false,
      T_TBOOL, ss_minus, STEP);
    result = tsequenceset_merge(res_true, res_false);
    pfree(res_true); pfree(ss_minus); pfree(res_false);
  }
  else
    result = res_true;
  pfree(ss); pfree(ss_time); 
  return (Temporal *) result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer sequence with linear interpolation and a geometry intersect
 * @param[in] seq Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
static Temporal *
tinterrel_tcbufferseq_linear_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool tinter)
{
  assert(seq); assert(gs); assert(seq->count > 1);
  assert(! gserialized_is_empty(gs)); assert(seq->temptype == T_TCBUFFER);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  /* Compute the intersection of the traversed area of a temporal circular
   * buffer and the geometry */
  GSERIALIZED *trav = tcbufferseq_traversed_area(seq, false);
  GSERIALIZED *inter = geom_intersection2d_coll(trav, gs);
  pfree(trav);
  /* If there is no intersection */
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! inter)
    return (Temporal *) tsequence_from_base_temp(datum_false, T_TBOOL, seq);
  /* Get the points composing the intersection */
  int npoints;
  GSERIALIZED **points = geo_pointarr(inter, &npoints);
  pfree(inter);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  SpanSet *ss = NULL;
  bool lower_inc = seq->period.lower_inc;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    TimestampTz mint = DT_NOEND, maxt = DT_NOBEGIN;
    bool upper_inc = seq->period.upper_inc;
    /* Loop for each point in the intersection */
    const Cbuffer *cb1_start = DatumGetCbufferP(tinstant_value_p(inst1));
    const Cbuffer *cb1_end   = DatumGetCbufferP(tinstant_value_p(inst2));
    for (int j = 0; j < npoints; j++)
    {
      Cbuffer *cb = cbuffer_make(points[j], 0.0);
      /* Check whether the segment endpoints already satisfy the condition */
      bool start_in = (cbuffer_distance(cb1_start, cb) <= 0.0);
      bool end_in   = (cbuffer_distance(cb1_end,   cb) <= 0.0);
      TimestampTz t1, t2;
      int found = tcbuffersegm_intersection_value(tinstant_value_p(inst1),
        tinstant_value_p(inst2), PointerGetDatum(cb), inst1->t, inst2->t,
        &t1, &t2);
      TimestampTz seg_t1 = DT_NOEND, seg_t2 = DT_NOBEGIN;
      if (found == 0)
      {
        /* No crossing root: wholly inside or wholly outside */
        if (start_in)
        {
          seg_t1 = inst1->t;
          seg_t2 = inst2->t;
        }
      }
      else if (found == 1)
      {
        /* One crossing: either entry or exit */
        if (start_in)
        {
          seg_t1 = inst1->t; /* start is inside → root is exit */
          seg_t2 = t1;
        }
        else
        {
          seg_t1 = t1;       /* start is outside → root is entry */
          seg_t2 = inst2->t;
        }
      }
      else /* found == 2 */
      {
        seg_t1 = t1;
        seg_t2 = t2;
      }
      /* Suppress: if end is outside and start is also outside but we set
       * seg_t2 = inst2->t above, verify the end condition */
      if (found == 1 && ! start_in && ! end_in)
        seg_t2 = t1; /* tangent touch: point span */
      if (seg_t1 != DT_NOEND && seg_t2 != DT_NOBEGIN)
      {
        if (timestamptz_cmp_internal(seg_t1, mint) < 0)
          mint = seg_t1;
        if (timestamptz_cmp_internal(seg_t2, maxt) > 0)
          maxt = seg_t2;
      }
      pfree(cb);
    }
    /* If intersection found */
    if (mint != DT_NOEND && maxt != DT_NOBEGIN)
    {
      bool lower_inc1 = (mint == inst1->t) ? lower_inc : true;
      bool upper_inc1 = (maxt == inst2->t) ? upper_inc : true;
      Span *s = span_make(TimestampTzGetDatum(mint), TimestampTzGetDatum(maxt),
        lower_inc1, upper_inc1, T_TIMESTAMPTZ);
      if (! ss)
        /* Initialize the spanset for the first time */
        ss = span_to_spanset(s);
      else
      {
        /* Compute the union of the spanset and the newly found span bounds */
        SpanSet *ss1 = union_spanset_span(ss, s);
        pfree(ss);
        ss = ss1;
      }
      pfree(s);
    }
    inst1 = inst2;
  }
  pfree_array((void *) points, npoints);
  /* If there is no intersection */
  if (! ss)
    return NULL;
  Datum bool_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum bool_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  TSequenceSet *res_true = tsequenceset_from_base_tstzspanset(bool_true,
    T_TBOOL, ss, STEP);
  SpanSet *ss_time = tsequence_time(seq);
  SpanSet *ss_minus = minus_spanset_spanset(ss_time, ss);
  /* If the result is true for the whole sequence set `ss_minus` is empty */
  TSequenceSet *result;
  if (ss_minus)
  {
    TSequenceSet *res_false = tsequenceset_from_base_tstzspanset(bool_false,
      T_TBOOL, ss_minus, STEP);
    result = tsequenceset_merge(res_true, res_false);
    pfree(res_true); pfree(ss_minus); pfree(res_false);
  }
  else
    result = res_true;
  pfree(ss); pfree(ss_time); 
  return (Temporal *) result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer sequence and a geometry intersect (dispatch function)
 * @param[in] seq Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
static Temporal *
tinterrel_tcbufferseq_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool tinter)
{
  assert(seq); assert(gs); assert(! gserialized_is_empty(gs)); 
  assert(seq->temptype == T_TCBUFFER);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tinterrel_tcbufferinst_geom(TSEQUENCE_INST_N(seq, 0), gs, tinter);

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == DISCRETE)
    return tinterrel_tcbufferseq_disc_geom(seq, gs, tinter);
  else if (interp == STEP)
    return tinterrel_tcbufferseq_step_geom(seq, gs, tinter);
  else /* interp == LINEAR */
    return tinterrel_tcbufferseq_linear_geom(seq, gs, tinter);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer sequence set and a geometry intersect (dispatch function)
 * @param[in] ss Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
static Temporal *
tinterrel_tcbufferseqset_geom(const TSequenceSet *ss, const GSERIALIZED *gs,
  bool tinter)
{
  assert(ss); assert(gs); assert(! gserialized_is_empty(gs)); 
  assert(ss->temptype == T_TCBUFFER);
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));

  /* Singleton sequence set */
  if (ss->count == 1)
    return tinterrel_tcbufferseq_geom(TSEQUENCESET_SEQ_N(ss, 0), gs, tinter);

  /* General case */
  Temporal **res_seq = palloc(sizeof(Temporal *) * ss->count);
  int count = 0;
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *res = tinterrel_tcbufferseq_geom(TSEQUENCESET_SEQ_N(ss, i), gs,
      tinter);
    if (res)
      res_seq[count++] = res;
  }
  /* If there is no intersection */
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! count)
  {
    pfree(res_seq);
    return (Temporal *) tsequenceset_from_base_temp(datum_false, T_TBOOL, ss);
  }
  /* Compute the result */
  Temporal *result;
  if (count == 1)
  {
    result = res_seq[0];
    pfree(res_seq);
    return result;
  }
  result = temporal_merge_array(res_seq, count);
  pfree_array((void *) res_seq, count);
  return result;
}

/*****************************************************************************
 * Native (GEOS-free) temporal intersects/disjoint with a geometry
 *
 * These mirror the traversed-area functions above but discover the intersecting
 * sub-periods from the exact swept-capsule distance kernel instead of
 * linearizing the circular buffer through GEOS. They are used for every
 * non-curved geometry; the traversed-area functions remain the fallback for
 * curved input.
 *****************************************************************************/

/**
 * @brief Native version of #tinterrel_tcbufferinst_geom
 */
static Temporal *
tinterrel_tcbufferinst_geo_native(const TInstant *inst, bool tinter,
  double dist, const void *ctx)
{
  bool within = tcbuffer_disc_within_ctx(
    DatumGetCbufferP(tinstant_value_p(inst)), dist, ctx);
  Datum d = (within == tinter) ? BoolGetDatum(true) : BoolGetDatum(false);
  return (Temporal *) tinstant_make(d, T_TBOOL, inst->t);
}

/**
 * @brief Native version of #tinterrel_tcbufferseq_disc_geom
 */
static Temporal *
tinterrel_tcbufferseq_disc_geo_native(const TSequence *seq, bool tinter,
  double dist, const void *ctx)
{
  Set *s = NULL;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tcbuffer_disc_within_ctx(DatumGetCbufferP(tinstant_value_p(inst)), dist,
        ctx))
    {
      if (! s)
        s = value_set(TimestampTzGetDatum(inst->t), T_TIMESTAMPTZ);
      else
      {
        Set *s1 = union_set_value(s, TimestampTzGetDatum(inst->t));
        pfree(s);
        s = s1;
      }
    }
  }
  Datum bool_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum bool_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! s)
    return (Temporal *) tsequence_from_base_temp(bool_false, T_TBOOL, seq);
  TSequence *res_true = tsequence_from_base_tstzset(bool_true, T_TBOOL, s);
  int count;
  TimestampTz *times = tsequence_timestamps(seq, &count);
  Datum *datumarr = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    datumarr[i] = TimestampTzGetDatum(times[i]);
  pfree(times);
  Set *s_time = set_make_free(datumarr, count, T_TIMESTAMPTZ, ORDER);
  Set *s_minus = minus_set_set(s_time, s);
  Temporal *result;
  if (s_minus)
  {
    TSequence *res_false = tsequence_from_base_tstzset(bool_false, T_TBOOL,
      s_minus);
    result = tsequence_merge(res_true, res_false);
    pfree(res_true); pfree(s_minus); pfree(res_false);
  }
  else
    result = (Temporal *) res_true;
  pfree(s); pfree(s_time);
  return result;
}

/**
 * @brief Build the intersects/disjoint temporal Boolean of a sequence from the
 * spanset @p ss of intersecting sub-periods (shared by the step and linear
 * native paths)
 */
static Temporal *
tinterrel_tcbufferseq_from_spanset(const TSequence *seq, const SpanSet *ss,
  bool tinter)
{
  Datum bool_true = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum bool_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  /* No intersecting sub-period: the relationship is false over the whole
   * sequence (resp. true for disjoint), matching the traversed-area path */
  if (! ss)
    return (Temporal *) tsequence_from_base_temp(bool_false, T_TBOOL, seq);
  TSequenceSet *res_true = tsequenceset_from_base_tstzspanset(bool_true,
    T_TBOOL, ss, STEP);
  SpanSet *ss_time = tsequence_time(seq);
  SpanSet *ss_minus = minus_spanset_spanset(ss_time, ss);
  TSequenceSet *result;
  if (ss_minus)
  {
    TSequenceSet *res_false = tsequenceset_from_base_tstzspanset(bool_false,
      T_TBOOL, ss_minus, STEP);
    result = tsequenceset_merge(res_true, res_false);
    pfree(res_true); pfree(ss_minus); pfree(res_false);
  }
  else
    result = res_true;
  pfree(ss_time);
  return (Temporal *) result;
}

/**
 * @brief Return the spanset of sub-periods during which a step-interpolated
 * temporal circular buffer sequence intersects (is within distance 0 of) the
 * geometry (NULL if it never intersects)
 * @note Shared by the temporal-relationship tbool builder and the ever/always
 * coverage projection
 */
static SpanSet *
tcbufferseq_step_within_spanset(const TSequence *seq, double dist,
  const void *ctx)
{
  SpanSet *ss = NULL;
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const TInstant *next = (i < seq->count - 1) ?
      TSEQUENCE_INST_N(seq, i + 1) : inst;
    bool upper_inc = (i == seq->count - 1) ? false : seq->period.upper_inc;
    if (! tcbuffer_disc_within_ctx(DatumGetCbufferP(tinstant_value_p(inst)),
        dist, ctx))
      continue;
    /* The step value is constant over [inst, next), so the span is exactly
     * [inst->t, next->t); mint/maxt equal the segment endpoints, hence the
     * endpoint inclusivity is the sequence's own lower_inc/upper_inc. For the
     * last instant (next == inst) the span degenerates to a point. */
    TimestampTz mint = inst->t, maxt = next->t;
    if (mint == maxt && (! lower_inc || ! upper_inc))
      continue;
    bool lower_inc1, upper_inc1;
    if (mint == maxt)
      lower_inc1 = upper_inc1 = true;
    else
    {
      lower_inc1 = lower_inc;
      upper_inc1 = upper_inc;
    }
    Span *sp = span_make(TimestampTzGetDatum(mint), TimestampTzGetDatum(maxt),
      lower_inc1, upper_inc1, T_TIMESTAMPTZ);
    if (! ss)
      ss = span_to_spanset(sp);
    else
    {
      SpanSet *ss1 = union_spanset_span(ss, sp);
      pfree(ss);
      ss = ss1;
    }
    pfree(sp);
  }
  return ss;
}

/**
 * @brief Native version of #tinterrel_tcbufferseq_step_geom
 */
static Temporal *
tinterrel_tcbufferseq_step_geo_native(const TSequence *seq, bool tinter,
  double dist, const void *ctx)
{
  SpanSet *ss = tcbufferseq_step_within_spanset(seq, dist, ctx);
  Temporal *result = tinterrel_tcbufferseq_from_spanset(seq, ss, tinter);
  if (ss)
    pfree(ss);
  return result;
}

/**
 * @brief Return the spanset of sub-periods during which a linear temporal
 * circular buffer sequence intersects (is within distance 0 of) the geometry,
 * from the exact swept-capsule kernel (NULL if it never intersects)
 * @note Shared by the temporal-relationship tbool builder and the
 * ever/always coverage projection, so both use the identical crossing kernel
 */
static SpanSet *
tcbufferseq_linear_within_spanset(const TSequence *seq, double dist,
  const void *ctx)
{
  int maxo = tcbuffer_geo_ctx_nsegs(ctx) + 2;
  double *rlo = palloc(sizeof(double) * maxo);
  double *rhi = palloc(sizeof(double) * maxo);
  SpanSet *ss = NULL;
  bool lower_inc = seq->period.lower_inc;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = seq->period.upper_inc;
    double dur = (double) (inst2->t - inst1->t);
    int nr = tcbufferseg_within_ctx(DatumGetCbufferP(tinstant_value_p(inst1)),
      DatumGetCbufferP(tinstant_value_p(inst2)), dist, ctx, rlo, rhi, maxo);
    for (int j = 0; j < nr; j++)
    {
      TimestampTz mint = inst1->t + (TimestampTz) llround(rlo[j] * dur);
      TimestampTz maxt = inst1->t + (TimestampTz) llround(rhi[j] * dur);
      if (mint < inst1->t) mint = inst1->t;
      if (maxt > inst2->t) maxt = inst2->t;
      bool lower_inc1 = (mint == inst1->t) ? lower_inc : true;
      bool upper_inc1 = (maxt == inst2->t) ? upper_inc : true;
      if (mint == maxt && ! (lower_inc1 && upper_inc1))
        continue;
      Span *sp = span_make(TimestampTzGetDatum(mint),
        TimestampTzGetDatum(maxt), lower_inc1, upper_inc1, T_TIMESTAMPTZ);
      if (! ss)
        ss = span_to_spanset(sp);
      else
      {
        SpanSet *ss1 = union_spanset_span(ss, sp);
        pfree(ss);
        ss = ss1;
      }
      pfree(sp);
    }
    inst1 = inst2;
  }
  pfree(rlo); pfree(rhi);
  return ss;
}

/**
 * @brief Native version of #tinterrel_tcbufferseq_linear_geom
 */
static Temporal *
tinterrel_tcbufferseq_linear_geo_native(const TSequence *seq, bool tinter,
  double dist, const void *ctx)
{
  SpanSet *ss = tcbufferseq_linear_within_spanset(seq, dist, ctx);
  Temporal *result = tinterrel_tcbufferseq_from_spanset(seq, ss, tinter);
  if (ss)
    pfree(ss);
  return result;
}

/**
 * @brief Native dispatch over a temporal circular buffer sequence
 */
static Temporal *
tinterrel_tcbufferseq_geo_native(const TSequence *seq, bool tinter,
  double dist, const void *ctx)
{
  if (seq->count == 1)
    return tinterrel_tcbufferinst_geo_native(TSEQUENCE_INST_N(seq, 0), tinter,
      dist, ctx);
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tinterrel_tcbufferseq_disc_geo_native(seq, tinter, dist, ctx);
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == STEP)
    return tinterrel_tcbufferseq_step_geo_native(seq, tinter, dist, ctx);
  return tinterrel_tcbufferseq_linear_geo_native(seq, tinter, dist, ctx);
}

/**
 * @brief Native dispatch over a temporal circular buffer sequence set
 */
static Temporal *
tinterrel_tcbufferseqset_geo_native(const TSequenceSet *ss, bool tinter,
  double dist, const void *ctx)
{
  if (ss->count == 1)
    return tinterrel_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, 0), tinter,
      dist, ctx);
  Temporal **res_seq = palloc(sizeof(Temporal *) * ss->count);
  int count = 0;
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *res = tinterrel_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, i),
      tinter, dist, ctx);
    if (res)
      res_seq[count++] = res;
  }
  Datum datum_false = tinter ? BoolGetDatum(false) : BoolGetDatum(true);
  if (! count)
  {
    pfree(res_seq);
    return (Temporal *) tsequenceset_from_base_temp(datum_false, T_TBOOL, ss);
  }
  Temporal *result;
  if (count == 1)
  {
    result = res_seq[0];
    pfree(res_seq);
    return result;
  }
  result = temporal_merge_array(res_seq, count);
  pfree_array((void *) res_seq, count);
  return result;
}

/*****************************************************************************
 * Ever/always intersects and disjoint (GEOS-free)
 *
 * aIntersects and eDisjoint are the two ever/always intersection predicates
 * that do NOT reduce to the nearest-approach running minimum used by
 * eIntersects/aDisjoint. Instead of materializing the temporal intersects
 * Boolean and projecting it, they scan the same exact within-distance-0
 * sub-period set as the temporal relationship and test whether it COVERS the
 * whole definition time, with early exit on the first uncovered (disjoint)
 * instant or sequence:
 *   eDisjoint(temp, gs) ⟺ ∃t disk(t) ∩ gs = ∅ ⟺ the intersecting sub-periods
 *     do not cover the definition time;
 *   aIntersects(temp, gs) ⟺ ¬eDisjoint(temp, gs).
 * Curved or unsupported geometry (no context) returns -1 so the caller keeps
 * the exact traversed-area path.
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer sequence is ever disjoint
 * from the geometry, that is, the intersecting sub-periods do not cover the
 * whole sequence period
 */
static bool
tcbufferseq_ever_disjoint_native(const TSequence *seq, const void *ctx)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* Discrete and single-instant sequences are defined only at their instants;
   * an instant is disjoint when its disc is not within the geometry. */
  if (seq->count == 1 || interp == DISCRETE)
  {
    for (int i = 0; i < seq->count; i++)
      if (! tcbuffer_disc_within_ctx(
          DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), 0.0,
          ctx))
        return true;
    return false;
  }
  /* Step and linear: a vertex whose disc is already disjoint decides the
   * ever-disjoint result immediately, without building the intersecting
   * sub-periods. This is the common case for a buffer that spends most of its
   * life away from the geometry, and reuses the same instant test as the
   * discrete branch. */
  for (int i = 0; i < seq->count; i++)
    if (! tcbuffer_disc_within_ctx(
        DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), 0.0,
        ctx))
      return true;
  /* Every vertex intersects; a disjoint gap can still open inside a segment for
   * a non-convex geometry, so build the exact intersecting sub-periods (the same
   * set the temporal relationship uses) and test full coverage of the period. */
  SpanSet *ss = (interp == STEP) ?
    tcbufferseq_step_within_spanset(seq, 0.0, ctx) :
    tcbufferseq_linear_within_spanset(seq, 0.0, ctx);
  bool covered = (ss != NULL) && contains_spanset_span(ss, &seq->period);
  if (ss)
    pfree(ss);
  return ! covered;
}

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer is ever disjoint from a
 * geometry, 0 if it always intersects, and -1 for curved or unsupported
 * geometry (the caller then uses the traversed-area path)
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 */
int
edisjoint_tcbuffer_geo_native(const Temporal *temp, const GSERIALIZED *gs)
{
  void *ctx = tcbuffer_geo_ctx_make(gs);
  if (! ctx)
    return -1;
  int result = 0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tcbuffer_disc_within_ctx(
      DatumGetCbufferP(tinstant_value_p((TInstant *) temp)), 0.0, ctx) ? 0 : 1;
  else if (temp->subtype == TSEQUENCE)
    result = tcbufferseq_ever_disjoint_native((TSequence *) temp, ctx) ? 1 : 0;
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count; i++)
      if (tcbufferseq_ever_disjoint_native(TSEQUENCESET_SEQ_N(ss, i), ctx))
      {
        result = 1;
        break;
      }
  }
  tcbuffer_geo_ctx_free(ctx);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer expanded by a distance and a geometry intersect or are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 * @param[in] dist Distance by which the circular buffer is expanded, used to
 * back `tdwithin`; zero for `tintersects` and `tdisjoint`
 * @details The distance is threaded to the native within kernels, which fold
 * it into the disc radius, avoiding a separate radius-expansion pass. For
 * curved geometry, which has no native kernel, the radius is expanded and the
 * traversed-area path is used
 */
static Temporal *
tinterrel_tcbuffer_geo_dist(const Temporal *temp, const GSERIALIZED *gs,
  bool tinter, double dist)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test, expanding the temporal box by the distance */
  STBox box1, box1_exp, box2;
  tspatial_set_stbox(temp, &box1);
  if (dist > 0.0)
  {
    stbox_expand_space_set(&box1, dist, &box1_exp);
    box1 = box1_exp;
  }
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return tinter ?
      /* Computing intersection */
      temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp) :
      /* Computing disjoint */
      temporal_from_base_temp(BoolGetDatum(true), T_TBOOL, temp);

  /* Native GEOS-free path for non-curved geometry; the traversed-area path
   * remains the fallback for curved input */
  void *ctx = tcbuffer_geo_ctx_make(gs);
  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (ctx)
  {
    switch (temp->subtype)
    {
      case TINSTANT:
        result = tinterrel_tcbufferinst_geo_native((TInstant *) temp, tinter,
          dist, ctx);
        break;
      case TSEQUENCE:
        result = tinterrel_tcbufferseq_geo_native((TSequence *) temp, tinter,
          dist, ctx);
        break;
      default: /* TSEQUENCESET */
        result = tinterrel_tcbufferseqset_geo_native((TSequenceSet *) temp,
          tinter, dist, ctx);
    }
    tcbuffer_geo_ctx_free(ctx);
    return result;
  }
  /* Curved or unsupported geometry: expand the radius by the distance (a no-op
   * for a zero distance) and fall back to the traversed-area path */
  const Temporal *tw = (dist > 0.0) ? tcbuffer_expand(temp, dist) : temp;
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinterrel_tcbufferinst_geom((TInstant *) tw, gs, tinter);
      break;
    case TSEQUENCE:
      result = tinterrel_tcbufferseq_geom((TSequence *) tw, gs, tinter);
      break;
    default: /* TSEQUENCESET */
      result = tinterrel_tcbufferseqset_geom((TSequenceSet *) tw, gs,
        tinter);
  }
  if (tw != temp)
    pfree((Temporal *) tw);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect or are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
Temporal *
tinterrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool tinter)
{
  return tinterrel_tcbuffer_geo_dist(temp, gs, tinter, 0.0);
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect or are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 */
Temporal *
tinterrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool tinter)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  GSERIALIZED *geo = cbuffer_to_geom(cb);
  Temporal *result = tinterrel_tcbuffer_geo(temp, geo, tinter);
  pfree(geo);
  return result;
}

/*****************************************************************************
 * Generic ever/always spatiotemporal relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatiotemporal relationship for a temporal circular buffer
 * and a circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] param Parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return `NULL`
 */
static Temporal *
tspatialrel_tcbuffer_cbuffer_int(const Temporal *temp, const Cbuffer *cb,
  Datum param, varfunc func, int numparam, bool invert)
{
  assert(temp); assert(cb); assert(temp->temptype == T_TCBUFFER);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = numparam;
  lfinfo.param[0] = param;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = invert;
  lfinfo.discont = false;
  return tfunc_temporal_base(temp, PointerGetDatum(cb), &lfinfo);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry satisfy a spatial relationship
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] invert True if the arguments should be inverted
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool invert, datum_func2 func)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return tspatialrel_tcbuffer_cbuffer_int(temp, cb, (Datum) NULL,
    (varfunc) func, 0, invert);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

/* Native GEOS-free temporal contains (@p strict true) / covers (@p strict
 * false) of a moving disk by a geometry, defined with the touch machinery
 * below; returns NULL for curved or unsupported geometry so the caller keeps
 * the per-instant lifting path */
static Temporal *tcontains_geo_tcbuffer_native(const Temporal *temp,
  const GSERIALIZED *gs, bool strict);

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry contains a
 * temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcontains_geo_tcbuffer()
 */
Temporal *
tcontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  Temporal *res = tcontains_geo_tcbuffer_native(temp, gs, true);
  if (res)
    return res;
  return tspatialrel_tcbuffer_geo(temp, gs, INVERT,
    &datum_cbuffer_contains);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tcontains_geo_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tspatialrel_tcbuffer_geo(temp, gs, INVERT_NO,
    &datum_cbuffer_contains);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * contains a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcontains_cbuffer_tcbuffer()
 */
Temporal *
tcontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
    (Datum) NULL, (varfunc) &datum_cbuffer_contains, 0, INVERT);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Tcontains_cbuffer_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
    (Datum) NULL, (varfunc) &datum_cbuffer_contains, 0, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains another one
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Tcontains_tcbuffer_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_contains);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry covers a
 * temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcovers_geo_tcbuffer()
 */
Temporal *
tcovers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  Temporal *res = tcontains_geo_tcbuffer_native(temp, gs, false);
  if (res)
    return res;
  return tspatialrel_tcbuffer_geo(temp, gs, INVERT,
    &datum_cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tcovers_tcbuffer_geo()
 */
Temporal *
tcovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tspatialrel_tcbuffer_geo(temp, gs, INVERT_NO,
    &datum_cbuffer_covers);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * covers a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcovers_cbuffer_tcbuffer()
 */
Temporal *
tcovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tspatialrel_tcbuffer_cbuffer(temp, cb, INVERT,
    &datum_cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Tcovers_cbuffer_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tspatialrel_tcbuffer_cbuffer(temp, cb, INVERT_NO,
    &datum_cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers another one
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Tcovers_tcbuffer_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2,
    &datum_cbuffer_covers);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_tcbuffer_geo(temp, gs, TDISJOINT);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_tcbuffer_geo(temp, gs, TDISJOINT);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are disjoint
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Tdisjoint_tcbuffer_tcbuffer()
 */
Temporal *
tdisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_disjoint);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tintersects_tcbuffer_geo()
 */
inline Temporal *
tintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Tintersects_tcbuffer_geo()
 */
inline Temporal *
tintersects_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer intersect
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tintersects_cbuffer_tcbuffer()
 */
inline Temporal *
tintersects_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TINTERSECTS);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Tintersects_tcbuffer_cbuffer()
 */
inline Temporal *
tintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TINTERSECTS);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers intersect
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Tintersects_tcbuffer_tcbuffer()
 */
Temporal *
tintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_intersects);
}

/*****************************************************************************
 * Temporal touches (GEOS-free)
 *
 * A moving disk touches a geometry at the INSTANTS where its boundary meets the
 * geometry boundary with disjoint interiors (#tcbuffer_disc_touch_ctx /
 * #tcbufferseg_touch_roots). For linear interpolation these are isolated contact
 * instants; for step interpolation the constant disk touches over a whole
 * sub-period. The temporal touches Boolean is thus true exactly on the contact
 * set and false elsewhere, the moving-disk analogue of the temporal-point
 * boundary approach ttouches(tpoint, geo) = tintersects(tpoint, boundary(geo)).
 * Curved or unsupported geometry (no context) keeps the traversed-area GEOS
 * path.
 *****************************************************************************/

/**
 * @brief Union the degenerate instant span [t, t] into @p ss (a contact instant)
 */
static SpanSet *
tcbuffer_touch_union_instant(SpanSet *ss, TimestampTz t)
{
  Span *sp = span_make(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true,
    true, T_TIMESTAMPTZ);
  SpanSet *res;
  if (! ss)
    res = span_to_spanset(sp);
  else
  {
    res = union_spanset_span(ss, sp);
    pfree(ss);
  }
  pfree(sp);
  return res;
}

/**
 * @brief Native temporal touches Boolean for a temporal circular buffer instant
 */
static Temporal *
ttouches_tcbufferinst_geo_native(const TInstant *inst, const void *ctx)
{
  bool touch = tcbuffer_disc_touch_ctx(DatumGetCbufferP(tinstant_value_p(inst)),
    ctx);
  return (Temporal *) tinstant_make(BoolGetDatum(touch), T_TBOOL, inst->t);
}

/**
 * @brief Native temporal touches Boolean for a discrete temporal circular
 * buffer sequence (true at the touching instants, false at the others)
 */
static Temporal *
ttouches_tcbufferseq_disc_geo_native(const TSequence *seq, const void *ctx)
{
  Set *s = NULL;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tcbuffer_disc_touch_ctx(DatumGetCbufferP(tinstant_value_p(inst)), ctx))
    {
      if (! s)
        s = value_set(TimestampTzGetDatum(inst->t), T_TIMESTAMPTZ);
      else
      {
        Set *s1 = union_set_value(s, TimestampTzGetDatum(inst->t));
        pfree(s);
        s = s1;
      }
    }
  }
  if (! s)
    return (Temporal *) tsequence_from_base_temp(BoolGetDatum(false), T_TBOOL,
      seq);
  TSequence *res_true = tsequence_from_base_tstzset(BoolGetDatum(true), T_TBOOL,
    s);
  int count;
  TimestampTz *times = tsequence_timestamps(seq, &count);
  Datum *datumarr = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    datumarr[i] = TimestampTzGetDatum(times[i]);
  pfree(times);
  Set *s_time = set_make_free(datumarr, count, T_TIMESTAMPTZ, ORDER);
  Set *s_minus = minus_set_set(s_time, s);
  Temporal *result;
  if (s_minus)
  {
    TSequence *res_false = tsequence_from_base_tstzset(BoolGetDatum(false),
      T_TBOOL, s_minus);
    result = tsequence_merge(res_true, res_false);
    pfree(res_true); pfree(s_minus); pfree(res_false);
  }
  else
    result = (Temporal *) res_true;
  pfree(s); pfree(s_time);
  return result;
}

/**
 * @brief Return the spanset of sub-periods during which a step- or
 * linear-interpolated temporal circular buffer sequence touches the geometry
 * (NULL if it never touches): the touching sub-periods for step interpolation,
 * the isolated contact instants for linear
 */
static SpanSet *
tcbufferseq_touch_spanset(const TSequence *seq, const void *ctx)
{
  SpanSet *ss = NULL;
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == STEP)
  {
    bool lower_inc = seq->period.lower_inc;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      if (! tcbuffer_disc_touch_ctx(DatumGetCbufferP(tinstant_value_p(inst)),
          ctx))
        continue;
      const TInstant *next = (i < seq->count - 1) ?
        TSEQUENCE_INST_N(seq, i + 1) : inst;
      bool upper_inc = (i == seq->count - 1) ? false : seq->period.upper_inc;
      TimestampTz mint = inst->t, maxt = next->t;
      /* The last instant (next == inst) degenerates to a point */
      if (mint == maxt)
      {
        if (i == 0 ? lower_inc : true)
          ss = tcbuffer_touch_union_instant(ss, mint);
        continue;
      }
      bool lower_inc1 = (i == 0) ? lower_inc : true;
      Span *sp = span_make(TimestampTzGetDatum(mint), TimestampTzGetDatum(maxt),
        lower_inc1, upper_inc, T_TIMESTAMPTZ);
      if (! ss)
        ss = span_to_spanset(sp);
      else
      {
        SpanSet *s1 = union_spanset_span(ss, sp);
        pfree(ss);
        ss = s1;
      }
      pfree(sp);
    }
    return ss;
  }
  /* Linear: contact instants at the sequence instants and at each segment's
   * interior touch roots */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tcbuffer_disc_touch_ctx(DatumGetCbufferP(tinstant_value_p(inst)), ctx))
      ss = tcbuffer_touch_union_instant(ss, inst->t);
  }
  int maxo = tcbuffer_geo_ctx_nsegs(ctx) + 2;
  double *rt = palloc(sizeof(double) * maxo);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    double dur = (double) (inst2->t - inst1->t);
    int nr = tcbufferseg_touch_roots(DatumGetCbufferP(tinstant_value_p(inst1)),
      DatumGetCbufferP(tinstant_value_p(inst2)), ctx, rt, maxo);
    for (int j = 0; j < nr; j++)
    {
      TimestampTz t = inst1->t + (TimestampTz) llround(rt[j] * dur);
      if (t <= inst1->t || t >= inst2->t)
        continue;
      ss = tcbuffer_touch_union_instant(ss, t);
    }
    inst1 = inst2;
  }
  pfree(rt);
  return ss;
}

/**
 * @brief Native temporal touches Boolean for a step- or linear-interpolated
 * temporal circular buffer sequence
 */
static Temporal *
ttouches_tcbufferseq_geo_native(const TSequence *seq, const void *ctx)
{
  if (seq->count == 1)
    return ttouches_tcbufferinst_geo_native(TSEQUENCE_INST_N(seq, 0), ctx);
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return ttouches_tcbufferseq_disc_geo_native(seq, ctx);
  SpanSet *ss = tcbufferseq_touch_spanset(seq, ctx);
  Temporal *result = tinterrel_tcbufferseq_from_spanset(seq, ss, true);
  if (ss)
    pfree(ss);
  return result;
}

/**
 * @brief Native temporal touches Boolean for a temporal circular buffer
 * sequence set
 */
static Temporal *
ttouches_tcbufferseqset_geo_native(const TSequenceSet *ss, const void *ctx)
{
  if (ss->count == 1)
    return ttouches_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, 0), ctx);
  Temporal **res_seq = palloc(sizeof(Temporal *) * ss->count);
  int count = 0;
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *res = ttouches_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, i),
      ctx);
    if (res)
      res_seq[count++] = res;
  }
  if (! count)
  {
    pfree(res_seq);
    return (Temporal *) tsequenceset_from_base_temp(BoolGetDatum(false),
      T_TBOOL, ss);
  }
  Temporal *result;
  if (count == 1)
  {
    result = res_seq[0];
    pfree(res_seq);
    return result;
  }
  result = temporal_merge_array(res_seq, count);
  pfree_array((void *) res_seq, count);
  return result;
}

/*****************************************************************************
 * Temporal contains and covers (GEOS-free)
 *
 * A geometry contains (@p strict) or covers a moving disk on the sub-periods
 * where the disk centre lies in a polygon of the geometry and the disk clears
 * its boundary: strictly (contains, signed clearance sg > 0) or up to tangency
 * (covers, sg >= 0). The value changes only where sg crosses 0, the same
 * segment roots the touch predicate uses; between consecutive roots the disk
 * stays clear of the boundary so the relation is constant and decided by the
 * interior test at a sample point. A geometry with no polygonal component
 * cannot contain a positive-radius disk, which the point-in-polygon guard makes
 * false. Curved or unsupported geometry (no context) keeps the lifting path.
 *****************************************************************************/

/**
 * @brief Native temporal contains/covers Boolean for a temporal circular buffer
 * instant
 */
static Temporal *
tcontains_tcbufferinst_geo_native(const TInstant *inst, const void *ctx,
  bool strict)
{
  bool c = tcbuffer_disc_contains_ctx(DatumGetCbufferP(tinstant_value_p(inst)),
    ctx, strict);
  return (Temporal *) tinstant_make(BoolGetDatum(c), T_TBOOL, inst->t);
}

/**
 * @brief Native temporal contains/covers Boolean for a discrete temporal
 * circular buffer sequence
 */
static Temporal *
tcontains_tcbufferseq_disc_geo_native(const TSequence *seq, const void *ctx,
  bool strict)
{
  Set *s = NULL;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tcbuffer_disc_contains_ctx(DatumGetCbufferP(tinstant_value_p(inst)),
        ctx, strict))
    {
      if (! s)
        s = value_set(TimestampTzGetDatum(inst->t), T_TIMESTAMPTZ);
      else
      {
        Set *s1 = union_set_value(s, TimestampTzGetDatum(inst->t));
        pfree(s);
        s = s1;
      }
    }
  }
  if (! s)
    return (Temporal *) tsequence_from_base_temp(BoolGetDatum(false), T_TBOOL,
      seq);
  TSequence *res_true = tsequence_from_base_tstzset(BoolGetDatum(true), T_TBOOL,
    s);
  int count;
  TimestampTz *times = tsequence_timestamps(seq, &count);
  Datum *datumarr = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    datumarr[i] = TimestampTzGetDatum(times[i]);
  pfree(times);
  Set *s_time = set_make_free(datumarr, count, T_TIMESTAMPTZ, ORDER);
  Set *s_minus = minus_set_set(s_time, s);
  Temporal *result;
  if (s_minus)
  {
    TSequence *res_false = tsequence_from_base_tstzset(BoolGetDatum(false),
      T_TBOOL, s_minus);
    result = tsequence_merge(res_true, res_false);
    pfree(res_true); pfree(s_minus); pfree(res_false);
  }
  else
    result = (Temporal *) res_true;
  pfree(s); pfree(s_time);
  return result;
}

/**
 * @brief Return the spanset of sub-periods during which the geometry contains
 * (@p strict) or covers a step- or linear-interpolated temporal circular buffer
 * sequence (NULL if never)
 */
static SpanSet *
tcbufferseq_contains_spanset(const TSequence *seq, const void *ctx, bool strict)
{
  SpanSet *ss = NULL;
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == STEP)
  {
    bool lower_inc = seq->period.lower_inc;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      if (! tcbuffer_disc_contains_ctx(
          DatumGetCbufferP(tinstant_value_p(inst)), ctx, strict))
        continue;
      const TInstant *next = (i < seq->count - 1) ?
        TSEQUENCE_INST_N(seq, i + 1) : inst;
      bool upper_inc = (i == seq->count - 1) ? false : seq->period.upper_inc;
      TimestampTz mint = inst->t, maxt = next->t;
      if (mint == maxt)
      {
        if (i == 0 ? lower_inc : true)
          ss = tcbuffer_touch_union_instant(ss, mint);
        continue;
      }
      bool lower_inc1 = (i == 0) ? lower_inc : true;
      Span *sp = span_make(TimestampTzGetDatum(mint), TimestampTzGetDatum(maxt),
        lower_inc1, upper_inc, T_TIMESTAMPTZ);
      if (! ss)
        ss = span_to_spanset(sp);
      else
      {
        SpanSet *s1 = union_spanset_span(ss, sp);
        pfree(ss);
        ss = s1;
      }
      pfree(sp);
    }
    return ss;
  }
  /* Linear: the relation is constant between consecutive sg = 0 (touch) roots.
   * Union each vertex/root instant where it holds and each open sub-interval
   * whose interpolated midpoint satisfies it; the spanset union normalizes the
   * endpoint inclusivity, preserving an isolated tangency (covers) or a grazing
   * hole (contains). */
  int maxo = tcbuffer_geo_ctx_nsegs(ctx) + 2;
  double *rt = palloc(sizeof(double) * maxo);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  if (seq->period.lower_inc && tcbuffer_disc_contains_ctx(
      DatumGetCbufferP(tinstant_value_p(inst1)), ctx, strict))
    ss = tcbuffer_touch_union_instant(ss, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *cb1 = DatumGetCbufferP(tinstant_value_p(inst1));
    const Cbuffer *cb2 = DatumGetCbufferP(tinstant_value_p(inst2));
    double dur = (double) (inst2->t - inst1->t);
    int nr = tcbufferseg_touch_roots(cb1, cb2, ctx, rt, maxo);
    /* Sort the roots ascending (insertion sort; nr is small) */
    for (int a = 1; a < nr; a++)
    {
      double v = rt[a];
      int b = a - 1;
      while (b >= 0 && rt[b] > v) { rt[b + 1] = rt[b]; b--; }
      rt[b + 1] = v;
    }
    double prev = 0.0;
    for (int j = 0; j <= nr; j++)
    {
      double hi = (j < nr) ? rt[j] : 1.0;
      if (hi < prev)
        hi = prev;
      if (hi > prev)
      {
        Cbuffer *cbm = cbuffersegm_interpolate(cb1, cb2,
          (long double) (0.5 * (prev + hi)));
        bool in = tcbuffer_disc_contains_ctx(cbm, ctx, strict);
        pfree(cbm);
        if (in)
        {
          TimestampTz mint = inst1->t + (TimestampTz) llround(prev * dur);
          TimestampTz maxt = inst1->t + (TimestampTz) llround(hi * dur);
          if (mint < inst1->t) mint = inst1->t;
          if (maxt > inst2->t) maxt = inst2->t;
          if (maxt > mint)
          {
            Span *sp = span_make(TimestampTzGetDatum(mint),
              TimestampTzGetDatum(maxt), false, false, T_TIMESTAMPTZ);
            if (! ss)
              ss = span_to_spanset(sp);
            else
            {
              SpanSet *s1 = union_spanset_span(ss, sp);
              pfree(ss);
              ss = s1;
            }
            pfree(sp);
          }
        }
      }
      if (j < nr && rt[j] > 0.0 && rt[j] < 1.0)
      {
        TimestampTz t = inst1->t + (TimestampTz) llround(rt[j] * dur);
        if (t > inst1->t && t < inst2->t)
        {
          Cbuffer *cbr = cbuffersegm_interpolate(cb1, cb2, (long double) rt[j]);
          bool inr = tcbuffer_disc_contains_ctx(cbr, ctx, strict);
          pfree(cbr);
          if (inr)
            ss = tcbuffer_touch_union_instant(ss, t);
        }
      }
      prev = hi;
    }
    bool inc2 = (i < seq->count - 1) ? true : seq->period.upper_inc;
    if (inc2 && tcbuffer_disc_contains_ctx(cb2, ctx, strict))
      ss = tcbuffer_touch_union_instant(ss, inst2->t);
    inst1 = inst2;
  }
  pfree(rt);
  return ss;
}

/**
 * @brief Native temporal contains/covers Boolean for a step- or
 * linear-interpolated temporal circular buffer sequence
 */
static Temporal *
tcontains_tcbufferseq_geo_native(const TSequence *seq, const void *ctx,
  bool strict)
{
  if (seq->count == 1)
    return tcontains_tcbufferinst_geo_native(TSEQUENCE_INST_N(seq, 0), ctx,
      strict);
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tcontains_tcbufferseq_disc_geo_native(seq, ctx, strict);
  SpanSet *ss = tcbufferseq_contains_spanset(seq, ctx, strict);
  Temporal *result = tinterrel_tcbufferseq_from_spanset(seq, ss, true);
  if (ss)
    pfree(ss);
  return result;
}

/**
 * @brief Native temporal contains/covers Boolean for a temporal circular buffer
 * sequence set
 */
static Temporal *
tcontains_tcbufferseqset_geo_native(const TSequenceSet *ss, const void *ctx,
  bool strict)
{
  if (ss->count == 1)
    return tcontains_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, 0), ctx,
      strict);
  Temporal **res_seq = palloc(sizeof(Temporal *) * ss->count);
  int count = 0;
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *res = tcontains_tcbufferseq_geo_native(TSEQUENCESET_SEQ_N(ss, i),
      ctx, strict);
    if (res)
      res_seq[count++] = res;
  }
  if (! count)
  {
    pfree(res_seq);
    return (Temporal *) tsequenceset_from_base_temp(BoolGetDatum(false),
      T_TBOOL, ss);
  }
  Temporal *result;
  if (count == 1)
  {
    result = res_seq[0];
    pfree(res_seq);
    return result;
  }
  result = temporal_merge_array(res_seq, count);
  pfree_array((void *) res_seq, count);
  return result;
}

/**
 * @brief Native GEOS-free temporal contains/covers of a moving disk by a
 * geometry (NULL for curved or unsupported geometry so the caller falls back)
 */
static Temporal *
tcontains_geo_tcbuffer_native(const Temporal *temp, const GSERIALIZED *gs,
  bool strict)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test: a moving disk whose radius-aware bounding box is
   * disjoint from the geometry is never inside it, so it never contains/covers.
   * This constant-time reject avoids the per-segment clearance kernel on the
   * common non-overlapping case (e.g. a spatial join over disjoint extents) */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);

  void *ctx = tcbuffer_geo_ctx_make(gs);
  if (! ctx)
    return NULL;
  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tcontains_tcbufferinst_geo_native((TInstant *) temp, ctx,
        strict);
      break;
    case TSEQUENCE:
      result = tcontains_tcbufferseq_geo_native((TSequence *) temp, ctx,
        strict);
      break;
    default: /* TSEQUENCESET */
      result = tcontains_tcbufferseqset_geo_native((TSequenceSet *) temp, ctx,
        strict);
  }
  tcbuffer_geo_ctx_free(ctx);
  return result;
}

/*****************************************************************************
 * Ever/always touches (GEOS-free)
 *
 * eTouches and aTouches derive from the same contact set as the temporal
 * touches Boolean, so they are exactly consistent with ever/always(tTouches).
 * eTouches scans the instants and segment contact roots with early exit on the
 * first contact; aTouches tests whether the contact sub-periods cover the whole
 * definition time (true only in degenerate always-tangent configurations).
 * Curved or unsupported geometry (no context) returns -1 so the caller keeps
 * the traversed-area path.
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer sequence ever touches the
 * geometry, scanning the instants and (for linear interpolation) the per-segment
 * contact roots with early exit
 */
static bool
tcbufferseq_ever_touches_native(const TSequence *seq, const void *ctx)
{
  for (int i = 0; i < seq->count; i++)
    if (tcbuffer_disc_touch_ctx(
        DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), ctx))
      return true;
  /* For discrete and step interpolation a contact can only occur at an instant
   * (the disk is constant over each step), already covered above */
  if (seq->count == 1 || ! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    return false;
  int maxo = tcbuffer_geo_ctx_nsegs(ctx) + 2;
  double *rt = palloc(sizeof(double) * maxo);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  bool found = false;
  for (int i = 1; i < seq->count && ! found; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    if (tcbufferseg_touch_roots(DatumGetCbufferP(tinstant_value_p(inst1)),
        DatumGetCbufferP(tinstant_value_p(inst2)), ctx, rt, maxo) > 0)
      found = true;
    inst1 = inst2;
  }
  pfree(rt);
  return found;
}

/**
 * @brief Return true if a temporal circular buffer sequence always touches the
 * geometry, that is, the contact sub-periods cover the whole sequence period
 */
static bool
tcbufferseq_always_touches_native(const TSequence *seq, const void *ctx)
{
  if (seq->count == 1 || MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    for (int i = 0; i < seq->count; i++)
      if (! tcbuffer_disc_touch_ctx(
          DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), ctx))
        return false;
    return true;
  }
  SpanSet *ss = tcbufferseq_touch_spanset(seq, ctx);
  bool covered = (ss != NULL) && contains_spanset_span(ss, &seq->period);
  if (ss)
    pfree(ss);
  return covered;
}

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always touches a geometry,
 * 0 if not, and -1 for curved or unsupported geometry (the caller then uses the
 * traversed-area path)
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
eatouches_tcbuffer_geo_native(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  void *ctx = tcbuffer_geo_ctx_make(gs);
  if (! ctx)
    return -1;
  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tcbuffer_disc_touch_ctx(
      DatumGetCbufferP(tinstant_value_p((TInstant *) temp)), ctx) ? 1 : 0;
  else if (temp->subtype == TSEQUENCE)
    result = (ever ?
      tcbufferseq_ever_touches_native((TSequence *) temp, ctx) :
      tcbufferseq_always_touches_native((TSequence *) temp, ctx)) ? 1 : 0;
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    result = ever ? 0 : 1;
    for (int i = 0; i < ss->count; i++)
    {
      bool r = ever ?
        tcbufferseq_ever_touches_native(TSEQUENCESET_SEQ_N(ss, i), ctx) :
        tcbufferseq_always_touches_native(TSEQUENCESET_SEQ_N(ss, i), ctx);
      if (ever && r) { result = 1; break; }
      if (! ever && ! r) { result = 0; break; }
    }
  }
  tcbuffer_geo_ctx_free(ctx);
  return result;
}

/*****************************************************************************
 * Ever/always contains and covers (GEOS-free)
 *
 * eContains/aContains (and the covers variants) derive from the same
 * interior/clearance test as the temporal contains Boolean, so they are exactly
 * consistent with ever/always(tContains). eContains scans the instants and the
 * per-segment sub-intervals with early exit on the first that holds; aContains
 * tests whether the holding sub-periods cover the whole definition time.
 *****************************************************************************/

/**
 * @brief Return true if the geometry ever contains (@p strict) or covers a
 * temporal circular buffer sequence, scanning the instants and (for linear
 * interpolation) the per-segment sub-intervals with early exit
 */
static bool
tcbufferseq_ever_contains_native(const TSequence *seq, const void *ctx,
  bool strict)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  for (int i = 0; i < seq->count; i++)
  {
    /* For linear interpolation a boundary instant excluded by the period
     * inclusivity is not part of the value; for step it governs a sub-period,
     * so it is always considered. This keeps the scan consistent with the
     * temporal contains Boolean the spanset builder produces. */
    if (linear && i == 0 && ! seq->period.lower_inc)
      continue;
    if (linear && i == seq->count - 1 && ! seq->period.upper_inc)
      continue;
    if (tcbuffer_disc_contains_ctx(
        DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), ctx,
        strict))
      return true;
  }
  if (seq->count == 1 || ! linear)
    return false;
  int maxo = tcbuffer_geo_ctx_nsegs(ctx) + 2;
  double *rt = palloc(sizeof(double) * maxo);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  bool found = false;
  for (int i = 1; i < seq->count && ! found; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *cb1 = DatumGetCbufferP(tinstant_value_p(inst1));
    const Cbuffer *cb2 = DatumGetCbufferP(tinstant_value_p(inst2));
    int nr = tcbufferseg_touch_roots(cb1, cb2, ctx, rt, maxo);
    for (int a = 1; a < nr; a++)
    {
      double v = rt[a];
      int b = a - 1;
      while (b >= 0 && rt[b] > v) { rt[b + 1] = rt[b]; b--; }
      rt[b + 1] = v;
    }
    double prev = 0.0;
    for (int j = 0; j <= nr && ! found; j++)
    {
      double hi = (j < nr) ? rt[j] : 1.0;
      if (hi < prev)
        hi = prev;
      if (hi > prev)
      {
        Cbuffer *cbm = cbuffersegm_interpolate(cb1, cb2,
          (long double) (0.5 * (prev + hi)));
        if (tcbuffer_disc_contains_ctx(cbm, ctx, strict))
          found = true;
        pfree(cbm);
      }
      /* An interior root instant (sg = 0) contributes for covers, where a disk
       * tangent to the boundary from inside is an isolated holding instant */
      if (j < nr && rt[j] > 0.0 && rt[j] < 1.0)
      {
        Cbuffer *cbr = cbuffersegm_interpolate(cb1, cb2, (long double) rt[j]);
        if (tcbuffer_disc_contains_ctx(cbr, ctx, strict))
          found = true;
        pfree(cbr);
      }
      prev = hi;
    }
    inst1 = inst2;
  }
  pfree(rt);
  return found;
}

/**
 * @brief Return true if the geometry always contains (@p strict) or covers a
 * temporal circular buffer sequence, that is, the holding sub-periods cover the
 * whole sequence period
 */
static bool
tcbufferseq_always_contains_native(const TSequence *seq, const void *ctx,
  bool strict)
{
  if (seq->count == 1 || MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    for (int i = 0; i < seq->count; i++)
      if (! tcbuffer_disc_contains_ctx(
          DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))), ctx,
          strict))
        return false;
    return true;
  }
  SpanSet *ss = tcbufferseq_contains_spanset(seq, ctx, strict);
  bool covered = (ss != NULL) && contains_spanset_span(ss, &seq->period);
  if (ss)
    pfree(ss);
  return covered;
}

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a geometry ever/always contains (@p strict) or covers a
 * temporal circular buffer, 0 if not, and -1 for curved or unsupported geometry
 * (the caller then uses the traversed-area path)
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] strict True for contains, false for covers
 */
int
eacontains_tcbuffer_geo_native(const Temporal *temp, const GSERIALIZED *gs,
  bool ever, bool strict)
{
  if (gserialized_is_empty(gs))
    return -1;

  /* Bounding box test: a moving disk whose radius-aware bounding box is
   * disjoint from the geometry is never inside it, so it never contains/covers
   * at any instant. This constant-time reject avoids the per-segment clearance
   * scan on the common non-overlapping case (e.g. a spatial join over disjoint
   * extents), giving 0 for both the ever and always semantics */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return 0;

  void *ctx = tcbuffer_geo_ctx_make(gs);
  if (! ctx)
    return -1;
  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tcbuffer_disc_contains_ctx(
      DatumGetCbufferP(tinstant_value_p((TInstant *) temp)), ctx, strict) ?
      1 : 0;
  else if (temp->subtype == TSEQUENCE)
    result = (ever ?
      tcbufferseq_ever_contains_native((TSequence *) temp, ctx, strict) :
      tcbufferseq_always_contains_native((TSequence *) temp, ctx, strict)) ?
      1 : 0;
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    result = ever ? 0 : 1;
    for (int i = 0; i < ss->count; i++)
    {
      bool r = ever ?
        tcbufferseq_ever_contains_native(TSEQUENCESET_SEQ_N(ss, i), ctx,
          strict) :
        tcbufferseq_always_contains_native(TSEQUENCESET_SEQ_N(ss, i), ctx,
          strict);
      if (ever && r) { result = 1; break; }
      if (! ever && ! r) { result = 0; break; }
    }
  }
  tcbuffer_geo_ctx_free(ctx);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Ttouches_tcbuffer_geo()
 */
Temporal *
ttouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test: a moving disk whose radius-aware bounding box is
   * disjoint from the geometry never reaches it, so it never touches */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);

  /* Native GEOS-free path for non-curved geometry: the moving-disk boundary
   * contact instants. Curved or unsupported geometry keeps the traversed-area
   * path. */
  void *ctx = tcbuffer_geo_ctx_make(gs);
  if (! ctx)
    return tspatialrel_tcbuffer_geo(temp, gs, INVERT_NO, &datum_cbuffer_touches);

  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = ttouches_tcbufferinst_geo_native((TInstant *) temp, ctx);
      break;
    case TSEQUENCE:
      result = ttouches_tcbufferseq_geo_native((TSequence *) temp, ctx);
      break;
    default: /* TSEQUENCESET */
      result = ttouches_tcbufferseqset_geo_native((TSequenceSet *) temp, ctx);
  }
  tcbuffer_geo_ctx_free(ctx);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Ttouches_tcbuffer_geo()
 */
Temporal *
ttouches_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return ttouches_tcbuffer_geo(temp, gs);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * touches a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Ttouches_cbuffer_tcbuffer()
 */
Temporal *
ttouches_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tspatialrel_tcbuffer_cbuffer(temp, cb, INVERT,
    &datum_cbuffer_touches);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Ttouches_cbuffer_tcbuffer()
 */
Temporal *
ttouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tspatialrel_tcbuffer_cbuffer(temp, cb, INVERT_NO,
    &datum_cbuffer_touches);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches another one
 * @param[in] temp1,temp2 Temporal circular buffer
 * @csqlfn #Ttouches_tcbuffer_geo()
 */
Temporal *
ttouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_touches);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry are within a distance
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_tcbuffer_geo()
 */
Temporal *
tdwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  return tinterrel_tcbuffer_geo_dist(temp, gs, TINTERSECTS, dist);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry are within a distance
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_tcbuffer_geo()
 */
Temporal *
tdwithin_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, double dist)
{
  return tdwithin_tcbuffer_geo(temp, gs, dist);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are within a distance
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_tcbuffer_cbuffer()
 */
Temporal *
tdwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  /* Call the generic function passing the distance and the turning point
   * functions to be applied */
  return tdwithin_tspatial_spatial(temp, PointerGetDatum(cb),
    Float8GetDatum(dist), &datum_cbuffer_dwithin,
    &tcbuffersegm_tdwithin_turnpt);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are within a distance
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_tcbuffer_tcbuffer()
 */
Temporal *
tdwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  Temporal *sync1, *sync2;
  /* Return false if the temporal circular buffers do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  /* Call the generic function passing the distance and the turning point
   * functions to be applied */
  Temporal *result = tdwithin_tspatial_tspatial(sync1, sync2,
    Float8GetDatum(dist), &datum_cbuffer_dwithin,
    &tcbuffersegm_tdwithin_turnpt);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
