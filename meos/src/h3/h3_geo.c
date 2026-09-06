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
 * Covers every WKT/GSERIALIZED geometry type:
 *
 *   POINT             — single H3 cell via geo_to_h3index_cell.
 *   LINESTRING        — sample each segment at edge_length(res)/2 spacing
 *                       (Nyquist), latLngToCell per sample, dedup.
 *   POLYGON           — outer + holes converted to GeoPolygon in radians,
 *                       polygonToCells, dedup.
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
/* MEOS */
#include <postgres.h>
/* H3 */
#include <h3api.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "h3/h3index.h"
#include "geo/tgeo_spatialfuncs.h"  /* ensure_srid_is_latlong */
#include "temporal/temporal.h"  /* ORDER macro for set_make_free */

/*****************************************************************************
 * Growable buffer of H3Index — accumulator for the recursive walker
 *****************************************************************************/

/**
 * @brief Growable accumulator of H3 cells filled by the geometry walker
 */
typedef struct h3_buf
{
  H3Index *cells;
  int      count;
  int      capacity;
} h3_buf;

/**
 * @brief Initialize an accumulator with a starting capacity
 */
static void
h3_buf_init(h3_buf *buf, int initial_capacity)
{
  buf->capacity = initial_capacity > 0 ? initial_capacity : 64;
  buf->count    = 0;
  buf->cells    = palloc(sizeof(H3Index) * (size_t) buf->capacity);
}

/**
 * @brief Ensure that an accumulator can hold @p additional further cells
 */
static void
h3_buf_grow(h3_buf *buf, int additional)
{
  if (buf->count + additional <= buf->capacity)
    return;
  int new_cap = buf->capacity;
  while (new_cap < buf->count + additional)
    new_cap *= 2;
  buf->cells = repalloc(buf->cells, sizeof(H3Index) * (size_t) new_cap);
  buf->capacity = new_cap;
}

/**
 * @brief Append a cell to an accumulator, ignoring the null cell
 */
static inline void
h3_buf_push(h3_buf *buf, H3Index cell)
{
  if (cell == (H3Index) 0)
    return;
  h3_buf_grow(buf, 1);
  buf->cells[buf->count++] = cell;
}

/**
 * @brief Push the ring of radius one around a cell, that is the cell and the
 * six neighbours `gridDisk(c, 1)` returns
 * @details The ring is the unit by which both covers widen to stay
 * conservative: #polygon_to_cells_into applies it to the cells
 * #polygonToCells returns, which are those whose centre falls inside the
 * polygon, and #linestring_to_cells_into applies it to the cell holding each
 * sample of a segment. In both the omitted cell is one the geometry meets
 * while the test that selected the cells does not see it, and in both such a
 * cell neighbours one that was selected.
 */
static void
h3_buf_push_ring1(h3_buf *out, H3Index c)
{
  if (c == (H3Index) 0)
    return;
  H3Index neighbors[7];   /* gridDisk(_, 1) returns exactly 7 cells */
  memset(neighbors, 0, sizeof(neighbors));
  if (gridDisk(c, 1, neighbors) != E_SUCCESS)
  {
    /* gridDisk failure: fall back to the centre cell */
    h3_buf_push(out, c);
    return;
  }
  for (int i = 0; i < 7; i++)
    if (neighbors[i] != (H3Index) 0)
      h3_buf_push(out, neighbors[i]);
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

/*****************************************************************************
 * Dedup + Set construction
 *
 * Sort by H3Index value (uint64), then linear scan removing adjacent
 * duplicates.  Builds a Datum array and wraps in a Set via set_make_free.
 *****************************************************************************/

/**
 * @brief Compare two H3 cells by index value, the ordering used to sort an
 * accumulator
 */
static int
h3index_compare(const void *a, const void *b)
{
  H3Index av = *(const H3Index *) a;
  H3Index bv = *(const H3Index *) b;
  if (av < bv) return -1;
  if (av > bv) return  1;
  return 0;
}

/**
 * @brief Return the set of the distinct cells of an accumulator, which is freed
 */
static Set *
h3_buf_to_set(h3_buf *buf)
{
  if (buf->count == 0)
  {
    h3_buf_free(buf);
    return NULL;
  }
  qsort(buf->cells, (size_t) buf->count, sizeof(H3Index), h3index_compare);
  /* in-place dedup */
  int n = 1;
  for (int i = 1; i < buf->count; i++)
    if (buf->cells[i] != buf->cells[i - 1])
      buf->cells[n++] = buf->cells[i];
  Datum *datums = palloc(sizeof(Datum) * (size_t) n);
  for (int i = 0; i < n; i++)
    datums[i] = H3IndexGetDatum(buf->cells[i]);
  h3_buf_free(buf);
  return set_make_free(datums, n, T_H3INDEX, ORDER);
}

/*****************************************************************************
 * libh3 sampling step — segment Nyquist spacing in degrees
 *
 * Approximates lat/lng degrees per metre at the equator (1° ≈ 111 320 m).
 * The approximation is conservative away from the equator (longitude
 * degrees shrink); over-sampling is harmless after dedup.  Returned value
 * is in degrees per single sample.
 *****************************************************************************/

/**
 * @brief Return the spacing in degrees at which a segment is sampled for a given
 * resolution
 */
double
h3_sample_step_deg(int32 resolution)
{
  double edge_m;
  if (getHexagonEdgeLengthAvgM(resolution, &edge_m) != E_SUCCESS)
    edge_m = 1000.0;   /* fallback ~1 km */
  /* Nyquist: sample at edge/2, in degrees-per-sample. */
  return (edge_m / 2.0) / 111320.0;
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
 * @brief Return where a segment leaves the cell it currently sits in
 * @details The exit is the nearest crossing of the cell boundary strictly
 * ahead of `tmin`, found by intersecting the segment with each boundary edge
 * in the lon/lat plane the walk is stated in.
 * @return the segment parameter of the exit, or a value above 1 when the
 * segment ends inside the cell
 */
static double
h3_cell_exit_param(H3Index cell, double lon1, double lat1, double dlon,
  double dlat, double tmin)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS || bnd.numVerts < 3)
    return 2.0;
  double best = 2.0;
  for (int i = 0; i < bnd.numVerts; i++)
  {
    int j = (i + 1) % bnd.numVerts;
    double ax = radsToDegs(bnd.verts[i].lng), ay = radsToDegs(bnd.verts[i].lat);
    double bx = radsToDegs(bnd.verts[j].lng), by = radsToDegs(bnd.verts[j].lat);
    double ex = bx - ax, ey = by - ay;
    double den = dlon * ey - dlat * ex;
    if (den == 0.0)
      continue;              /* parallel to this edge */
    double t = ((ax - lon1) * ey - (ay - lat1) * ex) / den;
    double u = ((ax - lon1) * dlat - (ay - lat1) * dlon) / den;
    if (t > tmin && t <= 1.0 && u >= 0.0 && u <= 1.0 && t < best)
      best = t;
  }
  return best;
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
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] resolution H3 resolution
 * @param[out] cells,enter Arrays of at least `maxout` entries; `enter[0]` is
 *   always 0, the parameter of the first endpoint
 * @param[in] maxout Capacity of both arrays
 * @return Number of cells written, or 0 when the first lookup fails
 */
int
h3_segment_cells(double lon1, double lat1, double lon2, double lat2,
  int32 resolution, H3Index *cells, double *enter, int maxout)
{
  assert(cells); assert(enter);
  if (maxout < 1)
    return 0;
  H3Index cur = h3_latlng_deg_to_cell(lat1, lon1, resolution);
  if (cur == (H3Index) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;

  double dlon = lon2 - lon1, dlat = lat2 - lat1;
  double seg = sqrt(dlon * dlon + dlat * dlat);
  if (seg <= 0.0)
    return n;
  /* A nudge past the crossing lands inside the next cell without reaching
   * the one after it: a ten-thousandth of a cell edge is far below the
   * width of any cell and far above the rounding of the crossing itself */
  double edge_m;
  if (getHexagonEdgeLengthAvgM(resolution, &edge_m) != E_SUCCESS)
    edge_m = 1000.0;
  double nudge = (edge_m / 111320.0) * 1e-4 / seg;
  if (nudge <= 0.0 || nudge >= 1.0)
    nudge = 1e-9;

  /* A cell is convex, so a straight segment whose far endpoint lies in the
   * same cell as its near one never leaves it and there is no boundary to
   * find. That is the common case wherever the positions are closer together
   * than a cell is wide, and reading the boundary for it costs more than the
   * whole answer is worth */
  if (h3_latlng_deg_to_cell(lat2, lon2, resolution) == cur)
    return n;

  double t = 0.0;
  while (n < maxout)
  {
    double texit = h3_cell_exit_param(cur, lon1, lat1, dlon, dlat, t);
    if (texit > 1.0)
      break;                 /* the segment ends inside this cell */
    double tn = texit + nudge;
    H3Index next = (H3Index) 0;
    /* A nudge that lands back in the cell just left says the crossing sits
     * within its own rounding, so widen it rather than stall */
    for (int k = 0; k < 8 && tn < 1.0; k++)
    {
      next = h3_latlng_deg_to_cell(lat1 + tn * dlat, lon1 + tn * dlon,
        resolution);
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
 * POINT — single cell.  Uses the existing geo_to_h3index_cell which has
 * the SRID guard.
 *****************************************************************************/

/**
 * @brief Push the cell containing a point into the accumulator
 */
static void
point_to_cells_into(const LWPOINT *lwp, int32 resolution, h3_buf *out)
{
  POINT4D p;
  if (! lwpoint_getPoint4d_p(lwp, &p))
    return;
  H3Index cell = h3_latlng_deg_to_cell(p.y, p.x, resolution);
  h3_buf_push(out, cell);
}

/*****************************************************************************
 * LINESTRING — segment sampling, each sample expanded by one ring.
 *
 * For each adjacent pair of vertices, sample the segment at half a cell edge
 * and emit the ring around the cell holding each sample.
 *
 * THE RING IS WHAT MAKES THE COVER CONSERVATIVE, AND SAMPLING ALONE IS NOT.
 * A cover is read to prune, so it holds every cell the line meets: a cell it
 * omits is a row a caller filtering on the cover never sees. Sampling bounds
 * the distance between consecutive samples, which is a weaker statement than
 * "every cell the segment crosses holds a sample" — a segment clipping the
 * corner of a cell between two samples leaves that cell unsampled, and the
 * finer the resolution the smaller such a corner needs to be.
 *
 * A cell crossed between two consecutive samples lies within half an edge of
 * one of them, so it is that sample's own cell or a neighbour of it, and the
 * ring of radius one around each sample holds it. This is the rule
 * #polygon_to_cells_into already applies to the cells #polygonToCells returns.
 *****************************************************************************/

/**
 * @brief Emit the cells a linestring meets, each sample's ring included
 */
static void
linestring_to_cells_into(const LWLINE *line, int32 resolution, h3_buf *out)
{
  POINTARRAY *pa = line->points;
  if (pa == NULL || pa->npoints == 0)
    return;
  double step_deg = h3_sample_step_deg(resolution);
  if (step_deg <= 0.0)
    step_deg = 1e-5;   /* defensive — finest practical spacing */

  for (uint32_t i = 0; i + 1 < pa->npoints; i++)
  {
    POINT4D p0, p1;
    getPoint4d_p(pa, i,     &p0);
    getPoint4d_p(pa, i + 1, &p1);
    double dx = p1.x - p0.x;
    double dy = p1.y - p0.y;
    double seg_deg = sqrt(dx * dx + dy * dy);
    int nsamples = (int) ceil(seg_deg / step_deg);
    if (nsamples < 1)
      nsamples = 1;
    for (int s = 0; s <= nsamples; s++)
    {
      double t = (double) s / (double) nsamples;
      double lat = p0.y + t * dy;
      double lng = p0.x + t * dx;
      h3_buf_push_ring1(out, h3_latlng_deg_to_cell(lat, lng, resolution));
    }
  }
}

/*****************************************************************************
 * POLYGON — outer ring + holes → GeoPolygon (in radians) → polygonToCells.
 *****************************************************************************/

/**
 * @brief Convert a point array into an H3 geoloop in radians, dropping the
 * repeated closing vertex
 */
static void
pointarray_to_geoloop(const POINTARRAY *pa, GeoLoop *loop)
{
  uint32_t n = pa->npoints;
  /* H3 polygons must NOT repeat the first vertex at the end; drop it if
   * the ring is closed (npoints with last == first). */
  if (n >= 2)
  {
    POINT4D first, last;
    getPoint4d_p(pa, 0,     &first);
    getPoint4d_p(pa, n - 1, &last);
    if (first.x == last.x && first.y == last.y)
      n--;
  }
  loop->numVerts = (int) n;
  loop->verts    = palloc(sizeof(LatLng) * (size_t) (n > 0 ? n : 1));
  for (uint32_t i = 0; i < n; i++)
  {
    POINT4D p;
    getPoint4d_p(pa, i, &p);
    loop->verts[i].lng = degsToRads(p.x);
    loop->verts[i].lat = degsToRads(p.y);
  }
}

/**
 * @brief Free the vertices of a geoloop and reset it to empty
 */
static void
geoloop_free(GeoLoop *loop)
{
  if (loop->verts != NULL)
    pfree(loop->verts);
  loop->verts    = NULL;
  loop->numVerts = 0;
}

/**
 * @brief Push the cells covering an LWPOLY into the accumulator
 * @details Coverage is layered so that the union is a superset of every cell
 * whose interior intersects the polygon:
 *   (a) `polygonToCells` (cells with centroid inside the polygon),
 *       each expanded by `gridDisk(c, 1)` to include boundary cells.
 *   (b) Each polygon vertex's containing cell, also expanded by
 *       `gridDisk(c, 1)`.  Covers polygons that contain no cell
 *       centroid (i.e. polygons smaller than a hexagon at the
 *       chosen resolution).
 * Layers (a) and (b) merge via the sort+dedup in `h3_buf_to_set`.
 */
static void
polygon_to_cells_into(const LWPOLY *poly, int32 resolution, h3_buf *out)
{
  if (poly == NULL || poly->nrings == 0)
    return;
  GeoPolygon gp;
  pointarray_to_geoloop(poly->rings[0], &gp.geoloop);
  gp.numHoles = (int) poly->nrings - 1;
  if (gp.numHoles > 0)
  {
    gp.holes = palloc(sizeof(GeoLoop) * (size_t) gp.numHoles);
    for (int i = 0; i < gp.numHoles; i++)
      pointarray_to_geoloop(poly->rings[i + 1], &gp.holes[i]);
  }
  else
  {
    gp.holes = NULL;
  }

  /* (a) Centroid-containment cells, each expanded by gridDisk(k=1). */
  int64_t max_cells = 0;
  H3Error err = maxPolygonToCellsSize(&gp, resolution, 0, &max_cells);
  if (err == E_SUCCESS && max_cells > 0)
  {
    H3Index *cells = palloc0(sizeof(H3Index) * (size_t) max_cells);
    err = polygonToCells(&gp, resolution, 0, cells);
    if (err == E_SUCCESS)
    {
      for (int64_t i = 0; i < max_cells; i++)
        if (cells[i] != (H3Index) 0)
          h3_buf_push_ring1(out, cells[i]);
    }
    pfree(cells);
  }

  /* (b) Vertex cells, each expanded by gridDisk(k=1). */
  uint32_t nv = gp.geoloop.numVerts;
  for (uint32_t i = 0; i < nv; i++)
  {
    LatLng *ll = &gp.geoloop.verts[i];
    H3Index c;
    if (latLngToCell(ll, resolution, &c) == E_SUCCESS)
      h3_buf_push_ring1(out, c);
  }

  geoloop_free(&gp.geoloop);
  if (gp.holes != NULL)
  {
    for (int i = 0; i < gp.numHoles; i++)
      geoloop_free(&gp.holes[i]);
    pfree(gp.holes);
  }
}

/*****************************************************************************
 * Recursive walker — dispatch any LWGEOM type into the accumulator.
 *****************************************************************************/

/**
 * @brief Push the cells a geometry meets into the accumulator, recursing into
 * its components
 */
static void
lwgeom_to_cells_into(const LWGEOM *geom, int32 resolution, h3_buf *out)
{
  if (geom == NULL)
    return;
  switch (geom->type)
  {
    case POINTTYPE:
      point_to_cells_into((const LWPOINT *) geom, resolution, out);
      break;
    case LINETYPE:
      linestring_to_cells_into((const LWLINE *) geom, resolution, out);
      break;
    case POLYGONTYPE:
      polygon_to_cells_into((const LWPOLY *) geom, resolution, out);
      break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
        lwgeom_to_cells_into(col->geoms[i], resolution, out);
      break;
    }
    default:
      /* TIN / TRIANGLE / CURVE family etc. — silently skip; recursive
       * GeometryCollection callers continue with the components they do
       * understand. */
      break;
  }
}

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @ingroup meos_h3_conversion
 * @brief Return the set of H3 cells covering a static geometry at the given
 * resolution.
 * @details Handles POINT, LINESTRING, POLYGON, and MULTI* / GEOMETRYCOLLECTION
 * combinations recursively.  Unsupported geometry types (TIN, CURVE
 * family, etc.) contribute zero cells; for collections that mix
 * supported and unsupported types, only the supported components
 * contribute.
 *
 * Returns NULL when the geometry is empty, when no valid cells could be
 * produced, or on libh3 error.  The returned Set is owned by the caller
 * and freed via @ref free.
 *
 * @param[in] gs Geometry
 * @param[in] resolution H3 resolution (0..15).
 * @csqlfn #Geo_to_h3indexset()
 */
Set *
geo_to_h3index_set(const GSERIALIZED *gs, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (resolution < 0 || resolution > 15)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_to_h3index_set: resolution must be in [0..15]");
    return NULL;
  }
  if (! ensure_srid_is_latlong(gserialized_get_srid(gs)))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom == NULL)
    return NULL;

  h3_buf buf;
  h3_buf_init(&buf, 64);
  lwgeom_to_cells_into(lwgeom, resolution, &buf);
  lwgeom_free(lwgeom);

  return h3_buf_to_set(&buf);
}

/**
 * @brief Return true if any instant of a temporal sequence holds a value that
 * a set contains
 */
static bool
tsequence_ever_in_set(const TSequence *seq, const Set *s)
{
  for (int i = 0; i < seq->count; i++)
    if (contains_set_value(s, tinstant_value_p(TSEQUENCE_INST_N(seq, i))))
      return true;
  return false;
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
  VALIDATE_NOT_NULL(cells, -1); VALIDATE_NOT_NULL(th3idx, -1);
  /* The value the caller gets for a temporal value of another type is the one
   * #th3index_values gave it, which is the answer that it holds no cell */
  VALIDATE_TH3INDEX(th3idx, 0);

  assert(temptype_subtype(th3idx->subtype));
  switch (th3idx->subtype)
  {
    case TINSTANT:
      return contains_set_value(cells,
        tinstant_value_p((TInstant *) th3idx)) ? 1 : 0;
    case TSEQUENCE:
      return tsequence_ever_in_set((TSequence *) th3idx, cells) ? 1 : 0;
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (TSequenceSet *) th3idx;
      for (int i = 0; i < ss->count; i++)
        if (tsequence_ever_in_set(TSEQUENCESET_SEQ_N(ss, i), cells))
          return 1;
      return 0;
    }
  }
}

/*****************************************************************************/