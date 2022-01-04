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

#include <postgres.h>
#include <catalog/pg_type.h>

#include "general/temporal.h"
#include "tnpoint_static.h"

/*****************************************************************************/

/* Parameter tests */

extern void ensure_same_srid_tnpoint_stbox(const Temporal *temp,
  const STBOX *box);
extern void ensure_same_rid_tnpointinst(const TInstant *inst1,
  const TInstant *inst2);

/* Interpolation functions */

extern bool tnpointseq_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t);

/* Functions for spatial reference systems */

extern int tnpoint_srid_internal(const Temporal *temp);

extern Datum tnpoint_trajectory(PG_FUNCTION_ARGS);

extern Datum tnpointseq_trajectory1(const TInstant *inst1, const TInstant *inst2);
extern Datum tnpointseq_trajectory(const TSequence *seq);
extern Datum tnpointseqset_trajectory(const TSequenceSet *ts);

extern Datum tnpointinst_geom(const TInstant *inst);
extern Datum tnpointinstset_geom(const TInstantSet *ti);
extern Datum tnpointseq_geom(const TSequence *seq);
extern Datum tnpointseqset_geom(const TSequenceSet *ts);
extern Datum tnpoint_geom(const Temporal *temp);

extern Datum tnpoint_length(PG_FUNCTION_ARGS);
extern Datum tnpoint_cumulative_length(PG_FUNCTION_ARGS);
extern Datum tnpoint_speed(PG_FUNCTION_ARGS);
extern Datum tnpoint_twcentroid(PG_FUNCTION_ARGS);
extern Datum tnpoint_azimuth(PG_FUNCTION_ARGS);
extern Datum tnpoint_at_geometry(PG_FUNCTION_ARGS);
extern Datum tnpoint_minus_geometry(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif /* __TNPOINT_SPATIALFUNCS_H__ */
