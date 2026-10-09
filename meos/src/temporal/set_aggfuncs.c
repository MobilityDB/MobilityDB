/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Aggregate functions for set types shared by MEOS and MobilityDB
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
  #include "cbuffer/tcbuffer_boxops.h"
#endif
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif
#if POSE
  #include "pose/pose.h"
  #include "pose/posechain.h"
  #include "pose/tpose_boxops.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
  #include "rgeo/trgeo_boxops.h"
#endif

/*****************************************************************************
 * Aggregate functions for sets
 *****************************************************************************/

/**
 * @brief Return the size a variable-length value takes in a set, where it is
 * stored with a 4-byte header also when it comes with a 1-byte one
 * @param[in] ptr Value
 */
static inline size_t
set_varlena_size(const char *ptr)
{
  return VARATT_IS_SHORT(ptr) ?
    VARSIZE_SHORT(ptr) - VARHDRSZ_SHORT + VARHDRSZ : VARSIZE_ANY(ptr);
}

/**
 * @brief Copy a variable-length value into a set with a 4-byte header
 * @param[out] dst Destination in the set
 * @param[in] ptr Value
 * @param[in] size Size of the value in the set, see #set_varlena_size
 */
static inline void
set_varlena_copy(char *dst, const char *ptr, size_t size)
{
  if (VARATT_IS_SHORT(ptr))
  {
    SET_VARSIZE(dst, size);
    memcpy(dst + VARHDRSZ, VARDATA_SHORT(ptr), size - VARHDRSZ);
  }
  else
    memcpy(dst, ptr, size);
}

/**
 * @brief Return a copy of an expandable set with twice its maximum number of
 * values and twice the bytes its values take, room for a value of the given
 * size included
 * @details The header, the bounding box, the offsets and the bytes of the
 * values are copied as they are, so that no value is read again, as
 * #tsequence_append_tinstant passes its bounding box to the sequence it
 * rebuilds; the values keep the order they arrived in, which the final
 * function of the aggregate orders once
 * @param[in] set Set
 * @param[in] size_elem Size of the value to append
 */
static Set *
set_grow(const Set *set, size_t size_elem)
{
  int maxcount = set->maxcount * 2;
  size_t hdrsize = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(set->bboxsize);
  size_t oldpdata = hdrsize + sizeof(size_t) * set->maxcount;
  size_t newpdata = hdrsize + sizeof(size_t) * maxcount;
  /* Bytes taken by the values, nothing for values passed by value */
  size_t used = 0;
  if (! MEOS_FLAGS_GET_BYVAL(set->flags))
  {
    int16 typlen = meostype_length(set->basetype);
    Datum last = SET_VAL_N(set, set->count - 1);
    size_t size_last = (typlen == -1) ? VARSIZE_ANY(last) : (size_t) typlen;
    used = (SET_OFFSETS_PTR(set))[set->count - 1] + DOUBLE_PAD(size_last);
  }
  size_t room = MEOS_FLAGS_GET_BYVAL(set->flags) ? 0 :
    2 * (used + DOUBLE_PAD(size_elem));
#ifdef DEBUG_EXPAND
  meos_error(WARNING, " Set -> %d\n", maxcount);
#endif /* DEBUG_EXPAND */
  Set *result = palloc(newpdata + room);
  memcpy(result, set, hdrsize);
  SET_VARSIZE(result, newpdata + room);
  result->maxcount = maxcount;
  memcpy(SET_OFFSETS_PTR(result), SET_OFFSETS_PTR(set),
    sizeof(size_t) * set->count);
  if (used)
    memcpy(((char *) result) + newpdata, ((char *) set) + oldpdata, used);
  return result;
}

/**
 * @brief Append a value to a set
 * @details The values of the set keep the order they arrived in and the
 * bounding box of the set is the one it was created with: the final function
 * of the aggregate orders the values and computes the bounding box once, see
 * #set_compact
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
    int16 typlen = meostype_length(set->basetype);
    if (typlen == -1)
      size_elem = set_varlena_size(DatumGetPointer(value));
    else
      size_elem = typlen;

    /* The free bytes start after the last value */
    size_t pdata = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(set->bboxsize) +
      sizeof(size_t) * set->maxcount;
    size_t lastpos = (SET_OFFSETS_PTR(set))[set->count - 1];
    size_t size_last = (typlen == -1) ?
      VARSIZE_ANY(((char *) set) + pdata + lastpos) : size_elem;
    size_t pos = lastpos + DOUBLE_PAD(size_last);
    if (pdata + pos + DOUBLE_PAD(size_elem) > VARSIZE(set))
      /* There is NOT enough available space */
      break;

    /* The value fits in the free bytes */
    if (typlen == -1)
      set_varlena_copy(((char *) set) + pdata + pos, DatumGetPointer(value),
        size_elem);
    else
      memcpy(((char *) set) + pdata + pos, DatumGetPointer(value), size_elem);
    (SET_OFFSETS_PTR(set))[set->count++] = pos;
    return set;
  }

  /* There is no more free space: the set grows into a copy with twice the
   * room, where the value is appended */
  size_t size_elem = 0;
  if (! MEOS_FLAGS_GET_BYVAL(set->flags))
  {
    int16 typlen = meostype_length(set->basetype);
    size_elem = (typlen == -1) ? set_varlena_size(DatumGetPointer(value)) :
      (size_t) typlen;
  }
  Set *result = set_grow(set, size_elem);
  pfree(set);
  return set_append_value(result, value);
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
 * @csqlaggfn #setUnionTransition()
 */
Set *
value_union_transfn(Set *state, Datum value, MeosType basetype)
{
  /* Null state: create a new state with the value, read with a 4-byte header
   * when it comes with a 1-byte one */
  if (! state)
  {
    if (meostype_length(basetype) == -1 &&
        VARATT_IS_SHORT(DatumGetPointer(value)))
    {
      size_t size = set_varlena_size(DatumGetPointer(value));
      char *copy = palloc(size);
      set_varlena_copy(copy, DatumGetPointer(value), size);
      /* Arbitrary initialization to 64 elements */
      Datum d = PointerGetDatum(copy);
      Set *result = set_make_exp(&d, 1, 64, basetype, ORDER);
      pfree(copy);
      return result;
    }
    /* Arbitrary initialization to 64 elements */
    return set_make_exp(&value, 1, 64, basetype, ORDER);
  }

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
 * @csqlfn #Set_union_transfn()
 * @csqlaggfn #setUnionTransition()
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

  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(state, s))
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
 * @csqlfn #Set_union_finalfn()
 * @csqlaggfn #setUnionFinal()
 */
Set *
set_union_finalfn(Set *state)
{
  if (! state)
    return NULL;
  Set *result = set_compact(state);
  pfree(state);
  return result;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Combine function for set union aggregate
 * @param[in,out] state1 Current aggregate state
 * @param[in] state2 Aggregate state to combine with the first one
 * @return The state holding the values of both, which is the first state when
 * it has space for the values of the second one and a NEW state otherwise, the
 * first state being freed; the second state is left to the caller
 * @csqlfn #Set_union_combinefn()
 */
Set *
set_union_combinefn(Set *state1, Set *state2)
{
  if (! state2)
    return state1;
  if (! state1)
    return set_copy(state2);
  return set_union_transfn(state1, state2);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Return the bytes of the state of a set union aggregate, read back by
 * #setstate_deserialize
 * @details The bytes are the extended Well-Known Binary of the values of the
 * state, in the order the state holds them
 * @param[in] state State
 * @param[out] size_out Size of the result in bytes
 * @csqlfn #Setstate_serialize()
 */
uint8_t *
setstate_serialize(const Set *state, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(state, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return set_as_wkb(state, WKB_EXTENDED, size_out);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Return the state of a set union aggregate written by
 * #setstate_serialize
 * @param[in] bytes Bytes
 * @param[in] size Size of the bytes
 * @csqlfn #Setstate_deserialize()
 */
Set *
setstate_deserialize(const uint8_t *bytes, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(bytes, NULL);
  return set_from_wkb(bytes, size);
}

/*****************************************************************************/
