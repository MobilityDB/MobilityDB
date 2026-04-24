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
 * @brief PG wrappers for per-type pcpoint / pcpatch accessors.
 *
 * Set-level SQL bindings for @p pcpointset / @p pcpatchset delegate to
 * the generic @p Set_* wrappers in @p mobilitydb/src/temporal/set.c
 * (dispatch happens via the Oid-meosType cache). This file hosts the
 * type-specific accessors: the trivial @p pcid reads and the
 * schema-aware dimension getters (@c getX, @c getY, @c getZ,
 * @c getDim) that build on top of the PCSCHEMA cache.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/builtins.h>  /* text_to_cstring */
/* pgpointcloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

/*****************************************************************************
 * pcid accessors
 *****************************************************************************/

PGDLLEXPORT Datum Pcpoint_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_pcid);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return the schema id (pcid) of a pcpoint
 * @sqlfn pcid()
 */
Datum
Pcpoint_pcid(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  uint32_t pcid = pcpoint_get_pcid(pt);
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_INT32((int32) pcid);
}

PGDLLEXPORT Datum Pcpatch_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_pcid);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return the schema id (pcid) of a pcpatch
 * @sqlfn pcid()
 */
Datum
Pcpatch_pcid(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  uint32_t pcid = pcpatch_get_pcid(pa);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_INT32((int32) pcid);
}

/*****************************************************************************
 * Schema-aware pcpoint dimension getters
 *
 * Wrap a varlena-shape Pcpoint as a transient in-memory PCPOINT (libpc.a's
 * uncompressed struct) so we can reuse pgpointcloud's own dimension
 * readers — pc_point_get_x/y/z, pc_point_get_double_by_name. That keeps
 * scale/offset/interpretation handling in one place (pgpointcloud).
 *****************************************************************************/

/**
 * @brief Shim a varlena Pcpoint into a read-only libpc.a PCPOINT.
 * @note Shares the underlying dimension-byte pointer; the caller's
 *   Pcpoint is NOT copied. The PCSCHEMA is loaded from the per-backend
 *   cache on first touch per pcid.
 */
static inline void
pcpoint_as_pcpt(const Pcpoint *pt, PCPOINT *out)
{
  out->readonly = 1;
  out->schema = mobilitydb_pc_schema(pt->pcid);
  /* Cast away const — libpc.a uses a non-const pointer but never writes
   * to the buffer when readonly=1. */
  out->data = (uint8_t *) ((const Pcpoint *) pt)->data;
}

PGDLLEXPORT Datum Pcpoint_get_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_get_x);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return the X dimension of a pcpoint (schema-aware).
 * @sqlfn getX()
 */
Datum
Pcpoint_get_x(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, &pcpt);
  if (! pcpt.schema->xdim)
  {
    PG_FREE_IF_COPY(pt, 0);
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("pcpoint schema %u has no X dimension", pt->pcid)));
  }
  double x;
  if (! pc_point_get_x(&pcpt, &x))
  {
    PG_FREE_IF_COPY(pt, 0);
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Failed to read X from pcpoint (pcid=%u)", pt->pcid)));
  }
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_FLOAT8(x);
}

PGDLLEXPORT Datum Pcpoint_get_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_get_y);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return the Y dimension of a pcpoint (schema-aware).
 * @sqlfn getY()
 */
Datum
Pcpoint_get_y(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, &pcpt);
  if (! pcpt.schema->ydim)
  {
    PG_FREE_IF_COPY(pt, 0);
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("pcpoint schema %u has no Y dimension", pt->pcid)));
  }
  double y;
  if (! pc_point_get_y(&pcpt, &y))
  {
    PG_FREE_IF_COPY(pt, 0);
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Failed to read Y from pcpoint (pcid=%u)", pt->pcid)));
  }
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_FLOAT8(y);
}

PGDLLEXPORT Datum Pcpoint_get_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_get_z);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return the Z dimension of a pcpoint (NULL if schema has no Z).
 * @sqlfn getZ()
 */
Datum
Pcpoint_get_z(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, &pcpt);
  if (! pcpt.schema->zdim)
  {
    PG_FREE_IF_COPY(pt, 0);
    PG_RETURN_NULL();  /* idiomatic: no Z → NULL, not an error */
  }
  double z;
  if (! pc_point_get_z(&pcpt, &z))
  {
    PG_FREE_IF_COPY(pt, 0);
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Failed to read Z from pcpoint (pcid=%u)", pt->pcid)));
  }
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_FLOAT8(z);
}

PGDLLEXPORT Datum Pcpoint_get_dim(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_get_dim);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return an arbitrary dimension of a pcpoint by name.
 * @details Accepts any name defined in the schema XML (Intensity,
 *   ReturnNumber, Classification, GpsTime, …); NULL if the name is
 *   unknown for this schema. Uses pgpointcloud's own lookup so
 *   scale/offset/interpretation are applied identically to
 *   @c PC_Get(pcpoint, name).
 * @sqlfn getDim()
 */
Datum
Pcpoint_get_dim(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  text *name_txt = PG_GETARG_TEXT_P(1);
  char *name = text_to_cstring(name_txt);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, &pcpt);
  double v;
  int ok = pc_point_get_double_by_name(&pcpt, name, &v);
  pfree(name);
  PG_FREE_IF_COPY(pt, 0);
  PG_FREE_IF_COPY(name_txt, 1);
  if (! ok)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(v);
}

/*****************************************************************************/
