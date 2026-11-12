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
 * @brief Ever/always and temporal comparisons for temporal geos
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

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
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  if (! ensure_valid_tspatial_geo(temp, gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return -1;
  /* An empty geometry has no point, so it equals no value at any instant */
  if (gserialized_is_empty(gs))
    return (func == &datum2_ne) ? 1 : 0;
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
  Datum (*func)(Datum, Datum, MeosType), bool ever)
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
int
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
int
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
int
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
int
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
int
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
int
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
int
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
int
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
int
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
int
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
int
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
int
always_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tgeo_tgeo(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Ever/always overlaps of a temporal geo and a spatiotemporal box
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal geo ever (or always) lies in a spatiotemporal
 * box, 0 if not, and -1 on error, as #eacomp_tgeo_geo answers an ever or
 * always comparison
 * @details A temporal geo ever lies in the box when its restriction to the
 * box by #tgeo_restrict_stbox is not empty, and always lies in it when its
 * restriction to the complement of the box is empty; the borders of the box
 * belong to it, as the overlaps operator of the bounding boxes reads them
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tgeo_stbox(const Temporal *temp, const STBox *box, bool ever)
{
  /* Ensure the validity of the arguments: a box with only the T dimension
   * compares its period, and a box with the X dimension shares the SRID and
   * the geodetic flag of the temporal value, as #tgeo_restrict_stbox reads
   * them */
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(box, -1);
  if (MEOS_FLAGS_GET_X(box->flags) &&
      (! ensure_same_srid(tspatial_srid(temp), stbox_srid(box)) ||
       ! ensure_same_geodetic(temp->flags, box->flags) ||
       /* Generic 3D geometries cannot be restricted to a spatiotemporal box */
       (tgeo_type(temp->temptype) &&
        ! ensure_has_not_Z(temp->temptype, temp->flags))))
    return -1;

  Temporal *rest = tgeo_restrict_stbox(temp, box, true,
    ever ? REST_AT : REST_MINUS);
  int result = ever ? (rest != NULL) : (rest == NULL);
  if (rest)
    pfree(rest);
  return result;
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if a temporal geo ever lies in a spatiotemporal box, 0 if
 * not, and -1 on error
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @csqlfn #Ever_overlaps_tgeo_stbox()
 */
int
ever_overlaps_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tgeo_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if a temporal geo ever lies in a spatiotemporal box, 0 if
 * not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal geo
 * @csqlfn #Ever_overlaps_stbox_tgeo()
 */
int
ever_overlaps_stbox_tgeo(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tgeo_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if a temporal geo always lies in a spatiotemporal box, 0 if
 * not, and -1 on error
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @csqlfn #Always_overlaps_tgeo_stbox()
 */
int
always_overlaps_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tgeo_stbox(temp, box, ALWAYS);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if a temporal geo always lies in a spatiotemporal box, 0 if
 * not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal geo
 * @csqlfn #Always_overlaps_stbox_tgeo()
 */
int
always_overlaps_stbox_tgeo(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tgeo_stbox(temp, box, ALWAYS);
}

/**
 * @brief Return true if the bounding boxes of two geos overlap, the
 * comparison #datum2_eq is to the equality
 * @note The temporal geos are validated by #eacomp_tgeo_tgeo before the
 * instants are compared, so the boxes are compared by #stbox_overlaps
 */
static Datum
datum2_geo_box_overlaps(Datum geo1, Datum geo2, MeosType basetype UNUSED)
{
  STBox box1, box2;
  geo_set_stbox(DatumGetGserializedP(geo1), &box1);
  geo_set_stbox(DatumGetGserializedP(geo2), &box2);
  return BoolGetDatum(stbox_overlaps(&box1, &box2));
}

/**
 * @brief Return 1 if the values of two temporal geos ever (or always) have
 * overlapping bounding boxes at an instant they share, 0 if not, and -1 on
 * error, as #eacomp_tgeo_tgeo answers an ever or always comparison
 * @details The bounding box of a point is the point, so two temporal points
 * overlap at an instant when they are equal at it, which #datum2_eq answers
 * across the instants where linear segments cross; the other temporal geos
 * keep their value between two instants, so their instants decide
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  return eacomp_tgeo_tgeo(temp1, temp2, tpoint_type(temp1->temptype) ?
    &datum2_eq : &datum2_geo_box_overlaps, ever);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if the values of two temporal geos ever have overlapping
 * bounding boxes at an instant they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_overlaps_tgeo_tgeo()
 */
int
ever_overlaps_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_comp_ever
 * @brief Return 1 if the values of two temporal geos always have overlapping
 * bounding boxes at the instants they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_overlaps_tgeo_tgeo()
 */
int
always_overlaps_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tgeo_tgeo(temp1, temp2, ALWAYS);
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
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tspatial_geo(temp, gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return NULL;
  /* An empty geometry has no point, so it equals no value at any instant */
  if (gserialized_is_empty(gs))
    return temporal_from_base_temp(BoolGetDatum(func == &datum2_ne),
      T_TBOOL, temp);
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
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_tspatial_geo(temp, gs) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return NULL;
  /* An empty geometry has no point, so it equals no value at any instant */
  if (gserialized_is_empty(gs))
    return temporal_from_base_temp(BoolGetDatum(func == &datum2_ne),
      T_TBOOL, temp);
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
Temporal *
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
Temporal *
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
Temporal *
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
Temporal *
tne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_tgeo_geo(temp, gs, &datum2_ne);
}

/*****************************************************************************/
