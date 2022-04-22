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
 * @file timestampset.h
 * Basic functions for set of (distinct) timestamps.
 */

#ifndef __TIMESTAMPSET_H__
#define __TIMESTAMPSET_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/* Input/output functions */

extern Datum Timestampset_in(PG_FUNCTION_ARGS);
extern Datum Timestampset_out(PG_FUNCTION_ARGS);
extern Datum Timestampset_send(PG_FUNCTION_ARGS);
extern Datum Timestampset_recv(PG_FUNCTION_ARGS);

/* Constructor function */

extern Datum Timestampset_constructor(PG_FUNCTION_ARGS);

/* Cast function */

extern Datum Timestamp_to_timestampset(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Timestampset_mem_size(PG_FUNCTION_ARGS);
extern Datum Timestampset_timespan(PG_FUNCTION_ARGS);
extern Datum Timestampset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum Timestampset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum Timestampset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum Timestampset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum Timestampset_timestamps(PG_FUNCTION_ARGS);

/* Modification functions */

extern Datum Timestampset_shift(PG_FUNCTION_ARGS);
extern Datum Timestampset_tscale(PG_FUNCTION_ARGS);
extern Datum Timestampset_shift_tscale(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Timestampset_cmp(PG_FUNCTION_ARGS);
extern Datum Timestampset_eq(PG_FUNCTION_ARGS);
extern Datum Timestampset_ne(PG_FUNCTION_ARGS);
extern Datum Timestampset_lt(PG_FUNCTION_ARGS);
extern Datum Timestampset_le(PG_FUNCTION_ARGS);
extern Datum Timestampset_ge(PG_FUNCTION_ARGS);
extern Datum Timestampset_gt(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Timestampset_hash(PG_FUNCTION_ARGS);
extern Datum Timestampset_hash_extended(PG_FUNCTION_ARGS);

#endif

/*****************************************************************************/
