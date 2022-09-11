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
 * @brief Output of temporal points in WKT, EWKT, and MF-JSON format.
 */

#include "point/tpoint_out.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MobilityDB */
#include <meos.h>
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * Output a geometry in Well-Known Text (WKT) format.
 *
 * @note The parameter type is not needed for temporal points
 */
static char *
wkt_out(mobdbType basetype __attribute__((unused)), Datum value, Datum arg)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_ISO, DatumGetInt32(arg), &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom);
  pfree(wkt);
  return result;
}

/**
 * Output a geometry in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID.
 *
 * @note The parameter type is not needed for temporal points
 */
char *
ewkt_out(mobdbType basetype __attribute__((unused)), Datum value, Datum arg)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_EXTENDED, DatumGetInt32(arg), &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom);
  pfree(wkt);
  return result;
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return the Well-Known Text (WKT) representation of a temporal point.
 * @sqlfunc asText()
 */
char *
tpoint_as_text(const Temporal *temp, int maxdd)
{
  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_to_string((TInstant *) temp, Int32GetDatum(maxdd),
      &wkt_out);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_to_string((TSequence *) temp, Int32GetDatum(maxdd),
      false, &wkt_out);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *) temp, Int32GetDatum(maxdd),
      &wkt_out);
  return result;
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return the Extended Well-Known Text (EWKT) representation a temporal
 * point.
 * @sqlfunc asEWKT()
 */
char *
tpoint_as_ewkt(const Temporal *temp, int maxdd)
{
  int srid = tpoint_srid(temp);
  char str1[20];
  if (srid > 0)
    sprintf(str1, "SRID=%d%c", srid, 
      (MOBDB_FLAGS_GET_INTERP(temp->flags) == STEPWISE) ? ',' : ';');
  else
    str1[0] = '\0';
  char *str2 = tpoint_as_text(temp, maxdd);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return the Well-Known Text (WKT) or the Extended Well-Known Text (EWKT)
 * representation of a geometry/geography array.
 *
 * @param[in] geoarr Array of geometries/geographies
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @sqlfunc asText(), asEWKT()
 */
char **
geoarr_as_text(const Datum *geoarr, int count, int maxdd, bool extended)
{
  char **result = palloc(sizeof(char *) * count);
  Datum arg = Int32GetDatum(maxdd);
  for (int i = 0; i < count; i++)
    /* The wkt_out and ewkt_out functions do not use the first argument */
    result[i] = extended ?
      ewkt_out(0, geoarr[i], arg) : wkt_out(0, geoarr[i], arg);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return the Well-Known Text (WKT) or the Extended Well-Known Text (EWKT)
 * representation of a temporal point array
 * @sqlfunc asText(), asEWKT()
 */
char **
tpointarr_as_text(const Temporal **temparr, int count, int maxdd,
  bool extended)
{
  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = extended ? tpoint_as_ewkt(temparr[i], maxdd) :
      tpoint_as_text(temparr[i], maxdd);
  return result;
}

/*****************************************************************************/
