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
 * @brief Spatial relationships for temporal points.
 */

#ifndef __TCBUFFER_SPATIALRELS_H__
#define __TCBUFFER_SPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "temporal/temporal.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************/

extern int ea_contains_geo_tcbuffer(const GSERIALIZED *gs,
  const Temporal *temp, bool ever);
extern int ea_contains_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool ever);
extern int ea_contains_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb, bool ever);
extern int ea_contains_cbuffer_tcbuffer(const Cbuffer *cb,
  const Temporal *temp, bool ever);
// extern int ea_contains_tcbuffer_tcbuffer(const Temporal *temp1,
//   const Temporal *temp2, bool ever);

extern int ea_covers_geo_tcbuffer(const GSERIALIZED *gs,
  const Temporal *temp, bool ever);
extern int ea_covers_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool ever);
extern int ea_covers_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb, bool ever);
extern int ea_covers_cbuffer_tcbuffer(const Cbuffer *cb,
  const Temporal *temp, bool ever);
extern int ea_covers_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2, bool ever);

extern int ea_disjoint_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool ever);
extern int ea_disjoint_geo_tcbuffer(const GSERIALIZED *gs,
  const Temporal *temp, bool ever);
extern int ea_disjoint_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb, bool ever);
extern int ea_disjoint_cbuffer_tcbuffer(const Cbuffer *cb,
  const Temporal *temp, bool ever);
extern int ea_disjoint_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2, bool ever);

extern int ea_intersects_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool ever);
extern int ea_intersects_geo_tcbuffer(const GSERIALIZED *gs,
  const Temporal *temp, bool ever);
extern int ea_intersects_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb, bool ever);
extern int ea_intersects_cbuffer_tcbuffer(const Cbuffer *cb,
  const Temporal *temp, bool ever);
extern int ea_intersects_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2, bool ever);
  
extern int ea_touches_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool ever);
extern int ea_touches_geo_tcbuffer(const GSERIALIZED *gs,
  const Temporal *temp, bool ever);
extern int ea_touches_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb, bool ever);
extern int ea_touches_cbuffer_tcbuffer(const Cbuffer *cb,
  const Temporal *temp, bool ever);
extern int ea_touches_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2, bool ever);

extern int edwithin_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2, double dist);
extern int adwithin_tcbuffer_tcbuffer(const Temporal *temp1, 
  const Temporal *temp2, double dist);

/*****************************************************************************/

#endif
