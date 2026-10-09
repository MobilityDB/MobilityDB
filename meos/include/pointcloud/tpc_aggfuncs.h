/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Transforms of the aggregates of temporal pgpointcloud patches
 * @details The transforms are defined inline so that the MEOS transition
 * functions and the PostgreSQL transition functions of the aggregates
 * compile the same code into their own bodies.
 */

#ifndef __TPC_AGGFUNCS_H__
#define __TPC_AGGFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "pointcloud/pcpatch.h"

/*****************************************************************************/

/**
 * @brief Return the instants of a temporal pgpointcloud patch carrying the
 * number of points of the patch at each instant
 * @param[in] temp Temporal pgpointcloud patch
 * @param[out] count Number of instants
 */
static inline TInstant **
tpcpatch_transform_tnpoints(const Temporal *temp, int *count)
{
  int n = temporal_num_instants(temp);
  TInstant **result = palloc(sizeof(TInstant *) * n);
  for (int i = 0; i < n; i++)
  {
    const TInstant *inst = temporal_instant_n(temp, i + 1);
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(
      tinstant_value_p(inst));
    int32 npts = pcpatch_npoints(pa);
    result[i] = tinstant_make(Int32GetDatum(npts), T_TINT, inst->t);
  }
  *count = n;
  return result;
}

/**
 * @brief Return the instants of a temporal pgpointcloud patch carrying the
 * density of the patch at each instant, its number of points divided by the
 * area of its XY bounds, infinite for a patch whose bounds have no area
 * @param[in] temp Temporal pgpointcloud patch
 * @param[out] count Number of instants
 */
static inline TInstant **
tpcpatch_transform_tdensity(const Temporal *temp, int *count)
{
  int n = temporal_num_instants(temp);
  TInstant **result = palloc(sizeof(TInstant *) * n);
  for (int i = 0; i < n; i++)
  {
    const TInstant *inst = temporal_instant_n(temp, i + 1);
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(
      tinstant_value_p(inst));
    double xrange = pa->bounds[1] - pa->bounds[0];
    double yrange = pa->bounds[3] - pa->bounds[2];
    double area = xrange * yrange;
    double density = (area > 0.0) ? pa->npoints / area : pa->npoints / 0.0;
    result[i] = tinstant_make(Float8GetDatum(density), T_TFLOAT, inst->t);
  }
  *count = n;
  return result;
}

/*****************************************************************************/

#endif /* __TPC_AGGFUNCS_H__ */
