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
#include "rgeo/trgeo_spatialrels.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Return the temporal geometry that a temporal rigid geometry traces,
 * or NULL with an error if the rigid geometry has linear interpolation
 * @details A rigid geometry with step or discrete interpolation keeps its
 * pose from one of its instants to the next, so its value at an instant is
 * the value of the temporal geometry it traces. With linear interpolation the
 * body moves and turns between its instants, and the relationships are not
 * computed over that motion.
 * @param[in] temp Temporal rigid geometry
 */
static Temporal *
trgeometry_traced_tgeo(const Temporal *temp)
{
  if (! ensure_nonlinear_interp(temp->flags))
    return NULL;
  return trgeometry_to_tgeometry(temp);
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry ever/always
 * satisfy a spatial relationship, 0 if not, and -1 on error
 * @details The relationship is that of the temporal geometry the rigid
 * geometry traces (#trgeometry_traced_tgeo) and the geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] func Ever/always relationship of a temporal geometry and a
 * geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
ea_spatialrel_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs,
  int (*func)(const Temporal *, const GSERIALIZED *, bool), bool ever)
{
  if (! ensure_valid_trgeometry_geo(temp, gs))
    return -1;
  Temporal *tgeo = trgeometry_traced_tgeo(temp);
  if (! tgeo)
    return -1;
  int result = func(tgeo, gs, ever);
  pfree(tgeo);
  return result;
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry ever/always
 * satisfy a spatial relationship, 0 if not, and -1 on error
 * @details The relationship is that of the geometry and the temporal geometry
 * the rigid geometry traces (#trgeometry_traced_tgeo)
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] func Ever/always relationship of a geometry and a temporal
 * geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
ea_spatialrel_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp,
  int (*func)(const GSERIALIZED *, const Temporal *, bool), bool ever)
{
  if (! ensure_valid_trgeometry_geo(temp, gs))
    return -1;
  Temporal *tgeo = trgeometry_traced_tgeo(temp);
  if (! tgeo)
    return -1;
  int result = func(gs, tgeo, ever);
  pfree(tgeo);
  return result;
}

/**
 * @brief Return 1 if two temporal rigid geometries ever/always satisfy a
 * spatial relationship, 0 if not, and -1 on error or if they share no instant
 * @details The relationship is that of the temporal geometries the rigid
 * geometries trace (#trgeometry_traced_tgeo)
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] func Ever/always relationship of two temporal geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
ea_spatialrel_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  int (*func)(const Temporal *, const Temporal *, bool), bool ever)
{
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2))
    return -1;
  /* Common-time gate: two rigid geometries sharing no instant are compared
   * nowhere, so the relationship has no answer */
  if (! temporal_time_overlaps(temp1, temp2))
    return -1;
  Temporal *tgeo1 = trgeometry_traced_tgeo(temp1);
  if (! tgeo1)
    return -1;
  Temporal *tgeo2 = trgeometry_traced_tgeo(temp2);
  if (! tgeo2)
  {
    pfree(tgeo1);
    return -1;
  }
  int result = func(tgeo1, tgeo2, ever);
  pfree(tgeo1); pfree(tgeo2);
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
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_contains_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_spatialrel_geo_trgeometry(gs, temp, &ea_contains_geo_tgeo, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal rigid geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_trgeometry()
 */
int
econtains_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_trgeometry(gs, temp, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal rigid geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_trgeometry()
 */
int
acontains_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_trgeometry(gs, temp, ALWAYS);
}

/**
 * @brief Return 1 if a temporal rigid geometry ever/always contains
 * another, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_contains_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_trgeometry_trgeometry(temp1, temp2, &ea_contains_tgeo_tgeo,
    ever);
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
ea_contains_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_contains_tgeo_geo, ever);
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
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_covers_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_spatialrel_geo_trgeometry(gs, temp, &ea_covers_geo_tgeo, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever covers a temporal geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Ecovers_geo_trgeometry()
 */
int
ecovers_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_geo_trgeometry(gs, temp, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry always covers a temporal geometry,
 * 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acovers_geo_trgeometry()
 */
int
acovers_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_geo_trgeometry(gs, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a geometry ever covers a temporal geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 */
int
ea_covers_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_covers_tgeo_geo, ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a geometry ever covers a temporal geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Ecovers_trgeometry_geo()
 */
int
ecovers_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_trgeometry_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal geometry always covers a geometry, 0 if not,
 * and -1 on error
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @note The relationship holds at an instant where it holds for the body
 * placed at the pose of that instant
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acovers_trgeometry_geo()
 */
int
acovers_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_trgeometry_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if a temporal rigid geometry ever/always covers
 * another, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_covers_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_trgeometry_trgeometry(temp1, temp2, &ea_covers_tgeo_tgeo,
    ever);
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
ea_disjoint_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) )
    return -1;
  /* A body in linear motion is ever disjoint unless it is always in contact,
   * and always disjoint unless it is ever in contact; it is always disjoint
   * from an empty geometry */
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return gserialized_is_empty(gs) ? 1 :
      INVERT_RESULT(ea_dwithin_trgeometry_geo(temp, gs, 0.0, ! ever));
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_disjoint_tgeo_geo, ever);
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
  return ea_disjoint_trgeometry_geo(temp, gs, EVER);
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
  return ea_disjoint_trgeometry_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry are ever or
 * always disjoint, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Disjoint is symmetric: delegates to ea_disjoint_trgeometry_geo
 */
int
ea_disjoint_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_disjoint_trgeometry_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries are
 * ever or always disjoint, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_disjoint_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  /* See #ea_intersects_trgeometry_trgeometry */
  if (MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
    return INVERT_RESULT(ea_dwithin_trgeometry_trgeometry(temp1, temp2, 0.0,
      ! ever));
  return ea_spatialrel_trgeometry_trgeometry(temp1, temp2, &ea_disjoint_tgeo_tgeo,
    ever);
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
  return ea_disjoint_trgeometry_trgeometry(temp1, temp2, EVER);
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
  return ea_disjoint_trgeometry_trgeometry(temp1, temp2, ALWAYS);
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
ea_intersects_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  if (! ensure_valid_trgeometry_geo(temp, gs) )
    return -1;
  /* A body in linear motion meets a geometry between its poses as well, where
   * the temporal distance reaches zero, and never meets an empty one */
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return gserialized_is_empty(gs) ? 0 :
      ea_dwithin_trgeometry_geo(temp, gs, 0.0, ever);
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_intersects_tgeo_geo, ever);
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
  return ea_intersects_trgeometry_geo(temp, gs, EVER);
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
  return ea_intersects_trgeometry_geo(temp, gs, ALWAYS);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry are ever or
 * always intersecting, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Intersects is symmetric: delegates to ea_intersects_trgeometry_geo
 */
int
ea_intersects_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_intersects_trgeometry_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries are
 * ever or always intersecting, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_intersects_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  /* Two bodies of which one moves linearly meet between their poses as well,
   * where their temporal distance reaches zero */
  if (MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
    return ea_dwithin_trgeometry_trgeometry(temp1, temp2, 0.0, ever);
  return ea_spatialrel_trgeometry_trgeometry(temp1, temp2, &ea_intersects_tgeo_tgeo,
    ever);
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
  return ea_intersects_trgeometry_trgeometry(temp1, temp2, EVER);
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
  return ea_intersects_trgeometry_trgeometry(temp1, temp2, ALWAYS);
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
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_touches_tgeo_geo, EVER);
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
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_touches_tgeo_geo, ALWAYS);
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry ever/always
 * touch, 0 if not, and -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_trgeometry_geo(temp, gs, &ea_touches_tgeo_geo, ever);
}

/**
 * @brief Return 1 if a geometry and a temporal rigid geometry ever/always
 * touch, 0 if not, and -1 on error
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_touches_trgeometry_geo(temp, gs, ever);
}

/**
 * @brief Return 1 if the placements of two temporal rigid geometries
 * ever touch, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_touches_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_trgeometry_trgeometry(temp1, temp2, &ea_touches_tgeo_tgeo,
    ever);
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
ea_dwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool ever)
{
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  /* The ever semantics stops at the first distance within the bound */
  if (ever)
    return edwithin_trgeometry_geo_intl(temp, gs, dist);
  return ea_dwithin_tdist(tdistance_trgeometry_geo_intl(temp, gs, dist), dist,
    ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever within
 * a distance, 0 if not, -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_trgeometry_geo() #Edwithin_geo_trgeometry()
 * @errval -1
 */
int
edwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  if (! ensure_finite(dist))
    return -1;
  return ea_dwithin_trgeometry_geo(temp, gs, dist, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if a temporal rigid geometry and a geometry are always
 * within a distance, 0 if not, -1 on error
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeometry_geo() #Adwithin_geo_trgeometry()
 * @errval -1
 */
int
adwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  if (! ensure_finite(dist))
    return -1;
  return ea_dwithin_trgeometry_geo(temp, gs, dist, ALWAYS);
}

/**
 * @brief Return 1 if two temporal rigid geometries are ever or always within
 * a distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_dwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  /* The ever semantics stops at the first distance within the bound */
  if (ever)
    return edwithin_trgeometry_trgeometry_intl(temp1, temp2, dist);
  return ea_dwithin_tdist(tdistance_trgeometry_trgeometry_intl(temp1, temp2, dist), dist,
    ever);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are ever within a
 * distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #Edwithin_trgeometry_trgeometry()
 * @errval -1
 */
int
edwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, double dist)
{
  if (! ensure_finite(dist))
    return -1;
  return ea_dwithin_trgeometry_trgeometry(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_rgeo_rel_ever
 * @brief Return 1 if two temporal rigid geometries are always within a
 * distance, 0 if not, -1 on error or if they do not intersect in time
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @csqlfn #Adwithin_trgeometry_trgeometry()
 * @errval -1
 */
int
adwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2, double dist)
{
  if (! ensure_finite(dist))
    return -1;
  return ea_dwithin_trgeometry_trgeometry(temp1, temp2, dist, ALWAYS);
}

/*****************************************************************************/

/*****************************************************************************
 * Temporal disjoint and intersects
 *****************************************************************************/

/**
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry and a geometry intersect or are disjoint
 * @details A body in linear motion moves between its instants, which a temporal
 * geometry cannot state, so the answer is that of its temporal distance, which
 * has an instant at each time at which it reaches zero. The other interpolations
 * hold their value between the instants and are those of the temporal geometry
 * the body traces.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] disjoint True for disjoint, false for intersects
 */
static Temporal *
tinterrel_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool disjoint)
{
  if (! ensure_valid_trgeometry_geo(temp, gs))
    return NULL;
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags) && ! gserialized_is_empty(gs))
  {
    Temporal *within = tdwithin_trgeometry_geo(temp, gs, 0.0);
    if (! disjoint || ! within)
      return within;
    Temporal *result = tnot_tbool(within);
    pfree(within);
    return result;
  }
  Temporal *tgeo = trgeometry_to_tgeometry(temp);
  if (! tgeo)
    return NULL;
  Temporal *result = disjoint ? tdisjoint_tgeo_geo(tgeo, gs) :
    tintersects_tgeo_geo(tgeo, gs);
  pfree(tgeo);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries intersect or are disjoint
 * @details See #tinterrel_trgeometry_geo
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] disjoint True for disjoint, false for intersects
 */
static Temporal *
tinterrel_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  bool disjoint)
{
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2))
    return NULL;
  if (MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
  {
    Temporal *within = tdwithin_trgeometry_trgeometry(temp1, temp2, 0.0);
    if (! disjoint || ! within)
      return within;
    Temporal *result = tnot_tbool(within);
    pfree(within);
    return result;
  }
  Temporal *tgeo1 = trgeometry_to_tgeometry(temp1);
  Temporal *tgeo2 = tgeo1 ? trgeometry_to_tgeometry(temp2) : NULL;
  if (! tgeo2)
  {
    if (tgeo1)
      pfree(tgeo1);
    return NULL;
  }
  Temporal *result = disjoint ? tdisjoint_tgeo_tgeo(tgeo1, tgeo2) :
    tintersects_tgeo_tgeo(tgeo1, tgeo2);
  pfree(tgeo1); pfree(tgeo2);
  return result;
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry and a
 * temporal rigid geometry are disjoint
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tdisjoint_geo_trgeometry()
 */
Temporal *
tdisjoint_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_trgeometry_geo(temp, gs, true);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry and a geometry are disjoint
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tdisjoint_trgeometry_geo()
 */
Temporal *
tdisjoint_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_trgeometry_geo(temp, gs, true);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries are disjoint
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Tdisjoint_trgeometry_trgeometry()
 */
Temporal *
tdisjoint_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return tinterrel_trgeometry_trgeometry(temp1, temp2, true);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry and a
 * temporal rigid geometry intersect
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tintersects_geo_trgeometry()
 */
Temporal *
tintersects_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return tinterrel_trgeometry_geo(temp, gs, false);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry and a geometry intersect
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tintersects_trgeometry_geo()
 */
Temporal *
tintersects_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tinterrel_trgeometry_geo(temp, gs, false);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries intersect
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Tintersects_trgeometry_trgeometry()
 */
Temporal *
tintersects_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return tinterrel_trgeometry_trgeometry(temp1, temp2, false);
}

/*****************************************************************************/
