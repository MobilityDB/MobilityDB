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
 * @brief PG V1 wrappers for th3index vertex functions.
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
 * h3_cell_to_vertex(th3index, integer)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_vertex(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_vertex);
/**
 * @ingroup mobilitydb_h3_vertex
 * @brief Return the temporal H3-vertex index at the given vertex number of
 * a temporal H3 cell
 * @sqlfn h3_cell_to_vertex()
 */
Datum
Th3index_cell_to_vertex(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 vertex_num = PG_GETARG_INT32(1);
  Temporal *result = th3index_cell_to_vertex(temp, vertex_num);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_vertex_to_latlng
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_vertex_to_latlng(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_vertex_to_latlng);
/**
 * @ingroup mobilitydb_h3_vertex
 * @brief Return the geodetic coordinates of a temporal H3 vertex
 * @sqlfn h3_vertex_to_latlng()
 */
Datum
Th3index_vertex_to_latlng(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_vertex_to_latlng(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_is_valid_vertex
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_is_valid_vertex(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_is_valid_vertex);
/**
 * @ingroup mobilitydb_h3_vertex
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 vertex
 * @sqlfn h3_is_valid_vertex()
 */
Datum
Th3index_is_valid_vertex(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_is_valid_vertex(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
