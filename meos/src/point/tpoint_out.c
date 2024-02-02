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
 * @brief Output of temporal points in WKT, EWKT, and MF-JSON format
 */

#include "point/tpoint_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a geometry in the Well-Known Text (WKT) representation
 * @note The parameter @p type is not needed for temporal points
 */
char *
wkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_ISO, maxdd, &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom); pfree(wkt);
  return result;
}

/**
 * @brief Output a geometry in the Extended Well-Known Text (EWKT)
 * representation, that is, in WKT representation prefixed with the SRID
 * @note The parameter @p type is not needed for temporal points
 */
char *
ewkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_EXTENDED, maxdd, &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom); pfree(wkt);
  return result;
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal point
 * @param[in] temp Temporal point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpoint_as_text()
 */
char *
tpoint_as_text(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_string((TInstant *) temp, maxdd, &wkt_out);
    case TSEQUENCE:
      return tsequence_to_string((TSequence *) temp, maxdd, false, &wkt_out);
    default: /* TSEQUENCESET */
      return tsequenceset_to_string((TSequenceSet *) temp, maxdd, &wkt_out);
  }
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal point
 * @param[in] temp Temporal point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpoint_as_ewkt()
 */
char *
tpoint_as_ewkt(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  int srid = tpoint_srid(temp);
  char str1[20];
  if (srid > 0)
    sprintf(str1, "SRID=%d%c", srid,
      (MEOS_FLAGS_GET_INTERP(temp->flags) == STEP) ? ',' : ';');
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
 * @ingroup meos_internal_temporal_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation
 * of a geometry/geography array
 * @param[in] geoarr Array of geometries/geographies
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @csqlfn #Geoarr_as_text(), #Geoarr_as_ewkt()
 */
char **
geoarr_as_text(const Datum *geoarr, int count, int maxdd, bool extended)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) geoarr) || ! ensure_positive(count) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(char *) * count);
  for (int i = 0; i < count; i++)
    /* The wkt_out and ewkt_out functions do not use the second argument */
    result[i] = extended ?
      ewkt_out(geoarr[i], 0, maxdd) : wkt_out(geoarr[i], 0, maxdd);
  return result;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation
 * of an array of temporal points
 * @param[in] temparr Array of temporal points
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @csqlfn #Tpointarr_as_text(), #Tpointarr_as_ewkt()
 */
char **
tpointarr_as_text(const Temporal **temparr, int count, int maxdd,
  bool extended)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temparr) || ! ensure_positive(count) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = extended ? tpoint_as_ewkt(temparr[i], maxdd) :
      tpoint_as_text(temparr[i], maxdd);
  return result;
}

/*****************************************************************************/
