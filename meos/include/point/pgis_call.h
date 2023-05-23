/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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

#ifndef __PGIS_CALL_H__
#define __PGIS_CALL_H__

/*****************************************************************************/

/* GEOS */
#include <geos_c.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "general/meos_catalog.h"
#include "general/span.h"
#include "general/temporal.h"


/* Functions adapted from lwgeom_box.c */

extern LWGEOM *box2d_to_lwgeom(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern LWGEOM *box3d_to_lwgeom(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

/* The implementation of this function changed in PostGIS version 3.2 */
extern GSERIALIZED *gserialized_boundary(const GSERIALIZED *geom1);
extern GSERIALIZED *gserialized_shortestline2d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *gserialized_shortestline3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double gserialized_distance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double gserialized_3Ddistance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool gserialized_3Dintersects(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool gserialized_dwithin(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool gserialized_dwithin3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool gserialized_relate_pattern(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, char *patt);
extern GSERIALIZED *gserialized_reverse(const GSERIALIZED *geom);
extern bool gserialized_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2,
  double *result);

/* Functions adapted from lwgeom_geos.c */

extern GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *pglwgeom);
extern GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d);

extern bool gserialized_spatialrel(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, spatialRel rel);
extern GSERIALIZED *gserialized_intersection(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *gserialized_array_union(GSERIALIZED **gsarr, int nelems);
extern GSERIALIZED *gserialized_convex_hull(const GSERIALIZED *geom);
extern double gserialized_hausdorffdistance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);

/* Functions adapted from geography_measurement.c */

extern double gserialized_geog_length(GSERIALIZED *g, bool use_spheroid);
extern bool gserialized_geog_dwithin(GSERIALIZED *g1, GSERIALIZED *g2,
  double tolerance, bool use_spheroid);
extern double gserialized_geog_distance(const GSERIALIZED *g1,
  const GSERIALIZED *g2);

/* Functions adapted from geography_inout.c */

extern GSERIALIZED *gserialized_geog_in(char *str, int32 geog_typmod);
extern char *gserialized_geog_out(GSERIALIZED *g);

extern GSERIALIZED *gserialized_geog_from_geom(GSERIALIZED *geom);
extern GSERIALIZED *gserialized_geom_from_geog(GSERIALIZED *g_ser);

/* Functions adapted from lwgeom_functions_analytic.c */

extern GSERIALIZED *gserialized_line_interpolate_point(GSERIALIZED *gser,
  double distance_fraction, char repeat);
extern GSERIALIZED *gserialized_line_substring(GSERIALIZED *geom, double from,
  double to);

/* Functions adapted from lwgeom_lrs.c */

extern LWGEOM *lwgeom_line_interpolate_point(LWGEOM *lwgeom, double fraction,
  int32_t srid, char repeat);
extern double gserialized_line_locate_point(GSERIALIZED *geom1,
  GSERIALIZED *geom2);

/* Functions adapted from lwgeom_ogc.c */

extern GSERIALIZED *gserialized_pointn_linestring(const GSERIALIZED *geom,
  int where);
extern int gserialized_numpoints_linestring(const GSERIALIZED *geom);

/*****************************************************************************/

#endif /* __PGIS_CALL_H__ */
