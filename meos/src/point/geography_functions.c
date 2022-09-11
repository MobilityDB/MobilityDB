/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Spatial functions for PostGIS geography.
 *
 * These functions are supposed to be included in a forthcoming version of
 * PostGIS, to be proposed as a PR. This still remains to be done.
 * These functions are not needed in MobilityDB.
 */

#include "point/geography_funcs.h"

/* C */
#include <float.h>
/* PostgreSQL */
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#if ! MEOS
  #include <lwgeom_pg.h>
#endif /* ! MEOS */
#include <lwgeodetic_tree.h>
/* MobilityDB */
#include <meos.h>
#include "point/tpoint_spatialfuncs.h"

static LWGEOM *
geography_tree_shortestline(const GSERIALIZED* g1, const GSERIALIZED* g2,
  double threshold, const SPHEROID *spheroid)
{
  CIRC_NODE* circ_tree1 = NULL;
  CIRC_NODE* circ_tree2 = NULL;
  LWGEOM* lwgeom1 = NULL;
  LWGEOM* lwgeom2 = NULL;
  double min_dist = FLT_MAX;
  double max_dist = FLT_MAX;
  GEOGRAPHIC_POINT closest1, closest2;
  LWGEOM *geoms[2];
  LWGEOM *result;
  POINT4D p1, p2;

  lwgeom1 = lwgeom_from_gserialized(g1);
  lwgeom2 = lwgeom_from_gserialized(g2);
  circ_tree1 = lwgeom_calculate_circ_tree(lwgeom1);
  circ_tree2 = lwgeom_calculate_circ_tree(lwgeom2);

  /* Quietly decrease the threshold just a little to avoid cases where */
  /* the actual spheroid distance is larger than the sphere distance */
  /* causing the return value to be larger than the threshold value */
  // double threshold_radians = 0.95 * threshold / spheroid->radius;
  double threshold_radians = threshold / spheroid->radius;

  circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold_radians,
      &min_dist, &max_dist, &closest1, &closest2);

  p1.x = rad2deg(closest1.lon);
  p1.y = rad2deg(closest1.lat);
  p2.x = rad2deg(closest2.lon);
  p2.y = rad2deg(closest2.lat);

  geoms[0] = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p1.x, p1.y);
  geoms[1] = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p2.x, p2.y);
  result = (LWGEOM *)lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);

  lwgeom_free(geoms[0]);
  lwgeom_free(geoms[1]);
  circ_tree_free(circ_tree1);
  circ_tree_free(circ_tree2);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  return result;
}

/**
Returns the point in first input geography that is closest to the second
input geography in 2d (internal function)
*/
GSERIALIZED *
geography_shortestline_internal(const GSERIALIZED *g1, const GSERIALIZED *g2,
  bool use_spheroid)
{
  SPHEROID s;
  ensure_same_srid(gserialized_get_srid(g1), gserialized_get_srid(g2));

  /* Return NULL on empty arguments. */
  if ( gserialized_is_empty(g1) || gserialized_is_empty(g2) )
    return NULL;

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(fcinfo, srid, &s);
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *line = geography_tree_shortestline(g1, g2, FP_TOLERANCE, &s);

  if (lwgeom_is_empty(line))
    return NULL;

#if ! MEOS
  GSERIALIZED *result = geography_serialize(line);
#else
  GSERIALIZED *result = geo_serialize(line);
#endif /* ! MEOS */
  lwgeom_free(line);

  return result;
}

/*****************************************************************************/
