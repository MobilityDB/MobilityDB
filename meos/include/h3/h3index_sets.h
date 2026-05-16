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
 * @brief Set-returning static helpers on `h3index`.
 *
 * These are the MobilityDB ports of the nine h3-pg SETOF-returning
 * functions. Each call returns a finite collection of H3 cells
 * (or icosahedron face indexes), packaged as the right
 * MobilityDB set type — `h3indexset` for cell collections, `intset`
 * for the face index list.
 *
 * All functions here operate on STATIC `h3index` values — they
 * are not lifted to the temporal `th3index` type. The temporal
 * lift (producing a `tset<h3indexset>` or similar) is parked
 * pending a `tset<T>` primitive design.
 *
 * libh3 is the ultimate authority for semantics; this file is a
 * thin allocate / call / wrap layer:
 *
 *   size-query → palloc → libh3 fill → filter zeros → set_make_free.
 */

#ifndef __H3INDEX_SETS_H__
#define __H3INDEX_SETS_H__

/* PostgreSQL */
#include <postgres.h>
/* H3 */
#include <h3api.h>
/* MEOS */
#include <meos.h>  /* Set typedef */
#include "temporal/meos_catalog.h"

/*****************************************************************************
 * Set-returning h3 functions
 *
 * All functions below return a heap-allocated `Set *`. The caller
 * owns the return. NULL is returned on libh3 failure after raising
 * a `meos_error`.
 *****************************************************************************/

/**
 * Return all cells within `k` grid steps of `origin` (including
 * `origin` itself at k=0).
 */
extern Set *h3_grid_disk(H3Index origin, int k);

/**
 * Return all cells at exactly `k` grid steps from `origin`.
 * Fails near pentagons (libh3's unsafe ring).
 */
extern Set *h3_grid_ring(H3Index origin, int k);

/**
 * Return the cells on the inclusive path from `start` to `end`.
 * Fails on non-comparable resolutions or paths crossing pentagons.
 */
extern Set *h3_grid_path_cells(H3Index start, H3Index end);

/**
 * Return all children of `origin` at resolution `childRes`.
 */
extern Set *h3_cell_to_children(H3Index origin, int childRes);

/**
 * Return the compacted representation of `cells` (finer cells
 * merged up into parents where the full hexagonal set of siblings
 * is present).
 */
extern Set *h3_compact_cells(const Set *cells);

/**
 * Return the uncompacted representation of `cells` at resolution
 * `res` (fails if any input is finer than `res`).
 */
extern Set *h3_uncompact_cells(const Set *cells, int res);

/**
 * Return all outgoing directed edges of `origin` (up to 6;
 * pentagons have 5).
 */
extern Set *h3_origin_to_directed_edges(H3Index origin);

/**
 * Return all vertexes of `cell` (up to 6; pentagons have 5).
 */
extern Set *h3_cell_to_vertexes(H3Index cell);

/**
 * Return the icosahedron face indexes intersected by `cell` as
 * an intset. Each face index is in 0..19.
 */
extern Set *h3_get_icosahedron_faces(H3Index cell);

#endif /* __H3INDEX_SETS_H__ */
