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
 * @brief Bounding box operators for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "geo/stbox.h"
#include "rgeo/trgeo_boxops.h"
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"

/*****************************************************************************/

PGDLLEXPORT Datum Trgeo_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_stboxes);
/**
 * @ingroup mobilitydb_rgeo_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal rigid geometry
 * @sqlfn stboxes()
 */
Datum
Trgeo_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  STBox *boxes = trgeo_stboxes(temp, &count);
  PG_FREE_IF_COPY(temp, 0);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Trgeo_split_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_split_n_stboxes);
/**
 * @ingroup mobilitydb_rgeo_bbox
 * @brief Return an array of N spatiotemporal boxes from a temporal rigid
 * geometry
 * @sqlfn splitNStboxes()
 */
Datum
Trgeo_split_n_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int box_count = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = trgeo_split_n_stboxes(temp, box_count, &count);
  PG_FREE_IF_COPY(temp, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Trgeo_split_each_n_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_split_each_n_stboxes);
/**
 * @ingroup mobilitydb_rgeo_bbox
 * @brief Return an array of spatiotemporal boxes from a temporal rigid
 * geometry, one per every N instants or segments
 * @sqlfn splitEachNStboxes()
 */
Datum
Trgeo_split_each_n_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int elems_per_box = PG_GETARG_INT32(1);
  int count;
  STBox *boxes = trgeo_split_each_n_stboxes(temp, elems_per_box, &count);
  PG_FREE_IF_COPY(temp, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/
