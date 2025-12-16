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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] invert True if the arguments should be inverted
 * @param[in] atvalue Value to restrict
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, bool atvalue, bool invert, datum_func2 func)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  Cbuffer *cb = geom_to_cbuffer(gs);
  if (! cb)
    return NULL;

  Temporal *result = tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
    (Datum) NULL, (varfunc) func, 0, invert);
  pfree(cb);

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
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers satisfy a spatial relationship
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue, datum_func2 func)
{
  VALIDATE_TCBUFFER(temp1, NULL); VALIDATE_TCBUFFER(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *result = tspatialrel_tspatial_tspatial(temp1, temp2, (Datum) NULL,
    (varfunc) func, 0, INVERT_NO);

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

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect or are disjoint
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
  GSERIALIZED *geo = cbuffer_to_geom(cb);
  Temporal *result = tinterrel_tcbuffer_geo(temp, geo, tinter, restr, atvalue);
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
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = false;
  return tfunc_temporal_base(temp, PointerGetDatum(cb), &lfinfo);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry satisfy a spatial relationship
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] invert True if the arguments should be inverted
 * @param[in] atvalue Value to restrict
 * @param[in] func Spatial relationship function to be applied
 */
Temporal *
tspatialrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr,
  bool atvalue, bool invert, datum_func2 func)
{
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(cb, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  Temporal *result = tspatialrel_tcbuffer_cbuffer_int(temp, cb, (Datum) NULL,
    (varfunc) func, 0, invert);

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
  return tspatialrel_tcbuffer_geo(temp, gs, restr, atvalue, INVERT,
    &datum_cbuffer_contains);
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
  return tspatialrel_tcbuffer_geo(temp, gs, restr, atvalue, INVERT_NO,
    &datum_cbuffer_contains);
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

  Temporal *result = tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
    (Datum) NULL, (varfunc) &datum_cbuffer_contains, 0, INVERT);

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

  Temporal *result = tspatialrel_tspatial_base(temp, PointerGetDatum(cb),
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

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer contains another one
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_tcbuffer_tcbuffer()
 */
Temporal *
tcontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue,
    &datum_cbuffer_contains);
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
  return tspatialrel_tcbuffer_geo(temp, gs, restr, atvalue, INVERT,
    &datum_cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_tcbuffer_geo()
 */
Temporal *
tcovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  return tspatialrel_tcbuffer_geo(temp, gs, restr, atvalue, INVERT_NO,
    &datum_cbuffer_covers);
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
  return tspatialrel_tcbuffer_cbuffer(temp, cb, restr, atvalue, INVERT,
    &datum_cbuffer_covers);
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
  return tspatialrel_tcbuffer_cbuffer(temp, cb, restr, atvalue, INVERT_NO,
    &datum_cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer covers another one
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_tcbuffer_tcbuffer()
 */
Temporal *
tcovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue,
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
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_geo(temp, gs, TDISJOINT, restr, atvalue);
}

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
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT, restr, atvalue);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer are disjoint
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tdisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT, restr, atvalue);
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
  return tinterrel_tspatial_tspatial(temp1, temp2, TDISJOINT, restr, atvalue);
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
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tintersects_tcbuffer_geo()
 */
inline Temporal *
tintersects_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS, restr, atvalue);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tintersects_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT, restr, atvalue);
}

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a circular buffer intersect
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tcbuffer_geo()
 */
inline Temporal *
tintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool restr, bool atvalue)
{
  return tinterrel_tcbuffer_cbuffer(temp, cb, TDISJOINT, restr, atvalue);
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
  return tinterrel_tspatial_tspatial(temp1, temp2, TINTERSECTS, restr, atvalue);
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
  return tspatialrel_tcbuffer_geo(temp, gs, restr, atvalue, INVERT_NO,
    &datum_cbuffer_touches);
}

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
ttouches_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  return ttouches_tcbuffer_geo(temp, gs, restr, atvalue);
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
  return tspatialrel_tcbuffer_cbuffer(temp, cb, restr, atvalue, INVERT,
    &datum_cbuffer_touches);
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
  return tspatialrel_tcbuffer_cbuffer(temp, cb, restr, atvalue, INVERT_NO,
    &datum_cbuffer_touches);
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
  return tspatialrel_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue,
    &datum_cbuffer_touches);
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
 * @csqlfn #Tdwithin_tcbuffer_geo()
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

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer and a geometry are within a distance
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tcbuffer_geo()
 */
Temporal *
tdwithin_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, double dist,
  bool restr, bool atvalue)
{
  return tdwithin_tcbuffer_geo(temp, gs, dist, restr, atvalue);
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

  /* Call the generic function passing the distance and the turning point
   * functions to be applied */
  return tdwithin_tspatial_spatial(temp, PointerGetDatum(cb),
    Float8GetDatum(dist), restr, atvalue, &datum_cbuffer_dwithin,
    &tcbuffersegm_dwithin_turnpt);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are within a distance
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tcbuffer_tcbuffer()
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

  /* Call the generic function passing the distance and the turning point
   * functions to be applied */
  Temporal *result = tdwithin_tspatial_tspatial(sync1, sync2,
    Float8GetDatum(dist), restr, atvalue, &datum_cbuffer_dwithin,
    &tcbuffersegm_dwithin_turnpt);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
