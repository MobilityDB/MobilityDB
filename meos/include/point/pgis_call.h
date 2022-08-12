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
 * @brief MobilityDB functions PGIS_func(...) corresponding to external
 * PostGIS functions func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 */

#ifndef __PGIS_CALL_H__
#define __PGIS_CALL_H__

/*****************************************************************************/

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"
#include "general/temporal_catalog.h"

/* Functions adapted from lwgeom_box.c */

extern GSERIALIZED *PGIS_BOX2D_to_LWGEOM(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern GSERIALIZED *PGIS_BOX3D_to_LWGEOM(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

/* The implementation of this function changed in PostGIS version 3.2 */
extern GSERIALIZED *PGIS_boundary(const GSERIALIZED *geom1);
extern GSERIALIZED *gserialized_shortestline2d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *gserialized_shortestline3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_Distance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_3DDistance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool PGIS_ST_3DIntersects(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool gserialized_dwithin(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool gserialized_dwithin3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool PGIS_relate_pattern(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, char *patt);
extern GSERIALIZED *gserialized_reverse(const GSERIALIZED *geom);
extern bool gserialized_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2,
  double *result);

/* Functions adapted from lwgeom_btree.c */

extern bool PGIS_lwgeom_lt(GSERIALIZED *g1, GSERIALIZED *g2);

/* Functions adapted from lwgeom_geos.c */

extern bool PGIS_inter_contains(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, bool inter);
extern bool PGIS_touches(const GSERIALIZED *geom1, const GSERIALIZED *geom2);
extern GSERIALIZED *PGIS_ST_Intersection(GSERIALIZED *geom1,
  GSERIALIZED *geom2);
extern GSERIALIZED *PGIS_union_geometry_array(GSERIALIZED **gsarr, int nelems);

/* Functions adapted from geography_measurement.c */

extern double PGIS_geography_length(GSERIALIZED *g, bool use_spheroid);
extern bool PGIS_geography_dwithin(GSERIALIZED *g1, GSERIALIZED *g2,
  double tolerance, bool use_spheroid);
extern double PGIS_geography_distance(const GSERIALIZED *g1,
  const GSERIALIZED *g2);

/* Functions adapted from geography_inout.c */

extern GSERIALIZED *PGIS_geography_in(char *str, int32 geog_typmod);
extern char *PGIS_geography_out(GSERIALIZED *g);

extern GSERIALIZED *PGIS_geography_from_geometry(GSERIALIZED *geom);
extern GSERIALIZED *PGIS_geometry_from_geography(GSERIALIZED *g_ser);

/* Functions adapted from lwgeom_functions_analytic.c */

extern GSERIALIZED *PGIS_LWGEOM_line_interpolate_point(GSERIALIZED *gser,
  double distance_fraction, int repeat);
extern GSERIALIZED *PGIS_LWGEOM_line_substring(GSERIALIZED *geom, double from,
  double to);

/* Functions adapted from lwgeom_lrs.c */

extern LWGEOM *lwgeom_line_interpolate_point(LWGEOM *lwgeom, double fraction,
  int32_t srid, int repeat);
extern double PGIS_LWGEOM_line_locate_point(GSERIALIZED *geom1,
  GSERIALIZED *geom2);

/* Functions adapted from lwgeom_ogc.c */

extern GSERIALIZED *PGIS_LWGEOM_pointn_linestring(const GSERIALIZED *geom,
  int where);
extern int PGIS_LWGEOM_numpoints_linestring(const GSERIALIZED *geom);

/*****************************************************************************/

#endif /* __PGIS_CALL_H__ */
