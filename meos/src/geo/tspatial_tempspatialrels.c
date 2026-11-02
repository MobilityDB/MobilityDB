/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Spatiotemporal relationships for temporal geos
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
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/lifting.h"
#include "temporal/tbool_ops.h"
#include "temporal/temporal_compops.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_restrict.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/tcbuffer_spatialrels.h"

/*****************************************************************************
 * Generic functions for computing the spatiotemporal relationships
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
 * @brief Return a temporal Boolean that states whether a spatiotemporal
 * instant and a base value intersect or are disjoint
 * @param[in] inst Spatiotemporal value
 * @param[in] base Base value
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func Spatial relationship function to be applied
 */
TInstant *
tinterrel_tspatialinst_base(const TInstant *inst, Datum base, bool tinter,
  datum_func2 func)
{
  /* Result depends on whether we are computing tintersects or tdisjoint */
  bool result = DatumGetBool(func(tinstant_value_p(inst), base));
  /* Invert the result for disjoint */
  if (! tinter)
    result = ! result;
  return tinstant_make(BoolGetDatum(result), T_TBOOL, inst->t);
}

/**
 * @brief Return a temporal Boolean that states whether a spatiotemporal
 * sequence and a base value intersect or are disjoint
 * @param[in] seq Spatiotemporal value
 * @param[in] base Base value
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func Spatial relationship function to be applied
 */
TSequence *
tinterrel_tspatialseq_discstep_base(const TSequence *seq, Datum base,
  bool tinter, datum_func2 func)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp == DISCRETE || interp == STEP);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool result = DatumGetBool(func(tinstant_value_p(inst), base));
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
 * temporal point sequence and a geometry intersect or are disjoint
 * @details The temporal point sequence has linear interpolation and is simple.
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the base value
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
  const STBox *box1 = TSEQUENCE_BBOX_PTR(seq);
  if (! overlaps_stbox_stbox(box1, box))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL, &seq->period,
      STEP);
    *count = 1;
    return result;
  }

  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  const TInstant *end = TSEQUENCE_INST_N(seq, seq->count - 1);
  bool stationary = (seq->count == 2 &&
    datum_point_eq(tinstant_value_p(start), tinstant_value_p(end)));

  GSERIALIZED *traj = tpointseq_linear_trajectory(seq, UNARY_UNION_NO);
  GSERIALIZED *inter = geom_intersection2d(traj, gs);
  pfree(traj);
  if (gserialized_is_empty(inter))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL,
      &seq->period, STEP);
    *count = 1;
    pfree(inter);
    return result;
  }
  if (stationary)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_yes, T_TBOOL,
      &seq->period, STEP);
    *count = 1;
    pfree(inter);
    return result;
  }
  int npers;
  Span *periods = tpointseq_interperiods(seq, inter, &npers);
  pfree(inter);
  if (npers == 0)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_from_base_tstzspan(datum_no, T_TBOOL,
      &seq->period, STEP);
    *count = 1;
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
 * temporal point sequence and a geometry intersect or are disjoint
 * @details The temporal point sequence has linear interpolation. This is an
 * iterator function. The function splits the temporal geo in an array of
 * fragments that are simple (that is, not self-intersecting) and loops for
 * each fragment
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the base value
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func Spatial relationship function to be applied
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
    TInstant *inst = tinterrel_tspatialinst_base(TSEQUENCE_INST_N(seq, 0),
      PointerGetDatum(gs), tinter, func);
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
    pfree(simpleseqs[i]);
  }
  pfree(simpleseqs);
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
 * @param[in] box Bounding box of the base value
 * @param[in] func Spatial relationship function to be applied
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
 * @brief Return a temporal Boolean that states whether a spatiotemporal
 * sequence set and a base value intersect or are disjoint
 * @param[in] ss Spatiotemporal value
 * @param[in] base Base value
 * @param[in] box Bounding box of the base value
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func Spatial relationship function to be applied
 */
TSequenceSet *
tinterrel_tspatialseqset_base(const TSequenceSet *ss, Datum base,
  const STBox *box, bool tinter, datum_func2 func)
{
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
      return tinterrel_tpointseq_linear_geo(TSEQUENCESET_SEQ_N(ss, 0), 
        DatumGetGserializedP(base), box, tinter, func);
    TSequence *res = tinterrel_tspatialseq_discstep_base(
      TSEQUENCESET_SEQ_N(ss, 0), base, tinter, func);
    TSequenceSet *result = tsequence_as_tsequenceset(res);
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
        TSEQUENCESET_SEQ_N(ss, i), DatumGetGserializedP(base), box, tinter,
          func, &countseqs[i]);
      totalcount += countseqs[i];
    }
    allseqs = tseqarr2_to_tseqarr(sequences, countseqs, ss->count, totalcount);
  }
  else
  {
    allseqs = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
      allseqs[i] = tinterrel_tspatialseq_discstep_base(
        TSEQUENCESET_SEQ_N(ss, i), base, tinter, func);
    totalcount = ss->count;
  }
  return tsequenceset_make_free(allseqs, totalcount, NORMALIZE);
}

/**
 * @brief Return a temporal Boolean that states whether a spatialtemporal
 * value and a base value intersect or are disjoint
 * @param[in] temp Spatiotemporal value
 * @param[in] base Base value
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 * @param[in] func Spatial relationship function to be applied
 * @note The function assumes that all validity tests have been previously done
 */
Temporal *
tinterrel_tspatial_base(const Temporal *temp, Datum base, bool tinter,
  datum_func2 func)
{
  assert(temp); assert(DatumGetPointer(base));
  /* Bounding box test */
  MeosType basetype = temptype_basetype(temp->temptype);
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  spatial_set_stbox(base, basetype, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return tinter ?
      /* Computing intersection */
      temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp) :
      /* Computing disjoint */
      temporal_from_base_temp(BoolGetDatum(true), T_TBOOL, temp);

  Temporal *result = NULL;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tinterrel_tspatialinst_base((TInstant *) temp,
        base, tinter, func);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        (Temporal *) tinterrel_tpointseq_linear_geo((TSequence *) temp,
          DatumGetGserializedP(base), &box2, tinter, func) :
        (Temporal *) tinterrel_tspatialseq_discstep_base((TSequence *) temp,
          base, tinter, func);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tinterrel_tspatialseqset_base(
        (TSequenceSet *) temp, base, &box2, tinter, func);
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry intersect or are disjoint
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 */
Temporal *
tinterrel_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool tinter)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* tintersects is tdwithin with a zero distance and tdisjoint is its
   * negation (as in the static case, geo_intersects =
   * geo_dwithin(., ., 0.0)). For geodetic coordinates the planar
   * intersects below is not applicable; route through the
   * geodetic-capable tdwithin kernel. */
  if (MEOS_FLAGS_GET_GEODETIC(temp->flags))
  {
    /* A temporal geography point meets a geography point exactly where it
     * equals it, an instant the temporal equality finds on the great circle,
     * whereas a distance of zero is a degenerate crossing of the geodetic
     * distance */
    Temporal *res = (tpoint_type(temp->temptype) &&
        gserialized_get_type(gs) == POINTTYPE) ?
      tcomp_temporal_base(temp, PointerGetDatum(gs), &datum2_eq) :
      tdwithin_tgeo_geo(temp, gs, 0.0, true);
    if (! res || tinter)
      return res;
    Temporal *result = tnot_tbool(res);
    pfree(res);
    return result;
  }

  /* Native fast path for a temporal geometry point with linear interpolation
   * over a geometry the clip engine supports, avoiding the intersection of the
   * trajectory with the geometry (#geom_intersection2d) done by the general
   * path below. Instantaneous values and geometries with types the
   * clip does not handle fall through to the general path below. */
  if (temp->temptype == T_TGEOMPOINT && temp->subtype != TINSTANT &&
      MEOS_FLAGS_GET_INTERP(temp->flags) == LINEAR)
  {
    LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
    bool supported = geom_meos_coverage(lwgeom) == 1;
    lwgeom_free(lwgeom);
    if (supported)
    {
      Temporal *res = tpoint_linear_inter_geom(temp, gs, false);
      if (tinter)
        return res;
      Temporal *result = tnot_tbool(res);
      pfree(res);
      return result;
    }
  }

  /* 3D only if both arguments are 3D */
  datum_func2 func = MEOS_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags) ?
      &datum_geom_intersects3d : &datum_geo_intersects2d;
  return tinterrel_tspatial_base(temp, PointerGetDatum(gs), tinter, func);
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether two temporal geos
 * intersect or are disjoint
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] tinter True when computing tintersects, false for tdisjoint
 */
Temporal *
tinterrel_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  bool tinter)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2))
    return NULL;
  /* Two temporal points intersect exactly where they are equal */
  if (tpoint_type(temp1->temptype) && tpoint_type(temp2->temptype))
    return tinter ?
      tcomp_temporal_temporal(temp1, temp2, &datum2_eq) :
      tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
  /* The values of two temporal geometries or geographies are constant
   * between their instants, which have step or discrete interpolation, so
   * they intersect at an instant where their values intersect: in the plane,
   * by the intersection of the two geometries, and on the ellipsoid, where
   * the distance of the two geographies is zero */
  Temporal *res;
  if (MEOS_FLAGS_GET_GEODETIC(temp1->flags))
    res = tdwithin_tgeo_tgeo(temp1, temp2, 0.0, true);
  else
  {
    varfunc func = (MEOS_FLAGS_GET_Z(temp1->flags) &&
      MEOS_FLAGS_GET_Z(temp2->flags)) ?
      (varfunc) &datum_geom_intersects3d : (varfunc) &datum_geo_intersects2d;
    res = tspatialrel_tspatial_tspatial(temp1, temp2, NULL, func, 0,
      INVERT_NO);
  }
  if (! res || tinter)
    return res;
  Temporal *result = tnot_tbool(res);
  pfree(res);
  return result;
}

/*****************************************************************************
 * Generic temporal spatiotemporal relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatiotemporal relationship for a spatiotemporal value and a
 * base value
 * @param[in] temp Spatiotemporal value
 * @param[in] base Base value
 * @param[in] param Parameters of the function, @p numparam of them
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @errval NULL
 */
Temporal *
tspatialrel_tspatial_base(const Temporal *temp, Datum base,
  const Datum *param, varfunc func, int numparam, bool invert)
{
  assert(temp); assert(DatumGetPointer(base));
  assert(tspatial_type(temp->temptype));
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = numparam;
  for (int i = 0; i < numparam; i++)
    lfinfo.param[i] = param[i];
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = invert;
  lfinfo.discont = lfinfo.reslinear; /* If linear interpolation */
  return tfunc_temporal_base(temp, base, &lfinfo);
}

/**
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * and a geometry satisfy a spatial relationship
 * @details The temporal contains relationship for a temporal geometry and a
 * geometry is computed at each instant using the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] func Spatial relationship function to apply
 * @param[in] invert True if the arguments should be inverted
 */
Temporal *
tspatialrel_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
 varfunc func, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_tspatial_base(temp, PointerGetDatum(gs),
    NULL, func, 0, invert);
}

/*****************************************************************************/

/**
 * @brief Generic spatiotemporal relationship for two temporal geometries
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] param Parameters of the function, @p numparam of them
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @errval NULL
 */
Temporal *
tspatialrel_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  const Datum *param, varfunc func, int numparam, bool invert)
{
  assert(temp1); assert(temp2); assert(tspatial_type(temp1->temptype));
  assert(tspatial_type(temp2->temptype));
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = numparam;
  for (int i = 0; i < numparam; i++)
    lfinfo.param[i] = param[i];
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TBOOL;
  lfinfo.invert = invert;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/**
 * @brief Return a temporal Boolean that states whether two temporal geometries
 * satisfy a spatial relationship
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] func Spatial relationship function
 */
Temporal *
tspatialrel_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  varfunc func)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp1->flags) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  return tspatialrel_tspatial_tspatial(temp1, temp2, NULL, func, 0,
    INVERT_NO);
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
 *   where `&` and `~` are the temporal `and` and the temporal `not` operators.
 *   Notice that `tcontains(tpoint, geo)` is not defined, the `tintersects`
 *   function can be used instead.
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure.
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Tcontains_geo_tgeo()
 */
Temporal *
tcontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *result;
  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    /* A geometry contains a moving point when the point lies in the open
     * interior of the geometry, that is, it intersects the (closed) geometry
     * but not its boundary. Both intersections route through the native
     * arc-clip engine for a temporal geometry point with linear interpolation
     * over a clip-supported (possibly curved) geometry, and fall back to the
     * general path otherwise (see #tinterrel_tgeo_geo). */
    Temporal *inter = tinterrel_tgeo_geo(temp, gs, TINTERSECTS);
    if (! inter)
      return NULL;
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (gsbound && ! gserialized_is_empty(gsbound))
    {
      Temporal *inter_bound = tinterrel_tgeo_geo(temp, gsbound, TINTERSECTS);
      if (! inter_bound)
      {
        pfree(gsbound);
        return inter;
      }
      Temporal *not_inter_bound = tnot_tbool(inter_bound);
      result = boolop_tbool_tbool(inter, not_inter_bound, &datum_and);
      pfree(inter); pfree(inter_bound); pfree(not_inter_bound);
    }
    else
      result = inter;
    if (gsbound)
      pfree(gsbound);
  }
  else
  /* Temporal geometry case */
  {
    result = tspatialrel_tspatial_base(temp, PointerGetDatum(gs), NULL,
      (varfunc) &datum_geom_contains, 0, INVERT);
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
 * @csqlfn #Tcontains_tgeo_geo()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcontains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tspatialrel_tgeo_geo(temp, gs, (varfunc) datum_geom_contains,
    INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * contains another one
 * @details The temporal contains relationship for two temporal geometries
 * is computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geometries
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 * @csqlfn #Tcontains_tgeo_tgeo()
 */
Temporal *
tcontains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tgeo_tgeo(temp1, temp2, (varfunc) &datum_geom_contains);
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
 *     tcovers(geo, tpoint) = tintersects(geo, tpoint)
 * @endcode
 *   since a geometry covers a point that lies in the closed geometry, that is,
 *   in its interior together with its boundary. Notice that
 *   `tcovers(tpoint, geo)` is not defined, the `tintersects` function can be
 *   used instead.
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure.
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Tcovers_geo_tgeo()
 */
Temporal *
tcovers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  /* Temporal point case: a geometry covers a moving point when the point lies
   * in the closed geometry (interior together with boundary), which is exactly
   * the temporal intersects relationship. This routes through the native
   * arc-clip engine for a temporal geometry point with linear interpolation
   * over a clip-supported (possibly curved) geometry, and falls back to the
   * general path otherwise (see #tinterrel_tgeo_geo), keeping `tCovers`
   * consistent with `tIntersects` and `~ tDisjoint`. */
  if (tpoint_type(temp->temptype))
    return tinterrel_tgeo_geo(temp, gs, TINTERSECTS);
  return tspatialrel_tgeo_geo(temp, gs, (varfunc) datum_geo_covers2d,
    INVERT);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * covers a geometry
 * @details The temporal covers relationship for a temporal geometry and a
 * geometry is computed at each instant using the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Tcovers_tgeo_geo()
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 */
Temporal *
tcovers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tspatialrel_tgeo_geo(temp, gs, (varfunc) datum_geo_covers2d,
    INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a spatiotemporal value
 * covers another one
 * @details The temporal covers relationship for two temporal geometries
 * is computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geometries
 * @note The function is not available for temporal points, the `tintersects`
 * function can be used instead.
 * @csqlfn #Tcovers_tgeo_tgeo()
 */
Temporal *
tcovers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tgeo_tgeo(temp1, temp2, (varfunc) &datum_geo_covers2d);
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
 * @csqlfn #Tdisjoint_geo_tgeo()
 */
Temporal *
tdisjoint_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_tgeo_geo(temp, gs, TDISJOINT);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry are disjoint
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Tdisjoint_tgeo_geo()
 */
Temporal *
tdisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_tgeo_geo(temp, gs, TDISJOINT);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal geos are
 * disjoint
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Tdisjoint_tgeo_tgeo()
 */
Temporal *
tdisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return tinterrel_tspatial_tspatial(temp1, temp2, TDISJOINT);
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
 * @csqlfn #Tintersects_tgeo_geo()
 */
Temporal *
tintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_tgeo_geo(temp, gs, TINTERSECTS);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry intersect
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Tintersects_geo_tgeo()
 */
Temporal *
tintersects_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_tgeo_geo(temp, gs, TINTERSECTS);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal geos
 * intersect
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Tintersects_tgeo_tgeo()
 */
Temporal *
tintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return tinterrel_tspatial_tspatial(temp1, temp2, TINTERSECTS);
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
 * @csqlfn #Ttouches_tgeo_geo()
 * @note The function does not support 3D or geographies since the PostGIS
 * function `ST_Touches` only supports 2D geometries
 */
Temporal *
ttouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
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
    /* A moving point touches a geometry when the point lies on the boundary of
     * the geometry. The boundary intersection routes through the native
     * arc-clip engine for a temporal geometry point with linear interpolation
     * over a clip-supported (possibly curved) boundary, and falls back to the
     * general path otherwise (see #tinterrel_tgeo_geo). When the geometry has
     * an empty boundary (a point or multipoint) the result is false. */
    GSERIALIZED *gsbound = geom_boundary(gs);
    if (gsbound && ! gserialized_is_empty(gsbound))
      result = tinterrel_tgeo_geo(temp, gsbound, TINTERSECTS);
    else
      result = temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
    if (gsbound)
      pfree(gsbound);
  }
  /* Temporal geometry, temporal cbuffer case */
  else
  {
    result = tspatialrel_tspatial_base(temp, PointerGetDatum(gs), NULL,
      (varfunc) &datum_geom_touches, 0, INVERT_NO);
  }
  return result;
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry touches
 * a temporal geo
 * @details The temporal touches relationship is computed as follows:
 * - For temporal points
 *     ttouches(tpoint, geo) = tintersects(tpoint, st_boundary(geo))
 * - For temporal geometries, compute the relationship at each instant using
 *   the lifting infrastructure
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Ttouches_geo_tgeo()
 * @note The function does not support 3D or geographies since the PostGIS
 * function `ST_Touches` only supports 2D geometries
 */
Temporal *
ttouches_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return ttouches_tgeo_geo(temp, gs);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geometry
 * touches another one
 * @details The temporal `touches` relationship for two temporal geometries is
 * computed at each instant using the lifting infrastructure
 * @param[in] temp1,temp2 Temporal geo
 * @csqlfn #Ttouches_tgeo_tgeo()
 * @note The function does not support 3D or geographies since the PostGIS
 * function `ST_Touches` only supports 2D geometries.
 * @note The function is not defined for temporal points
 */
Temporal *
ttouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return tspatialrel_tgeo_tgeo(temp1, temp2, (varfunc) &datum_geom_touches);
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
 * @brief State of a walk emitting the sequences of a tdwithin result
 * @details Consecutive segments of a lifted walk mostly take the same value,
 * and the constant sequences they yield are joined again by the @p NORMALIZE
 * the sequence set is built with. The walk therefore HOLDS a constant
 * sequence instead of emitting it, and extends it while the segments keep the
 * value, so it allocates one sequence per run of equal value rather than one
 * per segment.
 *
 * Holding changes no answer: a run is extended only where
 * #tsequence_join_test joins, that is where the held sequence and the next
 * one are adjacent and take the same value, and the sequence set is still
 * built with @p NORMALIZE, which joins whatever the walk leaves unjoined. A
 * held sequence also states step interpolation validly whatever its bounds,
 * its two instants taking the same value.
 */
typedef struct
{
  TSequence **result;  /**< Array the emitted sequences are stored in */
  int nseqs;           /**< Number of sequences emitted so far */
  TInstant **instants; /**< Scratch instants an emission is built from */
  bool held;           /**< True when a constant sequence is held */
  Datum value;         /**< Value the held sequence takes */
  TimestampTz lower;   /**< Start of the held sequence */
  TimestampTz upper;   /**< End of the held sequence */
  bool lower_inc;      /**< Whether the held sequence contains its start */
  bool upper_inc;      /**< Whether the held sequence contains its end */
} TDwithinRun;

/**
 * @brief Emit the constant sequence a walk holds, where it holds one
 */
static void
tdwithin_run_flush(TDwithinRun *run)
{
  if (! run->held)
    return;
  tinstant_set(run->instants[0], run->value, run->lower);
  /* A held instant, the touch of a distance at a single instant, is a
   * sequence of one instant */
  int ninsts = 1;
  if (run->upper != run->lower)
    tinstant_set(run->instants[ninsts++], run->value, run->upper);
  run->result[run->nseqs++] = tsequence_make(run->instants, ninsts,
    run->lower_inc, run->upper_inc, STEP, NORMALIZE_NO);
  run->held = false;
  return;
}

/**
 * @brief Add to a walk the constant sequence a segment yields
 * @details The sequence extends the held one where the two are adjacent and
 * take the same value, which is where #tsequence_join_test joins them
 */
static void
tdwithin_run_add(TDwithinRun *run, Datum value, TimestampTz lower,
  TimestampTz upper, bool lower_inc, bool upper_inc)
{
  if (run->held && run->upper == lower &&
      (run->upper_inc || lower_inc) && datum_eq(run->value, value, T_BOOL))
  {
    run->upper = upper;
    run->upper_inc = upper_inc;
    return;
  }
  tdwithin_run_flush(run);
  run->held = true;
  run->value = value;
  run->lower = lower;
  run->upper = upper;
  run->lower_inc = lower_inc;
  run->upper_inc = upper_inc;
  return;
}

/**
 * @brief Set the values at the bounds of the period a turning point function
 * returns for a segment, true at a bound at an end of the segment
 * @details A bound strictly inside the segment takes the value the value
 * function of the lift states for it (#tcbuffersegm_tdwithin_at); without one,
 * every bound is within the distance
 */
static void
tdwithin_bounds_in(const LiftedFunctionInfo *lfinfo, int solutions,
  Datum start1, Datum end1, Datum start2, Datum end2, TimestampTz lower,
  TimestampTz upper, TimestampTz t1, TimestampTz t2, bool *in1, bool *in2)
{
  *in1 = *in2 = true;
  if (solutions == 0 || ! lfinfo->tpfn_temp_at)
    return;
  if (t1 > lower && t1 < upper)
    *in1 = lfinfo->tpfn_temp_at(start1, end1, start2, end2,
      lfinfo->param[0], lower, upper, t1);
  if (t2 == t1)
    *in2 = *in1;
  else if (t2 > lower && t2 < upper)
    *in2 = lfinfo->tpfn_temp_at(start1, end1, start2, end2,
      lfinfo->param[0], lower, upper, t2);
  return;
}

/**
 * @brief Add to a walk the result of the tdwithin function of a segment from
 * the solutions of the quadratic equation found previously
 * @details @p in1 and @p in2 are the values at the bounds @p t1 and @p t2
 * where these lie strictly inside the segment, as the value function of the
 * turning point function states them (#tcbuffersegm_tdwithin_at); a bound at
 * which the values are not within the distance is excluded from the run of
 * true values and added to the neighbouring run of false ones, each run
 * extended as #tdwithin_run_add extends it
 */
static void
tdwithin_add_solutions_run(int solutions, TimestampTz lower,
  TimestampTz upper, bool lower_inc, bool upper_inc, bool upper_inc1,
  TimestampTz t1, TimestampTz t2, bool in1, bool in2, TDwithinRun *run)
{
  const Datum datum_true = BoolGetDatum(true);
  const Datum datum_false = BoolGetDatum(false);
  /* <  F  > */
  if (solutions == 0 ||
  (solutions == 1 && ((t1 == lower && ! lower_inc) ||
    (t1 == upper && ! upper_inc))) ||
  (t1 == t2 && ! in1))
    tdwithin_run_add(run, datum_false, lower, upper, lower_inc, upper_inc1);
  /*
   * A bound strictly inside the segment dated before the values come within
   * the distance, or after they leave it, is an instant at which they are not
   * within, and the run of true values excludes it:
   *  (  F  ](  T  ](  F  )
   */
  else if (! in1 || ! in2)
  {
    if (t1 != lower)
      tdwithin_run_add(run, datum_false, lower, t1, lower_inc, ! in1);
    tdwithin_run_add(run, datum_true, t1, t2,
      (t1 == lower) ? lower_inc : in1, (t2 == upper) ? upper_inc1 : in2);
    if (t2 != upper)
      tdwithin_run_add(run, datum_false, t2, upper, ! in2, upper_inc1);
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
    /* The scratch instants a held sequence is emitted from are the ones this
     * branch overwrites, so the walk emits before writing them */
    tdwithin_run_flush(run);
    int ninsts = 0;
    if (t1 != lower)
      tinstant_set(run->instants[ninsts++], datum_false, lower);
    tinstant_set(run->instants[ninsts++], datum_true, t1);
    if (solutions == 2 && t1 != t2)
      tinstant_set(run->instants[ninsts++], datum_true, t2);
    run->result[run->nseqs++] = tsequence_make(run->instants, ninsts,
      lower_inc, (t2 != upper) ? true : upper_inc1, STEP, NORMALIZE_NO);
    if (t2 != upper)
      tdwithin_run_add(run, datum_false, t2, upper, false, upper_inc1);
  }
  return;
}

/**
 * @brief Construct the result of the tdwithin function of a segment from
 * the solutions of the quadratic equation found previously
 * @return Number of sequences of the result
 */
int
tdwithin_add_solutions(int solutions, TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc, bool upper_inc1, TimestampTz t1,
  TimestampTz t2, TInstant **instants, TSequence **result)
{
  TDwithinRun run = { .result = result, .nseqs = 0, .instants = instants,
    .held = false };
  tdwithin_add_solutions_run(solutions, lower, upper, lower_inc, upper_inc,
    upper_inc1, t1, t2, true, true, &run);
  tdwithin_run_flush(&run);
  return run.nseqs;
}

/**
 * @brief Return the timestamps at which the segments of two temporal point
 * sequences are within a distance (iterator function)
 * @param[in] seq1,seq2 Temporal points
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tlinearseq_tlinearseq_iter(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn,
  TSequence **result)
{
  const TInstant *start1 = TSEQUENCE_INST_N(seq1, 0);
  const TInstant *start2 = TSEQUENCE_INST_N(seq2, 0);
  if (seq1->count == 1)
  {
    TInstant *inst = tinstant_make(tfunc_base_base(tinstant_value_p(start1),
      tinstant_value_p(start2), lfinfo), T_TBOOL, start1->t);
    result[0] = tinstant_to_tsequence_free(inst, STEP);
    return 1;
  }

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  Datum sv1 = tinstant_value_p(start1);
  Datum sv2 = tinstant_value_p(start2);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  MeosType basetype = temptype_basetype(seq1->temptype);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  const Datum datum_true = BoolGetDatum(true);
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, T_TBOOL, lower);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  TDwithinRun run = { .result = result, .nseqs = 0, .instants = instants,
    .held = false };
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
    if (datum_eq(sv1, ev1, basetype) && datum_eq(sv2, ev2, basetype))
      tdwithin_run_add(&run, tfunc_base_base(sv1, sv2, lfinfo), lower, upper,
        lower_inc, upper_inc);
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      Datum sev1 = linear1 ? ev1 : sv1;
      Datum sev2 = linear2 ? ev2 : sv2;
      int solutions = tpfn(sv1, sev1, sv2, sev2, lfinfo->param[0], lower,
        upper, &t1, &t2);
      bool upper_inc1 = linear1 && linear2 && upper_inc;
      bool in1 = true, in2 = true;
      tdwithin_bounds_in(lfinfo, solutions, sv1, sev1, sv2, sev2, lower, upper,
        t1, t2, &in1, &in2);
      tdwithin_add_solutions_run(solutions, lower, upper, lower_inc,
        upper_inc, upper_inc1, t1, t2, in1, in2, &run);
      /* Add extra final point if only one segment is linear */
      if (upper_inc && (! linear1 || ! linear2))
      {
        Datum value = tfunc_base_base(ev1, ev2, lfinfo);
        tdwithin_run_flush(&run);
        tinstant_set(instants[0], value, upper);
        run.result[run.nseqs++] = tinstant_as_tsequence(instants[0], STEP);
      }
    }
    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  tdwithin_run_flush(&run);
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return run.nseqs;
}

/**
 * @brief Return the temporal dwithin relationship between two temporal point
 * sequences
 * @param[in] seq1,seq2 Temporal points
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tlinearseq_tlinearseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq1->count * 4);
  int count = tdwithin_tlinearseq_tlinearseq_iter(seq1, seq2, lfinfo, tpfn,
    sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which the segments of two temporal point
 * sequence sets are within a distance
 * @param[in] ss1,ss2 Temporal points
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tlinearseqset_tlinearseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  /* Singleton sequence set */
  if (ss1->count == 1)
    return tdwithin_tlinearseq_tlinearseq(TSEQUENCESET_SEQ_N(ss1, 0),
      TSEQUENCESET_SEQ_N(ss2, 0), lfinfo, tpfn);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss1->count; i++)
    nseqs += tdwithin_tlinearseq_tlinearseq_iter(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i), lfinfo, tpfn, &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Return the timestamps at which a temporal point sequence and a point
 * are within a distance (iterator function)
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the resulting array
 */
static int
tdwithin_tlinearseq_base_iter(const TSequence *seq, Datum point,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn, TSequence **result)
{
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  Datum startvalue = tinstant_value_p(start);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(tfunc_base_base(startvalue, point,
      lfinfo), T_TBOOL, start->t);
    result[0] = tinstant_to_tsequence_free(inst, STEP);
    return 1;
  }

  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  TimestampTz lower = start->t;
  bool lower_inc = seq->period.lower_inc;
  MeosType basetype = temptype_basetype(seq->temptype);
  /* We create three temporal instants with arbitrary values that are set in
   * the for loop to avoid creating and freeing the instants each time a
   * segment of the result is computed */
  const Datum datum_true = BoolGetDatum(true);
  TInstant *instants[3];
  instants[0] = tinstant_make(datum_true, T_TBOOL, lower);
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  TDwithinRun run = { .result = result, .nseqs = 0, .instants = instants,
    .held = false };
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the for loop adds between one and three sequences */
    const TInstant *end = TSEQUENCE_INST_N(seq, i);
    Datum endvalue = tinstant_value_p(end);
    TimestampTz upper = end->t;
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;

    /* Segment is constant or has step interpolation */
    if (datum_eq(startvalue, endvalue, basetype))
      tdwithin_run_add(&run, tfunc_base_base(startvalue, point, lfinfo),
        lower, upper, lower_inc, upper_inc);
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      int solutions = tpfn(startvalue, endvalue, point, point,
        lfinfo->param[0], lower,
        upper, &t1, &t2);
      bool upper_inc1 = linear && upper_inc;
      bool in1 = true, in2 = true;
      tdwithin_bounds_in(lfinfo, solutions, startvalue, endvalue, point, point,
        lower, upper, t1, t2, &in1, &in2);
      tdwithin_add_solutions_run(solutions, lower, upper, lower_inc,
        upper_inc, upper_inc1, t1, t2, in1, in2, &run);
    }
    startvalue = endvalue;
    lower = upper;
    lower_inc = true;
  }
  tdwithin_run_flush(&run);
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  return run.nseqs;
}

/**
 * @brief Return the timestamps at which a temporal point sequence and a point
 * are within a distance
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 */
static TSequenceSet *
tdwithin_tlinearseq_base(const TSequence *seq, Datum point,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * 4);
  int count = tdwithin_tlinearseq_base_iter(seq, point, lfinfo, tpfn,
    sequences);
  /* We are sure that nseqs > 0 since the point is non-empty */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the timestamps at which a temporal point sequence set and a
 * point are within a distance
 * @param[in] ss Temporal point
 * @param[in] point Point
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 */
static TSequenceSet *
tdwithin_tlinearseqset_base(const TSequenceSet *ss, Datum point,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tdwithin_tlinearseq_base(TSEQUENCESET_SEQ_N(ss, 0), point, lfinfo,
      tpfn);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount * 4);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tdwithin_tlinearseq_base_iter(TSEQUENCESET_SEQ_N(ss, i), point,
      lfinfo, tpfn, &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a spatiotemporal value
 * and a base value are within a distance
 * @param[in] temp Spatiotemporal value
 * @param[in] base Base value
 * @param[in] lfinfo Function to be applied and its parameters, the distance
 * first
 * @param[in] tpfn Turning point function to be applied
 * @csqlfn #Tdwithin_tgeo_geo(), #Tdwithin_tcbuffer_cbuffer(), ...
 * @note The function assumes that all validity tests have been previously done
 */
Temporal *
tdwithin_tspatial_spatial(const Temporal *temp, Datum base,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  assert(temp); assert(DatumGetPointer(base));
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      Datum value = tinstant_value_p((TInstant *) temp);
      result = (Temporal *) tinstant_make(tfunc_base_base(value, base,
        lfinfo), T_TBOOL, ((TInstant *) temp)->t);
      break;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tlinearseq_base((TSequence *) temp,
            base, lfinfo, tpfn);
      else
      {
        result = tspatialrel_tspatial_base(temp, base, lfinfo->param,
          lfinfo->func, lfinfo->numparam, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
      if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
        result = (Temporal *) tdwithin_tlinearseqset_base(
          (TSequenceSet *) temp, base, lfinfo, tpfn);
      else
      {
        result = tspatialrel_tspatial_base(temp, base, lfinfo->param,
          lfinfo->func, lfinfo->numparam, INVERT_NO);
      }
  }
  return result;
}

/*****************************************************************************
 * Temporal dwithin of temporal geography points
 *****************************************************************************/

/**
 * @brief Maximum number of crossings of a distance that the geodetic distance
 * of two geography point segments may have in one segment
 */
#define GEOG_DWITHIN_MAXCROSS 64

/**
 * @brief Two geography point segments over the same period
 * @details A geography point segment moves along the great circle of its ends
 * at a constant angular speed, where #interpolate_point4d_spheroid places it
 * for #pointsegm_interpolate: at the fraction f of the segment it stands at
 * A cos(om f) + C sin(om f)
 * on the unit sphere, A being its start, C the unit vector orthogonal to A in
 * the plane of the great circle toward its end, and om the angle between its
 * ends. A constant segment has om = 0.
 */
typedef struct
{
  POINT2D s1, e1;   /**< Ends of the first segment, longitude and latitude */
  POINT2D s2, e2;   /**< Ends of the second segment */
  POINT3D A1, C1;   /**< Great circle of the first segment */
  POINT3D A2, C2;   /**< Great circle of the second segment */
  double om1, om2;  /**< Angles the segments sweep */
  SPHEROID s;       /**< Model of the earth on which the distance is measured */
  bool sphere;      /**< True when the distance is measured on the sphere */
} GeogSegmPair;

/**
 * @brief Set the great circle on which a geography point segment moves
 * @return False when the ends of the segment are antipodal, which states no
 * great circle
 */
static bool
geogsegm_circle(const POINT2D *p1, const POINT2D *p2, POINT3D *A, POINT3D *C,
  double *om)
{
  GEOGRAPHIC_POINT g1, g2;
  POINT3D B;
  geographic_point_init(p1->x, p1->y, &g1);
  geographic_point_init(p2->x, p2->y, &g2);
  geog2cart(&g1, A);
  geog2cart(&g2, &B);
  double dot = A->x * B.x + A->y * B.y + A->z * B.z;
  double crx = A->y * B.z - A->z * B.y;
  double cry = A->z * B.x - A->x * B.z;
  double crz = A->x * B.y - A->y * B.x;
  double crn = sqrt(crx * crx + cry * cry + crz * crz);
  C->x = B.x - dot * A->x;
  C->y = B.y - dot * A->y;
  C->z = B.z - dot * A->z;
  double norm = sqrt(C->x * C->x + C->y * C->y + C->z * C->z);
  if (crn == 0.0 || norm == 0.0)
  {
    C->x = C->y = C->z = 0.0;
    *om = 0.0;
    return dot > 0.0;
  }
  C->x /= norm; C->y /= norm; C->z /= norm;
  *om = atan2(crn, dot);
  return true;
}

/**
 * @brief Return the derivative with respect to the fraction of the segments of
 * the cosine of the angle between the positions of two geography point
 * segments, the positions being P(f) = A cos(om f) + C sin(om f)
 */
static double
geogsegmpair_dcos(const GeogSegmPair *sp, double f)
{
  double c1 = cos(sp->om1 * f), s1 = sin(sp->om1 * f);
  double c2 = cos(sp->om2 * f), s2 = sin(sp->om2 * f);
  POINT3D P1 = { sp->A1.x * c1 + sp->C1.x * s1, sp->A1.y * c1 + sp->C1.y * s1,
    sp->A1.z * c1 + sp->C1.z * s1 };
  POINT3D P2 = { sp->A2.x * c2 + sp->C2.x * s2, sp->A2.y * c2 + sp->C2.y * s2,
    sp->A2.z * c2 + sp->C2.z * s2 };
  POINT3D D1 = { sp->om1 * (sp->C1.x * c1 - sp->A1.x * s1),
    sp->om1 * (sp->C1.y * c1 - sp->A1.y * s1),
    sp->om1 * (sp->C1.z * c1 - sp->A1.z * s1) };
  POINT3D D2 = { sp->om2 * (sp->C2.x * c2 - sp->A2.x * s2),
    sp->om2 * (sp->C2.y * c2 - sp->A2.y * s2),
    sp->om2 * (sp->C2.z * c2 - sp->A2.z * s2) };
  return D1.x * P2.x + D1.y * P2.y + D1.z * P2.z +
    P1.x * D2.x + P1.y * D2.y + P1.z * D2.z;
}

/**
 * @brief Return the distance between the positions of two geography point
 * segments at a fraction of the segments, measured as #geog_dwithin measures
 * the distance of two points
 */
static double
geogsegmpair_dist(const GeogSegmPair *sp, double f)
{
  POINT4D s1 = { sp->s1.x, sp->s1.y, 0.0, 0.0 };
  POINT4D e1 = { sp->e1.x, sp->e1.y, 0.0, 0.0 };
  POINT4D s2 = { sp->s2.x, sp->s2.y, 0.0, 0.0 };
  POINT4D e2 = { sp->e2.x, sp->e2.y, 0.0, 0.0 };
  POINT4D p1, p2;
  GEOGRAPHIC_POINT g1, g2;
  interpolate_point4d_spheroid(&s1, &e1, &p1, NULL, f);
  interpolate_point4d_spheroid(&s2, &e2, &p2, NULL, f);
  geographic_point_init(p1.x, p1.y, &g1);
  geographic_point_init(p2.x, p2.y, &g2);
  return sp->sphere ? sp->s.radius * sphere_distance(&g1, &g2) :
    spheroid_distance(&g1, &g2, &sp->s);
}

/**
 * @brief Return in @p crit the fractions of the segments, in increasing order,
 * at which the angle between the positions of two geography point segments
 * stops increasing or decreasing
 * @details The cosine g of the angle is the sum of two sinusoids of the
 * fraction, of amplitude Rs and frequency om1 + om2 and of amplitude Rd and
 * frequency |om1 - om2|, so that its second derivative is bounded by
 * L = Rs (om1 + om2)^2 + Rd (om1 - om2)^2. An interval [lo, hi] on which the
 * derivative at the middle exceeds L (hi - lo) / 2 in absolute value holds no
 * zero of the derivative, so that splitting the other intervals isolates
 * every zero. The planar turning-point functions #tpointsegm_tdwithin_turnpt
 * and #tcbuffersegm_length_roots solve a quadratic with at most two roots;
 * no closed form bounds the number of zeros here, hence the isolation.
 * @return Number of fractions found, -1 when they cannot be isolated
 */
static int
geogsegmpair_critical(const GeogSegmPair *sp, double *crit, int maxcrit)
{
  /* With P(f) = A cos(om f) + C sin(om f), the product P1.P2 is
   * ((AA + CC) cos(d) + (CA - AC) sin(d) + (AA - CC) cos(s) + (AC + CA) sin(s))
   * / 2, where d and s are the difference and the sum of the two angles */
  double AA = sp->A1.x * sp->A2.x + sp->A1.y * sp->A2.y + sp->A1.z * sp->A2.z;
  double CC = sp->C1.x * sp->C2.x + sp->C1.y * sp->C2.y + sp->C1.z * sp->C2.z;
  double AC = sp->A1.x * sp->C2.x + sp->A1.y * sp->C2.y + sp->A1.z * sp->C2.z;
  double CA = sp->C1.x * sp->A2.x + sp->C1.y * sp->A2.y + sp->C1.z * sp->A2.z;
  double Rd = hypot(AA + CC, CA - AC) / 2, Rs = hypot(AA - CC, AC + CA) / 2;
  double ws = sp->om1 + sp->om2, wd = sp->om1 - sp->om2;
  double L = Rs * ws * ws + Rd * wd * wd;
  if (L == 0.0)
    return 0;
  /* The evaluation of the derivative carries an error bounded by noise, the
   * angles it subtracts cancelling near a zero, so that an interval is set
   * aside only when the derivative at its middle exceeds the bound by more
   * than the error, and the zeros are located to the width over which the
   * error hides the sign of the derivative */
  double noise = 64 * DBL_EPSILON * (sp->om1 + sp->om2);
  double leafw = fmin(1.0 / 64, fmax(1e-12, 4 * noise / L));
  /* A distance constant over the segment has no critical point: the
   * derivative is then zero up to the error of its evaluation */
  bool flat = true;
  for (int i = 0; i <= 8 && flat; i++)
    flat = fabs(geogsegmpair_dcos(sp, i / 8.0)) <= noise;
  if (flat)
    return 0;
  double stlo[128], sthi[128];
  int top = 0, ncrit = 0, nvisit = 0;
  stlo[top] = 0.0; sthi[top++] = 1.0;
  while (top > 0)
  {
    top--;
    double lo = stlo[top], hi = sthi[top];
    if (++nvisit > 1000000)
      return -1;
    double mid = lo + (hi - lo) / 2, half = (hi - lo) / 2;
    if (fabs(geogsegmpair_dcos(sp, mid)) > L * half + noise)
      continue;
    if (hi - lo <= leafw)
    {
      if (ncrit == 0 || mid - crit[ncrit - 1] > 2 * leafw)
      {
        if (ncrit == maxcrit)
          return -1;
        crit[ncrit++] = mid;
      }
      continue;
    }
    if (top + 2 > 128)
      return -1;
    /* The upper half is pushed first so that the fractions come in order */
    stlo[top] = mid; sthi[top++] = hi;
    stlo[top] = lo; sthi[top++] = mid;
  }
  return ncrit;
}

/**
 * @brief Return the fraction at which the distance between two geography point
 * segments reaches its extremum within a window, by a golden-section search
 * @param[in] sp Segments
 * @param[in] a,b Window, on which the distance has a single extremum
 * @param[in] min True for a minimum, false for a maximum
 */
static double
geogsegmpair_extremum(const GeogSegmPair *sp, double a, double b, bool min)
{
  const double ratio = 0.6180339887498949;
  double x1 = b - ratio * (b - a), x2 = a + ratio * (b - a);
  double f1 = geogsegmpair_dist(sp, x1), f2 = geogsegmpair_dist(sp, x2);
  for (int i = 0; i < 100 && b - a > 4 * DBL_EPSILON; i++)
  {
    if (min ? f1 <= f2 : f1 >= f2)
    {
      b = x2; x2 = x1; f2 = f1;
      x1 = b - ratio * (b - a);
      f1 = geogsegmpair_dist(sp, x1);
    }
    else
    {
      a = x1; x1 = x2; f1 = f2;
      x2 = a + ratio * (b - a);
      f2 = geogsegmpair_dist(sp, x2);
    }
  }
  return a + (b - a) / 2;
}

/**
 * @brief Return in @p cross the fractions of the segments, in increasing order,
 * at which the distance between two geography point segments crosses a
 * distance, n of them in general
 * @details The distance is monotone between two critical points of the angle
 * between the positions, each critical point being moved to the extremum of
 * the distance measured on the model of the earth near it. On each monotone
 * piece whose ends lie on different sides of the distance, the crossing is
 * bisected until its two bounds are consecutive doubles, on the predicate
 * distance <= dist that #geog_dwithin decides, and the bound within the
 * distance is kept.
 * @return Number of crossings, -1 when the critical points cannot be isolated
 */
static int
geogsegmpair_crossings(const GeogSegmPair *sp, double dist, double *cross)
{
  double crit[GEOG_DWITHIN_MAXCROSS];
  int ncrit = geogsegmpair_critical(sp, crit, GEOG_DWITHIN_MAXCROSS - 2);
  if (ncrit < 0)
    return -1;
  double split[GEOG_DWITHIN_MAXCROSS];
  int nsplit = 0;
  split[nsplit++] = 0.0;
  for (int k = 0; k < ncrit; k++)
  {
    double prev = (k == 0) ? 0.0 : crit[k - 1];
    double next = (k == ncrit - 1) ? 1.0 : crit[k + 1];
    double a = prev + (crit[k] - prev) / 2, b = crit[k] + (next - crit[k]) / 2;
    double da = geogsegmpair_dist(sp, a), db = geogsegmpair_dist(sp, b);
    double dc = geogsegmpair_dist(sp, crit[k]);
    double c = crit[k];
    if (dc <= da && dc <= db)
      c = geogsegmpair_extremum(sp, a, b, true);
    else if (dc >= da && dc >= db)
      c = geogsegmpair_extremum(sp, a, b, false);
    if (c > split[nsplit - 1] && c < 1.0)
      split[nsplit++] = c;
  }
  split[nsplit++] = 1.0;

  int ncross = 0;
  bool in0 = geogsegmpair_dist(sp, 0.0) <= dist;
  for (int k = 0; k < nsplit - 1; k++)
  {
    double lo = split[k], hi = split[k + 1];
    bool inhi = geogsegmpair_dist(sp, hi) <= dist;
    if (in0 != inhi)
    {
      bool inlo = in0;
      for (int i = 0; i < 200; i++)
      {
        double mid = lo + (hi - lo) / 2;
        if (mid <= lo || mid >= hi)
          break;
        if ((geogsegmpair_dist(sp, mid) <= dist) == inlo)
          lo = mid;
        else
          hi = mid;
      }
      if (ncross == GEOG_DWITHIN_MAXCROSS)
        return -1;
      cross[ncross++] = inlo ? lo : hi;
    }
    in0 = inhi;
  }
  return ncross;
}

/**
 * @brief Add to a walk the result of the temporal dwithin of one segment of
 * two geography points, from the crossings of the distance in the segment
 * @details Each crossing is an instant at which the two points are within the
 * distance, dated by truncation as #tpointsegm_tdwithin_turnpt dates one, and
 * the value between two consecutive instants is the one at their middle
 */
static bool
tdwithin_geogsegm_run(const GeogSegmPair *sp, double dist,
  TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc,
  double dend, TDwithinRun *run)
{
  const Datum datum_true = BoolGetDatum(true);
  double cross[GEOG_DWITHIN_MAXCROSS];
  int ncross = geogsegmpair_crossings(sp, dist, cross);
  if (ncross < 0)
    return false;
  double duration = (double) (upper - lower);
  if (lower_inc)
    tdwithin_run_add(run, BoolGetDatum(geogsegmpair_dist(sp, 0.0) <= dist),
      lower, lower, true, true);
  TimestampTz prev = lower;
  for (int k = 0; k < ncross; k++)
  {
    TimestampTz t = lower + (TimestampTz) (duration * cross[k]);
    if (t <= prev || t >= upper)
      continue;
    double fmid = (double) ((prev + (t - prev) / 2) - lower) / duration;
    tdwithin_run_add(run, BoolGetDatum(geogsegmpair_dist(sp, fmid) <= dist),
      prev, t, false, false);
    tdwithin_run_add(run, datum_true, t, t, true, true);
    prev = t;
  }
  double fmid = (double) ((prev + (upper - prev) / 2) - lower) / duration;
  tdwithin_run_add(run, BoolGetDatum(geogsegmpair_dist(sp, fmid) <= dist),
    prev, upper, false, false);
  if (upper_inc)
    tdwithin_run_add(run, BoolGetDatum(dend <= dist), upper, upper, true,
      true);
  return true;
}

/**
 * @brief Return the longitude and latitude of the point of an instant of a
 * temporal geography point
 */
static inline const POINT2D *
tgeogpointinst_point2d(const TInstant *inst)
{
  return GSERIALIZED_POINT2D_P(DatumGetGserializedP(tinstant_value_p(inst)));
}

/**
 * @brief Return a temporal Boolean that states whether a temporal geography
 * point and a geography point, or two synchronized temporal geography points,
 * are within a distance, from the crossings of their geodetic distance
 * @param[in] temp1 Temporal geography point with linear interpolation, or
 * one of two synchronized temporal geography points
 * @param[in] temp2 The other synchronized temporal geography point @nullable
 * @param[in] gs Geography point when @p temp2 is NULL
 * @param[in] dist Distance
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere, as #geog_dwithin measures
 * @pre The temporal values are sequences or sequence sets
 */
static Temporal *
tdwithin_tgeogpoint_geogpoint(const Temporal *temp1, const Temporal *temp2,
  const GSERIALIZED *gs, double dist, bool spheroid)
{
  assert(temp1); assert((temp2 == NULL) != (gs == NULL));
  assert(MEOS_FLAGS_GET_GEODETIC(temp1->flags)); assert(dist >= 0.0);
  GeogSegmPair sp;
  memset(&sp, 0, sizeof(GeogSegmPair));
  spheroid_init_from_srid(tspatial_srid(temp1), &sp.s);
  if (! spheroid)
    sp.s.a = sp.s.b = sp.s.radius;
  sp.sphere = (sp.s.a == sp.s.b);
  int nseqs1 = (temp1->subtype == TSEQUENCE) ? 1 :
    ((const TSequenceSet *) temp1)->count;
  int totalcount = (temp1->subtype == TSEQUENCE) ?
    ((const TSequence *) temp1)->count :
    ((const TSequenceSet *) temp1)->totalcount;
  int size = totalcount * 4 + 4;
  TSequence **sequences = palloc(sizeof(TSequence *) * size);
  TInstant *instants[3];
  instants[0] = tinstant_make(BoolGetDatum(true), T_TBOOL,
    temporal_start_timestamptz(temp1));
  instants[1] = tinstant_copy(instants[0]);
  instants[2] = tinstant_copy(instants[0]);
  TDwithinRun run = { .result = sequences, .nseqs = 0, .instants = instants,
    .held = false };
  const POINT2D *pt = gs ? GSERIALIZED_POINT2D_P(gs) : NULL;
  bool ok = true;
  for (int j = 0; j < nseqs1 && ok; j++)
  {
    const TSequence *seq1 = (temp1->subtype == TSEQUENCE) ?
      (const TSequence *) temp1 : TSEQUENCESET_SEQ_N((TSequenceSet *) temp1, j);
    const TSequence *seq2 = ! temp2 ? NULL : (temp2->subtype == TSEQUENCE) ?
      (const TSequence *) temp2 : TSEQUENCESET_SEQ_N((TSequenceSet *) temp2, j);
    bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
    bool linear2 = seq2 && MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
    /* Room for the pieces of the segments of the sequence */
    int need = run.nseqs + seq1->count * (2 * GEOG_DWITHIN_MAXCROSS + 4);
    if (need > size)
    {
      size = need;
      sequences = repalloc(sequences, sizeof(TSequence *) * size);
      run.result = sequences;
    }
    for (int i = 0; i < seq1->count && ok; i++)
    {
      const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
      const TInstant *inst2 = seq2 ? TSEQUENCE_INST_N(seq2, i) : NULL;
      if (seq1->count == 1)
      {
        sp.s1 = sp.e1 = *tgeogpointinst_point2d(inst1);
        sp.s2 = sp.e2 = inst2 ? *tgeogpointinst_point2d(inst2) : *pt;
        tdwithin_run_add(&run, BoolGetDatum(geogsegmpair_dist(&sp, 0.0) <=
          dist), inst1->t, inst1->t, true, true);
        break;
      }
      if (i == seq1->count - 1)
        break;
      const TInstant *end1 = TSEQUENCE_INST_N(seq1, i + 1);
      const TInstant *end2 = seq2 ? TSEQUENCE_INST_N(seq2, i + 1) : NULL;
      sp.s1 = *tgeogpointinst_point2d(inst1);
      sp.e1 = linear1 ? *tgeogpointinst_point2d(end1) : sp.s1;
      sp.s2 = inst2 ? *tgeogpointinst_point2d(inst2) : *pt;
      sp.e2 = (inst2 && linear2) ? *tgeogpointinst_point2d(end2) : sp.s2;
      bool upper_inc = (i == seq1->count - 2) ? seq1->period.upper_inc : false;
      /* The distance at the end of the last segment is the one of the values
       * of the two instants, which a step segment reaches only there */
      double dend = 0.0;
      if (upper_inc)
      {
        GeogSegmPair last = sp;
        last.s1 = last.e1 = *tgeogpointinst_point2d(end1);
        last.s2 = last.e2 = end2 ? *tgeogpointinst_point2d(end2) : *pt;
        dend = geogsegmpair_dist(&last, 0.0);
      }
      if (! geogsegm_circle(&sp.s1, &sp.e1, &sp.A1, &sp.C1, &sp.om1) ||
          ! geogsegm_circle(&sp.s2, &sp.e2, &sp.A2, &sp.C2, &sp.om2))
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "A segment between antipodal positions states no great circle");
        ok = false;
        break;
      }
      bool lower_inc = (i == 0) ? seq1->period.lower_inc : true;
      if (! tdwithin_geogsegm_run(&sp, dist, inst1->t, end1->t, lower_inc,
          upper_inc, dend, &run))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "The crossings of the distance of two geography points could not "
          "be isolated");
        ok = false;
      }
    }
  }
  tdwithin_run_flush(&run);
  pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
  if (! ok)
  {
    pfree_array((void **) run.result, run.nseqs);
    return NULL;
  }
  return (Temporal *) tsequenceset_make_free(run.result, run.nseqs, NORMALIZE);
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
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere, read for geographies only, as #edwithin_tgeo_geo reads it
 * @csqlfn #Tdwithin_tgeo_geo()
 * @note A temporal geography point is answered against a geography point
 * only, from the crossings of their geodetic distance, which a segment may
 * cross several times
 */
Temporal *
tdwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool spheroid)
{
  /* Ensure the validity of the arguments. ensure_valid_tspatial_geo
   * already enforces that the temporal geo and the geometry have the
   * same geodetic flag, so geodetic coordinates are supported here the
   * same way as in #Tdistance_tgeo_geo. A temporal geography point against
   * a geography other than a point is rejected; non-point geometries are
   * routed through the polygonal buffer below. */
  if (! ensure_finite(dist))
    return NULL;
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      (tpoint_type(temp->temptype) && gserialized_get_type(gs) != POINTTYPE &&
        ! ensure_not_geodetic_geo(gs)) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  /* A temporal geography point moving along great circles and a geography
   * point are within the distance between the crossings of their geodetic
   * distance; at a distance of zero they are where they are equal */
  if (tpoint_type(temp->temptype) && MEOS_FLAGS_GET_GEODETIC(temp->flags) &&
      temp->subtype != TINSTANT && MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return (dist == 0.0) ?
      tcomp_temporal_base(temp, PointerGetDatum(gs), &datum2_eq) :
      tdwithin_tgeogpoint_geogpoint(temp, NULL, gs, dist, spheroid);

  /* For a temporal point against a non-point geometry, prefer the native
   * within engine when the temporal geometry point has linear interpolation,
   * is planar and 2D, and the geometry is one the clip engine supports
   * (this includes curved geometries handled by the arc kernel).
   * The native path solves the per-segment within-distance sub-intervals in
   * closed form, mirroring the merged temporal circular-buffer engine
   * specialized to a moving point. Otherwise (temporal geographies, step or
   * discrete interpolation, 3D, TIN/polyhedral or other unsupported types)
   * route through tIntersects on a polygonal d-expanded buffer of gs, the
   * polygonal approximation that the always-quantifier branch of
   * ea_dwithin_tgeo_geo also uses internally via geom_buffer. */
  if (tpoint_type(temp->temptype) && gserialized_get_type(gs) != POINTTYPE)
  {
    if (temp->temptype == T_TGEOMPOINT && temp->subtype != TINSTANT &&
        MEOS_FLAGS_GET_INTERP(temp->flags) == LINEAR &&
        ! MEOS_FLAGS_GET_GEODETIC(temp->flags) &&
        ! MEOS_FLAGS_GET_Z(temp->flags) && ! FLAGS_GET_Z(gs->gflags))
    {
      LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
      bool supported = geom_meos_coverage(lwgeom) == 1;
      lwgeom_free(lwgeom);
      if (supported)
        return tpoint_linear_dwithin_geom(temp, gs, dist);
    }
    /* A zero distance is the intersection relationship, which the buffer below
     * cannot express: the buffer of a geometry of a lower dimension than a
     * surface by a zero distance is empty, and an empty geometry carries no
     * relationship */
    if (dist == 0.0)
      return tinterrel_tgeo_geo(temp, gs, TINTERSECTS);

    GSERIALIZED *buffer = geom_buffer(gs, dist, "");
    Temporal *result = tinterrel_tgeo_geo(temp, buffer, TINTERSECTS);
    pfree(buffer);
    return result;
  }

  /* Determine the distance and the turning point functions to be applied.
   * The selectors choose the geodetic dwithin for geodetic coordinates and the
   * 2D/3D planar dwithin otherwise, mirroring how Tdistance_tgeo_geo selects
   * its distance function. A temporal point against a point geometry carries a
   * point pair at every instant, whose distance is a question about the four
   * coordinates, so it takes the exact sign of #point_within_distance_sign
   * rather than the generic any-geometry entry; this is the twin of the choice Tdwithin_tgeo_tgeo
   * makes for two temporal points. The geometry here is never empty, the
   * validity check above returning on an empty one. */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  geo_dwithin_lfinfo_geo(temp->flags, gs->gflags, dist, spheroid, &lfinfo);
  if (tpoint_type(temp->temptype) && gserialized_get_type(gs) == POINTTYPE)
    lfinfo.func = pt_dwithin_fn_geo(temp->flags, gs->gflags);
  lfinfo.tpfn_temp_at = &tpointsegm_tdwithin_at;
  /* Call the generic function passing the two functions as arguments */
  return tdwithin_tspatial_spatial(temp, PointerGetDatum(gs), &lfinfo,
    &tpointsegm_tdwithin_turnpt);
}

/**
 * @ingroup meos_geo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal geo and a
 * geometry are within a distance
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere, read for geographies only, as #tdwithin_tgeo_geo reads it
 * @csqlfn #Tdwithin_geo_tgeo()
 */
Temporal *
tdwithin_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, double dist,
  bool spheroid)
{
  if (! ensure_finite(dist))
    return NULL;
  return tdwithin_tgeo_geo(temp, gs, dist, spheroid);
}

/*****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether two temporal geos
 * are within a distance
 * @pre The spatiotemporal values are synchronized.
 */
Temporal *
tdwithin_tspatial_tspatial(const Temporal *sync1, const Temporal *sync2,
  LiftedFunctionInfo *lfinfo, tpfunc_temp tpfn)
{
  assert(sync1); assert(sync2);
  Temporal *result;
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
    {
      Datum value1 = tinstant_value_p((TInstant *) sync1);
      Datum value2 = tinstant_value_p((TInstant *) sync2);
      result = (Temporal *) tinstant_make(tfunc_base_base(value1, value2,
        lfinfo), T_TBOOL, ((TInstant *) sync1)->t);
      break;
    }
    case TSEQUENCE:
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(sync1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(sync2->flags);
      if (interp1 == LINEAR || interp2 == LINEAR)
        result = (Temporal *) tdwithin_tlinearseq_tlinearseq(
          (TSequence *) sync1, (TSequence *) sync2, lfinfo, tpfn);
      else
      {
        /* Both sequences have either discrete or step interpolation */
        result = tspatialrel_tspatial_tspatial(sync1, sync2, lfinfo->param,
          lfinfo->func, lfinfo->numparam, INVERT_NO);
      }
      break;
    }
    default: /* TSEQUENCESET */
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(sync1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(sync2->flags);
      if (interp1 == LINEAR || interp2 == LINEAR)
        result = (Temporal *) tdwithin_tlinearseqset_tlinearseqset(
          (TSequenceSet *) sync1, (TSequenceSet *) sync2, lfinfo, tpfn);
      else
      {
        /* Both sequence sets have step interpolation */
        result = tspatialrel_tspatial_tspatial(sync1, sync2, lfinfo->param,
          lfinfo->func, lfinfo->numparam, INVERT_NO);
      }
    }
  }
  return result;
}

/*****************************************************************************/

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
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere, read for geographies only, as #edwithin_tgeo_tgeo reads it
 * @csqlfn #Tdwithin_tgeo_tgeo()
 * @note Two temporal geography points are answered from the crossings of
 * their geodetic distance, which a pair of segments may cross several times
 */
Temporal *
tdwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist,
  bool spheroid)
{
  /* Ensure the validity of the arguments */
  if (! ensure_finite(dist))
    return NULL;
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  Temporal *sync1, *sync2;
  /* Return false if the temporal geos do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  /* Two temporal geography points, one of which moves along great circles,
   * are within the distance between the crossings of their geodetic
   * distance */
  if (tpoint_type(sync1->temptype) && MEOS_FLAGS_GET_GEODETIC(sync1->flags) &&
      sync1->subtype != TINSTANT && (MEOS_FLAGS_LINEAR_INTERP(sync1->flags) ||
        MEOS_FLAGS_LINEAR_INTERP(sync2->flags)))
  {
    /* At a distance of zero they are where they are equal */
    Temporal *result = (dist == 0.0) ?
      tcomp_temporal_temporal(temp1, temp2, &datum2_eq) :
      tdwithin_tgeogpoint_geogpoint(sync1, sync2, NULL, dist, spheroid);
    pfree(sync1); pfree(sync2);
    return result;
  }

  /* Call the generic function passing the distance and the turning point
   * functions to be applied */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  geo_dwithin_lfinfo(sync1->flags, sync2->flags, dist, spheroid, &lfinfo);
  /* A temporal point carries a point at every instant, so the pair is answered
   * by the exact sign of #point_within_distance_sign rather than by the
   * generic any-geometry entry */
  if (tpoint_type(sync1->temptype) && tpoint_type(sync2->temptype))
    lfinfo.func = pt_dwithin_fn(sync1->flags, sync2->flags);
  lfinfo.tpfn_temp_at = &tpointsegm_tdwithin_at;
  Temporal *result = tdwithin_tspatial_tspatial(sync1, sync2, &lfinfo,
    &tpointsegm_tdwithin_turnpt);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************/
