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
 * @file time_ops.h
 * Operators for time types.
 */

#ifndef __TIME_OPS_H__
#define __TIME_OPS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

extern Datum Contains_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_period_period(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_period(PG_FUNCTION_ARGS);
extern Datum Contains_period_periodset(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_periodset(PG_FUNCTION_ARGS);

/* contained? */

extern Datum Contained_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Contained_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Contained_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_period_period(PG_FUNCTION_ARGS);
extern Datum Contained_period_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_periodset_period(PG_FUNCTION_ARGS);
extern Datum Contained_periodset_periodset(PG_FUNCTION_ARGS);

/* overlaps? */

extern Datum Overlaps_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_period(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_periodset(PG_FUNCTION_ARGS);

/* before */

extern Datum Before_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Before_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Before_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_period_period(PG_FUNCTION_ARGS);
extern Datum Before_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_periodset_period(PG_FUNCTION_ARGS);
extern Datum Before_period_periodset(PG_FUNCTION_ARGS);
extern Datum Before_periodset_periodset(PG_FUNCTION_ARGS);

/* after */

extern Datum After_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum After_timestamp_period(PG_FUNCTION_ARGS);
extern Datum After_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum After_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum After_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum After_timestampset_period(PG_FUNCTION_ARGS);
extern Datum After_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum After_period_timestamp(PG_FUNCTION_ARGS);
extern Datum After_period_period(PG_FUNCTION_ARGS);
extern Datum After_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum After_period_timestampset(PG_FUNCTION_ARGS);
extern Datum After_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum After_periodset_period(PG_FUNCTION_ARGS);
extern Datum After_period_periodset(PG_FUNCTION_ARGS);
extern Datum After_periodset_periodset(PG_FUNCTION_ARGS);

/* overbefore */

extern Datum Overbefore_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_periodset(PG_FUNCTION_ARGS);

/* overafter */

extern Datum Overafter_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Overafter_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_period_period(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overafter_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_periodset(PG_FUNCTION_ARGS);

/* adjacent */

extern Datum Adjacent_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_periodset(PG_FUNCTION_ARGS);

/* union */

extern Datum Union_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Union_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_period_period(PG_FUNCTION_ARGS);
extern Datum Union_period_periodset(PG_FUNCTION_ARGS);
extern Datum Union_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_periodset_period(PG_FUNCTION_ARGS);
extern Datum Union_periodset_periodset(PG_FUNCTION_ARGS);

/* intersection */

extern Datum Intersection_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_period_period(PG_FUNCTION_ARGS);
extern Datum Intersection_period_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_period(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_periodset(PG_FUNCTION_ARGS);

/* minus */

extern Datum Minus_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_period_period(PG_FUNCTION_ARGS);
extern Datum Minus_period_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_period(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_periodset(PG_FUNCTION_ARGS);

/* Distance returning an Interval */

extern Datum Distance_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_period_period(PG_FUNCTION_ARGS);
extern Datum Distance_period_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_period(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_periodset(PG_FUNCTION_ARGS);

/* Distance returning a float in seconds for use with indexes in
 * nearest neighbor searches */

extern Datum Distance_secs_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_periodset(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
