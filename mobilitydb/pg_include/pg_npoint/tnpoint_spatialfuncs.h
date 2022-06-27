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
 * @file tnpoint_spatialfuncs.c
 * Geospatial functions for temporal network points.
 */

#ifndef __TNPOINT_SPATIALFUNCS_H__
#define __TNPOINT_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/temporal.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

/* Parameter tests */

extern void ensure_same_srid_tnpoint_stbox(const Temporal *temp,
  const STBOX *box);
extern void ensure_same_rid_tnpointinst(const TInstant *inst1,
  const TInstant *inst2);

/* Interpolation functions */

extern bool tnpointsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t);

/* Functions for spatial reference systems */

extern int tnpointinst_srid(const TInstant *inst);
extern int tnpoint_srid(const Temporal *temp);
extern Datum tnpointinst_geom(const TInstant *inst);
extern Datum tnpointinstset_geom(const TInstantSet *ti);
extern Datum tnpointseq_geom(const TSequence *seq);
extern Datum tnpointseqset_geom(const TSequenceSet *ts);
extern Datum tnpoint_geom(const Temporal *temp);

extern bool npoint_same(const Npoint *np1, const Npoint *np2);

extern double tnpoint_length(Temporal *temp);
extern Temporal *tnpoint_cumulative_length(Temporal *temp);
extern Temporal *tnpoint_speed(Temporal *temp);
extern Datum tnpoint_twcentroid(Temporal *temp);
extern Temporal *tnpoint_azimuth(Temporal *temp);
extern Temporal *tnpoint_restrict_geometry(Temporal *temp, GSERIALIZED *gs,
  bool atfunc);

/*****************************************************************************/

#endif /* __TNPOINT_SPATIALFUNCS_H__ */
