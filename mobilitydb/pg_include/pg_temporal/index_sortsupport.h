/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Sort support for the bottom-up GiST index build
 * @details PostgreSQL builds a GiST index by sorting the leaf entries whenever
 * the operator class supplies a `sortsupport` support function
 * (`GIST_SORTSUPPORT_PROC`, available since PostgreSQL 14); otherwise it
 * inserts the entries one at a time. The sort order decides how tightly the
 * resulting pages bound their entries, so it must place values that are close
 * in space AND in time next to each other.
 *
 * The key is a 64-bit index on a Hilbert curve, composed from the vendored
 * PostGIS primitives rather than computed by a curve of our own:
 * - the spatial half is #gbox_get_sortable_hash(), which is what PostGIS
 *   itself sorts a geometry column by,
 * - the temporal half is a rank of the period, and
 * - the two are combined with #uint32_hilbert(), the same primitive a second
 *   time, so that neither dimension leads the other.
 *
 * A value carrying only one of the two dimensions contributes zero for the
 * other, which keeps the order well defined for every box the types admit.
 */

#ifndef __PG_INDEX_SORTSUPPORT_H__
#define __PG_INDEX_SORTSUPPORT_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/sortsupport.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************/

/**
 * @brief Number of low bits dropped from a timestamp before it is ranked
 * @details A `TimestampTz` counts microseconds, a resolution far finer than an
 * index page can exploit. Dropping 24 bits makes the unit about 16.8 seconds,
 * which keeps roughly 1100 years on either side of the epoch inside the 32
 * bits the curve is given.
 */
#define MEOS_SORT_TIME_SHIFT 24

extern uint32 sortsupport_rank_int32(int32 value);
extern uint32 sortsupport_rank_int64(int64 value);
extern uint32 sortsupport_rank_double(double value);
extern uint32 sortsupport_rank_timestamptz(TimestampTz t);
extern uint32 sortsupport_rank_span_center(const Span *s);
extern uint64 sortsupport_hilbert(uint32 rank1, uint32 rank2);

extern uint64 stbox_sort_hash(const STBox *box);
extern uint64 tbox_sort_hash(const TBox *box);
extern uint64 span_sort_hash(const Span *s);

extern int sortsupport_abbrev_cmp(Datum x, Datum y, SortSupport ssup);
extern bool sortsupport_abbrev_abort(int memtupcount, SortSupport ssup);

/*****************************************************************************/

#endif
