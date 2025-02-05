/*****************************************************************************
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
 * @brief Bounding box operators for temporal network points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * STBox boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "npoint/tnpoint_boxops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "geo/stbox.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_geo/tspatial_boxops.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network point converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Npoint_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_STBOX_P(npoint_to_stbox(np));
}

PGDLLEXPORT Datum Nsegment_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network segment converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Nsegment_to_stbox(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_STBOX_P(nsegment_to_stbox(ns));
}

/*****************************************************************************/

PGDLLEXPORT Datum Npointset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npointset_to_stbox);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a network point set converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Npointset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = spatialset_to_stbox(set);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Npoint_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network point and a timestamptz to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Npoint_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(npoint_timestamptz_to_stbox(np, t));
}

PGDLLEXPORT Datum Npoint_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network point and a timestamptz span to a spatiotemporal
 * box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Npoint_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(npoint_tstzspan_to_stbox(np, s));
}

/*****************************************************************************/
