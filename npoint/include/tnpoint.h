/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/* Network-based point */

typedef struct
{
  int64   rid;       /* route identifier */
  double    pos;       /* position */
} npoint;

/* Network-based segment */

typedef struct
{
  int64   rid;      /* route identifier */
  double    pos1;     /* position1 */
  double    pos2;     /* position2 */
} nsegment;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* npoint */
#define DatumGetNpoint(X)   ((npoint *) DatumGetPointer(X))
#define NpointGetDatum(X)   PointerGetDatum(X)
#define PG_GETARG_NPOINT(i)   ((npoint *) PG_GETARG_POINTER(i))

/* nsegment */
#define DatumGetNsegment(X)   ((nsegment *) DatumGetPointer(X))
#define NsegmentGetDatum(X)   PointerGetDatum(X)
#define PG_GETARG_NSEGMENT(i) ((nsegment *) PG_GETARG_POINTER(i))

/*****************************************************************************
 * TNPoint.c
 *****************************************************************************/

extern Datum tnpoint_in(PG_FUNCTION_ARGS);

extern Datum tnpoint_make_tnpointseq(PG_FUNCTION_ARGS);

extern Datum tnpoint_as_tgeompoint(PG_FUNCTION_ARGS);
extern Datum tgeompoint_as_tnpoint(PG_FUNCTION_ARGS);

extern TInstant *tnpointinst_as_tgeompointinst(const TInstant *inst);
extern TInstantSet *tnpointi_as_tgeompointi(const TInstantSet *ti);
extern TSequence *tnpointseq_as_tgeompointseq(const TSequence *seq);
extern TSequenceSet *tnpoints_as_tgeompoints(const TSequenceSet *ts);
extern Temporal *tnpoint_as_tgeompoint_internal(const Temporal *temp);
extern Temporal *tgeompoint_as_tnpoint_internal(Temporal *temp);

extern TInstant *tgeompointinst_as_tnpointinst(const TInstant *inst);
extern TInstantSet *tgeompointi_as_tnpointi(const TInstantSet *ti);
extern TSequence *tgeompointseq_as_tnpointseq(const TSequence *seq);
extern TSequenceSet *tgeompoints_as_tnpoints(const TSequenceSet *ts);

extern Datum tnpoint_positions(PG_FUNCTION_ARGS);
extern Datum tnpoint_route(PG_FUNCTION_ARGS);
extern Datum tnpoint_routes(PG_FUNCTION_ARGS);

extern int64 tnpointinst_route(const TInstant *inst);
extern int64 tnpointiseq_route(const TSequence *seq);
extern nsegment **tnpointinst_positions(const TInstant *inst);
extern nsegment **tnpointi_positions(const TInstantSet *ti, int *count);
extern nsegment *tnpointseq_linear_positions(const TSequence *seq);
extern nsegment **tnpointseq_positions(const TSequence *seq, int *count);
extern nsegment **tnpoints_positions(const TSequenceSet *ts, int *count);
extern nsegment **tnpoint_positions_internal(const Temporal *temp, int *count);

extern ArrayType *tnpointinst_routes(const TInstant *inst);
extern ArrayType *tnpointi_routes(const TInstantSet *ti);
extern ArrayType *tnpointseq_routes(const TSequence *seq);
extern ArrayType *tnpoints_routes(const TSequenceSet *ts);

/*****************************************************************************/

#endif /* __TNPOINT_H__ */
