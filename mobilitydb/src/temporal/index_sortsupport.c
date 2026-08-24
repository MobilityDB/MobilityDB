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
 * @file
 * @brief Sort support for the bottom-up GiST index build
 */

#include "pg_temporal/index_sortsupport.h"

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/tbox.h"

/*****************************************************************************
 * Ranking functions
 *
 * Each of them maps a value to a 32-bit rank that increases with it, so that
 * the curve below receives one comparable coordinate per dimension.
 *****************************************************************************/

/**
 * @brief Return the bit pattern of a double as an integer that increases with
 * the value
 * @details The bit pattern of a non-negative IEEE 754 double already increases
 * with the value, while that of a negative one decreases. Flipping the sign
 * bit of the first and every bit of the second gives an integer ordered as the
 * double is, over the whole range and without any assumption about it.
 */
static uint64
sortsupport_sortable_double(double value)
{
  uint64 bits;
  memcpy(&bits, &value, sizeof(bits));
  return (bits & UINT64CONST(0x8000000000000000)) ? ~bits :
    (bits | UINT64CONST(0x8000000000000000));
}

/**
 * @brief Return the rank of a 32-bit integer
 */
uint32
sortsupport_rank_int32(int32 value)
{
  return ((uint32) value) ^ ((uint32) 0x80000000);
}

/**
 * @brief Return the rank of a 64-bit integer
 * @details The 32 high bits are kept, which is the resolution the curve is
 * given for a value spanning the whole range of the type.
 */
uint32
sortsupport_rank_int64(int64 value)
{
  return (uint32) ((((uint64) value) ^ UINT64CONST(0x8000000000000000)) >> 32);
}

/**
 * @brief Return the rank of a double
 * @details The 32 high bits of the sortable form are kept, leaving the sign,
 * the exponent and 20 bits of the mantissa.
 */
uint32
sortsupport_rank_double(double value)
{
  return (uint32) (sortsupport_sortable_double(value) >> 32);
}

/**
 * @brief Return the rank of a timestamp
 * @details The low bits are dropped before the rank is taken, so that a range
 * of dates far wider than any data set fits the 32 bits the curve is given.
 * A timestamp outside that range saturates rather than wrapping, which keeps
 * the rank increasing for the unbounded bounds of a span.
 */
uint32
sortsupport_rank_timestamptz(TimestampTz t)
{
  int64 ticks = ((int64) t) / (INT64CONST(1) << MEOS_SORT_TIME_SHIFT);
  if (ticks > (int64) PG_INT32_MAX)
    ticks = (int64) PG_INT32_MAX;
  else if (ticks < (int64) PG_INT32_MIN)
    ticks = (int64) PG_INT32_MIN;
  return (uint32) (ticks - (int64) PG_INT32_MIN);
}

/**
 * @brief Return the rank of a span bound
 */
static uint32
sortsupport_rank_bound(MeosType basetype, Datum value)
{
  switch (basetype)
  {
    case T_INT4:
      return sortsupport_rank_int32(DatumGetInt32(value));
    case T_INT8:
      return sortsupport_rank_int64(DatumGetInt64(value));
    case T_DATE:
      return sortsupport_rank_int32((int32) DatumGetDateADT(value));
    case T_TIMESTAMPTZ:
      return sortsupport_rank_timestamptz(DatumGetTimestampTz(value));
    default: /* T_FLOAT8 */
      return sortsupport_rank_double(DatumGetFloat8(value));
  }
}

/**
 * @brief Return the rank of the center of a span
 * @details The two bounds are ranked and the middle of the two ranks is taken,
 * since the middle of the bounds themselves overflows on an unbounded span.
 */
uint32
sortsupport_rank_span_center(const Span *s)
{
  uint64 lower = (uint64) sortsupport_rank_bound(s->basetype, s->lower);
  uint64 upper = (uint64) sortsupport_rank_bound(s->basetype, s->upper);
  return (uint32) ((lower + upper) / 2);
}

/*****************************************************************************
 * Sort keys
 *****************************************************************************/

/**
 * @brief Return the index on a Hilbert curve of two ranks
 * @details The curve treats its two arguments alike, so neither of the
 * dimensions composed through it leads the other.
 */
uint64
sortsupport_hilbert(uint32 rank1, uint32 rank2)
{
  return uint32_hilbert(rank1, rank2);
}

/**
 * @brief Return the sort key of a temporal box
 */
uint64
tbox_sort_hash(const TBox *box)
{
  uint32 value = MEOS_FLAGS_GET_X(box->flags) ?
    sortsupport_rank_span_center(&box->span) : 0;
  uint32 time = MEOS_FLAGS_GET_T(box->flags) ?
    sortsupport_rank_span_center(&box->period) : 0;
  return sortsupport_hilbert(value, time);
}

/**
 * @brief Return the sort key of a span
 * @details A span has a single dimension, where sorting on the lower bound is
 * already the order that packs an index, so no curve is involved. The key
 * keeps the whole 64 bits, and #span_cmp() settles the spans it ties.
 */
uint64
span_sort_hash(const Span *s)
{
  switch (s->basetype)
  {
    case T_INT4:
      return (uint64) sortsupport_rank_int32(DatumGetInt32(s->lower));
    case T_INT8:
      return ((uint64) DatumGetInt64(s->lower)) ^
        UINT64CONST(0x8000000000000000);
    case T_DATE:
      return (uint64) sortsupport_rank_int32((int32) DatumGetDateADT(s->lower));
    case T_TIMESTAMPTZ:
      return ((uint64) DatumGetTimestampTz(s->lower)) ^
        UINT64CONST(0x8000000000000000);
    default: /* T_FLOAT8 */
      return sortsupport_sortable_double(DatumGetFloat8(s->lower));
  }
}

/*****************************************************************************
 * Comparison functions shared by every operator class
 *****************************************************************************/

/**
 * @brief Compare two abbreviated keys
 */
int
sortsupport_abbrev_cmp(Datum x, Datum y, SortSupport ssup)
{
  uint64 hash1 = DatumGetUInt64(x);
  uint64 hash2 = DatumGetUInt64(y);
  (void) ssup;
  if (hash1 > hash2)
    return 1;
  if (hash1 < hash2)
    return -1;
  /* The keys are read again by the full comparator */
  return 0;
}

/**
 * @brief Return whether the abbreviated keys are worth keeping
 * @details They always are: the key is what the order is defined by, so
 * computing it once per entry is never wasted.
 */
bool
sortsupport_abbrev_abort(int memtupcount, SortSupport ssup)
{
  (void) memtupcount; (void) ssup;
  return false;
}

/*****************************************************************************/
