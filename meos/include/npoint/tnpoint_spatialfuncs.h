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
 * @brief Geospatial functions for temporal network points.
 */

#ifndef __TNPOINT_SPATIALFUNCS_H__
#define __TNPOINT_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

/* Parameter tests */

extern void ensure_same_srid_tnpoint_stbox(const Temporal *temp,
  const STBox *box);
extern void ensure_same_rid_tnpointinst(const TInstant *inst1,
  const TInstant *inst2);

/* Interpolation functions */

extern bool tnpointsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t);

/* Functions for spatial reference systems */

extern int tnpointinst_srid(const TInstant *inst);
extern int tnpoint_srid(const Temporal *temp);
extern GSERIALIZED *tnpointinst_geom(const TInstant *inst);
extern GSERIALIZED *tnpointseq_geom(const TSequence *seq);
extern GSERIALIZED *tnpointseqset_geom(const TSequenceSet *ss);
extern GSERIALIZED *tnpoint_geom(const Temporal *temp);

extern bool npoint_same(const Npoint *np1, const Npoint *np2);

extern double tnpoint_length(const Temporal *temp);
extern Temporal *tnpoint_cumulative_length(const Temporal *temp);
extern Temporal *tnpoint_speed(const Temporal *temp);
extern Datum tnpoint_twcentroid(const Temporal *temp);
extern Temporal *tnpoint_azimuth(const Temporal *temp);
extern Temporal *tnpoint_restrict_geom_time(const Temporal *temp,
  const GSERIALIZED *gs, const Span *zspan, const Span *period, bool atfunc);

/*****************************************************************************/

#endif /* __TNPOINT_SPATIALFUNCS_H__ */
