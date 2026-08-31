/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Set-returning static helpers on `s2cell`.
 *
 * These wrap the S2 cell kernel's array-returning primitives into the
 * `s2cellset` MobilityDB set type. All functions here operate on STATIC
 * `s2cell` values; the temporal lift is handled by the ts2cell layer.
 */

#ifndef __S2CELLSET_H__
#define __S2CELLSET_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>  /* Set typedef */
#include <meos_s2cell.h>

/*****************************************************************************
 * Set-returning S2 functions
 *
 * All functions below return a heap-allocated `Set *`. The caller owns the
 * return. NULL is returned on failure after raising a `meos_error`.
 *****************************************************************************/

/** Return the four cells sharing an edge with `cell`. */
extern Set *s2cell_edge_neighbors_set(S2CellId cell);

/** Return the children of `cell` at level `children_level`. */
extern Set *s2cell_cell_to_children_set(S2CellId cell, int children_level);

#endif /* __S2CELLSET_H__ */
