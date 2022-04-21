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
 * @file tpoint_spatialfuncs.h
 * Spatial functions for temporal points.
 */

#ifndef __TPOINT_SPATIALFUNCS_H__
#define __TPOINT_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/* Ever equal comparison operator */

extern Datum Tpoint_ever_eq(PG_FUNCTION_ARGS);
extern Datum Tpoint_always_eq(PG_FUNCTION_ARGS);
extern Datum Tpoint_ever_ne(PG_FUNCTION_ARGS);
extern Datum Tpoint_always_ne(PG_FUNCTION_ARGS);

/* Trajectory functions */

extern Datum Tpoint_get_trajectory(PG_FUNCTION_ARGS);

/* Functions for spatial reference systems */

extern Datum Tpoint_get_srid(PG_FUNCTION_ARGS);
extern Datum Tpoint_set_srid(PG_FUNCTION_ARGS);
extern Datum Tpoint_transform(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS);
extern Datum Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS);

/* Set precision of the coordinates */

extern Datum Geo_round(PG_FUNCTION_ARGS);
extern Datum Tpoint_round(PG_FUNCTION_ARGS);

/* Functions for extracting coordinates */

extern Datum Tpoint_get_x(PG_FUNCTION_ARGS);
extern Datum Tpoint_get_y(PG_FUNCTION_ARGS);
extern Datum Tpoint_get_z(PG_FUNCTION_ARGS);

/* Length, speed, time-weighted centroid, temporal azimuth, and
 * temporal bearing functions */

extern Datum Tpoint_length(PG_FUNCTION_ARGS);
extern Datum Tpoint_cumulative_length(PG_FUNCTION_ARGS);
extern Datum Tpoint_speed(PG_FUNCTION_ARGS);
extern Datum Tpoint_twcentroid(PG_FUNCTION_ARGS);
extern Datum Tpoint_azimuth(PG_FUNCTION_ARGS);

extern Datum Bearing_geo_geo(PG_FUNCTION_ARGS);
extern Datum Bearing_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum Bearing_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum Bearing_tpoint_tpoint(PG_FUNCTION_ARGS);

/* Non self-intersecting (a.k.a. simple) functions */

extern Datum Tpoint_is_simple(PG_FUNCTION_ARGS);
extern Datum Tpoint_make_simple(PG_FUNCTION_ARGS);

/* Restriction functions */

extern Datum Tpoint_at_geometry(PG_FUNCTION_ARGS);
extern Datum Tpoint_at_stbox(PG_FUNCTION_ARGS);
extern Datum Tpoint_minus_geometry(PG_FUNCTION_ARGS);
extern Datum Tpoint_minus_stbox(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
