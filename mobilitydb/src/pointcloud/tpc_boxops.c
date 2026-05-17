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
 * @brief PG wrappers for TPCBox-based bounding-box operators on the
 *   pgPointCloud temporal types. Mirrors the stbox / tspatial bbox
 *   surface in @c mobilitydb/src/geo/tgeo_boxops.c.
 *
 * The five families exposed are overlaps (&&), contains (\@>),
 * contained (<\@), same (~=), and adjacent (-|-). Each is wired to:
 *   * tpcbox vs (tpcpoint | tpcpatch)        via @c Boxop_tpcbox_tpointcloud
 *   * (tpcpoint | tpcpatch) vs tpcbox        via @c Boxop_tpointcloud_tpcbox
 *   * tpointcloud vs tpointcloud             via @c Boxop_tpointcloud_tpointcloud
 *
 * tstzspan-based variants (tpcpoint vs tstzspan, etc.) are not
 * registered here — the generic @c Boxop_temporal_tstzspan in
 * @c mobilitydb/src/temporal/temporal_boxops.c already handles every
 * temporal type, so the SQL operator declarations in
 * @c 420_tpcpoint.in.sql / @c 430_tpcpatch.in.sql can target it
 * directly.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "pointcloud/tpc_boxops.h"
#include "pointcloud/tpcbox.h"          /* PG_GETARG_TPCBOX_P */
#include "temporal/temporal.h"

/*****************************************************************************
 * Generic dispatchers
 *****************************************************************************/

static Datum
Boxop_tpcbox_tpointcloud(FunctionCallInfo fcinfo,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpointcloud_tpcbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

static Datum
Boxop_tpointcloud_tpcbox(FunctionCallInfo fcinfo,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool result = boxop_tpointcloud_tpcbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

static Datum
Boxop_tpointcloud_tpointcloud(FunctionCallInfo fcinfo,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpointcloud_tpointcloud(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Per-operator wrappers
 *
 * Two PG-exposed functions per (operator, direction) pair — one
 * static dispatch line each. Macros would obscure the @ingroup tags
 * Doxygen needs, so we expand them by hand.
 *****************************************************************************/

#define DEFINE_BOXOP3(opname, primitive)                                       \
PGDLLEXPORT Datum opname##_tpcbox_tpointcloud(PG_FUNCTION_ARGS);               \
PG_FUNCTION_INFO_V1(opname##_tpcbox_tpointcloud);                              \
Datum                                                                          \
opname##_tpcbox_tpointcloud(PG_FUNCTION_ARGS)                                  \
{                                                                              \
  return Boxop_tpcbox_tpointcloud(fcinfo, &primitive);                         \
}                                                                              \
PGDLLEXPORT Datum opname##_tpointcloud_tpcbox(PG_FUNCTION_ARGS);               \
PG_FUNCTION_INFO_V1(opname##_tpointcloud_tpcbox);                              \
Datum                                                                          \
opname##_tpointcloud_tpcbox(PG_FUNCTION_ARGS)                                  \
{                                                                              \
  return Boxop_tpointcloud_tpcbox(fcinfo, &primitive);                         \
}                                                                              \
PGDLLEXPORT Datum opname##_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);          \
PG_FUNCTION_INFO_V1(opname##_tpointcloud_tpointcloud);                         \
Datum                                                                          \
opname##_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)                             \
{                                                                              \
  return Boxop_tpointcloud_tpointcloud(fcinfo, &primitive);                    \
}

/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Bounding-box overlap (`&&`) between a tpcbox and a temporal
 *   pgPointCloud value, or between two such values.
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
DEFINE_BOXOP3(Overlaps, overlaps_tpcbox_tpcbox)

/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Bounding-box contains (`\@>`).
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
DEFINE_BOXOP3(Contains, contains_tpcbox_tpcbox)

/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Bounding-box contained (`<\@`).
 * @sqlfn contained_bbox()
 * @sqlop @p <\@
 */
DEFINE_BOXOP3(Contained, contained_tpcbox_tpcbox)

/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Bounding-box equality (`~=`).
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
DEFINE_BOXOP3(Same, same_tpcbox_tpcbox)

/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Bounding-box adjacency (`-|-`).
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
DEFINE_BOXOP3(Adjacent, adjacent_tpcbox_tpcbox)

/*****************************************************************************
 * Position operators — strictly left / right / below / above / front /
 * back / before / after, plus their "overlaps-or-X" variants.
 *****************************************************************************/

DEFINE_BOXOP3(Left,        left_tpcbox_tpcbox)
DEFINE_BOXOP3(Overleft,    overleft_tpcbox_tpcbox)
DEFINE_BOXOP3(Right,       right_tpcbox_tpcbox)
DEFINE_BOXOP3(Overright,   overright_tpcbox_tpcbox)
DEFINE_BOXOP3(Below,       below_tpcbox_tpcbox)
DEFINE_BOXOP3(Overbelow,   overbelow_tpcbox_tpcbox)
DEFINE_BOXOP3(Above,       above_tpcbox_tpcbox)
DEFINE_BOXOP3(Overabove,   overabove_tpcbox_tpcbox)
DEFINE_BOXOP3(Front,       front_tpcbox_tpcbox)
DEFINE_BOXOP3(Overfront,   overfront_tpcbox_tpcbox)
DEFINE_BOXOP3(Back,        back_tpcbox_tpcbox)
DEFINE_BOXOP3(Overback,    overback_tpcbox_tpcbox)
DEFINE_BOXOP3(Before,      before_tpcbox_tpcbox)
DEFINE_BOXOP3(Overbefore,  overbefore_tpcbox_tpcbox)
DEFINE_BOXOP3(After,       after_tpcbox_tpcbox)
DEFINE_BOXOP3(Overafter,   overafter_tpcbox_tpcbox)

#undef DEFINE_BOXOP3

/*****************************************************************************
 * Nearest-approach distance (|=|)
 *****************************************************************************/

PGDLLEXPORT Datum NAD_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Nearest-approach distance between two TPCBox values.
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{
  TPCBox *box1 = PG_GETARG_TPCBOX_P(0);
  TPCBox *box2 = PG_GETARG_TPCBOX_P(1);
  PG_RETURN_FLOAT8(nad_tpcbox_tpcbox(box1, box2));
}

PGDLLEXPORT Datum NAD_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Nearest-approach distance between a tpcpoint/tpcpatch and a TPCBox.
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  double result = nad_tpointcloud_tpcbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Nearest-approach distance between a TPCBox and a tpcpoint/tpcpatch.
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tpointcloud_tpcbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Nearest-approach distance between two temporal pointcloud values.
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tpointcloud_tpointcloud(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
