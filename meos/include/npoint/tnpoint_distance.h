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
 * @brief Temporal distance for temporal network points.
 */

#ifndef __TNPOINT_DISTANCE_H__
#define __TNPOINT_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

extern Datum npoint_distance(Datum np1, Datum np2);
extern Temporal *distance_tnpoint_geo(const Temporal *temp,
  const GSERIALIZED *geo);
extern Temporal *distance_tnpoint_npoint(const Temporal *temp,
  const Npoint *np);
extern Temporal *distance_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);

extern TInstant *nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo);
extern TInstant *nai_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern TInstant *nai_tnpoint_tnpoint(const Temporal *temp,
  const Temporal *temp2);

extern double nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo);
extern double nad_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern double nad_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);

extern bool shortestline_tnpoint_geo(const Temporal *temp,
  const GSERIALIZED *geo, GSERIALIZED **result);
extern GSERIALIZED *shortestline_tnpoint_npoint(const Temporal *temp,
  const Npoint *np);
extern bool shortestline_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2, GSERIALIZED **result);

/*****************************************************************************/

#endif /* __TNPOINT_DISTANCE_H__ */
