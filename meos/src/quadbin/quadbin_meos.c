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
 * @brief Static quadbin SQL type — validating parser plus comparison /
 * hash helpers.
 *
 * The static quadbin type is the square-quadtree analogue of the static
 * `h3index` type. The kernel in quadbin.c owns the bit / value algebra
 * (tile/cell conversion, hierarchy, lat/lng, serialization); this file
 * adds the I/O and ordering layer the temporal `tquadbin` type relies on:
 *
 *   * a validating string parser that rejects values not encoding a
 *     valid quadbin cell,
 *   * comparison / ordering / hashing helpers.
 *
 * Quadbin is a uint64-backed value. Comparison and hashing reduce to
 * plain int64 bit operations — they carry no geographic meaning but are
 * required for btree indexing, ORDER BY, GROUP BY, DISTINCT, etc.
 *
 * Datum packing for quadbin values uses `DatumGetQuadbin` /
 * `QuadbinGetDatum` from `quadbin_meos.h`.
 */

#include "quadbin/quadbin_meos.h"

#include <ctype.h>
#include <string.h>

#include <postgres.h>

#include <meos.h>
#include <pgtypes.h>

/*****************************************************************************
 * Parsing
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_base_inout
 * @brief Parse a string into a quadbin cell. See header for the accepted
 * input shapes.
 */
Quadbin
quadbin_parse(const char *str)
{
  if (str == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin input must not be null");
    return (Quadbin) 0;
  }

  /* Strip leading whitespace. */
  while (*str && isspace((unsigned char) *str))
    str++;

  /* Skip an optional "0x" / "0X" hex prefix; the canonical quadbin
   * output is unprefixed lowercase hex. */
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    str += 2;

  Quadbin cell = quadbin_string_to_index(str);

  /* Reject anything that does not encode a valid quadbin cell. */
  if (! quadbin_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin value \"%s\" does not encode a valid quadbin cell", str);
    return (Quadbin) 0;
  }

  return cell;
}

/*****************************************************************************
 * Comparison / ordering
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if two quadbin values are equal
 */
bool
quadbin_eq(Quadbin a, Quadbin b)
{
  return a == b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if two quadbin values are not equal
 */
bool
quadbin_ne(Quadbin a, Quadbin b)
{
  return a != b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is less than the second
 */
bool
quadbin_lt(Quadbin a, Quadbin b)
{
  return a < b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is less than or equal to
 * the second
 */
bool
quadbin_le(Quadbin a, Quadbin b)
{
  return a <= b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than the second
 */
bool
quadbin_gt(Quadbin a, Quadbin b)
{
  return a > b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than or equal
 * to the second
 */
bool
quadbin_ge(Quadbin a, Quadbin b)
{
  return a >= b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return -1 / 0 / 1 depending on whether the first quadbin is
 * less than, equal to, or greater than the second
 */
int
quadbin_cmp(Quadbin a, Quadbin b)
{
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/*****************************************************************************
 * Hashing
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_base_accessor
 * @brief Return the 32-bit hash value of a quadbin — matches the result
 * `hashint8` would produce on the same bit pattern.
 */
uint32
quadbin_hash(Quadbin cell)
{
  return int64_hash((int64) cell);
}

/*****************************************************************************/
