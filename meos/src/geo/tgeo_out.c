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
 * @brief Output of temporal geos in WKT, EWKT, and MF-JSON format
 */

#include "geo/tgeo_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a geometry/geography in the Well-Known Text (WKT)
 * representation
 * @note The parameter @p type is not needed for geometries/geographies
 */
char *
geo_wkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
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
 * @brief Output a geometry/geography in the Extended Well-Known Text (EWKT)
 * representation, that is, in WKT representation prefixed with the SRID
 * @note The parameter @p type is not needed for geometries/geographies
 */
char *
geo_ewkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
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

/*****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a geo set
 * @csqlfn #Geoset_as_text()
 */
char *
geoset_as_text(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return set_out_fn(s, maxdd, &geo_wkt_out);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a geo set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Geoset_as_ewkt()
 */
char *
geoset_as_ewkt(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return set_out_fn(s, maxdd, &geo_ewkt_out);
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal geo
 * @param[in] temp Temporal geo
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tgeo_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_tgeo_type_all(temp->temptype))
    return NULL;
  return temporal_out(temp, maxdd);
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal geo
 * @param[in] temp Temporal geo
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tgeo_as_text()
 */
char *
tgeo_as_text(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type_all(temp->temptype) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_string((TInstant *) temp, maxdd, &geo_wkt_out);
    case TSEQUENCE:
      return tsequence_to_string((TSequence *) temp, maxdd, false, 
        &geo_wkt_out);
    default: /* TSEQUENCESET */
      return tsequenceset_to_string((TSequenceSet *) temp, maxdd, 
        &geo_wkt_out);
  }
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal geo
 * @param[in] temp Temporal geo
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tgeo_as_ewkt()
 */
char *
tgeo_as_ewkt(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type_all(temp->temptype) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  int32_t srid = tspatial_srid(temp);
  char str1[18];
  if (srid > 0)
  {
    char delim = (MEOS_FLAGS_GET_INTERP(temp->flags) == STEP && 
      temptype_continuous(temp->temptype)) ? ',' : ';';
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d%c", srid, delim);
  }
  else
    str1[0] = '\0';
  char *str2 = tgeo_as_text(temp, maxdd);
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
    /* The geo_wkt_out and geo_ewkt_out functions do not use the second argument */
    result[i] = extended ?
      geo_ewkt_out(geoarr[i], 0, maxdd) : geo_wkt_out(geoarr[i], 0, maxdd);
  return result;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation
 * of an array of temporal geos
 * @param[in] temparr Array of temporal geos
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @csqlfn #Tgeoarr_as_text(), #Tgeoarr_as_ewkt()
 */
char **
tgeoarr_as_text(const Temporal **temparr, int count, int maxdd, bool extended)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temparr) || ! ensure_positive(count) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = extended ? tgeo_as_ewkt(temparr[i], maxdd) :
      tgeo_as_text(temparr[i], maxdd);
  return result;
}

/*****************************************************************************/
