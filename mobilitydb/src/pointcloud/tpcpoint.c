/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief PG wrappers for the tpcpoint lifted temporal type
 * @details Most SQL bindings delegate to the generic @c Temporal_* PG wrappers
 * in
 * @c mobilitydb/src/temporal/temporal.c — the generic parser / output /
 * constructor / accessor path dispatches through @c MeosType and works
 * correctly for @c T_TPCPOINT because the base type is wired into
 * @c basetype_in_state / @c basetype_out / @c datum_cmp / @c datum_eq /
 * @c datum_hash. This file holds only the per-type accessors that need
 * to know they are looking at pcpoint data — namely @c pcid and the
 * schema-aware dimension projections.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <varatt.h>
/* pgPointCloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <pgtypes.h>            /* text_to_cstring — NOT utils/builtins.h */
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>  /* tgeo_restrict_stbox */
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/tpcbox.h"     /* PG_GETARG_TPCBOX_P */

/*****************************************************************************
 * Per-type pcid accessor
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpoint_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpoint
 * @sqlfn pcid()
 */
Datum
Tpcpoint_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32_t result = tpcpoint_pcid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) result);
}

/*****************************************************************************
 * Per-dimension projection — tpcpoint → tfloat
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpoint_get_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_get_x);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the X coordinate of a tpcpoint as a tfloat
 * @sqlfn getX()
 */
Datum
Tpcpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpoint_get_x(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpcpoint_get_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_get_y);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the Y coordinate of a tpcpoint as a tfloat
 * @sqlfn getY()
 */
Datum
Tpcpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpoint_get_y(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpcpoint_get_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_get_z);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the Z coordinate of a tpcpoint as a tfloat, NULL where its
 * schema has no Z dimension
 * @sqlfn getZ()
 */
Datum
Tpcpoint_get_z(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpoint_get_z(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Projection to tgeompoint
 *
 * The per-instant schema-aware projection (pcpoint -> POINT gserialized)
 * lives in the MEOS function @c tpcpoint_to_tgeompoint, shared by this
 * wrapper, the bbox-based restrictions below, and the spatial
 * relationships / nearest-approach distance.
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_pointcloud_conversion
 * @brief Project a tpcpoint onto a tgeompoint by extracting X/Y/[Z] from each
 * instant's pcpoint via the schema cache
 * @sqlfn tgeompoint()
 */
Datum
Tpcpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpoint_to_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpoint_to_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_to_tgeogpoint);
/**
 * @ingroup mobilitydb_pointcloud_conversion
 * @brief Convert a temporal point cloud point into a temporal geography point
 * @sqlfn tgeogpoint()
 * @sqlop @p ::
 */
Datum
Tpcpoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpoint_to_tgeogpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpcpoint_at_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_at_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpoint to the spatial / temporal extent of a tpcbox
 * @sqlfn atTpcbox()
 */
Datum
Tpcpoint_at_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpoint_at_tpcbox(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpoint_minus_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_minus_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Remove from a tpcpoint the part covered by a tpcbox
 * @sqlfn minusTpcbox()
 */
Datum
Tpcpoint_minus_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpoint_minus_tpcbox(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpoint_get_dim(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_get_dim);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the per-instant projection of a tpcpoint onto a named dimension
 * (Intensity, GpsTime, etc.) as a tfloat
 * @sqlfn getDim()
 */
Datum
Tpcpoint_get_dim(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *name_txt = PG_GETARG_TEXT_P(1);
  char *name = text_to_cstring(name_txt);
  Temporal *result = tpcpoint_get_dim(temp, name);
  pfree(name);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(name_txt, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/*****************************************************************************
 * Transformation into the schema of another pcid
 *****************************************************************************/

PGDLLEXPORT Datum Tpointcloud_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointcloud_transform);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return a temporal pcpoint or pcpatch transformed into the schema of
 * another pcid, which states the target SRID
 * @sqlfn transform()
 * @altsqlfn pcpointTransform(), pcpatchTransform()
 */
Datum
Tpointcloud_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32_t pcid = (uint32_t) PG_GETARG_INT32(1);
  Temporal *result = tpointcloud_transform(temp, pcid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}
