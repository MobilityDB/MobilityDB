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
/* pgpointcloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <pgtypes.h>  /* text_to_cstring — NOT utils/builtins.h, whose
                       * fmgrprotos.h json_object collides with json-c */
#include <meos_pointcloud.h>
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"
#include "pointcloud/meos_schema_hook.h"
/* MobilityDB */
#include "pg_geo/postgis.h"  /* PG_RETURN_GSERIALIZED_P */
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

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Pcpoint_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_srid);
/**
 * @ingroup mobilitydb_pointcloud_base_srid
 * @brief Return the SRID of a pcpoint
 * @sqlfn SRID()
 */
Datum
Pcpoint_srid(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  int32_t srid = meos_pc_schema_get_srid(pcpoint_get_pcid(pt));
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_INT32(srid);
}

PGDLLEXPORT Datum Pcpatch_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_srid);
/**
 * @ingroup mobilitydb_pointcloud_base_srid
 * @brief Return the SRID of a pcpatch
 * @sqlfn SRID()
 */
Datum
Pcpatch_srid(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  int32_t srid = meos_pc_schema_get_srid(pcpatch_get_pcid(pa));
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_INT32(srid);
}

/*****************************************************************************
 * Comparison functions for pcpoint
 *****************************************************************************/

PGDLLEXPORT Datum Pcpoint_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_eq);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is equal to the second one
 * @sqlfn eq()
 * @sqlop @p =
 */
Datum
Pcpoint_eq(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_eq(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpoint_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_ne);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is different from the second one
 * @sqlfn ne()
 * @sqlop @p <>
 */
Datum
Pcpoint_ne(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_ne(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpoint_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_cmp);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pcpoint is less
 * than, equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn cmp()
 */
Datum
Pcpoint_cmp(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  int result = pcpoint_cmp(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Pcpoint_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_lt);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is less than the second one
 * @sqlfn lt()
 * @sqlop @p <
 */
Datum
Pcpoint_lt(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_lt(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpoint_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_le);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is less than or equal to the
 * second one
 * @sqlfn le()
 * @sqlop @p <=
 */
Datum
Pcpoint_le(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_le(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpoint_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_ge);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is greater than or equal to the
 * second one
 * @sqlfn ge()
 * @sqlop @p >=
 */
Datum
Pcpoint_ge(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_ge(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpoint_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_gt);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpoint is greater than the second one
 * @sqlfn gt()
 * @sqlop @p >
 */
Datum
Pcpoint_gt(PG_FUNCTION_ARGS)
{
  Pcpoint *pt1 = PG_GETARG_PCPOINT_P(0);
  Pcpoint *pt2 = PG_GETARG_PCPOINT_P(1);
  bool result = pcpoint_gt(pt1, pt2);
  PG_FREE_IF_COPY(pt1, 0);
  PG_FREE_IF_COPY(pt2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Functions for defining hash indexes on pcpoint
 *****************************************************************************/

PGDLLEXPORT Datum Pcpoint_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_hash);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return the 32-bit hash value of a pcpoint
 * @sqlfn hash()
 */
Datum
Pcpoint_hash(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  uint32 result = pcpoint_hash(pt);
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Pcpoint_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_hash_extended);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return the 64-bit hash value of a pcpoint using a seed
 * @sqlfn hashExtended()
 */
Datum
Pcpoint_hash_extended(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = pcpoint_hash_extended(pt, seed);
  PG_FREE_IF_COPY(pt, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************
 * Comparison functions for pcpatch
 *****************************************************************************/

/*****************************************************************************
 * Conversion of a patch into its geometry
 *****************************************************************************/

PGDLLEXPORT Datum Pcpatch_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_to_geom);
/**
 * @ingroup mobilitydb_pointcloud_base_conversion
 * @brief Convert a pcpatch into the multipoint of the positions its points
 * occupy
 * @sqlfn geometry()
 */
Datum
Pcpatch_to_geom(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  GSERIALIZED *result = pcpatch_to_geom(pa);
  PG_FREE_IF_COPY(pa, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Pcpatch_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_eq);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is equal to the second one
 * @sqlfn eq()
 * @sqlop @p =
 */
Datum
Pcpatch_eq(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_eq(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpatch_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_ne);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is different from the second one
 * @sqlfn ne()
 * @sqlop @p <>
 */
Datum
Pcpatch_ne(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_ne(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpatch_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_cmp);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pcpatch is less
 * than, equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn cmp()
 */
Datum
Pcpatch_cmp(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  int result = pcpatch_cmp(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Pcpatch_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_lt);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is less than the second one
 * @sqlfn lt()
 * @sqlop @p <
 */
Datum
Pcpatch_lt(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_lt(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpatch_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_le);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is less than or equal to the
 * second one
 * @sqlfn le()
 * @sqlop @p <=
 */
Datum
Pcpatch_le(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_le(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpatch_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_ge);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is greater than or equal to the
 * second one
 * @sqlfn ge()
 * @sqlop @p >=
 */
Datum
Pcpatch_ge(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_ge(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Pcpatch_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_gt);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return true if the first pcpatch is greater than the second one
 * @sqlfn gt()
 * @sqlop @p >
 */
Datum
Pcpatch_gt(PG_FUNCTION_ARGS)
{
  Pcpatch *pa1 = PG_GETARG_PCPATCH_P(0);
  Pcpatch *pa2 = PG_GETARG_PCPATCH_P(1);
  bool result = pcpatch_gt(pa1, pa2);
  PG_FREE_IF_COPY(pa1, 0);
  PG_FREE_IF_COPY(pa2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Functions for defining hash indexes on pcpatch
 *****************************************************************************/

PGDLLEXPORT Datum Pcpatch_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_hash);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return the 32-bit hash value of a pcpatch
 * @sqlfn hash()
 */
Datum
Pcpatch_hash(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  uint32 result = pcpatch_hash(pa);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Pcpatch_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_hash_extended);
/**
 * @ingroup mobilitydb_pointcloud_base_comp
 * @brief Return the 64-bit hash value of a pcpatch using a seed
 * @sqlfn hashExtended()
 */
Datum
Pcpatch_hash_extended(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = pcpatch_hash_extended(pa, seed);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
