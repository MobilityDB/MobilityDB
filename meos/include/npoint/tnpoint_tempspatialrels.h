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
 * @brief Temporal spatial relationships for temporal network points.
 */

#ifndef __TNPOINT_TEMPSPATIALRELS_H__
#define __TNPOINT_TEMPSPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

extern Temporal *tinterrel_tnpoint_npoint(const Temporal *temp,
  const Npoint *np, bool tinter, bool restr, bool atvalue);
extern Temporal *tinterrel_tnpoint_geo(const Temporal *temp,
  const GSERIALIZED *geo, bool tinter, bool restr, bool atvalue);

extern Temporal *tcontains_geo_tnpoint(GSERIALIZED *geo, Temporal *temp,
  bool restr, bool atvalue);
extern Temporal *ttouches_tnpoint_geo(const Temporal *temp,
  const GSERIALIZED *geo, bool restr, bool atvalue);
extern Temporal *ttouches_geo_tnpoint(const GSERIALIZED *geo,
  const Temporal *temp, bool restr, bool atvalue);
extern Temporal *ttouches_tnpoint_npoint(const Temporal *temp,
  const Npoint *np, bool restr, bool atvalue);
extern Temporal *ttouches_npoint_tnpoint(const Npoint *np,
  const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_geo(Temporal *temp, GSERIALIZED *geo,
  double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_geo_tnpoint(GSERIALIZED *gs, Temporal *temp,
  double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_npoint(Temporal *temp, Npoint *np,
  double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_npoint_tnpoint(Npoint *np, Temporal *temp,
  double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2,
  double dist, bool restr, bool atvalue);


/*****************************************************************************/

#endif /* __TNPOINT_TEMPSPATIALRELS_H__ */
