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
 * @brief The integer addresses of the points of the outline of an H3 cell,
 * read from the core library of H3 vendored under h3lib/
 * @details Decision D8 of the cell walks states each point of the outline of
 * an H3 cell as the double nearest the degrees of the value H3's formula
 * gives it. The inputs of that formula are integers: a face of the
 * icosahedron, the resolution of the substrate grid and a point of its
 * lattice, or for an extra vertex, the crossing of the segment between two
 * points of the lattice with an edge of the face. h3_vertex.c reads them from
 * the vendored library, whose headers it alone includes, so this header
 * names them in the integer types of pg_basetypes.h. A point of the lattice is given as (2x,
 * 2y / sqrt 3) of the planar point (x, y) H3 computes (`_ijkToHex2d`): two
 * integers, as are the ends of a face edge.
 */
/* NO-MEASUREMENT-NEEDED: an interface declaration, no behaviour of its own */

#ifndef __H3_VERTEX_H__
#define __H3_VERTEX_H__

#include <stdbool.h>
#include <pg_basetypes.h>

/** @brief The integer inputs of a point of the outline of an H3 cell */
typedef struct
{
  int face;              /**< Face of the icosahedron */
  int res;               /**< Resolution of the substrate grid, Class II */
  int64 p0[2];         /**< Point of the lattice, (2x, 2y / sqrt 3) */
  int64 p1[2];         /**< Second end of the segment of an extra vertex */
  int64 e0[2], e1[2];  /**< Ends of the face edge it crosses */
  bool crossing;         /**< The point is the crossing of p0 p1 with e0 e1 */
  bool substrate;        /**< The grid is the substrate: a point of an outline */
  bool rotate;           /**< The angle turns by M_AP7_ROT_RADS: the center of
                              a cell of a Class III resolution */
} H3VertexRecipe;

/** @brief The largest number of points of the boundary of a cell */
#define H3_BOUNDARY_MAX 10

extern bool h3_vertex_recipe(uint64 vertex, H3VertexRecipe *r);
extern int h3_edge_extra_recipe(uint64 edge, H3VertexRecipe *r);
extern int h3_edge_boundary_recipes(uint64 edge, H3VertexRecipe *out);
extern int h3_cell_vertexes(uint64 cell, uint64 *out, uint64 *across);
extern int h3_cell_boundary_recipes(uint64 cell, H3VertexRecipe *out);
extern bool h3_cell_center_recipe(uint64 cell, H3VertexRecipe *r);

#endif /* __H3_VERTEX_H__ */
