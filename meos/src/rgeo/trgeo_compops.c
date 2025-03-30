/***********************************************************************
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
 * @brief Ever/always and temporal comparisons for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "general/pg_types.h"
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/temporal_compops.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal geo and a geo satisfy the ever/always
 * comparison
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return -1;
#else
  assert(temp); assert(gs); assert(func);
  assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_not_empty(gs) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(gs), func, ever);
}

/**
 * @brief Return true if two temporal poses satisfy the ever/always
 * comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) || 
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp1, T_TRGEOMETRY) ||
      ! ensure_temporal_isof_type(temp2, T_TRGEOMETRY))
    return -1;
#else
  assert(temp1); assert(temp2); assert(func); 
  assert(temp1->temptype == T_TRGEOMETRY); assert(temp2->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is ever equal to a temporal circular
 * buffer
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_geo_trgeo()
 */
int
ever_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is ever equal to a circular
 * buffer
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_eq_trgeo_geo()
 */
int
ever_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is ever different from a temporal geo
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_geo_trgeo()
 */
int
ever_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is ever different from a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_ne_trgeo_geo()
 */
int
ever_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is always equal to a temporal geo
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_geo_trgeo()
 */
int
always_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is always equal to a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_eq_trgeo_geo()
 */
int
always_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is always different from a temporal geo
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_geo_trgeo()
 */
int
always_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is always different from a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_ne_trgeo_geo()
 */
int
always_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal poses are ever equal
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Ever_eq_trgeo_trgeo()
 */
int
ever_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal poses are ever different
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Ever_ne_trgeo_trgeo()
 */
int
ever_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal poses are always equal
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Always_eq_trgeo_trgeo()
 */
int
always_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal poses are always different
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Always_ne_trgeo_trgeo()
 */
int
always_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a geo and a temporal geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(gs); assert(func);
  assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  /* Empty geometries accepted, i.e., there is no associated error message */
  if (gserialized_is_empty(gs) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(gs), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal geo and a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(gs); assert(func);
  assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_not_empty(gs) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(gs), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal equality of a geo and a temporal geo
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Teq_geo_trgeo()
 */
Temporal *
teq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_trgeo(gs, temp, &datum2_eq);
}

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal inequality of a geo and a temporal geo
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Tne_geo_trgeo()
 */
Temporal *
tne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_trgeo(gs, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal equality of a temporal geo and a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Teq_trgeo_geo()
 */
Temporal *
teq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_eq);
}

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal inequality of a temporal geo and a
 * geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Tne_trgeo_geo()
 */
Temporal *
tne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_ne);
}

/*****************************************************************************/
