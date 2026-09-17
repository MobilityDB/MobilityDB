/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
#include <inttypes.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#if H3
  #include "h3/h3index.h"
#endif
#if QUADBIN
  #include <meos_quadbin.h>
#endif

/* Per-DGGS descriptors, defined in each family and referenced here under the
 * same build-flag guard that compiles the family. */
#if H3
extern const DggsCellOps h3_cellops;
#endif
#if QUADBIN
extern const DggsCellOps quadbin_cellops;
#endif
#if S2CELL
extern const DggsCellOps s2_cellops;
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
#if S2CELL
    type == T_TS2CELL ||
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
#if S2CELL
    case T_TS2CELL:
      return &s2_cellops;
#endif
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Type %d is not a temporal DGGS cell-index type", temptype);
      return NULL;
  }
}

/**
 * @brief Ensure that a resolution lies in the range of the grid of a temporal
 * cell-index type, or raise an error
 * @param[in] temptype Temporal cell-index type naming the grid
 * @param[in] resolution Resolution
 */
bool
ensure_valid_cell_resolution(MeosType temptype, int32 resolution)
{
  const DggsCellOps *ops = dggs_cellops(temptype);
  if (ops == NULL)
    return false;
  if (resolution >= ops->min_resolution && resolution <= ops->max_resolution)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The resolution must be between %d and %d", ops->min_resolution,
    ops->max_resolution);
  return false;
}

/**
 * @brief Ensure that a 64-bit integer encodes a cell of the grid of a temporal
 * cell-index type, or raise an error
 * @details A cell of each grid is a 64-bit integer with the structure its grid
 * defines: QUADBIN by its header, resolution and unused bits, S2 by its face
 * and the one bit ending its position, H3 by its mode bits. Not every integer
 * is a value of a cell-index type, so an integer entering one is checked as
 * the text input of the type checks its string: an S2 value is a cell of its
 * grid, a QUADBIN value a well-formed index of any mode, since the quadbin
 * type holds one, and an H3 value a cell, a directed edge, a vertex or the
 * zero sentinel, since the h3index type holds all of them.
 * @param[in] value Integer, as a Datum
 * @param[in] temptype Temporal cell-index type naming the grid
 */
bool
ensure_valid_cell(Datum value, MeosType temptype)
{
#if H3
  if (temptype == T_TH3INDEX)
  {
    if (h3index_is_valid_input((H3Index) DatumGetInt64(value)))
      return true;
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value %" PRId64 " does not encode a valid H3 cell, directed edge "
      "or vertex", DatumGetInt64(value));
    return false;
  }
#endif
#if QUADBIN
  if (temptype == T_TQUADBIN)
  {
    if (quadbin_is_valid_index((Quadbin) DatumGetInt64(value)))
      return true;
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value %" PRId64 " does not encode a valid quadbin index",
      DatumGetInt64(value));
    return false;
  }
#endif
  const DggsCellOps *ops = dggs_cellops(temptype);
  if (! ops)
    return false;
  if (DatumGetBool(ops->is_valid_cell(value)))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The value %" PRId64 " does not encode a valid %s",
    DatumGetInt64(value), meostype_name(ops->celltype));
  return false;
}

/**
 * @brief Ensure that every value of a temporal 64-bit integer encodes a cell
 * of the grid of a temporal cell-index type, or raise an error
 * @param[in] temp Temporal 64-bit integer
 * @param[in] temptype Temporal cell-index type naming the grid
 */
bool
ensure_valid_tcell(const Temporal *temp, MeosType temptype)
{
  int count;
  Datum *values = temporal_values_p(temp, &count);
  for (int i = 0; i < count; i++)
  {
    if (! ensure_valid_cell(values[i], temptype))
    {
      pfree(values);
      return false;
    }
  }
  pfree(values);
  return true;
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
 * @csqlfn #Tquadbin_get_resolution(), #Th3index_get_resolution(), #Ts2cell_get_resolution()
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
 * @csqlfn #Tquadbin_is_valid_cell(), #Th3index_is_valid_cell(), #Ts2cell_is_valid_cell()
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
 * @csqlfn #Tquadbin_cell_to_parent(), #Th3index_cell_to_parent(), #Ts2cell_cell_to_parent()
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
 * @csqlfn #Tquadbin_cell_to_point(), #Ts2cell_cell_to_point()
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
 * @csqlfn #Tquadbin_cell_to_boundary(), #Th3index_cell_to_boundary(), #Ts2cell_cell_to_boundary()
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
 * @csqlfn #Tquadbin_cell_area(), #Th3index_cell_area(), #Ts2cell_cell_area()
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

/**
 * @brief Extend a box of longitudes and latitudes by the geodesic joining two
 * positions
 * @details A geodetic point travels the great circle between two positions,
 * which rises above, or falls below, both of them, so a box holding the two
 * does not hold the path: the extremes #dggs_arc_lat_extreme() states are what
 * the endpoints alone leave out. A path whose endpoints lie more than half the
 * globe apart in longitude travels the short way, across the antimeridian, and
 * the longitudes it passes are the ones outside the interval its endpoints
 * bound, which a box of one interval states as the whole range.
 * @param[in] lon1,lat1,lon2,lat2 Positions in degrees
 * @param[inout] xmin,ymin,xmax,ymax Bounds of the box, in degrees
 */
void
dggs_lonlat_segment_extend_box(double lon1, double lat1, double lon2,
  double lat2, double *xmin, double *ymin, double *xmax, double *ymax)
{
  assert(xmin); assert(ymin); assert(xmax); assert(ymax);
  double a[3], b[3], lat;
  dggs_lonlat_to_xyz(lon1, lat1, a);
  dggs_lonlat_to_xyz(lon2, lat2, b);
  /* The extreme is rounded away from the box, to the microdegree: a box holds
   * what it bounds, and the position a query reads at the extreme is the same
   * quantity computed along another route, which the rounding of a double
   * places a few units of the last place away. The rounding also states the
   * same bound on every platform, which an index key compared across them
   * needs */
  if (dggs_arc_lat_extreme(a, b, true, &lat))
  {
    lat = ceil(lat * 1.0e6) / 1.0e6;
    if (lat > *ymax)
      *ymax = lat;
  }
  if (dggs_arc_lat_extreme(a, b, false, &lat))
  {
    lat = floor(lat * 1.0e6) / 1.0e6;
    if (lat < *ymin)
      *ymin = lat;
  }
  if (fabs(lon1 - lon2) > 180.0)
  {
    *xmin = -180.0;
    *xmax = 180.0;
  }
  return;
}

/*****************************************************************************
 * Geodetic path of a segment
 *****************************************************************************/

/**
 * @brief Return the dot product of two vectors
 */
static double
dggs_vec_dot(const POINT3D *p, const POINT3D *q)
{
  return p->x * q->x + p->y * q->y + p->z * q->z;
}

/**
 * @brief Set the last argument to the cross product of the first two
 */
static void
dggs_vec_cross(const POINT3D *p, const POINT3D *q, POINT3D *r)
{
  r->x = p->y * q->z - p->z * q->y;
  r->y = p->z * q->x - p->x * q->z;
  r->z = p->x * q->y - p->y * q->x;
  return;
}

/**
 * @brief Return in the last argument the great-circle path of a geodetic
 * segment
 * @param[in] lon1,lat1,lon2,lat2 Endpoints in degrees
 * @param[out] arc Path
 * @return False when the path has no length, since it then leaves no cell
 */
bool
dggs_arc_init(double lon1, double lat1, double lon2, double lat2,
  DggsArc *arc)
{
  assert(arc);
  memset(arc, 0, sizeof(DggsArc));
  GEOGRAPHIC_POINT g1, g2, gm;
  geographic_point_init(lon1, lat1, &g1);
  geographic_point_init(lon2, lat2, &g2);
  arc->lon = g1.lon;
  arc->lat = g1.lat;
  arc->dist = sphere_distance(&g1, &g2);
  if (arc->dist <= 0.0)
    return false;
  arc->azimuth = sphere_direction(&g1, &g2, arc->dist);
  POINT3D a, normal;
  geog2cart(&g1, &a);
  /* The circle is the one the path follows: through its first endpoint and
   * the point it reaches halfway, which also fixes it where the endpoints are
   * antipodal and every great circle joins them */
  if (sphere_project(&g1, arc->dist / 2.0, arc->azimuth, &gm) != LW_SUCCESS)
    return false;
  /* Read from the angles of the two points: the Cartesian cross product of
   * two close unit vectors loses its precision to cancellation */
  robust_cross_product(&g1, &gm, &normal);
  if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
    return false;
  normalize(&normal);
  arc->a[0] = a.x; arc->a[1] = a.y; arc->a[2] = a.z;
  arc->normal[0] = normal.x;
  arc->normal[1] = normal.y;
  arc->normal[2] = normal.z;
  return true;
}

/**
 * @brief Return in the last two arguments the position a geodetic path
 * reaches at a parameter
 * @details The position is the point `sphere_project` reaches along the great
 * circle at that fraction of the path's angle, the one `pointsegm_interpolate`
 * answers for a temporal geodetic point.
 * @param[in] arc Path
 * @param[in] t Parameter, the fraction of the path's angle
 * @param[out] lon,lat Position in radians, the longitude normalized
 * @return False when the position cannot be projected
 */
bool
dggs_arc_point(const DggsArc *arc, double t, double *lon, double *lat)
{
  assert(arc); assert(lon); assert(lat);
  GEOGRAPHIC_POINT g1 = { .lat = arc->lat, .lon = arc->lon }, g;
  if (sphere_project(&g1, arc->dist * t, arc->azimuth, &g) != LW_SUCCESS)
    return false;
  *lon = longitude_radians_normalize(g.lon);
  *lat = g.lat;
  return true;
}

/**
 * @brief Return where a geodetic path leaves a cell
 * @details A cell edge is an arc of a great circle, as the path is, so the
 * circle of each edge meets the circle of the path at two antipodal points,
 * along the intersection of their planes. The exit is the nearest of those
 * points that lies on its edge and strictly ahead of `tmin` on the path; its
 * parameter is the fraction of the path's angle reached there.
 *
 * A convex cell is the intersection of the hemispheres its edge circles bound,
 * so a path inside it leaves it where it first crosses any of those circles,
 * and that crossing lies on its edge by construction. The test of lying on the
 * edge reads the sign of a rounded product, which a crossing at a vertex can
 * fail, as for a path running along one cell edge and meeting the next edge
 * at its vertex; a convex cell does without it.
 * @param[in] arc Path
 * @param[in] lons,lats Vertices of the cell boundary in radians, in the
 * order they join
 * @param[in] count Number of vertices
 * @param[in] tmin Parameter the exit lies strictly ahead of
 * @param[in] convex True when the cell is convex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_arc_exit_param(const DggsArc *arc, const double *lons,
  const double *lats, int count, double tmin, bool convex)
{
  assert(arc); assert(lons); assert(lats);
  POINT3D a = { .x = arc->a[0], .y = arc->a[1], .z = arc->a[2] };
  POINT3D normal = { .x = arc->normal[0], .y = arc->normal[1],
    .z = arc->normal[2] };
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D vi, vj, m, d, c;
    geog2cart(&gi, &vi);
    geog2cart(&gj, &vj);
    /* The normal of the circle of the edge, read from the angles of its
     * vertices: their Cartesian cross product loses to cancellation all but
     * the rounding of a unit vector, which on the edge of a fine cell places
     * the circle millimetres off the vertices */
    robust_cross_product(&gi, &gj, &m);
    dggs_vec_cross(&normal, &m, &d);
    if (d.x == 0.0 && d.y == 0.0 && d.z == 0.0)
      continue;              /* the edge lies on the circle of the path */
    for (int s = 0; s < 2; s++)
    {
      POINT3D p = d;
      if (s)
      {
        p.x = -d.x; p.y = -d.y; p.z = -d.z;
      }
      /* On the edge: between its two vertices along the circle of the edge */
      if (! convex)
      {
        dggs_vec_cross(&vi, &p, &c);
        if (dggs_vec_dot(&c, &m) < 0.0)
          continue;
        dggs_vec_cross(&p, &vj, &c);
        if (dggs_vec_dot(&c, &m) < 0.0)
          continue;
      }
      /* Ahead on the path: the angle from its first endpoint, measured in the
       * direction the path travels */
      dggs_vec_cross(&a, &p, &c);
      double t = atan2(dggs_vec_dot(&c, &normal), dggs_vec_dot(&a, &p)) /
        arc->dist;
      if (t > tmin && t <= 1.0 && t < best)
        best = t;
    }
  }
  return best;
}

/**
 * @brief Return where a path leaves a cell, read from a parameter at which it
 * still holds the cell and one at which it holds another
 * @details The closed form of a crossing reads the circle of a cell edge, and
 * a path running along that edge meets the circle everywhere: the parameter it
 * answers is then the rounding of the path's own position and no crossing at
 * all, which the walk sees as a path still in the cell past its own exit. The
 * probes of the walk bracket the crossing, and halving the bracket closes on
 * it, as the clip of a rigid geometry closes on a root its closed form does
 * not state.
 * @param[in] tin Parameter at which the path holds @p cell
 * @param[in] tout Parameter at which the path holds another cell
 * @param[in] cell Cell the path holds at @p tin
 * @param[in] cell_at Cell of the path at a parameter, 0 when the position
 * cannot be projected
 * @param[in] state Passed to @p cell_at
 * @return The parameter of the first position the path holds another cell at,
 * within the halving
 */
double
dggs_crossing_param(double tin, double tout, uint64 cell,
  uint64 (*cell_at)(void *, double), void *state)
{
  assert(cell_at);
  for (int i = 0; i < 60; i++)
  {
    double tm = (tin + tout) / 2.0;
    if (tm <= tin || tm >= tout)
      break;                 /* the bracket holds no parameter between them */
    uint64 at = cell_at(state, tm);
    if (at == cell)
      tin = tm;
    else if (at != 0)
      tout = tm;
    else
      break;                 /* the position cannot be projected */
  }
  return tout;
}

/**
 * @brief Return in @p params the parameters at which a geodetic path meets a
 * plane through the centre of the sphere, and their number
 * @details The circle of the path meets the plane `<p, m> = c` where its angle
 * from the first endpoint solves `ca cos(theta) + cb sin(theta) = c`, with
 * `ca` and `cb` the projections on the plane of the endpoint and of the
 * direction perpendicular to it in the circle. The equation has two solutions,
 * one, or none, as the circle crosses the plane, touches it, or misses it.
 * @param[in] arc Path
 * @param[in] m Unit normal of the plane
 * @param[in] c Offset of the plane from the centre
 * @param[out] params Array of at least two parameters, in `[0, 1]` and
 * ascending
 * @return Number of parameters written
 */
int
dggs_arc_plane_params(const DggsArc *arc, const double m[3], double c,
  double *params)
{
  assert(arc); assert(m); assert(params);
  const double *a = arc->a, *nm = arc->normal;
  const double b[3] = { nm[1] * a[2] - nm[2] * a[1],
    nm[2] * a[0] - nm[0] * a[2], nm[0] * a[1] - nm[1] * a[0] };
  double ca = a[0] * m[0] + a[1] * m[1] + a[2] * m[2];
  double cb = b[0] * m[0] + b[1] * m[1] + b[2] * m[2];
  double r = hypot(ca, cb);
  if (r == 0.0 || fabs(c) > r)
    return 0;
  double base = atan2(cb, ca), half = acos(c / r);
  int count = 0;
  for (int s = -1; s <= 1; s += 2)
  {
    double theta = fmod(base + s * half, 2.0 * M_PI);
    if (theta < 0.0)
      theta += 2.0 * M_PI;
    double t = theta / arc->dist;
    if (t >= 0.0 && t <= 1.0)
      params[count++] = t;
  }
  if (count == 2 && params[0] > params[1])
  {
    double swap = params[0]; params[0] = params[1]; params[1] = swap;
  }
  return count;
}

/**
 * @brief Return where a geodetic path first reaches a plane through the centre
 * of the sphere after a parameter
 * @param[in] arc Path
 * @param[in] m Unit normal of the plane
 * @param[in] c Offset of the plane from the centre
 * @param[in] tmin Parameter the crossing lies strictly ahead of
 * @return The path parameter of the crossing, or a value above 1 when the path
 * reaches the plane nowhere ahead of @p tmin
 */
double
dggs_arc_plane_param(const DggsArc *arc, const double m[3], double c,
  double tmin)
{
  double params[2], best = 2.0;
  int count = dggs_arc_plane_params(arc, m, c, params);
  for (int i = 0; i < count; i++)
    if (params[i] > tmin && params[i] < best)
      best = params[i];
  return best;
}

/**
 * @brief Return true if a position lies in a box of longitudes and latitudes
 */
static bool
dggs_lonlat_box_holds(double lon, double lat, double xmin, double ymin,
  double xmax, double ymax)
{
  assert(xmin <= xmax); assert(ymin <= ymax);
  return lon >= xmin && lon <= xmax && lat >= ymin && lat <= ymax;
}

/**
 * @brief Return in @p tin and @p tout the parameters between which a geodetic
 * path lies in a box of longitudes and latitudes, and the number of such
 * spans
 * @details A box is bounded by the planes of two meridians, which pass through
 * the centre of the sphere, and by the planes of constant height of two
 * parallels. The path meets each of them at parameters #dggs_arc_plane_params
 * states, and those parameters cut the path into pieces that lie wholly inside
 * the box or wholly outside it, so the position halfway along a piece says
 * which. A path is therefore clipped where it crosses the box and not where a
 * straight line in longitude and latitude would, which no geodetic path
 * follows.
 * @param[in] arc Path
 * @param[in] xmin,ymin,xmax,ymax Bounds of the box, in degrees
 * @param[out] tin,tout Arrays of at least @p maxout parameters
 * @param[in] maxout Capacity of both arrays
 * @return Number of spans written
 */
int
dggs_arc_lonlat_box_spans(const DggsArc *arc, double xmin, double ymin,
  double xmax, double ymax, double *tin, double *tout, int maxout)
{
  assert(arc); assert(tin); assert(tout);
  if (maxout < 1)
    return 0;
  /* The parameters at which the path reaches a bound of the box */
  double cuts[12], params[2];
  int ncuts = 0;
  cuts[ncuts++] = 0.0;
  cuts[ncuts++] = 1.0;
  for (int k = 0; k < 2; k++)
  {
    double lon = (k == 0 ? xmin : xmax) * M_PI / 180.0;
    const double m[3] = { -sin(lon), cos(lon), 0.0 };
    int count = dggs_arc_plane_params(arc, m, 0.0, params);
    for (int i = 0; i < count; i++)
      cuts[ncuts++] = params[i];
  }
  const double pole[3] = { 0.0, 0.0, 1.0 };
  for (int k = 0; k < 2; k++)
  {
    double lat = (k == 0 ? ymin : ymax) * M_PI / 180.0;
    int count = dggs_arc_plane_params(arc, pole, sin(lat), params);
    for (int i = 0; i < count; i++)
      cuts[ncuts++] = params[i];
  }
  /* In ascending order, which is the order the path passes them */
  for (int i = 1; i < ncuts; i++)
  {
    double v = cuts[i];
    int j = i - 1;
    while (j >= 0 && cuts[j] > v)
    {
      cuts[j + 1] = cuts[j]; j--;
    }
    cuts[j + 1] = v;
  }
  /* A piece between two cuts lies wholly inside the box or wholly outside it,
   * and the position halfway along it says which; a piece following one that
   * is inside extends its span */
  int nspans = 0;
  for (int i = 0; i + 1 < ncuts; i++)
  {
    if (cuts[i + 1] <= cuts[i])
      continue;
    double lon, lat;
    if (! dggs_arc_point(arc, (cuts[i] + cuts[i + 1]) / 2.0, &lon, &lat))
      continue;
    if (! dggs_lonlat_box_holds(lon * 180.0 / M_PI, lat * 180.0 / M_PI, xmin,
          ymin, xmax, ymax))
      continue;
    if (nspans > 0 && tout[nspans - 1] == cuts[i])
      tout[nspans - 1] = cuts[i + 1];
    else if (nspans < maxout)
    {
      tin[nspans] = cuts[i];
      tout[nspans++] = cuts[i + 1];
    }
    else
      break;
  }
  return nspans;
}

/*****************************************************************************
 * Planar path of a segment
 *****************************************************************************/

/**
 * @brief Return in the last argument the straight path in longitude and
 * latitude of a planar segment
 * @details The position at parameter `t` is `p(t) = (cos φ cos λ, cos φ sin λ,
 * sin φ)` with `λ = λ1 + t Δλ` and `φ = φ1 + t Δφ`. Its second derivative has
 * the squared norm `(Δλ² + Δφ²)² cos² φ + (4 Δλ² Δφ² + Δφ⁴) sin² φ`, which is
 * at most the larger of its two coefficients: the bound the exit search steps
 * by.
 * @param[in] lon1,lat1,lon2,lat2 Endpoints in degrees
 * @param[out] line Path
 * @return False when the path has no length, since it then leaves no cell
 */
bool
dggs_line_init(double lon1, double lat1, double lon2, double lat2,
  DggsLine *line)
{
  assert(line);
  memset(line, 0, sizeof(DggsLine));
  line->lon = deg2rad(lon1);
  line->lat = deg2rad(lat1);
  line->dlon = deg2rad(lon2 - lon1);
  line->dlat = deg2rad(lat2 - lat1);
  double a2 = line->dlon * line->dlon, b2 = line->dlat * line->dlat;
  line->length = sqrt(a2 + b2);
  if (line->length <= 0.0)
    return false;
  double c1 = (a2 + b2) * (a2 + b2), c2 = 4.0 * a2 * b2 + b2 * b2;
  line->curvature = sqrt(c1 > c2 ? c1 : c2);
  return true;
}

/**
 * @brief Return in the last two arguments the position a planar path reaches
 * at a parameter, in degrees
 */
void
dggs_line_point(const DggsLine *line, double t, double *lon, double *lat)
{
  assert(line); assert(lon); assert(lat);
  *lon = rad2deg(line->lon + t * line->dlon);
  *lat = rad2deg(line->lat + t * line->dlat);
  return;
}

/**
 * @brief Return in the last two arguments the value and the derivative at a
 * parameter of the height of a planar path above the plane of normal `m`
 */
static void
dggs_line_height(const DggsLine *line, const POINT3D *m, double t,
  double *value, double *slope)
{
  double lon = line->lon + t * line->dlon, lat = line->lat + t * line->dlat;
  double cl = cos(lon), sl = sin(lon), cp = cos(lat), sp = sin(lat);
  POINT3D p = { .x = cp * cl, .y = cp * sl, .z = sp };
  POINT3D d = { .x = -line->dlat * sp * cl - line->dlon * cp * sl,
    .y = -line->dlat * sp * sl + line->dlon * cp * cl,
    .z = line->dlat * cp };
  *value = dggs_vec_dot(m, &p);
  *slope = dggs_vec_dot(m, &d);
  return;
}

/**
 * @brief Return the first parameter strictly ahead of `tmin` at which a
 * planar path reaches the plane of normal `m` from its positive side, or a
 * value above 1 when it does not before its end
 * @details The height `f` of the path above the plane has a second derivative
 * of norm at most `M`, the curvature bound of the path, so over a step `h` it
 * stays above `f + f' h - M h² / 2`. The search steps to the first zero of
 * that bound, which the height cannot reach sooner: no crossing is stepped
 * over, however short the stretch the path spends beyond the plane. Near a
 * crossing the steps shrink quadratically onto it.
 */
static double
dggs_line_plane_param(const DggsLine *line, const POINT3D *m, double tmin)
{
  double t = tmin, f, d;
  double mm = line->curvature;
  for (int i = 0; i < 1024 && t <= 1.0; i++)
  {
    dggs_line_height(line, m, t, &f, &d);
    if (f <= 0.0)
    {
      /* A position on the plane within its rounding: the path leaves here
       * unless it heads inward */
      if (d <= 0.0)
        return (t > tmin) ? t : nextafter(tmin, 2.0);
      f = 0.0;
    }
    double h = (d + sqrt(d * d + 2.0 * mm * f)) / mm;
    if (h <= 1e-15)
      return (t > tmin) ? t : nextafter(tmin, 2.0);
    t += h;
  }
  return (t > 1.0) ? 2.0 : t;
}

/**
 * @brief Return the first parameter strictly ahead of `tmin` at which the
 * height of a planar path above the plane of normal `m` changes sign, or a
 * value above 1 when it does not before its end
 * @details The search of #dggs_line_plane_param, read from whichever side of
 * the plane the path lies on at `tmin`
 */
static double
dggs_line_plane_sign_change(const DggsLine *line, const POINT3D *m,
  double tmin)
{
  double t = tmin, f, d;
  double mm = line->curvature;
  dggs_line_height(line, m, t, &f, &d);
  /* A start on the plane lies on the side the path heads to */
  double side = (f > 0.0 || (f == 0.0 && d >= 0.0)) ? 1.0 : -1.0;
  for (int i = 0; i < 1024 && t <= 1.0; i++)
  {
    double g = side * f, e = side * d;
    if (g <= 0.0 && t > tmin)
      return t;
    if (g < 0.0)
      g = 0.0;
    double h = (e + sqrt(e * e + 2.0 * mm * g)) / mm;
    t += (h > 1e-15) ? h : 1e-15;
    dggs_line_height(line, m, t, &f, &d);
  }
  return (t > 1.0) ? 2.0 : t;
}

/**
 * @brief Return where a planar path first crosses an edge of a cell, a
 * crossing of the circle of an edge counting where it lies between the two
 * vertices of the edge
 */
static double
dggs_line_exit_param_edges(const DggsLine *line, const double *lons,
  const double *lats, int count, double tmin)
{
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D vi, vj, m, c;
    geog2cart(&gi, &vi);
    geog2cart(&gj, &vj);
    robust_cross_product(&gi, &gj, &m);
    if (m.x == 0.0 && m.y == 0.0 && m.z == 0.0)
      continue;
    normalize(&m);
    /* A straight line in longitude and latitude meets a great circle a few
     * times at most over a segment, and a crossing off the edge is followed
     * by the next one */
    double t = tmin;
    for (int k = 0; k < 8; k++)
    {
      t = dggs_line_plane_sign_change(line, &m, t);
      if (t > 1.0 || t >= best)
        break;
      double lon, lat;
      dggs_line_point(line, t, &lon, &lat);
      GEOGRAPHIC_POINT gp = { .lat = deg2rad(lat), .lon = deg2rad(lon) };
      POINT3D p;
      geog2cart(&gp, &p);
      dggs_vec_cross(&vi, &p, &c);
      if (dggs_vec_dot(&c, &m) < 0.0)
        continue;
      dggs_vec_cross(&p, &vj, &c);
      if (dggs_vec_dot(&c, &m) < 0.0)
        continue;
      best = t;
      break;
    }
  }
  return best;
}

/**
 * @brief Return where a planar path leaves a cell
 * @details A convex cell is the intersection of the hemispheres its edge
 * circles bound, so a path inside it leaves it where it first reaches any of
 * their planes. A cell that is not convex is left where the path first crosses
 * an edge: a crossing of the circle of an edge counts where it lies between the
 * two vertices of the edge, as #dggs_arc_exit_param tests it. The straight line
 * in longitude and latitude is no great circle, so a crossing has no closed
 * form and is searched for along the path.
 * @param[in] line Path
 * @param[in] lons,lats Vertices of the cell boundary in radians, in the order
 * they join
 * @param[in] count Number of vertices
 * @param[in] tmin Parameter the exit lies strictly ahead of
 * @param[in] convex True when the cell is convex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_line_exit_param(const DggsLine *line, const double *lons,
  const double *lats, int count, double tmin, bool convex)
{
  assert(line); assert(lons); assert(lats);
  if (! convex)
    return dggs_line_exit_param_edges(line, lons, lats, count, tmin);
  /* The interior lies on the side of every edge circle the centre of the
   * vertices lies on */
  POINT3D centre = { .x = 0.0, .y = 0.0, .z = 0.0 };
  for (int i = 0; i < count; i++)
  {
    GEOGRAPHIC_POINT g = { .lat = lats[i], .lon = lons[i] };
    POINT3D v;
    geog2cart(&g, &v);
    centre.x += v.x; centre.y += v.y; centre.z += v.z;
  }
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D m;
    /* Read from the angles of the vertices, as #dggs_arc_exit_param does */
    robust_cross_product(&gi, &gj, &m);
    if (m.x == 0.0 && m.y == 0.0 && m.z == 0.0)
      continue;
    normalize(&m);
    if (dggs_vec_dot(&m, &centre) < 0.0)
    {
      m.x = -m.x; m.y = -m.y; m.z = -m.z;
    }
    double t = dggs_line_plane_param(line, &m, tmin);
    if (t < best)
      best = t;
  }
  return best;
}

/*****************************************************************************/


/*****************************************************************************
 * Compaction of a set of cells of a quadtree grid
 *****************************************************************************/

/** @brief Largest number of cells an uncompacted set holds */
#define DGGS_MAX_UNCOMPACT_CELLS 4194304

/**
 * @brief Comparator of two cell identifiers in ascending order
 */
static int
dggs_cell_cmp(const void *a, const void *b)
{
  uint64 x = *(const uint64 *) a, y = *(const uint64 *) b;
  return (x > y) - (x < y);
}

/**
 * @brief A cell with the parent it merges into
 */
typedef struct
{
  uint64 parent;          /**< Parent one level coarser */
  uint64 cell;            /**< Cell */
} DggsCellParent;

/**
 * @brief Comparator of two cells by their parent, then by themselves
 */
static int
dggs_cell_parent_cmp(const void *a, const void *b)
{
  const DggsCellParent *x = a, *y = b;
  if (x->parent != y->parent)
    return (x->parent > y->parent) - (x->parent < y->parent);
  return (x->cell > y->cell) - (x->cell < y->cell);
}

/**
 * @brief Return the resolution of a cell through the descriptor of its grid
 */
static int32
dggs_cell_resolution(const DggsCellOps *ops, uint64 cell)
{
  return DatumGetInt32(ops->get_resolution(Int64GetDatum((int64) cell)));
}

/**
 * @brief Return the ancestor of a cell at a resolution through the descriptor
 * of its grid
 */
static uint64
dggs_cell_parent(const DggsCellOps *ops, uint64 cell, int32 resolution)
{
  return (uint64) DatumGetInt64(ops->cell_to_parent(
    Int64GetDatum((int64) cell), Int32GetDatum(resolution)));
}

/**
 * @brief Return the compacted set of a set of cells of a quadtree grid
 * @details A QUADBIN tile and an S2 cell are each exactly the union of their
 * four children, so a set of cells states the same region once a cell covered
 * by a coarser cell of the set is dropped and every four children of one
 * parent merge into that parent. The merge runs from the finest
 * resolution up, since a parent it adds may complete a group one level
 * coarser. This is the normalization the S2 library states for a cell union:
 * the cells may be of any resolutions, and the result holds no cell covered by
 * another and no four children of one parent.
 * @param[in] cells Set of cells
 * @param[in] temptype Temporal cell-index type naming the grid
 */
Set *
dggs_quadtree_compact_cells(const Set *cells, MeosType temptype)
{
  assert(cells);
  const DggsCellOps *ops = dggs_cellops(temptype);
  if (! ops)
    return NULL;
  int n = cells->count;
  uint64 *ids = palloc(sizeof(uint64) * (size_t) n);
  for (int i = 0; i < n; i++)
    ids[i] = (uint64) DatumGetInt64(SET_VAL_N(cells, i));
  qsort(ids, (size_t) n, sizeof(uint64), dggs_cell_cmp);

  /* A cell covered by a coarser cell of the set adds nothing to the region */
  uint64 *cur = palloc(sizeof(uint64) * (size_t) n);
  int m = 0;
  int32 maxres = ops->min_resolution;
  for (int i = 0; i < n; i++)
  {
    int32 res = dggs_cell_resolution(ops, ids[i]);
    bool covered = false;
    for (int32 r = ops->min_resolution; r < res && ! covered; r++)
    {
      uint64 parent = dggs_cell_parent(ops, ids[i], r);
      covered = bsearch(&parent, ids, (size_t) n, sizeof(uint64),
        dggs_cell_cmp) != NULL;
    }
    if (covered)
      continue;
    cur[m++] = ids[i];
    if (res > maxres)
      maxres = res;
  }
  pfree(ids);

  /* The four children of one parent are that parent, from the finest
   * resolution up */
  for (int32 r = maxres; r > ops->min_resolution; r--)
  {
    DggsCellParent *pairs = palloc(sizeof(DggsCellParent) * (size_t) m);
    uint64 *next = palloc(sizeof(uint64) * (size_t) m);
    int k = 0, nm = 0;
    for (int j = 0; j < m; j++)
    {
      if (dggs_cell_resolution(ops, cur[j]) == r)
      {
        pairs[k].parent = dggs_cell_parent(ops, cur[j], r - 1);
        pairs[k++].cell = cur[j];
      }
      else
        next[nm++] = cur[j];
    }
    qsort(pairs, (size_t) k, sizeof(DggsCellParent), dggs_cell_parent_cmp);
    for (int g = 0; g < k; )
    {
      int h = g;
      while (h < k && pairs[h].parent == pairs[g].parent)
        h++;
      /* The set holds distinct cells, so a group of four is every child */
      if (h - g == 4)
        next[nm++] = pairs[g].parent;
      else
        for (int j = g; j < h; j++)
          next[nm++] = pairs[j].cell;
      g = h;
    }
    pfree(pairs); pfree(cur);
    cur = next;
    m = nm;
  }

  Datum *datums = palloc(sizeof(Datum) * (size_t) m);
  for (int j = 0; j < m; j++)
    datums[j] = Int64GetDatum((int64) cur[j]);
  pfree(cur);
  return set_make_free(datums, m, ops->celltype, ORDER);
}

/**
 * @brief Return the set of cells at a resolution covering a set of cells of a
 * quadtree grid
 * @details A cell coarser than the resolution yields its descendants at the
 * resolution, and a cell at the resolution is kept. A cell finer than
 * the resolution cannot be stated at it, so it raises an error, as it does
 * when the result would hold more than #DGGS_MAX_UNCOMPACT_CELLS cells.
 * @param[in] cells Set of cells
 * @param[in] resolution Resolution of the result
 * @param[in] temptype Temporal cell-index type naming the grid
 * @param[in] children Function returning the descendants of a cell at a
 * resolution, with their number in its last argument
 */
Set *
dggs_quadtree_uncompact_cells(const Set *cells, int32 resolution,
  MeosType temptype, uint64 *(*children)(uint64, uint32_t, int *))
{
  assert(cells); assert(children);
  const DggsCellOps *ops = dggs_cellops(temptype);
  if (! ops || ! ensure_valid_cell_resolution(temptype, resolution))
    return NULL;
  int64 total = 0;
  for (int i = 0; i < cells->count; i++)
  {
    uint64 cell = (uint64) DatumGetInt64(SET_VAL_N(cells, i));
    int32 res = dggs_cell_resolution(ops, cell);
    if (res > resolution)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The cell %" PRIu64 " of resolution %d is finer than the resolution %d",
        cell, res, resolution);
      return NULL;
    }
    total += INT64_C(1) << (2 * (resolution - res));
    if (total > DGGS_MAX_UNCOMPACT_CELLS)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The cells at resolution %d would be more than %d", resolution,
        DGGS_MAX_UNCOMPACT_CELLS);
      return NULL;
    }
  }
  Datum *datums = palloc(sizeof(Datum) * (size_t) total);
  int k = 0;
  for (int i = 0; i < cells->count; i++)
  {
    uint64 cell = (uint64) DatumGetInt64(SET_VAL_N(cells, i));
    if (dggs_cell_resolution(ops, cell) == resolution)
    {
      datums[k++] = SET_VAL_N(cells, i);
      continue;
    }
    int count;
    uint64 *desc = children(cell, (uint32_t) resolution, &count);
    if (! desc)
    {
      pfree(datums);
      return NULL;
    }
    for (int j = 0; j < count; j++)
      datums[k++] = Int64GetDatum((int64) desc[j]);
    pfree(desc);
  }
  return set_make_free(datums, k, ops->celltype, ORDER);
}

/*****************************************************************************
 * Membership of a temporal cell index in a cell set
 *****************************************************************************/

/**
 * @brief Return true if a temporal sequence holds a value the set contains
 */
static bool
tsequence_ever_in_set(const TSequence *seq, const Set *cells)
{
  for (int i = 0; i < seq->count; i++)
    if (contains_set_value(cells, tinstant_value_p(TSEQUENCE_INST_N(seq, i))))
      return true;
  return false;
}

/**
 * @brief Return true if a temporal cell index ever takes a cell of a cell set
 * @details The walk stops at the first instant the set contains. It reads the
 * values of @p temp and of @p cells alone, so it answers for every grid; the
 * types are validated by the family entry point that calls it.
 * @param[in] temp Temporal cell index
 * @param[in] cells Set of cells of the grid of @p temp
 */
bool
tcellindex_ever_in_set(const Temporal *temp, const Set *cells)
{
  assert(temp); assert(cells);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return contains_set_value(cells,
        tinstant_value_p((TInstant *) temp));
    case TSEQUENCE:
      return tsequence_ever_in_set((TSequence *) temp, cells);
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        if (tsequence_ever_in_set(TSEQUENCESET_SEQ_N(ss, i), cells))
          return true;
      return false;
    }
  }
}

/*****************************************************************************/
