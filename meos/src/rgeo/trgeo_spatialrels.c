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
 * @brief Ever/always spatial relationships for temporal rigid geometries
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding spatiotemporal
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
#include "temporal/lifting.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_distance.h"
#include "rgeo/trgeo_spatialfuncs.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatial relationship for the placements of a temporal
 * rigid geometry and a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func Spatial relationship function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @errval -1
 */
int
spatialrel_trgeo_trav_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  Datum places = PointerGetDatum(trgeo_placements(temp));
  Datum result;
  if (numparam == 2)
  {
    datum_func2 func2 = (datum_func2) func;
    result = invert ? func2(geo, places) : func2(places, geo);
  }
  else /* numparam == 3 */
  {
    datum_func3 func3 = (datum_func3) func;
    result = invert ? func3(geo, places, param) : func3(places, geo, param);
  }
  pfree(DatumGetPointer(places));
  return result ? 1 : 0;
}

/**
 * @brief Return 1 if the poses of a temporal rigid geometry and a geometry
 * ever/always satisfy a spatial relationship, 0 if not, and -1 on error
 * @details The placements are the reference geometry at each of the poses the
 * value takes, so the relationship is asked of each of them: the ever
 * semantics hold where one pose satisfies it, the always semantics where
 * every pose does. Asking it of the placements as a whole answers for the
 * union of the poses, which is a region the body occupies at no single instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] func Spatial relationship function to be called
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @errval -1
 */
static int
ea_spatialrel_trgeo_poses_geo(const Temporal *temp, const GSERIALIZED *gs,
  datum_func2 func, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;

  GSERIALIZED *places = trgeo_placements(temp);
  Datum geo = PointerGetDatum(gs);
  int count;
  GSERIALIZED **poses = geo_extract_elements(places, &count);
  /* The vacuous default: false for the ever semantics, true for the always
   * ones */
  int result = ever ? 0 : 1;
  for (int i = 0; i < count; i++)
  {
    bool res = DatumGetBool(func(PointerGetDatum(poses[i]), geo));
    if ((ever && res) || (! ever && ! res))
    {
      result = res ? 1 : 0;
      break;
    }
  }
  pfree_array((void *) poses, count);
  pfree(places);
  return result;
}

/*****************************************************************************/

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @brief Return 1 if a geometry ever contains a temporal rigid geometry, 0 if
 * not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the placements intersect the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_contains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;
  GSERIALIZED *places = trgeo_placements(temp);
  bool result = ever ? geom_relate_pattern(gs, places, "T********") :
    geom_contains(gs, places);
  pfree(places);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal rigid geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The function tests whether the placements are contained in the
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_trgeometry()
 */
int
econtains_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_trgeo(gs, temp, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal rigid geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The function tests whether the placements are contained in the
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_trgeometry()
 */
int
acontains_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_trgeo(gs, temp, ALWAYS);
}

/**
 * @brief Return 1 if the placements of a temporal rigid geometry ever
 * contain the placements of another, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_contains_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return -1;
  /* Common-time gate: the placements below hold where each body passes and carry no time,
   * so two rigid geometries sharing no instant would be related through positions they
   * never hold together */
  if (! temporal_time_overlaps(temp1, temp2))
    return -1;
  GSERIALIZED *places1 = trgeo_placements(temp1);
  GSERIALIZED *places2 = trgeo_placements(temp2);
  bool result = ever ? geom_relate_pattern(places1, places2, "T********") :
    geom_contains(places1, places2);
  pfree(places1); pfree(places2);
  return result ? 1 : 0;
}

/**
 * @brief Return whether the placements of a temporal rigid geometry ever or
 * always contain a geometry
 * @details The result is 1 if they do, 0 if not, and -1 on error or if the
 * geometry is empty.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_contains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;
  GSERIALIZED *places = trgeo_placements(temp);
  bool result = ever ? geom_relate_pattern(places, gs, "T********") :
    geom_contains(places, gs);
  pfree(places);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always covers
 *****************************************************************************/

/**
 * @brief Return 1 if a geometry ever covers a temporal geometry, 0 if not,
 * and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the placements intersect the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_covers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;
  GSERIALIZED *places = trgeo_placements(temp);
  bool result = ever ? geom_relate_pattern(gs, places, "T********") :
    geom_covers(gs, places);
  pfree(places);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever covers a temporal geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal geometry
 * @note The function tests whether the placements are covered in the
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Ecovers_geo_trgeometry()
 */
int
ecovers_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_geo_trgeo(gs, temp, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry always covers a temporal geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal geometry
 * @note The function tests whether the placements are covered in the
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acovers_geo_trgeometry()
 */
int
acovers_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_geo_trgeo(gs, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a geometry ever covers a temporal geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the placements intersect the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_covers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_trgeo_poses_geo(temp, gs, &datum_geom_covers, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever covers a temporal geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @note The function tests whether the placements cover the geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Ecovers_trgeometry_geo()
 */
int
ecovers_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_trgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal geometry always covers a geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @note The function tests whether the placements cover the geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acovers_trgeometry_geo()
 */
int
acovers_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_trgeo_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if the placements of a temporal rigid geometry ever
 * cover the placements of another, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_covers_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return -1;
  /* Common-time gate, as in #ea_contains_trgeo_trgeo: the placements carry no time */
  if (! temporal_time_overlaps(temp1, temp2))
    return -1;
  GSERIALIZED *places1 = trgeo_placements(temp1);
  GSERIALIZED *places2 = trgeo_placements(temp2);
  bool result = ever ? geom_relate_pattern(places1, places2, "T********") :
    geom_covers(places1, places2);
  pfree(places1); pfree(places2);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever
 * disjoint,0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_disjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;
  if (ever)
    return ea_spatialrel_trgeo_poses_geo(temp, gs, &datum_geom_disjoint2d,
      EVER);
  /* aDisjoint(trgeo, geo) ≡ NOT eIntersects(trgeo, geo) */
  return INVERT_RESULT(eintersects_trgeometry_geo(temp, gs));
}
/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever
 * disjoint, 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_trgeometry_geo()
 */
int
edisjoint_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_trgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are always
 * disjoint,0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @note aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @csqlfn #Adisjoint_trgeometry_geo()
 */
int
adisjoint_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_trgeo_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry are ever or
 * always disjoint, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Disjoint is symmetric: delegates to ea_disjoint_trgeo_geo
 */
int
ea_disjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_disjoint_trgeo_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries are
 * ever or always disjoint, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_disjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_ne, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries are ever disjoint, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Edisjoint_trgeometry_trgeometry()
 */
int
edisjoint_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return ea_disjoint_trgeo_trgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return whether the temporal rigid geometries are always disjoint
 * @details The function returns 1 if they are, 0 if not, and -1 on error or if
 * the temporal rigid geometries do not intersect in time.
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Adisjoint_trgeometry_trgeometry()
 */
int
adisjoint_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return ea_disjoint_trgeo_trgeo(temp1, temp2, ALWAYS);
}

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
 * @brief Return whether the placements of a temporal rigid geometry ever or
 * always intersect a geometry
 * @details The result is 1 if they do, 0 if not, and -1 on error or if the
 * geometry is empty.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_intersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  if (! ensure_valid_trgeo_geo(temp, gs) )
    return -1;
  return ea_spatialrel_trgeo_poses_geo(temp, gs, &datum_geom_intersects2d,
    ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry ever intersect,
 * 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_trgeometry_geo()
 */
int
eintersects_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_trgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry and a temporal rigid geometry always
 * intersect, 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @note aIntersects(trgeo, gs) is equivalent to NOT eDisjoint(trgeo, gs)
 * @csqlfn #Aintersects_trgeometry_geo()
 */
int
aintersects_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_trgeo_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry are ever or
 * always intersecting, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Intersects is symmetric: delegates to ea_intersects_trgeo_geo
 */
int
ea_intersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_intersects_trgeo_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries are
 * ever or always intersecting, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_intersects_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_eq, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries ever intersect, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Eintersects_trgeometry_trgeometry()
 */
int
eintersects_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return ea_intersects_trgeo_trgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if the temporal rigid geometries always intersect, 0 if not,
 * and -1 on error or if the temporal rigid geometries do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Aintersects_trgeometry_trgeometry()
 */
int
aintersects_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return ea_intersects_trgeo_trgeo(temp1, temp2, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry ever touch, 0
 * if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Etouches_trgeometry_geo()
 */
int
etouches_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_spatialrel_trgeo_poses_geo(temp, gs, &datum_geom_touches, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry always touch,
 * 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Atouches_trgeometry_geo()
 */
int
atouches_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_spatialrel_trgeo_poses_geo(temp, gs, &datum_geom_touches, ALWAYS);
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry ever/always
 * touch, 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ever ? etouches_trgeometry_geo(temp, gs) : atouches_trgeometry_geo(temp, gs);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry ever/always
 * touch, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_touches_trgeo_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries
 * ever touch, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return -1;
  /* Common-time gate, as in #ea_contains_trgeo_trgeo: the placements carry no time */
  if (! temporal_time_overlaps(temp1, temp2))
    return -1;
  GSERIALIZED *places1 = trgeo_placements(temp1);
  GSERIALIZED *places2 = trgeo_placements(temp2);
  bool result = geom_touches(places1, places2);
  (void) ever;
  pfree(places1); pfree(places2);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always dwithin
 *
 * The functions use the temporal distance.  The bodies are ever within the
 * distance if the minimum of the temporal distance is at most the distance.
 * They are always within the distance if its maximum is at most the
 * distance.  For the maximum, the temporal distance has an instant at each
 * crossing of the distance, and at each extremum between two crossings, see
 * trgeo_distance.c.  Thus the maximum of its instants is above the distance
 * if the true distance is above it at some time, and the answer is exact.
 *****************************************************************************/

/**
 * @brief Return 1 if the minimum (@p ever) or the maximum of the temporal
 * distance @p tdist is at most @p dist, 0 if not, -1 if @p tdist is NULL
 * @param[in] tdist Temporal distance, freed
 */
static int
ea_dwithin_tdist(Temporal *tdist, double dist, bool ever)
{
  if (! tdist)
    return -1;
  double bound = DatumGetFloat8(ever ? temporal_min_value(tdist) :
    temporal_max_value(tdist));
  pfree(tdist);
  return (bound <= dist) ? 1 : 0;
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever or
 * always within a distance, 0 if not, -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_dwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool ever)
{
  if (! ensure_valid_trgeo_geo(temp, gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  return ea_dwithin_tdist(trgeo_tdistance_geo(temp, gs, ever ? -1.0 : dist),
    dist, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever within
 * a distance, 0 if not, -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_trgeometry_geo() #Edwithin_geo_trgeometry()
 */
int
edwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  return ea_dwithin_trgeo_geo(temp, gs, dist, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are always
 * within a distance, 0 if not, -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeometry_geo() #Adwithin_geo_trgeometry()
 */
int
adwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  return ea_dwithin_trgeo_geo(temp, gs, dist, ALWAYS);
}

/**
 * @brief Return 1 if two temporal rigid geometries are ever or always within
 * a distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_dwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  /* The ever semantics stops at the first distance within the bound */
  if (ever)
    return trgeo_edwithin_trgeo(temp1, temp2, dist);
  return ea_dwithin_tdist(trgeo_tdistance_trgeo(temp1, temp2, dist), dist,
    ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are ever within a
 * distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #Edwithin_trgeometry_trgeometry()
 */
int
edwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_trgeo_trgeo(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are always within a
 * distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeometry_trgeometry()
 */
int
adwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_trgeo_trgeo(temp1, temp2, dist, ALWAYS);
}

/*****************************************************************************/
