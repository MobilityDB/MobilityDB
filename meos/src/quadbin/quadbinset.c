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
 * @brief MEOS implementations of the set-returning quadbin functions,
 * returning `quadbinset`
 * @details Shape of every function below:
 * 1. Ask the quadbin kernel for the cell array (palloc'd).
 * 2. Pack the array into a Datum array.
 * 3. `set_make_free` the Datum array — the constructor copies into its
 * own storage and frees the input.
 * 4. Return the Set.
 */

#include "quadbin/quadbinset.h"

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
#include "quadbin/quadbin.h"

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return a quadbinset from a freshly-allocated Quadbin buffer of size
 * `count`
 * @details The input buffer is pfree'd. On empty result, returns NULL after
 * raising a meos_error (set_make requires count >= 1).
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
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_set_inout
 * @brief Return a QUADBIN cell set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
quadbinset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_QUADBINSET);
}

/**
 * @ingroup meos_quadbin_set_inout
 * @brief Return the string representation of a QUADBIN cell set
 * @param[in] s Set
 * @csqlfn #Set_out(), #Set_as_text()
 */
char *
quadbinset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(s, NULL);
  return set_out(s, 0);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_set_constructor
 * @brief Return a QUADBIN cell set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
quadbinset_make(const Quadbin *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = QuadbinGetDatum(values[i]);
  return set_make_free(datums, count, T_QUADBIN, ORDER);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_set_accessor
 * @brief Return the array of values of a QUADBIN cell set
 * @param[in] s Set
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Set_values(), #Set_unnest()
 */
Quadbin *
quadbinset_values(const Set *s, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(s, NULL);
  Quadbin *result = palloc(sizeof(Quadbin) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetQuadbin(SET_VAL_N(s, i));
  *count = s->count;
  return result;
}

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the set of QUADBIN cells within grid distance k of an origin cell
 * @csqlfn #Quadbin_grid_disk()
 */
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

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the set of QUADBIN cells within grid distance k of a cell of a
 * QUADBIN cell set
 * @details The union of the disks of the cells of the set, which at k = 1
 * widens a cover by the ring of cells around it
 * @param[in] cells Set of QUADBIN cells
 * @param[in] k Grid distance
 * @csqlfn #Quadbinset_grid_disk()
 */
Set *
quadbinset_grid_disk(const Set *cells, int k)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(cells, NULL);
  int capacity = cells->count * 9, count = 0;
  Datum *datums = palloc(sizeof(Datum) * (size_t) capacity);
  for (int i = 0; i < cells->count; i++)
  {
    Set *disk = quadbin_grid_disk(DatumGetQuadbin(SET_VAL_N(cells, i)), k);
    if (! disk)
    {
      pfree(datums);
      return NULL;
    }
    if (count + disk->count > capacity)
    {
      while (count + disk->count > capacity)
        capacity *= 2;
      datums = repalloc(datums, sizeof(Datum) * (size_t) capacity);
    }
    for (int j = 0; j < disk->count; j++)
      datums[count++] = SET_VAL_N(disk, j);
    pfree(disk);
  }
  return set_make_free(datums, count, T_QUADBIN, ORDER);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the set of children of a QUADBIN cell at a finer resolution
 * @csqlfn #Quadbin_cell_to_children()
 */
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

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the compacted set of a QUADBIN cell set
 * @details A cell covered by a coarser cell of the set is dropped, and every
 * four children of one parent merge into that parent, from the finest
 * resolution up, so the result covers the region of the set with the fewest
 * cells
 * @param[in] cells Set of QUADBIN cells
 * @csqlfn #Quadbinset_compact_cells()
 */
Set *
quadbinset_compact_cells(const Set *cells)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(cells, NULL);
  return dggs_quadtree_compact_cells(cells, T_TQUADBIN);
}

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the set of cells at a resolution covering a QUADBIN cell
 * set
 * @details A cell coarser than the resolution yields its descendants at the
 * resolution; a cell finer than the resolution raises an error
 * @param[in] cells Set of QUADBIN cells
 * @param[in] resolution Resolution of the result
 * @csqlfn #Quadbinset_uncompact_cells()
 */
Set *
quadbinset_uncompact_cells(const Set *cells, int resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_QUADBINSET(cells, NULL);
  return dggs_quadtree_uncompact_cells(cells, resolution, T_TQUADBIN,
    &quadbin_cell_to_children);
}

/*****************************************************************************/

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_set_conversion
 * @brief Convert a quadbin cell into a quadbin cell set
 * @param[in] cell Value
 * @csqlfn #Value_to_set()
 */
Set *
quadbin_to_set(Quadbin cell)
{
  Datum v = QuadbinGetDatum(cell);
  return set_make_exp(&v, 1, 1, T_QUADBIN, ORDER_NO);
}
