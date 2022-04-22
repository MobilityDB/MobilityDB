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
 * @file periodset.c
 * Basic functions for set of disjoint periods.
 */

#ifndef __PERIODSET_H__
#define __PERIODSET_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/* Input/output functions */

extern Datum Periodset_in(PG_FUNCTION_ARGS);
extern Datum Periodset_out(PG_FUNCTION_ARGS);
extern Datum Periodset_send(PG_FUNCTION_ARGS);
extern Datum Periodset_recv(PG_FUNCTION_ARGS);

/* Constructor function */

extern Datum Periodset_constructor(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum Timestamp_to_periodset(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_periodset(PG_FUNCTION_ARGS);
extern Datum Period_to_periodset(PG_FUNCTION_ARGS);
extern Datum Periodset_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Periodset_mem_size(PG_FUNCTION_ARGS);
extern Datum Periodset_timespan(PG_FUNCTION_ARGS);
extern Datum Periodset_duration(PG_FUNCTION_ARGS);
extern Datum Periodset_num_periods(PG_FUNCTION_ARGS);
extern Datum Periodset_start_period(PG_FUNCTION_ARGS);
extern Datum Periodset_end_period(PG_FUNCTION_ARGS);
extern Datum Periodset_period_n(PG_FUNCTION_ARGS);
extern Datum Periodset_periods(PG_FUNCTION_ARGS);
extern Datum Periodset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum Periodset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum Periodset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum Periodset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum Periodset_timestamps(PG_FUNCTION_ARGS);

/* Modification functions */

extern Datum Periodset_shift(PG_FUNCTION_ARGS);
extern Datum Periodset_tscale(PG_FUNCTION_ARGS);
extern Datum Periodset_shift_tscale(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Periodset_cmp(PG_FUNCTION_ARGS);
extern Datum Periodset_eq(PG_FUNCTION_ARGS);
extern Datum Periodset_ne(PG_FUNCTION_ARGS);
extern Datum Periodset_lt(PG_FUNCTION_ARGS);
extern Datum Periodset_le(PG_FUNCTION_ARGS);
extern Datum Periodset_ge(PG_FUNCTION_ARGS);
extern Datum Periodset_gt(PG_FUNCTION_ARGS);

/* Hash functions */

extern Datum Periodset_hash(PG_FUNCTION_ARGS);
extern Datum Periodset_hash_extended(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
