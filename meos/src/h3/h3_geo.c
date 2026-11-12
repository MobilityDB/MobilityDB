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
#include "h3/th3index_internal.h"  /* H3Outline */
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
 * @brief Fill the outline of a cell bent across no face of the icosahedron
 * and return true, or return false for a bent cell
 * @details A cell whose boundary in the library (`cellToBoundary`) holds no
 * more vertices than the cell has edges crosses no face of the icosahedron,
 * so its edges bend at no extra vertex, and that boundary lists the vertices
 * of the vertex index in their order (both measured over every cell of the
 * resolutions 0 to 5). The cell across the edge from the vertex k is then
 * the destination of the directed edge of the cell whose boundary holds the
 * k-th and the next vertex of the boundary, which the library computes
 * alike, as #h3_cell_across_edge reads it, so no vertex of a neighbour is
 * read
 * @param[in] cell Cell
 * @param[in] verts,n Its vertices in the vertex index and their number
 * @param[out] out Outline, its vertices and the cells across its edges
 */
static bool
h3_cell_outline_straight(H3Index cell, const H3Index *verts, int n,
  H3Outline *out)
{
  CellBoundary bnd;
  H3Index edges[6] = { 0 };
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts != n ||
      originToDirectedEdges(cell, edges) != E_SUCCESS)
    return false;
  CellBoundary eb[6];
  H3Index dest[6];
  int ne = 0;
  for (int j = 0; j < 6; j++)
  {
    H3Index ends[2];
    if (edges[j] == (H3Index) 0)
      continue;
    if (directedEdgeToBoundary(edges[j], &eb[ne]) != E_SUCCESS ||
        directedEdgeToCells(edges[j], ends) != E_SUCCESS)
      return false;
    dest[ne++] = ends[1];
  }
  for (int k = 0; k < n; k++)
  {
    LatLng v;
    if (vertexToLatLng(verts[k], &v) != E_SUCCESS)
      return false;
    out->lon[k] = radsToDegs(v.lng);
    out->lat[k] = radsToDegs(v.lat);
    out->across[k] = (H3Index) 0;
    for (int j = 0; j < ne && ! out->across[k]; j++)
      if (eb[j].numVerts == 2 && h3_boundary_holds(&eb[j], &bnd.verts[k]) &&
          h3_boundary_holds(&eb[j], &bnd.verts[(k + 1) % n]))
        out->across[k] = dest[j];
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
h3_cell_outline(H3Index cell, H3Outline *out)
{
  H3Index verts[6] = { 0 };
  if (cellToVertexes(cell, verts) != E_SUCCESS)
    return false;
  H3Index north, south;
  h3index_pole_cells(getResolution(cell), &north, &south);
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
 * @brief Return the offset that reads a longitude in the chart of another
 * @details The offset, 0 or a multiple of 360, puts the longitude less than
 * 180 degrees from the reference, so that an edge of an outline crossing the
 * antimeridian is read along its path. An edge of a cell spans far less than
 * 180 degrees of longitude, so the rounded difference decides the chart; the
 * offset is never added to the longitude, whose shifted value would round
 */
static double
h3_lon_offset(double ref, double lon)
{
  if (lon - ref > 180.0)
    return -360.0;
  if (ref - lon > 180.0)
    return 360.0;
  return 0.0;
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
  int dx = offset_x_cmp(bx, h3_lon_offset(ax, bx), ax);
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
h3_outline_edge_held(const H3Outline *o, int k)
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
h3_outline_vertex_held(const H3Outline *o, int k)
{
  int kp = (k + o->count - 1) % o->count, kn = (k + 1) % o->count;
  double x = o->lon[k], y = o->lat[k];
  double px = o->lon[kp], py = o->lat[kp];
  double nx = o->lon[kn], ny = o->lat[kn];
  int in = h3_edge_south_side(px, py, x, y);
  int out = h3_edge_south_side(x, y, nx, ny);
  /* (here - prev) x (next - here), prev and next read in the chart of here */
  int turn = cross_product_sign_offset(px, py, x, y, x, y, nx, ny,
    - h3_lon_offset(x, px), h3_lon_offset(x, nx));
  return turn > 0 ? (in && out) : (in || out);
}

/**
 * @brief Return in @p v the geodetic position of the vertex k of an outline,
 * the vector of decision D6 of the cell walks (#lonlat_vector_nearest)
 */
static void
h3_outline_vector(const H3Outline *o, int k, POINT3D *v)
{
  double d[3];
  lonlat_vector_nearest(o->lon[k], o->lat[k], d);
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
h3_outline_edge_held_geodetic(const H3Outline *o, int k)
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
h3_outline_vertex_held_geodetic(const H3Outline *o, int k)
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

/**
 * @brief Return the microsecond at which a segment crosses the line of the
 * edge of an outline from the vertex k to the vertex k + 1, as the offset
 * from the start of the segment, truncated
 * @details The edge is read in the chart of the start of the segment, each
 * vertex through an exact offset of 360 (#h3_lon_offset), and the crossing
 * is dated by #segment_edge_cross_offset, the truncation of its exact
 * parameter. Whether the segment meets the edge itself, between its
 * vertices, is a question of its own.
 * @param[in] o Outline of the cell
 * @param[in] k Index of the first vertex of the edge
 * @param[in] x0,y0,x1,y1 Start and end points of the segment, in degrees
 * @param[in] D Duration of the segment in microseconds
 * @param[out] whole True when the segment lies on the line exactly at the
 * microsecond returned
 */
int64
h3_outline_edge_offset(const H3Outline *o, int k, double x0, double y0,
  double x1, double y1, int64 D, bool *whole)
{
  int k1 = (k + 1) % o->count;
  double ax = o->lon[k], bx = o->lon[k1];
  double oa = h3_lon_offset(x0, ax);
  /* Both offsets are multiples of 360 of small magnitude, so their sum is
   * exact */
  double ob = oa + h3_lon_offset(ax, bx);
  return segment_edge_cross_offset(x0, y0, x1, y1, D, ax, o->lat[k], oa, bx,
    o->lat[k1], ob, whole);
}

/**
 * @brief Return in the last arguments the chart in which an outline is read
 * against a longitude
 * @details The offset of each vertex from the first along the ring, chained
 * by #h3_lon_offset, so that no edge of the ring spans the antimeridian, and
 * the offset of the copy of the ring read against the longitude: the copy
 * whose middle lies within 180 degrees of it, or around a pole the copy
 * whose 360 degrees, from the first vertex in the sense of the ring, hold
 * it. The estimate of the copy is rounded, its correction exact
 * @param[in] o Outline of the cell
 * @param[in] x Longitude, in degrees
 * @param[out] xo Offset of each vertex from the first
 * @param[out] s Offset of the copy
 * @param[out] wrap Around a pole, the offset of the ring after one turn,
 * 360 or -360, and 0 for any other cell
 */
static void
h3_outline_chart(const H3Outline *o, double x, double *xo, double *s,
  double *wrap)
{
  int n = o->count;
  xo[0] = 0.0;
  for (int k = 1; k < n; k++)
    xo[k] = xo[k - 1] + h3_lon_offset(o->lon[k - 1], o->lon[k]);
  double x0 = o->lon[0];
  *wrap = 0.0;
  if (o->pole)
  {
    *wrap = xo[n - 1] + h3_lon_offset(o->lon[n - 1], x0);
    double c = 360.0 * floor((x - x0) / 360.0);
    if (*wrap > 0.0)
    {
      while (offset_x_cmp(x0, c, x) > 0)
        c -= 360.0;
      while (offset_x_cmp(x0, c + 360.0, x) <= 0)
        c += 360.0;
    }
    else
    {
      c += 360.0;
      while (offset_x_cmp(x0, c, x) < 0)
        c += 360.0;
      while (offset_x_cmp(x0, c - 360.0, x) >= 0)
        c -= 360.0;
    }
    *s = c;
    return;
  }
  double lo = x0, hi = x0;
  for (int k = 1; k < n; k++)
  {
    double v = o->lon[k] + xo[k];
    lo = fmin(lo, v);
    hi = fmax(hi, v);
  }
  double mid = (lo + hi) / 2.0;
  double c = 360.0 * round((x - mid) / 360.0);
  while (mid + c - x > 180.0)
    c -= 360.0;
  while (x - (mid + c) > 180.0)
    c += 360.0;
  *s = c;
  return;
}

/**
 * @brief Return true if a cell holds a position of the plane of longitude
 * and latitude
 * @details A cell is its outline joined by straight edges in longitude and
 * latitude, the ring read along its path across the antimeridian through
 * exact offsets of 360 chained from its first vertex, and the cell around a
 * pole is the region between its ring and the pole. A position strictly
 * inside or outside is read by #point_in_ring_offset_exact on the copy of
 * the ring #h3_outline_chart reads against it; a position on the ring belongs to the cell when
 * it holds the vertex (#h3_outline_vertex_held) or the edge
 * (#h3_outline_edge_held) the position lies on. A position on a side of the
 * region around a pole that is no edge of the cell, the meridian where the
 * ring is closed or the pole, lies inside it. Every sign is exact on the
 * doubles of the outline, which neighbours share, so exactly one cell holds
 * each position.
 * @param[in] o Outline of the cell
 * @param[in] x,y Longitude and latitude of the position, in degrees
 */
bool
h3_outline_holds_point(const H3Outline *o, double x, double y)
{
  if (! isfinite(x) || ! isfinite(y))
    return false;
  int n = o->count;
  double xo[MAX_CELL_BNDRY_VERTS], s, wrap;
  h3_outline_chart(o, x, xo, &s, &wrap);
  double x0 = o->lon[0];
  /* The closed polygon: the ring, and around a pole the vertex closing it
   * 360 degrees on, then the pole at both ends */
  double rx[MAX_CELL_BNDRY_VERTS + 3] = { 0 },
    ry[MAX_CELL_BNDRY_VERTS + 3] = { 0 }, ro[MAX_CELL_BNDRY_VERTS + 3] = { 0 };
  int m = 0;
  for (int k = 0; k < n; k++, m++)
  {
    rx[m] = o->lon[k];
    ry[m] = o->lat[k];
    ro[m] = xo[k] + s;
  }
  if (o->pole)
  {
    double py = o->pole > 0 ? 90.0 : -90.0;
    rx[m] = x0; ry[m] = o->lat[0]; ro[m++] = wrap + s;
    rx[m] = x0; ry[m] = py; ro[m++] = wrap + s;
    rx[m] = x0; ry[m] = py; ro[m++] = s;
  }
  int loc = point_in_ring_offset_exact(rx, ry, ro, m, x, y);
  if (loc >= 0)
    return loc == 1;
  /* On the polygon: the vertex or the edge of the ring the position lies
   * on decides */
  for (int k = 0; k < n; k++)
  {
    int j = o->pole ? k + 1 : (k + 1) % n;
    int sa = offset_x_cmp(rx[k], ro[k], x), sb = offset_x_cmp(rx[j], ro[j], x);
    if (sa == 0 && ry[k] == y)
      return h3_outline_vertex_held(o, k);
    if (sb == 0 && ry[j] == y)
      return h3_outline_vertex_held(o, (k + 1) % n);
    if (sa * sb <= 0 && fmin(ry[k], ry[j]) <= y && y <= fmax(ry[k], ry[j]) &&
        cross_product_sign_offset(rx[k], ry[k], rx[j], ry[j], rx[k], ry[k], x,
          y, ro[j] - ro[k], - ro[k]) == 0)
      return h3_outline_edge_held(o, k);
  }
  /* The meridian closing the ring around a pole, or the pole */
  return o->pole != 0;
}

/** @brief Outline of the last cell #h3_cell_outline_last read, per thread */
static MEOS_TLS H3Outline h3_last_outline;
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
static const H3Outline *
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

/**
 * @brief Return true if a cell holds a position, by #h3_outline_holds_point
 * on the outline #h3_cell_outline_last reads
 */
static bool
h3_cell_holds_point(H3Index cell, double x, double y)
{
  const H3Outline *o = h3_cell_outline_last(cell);
  return o && h3_outline_holds_point(o, x, y);
}

/** @brief Number of rings around the cell of the library a planar position
 * is looked for in */
#define H3_POINT_CELL_RINGS 16

/**
 * @brief Return the cell of a given resolution holding a position, or 0
 * when none is found
 * @details A geodetic position is read by the library
 * (#h3_latlng_deg_to_cell). A planar cell is its straight-edged outline
 * (#h3_outline_holds_point), which departs from the cell of the library,
 * whose edges are not straight in longitude and latitude: the cell of the
 * library (#h3_latlng_deg_to_cell) is the estimate, and the cells around it,
 * ring by ring, are asked in turn until one holds the position. Near a pole
 * a straight-edged cell spans many degrees of longitude, so the owner may
 * lie rings away from the estimate.
 * @param[in] lon,lat Longitude and latitude of the position, in degrees
 * @param[in] geodetic True when the position is geodetic
 * @param[in] resolution H3 resolution
 */
H3Index
h3_point_cell(double lon, double lat, bool geodetic, int32 resolution)
{
  H3Index c0 = h3_latlng_deg_to_cell(lat, lon, resolution);
  if (geodetic || c0 == (H3Index) 0 || h3_cell_holds_point(c0, lon, lat))
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
      if (cells[i] != (H3Index) 0 && dist[i] == k &&
          h3_cell_holds_point(cells[i], lon, lat))
        result = cells[i];
  }
  pfree(cells); pfree(dist);
  return result;
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
 * @brief Return true if the boundary of a cell is convex
 * @details A cell is a hexagon or a pentagon whose edges are arcs of great
 * circles, and it is convex unless it spans two faces of the icosahedron,
 * where its boundary bends at the vertices it crosses between them and holds
 * more vertices than the cell has edges
 */
static bool
h3_cell_is_convex(H3Index cell, const CellBoundary *bnd)
{
  return bnd->numVerts == (isPentagon(cell) ? 5 : 6);
}

/**
 * @brief Return where the great circle of a geodetic segment leaves a cell,
 * and in the last argument the edge of the cell boundary it crosses there
 * @details The path leaves where its great circle first leaves the
 * hemisphere of an edge, found in closed form by
 * #dggs_arc_hemisphere_exit_param, or for a bent cell where it first crosses
 * an edge, by #dggs_arc_exit_param. The
 * exit lies strictly ahead of `tmin`, or at `tmin` for a path on the boundary
 * there and heading out, and never on an edge the path entered the cell
 * through. A cell spanning two faces of the icosahedron bends at the vertices
 * where it crosses between them, so a crossing counts there where it lies on
 * its edge.
 * @param[in] cell Cell
 * @param[in] bnd Boundary of the cell
 * @param[in] entry Mask of the edges of @p bnd the path entered the cell
 * through
 * @param[in] arc Great circle of the segment
 * @param[in] tmin Parameter at which the path entered the cell
 * @param[out] edge Edge of @p bnd crossed at the exit
 * @return the path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
static double
h3_cell_exit_param(H3Index cell, const CellBoundary *bnd, uint32 entry,
  const DggsArc *arc, double tmin, int *edge)
{
  double lons[MAX_CELL_BNDRY_VERTS], lats[MAX_CELL_BNDRY_VERTS];
  for (int i = 0; i < bnd->numVerts; i++)
  {
    lons[i] = bnd->verts[i].lng;
    lats[i] = bnd->verts[i].lat;
  }
  if (h3_cell_is_convex(cell, bnd))
    return dggs_arc_hemisphere_exit_param(arc, lons, lats, bnd->numVerts,
      tmin, entry, edge);
  return dggs_arc_exit_param(arc, lons, lats, bnd->numVerts, tmin, entry,
    edge);
}

/**
 * @brief Return true if a directed edge of the grid holds a vertex of a cell
 * boundary
 * @details The boundary of a cell and the boundary of each of its directed
 * edges state a shared vertex by the same computation, so a vertex of one is
 * a vertex of the other exactly
 */
static bool
h3_edge_holds_vertex(const CellBoundary *edge, const LatLng *vertex)
{
  for (int k = 0; k < edge->numVerts; k++)
    if (memcmp(&edge->verts[k], vertex, sizeof(LatLng)) == 0)
      return true;
  return false;
}

/**
 * @brief Return the cell lying across an edge of the boundary of a cell, or 0
 * when the position across that edge holds no cell
 * @details The GRID states which cells adjoin a cell, and the walk steps to
 * the one across the edge it crosses by asking it: the directed edges of the
 * cell name its neighbours, and the boundary of a directed edge shares both
 * vertices of exactly the boundary edge it runs along, which is the match
 * #h3_cell_entry_edges makes to read the edges a neighbour is reached
 * through. The neighbour is the destination of that directed edge.
 *
 * A POSITION CONSTRUCTED ACROSS THE EDGE AND HANDED BACK TO #latLngToCell
 * states the neighbour only as closely as the construction lands: a centre
 * reflected through the midpoint of an edge carries the rounding of the
 * vertices, of the reflection, and of the angles it is turned back into, and
 * at a CORNER, where three cells meet within a last place of one another, the
 * cell holding that position is decided by that rounding. The grid's own
 * adjacency is exact and answers with no position at all, as
 * #s2cell_edge_neighbors answers for an S2 cell and a step of one tile
 * answers for a quadbin one.
 * @param[in] cell Cell the walk stands in
 * @param[in] bnd Boundary of @p cell
 * @param[in] edge Edge of @p bnd, from vertex @p edge to the one after it
 */
static H3Index
h3_cell_across_edge(H3Index cell, const CellBoundary *bnd, int edge)
{
  H3Index edges[6];
  if (originToDirectedEdges(cell, edges) != E_SUCCESS)
    return (H3Index) 0;
  const LatLng *v0 = &bnd->verts[edge];
  const LatLng *v1 = &bnd->verts[(edge + 1) % bnd->numVerts];
  for (int k = 0; k < 6; k++)
  {
    CellBoundary eb;
    if (edges[k] == (H3Index) 0 ||
        directedEdgeToBoundary(edges[k], &eb) != E_SUCCESS)
      continue;
    if (! h3_edge_holds_vertex(&eb, v0) || ! h3_edge_holds_vertex(&eb, v1))
      continue;
    H3Index ends[2];
    if (directedEdgeToCells(edges[k], ends) != E_SUCCESS)
      return (H3Index) 0;
    return ends[1];
  }
  return (H3Index) 0;
}

/**
 * @brief Return the mask of the edges of the boundary of a cell it shares
 * with a neighbour, bit `i` for the edge from vertex `i`
 * @details The edges shared with a neighbour are those of the directed edge
 * from the cell to it; a cell bent across a face of the icosahedron states
 * that edge in two pieces
 */
static uint32
h3_cell_entry_edges(H3Index cell, const CellBoundary *bnd, H3Index neighbour)
{
  H3Index dedge;
  CellBoundary eb;
  if (cellsToDirectedEdge(cell, neighbour, &dedge) != E_SUCCESS ||
      directedEdgeToBoundary(dedge, &eb) != E_SUCCESS)
    return 0;
  uint32 mask = 0;
  for (int i = 0; i < bnd->numVerts; i++)
    if (h3_edge_holds_vertex(&eb, &bnd->verts[i]) &&
        h3_edge_holds_vertex(&eb, &bnd->verts[(i + 1) % bnd->numVerts]))
      mask |= 1u << i;
  return mask;
}

/*****************************************************************************
 * The planar walk on the straight-edged outlines
 *****************************************************************************/

/**
 * @brief Outline of a cell read in the chart of a segment
 * @details Vertex `i` of the chart, for any integer `i`, is the vertex `i mod
 * n` of the outline, its longitude carrying the exact offset `xo[i mod n] +
 * s`, and around a pole `wrap` more for each turn of the ring, so that the
 * edge from each vertex to the next runs along the ring in the chart of the
 * segment, the ring around a pole repeated every 360 degrees. No offset is
 * added to a longitude, where it would round.
 */
typedef struct
{
  H3Index cell;                         /**< Cell */
  H3Outline o;                          /**< Its outline */
  double xo[MAX_CELL_BNDRY_VERTS];      /**< Offset of each vertex from the
                                             first along the ring */
  double s;                             /**< Offset of the copy */
  double wrap;                          /**< Around a pole, the offset of
                                             the ring after one turn */
} H3Chart;

/**
 * @brief A crossing of the planar walk: the line of an edge of an outline
 * in the chart of the segment, which the segment crosses where it leaves a
 * cell
 */
typedef struct
{
  double ax, ay, oa;      /**< First vertex of the edge and its offset */
  double bx, by, ob;      /**< Second vertex of the edge and its offset */
  int index;              /**< Chart index of the edge crossed, or of the
                               vertex passed */
  bool vertex;            /**< True when the segment leaves at a vertex */
  int cout;               /**< At a vertex, the side of the edge leaving it
                               the segment heads to */
  bool along_out;         /**< At a vertex, the segment runs along the edge
                               leaving it */
  bool back_in;           /**< At a vertex, the segment runs back along the
                               edge reaching it */
} H3Crossing;

/**
 * @brief Return in the last arguments the vertex `i` of a chart and the
 * offset of its longitude
 */
static void
h3_chart_vertex(const H3Chart *c, int i, double *x, double *y, double *off)
{
  int n = c->o.count;
  int q = (i >= 0) ? i / n : - ((n - 1 - i) / n);
  int k = i - q * n;
  *x = c->o.lon[k];
  *y = c->o.lat[k];
  *off = c->xo[k] + c->s + (c->o.pole ? q * c->wrap : 0.0);
  return;
}

/**
 * @brief Return in the last argument the chart of a cell whose copy holds a
 * longitude, as #h3_outline_holds_point reads it, false on an error of the
 * H3 library
 */
static bool
h3_chart_at_lon(H3Index cell, double x, H3Chart *c)
{
  const H3Outline *o = h3_cell_outline_last(cell);
  if (! o)
    return false;
  c->cell = cell;
  c->o = *o;
  h3_outline_chart(o, x, c->xo, &c->s, &c->wrap);
  return true;
}

/**
 * @brief Return in the next to last argument the chart of a cell that puts
 * one of its vertices at a given offset, and in the last one the index of
 * that vertex, false when the cell has no such vertex
 * @details The walk steps from a cell to the next through a vertex both
 * outlines state with the same doubles, so the next chart continues the
 * first along the segment
 */
static bool
h3_chart_at_vertex(H3Index cell, double x, double y, double off, H3Chart *c,
  int *vertex)
{
  if (! h3_chart_at_lon(cell, x, c))
    return false;
  for (int k = 0; k < c->o.count; k++)
    if (c->o.lon[k] == x && c->o.lat[k] == y)
    {
      c->s = off - c->xo[k];
      *vertex = k;
      return true;
    }
  return false;
}

/**
 * @brief Return true if the outline of a chart turns left, or goes
 * straight, at every vertex, by the exact sign of each turn
 */
static bool
h3_chart_convex(const H3Chart *c)
{
  if (c->o.pole)
    return false;
  for (int k = 0; k < c->o.count; k++)
  {
    double px, py, po, x, y, o, nx, ny, no;
    h3_chart_vertex(c, k - 1, &px, &py, &po);
    h3_chart_vertex(c, k, &x, &y, &o);
    h3_chart_vertex(c, k + 1, &nx, &ny, &no);
    if (cross_product_sign_offset(px, py, x, y, x, y, nx, ny, o - po,
        no - o) < 0)
      return false;
  }
  return true;
}

/**
 * @brief Return the sign of the dot product of the vector from A to B with
 * the vector of a segment parallel to it, from the signs of their components
 */
static int
h3_parallel_dot_sign(double ax, double ay, double oa, double bx, double by,
  double ob, int dxs, int dys)
{
  if (dxs != 0)
    return offset_x_cmp(bx, ob - oa, ax) * dxs;
  return ((by > ay) - (by < ay)) * dys;
}

/**
 * @brief Return true if a segment passing through the vertex `j` of a chart
 * lies in the cell just after it, and fill the fields of the crossing that
 * name the cell it heads into otherwise
 * @details Just after the vertex the segment lies in the cell where it lies
 * left of both edges meeting there at a convex corner, or of either at a
 * reflex one, since the cell lies left of its edges; running along an edge,
 * it lies in the cell when the cell holds the edge (#h3_outline_edge_held).
 * Every sign is exact on the doubles of the outline and the offsets of the
 * chart (#cross_product_sign_offset)
 */
static bool
h3_chart_vertex_inside(const H3Chart *c, int j, double x0, double y0,
  double x1, double y1, H3Crossing *cr)
{
  double px, py, po, vx, vy, vo, nx, ny, no;
  h3_chart_vertex(c, j - 1, &px, &py, &po);
  h3_chart_vertex(c, j, &vx, &vy, &vo);
  h3_chart_vertex(c, j + 1, &nx, &ny, &no);
  int n = c->o.count;
  int k = ((j % n) + n) % n;
  int cin = cross_product_sign_offset(px, py, vx, vy, x0, y0, x1, y1,
    vo - po, 0.0);
  int cout = cross_product_sign_offset(vx, vy, nx, ny, x0, y0, x1, y1,
    no - vo, 0.0);
  int turn = cross_product_sign_offset(px, py, vx, vy, vx, vy, nx, ny,
    vo - po, no - vo);
  int dxs = (x1 > x0) - (x1 < x0), dys = (y1 > y0) - (y1 < y0);
  cr->cout = cout;
  cr->along_out = cout == 0 &&
    h3_parallel_dot_sign(vx, vy, vo, nx, ny, no, dxs, dys) > 0;
  cr->back_in = cin == 0 &&
    h3_parallel_dot_sign(px, py, po, vx, vy, vo, dxs, dys) < 0;
  if (cr->along_out)
    return h3_outline_edge_held(&c->o, k);
  if (cr->back_in)
    return h3_outline_edge_held(&c->o, (k + n - 1) % n);
  return (turn >= 0) ? (cin > 0 && cout > 0) : (cin > 0 || cout > 0);
}

/**
 * @brief Return true and in the last argument where a planar segment leaves
 * the cell of a chart, at or after a crossing, false when it ends in the
 * cell
 * @details The segment meets an edge of the outline where the two vertices of
 * the edge lie on either side of its line or on it. Where it crosses the
 * edge between its vertices, it leaves when it heads out, to the right of the
 * edge; where it meets a vertex, it leaves when it does not lie in the cell
 * just after (#h3_chart_vertex_inside). A meeting at the start of the
 * segment, or at the crossing the walk entered the cell by, counts, so a
 * segment passing a vertex reaches the third cell meeting there at the same
 * parameter. The first meeting where it leaves is the exit, the meetings
 * ordered by #segment_edge_cross_cmp; a meeting at the end of the segment is
 * none, the end belonging to the cell of its own position. Around a pole the
 * ring is read in the copies the segment reaches.
 * @param[in] c Chart of the cell
 * @param[in] x0,y0,x1,y1 The segment
 * @param[in] t Crossing the walk entered the cell by, or NULL at the start
 * @param[out] exit The exit
 */
static bool
h3_chart_exit(const H3Chart *c, double x0, double y0, double x1, double y1,
  const H3Crossing *t, H3Crossing *exit)
{
  int n = c->o.count, ilo = 0, ihi = n;
  if (c->o.pole)
  {
    int nq = (int) ceil(fabs(x1 - x0) / 360.0) + 2;
    ilo = - nq * n;
    ihi = (nq + 1) * n;
  }
  bool found = false;
  for (int i = ilo; i < ihi; i++)
  {
    H3Crossing cr;
    memset(&cr, 0, sizeof(H3Crossing));
    h3_chart_vertex(c, i, &cr.ax, &cr.ay, &cr.oa);
    h3_chart_vertex(c, i + 1, &cr.bx, &cr.by, &cr.ob);
    /* The sides of the line of the segment the vertices of the edge lie on */
    int sa = cross_product_sign_offset(x0, y0, x1, y1, x0, y0, cr.ax, cr.ay,
      0.0, cr.oa);
    int sb = cross_product_sign_offset(x0, y0, x1, y1, x0, y0, cr.bx, cr.by,
      0.0, cr.ob);
    if ((sa == 0 && sb == 0) || sa * sb > 0)
      continue;
    /* The sign of f(P0) - f(P1), f the side of the edge, which is not 0
     * since the vertices lie apart from the line of the segment; a crossing
     * at or after the end of the segment is none */
    int den = cross_product_sign_offset(cr.ax, cr.ay, cr.bx, cr.by, x1, y1,
      x0, y0, cr.ob - cr.oa, 0.0);
    int f1 = cross_product_sign_offset(cr.ax, cr.ay, cr.bx, cr.by, cr.ax,
      cr.ay, x1, y1, cr.ob - cr.oa, - cr.oa);
    if (f1 * den >= 0)
      continue;
    /* At or after the crossing the walk stands at */
    int cmp;
    if (! t)
      cmp = cross_product_sign_offset(cr.ax, cr.ay, cr.bx, cr.by, cr.ax,
        cr.ay, x0, y0, cr.ob - cr.oa, - cr.oa) * den;
    else
      cmp = segment_edge_cross_cmp(x0, y0, x1, y1, cr.ax, cr.ay, cr.oa,
        cr.bx, cr.by, cr.ob, t->ax, t->ay, t->oa, t->bx, t->by, t->ob);
    if (cmp < 0)
      continue;
    if (sa != 0 && sb != 0)
    {
      /* Across the edge between its vertices, heading out */
      if (den <= 0)
        continue;
      cr.index = i;
      cr.vertex = false;
    }
    else
    {
      cr.index = (sa == 0) ? i : i + 1;
      cr.vertex = true;
      if (h3_chart_vertex_inside(c, cr.index, x0, y0, x1, y1, &cr))
        continue;
    }
    if (! found || segment_edge_cross_cmp(x0, y0, x1, y1, cr.ax, cr.ay,
        cr.oa, cr.bx, cr.by, cr.ob, exit->ax, exit->ay, exit->oa, exit->bx,
        exit->by, exit->ob) < 0)
      *exit = cr;
    found = true;
  }
  return found;
}

/**
 * @brief Return the parameter at which a segment crosses the line of a
 * crossing, in doubles
 */
static double
h3_crossing_param(const H3Crossing *cr, double x0, double y0, double x1,
  double y1)
{
  double ux = (cr->bx - cr->ax) + (cr->ob - cr->oa), uy = cr->by - cr->ay;
  double f0 = ux * (y0 - cr->ay) - uy * ((x0 - cr->ax) - cr->oa);
  double f1 = ux * (y1 - cr->ay) - uy * ((x1 - cr->ax) - cr->oa);
  double t = (f0 == f1) ? 0.0 : f0 / (f0 - f1);
  return fmin(1.0, fmax(0.0, t));
}

/** @brief Number of cells a planar walk steps through at one parameter
 * before it gives up, at most three cells meeting at a vertex */
#define H3_WALK_SAME_PARAM 6

/**
 * @brief Fill `cells` with every cell a planar segment crosses, and `enter`
 * with the segment parameter at which it reaches each
 * @details The walk of #h3_segment_cells for a planar segment, on the
 * straight-edged outlines: the segment starts in the cell of its first
 * position (#h3_point_cell), leaves each cell where #h3_chart_exit finds,
 * and enters the cell across the edge it leaves by, or at a vertex the cell
 * across the edge on whose side it heads, and from there the third cell
 * meeting at the vertex at the same parameter where it lies in neither
 * first. A segment ending in the convex cell it starts in never leaves it.
 * Each entry states the edge crossed and which cell holds the crossing
 * position: across an edge between its vertices, the one of the two holding
 * the edge (#h3_outline_edge_held), at a vertex, the cells holding the vertex
 * (#h3_outline_vertex_held).
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] resolution H3 resolution
 * @param[out] cells,enter,cross Arrays of at least `maxout` entries, the last
 * one possibly NULL
 * @param[in] maxout Capacity of the arrays, at least 1
 * @return Number of cells written, or 0 when the first lookup fails
 */
static int
h3_segment_cells_planar(double lon1, double lat1, double lon2, double lat2,
  int32 resolution, H3Index *cells, double *enter, H3EdgeCross *cross,
  int maxout)
{
  H3Index cur = h3_point_cell(lon1, lat1, false, resolution);
  if (cur == (H3Index) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;
  if (lon1 == lon2 && lat1 == lat2)
    return n;
  H3Chart chart;
  if (! h3_chart_at_lon(cur, lon1, &chart))
    return n;
  if (h3_chart_convex(&chart) &&
      h3_point_cell(lon2, lat2, false, resolution) == cur)
    return n;

  H3Crossing t, exit;
  memset(&t, 0, sizeof(H3Crossing));
  bool started = false;
  H3Index prev = (H3Index) 0;
  int same = 0;
  while (n < maxout &&
      h3_chart_exit(&chart, lon1, lat1, lon2, lat2, started ? &t : NULL,
        &exit))
  {
    int nv = chart.o.count;
    int k = ((exit.index % nv) + nv) % nv;
    double vx, vy, vo;
    h3_chart_vertex(&chart, exit.index, &vx, &vy, &vo);
    bool at_t = started && segment_edge_cross_cmp(lon1, lat1, lon2, lat2,
      exit.ax, exit.ay, exit.oa, exit.bx, exit.by, exit.ob, t.ax, t.ay, t.oa,
      t.bx, t.by, t.ob) == 0;
    same = at_t ? same + 1 : 0;
    if (same > H3_WALK_SAME_PARAM)
      break;
    H3Index next;
    if (! exit.vertex)
      next = chart.o.across[k];
    else
    {
      /* The cell across the edge leaving the vertex or the edge reaching
       * it, never the one the walk left at this parameter */
      int kp = (k + nv - 1) % nv;
      H3Index first = chart.o.across[k], second = chart.o.across[kp];
      if (exit.back_in || (! exit.along_out && exit.cout >= 0))
      {
        first = chart.o.across[kp];
        second = chart.o.across[k];
      }
      next = (at_t && first == prev) ? second : first;
    }
    H3Chart nc;
    int vk;
    if (next == (H3Index) 0 ||
        ! h3_chart_at_vertex(next, vx, vy, vo, &nc, &vk))
      break;
    cells[n] = next;
    enter[n] = h3_crossing_param(&exit, lon1, lat1, lon2, lat2);
    if (cross)
    {
      H3EdgeCross *x = &cross[n];
      x->ax = exit.ax; x->ay = exit.ay; x->oa = exit.oa;
      x->bx = exit.bx; x->by = exit.by; x->ob = exit.ob;
      if (exit.vertex)
      {
        x->held_left = h3_outline_vertex_held(&chart.o, k);
        x->held_entered = h3_outline_vertex_held(&nc.o, vk);
      }
      else
      {
        x->held_left = h3_outline_edge_held(&chart.o, k);
        x->held_entered = ! x->held_left;
      }
    }
    n++;
    prev = cur;
    cur = next;
    chart = nc;
    t = exit;
    started = true;
  }
  return n;
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
 * outlines of the cells by exact signs (#h3_segment_cells_planar), the
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
 *   (#h3_segment_cells_planar)
 * @param[in] maxout Capacity of the arrays
 * @return Number of cells written, or 0 when the first lookup fails
 */
int
h3_segment_cells(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, int32 resolution, H3Index *cells, double *enter,
  H3EdgeCross *cross, int maxout)
{
  assert(cells); assert(enter);
  if (maxout < 1)
    return 0;
  if (! geodetic)
    return h3_segment_cells_planar(lon1, lat1, lon2, lat2, resolution, cells,
      enter, cross, maxout);
  H3Index cur = h3_point_cell(lon1, lat1, geodetic, resolution);
  if (cur == (H3Index) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;

  DggsArc arc;
  if (! dggs_arc_init(lon1, lat1, lon2, lat2, &arc))
    return n;
  /* A great circle meets a convex cell in a single arc, so a geodetic path
   * whose far endpoint lies in the same cell as its near one never leaves it
   * and there is no boundary to find. That is the common case wherever the
   * positions are closer together than a cell is wide, and reading the
   * boundary for it costs more than the whole answer is worth */
  H3Index end = h3_point_cell(lon2, lat2, geodetic, resolution);
  if (end == cur)
    return n;

  double t = 0.0;
  H3Index prev = (H3Index) 0;
  while (n < maxout && cur != end)
  {
    CellBoundary bnd;
    if (cellToBoundary(cur, &bnd) != E_SUCCESS || bnd.numVerts < 3)
      break;
    uint32 entry = prev ? h3_cell_entry_edges(cur, &bnd, prev) : 0;
    int edge = -1;
    double texit = h3_cell_exit_param(cur, &bnd, entry, &arc, t, &edge);
    if (texit > 1.0 || edge < 0)
      break;                 /* the segment ends inside this cell */
    H3Index next = h3_cell_across_edge(cur, &bnd, edge);
    if (next == (H3Index) 0)
      break;
    cells[n] = next;
    enter[n] = texit;
    n++;
    prev = cur;
    cur = next;
    t = texit;
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