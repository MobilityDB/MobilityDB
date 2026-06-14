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
 * @file
 * @brief Set-returning static helpers on `quadbin`.
 *
 * These wrap the quadbin cell kernel's array-returning primitives into
 * the `quadbinset` MobilityDB set type. They are the square-quadtree
 * counterpart of the h3index set-returning functions, dropping the
 * hexagon-only families (directed edges, vertexes, icosahedron faces,
 * compact / uncompact, grid path / ring) and keeping the operations with
 * a square analogue:
 *
 *   * the k-ring (square Chebyshev disk) around a cell,
 *   * the four-per-level children of a cell at a finer resolution.
 *
 * All functions here operate on STATIC `quadbin` values; the temporal
 * lift is handled by the tquadbin layer.
 */

#ifndef __QUADBINSET_MEOS_H__
#define __QUADBINSET_MEOS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>  /* Set typedef */
#include <meos_quadbin.h>

/*****************************************************************************
 * Set-returning quadbin functions
 *
 * All functions below return a heap-allocated `Set *`. The caller owns
 * the return. NULL is returned on failure after raising a `meos_error`.
 *****************************************************************************/

/**
 * Return all cells within `k` grid steps of `origin` (the square
 * Chebyshev disk; `origin` itself is included at k=0).
 */
extern Set *quadbin_grid_disk(Quadbin origin, int k);

/**
 * Return all children of `origin` at resolution `children_resolution`
 * (fails if the target resolution is not finer than `origin`).
 */
extern Set *quadbin_cell_to_children_set(Quadbin origin,
  int children_resolution);

#endif /* __QUADBINSET_MEOS_H__ */
