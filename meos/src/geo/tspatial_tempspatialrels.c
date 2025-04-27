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
 * @brief Temporal spatial relationships for temporal geos
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean.
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
 */

#include "geo/tgeo_tempspatialrels.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "temporal/tbool_ops.h"
#include "temporal/temporal_compops.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tpoint_restrfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/tcbuffer_spatialrels.h"

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
 * `tintersects` and `tdisjoint` functions
 * The case for a temporal point and a geometry allow a fast implementation by
 * (1) using bounding box tests, and (2) splitting temporal point sequences
 * into an array of simple (that is, not self-intersecting) fragments where
 * the answer is computed for each fragment with a single call to PostGIS.
 *****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal geo instant
 * and a geometry intersect or are disjoint
 * @param[in] inst Temporal geo
 * @param[in] gs Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used
 */
TInstant *
tinterrel_tspatialinst_geo(const TInstant *inst, const GSERIALIZED *gs,
  bool tinter, datum_func2 func)
{
  /* Result depends on whether we are computing tintersects or tdisjoint */
  bool result = DatumGetBool(func(tinstant_value_p(inst),
    GserializedPGetDatum(gs)));
  /* Invert the result for disjoint */
  if (! tinter)
    result = ! result;
  return tinstant_make(BoolGetDatum(result), T_TBOOL, inst->t);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal geo sequence
 * and a geometry intersect or are disjoint
 * @param[in] seq Temporal geo
 * @param[in] gs Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used
 */
TSequence *
tinterrel_tspatialseq_discstep_geo(const TSequence *seq, const GSERIALIZED *gs,
  bool tinter, datum_func2 func)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp == DISCRETE || interp == STEP);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool result = DatumGetBool(func(tinstant_value_p(inst),
      GserializedPGetDatum(gs)));
    /* Invert the result for disjoint */
    if (! tinter)
      result = ! result;
    instants[i] = tinstant_make(BoolGetDatum(result), T_TBOOL, inst->t);
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
     seq->period.upper_inc, interp, NORMALIZE_NO);
}

/**
 * @brief Return an array of temporal Boolean sequences that state whether a
 * temporal point sequence with linear interpolation that is simple and a
 * geometry intersect or are disjoint
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point has linear interpolation and is simple, that is,
 * it is non self-intersecting
 */
static TSequence **
tinterrel_tpointseq_simple_geo(const TSequence *seq, const GSERIALIZED *gs,
  const STBox *box, bool tinter, int *count)
{
  /* The temporal sequence has at least 2 instants since
   * (1) the instantaneous full sequence test is done in the calling function
   * (2) the simple components of a non self-intersecting sequence have at
   *     least two instants */
  assert(seq->count > 1); assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  TSequence **result;
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Datum datum_yes = tinter ? BoolGetDatum(true) : BoolGetDatum(false);
  Datum datum_no = tinter ? BoolGetDatum(false) : BoolGetDatum(true);

  /* Bounding box test */
  STBox *box1 = TSEQUENCE_BBOX_PTR(seq);
  if (! overlaps_stbox_stbox(box1, box))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL, &seq->period,
      STEP);
    *count = 1;
    return result;
  }

  GSERIALIZED *traj = tpointseq_linear_trajectory(seq, UNARY_UNION);
  GSERIALIZED *inter = geom_intersection2d(traj, gs);
  if (gserialized_is_empty(inter))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL, &seq->period,
      STEP);
    *count = 1;
    pfree(inter);
    return result;
  }

  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  const TInstant *end = TSEQUENCE_INST_N(seq, seq->count - 1);
  /* If the trajectory is a point the result is true due to the
   * non-empty intersection test above */
  if (seq->count == 2 &&
    datum_point_eq(tinstant_value_p(start), tinstant_value_p(end)))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_yes, T_TBOOL, &seq->period,
      STEP);
    *count = 1;
    pfree(inter);
    return result;
  }

  /* Get the periods at which the temporal point intersects the geometry */
  int npers;
  Span *periods = tpointseq_interperiods(seq, inter, &npers);
  if (npers == 0)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL, &seq->period,
      STEP);
    *count = 1;
    pfree(inter);
    return result;
  }
  SpanSet *ss;
  if (npers == 1)
    ss = minus_span_span(&seq->period, &periods[0]);
  else
  {
    /* It is necessary to sort the periods */
    SpanSet *ps1 = spanset_make_exp(periods, npers, npers, NORMALIZE, ORDER);
    ss = minus_span_spanset(&seq->period, ps1);
    pfree(ps1);
  }
  int nseqs = npers;
  if (ss)
    nseqs += ss->count;
  result = palloc(sizeof(TSequence *) * nseqs);
  for (int i = 0; i < npers; i++)
    result[i] = tsequence_from_base_tstzspan(datum_yes, T_TBOOL, &periods[i],
      STEP);
  if (ss)
  {
    for (int i = 0; i < ss->count; i++)
      result[i + npers] = tsequence_from_base_tstzspan(datum_no, T_TBOOL,
        SPANSET_SP_N(ss, i), STEP);
    tseqarr_sort(result, nseqs);
    pfree(ss);
  }
  *count = nseqs;
  pfree(periods);
  return result;
}

/**
 * @brief Return an array of temporal Boolean sequences that state whether a
 * temporal point sequence with linear interpolation and a geometry intersect
 * or are disjoint (iterator function)
 * @details The function splits the temporal geo in an array of fragments that
 * are simple (that is, not self-intersecting) and loops for each fragment
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used
 * @param[out] count Number of elements in the output array
 */
static TSequence **
tinterrel_tpointseq_linear_geo_iter(const TSequence *seq, const GSERIALIZED *gs,
  const STBox *box, bool tinter, datum_func2 func, int *count)
{
  assert(seq); assert(box); assert(count); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tinterrel_tspatialinst_geo(TSEQUENCE_INST_N(seq, 0), gs,
      tinter, func);
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tinstant_to_tsequence_free(inst, STEP);
    *count = 1;
    return result;
  }

  /* Split the temporal point in an array of non self-intersecting temporal
   * points */
  int nsimple;
  TSequence **simpleseqs = tpointseq_make_simple(seq, &nsimple);
  TSequence ***sequences = palloc(sizeof(TSequence *) * nsimple);
  /* palloc0 used to initialize the counters to 0 */
  int *countseqs = palloc0(sizeof(int) * nsimple);
  int totalcount = 0;
  for (int i = 0; i < nsimple; i++)
  {
    sequences[i] = tinterrel_tpointseq_simple_geo(simpleseqs[i], gs, box,
      tinter, &countseqs[i]);
    totalcount += countseqs[i];
  }
  *count = totalcount;
  return tseqarr2_to_tseqarr(sequences, countseqs, nsimple, totalcount);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal point with
 * linear interpolation and a geometry intersect or are disjoint
 * @details The function splits the temporal point in an array of temporal
 * point sequences that are simple (that is, not self-intersecting) and loops
 * for each piece.
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] func PostGIS function to be used
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 */
TSequenceSet *
tinterrel_tpointseq_linear_geo(const TSequence *seq, const GSERIALIZED *gs,
  const STBox *box, bool tinter, datum_func2 func)
{
  assert(seq); assert(box); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int count;
  TSequence **sequences = tinterrel_tpointseq_linear_geo_iter(seq, gs, box,
    tinter, func, &count);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal geo sequence
 * set and a geometry intersect or are disjoint
 * @param[in] ss Temporal geo
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func PostGIS function to be used
 */
TSequenceSet *
tinterrel_tspatialseqset_geo(const TSequenceSet *ss, const GSERIALIZED *gs,
  const STBox *box, bool tinter, datum_func2 func)
{
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
      return tinterrel_tpointseq_linear_geo(TSEQUENCESET_SEQ_N(ss, 0), gs,
        box, tinter, func);
    TSequence *res = tinterrel_tspatialseq_discstep_geo(
      TSEQUENCESET_SEQ_N(ss, 0), gs, tinter, func);
    TSequenceSet *result = tsequence_to_tsequenceset(res);
    pfree(res);
    return result;
  }

  int totalcount;
  TSequence **allseqs;

  /* Linear interpolation */
  if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
  {
    TSequence ***sequences = palloc(sizeof(TSequence *) * ss->count);
    /* palloc0 used to initialize the counters to 0 */
    int *countseqs = palloc0(sizeof(int) * ss->count);
    totalcount = 0;
    for (int i = 0; i < ss->count; i++)
    {
      sequences[i] = tinterrel_tpointseq_linear_geo_iter(
        TSEQUENCESET_SEQ_N(ss, i), gs, box, tinter, func, &countseqs[i]);
      totalcount += countseqs[i];
    }
    allseqs = tseqarr2_to_tseqarr(sequences, countseqs, ss->count, totalcount);
  }
  else
  {
    allseqs = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
      allseqs[i] = tinterrel_tspatialseq_discstep_geo(TSEQUENCESET_SEQ_N(ss, i),
        gs, tinter, func);
    totalcount = ss->count;
  }
  return tsequenceset_make_free(allseqs, totalcount, NORMALIZE);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry intersect or are disjoint
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] restr True if the atValue function is applied to the result
 * @param[in] atvalue Value to be used for the atValue function
 */
Temporal *
tinterrel_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs, bool tinter,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
    /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp,gs) || gserialized_is_empty(gs))
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

  /* 3D only if both arguments are 3D */
  datum_func2 func = MEOS_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags) ?
      &datum_geom_intersects3d : &datum_geom_intersects2d;

  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tinterrel_tspatialinst_geo((TInstant *) temp, gs,
        tinter, func);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        (Temporal *) tinterrel_tpointseq_linear_geo((TSequence *) temp, gs,
          &box2, tinter, func) :
        (Temporal *) tinterrel_tspatialseq_discstep_geo((TSequence *) temp, gs,
          tinter, func);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tinterrel_tspatialseqset_geo(
        (TSequenceSet *) temp, gs, &box2, tinter, func);
  }
  /* Restrict the result to the Boolean value in the third argument if any */
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
 * @brief Return a temporal Boolean that states whether two temporal geos
 * intersect or are disjoint
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] restr True if the atValue function is applied to the result
 * @param[in] atvalue Value to be used for the atValue function
 */
Temporal *
tinterrel_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool tinter, bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp1, NULL); VALIDATE_TSPATIAL(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2))
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
 * @brief Generic spatiotemporal relationship for a temporal geometry and a
 * geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return `NULL`
 */
static Temporal *
tspatialrel_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  assert(temp); assert(gs); assert(! gserialized_is_empty(gs));
  assert(tgeo_type_all(temp->temptype));
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = numparam;
  lfinfo.param[0] = param;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = invert;
  lfinfo.discont = lfinfo.reslinear; /* If linear interpolation */
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/*****************************************************************************/

/**
 * @brief Generic spatiotemporal relationship for two temporal geometries
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return `NULL`
 */
static Temporal *
tspatialrel_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  Datum param, varfunc func, int numparam, bool invert)
{
  assert(temp1); assert(temp2); assert(tgeo_type_all(temp1->temptype));
  assert(tgeo_type_all(temp2->temptype));
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = numparam;
  lfinfo.param[0] = param;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = invert;
  lfinfo.discont = lfinfo.reslinear; /* If linear interpolation */;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry contains a
 * temporal geometry
 * @details The temporal contains relationship is computed as follows:
 * - For temporal points
 * @code
 *     tcontains(geo, tpoint) = tintersects(geo, tpoint) &
 *     ~ tintersects(st_boundary(geo), tpoint)
 * @endcode
 *   where `&` and `~` are the temporal `and` and the temporal `or` operators.
 *   Notice that `tcontains(tpoint, geo)` is not defined, the `tintersects`
 *   function can be used instead.
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure.
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tspatial()
 */
Temporal *
tcontains_geo_tspatial(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result;
  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    Temporal *inter = tinterrel_tspatial_geo(temp, gs, TINTERSECTS, restr,
      atvalue);
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (! gserialized_is_empty(gsbound))
    {
      Temporal *inter_bound = tinterrel_tspatial_geo(temp, gsbound,
        TINTERSECTS, restr, atvalue);
      Temporal *not_inter_bound = tnot_tbool(inter_bound);
      result = boolop_tbool_tbool(inter, not_inter_bound, &datum_and);
      pfree(inter); pfree(gsbound); pfree(inter_bound); pfree(not_inter_bound);
    }
    else
      result = inter;
  }
  else
  /* Temporal geometry case */
  {
    result = tspatialrel_tspatial_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_contains, 0, INVERT);
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
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * contains a geometry
 * @details The temporal contains relationship for a temporal geometry and a
 * geometry is computed at each instant using the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tspatial()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcontains_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo(temp, gs, (Datum) NULL,
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

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * contains another temporal geometry
 * @details The temporal contains relationship for two temporal geometries
 * is computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tspatial()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcontains_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp1, NULL); VALIDATE_TSPATIAL(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial( temp1, temp2) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp1->flags) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *result = tspatialrel_tspatial_tspatial(temp1, temp2, (Datum) NULL,
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

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry covers a
 * temporal geometry
 * @details The temporal covers relationship is computed as follows:
 * - For temporal points
 * @code
 *     tcovers(geo, tpoint) = tintersects(geo, tpoint) &
 *     ~ tintersects(st_boundary(geo), tpoint)
 * @endcode
 *   where `&` and `~` are the temporal `and` and the temporal `or` operators.
 *   Notice that `tcovers(tpoint, geo)` is not defined, the `tintersects`
 *   function can be used instead.
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure.
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_geo_tspatial()
 */
Temporal *
tcovers_geo_tspatial(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_covers, 0, INVERT);

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
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * covers a geometry
 * @details The temporal covers relationship for a temporal geometry and a
 * geometry is computed at each instant using the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_geo_tspatial()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcovers_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result = tspatialrel_tspatial_geo(temp, gs, (Datum) NULL,
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

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * covers another temporal geometry
 * @details The temporal covers relationship for two temporal geometries
 * is computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcovers_geo_tspatial()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcovers_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp1, NULL); VALIDATE_TSPATIAL(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial( temp1, temp2) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp1->flags) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *result = tspatialrel_tspatial_tspatial(temp1, temp2, (Datum) NULL,
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

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry are disjoint
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tspatial_geo()
 */
inline Temporal *
tdisjoint_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, bool atvalue)
{
  return tinterrel_tspatial_geo(temp, gs, TDISJOINT, restr, atvalue);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal geos are
 * disjoint
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdisjoint_tspatial_tspatial()
 */
inline Temporal *
tdisjoint_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tinterrel_tspatial_tspatial(temp1, temp2, TDISJOINT, restr, atvalue);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry intersect
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tintersects_tspatial_geo()
 */
inline Temporal *
tintersects_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool restr, bool atvalue)
{
  return tinterrel_tspatial_geo(temp, gs, TINTERSECTS, restr, atvalue);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal geos
 * intersect
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tintersects_tspatial_tspatial()
 */
inline Temporal *
tintersects_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  return tinterrel_tspatial_tspatial(temp1, temp2, TINTERSECTS, restr,
    atvalue);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo touches
 * a geometry
 * @details The temporal touches relationship is computed as follows:
 * - For temporal points
 *     ttouches(tpoint, geo) = tintersects(tpoint, st_boundary(geo))
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tspatial_geo()
 * @note The function does not support 3D or geographies since the PostGIS
 * function `ST_Touches` only supports 2D geometries
 */
Temporal *
ttouches_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result;
  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (! gserialized_is_empty(gsbound))
    {
      result = tinterrel_tspatial_geo(temp, gsbound, TINTERSECTS, restr,
        atvalue);
      pfree(gsbound);
    }
    else
      result = temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
  }
  else
  /* Temporal geometry case */
  {
    result = tspatialrel_tspatial_geo(temp, gs, (Datum) NULL,
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

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * touches another one
 * @details The temporal `touches` relationship for two temporal geometries is
 * computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geo
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tspatial_geo()
 * @note The function does not support 3D or geographies since the PostGIS
 * function `ST_Touches` only supports 2D geometries.
 * @note The function is not defined for temporal points
 */
Temporal *
ttouches_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp1, NULL); VALIDATE_TSPATIAL(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp1->temptype) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *result =  tspatialrel_tspatial_tspatial(temp1, temp2, (Datum) NULL,
    (varfunc) &datum_geom_touches, 0, INVERT_NO);

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
 * Functions to compute the tdwithin relationship between temporal point
 * sequences. This requires to determine the instants t1 and t2 at which two
 * temporal sequences have a distance d between each other. This amounts to
 * solve the equation
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
 * @brief Return the timestamps at which EITHER the segments of the two
 * temporal points OR a segment of a temporal point and a point are within a
 * distance
 * @param[in] sv1,ev1 Points defining the first segment
 * @param[in] sv2,ev2 Points defining the second segment
 * @param[in] lower,upper Timestamps associated to the segments
 * @param[in] dist Distance
 * @param[in] hasz True for 3D segments
 * @param[in] func Distance function (2D or 3D)
 * @param[out] t1,t2 
 * @return Number of timestamps in the result, between 0 and 2. If there is
 * a single result both t1 and t2 are set to the unique timestamp
 * @note The function is used for both `tDwithin(tpoint, tpoint)` and
 * `tDwithin(tpoint, geo)`, where for the latter the second segment will be
 * constant, that is, the two values `sv2`and `ev2` are set to `geo`
 */
int
tgeosegm_tdwithin(Datum sv1, Datum ev1, Datum sv2, Datum ev2,
  TimestampTz lower, TimestampTz upper, double dist, bool hasz,
  datum_func3 func, TimestampTz *t1, TimestampTz *t2)
{
  double duration = (double) (upper - lower);
  long double a, b, c;
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(sv1);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(ev1);
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(sv2);
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(ev2);

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
    const POINT2D *p1 = DATUM_POINT2D_P(sv1);
    const POINT2D *p2 = DATUM_POINT2D_P(ev1);
    const POINT2D *p3 = DATUM_POINT2D_P(sv2);
    const POINT2D *p4 = DATUM_POINT2D_P(ev2);
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
    if (! func(sv1, sv2, Float8GetDatum(dist)))
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
  /* At most two solutions within the time interval */
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
    if (fabsl(t7 - t8) < MEOS_EPSILON)
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
 * @brief Return the timestamps at which the segments of two temporal point
 * sequences are within a distance (iterator function)
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tpointseq_tpointseq_iter(const TSequence *seq1, const TSequence *seq2,
  Datum dist, datum_func3 func, TSequence **result)
{
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
  bool hasz = MEOS_FLAGS_GET_Z(seq1->flags);
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
    if (datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2))
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
      int solutions = tgeosegm_tdwithin(sv1, sev1, sv2, sev2, lower, upper,
        DatumGetFloat8(dist), hasz, func, &t1, &t2);
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
 * @brief Return the temporal dwithin relationship between two temporal point
 * sequences
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseq_tpointseq(const TSequence *seq1, const TSequence *seq2,
  double dist, datum_func3 func)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq1->count * 4);
  int count = tdwithin_tpointseq_tpointseq_iter(seq1, seq2, Float8GetDatum(dist),
    func, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which the segments of two temporal point
 * sequence sets are within a distance
 * @param[in] ss1,ss2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseqset_tpointseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, datum_func3 func)
{
  /* Singleton sequence set */
  if (ss1->count == 1)
    return tdwithin_tpointseq_tpointseq(TSEQUENCESET_SEQ_N(ss1, 0),
      TSEQUENCESET_SEQ_N(ss2, 0), dist, func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss1->count; i++)
    nseqs += tdwithin_tpointseq_tpointseq_iter(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i), Float8GetDatum(dist), func,
      &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Return the timestamps at which a temporal point sequence and a point
 * are within a distance (iterator function)
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 */
static int
tdwithin_tpointseq_point_iter(const TSequence *seq, Datum point, Datum dist,
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
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
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
      int solutions = tgeosegm_tdwithin(startvalue, endvalue, point, point,
        lower, upper, DatumGetFloat8(dist), hasz, func, &t1, &t2);
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
 * @brief Return the timestamps at which a temporal point sequence and a point
 * are within a distance
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
  int count = tdwithin_tpointseq_point_iter(seq, point, dist, func, sequences);
  /* We are sure that nseqs > 0 since the point is non-empty */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which a temporal point sequence set and a
 * point are within a distance
 * @param[in] ss Temporal point
 * @param[in] point Point
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 */
static TSequenceSet *
tdwithin_tpointseqset_point(const TSequenceSet *ss, Datum point, Datum dist,
  datum_func3 func)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tdwithin_tpointseq_point(TSEQUENCESET_SEQ_N(ss, 0), point, dist,
      func);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tdwithin_tpointseq_point_iter(TSEQUENCESET_SEQ_N(ss, i), point,
      dist, func, &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and
 * a geometry are within a distance
 * @details The temporal `tdwithin` relationship is computed as follows:
 * - For temporal points, compute the instants in which the temporal sequence
 *   has a distance `d` from the geometry, which amounts to solve the equation
 *   `distance(seg(t), geo) = d` for each segment `seg` of the sequence.
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure.
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tspatial_geo()
 * @note The function is available for temporal geographies but not for
 * temporal geography points since this requires to compute the solutions of
 * the quadatric equation for each segment of the temporal point
 */
Temporal *
tdwithin_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      (tpoint_type(temp->temptype) &&
        (! ensure_point_type(gs) || ! ensure_not_geodetic_geo(gs))) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  datum_func3 func =
    /* 3D only if both arguments are 3D */
    MEOS_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags) ?
    &datum_geom_dwithin3d : &datum_geom_dwithin2d;
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      Datum value = tinstant_value_p((TInstant *) temp);
      result = (Temporal *) tinstant_make(func(value, GserializedPGetDatum(gs),
        Float8GetDatum(dist)), T_TBOOL, ((TInstant *) temp)->t);
      break;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tpointseq_point((TSequence *) temp,
            GserializedPGetDatum(gs), Float8GetDatum(dist), func);
      else
      {
        result = tspatialrel_tspatial_geo(temp, gs, Float8GetDatum(dist),
          (varfunc) func, 1, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tpointseqset_point((TSequenceSet *) temp,
          GserializedPGetDatum(gs), Float8GetDatum(dist), func);
      else
      {
        result = tspatialrel_tspatial_geo(temp, gs, Float8GetDatum(dist),
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

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether two temporal geos
 * are within a distance
 * @pre The temporal geos are synchronized.
 */
Temporal *
tdwithin_tspatial_tspatial_sync(const Temporal *sync1, const Temporal *sync2,
  double dist, bool restr, bool atvalue)
{
  datum_func3 func = geo_dwithin_fn(sync1->flags, sync2->flags);
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
        result = (Temporal *) tdwithin_tpointseq_tpointseq((TSequence *) sync1,
          (TSequence *) sync2, dist, func);
      else
      {
        /* Both sequences have either discrete or step interpolation */
        result = tspatialrel_tspatial_tspatial(sync1, sync2, Float8GetDatum(dist),
          (varfunc) func, 1, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(sync1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(sync2->flags);
      if (interp1 == LINEAR || interp2 == LINEAR)
        result = (Temporal *) tdwithin_tpointseqset_tpointseqset(
          (TSequenceSet *) sync1, (TSequenceSet *) sync2, dist, func);
      else
      {
        /* Both sequence sets have step interpolation */
        result = tspatialrel_tspatial_tspatial(sync1, sync2, Float8GetDatum(dist),
          (varfunc) func, 1, INVERT_NO);
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
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal geos
 * are within a distance
 * @details The temporal `tdwithin` relationship is computed as follows:
 * - For temporal points, compute the instants at which two temporal sequences
 *   have a distance `d` between each other, which amounts to solve the
 *   equation `distance(seg1(t), seg2(t)) = d` for each pair of synchronized
 *   segments `seg1`, `seg2`.
 * - For temporal geometries, compute the relationship at each instant with the
 *   lifting infrastructure.
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tspatial_tspatial()
 * @note The function is available for temporal geographies but not for
 * temporal geography points since this requires to compute the solutions of
 * the quadatric equation for each pair of segments of the temporal points
 */
Temporal *
tdwithin_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2, double dist,
  bool restr, bool atvalue)
{
  VALIDATE_TSPATIAL(temp1, NULL); VALIDATE_TSPATIAL(temp2, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  Temporal *sync1, *sync2;
  /* Return false if the temporal geos do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  Temporal *result = tdwithin_tspatial_tspatial_sync(sync1, sync2, dist, restr,
    atvalue);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
