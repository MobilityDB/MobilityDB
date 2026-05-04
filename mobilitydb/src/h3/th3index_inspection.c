/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG V1 wrappers for th3index inspection functions.
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
 * h3_get_resolution
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_get_resolution);
/**
 * @ingroup mobilitydb_h3_inspection
 * @brief Return a temporal integer of the resolution of a temporal H3 cell
 * @sqlfn h3_get_resolution()
 */
Datum
Th3index_get_resolution(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_get_resolution(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_get_base_cell_number
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_get_base_cell_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_get_base_cell_number);
/**
 * @ingroup mobilitydb_h3_inspection
 * @brief Return a temporal integer of the base-cell number of a temporal H3
 * cell
 * @sqlfn h3_get_base_cell_number()
 */
Datum
Th3index_get_base_cell_number(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_get_base_cell_number(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_is_valid_cell
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_is_valid_cell);
/**
 * @ingroup mobilitydb_h3_inspection
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 cell
 * @sqlfn h3_is_valid_cell()
 */
Datum
Th3index_is_valid_cell(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_is_valid_cell(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_is_res_class_iii
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_is_res_class_iii(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_is_res_class_iii);
/**
 * @ingroup mobilitydb_h3_inspection
 * @brief Return a temporal boolean stating at each instant whether the cell
 * has Class-III orientation
 * @sqlfn h3_is_res_class_iii()
 */
Datum
Th3index_is_res_class_iii(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_is_res_class_iii(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_is_pentagon
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_is_pentagon(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_is_pentagon);
/**
 * @ingroup mobilitydb_h3_inspection
 * @brief Return a temporal boolean stating at each instant whether the cell
 * is a pentagon
 * @sqlfn h3_is_pentagon()
 */
Datum
Th3index_is_pentagon(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_is_pentagon(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
