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
 * @brief Basic routines for indexing temporal values
 */

#ifndef __TBOX_INDEX_H__
#define __TBOX_INDEX_H__

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
 * set of temporal boxes
 * @details The left box keeps, for the X and T dimensions, the ranges of the
 * lower bounds of the boxes in the quadrant, while the right box keeps the
 * ranges of the upper boxes.
 *
 * As an example, suppose that a quadrant contains two boxes 
 * @code
 * b1 = TBOXFLOAT XT([3, 5],[2001-01-03, 2001-01-05])
 * b3 = TBOXFLOAT XT([7, 9],[2001-01-07, 2001-01-09])
 * @endcode
 * The corresponding `TboxNode` will be 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 */
typedef struct
{
  TBox left;
  TBox right;
} TboxNode;

/**
 * @brief Structure to sort the temporal boxes of an inner node
 */
typedef struct SortedTbox
{
  TBox box;
  int i;
} SortedTbox;

/*****************************************************************************/

extern bool tbox_index_leaf_consistent(const TBox *key, const TBox *query,
  StrategyNumber strategy);
extern bool tbox_gist_inner_consistent(const TBox *key, const TBox *query,
  StrategyNumber strategy);
extern bool tbox_index_recheck(StrategyNumber strategy);

extern void tboxnode_init(TBox *centroid, TboxNode *nodebox);
extern TboxNode *tboxnode_copy(const TboxNode *box);
extern uint8 getQuadrant4D(const TBox *centroid, const TBox *inBox);
extern void tboxnode_quadtree_next(const TboxNode *nodebox,
  const TBox *centroid, uint8 quadrant, TboxNode *next_nodebox);
extern void tboxnode_kdtree_next(const TboxNode *nodebox, const TBox *centroid,
  uint8 node, int level, TboxNode *next_nodebox);
extern bool overlap4D(const TboxNode *nodebox, const TBox *query);
extern bool contain4D(const TboxNode *nodebox, const TBox *query);
extern bool left4D(const TboxNode *nodebox, const TBox *query);
extern bool overLeft4D(const TboxNode *nodebox, const TBox *query);
extern bool right4D(const TboxNode *nodebox, const TBox *query);
extern bool overRight4D(const TboxNode *nodebox, const TBox *query);
extern bool before4D(const TboxNode *nodebox, const TBox *query);
extern bool overBefore4D(const TboxNode *nodebox, const TBox *query);
extern bool after4D(const TboxNode *nodebox, const TBox *query);
extern bool overAfter4D(const TboxNode *nodebox, const TBox *query);
extern double distance_tbox_nodebox(const TBox *query,
  const TboxNode *nodebox);
extern bool tnumber_spgist_get_tbox(Datum value, meosType type, TBox *result);
extern int tbox_xmin_cmp(const TBox *box1, const TBox *box2);
extern int tbox_xmax_cmp(const TBox *box1, const TBox *box2);
extern int tbox_tmin_cmp(const TBox *box1, const TBox *box2);
extern int tbox_tmax_cmp(const TBox *box1, const TBox *box2);
extern int tbox_level_cmp(TBox *centroid, TBox *query, int level);

/*****************************************************************************/

#endif /* __TBOX_INDEX_H__ */
