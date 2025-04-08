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
 * @brief Ever/always spatial relationships for temporal rigid geometries
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding temporal spatial
 * relationship.
 *
 * The following relationships are supported: `contains`, `disjoint`,
 * `intersects`, `touches`, and `dwithin`.
 */

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatial relationship for the traversed area of a temporal
 * rigid geometry and a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
int
spatialrel_trgeo_trav_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  Datum trav = PointerGetDatum(trgeo_traversed_area(temp));
  Datum result;
  if (numparam == 2)
  {
    datum_func2 func2 = (datum_func2) func;
    result = invert ? func2(geo, trav) : func2(trav, geo);
  }
  else /* numparam == 3 */
  {
    datum_func3 func3 = (datum_func3) func;
    result = invert ? func3(geo, trav, param) : func3(trav, geo, param);
  }
  pfree(DatumGetPointer(trav));
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @brief Generic spatial relationship for the traversed area of a temporal
 * rigid geometry and a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
static int
spatialrel_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum dgeo = PointerGetDatum(geo);
  Datum dtrav = PointerGetDatum(trgeo_traversed_area(temp));
  Datum result;
  if (numparam == 2)
  {
    datum_func2 func2 = (datum_func2) func;
    result = invert ? func2(dgeo, dtrav) : func2(dtrav, dtrav);
  }
  else /* numparam == 3 */
  {
    datum_func3 func3 = (datum_func3) func;
    result = invert ? func3(dgeo, dtrav, param) : func3(dtrav, dgeo, param);
  }
  pfree(DatumGetPointer(dtrav));
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @brief Return 1 if a geometry ever contains a temporal rigid geometry, 0 if 
 * not, and -1 on error or if the geometry is empty
 * @param[in] geo Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the traversed area intersects the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #EA_contains_geo_trgeo()
 */
int
ea_contains_geo_trgeo(const GSERIALIZED *geo, const Temporal *temp, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo))
    return -1;
  GSERIALIZED *trav = trgeo_traversed_area(temp);
  bool result = ever ? geom_relate_pattern(geo, trav, "T********") :
    geom_contains(geo, trav);
  pfree(trav);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal rigid geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] geo Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_trgeo()
 */
inline int
econtains_geo_trgeo(const GSERIALIZED *geo, const Temporal *temp)
{
  return ea_contains_geo_trgeo(geo, temp, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal rigid geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] geo Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_trgeo()
 */
inline int
acontains_geo_trgeo(const GSERIALIZED *geo, const Temporal *temp)
{
  return ea_contains_geo_trgeo(geo, temp, ALWAYS);
}

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever
 * disjoint,0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #EA_disjoint_trgeo_geo()
 */
int
ea_disjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo))
    return -1;
  int result = ever ?
    spatialrel_trgeo_trav_geo(temp, geo, (Datum) NULL,
      (varfunc) &datum_geom_covers, 2, INVERT) :
    eintersects_trgeo_geo(temp, geo);
  return INVERT_RESULT(result);
}
/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever 
 * disjoint, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @csqlfn #EA_disjoint_trgeo_geo()
 */
inline int
edisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  return ea_disjoint_trgeo_geo(temp, geo, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are always 
 * disjoint,0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @note aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @csqlfn #Adisjoint_trgeo_geo()
 */
inline int
adisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  return ea_disjoint_trgeo_geo(temp, geo, ALWAYS);
}

#if MEOS
/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries are ever disjoint, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #EA_disjoint_trgeo_trgeo()
 */
inline int
edisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_ne, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries are always disjoint, 0 if
 * not, and -1 on error or if the temporal rigid geometries do not intersect
 * in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Adisjoint_trgeo_trgeo()
 */
inline int
adisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_ne,
    ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry ever intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @csqlfn #EA_intersects_trgeo_geo()
 */
inline int
eintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  return spatialrel_trgeo_trav_geo(temp, geo, (Datum) NULL, 
    (varfunc) &datum_geom_intersects2d, 2, INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry always 
 * intersect, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @note aIntersects(trgeo, geo) is equivalent to NOT eDisjoint(trgeo, geo)
 * @csqlfn #Aintersects_trgeo_geo()
 */
inline int
aintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  return INVERT_RESULT(edisjoint_trgeo_geo(temp, geo));
}

#if MEOS
/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries ever intersect, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #EA_intersects_trgeo_trgeo()
 */
inline int
eintersects_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_eq, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries always intersect, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Aintersects_trgeo_trgeo()
 */
inline int
aintersects_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_eq,
    ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry ever touch, 0 
 * if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @csqlfn #EA_touches_trgeo_geo()
 */
int
etouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  datum_func2 func = get_intersects_fn_geo(temp->flags, geo->gflags);
  GSERIALIZED *trav = trgeo_traversed_area(temp);
  GSERIALIZED *geobound = geom_boundary(geo);
  bool result = false;
  if (geobound && ! gserialized_is_empty(geobound))
    result = func(GserializedPGetDatum(geobound), GserializedPGetDatum(trav));
  /* TODO */
  // else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  // {
    // /* The geometry is a point or a multipoint -> the boundary is empty */
    // GSERIALIZED *tempbound = geom_boundary(trav);
    // if (tempbound)
    // {
      // result = func(GserializedPGetDatum(tempbound), GserializedPGetDatum(geo));
      // pfree(tempbound);
    // }
  // }
  pfree(trav); pfree(geobound);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry always touch, 
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @csqlfn #Atouches_trgeo_geo()
 */
int
atouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  GSERIALIZED *geobound = geom_boundary(geo);
  bool result = false;
  if (geobound && ! gserialized_is_empty(geobound))
  {
    // TODO trgeo_minus_geom(temp, geobound, NULL);
    Temporal *temp1 = (Temporal *) temp;
    result = (temp1 == NULL);
    if (temp1)
      pfree(temp1);
  }
  pfree(geobound);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always dwithin
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry are ever within
 * the given distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @param[in] dist Distance
 * @csqlfn #EA_dwithin_trgeo_geo()
 */
int
edwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  return spatialrel_trgeo_trav_geo(temp, geo, Float8GetDatum(dist),
    (varfunc) &datum_geom_dwithin2d, 3, INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry are always 
 * within a distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal rigid geometry
 * @param[in] geo Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeo_geo()
 */
int
adwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *geo, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, geo) || gserialized_is_empty(geo) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  GSERIALIZED *buffer = geom_buffer(geo, dist, "");
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_trgeo_trav_geo(temp, buffer, (Datum) NULL,
    (varfunc) func, 2, INVERT);
  pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal rigid geometries are ever within a
 * distance
 * @param[in] inst1,inst2 Temporal rigid geometries
 * @param[in] dist Distance
 * @pre The temporal rigid geometries are synchronized
 */
static bool
ea_dwithin_trgeoinst_trgeoinst(const TInstant *inst1, const TInstant *inst2,
  double dist)
{
  assert(inst1); assert(inst2);
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(datum_geom_dwithin2d(tinstant_value_p(inst1),
    tinstant_value_p(inst2), Float8GetDatum(dist)));
}

/**
 * @brief Return true if two temporal rigid geometries are ever within a
 * distance
 * @param[in] seq1,seq2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal rigid geometries are synchronized
 */
static bool
ea_dwithin_trgeoseq_trgeoseq_discstep(const TSequence *seq1,
  const TSequence *seq2, double dist, bool ever)
{
  assert(seq1); assert(seq2);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < seq1->count; i++)
  {
    bool res = ea_dwithin_trgeoinst_trgeoinst(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i), dist);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @brief Return the timestamps at which EITHER the segments of the two
 * temporal points OR a segment of a temporal point and a point are within a
 * distance
 * @param[in] sv1,ev1 Points defining the first segment
 * @param[in] sv2,ev2 Points defining the second segment
 * @param[in] lower,upper Timestamps associated to the segments
 * @param[in] dist Distance
 * @param[out] t1,t2 Resulting timestamps
 * @return Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
int
tdwithin_trgeosegm_trgeosegm(Datum sv1 __attribute__((unused)), 
  Datum ev1 __attribute__((unused)), Datum sv2 __attribute__((unused)), 
  Datum ev2 __attribute__((unused)), TimestampTz lower __attribute__((unused)), 
  TimestampTz upper __attribute__((unused)), 
  double dist __attribute__((unused)), TimestampTz *t1 __attribute__((unused)), 
  TimestampTz *t2 __attribute__((unused)))
{
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return 0;
}

/**
 * @brief Return true if two temporal rigid geometries are ever within a
 * distance
 * @param[in] seq1,seq2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal rigid geometries are synchronized
 */
static bool
ea_dwithin_trgeoseq_trgeoseq_cont(const TSequence *seq1, const TSequence *seq2,
  double dist, bool ever)
{
  assert(seq1); assert(seq2);

  const TInstant *start1, *start2;
  if (seq1->count == 1)
  {
    start1 = TSEQUENCE_INST_N(seq1, 0);
    start2 = TSEQUENCE_INST_N(seq2, 0);
    return ea_dwithin_trgeoinst_trgeoinst(start1, start2, dist);
  }

  start1 = TSEQUENCE_INST_N(seq1, 0);
  start2 = TSEQUENCE_INST_N(seq2, 0);
  Datum sv1 = tinstant_value_p(start1);
  Datum sv2 = tinstant_value_p(start2);

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  bool ret_loop = ever ? true : false;
  for (int i = 1; i < seq1->count; i++)
  {
    const TInstant *end1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *end2 = TSEQUENCE_INST_N(seq2, i);
    Datum ev1 = tinstant_value_p(end1);
    Datum ev2 = tinstant_value_p(end2);
    TimestampTz upper = end1->t;
    bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

    /* Both segments are constant */
    if (datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2))
    {
      bool res = DatumGetBool(datum_geom_dwithin2d(sv1, sv2, 
        Float8GetDatum(dist)));
      if ((ever && res) || (! ever && ! res))
        return ret_loop;
    }

    /* General case */
    TimestampTz t1, t2;
    Datum sev1 = linear1 ? ev1 : sv1;
    Datum sev2 = linear2 ? ev2 : sv2;
    /* Find the instants t1 and t2 (if any) during which the dwithin function
     * is true */
    int solutions = tdwithin_trgeosegm_trgeosegm(sv1, sev1, sv2, sev2,
      lower, upper, dist, &t1, &t2);
    bool res = (solutions == 2 ||
      (solutions == 1 && ((t1 != lower || lower_inc) &&
        (t1 != upper || upper_inc))));
    if ((ever && res) || (! ever && ! res))
      return ret_loop;

    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal rigid geometries are ever within a
 * distance
 * @param[in] ss1,ss2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal rigid geometries are synchronized
 */
static bool
ea_dwithin_trgeoseqset_trgeoseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, bool ever)
{
  assert(ss1); assert(ss2);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(ss2->flags);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < ss1->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, i);
    bool res = linear ?
      ea_dwithin_trgeoseq_trgeoseq_cont(seq1, seq2, dist, ever) :
      ea_dwithin_trgeoseq_trgeoseq_discstep(seq1, seq2, dist, ever);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/*****************************************************************************/

/**
 * @brief Return 1 if two temporal rigid geometries are ever within a distance,
 * 0 if not, -1 if the temporal rigid geometries do not intersect on time
 * @pre The temporal rigid geometries are synchronized
 */
int
ea_dwithin_trgeo_trgeo_sync(const Temporal *sync1, const Temporal *sync2,
  double dist, bool ever)
{
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      return ea_dwithin_trgeoinst_trgeoinst((TInstant *) sync1,
        (TInstant *) sync2, dist);
    case TSEQUENCE:
      return MEOS_FLAGS_LINEAR_INTERP(sync1->flags) ||
          MEOS_FLAGS_LINEAR_INTERP(sync2->flags) ?
        ea_dwithin_trgeoseq_trgeoseq_cont((TSequence *) sync1,
          (TSequence *) sync2, dist, ever) :
        ea_dwithin_trgeoseq_trgeoseq_discstep((TSequence *) sync1,
          (TSequence *) sync2, dist, ever);
    default: /* TSEQUENCESET */
      return ea_dwithin_trgeoseqset_trgeoseqset((TSequenceSet *) sync1,
        (TSequenceSet *) sync2, dist, ever);
  }
}

/**
 * @ingroup meos_internal_geo_spatial_rel_ever
 * @brief Return 1 if two temporal rigid geometries are ever within a distance,
 * 0 if not, -1 on error or if the temporal rigid geometries do not intersect
 * on time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #EA_dwithin_trgeo_trgeo()
 */
int
ea_dwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal rigid geometries do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return -1;

  bool result = ea_dwithin_trgeo_trgeo_sync(sync1, sync2, dist, ever);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are ever within a distance,
 * 0 if not, -1 on error or if the temporal rigid geometries do not intersect
 * on time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #EA_dwithin_trgeo_trgeo()
 */
inline int
edwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_trgeo_trgeo(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are always within a 
 * distance, 0 if not, -1 on error or if the temporal rigid geometries do not
 * intersect on time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeo_trgeo()
 */
inline int
adwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_trgeo_trgeo(temp1, temp2, dist, ALWAYS);
}

/*****************************************************************************/
