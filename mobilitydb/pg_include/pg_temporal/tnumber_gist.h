/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief R-tree GiST index for temporal integers and temporal floats
 */

#ifndef __PG_TNUMBER_GIST_H__
#define __PG_TNUMBER_GIST_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <access/gist.h>
#include <access/stratnum.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************/

/*****************************************************************************/

/* The split is computed by MEOS (#bbox_split); the extension reads and writes
 * the GiST entries around it */
extern Datum bbox_gist_picksplit(FunctionCallInfo fcinfo, MeosType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *));

/* The following functions are also called by tnumber_spgist.c */
extern bool tbox_index_leaf_consistent(const TBox *key, const TBox *query,
  StrategyNumber strategy);

/*****************************************************************************/

#endif /* __PG_TNUMBER_GIST_H__ */
