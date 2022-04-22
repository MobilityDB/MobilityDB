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
 * @file tpoint_distance.h
 * Distance functions for temporal points.
 */

#ifndef __TPOINT_DISTANCE_H__
#define __TPOINT_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "general/temporal.h"
#include "point/tpoint.h"

/*****************************************************************************/

/* Distance functions */

extern Temporal *distance_tpoint_geo(const Temporal *temp,
  const GSERIALIZED *geo);
extern Temporal *distance_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2);

/* Nearest approach distance/instance and shortest line functions */

extern TInstant *nai_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2);

extern double nad_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_stbox_geo(const STBOX *box, const GSERIALIZED *gs);
extern double nad_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern double nad_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern double nad_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);

extern bool shortestline_tpoint_geo(const Temporal *temp,
  const GSERIALIZED *gs, Datum *result);
extern bool shortestline_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2, Datum *line);

/*****************************************************************************/

#endif
