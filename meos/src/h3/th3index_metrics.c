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

#include <string.h>

#include <meos.h>
#include <meos_h3.h>
#include <h3api.h>

#include "geo/tgeo_spatialfuncs.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * Unit-string dispatcher (h3-pg: miscellaneous.c)
 *****************************************************************************/

H3Unit
h3_unit_from_cstring(const char *unit)
{
  if (unit == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unit must not be null");
    return H3_UNIT_KM;  /* unreachable */
  }
  if (strcasecmp(unit, "km") == 0)    return H3_UNIT_KM;
  if (strcasecmp(unit, "m") == 0)     return H3_UNIT_M;
  if (strcasecmp(unit, "rads") == 0)  return H3_UNIT_RADS;
  if (strcasecmp(unit, "km2") == 0)   return H3_UNIT_KM2;
  if (strcasecmp(unit, "m2") == 0)    return H3_UNIT_M2;
  if (strcasecmp(unit, "rads2") == 0) return H3_UNIT_RADS2;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "invalid h3 unit \"%s\" (expected one of km, m, rads, km2, m2, rads2)",
    unit);
  return H3_UNIT_KM;  /* unreachable */
}

/*****************************************************************************
 * Static adapters — libh3 metric dispatch by unit
 *****************************************************************************/

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
        "h3_cell_area: expected an area unit (km2, m2, rads2)");
      return 0.0;
  }
  if (err != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return 0.0;
  }
  return area;
}

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
 * h3_cell_area(th3index, text) — lift_with_const (unit as H3Unit enum)
 *****************************************************************************/

/**
 * @ingroup meos_h3_metrics
 * @brief Return the per-instant area of a temporal H3 cell in the given
 * unit.
 */
Temporal *
th3index_cell_area(const Temporal *temp, const char *unit)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  H3Unit u = h3_unit_from_cstring(unit);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_area;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum((int32) u);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_edge_length(th3index, text) — lift_with_const
 *****************************************************************************/

/**
 * @ingroup meos_h3_metrics
 * @brief Return the per-instant length of a temporal H3 directed edge in
 * the given unit.
 */
Temporal *
th3index_edge_length(const Temporal *temp, const char *unit)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  H3Unit u = h3_unit_from_cstring(unit);

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

/*****************************************************************************
 * h3_great_circle_distance(tgeogpoint, tgeogpoint, text) — binary_synced
 *
 * Two temporal geodetic points are synchronised over their shared
 * time axis; the unit is constant across instants and is threaded
 * through via `lfinfo.param[0]` — the existing binary-synced
 * lifting machinery supports it without needing a dedicated
 * `tfunc_temporal_temporal_const` variant.
 *****************************************************************************/

/**
 * @ingroup meos_h3_metrics
 * @brief Return the per-instant great-circle distance between two temporal
 * geodetic points in the given unit.
 */
Temporal *
tgeogpoint_great_circle_distance(const Temporal *a, const Temporal *b,
  const char *unit)
{
  assert(a); assert(b);
  assert(a->temptype == T_TGEOGPOINT);
  assert(b->temptype == T_TGEOGPOINT);

  H3Unit u = h3_unit_from_cstring(unit);

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

/*****************************************************************************/
