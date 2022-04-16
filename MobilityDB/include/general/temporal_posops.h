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
 * @file temporal_posops.h
 * Relative position operators for temporal types.
 */

#ifndef __TEMPORAL_POSOPS_H__
#define __TEMPORAL_POSOPS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

extern Datum Before_period_temporal(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_temporal(PG_FUNCTION_ARGS);
extern Datum After_period_temporal(PG_FUNCTION_ARGS);
extern Datum Overafter_period_temporal(PG_FUNCTION_ARGS);

extern Datum Before_temporal_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_temporal_period(PG_FUNCTION_ARGS);
extern Datum After_temporal_period(PG_FUNCTION_ARGS);
extern Datum Overafter_temporal_period(PG_FUNCTION_ARGS);

extern Datum Before_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum Overbefore_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum After_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum Overafter_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************/

extern Datum Left_range_tnumber(PG_FUNCTION_ARGS);
extern Datum Overleft_range_tnumber(PG_FUNCTION_ARGS);
extern Datum Right_range_tnumber(PG_FUNCTION_ARGS);
extern Datum Overright_range_tnumber(PG_FUNCTION_ARGS);

extern Datum Left_tnumber_range(PG_FUNCTION_ARGS);
extern Datum Overleft_tnumber_range(PG_FUNCTION_ARGS);
extern Datum Overright_tnumber_range(PG_FUNCTION_ARGS);
extern Datum Right_tnumber_range(PG_FUNCTION_ARGS);

extern Datum Left_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Overleft_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Right_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Overright_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Before_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Overbefore_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum After_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum Overafter_tbox_tnumber(PG_FUNCTION_ARGS);

extern Datum Left_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Overleft_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Right_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Overright_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Before_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Overbefore_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum After_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum Overafter_tnumber_tbox(PG_FUNCTION_ARGS);

extern Datum Left_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Overleft_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Right_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Overright_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Before_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum After_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum Overafter_tnumber_tnumber(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
