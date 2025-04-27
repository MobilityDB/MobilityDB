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
 * @brief Spatial functions for temporal geos
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
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
int
eacomp_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(gs), func, ever);
}

/**
 * @brief Return true if two temporal geos satisfy the ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is ever equal to a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Ever_eq_geo_tgeo()
 */
inline int
ever_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is ever equal to a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Ever_eq_tgeo_geo()
 */
inline int
ever_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is ever different from a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Ever_ne_geo_tgeo()
 */
inline int
ever_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is ever different from a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Ever_ne_tgeo_geo()
 */
inline int
ever_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is always equal to a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Always_eq_geo_tgeo()
 */
inline int
always_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is always equal to a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Always_eq_tgeo_geo()
 */
inline int
always_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a geo is always different from a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Always_ne_geo_tgeo()
 */
inline int
always_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if a temporal geo is always different from a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Always_ne_tgeo_geo()
 */
inline int
always_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal geos are ever equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_eq_tgeo_tgeo()
 */
inline int
ever_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tgeo_tgeo(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal geos are ever different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_ne_tgeo_tgeo()
 */
inline int
ever_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tgeo_tgeo(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal geos are always equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_eq_tgeo_tgeo()
 */
inline int
always_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tgeo_tgeo(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return true if two temporal geos are always different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_ne_tgeo_tgeo()
 */
inline int
always_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tgeo_tgeo(temp1, temp2, &datum2_ne, ALWAYS);
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
tcomp_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(gs), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal geo and a geo
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tspatial_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(gs), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal equality of a geo and a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Teq_geo_tgeo()
 */
inline Temporal *
teq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_tgeo(gs, temp, &datum2_eq);
}

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal inequality of a geo and a temporal geo
 * @param[in] gs Geo
 * @param[in] temp Temporal geo
 * @csqlfn #Tne_geo_tgeo()
 */
inline Temporal *
tne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_tgeo(gs, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal equality of a temporal geo and a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Teq_tgeo_geo()
 */
inline Temporal *
teq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_tgeo_geo(temp, gs, &datum2_eq);
}

/**
 * @ingroup meos_geo_comp_temp
 * @brief Return the temporal inequality of a temporal geo and a geo
 * @param[in] temp Temporal geo
 * @param[in] gs Geo
 * @csqlfn #Tne_tgeo_geo()
 */
inline Temporal *
tne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_tgeo_geo(temp, gs, &datum2_ne);
}

/*****************************************************************************/
