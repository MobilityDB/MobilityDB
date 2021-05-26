/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * tnpoint_posops.c
 * Relative position operators for temporal network points.
 *
 * The following operators are defined for the spatial dimension:
 * - left, overleft, right, overright, below, overbelow, above, overabove,
 *   front, overfront, back, overback
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 * The following operators for the temporal dimension:
 * - before, overbefore, after, overafter
 * for both temporal geometry and geography points are "inherited" from the
 * basic temporal types. In this file they are defined when one of the
 * arguments is a stbox.
 */
/*****************************************************************************
 *
 * 
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tnpoint_posops.h"

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <liblwgeom.h>
#include "postgis.h"
#include "tpoint_boxops.h"
#include "tpoint_posops.h"
#include "tpoint_spatialfuncs.h"
#include "tnpoint.h"
#include "tnpoint_static.h"
#include "tnpoint_boxops.h"
#include "tnpoint_spatialfuncs.h"

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(left_geom_tnpoint);

PGDLLEXPORT Datum
left_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = left_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_geom_tnpoint);

PGDLLEXPORT Datum
overleft_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = overleft_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_geom_tnpoint);

PGDLLEXPORT Datum
right_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = right_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_geom_tnpoint);

PGDLLEXPORT Datum
overright_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = overright_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_geom_tnpoint);

PGDLLEXPORT Datum
below_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = below_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_geom_tnpoint);

PGDLLEXPORT Datum
overbelow_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_geom_tnpoint);

PGDLLEXPORT Datum
above_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = above_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_geom_tnpoint);

PGDLLEXPORT Datum
overabove_geom_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box1, gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box2, temp);
  bool result = overabove_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(left_tnpoint_geom);

PGDLLEXPORT Datum
left_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = left_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnpoint_geom);

PGDLLEXPORT Datum
overleft_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = overleft_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnpoint_geom);

PGDLLEXPORT Datum
right_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = right_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnpoint_geom);

PGDLLEXPORT Datum
overright_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = overright_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tnpoint_geom);

PGDLLEXPORT Datum
below_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = below_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tnpoint_geom);

PGDLLEXPORT Datum
overbelow_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tnpoint_geom);

PGDLLEXPORT Datum
above_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = above_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tnpoint_geom);

PGDLLEXPORT Datum
overabove_tnpoint_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_same_srid_tnpoint_gs(temp, gs);
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  if (!geo_to_stbox_internal(&box2, gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }
  temporal_bbox(&box1, temp);
  bool result = overabove_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(left_stbox_tnpoint);

PGDLLEXPORT Datum
left_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = left_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_stbox_tnpoint);

PGDLLEXPORT Datum
overleft_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overleft_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_stbox_tnpoint);

PGDLLEXPORT Datum
right_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = right_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_stbox_tnpoint);

PGDLLEXPORT Datum
overright_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overright_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_stbox_tnpoint);

PGDLLEXPORT Datum
below_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = below_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_tnpoint);

PGDLLEXPORT Datum
overbelow_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_stbox_tnpoint);

PGDLLEXPORT Datum
above_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = above_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_stbox_tnpoint);

PGDLLEXPORT Datum
overabove_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overabove_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_stbox_tnpoint);

PGDLLEXPORT Datum
before_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = before_stbox_stbox_internal(box, &box1);
  }
  PG_FREE_IF_COPY(temp, 1);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_tnpoint);

PGDLLEXPORT Datum
overbefore_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = overbefore_stbox_stbox_internal(box, &box1);
  }
  PG_FREE_IF_COPY(temp, 1);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_stbox_tnpoint);

PGDLLEXPORT Datum
after_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = after_stbox_stbox_internal(box, &box1);
  }
  PG_FREE_IF_COPY(temp, 1);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_stbox_tnpoint);

PGDLLEXPORT Datum
overafter_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = overafter_stbox_stbox_internal(box, &box1);
  }
  PG_FREE_IF_COPY(temp, 1);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(left_tnpoint_stbox);

PGDLLEXPORT Datum
left_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = left_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnpoint_stbox);

PGDLLEXPORT Datum
overleft_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overleft_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnpoint_stbox);

PGDLLEXPORT Datum
right_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = right_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnpoint_stbox);

PGDLLEXPORT Datum
overright_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overright_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tnpoint_stbox);

PGDLLEXPORT Datum
below_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = below_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tnpoint_stbox);

PGDLLEXPORT Datum
overbelow_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tnpoint_stbox);

PGDLLEXPORT Datum
above_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = above_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tnpoint_stbox);

PGDLLEXPORT Datum
overabove_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_not_geodetic_stbox(box);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  bool result = overabove_stbox_stbox_internal(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tnpoint_stbox);

PGDLLEXPORT Datum
before_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = before_stbox_stbox_internal(&box1, box);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tnpoint_stbox);

PGDLLEXPORT Datum
overbefore_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = overbefore_stbox_stbox_internal(&box1, box);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tnpoint_stbox);

PGDLLEXPORT Datum
after_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = after_stbox_stbox_internal(&box1, box);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tnpoint_stbox);

PGDLLEXPORT Datum
overafter_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    result = overafter_stbox_stbox_internal(&box1, box);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* npoint op Temporal */

PG_FUNCTION_INFO_V1(left_npoint_tnpoint);

PGDLLEXPORT Datum
left_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = left_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_npoint_tnpoint);

PGDLLEXPORT Datum
overleft_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = overleft_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_npoint_tnpoint);

PGDLLEXPORT Datum
right_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = right_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_npoint_tnpoint);

PGDLLEXPORT Datum
overright_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = overright_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_npoint_tnpoint);

PGDLLEXPORT Datum
below_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = below_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_npoint_tnpoint);

PGDLLEXPORT Datum
overbelow_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_npoint_tnpoint);

PGDLLEXPORT Datum
above_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = above_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_npoint_tnpoint);

PGDLLEXPORT Datum
overabove_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = overabove_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op npoint */

PG_FUNCTION_INFO_V1(left_tnpoint_npoint);

PGDLLEXPORT Datum
left_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = left_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnpoint_npoint);

PGDLLEXPORT Datum
overleft_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = overleft_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnpoint_npoint);

PGDLLEXPORT Datum
right_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = right_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnpoint_npoint);

PGDLLEXPORT Datum
overright_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = overright_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tnpoint_npoint);

PGDLLEXPORT Datum
below_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = below_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tnpoint_npoint);

PGDLLEXPORT Datum
overbelow_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = overbelow_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tnpoint_npoint);

PGDLLEXPORT Datum
above_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = above_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tnpoint_npoint);

PGDLLEXPORT Datum
overabove_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid_tnpoint_npoint(temp, np);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  temporal_bbox(&box1, temp);
  bool result = overabove_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(left_tnpoint_tnpoint);

PGDLLEXPORT Datum
left_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = left_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnpoint_tnpoint);

PGDLLEXPORT Datum
overleft_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overleft_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnpoint_tnpoint);

PGDLLEXPORT Datum
right_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = right_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnpoint_tnpoint);

PGDLLEXPORT Datum
overright_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overright_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tnpoint_tnpoint);

PGDLLEXPORT Datum
below_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = below_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tnpoint_tnpoint);

PGDLLEXPORT Datum
overbelow_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overbelow_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tnpoint_tnpoint);

PGDLLEXPORT Datum
above_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = above_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tnpoint_tnpoint);

PGDLLEXPORT Datum
overabove_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tnpoint(temp1, temp2);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overabove_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tnpoint_tnpoint);

PGDLLEXPORT Datum
before_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = before_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tnpoint_tnpoint);

PGDLLEXPORT Datum
overbefore_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overbefore_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tnpoint_tnpoint);

PGDLLEXPORT Datum
after_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = after_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tnpoint_tnpoint);

PGDLLEXPORT Datum
overafter_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  bool result = overafter_stbox_stbox_internal(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
