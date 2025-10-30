/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Basic routines for indexing spans
 */

#ifndef __SPAN_INDEX_H__
#define __SPAN_INDEX_H__

/* PostgreSQL */
#include <postgres.h>
#include <access/stratnum.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Structure to represent the bounding box of an inner node containing a
 * set of spans
 */
typedef struct
{
  Span left;
  Span right;
} SpanNode;

/**
 * @brief Structure to sort a set of spans of an inner node
 */
typedef struct SortedSpan
{
  Span s;
  int i;
} SortedSpan;

/*****************************************************************************/

extern int common_entry_cmp(const void *i1, const void *i2);

extern bool span_index_leaf_consistent(const Span *key, const Span *query,
  StrategyNumber strategy);
extern bool span_gist_inner_consistent(const Span *key, const Span *query,
  StrategyNumber strategy);
extern bool span_index_recheck(StrategyNumber strategy);

extern int span_lower_qsort_cmp(const void *a, const void *b);
extern int span_upper_qsort_cmp(const void *a, const void *b);
extern uint8 getQuadrant2D(const Span *centroid, const Span *query);

extern bool overlap2D(const SpanNode *nodebox, const Span *query);
extern bool contain2D(const SpanNode *nodebox, const Span *query);
extern bool left2D(const SpanNode *nodebox, const Span *query);
extern bool overLeft2D(const SpanNode *nodebox, const Span *query);
extern bool right2D(const SpanNode *nodebox, const Span *query);
extern bool overRight2D(const SpanNode *nodebox, const Span *query);
extern bool adjacent2D(const SpanNode *nodebox, const Span *query);
extern double distance_span_nodespan(Span *query, SpanNode *nodebox);

extern bool span_spgist_get_span(Datum value, meosType type, Span *result);

extern void spannode_init(SpanNode *nodebox, meosType spantype,
  meosType basetype);
extern SpanNode *spannode_copy(const SpanNode *orig);
extern void spannode_quadtree_next(const SpanNode *nodebox,
  const Span *centroid, uint8 quadrant, SpanNode *next_nodespan);
extern void spannode_kdtree_next(const SpanNode *nodebox, const Span *centroid,
  uint8 node, int level, SpanNode *next_nodespan);

/*****************************************************************************/

#endif /* __SPAN_INDEX_H__ */
