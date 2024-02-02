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
 * @brief Functions for parsing static network types
 */

#include "npoint/tnpoint_parser.h"

/* MEOS */
#include <meos.h>
#include "general/type_parser.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

/**
 * @brief Parse a network point from its string representation
 */
Npoint *
npoint_parse(const char **str, bool end)
{
  const char *type_str = "network point";
  p_whitespace(str);
  if (pg_strncasecmp(*str, "NPOINT", 6) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse network point");
    return NULL;
  }

  *str += 6;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse rid */
  p_whitespace(str);
  Datum d;
  if (! elem_parse(str, T_INT8, &d))
    return NULL;
  int64 rid = DatumGetInt64(d);

  p_comma(str);

  /* Parse pos */
  p_whitespace(str);
  if (! elem_parse(str, T_FLOAT8, &d))
    return NULL;
  double pos = DatumGetFloat8(d);
  if (pos < 0 || pos > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  return npoint_make(rid, pos);
}

/**
 * @brief Parse a network segment from its string representation
 */
Nsegment *
nsegment_parse(const char **str)
{
  const char *type_str = "network segment";
  p_whitespace(str);

  if (pg_strncasecmp(*str, "NSEGMENT", 8) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse network segment");
    return NULL;
  }

  *str += 8;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse rid */
  p_whitespace(str);
  Datum d;
  if (! elem_parse(str, T_INT8, &d))
    return NULL;
  int64 rid = DatumGetInt64(d);

  p_comma(str);

  /* Parse pos1 */
  p_whitespace(str);
  if (! elem_parse(str, T_FLOAT8, &d))
    return NULL;
  double pos1 = DatumGetFloat8(d);
  if (pos1 < 0 || pos1 > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }
  p_comma(str);

  /* Parse pos2 */
  p_whitespace(str);
  if (! elem_parse(str, T_FLOAT8, &d))
    return NULL;
  double pos2 = DatumGetFloat8(d);
  if (pos2 < 0 || pos2 > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) || ! ensure_end_input(str, type_str))
    return NULL;

  return nsegment_make(rid, pos1, pos2);
}

/*****************************************************************************/
