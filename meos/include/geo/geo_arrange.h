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
 * @brief The arrangement of closed curves of segments and circular arcs, and
 * the boundary of the region they wind around
 */

#ifndef __GEO_ARRANGE_H__
#define __GEO_ARRANGE_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include "geo/geo_funcs.h"

/**
 * @brief What an arrangement answers, or the question it leaves unanswered
 */
typedef enum
{
  ARR_ANSWERED,     /**< The boundary is answered */
  ARR_NOT_CLOSED,   /**< The edges are not closed curves sharing their ends */
  ARR_OVERLAP,      /**< Two arcs run along each other */
  ARR_MEETING,      /**< Where two edges meet is not decided */
  ARR_ORDER,        /**< The order of two meetings along an edge is not
                         decided */
  ARR_TURN,         /**< The order of the edges around a node is not decided */
  ARR_DEPTH,        /**< The winding numbers do not agree around a node */
  ARR_SHELLS        /**< The boundary is not one shell and its holes */
} ArrangeStatus;

/**
 * @brief A piece of a ring an arrangement answers, in the direction it is
 * walked: a segment from its start to its end, or an arc through its start,
 * a middle and its end
 */
typedef struct
{
  double x1, y1;   /**< Start */
  double x2, y2;   /**< End */
  double mx, my;   /**< Middle of an arc */
  bool arc;        /**< An arc rather than a segment */
} ArrangePiece;

extern ArrangeStatus arrange_wound_boundary(const MeosArray *edges,
  MeosArray ***rings, uint32_t *nrings);
extern ArrangeStatus arrange_wound_surfaces(const MeosArray *edges,
  MeosArray ***rings, uint32_t *nrings, int32_t **owner);
extern ArrangeStatus arrange_union_surfaces(const MeosArray *edges,
  const uint32_t *soff, uint32_t nsurf, MeosArray ***rings, uint32_t *nrings,
  int32_t **owner);
extern ArrangeStatus arrange_union_surfaces_lines(const MeosArray *edges,
  const uint32_t *soff, uint32_t nsurf, const Edge *lines, uint32_t nlines,
  const POINT2D *pts, uint32_t npts, MeosArray ***rings, uint32_t *nrings,
  int32_t **owner, MeosArray **lout, bool *pkeep);
extern void arrange_rings_free(MeosArray **rings, uint32_t nrings);

#endif /* __GEO_ARRANGE_H__ */
