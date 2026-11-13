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
 * @brief Ever/always and temporal comparisons for temporal network points
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "temporal/temporal_compops.h"
#include "geo/geo_funcs.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal network point and a network point satisfy
 * the ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] np Network point
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, -1); VALIDATE_NOT_NULL(np, -1);
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(np), func, ever);
}

/**
 * @brief Return true if two temporal network points satisfy the ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, -1); VALIDATE_TNPOINT(temp2, -1);
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is ever equal to a temporal network
 * point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Ever_eq_npoint_tnpoint()
 */
int
ever_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is ever equal to a network
 * point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Ever_eq_tnpoint_npoint()
 */
int
ever_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is ever different from a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Ever_ne_npoint_tnpoint()
 */
int
ever_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is ever different from a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Ever_ne_tnpoint_npoint()
 */
int
ever_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is always equal to a temporal network
 * point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Always_eq_npoint_tnpoint()
 */
int
always_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is always equal to a network
 * point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Always_eq_tnpoint_npoint()
 */
int
always_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is always different from a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Always_ne_npoint_tnpoint()
 */
int
always_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is always different from a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Always_ne_tnpoint_npoint()
 */
int
always_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are ever equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_eq_tnpoint_tnpoint()
 */
int
ever_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are ever different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_ne_tnpoint_tnpoint()
 */
int
ever_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are always equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_eq_tnpoint_tnpoint()
 */
int
always_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are always different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_ne_tnpoint_tnpoint()
 */
int
always_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Ever/always overlaps of a temporal network point and a spatiotemporal box
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal network point ever (or always) lies in a
 * spatiotemporal box, 0 if not, and -1 on error, as #eaoverlaps_tgeo_stbox
 * answers it for a temporal geo
 * @details The value ever lies in the box when its restriction to the box by
 * #tnpoint_restrict_stbox is not empty, and always lies in it when its
 * restriction to the complement of the box is empty; a box with only the T
 * dimension restricts the value to its period
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tnpoint_stbox(const Temporal *temp, const STBox *box, bool ever)
{
  /* Ensure the validity of the arguments: a box with the X dimension shares
   * the SRID of the value, as #tnpoint_restrict_stbox reads it */
  VALIDATE_TNPOINT(temp, -1); VALIDATE_NOT_NULL(box, -1);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  if (hasx && ! ensure_valid_tnpoint_stbox(temp, box))
    return -1;

  uint8 rest = ever ? REST_AT : REST_MINUS;
  Temporal *res = hasx ? tnpoint_restrict_stbox(temp, box, true, rest) :
    temporal_restrict_tstzspan(temp, &box->period, rest);
  int result = ever ? (res != NULL) : (res == NULL);
  if (res)
    pfree(res);
  return result;
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if a temporal network point ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Ever_overlaps_tnpoint_stbox()
 */
int
ever_overlaps_tnpoint_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tnpoint_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if a temporal network point ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal network point
 * @csqlfn #Ever_overlaps_stbox_tnpoint()
 */
int
ever_overlaps_stbox_tnpoint(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tnpoint_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if a temporal network point always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal network point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Always_overlaps_tnpoint_stbox()
 */
int
always_overlaps_tnpoint_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tnpoint_stbox(temp, box, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if a temporal network point always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal network point
 * @csqlfn #Always_overlaps_stbox_tnpoint()
 */
int
always_overlaps_stbox_tnpoint(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tnpoint_stbox(temp, box, ALWAYS);
}

/**
 * @brief Return the distance within which the points the routes of two
 * temporal network points construct at one instant are one place
 * @details A position along a route is a point computed in doubles, as
 * #npoint_to_geompoint computes it, so two network points at one place of
 * the map, a crossing of their routes, give two points apart by the rounding
 * of their coordinates, and the exact distance between them is almost never
 * zero. The distance is the band of a constructed point the exact
 * computation of MEOS admits, as #geogpoint_meeting_band reads it for the
 * meeting of two geography points: #MEOS_GEOM_TOLERANCE plus four units of
 * rounding of the largest
 * coordinate of the two values, which no two points genuinely apart are
 * within
 * @param[in] box1,box2 Spatiotemporal boxes of the two values
 */
static double
tnpoint_meeting_band(const STBox *box1, const STBox *box2)
{
  double mag1 = Max(Max(fabs(box1->xmin), fabs(box1->xmax)),
    Max(fabs(box1->ymin), fabs(box1->ymax)));
  double mag2 = Max(Max(fabs(box2->xmin), fabs(box2->xmax)),
    Max(fabs(box2->ymin), fabs(box2->ymax)));
  return MEOS_GEOM_TOLERANCE + 4.0 * DBL_EPSILON * Max(mag1, mag2);
}

/**
 * @brief Return 1 if two temporal network points are ever (or always) at one
 * place of the map at a time they share, 0 if not, and -1 on error
 * @details Two network points are at one place where the points their routes
 * construct for them coincide: on one route at one fraction, at a junction
 * where one route ends and another starts, or at a crossing of two routes,
 * the spatial equality of the two values as the equality of their geometries
 * is to `=` and `<>`, which compare the route and the fraction. The
 * positions are constructed points, so they coincide within the band of
 * #tnpoint_meeting_band, and the values are at one place at a time where the
 * temporal points of their positions (#tnpoint_to_tgeompoint) are within that
 * distance (#edwithin_tgeo_tgeo, #adwithin_tgeo_tgeo)
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return -1;
  STBox box1, box2;
  tspatial_set_stbox(temp1, &box1);
  tspatial_set_stbox(temp2, &box2);
  double band = tnpoint_meeting_band(&box1, &box2);
  Temporal *tpoint1 = tnpoint_to_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_to_tgeompoint(temp2);
  int result = -1;
  if (tpoint1 && tpoint2)
    result = ever ? edwithin_tgeo_tgeo(tpoint1, tpoint2, band, false) :
      adwithin_tgeo_tgeo(tpoint1, tpoint2, band, false);
  if (tpoint1)
    pfree(tpoint1);
  if (tpoint2)
    pfree(tpoint2);
  return result;
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if two temporal network points are ever at one place of
 * the map at a time they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal network points
 * @errval -1
 * @csqlfn #Ever_overlaps_tnpoint_tnpoint()
 */
int
ever_overlaps_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tnpoint_tnpoint(temp1, temp2, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return 1 if two temporal network points are always at one place of
 * the map at the times they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal network points
 * @errval -1
 * @csqlfn #Always_overlaps_tnpoint_tnpoint()
 */
int
always_overlaps_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tnpoint_tnpoint(temp1, temp2, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a temporal network point and a
 * network point
 * @param[in] temp Temporal value
 * @param[in] np Network point
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(np), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_comp_temp
 * @brief Return the temporal equality of a temporal network point and a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Teq_tnpoint_npoint() #Teq_npoint_tnpoint()
 */
Temporal *
teq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return tcomp_tnpoint_npoint(temp, np, &datum2_eq);
}

/**
 * @ingroup meos_npoint_comp_temp
 * @brief Return the temporal inequality of a temporal network point and a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Tne_tnpoint_npoint() #Tne_npoint_tnpoint()
 */
Temporal *
tne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return tcomp_tnpoint_npoint(temp, np, &datum2_ne);
}

/*****************************************************************************/
