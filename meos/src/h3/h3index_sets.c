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
 * @brief MEOS implementations of the nine set-returning h3-pg
 * functions, returning `h3indexset` / `intset`.
 *
 * Shape of every function below:
 *
 *   1. Ask libh3 for the output buffer size (where applicable).
 *   2. `palloc` the buffer.
 *   3. Call the libh3 fill function.
 *   4. Walk the buffer, dropping any 0 entries libh3 used as
 *      "no cell here" padding (near pentagons, icosahedron faces,
 *      etc.).
 *   5. `set_make_free` the Datum array — the constructor copies
 *      into its own storage and frees the input.
 *   6. Return the Set.
 */

#include "h3/h3index_sets.h"

#include <postgres.h>
#include <meos.h>
#include <meos_internal.h>

#include "h3/h3index.h"
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"  /* ORDER / SET_VAL_N */

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Pack a possibly-padded H3Index array into a Datum array,
 * dropping any zero entries. The input buffer is pfree'd.
 */
static Datum *
h3index_array_compact(H3Index *cells, int64_t max, int *count_out)
{
  Datum *datums = palloc(sizeof(Datum) * max);
  int n = 0;
  for (int64_t i = 0; i < max; ++i)
  {
    if (cells[i] == (H3Index) 0)
      continue;
    datums[n++] = H3IndexGetDatum(cells[i]);
  }
  pfree(cells);
  *count_out = n;
  return datums;
}

/**
 * @brief Return an h3indexset from a freshly-allocated H3Index
 * buffer of size `max`. Zero entries are dropped. The input
 * buffer is pfree'd. On empty result, returns NULL after raising
 * a meos_error (set_make requires count >= 1).
 */
static Set *
h3index_set_from_buffer(H3Index *cells, int64_t max)
{
  int count;
  Datum *datums = h3index_array_compact(cells, max, &count);
  if (count == 0)
  {
    pfree(datums);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "libh3 returned no valid cells");
    return NULL;
  }
  return set_make_free(datums, count, T_H3INDEX, ORDER);
}

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

Set *
h3_grid_disk(H3Index origin, int k)
{
  if (k < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_disk: k must be >= 0");
    return NULL;
  }
  int64_t max;
  if (maxGridDiskSize(k, &max) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_disk: maxGridDiskSize failed");
    return NULL;
  }
  H3Index *cells = palloc0(max * sizeof(H3Index));
  if (gridDisk(origin, k, cells) != E_SUCCESS)
  {
    pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_disk: gridDisk failed");
    return NULL;
  }
  return h3index_set_from_buffer(cells, max);
}

Set *
h3_grid_ring(H3Index origin, int k)
{
  if (k < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_ring: k must be >= 0");
    return NULL;
  }
  int64_t max, inner = 0;
  if (maxGridDiskSize(k, &max) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_ring: maxGridDiskSize failed");
    return NULL;
  }
  if (k > 0 && maxGridDiskSize(k - 1, &inner) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_ring: maxGridDiskSize failed");
    return NULL;
  }
  int64_t ring = max - inner;
  H3Index *cells = palloc0(ring * sizeof(H3Index));
  if (gridRingUnsafe(origin, k, cells) != E_SUCCESS)
  {
    pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_ring: gridRingUnsafe failed (pentagonal distortion?)");
    return NULL;
  }
  return h3index_set_from_buffer(cells, ring);
}

Set *
h3_grid_path_cells(H3Index start, H3Index end)
{
  int64_t size;
  if (gridPathCellsSize(start, end, &size) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_path_cells: gridPathCellsSize failed "
      "(incomparable resolutions or pentagonal distortion?)");
    return NULL;
  }
  H3Index *cells = palloc0(size * sizeof(H3Index));
  if (gridPathCells(start, end, cells) != E_SUCCESS)
  {
    pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_grid_path_cells: gridPathCells failed");
    return NULL;
  }
  return h3index_set_from_buffer(cells, size);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

Set *
h3_cell_to_children(H3Index origin, int childRes)
{
  int64_t max;
  if (cellToChildrenSize(origin, childRes, &max) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_cell_to_children: cellToChildrenSize failed "
      "(child resolution coarser than cell resolution?)");
    return NULL;
  }
  H3Index *cells = palloc0(max * sizeof(H3Index));
  if (cellToChildren(origin, childRes, cells) != E_SUCCESS)
  {
    pfree(cells);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_cell_to_children: cellToChildren failed");
    return NULL;
  }
  return h3index_set_from_buffer(cells, max);
}

Set *
h3_compact_cells(const Set *cells)
{
  if (! cells)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_compact_cells: input must not be NULL");
    return NULL;
  }
  int n = cells->count;
  H3Index *in = palloc(n * sizeof(H3Index));
  for (int i = 0; i < n; ++i)
    in[i] = DatumGetH3Index(SET_VAL_N(cells, i));
  H3Index *out = palloc0(n * sizeof(H3Index));
  if (compactCells(in, out, n) != E_SUCCESS)
  {
    pfree(in); pfree(out);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_compact_cells: compactCells failed");
    return NULL;
  }
  pfree(in);
  return h3index_set_from_buffer(out, n);
}

Set *
h3_uncompact_cells(const Set *cells, int res)
{
  if (! cells)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_uncompact_cells: input must not be NULL");
    return NULL;
  }
  int n = cells->count;
  H3Index *in = palloc(n * sizeof(H3Index));
  for (int i = 0; i < n; ++i)
    in[i] = DatumGetH3Index(SET_VAL_N(cells, i));
  int64_t max;
  if (uncompactCellsSize(in, n, res, &max) != E_SUCCESS)
  {
    pfree(in);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_uncompact_cells: uncompactCellsSize failed "
      "(target resolution is coarser than an input cell?)");
    return NULL;
  }
  H3Index *out = palloc0(max * sizeof(H3Index));
  if (uncompactCells(in, n, out, max, res) != E_SUCCESS)
  {
    pfree(in); pfree(out);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_uncompact_cells: uncompactCells failed");
    return NULL;
  }
  pfree(in);
  return h3index_set_from_buffer(out, max);
}

/*****************************************************************************
 * Edges and vertexes
 *****************************************************************************/

Set *
h3_origin_to_directed_edges(H3Index origin)
{
  /* Always 6 slots; pentagons fill only 5 (one slot is 0). */
  H3Index *edges = palloc0(6 * sizeof(H3Index));
  if (originToDirectedEdges(origin, edges) != E_SUCCESS)
  {
    pfree(edges);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_origin_to_directed_edges: originToDirectedEdges failed");
    return NULL;
  }
  return h3index_set_from_buffer(edges, 6);
}

Set *
h3_cell_to_vertexes(H3Index cell)
{
  /* Always 6 slots; pentagons fill only 5 (one slot is 0). */
  H3Index *vertexes = palloc0(6 * sizeof(H3Index));
  if (cellToVertexes(cell, vertexes) != E_SUCCESS)
  {
    pfree(vertexes);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_cell_to_vertexes: cellToVertexes failed");
    return NULL;
  }
  return h3index_set_from_buffer(vertexes, 6);
}

/*****************************************************************************
 * Icosahedron faces
 *****************************************************************************/

Set *
h3_get_icosahedron_faces(H3Index cell)
{
  int maxFaces;
  if (maxFaceCount(cell, &maxFaces) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_get_icosahedron_faces: maxFaceCount failed");
    return NULL;
  }
  int *faces = palloc(maxFaces * sizeof(int));
  /* libh3 writes -1 for unused slots. */
  for (int i = 0; i < maxFaces; ++i)
    faces[i] = -1;
  if (getIcosahedronFaces(cell, faces) != E_SUCCESS)
  {
    pfree(faces);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_get_icosahedron_faces: getIcosahedronFaces failed");
    return NULL;
  }
  Datum *datums = palloc(sizeof(Datum) * maxFaces);
  int count = 0;
  for (int i = 0; i < maxFaces; ++i)
    if (faces[i] >= 0)
      datums[count++] = Int32GetDatum(faces[i]);
  pfree(faces);
  if (count == 0)
  {
    pfree(datums);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3_get_icosahedron_faces: libh3 returned no faces");
    return NULL;
  }
  return set_make_free(datums, count, T_INT4, ORDER);
}

/*****************************************************************************/
