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
 * @brief Bounding box operators for temporal rigid geometries
 */

#include "rgeo/trgeo_boxops.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos_geo.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_inst.h"

/*****************************************************************************/

/**
 * @brief Return true if two point arrays hold the same points in the same
 * order, coordinate by coordinate
 * @details Two coordinates are the same where they are equal as doubles, so 0
 * and -0 are the same, as in #ptarray_same of liblwgeom, which compares their
 * bytes instead
 */
static bool
same_ptarray(const POINTARRAY *pa1, const POINTARRAY *pa2)
{
  if (pa1->npoints != pa2->npoints ||
      FLAGS_GET_ZM(pa1->flags) != FLAGS_GET_ZM(pa2->flags))
    return false;
  int ndims = FLAGS_NDIMS(pa1->flags);
  for (uint32_t i = 0; i < pa1->npoints; i++)
  {
    const double *p1 = (const double *) getPoint2d_cp(pa1, i);
    const double *p2 = (const double *) getPoint2d_cp(pa2, i);
    for (int k = 0; k < ndims; k++)
      if (p1[k] != p2[k])
        return false;
  }
  return true;
}

/**
 * @brief Return true if two polygons have the same rings, as #same_ptarray
 * reads them
 */
static bool
same_lwpoly(const LWPOLY *poly1, const LWPOLY *poly2)
{
  if (poly1->nrings != poly2->nrings)
    return false;
  for (uint32_t i = 0; i < poly1->nrings; i++)
    if (! same_ptarray(poly1->rings[i], poly2->rings[i]))
      return false;
  return true;
}

/**
 * @brief Ensure that two reference geometries of temporal rigid geometries are
 * the same body: of one type, with the same rings and faces, and the same
 * points in the same order, coordinate by coordinate (#same_ptarray)
 */
bool
ensure_same_geom(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (gs1 == gs2)
    return true;
  bool same = gserialized_get_type(gs1) == gserialized_get_type(gs2);
  if (same)
  {
    LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
    LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
    if (geom1->type == POLYGONTYPE)
      same = same_lwpoly((LWPOLY *) geom1, (LWPOLY *) geom2);
    else
    {
      const LWPSURFACE *ps1 = (LWPSURFACE *) geom1, *ps2 = (LWPSURFACE *) geom2;
      same = ps1->ngeoms == ps2->ngeoms;
      for (uint32_t i = 0; same && i < ps1->ngeoms; i++)
        same = same_lwpoly(ps1->geoms[i], ps2->geoms[i]);
    }
    lwgeom_free(geom1);
    lwgeom_free(geom2);
  }
  if (! same)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on different reference geometries");
    return false;
  }
  return true;
}

/*****************************************************************************/

/*****************************************************************************/

/**
 * @brief Return the radius of a geometry, that is the greatest distance from the
 * origin to one of its points
 */
double
geom_radius(const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  LWPOINTITERATOR *it = lwpointiterator_create(geom);
  double r = 0;
  POINT4D p;
  while (lwpointiterator_next(it, &p))
  {
    r = FLAGS_GET_Z(geom->flags) ?
      fmax(r, sqrt(p.x * p.x + p.y * p.y + p.z * p.z)) :
      fmax(r, sqrt(p.x * p.x + p.y * p.y));
  }
  lwpointiterator_destroy(it);
  lwgeom_free(geom);
  return r;
}

/*****************************************************************************/
