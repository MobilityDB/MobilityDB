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

#include <postgres.h>
#include <catalog/pg_type.h>
#include "general/temporal.h"

/*****************************************************************************/

extern Datum left_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum right_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum below_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum above_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum front_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum back_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_geom_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum right_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum above_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum below_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum front_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum back_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_geom(PG_FUNCTION_ARGS);

extern Datum left_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum right_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum below_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum above_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum front_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum back_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum before_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbefore_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum after_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overafter_stbox_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum right_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum above_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum below_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum front_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum back_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum before_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum after_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overafter_tpoint_stbox(PG_FUNCTION_ARGS);

extern Datum left_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum right_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum above_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum below_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum front_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum back_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum before_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overbefore_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum after_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overafter_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
