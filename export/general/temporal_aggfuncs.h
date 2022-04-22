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
 * @file temporal_aggfuncs.h
 * Temporal aggregate functions
 */

#ifndef __TEMPORAL_AGGFUNCS_H__
#define __TEMPORAL_AGGFUNCS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

extern Datum Temporal_extent_transfn(PG_FUNCTION_ARGS);
extern Datum Temporal_extent_combinefn(PG_FUNCTION_ARGS);
extern Datum Tnumber_extent_transfn(PG_FUNCTION_ARGS);
extern Datum Tnumber_extent_combinefn(PG_FUNCTION_ARGS);

extern Datum Tbool_tand_transfn(PG_FUNCTION_ARGS);
extern Datum Tbool_tand_combinefn(PG_FUNCTION_ARGS);
extern Datum Tbool_tor_transfn(PG_FUNCTION_ARGS);
extern Datum Tbool_tor_combinefn(PG_FUNCTION_ARGS);
extern Datum Tint_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum Tint_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum Tint_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum Tint_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum Tint_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum Tint_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum Tfloat_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum Temporal_tcount_transfn(PG_FUNCTION_ARGS);
extern Datum Temporal_tcount_combinefn(PG_FUNCTION_ARGS);
extern Datum Tnumber_tavg_transfn(PG_FUNCTION_ARGS);
extern Datum Tnumber_tavg_combinefn(PG_FUNCTION_ARGS);
extern Datum Temporal_tagg_finalfn(PG_FUNCTION_ARGS);
extern Datum Tnumber_tavg_finalfn(PG_FUNCTION_ARGS);
extern Datum Ttext_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum Ttext_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum Ttext_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum Ttext_tmax_combinefn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
