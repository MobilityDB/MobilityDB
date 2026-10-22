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
 * @brief JSON processing functions for temporal JSON and JSONB values that
 * the MEOS library defines
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * Temporal JSON functions
 *****************************************************************************/

/**
 * @ingroup meos_json_json
 * @brief Extract a JSON object field with the given key from a temporal JSON
 * value
 * @param[in] temp Temporal JSON value
 * @param[in] key Key
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjson_object_field(), #Tjson_object_field_opr()
 */
Temporal *
tjson_object_field(const Temporal *temp, const text *key,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL); VALIDATE_NOT_NULL(key, NULL);
  return tjson_object_field_common(temp, key, false, null_handle);
}

/**
 * @ingroup meos_json_json
 * @brief Extract a JSONB object field with the given key from a temporal
 * JSONB value
 * @param[in] temp Temporal JSONB value
 * @param[in] key Key
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_object_field(), #Tjsonb_object_field_opr()
 */
Temporal *
tjsonb_object_field(const Temporal *temp, const text *key,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(key, NULL);
  return tjsonb_object_field_common(temp, key, false, null_handle);
}

/**
 * @ingroup meos_json_json
 * @brief Extract a JSONB object field with the given key from a temporal
 * JSONB value as a temporal text
 * @param[in] temp Temporal JSONB value
 * @param[in] key Key
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_object_field_text(), #Tjsonb_object_field_text_opr()
 */
Temporal *
tjsonb_object_field_text(const Temporal *temp, const text *key,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(key, NULL);
  return tjsonb_object_field_common(temp, key, true, null_handle);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_json
 * @brief Extract an array element from a temporal JSONB value
 * @param[in] temp Temporal JSONB value
 * @param[in] idx Index
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_array_element(), #Tjsonb_array_element_opr()
 */
Temporal *
tjsonb_array_element(const Temporal *temp, int idx,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  return tjsonb_array_element_common(temp, idx, false, null_handle);
}

/**
 * @ingroup meos_json_json
 * @brief Extract an array element from a temporal JSONB value as a temporal
 * text
 * @param[in] temp Temporal JSONB value
 * @param[in] idx Index
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_array_element_text(), #Tjsonb_array_element_text_opr()
 */
Temporal *
tjsonb_array_element_text(const Temporal *temp, int idx,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  return tjsonb_array_element_common(temp, idx, true, null_handle);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_json
 * @brief Extract a temporal JSONB value specified by a path
 * @param[in] temp Temporal JSONB value
 * @param[in] path_elems Keys
 * @param[in] path_len Number of elements in the input array
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_extract_path(), #Tjsonb_extract_path_opr()
 */
Temporal *
tjsonb_extract_path(const Temporal *temp, text **path_elems, int path_len,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;
  return tjsonb_extract_path_common(temp, path_elems, path_len, false,
    null_handle);
}

/**
 * @ingroup meos_json_json
 * @brief Extract a temporal JSONB value specified by a path as a temporal
 * text
 * @param[in] temp Temporal JSONB value
 * @param[in] path_elems Keys
 * @param[in] path_len Number of elements in the input array
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Tjsonb_extract_path_text(), #Tjsonb_extract_path_text_opr()
 */
Temporal *
tjsonb_extract_path_text(const Temporal *temp, text **path_elems,
  int path_len, nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;
  return tjsonb_extract_path_common(temp, path_elems, path_len, true,
    null_handle);
}

/*****************************************************************************/
