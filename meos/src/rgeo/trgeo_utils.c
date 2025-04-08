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
#include <meos_internal.h>
#include "geo/stbox.h"
#include "pose/pose.h"
#include "rgeo/trgeo_inst.h"

/*****************************************************************************/

/**
 * @brief 
 */
static void
ensure_same_rings_lwpoly(const LWPOLY *poly1, const LWPOLY *poly2)
{
  if (poly1->nrings != poly2->nrings)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Operation on different reference geometries");
  for (int i = 0; i < (int) poly1->nrings; ++i)
    if (poly1->rings[i]->npoints != poly2->rings[i]->npoints)
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Operation on different reference geometries");
}

/**
 * @brief 
 */
static void
ensure_same_geoms_lwpsurface(const LWPSURFACE *psurface1,
  const LWPSURFACE *psurface2)
{
  if (psurface1->ngeoms != psurface2->ngeoms)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on different reference geometries");
  for (int i = 0; i < (int) psurface1->ngeoms; ++i)
    ensure_same_rings_lwpoly(psurface1->geoms[i], psurface2->geoms[i]);
}

/**
 * @brief 
 */
static bool
same_lwgeom(const LWGEOM *geom1, const LWGEOM *geom2)
{
  LWPOINTITERATOR *it1 = lwpointiterator_create(geom1);
  LWPOINTITERATOR *it2 = lwpointiterator_create(geom2);
  POINT4D p1;
  POINT4D p2;

  bool result = true;
  while (lwpointiterator_next(it1, &p1) && lwpointiterator_next(it2, &p2) &&
    result)
  {
    if (FLAGS_GET_Z(geom1->flags))
    {
      result = fabs(p1.x - p2.x) < MEOS_EPSILON && 
        fabs(p1.y - p2.y) < MEOS_EPSILON && fabs(p1.z - p2.z) < MEOS_EPSILON;
    }
    else
    {
      result = fabs(p1.x - p2.x) < MEOS_EPSILON && 
        fabs(p1.y - p2.y) < MEOS_EPSILON;
    }
  }
  lwpointiterator_destroy(it1);
  lwpointiterator_destroy(it2);
  return result;
}

/**
 * @brief Ensure that the temporal rigid geometry instants have the same
 * reference geometry
 */
bool
ensure_same_geom(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (gs1 == gs2)
    return true;

  if (gserialized_get_type(gs1) != gserialized_get_type(gs2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, 
      "Operation on different reference geometries");
    return false;
  }

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  if (gserialized_get_type(gs1) == POLYGONTYPE)
    ensure_same_rings_lwpoly((LWPOLY *) geom1, (LWPOLY *) geom2);
  else
    ensure_same_geoms_lwpsurface((LWPSURFACE *) geom1, (LWPSURFACE *) geom2);

  if (! same_lwgeom(geom1, geom2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, 
      "Operation on different reference geometries");
    return false;
  }

  lwgeom_free(geom1);
  lwgeom_free(geom2);
  return true;
}

/*****************************************************************************/

/**
 * @brief 
 */
static void
lwgeom_affine_transform(LWGEOM *geom,
  double a, double b, double c,
  double d, double e, double f,
  double g, double h, double i,
  double xoff, double yoff, double zoff)
{
  AFFINE affine;
  affine.afac =  a;
  affine.bfac =  b;
  affine.cfac =  c;
  affine.dfac =  d;
  affine.efac =  e;
  affine.ffac =  f;
  affine.gfac =  g;
  affine.hfac =  h;
  affine.ifac =  i;
  affine.xoff =  xoff;
  affine.yoff =  yoff;
  affine.zoff =  zoff;
  lwgeom_affine(geom, &affine);
  return;
}

/**
 * @brief 
 */
void
lwgeom_apply_pose(const Pose *pose, LWGEOM *geom)
{
  if (! MEOS_FLAGS_GET_Z(pose->flags))
  {
    double a = cos(pose->data[2]);
    double b = sin(pose->data[2]);

    lwgeom_affine_transform(geom,
      a, -b, 0,
      b, a, 0,
      0, 0, 1,
      pose->data[0], pose->data[1], 0);
  }
  else
  {
    double W = pose->data[3];
    double X = pose->data[4];
    double Y = pose->data[5];
    double Z = pose->data[6];

    double a = W*W + X*X - Y*Y - Z*Z;
    double b = 2*X*Y - 2*W*Z;
    double c = 2*X*Z + 2*W*Y;
    double d = 2*X*Y + 2*W*Z;
    double e = W*W - X*X + Y*Y - Z*Z;
    double f = 2*Y*Z - 2*W*X;
    double g = 2*X*Z - 2*W*Y;
    double h = 2*Y*Z + 2*W*X;
    double i = W*W - X*X - Y*Y + Z*Z;

    lwgeom_affine_transform(geom,
      a, b, c,
      d, e, f,
      g, h, i,
      pose->data[0], pose->data[1], pose->data[2]);
  }
  return;
}

/*****************************************************************************/

/**
 * @brief 
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
      fmax(r, sqrt(pow(p.x, 2) + pow(p.y, 2) + pow(p.z, 2))) :
      fmax(r, sqrt(pow(p.x, 2) + pow(p.y, 2)));
  }
  lwpointiterator_destroy(it);
  lwgeom_free(geom);
  return r;
}

/*****************************************************************************/
