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
 * @brief Functions for expandable arrays.
 */


/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * Helper functions
 *****************************************************************************/

/**
 * @brief Get the address of the n-th slot of the array
 */
static inline void *
array_slot(const MeosArray *array, int n)
{
  return (char *) array->elems + ((size_t)n * array->elem_size);
}

/**
 * @brief Reset the array
 * @param[in] array Array
 * @param[in] free_elems If true and the array is varlength, pfree each stored
 * pointer before resetting
 */
static void
meos_array_reset_int(MeosArray *array, bool free_elems)
{
  if (! array)
    return;
  if (free_elems && array->varlength)
  {
    for (size_t i = 0; i < array->count; i++)
      pfree(meos_array_get(array, (int) i));
  }
  array->count = 0;
}

/*****************************************************************************
 * Public functions
 *****************************************************************************/

/**
 * @ingroup meos_misc
 * @brief Create an expandable array
 * @details The array is initialized with a fixed number of elements but
 * expands automatically when full. Use a positive @p elem_size for
 * fixed-size elements (e.g., `sizeof(int)`), or a negative value to store
 * variable-length pointers (the array stores the pointers, not the data
 * they point to).
 * @param[in] elem_size Size of a single element in bytes, or negative for
 * variable-length pointer storage
 * @return New array on success, NULL on error
 */
MeosArray *
meos_array_create(int elem_size)
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
 * @ingroup meos_misc
 * @brief Destroy the array, freeing its internal storage
 * @details For varlength arrays, the caller is responsible for freeing the
 * stored pointers before calling this function. Use #meos_array_destroy_free
 * to have the array free the stored pointers automatically.
 * @param[in] array Array
 */
void
meos_array_destroy(MeosArray *array)
{
  if (! array)
    return;
  pfree(array->elems);
  pfree(array);
}

/**
 * @ingroup meos_misc
 * @brief Destroy a varlength array, freeing both the stored pointers and the
 * internal storage
 * @details This is equivalent to calling #meos_array_reset_free followed by
 * #meos_array_destroy. For fixed-size arrays, this is identical to
 * #meos_array_destroy.
 * @param[in] array Array
 */
void
meos_array_destroy_free(MeosArray *array)
{
  if (! array)
    return;
  meos_array_reset_int(array, true);
  pfree(array->elems);
  pfree(array);
}

/**
 * @ingroup meos_misc
 * @brief Add a value to the array
 * @param[in] array Array
 * @param[in] value Pointer to the value to add. For fixed-size arrays, the
 * value is copied. For varlength arrays, the pointer itself is stored.
 */
void
meos_array_add(MeosArray *array, void *value)
{
  assert(array);
  /* Enlarge the values array if necessary */
  if (array->count >= array->capacity)
  {
    array->capacity *= 2;
    array->elems = repalloc(array->elems, array->capacity * array->elem_size);
  }
  /* Store the value */
  char *dest = array_slot(array, array->count);
  if (array->varlength)
  {
    /* Store the pointer itself as a Datum */
    Datum d = PointerGetDatum(value);
    memcpy(dest, &d, sizeof(Datum));
  }
  else
  {
    /* Copy the value */
    memcpy(dest, value, array->elem_size);
  }
  array->count++;
}

/**
 * @ingroup meos_misc
 * @brief Get the n-th element of the array (0-based)
 * @param[in] array Array
 * @param[in] n Index
 * @return For fixed-size arrays, a pointer to the element in the internal
 * buffer. For varlength arrays, the stored pointer. NULL on error.
 */
void *
meos_array_get(const MeosArray *array, int n)
{
  assert(array);
  if (n < 0 || (size_t) n >= array->count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Invalid array index %d", n);
    return NULL;
  }
  void *slot = array_slot(array, n);
  if (array->varlength)
    return (void *) (*(Datum *) slot);
  return slot;
}

/**
 * @ingroup meos_misc
 * @brief Return the number of elements in the array
 * @param[in] array Array
 * @return Number of elements
 */
int
meos_array_count(const MeosArray *array)
{
  assert(array);
  return (int) array->count;
}

/**
 * @ingroup meos_misc
 * @brief Reset the array, keeping the allocated memory for reuse
 * @details For varlength arrays, the caller is responsible for freeing the
 * stored pointers before calling this function. Use #meos_array_reset_free
 * to have the array free the stored pointers automatically.
 * @param[in] array Array
 */
void
meos_array_reset(MeosArray *array)
{
  meos_array_reset_int(array, false);
}

/**
 * @ingroup meos_misc
 * @brief Reset a varlength array, freeing the stored pointers and keeping
 * the allocated memory for reuse
 * @details For fixed-size arrays, this is identical to #meos_array_reset.
 * @param[in] array Array
 */
void
meos_array_reset_free(MeosArray *array)
{
  meos_array_reset_int(array, true);
}

/*****************************************************************************/
