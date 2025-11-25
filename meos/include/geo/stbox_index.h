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
 * @brief Basic routines for indexing temporal numbers
 */

#ifndef __STBOX_INDEX_H__
#define __STBOX_INDEX_H__

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
 * @brief Structure to represent the bounding box of a spatiotemporal value
 * as a 6- or 8-dimensional point depending on whether the spatiotemporal
 * value is in 2D+T or 3D+T.
 */
typedef struct
{
  STBox left;
  STBox right;
} STboxNode;

/**
 * @brief Structure to sort the temporal boxes of an inner node
 */
typedef struct SortedSTbox
{
  STBox box;
  int i;
} SortedSTbox;

/*****************************************************************************/

extern bool stbox_index_leaf_consistent(const STBox *key, const STBox *query,
  StrategyNumber strategy);
extern bool stbox_gist_inner_consistent(const STBox *key, const STBox *query,
  StrategyNumber strategy);
extern bool stbox_index_recheck(StrategyNumber strategy);

extern STboxNode *stboxnode_copy(const STboxNode *box);
extern uint8 getQuadrant8D(const STBox *centroid, const STBox *inBox);
extern void stboxnode_init(const STBox *centroid, STboxNode *nodebox);
extern void stboxnode_quadtree_next(const STboxNode *nodebox,
  const STBox *centroid, uint8 quadrant, STboxNode *next_nodebox);
extern void stboxnode_kdtree_next(const STboxNode *nodebox,
  const STBox *centroid, uint8 node, int level, STboxNode *next_nodebox);
extern bool overlap8D(const STboxNode *nodebox, const STBox *query);
extern bool overlapKD(const STboxNode *nodebox, const STBox *query, int level);
extern bool contain8D(const STboxNode *nodebox, const STBox *query);
extern bool containKD(const STboxNode *nodebox, const STBox *query, int level);
extern bool left8D(const STboxNode *nodebox, const STBox *query);
extern bool overLeft8D(const STboxNode *nodebox, const STBox *query);
extern bool right8D(const STboxNode *nodebox, const STBox *query);
extern bool overRight8D(const STboxNode *nodebox, const STBox *query);
extern bool below8D(const STboxNode *nodebox, const STBox *query);
extern bool overBelow8D(const STboxNode *nodebox, const STBox *query);
extern bool above8D(const STboxNode *nodebox, const STBox *query);
extern bool overAbove8D(const STboxNode *nodebox, const STBox *query);
extern bool front8D(STboxNode *nodebox, STBox *query);
extern bool overFront8D(const STboxNode *nodebox, const STBox *query);
extern bool back8D(const STboxNode *nodebox, const STBox *query);
extern bool overBack8D(const STboxNode *nodebox, const STBox *query);
extern bool before8D(const STboxNode *nodebox, const STBox *query);
extern bool overBefore8D(const STboxNode *nodebox, const STBox *query);
extern bool after8D(const STboxNode *nodebox, const STBox *query);
extern bool overAfter8D(const STboxNode *nodebox, const STBox *query);
extern double distance_stbox_nodebox(const STBox *query,
  const STboxNode *nodebox);
extern bool tspatial_spgist_get_stbox(Datum value, meosType type,
  STBox *result);

/*****************************************************************************/

#endif /* __STBOX_INDEX_H__ */
