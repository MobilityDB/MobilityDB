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
 * @file stbox.h
 * Functions for spatiotemporal bounding boxes.
 */

#ifndef __STBOX_H__
#define __STBOX_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <liblwgeom.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************
 * Struct definition
 *****************************************************************************/

/**
 * Structure to represent spatiotemporal boxes
 */
typedef struct
{
  double      xmin;   /**< minimum x value */
  double      xmax;   /**< maximum x value */
  double      ymin;   /**< minimum y value */
  double      ymax;   /**< maximum y value */
  double      zmin;   /**< minimum z value */
  double      zmax;   /**< maximum z value */
  TimestampTz tmin;   /**< minimum timestamp */
  TimestampTz tmax;   /**< maximum timestamp */
  int32       srid;   /**< SRID */
  int16       flags;  /**< flags */
} STBOX;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetSTboxP(X)    ((STBOX *) DatumGetPointer(X))
#define STboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_STBOX_P(n) DatumGetSTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_STBOX_P(x) return STboxPGetDatum(x)

/*****************************************************************************/

/* General functions */

extern STBOX *stbox_make(bool hasx, bool hasz, bool hast, bool geodetic,
  int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, TimestampTz tmin, TimestampTz tmax);
extern void stbox_set(bool hasx, bool hasz, bool hast, bool geodetic,
  int32 srid, double xmin, double xmax, double ymin, double ymax,
  double zmin, double zmax, TimestampTz tmin, TimestampTz tmax, STBOX *box);
extern STBOX *stbox_copy(const STBOX *box);
extern void stbox_expand(const STBOX *box1, STBOX *box2);
extern void stbox_shift_tscale(const Interval *start, const Interval *duration,
  STBOX *box);

/* Parameter tests */

extern void ensure_has_X_stbox(const STBOX *box);
extern void ensure_has_T_stbox(const STBOX *box);

/* Input/Ouput functions */

extern Datum Stbox_in(PG_FUNCTION_ARGS);
extern Datum Stbox_out(PG_FUNCTION_ARGS);
extern Datum Stbox_send(PG_FUNCTION_ARGS);
extern Datum Stbox_recv(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum Stbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum Stbox_constructor(PG_FUNCTION_ARGS);
extern Datum Stbox_constructor_z(PG_FUNCTION_ARGS);
extern Datum Stbox_constructor_zt(PG_FUNCTION_ARGS);
extern Datum Geodstbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum Geodstbox_constructor(PG_FUNCTION_ARGS);
extern Datum Geodstbox_constructor_z(PG_FUNCTION_ARGS);
extern Datum Geodstbox_constructor_zt(PG_FUNCTION_ARGS);

/* Casting */

extern Datum Stbox_to_period(PG_FUNCTION_ARGS);
extern Datum Stbox_to_box2d(PG_FUNCTION_ARGS);
extern Datum Stbox_to_box3d(PG_FUNCTION_ARGS);
extern Datum Stbox_to_geometry(PG_FUNCTION_ARGS);

extern void stbox_gbox(const STBOX *box, GBOX * gbox);
extern void stbox_box3d(const STBOX *box, BOX3D *box3d);

/* Transform a <Type> to a STBOX */

extern Datum Box2d_to_stbox(PG_FUNCTION_ARGS);
extern Datum Box3d_to_stbox(PG_FUNCTION_ARGS);
extern Datum Geo_to_stbox(PG_FUNCTION_ARGS);
extern Datum Timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_stbox(PG_FUNCTION_ARGS);
extern Datum Period_to_stbox(PG_FUNCTION_ARGS);
extern Datum Periodset_to_stbox(PG_FUNCTION_ARGS);
extern Datum Geo_timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum Geo_period_to_stbox(PG_FUNCTION_ARGS);

extern bool geo_stbox(const GSERIALIZED *gs, STBOX *box);
extern void timestamp_stbox(TimestampTz t, STBOX *box);
extern void timestampset_stbox(const TimestampSet *ps, STBOX *box);
extern void timestampset_stbox_slice(Datum tsdatum, STBOX *box);
extern void period_stbox(const Period *p, STBOX *box);
extern void periodset_stbox(const PeriodSet *ps, STBOX *box);
extern void periodset_stbox_slice(Datum psdatum, STBOX *box);

/* Accessor functions */

extern Datum Stbox_hasx(PG_FUNCTION_ARGS);
extern Datum Stbox_hasz(PG_FUNCTION_ARGS);
extern Datum Stbox_hast(PG_FUNCTION_ARGS);
extern Datum Stbox_isgeodetic(PG_FUNCTION_ARGS);
extern Datum Stbox_xmin(PG_FUNCTION_ARGS);
extern Datum Stbox_xmax(PG_FUNCTION_ARGS);
extern Datum Stbox_ymin(PG_FUNCTION_ARGS);
extern Datum Stbox_ymax(PG_FUNCTION_ARGS);
extern Datum Stbox_zmin(PG_FUNCTION_ARGS);
extern Datum Stbox_zmax(PG_FUNCTION_ARGS);
extern Datum Stbox_tmin(PG_FUNCTION_ARGS);
extern Datum Stbox_tmax(PG_FUNCTION_ARGS);

/* SRID functions */

extern Datum Stbox_srid(PG_FUNCTION_ARGS);
extern Datum Stbox_set_srid(PG_FUNCTION_ARGS);
extern Datum Stbox_transform(PG_FUNCTION_ARGS);

/* Transformation functions */

extern Datum Stbox_expand_spatial(PG_FUNCTION_ARGS);
extern Datum Stbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum Stbox_round(PG_FUNCTION_ARGS);

extern STBOX *stbox_expand_spatial(const STBOX *box, double d);
extern STBOX *stbox_expand_temporal(const STBOX *box, Datum interval);

/* Topological operators */

extern Datum Contains_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Contained_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overlaps_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Same_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Adjacent_stbox_stbox(PG_FUNCTION_ARGS);

extern bool contains_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool adjacent_stbox_stbox(const STBOX *box1, const STBOX *box2);

/* Position operators */

extern Datum Left_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overleft_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Right_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overright_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Below_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overbelow_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Above_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overabove_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Front_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overfront_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Back_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overback_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Before_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overbefore_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum After_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overafter_stbox_stbox(PG_FUNCTION_ARGS);

extern bool left_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overleft_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool right_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overright_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool below_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overbelow_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool above_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overabove_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool front_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overfront_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool back_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overback_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool before_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overbefore_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool after_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overafter_stbox_stbox(const STBOX *box1, const STBOX *box2);

/* Set operators */

extern Datum Union_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Intersection_stbox_stbox(PG_FUNCTION_ARGS);

/* Extent aggregation */

extern Datum Stbox_extent_transfn(PG_FUNCTION_ARGS);
extern Datum Stbox_extent_combinefn(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Stbox_cmp(PG_FUNCTION_ARGS);
extern Datum Stbox_eq(PG_FUNCTION_ARGS);
extern Datum Stbox_ne(PG_FUNCTION_ARGS);
extern Datum Stbox_lt(PG_FUNCTION_ARGS);
extern Datum Stbox_le(PG_FUNCTION_ARGS);
extern Datum Stbox_gt(PG_FUNCTION_ARGS);
extern Datum Stbox_ge(PG_FUNCTION_ARGS);

extern int stbox_cmp(const STBOX *box1, const STBOX *box2);
extern bool stbox_eq(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
