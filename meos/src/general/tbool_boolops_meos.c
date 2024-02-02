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
 * @brief Temporal Boolean operators: and, or, not
 */

#include "general/tbool_boolops.h"

/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean and of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Tand_bool_tbool()
 */
Temporal *
tand_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return boolop_tbool_bool(temp, b, &datum_and, INVERT);
}

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean and of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Tand_tbool_bool()
 */
Temporal *
tand_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return boolop_tbool_bool(temp, b, &datum_and, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean and of the temporal booleans
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tand_tbool_tbool()
 */
Temporal *
tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type(temp1, T_TBOOL) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return NULL;
  return boolop_tbool_tbool(temp1, temp2, &datum_and);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean or of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Tor_bool_tbool()
 */
Temporal *
tor_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return boolop_tbool_bool(temp, b, &datum_or, INVERT);
}

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean or of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Tor_tbool_bool()
 */
Temporal *
tor_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return boolop_tbool_bool(temp, b, &datum_or, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bool
 * @brief Return the boolean or of the temporal booleans
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Tor_tbool_tbool()
 */
Temporal *
tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type(temp1, T_TBOOL) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return NULL;
  return boolop_tbool_tbool(temp1, temp2, &datum_or);
}

/*****************************************************************************/

