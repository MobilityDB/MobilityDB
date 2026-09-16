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
 * @brief Public API: static GSERIALIZED geometry → set of H3 cells.
 *
 * The cover of a geometry is the set of the cells that hold a point of it:
 *
 *   POINT             — the cell holding it.
 *   LINESTRING        — the cells its segments pass through.
 *   POLYGON           — the cells its rings pass through, together with the
 *                       cells whose centre it holds.
 *   MULTIPOINT        — union of per-component POINTs.
 *   MULTILINESTRING   — union of per-component LINESTRINGs.
 *   MULTIPOLYGON      — union of per-component POLYGONs.
 *   GEOMETRYCOLLECTION — recursive union of per-component geometries.
 *
 * The companion `ever_eq_h3indexset_th3index` predicate enables the
 * cross-platform spatial prefilter: a th3index trip path ever-touches a
 * cell in the given set iff (for any common instant) the trip is in one
 * of those cells.
 */

/* C */
#include <math.h>
#include <string.h>
/* MEOS */
#include <postgres.h>
/* H3 */
#include <h3api.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>  /* lwpoly_contains_point, LW_OUTSIDE */
#include <lwgeodetic.h>  /* lwpoly_covers_point2d */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "h3/h3index.h"
#include "geo/tgeo_spatialfuncs.h"  /* ensure_srid_is_latlong */
#include "temporal/set.h"  /* ensure_set_isof_type */
#include "temporal/tcellindex.h"
#include "temporal/temporal.h"  /* ORDER macro for set_make_free */

/*****************************************************************************
 * Growable buffer of H3Index — accumulator for the recursive walker
 *****************************************************************************/

/** @brief Number of cells a cover gathers at most, the count at which a
 * request states a resolution far finer than the geometry it covers */
#define H3_MAX_COVER_CELLS 4194304

/**
 * @brief Growable accumulator of H3 cells filled by the geometry walker
 * @details The limit is enforced as the cells arrive, since a polygon at a
 * resolution far finer than its extent encloses more cells than memory holds
 */
typedef struct h3_buf
{
  H3Index *cells;
  int      count;
  int      capacity;
  bool     overflow;   /**< True once the cover would pass the limit */
  bool     error;      /**< True once a component raised an error */
} h3_buf;

/**
 * @brief Initialize an accumulator with a starting capacity
 */
static void
h3_buf_init(h3_buf *buf, int initial_capacity)
{
  buf->capacity = initial_capacity > 0 ? initial_capacity : 64;
  buf->count    = 0;
  buf->overflow = false;
  buf->error    = false;
  buf->cells    = palloc(sizeof(H3Index) * (size_t) buf->capacity);
}

/**
 * @brief Make room for @p additional further cells in an accumulator
 * @return False, and the accumulator marked as overflowing, when the cells
 * would pass #H3_MAX_COVER_CELLS
 */
static bool
h3_buf_reserve(h3_buf *buf, long additional)
{
  if (buf->overflow)
    return false;
  if ((long) buf->count + additional > H3_MAX_COVER_CELLS)
  {
    buf->overflow = true;
    return false;
  }
  int needed = buf->count + (int) additional;
  if (needed <= buf->capacity)
    return true;
  int new_cap = buf->capacity;
  while (new_cap < needed)
    new_cap *= 2;
  buf->cells = repalloc(buf->cells, sizeof(H3Index) * (size_t) new_cap);
  buf->capacity = new_cap;
  return true;
}

/**
 * @brief Append a cell to an accumulator, ignoring the null cell
 */
static inline void
h3_buf_push(h3_buf *buf, H3Index cell)
{
  if (cell != (H3Index) 0 && h3_buf_reserve(buf, 1))
    buf->cells[buf->count++] = cell;
}

/**
 * @brief Free the cells held by an accumulator and reset it to empty
 */
static void
h3_buf_free(h3_buf *buf)
{
  if (buf->cells != NULL)
    pfree(buf->cells);
  buf->cells = NULL;
  buf->count = 0;
  buf->capacity = 0;
}

/**
 * @brief Return the set of the distinct cells of an accumulator, which is freed
 * @details The set orders its values and drops the duplicates, which the
 * accumulator holds wherever two segments pass through one cell
 */
static Set *
h3_buf_to_set(h3_buf *buf)
{
  if (buf->count == 0)
  {
    h3_buf_free(buf);
    return NULL;
  }
  int count = buf->count;
  Datum *datums = palloc(sizeof(Datum) * (size_t) count);
  for (int i = 0; i < count; i++)
    datums[i] = H3IndexGetDatum(buf->cells[i]);
  h3_buf_free(buf);
  return set_make_free(datums, count, T_H3INDEX, ORDER);
}

/*****************************************************************************
 * Hash set of H3Index — the cells the fill of a polygon has read
 *****************************************************************************/

/**
 * @brief Open-addressing hash set of H3 cells, the null cell marking a free
 * slot
 */
typedef struct
{
  H3Index *slots;
  uint64   mask;       /**< Number of slots minus one, a power of two */
  uint64   count;      /**< Number of cells held */
} h3_cellset;

/**
 * @brief Initialize a hash set with room for @p expected cells
 */
static void
h3_cellset_init(h3_cellset *set, int expected)
{
  uint64 size = 64;
  while (size < (uint64) expected * 2)
    size <<= 1;
  set->slots = palloc0(sizeof(H3Index) * size);
  set->mask  = size - 1;
  set->count = 0;
}

/**
 * @brief Return the slot of a cell in the slots of a hash set: the slot
 * holding it, or the free slot where it belongs
 */
static uint64
h3_cellset_slot(const H3Index *slots, uint64 mask, H3Index cell)
{
  /* The SplitMix64 finalizer spreads the bits neighbouring cells share over
   * the whole word */
  uint64 h = cell;
  h = (h ^ (h >> 30)) * UINT64CONST(0xbf58476d1ce4e5b9);
  h = (h ^ (h >> 27)) * UINT64CONST(0x94d049bb133111eb);
  h ^= h >> 31;
  uint64 i = h & mask;
  while (slots[i] != (H3Index) 0 && slots[i] != cell)
    i = (i + 1) & mask;
  return i;
}

/**
 * @brief Add a cell to a hash set
 * @return True when the set did not hold the cell
 */
static bool
h3_cellset_add(h3_cellset *set, H3Index cell)
{
  uint64 i = h3_cellset_slot(set->slots, set->mask, cell);
  if (set->slots[i] == cell)
    return false;
  set->slots[i] = cell;
  set->count++;
  if (set->count * 2 > set->mask + 1)
  {
    /* Keep the load below one half */
    uint64 size = (set->mask + 1) << 1;
    H3Index *slots = palloc0(sizeof(H3Index) * size);
    for (uint64 k = 0; k <= set->mask; k++)
    {
      if (set->slots[k] != (H3Index) 0)
        slots[h3_cellset_slot(slots, size - 1, set->slots[k])] =
          set->slots[k];
    }
    pfree(set->slots);
    set->slots = slots;
    set->mask  = size - 1;
  }
  return true;
}

/**
 * @brief Free the slots of a hash set
 */
static void
h3_cellset_free(h3_cellset *set)
{
  pfree(set->slots);
  set->slots = NULL;
}

/**
 * @brief Return the cell of a given resolution containing a lat/lng in degrees
 */
H3Index
h3_latlng_deg_to_cell(double lat_deg, double lng_deg, int32 resolution)
{
  LatLng ll = { .lat = degsToRads(lat_deg), .lng = degsToRads(lng_deg) };
  H3Index cell;
  if (latLngToCell(&ll, resolution, &cell) != E_SUCCESS)
    return (H3Index) 0;
  return cell;
}

/**
 * @brief Path a segment follows between its two endpoints
 * @details A planar point moves along the straight line in longitude and
 * latitude, a geodetic one along the great circle through its endpoints, as
 * `pointsegm_interpolate` places a temporal point between two instants.
 */
typedef struct
{
  bool geodetic;          /**< True when the path is a great circle */
  double lon1, lat1;      /**< First endpoint, in degrees */
  double dlon, dlat;      /**< Planar path: the step to the second endpoint */
  DggsLine line;          /**< Planar path: its straight line */
  DggsArc arc;            /**< Geodetic path: its great circle */
} H3SegmentPath;

/**
 * @brief Return the length in degrees of the path between two positions: of
 * the straight line in longitude and latitude for a planar point, of the
 * great-circle arc for a geodetic one
 * @param[in] lon1,lat1,lon2,lat2 Positions in degrees
 * @param[in] geodetic True when the positions are geodetic
 */
double
h3_segment_length_deg(double lon1, double lat1, double lon2, double lat2,
  bool geodetic)
{
  if (! geodetic)
  {
    double dlon = lon2 - lon1, dlat = lat2 - lat1;
    return sqrt(dlon * dlon + dlat * dlat);
  }
  GEOGRAPHIC_POINT g1, g2;
  geographic_point_init(lon1, lat1, &g1);
  geographic_point_init(lon2, lat2, &g2);
  return rad2deg(sphere_distance(&g1, &g2));
}

/**
 * @brief Return in the last argument the path between two positions
 * @return False when the path has no length, since it then leaves no cell
 */
static bool
h3_segment_path_init(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, H3SegmentPath *path)
{
  memset(path, 0, sizeof(H3SegmentPath));
  path->geodetic = geodetic;
  path->lon1 = lon1; path->lat1 = lat1;
  path->dlon = lon2 - lon1; path->dlat = lat2 - lat1;
  if (! geodetic)
    return dggs_line_init(lon1, lat1, lon2, lat2, &path->line);
  return dggs_arc_init(lon1, lat1, lon2, lat2, &path->arc);
}

/**
 * @brief Return the cell holding the position a path reaches at a parameter
 * @details A geodetic position is the point `sphere_project` reaches along
 * the great circle at that fraction of the path's angle, the one
 * `pointsegm_interpolate` answers for a temporal geodetic point.
 */
static H3Index
h3_segment_path_cell(const H3SegmentPath *path, double t, int32 resolution)
{
  if (! path->geodetic)
    return h3_latlng_deg_to_cell(path->lat1 + t * path->dlat,
      path->lon1 + t * path->dlon, resolution);
  double lon, lat;
  if (! dggs_arc_point(&path->arc, t, &lon, &lat))
    return (H3Index) 0;
  LatLng ll = { .lat = lat, .lng = lon };
  H3Index cell;
  if (latLngToCell(&ll, resolution, &cell) != E_SUCCESS)
    return (H3Index) 0;
  return cell;
}

/**
 * @brief Return where a geodetic path leaves the cell holding it
 * @details A cell edge is an arc of a great circle, as the path is, so the
 * circle of each edge meets the circle of the path at two antipodal points,
 * along the intersection of their planes. The exit is the nearest of those
 * points that lies on its edge and strictly ahead of `tmin` on the path; its
 * parameter is the fraction of the path's angle reached there.
 * @return the path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
static double
h3_cell_exit_param_geodetic(H3Index cell, const H3SegmentPath *path,
  double tmin)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts < 3)
    return 2.0;
  double lons[MAX_CELL_BNDRY_VERTS], lats[MAX_CELL_BNDRY_VERTS];
  for (int i = 0; i < bnd.numVerts; i++)
  {
    lons[i] = bnd.verts[i].lng;
    lats[i] = bnd.verts[i].lat;
  }
  return dggs_arc_exit_param(&path->arc, lons, lats, bnd.numVerts, tmin,
    false);
}

/**
 * @brief Return where a planar path leaves the cell holding it
 * @details A cell edge is an arc of a great circle, which the straight line in
 * longitude and latitude of the path is not, so the exit is searched for along
 * the path by #dggs_line_exit_param: the nearest crossing of an edge strictly
 * ahead of `tmin`. A cell spanning two faces of the icosahedron bends at the
 * vertices where it crosses between them, so the crossing counts where it lies
 * on its edge.
 * @return the path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
static double
h3_cell_exit_param_planar(H3Index cell, const H3SegmentPath *path,
  double tmin)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts < 3)
    return 2.0;
  double lons[MAX_CELL_BNDRY_VERTS], lats[MAX_CELL_BNDRY_VERTS];
  for (int i = 0; i < bnd.numVerts; i++)
  {
    lons[i] = bnd.verts[i].lng;
    lats[i] = bnd.verts[i].lat;
  }
  return dggs_line_exit_param(&path->line, lons, lats, bnd.numVerts, tmin,
    false);
}

/**
 * @brief Fill `cells` with every cell the segment crosses, and `enter` with
 * the segment parameter at which it reaches each
 * @details A traversal, not a sampling walk: from the cell in hand the walk
 * leaves through its boundary, and the cell just beyond that crossing is a
 * NEIGHBOUR of it, so no cell between the two can be passed over. A sampling
 * walk has no such property at any spacing, because a segment clips a cell
 * corner over an arbitrarily short chord and every spacing is longer than
 * some chord.
 *
 * The walk follows the path the point moves along: the straight line in
 * longitude and latitude of a planar point, the great circle of a geodetic
 * one. A geodetic crossing is found on the sphere, where a path across the
 * antimeridian or over a pole is an arc like any other.
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] geodetic True when the segment is geodetic
 * @param[in] resolution H3 resolution
 * @param[out] cells,enter Arrays of at least `maxout` entries; `enter[0]` is
 *   always 0, the parameter of the first endpoint
 * @param[in] maxout Capacity of both arrays
 * @return Number of cells written, or 0 when the first lookup fails
 */
int
h3_segment_cells(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, int32 resolution, H3Index *cells, double *enter, int maxout)
{
  assert(cells); assert(enter);
  if (maxout < 1)
    return 0;
  H3Index cur = h3_latlng_deg_to_cell(lat1, lon1, resolution);
  if (cur == (H3Index) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;

  H3SegmentPath path;
  if (! h3_segment_path_init(lon1, lat1, lon2, lat2, geodetic, &path))
    return n;
  /* The length of the path in degrees */
  double seg = geodetic ? rad2deg(path.arc.dist) :
    sqrt(path.dlon * path.dlon + path.dlat * path.dlat);
  /* A nudge past the crossing lands inside the next cell without reaching
   * the one after it: a ten-thousandth of a cell edge is far below the
   * width of any cell and far above the rounding of the crossing itself */
  double edge_m;
  if (getHexagonEdgeLengthAvgM(resolution, &edge_m) != E_SUCCESS)
    edge_m = 1000.0;
  double nudge = (edge_m / 111320.0) * 1e-4 / seg;
  if (nudge <= 0.0 || nudge >= 1.0)
    nudge = 1e-9;

  /* A cell is convex, on the plane and on the sphere, so a path whose far
   * endpoint lies in the same cell as its near one never leaves it and there
   * is no boundary to find. That is the common case wherever the positions
   * are closer together than a cell is wide, and reading the boundary for it
   * costs more than the whole answer is worth */
  if (h3_latlng_deg_to_cell(lat2, lon2, resolution) == cur)
    return n;

  double t = 0.0;
  while (n < maxout)
  {
    double texit = geodetic ? h3_cell_exit_param_geodetic(cur, &path, t) :
      h3_cell_exit_param_planar(cur, &path, t);
    if (texit > 1.0)
      break;                 /* the segment ends inside this cell */
    double tn = texit + nudge;
    H3Index next = (H3Index) 0;
    /* A nudge that lands back in the cell just left says the crossing sits
     * within its own rounding, so widen it rather than stall */
    for (int k = 0; k < 8 && tn < 1.0; k++)
    {
      next = h3_segment_path_cell(&path, tn, resolution);
      if (next != (H3Index) 0 && next != cur)
        break;
      tn += nudge * (double) (1 << k);
      next = (H3Index) 0;
    }
    if (next == (H3Index) 0 || tn >= 1.0)
      break;
    cells[n] = next; enter[n] = texit; n++;
    cur = next;
    t = tn;
  }
  return n;
}


/*****************************************************************************
 * Geometry to cell set
 *
 * The cover of a geometry is the set of the cells that hold a point of it,
 * each point assigned to its cell as #h3_latlng_deg_to_cell assigns it, which
 * is the assignment a temporal H3 cell takes its values from. A line gives
 * the cells its segments pass through, found by the traversal of
 * #h3_segment_cells. A polygon gives the cells its rings pass through together
 * with the cells it encloses: a cell holding no point of a ring holds no
 * boundary point, so its points lie all inside the polygon or all outside it,
 * and the centre of the cell decides which.
 *****************************************************************************/

/**
 * @brief Push the cells a segment passes through into the accumulator
 * @details The traversal writes into arrays that grow until the whole segment
 * fits, and the cell of the far endpoint closes the segment when the
 * traversal stopped short of it, as for an endpoint lying on a cell boundary
 * within the rounding of the crossing
 */
static void
segment_to_cells_into(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, int32 resolution, h3_buf *out)
{
  int maxout = 64, ncells;
  H3Index *cells = palloc(sizeof(H3Index) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  while ((ncells = h3_segment_cells(lon1, lat1, lon2, lat2, geodetic,
      resolution, cells, enter, maxout)) == maxout)
  {
    if (maxout >= H3_MAX_COVER_CELLS)
    {
      out->overflow = true;
      pfree(cells); pfree(enter);
      return;
    }
    maxout *= 2;
    cells = repalloc(cells, sizeof(H3Index) * (size_t) maxout);
    enter = repalloc(enter, sizeof(double) * (size_t) maxout);
  }
  for (int k = 0; k < ncells; k++)
    h3_buf_push(out, cells[k]);
  h3_buf_push(out, h3_latlng_deg_to_cell(lat2, lon2, resolution));
  pfree(cells); pfree(enter);
}

/**
 * @brief Push the cells a point array passes through into the accumulator
 */
static void
pointarray_to_cells_into(const POINTARRAY *pa, bool geodetic,
  int32 resolution, h3_buf *out)
{
  if (pa == NULL || pa->npoints == 0)
    return;
  const POINT2D *p = getPoint2d_cp(pa, 0);
  h3_buf_push(out, h3_latlng_deg_to_cell(p->y, p->x, resolution));
  for (uint32_t i = 0; i + 1 < pa->npoints && ! out->overflow; i++)
  {
    const POINT2D *p0 = getPoint2d_cp(pa, i);
    const POINT2D *p1 = getPoint2d_cp(pa, i + 1);
    segment_to_cells_into(p0->x, p0->y, p1->x, p1->y, geodetic, resolution,
      out);
  }
}

/**
 * @brief Return true if a polygon holds the centre of a cell
 * @details A planar polygon reads its edges as straight lines in longitude
 * and latitude, a geodetic one as arcs of great circles, the paths the walk
 * of its rings follows
 */
static bool
polygon_holds_cell_centre(const LWPOLY *poly, bool geodetic, H3Index cell)
{
  LatLng ll;
  if (cellToLatLng(cell, &ll) != E_SUCCESS)
    return false;
  POINT2D pt;
  pt.x = radsToDegs(ll.lng); pt.y = radsToDegs(ll.lat);
  if (geodetic)
    return lwpoly_covers_point2d(poly, &pt);
  return lwpoly_contains_point(poly, &pt) != LW_OUTSIDE;
}

/**
 * @brief Return true if a hash set holds a cell
 */
static bool
h3_cellset_contains(const h3_cellset *set, H3Index cell)
{
  return set->slots[h3_cellset_slot(set->slots, set->mask, cell)] == cell;
}

/**
 * @brief Growable stack of the cells a fill has still to read
 */
typedef struct
{
  H3Index *cells;
  int      count;
  int      capacity;
} h3_stack;

/**
 * @brief Push a cell onto a stack
 */
static void
h3_stack_push(h3_stack *stack, H3Index cell)
{
  if (stack->count == stack->capacity)
  {
    stack->capacity *= 2;
    stack->cells = repalloc(stack->cells,
      sizeof(H3Index) * (size_t) stack->capacity);
  }
  stack->cells[stack->count++] = cell;
}

/**
 * @brief Return in the last argument the neighbours of a cell, the null cell
 * filling the unused entries
 */
static void
h3_cell_neighbours(H3Index cell, H3Index *neighbours)
{
  memset(neighbours, 0, sizeof(H3Index) * 7);
  if (gridDisk(cell, 1, neighbours) != E_SUCCESS)
    memset(neighbours, 0, sizeof(H3Index) * 7);
}

/**
 * @brief Return true if a cell shares an edge with a cell of a ring
 */
static bool
h3_cell_borders_ring(H3Index cell, const h3_cellset *ring)
{
  H3Index neighbours[7];   /* gridDisk(_, 1) returns at most 7 cells */
  h3_cell_neighbours(cell, neighbours);
  for (int k = 0; k < 7; k++)
  {
    if (neighbours[k] != (H3Index) 0 && neighbours[k] != cell &&
        h3_cellset_contains(ring, neighbours[k]))
      return true;
  }
  return false;
}

/**
 * @brief Push the cells of a polygon into the accumulator
 * @details The rings give the cells their points are assigned to, and the
 * cells holding no point of a ring give the cells the polygon encloses. Two
 * such cells sharing an edge lie on the same side of the boundary: a path
 * between their centres through the two cells passes no point of a ring,
 * since every such point is assigned to a cell of a ring. The cells holding
 * no point of a ring therefore fall into components, each wholly inside the
 * polygon or wholly outside it, and each component borders a cell of a ring.
 * The fill reads the band of the cells bordering a ring cell. The centre of
 * the first cell of a component read in the band decides the component: one
 * inside is taken whole, spreading through the cells holding no ring point,
 * which the rings enclose; one outside is followed along the band alone, so
 * the space outside the polygon is never read beyond it. The cost is one
 * point-in-polygon test per component, and a bounded number of neighbour
 * reads per cell of the cover and of the band.
 */
static void
polygon_to_cells_into(LWPOLY *poly, bool geodetic, int32 resolution,
  h3_buf *out)
{
  if (lwpoly_is_empty(poly))
    return;

  /* (a) The cells the rings pass through */
  int first = out->count;
  for (uint32_t i = 0; i < poly->nrings && ! out->overflow; i++)
    pointarray_to_cells_into(poly->rings[i], geodetic, resolution, out);
  int nbnd = out->count - first;
  if (nbnd == 0 || out->overflow)
    return;

  /* (b) The cells the rings enclose, one component at a time */
  if (geodetic)
    /* The geodetic point-in-polygon test reads the box of the polygon */
    lwgeom_add_bbox(lwpoly_as_lwgeom(poly));
  h3_cellset ring, seen;
  h3_cellset_init(&ring, nbnd);
  h3_cellset_init(&seen, nbnd * 4);
  /* The ring cells are read from a copy, since the fill grows the cells of
   * the accumulator */
  H3Index *bnd = palloc(sizeof(H3Index) * (size_t) nbnd);
  for (int i = 0; i < nbnd; i++)
  {
    bnd[i] = out->cells[first + i];
    h3_cellset_add(&ring, bnd[i]);
  }
  h3_stack stack;
  stack.capacity = 64;
  stack.count = 0;
  stack.cells = palloc(sizeof(H3Index) * (size_t) stack.capacity);

  for (int i = 0; i < nbnd && ! out->overflow; i++)
  {
    H3Index seeds[7];
    h3_cell_neighbours(bnd[i], seeds);
    for (int j = 0; j < 7 && ! out->overflow; j++)
    {
      H3Index seed = seeds[j];
      if (seed == (H3Index) 0 || h3_cellset_contains(&ring, seed) ||
          ! h3_cellset_add(&seen, seed))
        continue;
      bool inside = polygon_holds_cell_centre(poly, geodetic, seed);
      if (inside)
        h3_buf_push(out, seed);
      stack.count = 0;
      h3_stack_push(&stack, seed);
      while (stack.count > 0 && ! out->overflow)
      {
        H3Index neighbours[7];
        h3_cell_neighbours(stack.cells[--stack.count], neighbours);
        for (int k = 0; k < 7; k++)
        {
          H3Index cell = neighbours[k];
          if (cell == (H3Index) 0 || h3_cellset_contains(&ring, cell) ||
              h3_cellset_contains(&seen, cell) ||
              (! inside && ! h3_cell_borders_ring(cell, &ring)))
            continue;
          h3_cellset_add(&seen, cell);
          if (inside)
            h3_buf_push(out, cell);
          h3_stack_push(&stack, cell);
        }
      }
    }
  }
  pfree(bnd);
  pfree(stack.cells);
  h3_cellset_free(&seen);
  h3_cellset_free(&ring);
}

/**
 * @brief Push the cells of a geometry into the accumulator, recursing into
 * its components
 */
static void
lwgeom_to_cells_into(LWGEOM *geom, bool geodetic, int32 resolution,
  h3_buf *out)
{
  if (geom == NULL || lwgeom_is_empty(geom))
    return;
  switch (geom->type)
  {
    case POINTTYPE:
    {
      const POINT2D *p = getPoint2d_cp(((const LWPOINT *) geom)->point, 0);
      h3_buf_push(out, h3_latlng_deg_to_cell(p->y, p->x, resolution));
      break;
    }
    case LINETYPE:
      pointarray_to_cells_into(((const LWLINE *) geom)->points, geodetic,
        resolution, out);
      break;
    case POLYGONTYPE:
      polygon_to_cells_into((LWPOLY *) geom, geodetic, resolution, out);
      break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      LWCOLLECTION *col = (LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms && ! out->overflow && ! out->error;
          i++)
        lwgeom_to_cells_into(col->geoms[i], geodetic, resolution, out);
      break;
    }
    default:
      /* A cover omitting the cells of a component would drop a trajectory
       * the prefilter must keep, so a type the walk does not state is refused
       * rather than read as holding no cell */
      out->error = true;
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "The cover of a geometry of type %s is not supported",
        lwtype_name(geom->type));
      return;
  }
}

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @ingroup meos_h3_conversion
 * @brief Return the set of H3 cells covering a static geometry at the given
 * resolution
 * @details The cover is the set of the cells that hold a point of the
 * geometry, each point assigned to its cell as #geo_to_h3index_cell assigns
 * it: a point gives the cell holding it, a line the cells its segments pass
 * through, and a polygon those of its rings together with the cells they
 * enclose. It is the set a temporal H3 cell of a trajectory takes its values
 * from, so a trajectory sharing a point with the geometry takes a cell of the
 * cover there. MULTI* and GEOMETRYCOLLECTION values give the union of the
 * cells of their components, and any other type (TIN, TRIANGLE, the curve
 * family), alone or inside a collection, is refused, since a cover omitting
 * its cells would drop a trajectory the prefilter must keep. An empty
 * geometry holds no point and gives `NULL`.
 * @param[in] gs Geometry in a lon/lat (SRID 4326) reference system
 * @param[in] resolution H3 resolution
 * @errval NULL
 * @csqlfn #Geo_to_h3indexset()
 */
Set *
geo_to_h3index_set(const GSERIALIZED *gs, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_cell_resolution(T_TH3INDEX, resolution) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(gs)))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom == NULL)
    return NULL;
  /* A geodetic segment follows its great circle, a planar one its straight
   * line in longitude and latitude */
  bool geodetic = FLAGS_GET_GEODETIC(lwgeom->flags);
  h3_buf buf;
  h3_buf_init(&buf, 64);
  lwgeom_to_cells_into(lwgeom, geodetic, resolution, &buf);
  lwgeom_free(lwgeom);
  if (buf.error)
  {
    h3_buf_free(&buf);
    return NULL;
  }
  if (buf.overflow)
  {
    h3_buf_free(&buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The cover of the geometry at resolution %d exceeds %d cells",
      resolution, H3_MAX_COVER_CELLS);
    return NULL;
  }
  return h3_buf_to_set(&buf);
}

/**
 * @ingroup meos_h3_comp
 * @brief Return true if a temporal H3 cell is ever equal to a cell of an H3
 * cell set
 * @details Returns 1 if any cell of @p cells appears among the values of
 * @p th3idx, 0 if none does, and -1 on error. This is the cross-platform
 * spatial prefilter the `eIntersects` SQL wrappers and Spark UDFs consume: it
 * walks the instants of the temporal H3 cell and stops at the first instant
 * the set contains.
 * @param[in] cells The candidate H3 cell set (T_H3INDEX).
 * @param[in] th3idx The th3index temporal value.
 * @csqlfn #Ever_eq_h3indexset_th3index()
 */
int
ever_eq_h3indexset_th3index(const Set *cells, const Temporal *th3idx)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEXSET(cells, -1); VALIDATE_TH3INDEX(th3idx, -1);
  return tcellindex_ever_in_set(th3idx, cells) ? 1 : 0;
}

/*****************************************************************************/