/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Temporal distance for temporal network points.
 */

#ifndef __TCBUFFER_H__
#define __TCBUFFER_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "temporal/temporal.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_tcbuffer_cbuffer(const Temporal *temp,
  const Cbuffer *cb);
extern bool ensure_valid_tcbuffer_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_valid_tcbuffer_stbox(const Temporal *temp,
  const STBox *box);
extern bool ensure_valid_tcbuffer_tcbuffer(const Temporal *temp1,
  const Temporal *temp2);

/* Interpolation functions */

extern int tcbuffersegm_intersection_value(Datum start, Datum end,
  Datum value, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);
extern int tcbuffersegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);

extern int tcbuffersegm_dwithin_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum dist, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern int tcbuffersegm_tdwithin_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum dist, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern int tcbuffersegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum dist UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);

/* Native (GEOS-free) temporal within relationship helpers */

extern void *tcbuffer_geo_ctx_make(const GSERIALIZED *gs);
extern void tcbuffer_geo_ctx_free(void *ctx);
extern int tcbuffer_geo_ctx_nsegs(const void *ctx);
extern bool tcbuffer_disc_within_ctx(const Cbuffer *cb, double dist,
  const void *ctx);
extern int tcbufferseg_within_ctx(const Cbuffer *cb1, const Cbuffer *cb2,
  double dist, const void *ctx, double *outlo, double *outhi, int maxout);
extern bool tcbuffer_disc_touch_ctx(const Cbuffer *cb, const void *ctx);
extern int tcbufferseg_touch_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const void *ctx, double *outt, int maxout);
extern bool tcbuffer_disc_contains_ctx(const Cbuffer *cb, const void *ctx,
  bool strict);

/*****************************************************************************/

#endif /* __TCBUFFER_H__ */
