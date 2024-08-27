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
 * @brief Bounding box operators for temporal poses.
 *
 * These operators test the bounding boxes of temporal poses, which are
 * STBox boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "pose/tpose_boxops.h"


/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "pose/tpose.h"
#include "pose/tpose_boxops.h"

/*****************************************************************************
 * Transform a temporal pose to a STBox
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return the bounding box of the network point value
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Pose_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  STBox *result = palloc0(sizeof(STBox));
  pose_set_stbox(pose, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Pose_timestamp_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_timestamp_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Transform a network point and a timestamp to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Pose_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBox *result = palloc0(sizeof(STBox));
  pose_timestamp_set_stbox(pose, t, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Pose_period_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_period_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Transform a network point and a period to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Pose_period_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Span *p = PG_GETARG_SPAN_P(1);
  STBox *result = palloc0(sizeof(STBox));
  pose_period_set_stbox(pose, p, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
