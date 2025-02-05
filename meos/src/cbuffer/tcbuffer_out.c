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
 * @brief Output of temporal circular buffers in WKT, EWKT, and MF-JSON format
 */

#include "cbuffer/tcbuffer_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "cbuffer/tcbuffer.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a circular buffer in the Well-Known Text (WKT) representation
 * @note The parameter @p type is not needed for temporal points
 */
char *
cbuffer_wkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
{
  Cbuffer *cbuf = DatumGetCbufferP(value);
  Datum d = PointerGetDatum(&cbuf->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_ISO, maxdd, &len);
  char *radius = float8_out(cbuf->radius, maxdd);
  len += strlen(radius) + 11; // Cbuffer(,) + end NULL
  char *result = palloc(len);
  snprintf(result, len, "Cbuffer(%s,%s)", wkt, radius);
  lwgeom_free(geom); pfree(wkt); pfree(radius);
  return result;
}

/**
 * @brief Output a circular buffer in the Extended Well-Known Text (EWKT)
 * representation, that is, in WKT representation prefixed with the SRID
 * @note The parameter @p type is not needed for temporal points
 */
char *
cbuffer_ewkt_out(Datum value, meosType type __attribute__((unused)), int maxdd)
{
  Cbuffer *cbuf = DatumGetCbufferP(value);
  Datum d = PointerGetDatum(&cbuf->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_EXTENDED, maxdd, &len);
  char *radius = float8_out(cbuf->radius, maxdd);
  len += strlen(radius) + 11; // Cbuffer(,) + end NULL
  char *result = palloc(len);
  snprintf(result, len, "Cbuffer(%s,%s)", wkt, radius);
  lwgeom_free(geom); pfree(wkt); pfree(radius);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_as_text()
 */
char *
cbuffer_as_text(const Cbuffer *cbuf, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_negative(maxdd))
    return NULL;

  return cbuffer_wkt_out(PointerGetDatum(cbuf), 0, maxdd);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tcbuffer_as_ewkt()
 */
char *
cbuffer_as_ewkt(const Cbuffer *cbuf, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_negative(maxdd))
    return NULL;

  return cbuffer_ewkt_out(PointerGetDatum(cbuf), 0, maxdd);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tcbuffer_as_text()
 */
char *
tcbuffer_as_text(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_string((TInstant *) temp, maxdd, &cbuffer_wkt_out);
    case TSEQUENCE:
      return tsequence_to_string((TSequence *) temp, maxdd, false, &cbuffer_wkt_out);
    default: /* TSEQUENCESET */
      return tsequenceset_to_string((TSequenceSet *) temp, maxdd, &cbuffer_wkt_out);
  }
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tcbuffer_as_ewkt()
 */
char *
tcbuffer_as_ewkt(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  int32_t srid = tcbuffer_srid(temp);
  char str1[18];
  if (srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d%c", srid,
      (MEOS_FLAGS_GET_INTERP(temp->flags) == STEP) ? ',' : ';');
  else
    str1[0] = '\0';
  char *str2 = tcbuffer_as_text(temp, maxdd);
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
 * of a circular buffer array
 * @param[in] cbufarr Array of cbuffer
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @csqlfn #Cbufferarr_as_text()
 */
char **
cbufferarr_as_text(const Datum *cbufarr, int count, int maxdd, bool extended)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbufarr) || ! ensure_positive(count) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(char *) * count);
  for (int i = 0; i < count; i++)
    /* The wkt_out and ewkt_out functions do not use the second argument */
    result[i] = extended ? cbuffer_ewkt_out(cbufarr[i], 0, maxdd) : 
      cbuffer_wkt_out(cbufarr[i], 0, maxdd);
  return result;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation
 * of an array of temporal circular buffers
 * @param[in] temparr Array of temporal circular buffers
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 * @csqlfn #Tcbufferarr_as_text(), #Tcbufferarr_as_ewkt()
 */
char **
tcbufferarr_as_text(const Temporal **temparr, int count, int maxdd,
  bool extended)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temparr) || ! ensure_positive(count) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = extended ? tcbuffer_as_ewkt(temparr[i], maxdd) :
      tcbuffer_as_text(temparr[i], maxdd);
  return result;
}

/*****************************************************************************/
