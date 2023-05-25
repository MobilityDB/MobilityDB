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
 * @file
 * @brief Aggregate functions for set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_util.h"
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @brief Expand a bounding box with a value
 * @param[in] d Value to append to the set
 * @param[in] basetype Type of the value
 * @param[out] box Bounding box
 */
void
set_expand_bbox(Datum d, meosType basetype, void *box)
{
  /* Currently, only spatial set types have bounding box */
  assert(set_basetype(basetype));
  assert(! alphanum_basetype(basetype));
  if (geo_basetype(basetype))
  {
    STBox box1;
    geo_set_stbox(DatumGetGserializedP(d), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#if NPOINT
  else if (basetype == T_NPOINT)
  {
    STBox box1;
    npoint_set_stbox(DatumGetNpointP(d), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#endif
  else
    elog(ERROR, "unknown set type for expanding bounding box: %d", basetype);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Append a value to a set
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
    if (MEOS_FLAGS_GET_BYVAL(set->flags))
    {
      (SET_OFFSETS_PTR(set))[set->count++] = d;
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
    Datum last = SET_VAL_N(set, set->count - 1);
    size_t size_last = (typlen == -1) ? VARSIZE_ANY(last) : size_elem;
    size_t avail_size = ((char *) set + VARSIZE_ANY(set)) -
      ((char *) DatumGetPointer(last) + DOUBLE_PAD(size_last));
    if (DOUBLE_PAD(size_elem) > avail_size)
      /* There is NOT enough available space */
      break;

    /* There is enough space to add the new value */
    size_t pdata = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(set->bboxsize) +
      sizeof(size_t) * set->maxcount;
    size_t pos = (SET_OFFSETS_PTR(set))[set->count - 1] + DOUBLE_PAD(size_last);
    memcpy(((char *) set) + pdata + pos, DatumGetPointer(d), size_elem);
    (SET_OFFSETS_PTR(set))[set->count++] = pos;
    /* Expand the bounding box and return */
    if (set->bboxsize != 0)
      set_expand_bbox(d, basetype, SET_BBOX_PTR(set));
    return set;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  Datum *values = palloc(sizeof(Datum) * (set->count + 1));
  for (int i = 0; i < set->count; i++)
    values[i] = SET_VAL_N(set, i);
  values[set->count] = d;
  int maxcount = (set->count < set->maxcount) ?
    set->maxcount : set->maxcount * 2;
#ifdef DEBUG_BUILD
  printf(" set -> %d\n", maxcount);
#endif /* DEBUG_BUILD */

  Set *result = set_make_exp(values, set->count + 1, maxcount, set->basetype,
    ORDERED_NO);
  pfree(values);
  pfree(set);
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_agg
 * @brief Transition function for set union aggregate of values
 */
Set *
value_union_transfn(Set *state, Datum d, meosType basetype)
{
  /* Null state: create a new state with the value */
  if (! state)
  {
    /* Arbitrary initialization to 64 elements */
    Set *result = set_make_exp(&d, 1, 64, basetype, ORDERED_NO);
    return result;
  }

  return set_append_value(state, d, basetype);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of integers
 */
Set *
int_union_transfn(Set *state, int32 i)
{
  return value_union_transfn(state, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of big integers
 */
Set *
bigint_union_transfn(Set *state, int64 i)
{
  return value_union_transfn(state, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of floats
 */
Set *
float_union_transfn(Set *state, double d)
{
  return value_union_transfn(state, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of timestamps
 */
Set *
timestamp_union_transfn(Set *state, TimestampTz t)
{
  return value_union_transfn(state, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of texts
 */
Set *
text_union_transfn(Set *state, const text *txt)
{
  return value_union_transfn(state, PointerGetDatum(txt), T_TEXT);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set union aggregate of sets
 */
Set *
set_union_transfn(Set *state, Set *set)
{
  int start = 0;
  Datum d;
  /* Null state: create a new state with the first value of the set */
  if (! state)
  {
    start = 1;
    d = SET_VAL_N(set, 0);
    /* Arbitrary initialization to 64 elements */
    state = set_make_exp(&d, 1, 64, set->basetype, ORDERED_NO);
  }

  for (int i = start; i < set->count; i++)
  {
    d = SET_VAL_N(set, i);
    state = set_append_value(state, d, set->basetype);
  }
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Final function for set union aggregate
 * @note The input state is NOT freed, this should be done by the calling
 * function
 */
Set *
set_union_finalfn(Set *state)
{
  if (! state)
    return NULL;

  /* Collect the UNSORTED values */
  Datum *values = palloc(sizeof(Datum) * state->count);
  for (int i = 0; i < state->count; i++)
    values[i] = SET_VAL_N(state, i);

  Set *result = set_make_exp(values, state->count, state->count,
    state->basetype, ORDERED);
  pfree(values);
  return result;
}

/*****************************************************************************/
