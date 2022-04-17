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

/*****************************************************************************/

/* Input/output functions */

extern Datum Tbox_in(PG_FUNCTION_ARGS);
extern Datum Tbox_out(PG_FUNCTION_ARGS);
extern Datum Tbox_send(PG_FUNCTION_ARGS);
extern Datum Tbox_recv(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum Tbox_constructor(PG_FUNCTION_ARGS);
extern Datum Tbox_constructor_t(PG_FUNCTION_ARGS);

/* Casting */

extern Datum Int_to_tbox(PG_FUNCTION_ARGS);
extern Datum Float_to_tbox(PG_FUNCTION_ARGS);
extern Datum Numeric_to_tbox(PG_FUNCTION_ARGS);
extern Datum Range_to_tbox(PG_FUNCTION_ARGS);
extern Datum Timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_tbox(PG_FUNCTION_ARGS);
extern Datum Period_to_tbox(PG_FUNCTION_ARGS);
extern Datum Periodset_to_tbox(PG_FUNCTION_ARGS);
extern Datum Int_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum Float_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum Int_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum Float_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum Range_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum Range_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum Tnumber_to_tbox(PG_FUNCTION_ARGS);

extern Datum Tbox_to_floatrange(PG_FUNCTION_ARGS);
extern Datum Tbox_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Tbox_hasx(PG_FUNCTION_ARGS);
extern Datum Tbox_hast(PG_FUNCTION_ARGS);
extern Datum Tbox_xmin(PG_FUNCTION_ARGS);
extern Datum Tbox_xmax(PG_FUNCTION_ARGS);
extern Datum Tbox_tmin(PG_FUNCTION_ARGS);
extern Datum Tbox_tmax(PG_FUNCTION_ARGS);

/* Transformation functions */

extern Datum Tbox_expand_value(PG_FUNCTION_ARGS);
extern Datum Tbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum Tbox_round(PG_FUNCTION_ARGS);

/* Topological functions */

extern Datum Contains_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Contained_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Overlaps_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Same_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Adjacent_tbox_tbox(PG_FUNCTION_ARGS);

/* Relative position functions */

extern Datum Left_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Overleft_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Right_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Overright_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Before_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Overbefore_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum After_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Overafter_tbox_tbox(PG_FUNCTION_ARGS);

/* Set functions */

extern Datum Union_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum Intersection_tbox_tbox(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Tbox_cmp(PG_FUNCTION_ARGS);
extern Datum Tbox_lt(PG_FUNCTION_ARGS);
extern Datum Tbox_le(PG_FUNCTION_ARGS);
extern Datum Tbox_gt(PG_FUNCTION_ARGS);
extern Datum Tbox_ge(PG_FUNCTION_ARGS);
extern Datum Tbox_eq(PG_FUNCTION_ARGS);
extern Datum Tbox_ne(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
