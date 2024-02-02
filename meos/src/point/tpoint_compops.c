/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Spatial functions for temporal points
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
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal point and a point satisfy the ever/always
 * comparison
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tpoint_point(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_point_type(gs) ||
      ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(gs), func, ever);
}

/**
 * @brief Return true if two temporal points satisfy the ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a point is ever equal to a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Ever_eq_point_tpoint()
 */
int
ever_eq_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tpoint_point(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Ever_eq_tpoint_point()
 */
int
ever_eq_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tpoint_point(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a point is ever different to a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Ever_ne_point_tpoint()
 */
int
ever_ne_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tpoint_point(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal point is ever different to a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Ever_ne_tpoint_point()
 */
int
ever_ne_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tpoint_point(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a point is always equal to a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Always_eq_point_tpoint()
 */
int
always_eq_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tpoint_point(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal point is always equal to a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Always_eq_tpoint_point()
 */
int
always_eq_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tpoint_point(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a point is always different from a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Always_ne_point_tpoint()
 */
int
always_ne_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_tpoint_point(temp, gs, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal point is always different from a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Always_ne_tpoint_point()
 */
int
always_ne_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_tpoint_point(temp, gs, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal points are ever equal
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Ever_eq_tpoint_tpoint()
 */
int
ever_eq_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpoint_tpoint(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal points are ever different
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Ever_ne_tpoint_tpoint()
 */
int
ever_ne_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpoint_tpoint(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal points are always equal
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Always_eq_tpoint_tpoint()
 */
int
always_eq_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpoint_tpoint(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if two temporal points are always different
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Always_ne_tpoint_tpoint()
 */
int
always_ne_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpoint_tpoint(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a point and a temporal point
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_point_tpoint(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_point_type(gs) ||
      ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(gs), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal point and a point
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tpoint_point(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_point_type(gs) ||
      ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(gs), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a point and a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Teq_point_tpoint()
 */
Temporal *
teq_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_point_tpoint(gs, temp, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a point and a temporal point
 * @param[in] gs Point
 * @param[in] temp Temporal point
 * @csqlfn #Tne_point_tpoint()
 */
Temporal *
tne_point_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_point_tpoint(gs, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal point and a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Teq_tpoint_point()
 */
Temporal *
teq_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_tpoint_point(temp, gs, &datum2_eq);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal point and a point
 * @param[in] temp Temporal point
 * @param[in] gs Point
 * @csqlfn #Tne_tpoint_point()
 */
Temporal *
tne_tpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_tpoint_point(temp, gs, &datum2_ne);
}

/*****************************************************************************/
