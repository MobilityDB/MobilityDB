/*****************************************************************************
 *
 * temporal_posops.h
 *    Relative position operators for temporal types.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#ifndef __TEMPORAL_POSOPS_H__
#define __TEMPORAL_POSOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "temporal.h"

/*****************************************************************************/

extern Datum before_period_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_period_temporal(PG_FUNCTION_ARGS);
extern Datum after_period_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_period_temporal(PG_FUNCTION_ARGS);

extern Datum before_temporal_period(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_period(PG_FUNCTION_ARGS);
extern Datum after_temporal_period(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_period(PG_FUNCTION_ARGS);

extern Datum before_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum after_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************/

extern Datum left_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_range_tnumber(PG_FUNCTION_ARGS);
extern Datum right_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_range_tnumber(PG_FUNCTION_ARGS);

extern Datum left_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_range(PG_FUNCTION_ARGS);
extern Datum right_tnumber_range(PG_FUNCTION_ARGS);

extern Datum left_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum right_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum before_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overbefore_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum after_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overafter_tbox_tnumber(PG_FUNCTION_ARGS);

extern Datum left_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum right_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum before_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum after_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overafter_tnumber_tbox(PG_FUNCTION_ARGS);

extern Datum left_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum right_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum before_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overbefore_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum after_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overafter_tnumber_tnumber(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
