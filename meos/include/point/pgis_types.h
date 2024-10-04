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
 * @file
 * @brief Functions for geometry types corresponding to external
 * PostGIS functions in order to bypass the function manager @p fmgr.c.
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

/* Functions borrowed from lwgeom_pg.c */

extern GSERIALIZED* geom_serialize(LWGEOM *lwgeom);
extern GSERIALIZED* geog_serialize(LWGEOM *lwgeom);

/* Functions adapted from lwgeom_box.c */

extern LWGEOM *box2d_to_lwgeom(GBOX *box, int srid);

/* Functions adapted from lwgeom_box3d.c */

extern LWGEOM *box3d_to_lwgeom(BOX3D *box);

/* Functions adapted from lwgeom_functions_basic.c */

extern GSERIALIZED *geo_reverse(const GSERIALIZED *geom);

/* Functions adapted from lwgeom_geos.c */

extern GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *pglwgeom);
extern GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d);

extern bool geom_spatialrel(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  spatialRel rel);


/* Functions adapted from lwgeom_lrs.c */

extern LWGEOM *lwgeom_line_interpolate_point(LWGEOM *geom, double fraction,
  int32_t srid, char repeat);


/*****************************************************************************/

#endif /* __PGIS_TYPES_H__ */
