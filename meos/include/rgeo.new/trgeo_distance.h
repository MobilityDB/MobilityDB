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
 * documentation for any purrgeo, without fee, and without a written
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
 * AND FITNESS FOR A PARTICULAR PURRGEO. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief Temporal distance for temporal circular buffers.
 */

#ifndef __TRGEO_DISTANCE_H__
#define __TRGEO_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "rgeo/trgeo.h"

/*****************************************************************************/

extern Temporal *distance_trgeo_point(const Temporal *temp,
  const GSERIALIZED *gs);
extern Temporal *distance_trgeo_rgeo(const Temporal *temp, const Rgeo *rgeo);
extern Temporal *distance_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);

extern TInstant *nai_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_trgeo_rgeo(const Temporal *temp, const Rgeo *rgeo);
extern TInstant *nai_trgeo_trgeo(const Temporal *temp, const Temporal *temp2);

extern double nad_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_trgeo_rgeo(const Temporal *temp, const Rgeo *rgeo);
extern double nad_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);

extern GSERIALIZED *shortestline_trgeo_geo(const Temporal *temp,
  const GSERIALIZED *geo);
extern GSERIALIZED *shortestline_trgeo_rgeo(const Temporal *temp,
  const Rgeo *rgeo);
extern GSERIALIZED *shortestline_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************/

#endif /* __TRGEO_DISTANCE_H__ */
