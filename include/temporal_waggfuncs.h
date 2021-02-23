/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file temporal_waggfuncs.c
 * Window temporal aggregate functions
 */

#ifndef __TEMPORAL_WAGGFUNCS_H__
#define __TEMPORAL_WAGGFUNCS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tint_wmin_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wmin_transfn(PG_FUNCTION_ARGS);
extern Datum tint_wmax_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wmax_transfn(PG_FUNCTION_ARGS);
extern Datum tint_wsum_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wsum_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_wcount_transfn(PG_FUNCTION_ARGS);
extern Datum tnumber_wavg_transfn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
