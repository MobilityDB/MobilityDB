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

/* Parameter tests */

extern void ensure_has_X_stbox(const STBOX *box);
extern void ensure_has_T_stbox(const STBOX *box);

/* Set an STBOX from a <Type> */

extern void timestampset_stbox_slice(Datum tsdatum, STBOX *box);
extern void periodset_stbox_slice(Datum psdatum, STBOX *box);

/* SRID functions */

extern STBOX * stbox_set_srid(const STBOX *box, int32 srid);
extern STBOX * stbox_transform(const STBOX *box, int32 srid);

/* Transformation functions */

extern STBOX *stbox_round(const STBOX *box, Datum prec);

/*****************************************************************************/

#endif
