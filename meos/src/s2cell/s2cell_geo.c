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
 * @brief Geometry adapters between an S2 cell and PostGIS geometries.
 *
 * These typed wrappers keep the geometry construction inside MEOS so the SQL
 * cell/geometry conversions are pure catalog projections and the PG V1
 * wrappers stay thin. The pure cell kernel meos/src/s2cell/s2cell.c carries no
 * geometry dependency; the lon/lat coupling lives here, mirroring the quadbin
 * split between quadbin.c and quadbin_geo.c.
 *
 * An S2 cell is defined on the sphere, so its centre and its boundary are
 * geodetic and its four edges are geodesics rather than the straight segments
 * a planar grid carries.
 */

#include "s2cell/s2cell.h"

/* C */
#include <stdlib.h>
#include <string.h>
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeodetic.h>  /* lwpoly_covers_point2d */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>  /* GSERIALIZED_POINT2D_P */
#include <meos_s2cell.h>
#include "geo/geo_funcs.h"  /* ensure_geodetic_geo */
#include "geo/tgeo_spatialfuncs.h"
#include "temporal/set.h"  /* ensure_set_isof_type */
#include "temporal/temporal.h"  /* ORDER macro for set_make_free */
#include "temporal/tcellindex.h"

/*****************************************************************************
 * Geometry to cell
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the S2 cell covering a lon/lat point at a level
 * @param[in] point Point geometry in a lon/lat (SRID 4326) reference system
 * @param[in] level S2 level
 * @csqlfn #S2cell_point_to_cell()
 */
S2CellId
geo_to_s2cell_cell(const GSERIALIZED *point, int32 level)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(point, (S2CellId) 0);
  if (! ensure_point_type(point) || ! ensure_not_empty(point) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(point)) ||
      ! ensure_valid_cell_resolution(T_TS2CELL, level))
    return (S2CellId) 0;
  const POINT2D *p = GSERIALIZED_POINT2D_P(point);
  return s2cell_point_to_cell(p->x, p->y, (uint32_t) level);
}

/*****************************************************************************
 * Cell to geometry
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the centre of an S2 cell as a geodetic point (SRID 4326)
 * @param[in] cell S2 cell
 * @csqlfn #S2cell_cell_to_point()
 */
GSERIALIZED *
s2cell_cell_to_geogpoint(S2CellId cell)
{
  double lon, lat;
  s2cell_cell_point(cell, &lon, &lat);
  /* The cell is defined on the sphere, so the centre is geodetic */
  return geopoint_make(lon, lat, 0.0, false, true, SRID_DEFAULT);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the boundary of an S2 cell as a geodetic polygon (SRID 4326)
 * @details The four vertices are the corners of the cell on the sphere and the
 * ring closes on the first of them. The edges joining them are geodesics, so
 * the polygon states the cell exactly only when read as a geography.
 * @param[in] cell S2 cell
 * @csqlfn #S2cell_cell_to_boundary()
 */
GSERIALIZED *
s2cell_cell_to_geog(S2CellId cell)
{
  double lons[4], lats[4];
  s2cell_cell_vertices(cell, lons, lats);
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE, 5);
  POINT4D pt;
  pt.z = 0.0; pt.m = 0.0;
  for (int k = 0; k < 4; k++)
  {
    pt.x = lons[k]; pt.y = lats[k];
    ptarray_append_point(pa, &pt, LW_TRUE);
  }
  pt.x = lons[0]; pt.y = lats[0];
  ptarray_append_point(pa, &pt, LW_TRUE); /* close the ring */
  LWPOLY *poly = lwpoly_construct_empty(SRID_DEFAULT, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  FLAGS_SET_GEODETIC(poly->flags, 1);
  GSERIALIZED *gs = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return gs;
}

/*****************************************************************************
 * Geography to cell set
 *
 * The cover of a geography is every cell it meets. An S2 cell is bounded by
 * four arcs of great circles, the path a geodetic segment follows, so the
 * cells a segment passes through are those the traversal of
 * #s2cell_segment_cells enters. The cells a polygon encloses follow from the
 * cells of its rings and from the hierarchy: a cell holding no boundary cell
 * among its descendants meets no boundary point, so it lies wholly inside the
 * polygon or wholly outside it, and one point-in-polygon test at its centre
 * answers for all its descendants. A cell holding one is split into its four
 * children, down to the level of the cover.
 *****************************************************************************/

/** @brief Number of cells a cover gathers at most, the count at which a
 * request states a level far finer than the geography it covers */
#define S2CELL_MAX_COVER_CELLS 4194304

/**
 * @brief Growable accumulator of S2 cells filled by the geography walker
 * @details The limit is enforced as the cells arrive, since a polygon at a
 * level far finer than its extent encloses more cells than memory holds
 */
typedef struct
{
  S2CellId *cells;
  int       count;
  int       capacity;
  bool      overflow;   /**< True once the cover would pass the limit */
  bool      error;      /**< True once a component raised an error */
} s2cell_buf;

/**
 * @brief Initialize an accumulator with a starting capacity
 */
static void
s2cell_buf_init(s2cell_buf *buf, int initial_capacity)
{
  buf->capacity = initial_capacity > 0 ? initial_capacity : 64;
  buf->count    = 0;
  buf->overflow = false;
  buf->error    = false;
  buf->cells    = palloc(sizeof(S2CellId) * (size_t) buf->capacity);
}

/**
 * @brief Make room for @p additional further cells in an accumulator
 * @return False, and the accumulator marked as overflowing, when the cells
 * would pass #S2CELL_MAX_COVER_CELLS
 */
static bool
s2cell_buf_reserve(s2cell_buf *buf, long additional)
{
  if (buf->overflow)
    return false;
  if ((long) buf->count + additional > S2CELL_MAX_COVER_CELLS)
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
  buf->cells = repalloc(buf->cells, sizeof(S2CellId) * (size_t) new_cap);
  buf->capacity = new_cap;
  return true;
}

/**
 * @brief Append a cell to an accumulator, ignoring the null cell
 */
static inline void
s2cell_buf_push(s2cell_buf *buf, S2CellId cell)
{
  if (cell != (S2CellId) 0 && s2cell_buf_reserve(buf, 1))
    buf->cells[buf->count++] = cell;
}

/**
 * @brief Free the cells held by an accumulator and reset it to empty
 */
static void
s2cell_buf_free(s2cell_buf *buf)
{
  if (buf->cells != NULL)
    pfree(buf->cells);
  buf->cells    = NULL;
  buf->count    = 0;
  buf->capacity = 0;
}

/**
 * @brief Return the set of the distinct cells of an accumulator, which is freed
 * @details The set orders its values and drops the duplicates, which the
 * accumulator holds wherever two segments pass through one cell
 */
static Set *
s2cell_buf_to_set(s2cell_buf *buf)
{
  if (buf->count == 0)
  {
    s2cell_buf_free(buf);
    return NULL;
  }
  int count = buf->count;
  Datum *datums = palloc(sizeof(Datum) * (size_t) count);
  for (int i = 0; i < count; i++)
    datums[i] = S2CellGetDatum(buf->cells[i]);
  s2cell_buf_free(buf);
  return set_make_free(datums, count, T_S2CELL, ORDER);
}

/**
 * @brief Push the cells a geodetic segment passes through into the
 * accumulator
 * @details The traversal writes into arrays that grow until the whole segment
 * fits, and the cell of the far endpoint closes the segment when the
 * traversal stopped short of it, as for an endpoint lying on a cell boundary
 * within the rounding of the crossing, as #tgeogpoint_to_ts2cell reads it
 */
static void
segment_to_cells_into(double lon1, double lat1, double lon2, double lat2,
  int32 level, s2cell_buf *out)
{
  int maxout = 64, ncells;
  S2CellId *cells = palloc(sizeof(S2CellId) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  while ((ncells = s2cell_segment_cells(lon1, lat1, lon2, lat2, true,
      (uint32_t) level, cells, enter, maxout)) == maxout)
  {
    if (maxout >= S2CELL_MAX_COVER_CELLS)
    {
      out->overflow = true;
      pfree(cells); pfree(enter);
      return;
    }
    maxout *= 2;
    cells = repalloc(cells, sizeof(S2CellId) * (size_t) maxout);
    enter = repalloc(enter, sizeof(double) * (size_t) maxout);
  }
  for (int k = 0; k < ncells; k++)
    s2cell_buf_push(out, cells[k]);
  s2cell_buf_push(out, s2cell_point_to_cell(lon2, lat2, (uint32_t) level));
  pfree(cells); pfree(enter);
}

/**
 * @brief Push the cells a point array passes through into the accumulator
 */
static void
pointarray_to_cells_into(const POINTARRAY *pa, int32 level, s2cell_buf *out)
{
  if (pa == NULL || pa->npoints == 0)
    return;
  const POINT2D *p = getPoint2d_cp(pa, 0);
  s2cell_buf_push(out, s2cell_point_to_cell(p->x, p->y, (uint32_t) level));
  for (uint32_t i = 0; i + 1 < pa->npoints && ! out->overflow; i++)
  {
    const POINT2D *p0 = getPoint2d_cp(pa, i);
    const POINT2D *p1 = getPoint2d_cp(pa, i + 1);
    segment_to_cells_into(p0->x, p0->y, p1->x, p1->y, level, out);
  }
}

/**
 * @brief Compare two S2 cells by identifier, the ordering of the boundary
 * cells the range test searches
 */
static int
s2cell_id_compare(const void *a, const void *b)
{
  S2CellId av = *(const S2CellId *) a;
  S2CellId bv = *(const S2CellId *) b;
  return (av < bv) ? -1 : ((av > bv) ? 1 : 0);
}

/**
 * @brief Return true if a sorted array of cell identifiers holds one in the
 * inclusive range [@p lo, @p hi]
 */
static bool
s2cell_ids_in_range(const S2CellId *ids, int count, S2CellId lo, S2CellId hi)
{
  int first = 0, last = count;
  while (first < last)
  {
    int mid = first + (last - first) / 2;
    if (ids[mid] < lo)
      first = mid + 1;
    else
      last = mid;
  }
  return first < count && ids[first] <= hi;
}

/**
 * @brief Push the cells of a level that a polygon encloses within a cell
 * @details The descendants of a cell are the identifiers of its range, so the
 * cell holds a boundary cell of the level exactly when one lies in that
 * range. A cell holding none meets no boundary point: it lies wholly inside
 * the polygon or wholly outside it, and its centre decides which
 * @param[in] poly Polygon, carrying its geodetic bounding box
 * @param[in] cell Cell to read
 * @param[in] level Level of the cover
 * @param[in] bnd,nbnd Sorted cells of the rings at the level
 * @param[in,out] out Accumulator
 */
static void
polygon_fill_cell(const LWPOLY *poly, S2CellId cell, int32 level,
  const S2CellId *bnd, int nbnd, s2cell_buf *out)
{
  if (out->overflow)
    return;
  uint64 lsb = s2cell_lsb(cell);
  if (s2cell_ids_in_range(bnd, nbnd, s2cell_range_min(cell),
      s2cell_range_max(cell)))
  {
    /* A boundary cell of the level is in the cover already */
    if (s2cell_get_resolution(cell) >= (uint32_t) level)
      return;
    S2CellId child = cell - lsb + (lsb >> 2);
    for (int k = 0; k < 4; k++, child += lsb >> 1)
      polygon_fill_cell(poly, child, level, bnd, nbnd, out);
    return;
  }
  double lon, lat;
  s2cell_cell_point(cell, &lon, &lat);
  POINT2D pt;
  pt.x = lon; pt.y = lat;
  if (! lwpoly_covers_point2d(poly, &pt))
    return;
  uint64 lsb_level = s2cell_lsb_for_level((uint32_t) level);
  long count = (long) (lsb / lsb_level);
  if (! s2cell_buf_reserve(out, count))
    return;
  S2CellId first = cell - lsb + lsb_level;
  for (long k = 0; k < count; k++)
    out->cells[out->count++] = first + (S2CellId) k * 2 * lsb_level;
}

/**
 * @brief Push the cells a polygon meets into the accumulator
 * @details The rings give the cells the boundary passes through, and reading
 * the six cube faces down the hierarchy against them gives the cells the
 * polygon encloses
 */
static void
polygon_to_cells_into(LWPOLY *poly, int32 level, s2cell_buf *out)
{
  if (lwpoly_is_empty(poly))
    return;
  int first = out->count;
  for (uint32_t i = 0; i < poly->nrings && ! out->overflow; i++)
    pointarray_to_cells_into(poly->rings[i], level, out);
  int nbnd = out->count - first;
  if (nbnd == 0 || out->overflow)
    return;

  S2CellId *bnd = palloc(sizeof(S2CellId) * (size_t) nbnd);
  memcpy(bnd, out->cells + first, sizeof(S2CellId) * (size_t) nbnd);
  qsort(bnd, (size_t) nbnd, sizeof(S2CellId), s2cell_id_compare);
  /* The point-in-polygon test reads the box of the polygon, computed once */
  lwgeom_add_bbox(lwpoly_as_lwgeom(poly));
  for (uint64 face = 0; face < 6 && ! out->overflow; face++)
    polygon_fill_cell(poly, (face << S2_FACE_SHIFT) + s2cell_lsb_for_level(0),
      level, bnd, nbnd, out);
  pfree(bnd);
}

/**
 * @brief Push the cells a geography meets into the accumulator, recursing
 * into its components
 */
static void
lwgeom_to_cells_into(LWGEOM *geom, int32 level, s2cell_buf *out)
{
  if (geom == NULL || lwgeom_is_empty(geom))
    return;
  switch (geom->type)
  {
    case POINTTYPE:
    {
      const POINT2D *p = getPoint2d_cp(((const LWPOINT *) geom)->point, 0);
      s2cell_buf_push(out, s2cell_point_to_cell(p->x, p->y,
        (uint32_t) level));
      break;
    }
    case LINETYPE:
      pointarray_to_cells_into(((const LWLINE *) geom)->points, level, out);
      break;
    case POLYGONTYPE:
      polygon_to_cells_into((LWPOLY *) geom, level, out);
      break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      LWCOLLECTION *col = (LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms && ! out->overflow && ! out->error;
          i++)
        lwgeom_to_cells_into(col->geoms[i], level, out);
      break;
    }
    default:
      /* A cover omitting the cells of a component would drop a trajectory
       * the prefilter must keep, so a type the walk does not state is refused
       * rather than read as meeting no cell */
      out->error = true;
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "The cover of a geometry of type %s is not supported",
        lwtype_name(geom->type));
      return;
  }
}

/**
 * @ingroup meos_s2cell
 * @brief Return the set of S2 cells covering a geography at the given level
 * @details The cover holds every cell the geography meets and no other: a
 * point gives the cell holding it, a line the cells its geodesic segments pass
 * through, and a polygon those of its rings together with the cells they
 * enclose. MULTI* and GEOMETRYCOLLECTION values give the union of the cells of
 * their components, and any other type, alone or inside a collection, is
 * refused. An empty geography meets no cell and gives `NULL`.
 * @param[in] gs Geography
 * @param[in] level S2 level
 * @errval NULL
 * @csqlfn #Geo_to_s2cellset()
 */
Set *
geo_to_s2cell_set(const GSERIALIZED *gs, int32 level)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_cell_resolution(T_TS2CELL, level) ||
      ! ensure_geodetic_geo(gs) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(gs)))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom == NULL)
    return NULL;
  s2cell_buf buf;
  s2cell_buf_init(&buf, 64);
  lwgeom_to_cells_into(lwgeom, level, &buf);
  lwgeom_free(lwgeom);
  if (buf.error)
  {
    s2cell_buf_free(&buf);
    return NULL;
  }
  if (buf.overflow)
  {
    s2cell_buf_free(&buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The cover of the geometry at level %d exceeds %d cells",
      level, S2CELL_MAX_COVER_CELLS);
    return NULL;
  }
  return s2cell_buf_to_set(&buf);
}

/*****************************************************************************
 * Membership of a temporal cell in a cell set
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return true if a temporal S2 cell ever takes a cell of an S2 cell set
 * @details Returns 1 if any cell of @p cells appears among the values of
 * @p temp, 0 if none does, and -1 on error. Read with #geo_to_s2cell_set it
 * is the spatial prefilter of the exact `eIntersects`: the cover holds every
 * cell the geography meets, so a trajectory the test drops meets no cell of it
 * @param[in] cells Set of S2 cells
 * @param[in] temp Temporal S2 cell
 * @csqlfn #Ever_eq_s2cellset_ts2cell()
 */
int
ever_eq_s2cellset_ts2cell(const Set *cells, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_S2CELLSET(cells, -1); VALIDATE_TS2CELL(temp, -1);
  return tcellindex_ever_in_set(temp, cells) ? 1 : 0;
}

/*****************************************************************************/
