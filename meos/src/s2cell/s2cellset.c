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
 * @brief MEOS implementations of the set-returning S2 functions, returning
 * `s2cellset`.
 * @details Shape of every function below:
 *   1. Ask the S2 kernel for the cell array (palloc'd).
 *   2. Pack the array into a Datum array.
 *   3. `set_make_free` the Datum array — the constructor copies into its own
 *      storage and frees the input.
 *   4. Return the Set.
 *
 * These are the quadtree-on-the-sphere counterpart of the quadbin set
 * helpers, keeping the operations an S2 cell answers — the four cells sharing
 * an edge with a cell, and the four-per-level children of a cell — and
 * dropping the square-grid k-ring, which the Hilbert curve does not induce.
 */

#include "s2cell/s2cellset.h"

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/meos_catalog.h"
#include "temporal/set.h"  /* ensure_set_isof_type */
#include "temporal/temporal.h"  /* ORDER */
#include "temporal/type_parser.h"  /* set_parse */
#include "temporal/tcellindex.h"
#include "s2cell/s2cell.h"

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return an s2cellset from a freshly-allocated S2CellId buffer of size
 * @p count
 * @details The input buffer is pfree'd. On an empty result the function
 * returns NULL after raising a meos_error, `set_make` requiring count >= 1.
 */
static Set *
s2cellset_from_buffer(S2CellId *cells, int count)
{
  if (count <= 0)
  {
    if (cells)
      pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The S2 cell has no valid cells to answer with");
    return NULL;
  }
  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = S2CellGetDatum(cells[i]);
  pfree(cells);
  return set_make_free(datums, count, T_S2CELL, ORDER);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_set_inout
 * @brief Return an S2 cell set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
s2cellset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_S2CELLSET);
}

/**
 * @ingroup meos_s2cell_set_inout
 * @brief Return the string representation of an S2 cell set
 * @param[in] s Set
 * @csqlfn #Set_out(), #Set_as_text()
 */
char *
s2cellset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_S2CELLSET(s, NULL);
  return set_out(s, 0);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_set_constructor
 * @brief Return an S2 cell set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
s2cellset_make(const S2CellId *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = S2CellGetDatum(values[i]);
  return set_make_free(datums, count, T_S2CELL, ORDER);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_set_accessor
 * @brief Return the array of values of an S2 cell set
 * @param[in] s Set
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Set_values(), #Set_unnest()
 */
S2CellId *
s2cellset_values(const Set *s, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_S2CELLSET(s, NULL);
  S2CellId *result = palloc(sizeof(S2CellId) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetS2Cell(SET_VAL_N(s, i));
  *count = s->count;
  return result;
}

/*****************************************************************************
 * Traversal
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the set of the four S2 cells sharing an edge with a cell
 * @details A neighbour across a cube-face boundary belongs to the adjacent
 * face, which the kernel resolves, so the set holds four cells at every level
 * including the six face cells.
 * @param[in] cell S2 cell
 * @csqlfn #S2cell_edge_neighbors()
 */
Set *
s2cell_edge_neighbors_set(S2CellId cell)
{
  int count;
  S2CellId *cells = s2cell_edge_neighbors(cell, &count);
  return s2cellset_from_buffer(cells, count);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the set of children of an S2 cell at a finer level
 * @param[in] cell S2 cell
 * @param[in] children_level Level of the children, finer than the cell's own
 * @csqlfn #S2cell_cell_to_children()
 */
Set *
s2cell_cell_to_children_set(S2CellId cell, int children_level)
{
  int count;
  S2CellId *cells = s2cell_cell_to_children(cell, (uint32_t) children_level,
    &count);
  if (! cells)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The children level must be finer than the cell level and within range");
    return NULL;
  }
  return s2cellset_from_buffer(cells, count);
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the compacted set of an S2 cell set
 * @details A cell covered by a coarser cell of the set is dropped, and every
 * four children of one parent merge into that parent, from the finest
 * resolution up, so the result covers the region of the set with the fewest
 * cells
 * @param[in] cells Set of S2 cells
 * @csqlfn #S2cellset_compact_cells()
 */
Set *
s2cellset_compact_cells(const Set *cells)
{
  /* Ensure the validity of the arguments */
  VALIDATE_S2CELLSET(cells, NULL);
  return dggs_quadtree_compact_cells(cells, T_TS2CELL);
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the set of cells at a resolution covering an S2 cell
 * set
 * @details A cell coarser than the resolution yields its descendants at the
 * resolution; a cell finer than the resolution raises an error
 * @param[in] cells Set of S2 cells
 * @param[in] resolution Resolution of the result
 * @csqlfn #S2cellset_uncompact_cells()
 */
Set *
s2cellset_uncompact_cells(const Set *cells, int resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_S2CELLSET(cells, NULL);
  return dggs_quadtree_uncompact_cells(cells, resolution, T_TS2CELL,
    &s2cell_cell_to_children);
}

/*****************************************************************************/

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_set_conversion
 * @brief Convert an S2 cell into a S2 cell set
 * @param[in] cell Value
 * @csqlfn #Value_to_set()
 */
Set *
s2cell_to_set(S2CellId cell)
{
  Datum v = S2CellGetDatum(cell);
  return set_make_exp(&v, 1, 1, T_S2CELL, ORDER_NO);
}
