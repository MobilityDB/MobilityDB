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
 * @brief Basic functions for temporal network points
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Return true if temporal network point and a network point have a
 * common route identifier
 */
bool
common_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  assert(temp); assert(np);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(np->rid));
  pfree(routes);
  return result;
}

/**
 * @brief Return true if temporal network point and a network point have a
 * common route identifier
 */
bool
common_rid_tnpoint_npointset(const Temporal *temp, const Set *s)
{
  assert(temp); assert(s); assert(s->settype == T_NPOINTSET);
  Set *routes1 = tnpoint_routes(temp);
  Set *routes2 = npointset_routes(s);
  bool result = overlaps_set_set(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/**
 * @brief Ensure that two temporal network point instants have the same route
 * identifier
 */
bool
ensure_same_rid_tnpointinst(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2);
  if (tnpointinst_route(inst1) != tnpointinst_route(inst2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "All network points composing a temporal sequence must have same route identifier");
    return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Return true if a temporal network point and a network point are
 * valid for operations
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 */
bool
ensure_valid_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(np, false);
  if (! ensure_same_srid(tspatial_srid(temp), npoint_srid(np)))
    return false;
  return true;
}

/**
 * @brief Return true if a temporal network point and a network point set are
 * valid for operations
 * @param[in] temp Temporal network point
 * @param[in] s Network point set
 */
bool
ensure_valid_tnpoint_npointset(const Temporal *temp, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_NPOINTSET(s, false);
  if (! ensure_same_srid(tspatial_srid(temp), spatialset_srid(s)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal network point and a geometry
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 */
bool
ensure_valid_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal network point and a
 * spatiotemporal box
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 */
bool
ensure_valid_tnpoint_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) || 
      ! ensure_same_srid(tspatial_srid(temp), box->srid))
    return false;
  return true;
}

/**
 * @brief Return true if a temporal network point and a network point are
 * valid for operations
 * @param[in] temp1,temp2 Temporal network points
 */
bool
ensure_valid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, false); VALIDATE_TNPOINT(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return false;
  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Return 1 or 2 if two temporal network point segments intersect
 * during the period defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tnpointsegm_intersection(Datum start1, Datum end1, Datum start2, Datum end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  /* Convert the network points into geometric points */
  GSERIALIZED *sv1 = npoint_to_geompoint(DatumGetNpointP(start1));
  GSERIALIZED *ev1 = npoint_to_geompoint(DatumGetNpointP(end1));
  GSERIALIZED *sv2 = npoint_to_geompoint(DatumGetNpointP(start2));
  GSERIALIZED *ev2 = npoint_to_geompoint(DatumGetNpointP(end2));
  int result = tgeompointsegm_intersection(PointerGetDatum(sv1),
    PointerGetDatum(ev1), PointerGetDatum(sv2), PointerGetDatum(ev2),
    lower, upper, t1, t2);
  pfree(sv1); pfree(ev1); pfree(sv2); pfree(ev2);
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_npoint_inout
 * @brief Return a temporal network point from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tnpoint_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TNPOINT);
}

/**
 * @ingroup meos_npoint_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * network point
 * @param[in] temp Temporal network point
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tnpoint_out(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);
  return temporal_out(temp, maxdd);
}

/**
 * @ingroup meos_internal_npoint_inout
 * @brief Return a temporal network point instant from its Well-Known Text 
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tnpointinst_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tnpoint_in(str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_npoint_inout
 * @brief Return a temporal network point sequence from its Well-Known Text 
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tnpointseq_in(const char *str, interpType interp UNUSED)
{
  /* Call the superclass function */
  Temporal *temp = tnpoint_in(str);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_npoint_inout
 * @brief Return a temporal network point sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tnpointseqset_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tnpoint_in(str);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_npoint_constructor
 * @brief Return a temporal network point instant from a network point and a
 * timestamptz
 * @param[in] np Network point
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tnpointinst_make(const Npoint *np, TimestampTz t)
{
  return tinstant_make(PointerGetDatum(np), T_TNPOINT, t);
}
#endif /* MEOS */

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Convert a temporal network point into a temporal geometry point
 */
TInstant *
tnpointinst_tgeompointinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TNPOINT);
  GSERIALIZED *geom =
    npoint_to_geompoint(DatumGetNpointP(tinstant_value_p(inst)));
  return tinstant_make_free(PointerGetDatum(geom), T_TGEOMPOINT, inst->t);
}

/**
 * @brief Convert a temporal network point into a temporal geometry point
 */
TSequence *
tnpointseq_tgeompointseq_disc(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TNPOINT);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tnpointinst_tgeompointinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Convert a temporal network point into a temporal geometry point
 */
TSequence *
tnpointseq_tgeompointseq_cont(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TNPOINT);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  /* We are sure line is not empty */
  GSERIALIZED *line = route_geom(np->rid);
  int32_t srid = gserialized_get_srid(line);
  LWLINE *lwline = (LWLINE *) lwgeom_from_gserialized(line);
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    np = DatumGetNpointP(tinstant_value_p(inst));
    POINTARRAY *opa = lwline_interpolate_points(lwline, np->pos, 0);
    assert(opa->npoints <= 1);
    LWGEOM *lwpoint = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
    Datum point = PointerGetDatum(geo_serialize(lwpoint));
    lwgeom_free(lwpoint);
    instants[i] = tinstant_make_free(point, T_TGEOMPOINT, inst->t);
  }

  pfree(line); lwline_free(lwline);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Convert a temporal network point into a temporal geometry point
 */
TSequenceSet *
tnpointseqset_tgeompointseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TNPOINT);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tnpointseq_tgeompointseq_cont(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_npoint_conversion
 * @brief Convert a temporal network point into a temporal geometry point
 * @param[in] temp Temporal point
 * @csqlfn #Tnpoint_to_tgeompoint()
 */
Temporal *
tnpoint_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tnpointinst_tgeompointinst((TInstant *) temp);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal *) tnpointseq_tgeompointseq_disc((TSequence *) temp) :
        (Temporal *) tnpointseq_tgeompointseq_cont((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnpointseqset_tgeompointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Convert a temporal geometry point into a temporal network point
 */
TInstant *
tgeompointinst_tnpointinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TGEOMPOINT);
  Npoint *np = geompoint_to_npoint(DatumGetGserializedP(
    tinstant_value_p(inst)));
  if (np == NULL)
    return NULL;
  return tinstant_make_free(PointerGetDatum(np), T_TNPOINT, inst->t);
}

/**
 * @brief Convert a temporal geometry point into a temporal network point
 */
TSequence *
tgeompointseq_tnpointseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TGEOMPOINT);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tgeompointinst_tnpointinst(TSEQUENCE_INST_N(seq, i));
    if (inst == NULL)
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
    instants[i] = inst;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Convert a temporal geometry point into a temporal network point
 */
TSequenceSet *
tgeompointseqset_tnpointseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TGEOMPOINT);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tgeompointseq_tnpointseq(TSEQUENCESET_SEQ_N(ss, i));
    if (seq == NULL)
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
    sequences[i] = seq;
  }
  return tsequenceset_make_free(sequences, ss->count, true);
}

/**
 * @ingroup meos_npoint_conversion
 * @brief Convert a temporal geometry point into a temporal network point
 * @param[in] temp Temporal point
 * @csqlfn #Tgeompoint_to_tnpoint()
 */
Temporal *
tgeompoint_to_tnpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);

  int32_t srid_tpoint = tspatial_srid(temp);
  int32_t srid_ways = get_srid_ways();
  if (! ensure_same_srid(srid_tpoint, srid_ways))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeompointinst_tnpointinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tgeompointseq_tnpointseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeompointseqset_tnpointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointinst_positions(const TInstant *inst)
{
  Nsegment **result = palloc(sizeof(Nsegment *));
  Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  result[0] = nsegment_make(np->rid, np->pos, np->pos);
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 * @note The function is used for both discrete and step interpolation
 */
Nsegment **
tnpointseq_step_positions(const TSequence *seq, int *count)
{
  int count1;
  /* The following function removes duplicate values */
  Datum *values = tsequence_values_p(seq, &count1);
  Nsegment **result = palloc(sizeof(Nsegment *) * count1);
  for (int i = 0; i < count1; i++)
  {
    Npoint *np = DatumGetNpointP(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  pfree(values);
  *count = count1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment *
tnpointseq_linear_positions(const TSequence *seq)
{
  Npoint *np = DatumGetNpointP(tinstant_value_p(TSEQUENCE_INST_N(seq, 0)));
  int64 rid = np->rid;
  double minPos, maxPos;
  minPos = maxPos = np->pos;
  for (int i = 1; i < seq->count; i++)
  {
    np = DatumGetNpointP(tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    minPos = Min(minPos, np->pos);
    maxPos = Max(maxPos, np->pos);
  }
  return nsegment_make(rid, minPos, maxPos);
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseq_positions(const TSequence *seq, int *count)
{
  if (! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    return tnpointseq_step_positions(seq, count);

  Nsegment **result = palloc(sizeof(Nsegment *));
  result[0] = tnpointseq_linear_positions(seq);
  *count = 1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_linear_positions(const TSequenceSet *ss, int *count)
{
  Nsegment **segments = palloc(sizeof(Nsegment *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    segments[i] = tnpointseq_linear_positions(TSEQUENCESET_SEQ_N(ss, i));
  Nsegment **result = segments;
  int count1 = ss->count;
  if (count1 > 1)
    result = nsegmentarr_normalize(segments, &count1);
  *count = count1;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_step_positions(const TSequenceSet *ss, int *count)
{
  /* The following function removes duplicate values */
  int newcount;
  Datum *values = tsequenceset_values_p(ss, &newcount);
  Nsegment **result = palloc(sizeof(Nsegment *) * newcount);
  for (int i = 0; i < newcount; i++)
  {
    const Npoint *np = DatumGetNpointP(values[i]);
    result[i] = nsegment_make(np->rid, np->pos, np->pos);
  }
  pfree(values);
  *count = newcount;
  return result;
}

/**
 * @brief Return the network segments covered by the temporal network point
 */
Nsegment **
tnpointseqset_positions(const TSequenceSet *ss, int *count)
{
  return (MEOS_FLAGS_LINEAR_INTERP(ss->flags)) ?
    tnpointseqset_linear_positions(ss, count) :
    tnpointseqset_step_positions(ss, count);
}

/**
 * @ingroup meos_npoint_accessor
 * @brief Return the network segments covered by the temporal network point
 * @csqlfn #Tnpoint_positions()
 */
Nsegment **
tnpoint_positions(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnpointinst_positions((TInstant *) temp);
    case TSEQUENCE:
      return tnpointseq_positions((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tnpointseqset_positions((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return the route of the temporal network point
 */
int64
tnpointinst_route(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  return np->rid;
}

/**
 * @ingroup meos_npoint_accessor
 * @brief Return the single route of a temporal network point
 * @return On error return @p INT_MAX
 * @csqlfn #Tnpoint_route()
 */
int64
tnpoint_route(const Temporal *temp)
{
  if (temp->subtype != TINSTANT && MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Input must be a temporal instant or a temporal sequence with continuous interpolation");
    return INT_MAX;
  }
  const TInstant *inst = (temp->subtype == TINSTANT) ?
    (const TInstant *) temp : TSEQUENCE_INST_N((const TSequence *) temp, 0);
  const Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  return np->rid;
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointinst_routes(const TInstant *inst)
{
  Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
  Datum value = Int64GetDatum(np->rid);
  return set_make_exp(&value, 1, 1, T_INT8, ORDER_NO);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointseq_disc_routes(const TSequence *seq)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Npoint *np = DatumGetNpointP(tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    values[i] = Int64GetDatum(np->rid);
  }
  datumarr_sort(values, seq->count, T_INT8);
  int count = datumarr_remove_duplicates(values, seq->count, T_INT8);
  return set_make_free(values, count, T_INT8, ORDER_NO);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointseq_cont_routes(const TSequence *seq)
{
  assert(seq); assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  const Npoint *np = DatumGetNpointP(tinstant_value_p(TSEQUENCE_INST_N(seq, 0)));
  Datum value = Int64GetDatum(np->rid);
  return set_make_exp(&value, 1, 1, T_INT8, ORDER_NO);
}

/**
 * @brief Return the routes of a temporal network point
 */
Set *
tnpointseqset_routes(const TSequenceSet *ss)
{
  Datum *values = palloc(sizeof(int64) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, i), 0);
    Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
    values[i] = np->rid;
  }
  datumarr_sort(values, ss->count, T_INT8);
  int count = datumarr_remove_duplicates(values, ss->count, T_INT8);
  return set_make_free(values, count, T_INT8, ORDER_NO);
}

/**
 * @ingroup meos_npoint_accessor
 * @brief Return the array of routes of a temporal network point
 * @csqlfn #Tnpoint_routes()
 */
Set *
tnpoint_routes(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnpointinst_routes((TInstant *) temp);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        tnpointseq_disc_routes((TSequence *) temp) :
        tnpointseq_cont_routes((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnpointseqset_routes((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_set_accessor
 * @brief Return the routes of a network point set
 * @param[in] s Set
 * @csqlfn #Npointset_routes()
 */
Set *
npointset_routes(const Set *s)
{
  VALIDATE_NPOINTSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
  {
    const Npoint *np = DatumGetNpointP(SET_VAL_N(s, i));
    values[i] = Int64GetDatum(np->rid);
  }
  datumarr_sort(values, s->count, T_INT8);
  int count = datumarr_remove_duplicates(values, s->count, T_INT8);
  return set_make_free(values, count, T_INT8, ORDER_NO);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_npoint_restrict
 * @brief Restrict a temporal network point to (the complement of) a network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @csqlfn #Tnpoint_restrict_npoint()
 */
Temporal *
tnpoint_restrict_npoint(const Temporal *temp, const Npoint *np, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;
  /* If the temporal network point does not contain the RID of the network point */
  if (! common_rid_tnpoint_npoint(temp, np))
    return atfunc ? NULL : temporal_copy(temp);

  return temporal_restrict_value(temp, PointerGetDatum(np), atfunc);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Restrict a temporal network point to a network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Tnpoint_at_npoint()
 */
inline Temporal *
tnpoint_at_npoint(const Temporal *temp, const Npoint *np)
{
  return tnpoint_restrict_npoint(temp, np, REST_AT);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Restrict a temporal network point to the complement of a network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Tnpoint_minus_npoint()
 */
inline Temporal *
tnpoint_minus_npoint(const Temporal *temp, const Npoint *np)
{
  return tnpoint_restrict_npoint(temp, np, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_npoint_restrict
 * @brief Restrict a temporal network point to (the complement of) a set of
 * network points
 * @param[in] temp Temporal network point
 * @param[in] s Set of network points
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Tnpoint_restrict_npointset()
 */
Temporal *
tnpoint_restrict_npointset(const Temporal *temp, const Set *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NPOINTSET(s, NULL); 
  if (! ensure_valid_tnpoint_npointset(temp, s))
    return NULL;
  /* If the temporal network point and the set do not have a common RID */
  if (! common_rid_tnpoint_npointset(temp, s))
    return atfunc ? NULL : temporal_copy(temp);
  return temporal_restrict_values(temp, s, atfunc);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Restrict a temporal network point to a set of network points
 * @param[in] temp Temporal network point
 * @param[in] s Set of network points
 * @csqlfn #Tnpoint_at_npointset()
 */
inline Temporal *
tnpoint_at_npointset(const Temporal *temp, const Set *s)
{
  return tnpoint_restrict_npointset(temp, s, REST_AT);
}

/**
 * @ingroup meos_npoint_restrict
 * @brief Restrict a temporal network point to the complement of a set of
 * network points
 * @param[in] temp Temporal network point
 * @param[in] s Set of network points
 * @csqlfn #Tnpoint_minus_npointset()
 */
inline Temporal *
tnpoint_minus_npointset(const Temporal *temp, const Set *s)
{
  return tnpoint_restrict_npointset(temp, s, REST_MINUS);
}

/*****************************************************************************/

