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
 * @file tnpoint_static.h
 * Network-based static point/segments
 */

#ifndef __TNPOINT_STATIC_H__
#define __TNPOINT_STATIC_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/* Input/Output functions for npoint */

extern Datum Npoint_in(PG_FUNCTION_ARGS);
extern Datum Npoint_out(PG_FUNCTION_ARGS);
extern Datum Npoint_recv(PG_FUNCTION_ARGS);
extern Datum Npoint_send(PG_FUNCTION_ARGS);

/* Input/Output functions for nsegment */

extern Datum Nsegment_in(PG_FUNCTION_ARGS);
extern Datum Nsegment_out(PG_FUNCTION_ARGS);
extern Datum Nsegment_recv(PG_FUNCTION_ARGS);
extern Datum Nsegment_send(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum Npoint_constructor(PG_FUNCTION_ARGS);
extern Datum Nsegment_constructor(PG_FUNCTION_ARGS);
extern Datum Npoint_to_nsegment(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Npoint_route(PG_FUNCTION_ARGS);
extern Datum Npoint_position(PG_FUNCTION_ARGS);
extern Datum Nsegment_route(PG_FUNCTION_ARGS);
extern Datum Nsegment_start_position(PG_FUNCTION_ARGS);
extern Datum Nsegment_end_position(PG_FUNCTION_ARGS);

/* Transformation functions */

extern Datum Npoint_round(PG_FUNCTION_ARGS);
extern Datum Nsegment_round(PG_FUNCTION_ARGS);

/* Conversions between network and Euclidean space */

extern Datum Npoint_to_geom(PG_FUNCTION_ARGS);
extern Datum Geom_to_npoint(PG_FUNCTION_ARGS);
extern Datum Nsegment_to_geom(PG_FUNCTION_ARGS);
extern Datum Geom_to_nsegment(PG_FUNCTION_ARGS);

/* SRID functions */

extern Datum Npoint_get_srid(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Npoint_eq(PG_FUNCTION_ARGS);
extern Datum Npoint_ne(PG_FUNCTION_ARGS);
extern Datum Npoint_lt(PG_FUNCTION_ARGS);
extern Datum Npoint_le(PG_FUNCTION_ARGS);
extern Datum Npoint_gt(PG_FUNCTION_ARGS);
extern Datum Npoint_ge(PG_FUNCTION_ARGS);

extern Datum Nsegment_eq(PG_FUNCTION_ARGS);
extern Datum Nsegment_ne(PG_FUNCTION_ARGS);
extern Datum Nsegment_lt(PG_FUNCTION_ARGS);
extern Datum Nsegment_le(PG_FUNCTION_ARGS);
extern Datum Nsegment_gt(PG_FUNCTION_ARGS);
extern Datum Nsegment_ge(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif /* __TNPOINT_STATIC_H__ */
