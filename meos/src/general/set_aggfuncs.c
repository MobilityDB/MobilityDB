/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Aggregate functions for set values composed of an ordered list of
 * distinct values
 */

#include "general/set.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/ttext_funcs.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_boxops.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
  #include "cbuffer/tcbuffer_boxops.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
  #include "npoint/tnpoint_boxops.h"
#endif
#if POSE
  #include "pose/pose.h"
  #include "pose/tpose_boxops.h"
#endif

/*****************************************************************************
 * Aggregate functions for sets
 *****************************************************************************/

/**
 * @brief Expand a bounding box with a value
 * @param[in] value Value to append to the set
 * @param[in] basetype Type of the value
 * @param[out] box Bounding box
 */
void
set_expand_bbox(Datum value, meosType basetype, void *box)
{
  /* Currently, only spatial set types have bounding box */
  assert(set_basetype(basetype));
  assert(! alphanum_basetype(basetype));
  if (geo_basetype(basetype))
  {
    STBox box1;
    geo_set_stbox(DatumGetGserializedP(value), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#if CBUFFER
  else if (basetype == T_CBUFFER)
  {
    STBox box1;
    cbuffer_set_stbox(DatumGetCbufferP(value), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#endif
#if NPOINT
  else if (basetype == T_NPOINT)
  {
    STBox box1;
    npoint_set_stbox(DatumGetNpointP(value), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#endif
#if POSE
  else if (basetype == T_POSE)
  {
    STBox box1;
    pose_set_stbox(DatumGetPoseP(value), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#endif
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Unknown set type for expanding bounding box: %s",
      meostype_name(basetype));
  }
  return;
}

/**
 * @brief Append a value to a set
 * @param[in,out] set Set
 * @param[in] value Value
 */
Set *
set_append_value(Set *set, Datum value)
{
  assert(set);

  /* Account for expandable structures
   * A while is used instead of an if to enable to break the loop if there is
   * no more available space */
  while (set->count < set->maxcount)
  {
    /* If passed by value, set datum in the offsets array */
    if (MEOS_FLAGS_GET_BYVAL(set->flags))
    {
      (SET_OFFSETS_PTR(set))[set->count++] = value;
      return set;
    }

    /* Determine whether there is enough available space */
    size_t size_elem;
    int16 typlen = basetype_length(set->basetype);
    if (typlen == -1)
      /* VARSIZE_ANY is used for oblivious data alignment, see postgres.h */
      size_elem = VARSIZE_ANY(DatumGetPointer(value));
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
    memcpy(((char *) set) + pdata + pos, DatumGetPointer(value), size_elem);
    (SET_OFFSETS_PTR(set))[set->count++] = pos;
    /* Expand the bounding box and return */
    if (set->bboxsize != 0)
      set_expand_bbox(value, set->basetype, SET_BBOX_PTR(set));
    return set;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  Datum *values = palloc(sizeof(Datum) * (set->count + 1));
  for (int i = 0; i < set->count; i++)
    values[i] = SET_VAL_N(set, i);
  values[set->count] = value;
  int maxcount = (set->count < set->maxcount) ?
    set->maxcount : set->maxcount * 2;
#ifdef DEBUG_EXPAND
  meos_error(WARNING, " Set -> %d\n", maxcount);
#endif /* DEBUG_EXPAND */

  Set *result = set_make_exp(values, set->count + 1, maxcount, set->basetype,
    ORDER);
  pfree(values); pfree(set);
  return result;
}

/**
 * @ingroup meos_internal_setspan_agg
 * @brief Transition function for set union aggregate of values
 * @param[in,out] state Current aggregate state
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @return When the state variable has space for adding the new value, the 
 * function returns the current state variable. Otherwise, a NEW state 
 * variable is returned and the input state is freed.
 * @note Always use the function to overwrite the existing state as in: 
 * @code
 * state = value_union_transfn(state, value, basetype);
 * @endcode
 */
Set *
value_union_transfn(Set *state, Datum value, meosType basetype)
{
  /* Null state: create a new state with the value */
  if (! state)
    /* Arbitrary initialization to 64 elements */
    return set_make_exp(&value, 1, 64, basetype, ORDER);

  return set_append_value(state, value);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for set union aggregate of sets
 * @param[in,out] state Current aggregate state
 * @param[in] s Set to aggregate
 * @return When the state variable has space for adding the new set, the 
 * function returns the current state variable. Otherwise, a NEW state 
 * variable is returned and the input state is freed.
 * @note Always use the function to overwrite the existing state as in: 
 * @code
 * state = set_union_transfn(state, set);
 * @endcode
 */
Set *
set_union_transfn(Set *state, Set *s)
{
  /* Null set: return state */
  if (! s)
    return state;
  /* Null state: create a new state with the first value of the set */
  if (! state)
  {
    Datum value = SET_VAL_N(s, 0);
    /* Arbitrary initialization to 64 elements */
    state = set_make_exp(&value, 1, 64, s->basetype, ORDER);
  }

  /* Ensure validity of the arguments */
  if (! ensure_same_set_type(state, s))
    return NULL;

  for (int i = 0; i < s->count; i++)
    state = set_append_value(state, SET_VAL_N(s, i));
  return state;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Final function for set union aggregate
 * @param[in,out] state Current aggregate state
 * @note The input state must be free by the calling function
 */
Set *
set_union_finalfn(Set *state)
{
  if (! state)
    return NULL;

  Datum *values = palloc0(sizeof(Datum) * state->count);
  for (int i = 0; i < state->count; i++)
    values[i] = SET_VAL_N(state, i);
  meosType basetype = settype_basetype(state->settype);
  Set *result = set_make_exp(values, state->count, state->count, basetype,
    ORDER);

  /* Clean up and return */
  pfree(values);
  return result;
}

/*****************************************************************************/
