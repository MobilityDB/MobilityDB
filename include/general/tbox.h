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
 * @file tbox.h
 * Functions for temporal bounding boxes.
 */

#ifndef __TBOX_H__
#define __TBOX_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <libpq/pqformat.h>
#include <utils/rangetypes.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timetypes.h"

/*****************************************************************************/

/**
 * Structure to represent temporal boxes
 */
typedef struct
{
  double      xmin;   /**< minimum number value */
  double      xmax;   /**< maximum number value */
  TimestampTz tmin;   /**< minimum timestamp */
  TimestampTz tmax;   /**< maximum timestamp */
  int16       flags;  /**< flags */
} TBOX;

/* fmgr macros temporal types */

#define DatumGetTboxP(X)    ((TBOX *) DatumGetPointer(X))
#define TboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_TBOX_P(n) DatumGetTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_TBOX_P(x) return TboxPGetDatum(x)

/*****************************************************************************/

/* General functions */

extern TBOX *tbox_make(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax);
extern void tbox_set(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax, TBOX *box);
extern TBOX *tbox_copy(const TBOX *box);
extern void tbox_expand(const TBOX *box1, TBOX *box2);
extern void tbox_shift_tscale(const Interval *start, const Interval *duration,
  TBOX *box);
extern bool inter_tbox_tbox(const TBOX *box1, const TBOX *box2, TBOX *result);

/* Parameter tests */

extern void ensure_has_X_tbox(const TBOX *box);
extern void ensure_has_T_tbox(const TBOX *box);
extern void ensure_same_dimensionality_tbox(const TBOX *box1, const TBOX *box2);

/* Input/output functions */

extern char *tbox_to_string(const TBOX *box);
extern void tbox_write(const TBOX *box, StringInfo buf);
extern TBOX *tbox_read(StringInfo buf);

/* Constructor functions */


/* Casting */

extern void number_tbox(Datum value, CachedType basetype, TBOX *box);
extern void int_tbox(int i, TBOX *box);
extern void float_tbox(double d, TBOX *box);
extern void range_tbox(const RangeType *r, TBOX *box);
extern void timestamp_tbox(TimestampTz t, TBOX *box);
extern void timestampset_tbox(const TimestampSet *ts, TBOX *box);
extern void timestampset_tbox_slice(Datum tsdatum, TBOX *box);
extern void period_tbox(const Period *p, TBOX *box);
extern void periodset_tbox(const PeriodSet *ps, TBOX *box);
extern void periodset_tbox_slice(Datum psdatum, TBOX *box);

extern TBOX *int_timestamp_to_tbox(int i, TimestampTz t);
extern TBOX *float_timestamp_to_tbox(double d, TimestampTz t);
extern TBOX *int_period_to_tbox(int i, Period *p);
extern TBOX *float_period_to_tbox(double d, Period *p);
extern TBOX *range_timestamp_to_tbox(RangeType *range, TimestampTz t);
extern TBOX *range_period_to_tbox(RangeType *range, Period *p);
extern RangeType *tbox_to_floatrange(TBOX *box);
extern Period *tbox_to_period(TBOX *box);

/* Accessor functions */

extern bool tbox_hasx(const TBOX *box);
extern bool tbox_hast(const TBOX *box);
extern bool tbox_xmin(const TBOX *box, double *result);
extern bool tbox_xmax(const TBOX *box, double *result);
extern bool tbox_tmin(const TBOX *box, TimestampTz *result);
extern bool tbox_tmax(const TBOX *box, TimestampTz *result);

/* Transformation functions */

extern TBOX *tbox_expand_value(const TBOX *box, const double d);
extern TBOX *tbox_expand_temporal(const TBOX *box, const Interval *interval);
extern TBOX *tbox_round(const TBOX *box, int size);

/* Topological functions */

extern bool contains_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool contained_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overlaps_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool same_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool adjacent_tbox_tbox(const TBOX *box1, const TBOX *box2);

/* Relative position functions */

extern bool left_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overleft_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool right_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overright_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool before_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overbefore_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool after_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern bool overafter_tbox_tbox(const TBOX *box1, const TBOX *box2);

/* Set functions */

extern TBOX *union_tbox_tbox(const TBOX *box1, const TBOX *box2);

/* Comparison functions */

extern int tbox_cmp(const TBOX *box1, const TBOX *box2);
extern bool tbox_eq(const TBOX *box1, const TBOX *box2);

/*****************************************************************************/

#endif
