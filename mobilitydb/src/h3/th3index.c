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
 * @brief PG V1 wrappers for the th3index conversion casts.
 *
 * The int64 payload is identical between tbigint and th3index, but
 * the embedded bounding box differs (TBox vs STBox). The MEOS-level
 * tbigint_to_th3index / th3index_to_tbigint helpers lift an identity
 * Datum function so the result is rebuilt at the correct shape.
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

PGDLLEXPORT Datum Tbigint_to_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbigint_to_th3index);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Convert a temporal big integer into a temporal H3 cell index
 * @sqlfn th3index()
 * @sqlop @p ::
 */
Datum
Tbigint_to_th3index(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tbigint_to_th3index(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_to_tbigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_to_tbigint);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Convert a temporal H3 cell index into a temporal big integer
 * @sqlfn tbigint()
 * @sqlop @p ::
 */
Datum
Th3index_to_tbigint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_to_tbigint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}
