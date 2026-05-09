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
 * @brief Spatial functions for temporal rigid geometries.
 */

#ifndef __TRGEO_SPATIALFUNCS_H__
#define __TRGEO_SPATIALFUNCS_H__

#include <meos.h>
#include "temporal/temporal.h"
#include "pose/pose.h"

/*****************************************************************************/

/* Accessor functions */

extern GSERIALIZED *trgeo_traversed_area(const Temporal *temp,
  bool unary_union);
extern Temporal *trgeo_centroid(const Temporal *temp);
extern GSERIALIZED *trgeo_convex_hull(const Temporal *temp);

/* Body-frame trajectory functions */

extern Temporal *trgeo_body_point_trajectory(const Temporal *temp,
  const GSERIALIZED *gs);

/* Restriction functions */

extern Temporal *trgeo_restrict_geom(const Temporal *temp,
  const GSERIALIZED *gs, bool atfunc);
extern Temporal *trgeo_restrict_stbox(const Temporal *temp, const STBox *box,
  bool border_inc, bool atfunc);

/* Similarity distance functions */

extern double trgeo_hausdorff_distance(const Temporal *temp1,
  const Temporal *temp2);

#if MEOS
extern double trgeo_frechet_distance(const Temporal *temp1,
  const Temporal *temp2);
extern double trgeo_dyntimewarp_distance(const Temporal *temp1,
  const Temporal *temp2);
extern Match *trgeo_frechet_path(const Temporal *temp1, const Temporal *temp2,
  int *count);
extern Match *trgeo_dyntimewarp_path(const Temporal *temp1,
  const Temporal *temp2, int *count);
#endif /* MEOS */

/*****************************************************************************/

#endif /* __TRGEO_SPATIALFUNCS_H__ */
