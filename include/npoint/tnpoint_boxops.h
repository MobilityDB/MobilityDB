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
 * @file tnpoint_boxops.h
 * Bounding box operators for temporal network points.
 */

#ifndef __TNPOINT_BOXOPS_H__
#define __TNPOINT_BOXOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "general/temporal.h"
#include "tnpoint.h"

/*****************************************************************************/

extern Datum npoint_to_stbox(PG_FUNCTION_ARGS);
extern Datum npoint_timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum npoint_period_to_stbox(PG_FUNCTION_ARGS);
extern Datum tnpoint_to_stbox(PG_FUNCTION_ARGS);

extern bool npoint_stbox(const npoint *np, STBOX *box);
extern void tnpointinst_make_stbox(const TInstant *inst, STBOX *box);
extern void tnpointinstarr_stbox(const TInstant **inst, int count, STBOX *box);
extern void tnpointseq_make_stbox(const TInstant **inst, int count,
  bool linear, STBOX *box);

extern Datum overlaps_bbox_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnpoint_npoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnpoint_npoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnpoint_npoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnpoint_npoint(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_tnpoint_npoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif /* __TNPOINT_BOXOPS_H__ */
