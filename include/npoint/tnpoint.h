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
 * @file tnpoint.h
 * Functions for temporal network points.
 */

#ifndef __TNPOINT_H__
#define __TNPOINT_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/* Network-based point */

typedef struct
{
  int64 rid;        /**< route identifier */
  double pos;       /**< position */
} Npoint;

/* Network-based segment */

typedef struct
{
  int64 rid;       /**< route identifier */
  double pos1;     /**< position1 */
  double pos2;     /**< position2 */
} Nsegment;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* Npoint */
#define DatumGetNpointP(X)         ((Npoint *) DatumGetPointer(X))
#define NpointPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_NPOINT_P(X)      DatumGetNpointP(PG_GETARG_DATUM(X))
#define PG_RETURN_NPOINT_P(X)      PG_RETURN_POINTER(X)

/* Nsegment */
#define DatumGetNsegmentP(X)       ((Nsegment *) DatumGetPointer(X))
#define NsegmentPGetDatum(X)       PointerGetDatum(X)
#define PG_GETARG_NSEGMENT_P(X)    DatumGetNsegmentP(PG_GETARG_DATUM(X))

/*****************************************************************************/

/* Input/output functions */

/* Cast functions */

extern TInstant *tnpointinst_tgeompointinst(const TInstant *inst);
extern TInstantSet *tnpointinstset_tgeompointinstset(const TInstantSet *ti);
extern TSequence *tnpointseq_tgeompointseq(const TSequence *seq);
extern TSequenceSet *tnpointseqset_tgeompointseqset(const TSequenceSet *ts);
extern Temporal *tnpoint_tgeompoint(const Temporal *temp);

extern TInstant *tgeompointinst_tnpointinst(const TInstant *inst);
extern TInstantSet *tgeompointinstset_tnpointinstset(const TInstantSet *ti);
extern TSequence *tgeompointseq_tnpointseq(const TSequence *seq);
extern TSequenceSet *tgeompointseqset_tnpointseqset(const TSequenceSet *ts);
extern Temporal *tgeompoint_tnpoint(const Temporal *temp);

/* Transformation functions */

extern Temporal *tnpoint_round(const Temporal *temp, Datum size);

/* Accessor functions */

extern Nsegment **tnpointinst_positions(const TInstant *inst);
extern Nsegment **tnpointinstset_positions(const TInstantSet *ti, int *count);
extern Nsegment **tnpointseq_positions(const TSequence *seq, int *count);
extern Nsegment **tnpointseqset_positions(const TSequenceSet *ts, int *count);
extern Nsegment **tnpoint_positions(const Temporal *temp, int *count);
extern int64 tnpointinst_route(const TInstant *inst);
extern int64 tnpoint_route(const Temporal *temp);
extern int64 *tnpointinst_routes(const TInstant *inst);
extern int64 *tnpointinstset_routes(const TInstantSet *ti);
extern int64 *tnpointseq_routes(const TSequence *seq);
extern int64 *tnpointseqset_routes(const TSequenceSet *ts);
extern int64 *tnpoint_routes(const Temporal *temp, int *count);

extern Nsegment *tnpointseq_linear_positions(const TSequence *seq);

/*****************************************************************************/

#endif /* __TNPOINT_H__ */
