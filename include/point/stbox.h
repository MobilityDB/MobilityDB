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
#include <libpq/pqformat.h>
/* PostGIS */
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

extern char *stbox_to_string(const STBOX *box);
extern void stbox_write(const STBOX *box, StringInfo buf);
extern STBOX *stbox_read(StringInfo buf);

/* Constructor functions */


/* Casting */

extern void stbox_gbox(const STBOX *box, GBOX * gbox);
extern void stbox_box3d(const STBOX *box, BOX3D *box3d);
extern Datum stbox_geometry(const STBOX *box);

/* Transform a <Type> to a STBOX */

extern bool geo_stbox(const GSERIALIZED *gs, STBOX *box);
extern void timestamp_stbox(TimestampTz t, STBOX *box);
extern void timestampset_stbox(const TimestampSet *ps, STBOX *box);
extern void timestampset_stbox_slice(Datum tsdatum, STBOX *box);
extern void period_stbox(const Period *p, STBOX *box);
extern void periodset_stbox(const PeriodSet *ps, STBOX *box);
extern void periodset_stbox_slice(Datum psdatum, STBOX *box);
extern STBOX *geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t);
extern STBOX *geo_period_to_stbox(const GSERIALIZED *gs, const Period *p);

/* Accessor functions */

extern bool stbox_hasx(const STBOX *box);
extern bool stbox_hasz(const STBOX *box);
extern bool stbox_hast(const STBOX *box);
extern bool stbox_isgeodetic(const STBOX *box);
extern bool stbox_xmin(const STBOX *box, double *result);
extern bool stbox_xmax(const STBOX *box, double *result);
extern bool stbox_ymin(const STBOX *box, double *result);
extern bool stbox_ymax(const STBOX *box, double *result);
extern bool stbox_zmin(const STBOX *box, double *result);
extern bool stbox_zmax(const STBOX *box, double *result);
extern bool stbox_tmin(const STBOX *box, TimestampTz *result);
extern bool stbox_tmax(const STBOX *box, TimestampTz *result);

/* SRID functions */

extern int32 stbox_get_srid(const STBOX *box);
extern STBOX * stbox_set_srid(const STBOX *box, int32 srid);
extern STBOX * stbox_transform(const STBOX *box, int32 srid);

/* Transformation functions */

extern STBOX *stbox_expand_spatial(const STBOX *box, double d);
extern STBOX *stbox_expand_temporal(const STBOX *box, Datum interval);
extern STBOX *stbox_round(const STBOX *box, Datum prec);

/* Topological operators */

extern bool contains_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool adjacent_stbox_stbox(const STBOX *box1, const STBOX *box2);

/* Position operators */

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

extern STBOX *union_stbox_stbox(const STBOX *box1, const STBOX *box2,
  bool strict);
extern STBOX *intersection_stbox_stbox(const STBOX *box1, const STBOX *box2);
extern bool inter_stbox_stbox(const STBOX *box1, const STBOX *box2,
  STBOX *result);

/* Comparison functions */

extern bool stbox_eq(const STBOX *box1, const STBOX *box2);
extern bool stbox_ne(const STBOX *box1, const STBOX *box2);
extern int stbox_cmp(const STBOX *box1, const STBOX *box2);
extern bool stbox_lt(const STBOX *box1, const STBOX *box2);
extern bool stbox_le(const STBOX *box1, const STBOX *box2);
extern bool stbox_gt(const STBOX *box1, const STBOX *box2);
extern bool stbox_ge(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
