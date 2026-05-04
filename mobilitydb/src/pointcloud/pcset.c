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
 * (dispatch happens via the Oid-MeosType cache). This file hosts the
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
 * Schema-aware dimension accessors are now in MEOS
 * (`meos/src/pointcloud/pcpoint.c::pcpoint_get_x/y/z/dim`).  The PG
 * wrappers below just unpack PG arguments, resolve the schema via the
 * MEOS cache, dispatch, and convert the boolean result to NULL/float8.
 *****************************************************************************/

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
  double x;
  bool ok = pcpoint_get_x(pt, meos_pc_schema(pt->pcid), &x);
  PG_FREE_IF_COPY(pt, 0);
  if (! ok)
    PG_RETURN_NULL();
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
  double y;
  bool ok = pcpoint_get_y(pt, meos_pc_schema(pt->pcid), &y);
  PG_FREE_IF_COPY(pt, 0);
  if (! ok)
    PG_RETURN_NULL();
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
  double z;
  bool ok = pcpoint_get_z(pt, meos_pc_schema(pt->pcid), &z);
  PG_FREE_IF_COPY(pt, 0);
  if (! ok)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(z);
}

PGDLLEXPORT Datum Pcpoint_get_dim(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_get_dim);
/**
 * @ingroup mobilitydb_pointcloud_base_accessor
 * @brief Return an arbitrary dimension of a pcpoint by name.
 * @sqlfn getDim()
 */
Datum
Pcpoint_get_dim(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  text *name_txt = PG_GETARG_TEXT_P(1);
  char *name = text_to_cstring(name_txt);
  double v;
  bool ok = pcpoint_get_dim(pt, meos_pc_schema(pt->pcid), name, &v);
  pfree(name);
  PG_FREE_IF_COPY(pt, 0);
  PG_FREE_IF_COPY(name_txt, 1);
  if (! ok)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(v);
}

/*****************************************************************************/
