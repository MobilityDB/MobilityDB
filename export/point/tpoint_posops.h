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
 * @file tpoint_posops.h
 * Relative position operators for temporal geometry points.
 */

#ifndef __TPOINT_POSOPS_H__
#define __TPOINT_POSOPS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

extern Datum Left_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overleft_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Right_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overright_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Below_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overbelow_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Above_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overabove_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Front_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overfront_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Back_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum Overback_geom_tpoint(PG_FUNCTION_ARGS);

extern Datum Left_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overleft_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Right_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overright_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Above_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overabove_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Below_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overbelow_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Front_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overfront_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Back_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum Overback_tpoint_geom(PG_FUNCTION_ARGS);

extern Datum Left_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overleft_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Right_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overright_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Below_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overbelow_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Above_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overabove_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Front_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overfront_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Back_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overback_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Before_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overbefore_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum After_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum Overafter_stbox_tpoint(PG_FUNCTION_ARGS);

extern Datum Left_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overleft_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Right_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overright_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Above_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overabove_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Below_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overbelow_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Front_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overfront_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Back_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overback_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Before_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overbefore_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum After_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum Overafter_tpoint_stbox(PG_FUNCTION_ARGS);

extern Datum Left_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overleft_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Right_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overright_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Above_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overabove_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Below_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overbelow_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Front_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overfront_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Back_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overback_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Before_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overbefore_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum After_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum Overafter_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
