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
 * @brief PG wrappers for pcpoint / pcpatch — Phase 8E scope.
 *
 * Most set-level SQL bindings for @p pcpointset / @p pcpatchset delegate
 * to the generic @p Set_* wrappers in @p mobilitydb/src/temporal/set.c
 * (they dispatch on settype → basetype through the Oid-meosType cache).
 * This file holds only the type-specific accessors that depend on the
 * argument type being @p pcpoint or @p pcpatch.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"

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
  uint32_t pcid = pcpoint_pcid(pt);
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
  uint32_t pcid = pcpatch_pcid(pa);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_INT32((int32) pcid);
}

/*****************************************************************************/
