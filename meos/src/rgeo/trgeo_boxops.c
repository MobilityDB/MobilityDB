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
 * @brief Bounding box operators for temporal rigid geometries
 */

#include "rgeo/trgeo_boxops.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos_internal.h>
#include "geo/stbox.h"
#include "pose/pose.h"
#include "rgeo/trgeo_inst.h"
#include "rgeo/trgeo_utils.h"

/*****************************************************************************
 * Transform a temporal geometry to a STBox
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the geometry value
 * @param[in] geom Reference geometry
 * @param[in] inst Temporal network point
 * @param[out] box Spatiotemporal box
 */
void
trgeoinst_set_stbox(const GSERIALIZED *geom, const TInstant *inst, STBox *box)
{
  const Pose *pose = DatumGetPoseP(tinstant_value(inst));

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  bool hasz = (bool) FLAGS_GET_Z(geom->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(geom->gflags);
  box->srid = gserialized_get_srid(geom);
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_T(box->flags, true);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);

  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);

  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  LWGEOM *lwgeom_copy = lwgeom_clone_deep(lwgeom);
  lwgeom_apply_pose(pose, lwgeom_copy);

  GBOX gbox;
  lwgeom_calculate_gbox(lwgeom_copy, &gbox);
  lwgeom_free(lwgeom_copy);
  lwgeom_free(lwgeom);

  box->xmin = gbox.xmin;
  box->xmax = gbox.xmax;
  box->ymin = gbox.ymin;
  box->ymax = gbox.ymax;
  if (hasz || geodetic)
  {
    box->zmin = gbox.zmin;
    box->zmax = gbox.zmax;
  }
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal rigid geometry values
 * @param[in] geom Reference geometry
 * @param[in] instants Temporal geometry values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
trgeoinstarr_static_stbox(const GSERIALIZED *geom, const TInstant **instants,
  int count, STBox *box)
{
  trgeoinst_set_stbox(geom, instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    memset(&box1, 0, sizeof(STBox));
    trgeoinst_set_stbox(geom, instants[i], &box1);
    stbox_expand(&box1, box);
  }
}

/**
 * @brief Set the spatiotemporal box from the pose value
 * @param[out] box Spatiotemporal box
 * @param[in] inst Temporal network point
 */
static void
trgeoinst_make_pose_stbox(const TInstant *inst, STBox *box)
{
  Pose *pose = DatumGetPoseP(tinstant_value(inst));
  box->xmin = box->xmax = pose->data[0];
  box->ymin = box->ymax = pose->data[1];
  if (MEOS_FLAGS_GET_Z(pose->flags))
    box->zmin = box->zmax = pose->data[2];
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, MEOS_FLAGS_GET_Z(pose->flags));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal rigid geometry
 * values
 * @param[in] geom Geometry
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
trgeoinstarr_rotating_stbox(const GSERIALIZED *geom, const TInstant **instants,
  int count, STBox *box)
{
  double r = geom_radius(geom);
  trgeoinst_make_pose_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    memset(&box1, 0, sizeof(STBox));
    trgeoinst_make_pose_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  box->xmin -= r;
  box->xmax += r;
  box->ymin -= r;
  box->ymax += r;
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    box->zmin -= r;
    box->zmax += r;
  }
  box->srid = gserialized_get_srid(geom);
  MEOS_FLAGS_SET_GEODETIC(box->flags, false);
  return;
}

/*****************************************************************************/

/**
 * @brief Set the bounding box from the array of temporal instant values
 * (dispatch function)
 * @param[in] geom Geometry
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Box
 */
void
trgeoinstarr_compute_bbox(const GSERIALIZED *geom, const TInstant **instants,
  int count, interpType interp, void *box)
{
  /* Only external types have bounding box */
  assert(temporal_type(instants[0]->temptype));
  if (instants[0]->temptype == T_TRGEOMETRY)
  {
    if (interp == LINEAR)
      trgeoinstarr_rotating_stbox(geom, instants, count, (STBox *) box);
    else
      trgeoinstarr_static_stbox(geom, instants, count, (STBox *) box);
  }
  else
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unknown bounding box function for temporal type: %d",
      meostype_name(instants[0]->temptype));
  return;
}

/*****************************************************************************/
