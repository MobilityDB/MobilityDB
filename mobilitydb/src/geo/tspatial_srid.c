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
#include "geo/tgeo_spatialfuncs.h"
#include "geo/stbox.h"
#include "geo/tpoint_restrfuncs.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/tcbuffer.h"
#endif /* CBUFFER */
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_srid);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the SRID of a geo set
 * @sqlfn SRID()
 */
Datum
Spatialset_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int result = spatialset_srid(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Spatialset_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_set_srid);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a spatial set with the coordinates set to an SRID
 * @sqlfn setSRID()
 */
Datum
Spatialset_set_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = spatialset_set_srid(s, srid);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_srid);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the SRID of a spatiotemporal box
 * @sqlfn SRID()
 */
Datum
Stbox_srid(PG_FUNCTION_ARGS)
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

PGDLLEXPORT Datum Tspatial_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal spatial type
 * @sqlfn SRID()
 */
Datum
Tspatial_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tspatial_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tspatial_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_transf 
 * @brief Return a temporal point with the SRID set to a value
 * @sqlfn setSRID()
 */
Datum
Tspatial_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tspatial_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Functions for transforming between spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_transform);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set with the coordinates transformed to an SRID
 * @sqlfn transform()
 */
Datum
Spatialset_transform(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = spatialset_transform(s, srid);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Spatialset_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_transform_pipeline);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set transformed to another spatial reference
 * system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Spatialset_transform_pipeline(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Set *result = spatialset_transform_pipeline(s, pipelinestr, srid, is_forward);
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
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  STBox *result = stbox_transform_pipeline(box, pipelinestr, srid, is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

#if CBUFFER
PGDLLEXPORT Datum Cbuffer_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_transform);
/**
 * @ingroup mobilitydb_cbuffer_spatial_transf
 * @brief Return a circular buffer transformed to another spatial reference
 * system
 * @sqlfn transform()
 */
Datum
Cbuffer_transform(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Cbuffer *result = cbuffer_transform(cbuf, srid);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_RETURN_CBUFFER_P(result);
}

PGDLLEXPORT Datum Cbuffer_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_transform_pipeline);
/**
 * @ingroup mobilitydb_cbuffer_spatial_transf
 * @brief Return a circular buffer transformed to another spatial reference
 * system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Cbuffer_transform_pipeline(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Cbuffer *result = cbuffer_transform_pipeline(cbuf, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_CBUFFER_P(result);
}
#endif /* CBUFFER */

/*****************************************************************************/

PGDLLEXPORT Datum Tspatial_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_transform);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return a temporal circular buffer transformed to another spatial
 * reference system
 * @sqlfn transform()
 */
Datum
Tspatial_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Temporal *result = tspatial_transform(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tspatial_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_transform_pipeline);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return a temporal circular buffer transformed to another spatial
 * reference system using a transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Tspatial_transform_pipeline(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Temporal *result = tspatial_transform_pipeline(temp, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
