/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tpoint_boxops.c
 * @brief Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are an
 * `STBOX`, where the *x*, *y*, and optional *z* coordinates are for the space
 * (value) dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "point/tpoint_boxops.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "point/tpoint.h"
#include "point/stbox.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

extern void ll2cart(const POINT2D *g, POINT3D *p);
extern int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox);

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * Set the spatiotemporal box from the temporal point value
 */
void
tpointinst_stbox(const TInstant *inst, STBOX *box)
{
  Datum value = tinstant_value(inst);
  GSERIALIZED *gs = (GSERIALIZED *) PointerGetDatum(value);
  /* Non-empty geometries have a bounding box */
  geo_stbox(gs, box);
  box->tmin = box->tmax = inst->t;
  MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from the array of temporal point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @note Temporal instant values do not have a precomputed bounding box
 */
void
tgeompointinstarr_stbox(const TInstant **instants, int count, STBOX *box)
{
  tpointinst_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    tpointinst_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * Compute the GBOX bounding box of an array of temporal point instants
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[out] box Resulting bounding box
 */
static void
tpointinstarr_gbox(const TInstant **instants, int count, GBOX *box)
{
  assert(box);
  assert(count > 0);
  const POINT2D *p;
  POINT3D A1, A2;
  GBOX edge_gbox;
  gbox_init(&edge_gbox);
  edge_gbox.flags = box->flags;

  /* Initialization with the first instant */
  p = datum_point2d_p(tinstant_value(instants[0]));
  ll2cart(p, &A1);
  box->xmin = box->xmax = A1.x;
  box->ymin = box->ymax = A1.y;
  box->zmin = box->zmax = A1.z;
  for (int i = 1; i < count; i++)
  {
    p = datum_point2d_p(tinstant_value(instants[i]));
    ll2cart(p, &A2);
    edge_calculate_gbox(&A1, &A2, &edge_gbox);
    /* Expand the box where necessary */
    gbox_merge(&edge_gbox, box);
    A1 = A2;
  }
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal geography point values
 *
 * @note This function is called by the constructor of a temporal point
 * sequence when the points are geodetic to compute the bounding box.
 * Since the composing points have been already validated in the constructor
 * there is no verification of the input in this function, in particular
 * for geographies it is supposed that the composing points are geodetic
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @note In the current PostGIS version the difference when computing the
 * gbox for a MultiPoint and a Linestring is around 2e-7
 */
void
tgeogpointinstarr_stbox(const TInstant **instants, int count, STBOX *box)
{
  GBOX gbox;
  gbox_init(&gbox);
  FLAGS_SET_Z(gbox.flags, 1);
  FLAGS_SET_M(gbox.flags, 0);
  FLAGS_SET_GEODETIC(gbox.flags, 1);
  tpointinstarr_gbox(instants, count, &gbox);
  bool hasz = MOBDB_FLAGS_GET_Z(instants[0]->flags);
  int32 srid = tpointinst_srid(instants[0]);
  stbox_set(true, hasz, true, true, srid, gbox.xmin, gbox.xmax,
    gbox.ymin, gbox.ymax, gbox.zmin, gbox.zmax,
    instants[0]->t, instants[count - 1]->t, box);
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tpointseqarr_stbox(const TSequence **sequences, int count, STBOX *box)
{
  memcpy(box, tsequence_bbox_ptr(sequences[0]), sizeof(STBOX));
  for (int i = 1; i < count; i++)
  {
    const STBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    stbox_expand(box1, box);
  }
  return;
}

/*****************************************************************************
 * Boxes functions
 * These functions can be used for defining VODKA indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 *****************************************************************************/

/**
 * Return an array of spatiotemporal boxes from the segments of the
 * temporal sequence point value
 *
 * @param[out] result Spatiotemporal box
 * @param[in] seq Temporal value
 * @return Number of elements in the array
 */
static int
tpointseq_stboxes1(const TSequence *seq, STBOX *result)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    tpointinst_stbox(inst1, &result[0]);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  inst1 = tsequence_inst_n(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_stbox(inst1, &result[i]);
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    STBOX box;
    tpointinst_stbox(inst2, &box);
    stbox_expand(&box, &result[i]);
    inst1 = inst2;
  }
  return seq->count - 1;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of the
 * temporal sequence point value.
 *
 * @param[in] seq Temporal value
 * @param[out] count Number of elements in the output array
 */
STBOX *
tpointseq_stboxes(const TSequence *seq, int *count)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  int newcount = seq->count == 1 ? 1 : seq->count - 1;
  STBOX *result = palloc0(sizeof(STBOX) * newcount);
  tpointseq_stboxes1(seq, result);
  *count = newcount;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of the
 * temporal sequence set point value.
 *
 * @param[in] ts Temporal value
 * @param[out] count Number of elements in the output array
 */
STBOX *
tpointseqset_stboxes(const TSequenceSet *ts, int *count)
{
  assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
  STBOX *result = palloc0(sizeof(STBOX) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tpointseq_stboxes1(seq, &result[k]);
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the temporal point
 */
STBOX *
tpoint_stboxes(const Temporal *temp, int *count)
{
  STBOX *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    ;
  else if (temp->subtype == SEQUENCE)
    result = tpointseq_stboxes((TSequence *)temp, count);
  else /* temp->subtype == SEQUENCESET */
    result = tpointseqset_stboxes((TSequenceSet *)temp, count);
  return result;
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Generic bounding box function for a temporal point and a geometry.
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] func Bounding box function
 * @param[in] invert True when the geometry is the first argument of the
 * function
 */
int
boxop_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  if (gserialized_is_empty(gs))
    return -1;
  STBOX box1, box2;
  temporal_bbox(temp, &box1);
  geo_stbox(gs, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result ? 1 : 0;
}

/**
 * @brief Generic bounding box function for a temporal point and a
 * spatiotemporal box
 *
 * @param[in] temp Temporal point
 * @param[in] box Box
 * @param[in] func Bounding box function
 * @param[in] invert True when the geometry is the first argument of the
 */
Datum
boxop_tpoint_stbox(const Temporal *temp, const STBOX *box,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result;
}

/**
 * @brief Generic bounding box function for two temporal points.
 *
 * @param[in] temp1,temp2 Temporal points
 * @param[in] func Bounding box function
 */
bool
boxop_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Boxes function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_stboxes);
/**
 * Return an array of spatiotemporal boxes from the temporal point
 */
PGDLLEXPORT Datum
Tpoint_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  STBOX *boxes = tpoint_stboxes(temp, &count);
  PG_FREE_IF_COPY(temp, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * Generic bounding box function for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_geo_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = boxop_tpoint_geo(temp, gs, func, true);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * Generic bounding box function for a temporal point and a geometry.
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = boxop_tpoint_geo(temp, gs, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * Generic bounding box function for a spatiotemporal box and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_stbox_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpoint_stbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic bounding box function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool result = boxop_tpoint_stbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for two temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpoint_tpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_geo_tpoint);
/**
 * Return true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point overlap
 */
PGDLLEXPORT Datum
Overlaps_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_stbox_tpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point overlap
 */
PGDLLEXPORT Datum
Overlaps_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal point and the geometry/geography overlap
 */
PGDLLEXPORT Datum
Overlaps_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal point and the spatiotemporal box overlap
 */
PGDLLEXPORT Datum
Overlaps_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_tpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal points overlap
 */
PGDLLEXPORT Datum
Overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_bbox_geo_tpoint);
/**
 * Return true if the spatiotemporal box of the geometry/geography contains
 * the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
Contains_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_stbox_tpoint);
/**
 * Return true if the spatiotemporal box contains the spatiotemporal box of the
 * temporal point
 */
PGDLLEXPORT Datum
Contains_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_geo);
/**
 * Return true if the spatiotemporal box of the temporal point contains the
 * one of the geometry/geography
 */
PGDLLEXPORT Datum
Contains_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal point contains the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Contains_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_tpoint);
/**
 * Return true if the spatiotemporal box of the first temporal point contains
 * the one of the second temporal point
 */
PGDLLEXPORT Datum
Contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_geo_tpoint);
/**
 * Return true if the spatiotemporal box of the geometry/geography is
 * contained in the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
Contained_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is contained in the spatiotemporal
 * box of the temporal point
 */
PGDLLEXPORT Datum
Contained_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_geo);
/**
 * Return true if the spatiotemporal box of the temporal point is contained
 * in the one of the geometry/geography
 */
PGDLLEXPORT Datum
Contained_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal point is contained
 * in the spatiotemporal box
 */
PGDLLEXPORT Datum
Contained_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_tpoint);
/**
 * Return true if the spatiotemporal box of the first temporal point is contained
 * in the one of the second temporal point
 */
PGDLLEXPORT Datum
Contained_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Same_geo_tpoint);
/**
 * Return true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_stbox_tpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal point and
 * geometry/geography are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal point and the
 * spatiotemporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_tpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal points are equal
 * in the common dimensions
 */
PGDLLEXPORT Datum
Same_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_geo_tpoint);
/**
 * Return true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_stbox_tpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal point and
 * geometry/geography are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal point and the
 * spatiotemporal box are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_tpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal points are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
