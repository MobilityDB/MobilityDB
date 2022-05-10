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
 * @file pgis_call.h
 * @brief MobilityDB functions PGIS_func(...) corresponding to external
 * PostGIS functions func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 */

#ifndef __PGIS_CALL_H__
#define __PGIS_CALL_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"
#include "general/temporal_catalog.h"
#include "point/postgis.h"

/*****************************************************************************/

/* Functions adapted from lwgeom_box.c */

extern GSERIALIZED *PGIS_BOX2D_to_LWGEOM(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern GSERIALIZED *PGIS_BOX3D_to_LWGEOM(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

extern GSERIALIZED *PGIS_LWGEOM_shortestline2d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *PGIS_LWGEOM_shortestline3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_Distance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_3DDistance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *PGIS_LWGEOM_reverse(GSERIALIZED *geom);
extern bool PGIS_ST_3DIntersects(GSERIALIZED *geom1, GSERIALIZED *geom2);
extern bool PGIS_LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2,
  double *result);

/* Functions adapted from lwgeom_btree.c */

extern bool PGIS_lwgeom_lt(GSERIALIZED *g1, GSERIALIZED *g2);

/* Functions adapted from lwgeom_functions_lrs.c */

extern double PGIS_LWGEOM_line_locate_point(GSERIALIZED *geom1,
  GSERIALIZED *geom2);

/* Functions adapted from lwgeom_functions_analytic.c */

extern GSERIALIZED *PGIS_LWGEOM_line_substring(GSERIALIZED *geom, double from,
  double to);
extern GSERIALIZED *PGIS_LWGEOM_line_interpolate_point(GSERIALIZED *gser,
  double distance_fraction);

/* Functions adapted from lwgeom_geos.c */

extern GSERIALIZED *PGIS_ST_Intersection(GSERIALIZED *geom1,
  GSERIALIZED *geom2);

/* Functions adapted from geography_measurement.c */

extern double PGIS_geography_length(GSERIALIZED *g, bool use_spheroid);

/* Functions adapted from lwgeom_inout.c */

extern GSERIALIZED *PGIS_LWGEOM_in(char *input, int32 geom_typmod);
extern char *PGIS_LWGEOM_out(GSERIALIZED *geom);
extern GSERIALIZED *PGIS_LWGEOM_recv(StringInfo buf);
extern bytea *PGIS_LWGEOM_send(GSERIALIZED *geo);

/* Functions adapted from geography_inout.c */

extern GSERIALIZED *PGIS_geography_in(char *str, int32 geog_typmod);
extern char *PGIS_geography_out(GSERIALIZED *g);
extern GSERIALIZED *PGIS_geography_recv(StringInfo buf);
extern bytea *PGIS_geography_send(GSERIALIZED *g);

extern GSERIALIZED *PGIS_geography_from_geometry(GSERIALIZED *geom);
extern GSERIALIZED *PGIS_geometry_from_geography(GSERIALIZED *g_ser);

/*****************************************************************************/

#endif /* __PGIS_CALL_H__ */
