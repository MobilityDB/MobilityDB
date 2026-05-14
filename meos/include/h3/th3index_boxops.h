/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Internal declarations for the th3index bounding box functions.
 */

#ifndef __TH3INDEX_BOXOPS_H__
#define __TH3INDEX_BOXOPS_H__

/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * Function prototypes
 *****************************************************************************/

extern void th3indexinst_set_stbox(const TInstant *inst, STBox *box);
extern void th3indexinstarr_set_stbox(TInstant **instants, int count,
  STBox *box);
extern void th3indexseq_expand_stbox(const TSequence *seq,
  const TInstant *inst);

#endif /* __TH3INDEX_BOXOPS_H__ */
