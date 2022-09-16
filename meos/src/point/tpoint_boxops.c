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

/* C */
#include <assert.h>
/* PostgreSQL */
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

extern void ll2cart(const POINT2D *g, POINT3D *p);
extern int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox);

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * Set the spatiotemporal box from a temporal instant point
 */
void
tpointinst_set_stbox(const TInstant *inst, STBOX *box)
{
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  /* Non-empty geometries have a bounding box
   * The argument box is set to 0 on the next call */
  geo_set_stbox(gs, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, &box->period);
  MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from an array of temporal instant points
 *
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 * @note Temporal instant values do not have a precomputed bounding box
 */
void
tgeompointinstarr_set_stbox(const TInstant **instants, int count, STBOX *box)
{
  tpointinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    tpointinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * Set the GBOX bounding box from an array of temporal geographic point instants
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] interp Interpolation
 * @param[out] box Resulting bounding box
 */
static void
tgeogpointinstarr_set_gbox(const TInstant **instants, int count,
  interpType interp, GBOX *box)
{
  LWPOINT **points = palloc(sizeof(LWPOINT *) * count);
  for (int i = 0; i < count; i++)
  {
    GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(instants[i]));
    points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  }
  LWGEOM *lwgeom = lwpointarr_make_trajectory((LWGEOM **) points, count,
    interp);
  lwgeom_calculate_gbox_geodetic(lwgeom, box);

  lwgeom_free(lwgeom);
  /* We cannot pfree(points); */
  return;
}

/**
 * Set the spatiotemporal box from an array of temporal instant geography point
 *
 * @note This function is called by the constructor of a temporal point
 * sequence when the points are geodetic to compute the bounding box.
 * Since the composing points have been already validated in the constructor
 * there is no verification of the input in this function, in particular
 * for geographies it is supposed that the composing points are geodetic
 *
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Spatiotemporal box
 * @note In the current PostGIS version the difference when computing the
 * gbox for a MultiPoint and a Linestring is around 2e-7
 */
void
tgeogpointinstarr_set_stbox(const TInstant **instants, int count,
  interpType interp, STBOX *box)
{
  GBOX gbox;
  gbox_init(&gbox);
  FLAGS_SET_Z(gbox.flags, 1);
  FLAGS_SET_M(gbox.flags, 0);
  FLAGS_SET_GEODETIC(gbox.flags, 1);
  tgeogpointinstarr_set_gbox(instants, count, interp, &gbox);
  bool hasz = MOBDB_FLAGS_GET_Z(instants[0]->flags);
  int32 srid = tpointinst_srid(instants[0]);
  Period period;
  span_set(instants[0]->t, instants[count - 1]->t, true, true, T_TIMESTAMPTZ,
    &period);
  stbox_set(&period, true, hasz, true, srid, gbox.xmin, gbox.xmax,
    gbox.ymin, gbox.ymax, gbox.zmin, gbox.zmax, box);
  return;
}

/**
 * Set the spatiotemporal box from an array of temporal sequence points
 *
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tpointseqarr_set_stbox(const TSequence **sequences, int count, STBOX *box)
{
  memcpy(box, TSEQUENCE_BBOX_PTR(sequences[0]), sizeof(STBOX));
  for (int i = 1; i < count; i++)
  {
    const STBOX *box1 = TSEQUENCE_BBOX_PTR(sequences[i]);
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
 * Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence point
 *
 * @param[in] seq Temporal value
 * @param[out] result Spatiotemporal box
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
    tpointinst_set_stbox(inst1, &result[0]);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  inst1 = tsequence_inst_n(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_set_stbox(inst1, &result[i]);
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    STBOX box;
    tpointinst_set_stbox(inst2, &box);
    stbox_expand(&box, &result[i]);
    inst1 = inst2;
  }
  return seq->count - 1;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence point.
 *
 * @param[in] seq Temporal value
 * @param[out] count Number of elements in the output array
 */
STBOX *
tpointseq_stboxes(const TSequence *seq, int *count)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  int newcount = seq->count == 1 ? 1 : seq->count - 1;
  STBOX *result = palloc(sizeof(STBOX) * newcount);
  tpointseq_stboxes1(seq, result);
  *count = newcount;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence set point.
 *
 * @param[in] ss Temporal value
 * @param[out] count Number of elements in the output array
 */
STBOX *
tpointseqset_stboxes(const TSequenceSet *ss, int *count)
{
  assert(MOBDB_FLAGS_GET_LINEAR(ss->flags));
  STBOX *result = palloc(sizeof(STBOX) * ss->totalcount);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    k += tpointseq_stboxes1(seq, &result[k]);
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from a temporal point
 * @sqlfunc stboxes()
 */
STBOX *
tpoint_stboxes(const Temporal *temp, int *count)
{
  STBOX *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || MOBDB_FLAGS_GET_DISCRETE(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_stboxes((TSequence *)temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_stboxes((TSequenceSet *)temp, count);
  return result;
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal point and a geometry.
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] func Bounding box function
 * @param[in] invert True if the geometry is the first argument of the
 * function
 */
int
boxop_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  if (gserialized_is_empty(gs))
    return -1;
  STBOX box1, box2;
  temporal_set_bbox(temp, &box1);
  geo_set_stbox(gs, &box2);
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
 * @param[in] invert True if the geometry is the first argument of the
 */
Datum
boxop_tpoint_stbox(const Temporal *temp, const STBOX *box,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  STBOX box1;
  temporal_set_bbox(temp, &box1);
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
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/
