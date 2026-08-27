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
 * @brief Shared temporal lifting for the DGGS cell-index family.
 *
 * The generic entry points dispatch on the temporal type via
 * `dggs_cellops()` and lift the descriptor's Datum-convention static kernel
 * with `tfunc_temporal`. Adding a DGGS requires only a new descriptor and one
 * line in `dggs_cellops()` — see meos/include/temporal/tcellindex.h.
 */

#include "temporal/tcellindex.h"

/* C */
#include <assert.h>
#include <math.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/lifting.h"

/* Per-DGGS descriptors, defined in each family and referenced here under the
 * same build-flag guard that compiles the family. */
#if H3
extern const DggsCellOps h3_cellops;
#endif
#if QUADBIN
extern const DggsCellOps quadbin_cellops;
#endif

/*****************************************************************************
 * Catalog predicate + descriptor registry
 *****************************************************************************/

/**
 * @brief Return true if @p type is a temporal DGGS cell-index type.
 */
bool
tcellindex_type(MeosType type UNUSED)
{
  return
#if H3
    type == T_TH3INDEX ||
#endif
#if QUADBIN
    type == T_TQUADBIN ||
#endif
    false;
}

/**
 * @brief Ensure that @p type is a temporal DGGS cell-index type.
 */
bool
ensure_tcellindex_type(MeosType type)
{
  if (tcellindex_type(type))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The temporal value must be a temporal cell index");
  return false;
}

/**
 * @brief Return the operations descriptor for a temporal cell-index type.
 */
const DggsCellOps *
dggs_cellops(MeosType temptype)
{
  switch (temptype)
  {
#if H3
    case T_TH3INDEX:
      return &h3_cellops;
#endif
#if QUADBIN
    case T_TQUADBIN:
      return &quadbin_cellops;
#endif
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Type %d is not a temporal DGGS cell-index type", temptype);
      return NULL;
  }
}

/*****************************************************************************
 * Generic lifting helpers
 *****************************************************************************/

/**
 * @brief Lift a unary Datum-convention cell function over a temporal value.
 */
static Temporal *
tcellindex_lift_unary(const Temporal *temp, Datum (*func)(Datum),
  const char *opname, MeosType restype)
{
  if (! func)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation \"%s\" is not available for this DGGS cell type", opname);
    return NULL;
  }
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = restype;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @brief Lift a one-parameter Datum-convention cell function over a temporal
 * value.
 */
static Temporal *
tcellindex_lift_param1(const Temporal *temp, Datum (*func)(Datum, Datum),
  const char *opname, Datum param, MeosType restype)
{
  if (! func)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation \"%s\" is not available for this DGGS cell type", opname);
    return NULL;
  }
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = param;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = restype;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Generic temporal entry points
 *****************************************************************************/

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal resolution (tint) of a temporal cell index.
 * @csqlfn #Tquadbin_get_resolution()
 */
Temporal *
tcellindex_get_resolution(const Temporal *temp)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  return tcellindex_lift_unary(temp, ops->get_resolution, "getResolution",
    T_TINT);
}

/**
 * @ingroup meos_cellindex
 * @brief Return a tbool stating at each instant whether the value is a valid
 * cell.
 * @csqlfn #Tquadbin_is_valid_cell()
 */
Temporal *
tcellindex_is_valid_cell(const Temporal *temp)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  return tcellindex_lift_unary(temp, ops->is_valid_cell, "isValidCell",
    T_TBOOL);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal parent cell at the given resolution.
 * @csqlfn #Tquadbin_cell_to_parent()
 */
Temporal *
tcellindex_cell_to_parent(const Temporal *temp, int32 resolution)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  return tcellindex_lift_param1(temp, ops->cell_to_parent, "cellToParent",
    Int32GetDatum(resolution), temp->temptype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell centroid as a temporal point (geodetic for
 * H3/S2, Web-Mercator for quadbin).
 * @csqlfn #Tquadbin_cell_to_point()
 */
Temporal *
tcellindex_cell_to_point(const Temporal *temp)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  return tcellindex_lift_unary(temp, ops->cell_to_point, "cellToPoint",
    ops->point_temptype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell boundary as a temporal (multi)polygon.
 * @csqlfn #Tquadbin_cell_to_boundary()
 */
Temporal *
tcellindex_cell_to_boundary(const Temporal *temp)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  MeosType restype = (ops->point_temptype == T_TGEOGPOINT) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tcellindex_lift_unary(temp, ops->cell_to_boundary, "cellToBoundary",
    restype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell area in square meters (tfloat).
 * @csqlfn #Tquadbin_cell_area()
 */
Temporal *
tcellindex_cell_area(const Temporal *temp)
{
  VALIDATE_TCELLINDEX(temp, NULL);
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  if (! ops)
    return NULL;
  return tcellindex_lift_unary(temp, ops->cell_area, "cellArea", T_TFLOAT);
}

/*****************************************************************************
 * Geodetic bounding box of a cell boundary
 *****************************************************************************/

/**
 * @brief Return in the last argument the unit vector of a lon/lat position
 */
static void
dggs_lonlat_to_xyz(double lon, double lat, double xyz[3])
{
  double lonr = lon * M_PI / 180.0;
  double latr = lat * M_PI / 180.0;
  double c = cos(latr);
  xyz[0] = c * cos(lonr);
  xyz[1] = c * sin(lonr);
  xyz[2] = sin(latr);
  return;
}

/**
 * @brief Return the latitude extreme of the geodesic arc joining two points
 * @details The great circle through @p a and @p b has normal `n = a x b`, and
 * every point `p` on it satisfies `p . n = 0`. Maximizing `p_z` under that
 * constraint gives `p* = normalize(z - n_z * n)`, whose latitude is the
 * highest the circle reaches and `-p*` the lowest. Only an extreme falling
 * INSIDE the arc counts, which is the case exactly when it lies on the same
 * side of both endpoints, so the two half-space tests
 * `(a x p*) . n > 0` and `(p* x b) . n > 0` decide it.
 * @param[in] a,b Endpoints of the arc, as unit vectors
 * @param[in] north True for the northern extreme, false for the southern
 * @param[out] result Latitude in degrees, written only when the extreme lies
 * inside the arc
 * @return True when the extreme lies inside the arc
 */
static bool
dggs_arc_lat_extreme(const double a[3], const double b[3], bool north,
  double *result)
{
  double n[3];
  n[0] = a[1] * b[2] - a[2] * b[1];
  n[1] = a[2] * b[0] - a[0] * b[2];
  n[2] = a[0] * b[1] - a[1] * b[0];
  double nnorm = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  /* Coincident or antipodal endpoints span no arc. The test is exact rather
   * than banded: any nonzero normal names a great circle, and a band wide
   * enough to matter at the coarsest resolution would swallow every edge at
   * the finest, whose endpoints subtend a few nanoradians */
  if (nnorm <= 0.0)
    return false;
  n[0] /= nnorm; n[1] /= nnorm; n[2] /= nnorm;
  /* The component of the pole direction orthogonal to the circle's normal */
  double p[3] = { -n[2] * n[0], -n[2] * n[1], 1.0 - n[2] * n[2] };
  double pnorm = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
  /* A circle through both poles reaches its extremes AT the poles, and every
   * point of it is then a longitude extreme rather than a latitude one */
  if (pnorm <= 0.0)
    return false;
  double sign = north ? 1.0 : -1.0;
  p[0] = sign * p[0] / pnorm;
  p[1] = sign * p[1] / pnorm;
  p[2] = sign * p[2] / pnorm;
  double cross1[3], cross2[3];
  cross1[0] = a[1] * p[2] - a[2] * p[1];
  cross1[1] = a[2] * p[0] - a[0] * p[2];
  cross1[2] = a[0] * p[1] - a[1] * p[0];
  cross2[0] = p[1] * b[2] - p[2] * b[1];
  cross2[1] = p[2] * b[0] - p[0] * b[2];
  cross2[2] = p[0] * b[1] - p[1] * b[0];
  if (cross1[0] * n[0] + cross1[1] * n[1] + cross1[2] * n[2] <= 0.0 ||
      cross2[0] * n[0] + cross2[1] * n[1] + cross2[2] * n[2] <= 0.0)
    return false;
  *result = asin(p[2]) * 180.0 / M_PI;
  return true;
}

/**
 * @brief Return in the last four arguments the lon/lat bounding box of a cell
 * boundary given as a closed ring of vertices
 * @details The box CONTAINS the cell, which the vertices alone do not
 * establish. A cell edge is a geodesic and reaches a latitude beyond both of
 * the vertices it joins, so every edge contributes its own extreme. A cell
 * holding a pole reaches that pole and spans every longitude. A cell crossing
 * the antimeridian has vertex longitudes near both `-180` and `+180`, whose
 * plain minimum and maximum name the COMPLEMENT of the cell rather than the
 * cell; it takes the full longitude range, which contains the cell at the
 * cost of a wider box.
 * @param[in] lons,lats Vertices of the boundary, in degrees
 * @param[in] count Number of vertices
 * @param[in] north_pole,south_pole Whether the cell holds each pole
 * @param[out] xmin,ymin,xmax,ymax The box
 */
void
dggs_lonlat_boundary_set_box(const double *lons, const double *lats,
  int count, bool north_pole, bool south_pole, double *xmin, double *ymin,
  double *xmax, double *ymax)
{
  assert(lons); assert(lats); assert(count > 0);
  assert(xmin); assert(ymin); assert(xmax); assert(ymax);
  *xmin = *xmax = lons[0];
  *ymin = *ymax = lats[0];
  for (int k = 1; k < count; k++)
  {
    if (lons[k] < *xmin) *xmin = lons[k];
    if (lons[k] > *xmax) *xmax = lons[k];
    if (lats[k] < *ymin) *ymin = lats[k];
    if (lats[k] > *ymax) *ymax = lats[k];
  }
  /* Each edge is a geodesic and may rise above, or fall below, both of the
   * vertices it joins */
  for (int k = 0; k < count; k++)
  {
    double a[3], b[3], lat;
    int next = (k + 1) % count;
    dggs_lonlat_to_xyz(lons[k], lats[k], a);
    dggs_lonlat_to_xyz(lons[next], lats[next], b);
    if (dggs_arc_lat_extreme(a, b, true, &lat) && lat > *ymax)
      *ymax = lat;
    if (dggs_arc_lat_extreme(a, b, false, &lat) && lat < *ymin)
      *ymin = lat;
  }
  /* A cell holding a pole reaches it */
  if (north_pole)
    *ymax = 90.0;
  if (south_pole)
    *ymin = -90.0;
  /* A cell holding a pole, and a cell crossing the antimeridian, take the
   * full longitude range */
  if (north_pole || south_pole || *xmax - *xmin > 180.0)
  {
    *xmin = -180.0;
    *xmax = 180.0;
  }
  return;
}

/*****************************************************************************/

