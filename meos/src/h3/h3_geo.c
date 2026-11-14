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
  DggsOutline *out)
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

/** @brief Number of entries of #h3_vec_cache, a power of 2 */
#define H3_VEC_CACHE 512

/** @brief The geodetic positions read last, per thread, by the bits of
 * their degrees */
static MEOS_TLS struct
{
  uint64 lon, lat;
  double v[3];
  bool used;
} h3_vec_cache[H3_VEC_CACHE];

/**
 * @brief Return in @p v the geodetic position of degrees, the vector of
 * decision D6 of the cell walks (#lonlat_vector_nearest)
 * @details The vector is a function of the doubles of the degrees alone and
 * costs some hundred thousand instructions, while a walk reads the vertices
 * a cell shares with its neighbours again and again, so the positions read
 * last are kept by the bits of their degrees and answer again
 */
static void
h3_lonlat_vector(double lon, double lat, double *v)
{
  uint64 a, b;
  memcpy(&a, &lon, sizeof(uint64));
  memcpy(&b, &lat, sizeof(uint64));
  uint64 h = (a ^ (b * UINT64CONST(0x9E3779B97F4A7C15))) *
    UINT64CONST(0xBF58476D1CE4E5B9);
  int i = (int) (h >> 55) & (H3_VEC_CACHE - 1);
  if (! h3_vec_cache[i].used || h3_vec_cache[i].lon != a ||
      h3_vec_cache[i].lat != b)
  {
    lonlat_vector_nearest(lon, lat, h3_vec_cache[i].v);
    h3_vec_cache[i].lon = a;
    h3_vec_cache[i].lat = b;
    h3_vec_cache[i].used = true;
  }
  memcpy(v, h3_vec_cache[i].v, 3 * sizeof(double));
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

/**
 * @brief Return true if a geodetic cell holds a direction
 * @details The geodetic twin of #h3_outline_holds_point: the cell is the
 * region of the sphere its outline bounds, each edge the great-circle arc
 * between the geodetic positions of its vertices (#h3_outline_vector),
 * holding its interior and its north and west sides. A direction on an edge
 * or at a vertex belongs to the cell holding it
 * (#h3_outline_edge_held_geodetic, #h3_outline_vertex_held_geodetic); one
 * strictly inside or outside is read by the crossing number in the gnomonic
 * chart about the sum c of the vertices, where every great circle is a
 * line: the chart coordinate of v along a direction e is (v . e) / (v . c),
 * so each comparison is the sign of #sphere_dir_dot_diff_sign, and the side
 * of the line of an edge from a to b is the sign of the triple product
 * (a x b) . p. Every sign is exact, so neighbours reading the same vertices
 * hold each direction once.
 * @param[in] o Outline of the cell
 * @param[in] p Direction, exact
 */
static bool
h3_outline_holds_dir(const DggsOutline *o, const SphereDir *p)
{
  int n = o->count;
  SphereDir *v[MAX_CELL_BNDRY_VERTS + 1] = { NULL };
  for (int k = 0; k < n; k++)
  {
    POINT3D q;
    h3_outline_vector(o, k, &q);
    const double d[3] = { q.x, q.y, q.z };
    v[k] = sphere_dir_make(d);
  }
  v[n] = v[0];
  SphereDir *c = sphere_dir_add(v[0], v[1]);
  for (int k = 2; k < n; k++)
  {
    SphereDir *s = sphere_dir_add(c, v[k]);
    sphere_dir_free(c);
    c = s;
  }
  bool result = false, decided = false;
  /* A direction behind the chart lies outside: the cell lies in the open
   * hemisphere about c */
  if (sphere_dir_dot_sign(c, p) <= 0)
    decided = true;
  /* On an edge, or at one of its ends */
  for (int k = 0; k < n && ! decided; k++)
  {
    SphereDir *ne = sphere_dir_cross(v[k], v[k + 1]);
    if (sphere_dir_dot_sign(ne, p) == 0 &&
        sphere_dir_triple_sign(v[k], p, ne) >= 0 &&
        sphere_dir_triple_sign(p, v[k + 1], ne) >= 0)
    {
      decided = true;
      if (sphere_dir_parallel(v[k], p))
        result = h3_outline_vertex_held_geodetic(o, k);
      else if (sphere_dir_parallel(v[k + 1], p))
        result = h3_outline_vertex_held_geodetic(o, (k + 1) % n);
      else
        result = h3_outline_edge_held_geodetic(o, k);
    }
    sphere_dir_free(ne);
  }
  if (! decided)
  {
    /* The axes of the chart: e1 = c x z, or c x x where c lies on the
     * z axis, and e2 = c x e1; the turn of the frame e1, e2, c fixes the
     * side of an edge a point to the right of it lies on */
    const double zd[3] = { 0.0, 0.0, 1.0 }, xd[3] = { 1.0, 0.0, 0.0 };
    SphereDir *z = sphere_dir_make(zd);
    SphereDir *e1 = sphere_dir_cross(c, z);
    if (sphere_dir_parallel(c, z))
    {
      SphereDir *x = sphere_dir_make(xd);
      sphere_dir_free(e1);
      e1 = sphere_dir_cross(c, x);
      sphere_dir_free(x);
    }
    SphereDir *e2 = sphere_dir_cross(c, e1);
    int frame = sphere_dir_triple_sign(e1, e2, c);
    bool inside = false;
    for (int k = 0; k < n; k++)
    {
      /* a above p, and b above p, in the coordinate along e2 */
      bool ua = sphere_dir_dot_diff_sign(v[k], e2, p, c) > 0;
      bool ub = sphere_dir_dot_diff_sign(v[k + 1], e2, p, c) > 0;
      if (ua == ub)
        continue;
      /* The edge meets the horizontal of p right of p when the turn of
       * a, b, p in the chart has the sign of b above a */
      int up = sphere_dir_dot_diff_sign(v[k + 1], e2, v[k], c);
      int turn = frame * sphere_dir_triple_sign(v[k], v[k + 1], p);
      if (turn == up)
        inside = ! inside;
    }
    result = inside;
    sphere_dir_free(z); sphere_dir_free(e1); sphere_dir_free(e2);
  }
  sphere_dir_free(c);
  for (int k = 0; k < n; k++)
    sphere_dir_free(v[k]);
  return result;
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
 * @brief Return true if the position of a geodetic segment at the offset
 * @p m of @p D has not passed the crossing, m / D <= f, exactly, the
 * comparison itself in @p cmp
 * @details The geodetic twin of #segment_edge_side_at, which the planar
 * search reads the same way. An undecided comparison, beyond the precision
 * #geodetic_arc_fraction_cmp reaches, reads as not passed.
 */
static bool
h3_geodetic_before(const double *p0, const double *p1, const double *a,
  const double *b, int64 m, int64 D, int *cmp)
{
  *cmp = geodetic_arc_fraction_cmp(p0, p1, a, b, m, D);
  return *cmp >= 0;
}

/**
 * @brief Return the microsecond at which a geodetic segment crosses the
 * great circle of an edge, as the offset from the start of the segment,
 * truncated
 * @details The geodetic twin of #dggs_outline_edge_offset, searched as
 * #segment_edge_cross_offset searches: the position moves at constant
 * angular speed along the great circle of the segment, so it meets the
 * circle of the edge at the fraction f of the segment the angles give, and
 * the crossing is dated by the truncation of f D, the last offset m with
 * m / D <= f, each comparison exact (#h3_geodetic_before). The fraction in
 * doubles @p f is the start of the search only, which gallops from it and
 * halves. The positions are the geodetic vectors of the degrees
 * (#lonlat_vector_nearest).
 * @param[in] x0,y0,x1,y1 Start and end points of the segment, in degrees
 * @param[in] D Duration of the segment in microseconds
 * @param[in] ax,ay,bx,by Vertices of an edge through the crossing, off the
 * great circle of the segment, in degrees
 * @param[in] f Fraction of the segment at the crossing, in doubles
 * @param[out] whole True when the segment meets the circle exactly at the
 * microsecond returned
 */
int64
h3_geodetic_edge_offset(double x0, double y0, double x1, double y1, int64 D,
  double ax, double ay, double bx, double by, double f, bool *whole)
{
  assert(D >= 0); assert(whole);
  double p0[3], p1[3], a[3], b[3];
  h3_lonlat_vector(x0, y0, p0);
  h3_lonlat_vector(x1, y1, p1);
  h3_lonlat_vector(ax, ay, a);
  h3_lonlat_vector(bx, by, b);
  if (D == 0)
  {
    *whole = (geodetic_arc_fraction_cmp(p0, p1, a, b, 0, 1) == 0);
    return 0;
  }
  /* The estimate in doubles, the start of the search only */
  double est = floor((double) D * f);
  int64 n = 0;
  if (isfinite(est) && est > 0.0)
    n = (est >= (double) D) ? D : llrint(est);
  /* lo is the last offset known not to have passed the crossing, and locmp
   * the comparison read there, 3 where none is; hi the first offset known
   * past it, D + 1 standing for none */
  int64 lo, hi;
  int cmp, locmp = 3;
  if (h3_geodetic_before(p0, p1, a, b, n, D, &cmp))
  {
    lo = n;
    locmp = cmp;
    int64 step = 1;
    for (;;)
    {
      if (lo == D)
      {
        hi = D + 1;
        break;
      }
      int64 c = (step > D - lo) ? D : lo + step;
      if (h3_geodetic_before(p0, p1, a, b, c, D, &cmp))
      {
        lo = c;
        locmp = cmp;
        step *= 2;
      }
      else
      {
        hi = c;
        break;
      }
    }
  }
  else
  {
    hi = n;
    int64 step = 1;
    for (;;)
    {
      int64 c = (step >= hi) ? 0 : hi - step;
      if (c == 0)
      {
        lo = 0;
        break;
      }
      if (h3_geodetic_before(p0, p1, a, b, c, D, &cmp))
      {
        lo = c;
        locmp = cmp;
        break;
      }
      hi = c;
      step *= 2;
    }
  }
  while (hi - lo > 1)
  {
    int64 mid = lo + (hi - lo) / 2;
    if (h3_geodetic_before(p0, p1, a, b, mid, D, &cmp))
    {
      lo = mid;
      locmp = cmp;
    }
    else
      hi = mid;
  }
  if (locmp == 3)
    locmp = geodetic_arc_fraction_cmp(p0, p1, a, b, lo, D);
  *whole = (locmp == 0);
  return lo;
}

/** @brief A point where a geodetic segment meets an edge of a cell */
typedef struct
{
  SphereDir *x;            /**< Direction of the point, exact */
  double f;                /**< Its fraction of the segment, in doubles */
  double ax, ay, bx, by;   /**< An edge through it, off the great circle of
                                the segment, in degrees */
} H3ArcCross;

static void
h3_vec_cross(const double *a, const double *b, double *r)
{
  r[0] = a[1] * b[2] - a[2] * b[1];
  r[1] = a[2] * b[0] - a[0] * b[2];
  r[2] = a[0] * b[1] - a[1] * b[0];
}

/**
 * @brief Return the fraction of the arc from @p p0 to @p p1 at the direction
 * @p x, the ratio of the angles, in doubles
 * @details The order of the crossings and the cells they part are read
 * exactly; the fraction only places a crossing in time, where its
 * microsecond is read exactly (#geodetic_arc_fraction_cmp)
 */
static double
h3_arc_fraction(const double *p0, const double *p1, const double *x)
{
  double c[3], m[3];
  h3_vec_cross(p0, x, c);
  h3_vec_cross(p0, p1, m);
  double ax = atan2(sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]),
    p0[0] * x[0] + p0[1] * x[1] + p0[2] * x[2]);
  double am = atan2(sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]),
    p0[0] * p1[0] + p0[1] * p1[1] + p0[2] * p1[2]);
  double f = am > 0.0 ? ax / am : 0.0;
  return f < 0.0 ? 0.0 : (f > 1.0 ? 1.0 : f);
}

/**
 * @brief Append the points strictly between @p s and the end of a geodetic
 * segment where it meets an edge of an outline
 * @details The segment runs along the great circle of normal m = p0 x p1
 * from p0 to p1; a direction x lies ahead of s on it when (s x x) . m > 0
 * and before p1 when (x x p1) . m > 0. It meets the great circle of the edge
 * from a to b, of normal n, in the direction m x n or its opposite, which
 * lies on the edge when (a x x) . n >= 0 and (x x b) . n >= 0. Where the
 * segment runs along the circle of the edge, the points are the vertices of
 * the edge lying on it. Each point records an edge through it that does not
 * lie on the circle of the segment, which dates it.
 */
static void
h3_arc_outline_crossings(const DggsOutline *o, const double *p0,
  const double *p1, const SphereDir *P1, const SphereDir *M,
  const SphereDir *s, H3ArcCross **out, int *count, int *size)
{
  int n = o->count;
  double v[MAX_CELL_BNDRY_VERTS][3];
  SphereDir *V[MAX_CELL_BNDRY_VERTS] = { NULL };
  for (int k = 0; k < n; k++)
  {
    POINT3D q;
    h3_outline_vector(o, k, &q);
    v[k][0] = q.x; v[k][1] = q.y; v[k][2] = q.z;
    V[k] = sphere_dir_make(v[k]);
  }
  double m[3];
  h3_vec_cross(p0, p1, m);
  for (int k = 0; k < n; k++)
  {
    int k1 = (k + 1) % n;
    SphereDir *ne = sphere_dir_cross(V[k], V[k1]);
    SphereDir *d = sphere_dir_cross(M, ne);
    SphereDir *cand[2] = { NULL, NULL };
    int ends[2] = { -1, -1 };
    if (sphere_dir_parallel(M, ne))
    {
      /* Along the circle of the edge: its vertices, each dated by the edge
       * of the outline meeting it off this circle */
      ends[0] = k; ends[1] = k1;
    }
    else
    {
      cand[0] = d;
      cand[1] = sphere_dir_neg(d);
    }
    for (int j = 0; j < 2; j++)
    {
      const SphereDir *x = ends[j] >= 0 ? V[ends[j]] : cand[j];
      bool keep = sphere_dir_triple_sign(s, x, M) > 0 &&
        sphere_dir_triple_sign(x, P1, M) > 0;
      if (keep && ends[j] < 0)
        keep = sphere_dir_triple_sign(V[k], x, ne) >= 0 &&
          sphere_dir_triple_sign(x, V[k1], ne) >= 0;
      if (! keep)
        continue;
      if (*count == *size)
      {
        *size *= 2;
        *out = repalloc(*out, sizeof(H3ArcCross) * (size_t) *size);
      }
      H3ArcCross *c = &(*out)[(*count)++];
      double xd[3];
      int ea = k, eb = k1;
      if (ends[j] >= 0)
      {
        int w = ends[j];
        xd[0] = v[w][0]; xd[1] = v[w][1]; xd[2] = v[w][2];
        c->x = sphere_dir_add(V[w], V[w]);
        /* The other edge of the outline at this vertex */
        if (w == k)
        {
          ea = (k + n - 1) % n; eb = k;
        }
        else
        {
          ea = k1; eb = (k1 + 1) % n;
        }
      }
      else
      {
        double nd[3];
        h3_vec_cross(v[k], v[k1], nd);
        h3_vec_cross(m, nd, xd);
        if (j == 1)
        {
          xd[0] = -xd[0]; xd[1] = -xd[1]; xd[2] = -xd[2];
        }
        c->x = sphere_dir_add(x, x);
      }
      c->f = h3_arc_fraction(p0, p1, xd);
      c->ax = o->lon[ea]; c->ay = o->lat[ea];
      c->bx = o->lon[eb]; c->by = o->lat[eb];
    }
    sphere_dir_free(ne); sphere_dir_free(d);
    sphere_dir_free(cand[1]);
  }
  for (int k = 0; k < n; k++)
    sphere_dir_free(V[k]);
}

/**
 * @brief Return true if a cell is convex on the sphere, every corner of its
 * outline turning left
 */
static bool
h3_outline_convex_geodetic(const DggsOutline *o)
{
  int n = o->count;
  for (int k = 0; k < n; k++)
  {
    POINT3D p, v, q;
    h3_outline_vector(o, (k + n - 1) % n, &p);
    h3_outline_vector(o, k, &v);
    h3_outline_vector(o, (k + 1) % n, &q);
    if (triple_product_sign_exact(&p, &v, &q) <= 0)
      return false;
  }
  return true;
}

/**
 * @brief Return the cell among @p cand holding a direction, or the one
 * found around @p cur when none of them does
 */
static H3Index
h3_dir_cell(const H3Index *cand, int ncand, H3Index cur,
  const SphereDir *p)
{
  for (int i = 0; i < ncand; i++)
    if (cand[i] != (H3Index) 0 && h3_cell_holds_dir(cand[i], p))
      return cand[i];
  return h3_cell_around(cur, 0.0, 0.0, NULL, p);
}

/**
 * @brief Find an edge of the cells @p cand holding the start @p p0 of a
 * geodetic segment off the great circle of the segment, its vertices in
 * degrees in @p ex (lon, lat of each), and return true if one is found
 * @details The edge that dates a crossing at the start of the segment
 * (#h3_geodetic_edge_offset): @p p0 lies on its circle, the triple product
 * of its vertices and @p p0 being 0, between its vertices, and the edge
 * does not lie on the circle of the segment, the triple products of the
 * ends of the segment and its vertices not both being 0. The twin of the
 * edge test of #h3_arc_outline_crossings.
 */
static bool
h3_start_edge(const H3Index *cand, int ncand, const double *p0,
  const double *p1, double *ex)
{
  POINT3D q0 = { p0[0], p0[1], p0[2] }, q1 = { p1[0], p1[1], p1[2] };
  for (int i = 0; i < ncand; i++)
  {
    DggsOutline co;
    if (cand[i] == (H3Index) 0 || ! h3_cell_outline(cand[i], &co))
      continue;
    int n = co.count;
    for (int k = 0; k < n; k++)
    {
      int k1 = (k + 1) % n;
      POINT3D a, b;
      h3_outline_vector(&co, k, &a);
      h3_outline_vector(&co, k1, &b);
      if (triple_product_sign_exact(&a, &b, &q0) != 0 ||
          (triple_product_sign_exact(&q0, &q1, &a) == 0 &&
           triple_product_sign_exact(&q0, &q1, &b) == 0))
        continue;
      /* Between the vertices: a x p0 and p0 x b point along a x b */
      const double ad[3] = { a.x, a.y, a.z }, bd[3] = { b.x, b.y, b.z };
      SphereDir *A = sphere_dir_make(ad), *B = sphere_dir_make(bd);
      SphereDir *Q = sphere_dir_make(p0), *N = sphere_dir_cross(A, B);
      bool on = sphere_dir_triple_sign(A, Q, N) >= 0 &&
        sphere_dir_triple_sign(Q, B, N) >= 0;
      sphere_dir_free(A); sphere_dir_free(B);
      sphere_dir_free(Q); sphere_dir_free(N);
      if (! on)
        continue;
      ex[0] = co.lon[k]; ex[1] = co.lat[k];
      ex[2] = co.lon[k1]; ex[3] = co.lat[k1];
      return true;
    }
  }
  return false;
}

/**
 * @brief Return the edge through whose interior a geodetic segment leaves a
 * convex cell, -1 when it leaves through none, or -2 when the step is not
 * of that kind
 * @details With m = p0 x p1 and n = a x b the normal of the circle of the
 * edge from a to b, the segment meets that circle in the direction m x n
 * strictly between its ends exactly when p0 lies strictly left of the edge
 * and p1 strictly right of it, and leaves the cell there; that direction
 * lies strictly inside the edge exactly when a lies strictly right of the
 * segment and b strictly left of it. Each side is the exact sign of a
 * triple product of the geodetic positions (#triple_product_sign_exact), and
 * a convex cell is left through one edge at most. Any other step, an end on
 * the circle of an edge, a vertex on the circle of the segment, or a cell
 * that is not convex, is for #h3_segment_cells_geodetic to read.
 */
static int
h3_convex_exit(const DggsOutline *o, const double *p0, const double *p1)
{
  if (! h3_outline_convex_geodetic(o))
    return -2;
  POINT3D q0 = { p0[0], p0[1], p0[2] }, q1 = { p1[0], p1[1], p1[2] }, a, b;
  int n = o->count, found = -1;
  h3_outline_vector(o, 0, &a);
  for (int k = 0; k < n; k++, a = b)
  {
    h3_outline_vector(o, (k + 1) % n, &b);
    int e0 = triple_product_sign_exact(&a, &b, &q0);
    int e1 = triple_product_sign_exact(&a, &b, &q1);
    if (e0 == 0 || e1 == 0)
      return -2;
    if (e0 < 0 || e1 > 0)
      continue;
    int sa = triple_product_sign_exact(&q0, &q1, &a);
    int sb = triple_product_sign_exact(&q0, &q1, &b);
    if (sa == 0 || sb == 0)
      return -2;
    if (sa < 0 && sb > 0)
    {
      if (found >= 0)
        return -2;
      found = k;
    }
  }
  return found;
}

/**
 * @brief Fill `cells` with every cell a geodetic segment crosses, and
 * `enter` with the fraction of the segment at which it reaches each
 * @details The geodetic twin of #dggs_segment_cells_planar, on the cells
 * bounded by great-circle arcs (#h3_outline_holds_dir). The segment starts
 * in the cell of its first position (#h3_point_cell). In a cell, the next
 * stretch of the segment begins at the first point x1 ahead where it meets
 * an edge of the cell or of a neighbour, and ends at the next one x2, or at
 * the end of the segment: no other cell has an edge crossing the segment
 * inside the cell. The stretch lies in the cell holding the direction
 * x1 + x2, which lies between them on the segment, and the point x1 in the
 * cell holding it, which at a vertex can be a third cell, passed for no
 * length. Every point and every order is decided by exact signs on the
 * geodetic positions of the vertices and of the ends of the segment, as the
 * oracle of the cell walks decides them. Each entry states an edge through
 * its point and which cell holds the point: the cell left, the cell entered,
 * or neither.
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] resolution H3 resolution
 * @param[out] cells,enter,cross Arrays of at least `maxout` entries, the last
 * one possibly NULL
 * @param[in] maxout Capacity of the arrays, at least 1
 * @return Number of cells written, `maxout` when they may not all fit, or 0
 * when the first lookup fails
 */
static int
h3_segment_cells_geodetic(double lon1, double lat1, double lon2, double lat2,
  int32 resolution, H3Index *cells, double *enter, DggsEdgeCross *cross,
  int maxout)
{
  H3Index cur = h3_point_cell(lon1, lat1, true, resolution);
  if (cur == (H3Index) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;
  double p0[3], p1[3];
  h3_lonlat_vector(lon1, lat1, p0);
  h3_lonlat_vector(lon2, lat2, p1);
  SphereDir *P0 = sphere_dir_make(p0), *P1 = sphere_dir_make(p1);
  SphereDir *M = sphere_dir_cross(P0, P1);
  bool degenerate = sphere_dir_parallel(P0, P1);
  /* Equal or opposite ends span no great circle; a convex cell holding both
   * ends holds the arc between them */
  const DggsOutline *o = h3_cell_outline_last(cur);
  if (degenerate || ! o || (h3_outline_convex_geodetic(o) &&
      h3_point_cell(lon2, lat2, true, resolution) == cur))
  {
    sphere_dir_free(P0); sphere_dir_free(P1); sphere_dir_free(M);
    return n;
  }
  int size = 32, count = 0;
  H3ArcCross *xs = palloc(sizeof(H3ArcCross) * (size_t) size);
  SphereDir *s = sphere_dir_add(P0, P0);
  H3Index cand[7];
  /* The edge of the last crossing of the common step, whose direction s
   * stands for once a step of another kind reads it */
  bool pending = false, at_start = true;
  double sa[3] = { 0 }, sb[3] = { 0 };
  double m[3];
  h3_vec_cross(p0, p1, m);
  while (true)
  {
    /* The common step: a convex cell left through the interior of an edge
     * (#h3_convex_exit), the next cell the one across it, the crossing held
     * by the one of the two holding the edge */
    o = h3_cell_outline_last(cur);
    int fk = o ? h3_convex_exit(o, p0, p1) : -2;
    if (fk == -1)
      break;
    if (fk >= 0)
    {
      int k1 = (fk + 1) % o->count;
      H3Index next = o->across[fk];
      if (next == (H3Index) 0)
        break;
      if (n == maxout)
        break;
      POINT3D a, b;
      h3_outline_vector(o, fk, &a);
      h3_outline_vector(o, k1, &b);
      sa[0] = a.x; sa[1] = a.y; sa[2] = a.z;
      sb[0] = b.x; sb[1] = b.y; sb[2] = b.z;
      double nd[3], xd[3];
      h3_vec_cross(sa, sb, nd);
      h3_vec_cross(m, nd, xd);
      cells[n] = next;
      enter[n] = h3_arc_fraction(p0, p1, xd);
      if (cross)
      {
        DggsEdgeCross *e = &cross[n];
        e->ax = o->lon[fk]; e->ay = o->lat[fk]; e->oa = 0.0;
        e->bx = o->lon[k1]; e->by = o->lat[k1]; e->ob = 0.0;
        e->held_left = h3_outline_edge_held_geodetic(o, fk);
        e->held_entered = ! e->held_left;
      }
      n++;
      cur = next;
      pending = true;
      at_start = false;
      continue;
    }
    if (pending)
    {
      /* The direction of the last crossing, m x (a x b), exactly */
      SphereDir *A = sphere_dir_make(sa), *B = sphere_dir_make(sb);
      SphereDir *ne = sphere_dir_cross(A, B);
      sphere_dir_free(s);
      s = sphere_dir_cross(M, ne);
      sphere_dir_free(A); sphere_dir_free(B); sphere_dir_free(ne);
      pending = false;
    }
    memset(cand, 0, sizeof(cand));
    if (gridDisk(cur, 1, cand) != E_SUCCESS)
      break;
    count = 0;
    for (int i = 0; i < 7; i++)
    {
      DggsOutline co;
      if (cand[i] != (H3Index) 0 && h3_cell_outline(cand[i], &co))
        h3_arc_outline_crossings(&co, p0, p1, P1, M, s, &xs, &count, &size);
    }
    /* The first point ahead */
    int i1 = 0;
    for (int i = 1; i < count; i++)
      if (sphere_dir_triple_sign(xs[i].x, xs[i1].x, M) > 0)
        i1 = i;
    /* The stretch from the start of the segment: a start on the boundary of
     * its cell can leave the cell at once, the stretch then lying in the
     * cell holding the direction between the start and the first point
     * ahead, entered at the start */
    if (at_start)
    {
      at_start = false;
      SphereDir *mid0 = sphere_dir_add(P0, count ? xs[i1].x : P1);
      H3Index first = h3_dir_cell(cand, 7, cur, mid0);
      sphere_dir_free(mid0);
      double ex[4];
      if (first != (H3Index) 0 && first != cur && n < maxout &&
          h3_start_edge(cand, 7, p0, p1, ex))
      {
        cells[n] = first;
        enter[n] = 0.0;
        if (cross)
        {
          DggsEdgeCross *e = &cross[n];
          e->ax = ex[0]; e->ay = ex[1]; e->oa = 0.0;
          e->bx = ex[2]; e->by = ex[3]; e->ob = 0.0;
          e->held_left = true;
          e->held_entered = false;
        }
        n++;
        cur = first;
      }
    }
    if (count == 0)
      break;
    /* The next point after the first */
    int i2 = -1;
    for (int i = 0; i < count; i++)
      if (sphere_dir_triple_sign(xs[i1].x, xs[i].x, M) > 0 &&
          (i2 < 0 || sphere_dir_triple_sign(xs[i].x, xs[i2].x, M) > 0))
        i2 = i;
    SphereDir *mid = sphere_dir_add(xs[i1].x, i2 >= 0 ? xs[i2].x : P1);
    H3Index next = h3_dir_cell(cand, 7, cur, mid);
    H3Index at = h3_dir_cell(cand, 7, cur, xs[i1].x);
    sphere_dir_free(mid);
    if (next == (H3Index) 0 || at == (H3Index) 0)
      break;
    H3Index step[2] = { at, next };
    bool full = false;
    for (int j = 0; j < 2; j++)
    {
      H3Index c = step[j];
      if (c == cur || (j == 1 && c == at))
        continue;
      if (n == maxout)
      {
        full = true;
        break;
      }
      cells[n] = c;
      enter[n] = xs[i1].f;
      if (cross)
      {
        DggsEdgeCross *e = &cross[n];
        e->ax = xs[i1].ax; e->ay = xs[i1].ay; e->oa = 0.0;
        e->bx = xs[i1].bx; e->by = xs[i1].by; e->ob = 0.0;
        e->held_left = (at == cur);
        e->held_entered = (at == c);
      }
      n++;
      cur = c;
    }
    sphere_dir_free(s);
    s = sphere_dir_add(xs[i1].x, xs[i1].x);
    for (int i = 0; i < count; i++)
      sphere_dir_free(xs[i].x);
    if (full)
      break;
    cur = next;
  }
  sphere_dir_free(s);
  pfree(xs);
  sphere_dir_free(P0); sphere_dir_free(P1); sphere_dir_free(M);
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