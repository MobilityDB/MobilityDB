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
 * @brief Shared temporal lifting for the DGGS cell-index family
 * @details The generic entry points dispatch on the temporal type via
 * `dggs_cellops()` and lift the descriptor's Datum-convention static kernel
 * with `tfunc_temporal`. Adding a DGGS requires only a new descriptor and one
 * line in `dggs_cellops()` — see meos/include/temporal/tcellindex.h.
 */

#include "temporal/tcellindex.h"

/* C */
#include <assert.h>
#include <float.h>
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
#include <pgtypes.h>
#include "geo/geo_funcs.h"
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#if H3
  #include "h3/h3index.h"
#endif
#if QUADBIN
  #include <meos_quadbin.h>
#endif

/** @brief Largest number of vertices a cell boundary of any grid states: a
 * quadbin tile has four, an S2 cell four, and an H3 cell ten where it bends
 * across a face of the icosahedron */
#define DGGS_MAX_CELL_VERTS 16

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
 * @brief Return true if @p type is a temporal DGGS cell-index type
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
 * @brief Ensure that @p type is a temporal DGGS cell-index type
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
 * @brief Return the operations descriptor for a temporal cell-index type
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
 * @brief Lift a unary Datum-convention cell function over a temporal value
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
 * value
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
 * @brief Return the temporal resolution (tint) of a temporal cell index
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
 * cell
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
 * @brief Return the temporal parent cell at the given resolution
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
 * H3/S2, Web-Mercator for quadbin)
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
 * @brief Return the temporal cell boundary as a temporal (multi)polygon
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
 * @brief Return the temporal cell area in square meters (tfloat)
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
  arc->endlon[0] = lon1; arc->endlat[0] = lat1;
  arc->endlon[1] = lon2; arc->endlat[1] = lat2;
  /* Two endpoints stating one position span no path: a pole stated at two
   * longitudes, or a position stated at longitudes a turn apart, which the
   * angles place a rounding apart instead */
  if (lat1 == lat2 && (fabs(lat1) == 90.0 || fabs(lon1 - lon2) == 360.0))
    return false;
  arc->dist = sphere_distance(&g1, &g2);
  if (arc->dist <= 0.0)
    return false;
  arc->azimuth = sphere_direction(&g1, &g2, arc->dist);
  POINT3D a, normal;
  /* The first endpoint as a unit vector, read from the DEGREES the path is
   * stated in. `float8_cosd` and `float8_sind` answer a quarter turn exactly,
   * where the cosine of pi/2 in a double is 6.12e-17 rather than zero: a
   * point at longitude 90 then carries an x of 1e-17 instead of none, and a
   * path along that meridian reads a height above the meridian's own plane
   * which is not zero, so the walk leaves through an edge it runs along */
  double clat = float8_cosd(lat1);
  a.x = clat * float8_cosd(lon1);
  a.y = clat * float8_sind(lon1);
  a.z = float8_sind(lat1);
  normalize(&a);
  /* The circle is the one through the two ENDPOINTS, read from their angles:
   * the Cartesian cross product of two close unit vectors loses its precision
   * to cancellation. The endpoints are what the path is stated by, so a path
   * along a meridian states one longitude twice and its circle comes out as
   * exactly the plane of that meridian. A point projected halfway along the
   * path carries the rounding of the projection instead, which leaves that
   * plane by a few units of the last place and lifts the path off an edge it
   * runs along, so the walk reads a crossing of an edge it never crosses */
  if (lon1 == lon2 || fabs(lon1 - lon2) == 180.0)
  {
    /* The path states one meridian: the same longitude twice, or a longitude
     * and the one half a turn from it, which lie in the same plane. That
     * plane states its own normal. The cross product of the two endpoints has
     * a third coordinate in which the same product is subtracted from itself,
     * so it cancels EXACTLY, and the other two carry the sine and the cosine
     * of the longitude, which `float8_sind` and `float8_cosd` answer exactly
     * at a quarter turn. Read through the angles in radians instead, the same
     * normal carries 6.12e-17 where it should carry none, and a walk along
     * that meridian reads a height above the meridian's own plane which is
     * not zero, so it leaves through an edge it runs along */
    double c1 = float8_cosd(lat1), s1 = float8_sind(lat1);
    double c2 = float8_cosd(lat2), s2 = float8_sind(lat2);
    /* The far endpoint is on the opposite side of the axis when the two
     * longitudes are half a turn apart */
    double k = (lon1 == lon2) ? c1 * s2 - s1 * c2 : c1 * s2 + s1 * c2;
    normal.x = float8_sind(lon1) * k;
    normal.y = -float8_cosd(lon1) * k;
    normal.z = 0.0;
  }
  else
    robust_cross_product(&g1, &g2, &normal);
  if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
  {
    /* The endpoints are antipodal and every great circle joins them, so the
     * one the path follows is fixed by the point it reaches halfway */
    if (sphere_project(&g1, arc->dist / 2.0, arc->azimuth, &gm) != LW_SUCCESS)
      return false;
    robust_cross_product(&g1, &gm, &normal);
    if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
      return false;
  }
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
 * @brief Return where a geodetic path leaves a convex cell given the inward
 * normals of its edge planes
 * @details A convex cell is the intersection of the hemispheres its edge
 * circles bound, so a path inside it leaves where it first leaves any of
 * them. Along the path `p(theta) = a cos(theta) + b sin(theta)`, with `b` the
 * direction the path travels at its first endpoint, the height `<m, p>` above
 * the plane of an edge of inward normal `m` is `R cos(theta - phi)` with
 * `R = hypot(<m, a>, <m, b>)` and `phi = atan2(<m, b>, <m, a>)`. It falls
 * through zero at `theta = phi + pi/2` alone: that is where the path leaves
 * the hemisphere, and the exit is the first such angle ahead of `tmin`.
 *
 * `R` is zero exactly when the path lies in the plane of the edge, and then
 * the height is zero everywhere and the path never leaves through that edge,
 * as a line running along a tile boundary never crosses it. The test is the
 * exact `R == 0`, the one #dggs_arc_plane_params applies to the same
 * quantity: a path lying in the plane has to be stated as lying in it, by a
 * normal the plane itself gives, rather than recognised inside a band around
 * zero, which a normal read from rounded vertices escapes.
 * @param[in] arc Path
 * @param[in] normals Inward unit normal of each edge plane, three coordinates
 * each, the edge from vertex `i` at `normals[3 * i]`
 * @param[in] count Number of edges
 * @param[in] tmin Parameter at which the path entered the cell
 * @param[in] entry Mask of the edges the path entered the cell through, bit
 * `i` for the edge from vertex `i`, which it never leaves through: a line
 * does not cross back over the tile boundary it has just crossed
 * @param[out] edge When not `NULL`, the edge crossed at the exit, the one
 * from vertex `edge` to the next vertex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_arc_normals_exit_param(const DggsArc *arc, const double *normals,
  int count, double tmin, uint32 entry, int *edge)
{
  assert(arc); assert(normals);
  POINT3D a = { .x = arc->a[0], .y = arc->a[1], .z = arc->a[2] };
  POINT3D normal = { .x = arc->normal[0], .y = arc->normal[1],
    .z = arc->normal[2] };
  POINT3D b;
  dggs_vec_cross(&normal, &a, &b);
  double theta0 = tmin * arc->dist;
  double c0 = cos(theta0), s0 = sin(theta0);
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    if (entry & (1u << i))
      continue;
    POINT3D m = { .x = normals[3 * i], .y = normals[3 * i + 1],
      .z = normals[3 * i + 2] };
    if (m.x == 0.0 && m.y == 0.0 && m.z == 0.0)
      continue;
    double ma = dggs_vec_dot(&m, &a), mb = dggs_vec_dot(&m, &b);
    if (hypot(ma, mb) == 0.0)
      continue;              /* the path lies in the plane of the edge */
    double t;
    /* The height above the plane at `tmin` and the rate it changes at. A path
     * at or outside the plane and DESCENDING through it leaves the cell at
     * `tmin` itself, as a line starting on a tile boundary and heading across
     * it leaves the tile at once: the exit is that parameter and never the one
     * after it. The rate separates the two ways a path can sit on an edge by
     * their kind and not by any magnitude: a path CROSSING the edge descends
     * at the rate of the path itself, while a path RUNNING ALONG it holds a
     * height of zero and a rate of zero */
    double f = ma * c0 + mb * s0, d = mb * c0 - ma * s0;
    if (f <= 0.0 && d < 0.0)
      t = tmin;
    else
    {
      double theta = atan2(mb, ma) + M_PI_2;
      /* The first such angle strictly ahead of `tmin` */
      theta += 2.0 * M_PI * ceil((theta0 - theta) / (2.0 * M_PI));
      if (theta <= theta0)
        theta += 2.0 * M_PI;
      /* A path inside the plane and descending toward it, `f > 0` and
       * `d < 0`, reaches it less than a quarter turn ahead, since the height
       * `f cos(delta) + d sin(delta)` falls to zero at `delta = atan2(f, -d)`,
       * which lies strictly between 0 and pi/2. The angle read from `phi`
       * loses that distance where it is below the last place of `phi`, and
       * the zero it states then falls at or behind `tmin` and is carried a
       * whole turn on. The distance is read from the height and rate at
       * `tmin` instead, where it is stated to its own precision */
      if (f > 0.0 && d < 0.0 && theta >= theta0 + M_PI)
        theta = theta0 + atan2(f, -d);
      t = theta / arc->dist;
    }
    if (t <= 1.0 && t < best)
    {
      best = t;
      if (edge)
        *edge = i;
    }
  }
  return best;
}

/**
 * @brief Return where a geodetic path leaves a convex cell, read as the
 * intersection of the hemispheres its edge circles bound
 * @details The cell is stated by its vertices, and the inward normal of each
 * edge plane is the cross product of the two vertices that join along it,
 * taken on the side the centre of the vertices lies on.
 * #dggs_arc_normals_exit_param states the exit from those normals.
 * @param[in] arc Path
 * @param[in] lons,lats Vertices of the cell boundary in radians, in the
 * order they join
 * @param[in] count Number of vertices
 * @param[in] tmin Parameter at which the path entered the cell
 * @param[in] entry Mask of the edges the path entered the cell through, bit
 * `i` for the edge from vertex `i`
 * @param[out] edge When not `NULL`, the edge crossed at the exit
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_arc_hemisphere_exit_param(const DggsArc *arc, const double *lons,
  const double *lats, int count, double tmin, uint32 entry, int *edge)
{
  assert(arc); assert(lons); assert(lats);
  POINT3D centre = { .x = 0.0, .y = 0.0, .z = 0.0 };
  for (int i = 0; i < count; i++)
  {
    GEOGRAPHIC_POINT g = { .lat = lats[i], .lon = lons[i] };
    POINT3D v;
    geog2cart(&g, &v);
    centre.x += v.x; centre.y += v.y; centre.z += v.z;
  }
  assert(count <= DGGS_MAX_CELL_VERTS);
  double normals[3 * DGGS_MAX_CELL_VERTS];
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D m;
    /* Read from the angles of the vertices, as #dggs_arc_exit_param does */
    robust_cross_product(&gi, &gj, &m);
    if (! (m.x == 0.0 && m.y == 0.0 && m.z == 0.0))
    {
      normalize(&m);
      if (dggs_vec_dot(&m, &centre) < 0.0)
      {
        m.x = -m.x; m.y = -m.y; m.z = -m.z;
      }
    }
    normals[3 * i] = m.x; normals[3 * i + 1] = m.y; normals[3 * i + 2] = m.z;
  }
  return dggs_arc_normals_exit_param(arc, normals, count, tmin, entry, edge);
}

/**
 * @brief Return where a geodetic path leaves a cell
 * @details A cell edge is an arc of a great circle, as the path is. The
 * decisions of the exit are signs on the input, the vertices of the cell and
 * the circle of the path, each decided exactly (#dot_product_sign,
 * #triple_product_sign); only the parameter of the exit is computed in
 * floating point.
 *
 * WHICH EDGES THE CIRCLE LEAVES THROUGH. The circle of the path meets an edge
 * exactly when it separates the edge's two vertices, the sign of the dot
 * product of the normal of the circle and each vertex. A vertex ON the circle
 * counts on its right, as if the path ran an infinitesimal distance to its
 * left, so each edge holds its first vertex and a crossing at a vertex belongs
 * to exactly one of the two edges meeting there. With the vertices in
 * counterclockwise order the cell lies on the left of each edge, so the
 * circle crosses the edge outward exactly where the edge runs from the right
 * of the path to its left. Holding its first vertex, the edge also holds the
 * crossing of a path passing through that vertex, and a path running along an
 * edge has both vertices on its right and never leaves through it.
 *
 * WHERE. The height of the path above the plane of the edge, `<m, p>` for the
 * inward normal `m`, is `f` at `tmin` and changes at the rate `d` there, so
 * it falls through zero at the angle `atan2(f, -d)` ahead of `tmin`: the
 * crossing the edge holds, since the circle crosses the edge's plane
 * downward once a turn. Where `d < 0` that zero lies within a quarter turn of
 * `tmin`, on either side of it: behind, `f < 0`, the path is past the plane
 * of the edge and heading further out, which in a cell that is not convex is
 * a crossing already passed, and near `tmin` rounding cannot tell the side at
 * all. The input decides it:
 *
 * - Past an entry edge adjoining the edge at a vertex, the path met both
 *   edges, and the turn of the boundary at that vertex orders the two
 *   crossings: where it turns left, the corner is convex and the cell lies
 *   between the edges, so the path enters and then leaves, and the exit is
 *   at `tmin` or ahead of it; where it turns right, the path leaves and then
 *   enters again, and the exit through that edge is behind it.
 * - At the first endpoint, which no entry edge states, the side of the edge's
 *   plane the endpoint lies on decides; an endpoint beyond that plane is
 *   outside the cell exactly when it lies in the sector of that edge, the
 *   cone from the centre of the cell through the edge's two vertices, since
 *   a cell is star-shaped from the sum of its vertices, even one bent across
 *   a face of the icosahedron, and holds of that sector only the side of the
 *   edge it lies on. The path then leaves at once.
 * - Any other edge shares no vertex with the edge the path entered through,
 *   so its crossing lies away from `tmin` by the distance between the two
 *   edges, and the side the computed angle reads is the side it lies on.
 *
 * @param[in] arc Path
 * @param[in] lons,lats Vertices of the cell boundary in radians, in
 * counterclockwise order
 * @param[in] count Number of vertices
 * @param[in] tmin Parameter at which the path entered the cell
 * @param[in] entry Mask of the edges the path entered the cell through, bit
 * `i` for the edge from vertex `i`, which it never leaves through: a line
 * does not cross back over the tile boundary it has just crossed
 * @param[out] edge When not `NULL`, the edge crossed at the exit, the one
 * from vertex `edge` to the next vertex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_arc_exit_param(const DggsArc *arc, const double *lons,
  const double *lats, int count, double tmin, uint32 entry, int *edge)
{
  assert(arc); assert(lons); assert(lats);
  assert(count <= DGGS_MAX_CELL_VERTS);
  POINT3D a = { .x = arc->a[0], .y = arc->a[1], .z = arc->a[2] };
  POINT3D normal = { .x = arc->normal[0], .y = arc->normal[1],
    .z = arc->normal[2] };
  POINT3D b;
  dggs_vec_cross(&normal, &a, &b);
  double theta0 = tmin * arc->dist;
  double c0 = cos(theta0), s0 = sin(theta0);
  POINT3D v[DGGS_MAX_CELL_VERTS], centre = { .x = 0.0, .y = 0.0, .z = 0.0 };
  bool left[DGGS_MAX_CELL_VERTS];
  for (int i = 0; i < count; i++)
  {
    GEOGRAPHIC_POINT g = { .lat = lats[i], .lon = lons[i] };
    geog2cart(&g, &v[i]);
    centre.x += v[i].x; centre.y += v[i].y; centre.z += v[i].z;
    left[i] = dot_product_sign(&normal, &v[i]) > 0;
  }
  bool start = (entry == 0 && tmin == 0.0);
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    if ((entry & (1u << i)) || left[i] || ! left[j])
      continue;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D m;
    /* The normal of the circle of the edge, read from the angles of its
     * vertices: their Cartesian cross product loses to cancellation all but
     * the rounding of a unit vector, which on the edge of a fine cell places
     * the circle millimetres off the vertices */
    robust_cross_product(&gi, &gj, &m);
    double ma = dggs_vec_dot(&m, &a), mb = dggs_vec_dot(&m, &b);
    double f = ma * c0 + mb * s0, d = mb * c0 - ma * s0;
    double delta = atan2(f, -d);
    if (d < 0.0)
    {
      /* The falling zero lies within a quarter turn of `tmin`, on either
       * side */
      int prev = (i + count - 1) % count, next = (j + 1) % count;
      if ((entry & (1u << prev)) && left[prev])
      {
        /* Entered through the edge ending at vertex `i` */
        if (triple_product_sign(&v[prev], &v[i], &v[j]) <= 0)
          continue;
        delta = fmax(delta, 0.0);
      }
      else if ((entry & (1u << j)) && ! left[next])
      {
        /* Entered through the edge starting at vertex `j` */
        if (triple_product_sign(&v[i], &v[j], &v[next]) <= 0)
          continue;
        delta = fmax(delta, 0.0);
      }
      else if (start)
      {
        if (triple_product_sign(&v[i], &v[j], &a) < 0 &&
            ! (triple_product_sign(&centre, &v[i], &a) >= 0 &&
               triple_product_sign(&centre, &v[j], &a) < 0))
          continue;
        delta = fmax(delta, 0.0);
      }
      else if (delta < 0.0)
        continue;
    }
    else if (delta < 0.0)
      delta += 2.0 * M_PI;
    double t = (theta0 + delta) / arc->dist;
    if (t <= 1.0 && t < best)
    {
      best = t;
      if (edge)
        *edge = i;
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
 * @brief Return true if an endpoint of a geodetic path lies on a meridian,
 * read from the degrees both are stated in
 * @details The longitudes -180 and 180 state one meridian, and a pole lies on
 * every meridian
 * @param[in] arc Path
 * @param[in] end 0 for the first endpoint, 1 for the second one
 * @param[in] lon Longitude of the meridian, in degrees
 */
bool
dggs_arc_end_on_meridian(const DggsArc *arc, int end, double lon)
{
  assert(arc); assert(end == 0 || end == 1);
  double elon = arc->endlon[end], elat = arc->endlat[end];
  return fabs(elat) == 90.0 || elon == lon || fabs(elon - lon) == 360.0;
}

/**
 * @brief Return true if an endpoint of a geodetic path lies on a parallel,
 * read from the degrees both are stated in
 * @param[in] arc Path
 * @param[in] end 0 for the first endpoint, 1 for the second one
 * @param[in] lat Latitude of the parallel, in degrees
 */
bool
dggs_arc_end_on_parallel(const DggsArc *arc, int end, double lat)
{
  assert(arc); assert(end == 0 || end == 1);
  return arc->endlat[end] == lat;
}

/**
 * @brief Return 1 when a geodetic path passes north of a latitude between its
 * endpoints, -1 when it passes south of it, and 0 otherwise
 * @details The path passes beyond the latitude where its circle reaches an
 * extreme of latitude between its endpoints lying beyond it, the extremes
 * being the ones #dggs_lonlat_segment_extend_box reads, so the box bounding
 * the path and the sides of a parallel the path reaches state one path
 * @param[in] arc Path
 * @param[in] lat Latitude, in degrees
 */
static int
dggs_arc_passes_beyond(const DggsArc *arc, double lat)
{
  double end1[3], end2[3], ext;
  dggs_lonlat_to_xyz(arc->endlon[0], arc->endlat[0], end1);
  dggs_lonlat_to_xyz(arc->endlon[1], arc->endlat[1], end2);
  if (dggs_arc_lat_extreme(end1, end2, true, &ext) && ext > lat)
    return 1;
  if (dggs_arc_lat_extreme(end1, end2, false, &ext) && ext < lat)
    return -1;
  return 0;
}

/**
 * @brief Return in the last argument the direction from the centre of the
 * sphere of a position stated in degrees
 * @details Read from the degrees as #dggs_arc_init reads the first endpoint of
 * a path, through `float8_sind` and `float8_cosd`, which answer a quarter
 * turn exactly; the vector is not normalized, since the signs it serves are
 * read from directions alone
 */
static void
dggs_lonlat_direction(double lon, double lat, POINT3D *p)
{
  double clat = float8_cosd(lat);
  p->x = clat * float8_cosd(lon);
  p->y = clat * float8_sind(lon);
  p->z = float8_sind(lat);
}

/**
 * @brief Return true if a geodetic path passes through a position strictly
 * between its endpoints
 * @details The position lies on the circle of the path when the directions of
 * the two endpoints and of the position are coplanar, the sign
 * #triple_product_sign decides exactly on the directions read from the
 * degrees each is stated in. It lies between the endpoints when it lies on
 * the side of the first endpoint's plane with the second endpoint and on the
 * side of the second endpoint's plane with the first, which is decided away
 * from zero since the position is apart from both.
 * @param[in] arc Path
 * @param[in] lon,lat Position, in degrees
 */
bool
dggs_arc_passes_through(const DggsArc *arc, double lon, double lat)
{
  assert(arc);
  if ((lon == arc->endlon[0] && lat == arc->endlat[0]) ||
      (lon == arc->endlon[1] && lat == arc->endlat[1]))
    return false;
  POINT3D a, b, v, n, av, vb;
  dggs_lonlat_direction(arc->endlon[0], arc->endlat[0], &a);
  dggs_lonlat_direction(arc->endlon[1], arc->endlat[1], &b);
  dggs_lonlat_direction(lon, lat, &v);
  if (triple_product_sign(&a, &b, &v) != 0)
    return false;
  dggs_vec_cross(&a, &b, &n);
  dggs_vec_cross(&a, &v, &av);
  dggs_vec_cross(&v, &b, &vb);
  return dggs_vec_dot(&av, &n) > 0.0 && dggs_vec_dot(&vb, &n) > 0.0;
}

/**
 * @brief Return in the last two arguments the signs of the eastward and of
 * the northward heading of a geodetic path at a position on its circle
 * @details The path travels its circle in the direction of the normal of the
 * circle crossed with the position, as #dggs_arc_normals_exit_param reads it
 * at the first endpoint. Its eastward heading is the component of that
 * direction along the normal of the position's meridian, and its northward
 * heading is the height the direction gains, since the direction is
 * perpendicular to the position. A sign is zero where the path runs along
 * the meridian or along the parallel there
 * @param[in] arc Path
 * @param[in] lon,lat Position on the circle of the path, in degrees
 * @param[out] east,north Signs of the two headings
 */
void
dggs_arc_heading_at(const DggsArc *arc, double lon, double lat, int *east,
  int *north)
{
  assert(arc); assert(east); assert(north);
  POINT3D v, dir;
  const POINT3D normal = { .x = arc->normal[0], .y = arc->normal[1],
    .z = arc->normal[2] };
  const POINT3D m = { .x = -float8_sind(lon), .y = float8_cosd(lon),
    .z = 0.0 };
  dggs_lonlat_direction(lon, lat, &v);
  dggs_vec_cross(&normal, &v, &dir);
  double e = dggs_vec_dot(&dir, &m);
  *east = (e > 0.0) - (e < 0.0);
  *north = (dir.z > 0.0) - (dir.z < 0.0);
}

/**
 * @brief Return in @p params the parameters at which a geodetic path meets the
 * plane of a meridian, and their number
 * @details The path leaves the hemisphere on one side of the plane where
 * #dggs_arc_normals_exit_param states for the normal of the plane read from
 * the degrees the meridian is stated in, and it leaves the hemisphere on the
 * other side where the same function states for the opposite normal, which is
 * how a walk of a grid reads a tile boundary. An endpoint the path states on
 * the meridian meets the plane there, at its own parameter: the plane passes
 * through the centre, so the path meets it nowhere else, and the parameter
 * the angles state for it instead is rounded to either side of the endpoint,
 * which dates the crossing a microsecond before the instant the trip states
 * it at, or past the end of the path.
 * @param[in] arc Path
 * @param[in] lon Longitude of the meridian, in degrees
 * @param[out] params Array of at least two parameters, in `[0, 1]`
 * @return Number of parameters written
 */
int
dggs_arc_meridian_params(const DggsArc *arc, double lon, double *params)
{
  assert(arc); assert(params);
  bool on1 = dggs_arc_end_on_meridian(arc, 0, lon);
  bool on2 = dggs_arc_end_on_meridian(arc, 1, lon);
  if (on1 && on2)
    return 0;                /* the path runs along the meridian */
  if (on1 || on2)
  {
    params[0] = on1 ? 0.0 : 1.0;
    return 1;
  }
  const double normals[6] = { -float8_sind(lon), float8_cosd(lon), 0.0,
    float8_sind(lon), -float8_cosd(lon), 0.0 };
  int count = 0;
  for (int s = 0; s < 2; s++)
  {
    double t = dggs_arc_normals_exit_param(arc, &normals[3 * s], 1, 0.0, 0,
      NULL);
    if (t <= 1.0)
      params[count++] = t;
  }
  return count;
}

/**
 * @brief Return in @p params the parameters at which a geodetic path meets a
 * parallel, and their number
 * @details The crossings #dggs_arc_plane_params states for the plane of
 * constant height of the parallel. An endpoint the path states on the parallel
 * meets it there, at its own parameter: of the two angles at which the circle
 * of the path meets the parallel, the one nearer the endpoint is that
 * endpoint's, and the other one is the crossing the path makes elsewhere.
 * @param[in] arc Path
 * @param[in] lat Latitude of the parallel, in degrees
 * @param[out] params Array of at least two parameters, in `[0, 1]` and
 * ascending
 * @return Number of parameters written
 */
int
dggs_arc_parallel_params(const DggsArc *arc, double lat, double *params)
{
  assert(arc); assert(params);
  bool on1 = dggs_arc_end_on_parallel(arc, 0, lat);
  bool on2 = dggs_arc_end_on_parallel(arc, 1, lat);
  const double pole[3] = { 0.0, 0.0, 1.0 };
  if (! on1 && ! on2)
    return dggs_arc_plane_params(arc, pole, sin(lat * M_PI / 180.0), params);
  if (on1 && on2)
  {
    /* Both endpoints lie on the parallel, which the circle of the path meets
     * at those two positions alone */
    params[0] = 0.0; params[1] = 1.0;
    return 2;
  }
  /* A path passing beyond the parallel through one of its endpoints nowhere
   * (#dggs_arc_passes_beyond) meets it at that endpoint alone: a crossing
   * the angles place beside an endpoint where the path touches the parallel
   * is none */
  if (dggs_arc_passes_beyond(arc, lat) == 0)
  {
    params[0] = on1 ? 0.0 : 1.0;
    return 1;
  }
  /* The two angles at which the circle of the path meets the parallel, as
   * #dggs_arc_plane_params reads them */
  const double *a = arc->a, *nm = arc->normal;
  const double b[3] = { nm[1] * a[2] - nm[2] * a[1],
    nm[2] * a[0] - nm[0] * a[2], nm[0] * a[1] - nm[1] * a[0] };
  double c = sin(lat * M_PI / 180.0);
  double r = hypot(a[2], b[2]);
  double other = -1.0;
  if (r > 0.0 && fabs(c) <= r)
  {
    double base = atan2(b[2], a[2]), half = acos(c / r);
    double theta[2];
    for (int s = 0; s < 2; s++)
    {
      theta[s] = fmod(base + (2 * s - 1) * half, 2.0 * M_PI);
      if (theta[s] < 0.0)
        theta[s] += 2.0 * M_PI;
    }
    /* The angle of the endpoint on the parallel, 0 or that of the path */
    double at = on1 ? 0.0 : arc->dist;
    double d0 = fabs(theta[0] - at), d1 = fabs(theta[1] - at);
    d0 = Min(d0, 2.0 * M_PI - d0);
    d1 = Min(d1, 2.0 * M_PI - d1);
    double t = ((d0 <= d1) ? theta[1] : theta[0]) / arc->dist;
    if (t >= 0.0 && t <= 1.0)
      other = t;
  }
  int count = 0;
  if (on1)
    params[count++] = 0.0;
  if (other >= 0.0)
    params[count++] = other;
  if (on2)
    params[count++] = 1.0;
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
 * @brief Return where a geodetic path reaches a pole, or a value above 1 when
 * it reaches none ahead of a parameter
 * @details A path reaches a pole exactly when the circle it follows holds the
 * axis of the sphere, which its normal states by a third coordinate of zero.
 * #dggs_arc_init reads the normal of a path along one meridian from the
 * ANGLES of its endpoints, where that coordinate cancels exactly, so the test
 * is the exact `normal[2] == 0` and no magnitude decides it.
 *
 * Every meridian meets at a pole, so a path reaching one leaves the meridians
 * bounding its cell ALL AT THE SAME PARAMETER, and every cell between the one
 * it arrives in and the one it leaves by is held for no time. The path
 * continues along the meridian half a turn from the one it arrived by.
 * @param[in] arc Path
 * @param[in] north True for the north pole, false for the south
 * @param[in] tmin Parameter the pole lies strictly ahead of
 * @return The path parameter at the pole, or a value above 1
 */
double
dggs_arc_pole_param(const DggsArc *arc, bool north, double tmin)
{
  assert(arc);
  if (arc->normal[2] != 0.0)
    return 2.0;                /* the circle does not hold the axis */
  const double *a = arc->a, *nm = arc->normal;
  const double b[3] = { nm[1] * a[2] - nm[2] * a[1],
    nm[2] * a[0] - nm[0] * a[2], nm[0] * a[1] - nm[1] * a[0] };
  /* The height above the equator is `a[2] cos(theta) + b[2] sin(theta)`, and
   * it reaches the pole where that height is at its extreme */
  double theta = north ? atan2(b[2], a[2]) : atan2(-b[2], -a[2]);
  if (theta < 0.0)
    theta += 2.0 * M_PI;
  double t = theta / arc->dist;
  return (t > tmin && t <= 1.0) ? t : 2.0;
}

/**
 * @brief Return where a geodetic path leaves the side of a plane through the
 * sphere that a cell lies on
 * @details The crossing of #dggs_arc_plane_param, and @p tmin itself for a
 * path that already sits ON the plane there and heads across it, which is the
 * rule #dggs_arc_normals_exit_param reads from the height and its rate for a
 * plane through the centre. A path reaching a CORNER of a cell crosses two of
 * its boundaries at ONE parameter, so the second is crossed exactly at the
 * parameter the first was: a search strictly ahead of that parameter states no
 * crossing, and the walk steps through one boundary and never through the
 * other.
 * @param[in] arc Path
 * @param[in] m Unit normal of the plane
 * @param[in] c Offset of the plane from the centre
 * @param[in] above True when the cell lies where the height of a position
 * above the plane is greater than @p c
 * @param[in] tmin Parameter the crossing lies at or ahead of
 * @return The path parameter of the exit, or a value above 1 when the path
 * stays on its side of the plane through the end
 */
double
dggs_arc_plane_exit_param(const DggsArc *arc, const double m[3], double c,
  bool above, double tmin)
{
  assert(arc); assert(m);
  const double *a = arc->a, *nm = arc->normal;
  const double b[3] = { nm[1] * a[2] - nm[2] * a[1],
    nm[2] * a[0] - nm[0] * a[2], nm[0] * a[1] - nm[1] * a[0] };
  double ma = a[0] * m[0] + a[1] * m[1] + a[2] * m[2];
  double mb = b[0] * m[0] + b[1] * m[1] + b[2] * m[2];
  double theta0 = tmin * arc->dist;
  double c0 = cos(theta0), s0 = sin(theta0);
  /* The height of the path above the plane on the side the cell lies, and the
   * rate it changes at. The side sets the sign of both, as the inward normal
   * does for a plane through the centre */
  double sign = above ? 1.0 : -1.0;
  double f = sign * (ma * c0 + mb * s0 - c);
  double d = sign * (mb * c0 - ma * s0);
  if (f <= 0.0 && d < 0.0)
    return tmin;
  /* The crossing a path makes at a CORNER lies at the parameter the other
   * boundary was crossed at, so it is taken AT `tmin` and not only ahead of
   * it. Which of the two holds reads from the RATE and never from the height:
   * a path DESCENDING toward the plane there is leaving through it, while one
   * that entered the cell through it rises away and keeps its crossing
   * strictly ahead. The height at a corner is a residue of the last place,
   * and no magnitude of it decides anything */
  double params[2], best = 2.0;
  int count = dggs_arc_plane_params(arc, m, c, params);
  for (int i = 0; i < count; i++)
    if ((d < 0.0 ? params[i] >= tmin : params[i] > tmin) && params[i] < best)
      best = params[i];
  return best;
}

/**
 * @brief Return where a geodetic path leaves the side of a parallel that a
 * cell lies on
 * @details The exit #dggs_arc_plane_exit_param states for the plane of
 * constant height of the parallel, read from the crossings
 * #dggs_arc_parallel_params states, so an endpoint the path states on the
 * parallel meets it at its own parameter
 * @param[in] arc Path
 * @param[in] lat Latitude of the parallel, in degrees
 * @param[in] above True when the cell lies north of the parallel
 * @param[in] tmin Parameter the crossing lies at or ahead of
 * @return The path parameter of the exit, or a value above 1 when the path
 * stays on its side of the parallel through the end
 */
double
dggs_arc_parallel_exit_param(const DggsArc *arc, double lat, bool above,
  double tmin)
{
  assert(arc);
  const double *a = arc->a, *nm = arc->normal;
  const double b[3] = { nm[1] * a[2] - nm[2] * a[1],
    nm[2] * a[0] - nm[0] * a[2], nm[0] * a[1] - nm[1] * a[0] };
  double c = sin(lat * M_PI / 180.0);
  double theta0 = tmin * arc->dist;
  double c0 = cos(theta0), s0 = sin(theta0);
  /* The height of the path above the parallel on the side the cell lies, and
   * the rate it changes at, as #dggs_arc_plane_exit_param reads them */
  double sign = above ? 1.0 : -1.0;
  double f = sign * (a[2] * c0 + b[2] * s0 - c);
  double d = sign * (b[2] * c0 - a[2] * s0);
  if (f <= 0.0 && d < 0.0)
    return tmin;
  double params[2], best = 2.0;
  int count = dggs_arc_parallel_params(arc, lat, params);
  for (int i = 0; i < count; i++)
    if ((d < 0.0 ? params[i] >= tmin : params[i] > tmin) && params[i] < best)
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
 * path lies in a box of longitudes and latitudes
 * @details The function also returns the number of such spans. A box is
 * bounded by the planes of two meridians, which pass through the centre of
 * the sphere, and by the planes of constant height of two parallels. The path
 * meets each of them at parameters that cut it into pieces lying wholly
 * inside the box or wholly outside it, so the position halfway along a piece
 * says which. A path is therefore clipped where it crosses the box and not
 * where a straight line in longitude and latitude would, which no geodetic
 * path follows.
 *
 * A meridian is crossed where #dggs_arc_meridian_params states and a parallel
 * where #dggs_arc_parallel_params states, which are the crossings a walk of a
 * grid on the sphere reads for the boundaries of its tiles, so a box and a
 * tile sharing a boundary are met at the same parameter, and an endpoint the
 * path states on a boundary meets it at its own parameter.
 *
 * A bound of a span lies on the upper border of the box when a crossing of
 * the east meridian or of the north parallel is at that parameter, so a box
 * leaving its upper border out states which bounds it leaves out. The plane of
 * a meridian holds the meridian half a turn from it too, and a crossing of that
 * one bounds no side of the box.
 * @param[in] arc Path
 * @param[in] xmin,ymin,xmax,ymax Bounds of the box, in degrees
 * @param[in] border_inc True when the box contains its upper border, which
 * a path running along that border then lies in
 * @param[out] tin,tout Arrays of at least @p maxout parameters
 * @param[out] tin_upper,tout_upper Arrays of at least @p maxout flags, true
 * when the bound lies on the upper border of the box
 * @param[in] maxout Capacity of the arrays
 * @return Number of spans written
 */
int
dggs_arc_lonlat_box_spans(const DggsArc *arc, double xmin, double ymin,
  double xmax, double ymax, bool border_inc, double *tin, double *tout,
  bool *tin_upper, bool *tout_upper, int maxout)
{
  assert(arc); assert(tin); assert(tout); assert(tin_upper);
  assert(tout_upper);
  if (maxout < 1)
    return 0;
  /* The crossings of the two meridians and of the two parallels of the box,
   * each crossing of a meridian with whether it lies on that meridian rather
   * than on the one half a turn from it */
  const double lons[2] = { xmin, xmax }, lats[2] = { ymin, ymax };
  double mer[2][2], par[2][2];
  bool side[2][2];
  int nmer[2], npar[2];
  for (int k = 0; k < 2; k++)
  {
    nmer[k] = dggs_arc_meridian_params(arc, lons[k], mer[k]);
    for (int i = 0; i < nmer[k]; i++)
    {
      double plon, plat;
      side[k][i] = dggs_arc_point(arc, mer[k][i], &plon, &plat) &&
        cos(plon - lons[k] * M_PI / 180.0) > 0.0;
    }
    npar[k] = dggs_arc_parallel_params(arc, lats[k], par[k]);
  }
  /* A path running along a meridian of the box states it by the longitude of
   * both endpoints, a pole standing on every meridian, and one running along
   * a parallel does so on the equator alone, the one parallel that is a great
   * circle. Its position there is the line itself, which the angles of a
   * position halfway along state to either side of it, and it lies on the
   * upper border of the box when that line is the east meridian or the north
   * parallel. The longitude is the one the endpoints state: a box holds the
   * longitudes it bounds as they are stated, so the antimeridian stated at
   * 180 lies in the tile starting at 180 and not in the one starting at
   * -180 */
  bool along_lon[2], along_lat[2];
  bool pole1 = fabs(arc->endlat[0]) == 90.0, pole2 = fabs(arc->endlat[1]) == 90.0;
  for (int k = 0; k < 2; k++)
  {
    along_lon[k] = ! (pole1 && pole2) &&
      (pole1 || arc->endlon[0] == lons[k]) &&
      (pole2 || arc->endlon[1] == lons[k]);
    along_lat[k] = lats[k] == 0.0 && dggs_arc_end_on_parallel(arc, 0, 0.0) &&
      dggs_arc_end_on_parallel(arc, 1, 0.0);
  }
  if (! border_inc && (along_lon[1] || along_lat[1]))
    return 0;
  /* A path through a corner of the box crosses its meridian and its parallel
   * at one position, which the angles of the two state at parameters rounded
   * apart, so the path would hold a neighbouring box for no time between
   * them. The path passes through the corner when #dggs_arc_passes_through
   * decides so exactly, or when the angles state the two crossings at one
   * parameter, which is then one position on both lines. The crossing is
   * then the one of the meridian, which the parallel states as well. The box
   * holds the corner though the path holds no piece of it, and the corner is
   * then held for that instant alone */
  double corner[4];
  bool corner_upper[4];
  int ncorners = 0;
  for (int kx = 0; kx < 2; kx++)
  {
    for (int ky = 0; ky < 2; ky++)
    {
      /* A path running along one of the two lines meets the other one there
       * and crosses nothing else */
      if (fabs(lats[ky]) == 90.0 || along_lon[kx] || along_lat[ky])
        continue;
      /* The position a path passes through on the antimeridian is stated at
       * 180, and lies in the tile starting there */
      if (lons[kx] == -180.0)
        continue;
      int im = -1;
      for (int i = 0; i < nmer[kx]; i++)
        if (side[kx][i])
          im = i;
      if (im < 0)
        continue;
      double tv = mer[kx][im];
      if (tv <= 0.0 || tv >= 1.0)
        continue;            /* an endpoint states its own position */
      int ip = -1;
      for (int i = 0; i < npar[ky]; i++)
        if (ip < 0 || fabs(par[ky][i] - tv) < fabs(par[ky][ip] - tv))
          ip = i;
      if (! (ip >= 0 && par[ky][ip] == tv) &&
          ! dggs_arc_passes_through(arc, lons[kx], lats[ky]))
        continue;
      if (ip >= 0)
        par[ky][ip] = tv;
      corner[ncorners] = tv;
      corner_upper[ncorners++] = (kx == 1 || ky == 1);
    }
  }
  /* The parameters at which the path reaches a bound of the box, each with
   * whether it lies on the upper border and with the lines it crosses there,
   * bit `k` for the west meridian, the east one, the south parallel and the
   * north one in turn */
  double cuts[12];
  bool upper[12];
  int flips[12];
  int ncuts = 0;
  cuts[ncuts] = 0.0; upper[ncuts] = false; flips[ncuts++] = 0;
  cuts[ncuts] = 1.0; upper[ncuts] = false; flips[ncuts++] = 0;
  for (int k = 0; k < 2; k++)
  {
    for (int i = 0; i < nmer[k]; i++)
    {
      cuts[ncuts] = mer[k][i]; upper[ncuts] = (k == 1) && side[k][i];
      flips[ncuts++] = 1 << k;
    }
    for (int i = 0; i < npar[k]; i++)
    {
      cuts[ncuts] = par[k][i]; upper[ncuts] = (k == 1);
      flips[ncuts++] = 1 << (2 + k);
    }
  }
  /* In ascending order, which is the order the path passes them */
  for (int i = 1; i < ncuts; i++)
  {
    double v = cuts[i];
    bool u = upper[i];
    int f = flips[i];
    int j = i - 1;
    while (j >= 0 && cuts[j] > v)
    {
      cuts[j + 1] = cuts[j]; upper[j + 1] = upper[j]; flips[j + 1] = flips[j];
      j--;
    }
    cuts[j + 1] = v; upper[j + 1] = u; flips[j + 1] = f;
  }
  /* The crossings at one parameter state one position, which lies on the upper
   * border when any of them does. A line met twice there is touched and not
   * crossed */
  int nuniq = 0;
  for (int i = 0; i < ncuts; i++)
  {
    if (nuniq > 0 && cuts[nuniq - 1] == cuts[i])
    {
      upper[nuniq - 1] |= upper[i];
      flips[nuniq - 1] ^= flips[i];
    }
    else
    {
      cuts[nuniq] = cuts[i]; upper[nuniq] = upper[i];
      flips[nuniq++] = flips[i];
    }
  }
  /* A box narrower than half a turn holds the longitudes of the two
   * hemispheres its meridians bound, and the path changes side of a line
   * exactly where it crosses that line, so the side of each line a piece
   * lies on follows from the side the path starts on and from the lines
   * crossed before the piece, in the order the path crosses them. A piece
   * whose two cuts are rounded apart is then read on the side the order of
   * the crossings puts it, as a walk of the grid reads it, where the angles
   * of its halfway position may state the other side. The side the path
   * starts on is read from the degrees of its first endpoint, and from its
   * heading when that endpoint lies on the line; a path running along a line
   * lies on the box's side of it. A wider box is read from the position
   * halfway along each piece */
  bool narrow = xmax - xmin < 180.0 && xmin >= -180.0 && xmax <= 180.0;
  int state = 0;
  if (narrow)
  {
    double lon1 = arc->endlon[0], lat1 = arc->endlat[0];
    int east, north;
    dggs_arc_heading_at(arc, lon1, lat1, &east, &north);
    /* A path starting on a parallel heads first to the side of it that it
     * passes beyond (#dggs_arc_passes_beyond), and toward its other endpoint
     * when it passes beyond neither. Its heading there is otherwise read
     * from a direction that is due east or west where the path starts at an
     * extreme of latitude */
    north = dggs_arc_passes_beyond(arc, lat1);
    if (north == 0)
      north = (arc->endlat[1] > lat1) - (arc->endlat[1] < lat1);
    /* A path leaving a pole runs along the meridian of its other endpoint,
     * whichever longitude states the pole, and lies on the side of each
     * meridian of the box that that one lies on */
    double slon = (fabs(lat1) == 90.0) ? arc->endlon[1] : lon1;
    for (int k = 0; k < 2; k++)
    {
      /* The side of the meridian's plane, read from the difference of the
       * longitudes: the start lies on the plane on the meridian itself and
       * on the one half a turn from it, where heading east leaves the plane
       * to the west of the meridian */
      double d = slon - lons[k];
      if (d > 180.0)
        d -= 360.0;
      else if (d <= -180.0)
        d += 360.0;
      double d2 = arc->endlon[1] - lons[k];
      if (d2 > 180.0)
        d2 -= 360.0;
      else if (d2 <= -180.0)
        d2 += 360.0;
      int s = (d > 0.0 && d < 180.0) ? 1 : ((d < 0.0) ? -1 : 0);
      /* A pole lies on every meridian, on no side of one */
      int s2 = (fabs(arc->endlat[1]) == 90.0) ? 0 :
        ((d2 > 0.0 && d2 < 180.0) ? 1 : ((d2 < 0.0 && d2 > -180.0) ? -1 : 0));
      /* A path with both endpoints on the plane of the meridian runs along
       * that plane and lies on neither side of it, which its heading states
       * only to the last place of the normal of its circle; a path starting
       * on the plane and leaving it heads to the side it goes on to */
      if (s == 0 && s2 != 0)
        s = (d == 0.0) ? east : -east;
      /* The side the path ends on follows from the side it starts on and
       * from the crossings of the line between them. A crossing the angles
       * state at or before the start of a path starting a rounding away from
       * the line is lost, and the path then starts on the side the crossings
       * and its end state, where the end lies farther from the line than the
       * start. The line is the plane of the meridian, which holds the
       * meridian half a turn from it too, so the distance of a position to it
       * is the smaller of its longitude difference to either */
      int ncross = 0;
      for (int i = 0; i < nmer[k]; i++)
        ncross += (mer[k][i] > 0.0 && mer[k][i] < 1.0);
      double dist1 = Min(fabs(d), 180.0 - fabs(d));
      double dist2 = Min(fabs(d2), 180.0 - fabs(d2));
      if (s != 0 && s2 != 0 && d != 0.0 && fabs(lat1) != 90.0 &&
          ((ncross % 2 == 1) != (s != s2)) && dist1 <= dist2)
        s = -s;
      /* East of the west meridian and west of the east one. A path lying on
       * the plane of the meridian runs along the meridian as its endpoints
       * state it, or along the meridian half a turn from it, which a box
       * narrower than half a turn does not hold */
      if (s == 0 ? along_lon[k] : (k == 0 ? s > 0 : s < 0))
        state |= 1 << k;
      int t = (lat1 > lats[k]) ? 1 : ((lat1 < lats[k]) ? -1 : north);
      /* The same reading of the sides of a parallel */
      double lat2 = arc->endlat[1];
      int t2 = (lat2 > lats[k]) - (lat2 < lats[k]);
      ncross = 0;
      for (int i = 0; i < npar[k]; i++)
        ncross += (par[k][i] > 0.0 && par[k][i] < 1.0);
      if (t != 0 && t2 != 0 && lat1 != lats[k] &&
          ((ncross % 2 == 1) != (t != t2)) &&
          fabs(lat1 - lats[k]) <= fabs(lat2 - lats[k]))
        t = -t;
      /* North of the south parallel and south of the north one */
      if (t == 0 || (k == 0 ? t > 0 : t < 0))
        state |= 1 << (2 + k);
      /* A path running along a line lies on it, which its heading states to
       * the last place of the normal of its circle */
      if (along_lon[k])
        state |= 1 << k;
      if (along_lat[k])
        state |= 1 << (2 + k);
    }
  }
  /* A piece between two cuts lies wholly inside the box or wholly outside
   * it; a piece following one that is inside extends its span */
  int nspans = 0;
  for (int i = 0; i + 1 < nuniq; i++)
  {
    bool inside;
    if (narrow)
    {
      if (cuts[i] > 0.0)
        state ^= flips[i];
      inside = (state == 15);
    }
    else
    {
      double lon, lat;
      if (! dggs_arc_point(arc, (cuts[i] + cuts[i + 1]) / 2.0, &lon, &lat))
        continue;
      lon *= 180.0 / M_PI;
      lat *= 180.0 / M_PI;
      for (int k = 0; k < 2; k++)
      {
        if (along_lon[k])
          lon = lons[k];
        if (along_lat[k])
          lat = lats[k];
      }
      /* A box starting at or past the antimeridian holds the longitudes the
       * positions state, of which 180 alone is one: a piece lies there only
       * running along the antimeridian as its endpoints state it, and a
       * position halfway along any other piece reads 180 only to the rounding
       * of its angles */
      if (xmin >= 180.0 && ! along_lon[0])
        inside = false;
      else
        inside = dggs_lonlat_box_holds(lon, lat, xmin, ymin, xmax, ymax);
    }
    if (! inside)
      continue;
    if (nspans > 0 && tout[nspans - 1] == cuts[i])
    {
      tout[nspans - 1] = cuts[i + 1];
      tout_upper[nspans - 1] = upper[i + 1];
    }
    else if (nspans < maxout)
    {
      tin[nspans] = cuts[i]; tin_upper[nspans] = upper[i];
      tout[nspans] = cuts[i + 1]; tout_upper[nspans++] = upper[i + 1];
    }
    else
      break;
  }
  /* A corner no span reaches is held for its instant alone, in the order of
   * the spans */
  for (int c = 0; c < ncorners; c++)
  {
    double tv = corner[c];
    int pos = 0;
    bool held = false;
    for (int i = 0; i < nspans; i++)
    {
      if (tin[i] <= tv && tv <= tout[i])
        held = true;
      if (tin[i] < tv)
        pos = i + 1;
    }
    if (held || nspans >= maxout)
      continue;
    for (int i = nspans; i > pos; i--)
    {
      tin[i] = tin[i - 1]; tin_upper[i] = tin_upper[i - 1];
      tout[i] = tout[i - 1]; tout_upper[i] = tout_upper[i - 1];
    }
    tin[pos] = tout[pos] = tv;
    tin_upper[pos] = tout_upper[pos] = corner_upper[c];
    nspans++;
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
 *
 * The step is the DIFFERENCE OF THE CONVERTED ENDPOINTS, never the conversion
 * of their difference, so the path holds each endpoint exactly at its own
 * parameter, the way #interpolate_point4d reaches the far end of a segment by
 * `A + (B - A) F`. A step converted on its own is rounded apart from the
 * endpoint it is a step to, and the position the path holds at `t = 1` is then
 * not the position the cell of that endpoint is read from. The two part by a
 * last place, which is the whole height a path running up to a cell boundary
 * has left there, so a boundary the segment ENDS on reads as one it crosses.
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
  line->dlon = deg2rad(lon2) - line->lon;
  line->dlat = deg2rad(lat2) - line->lat;
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
 * parameter of the height of a planar path above the plane of normal `m`,
 * measured from a position @p org the plane HOLDS
 * @details The plane passes through @p org, so the height above it is the same
 * quantity read from the centre of the sphere or read from @p org, and the
 * second is the one the doubles carry: measuring a position FROM ONE THE PLANE
 * HOLDS makes the difference the size of the STRETCH between them rather than
 * of the radius they both stand at, which is the cancellation
 * #emit_arc_edge measures its arc centre from a point of the arc to avoid. A
 * path AT @p org then reads a height of exactly zero, since the difference is
 * exactly zero, where a height read from the centre leaves the residue of a
 * dot product of two unit vectors and states a side no vertex of an edge has.
 * @param[in] org Position the plane holds, or NULL for a plane through the
 * centre of the sphere stated by a normal of its own
 */
static void
dggs_line_height(const DggsLine *line, const POINT3D *m, const POINT3D *org,
  double t, double *value, double *slope)
{
  double lon = line->lon + t * line->dlon, lat = line->lat + t * line->dlat;
  double cl = cos(lon), sl = sin(lon), cp = cos(lat), sp = sin(lat);
  POINT3D p = { .x = cp * cl, .y = cp * sl, .z = sp };
  if (org)
  {
    p.x -= org->x; p.y -= org->y; p.z -= org->z;
  }
  POINT3D d = { .x = -line->dlat * sp * cl - line->dlon * cp * sl,
    .y = -line->dlat * sp * sl + line->dlon * cp * cl,
    .z = line->dlat * cp };
  *value = dggs_vec_dot(m, &p);
  *slope = dggs_vec_dot(m, &d);
  return;
}

/** @brief Number of crossings a planar path states in closed form at most */
#define DGGS_MAX_PLANE_ROOTS 8

/**
 * @brief Set `roots` to every parameter in `[0, 1]` at which a planar path
 * meets the plane of normal `m` through the centre, in closed form, and
 * return how many, or -1 when the path states no closed form
 * @details A planar path carries a longitude AND a latitude each linear in
 * the parameter, so its height above a plane holds both angles and is no
 * sinusoid. Where one of the two is CONSTANT the other alone is left, the
 * height is a single sinusoid, and the inverse of a cosine states its zeros
 * exactly:
 *
 * - a path of constant latitude has the height
 *   `cos(φ) R cos(λ − ψ) + m₂ sin(φ)`, with `R = hypot(m₀, m₁)` and
 *   `ψ = atan2(m₁, m₀)`, which is zero at the longitudes
 *   `ψ ± acos(−m₂ tan(φ) / R)`, each of them once a turn;
 * - a path of constant longitude has the height `A cos(φ) + m₂ sin(φ)`, with
 *   `A = m₀ cos(λ) + m₁ sin(λ)`, which is zero at the latitudes
 *   `−atan2(A, m₂)`, once a half turn.
 *
 * A SEARCH along the path steps by a bound on its curvature, and the LENGTH
 * of the path sets that bound: over a long path every step is short and the
 * steps shrink onto a crossing until one reaches zero, where the search
 * states no crossing though the path makes one. The closed form holds however
 * long the path is.
 * @param[in] line Path
 * @param[in] m Unit normal of the plane
 * @param[out] roots Parameters of the crossings, #DGGS_MAX_PLANE_ROOTS at most
 * @return The number of crossings, or -1 when both the longitude and the
 * latitude of the path move and no closed form states them
 */
static int
dggs_line_plane_closed(const DggsLine *line, const double m[3], double *roots)
{
  assert(line); assert(m); assert(roots);
  int n = 0;
  if (line->dlat == 0.0 && line->dlon != 0.0)
  {
    double cl = cos(line->lat), sl = sin(line->lat);
    double r = hypot(m[0], m[1]);
    /* The plane of the equator holds no longitude, and a path at a pole
     * states none: neither carries the sinusoid the closed form reads */
    if (r == 0.0 || cl == 0.0)
      return -1;
    double rhs = -m[2] * sl / (cl * r);
    if (fabs(rhs) > 1.0)
      return 0;                  /* the path never reaches the plane */
    double psi = atan2(m[1], m[0]), half = acos(rhs);
    double lo = (line->dlon > 0.0) ? line->lon : line->lon + line->dlon;
    double hi = (line->dlon > 0.0) ? line->lon + line->dlon : line->lon;
    for (int s = -1; s <= 1; s += 2)
    {
      double base = psi + s * half;
      double kmin = ceil((lo - base) / (2.0 * M_PI));
      double kmax = floor((hi - base) / (2.0 * M_PI));
      for (double k = kmin; k <= kmax && n < DGGS_MAX_PLANE_ROOTS; k += 1.0)
        roots[n++] = (base + 2.0 * M_PI * k - line->lon) / line->dlon;
    }
    return n;
  }
  if (line->dlon == 0.0 && line->dlat != 0.0)
  {
    double a = m[0] * cos(line->lon) + m[1] * sin(line->lon);
    if (hypot(a, m[2]) == 0.0)
      return -1;               /* the path lies in the plane of the edge */
    double base = -atan2(a, m[2]);
    double lo = (line->dlat > 0.0) ? line->lat : line->lat + line->dlat;
    double hi = (line->dlat > 0.0) ? line->lat + line->dlat : line->lat;
    double kmin = ceil((lo - base) / M_PI), kmax = floor((hi - base) / M_PI);
    for (double k = kmin; k <= kmax && n < DGGS_MAX_PLANE_ROOTS; k += 1.0)
      roots[n++] = (base + M_PI * k - line->lat) / line->dlat;
    return n;
  }
  return -1;
}

/**
 * @brief Return the first parameter strictly ahead of `tmin` at which a
 * planar path reaches the plane of normal `m` from its positive side
 * @details The function returns a value above 1 when the path does not reach
 * the plane before its end. The height `f` of the path above the plane has a
 * second derivative of norm at most `M`, the curvature bound of the path, so
 * over a step `h` it stays above `f + f' h - M h² / 2`. The search steps to
 * the first zero of that bound, which the height cannot reach sooner: no
 * crossing is stepped over, however short the stretch the path spends beyond
 * the plane. Near a crossing the steps shrink quadratically onto it.
 */
static double
dggs_line_plane_param(const DggsLine *line, const POINT3D *m,
  const POINT3D *org, double tmin)
{
  double t = tmin, f, d;
  double mm = line->curvature;
  /* Where the path states its crossings in closed form they are read there,
   * as a search along a long path steps too short to reach them */
  const double mv[3] = { m->x, m->y, m->z };
  double roots[DGGS_MAX_PLANE_ROOTS];
  int nroots = dggs_line_plane_closed(line, mv, roots);
  if (nroots >= 0)
  {
    dggs_line_height(line, m, org, tmin, &f, &d);
    if (f <= 0.0 && d <= 0.0)
      return tmin;
    /* The cell lies where the height is positive, so it is LEFT where the
     * path descends through the plane and entered where it rises through it.
     * A path starting outside and heading in meets the plane twice, and the
     * exit is the second of the two: the rate at the crossing tells them
     * apart, as it does in the search below */
    double best = 2.0;
    for (int i = 0; i < nroots; i++)
    {
      if (roots[i] <= tmin || roots[i] > 1.0 || roots[i] >= best)
        continue;
      double rf, rd;
      dggs_line_height(line, m, org, roots[i], &rf, &rd);
      if (rd <= 0.0)
        best = roots[i];
    }
    return best;
  }
  for (int i = 0; i < 1024 && t <= 1.0; i++)
  {
    dggs_line_height(line, m, org, t, &f, &d);
    if (f <= 0.0)
    {
      /* A path at or outside the plane and heading out leaves THERE, at the
       * parameter it already holds, as a line starting on a tile boundary and
       * heading across it leaves the tile at once. The exit is that parameter
       * and never the one after it: a segment beginning exactly on a boundary
       * is the same path as the stretch of a longer segment running through
       * that boundary, so the two state the same cells */
      if (d <= 0.0)
        return t;
      f = 0.0;
    }
    double h = (d + sqrt(d * d + 2.0 * mm * f)) / mm;
    if (h <= 0.0)
      return t;
    t += h;
  }
  return (t > 1.0) ? 2.0 : t;
}

/**
 * @brief Return the first parameter strictly ahead of `tmin` at which the
 * height of a planar path above the plane of normal `m` changes sign
 * @details Return a value above 1 when the sign does not change before the end
 * of the path. The search is the one of #dggs_line_plane_param, read from
 * whichever side of the plane the path lies on at `tmin`.
 */
static double
dggs_line_plane_sign_change(const DggsLine *line, const POINT3D *m,
  const POINT3D *org, double tmin)
{
  double t = tmin, f, d;
  double mm = line->curvature;
  dggs_line_height(line, m, org, t, &f, &d);
  /* A start on the plane lies on the side the path heads to */
  double side = (f > 0.0 || (f == 0.0 && d >= 0.0)) ? 1.0 : -1.0;
  /* Where the path states its crossings in closed form they are read there,
   * as a search along a long path steps too short to reach them */
  const double mv[3] = { m->x, m->y, m->z };
  double roots[DGGS_MAX_PLANE_ROOTS];
  int nroots = dggs_line_plane_closed(line, mv, roots);
  if (nroots >= 0)
  {
    if (side * f <= 0.0 && side * d < 0.0)
      return tmin;
    double best = 2.0;
    for (int i = 0; i < nroots; i++)
      if (roots[i] > tmin && roots[i] <= 1.0 && roots[i] < best)
        best = roots[i];
    return best;
  }
  for (int i = 0; i < 1024 && t <= 1.0; i++)
  {
    double g = side * f, e = side * d;
    /* A path at or beyond the plane leaves THERE, at the parameter it already
     * holds. At `tmin` that reads the height AND its rate: a path sitting on
     * the plane and heading across it leaves at once, while one heading along
     * or back stays, which is the rule #dggs_arc_normals_exit_param applies to
     * the same two quantities */
    if (g <= 0.0 && (t > tmin || e < 0.0))
      return t;
    if (g < 0.0)
      g = 0.0;
    double h = (e + sqrt(e * e + 2.0 * mm * g)) / mm;
    /* A height ABOVE the plane leaves the step a positive length, so a step
     * that reads zero there is one the search has shortened onto the crossing
     * until the height it carries stands a last place above the plane: the
     * sign changes THERE, which is the step #dggs_line_plane_param reads the
     * same way. The step is zero for a path lying IN the plane and not
     * leaving it only where the height is zero as well, and such a path
     * crosses nowhere */
    if (h <= 0.0)
      return (g > 0.0) ? t : 2.0;
    t += h;
    dggs_line_height(line, m, org, t, &f, &d);
  }
  return (t > 1.0) ? 2.0 : t;
}

/**
 * @brief Return where a planar path first crosses an edge of a cell
 * @details A crossing of the circle of an edge counts where it lies between
 * the two vertices of the edge.
 */
static double
dggs_line_exit_param_edges(const DggsLine *line, const double *lons,
  const double *lats, int count, double tmin, uint32 entry, int *edge)
{
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    if (entry & (1u << i))
      continue;
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
      t = dggs_line_plane_sign_change(line, &m, &vi, t);
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
      if (edge)
        *edge = i;
      break;
    }
  }
  return best;
}

/**
 * @brief Return where a planar path leaves a convex cell stated by the plane
 * of each of its edges
 * @details A convex cell is the intersection of the hemispheres its edge
 * circles bound, so a path inside it leaves it where it first reaches any of
 * their planes. The straight line in longitude and latitude is no great
 * circle, so a crossing has no closed form and is searched for along the path
 * by #dggs_line_plane_param.
 *
 * The normals are the planes THEMSELVES, not planes read back from the angles
 * of rounded vertices. A grid stating an edge in closed form passes them here,
 * and the height of a path running along such an edge then reads the exact
 * zero the plane gives rather than the last-place residue a vertex turned into
 * an angle and back into a direction leaves behind.
 * @param[in] line Path
 * @param[in] normals Inward unit normal of each edge plane, three coordinates
 * each, the edge from vertex `i` at `normals[3 * i]`
 * @param[in] count Number of edges
 * @param[in] tmin Parameter the exit lies strictly ahead of
 * @param[in] entry Mask of the edges the path entered the cell through, bit
 * `i` for the edge from vertex `i`, which it never leaves through: a line
 * does not cross back over the tile boundary it has just crossed
 * @param[out] edge When not `NULL`, the edge crossed at the exit, the one
 * from vertex `edge` to the next vertex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_line_normals_exit_param(const DggsLine *line, const double *normals,
  const double *origins, int count, double tmin, uint32 entry, int *edge)
{
  assert(line); assert(normals);
  double best = 2.0;
  for (int i = 0; i < count; i++)
  {
    if (entry & (1u << i))
      continue;
    POINT3D m = { .x = normals[3 * i], .y = normals[3 * i + 1],
      .z = normals[3 * i + 2] };
    if (m.x == 0.0 && m.y == 0.0 && m.z == 0.0)
      continue;
    POINT3D org;
    if (origins)
    {
      org.x = origins[3 * i]; org.y = origins[3 * i + 1];
      org.z = origins[3 * i + 2];
    }
    double t = dggs_line_plane_param(line, &m,
      origins ? &org : NULL, tmin);
    if (t < best)
    {
      best = t;
      if (edge)
        *edge = i;
    }
  }
  return best;
}

/**
 * @brief Return where a planar path leaves a convex cell stated by its
 * vertices
 * @details The plane of each edge is read from the angles of the two vertices
 * it joins and turned inward, and #dggs_line_normals_exit_param states the
 * exit from those normals. A grid holding the plane of an edge in closed form
 * states it exactly and calls that function with its own normals.
 * @param[in] line Path
 * @param[in] lons,lats Vertices of the cell boundary in radians, in the order
 * they join
 * @param[in] count Number of vertices
 * @param[in] tmin Parameter the exit lies strictly ahead of
 * @param[in] convex True when the cell is convex
 * @param[in] entry Mask of the edges the path entered the cell through, bit
 * `i` for the edge from vertex `i`, which it never leaves through: a line
 * does not cross back over the tile boundary it has just crossed
 * @param[out] edge When not `NULL`, the edge crossed at the exit, the one
 * from vertex `edge` to the next vertex
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
double
dggs_line_exit_param(const DggsLine *line, const double *lons,
  const double *lats, int count, double tmin, bool convex, uint32 entry,
  int *edge)
{
  assert(line); assert(lons); assert(lats);
  if (! convex)
    return dggs_line_exit_param_edges(line, lons, lats, count, tmin, entry,
      edge);
  assert(count <= DGGS_MAX_CELL_VERTS);
  double normals[3 * DGGS_MAX_CELL_VERTS];
  double origins[3 * DGGS_MAX_CELL_VERTS];
  dggs_cell_edge_planes(lons, lats, count, normals, origins);
  return dggs_line_normals_exit_param(line, normals, origins, count, tmin,
    entry, edge);
}

/**
 * @brief Return true if a planar path stays inside a convex cell over its
 * whole length
 * @details The height `f` of the path above the plane of an edge has a second
 * derivative of norm at most the curvature bound `M` of the path, which
 * #dggs_line_plane_param steps by. The height therefore departs from the
 * straight line joining its values at the two endpoints by at most
 * `M t (1 - t) / 2`, which is at most `M / 8`, so it stays above
 * `min(f(0), f(1)) - M / 8`. Where that is positive for every edge the path
 * never reaches the boundary of the cell and leaves it nowhere. The heights
 * are read as #dggs_line_height reads them, from the endpoints the path
 * holds at its parameters 0 and 1, and a margin of a few last places of a
 * unit vector covers their rounding: the test only chooses between answering
 * at once and walking the path, and the walk answers the same for a path
 * that stays inside.
 * @param[in] line Path
 * @param[in] normals,origins Planes of the edges of the cell, as
 * #dggs_cell_edge_planes states them
 * @param[in] count Number of edges
 */
bool
dggs_line_stays_in_planes(const DggsLine *line, const double *normals,
  const double *origins, int count)
{
  assert(line); assert(normals); assert(origins);
  double p[2][3];
  for (int k = 0; k < 2; k++)
  {
    double lon = line->lon + k * line->dlon, lat = line->lat + k * line->dlat;
    double cl = cos(lon), sl = sin(lon), cp = cos(lat), sp = sin(lat);
    p[k][0] = cp * cl; p[k][1] = cp * sl; p[k][2] = sp;
  }
  double least = line->curvature / 8.0 + 16.0 * DBL_EPSILON;
  for (int i = 0; i < count; i++)
  {
    const double *m = &normals[3 * i], *o = &origins[3 * i];
    for (int k = 0; k < 2; k++)
    {
      double f = m[0] * (p[k][0] - o[0]) + m[1] * (p[k][1] - o[1]) +
        m[2] * (p[k][2] - o[2]);
      if (f <= least)
        return false;
    }
  }
  return true;
}

/**
 * @brief Fill `normals` with the inward unit normal of the plane of each edge
 * of a convex cell and `origins` with a position each plane holds
 * @details The interior lies on the side of every edge circle the centre of
 * the vertices lies on
 * @param[in] lons,lats Vertices of the cell boundary in radians, in the order
 * they join
 * @param[in] count Number of vertices
 * @param[out] normals,origins Arrays of `3 * count` coordinates
 */
void
dggs_cell_edge_planes(const double *lons, const double *lats, int count,
  double *normals, double *origins)
{
  POINT3D centre = { .x = 0.0, .y = 0.0, .z = 0.0 };
  for (int i = 0; i < count; i++)
  {
    GEOGRAPHIC_POINT g = { .lat = lats[i], .lon = lons[i] };
    POINT3D v;
    geog2cart(&g, &v);
    centre.x += v.x; centre.y += v.y; centre.z += v.z;
  }
  for (int i = 0; i < count; i++)
  {
    int j = (i + 1) % count;
    GEOGRAPHIC_POINT gi = { .lat = lats[i], .lon = lons[i] };
    GEOGRAPHIC_POINT gj = { .lat = lats[j], .lon = lons[j] };
    POINT3D m;
    /* Read from the angles of the vertices, as #dggs_arc_exit_param does */
    robust_cross_product(&gi, &gj, &m);
    if (! (m.x == 0.0 && m.y == 0.0 && m.z == 0.0))
    {
      normalize(&m);
      if (dggs_vec_dot(&m, &centre) < 0.0)
      {
        m.x = -m.x; m.y = -m.y; m.z = -m.z;
      }
    }
    normals[3 * i] = m.x; normals[3 * i + 1] = m.y; normals[3 * i + 2] = m.z;
    /* The plane of an edge HOLDS both of its vertices, and the height of the
     * path is read from the first of them */
    POINT3D vi;
    geog2cart(&gi, &vi);
    origins[3 * i] = vi.x; origins[3 * i + 1] = vi.y; origins[3 * i + 2] = vi.z;
  }
  return;
}

/*****************************************************************************/


/*****************************************************************************
 * Periods a trajectory holds its cells
 *****************************************************************************/

/**
 * @brief Return the step temporal cell of the visits a trajectory makes to
 * its cells, in the order it makes them
 * @details Two consecutive visits meet at the instant of their crossing, and
 * the grid gives that instant to one of them: the one it assigns the crossing
 * position to, which holds it, the other leaving it out. A timestamp holds
 * whole microseconds, so several crossings may fall at one instant. A visit
 * then holding no time, an empty period, is no part of the answer, and where
 * two visits both hold the instant they meet at, the later one holds it, as a
 * value landing on the instant already stated replaces it
 * (#raster_run_push). The periods then partition the time of the trajectory.
 *
 * The answer holds each cell over its period. A step sequence gives each of
 * its instants to the value starting there, so it runs on while every
 * crossing hands its instant to the cell entered there, and a crossing whose
 * instant the cell left holds closes it, as #raster_run_close closes a run:
 * the next sequence starts after that instant.
 * @param[in,out] visits Visits in order, their periods rewritten in place
 * @param[in] count Number of visits
 * @param[in] temptype Temporal cell type of the answer
 * @return A sequence, or a sequence set when a crossing gives its instant to
 * the cell left, NULL when no visit holds any time
 */
Temporal *
dggs_visits_to_temporal(DggsVisit *visits, int count, MeosType temptype)
{
  assert(visits); assert(count > 0);
  /* The visits holding time, an instant two of them hold going to the later */
  int n = 0;
  for (int i = 0; i < count; i++)
  {
    DggsVisit v = visits[i];
    if (v.lower == v.upper && ! (v.lower_inc && v.upper_inc))
      continue;
    while (n > 0 && visits[n - 1].upper == v.lower &&
      visits[n - 1].upper_inc && v.lower_inc)
    {
      visits[n - 1].upper_inc = false;
      if (visits[n - 1].lower < visits[n - 1].upper)
        break;
      n--;
    }
    visits[n++] = v;
  }
  if (n == 0)
    return NULL;

  TSequence **seqs = palloc(sizeof(TSequence *) * (size_t) n);
  TInstant **insts = palloc(sizeof(TInstant *) * (size_t) (n + 1));
  int nseqs = 0, ninsts = 0;
  bool lower_inc = visits[0].lower_inc;
  insts[ninsts++] = tinstant_make(visits[0].cell, temptype, visits[0].lower);
  for (int i = 1; i <= n; i++)
  {
    const DggsVisit *prev = &visits[i - 1];
    /* The cell entered holds the instant it is entered at: the sequence
     * runs on */
    if (i < n && prev->upper == visits[i].lower && ! prev->upper_inc &&
        visits[i].lower_inc)
    {
      insts[ninsts++] = tinstant_make(visits[i].cell, temptype,
        visits[i].lower);
      continue;
    }
    /* The sequence ends on the cell it holds up to the end of that visit */
    if (prev->upper > insts[ninsts - 1]->t)
      insts[ninsts++] = tinstant_make(prev->cell, temptype, prev->upper);
    bool upper_inc = prev->upper_inc;
    if (ninsts == 1)
      lower_inc = upper_inc = true;
    seqs[nseqs++] = tsequence_make_free(insts, ninsts, lower_inc, upper_inc,
      STEP, NORMALIZE);
    if (i == n)
      break;
    insts = palloc(sizeof(TInstant *) * (size_t) (n - i + 1));
    ninsts = 0;
    lower_inc = visits[i].lower_inc;
    insts[ninsts++] = tinstant_make(visits[i].cell, temptype,
      visits[i].lower);
  }
  if (nseqs == 1)
  {
    Temporal *result = (Temporal *) seqs[0];
    pfree(seqs);
    return result;
  }
  return (Temporal *) tsequenceset_make_free(seqs, nseqs, NORMALIZE);
}

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

/**
 * @brief Return the set of cells at a resolution covering a set of cells of a
 * quadtree grid
 * @details The cover holds the cells that hold a point of a cell of the set.
 * A cell of a quadtree grid is exactly the union of its four children, so
 * every point of a cell lies in its ancestor at a coarser resolution, and the
 * cover of a cell there is that ancestor alone; at the cell's own resolution
 * it is the cell. A resolution finer than a cell's own is the uncompaction of
 * the cell, which #dggs_quadtree_uncompact_cells states, so it raises an
 * error here.
 * @param[in] cells Set of cells
 * @param[in] resolution Resolution of the cover
 * @param[in] temptype Temporal cell-index type naming the grid
 */
Set *
dggs_quadtree_cover_cells(const Set *cells, int32 resolution,
  MeosType temptype)
{
  assert(cells);
  const DggsCellOps *ops = dggs_cellops(temptype);
  if (! ops || ! ensure_valid_cell_resolution(temptype, resolution))
    return NULL;
  Datum *datums = palloc(sizeof(Datum) * (size_t) cells->count);
  for (int i = 0; i < cells->count; i++)
  {
    Datum cell = SET_VAL_N(cells, i);
    int32 res = DatumGetInt32(ops->get_resolution(cell));
    if (resolution > res)
    {
      pfree(datums);
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "The cover of a cell at resolution %d, finer than the cell's own %d, is not supported",
        resolution, res);
      return NULL;
    }
    datums[i] = (resolution == res) ? cell :
      ops->cell_to_parent(cell, Int32GetDatum(resolution));
  }
  return set_make_free(datums, cells->count, ops->celltype, ORDER);
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
