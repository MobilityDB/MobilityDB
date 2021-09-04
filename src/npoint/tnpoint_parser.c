/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * @file tnpoint_parser.c
 * Functions for parsing static network types.
 */

#include "npoint/tnpoint_parser.h"

#include "general/temporaltypes.h"
#include "general/temporal_parser.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"

/*****************************************************************************/

npoint *
npoint_parse(char **str)
{
  p_whitespace(str);

  if (strncasecmp(*str,"NPOINT",6) != 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse network point")));

  *str += 6;
  p_whitespace(str);

  int delim = 0;
  while ((*str)[delim] != ')' && (*str)[delim] != '\0')
    delim++;
  if ((*str)[delim] == '\0')
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse network point")));

  int64 rid;
  double pos;
#ifdef WIN32
  if (sscanf(*str, "( %lld , %lf )", &rid, &pos) != 2)
#else
  if (sscanf(*str, "( %ld , %lf )", &rid, &pos) != 2)
#endif
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("Could not parse network point")));
  if (pos < 0 || pos > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The relative position must be a real number between 0 and 1")));

  *str += delim + 1;

  return npoint_make(rid, pos);
}

nsegment *
nsegment_parse(char **str)
{
  p_whitespace(str);

  if (strncasecmp(*str,"NSEGMENT",8) != 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse network segment")));

  *str += 8;
  p_whitespace(str);

  int delim = 0;
  while ((*str)[delim] != ')' && (*str)[delim] != '\0')
    delim++;
  if ((*str)[delim] == '\0')
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse network segment")));

  int64 rid;
  double pos1;
  double pos2;
#ifdef WIN32
  if (sscanf(*str, "( %lld , %lf , %lf )", &rid, &pos1, &pos2) != 3)
#else
  if (sscanf(*str, "( %ld , %lf , %lf )", &rid, &pos1, &pos2) != 3)
#endif
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse network segment")));
  if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("the relative position must be a real number between 0 and 1")));

  *str += delim + 1;

  return nsegment_make(rid, pos1, pos2);
}

/*****************************************************************************/
