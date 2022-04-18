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
 * @file tpoint_boxops.h
 * Bounding box operators for temporal points.
 */

#ifndef __TPOINT_BOXOPS_H__
#define __TPOINT_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "point/stbox.h"

/*****************************************************************************/

/* Functions computing the bounding box at the creation of a temporal point */

extern void tpointinst_stbox(const TInstant *inst, STBOX *box);
extern void tgeompointinstarr_stbox(const TInstant **inst, int count,
  STBOX *box);
extern void tgeogpointinstarr_stbox(const TInstant **instants, int count,
  STBOX *box);
extern void tpointseqarr_stbox(const TSequence **seq, int count, STBOX *box);

/* Boxes functions */

extern STBOX *tpointseq_stboxes(const TSequence *seq, int *count);
extern STBOX *tpointseqset_stboxes(const TSequenceSet *ts, int *count);
extern STBOX * tpoint_stboxes(const Temporal *temp, int *count);

/* Generic box functions */

extern int boxop_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBOX *, const STBOX *), bool invert);
extern Datum boxop_tpoint_stbox(const Temporal *temp, const STBOX *box,
  bool (*func)(const STBOX *, const STBOX *), bool invert);
extern bool boxop_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBOX *, const STBOX *));

extern Datum boxop_geo_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *));
extern Datum boxop_tpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *));
extern Datum boxop_stbox_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *));
extern Datum boxop_tpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *));
extern Datum boxop_tpoint_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *));

/*****************************************************************************/

#endif
