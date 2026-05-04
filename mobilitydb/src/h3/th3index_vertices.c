/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
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
