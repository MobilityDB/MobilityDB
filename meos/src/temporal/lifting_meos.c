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
 * @brief Reading of the error number by the lifting functions under an error
 * handler that does not exit
 */

#include "temporal/lifting.h"

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>

/**
 * @brief Return the result of a lifted operation, or NULL when the function
 * it applies raised an error for one of the values
 * @param[in] result Result built by the lifting, freed on an error
 * @param[in] last_errno Error number found on entry, returned by
 * #meos_errno_reset
 * @note See the note on errors in the file comment of lifting.c
 */
void *
lifting_result(void *result, int last_errno)
{
  if (meos_errno())
  {
    if (result)
      pfree(result);
    return NULL;
  }
  meos_errno_restore(last_errno);
  return result;
}

/**
 * @brief Return the result of a lifted ever or always operation, or -1 when
 * the function it applies raised an error for one of the values
 * @param[in] result Result of the operation
 * @param[in] last_errno Error number found on entry, returned by
 * #meos_errno_reset
 * @note See the note on errors in the file comment of lifting.c
 */
int
lifting_result_int(int result, int last_errno)
{
  if (meos_errno())
    return -1;
  meos_errno_restore(last_errno);
  return result;
}

/*****************************************************************************/
