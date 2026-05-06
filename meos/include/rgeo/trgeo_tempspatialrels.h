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
 * @brief Temporal spatial relationships for temporal rigid geometries
 */

#ifndef __TRGEO_TEMPSPATIALRELS_H__
#define __TRGEO_TEMPSPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "temporal/temporal.h"

/*****************************************************************************/

/* tContains */
extern Temporal *tcontains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tcontains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tcontains_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/* tCovers */
extern Temporal *tcovers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tcovers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tcovers_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/* tDisjoint */
extern Temporal *tdisjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tdisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tdisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/* tIntersects */
extern Temporal *tintersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tintersects_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/* tTouches */
extern Temporal *ttouches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *ttouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *ttouches_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/* tDwithin */
extern Temporal *tdwithin_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, double dist);
extern Temporal *tdwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern Temporal *tdwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, double dist);

/*****************************************************************************/

#endif /* __TRGEO_TEMPSPATIALRELS_H__ */
