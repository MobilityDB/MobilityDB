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
 * @brief Spatial relationships for temporal points.
 */

#ifndef __TPOINT_SPATIALRELS_H__
#define __TPOINT_SPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"

/*****************************************************************************/

extern Datum geom_contains(Datum geom1, Datum geom2);
extern Datum geom_disjoint2d(Datum geom1, Datum geom2);
extern Datum geom_disjoint3d(Datum geom1, Datum geom2);
extern Datum geog_disjoint(Datum geog1, Datum geog2);
extern Datum geom_intersects2d(Datum geom1, Datum geom2);
extern Datum geom_intersects3d(Datum geom1, Datum geom2);
extern Datum geog_intersects(Datum geog1, Datum geog2);
extern Datum geom_touches(Datum geom1, Datum geom2);
extern Datum geom_dwithin2d(Datum geom1, Datum geom2, Datum dist);
extern Datum geom_dwithin3d(Datum geom1, Datum geom2, Datum dist);
extern Datum geog_dwithin(Datum geog1, Datum geog2, Datum dist);

extern datum_func3 get_dwithin_fn(int16 flags1, int16 flags2);

/*****************************************************************************/

extern int espatialrel_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2, Datum (*func)(Datum, Datum));

extern int edwithin_tpoint_tpoint1(const Temporal *sync1, const Temporal *sync2,
  double dist);

/*****************************************************************************/

#endif
