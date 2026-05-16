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
 * @brief MEOS lifting for lat/lng conversions, plus the static
 * adapter bodies that back them.
 *
 * The static h3 conversions `h3_gs_point_to_cell`,
 * `h3_cell_to_gs_point`, and `h3_cell_to_gs_boundary` live here
 * alongside the lifted entries that consume them. Point reads use
 * the MobilityDB peek macro `GSERIALIZED_POINT2D_P` rather than
 * `lwgeom_from_gserialized` — approved by the PostGIS team and a
 * meaningful speed-up for point-heavy paths.
 *
 * Both `tgeogpoint` (canonical, geodetic) and `tgeompoint`
 * (SRID 4326, planar-tagged) overloads are provided.
 */

#include <math.h>
#include <string.h>

#include <liblwgeom.h>

#include <meos.h>
#include <meos_h3.h>

#include "geo/tgeo_spatialfuncs.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * Static adapters — lat/lng ↔ cell / cell ↔ boundary
 *****************************************************************************/

H3Index
h3_gs_point_to_cell(const GSERIALIZED *point, int32 resolution)
{
  if (! ensure_srid_is_latlong(gserialized_get_srid(point)))
    return (H3Index) 0;
  const POINT2D *p = GSERIALIZED_POINT2D_P(point);
  LatLng ll = { .lat = degsToRads(p->y), .lng = degsToRads(p->x) };
  H3Index cell;
  if (latLngToCell(&ll, resolution, &cell) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return (H3Index) 0;
  }
  return cell;
}

GSERIALIZED *
h3_cell_to_gs_point(H3Index cell)
{
  LatLng ll;
  if (cellToLatLng(cell, &ll) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return geopoint_make(radsToDegs(ll.lng), radsToDegs(ll.lat), 0.0,
    false, true, SRID_DEFAULT);
}

/**
 * @brief Build a geodetic SRID 4326 LWPOLY from a libh3 CellBoundary
 * and serialise it. The ring is closed by repeating vertex 0. Shared
 * between cell and directed-edge boundary adapters.
 */
GSERIALIZED *
cell_boundary_to_gs(const CellBoundary *bnd)
{
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE,
    bnd->numVerts + 1);
  for (int v = 0; v < bnd->numVerts; v++)
  {
    POINT4D pt;
    pt.x = radsToDegs(bnd->verts[v].lng);
    pt.y = radsToDegs(bnd->verts[v].lat);
    pt.z = 0.0;
    pt.m = 0.0;
    ptarray_append_point(pa, &pt, LW_TRUE);
  }
  /* Close the ring. */
  POINT4D pt0;
  pt0.x = radsToDegs(bnd->verts[0].lng);
  pt0.y = radsToDegs(bnd->verts[0].lat);
  pt0.z = 0.0;
  pt0.m = 0.0;
  ptarray_append_point(pa, &pt0, LW_TRUE);

  LWPOLY *poly = lwpoly_construct_empty(SRID_DEFAULT, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  lwgeom_set_geodetic(lwpoly_as_lwgeom(poly), LW_TRUE);
  GSERIALIZED *result = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return result;
}

GSERIALIZED *
h3_cell_to_gs_boundary(H3Index cell)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return cell_boundary_to_gs(&bnd);
}

/*****************************************************************************
 * h3_latlng_to_cell(tgeompoint, integer) — lift_with_const
 *
 * The adapter `h3_gs_point_to_cell` is expected to verify SRID 4326
 * and raise on mismatch.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal planar point (SRID 4326)
 * at the given resolution.
 */
Temporal *
tgeompoint_to_th3index(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TGEOMPOINT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_latlng_to_cell;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(resolution);
  lfinfo.argtype[0] = T_TGEOMPOINT;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_latlng_to_cell(tgeogpoint, integer) — lift_with_const
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal geodetic point at the
 * given resolution.
 */
Temporal *
tgeogpoint_to_th3index(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TGEOGPOINT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_latlng_to_cell;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(resolution);
  lfinfo.argtype[0] = T_TGEOGPOINT;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_latlng (geodetic output)
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell.
 */
Temporal *
th3index_to_tgeogpoint(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_latlng;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_latlng (planar output, SRID 4326 overload)
 *
 * Both overloads share the same static adapter `h3_cell_to_gs_point`,
 * which emits an SRID-4326 point. The geography-vs-geometry nature
 * of the result is disambiguated at the lifting layer via the
 * `restype` setting — downstream consumers see the intended type.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal
 * H3 cell.
 */
Temporal *
th3index_to_tgeompoint(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_latlng;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_boundary — polygon per instant, emitted as tgeography
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the per-instant polygon boundary of a temporal H3 cell as
 * a temporal geography.
 */
Temporal *
th3index_cell_to_boundary(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_boundary;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGRAPHY;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
