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

/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>  /* GSERIALIZED_POINT2D_P */
#include <meos_s2cell.h>
#include "geo/tgeo_spatialfuncs.h"

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

  if (! ensure_srid_is_latlong(gserialized_get_srid(point)))
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

/*****************************************************************************/
