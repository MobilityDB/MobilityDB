/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tbool_boolops.c
 * @brief Temporal Boolean operators: and, or, not.
 */

#include "general/tbool_boolops.h"

/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/lifting.h"

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean and of the value and the temporal value
 */
Temporal *
tand_bool_tbool(bool b, const Temporal *temp)
{
  return boolop_tbool_bool(temp, b, &datum_and, INVERT);
}

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean and of the temporal value and the value
 */
Temporal *
tand_tbool_bool(const Temporal *temp, bool b)
{
  return boolop_tbool_bool(temp, b, &datum_and, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean and of the temporal values
 */
Temporal *
tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2)
{
  return boolop_tbool_tbool(temp1, temp2, &datum_and);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean or of the value and the temporal value
 */
Temporal *
tor_bool_tbool(bool b, const Temporal *temp)
{
  return boolop_tbool_bool(temp, b, &datum_or, INVERT);
}

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean or of the temporal value and the value
 */
Temporal *
tor_tbool_bool(const Temporal *temp, bool b)
{
  return boolop_tbool_bool(temp, b, &datum_or, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bool
 * @brief Return the temporal boolean or of the temporal values
 */
Temporal *
tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2)
{
  return boolop_tbool_tbool(temp1, temp2, &datum_or);
}

/*****************************************************************************/

