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
 * @brief PG V1 wrappers for th3index grid-traversal functions.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * h3_grid_distance — also the procedure behind the `<->` operator.
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_grid_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_grid_distance);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal grid-hop distance between two temporal H3 cells
 * @sqlfn h3_grid_distance()
 * @sqlop @p \<->
 */
Datum
Th3index_grid_distance(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *dest = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_grid_distance(origin, dest);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(dest, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_local_ij
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_local_ij(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_local_ij);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal local (I, J) coordinate pair of a cell with
 * respect to an origin cell, as a temporal planar point
 * @sqlfn h3_cell_to_local_ij()
 */
Datum
Th3index_cell_to_local_ij(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *cell = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_cell_to_local_ij(origin, cell);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(cell, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_local_ij_to_cell
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_local_ij_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_local_ij_to_cell);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal H3 cell at the given local (I, J) coordinate
 * relative to an origin cell
 * @sqlfn h3_local_ij_to_cell()
 */
Datum
Th3index_local_ij_to_cell(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *coord = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_local_ij_to_cell(origin, coord);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(coord, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
