/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
#include "temporal/lifting.h"
#include "temporal/tbool_ops.h"
#include "temporal/temporal_compops.h"
#include "temporal/tinstant.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_boxops.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"
#include "cbuffer/tcbuffer_spatialrels.h"

/*****************************************************************************
 * `tintersects` and `tdisjoint` functions
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
  GSERIALIZED *trav = tcbufferinst_trav_area(inst);
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
  GSERIALIZED *trav = tcbufferseq_trav_area(seq);
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
    GSERIALIZED *circle = tcbufferinst_trav_area(inst);
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
  /* If there is no intersection */
  if (! s)
    return (Temporal *) tsequence_from_base_temp(bool_true, T_TBOOL, seq);
  TSequence *res_true = tsequence_from_base_tstzset(bool_false, T_TBOOL, s);
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
  GSERIALIZED *trav = tcbufferseq_trav_area(seq);
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
    GSERIALIZED *circle = tcbufferinst_trav_area(inst);
    /* Loop for each point in the intersection */
    bool found = false;
    for (int j = 0; j < npoints; j++)
    {
      found = geom_intersects2d(circle, points[j]);
      if (found)
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
          ss = span_spanset(s);
        else
        {
          /* Compute the union of the spanset and the newly found span bounds */
          SpanSet *ss1 = union_spanset_span(ss, s);
          pfree(ss);
          ss = ss1;
        }
        pfree(s);
      }
      else
      {}
    }
    pfree(circle);
    inst = next;
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
  GSERIALIZED *trav = tcbufferseq_trav_area(seq);
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
    bool upper_inc = (i == seq->count - 1) ? false : seq->period.upper_inc;
    /* Loop for each point in the intersection */
    for (int j = 0; j < npoints; j++)
    {
      Cbuffer *cb = cbuffer_make(points[j], 0.0);
      TimestampTz t1, t2;
      int found = tcbuffersegm_intersection_value(tinstant_value_p(inst1),
        tinstant_value_p(inst2), PointerGetDatum(cb), inst1->t, inst2->t,
        &t1, &t2);
      if (found)
      {
        if (timestamptz_cmp_internal(t1, mint) < 0)
          mint = t1;
        if (timestamptz_cmp_internal(t2, maxt) > 0)
          maxt = t2;
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
        ss = span_spanset(s);
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
  result = temporal_merge_array((const Temporal **) res_seq, count);
  pfree_array((void *) res_seq, count);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 * @param[in] restr True if the atValue function is applied to the result
 * @param[in] atvalue Value to be used for the atValue function
 */
Temporal *
tinterrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool tinter, bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return restr && atvalue ? NULL :
      temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);

  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinterrel_tcbufferinst_geom((TInstant *) temp, gs, tinter);
      break;
    case TSEQUENCE:
      result = tinterrel_tcbufferseq_geom((TSequence *) temp, gs, tinter);
      break;
    default: /* TSEQUENCESET */
      result = tinterrel_tcbufferseqset_geom((TSequenceSet *) temp, gs,
        tinter);
  }
  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect or are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 * @param[in] restr True if the atValue function is applied to the result
 * @param[in] atvalue Value to be used for the atValue function
 */
Temporal *
tinterrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool tinter, bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  cbuffer_set_stbox(cb, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
  {
    if (tinter)
      /* Computing intersection */
      return restr && atvalue ? NULL :
        temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
    else
      /* Computing disjoint */
      return restr && ! atvalue ? NULL :
        temporal_from_base_temp(BoolGetDatum(true), T_TBOOL, temp);
  }

  datum_func2 func = &datum_geom_intersects2d;
  /* Cheat the compiler to avoid warnings before having the implementation */
  assert(func); 
  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  // switch (temp->subtype)
  // {
    // case TINSTANT:
      // result = (Temporal *) tinterrel_tcbufferinst_cbuffer((TInstant *) temp,
        // cb, tinter, func);
      // break;
    // case TSEQUENCE:
      // result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        // (Temporal *) tinterrel_tcbufferseq_linear_cbuffer((TSequence *) temp,
          // cb, &box2, tinter, func) :
        // (Temporal *) tinterrel_tcbufferseq_discstep_cbuffer((TSequence *) temp,
          // cb, tinter, func);
      // break;
    // default: /* TSEQUENCESET */
      // result = (Temporal *) tinterrel_tcbufferseqset_cbuffer(
        // (TSequenceSet *) temp, cb, &box2, tinter, func);
  // }

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers intersect or are disjoint
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] tinter True when computing `tintersects`, false for `tdisjoint`
 * @param[in] restr True if the atValue function is applied to the result
 * @param[in] atvalue Value to be used for the atValue function
 */
Temporal *
tinterrel_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool tinter, bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *result = tinter ?
      tcomp_temporal_temporal(temp1, temp2, &datum2_eq) :
      tcomp_temporal_temporal(temp1, temp2, &datum2_ne);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, BoolGetDatum(atvalue),
      REST_AT);
    pfree(result);
    result = atresult;
  }
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
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return `NULL`
 */
static Temporal *
tspatialrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
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
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = false;
  return tfunc_temporal_base(temp, PointerGetDatum(cb), &lfinfo);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry contains a
 * temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tcbuffer()
 */
Temporal *
tcontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo_int(temp, gs, (Datum) NULL,
    (varfunc) &datum_geom_contains, 0, INVERT);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo_int(temp, gs, (Datum) NULL,
    (varfunc) &datum_geom_contains, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * contains a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_cbuffer_tcbuffer()
 */
Temporal *
tcontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_contains, 0, INVERT);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_cbuffer_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_contains, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains another one
 * @param[in] temp1,temp2 Temporal circular buffermetries
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *result = tspatialrel_tspatial_tspatial_int(temp1, temp2,
    (Datum) NULL, (varfunc) &datum_cbuffer_contains, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_geo_tcbuffer()
 */
Temporal *
tcovers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Temporal *result;
  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    Temporal *inter = tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS, restr,
      atvalue);
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (! gserialized_is_empty(gsbound))
    {
      Temporal *inter_bound = tinterrel_tcbuffer_geo(temp, gsbound,
        TINTERSECTS, restr, atvalue);
      Temporal *not_inter_bound = tnot_tbool(inter_bound);
      result = boolop_tbool_tbool(inter, not_inter_bound, &datum_and);
      pfree(inter); pfree(gsbound); pfree(inter_bound); pfree(not_inter_bound);
    }
    else
      result = inter;
  }
  else
  /* Temporal circular buffermetry case */
  {
    result = tspatialrel_tspatial_geo_int(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_covers, 0, INVERT);
  }

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_geo_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo_int(temp, gs, (Datum) NULL,
    (varfunc) &datum_geom_covers, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * covers a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_cbuffer_tcbuffer()
 */
Temporal *
tcovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_covers, 0, INVERT);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_cbuffer_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_covers, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers another one
 * @param[in] temp1,temp2 Temporal circular buffermetries
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_tcbuffer_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *result = tspatialrel_tspatial_tspatial_int(temp1, temp2,
    (Datum) NULL, (varfunc) &datum_cbuffer_covers, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_geo(temp, gs, TDISJOINT, restr, atvalue);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are disjoint
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_tcbuffer()
 */
inline Temporal *
tdisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_tcbuffer(temp1, temp2, TDISJOINT, restr, atvalue);
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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tintersects_tcbuffer_geo()
 */
inline Temporal *
tintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS, restr, atvalue);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers intersect
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tintersects_tcbuffer_tcbuffer()
 */
inline Temporal *
tintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_tcbuffer(temp1, temp2, TINTERSECTS, restr, atvalue);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tcbuffer_geo()
 */
Temporal *
ttouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Temporal *result;
  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (! gserialized_is_empty(gsbound))
    {
      result = tinterrel_tcbuffer_geo(temp, gsbound, TINTERSECTS, restr,
        atvalue);
      pfree(gsbound);
    }
    else
      result = temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
  }
  else
  /* Temporal circular buffer case */
  {
    result = tspatialrel_tspatial_geo_int(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_touches, 0, INVERT_NO);
  }

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a circular buffer
 * touches a temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_cbuffer_tcbuffer()
 */
Temporal *
ttouches_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_touches, 0, INVERT);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_cbuffer_tcbuffer()
 */
Temporal *
ttouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr,
  bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_cbuffer_touches, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer touches another one
 * @param[in] temp1,temp2 Temporal circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tcbuffer_geo()
 */
Temporal *
ttouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *result =  tspatialrel_tspatial_tspatial_int(temp1, temp2,
    (Datum) NULL, (varfunc) &datum_cbuffer_touches, 0, INVERT_NO);

  /* Restrict the result to the Boolean value in the last argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/*****************************************************************************
 * Functions to compute the tdwithin relationship between temporal circular
 * buffer sequences. This requires to determine the instants t1 and t2 at which
 * two temporal sequences have a distance d between each other. This amounts to
 * solve the equation
 *     distance(seg1(t), seg2(t)) = d
 * The function assumes that the two segments are synchronized, that they are
 * not instants, and that they are not both constant.
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
 * @brief Construct the result of the tdwithin function of a segment from
 * the solutions of the quadratic equation found previously
 * @return Number of sequences of the result
 */
static int
tdwithin_add_solutions(int solutions, TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc, bool upper_inc1, TimestampTz t1,
  TimestampTz t2, TInstant **instants, TSequence **result)
{
  const Datum datum_true = BoolGetDatum(true);
  const Datum datum_false = BoolGetDatum(false);
  int nseqs = 0;
  /* <  F  > */
  if (solutions == 0 ||
  (solutions == 1 && ((t1 == lower && ! lower_inc) ||
    (t1 == upper && ! upper_inc))))
  {
    tinstant_set(instants[0], datum_false, lower);
    tinstant_set(instants[1], datum_false, upper);
    result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
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
    int ninsts = 0;
    if (t1 != lower)
      tinstant_set(instants[ninsts++], datum_false, lower);
    tinstant_set(instants[ninsts++], datum_true, t1);
    if (solutions == 2 && t1 != t2)
      tinstant_set(instants[ninsts++], datum_true, t2);
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      lower_inc, (t2 != upper) ? true : upper_inc1, STEP, NORMALIZE_NO);
    if (t2 != upper)
    {
      tinstant_set(instants[0], datum_false, t2);
      tinstant_set(instants[1], datum_false, upper);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2, false,
        upper_inc1, STEP, NORMALIZE_NO);
    }
  }
  return nseqs;
}

/**
 * @brief Return the timestamps at which the segments of two temporal circular
 * buffer sequences are within a distance (iterator function)
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 * @pre The temporal circular buffers must be synchronized.
 */
static int
tdwithin_tcbufferseq_tcbufferseq_iter(const TSequence *seq1,
  const TSequence *seq2, Datum dist, TSequence **result)
{
  datum_func3 func = &datum_cbuffer_dwithin;
  const TInstant *start1 = TSEQUENCE_INST_N(seq1, 0);
  const TInstant *start2 = TSEQUENCE_INST_N(seq2, 0);
  if (seq1->count == 1)
  {
    TInstant *inst = tinstant_make(func(tinstant_value_p(start1),
      tinstant_value_p(start2), dist), T_TBOOL, start1->t);
    result[0] = tinstant_to_tsequence_free(inst, STEP);
    return 1;
  }

  int nseqs = 0;
  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  Datum sv1 = tinstant_value_p(start1);
  Datum sv2 = tinstant_value_p(start2);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  const Datum datum_true = BoolGetDatum(true);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, T_TBOOL, lower);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  for (int i = 1; i < seq1->count; i++)
  {
    /* Each iteration of the for loop adds between one and three sequences */
    const TInstant *end1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *end2 = TSEQUENCE_INST_N(seq2, i);
    Datum ev1 = tinstant_value_p(end1);
    Datum ev2 = tinstant_value_p(end2);
    TimestampTz upper = end1->t;
    bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

    /* Both segments are constant */
    if (cbuffer_eq(DatumGetCbufferP(sv1), DatumGetCbufferP(ev1)) &&
        cbuffer_eq(DatumGetCbufferP(sv2), DatumGetCbufferP(ev2)))
    {
      Datum value = func(sv1, sv2, dist);
      tinstant_set(instants[0], value, lower);
      tinstant_set(instants[1], value, upper);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      Datum sev1 = linear1 ? ev1 : sv1;
      Datum sev2 = linear2 ? ev2 : sv2;
      int solutions = tcbuffersegm_dwithin_turnpt(sv1, sev1, sv2, sev2,
        DatumGetFloat8(dist), lower, upper, &t1, &t2);
      bool upper_inc1 = linear1 && linear2 && upper_inc;
      nseqs += tdwithin_add_solutions(solutions, lower, upper, lower_inc,
        upper_inc, upper_inc1, t1, t2, instants, &result[nseqs]);
      /* Add extra final point if only one segment is linear */
      if (upper_inc && (! linear1 || ! linear2))
      {
        Datum value = func(ev1, ev2, dist);
        tinstant_set(instants[0], value, upper);
        result[nseqs++] = tinstant_to_tsequence(instants[0], STEP);
      }
    }
    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return nseqs;
}

/**
 * @brief Return the temporal dwithin relationship between two temporal
 $ circular buffer sequences
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal circular buffers must be synchronized.
 */
static TSequenceSet *
tdwithin_tcbufferseq_tcbufferseq(const TSequence *seq1, const TSequence *seq2,
  double dist, datum_func3 func UNUSED)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq1->count * 4);
  int count = tdwithin_tcbufferseq_tcbufferseq_iter(seq1, seq2,
    Float8GetDatum(dist), sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which the segments of two temporal circular
 * buffer sequence sets are within a distance
 * @param[in] ss1,ss2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal circular buffers must be synchronized.
 */
static TSequenceSet *
tdwithin_tcbufferseqset_tcbufferseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, datum_func3 func)
{
  /* Singleton sequence set */
  if (ss1->count == 1)
    return tdwithin_tcbufferseq_tcbufferseq(TSEQUENCESET_SEQ_N(ss1, 0),
      TSEQUENCESET_SEQ_N(ss2, 0), dist, func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss1->count; i++)
    nseqs += tdwithin_tcbufferseq_tcbufferseq_iter(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i), Float8GetDatum(dist), &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Return the timestamps at which a temporal circular buffer sequence
 * and a point are within a distance (iterator function)
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 */
static int
tdwithin_tcbufferseq_point_iter(const TSequence *seq, Datum point, Datum dist,
  datum_func3 func, TSequence **result)
{
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  Datum startvalue = tinstant_value_p(start);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(func(startvalue, point, dist), T_TBOOL,
      start->t);
    result[0] = tinstant_to_tsequence_free(inst, STEP);
    return 1;
  }

  int nseqs = 0;
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  TimestampTz lower = start->t;
  bool lower_inc = seq->period.lower_inc;
  const Datum datum_true = BoolGetDatum(true);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, T_TBOOL, lower);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the for loop adds between one and three sequences */
    const TInstant *end = TSEQUENCE_INST_N(seq, i);
    Datum endvalue = tinstant_value_p(end);
    TimestampTz upper = end->t;
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;

    /* Segment is constant or has step interpolation */
    if (datum_point_eq(startvalue, endvalue) || ! linear)
    {
      Datum value = func(startvalue, point, dist);
      tinstant_set(instants[0], value, lower);
      if (! linear && upper_inc)
      {
        Datum value1 = func(endvalue, point, dist);
        tinstant_set(instants[1], value1, upper);
      }
      else
        tinstant_set(instants[1], value, upper);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      int solutions = tcbuffersegm_dwithin_turnpt(startvalue, endvalue, point,
        point,DatumGetFloat8(dist), lower, upper, &t1, &t2);
      bool upper_inc1 = linear && upper_inc;
      nseqs += tdwithin_add_solutions(solutions, lower, upper, lower_inc,
        upper_inc, upper_inc1, t1, t2, instants, &result[nseqs]);
    }
    start = end;
    startvalue = endvalue;
    lower = upper;
    lower_inc = true;
  }
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return nseqs;
}

/**
 * @brief Return the timestamps at which a temporal circular buffer sequence
 * and a point are within a distance
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal circular buffers must be synchronized.
 */
static TSequenceSet *
tdwithin_tcbufferseq_point(const TSequence *seq, Datum point, Datum dist,
  datum_func3 func)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * 4);
  int count = tdwithin_tcbufferseq_point_iter(seq, point, dist, func, sequences);
  /* We are sure that nseqs > 0 since the point is non-empty */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which a temporal circular buffer sequence
 * set and a point are within a distance
 * @param[in] ss Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 */
static TSequenceSet *
tdwithin_tcbufferseqset_point(const TSequenceSet *ss, Datum point, Datum dist,
  datum_func3 func)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tdwithin_tcbufferseq_point(TSEQUENCESET_SEQ_N(ss, 0), point, dist,
      func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tdwithin_tcbufferseq_point_iter(TSEQUENCESET_SEQ_N(ss, i), point,
      dist, func, &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tspatial_geo()
 */
Temporal *
tdwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  Temporal *temp_exp = tcbuffer_expand(temp, dist);
  Temporal *result = tinterrel_tcbuffer_geo(temp_exp, gs, TINTERSECTS, restr,
    atvalue);
  pfree(temp_exp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are within a distance
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tcbuffer_cbuffer()
 */
Temporal *
tdwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist,
  bool restr, bool atvalue)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  datum_func3 func = &datum_cbuffer_dwithin;
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      Datum value = tinstant_value_p((TInstant *) temp);
      result = (Temporal *) tinstant_make(func(value, GserializedPGetDatum(cb),
        Float8GetDatum(dist)), T_TBOOL, ((TInstant *) temp)->t);
      break;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tcbufferseq_point((TSequence *) temp,
            GserializedPGetDatum(cb), Float8GetDatum(dist), func);
      else
      {
        result = tspatialrel_tcbuffer_cbuffer(temp, cb, Float8GetDatum(dist),
          (varfunc) func, 1, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tcbufferseqset_point(
          (TSequenceSet *) temp, GserializedPGetDatum(cb),
          Float8GetDatum(dist), func);
      else
      {
        result = tspatialrel_tcbuffer_cbuffer(temp, cb, Float8GetDatum(dist),
          (varfunc) func, 1, INVERT_NO);
      }
  }
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are within a distance
 * @pre The temporal circular buffers are synchronized.
 */
Temporal *
tdwithin_tcbuffer_tcbuffer_sync(const Temporal *sync1, const Temporal *sync2,
  double dist, bool restr, bool atvalue)
{
  datum_func3 func = &datum_cbuffer_dwithin;
  Temporal *result;
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
    {
      Datum value1 = tinstant_value_p((TInstant *) sync1);
      Datum value2 = tinstant_value_p((TInstant *) sync2);
      result = (Temporal *) tinstant_make(func(value1, value2,
        Float8GetDatum(dist)), T_TBOOL, ((TInstant *) sync1)->t);
      break;
    }
    case TSEQUENCE:
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(sync1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(sync2->flags);
      if (interp1 == LINEAR || interp2 == LINEAR)
        result = (Temporal *) tdwithin_tcbufferseq_tcbufferseq(
          (TSequence *) sync1, (TSequence *) sync2, dist, func);
      else
      {
        /* Both sequences have either discrete or step interpolation */
        result = tspatialrel_tspatial_tspatial_int(sync1, sync2,
          Float8GetDatum(dist), (varfunc) func, 1, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(sync1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(sync2->flags);
      if (interp1 == LINEAR || interp2 == LINEAR)
        result = (Temporal *) tdwithin_tcbufferseqset_tcbufferseqset(
          (TSequenceSet *) sync1, (TSequenceSet *) sync2, dist, func);
      else
      {
        /* Both sequence sets have step interpolation */
        result = tspatialrel_tspatial_tspatial_int(sync1, sync2,
          Float8GetDatum(dist), (varfunc) func, 1, INVERT_NO);
      }
    }
  }
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result && restr)
  {
    Temporal *atresult = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = atresult;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are within a distance
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tspatial_tspatial()
 */
Temporal *
tdwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist, bool restr, bool atvalue)
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

  Temporal *result = tdwithin_tcbuffer_tcbuffer_sync(sync1, sync2, dist, restr,
    atvalue);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
