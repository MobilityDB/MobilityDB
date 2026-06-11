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
 * @brief PG V1 wrappers for th3index hierarchy functions.
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
 * h3_cell_to_parent(th3index, integer)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_parent);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal parent cell at the given resolution
 * @sqlfn h3_cell_to_parent()
 */
Datum
Th3index_cell_to_parent(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = th3index_cell_to_parent(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_cell_to_parent_next(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_parent_next);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal parent cell at the next-coarser resolution
 * @sqlfn h3_cell_to_parent()
 */
Datum
Th3index_cell_to_parent_next(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_cell_to_parent_next(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_center_child
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_center_child(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_center_child);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal center-child cell at the given resolution
 * @sqlfn h3_cell_to_center_child()
 */
Datum
Th3index_cell_to_center_child(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = th3index_cell_to_center_child(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_cell_to_center_child_next(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_center_child_next);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal center-child cell at the next-finer resolution
 * @sqlfn h3_cell_to_center_child()
 */
Datum
Th3index_cell_to_center_child_next(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_cell_to_center_child_next(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_child_pos(th3index, integer)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_child_pos(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_child_pos);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal position of the cell among the siblings of its
 * parent at the given resolution
 * @sqlfn h3_cell_to_child_pos()
 */
Datum
Th3index_cell_to_child_pos(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 parent_res = PG_GETARG_INT32(1);
  Temporal *result = th3index_cell_to_child_pos(temp, parent_res);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_child_pos_to_cell(tbigint, th3index, integer)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_child_pos_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_child_pos_to_cell);
/**
 * @ingroup mobilitydb_h3_hierarchy
 * @brief Return the temporal child cell of the given parent at a given
 * ordinal position among siblings, at a given child resolution
 * @sqlfn h3_child_pos_to_cell()
 */
Datum
Th3index_child_pos_to_cell(PG_FUNCTION_ARGS)
{
  Temporal *child_pos = PG_GETARG_TEMPORAL_P(0);
  Temporal *parent = PG_GETARG_TEMPORAL_P(1);
  int32 child_res = PG_GETARG_INT32(2);
  Temporal *result = th3index_child_pos_to_cell(child_pos, parent, child_res);
  PG_FREE_IF_COPY(child_pos, 0);
  PG_FREE_IF_COPY(parent, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
