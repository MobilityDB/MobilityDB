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
 * @brief PG V1 wrappers for th3index metric functions.
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
 * h3_cell_area(th3index, text)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_area);
/**
 * @ingroup mobilitydb_h3_metrics
 * @brief Return the per-instant area of a temporal H3 cell in the given unit
 * @sqlfn h3_cell_area()
 */
Datum
Th3index_cell_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *unit_txt = PG_GETARG_TEXT_P(1);
  char *unit = text2cstring(unit_txt);
  Temporal *result = th3index_cell_area(temp, unit);
  pfree(unit);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_edge_length(th3index, text)
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_edge_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_edge_length);
/**
 * @ingroup mobilitydb_h3_metrics
 * @brief Return the per-instant length of a temporal H3 directed edge in
 * the given unit
 * @sqlfn h3_edge_length()
 */
Datum
Th3index_edge_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *unit_txt = PG_GETARG_TEXT_P(1);
  char *unit = text2cstring(unit_txt);
  Temporal *result = th3index_edge_length(temp, unit);
  pfree(unit);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_great_circle_distance(tgeogpoint, tgeogpoint, text)
 *****************************************************************************/

PGDLLEXPORT Datum Tgeogpoint_great_circle_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_great_circle_distance);
/**
 * @ingroup mobilitydb_h3_metrics
 * @brief Return the per-instant great-circle distance between two temporal
 * geodetic points in the given unit
 * @sqlfn h3_great_circle_distance()
 */
Datum
Tgeogpoint_great_circle_distance(PG_FUNCTION_ARGS)
{
  Temporal *a = PG_GETARG_TEMPORAL_P(0);
  Temporal *b = PG_GETARG_TEMPORAL_P(1);
  text *unit_txt = PG_GETARG_TEXT_P(2);
  char *unit = text2cstring(unit_txt);
  Temporal *result = tgeogpoint_great_circle_distance(a, b, unit);
  pfree(unit);
  PG_FREE_IF_COPY(a, 0);
  PG_FREE_IF_COPY(b, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
