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
 * @brief Distance functions for temporal points.
 */

#ifndef __TGEO_DISTANCE_H__
#define __TGEO_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************/

extern bool point3d_min_dist(const POINT3DZ *p1, const POINT3DZ *p2,
  const POINT3DZ *p3, const POINT3DZ *p4, double *fraction);

extern int tgeompointsegm_distance_turnpt(Datum start1, Datum end1,
  Datum start2, Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern int tgeogpointsegm_distance_turnpt(Datum start1, Datum end1,
  Datum start2, Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
  
extern double tnumberinst_distance(const TInstant *inst1,
  const TInstant *inst2);
extern double tinstant_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func);

/*****************************************************************************/

#endif /* __TGEO_DISTANCE_H__ */
