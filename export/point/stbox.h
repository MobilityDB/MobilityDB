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

/*****************************************************************************/

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

/* Topological operators */

extern Datum Contains_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Contained_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Overlaps_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Same_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum Adjacent_stbox_stbox(PG_FUNCTION_ARGS);

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

/*****************************************************************************/

#endif
