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
 * @brief Ever/always spatial relationships for temporal rigid geometries
 */

#ifndef __TRGEO_SPATIALRELS_H__
#define __TRGEO_SPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "temporal/temporal.h"
#include "pose/pose.h"

/*****************************************************************************/

/* Generic */
extern int spatialrel_trgeo_trav_geo(const Temporal *temp,
  const GSERIALIZED *gs, Datum param, varfunc func, int numparam, bool invert);

/* eContains / aContains */
extern int ea_contains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_contains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_contains_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, bool ever);

/* eCovers / aCovers */
extern int ea_covers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_covers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_covers_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, bool ever);

/* eDisjoint / aDisjoint */
extern int ea_disjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_disjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_disjoint_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, bool ever);

/* eIntersects / aIntersects */
extern int ea_intersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_intersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_intersects_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, bool ever);

/* eTouches / aTouches */
extern int ea_touches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_touches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int etouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int atouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ea_touches_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, bool ever);

/* eDwithin / aDwithin */
extern int ea_dwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist, bool ever);
extern int ea_dwithin_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2, double dist, bool ever);

/*****************************************************************************/

#endif /* __TRGEO_SPATIALRELS_H__ */
