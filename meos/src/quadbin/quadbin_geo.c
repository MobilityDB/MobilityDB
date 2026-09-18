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
 * @brief Geometry adapters between a QUADBIN cell and PostGIS geometries.
 *
 * These typed wrappers keep the geometry construction inside MEOS so the
 * SQL cell/geometry conversions are pure catalog projections (the PG V1
 * wrappers in mobilitydb/src/quadbin/quadbin_ops.c are thin). The cover of a
 * geometry by the cells it meets, and the test of a temporal cell against
 * such a cover, live here too, mirroring meos/src/h3/h3_geo.c. The pure
 * cell kernel meos/src/quadbin/quadbin.c stays free of any geometry
 * dependency; the lon/lat coupling lives here, mirroring the h3 split
 * between h3index.c and th3index_latlng.c.
 */

#include "quadbin/quadbin.h"

/* C */
#include <math.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>  /* lwpoly_contains_point, LW_INSIDE */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>  /* GSERIALIZED_POINT2D_P */
#include <meos_quadbin.h>
#include "geo/tgeo_spatialfuncs.h"
#include "temporal/set.h"  /* ensure_set_isof_type */
#include "temporal/temporal.h"  /* ORDER macro for set_make_free */
#include "temporal/tcellindex.h"

/*****************************************************************************
 * Geometry to cell
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the quadbin cell covering a lon/lat point at a resolution
 * @param[in] point Point geometry in a lon/lat (SRID 4326) reference system
 * @param[in] resolution Quadbin resolution
 * @csqlfn #Quadbin_point_to_cell()
 */
Quadbin
geo_to_quadbin_cell(const GSERIALIZED *point, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(point, (Quadbin) 0);
  if (! ensure_point_type(point) || ! ensure_not_empty(point) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(point)) ||
      ! ensure_valid_cell_resolution(T_TQUADBIN, resolution))
    return (Quadbin) 0;
  const POINT2D *p = GSERIALIZED_POINT2D_P(point);
  return quadbin_point_to_cell(p->x, p->y, (uint32_t) resolution);
}

/*****************************************************************************
 * Cell to geometry
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the centroid of a quadbin cell as a lon/lat point (SRID 4326)
 * @param[in] cell Quadbin cell
 * @csqlfn #Quadbin_cell_to_point()
 */
GSERIALIZED *
quadbin_cell_to_geompoint(Quadbin cell)
{
  double lon, lat;
  quadbin_cell_point(cell, &lon, &lat);
  /* Planar (non-geodetic) lon/lat point */
  return geopoint_make(lon, lat, 0.0, false, false, 4326);
}

/**
 * @ingroup meos_quadbin
 * @brief Return the boundary of a quadbin cell as a square polygon (SRID 4326)
 * @details A quadbin cell is an axis-aligned square tile, so in a lon/lat
 * reference system its boundary polygon coincides with its envelope; it is
 * built from the cell extent as a closed 5-point ring.
 * @param[in] cell Quadbin cell
 * @csqlfn #Quadbin_cell_to_boundary()
 */
GSERIALIZED *
quadbin_cell_to_geom(Quadbin cell)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE, 5);
  POINT4D pt;
  pt.z = 0.0; pt.m = 0.0;
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE); /* close */
  LWPOLY *poly = lwpoly_construct_empty(4326, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  GSERIALIZED *gs = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return gs;
}

/*****************************************************************************
 * Geometry to cell set
 *
 * The cover of a geometry is every tile the geometry meets, which the tile
 * grid states exactly: a QUADBIN tile is bounded by two meridians and two
 * parallels, so in longitude and latitude it is an axis-aligned rectangle and
 * a segment leaves it at a boundary the walk of #quadbin_segment_cells already
 * finds. A ring's tiles are therefore the tiles its segments pass through, and
 * the tiles enclosed by the ring follow from them: between two tiles of a row
 * that the boundary does not meet, no tile holds a boundary point, so each is
 * wholly inside the polygon or wholly outside it, and one point-in-polygon
 * test at the centre of the first answers for all of them.
 *****************************************************************************/

/** @brief Number of cells a cover gathers at most, the count at which a
 * request states a resolution far finer than the geometry it covers */
#define QUADBIN_MAX_COVER_CELLS 4194304

/**
 * @brief Growable accumulator of QUADBIN cells filled by the geometry walker
 * @details The limit is enforced as the cells arrive, since a polygon at a
 * resolution far finer than its extent encloses more tiles than memory holds
 */
typedef struct
{
  Quadbin *cells;
  int      count;
  int      capacity;
  bool     overflow;   /**< True once the cover would pass the limit */
  bool     error;      /**< True once a component raised an error */
} quadbin_buf;

/**
 * @brief Initialize an accumulator with a starting capacity
 */
static void
quadbin_buf_init(quadbin_buf *buf, int initial_capacity)
{
  buf->capacity = initial_capacity > 0 ? initial_capacity : 64;
  buf->count    = 0;
  buf->overflow = false;
  buf->error    = false;
  buf->cells    = palloc(sizeof(Quadbin) * (size_t) buf->capacity);
}

/**
 * @brief Make room for @p additional further cells in an accumulator
 * @return False, and the accumulator marked as overflowing, when the cells
 * would pass #QUADBIN_MAX_COVER_CELLS
 */
static bool
quadbin_buf_reserve(quadbin_buf *buf, long additional)
{
  if (buf->overflow)
    return false;
  if ((long) buf->count + additional > QUADBIN_MAX_COVER_CELLS)
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
  buf->cells = repalloc(buf->cells, sizeof(Quadbin) * (size_t) new_cap);
  buf->capacity = new_cap;
  return true;
}

/**
 * @brief Append a cell to an accumulator
 */
static inline void
quadbin_buf_push(quadbin_buf *buf, Quadbin cell)
{
  if (quadbin_buf_reserve(buf, 1))
    buf->cells[buf->count++] = cell;
}

/**
 * @brief Free the cells held by an accumulator and reset it to empty
 */
static void
quadbin_buf_free(quadbin_buf *buf)
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
 * accumulator holds wherever two segments pass through one tile
 */
static Set *
quadbin_buf_to_set(quadbin_buf *buf)
{
  if (buf->count == 0)
  {
    quadbin_buf_free(buf);
    return NULL;
  }
  int count = buf->count;
  Datum *datums = palloc(sizeof(Datum) * (size_t) count);
  for (int i = 0; i < count; i++)
    datums[i] = QuadbinGetDatum(buf->cells[i]);
  quadbin_buf_free(buf);
  return set_make_free(datums, count, T_QUADBIN, ORDER);
}

/**
 * @brief Push the tiles a segment passes through into the accumulator
 * @details The walk is the one a moving point takes between two instants:
 * a segment crosses one tile per column and one per row between the tiles of
 * its endpoints, and the corner where four tiles meet adds the one the path
 * holds at the crossing
 */
static void
segment_to_cells_into(double lon1, double lat1, double lon2, double lat2,
  int32 resolution, quadbin_buf *out)
{
  uint32_t xa, ya, xb, yb, z;
  quadbin_cell_tile(quadbin_point_to_cell(lon1, lat1, (uint32_t) resolution),
    &xa, &ya, &z);
  quadbin_cell_tile(quadbin_point_to_cell(lon2, lat2, (uint32_t) resolution),
    &xb, &yb, &z);
  long span = labs((long) xb - (long) xa) + labs((long) yb - (long) ya) + 2;
  if (! quadbin_buf_reserve(out, span))
    return;
  double *enter = palloc(sizeof(double) * (size_t) span);
  out->count += quadbin_segment_cells(lon1, lat1, lon2, lat2, false,
    (uint32_t) resolution, out->cells + out->count, enter, (int) span);
  pfree(enter);
}

/**
 * @brief Push the tiles a point array passes through into the accumulator
 */
static void
pointarray_to_cells_into(const POINTARRAY *pa, int32 resolution,
  quadbin_buf *out)
{
  if (pa == NULL || pa->npoints == 0)
    return;
  if (pa->npoints == 1)
  {
    const POINT2D *p = getPoint2d_cp(pa, 0);
    quadbin_buf_push(out,
      quadbin_point_to_cell(p->x, p->y, (uint32_t) resolution));
    return;
  }
  for (uint32_t i = 0; i + 1 < pa->npoints && ! out->overflow; i++)
  {
    const POINT2D *p0 = getPoint2d_cp(pa, i);
    const POINT2D *p1 = getPoint2d_cp(pa, i + 1);
    segment_to_cells_into(p0->x, p0->y, p1->x, p1->y, resolution, out);
  }
}

/**
 * @brief One tile of the boundary of a polygon, by the column and the row it
 * sits in
 */
typedef struct
{
  uint32_t x;
  uint32_t y;
} quadbin_tile;

/**
 * @brief Compare two tiles by row and then by column, the ordering that reads
 * the boundary of a polygon row by row
 */
static int
quadbin_tile_compare(const void *a, const void *b)
{
  const quadbin_tile *ta = (const quadbin_tile *) a;
  const quadbin_tile *tb = (const quadbin_tile *) b;
  if (ta->y != tb->y)
    return (ta->y < tb->y) ? -1 : 1;
  if (ta->x != tb->x)
    return (ta->x < tb->x) ? -1 : 1;
  return 0;
}

/**
 * @brief Push the tiles of a row that a polygon holds, when it holds them
 * @details The tiles of the interval meet no boundary of the polygon, so each
 * is wholly inside it or wholly outside it, and adjacent ones agree: to differ
 * they would share an edge the boundary crosses, which would put that boundary
 * in both. The centre of the first therefore answers for the interval
 */
static void
row_interval_into(const LWPOLY *poly, long xa, long xb, uint32_t row,
  int32 resolution, quadbin_buf *out)
{
  if (xa > xb)
    return;
  Quadbin first = quadbin_tile_to_cell((uint32_t) xa, row,
    (uint32_t) resolution);
  double lon, lat;
  quadbin_cell_point(first, &lon, &lat);
  POINT2D pt;
  pt.x = lon; pt.y = lat;
  if (lwpoly_contains_point(poly, &pt) == LW_OUTSIDE ||
      ! quadbin_buf_reserve(out, xb - xa + 1))
    return;
  for (long x = xa; x <= xb; x++)
    out->cells[out->count++] = quadbin_tile_to_cell((uint32_t) x, row,
      (uint32_t) resolution);
}

/**
 * @brief Push the tiles a polygon meets into the accumulator
 * @details The rings give the tiles the boundary passes through, and the rows
 * of those tiles give the tiles the polygon encloses. Every row the polygon
 * meets holds a boundary tile: from a point of the polygon in the row, moving
 * west along the row leaves the polygon, which is bounded, at a boundary point
 * of the same row. For the same reason no tile west of the first boundary tile
 * of a row, or east of the last, lies inside the polygon, so the tiles it
 * encloses are those of the intervals between two boundary tiles of a row
 */
static void
polygon_to_cells_into(const LWPOLY *poly, int32 resolution, quadbin_buf *out)
{
  if (lwpoly_is_empty(poly))
    return;

  /* (a) The tiles the rings pass through */
  int first = out->count;
  for (uint32_t i = 0; i < poly->nrings; i++)
    pointarray_to_cells_into(poly->rings[i], resolution, out);
  int nbnd = out->count - first;
  if (nbnd == 0 || out->overflow)
    return;

  /* (b) The tiles the rings enclose, read row by row */
  quadbin_tile *tiles = palloc(sizeof(quadbin_tile) * (size_t) nbnd);
  for (int i = 0; i < nbnd; i++)
  {
    uint32_t z;
    quadbin_cell_tile(out->cells[first + i], &tiles[i].x, &tiles[i].y, &z);
  }
  qsort(tiles, (size_t) nbnd, sizeof(quadbin_tile), quadbin_tile_compare);

  int i = 0;
  while (i < nbnd && ! out->overflow)
  {
    uint32_t row = tiles[i].y;
    int j = i;
    while (j < nbnd && tiles[j].y == row)
      j++;
    /* The intervals between two consecutive boundary tiles of the row */
    for (int k = i + 1; k < j; k++)
    {
      if (tiles[k].x > tiles[k - 1].x + 1)
        row_interval_into(poly, (long) tiles[k - 1].x + 1,
          (long) tiles[k].x - 1, row, resolution, out);
    }
    i = j;
  }
  pfree(tiles);
}

/**
 * @brief Push the cells a geometry meets into the accumulator, recursing into
 * its components
 */
static void
lwgeom_to_cells_into(const LWGEOM *geom, int32 resolution, quadbin_buf *out)
{
  if (geom == NULL || lwgeom_is_empty(geom))
    return;
  switch (geom->type)
  {
    case POINTTYPE:
    {
      const POINT2D *p = getPoint2d_cp(((const LWPOINT *) geom)->point, 0);
      quadbin_buf_push(out,
        quadbin_point_to_cell(p->x, p->y, (uint32_t) resolution));
      break;
    }
    case LINETYPE:
      pointarray_to_cells_into(((const LWLINE *) geom)->points, resolution,
        out);
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
      for (uint32_t i = 0; i < col->ngeoms && ! out->overflow && ! out->error;
          i++)
        lwgeom_to_cells_into(col->geoms[i], resolution, out);
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
 * @ingroup meos_quadbin
 * @brief Return the set of QUADBIN cells covering a static geometry at the
 * given resolution
 * @details The cover holds every cell the geometry meets and no other: a
 * point gives the cell holding it, a line the tiles its segments pass through,
 * and a polygon those of its rings together with the tiles they enclose.
 * MULTI* and GEOMETRYCOLLECTION values give the union of the cells of their
 * components.
 * An empty geometry meets no cell and gives `NULL`.
 * @param[in] gs Geometry in a lon/lat (SRID 4326) reference system
 * @param[in] resolution Quadbin resolution
 * @errval NULL
 * @csqlfn #Geo_to_quadbinset()
 */
Set *
geo_to_quadbin_set(const GSERIALIZED *gs, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_cell_resolution(T_TQUADBIN, resolution) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(gs)))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom == NULL)
    return NULL;
  quadbin_buf buf;
  quadbin_buf_init(&buf, 64);
  lwgeom_to_cells_into(lwgeom, resolution, &buf);
  lwgeom_free(lwgeom);
  if (buf.error)
  {
    quadbin_buf_free(&buf);
    return NULL;
  }
  if (buf.overflow)
  {
    quadbin_buf_free(&buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The cover of the geometry at resolution %d exceeds %d cells",
      resolution, QUADBIN_MAX_COVER_CELLS);
    return NULL;
  }
  return quadbin_buf_to_set(&buf);
}

/*****************************************************************************
 * Membership of a temporal cell in a cell set
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return true if a temporal QUADBIN cell ever takes a cell of a QUADBIN
 * cell set
 * @details Returns 1 if any cell of @p cells appears among the values of
 * @p tqb, 0 if none does, and -1 on error. Read with #geo_to_quadbin_set it is
 * the spatial prefilter of the exact `eIntersects`: the cover holds every cell
 * the geometry meets, so a trajectory the test drops meets no cell of it
 * @param[in] cells Set of QUADBIN cells
 * @param[in] tqb Temporal QUADBIN cell
 * @csqlfn #Ever_eq_quadbinset_tquadbin()
 */
int
ever_eq_quadbinset_tquadbin(const Set *cells, const Temporal *tqb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(cells, -1); VALIDATE_TQUADBIN(tqb, -1);
  return tcellindex_ever_in_set(tqb, cells) ? 1 : 0;
}

/*****************************************************************************/
