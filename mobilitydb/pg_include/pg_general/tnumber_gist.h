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
 * @brief R-tree GiST index for temporal integers and temporal floats
 */

#ifndef __TNUMBER_GIST_H__
#define __TNUMBER_GIST_H__

/* PostgreSQL */
#include <postgres.h>
#include <access/gist.h>
#include <access/stratnum.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

/**
 * Structure keeping context for the function stbox_gist_consider_split.
 *
 * Contains information about currently selected split and some general
 * information.
 */
typedef struct
{
  int  entriesCount;  /**< total number of entries being split */
  bboxunion boundingBox;  /**< minimum bounding box across all entries */
  /** Information about currently selected split follows */
  bool first;        /**< true if no split was selected yet */
  double leftUpper;  /**< upper bound of left interval */
  double rightLower; /**< lower bound of right interval */
  float4 ratio;
  float4 overlap;
  int  dim;          /**< axis of this split */
  double range;      /**< width of general MBR projection to the selected axis */
} ConsiderSplitContext;

/*****************************************************************************/

/* The following functions are also called by tpoint_gist.c */
extern void bbox_gist_fallback_split(GistEntryVector *entryvec,
  GIST_SPLITVEC *v, mobdbType bboxtype, void (*bbox_adjust)(void *, void *));
extern int interval_cmp_lower(const void *i1, const void *i2);
extern int interval_cmp_upper(const void *i1, const void *i2);
extern float non_negative(float val);
extern void bbox_gist_consider_split(ConsiderSplitContext *context, int dimNum,
  mobdbType bboxtype, double rightLower, int minLeftCount, double leftUpper,
  int maxLeftCount);
extern Datum bbox_gist_picksplit_ext(FunctionCallInfo fcinfo, mobdbType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *));

/* The following functions are also called by tnumber_spgist.c */
extern bool tbox_index_consistent_leaf(const TBOX *key, const TBOX *query,
  StrategyNumber strategy);

/*****************************************************************************/

#endif
