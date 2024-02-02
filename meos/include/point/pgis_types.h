/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief MobilityDB functions gserialized_func(...) corresponding to external
 * PostGIS functions XXX_func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 */

#ifndef __PGIS_TYPES_H__
#define __PGIS_TYPES_H__

/*****************************************************************************/

/* GEOS */
#include <geos_c.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>


/* Functions adapted from lwgeom_box.c */

extern LWGEOM *box2d_to_lwgeom(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern LWGEOM *box3d_to_lwgeom(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

/* The implementation of this function changed in PostGIS version 3.2 */
extern GSERIALIZED *geometry_boundary(const GSERIALIZED *gs);
extern GSERIALIZED *geo_shortestline2d(const GSERIALIZED *gs1,
  const GSERIALIZED *s2);
extern GSERIALIZED *geometry_shortestline3d(const GSERIALIZED *gs1,
  const GSERIALIZED *s2);
extern double geo_distance(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern double geometry_3Ddistance(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool geometry_3Dintersects(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool geometry_dwithin2d(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2, double tolerance);
extern bool geometry_dwithin3d(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2, double tolerance);
extern bool geo_relate_pattern(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2, char *patt);
extern GSERIALIZED *geo_reverse(const GSERIALIZED *geom);
extern bool gserialized_azimuth(GSERIALIZED *gs1, GSERIALIZED *gs2,
  double *result);

/* Functions adapted from lwgeom_geos.c */

extern GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *pglwgeom);
extern GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d);

extern bool geometry_spatialrel(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2, spatialRel rel);
extern GSERIALIZED *geometry_intersection(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern GSERIALIZED *geometry_array_union(GSERIALIZED **gsarr, int nelems);
extern GSERIALIZED *geometry_buffer(const GSERIALIZED *gs, double size,
  char *params);
extern GSERIALIZED *geometry_convex_hull(const GSERIALIZED *gs);

/* Functions adapted from geography_measurement.c */

extern double pgis_geography_length(GSERIALIZED *g, bool use_spheroid);
extern bool pgis_geography_dwithin(GSERIALIZED *g1, GSERIALIZED *g2,
  double tolerance, bool use_spheroid);
extern double pgis_geography_distance(const GSERIALIZED *g1,
  const GSERIALIZED *g2);

/* Functions adapted from geography_inout.c */

extern GSERIALIZED *gserialized_geog_from_geom(GSERIALIZED *geom);
extern GSERIALIZED *gserialized_geom_from_geog(GSERIALIZED *geog);

/* Functions adapted from lwgeom_functions_analytic.c */

extern GSERIALIZED *linestring_line_interpolate_point(GSERIALIZED *gser,
  double distance_fraction, char repeat);
extern GSERIALIZED *linestring_substring(GSERIALIZED *geom, double from,
  double to);

/* Functions adapted from lwgeom_lrs.c */

extern LWGEOM *lwgeom_line_interpolate_point(LWGEOM *geom, double fraction,
  int32_t srid, char repeat);
extern double linestring_locate_point(GSERIALIZED *gs1,
  GSERIALIZED *gs2);

/* Functions adapted from lwgeom_ogc.c */

extern GSERIALIZED *linestring_point_n(const GSERIALIZED *geom,
  int where);
extern int linestring_numpoints(const GSERIALIZED *geom);

/*****************************************************************************/

#endif /* __PGIS_TYPES_H__ */
