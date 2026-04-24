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
 * @brief PG wrappers for the tpcpatch lifted temporal type (Phase 8I).
 *
 * Structural mirror of mobilitydb/src/pointcloud/tpcpoint.c. The generic
 * Temporal_* dispatch tables already handle @c T_TPCPATCH via Phase 8D's
 * base-type wiring; this file holds only the per-type pcid accessor
 * (and will grow a per-instant points-count accessor later).
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/pcpatch.h"

/*****************************************************************************
 * Per-type accessors
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpatch.
 * @details All instants share the same pcid — enforced at construction
 *   time by the Phase 8E set_make_exp machinery. This is just a first-
 *   instant read.
 * @sqlfn pcid()
 */
Datum
Tpcpatch_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(first);
  uint32_t pcid = pcpatch_pcid(pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) pcid);
}

PGDLLEXPORT Datum Tpcpatch_start_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_start_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the number of points in the first instant's pcpatch.
 * @sqlfn startNumPoints()
 */
Datum
Tpcpatch_start_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(first);
  uint32_t n = pcpatch_npoints(pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) n);
}

/*****************************************************************************/
