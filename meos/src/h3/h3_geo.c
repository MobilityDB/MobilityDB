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
 * @brief Public API: static GSERIALIZED geometry → set of H3 cells
 * @details The cover of a geometry is the set of the cells that hold a point
 * of it:
 *
 * POINT             — the cell holding it.
 * LINESTRING        — the cells its segments pass through.
 * POLYGON           — the cells its rings pass through, together with the
 * cells whose centre it holds.
 * MULTIPOINT        — union of per-component POINTs.
 * MULTILINESTRING   — union of per-component LINESTRINGs.
 * MULTIPOLYGON      — union of per-component POLYGONs.
 * GEOMETRYCOLLECTION — recursive union of per-component geometries.
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
#include "h3/th3index_internal.h"  /* DggsOutline */
#include "geo/geo_funcs.h"  /* cross_product_sign_offset, ExactSum */
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
 * @brief Return true if a boundary of the library holds a vertex, as
 * #h3_edge_holds_vertex reads it
 */
static bool
h3_boundary_holds(const CellBoundary *bnd, const LatLng *vertex)
{
  for (int k = 0; k < bnd->numVerts; k++)
    if (memcmp(&bnd->verts[k], vertex, sizeof(LatLng)) == 0)
      return true;
  return false;
}

/**
 * @brief Return the direction of a directed edge from its origin, a digit
 * of the IJK axes of H3 (1 to 6)
 * @details The H3 index format holds it in the three mode-dependent bits 56
 * to 58 of a directed edge (https://h3geo.org/docs/library/index/directededge)
 */
#define H3_EDGE_DIRECTION(edge) ((int) (((edge) >> 56) & 7))

/** @brief Directions of the edges of a hexagon, in the order of its
 * vertices up to a rotation: IJ, J, JK, K, IK, I */
static const int H3_HEX_DIRECTIONS[6] = { 6, 2, 3, 1, 5, 4 };
/** @brief Directions of the edges of a pentagon, which has no K axis, in the
 * order of its vertices up to a rotation: IJ, J, JK, IK, I */
static const int H3_PENT_DIRECTIONS[5] = { 6, 2, 3, 5, 4 };

/**
 * @brief Fill the outline of a cell bent across no face of the icosahedron
 * and return true, or return false for a bent cell
 * @details A cell whose boundary in the library (`cellToBoundary`) holds no
 * more vertices than the cell has edges crosses no face of the icosahedron,
 * so its edges bend at no extra vertex, and that boundary lists the vertices
 * of the vertex index in their order (both measured over every cell of the
 * resolutions 0 to 5). The edges of the cell take the directions of the IJK
 * axes in one cyclic order of its vertices, #H3_HEX_DIRECTIONS or
 * #H3_PENT_DIRECTIONS, from a vertex that depends on the cell: H3 rotates
 * that order by the orientation of the face of the cell (`vertexRotations`
 * of its vertex.c). One directed edge, the one whose boundary holds the k-th
 * and the next vertex of the boundary, places the order, and the cell across
 * each edge is the destination of the directed edge of its direction, so no
 * vertex of a neighbour is read and one directed edge is read instead of
 * every one.
 * @param[in] cell Cell
 * @param[in] verts,n Its vertices in the vertex index and their number
 * @param[out] out Outline, its vertices and the cells across its edges
 */
static bool
h3_cell_outline_straight(H3Index cell, const H3Index *verts, int n,
  DggsOutline *out)
{
  CellBoundary bnd;
  H3Index edges[6] = { 0 };
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts != n ||
      originToDirectedEdges(cell, edges) != E_SUCCESS)
    return false;
  const int *dirs = n == 5 ? H3_PENT_DIRECTIONS : H3_HEX_DIRECTIONS;
  /* The destination of the directed edge in each direction */
  H3Index dest[7] = { 0 };
  int first = -1;
  for (int j = 0; j < 6; j++)
  {
    H3Index ends[2];
    if (edges[j] == (H3Index) 0)
      continue;
    if (directedEdgeToCells(edges[j], ends) != E_SUCCESS)
      return false;
    dest[H3_EDGE_DIRECTION(edges[j])] = ends[1];
    if (first < 0)
      first = j;
  }
  if (first < 0)
    return false;
  /* The vertex k0 starting the edge of the first directed edge, and the
   * place p0 of its direction in the order */
  CellBoundary eb;
  if (directedEdgeToBoundary(edges[first], &eb) != E_SUCCESS ||
      eb.numVerts != 2)
    return false;
  int k0 = -1, p0 = -1, d0 = H3_EDGE_DIRECTION(edges[first]);
  for (int k = 0; k < n && k0 < 0; k++)
    if (h3_boundary_holds(&eb, &bnd.verts[k]) &&
        h3_boundary_holds(&eb, &bnd.verts[(k + 1) % n]))
      k0 = k;
  for (int i = 0; i < n; i++)
    if (dirs[i] == d0)
      p0 = i;
  if (k0 < 0 || p0 < 0)
    return false;
  for (int k = 0; k < n; k++)
  {
    LatLng v;
    if (vertexToLatLng(verts[k], &v) != E_SUCCESS)
      return false;
    out->lon[k] = radsToDegs(v.lng);
    out->lat[k] = radsToDegs(v.lat);
    out->across[k] = dest[dirs[(p0 + k - k0 + n) % n]];
    if (! out->across[k])
      return false;
  }
  out->count = n;
  return true;
}

/**
 * @brief Return in the last argument the canonical outline of a cell
 * @details Two neighbouring cells must state the vertices they share with the
 * same doubles, so that a boundary point belongs to exactly one of them. The
 * boundary of the library, `cellToBoundary`, does not: neighbours
 * state a shared vertex with different doubles in 168 of the 360 pairs at
 * resolution 0 and 16318 of the 6050520 at resolution 5. The outline reads
 * each vertex once, from the vertex index (`cellToVertexes`,
 * `vertexToLatLng`), in degrees by `radsToDegs`. An edge crossing a face of
 * the icosahedron bends at an extra vertex, which the vertex index does not
 * name; it is read from the directed edge leaving the lower of the two cells
 * sharing the edge (`directedEdgeToBoundary`), so both cells read it from the
 * same edge. Each edge of the outline records the cell across it, and the
 * outline records whether the cell lies around a pole, where it is the
 * region between its ring and the pole. A cell bent across no face reads the
 * cells across its edges from its own directed edges
 * (#h3_cell_outline_straight).
 * @param[in] cell Cell
 * @param[out] out Outline, its vertices in the order of the vertex index
 * @return False on an error of the H3 library
 */
bool
h3_cell_outline(H3Index cell, DggsOutline *out)
{
  H3Index verts[6] = { 0 };
  if (cellToVertexes(cell, verts) != E_SUCCESS)
    return false;
  H3Index north, south;
  h3index_pole_cells(getResolution(cell), &north, &south);
  out->cell = cell;
  out->pole = cell == north ? 1 : (cell == south ? -1 : 0);
  if (h3_cell_outline_straight(cell, verts, isPentagon(cell) ? 5 : 6, out))
    return true;
  H3Index nbrs[7] = { 0 };
  if (gridDisk(cell, 1, nbrs) != E_SUCCESS)
    return false;
  /* The vertices of each neighbour, read once */
  H3Index nverts[7][6] = { { 0 } };
  for (int j = 0; j < 7; j++)
    if (nbrs[j] != 0 && nbrs[j] != cell &&
        cellToVertexes(nbrs[j], nverts[j]) != E_SUCCESS)
      return false;
  int n = isPentagon(cell) ? 5 : 6;
  out->count = 0;
  for (int k = 0; k < n; k++)
  {
    LatLng v;
    if (vertexToLatLng(verts[k], &v) != E_SUCCESS)
      return false;
    /* The neighbour sharing the vertices k and k + 1 */
    H3Index next = verts[(k + 1) % n], across = 0;
    for (int j = 0; j < 7 && ! across; j++)
    {
      if (nbrs[j] == 0 || nbrs[j] == cell)
        continue;
      const H3Index *nv = nverts[j];
      int shared = 0;
      for (int m = 0; m < 6; m++)
        if (nv[m] != 0 && (nv[m] == verts[k] || nv[m] == next))
          shared++;
      if (shared == 2)
        across = nbrs[j];
    }
    if (! across)
      return false;
    out->lon[out->count] = radsToDegs(v.lng);
    out->lat[out->count] = radsToDegs(v.lat);
    out->across[out->count++] = across;
    /* The extra vertex of the edge, from the directed edge leaving the lower
     * of the two cells */
    H3Index edge;
    CellBoundary eb;
    if (cellsToDirectedEdge(cell < across ? cell : across,
          cell < across ? across : cell, &edge) != E_SUCCESS ||
        directedEdgeToBoundary(edge, &eb) != E_SUCCESS)
      return false;
    if (eb.numVerts == 3)
    {
      out->lon[out->count] = radsToDegs(eb.verts[1].lng);
      out->lat[out->count] = radsToDegs(eb.verts[1].lat);
      out->across[out->count++] = across;
    }
    else if (eb.numVerts != 2)
      return false;
  }
  return true;
}

/**
 * @brief Return 1 if the cell left of the edge from A to B lies south of it,
 * or east of it where it runs north-south, and 0 otherwise
 * @details The sign of the longitude difference, then of the latitude
 * difference, each exact, B read in the chart of A
 */
static int
h3_edge_south_side(double ax, double ay, double bx, double by)
{
  int dx = offset_x_cmp(bx, dggs_lon_offset(ax, bx), ax);
  if (dx != 0)
    return dx < 0;
  return by < ay;
}

/**
 * @brief Return true if a cell holds the edge of its outline from the vertex
 * k to the vertex k + 1
 * @details A cell holds its north and west sides: an edge belongs to the cell
 * south of it, and to the cell east of it where it runs north-south. The
 * outline turns counter-clockwise, so the cell lies left of each edge, and
 * the edge is a north side when it runs west. Each sign is exact on the
 * doubles of the outline, which two neighbours share, so exactly one of the
 * two cells holds the edge.
 * @param[in] o Outline of the cell
 * @param[in] k Index of the first vertex of the edge
 */
bool
h3_outline_edge_held(const DggsOutline *o, int k)
{
  int k1 = (k + 1) % o->count;
  return h3_edge_south_side(o->lon[k], o->lat[k], o->lon[k1], o->lat[k1]);
}

/**
 * @brief Return true if a cell holds the vertex k of its outline
 * @details A vertex belongs to the cell whose corner there opens south, or
 * east where one of its edges runs due south. A convex corner opens south
 * when the cell lies south of both its edges, a reflex one when it lies
 * south of either; the turn of the corner is #cross_product_sign_offset on
 * the neighbouring vertices read in the chart of the vertex, so no shifted
 * longitude is rounded.
 * @param[in] o Outline of the cell
 * @param[in] k Index of the vertex
 */
bool
h3_outline_vertex_held(const DggsOutline *o, int k)
{
  int kp = (k + o->count - 1) % o->count, kn = (k + 1) % o->count;
  double x = o->lon[k], y = o->lat[k];
  double px = o->lon[kp], py = o->lat[kp];
  double nx = o->lon[kn], ny = o->lat[kn];
  int in = h3_edge_south_side(px, py, x, y);
  int out = h3_edge_south_side(x, y, nx, ny);
  /* (here - prev) x (next - here), prev and next read in the chart of here */
  int turn = cross_product_sign_offset(px, py, x, y, x, y, nx, ny,
    - dggs_lon_offset(x, px), dggs_lon_offset(x, nx));
  return turn > 0 ? (in && out) : (in || out);
}

/**
 * @brief Return in @p v the geodetic position of degrees, the vector of
 * decision D6 of the cell walks (#dggs_lonlat_vector)
 */
static void
h3_lonlat_vector(double lon, double lat, double *v)
{
  dggs_lonlat_vector(lon, lat, v);
}

/**
 * @brief Return in @p v the geodetic position of the vertex k of an outline,
 * the vector of decision D6 of the cell walks (#h3_lonlat_vector)
 */
static void
h3_outline_vector(const DggsOutline *o, int k, POINT3D *v)
{
  double d[3];
  h3_lonlat_vector(o->lon[k], o->lat[k], d);
  v->x = d[0];
  v->y = d[1];
  v->z = d[2];
}

/**
 * @brief Return true if the cell left of the great-circle arc from @p a to
 * @p b lies south of it, or east of it where the arc runs north-south
 * @details The cell lies on the side the normal a x b points to, so it lies
 * south of the arc when the z component of the normal, ax by - ay bx, is
 * negative, an exact sign of the doubles of the two vectors; where it is 0
 * the arc lies in a meridian plane, and the cell left of it is east of it
 * when the arc runs south
 */
static bool
h3_arc_south_side(const POINT3D *a, const POINT3D *b)
{
  int nz = cross_product_sign_exact(0.0, 0.0, a->x, a->y, 0.0, 0.0, b->x,
    b->y);
  if (nz != 0)
    return nz < 0;
  return b->z < a->z;
}

/**
 * @brief Return true if a cell holds the edge of its outline from the vertex
 * k to the vertex k + 1, the edge read as a great-circle arc
 * @details The geodetic twin of #h3_outline_edge_held: a cell holds its north
 * and west sides, each sign exact on the geodetic positions of the vertices,
 * which two neighbours read from the same doubles, so exactly one of the two
 * cells holds the edge
 * @param[in] o Outline of the cell
 * @param[in] k Index of the first vertex of the edge
 */
bool
h3_outline_edge_held_geodetic(const DggsOutline *o, int k)
{
  POINT3D a, b;
  h3_outline_vector(o, k, &a);
  h3_outline_vector(o, (k + 1) % o->count, &b);
  return h3_arc_south_side(&a, &b);
}

/**
 * @brief Return true if a cell holds the vertex k of its outline, its edges
 * read as great-circle arcs
 * @details The geodetic twin of #h3_outline_vertex_held: the vertex belongs
 * to the cell whose corner there opens south, or east where one of its edges
 * runs due south; the turn of the corner is the sign of the triple product of
 * the previous, the current and the next vertex (#triple_product_sign_exact),
 * positive at a convex corner of an outline turning counter-clockwise
 * @param[in] o Outline of the cell
 * @param[in] k Index of the vertex
 */
bool
h3_outline_vertex_held_geodetic(const DggsOutline *o, int k)
{
  POINT3D p, v, n;
  h3_outline_vector(o, (k + o->count - 1) % o->count, &p);
  h3_outline_vector(o, k, &v);
  h3_outline_vector(o, (k + 1) % o->count, &n);
  bool in = h3_arc_south_side(&p, &v);
  bool out = h3_arc_south_side(&v, &n);
  int turn = triple_product_sign_exact(&p, &v, &n);
  return turn > 0 ? (in && out) : (in || out);
}

static const DggsGeodeticOps h3_geodetic_ops;

/**
 * @brief Return true if a geodetic cell holds a direction, its boundary
 * points owned by the rule of H3 (#dggs_outline_holds_dir)
 * @param[in] o Outline of the cell
 * @param[in] p Direction, exact
 */
static bool
h3_outline_holds_dir(const DggsOutline *o, const SphereDir *p)
{
  return dggs_outline_holds_dir(&h3_geodetic_ops, o, p);
}

/** @brief Outline of the last cell #h3_cell_outline_last read, per thread */
static MEOS_TLS DggsOutline h3_last_outline;
/** @brief Cell of #h3_last_outline, or 0 when none */
static MEOS_TLS H3Index h3_last_outline_cell = 0;

/**
 * @brief Return the outline of a cell, or NULL on an error of the H3
 * library
 * @details The outline is a function of the cell alone, so the one of the
 * last cell asked for answers again for the same cell: consecutive positions
 * of a trajectory mostly lie in one cell, and reading an outline costs more
 * than the rest of the answer for such a position
 */
static const DggsOutline *
h3_cell_outline_last(H3Index cell)
{
  if (h3_last_outline_cell != cell)
  {
    h3_last_outline_cell = 0;
    if (! h3_cell_outline(cell, &h3_last_outline))
      return NULL;
    h3_last_outline_cell = cell;
  }
  return &h3_last_outline;
}

/** @brief The outline of a cell for the planar walk, #h3_cell_outline_last */
static const DggsOutline *
h3_planar_outline(uint64 cell)
{
  return h3_cell_outline_last((H3Index) cell);
}

/** @brief The planar cell of a position for the planar walk,
 * #h3_point_cell */
static uint64
h3_planar_point_cell(double lon, double lat, int32 resolution)
{
  return h3_point_cell(lon, lat, false, resolution);
}

StaticAssertDecl(DGGS_OUTLINE_MAX >= MAX_CELL_BNDRY_VERTS,
  "an outline holds every vertex of an H3 cell boundary");

/** @brief The planar walk of H3: its outlines and its owner rule, a cell
 * holding its north and west sides (#h3_outline_edge_held,
 * #h3_outline_vertex_held) */
static const DggsPlanarOps h3_planar_ops =
{
  h3_planar_outline,
  h3_planar_point_cell,
  h3_outline_edge_held,
  h3_outline_vertex_held,
  NULL
};

/**
 * @brief Return true if a cell holds a position of the plane of longitude
 * and latitude, its boundary points owned by the rule of H3
 * (#dggs_outline_holds_point)
 * @param[in] o Outline of the cell
 * @param[in] x,y Longitude and latitude of the position, in degrees
 */
bool
h3_outline_holds_point(const DggsOutline *o, double x, double y)
{
  return dggs_outline_holds_point(&h3_planar_ops, o, x, y);
}

/**
 * @brief Return true if a cell holds a position, by #h3_outline_holds_point
 * on the outline #h3_cell_outline_last reads
 */
static bool
h3_cell_holds_point(H3Index cell, double x, double y)
{
  const DggsOutline *o = h3_cell_outline_last(cell);
  return o && h3_outline_holds_point(o, x, y);
}

/**
 * @brief Return true if a geodetic cell holds a direction, by
 * #h3_outline_holds_dir on the outline #h3_cell_outline_last reads
 */
static bool
h3_cell_holds_dir(H3Index cell, const SphereDir *p)
{
  const DggsOutline *o = h3_cell_outline_last(cell);
  return o && h3_outline_holds_dir(o, p);
}

static bool h3_outline_convex_geodetic(const DggsOutline *o);

/**
 * @brief Return true if a geodetic cell holds a geodetic position
 * @details A convex cell holds a position strictly left of every edge of
 * its ring and none strictly right of one, each side the exact sign of the
 * triple product of the vertices of the edge and the position
 * (#triple_product_sign_exact); a position on the circle of an edge, or a
 * cell that is not convex, is read by #h3_outline_holds_dir, which answers
 * the same on every position.
 */
static bool
h3_cell_holds_vec(H3Index cell, const double *p)
{
  const DggsOutline *o = h3_cell_outline_last(cell);
  if (! o)
    return false;
  if (h3_outline_convex_geodetic(o))
  {
    POINT3D q = { p[0], p[1], p[2] }, a, b;
    bool zero = false;
    int n = o->count;
    h3_outline_vector(o, 0, &a);
    for (int k = 0; k < n; k++)
    {
      h3_outline_vector(o, (k + 1) % n, &b);
      int s = triple_product_sign_exact(&a, &b, &q);
      if (s < 0)
        return false;
      zero |= (s == 0);
      a = b;
    }
    if (! zero)
      return true;
  }
  SphereDir *d = sphere_dir_make(p);
  bool result = h3_outline_holds_dir(o, d);
  sphere_dir_free(d);
  return result;
}

/*****************************************************************************
 * The meeting of a geodetic cell with a box of longitudes and latitudes
 *****************************************************************************/

/** @brief Number of spans of an edge of a cell in a box asked for, more than
 * a great-circle arc shorter than half a turn has */
#define H3_BOX_SPANS 4

/**
 * @brief Return true if a geodetic cell holds the position of degrees, read
 * as its geodetic position (#h3_cell_holds_vec)
 */
static bool
h3_cell_holds_lonlat(H3Index cell, double lon, double lat)
{
  double v[3];
  h3_lonlat_vector(lon, lat, v);
  return h3_cell_holds_vec(cell, v);
}

/**
 * @brief Return true if a geodetic cell meets a box of no width or no height
 * @details The box is a piece of a meridian or of a parallel. The edges of the
 * outline cross it at positions that cut it into pieces lying wholly inside
 * the cell or wholly outside it, each crossing read where
 * #dggs_arc_meridian_params or #dggs_arc_parallel_params states it, as the
 * restriction of a geodetic path to a box reads it. The ends of the piece, the
 * crossings and a position halfway between each two consecutive ones, each
 * read as a geodetic position the cell holds or not, say whether the cell
 * holds a position of the box.
 * @param[in] cell Cell
 * @param[in] o Outline of the cell
 * @param[in] xmin,ymin,xmax,ymax Bounds of the box, in degrees, with
 * `xmin == xmax` or `ymin == ymax`
 */
static bool
h3_outline_meets_line(H3Index cell, const DggsOutline *o, double xmin,
  double ymin, double xmax, double ymax)
{
  bool meridian = (xmin == xmax);
  double lo = meridian ? ymin : xmin, hi = meridian ? ymax : xmax;
  double cut[2 + 2 * MAX_CELL_BNDRY_VERTS];
  int ncut = 0, n = o->count;
  cut[ncut++] = lo;
  cut[ncut++] = hi;
  for (int k = 0; k < n; k++)
  {
    DggsArc arc;
    int k1 = (k + 1) % n;
    if (! dggs_arc_init(o->lon[k], o->lat[k], o->lon[k1], o->lat[k1], &arc))
      continue;
    double params[2];
    int np = meridian ? dggs_arc_meridian_params(&arc, xmin, params) :
      dggs_arc_parallel_params(&arc, ymin, params);
    for (int i = 0; i < np; i++)
    {
      double lon, lat;
      if (! dggs_arc_point(&arc, params[i], &lon, &lat))
        continue;
      /* The plane of a meridian holds the meridian half a turn from it too */
      if (meridian && cos(lon - xmin * M_PI / 180.0) <= 0.0)
        continue;
      double c = (meridian ? lat : lon) * 180.0 / M_PI;
      if (c > lo && c < hi)
        cut[ncut++] = c;
    }
  }
  /* In ascending order along the piece */
  for (int i = 1; i < ncut; i++)
  {
    double v = cut[i];
    int j = i - 1;
    while (j >= 0 && cut[j] > v)
    {
      cut[j + 1] = cut[j];
      j--;
    }
    cut[j + 1] = v;
  }
  for (int i = 0; i < ncut; i++)
  {
    double c = cut[i];
    if (h3_cell_holds_lonlat(cell, meridian ? xmin : c, meridian ? c : ymin))
      return true;
    if (i > 0 && cut[i - 1] < c)
    {
      double m = (cut[i - 1] + c) / 2.0;
      if (h3_cell_holds_lonlat(cell, meridian ? xmin : m, meridian ? m : ymin))
        return true;
    }
  }
  return false;
}

/**
 * @brief Return the side of an edge of an outline a box lying along it is on
 * @details The edge runs along a meridian or the equator bounding the box, and
 * the cell lies on the side the normal a x b of its vertices points to, as
 * #h3_arc_south_side reads it. The function returns 1 when the cell lies on
 * the side of the box, -1 when it lies on the other side, and 0 when the edge
 * does not run along a bound of the box.
 */
static int
h3_outline_edge_box_side(const DggsOutline *o, int k, double xmin, double ymin,
  double xmax, double ymax)
{
  int k1 = (k + 1) % o->count;
  POINT3D a, b, d = { 0.0, 0.0, 0.0 };
  if (o->lon[k] == o->lon[k1] && (o->lon[k] == xmin || o->lon[k] == xmax))
  {
    /* East of the meridian from the west bound, west of it from the east */
    double x = o->lon[k] * M_PI / 180.0;
    double sign = (o->lon[k] == xmin) ? 1.0 : -1.0;
    d.x = -sign * sin(x);
    d.y = sign * cos(x);
  }
  else if (o->lat[k] == 0.0 && o->lat[k1] == 0.0 &&
      (ymin == 0.0 || ymax == 0.0))
    /* North of the equator from the south bound, south of it from the north */
    d.z = (ymin == 0.0) ? 1.0 : -1.0;
  else
    return 0;
  h3_outline_vector(o, k, &a);
  h3_outline_vector(o, k1, &b);
  return triple_product_sign_exact(&a, &b, &d) > 0 ? 1 : -1;
}

/**
 * @brief Return true if a geodetic H3 cell shares a position with a geodetic
 * box
 * @details The cell is the region its outline bounds with the boundary points
 * it owns, as the cell walks read it (#h3_cell_outline). The box holds the
 * positions whose longitude and latitude lie within its bounds, closed, or
 * open at their upper ends when @p border_inc is false. Each part of the
 * question is decided as MEOS decides it for a position or a path:
 * - a vertex of the outline lies in the box by its degrees, as a position
 *   does, and the cell meets the box there when the vertex lies inside the
 *   bounds or the cell owns it;
 * - an edge enters the box where #dggs_arc_lonlat_box_spans states, as a
 *   geodetic path along that arc does: a piece of some length holds positions
 *   of the cell's interior in the box, unless the edge runs along a meridian
 *   or the equator bounding the box with the cell on the other side; a single
 *   position counts when the cell owns the edge;
 * - otherwise the box lies inside the cell or apart from it, and its middle
 *   position says which, read as a geodetic position (#h3_cell_holds_vec).
 * The parallels of a box are not great circles, and they are read through the
 * trigonometry of the restriction of a path, so the answer agrees with the
 * restriction of a temporal geography point travelling along the edges of the
 * cell. The QUADBIN twin is #quadbin_cell_meets_box.
 * @param[in] cell Cell
 * @param[in] box Box, geodetic
 * @param[in] border_inc True when the box contains its upper bounds
 */
bool
h3_cell_meets_box(H3Index cell, const STBox *box, bool border_inc)
{
  assert(box);
  double xmin = box->xmin, ymin = box->ymin, xmax = box->xmax,
    ymax = box->ymax;
  /* A box of no width or no height holds no position when it leaves out its
   * upper bounds */
  if (! border_inc && (xmin == xmax || ymin == ymax))
    return false;
  const DggsOutline *o = h3_cell_outline_last(cell);
  if (! o)
    return false;
  if (xmin == xmax && ymin == ymax)
    return h3_cell_holds_lonlat(cell, xmin, ymin);
  if (xmin == xmax || ymin == ymax)
    return h3_outline_meets_line(cell, o, xmin, ymin, xmax, ymax);
  int n = o->count;
  /* A vertex in the box */
  for (int k = 0; k < n; k++)
  {
    double lon = o->lon[k], lat = o->lat[k];
    if (lon < xmin || lat < ymin || (border_inc ?
          (lon > xmax || lat > ymax) : (lon >= xmax || lat >= ymax)))
      continue;
    if ((lon > xmin && lon < xmax && lat > ymin && lat < ymax) ||
        h3_outline_vertex_held_geodetic(o, k))
      return true;
  }
  /* An edge entering the box */
  for (int k = 0; k < n; k++)
  {
    DggsArc arc;
    int k1 = (k + 1) % n;
    if (! dggs_arc_init(o->lon[k], o->lat[k], o->lon[k1], o->lat[k1], &arc))
      continue;
    double tin[H3_BOX_SPANS], tout[H3_BOX_SPANS];
    bool tin_upper[H3_BOX_SPANS], tout_upper[H3_BOX_SPANS];
    int count = dggs_arc_lonlat_box_spans(&arc, xmin, ymin, xmax, ymax,
      border_inc, tin, tout, tin_upper, tout_upper, H3_BOX_SPANS);
    for (int i = 0; i < count; i++)
    {
      if (tout[i] > tin[i])
      {
        if (h3_outline_edge_box_side(o, k, xmin, ymin, xmax, ymax) >= 0 ||
            h3_outline_edge_held_geodetic(o, k))
          return true;
      }
      else if (tin[i] > 0.0 && tin[i] < 1.0 &&
          h3_outline_edge_held_geodetic(o, k))
        return true;
    }
  }
  /* The box inside the cell */
  return h3_cell_holds_lonlat(cell, (xmin + xmax) / 2.0, (ymin + ymax) / 2.0);
}

/** @brief Number of rings around the cell of the library a position is
 * looked for in */
#define H3_POINT_CELL_RINGS 16

/**
 * @brief Return the cell holding a position, looked for from an estimate,
 * or 0 when none is found
 * @details The estimate is asked first, then the cells around it, ring by
 * ring, until one holds the position: the direction @p p when given
 * (#h3_cell_holds_dir), else the geodetic position @p v when given
 * (#h3_cell_holds_vec), else the planar position (@p lon, @p lat)
 * (#h3_cell_holds_point)
 */
static H3Index
h3_cell_around(H3Index c0, double lon, double lat, const double *v,
  const SphereDir *p)
{
#define H3_HOLDS(h) (p ? h3_cell_holds_dir((h), p) : \
  v ? h3_cell_holds_vec((h), v) : h3_cell_holds_point((h), lon, lat))
  if (c0 == (H3Index) 0 || H3_HOLDS(c0))
    return c0;
  int64_t size;
  if (maxGridDiskSize(H3_POINT_CELL_RINGS, &size) != E_SUCCESS)
    return (H3Index) 0;
  H3Index *cells = palloc(sizeof(H3Index) * (size_t) size);
  int *dist = palloc(sizeof(int) * (size_t) size);
  H3Index result = (H3Index) 0;
  for (int k = 1; k <= H3_POINT_CELL_RINGS && ! result; k++)
  {
    int64_t sz;
    if (maxGridDiskSize(k, &sz) != E_SUCCESS)
      break;
    memset(cells, 0, sizeof(H3Index) * (size_t) sz);
    if (gridDiskDistances(c0, k, cells, dist) != E_SUCCESS)
      break;
    for (int64_t i = 0; i < sz && ! result; i++)
      if (cells[i] != (H3Index) 0 && dist[i] == k && H3_HOLDS(cells[i]))
        result = cells[i];
  }
  pfree(cells); pfree(dist);
  return result;
#undef H3_HOLDS
}

/**
 * @brief Return the cell of a given resolution holding a position, or 0
 * when none is found
 * @details A cell is its outline in the model of the position: a planar
 * cell its straight-edged outline (#h3_outline_holds_point), which departs
 * from the cell of the library, whose edges are not straight in longitude
 * and latitude; a geodetic cell the region of the sphere its outline bounds
 * with great-circle arcs (#h3_outline_holds_dir), the position read as its
 * geodetic vector (#lonlat_vector_nearest), which the library, reading its
 * own trigonometry in doubles, places in a neighbour near an edge. The cell
 * of the library (#h3_latlng_deg_to_cell) is the estimate, and the cells
 * around it, ring by ring, are asked in turn until one holds the position
 * (#h3_cell_around). Near a pole a straight-edged cell spans many degrees of
 * longitude, so the owner may lie rings away from the estimate.
 * @param[in] lon,lat Longitude and latitude of the position, in degrees
 * @param[in] geodetic True when the position is geodetic
 * @param[in] resolution H3 resolution
 */
H3Index
h3_point_cell(double lon, double lat, bool geodetic, int32 resolution)
{
  H3Index c0 = h3_latlng_deg_to_cell(lat, lon, resolution);
  if (! geodetic)
    return h3_cell_around(c0, lon, lat, NULL, NULL);
  double d[3];
  h3_lonlat_vector(lon, lat, d);
  return h3_cell_around(c0, lon, lat, d, NULL);
}

/**
 * @brief Return the length in degrees of the path between two positions
 * @details The path is the straight line in longitude and latitude for a
 * planar point and the great-circle arc for a geodetic one.
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
 * @brief Return the microsecond at which a geodetic segment crosses the
 * great circle of an edge, as the offset from the start of the segment,
 * truncated, its points given in degrees (#dggs_geodetic_edge_offset on
 * their geodetic positions, #h3_lonlat_vector)
 */
int64
h3_geodetic_edge_offset(double x0, double y0, double x1, double y1, int64 D,
  double ax, double ay, double bx, double by, double f, bool *whole)
{
  double p0[3], p1[3], a[3], b[3];
  h3_lonlat_vector(x0, y0, p0);
  h3_lonlat_vector(x1, y1, p1);
  h3_lonlat_vector(ax, ay, a);
  h3_lonlat_vector(bx, by, b);
  return dggs_geodetic_edge_offset(p0, p1, D, a, b, f, whole);
}

/** @brief Return true if a cell is convex on the sphere
 * (#dggs_outline_convex_geodetic) */
static bool
h3_outline_convex_geodetic(const DggsOutline *o)
{
  return dggs_outline_convex_geodetic(&h3_geodetic_ops, o);
}

/** @brief The outline of a cell for the geodetic walk, into the caller's
 * copy (#h3_cell_outline) */
static bool
h3_geo_outline_copy(uint64 cell, DggsOutline *out)
{
  return h3_cell_outline((H3Index) cell, out);
}

/** @brief The geodetic position of the vertex k of an outline
 * (#h3_outline_vector) */
static void
h3_geo_vector(const DggsOutline *o, int k, double *v)
{
  POINT3D q;
  h3_outline_vector(o, k, &q);
  v[0] = q.x; v[1] = q.y; v[2] = q.z;
}

/** @brief The geodetic cell of a position for the geodetic walk
 * (#h3_point_cell) */
static uint64
h3_geo_point_cell(double lon, double lat, int32 resolution)
{
  return h3_point_cell(lon, lat, true, resolution);
}

/** @brief The cell holding a direction, searched around a cell
 * (#h3_cell_around) */
static uint64
h3_geo_dir_around(uint64 cell, const SphereDir *p)
{
  return h3_cell_around((H3Index) cell, 0.0, 0.0, NULL, p);
}

/** @brief The cell and its neighbours, the disk of radius 1 */
static int
h3_geo_neighbours(uint64 cell, uint64 *out, int max)
{
  H3Index disk[7] = { 0 };
  if (max < 7 || gridDisk((H3Index) cell, 1, disk) != E_SUCCESS)
    return 0;
  for (int i = 0; i < 7; i++)
    out[i] = (uint64) disk[i];
  return 7;
}

/** @brief The geodetic walk of H3: its outlines, the vectors of decision D6
 * of their vertices, and its owner rule, a cell holding its north and west
 * sides (#h3_outline_edge_held_geodetic, #h3_outline_vertex_held_geodetic) */
static const DggsGeodeticOps h3_geodetic_ops =
{
  h3_planar_outline,
  h3_geo_outline_copy,
  h3_geo_vector,
  h3_geo_point_cell,
  h3_geo_dir_around,
  h3_geo_neighbours,
  h3_outline_edge_held_geodetic,
  h3_outline_vertex_held_geodetic
};

/**
 * @brief Fill `cells` with every cell a geodetic segment crosses, and
 * `enter` with the fraction of the segment at which it reaches each
 * (#dggs_segment_cells_geodetic on the table of H3)
 */
static int
h3_segment_cells_geodetic(double lon1, double lat1, double lon2, double lat2,
  int32 resolution, H3Index *cells, double *enter, DggsEdgeCross *cross,
  int maxout)
{
  return dggs_segment_cells_geodetic(&h3_geodetic_ops, lon1, lat1, lon2,
    lat2, resolution, (uint64 *) cells, enter, cross, maxout);
}

/**
 * @brief Fill `cells` with every cell the segment crosses, and `enter` with
 * the segment parameter at which it reaches each
 * @details The traversal of a grid by a line, as the split of a temporal point
 * by a space grid walks its tiles: from the cell in hand, the path leaves
 * through the edge it crosses first, and the cell across that edge is the
 * next one, so each cell of the walk is a neighbour of the one before it and
 * no cell between the two is passed over, however short the chord the path
 * clips from it. The walk ends at the cell of the far endpoint. A path never
 * leaves a cell through the edge it entered it by, as a line never crosses
 * back over the tile boundary it has just crossed: across a face of the
 * icosahedron the two cells sharing an edge state it apart, one bent at the
 * face and the other not, so the crossing of one lies just outside the other.
 *
 * A crossing is the parameter at which the path meets the edge, and a cell is
 * entered there. A path passing through a vertex, where three cells meet,
 * leaves through one of the two edges meeting there and reaches the third
 * cell from the second at the same parameter, as a line through the corner of
 * four tiles passes one of the two side tiles for no length.
 *
 * The walk follows the path the point moves along: the straight line in
 * longitude and latitude of a planar point, walked on the straight-edged
 * outlines of the cells by exact signs (#dggs_segment_cells_planar), the
 * great circle of a geodetic one on the boundaries of the library. A
 * geodetic crossing is found on the sphere, where a path across the
 * antimeridian or over a pole is an arc like any other.
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] geodetic True when the segment is geodetic
 * @param[in] resolution H3 resolution
 * @param[out] cells,enter Arrays of at least `maxout` entries; `enter[0]` is
 *   always 0, the parameter of the first endpoint
 * @param[out] cross Array of at least `maxout` entries, or NULL; for a
 *   planar segment, the crossing into each cell after the first
 *   (#dggs_segment_cells_planar)
 * @param[in] maxout Capacity of the arrays
 * @return Number of cells written, or 0 when the first lookup fails
 */
int
h3_segment_cells(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, int32 resolution, H3Index *cells, double *enter,
  DggsEdgeCross *cross, int maxout)
{
  assert(cells); assert(enter);
  if (maxout < 1)
    return 0;
  if (! geodetic)
    return dggs_segment_cells_planar(&h3_planar_ops, lon1, lat1, lon2,
      lat2, resolution, cells, enter, cross, maxout);
  return h3_segment_cells_geodetic(lon1, lat1, lon2, lat2, resolution, cells,
    enter, cross, maxout);
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
      resolution, cells, enter, NULL, maxout)) == maxout)
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
  h3_buf_push(out, h3_point_cell(lon2, lat2, geodetic, resolution));
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
  h3_buf_push(out, h3_point_cell(p->x, p->y, geodetic, resolution));
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
      h3_buf_push(out, h3_point_cell(p->x, p->y, geodetic, resolution));
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
 * @ingroup meos_h3_conversion
 * @brief Return the set of H3 cells covering an H3 cell at the given
 * resolution
 * @details The cover is the set of the cells that hold a point of the cell,
 * each point assigned to its cell as #geo_to_h3index_cell assigns it, so a
 * trajectory passing through the cell takes a cell of the cover there. The
 * cover of the values of a temporal H3 cell is therefore a cover of the
 * trajectory they come from, read at a resolution the values are not stored
 * at.
 *
 * At a resolution no finer than the cell's own, the cover is the walk of the
 * cell's boundary: a cell of that resolution is at least as wide as this one,
 * so it cannot meet it without holding a point of its boundary, and the walk
 * of #h3_segment_cells passes through every cell a boundary reaches. At a
 * finer resolution the cell is read as the polygon it is, since cells of that
 * resolution lie in its interior, which no boundary reaches.
 * @param[in] cell H3 cell
 * @param[in] resolution H3 resolution of the cover
 * @errval NULL
 * @csqlfn #H3_cell_to_cover()
 */
Set *
h3index_cell_to_cover(H3Index cell, int32 resolution)
{
  /* Ensure the validity of the arguments */
  if (! ensure_h3index_cell(cell) ||
      ! ensure_valid_cell_resolution(T_TH3INDEX, resolution))
    return NULL;

  /* A cell of a finer resolution lies in the interior of this one, which the
   * walk of a boundary never reaches, so a finer cover is a different
   * construction and is refused rather than approximated */
  int32_t res = getResolution(cell);
  if (resolution > res)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The cover of a cell at resolution %d, finer than the cell's own %d, is not supported",
      resolution, res);
    return NULL;
  }

  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts < 3)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  h3_buf buf;
  h3_buf_init(&buf, 2 * MAX_CELL_BNDRY_VERTS);
  /* The edges of a cell are arcs of great circles, the path the geodetic
   * walk follows, and the last edge closes the boundary on its first vertex */
  for (int i = 0; i < bnd.numVerts && ! buf.overflow; i++)
  {
    const LatLng *v0 = &bnd.verts[i];
    const LatLng *v1 = &bnd.verts[(i + 1) % bnd.numVerts];
    segment_to_cells_into(radsToDegs(v0->lng), radsToDegs(v0->lat),
      radsToDegs(v1->lng), radsToDegs(v1->lat), true, resolution, &buf);
  }
  if (buf.overflow)
  {
    h3_buf_free(&buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The cover of the cell at resolution %d exceeds %d cells", resolution,
      H3_MAX_COVER_CELLS);
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
 * @errval -1
 */
int
ever_eq_h3indexset_th3index(const Set *cells, const Temporal *th3idx)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEXSET(cells, -1); VALIDATE_TH3INDEX(th3idx, -1);
  return tcellindex_ever_in_set(th3idx, cells) ? 1 : 0;
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
 * @param[in] th3idx The th3index temporal value.
 * @param[in] cells The candidate H3 cell set (T_H3INDEX).
 * @csqlfn #Ever_eq_th3index_h3indexset()
 * @errval -1
 */
int
ever_eq_th3index_h3indexset(const Temporal *th3idx, const Set *cells)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(th3idx, -1); VALIDATE_H3INDEXSET(cells, -1);
  return tcellindex_ever_in_set(th3idx, cells) ? 1 : 0;
}

/*****************************************************************************/