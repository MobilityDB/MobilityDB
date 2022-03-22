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

#include <postgres.h>
#include <catalog/pg_type.h>
#include "general/temporal.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/* Network-based point */

typedef struct
{
  int64 rid;        /**< route identifier */
  double pos;       /**< position */
} npoint;

/* Network-based segment */

typedef struct
{
  int64 rid;       /**< route identifier */
  double pos1;     /**< position1 */
  double pos2;     /**< position2 */
} nsegment;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* npoint */
#define DatumGetNpoint(X)          ((npoint *) DatumGetPointer(X))
#define NpointGetDatum(X)          PointerGetDatum(X)
#define PG_GETARG_NPOINT(X)        DatumGetNpoint(PG_GETARG_DATUM(X))
#define PG_RETURN_NPOINT(X)        PG_RETURN_POINTER(X)

/* nsegment */
#define DatumGetNsegment(X)   ((nsegment *) DatumGetPointer(X))
#define NsegmentGetDatum(X)   PointerGetDatum(X)
#define PG_GETARG_NSEGMENT(X) DatumGetNsegment(PG_GETARG_DATUM(X))

/*****************************************************************************/

/* Input/output functions */

extern Datum tnpoint_in(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tnpoint_to_tgeompoint(PG_FUNCTION_ARGS);
extern Datum tgeompoint_to_tnpoint(PG_FUNCTION_ARGS);

extern Temporal *tnpoint_tgeompoint(const Temporal *temp);

/* Transformation functions */

extern Datum tnpoint_round(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum tnpoint_positions(PG_FUNCTION_ARGS);
extern Datum tnpoint_route(PG_FUNCTION_ARGS);
extern Datum tnpoint_routes(PG_FUNCTION_ARGS);

extern int64 tnpointinst_route(const TInstant *inst);

extern nsegment *tnpointseq_linear_positions(const TSequence *seq);
extern nsegment **tnpointseqset_positions(const TSequenceSet *ts, int *count);

/*****************************************************************************/

#endif /* __TNPOINT_H__ */
