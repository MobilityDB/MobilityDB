/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * pgPointCloud temporal types
 * @details Mirrors the stbox / tspatial bbox
 * surface in @c mobilitydb/src/geo/tgeo_boxops.c.
 *
 * The five topological families exposed are overlaps (&&), contains
 * (\@>), contained (<\@), same (~=) and adjacent (-|-), and the sixteen
 * position operators: strictly left, right, below, above, front, back,
 * before and after, and their overlapping variants. Each is wired to:
 * - tpcbox vs (tpcpoint | tpcpatch)        via @c Boxop_tpcbox_tpointcloud
 * - (tpcpoint | tpcpatch) vs tpcbox        via @c Boxop_tpointcloud_tpcbox
 * - tpointcloud vs tpointcloud             via @c Boxop_tpointcloud_tpointcloud
 *
 * tstzspan-based variants (tpcpoint vs tstzspan, etc.) are not
 * registered here — the generic @c Boxop_temporal_tstzspan in
 * @c mobilitydb/src/temporal/temporal_boxops.c already handles every
 * temporal type, so the SQL operator declarations in
 * @c 420_tpcpoint.in.sql / @c 430_tpcpatch.in.sql can target it
 * directly.
 */

/* C */
#include <float.h>
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

/* GENERATED-BOXOPS-BEGIN tpcbox — tools/codegen/inherited/generate.py from templates/boxops.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxtypes.yaml and re-run. */
/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a tpcbox and a temporal pointcloud value
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tpcbox_tpointcloud(FunctionCallInfo fcinfo,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpointcloud_tpcbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal pointcloud value and a
 * tpcbox
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tpointcloud_tpcbox(FunctionCallInfo fcinfo,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool result = boxop_tpointcloud_tpcbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic topological function for two temporal pointcloud values
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
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
 * overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if a tpcbox and the tpcbox of a
 * temporal pointcloud value overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overlaps_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overlaps_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of a temporal pointcloud value and
 * a tpcbox overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overlaps_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overlaps_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcboxes of two temporal pointcloud values
 * overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overlaps_tpcbox_tpcbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if a tpcbox contains the one of a
 * temporal pointcloud value
 * @sqlfn contains_bbox()
 * @sqlop @p @>
 */
inline Datum
Contains_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &contains_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Contains_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of a temporal pointcloud value
 * contains a tpcbox
 * @sqlfn contains_bbox()
 * @sqlop @p @>
 */
inline Datum
Contains_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &contains_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Contains_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of the first temporal pointcloud value
 * contains the one of the second temporal pointcloud value
 * @sqlfn contains_bbox()
 * @sqlop @p @>
 */
inline Datum
Contains_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &contains_tpcbox_tpcbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if a tpcbox is contained in the
 * tpcbox of a temporal pointcloud value
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
inline Datum
Contained_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &contained_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Contained_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of a temporal pointcloud value is
 * contained in the tpcbox
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
inline Datum
Contained_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &contained_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Contained_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of the first temporal pointcloud value
 * is contained in the one of the second temporal pointcloud value
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
inline Datum
Contained_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &contained_tpcbox_tpcbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PGDLLEXPORT Datum Same_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if a tpcbox and the tpcbox of a
 * temporal pointcloud value are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &same_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Same_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of a temporal pointcloud value and
 * a tpcbox are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &same_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Same_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcboxes of two temporal pointcloud values
 * are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &same_tpcbox_tpcbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if a tpcbox and the tpcbox of a
 * temporal pointcloud value are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &adjacent_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Adjacent_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcbox of a temporal pointcloud value
 * and a tpcbox are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &adjacent_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Adjacent_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true if the tpcboxes of two temporal pointcloud values
 * are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &adjacent_tpcbox_tpcbox);
}

/*****************************************************************************/
/* GENERATED-BOXOPS-END tpcbox */

/* GENERATED-POSOPS-BEGIN tpcbox — tools/codegen/inherited/generate.py from templates/posops.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxtypes.yaml and re-run. */
/*****************************************************************************
 * Position operators
 *****************************************************************************/

/*****************************************************************************/
/* tpcbox op temporal pointcloud value */

PGDLLEXPORT Datum Left_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is to the left of a temporal pointcloud value
 * @sqlfn tpcboxLeft()
 * @sqlop @p <<
 */
inline Datum
Left_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &left_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overleft_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend to the right of a temporal
 * pointcloud value
 * @sqlfn tpcboxOverleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overleft_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Right_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is to the right of a temporal pointcloud value
 * @sqlfn tpcboxRight()
 * @sqlop @p >>
 */
inline Datum
Right_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &right_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overright_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend to the left of a temporal
 * pointcloud value
 * @sqlfn tpcboxOverright()
 * @sqlop @p &>
 */
inline Datum
Overright_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overright_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Below_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is below a temporal pointcloud value
 * @sqlfn tpcboxBelow()
 * @sqlop @p <<|
 */
inline Datum
Below_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &below_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbelow_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend above a temporal pointcloud
 * value
 * @sqlfn tpcboxOverbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overbelow_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Above_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is above a temporal pointcloud value
 * @sqlfn tpcboxAbove()
 * @sqlop @p |>>
 */
inline Datum
Above_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &above_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overabove_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend below a temporal pointcloud
 * value
 * @sqlfn tpcboxOverabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overabove_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Front_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is in front of a temporal pointcloud value
 * @sqlfn tpcboxFront()
 * @sqlop @p <</
 */
inline Datum
Front_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &front_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overfront_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend to the back of a temporal
 * pointcloud value
 * @sqlfn tpcboxOverfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overfront_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Back_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is at the back of a temporal pointcloud value
 * @sqlfn tpcboxBack()
 * @sqlop @p />>
 */
inline Datum
Back_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &back_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overback_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox does not extend to the front of a temporal
 * pointcloud value
 * @sqlfn tpcboxOverback()
 * @sqlop @p /&>
 */
inline Datum
Overback_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overback_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Before_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is before a temporal pointcloud value
 * @sqlfn tpcboxBefore()
 * @sqlop @p <<#
 */
inline Datum
Before_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &before_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbefore_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is not after a temporal pointcloud value
 * @sqlfn tpcboxOverbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overbefore_tpcbox_tpcbox);
}

PGDLLEXPORT Datum After_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is after a temporal pointcloud value
 * @sqlfn tpcboxAfter()
 * @sqlop @p #>>
 */
inline Datum
After_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &after_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overafter_tpcbox_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpcbox_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a tpcbox is not before a temporal pointcloud value
 * @sqlfn tpcboxOverafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_tpcbox_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpcbox_tpointcloud(fcinfo, &overafter_tpcbox_tpcbox);
}

/*****************************************************************************/
/* temporal pointcloud value op tpcbox */

PGDLLEXPORT Datum Left_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is to the left of a tpcbox
 * @sqlfn tpcboxLeft()
 * @sqlop @p <<
 */
inline Datum
Left_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &left_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overleft_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend to the
 * right of a tpcbox
 * @sqlfn tpcboxOverleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overleft_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Right_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is to the right of a tpcbox
 * @sqlfn tpcboxRight()
 * @sqlop @p >>
 */
inline Datum
Right_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &right_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overright_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend to the left
 * of a tpcbox
 * @sqlfn tpcboxOverright()
 * @sqlop @p &>
 */
inline Datum
Overright_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overright_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Below_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is below a tpcbox
 * @sqlfn tpcboxBelow()
 * @sqlop @p <<|
 */
inline Datum
Below_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &below_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbelow_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend above a
 * tpcbox
 * @sqlfn tpcboxOverbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overbelow_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Above_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is above a tpcbox
 * @sqlfn tpcboxAbove()
 * @sqlop @p |>>
 */
inline Datum
Above_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &above_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overabove_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend below a
 * tpcbox
 * @sqlfn tpcboxOverabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overabove_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Front_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is in front of a tpcbox
 * @sqlfn tpcboxFront()
 * @sqlop @p <</
 */
inline Datum
Front_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &front_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overfront_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend to the back
 * of a tpcbox
 * @sqlfn tpcboxOverfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overfront_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Back_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is at the back of a tpcbox
 * @sqlfn tpcboxBack()
 * @sqlop @p />>
 */
inline Datum
Back_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &back_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overback_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value does not extend to the
 * front of a tpcbox
 * @sqlfn tpcboxOverback()
 * @sqlop @p /&>
 */
inline Datum
Overback_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overback_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Before_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is before a tpcbox
 * @sqlfn tpcboxBefore()
 * @sqlop @p <<#
 */
inline Datum
Before_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &before_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbefore_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is not after a tpcbox
 * @sqlfn tpcboxOverbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overbefore_tpcbox_tpcbox);
}

PGDLLEXPORT Datum After_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is after a tpcbox
 * @sqlfn tpcboxAfter()
 * @sqlop @p #>>
 */
inline Datum
After_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &after_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overafter_tpointcloud_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpointcloud_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if a temporal pointcloud value is not before a tpcbox
 * @sqlfn tpcboxOverafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_tpointcloud_tpcbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpcbox(fcinfo, &overafter_tpcbox_tpcbox);
}

/*****************************************************************************/
/* temporal pointcloud value op temporal pointcloud value */

PGDLLEXPORT Datum Left_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is to the left of
 * the second one
 * @sqlfn tpcboxLeft()
 * @sqlop @p <<
 */
inline Datum
Left_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &left_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overleft_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend to
 * the right of the second one
 * @sqlfn tpcboxOverleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overleft_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Right_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is to the right of
 * the second one
 * @sqlfn tpcboxRight()
 * @sqlop @p >>
 */
inline Datum
Right_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &right_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overright_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend to
 * the left of the second one
 * @sqlfn tpcboxOverright()
 * @sqlop @p &>
 */
inline Datum
Overright_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overright_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Below_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is below the second
 * one
 * @sqlfn tpcboxBelow()
 * @sqlop @p <<|
 */
inline Datum
Below_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &below_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbelow_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend
 * above the second one
 * @sqlfn tpcboxOverbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overbelow_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Above_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is above the second
 * one
 * @sqlfn tpcboxAbove()
 * @sqlop @p |>>
 */
inline Datum
Above_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &above_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overabove_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend
 * below the second one
 * @sqlfn tpcboxOverabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overabove_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Front_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is in front of the
 * second one
 * @sqlfn tpcboxFront()
 * @sqlop @p <</
 */
inline Datum
Front_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &front_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overfront_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend to
 * the back of the second one
 * @sqlfn tpcboxOverfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overfront_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Back_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is at the back of
 * the second one
 * @sqlfn tpcboxBack()
 * @sqlop @p />>
 */
inline Datum
Back_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &back_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overback_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value does not extend to
 * the front of the second one
 * @sqlfn tpcboxOverback()
 * @sqlop @p /&>
 */
inline Datum
Overback_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overback_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Before_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is before the
 * second one
 * @sqlfn tpcboxBefore()
 * @sqlop @p <<#
 */
inline Datum
Before_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &before_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overbefore_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is not after the
 * second one
 * @sqlfn tpcboxOverbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overbefore_tpcbox_tpcbox);
}

PGDLLEXPORT Datum After_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is after the second
 * one
 * @sqlfn tpcboxAfter()
 * @sqlop @p #>>
 */
inline Datum
After_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &after_tpcbox_tpcbox);
}

PGDLLEXPORT Datum Overafter_tpointcloud_tpointcloud(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpointcloud_tpointcloud);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief Return true if the first temporal pointcloud value is not before the
 * second one
 * @sqlfn tpcboxOverafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_tpointcloud_tpointcloud(PG_FUNCTION_ARGS)
{
  return Boxop_tpointcloud_tpointcloud(fcinfo, &overafter_tpcbox_tpcbox);
}

/*****************************************************************************/
/* GENERATED-POSOPS-END tpcbox */

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
  double result = nad_tpcbox_tpcbox(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
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
  if (result == DBL_MAX)
    PG_RETURN_NULL();
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
  if (result == DBL_MAX)
    PG_RETURN_NULL();
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
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
