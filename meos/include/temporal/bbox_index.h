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
 * @brief Splitting a set of bounding boxes in two for an index node
 * @details The double sorting split of Korotkov, shared by the GiST operator
 * classes of the extension and the in-memory RTree of MEOS
 */

#ifndef __BBOX_INDEX_H__
#define __BBOX_INDEX_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"

/*****************************************************************************/

/* The NaN-aware comparison of pgtypes, declared here rather than through
 * pgtypes.h, which the extension cannot include beside the server headers */
extern int pg_float8_cmp(float8 a, float8 b);

/* Minimum accepted ratio of split */
#define LIMIT_RATIO 0.3

/* Convenience macros for NaN-aware comparisons */
#define FLOAT8_EQ(a,b)   (pg_float8_cmp(a, b) == 0)
#define FLOAT8_LT(a,b)   (pg_float8_cmp(a, b) < 0)
#define FLOAT8_LE(a,b)   (pg_float8_cmp(a, b) <= 0)
#define FLOAT8_GT(a,b)   (pg_float8_cmp(a, b) > 0)
#define FLOAT8_GE(a,b)   (pg_float8_cmp(a, b) >= 0)
#define FLOAT8_MAX(a,b)  (FLOAT8_GT(a, b) ? (a) : (b))
#define FLOAT8_MIN(a,b)  (FLOAT8_LT(a, b) ? (a) : (b))

/**
 * @brief Entry that can be placed in either group without affecting the
 * overlap on the selected axis ("common entry")
 */
typedef struct
{
  int index;      /**< Index of the entry in the initial array */
  double delta;   /**< Delta between the penalties of inserting the entry
                       into either group */
} CommonEntry;

/**
 * @brief Projection of a bounding box on an axis
 */
typedef struct
{
  double lower;
  double upper;
} SplitInterval;

/*****************************************************************************/

extern int common_entry_cmp(const void *i1, const void *i2);
extern void bbox_split(MeosType bboxtype, void **boxes, int count,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *),
  bool *left);

/*****************************************************************************/

#endif /* __BBOX_INDEX_H__ */
