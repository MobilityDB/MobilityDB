/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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

#include <postgres.h>
#include <catalog/pg_type.h>
#if POSTGRESQL_VERSION_NUMBER < 110000
#include <utils/timestamp.h>
#endif
#include <liblwgeom.h>

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

/* Miscellaneous functions */

extern STBOX *stbox_make(bool hasx, bool hasz, bool hast, bool geodetic,
  int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, TimestampTz tmin, TimestampTz tmax);
extern void stbox_set(STBOX *box, bool hasx, bool hasz, bool hast, bool geodetic,
  int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, TimestampTz tmin, TimestampTz tmax);
extern STBOX *stbox_copy(const STBOX *box);
extern void stbox_expand(STBOX *box1, const STBOX *box2);
extern void stbox_shift_tscale(STBOX *box, const Interval *start,
  const Interval *duration);

/* Parameter tests */

extern void ensure_has_X_stbox(const STBOX *box);
extern void ensure_has_T_stbox(const STBOX *box);

/* Input/Ouput functions */

extern Datum stbox_in(PG_FUNCTION_ARGS);
extern Datum stbox_out(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum stbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum stbox_constructor(PG_FUNCTION_ARGS);
extern Datum stbox_constructor_z(PG_FUNCTION_ARGS);
extern Datum stbox_constructor_zt(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_z(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_zt(PG_FUNCTION_ARGS);

/* Casting */

extern Datum stbox_to_period(PG_FUNCTION_ARGS);
extern Datum stbox_to_box2d(PG_FUNCTION_ARGS);
extern Datum stbox_to_box3d(PG_FUNCTION_ARGS);

extern void stbox_set_box3d(const STBOX *box, BOX3D *box3d);
extern void stbox_set_gbox(const STBOX *box, GBOX * gbox);

/* Transform a <Type> to a STBOX */

extern Datum box2d_to_stbox(PG_FUNCTION_ARGS);
extern Datum box3d_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_to_stbox(PG_FUNCTION_ARGS);
extern Datum timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum timestampset_to_stbox(PG_FUNCTION_ARGS);
extern Datum period_to_stbox(PG_FUNCTION_ARGS);
extern Datum periodset_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_period_to_stbox(PG_FUNCTION_ARGS);

extern bool geo_to_stbox_internal(STBOX *box, const GSERIALIZED *gs);
extern void timestamp_to_stbox_internal(STBOX *box, TimestampTz t);
extern void timestampset_to_stbox_internal(STBOX *box, const TimestampSet *ps);
extern void period_to_stbox_internal(STBOX *box, const Period *p);
extern void periodset_to_stbox_internal(STBOX *box, const PeriodSet *ps);

/* Accessor functions */

extern Datum stbox_hasx(PG_FUNCTION_ARGS);
extern Datum stbox_hasz(PG_FUNCTION_ARGS);
extern Datum stbox_hast(PG_FUNCTION_ARGS);
extern Datum stbox_geodetic(PG_FUNCTION_ARGS);
extern Datum stbox_xmin(PG_FUNCTION_ARGS);
extern Datum stbox_xmax(PG_FUNCTION_ARGS);
extern Datum stbox_ymin(PG_FUNCTION_ARGS);
extern Datum stbox_ymax(PG_FUNCTION_ARGS);
extern Datum stbox_zmin(PG_FUNCTION_ARGS);
extern Datum stbox_zmax(PG_FUNCTION_ARGS);
extern Datum stbox_tmin(PG_FUNCTION_ARGS);
extern Datum stbox_tmax(PG_FUNCTION_ARGS);

/* SRID functions */

extern Datum stbox_srid(PG_FUNCTION_ARGS);
extern Datum stbox_set_srid(PG_FUNCTION_ARGS);
extern Datum stbox_transform(PG_FUNCTION_ARGS);

/* Transformation functions */

extern Datum stbox_expand_spatial(PG_FUNCTION_ARGS);
extern Datum stbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum stbox_set_precision(PG_FUNCTION_ARGS);

extern STBOX *stbox_expand_spatial_internal(STBOX *box, double d);
extern STBOX *stbox_expand_temporal_internal(STBOX *box, Datum interval);

/* Topological operators */

extern Datum contains_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum contained_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overlaps_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum same_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum adjacent_stbox_stbox(PG_FUNCTION_ARGS);

extern bool contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool adjacent_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);

/* Position operators */

extern Datum left_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overleft_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum right_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overright_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum below_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbelow_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum above_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overabove_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum front_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overfront_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum back_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overback_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum before_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbefore_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum after_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overafter_stbox_stbox(PG_FUNCTION_ARGS);

extern bool left_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overleft_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool right_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overright_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool below_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overbelow_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool above_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overabove_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool front_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overfront_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool back_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overback_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool before_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overbefore_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool after_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overafter_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);

/* Set operators */

extern Datum stbox_union(PG_FUNCTION_ARGS);
extern Datum stbox_intersection(PG_FUNCTION_ARGS);

extern STBOX *stbox_union_internal(const STBOX *box1, const STBOX *box2,
  bool strict);
extern STBOX *stbox_intersection_internal(const STBOX *box1, const STBOX *box2);

/* Comparison functions */

extern Datum stbox_cmp(PG_FUNCTION_ARGS);
extern Datum stbox_eq(PG_FUNCTION_ARGS);
extern Datum stbox_ne(PG_FUNCTION_ARGS);
extern Datum stbox_lt(PG_FUNCTION_ARGS);
extern Datum stbox_le(PG_FUNCTION_ARGS);
extern Datum stbox_gt(PG_FUNCTION_ARGS);
extern Datum stbox_ge(PG_FUNCTION_ARGS);

extern int stbox_cmp_internal(const STBOX *box1, const STBOX *box2);
extern bool stbox_eq_internal(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
