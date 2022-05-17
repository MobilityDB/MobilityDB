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

#if POSTGIS_VERSION_NUMBER < 30000

#define PGIS_relate_pattern(X, Y, Z) \
  (call_function3(relate_pattern, PointerGetDatum(X), PointerGetDatum(Y), \
    PointerGetDatum(cstring2text(Z))))
#define PGIS_inter_contains(X, Y, Z) Z ? \
  (call_function2(intersects, PointerGetDatum(X), PointerGetDatum(Y))) : \
  (call_function2(contains, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_ST_3DIntersects(X, Y) \
  (call_function2(intersects3d, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_touches(X, Y) \
  (call_function2(touches, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_LWGEOM_dwithin(X, Y, Z) \
  (call_function3(LWGEOM_dwithin, PointerGetDatum(X), \
    PointerGetDatum(Y), Float8GetDatum(Z)))
#define PGIS_LWGEOM_dwithin3d(X, Y, Z) \
  (call_function3(LWGEOM_dwithin3d, PointerGetDatum(X), \
    PointerGetDatum(Y), Float8GetDatum(Z)))
#define PGIS_geography_dwithin(X, Y, Z, W) \
  (CallerFInfoFunctionCall4(geography_dwithin, (fetch_fcinfo())->flinfo, \
    InvalidOid, PointerGetDatum(X), PointerGetDatum(Y), Float8GetDatum(Z), \
    BoolGetDatum(W)))
#define PGIS_ST_Distance(X, Y) \
  (DatumGetFloat8(call_function2(distance, PointerGetDatum(X), PointerGetDatum(Y))))
#define PGIS_ST_3DDistance(X, Y) \
  (DatumGetFloat8(call_function2(distance3d, PointerGetDatum(X), PointerGetDatum(Y))))
#define PGIS_geography_distance(X, Y) \
  (DatumGetFloat8(CallerFInfoFunctionCall2(geography_distance, \
    (fetch_fcinfo())->flinfo, InvalidOid, PointerGetDatum(X), PointerGetDatum(Y))))
#define PGIS_LWGEOM_shortestline2d(X, Y) \
  (call_function2(LWGEOM_shortestline2d, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_LWGEOM_shortestline3d(X, Y) \
  (call_function2(LWGEOM_shortestline3d, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_geography_shortestline(X, Y) \
  (call_function2(geography_shortestline, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_ST_Intersection(X, Y) \
  (call_function2(intersection, PointerGetDatum(X), PointerGetDatum(Y)))
#define PGIS_boundary(X) \
  (call_function1(boundary, PointerGetDatum(X)))
#define PGIS_BOX3D_to_LWGEOM(X) \
  (DirectFunctionCall1(BOX3D_to_LWGEOM, STboxPGetDatum(X)))
#define PGIS_BOX2D_to_LWGEOM(X, Y) \
  do {                    \
      Datum geom = DirectFunctionCall1(BOX2D_to_LWGEOM, PointerGetDatum(X)); \
      GSERIALIZED *g = (GSERIALIZED *) PG_DETOAST_DATUM(geom); \
      gserialized_set_srid(g, Y); \
      result = PointerGetDatum(g); \
  } while (0)
#define PGIS_geometry_from_geography(X) \
  (DatumGetGserializedP(call_function1(geometry_from_geography, \
    PointerGetDatum(X))))
#define PGIS_geography_from_geometry(X) \
  (DatumGetGserializedP(call_function1(geography_from_geometry, \
    PointerGetDatum(X))))
#define PGIS_geography_length(X, Y) \
  (DatumGetFloat8(call_function2(geography_length, PointerGetDatum(X), \
    BoolGetDatum(Y))))

#else /* POSTGIS_VERSION_NUMBER >= 30000 */

/*****************************************************************************/

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"
#include "general/temporal_catalog.h"
#include "point/postgis.h"

/* Functions adapted from lwgeom_box.c */

extern GSERIALIZED *PGIS_BOX2D_to_LWGEOM(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern GSERIALIZED *PGIS_BOX3D_to_LWGEOM(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

/* The implementation of this function changed in PostGIS version 3.2 */
extern GSERIALIZED *PGIS_boundary(const GSERIALIZED *geom1);
extern GSERIALIZED *PGIS_LWGEOM_shortestline2d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern GSERIALIZED *PGIS_LWGEOM_shortestline3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_Distance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern double PGIS_ST_3DDistance(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool PGIS_ST_3DIntersects(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2);
extern bool PGIS_LWGEOM_dwithin(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool PGIS_LWGEOM_dwithin3d(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, double tolerance);
extern bool PGIS_relate_pattern(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, char *patt);

/* Functions adapted from lwgeom_btree.c */

extern bool PGIS_lwgeom_lt(GSERIALIZED *g1, GSERIALIZED *g2);

/* Functions adapted from lwgeom_geos.c */

extern bool PGIS_inter_contains(const GSERIALIZED *geom1,
  const GSERIALIZED *geom2, bool inter);
extern bool PGIS_touches(const GSERIALIZED *geom1, const GSERIALIZED *geom2);

extern GSERIALIZED *PGIS_ST_Intersection(GSERIALIZED *geom1,
  GSERIALIZED *geom2);

/* Functions adapted from geography_measurement.c */

extern double PGIS_geography_length(GSERIALIZED *g, bool use_spheroid);
extern bool PGIS_geography_dwithin(GSERIALIZED *g1, GSERIALIZED *g2,
  double tolerance, bool use_spheroid);
extern double PGIS_geography_distance(const GSERIALIZED *g1,
  const GSERIALIZED *g2);

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

#endif /* POSTGIS_VERSION_NUMBER < 30000 */

/*****************************************************************************/

#endif /* __PGIS_CALL_H__ */
