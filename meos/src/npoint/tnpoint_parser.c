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
 * @brief Functions for parsing static network types.
 */

#include "npoint/tnpoint_parser.h"

/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

/**
 * @brief Parse a network point from its string representation.
 */
Npoint *
npoint_parse(const char **str, bool end)
{
  p_whitespace(str);

  if (strncasecmp(*str, "NPOINT", 6) != 0)
    elog(ERROR, "Could not parse network point");

  *str += 6;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! p_oparen(str))
    elog(ERROR, "Could not parse network point: Missing opening parenthesis");

  /* Parse rid */
  p_whitespace(str);
  int64 rid = DatumGetInt64(elem_parse(str, T_INT8));

  p_whitespace(str);
  p_comma(str);
  p_whitespace(str);

  double pos = DatumGetFloat8(elem_parse(str, T_FLOAT8));
  if (pos < 0 || pos > 1)
    elog(ERROR, "The relative position must be a real number between 0 and 1");

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! p_cparen(str))
    elog(ERROR, "Could not parse network point: Missing closing parenthesis");

  /* Ensure there is no more input */
  ensure_end_input(str, end, "network point");

  return npoint_make(rid, pos);
}

/**
 * @brief Parse a network segment from its string representation.
 */
Nsegment *
nsegment_parse(const char **str)
{
  p_whitespace(str);

  if (strncasecmp(*str, "NSEGMENT", 8) != 0)
    elog(ERROR, "Could not parse network segment");

  *str += 8;
  p_whitespace(str);

  int delim = 0;
  while ((*str)[delim] != ')' && (*str)[delim] != '\0')
    delim++;
  if ((*str)[delim] == '\0')
    elog(ERROR, "Could not parse network segment");

  int64 rid;
  double pos1;
  double pos2;
  if (sscanf(*str, "( %ld , %lf , %lf )", &rid, &pos1, &pos2) != 3)
    elog(ERROR, "Could not parse network segment");
  if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
    elog(ERROR, "The relative position must be a real number between 0 and 1");

  *str += delim + 1;

  /* Ensure there is no more input */
  ensure_end_input(str, true, "network segment");

  return nsegment_make(rid, pos1, pos2);
}

/*****************************************************************************/
