/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Functions for expandable Datum arrays.
 */


/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

/*****************************************************************************/

/**
 * @brief Initialize an expandable array that keeps the elements parsed so far
 * @details The array is initialized with a fix number of elements but it
 * expands when adding elements and the array is full
 */
MeosArray *
meos_array_init(int elem_size)
{
  MeosArray *array = (MeosArray *) palloc0(sizeof(MeosArray));
  if (! array)
    return NULL;
  array->capacity = MEOS_ARRAY_INITIAL_SIZE;
  array->count = 0;
  if (elem_size < 0)
  {
    array->varlength = true;
    array->elem_size = sizeof(Datum);
  }
  else
  {
    array->varlength = false;
    array->elem_size = MAXALIGN(elem_size);
  }
  array->elems = palloc0(array->elem_size * array->capacity);
  return array;
}

/**
 * @brief Get the address of the n-th slot of the array
 */
static inline void *
array_slot(const MeosArray *array, int n)
{
  return (char *) array->elems + ((size_t)n * array->elem_size);
}

/**
 * @brief Add a value to the array
 */
void
meos_array_add(MeosArray *array, void *value)
{
  assert(array); assert(array->elem_size > 0);
  /* Enlarge the values array if necessary */
  if (array->count >= array->capacity)
  {
    array->capacity *= 2;
    array->elems = repalloc(array->elems, array->capacity * array->elem_size);
    assert(array->elems);
  }
  /* Store the value */
  char *dest = array_slot(array, array->count);
  memcpy(dest, value, array->elem_size);
  array->count++;
  return;
}

/**
 * @brief Get the n-th Datum value of the array (0-based) 
 * @note Function used to access Datum stored in the array
 */
Datum
meos_array_get_n(const MeosArray *array, int n)
{
  assert(array);
  if (n < 0 || (size_t) n >= array->count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Invalid array index %d", n);
    return (Datum) 0;
  }
  Datum d;
  memcpy(&d, array_slot(array, n), sizeof(Datum));
  return d;
}

/**
 * @brief Reset the array
 */
void
meos_array_reset(MeosArray *array)
{
  if (! array || ! array->count)
    return;
  if (array->varlength)
  {
    /* For variable-length values the value stored in the array is a Datum */
    for (size_t i = 0; i < array->count; i++)
      pfree((void *) ((Datum *)(array->elems))[i]);
  }
  array->count = 0;
  return;
}

/**
 * @brief Destroy the array
 */
void
meos_array_destroy(MeosArray *array)
{
  if (! array)
    return;
  meos_array_reset(array);
  pfree(array);
  return;
}

/*****************************************************************************/
