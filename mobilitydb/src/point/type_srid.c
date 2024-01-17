/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Functions for spatial reference systems for geo sets, spatiotemporal
 * boxes and temporal points
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/stbox.h"
#include "point/tpoint_restrfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Geoset_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_get_srid);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the SRID of a geo set
 * @sqlfn SRID()
 */
Datum
Geoset_get_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int result = geoset_srid(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Geoset_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_set_srid);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set with the coordinates set to an SRID
 * @sqlfn setSRID()
 */
Datum
Geoset_set_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = geoset_set_srid(s, srid);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_srid);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the SRID of a spatiotemporal box
 * @sqlfn SRID()
 */
Datum
Stbox_get_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(stbox_srid(box));
}

PGDLLEXPORT Datum Stbox_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_set_srid);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box with the coordinates set to an SRID
 * @sqlfn setSRID()
 */
Datum
Stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  PG_RETURN_STBOX_P(stbox_set_srid(box, srid));
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point
 * @sqlfn SRID()
 */
Datum
Tpoint_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tpoint_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tpoint_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_transf 
 * @brief Return a temporal point with the SRID set to a value
 * @sqlfn setSRID()
 */
Datum
Tpoint_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tpoint_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Functions for transforming between spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Geoset_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_transform);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set with the coordinates transformed to an SRID
 * @sqlfn transform()
 */
Datum
Geoset_transform(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = geoset_transform(s, srid);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Geoset_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_transform_pipeline);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set transformed to another spatial reference
 * system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Geoset_transform_pipeline(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Set *result = geoset_transform_pipeline(s, pipelinestr, srid, is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_transform);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box with the coordinates transformed to an 
 * SRID
 * @sqlfn transform()
 */
Datum
Stbox_transform(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBox *result = stbox_transform(box, srid);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_transform_pipeline);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box transformed to another spatial reference
 * system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Stbox_transform_pipeline(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  STBox *result = stbox_transform_pipeline(box, pipelinestr, srid, is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_transform);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return a temporal point transformed to another spatial reference
 * system
 * @sqlfn transform()
 */
Datum
Tpoint_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int srid = PG_GETARG_INT32(1);
  Temporal *result = tpoint_transform(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_transform_pipeline);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return a temporal point transformed to another spatial reference
 * system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Tpoint_transform_pipeline(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Temporal *result = tpoint_transform_pipeline(temp, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
