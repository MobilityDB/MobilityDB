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
 * @brief In memory index for STBox based on RTree
 */

#ifndef __TPOINT_RTREE__
#define __TPOINT_RTREE__

/* MEOS */
#include <meos.h>

#include "temporal/meos_catalog.h"

/*****************************************************************************
 * Definitions
 *****************************************************************************/

#define MAXITEMS 64
#define SEARCH_ARRAY_STARTING_SIZE 64
#define MINITEMS_PERCENTAGE 10
#define MINITEMS ((MAXITEMS) * (MINITEMS_PERCENTAGE) / 100 + 1)
#define RTREE_INNER_NODE_NO true
#define RTREE_INNER_NODE false

/*****************************************************************************
 * Structs
 *****************************************************************************/

/**
 * @brief Internal representation of an RTree node.
 */
typedef struct RTreeNode{
  bool kind;
  int count;
  union 
  {
    struct RTreeNode * nodes[MAXITEMS];
    int64 ids[MAXITEMS];
  };
  // TODO: Find a way to include box and span in the definition.
  STBox boxes[MAXITEMS];
} RTreeNode;

/**
 * @brief Rtree in memory index basic structure.
 * 
 * It works based on STBox. The spliting criteria is based on the largest axis.
 * The inserting criteria is based on least enlarging square.
 *
 * The get axis function makes it ease to implement with X,Y,Z and time or any
 * combination that you may want.
 */
struct RTree {
  meosType basetype;
  int dims;
  RTreeNode *root;
  STBox box;
  double (*get_axis)(const STBox*, int, bool);
};

/*****************************************************************************/

#endif /* __TPOINT_RTREE__ */
