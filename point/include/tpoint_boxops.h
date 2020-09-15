/*****************************************************************************
 *
 * tpoint_boxops.h
 *    Bounding box operators for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_BOXOPS_H__
#define __TPOINT_BOXOPS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#include <liblwgeom.h>

#include "temporal.h"
#include "temporal_util.h"
#include "stbox.h"

/*****************************************************************************/

/* Functions computing the bounding box at the creation of the temporal point */

extern void tpointinst_make_stbox(STBOX *box, const TInstant *inst);
extern void tpointinstarr_to_stbox(STBOX *box, TInstant **inst, int count);
extern void tpointseqarr_to_stbox(STBOX *box, TSequence **seq, int count);

/* Boxes functions */

extern Datum tpoint_stboxes(PG_FUNCTION_ARGS);

extern ArrayType *tpointseq_stboxes(const TSequence *seq);
extern ArrayType *tpointseqset_stboxes(const TSequenceSet *ts);

/*****************************************************************************/

#endif
