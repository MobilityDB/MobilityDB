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
 * @brief PG wrappers for the tpcpoint lifted temporal type (Phase 8H).
 *
 * Most SQL bindings delegate to the generic @c Temporal_* PG wrappers in
 * @c mobilitydb/src/temporal/temporal.c — the generic parser / output /
 * constructor / accessor path dispatches through @c meosType and works
 * correctly for @c T_TPCPOINT because Phase 8D wired the base type into
 * @c basetype_in_state / @c basetype_out / @c datum_cmp / @c datum_eq /
 * @c datum_hash. This file holds only the per-type accessors that need
 * to know they are looking at pcpoint data — namely @c pcid and the
 * schema-aware dimension projections.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/pcpoint.h"

/*****************************************************************************
 * Per-type pcid accessor
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpoint_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpoint.
 * @details All instants of a tpcpoint must share the same pcid — the
 *   Phase 8E set_make_exp enforcement guarantees this at construction
 *   time. This function just reads the first instant's pcid.
 * @sqlfn pcid()
 */
Datum
Tpcpoint_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpoint *pt = (const Pcpoint *) DatumGetPointer(first);
  uint32_t pcid = pcpoint_pcid(pt);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) pcid);
}

/*****************************************************************************/
