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
 * @brief Ever spatial relationships for temporal points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `disjoint`, `intersects`, `touches`, and `dwithin`.
 *
 * The following relationships are supported for geographies: `disjoint`,
 * `intersects`, `dwithin`.
 *
 * Only `disjoint`, `dwithin`, and `intersects` are supported for 3D geometries.
 */

#include "point/tpoint_spatialrels.h"

/* PostgreSQL */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporal_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tempspatialrels.h"

/*****************************************************************************
 * Spatial relationship functions
 * disjoint and intersects are inverse to each other
 *****************************************************************************/

/**
 * Call the PostGIS function ST_Contains with the 2 arguments
 */
Datum
geom_contains(Datum geom1, Datum geom2)
{
  return BoolGetDatum(gserialized_inter_contains(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), false));
}

/**
 * Call the PostGIS function ST_Intersects with the 2 arguments
 * and negate the result
 */
Datum
geom_disjoint2d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! DatumGetBool(geom_intersects2d(geom1, geom2)));
}

/**
 * Call the PostGIS function ST_Intersects3D with the 2 arguments
 * and negates the result
 */
Datum
geom_disjoint3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! DatumGetBool(geom_intersects3d(geom1, geom2)));
}

/**
 * Call the PostGIS function ST_Intersects for geographies with the 2 arguments
 */
Datum
geog_disjoint(Datum geog1, Datum geog2)
{
  return BoolGetDatum(! DatumGetBool(geog_intersects(geog1, geog2)));
}

/**
 * Call the PostGIS function ST_Intersects with the 2 arguments
 */
Datum
geom_intersects2d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(gserialized_inter_contains(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), true));
}

/**
 * Call the PostGIS function ST_3DIntersects with the 2 arguments
 */
Datum
geom_intersects3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(gserialized_3Dintersects(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * Call the PostGIS function ST_Intersects for geographies with the 2 arguments
 */
Datum
geog_intersects(Datum geog1, Datum geog2)
{
  return BoolGetDatum(gserialized_geog_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), 0.0, true));
}

/**
 * Call the PostGIS function ST_Touches with the 2 arguments
 */
Datum
geom_touches(Datum geom1, Datum geom2)
{
  return BoolGetDatum(gserialized_touches(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * Call the PostGIS function ST_DWithin with the 3 arguments
 */
Datum
geom_dwithin2d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(gserialized_dwithin(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * Call the PostGIS function ST_3DDWithin with the 3 arguments
 */
Datum
geom_dwithin3d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(gserialized_dwithin3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * Call the PostGIS function ST_DWithin for geographies with the 3 arguments
 */
Datum
geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
  return BoolGetDatum(gserialized_geog_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), DatumGetFloat8(dist), true));
}

/*****************************************************************************/

/**
 * Select the appropriate disjoint function for a MobilityDB type and a
 * GSERIALIZED. We need two parameters to cope with mixed 2D/3D arguments
 */
static datum_func2
get_disjoint_fn_gs(int16 flags1, uint8_t flags2)
{
  datum_func2 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags1))
    result = &geog_disjoint;
  else
    /* 3D only if both arguments are 3D */
    result = MOBDB_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_disjoint3d : &geom_disjoint2d;
  return result;
}

/**
 * Select the appropriate intersect function for a MobilityDB type and a
 * GSERIALIZED. We need two parameters to cope with mixed 2D/3D arguments
 */
static datum_func2
get_intersects_fn_gs(int16 flags1, uint8_t flags2)
{
  datum_func2 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags1))
    result = &geog_intersects;
  else
    /* 3D only if both arguments are 3D */
    result = MOBDB_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_intersects3d : &geom_intersects2d;
  return result;
}

/**
 * Select the appropriate dwithin function for two MobilityDB types.
 * We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
get_dwithin_fn(int16 flags1, int16 flags2)
{
  datum_func3 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags1))
    result = &geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    result = MOBDB_FLAGS_GET_Z(flags1) && MOBDB_FLAGS_GET_Z(flags2) ?
      &geom_dwithin3d : &geom_dwithin2d;
  return result;
}

/**
 * Select the appropriate dwithin function for a MobilityDB type and a
 * GSERIALIZED. We need two parameters to cope with mixed 2D/3D arguments
 */
static datum_func3
get_dwithin_fn_gs(int16 flags1, uint8_t flags2)
{
  datum_func3 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags1))
    result = &geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    result = MOBDB_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_dwithin3d : &geom_dwithin2d;
  return result;
}

/*****************************************************************************
 * Generic ever spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic ever spatial relationship for a temporal point and a geometry
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 */
bool
spatialrel_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, Datum param,
  Datum (*func)(Datum, ...), int numparam, bool invert)
{
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  Datum traj = PointerGetDatum(tpoint_trajectory(temp));
  Datum result;
  if (numparam == 2)
    result = invert ? func(geo, traj) : func(traj, geo);
  else /* numparam == 3 */
    result = invert ? func(geo, traj, param) : func(traj, geo, param);
  pfree(DatumGetPointer(traj));
  return DatumGetBool(result);
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal points ever satisfy the spatial
 * relationship.
 *
 * @param[in] temp1,temp2 Temporal points
 * @param[in] func Spatial relationship
 */
int
spatialrel_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum))
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp1->temptype);
  lfinfo.argtype[1] = temptype_basetype(temp2->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  int result = efunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

/*****************************************************************************
 * Ever contains
 * The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function
 *****************************************************************************/

/**
 * Call the PostGIS function ST_Relate twice to compute the ever contains
 * relationship between the trajectory of a temporal point and a geometry
 * @param[in] geom1 Trajectory of the temporal point
 * @param[in] geom2 Geometry
 * @note The order of the parameters CANNOT be inverted
 */
static Datum
geom_ever_contains(Datum geom1, Datum geom2)
{
  return BoolGetDatum(gserialized_relate_pattern(DatumGetGserializedP(geom1),
      DatumGetGserializedP(geom2), "T********")) ||
    BoolGetDatum(gserialized_relate_pattern(DatumGetGserializedP(geom1),
      DatumGetGserializedP(geom2), "***T*****"));
}

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if a geometry ever contains a temporal point,
 * 0 if not, and -1 if the geometry is empty.
 * @sqlfunc contains()
 */
int
contains_geo_tpoint(const GSERIALIZED *geo, const Temporal *temp)
{
  if (gserialized_is_empty(geo))
    return -1;
  ensure_has_not_Z_gs(geo);
  ensure_has_not_Z(temp->flags);
  bool result = spatialrel_tpoint_geo(temp, geo, (Datum) NULL,
    (varfunc) &geom_ever_contains, 2, INVERT_NO);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a temporal instant point and a geometry are ever
 * disjoint
 *
 * @param[in] inst Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be called
 */
bool
disjoint_tpointinst_geo(const TInstant *inst, Datum geo,
  Datum (*func)(Datum, ...))
{
  return DatumGetBool(func(tinstant_value(inst), geo));
}

/**
 * @brief Return true if a temporal sequence point and a geometry are ever
 * disjoint
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be called
 */
bool
disjoint_tpointseq_geo(const TSequence *seq, Datum geo,
  Datum (*func)(Datum, ...))
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (DatumGetBool(func(tinstant_value(inst), geo)))
      return true;
  }
  return false;
}

/**
 * @brief Return true if a temporal sequence set point and a geometry are ever
 * disjoint
 *
 * @param[in] ss Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be used for instantaneous sequences
 */
bool
disjoint_tpointseqset_geo(const TSequenceSet *ss, Datum geo,
  Datum (*func)(Datum, ...))
{
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    if (disjoint_tpointseq_geo(seq, geo, func))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if a temporal point and a geometry are ever disjoint,
 * 0 if not, and -1 if the geometry is empty.
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @sqlfunc disjoint()
 */
int
disjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  varfunc func = (varfunc) get_disjoint_fn_gs(temp->flags, gs->gflags);
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = disjoint_tpointinst_geo((TInstant *) temp, PointerGetDatum(gs),
      func);
  else if (temp->subtype == TSEQUENCE)
    result = disjoint_tpointseq_geo((TSequence *) temp, PointerGetDatum(gs),
      func);
  else /* temp->subtype == TSEQUENCESET */
    result = disjoint_tpointseqset_geo((TSequenceSet *) temp,
      PointerGetDatum(gs), func);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if a geometry and a temporal point ever intersect,
 * 0 if not, and -1 if the geometry is empty.
 * @sqlfunc intersects()
 */
int
intersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  datum_func2 func = get_intersects_fn_gs(temp->flags, gs->gflags);
  bool result = spatialrel_tpoint_geo(temp, gs, (Datum) NULL, (varfunc) func, 2,
    INVERT_NO);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if a temporal point and a geometry ever touch, 0 if not, and
 * -1 if the geometry is empty
 * @sqlfunc touches()
 */
int
touches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  GSERIALIZED *gsbound = gserialized_boundary(gs);
  bool result = false;
  if (! gserialized_is_empty(gsbound))
  {
    varfunc func =
      (MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags)) ?
      (varfunc) &geom_intersects3d : (varfunc) &geom_intersects2d;
    result = spatialrel_tpoint_geo(temp, gsbound, (Datum) NULL, func, 2,
      INVERT_NO);
  }
  pfree(gsbound);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if a geometry and a temporal point are ever within the
 * given distance, 0 if not, -1 if a geometry is empty
 * @sqlfunc dwithin()
 */
int
dwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  if (gserialized_is_empty(gs))
    return -1;
  datum_func3 func = get_dwithin_fn_gs(temp->flags, gs->gflags);
  bool result = spatialrel_tpoint_geo(temp, gs, Float8GetDatum(dist),
    (varfunc) func, 3, INVERT);
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * Return true if the temporal points are ever within the given distance
 *
 * @param[in] inst1,inst2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointinst_tpointinst(const TInstant *inst1, const TInstant *inst2,
  double dist, datum_func3 func)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  return DatumGetBool(func(value1, value2, Float8GetDatum(dist)));
}

/**
 * Return true if the temporal points are ever within the given distance
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointdiscseq_tpointdiscseq(const TSequence *seq1,
  const TSequence *seq2, double dist, datum_func3 func)
{
  for (int i = 0; i < seq1->count; i++)
  {
    const TInstant *inst1 = tsequence_inst_n(seq1, i);
    const TInstant *inst2 = tsequence_inst_n(seq2, i);
    if (dwithin_tpointinst_tpointinst(inst1, inst2, dist, func))
      return true;
  }
  return false;
}

/**
 * Return true if the temporal points are ever within the given distance
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointseq_tpointseq(const TSequence *seq1, const TSequence *seq2,
  double dist, datum_func3 func)
{
  const TInstant *start1, *start2;
  if (seq1->count == 1)
  {
    start1 = tsequence_inst_n(seq1, 0);
    start2 = tsequence_inst_n(seq2, 0);
    return dwithin_tpointinst_tpointinst(start1, start2, dist, func);
  }

  start1 = tsequence_inst_n(seq1, 0);
  start2 = tsequence_inst_n(seq2, 0);
  Datum sv1 = tinstant_value(start1);
  Datum sv2 = tinstant_value(start2);

  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(seq1->flags);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  for (int i = 1; i < seq1->count; i++)
  {
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
      if (DatumGetBool(func(sv1, sv2, Float8GetDatum(dist))))
        return true;
      if (! linear1 && ! linear2 && upper_inc &&
          DatumGetBool(func(ev1, ev2, Float8GetDatum(dist))))
        return true;
    }
    /* General case */
    else
    {
      /* Find the instants t1 and t2 (if any) during which the dwithin
       * function is true */
      TimestampTz t1, t2;
      Datum sev1 = linear1 ? ev1 : sv1;
      Datum sev2 = linear2 ? ev2 : sv2;
      int solutions = tdwithin_tpointsegm_tpointsegm(sv1, sev1, sv2, sev2,
        lower, upper, dist, hasz, func, &t1, &t2);
      if (solutions == 2 ||
      (solutions == 1 && ((t1 != lower || lower_inc) &&
        (t1 != upper || upper_inc))))
         return true;
    }
    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  return false;
}

/**
 * Return true if the temporal points are ever within the given distance
 *
 * @param[in] ss1,ss2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points are synchronized
 */
static bool
dwithin_tpointseqset_tpointseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, datum_func3 func)
{
  for (int i = 0; i < ss1->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ss1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ss2, i);
    if (dwithin_tpointseq_tpointseq(seq1, seq2, dist, func))
      return true;
  }
  return false;
}

/*****************************************************************************/

/**
 * @brief Return 1 if the temporal points are ever within the given distance,
 * 0 if not, -1 if the temporal points do not intersect on time
 * @pre The temporal points are synchronized
 */
int
dwithin_tpoint_tpoint1(const Temporal *sync1, const Temporal *sync2,
  double dist)
{
  datum_func3 func = get_dwithin_fn(sync1->flags, sync2->flags);
  bool result;
  ensure_valid_tempsubtype(sync1->subtype);
  if (sync1->subtype == TINSTANT)
    result = dwithin_tpointinst_tpointinst((TInstant *) sync1,
      (TInstant *) sync2, dist, func);
  else if (sync1->subtype == TSEQUENCE)
    result = MOBDB_FLAGS_GET_DISCRETE(sync1->flags) ?
      dwithin_tpointdiscseq_tpointdiscseq((TSequence *) sync1,
        (TSequence *) sync2, dist, func) :
      dwithin_tpointseq_tpointseq((TSequence *) sync1, (TSequence *) sync2,
        dist, func);
  else /* sync1->subtype == TSEQUENCESET */
    result = dwithin_tpointseqset_tpointseqset((TSequenceSet *) sync1,
      (TSequenceSet *) sync2, dist, func);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if the temporal points are ever within the given distance,
 * 0 if not, -1 if the temporal points do not intersect on time
 * @sqlfunc dwithin()
 */
int
dwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2, double dist)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return -1;

  bool result = dwithin_tpoint_tpoint1(sync1, sync2, dist);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/*****************************************************************************/
