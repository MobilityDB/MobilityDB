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
 * @file tpoint_in.c
 * @brief Input of temporal points in WKT, EWKT, , EWKB, and MF-JSON format.
 */

#include "point/tpoint_in.h"

/* C */
#include <assert.h>
#include <float.h>
/* MobilityDB */
#include <meos.h>
// #include "general/temporal_in.h"
#include "general/temporal_util.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal point from its WKT representation
 * @sqlfunc tgeompointFromText(), tgeogpointFromText()
 */
Temporal *
tpoint_from_text(const char *wkt, mobdbType temptype)
{
  Temporal *result = tpoint_parse((char **) &wkt, temptype);
  return result;
}
#endif

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal point from its EWKT representation
 * @sqlfunc tgeompointFromEWKT(), tgeogpointFromEWKT()
 */
Temporal *
tpoint_from_ewkt(const char *ewkt, mobdbType temptype)
{
  Temporal *result = tpoint_parse((char **) &ewkt, temptype);
  return result;
}

/*****************************************************************************/
