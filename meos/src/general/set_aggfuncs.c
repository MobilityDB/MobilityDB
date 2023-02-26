/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Aggregate functions for set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#if ! MEOS
  #include <fmgr.h>
  #include <utils/memutils.h>
#endif /* MEOS */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_util.h"

#if ! MEOS
  extern FunctionCallInfo fetch_fcinfo();
  extern void store_fcinfo(FunctionCallInfo fcinfo);
  extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
  extern void unset_aggregation_context(MemoryContext ctx);
#endif /* ! MEOS */

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Append a value to an unordered array.
 * @param[in,out] set Set
 * @param[in] d Value
 * @param[in] basetype Base type
 */
Set *
set_append_value(Set *set, Datum d, meosType basetype)
{
  assert(set->basetype == basetype);

  /* Account for expandable structures
   * A while is used instead of an if to enable to break the loop if there is
   * no more available space */
  while (set->count < set->maxcount)
  {
    /* If passed by value, set datum in the offsets array */
    if (MOBDB_FLAGS_GET_BYVAL(set->flags))
    {
      (set_offsets_ptr(set))[set->count++] = d;
      return set;
    }

    /* Determine whether there is enough available space */
    size_t size_elem;
    int16 typlen = basetype_length(basetype);
    if (typlen == -1)
      /* VARSIZE_ANY is used for oblivious data alignment, see postgres.h */
      size_elem = VARSIZE_ANY(DatumGetPointer(d));
    else
      size_elem = typlen;

    /* Get the last instant to keep */
    Datum last = set_val_n(set, set->count - 1);
    size_t size_last = (typlen == -1) ? VARSIZE_ANY(last) : size_elem;
    size_t avail_size = ((char *) set + VARSIZE_ANY(set)) -
      ((char *) DatumGetPointer(last) + DOUBLE_PAD(size_last));
    if (DOUBLE_PAD(size_elem) > avail_size)
      /* There is NOT enough available space */
      break;

    /* There is enough space to add the new value */
    size_t pdata = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(set->bboxsize) +
      sizeof(size_t) * set->maxcount;
    size_t pos = (set_offsets_ptr(set))[set->count - 1] + DOUBLE_PAD(size_last);
    memcpy(((char *) set) + pdata + pos, DatumGetPointer(d), size_elem);
    (set_offsets_ptr(set))[set->count++] = pos;
    /* Expand the bounding box and return */
    if (set->bboxsize != 0)
      set_expand_bbox(d, basetype, set_bbox_ptr(set));
    return set;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  Datum *values = palloc(sizeof(Datum) * set->count + 1);
  for (int i = 0; i < set->count; i++)
    values[i] = set_val_n(set, i);
  values[set->count] = d;
  int maxcount = set->maxcount * 2;
#ifdef DEBUG_BUILD
  printf(" set -> %d\n", maxcount);
#endif /* DEBUG_BUILD */

#if ! MEOS
  MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
  Set *result = set_make_exp(values, set->count + 1, maxcount, set->basetype,
    ORDERED_NO);
#if ! MEOS
  unset_aggregation_context(ctx);
#endif /* ! MEOS */
  pfree(values);
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
set_union_transfn(Set *state, Datum d, meosType basetype)
{
  /* Null set: create a new set with the value */
  if (! state)
  {
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    /* Arbitrary initialization to 64 elements */
    Set *result = set_make_exp(&d, 1, 64, basetype, ORDERED_NO);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
    return result;
  }

  return set_append_value(state, d, basetype);
  // return union_set_value(state, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
intset_union_transfn(Set *state, int32 i)
{
  return set_union_transfn(state, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
bigintset_union_transfn(Set *state, int64 i)
{
  return set_union_transfn(state, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
floatset_union_transfn(Set *state, double d)
{
  return set_union_transfn(state, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
tstzset_union_transfn(Set *state, TimestampTz t)
{
  return set_union_transfn(state, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
textset_union_transfn(Set *state, const text *txt)
{
  return set_union_transfn(state, PointerGetDatum(txt), T_TEXT);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_agg
 * @brief Transition function for set aggregate of values
 */
Set *
set_union_finalfn(Set *state)
{
  if (! state)
    return NULL;

  /* Collect the UNSORTED values */
  Datum *values = palloc(sizeof(Datum) * state->count);
  for (int i = 0; i < state->count; i++)
    values[i] = set_val_n(state, i);

  Set *result = set_make_exp(values, state->count, state->count,
    state->basetype, ORDERED);
  pfree(values);
  pfree(state);
  return result;
}

/*****************************************************************************/
