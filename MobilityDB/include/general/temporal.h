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
 * @file temporal.h
 * Basic functions for temporal types of any subtype.
 */

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/* Typmod functions */

extern Datum Temporal_typmod_in(PG_FUNCTION_ARGS);
extern Datum Temporal_typmod_out(PG_FUNCTION_ARGS);
extern Datum Temporal_enforce_typmod(PG_FUNCTION_ARGS);

/* Version functions */

extern Datum Mobilitydb_version(PG_FUNCTION_ARGS);
extern Datum Mobilitydb_full_version(PG_FUNCTION_ARGS);

/* Input/output functions */

extern Datum Temporal_in(PG_FUNCTION_ARGS);
extern Datum Temporal_out(PG_FUNCTION_ARGS);
extern Datum Temporal_send(PG_FUNCTION_ARGS);
extern Datum Temporal_recv(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum Tinstant_constructor(PG_FUNCTION_ARGS);
extern Datum Tinstantset_constructor(PG_FUNCTION_ARGS);
extern Datum Tlinearseq_constructor(PG_FUNCTION_ARGS);
extern Datum Tstepseq_constructor(PG_FUNCTION_ARGS);
extern Datum Tsequenceset_constructor(PG_FUNCTION_ARGS);
extern Datum Tstepseqset_constructor_gaps(PG_FUNCTION_ARGS);
extern Datum Tlinearseqset_constructor_gaps(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern Datum Temporal_append_tinstant(PG_FUNCTION_ARGS);
extern Datum Temporal_merge(PG_FUNCTION_ARGS);
extern Datum Temporal_merge_array(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum Tint_to_range(PG_FUNCTION_ARGS);
extern Datum Tfloat_to_range(PG_FUNCTION_ARGS);
extern Datum Tint_to_tfloat(PG_FUNCTION_ARGS);
extern Datum Tfloat_to_tint(PG_FUNCTION_ARGS);
extern Datum Temporal_to_period(PG_FUNCTION_ARGS);

extern Datum Tinstantset_from_base(PG_FUNCTION_ARGS);
extern Datum Tsequence_from_base(PG_FUNCTION_ARGS);
extern Datum Tsequenceset_from_base(PG_FUNCTION_ARGS);

/* Transformation functions */

extern Datum Temporal_to_tinstant(PG_FUNCTION_ARGS);
extern Datum Temporal_to_tinstantset(PG_FUNCTION_ARGS);
extern Datum Temporal_to_tsequence(PG_FUNCTION_ARGS);
extern Datum Temporal_to_tsequenceset(PG_FUNCTION_ARGS);
extern Datum Tempstep_to_templinear(PG_FUNCTION_ARGS);
extern Datum Temporal_shift(PG_FUNCTION_ARGS);
extern Datum Temporal_tscale(PG_FUNCTION_ARGS);
extern Datum Temporal_shift_tscale(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Temporal_subtype(PG_FUNCTION_ARGS);
extern Datum Temporal_interpolation(PG_FUNCTION_ARGS);
extern Datum Temporal_memory_size(PG_FUNCTION_ARGS);
extern Datum Temporal_values(PG_FUNCTION_ARGS);
extern Datum Tfloat_ranges(PG_FUNCTION_ARGS);
extern Datum Tinstant_get_value(PG_FUNCTION_ARGS);
extern Datum Temporal_time(PG_FUNCTION_ARGS);
extern Datum Tinstant_timestamp(PG_FUNCTION_ARGS);
extern Datum Tnumber_range(PG_FUNCTION_ARGS);
extern Datum Temporal_start_value(PG_FUNCTION_ARGS);
extern Datum Temporal_end_value(PG_FUNCTION_ARGS);
extern Datum Temporal_min_value(PG_FUNCTION_ARGS);
extern Datum Temporal_max_value(PG_FUNCTION_ARGS);
extern Datum Temporal_min_instant(PG_FUNCTION_ARGS);
extern Datum Temporal_max_instant(PG_FUNCTION_ARGS);
extern Datum Temporal_timespan(PG_FUNCTION_ARGS);
extern Datum Temporal_duration(PG_FUNCTION_ARGS);
extern Datum Temporal_num_sequences(PG_FUNCTION_ARGS);
extern Datum Temporal_start_sequence(PG_FUNCTION_ARGS);
extern Datum Temporal_end_sequence(PG_FUNCTION_ARGS);
extern Datum Temporal_sequence_n(PG_FUNCTION_ARGS);
extern Datum Temporal_sequences(PG_FUNCTION_ARGS);
extern Datum Temporal_segments(PG_FUNCTION_ARGS);
extern Datum Temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum Temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum Temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum Temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum Temporal_instants(PG_FUNCTION_ARGS);
extern Datum Temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum Temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum Temporal_timestamps(PG_FUNCTION_ARGS);

/* Ever/always equal operators */

extern Datum Temporal_ever_eq(PG_FUNCTION_ARGS);
extern Datum Temporal_always_eq(PG_FUNCTION_ARGS);
extern Datum Temporal_ever_ne(PG_FUNCTION_ARGS);
extern Datum Temporal_always_ne(PG_FUNCTION_ARGS);

extern Datum Temporal_ever_lt(PG_FUNCTION_ARGS);
extern Datum Temporal_ever_le(PG_FUNCTION_ARGS);
extern Datum Temporal_ever_gt(PG_FUNCTION_ARGS);
extern Datum Temporal_ever_ge(PG_FUNCTION_ARGS);
extern Datum Temporal_always_lt(PG_FUNCTION_ARGS);
extern Datum Temporal_always_le(PG_FUNCTION_ARGS);
extern Datum Temporal_always_gt(PG_FUNCTION_ARGS);
extern Datum Temporal_always_ge(PG_FUNCTION_ARGS);

/* Restriction functions */

extern Datum Temporal_at_value(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_value(PG_FUNCTION_ARGS);
extern Datum Temporal_at_values(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_values(PG_FUNCTION_ARGS);
extern Datum Tnumber_at_range(PG_FUNCTION_ARGS);
extern Datum Tnumber_minus_range(PG_FUNCTION_ARGS);
extern Datum Tnumber_at_ranges(PG_FUNCTION_ARGS);
extern Datum Tnumber_minus_ranges(PG_FUNCTION_ARGS);
extern Datum Temporal_at_min(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_min(PG_FUNCTION_ARGS);
extern Datum Temporal_at_max(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_max(PG_FUNCTION_ARGS);
extern Datum Temporal_at_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_value_at_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_at_timestampset(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_timestampset(PG_FUNCTION_ARGS);
extern Datum Temporal_at_period(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_period(PG_FUNCTION_ARGS);
extern Datum Temporal_at_periodset(PG_FUNCTION_ARGS);
extern Datum Temporal_minus_periodset(PG_FUNCTION_ARGS);
extern Datum Tnumber_at_tbox(PG_FUNCTION_ARGS);
extern Datum Tnumber_minus_tbox(PG_FUNCTION_ARGS);

/* Intersects functions */

extern Datum Temporal_intersects_timestamp(PG_FUNCTION_ARGS);
extern Datum Temporal_intersects_timestampset(PG_FUNCTION_ARGS);
extern Datum Temporal_intersects_period(PG_FUNCTION_ARGS);
extern Datum Temporal_intersects_periodset(PG_FUNCTION_ARGS);

/* Local aggregate functions */

extern Datum Tnumber_integral(PG_FUNCTION_ARGS);
extern Datum Tnumber_twavg(PG_FUNCTION_ARGS);

/* Comparison functions */

extern Datum Temporal_eq(PG_FUNCTION_ARGS);
extern Datum Temporal_ne(PG_FUNCTION_ARGS);

extern Datum Temporal_cmp(PG_FUNCTION_ARGS);
extern Datum Temporal_lt(PG_FUNCTION_ARGS);
extern Datum Temporal_le(PG_FUNCTION_ARGS);
extern Datum Temporal_ge(PG_FUNCTION_ARGS);
extern Datum Temporal_gt(PG_FUNCTION_ARGS);

/* Functions for defining hash index */

extern Datum Temporal_hash(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
