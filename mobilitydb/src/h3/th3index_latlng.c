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
 * @brief PG V1 wrappers for th3index lat/lng conversions.
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
 * h3_latlng_to_cell
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_latlng_to_cell_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_latlng_to_cell_tgeompoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the temporal H3 cell index of a temporal planar point at
 * the given resolution (SRID must be 4326)
 * @sqlfn h3_latlng_to_cell()
 */
Datum
Th3index_latlng_to_cell_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tgeompoint_to_th3index(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_latlng_to_cell_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_latlng_to_cell_tgeogpoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the temporal H3 cell index of a temporal geodetic point at
 * the given resolution
 * @sqlfn h3_latlng_to_cell()
 */
Datum
Th3index_latlng_to_cell_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tgeogpoint_to_th3index(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_latlng
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_tgeogpoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell
 * @sqlfn h3_cell_to_latlng()
 */
Datum
Th3index_cell_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_to_tgeogpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_cell_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_tgeompoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal
 * H3 cell
 * @sqlfn h3_cell_to_latlng_tgeompoint()
 */
Datum
Th3index_cell_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_to_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_boundary
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_boundary);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the per-instant polygon boundary of a temporal H3 cell as
 * a temporal geography
 * @sqlfn h3_cell_to_boundary()
 */
Datum
Th3index_cell_to_boundary(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_cell_to_boundary(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
