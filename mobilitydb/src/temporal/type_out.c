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
 * @brief Output of types in WKT, EWKT, WKB, EWKB, HexWKB, and MF-JSON
 * representation.
 */

/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Output in WKT and EWKT representation
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal value
 * @sqlfn asText()
 */
Datum
Temporal_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = temporal_out(temp, dbl_dig_for_wkt);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporalarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporalarr_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * temporal values
 * @sqlfn asText()
 */
Datum
Temporalarr_as_text(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);

  Temporal **temparr = temparr_extract(array, &count);
  char **strarr = temparr_out(temparr, count, Int32GetDatum(dbl_dig_for_wkt));
  ArrayType *result = strarr_to_textarray(strarr, count);
  /* We cannot use pfree_array */
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Output in Moving Features JSON MF-JSON representation
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_as_mfjson(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_as_mfjson);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Moving-Features JSON (MF-JSON) representation of a
 * temporal value
 * representation
 * @sqlfn asMFJSON()
 */
Datum
Temporal_as_mfjson(PG_FUNCTION_ARGS)
{
  bool with_bbox = 0;
  int precision = OUT_DEFAULT_DECIMAL_DIGITS;
  int option = 0;
  int flags = 0;
  char *srs = NULL;

  /* Get the temporal value */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);

  /* Retrieve output option
   * 0 = without option (default)
   * 1 = bbox
   * 2 = short crs, only for temporal points
   * 4 = long crs, only for temporal points
   */
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
   option = PG_GETARG_INT32(1);

  if (tspatial_type(temp->temptype))
  {
    /* Even if the option does not request to output the crs, we output the
     * short crs when the SRID is different from SRID_UNKNOWN. Otherwise,
     * it is not possible to reconstruct the temporal point from the output
     * of this function without loosing the SRID */
    int32_t srid = tspatial_srid(temp);
    if (srid != SRID_UNKNOWN && !(option & 2) && !(option & 4))
      option |= 2;
    if (srid != SRID_UNKNOWN)
    {
      if (option & 2)
        srs = getSRSbySRID(fcinfo, srid, true);
      else if (option & 4)
        srs = getSRSbySRID(fcinfo, srid, false);
      if (! srs)
      {
        elog(ERROR, "SRID %i unknown in spatial_ref_sys table", srid);
        PG_RETURN_NULL();
      }
    }
  }

  if (option & 1)
    with_bbox = 1;

  /* Retrieve JSON flags (e.g. for pretty print) if any (default is 0) */
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    flags = PG_GETARG_INT32(2);

  /* Retrieve precision if any (default is max) */
  if (PG_NARGS() > 3 && !PG_ARGISNULL(3))
  {
    precision = PG_GETARG_INT32(3);
    if (precision > OUT_DEFAULT_DECIMAL_DIGITS)
      precision = OUT_DEFAULT_DECIMAL_DIGITS;
    else if (precision < 0)
      precision = 0;
  }

  char *mfjson = temporal_as_mfjson(temp, with_bbox, flags, precision, srs);
  text *result = cstring_to_text(mfjson);
  // CURRENTLY we cannot pfree since we get the error message
  // pfree called with invalid pointer
  // pfree(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in (Extended) Well-Known Binary (WKB or EWKB) representation
 *****************************************************************************/

/**
 * @brief Ensure that a string represents a valid endian flag
 */
static uint8_t
get_endian_variant(const text *txt)
{
  uint8_t variant = 0;
  char *endian = text_to_cstring(txt);
  /* When the endian is not given the default value is an empty text */
  if (strlen(endian) == 0)
    ;
  else if (pg_strncasecmp(endian, "ndr", 3) != 0 &&
      pg_strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  else if (pg_strncasecmp(endian, "ndr", 3) == 0)
    variant = variant | (uint8_t) WKB_NDR;
  else /* txt = XDR */
    variant = variant | (uint8_t) WKB_XDR;
  pfree(endian);
  return variant;
}

/**
 * @brief Output a value in the (Extended) Well-Known Binary (WKB or EWKB)
 * representation
 */
bytea *
Datum_as_wkb(FunctionCallInfo fcinfo, Datum value, meosType type,
  bool extended)
{
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if (! PG_ARGISNULL(1))
  {
    text *txt = PG_GETARG_TEXT_P(1);
    variant = get_endian_variant(txt);
  }
  if (extended)
    variant |= (uint8_t) WKB_EXTENDED;

  /* Create WKB string */
  size_t wkb_size;
  uint8_t *wkb = datum_as_wkb(value, type, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  return result;
}

/**
 * @brief Output a value in the Well-Known Binary (WKB) or Extended Well-Known
 * Binary (EWKB) representation in ASCII hex-encoded
 */
text *
Datum_as_hexwkb(FunctionCallInfo fcinfo, Datum value, meosType type)
{
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if (! PG_ARGISNULL(1))
  {
    text *txt = PG_GETARG_TEXT_P(1);
    variant = get_endian_variant(txt);
  }

  /* Create WKB hex string */
  size_t hexwkb_size;
  char *hexwkb = datum_as_hexwkb(value, type, variant, &hexwkb_size);
  text *result = cstring_to_text(hexwkb);
  pfree(hexwkb);
  return result;
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_as_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal value
 * @note This will have no 'SRID=#;' for temporal points
 * @sqlfn asBinary()
 */
Datum
Temporal_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = Datum_as_wkb(fcinfo, PointerGetDatum(temp), temp->temptype,
    false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Temporal_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_as_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a temporal value
 * @note This will have 'SRID=#;' for temporal points
 * @sqlfn asHexWKB()
 */
Datum
Temporal_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *result = Datum_as_hexwkb(fcinfo, PointerGetDatum(temp),
    temp->temptype);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/
