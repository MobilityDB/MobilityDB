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
 * @brief Geometry adapters between a QUADBIN cell and PostGIS geometries.
 *
 * These typed wrappers keep the geometry construction inside MEOS so the
 * SQL cell/geometry conversions are pure catalog projections (the PG V1
 * wrappers in mobilitydb/src/quadbin/quadbin_ops.c are thin). The pure
 * cell kernel meos/src/quadbin/quadbin.c stays free of any geometry
 * dependency; the lon/lat coupling lives here, mirroring the h3 split
 * between h3index.c and th3index_latlng.c.
 */

#include "quadbin/quadbin.h"

/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>  /* GSERIALIZED_POINT2D_P */
#include <meos_quadbin.h>
#include "geo/tgeo_spatialfuncs.h"

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
  if (! ensure_srid_is_latlong(gserialized_get_srid(point)))
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
  quadbin_cell_to_point(cell, &lon, &lat);
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
 * @csqlfn #Quadbin_cell_to_boundary(), #Quadbin_cell_to_bounding_box()
 */
GSERIALIZED *
quadbin_cell_to_geom(Quadbin cell)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_to_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
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

/*****************************************************************************/
