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
 * @brief MEOS implementations of the set-returning quadbin functions,
 * returning `quadbinset`.
 *
 * Shape of every function below:
 *
 *   1. Ask the quadbin kernel for the cell array (palloc'd).
 *   2. Pack the array into a Datum array.
 *   3. `set_make_free` the Datum array — the constructor copies into its
 *      own storage and frees the input.
 *   4. Return the Set.
 */

#include "quadbin/quadbinset_meos.h"

#include <postgres.h>
#include <meos.h>
#include <meos_internal.h>

#include "quadbin/quadbin_meos.h"
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"  /* ORDER */

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return a quadbinset from a freshly-allocated Quadbin buffer of
 * size `count`. The input buffer is pfree'd. On empty result, returns
 * NULL after raising a meos_error (set_make requires count >= 1).
 */
static Set *
quadbinset_from_buffer(Quadbin *cells, int count)
{
  if (count <= 0)
  {
    if (cells)
      pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin returned no valid cells");
    return NULL;
  }
  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = QuadbinGetDatum(cells[i]);
  pfree(cells);
  return set_make_free(datums, count, T_QUADBIN, ORDER);
}

/*****************************************************************************
 * Grid traversal
 * @csqlfn #Quadbin_grid_disk()
 *****************************************************************************/

Set *
quadbin_grid_disk(Quadbin origin, int k)
{
  if (k < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin_grid_disk: k must be >= 0");
    return NULL;
  }
  int count;
  Quadbin *cells = quadbin_k_ring(origin, k, &count);
  return quadbinset_from_buffer(cells, count);
}

/*****************************************************************************
 * Hierarchy
 * @csqlfn #Quadbin_cell_to_children()
 *****************************************************************************/

Set *
quadbin_cell_to_children_set(Quadbin origin, int children_resolution)
{
  int count;
  Quadbin *cells = quadbin_cell_to_children(origin,
    (uint32_t) children_resolution, &count);
  if (! cells)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin_cell_to_children: children resolution must be finer than "
      "the cell resolution and within range");
    return NULL;
  }
  return quadbinset_from_buffer(cells, count);
}

/*****************************************************************************/
