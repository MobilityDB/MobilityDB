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
 * @brief Bounding box operators for spatiotemporal values
 * @details These operators test the bounding box of spatiotemporal values,
 * which is an`STBox`, where the *x*, *y*, and optional *z* coordinates are
 * or the space (value) dimension and the *t* coordinate is for the time 
 * dimension. The following operators are defined: `overlaps`, `contains`,
 * `contained`, and `same`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "geo/tspatial_boxops.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "geo/stbox.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Boxes function
 *****************************************************************************/

PGDLLEXPORT Datum Tgeo_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a spatiotemporal value, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or 
 * continuous
 * @sqlfn stboxes()
 */
Datum
Tgeo_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  STBox *boxes = tgeo_stboxes(temp, &count);
  PG_FREE_IF_COPY(temp, 0);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Geo_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of spatial boxes from the segments of a 
 * (mult)linestring
 * @sqlfn stboxes()
 */
Datum
Geo_stboxes(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int count;
  STBox *boxes = geo_stboxes(gs, &count);
  PG_FREE_IF_COPY(gs, 0);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tgeo_split_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_split_n_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of N spatiotemporal boxes from the instants or
 * segments of a spatiotemporal value, where the choice between instants or
 * segments depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @sqlfn splitNStboxes()
 */
Datum
Tgeo_split_n_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int box_count = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = tgeo_split_n_stboxes(temp, box_count, &count);
  PG_FREE_IF_COPY(temp, 0);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Geo_split_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_split_n_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of N spatial boxes from the segments of a 
 * (multi)linestring
 * @sqlfn splitNStboxes()
 */
Datum
Geo_split_n_stboxes(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int box_count = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = geo_split_n_stboxes(gs, box_count, &count);
  PG_FREE_IF_COPY(gs, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tgeo_split_each_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_split_each_n_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a spatiotemporal value, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @sqlfn splitEachNStboxes()
 */
Datum
Tgeo_split_each_n_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int elems_per_box = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = tgeo_split_each_n_stboxes(temp, elems_per_box, &count);
  PG_FREE_IF_COPY(temp, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Geo_split_each_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_split_each_n_stboxes);
/**
 * @ingroup mobilitydb_geo_bbox
 * @brief Return an array of spatial boxes from the segments of a
 * (multi)linestring
 * @sqlfn splitEachNStboxes()
 */
Datum
Geo_split_each_n_stboxes(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int elems_per_box = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = geo_split_each_n_stboxes(gs, elems_per_box, &count);
  PG_FREE_IF_COPY(gs, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a spatiotemporal box and a temporal
 * point
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_stbox_tspatial(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tspatial_stbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a spatiotemporal value and a
 * spatiotemporal box
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tspatial_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool result = boxop_tspatial_stbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic topological function for two spatiotemporal values
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tspatial_tspatial(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tspatial_tspatial(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value and
 * a spatiotemporal box overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal
 * values overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
inline Datum
Overlaps_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if a spatiotemporal box contains the one of a temporal
 * point
 * @sqlfn contains_bbox()
 * @sqlop @p <@
 */
inline Datum
Contains_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value
 * contains a spatiotemporal box
 * @sqlfn contains_bbox()
 * @sqlop @p <@
 */
inline Datum
Contains_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of the first spatiotemporal
 * value contains the one of the second spatiotemporal value
 * @sqlfn contains_bbox()
 * @sqlop @p <@
 */
inline Datum
Contains_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if a spatiotemporal box is contained in the
 * spatiotemporal box of a spatiotemporal value
 * @sqlfn contained_bbox()
 * @sqlop @p \@>
 */
inline Datum
Contained_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value is
 * contained in the spatiotemporal box
 * @sqlfn contained_bbox()
 * @sqlop @p \@>
 */
inline Datum
Contained_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of the first spatiotemporal
 * value is contained in the one of the second spatiotemporal value
 * @sqlfn contained_bbox()
 * @sqlop @p \@>
 */
inline Datum
Contained_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PGDLLEXPORT Datum Same_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value and
 * a spatiotemporal box are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal
 * values are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
inline Datum
Same_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value
 * and a spatiotemporal box are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal
 * values are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
inline Datum
Adjacent_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/
