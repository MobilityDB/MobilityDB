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
 * @brief JSON processing functions for JSONB sets that the MEOS library
 * defines
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * JSON set functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB object field with the given key from a JSONB set
 * @param[in] set JSONB set
 * @param[in] key Key
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_object_field(), #Jsonbset_object_field_opr()
 */
Set *
jsonbset_object_field(const Set *set, const text *key,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(key, NULL);
  return jsonbset_object_field_common(set, key, false, null_handle);
}

/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB object field with the given key from a JSONB set as
 * a text set
 * @param[in] set JSONB set
 * @param[in] key Key
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_object_field_text(), #Jsonbset_object_field_text_opr()
 */
Set *
jsonbset_object_field_text(const Set *set, const text *key,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(key, NULL);
  return jsonbset_object_field_common(set, key, true, null_handle);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract an array element from a JSONB set
 * @param[in] set JSONB set
 * @param[in] idx Index
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_array_element(), #Jsonbset_array_element_opr()
 */
Set *
jsonbset_array_element(const Set *set, int idx, nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL);
  return jsonbset_array_element_common(set, idx, false, null_handle);
}

/**
 * @ingroup meos_json_set_json
 * @brief Extract an array element from a JSONB set as a text set
 * @param[in] set JSONB set
 * @param[in] idx Index
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_array_element_text(), #Jsonbset_array_element_text_opr()
 */
Set *
jsonbset_array_element_text(const Set *set, int idx,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL);
  return jsonbset_array_element_common(set, idx, true, null_handle);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB set specified by a path
 * @param[in] set JSONB set
 * @param[in] path_elems Keys
 * @param[in] path_len Number of elements in the input array
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_extract_path(), #Jsonbset_extract_path_opr()
 */
Set *
jsonbset_extract_path(const Set *set, text **path_elems, int path_len,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;
  return jsonbset_extract_path_common(set, path_elems, path_len, false,
    null_handle);
}

/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB set specified by a path as a text set
 * @param[in] set JSONB set
 * @param[in] path_elems Keys
 * @param[in] path_len Number of elements in the input array
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_extract_path_text(), #Jsonbset_extract_path_text_opr()
 */
Set *
jsonbset_extract_path_text(const Set *set, text **path_elems, int path_len,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;
  return jsonbset_extract_path_common(set, path_elems, path_len, true,
    null_handle);
}

/*****************************************************************************/
