/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief TPCBox bounding-box helpers for temporal pointcloud types
 *   (tpcpoint, tpcpatch). Mirrors the tspatial_boxops.h API for the
 *   STBox case.
 */

#ifndef __TPC_BOXOPS_H__
#define __TPC_BOXOPS_H__

#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"

extern void tpointcloudinst_set_tpcbox(const TInstant *inst, TPCBox *box);
extern void tpointcloudinstarr_set_tpcbox(TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, TPCBox *box);
extern void tpointcloudseq_expand_tpcbox(TSequence *seq, const TInstant *inst);
extern void tpointcloudseqarr_set_tpcbox(TSequence **sequences, int count,
  TPCBox *box);

#endif /* __TPC_BOXOPS_H__ */
