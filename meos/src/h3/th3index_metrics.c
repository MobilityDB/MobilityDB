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
 * @brief MEOS lifting for metric functions, plus the static
 * unit-string dispatcher and libh3-wrapping adapters that back
 * them.
 *
 * All three entries take a text `unit` argument. The unit is
 * validated once at the MEOS entry point (via `h3_unit_from_cstring`
 * defined below) and then carried through the lifting machinery as
 * an `Int32` datum holding the `H3Unit` enum — saves parsing the
 * string at every instant.
 */

/* C */
#include <string.h>
/* H3 */
#include <h3api.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "geo/tgeo_spatialfuncs.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"
#include "h3/th3index_internal.h"

/*****************************************************************************
 * Unit-string dispatcher (h3-pg: miscellaneous.c)
 *****************************************************************************/

/*****************************************************************************
 * Static adapters — libh3 metric dispatch by unit
 *****************************************************************************/

/**
 * @brief 
 */
double
h3_cell_area_meos(H3Index cell, H3Unit unit)
{
  double area = 0.0;
  H3Error err = E_SUCCESS;
  switch (unit)
  {
    case H3_UNIT_KM2:   err = cellAreaKm2(cell, &area); break;
    case H3_UNIT_M2:    err = cellAreaM2(cell, &area); break;
    case H3_UNIT_RADS2: err = cellAreaRads2(cell, &area); break;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "h3_cell_area_meos: expected an area unit (km2, m2, rads2)");
      return 0.0;
  }
  if (err != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return 0.0;
  }
  return area;
}

/**
 * @brief 
 */
double
h3_edge_length_meos(H3Index edge, H3Unit unit)
{
  double len = 0.0;
  H3Error err = E_SUCCESS;
  switch (unit)
  {
    case H3_UNIT_KM:   err = edgeLengthKm(edge, &len); break;
    case H3_UNIT_M:    err = edgeLengthM(edge, &len); break;
    case H3_UNIT_RADS: err = edgeLengthRads(edge, &len); break;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "h3_edge_length: expected a length unit (km, m, rads)");
      return 0.0;
  }
  if (err != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return 0.0;
  }
  return len;
}

/**
 * @brief 
 */
double
h3_gs_great_circle_distance_meos(const GSERIALIZED *a, const GSERIALIZED *b,
  H3Unit unit)
{
  if (! ensure_srid_is_latlong(gserialized_get_srid(a)) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(b)))
    return 0.0;
  const POINT2D *pa = GSERIALIZED_POINT2D_P(a);
  const POINT2D *pb = GSERIALIZED_POINT2D_P(b);
  LatLng la = { .lat = degsToRads(pa->y), .lng = degsToRads(pa->x) };
  LatLng lb = { .lat = degsToRads(pb->y), .lng = degsToRads(pb->x) };
  switch (unit)
  {
    case H3_UNIT_KM:   return greatCircleDistanceKm(&la, &lb);
    case H3_UNIT_M:    return greatCircleDistanceM(&la, &lb);
    case H3_UNIT_RADS: return greatCircleDistanceRads(&la, &lb);
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "h3_great_circle_distance: expected a length unit "
        "(km, m, rads)");
      return 0.0;
  }
}

/*****************************************************************************
 * th3EdgeLength — lift_with_const
 *****************************************************************************/

/**
 * @brief Return the per-instant length of a temporal H3 directed edge in
 * the given H3 unit
 */
static Temporal *
th3index_edge_length_in(const Temporal *temp, H3Unit u)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_edge_length;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum((int32) u);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_h3_metrics
 * @brief Return the per-instant length of a temporal H3 directed edge in
 * metres, the quantity libh3 answers as edgeLengthM
 * @csqlfn #Th3index_edge_length()
 */
Temporal *
th3index_edge_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(temp, NULL);
  return th3index_edge_length_in(temp, H3_UNIT_M);
}

/*****************************************************************************
 * greatCircleDistanceKm, greatCircleDistanceM and greatCircleDistanceRads —
 * binary_synced
 *
 * Two temporal geodetic points are synchronised over their shared
 * time axis; the unit is constant across instants and is threaded
 * through via `lfinfo.param[0]` — the existing binary-synced
 * lifting machinery supports it without needing a dedicated
 * `tfunc_temporal_temporal_const` variant.
 *****************************************************************************/

/**
 * @brief Return the per-instant great-circle distance between two temporal
 * geodetic points in the given H3 unit
 */
static Temporal *
tgeogpoint_great_circle_distance_in(const Temporal *a, const Temporal *b,
  H3Unit u)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_great_circle_distance;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum((int32) u);
  lfinfo.argtype[0] = T_TGEOGPOINT;
  lfinfo.argtype[1] = T_TGEOGPOINT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(a, b, &lfinfo);
}

/**
 * @ingroup meos_h3_metrics
 * @brief Return the per-instant great-circle distance between two temporal
 * geodetic points in metres, the quantity libh3 answers as
 * greatCircleDistanceM
 * @csqlfn #Tgeogpoint_great_circle_distance()
 */
Temporal *
tgeogpoint_great_circle_distance(const Temporal *a, const Temporal *b)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(a, NULL); VALIDATE_TGEOGPOINT(b, NULL);
  return tgeogpoint_great_circle_distance_in(a, b, H3_UNIT_M);
}

/*****************************************************************************/
