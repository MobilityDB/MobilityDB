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
 * @file temporal_out.c
 * @brief Output of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB format.
 */

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"
#include "general/temporal_out.h"
/* MobilityDB */
#include "pg_general/temporal_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_as_text);
/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Temporal_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_out(temp);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporalarr_as_text);
/**
 * Output a temporal array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
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

  Temporal **temparr = temporalarr_extract(array, &count);
  char **strarr = temporalarr_out((const Temporal **) temparr, count);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_as_mfjson);
/**
 * Return the temporal value represented in MF-JSON format
 */
PGDLLEXPORT Datum
Temporal_as_mfjson(PG_FUNCTION_ARGS)
{
  bool with_bbox = 0;
  int precision = DBL_DIG;
  int option = 0;
  char *srs = NULL;

  /* Get the temporal value */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool isgeo = tgeo_type(temp->temptype);

  /* Retrieve output option
   * 0 = without option (default)
   * 1 = bbox
   * 2 = short crs, only for temporal points
   * 4 = long crs, only for temporal points
   */
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    option = PG_GETARG_INT32(1);

  if (isgeo)
  {
    /* Even if the option does not request to output the crs, we output the
     * short crs when the SRID is different from SRID_UNKNOWN. Otherwise,
     * it is not possible to reconstruct the temporal point from the output
     * of this function without loosing the SRID */
    int32_t srid = tpoint_srid(temp);
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

  /* Retrieve precision if any (default is max) */
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
  {
    precision = PG_GETARG_INT32(2);
    if (precision > DBL_DIG)
      precision = DBL_DIG;
    else if (precision < 0)
      precision = 0;
  }

  char *mfjson = temporal_as_mfjson(temp, with_bbox, precision, srs);
  text *result = cstring2text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in WKB format
 *****************************************************************************/

/**
 * @brief Ensure that a string represents a valid endian flag
 */
static uint8_t
get_endian_variant(const text *txt)
{
  uint8_t variant = 0;
  const char *endian = text2cstring(txt);
  if (strncasecmp(endian, "ndr", 3) != 0 && strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  if (strncasecmp(endian, "ndr", 3) == 0)
    variant = variant | (uint8_t) WKB_NDR;
  else /* txt = XDR */
    variant = variant | (uint8_t) WKB_XDR;
  return variant;
}

/**
 * @brief Output a generic value in WKB or EWKB format
 */
static bytea *
datum_as_wkb_ext(FunctionCallInfo fcinfo, Datum value, mobdbType type,
  bool extended)
{
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
  {
    text *txt = PG_GETARG_TEXT_P(1);
    variant = get_endian_variant(txt);
  }

  /* Create WKB string */
  size_t wkb_size;
  uint8_t *wkb = extended ?
    datum_as_wkb(value, type, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    datum_as_wkb(value, type, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);

  /* Clean up and return */
  pfree(wkb);
  return result;
}

/**
 * @brief Output a generic value in WKB or EWKB format as hex-encoded ASCII
 */
static text *
datum_as_hexwkb_ext(FunctionCallInfo fcinfo, Datum value, mobdbType type)
{
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (! PG_ARGISNULL(1)))
  {
    text *txt = PG_GETARG_TEXT_P(1);
    variant = get_endian_variant(txt);
  }

  /* Create WKB hex string */
  size_t hexwkb_size;
  char *hexwkb = datum_as_hexwkb(value, type, variant, &hexwkb_size);
  text *result = cstring2text(hexwkb);
  pfree(hexwkb);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_as_wkb);
/**
 * Output a span in WKB format.
 */
PGDLLEXPORT Datum
Span_as_wkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(s), s->spantype,
    false);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Span_as_hexwkb);
/**
 * Output a span in HexWKB format.
 */
PGDLLEXPORT Datum
Span_as_hexwkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(s), s->spantype);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_as_wkb);
/**
 * Output a timestamp set in WKB format.
 */
PGDLLEXPORT Datum
Timestampset_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(ts),
    T_TIMESTAMPSET, false);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Timestampset_as_hexwkb);
/**
 * Output the timestamp set in HexWKB format.
 */
PGDLLEXPORT Datum
Timestampset_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(ts),
    T_TIMESTAMPSET);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_as_wkb);
/**
 * Output a period set in WKB format.
 */
PGDLLEXPORT Datum
Periodset_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(ps),
    T_PERIODSET, false);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Periodset_as_hexwkb);
/**
 * Output the period set in HexWKB format.
 */
PGDLLEXPORT Datum
Periodset_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(ps),
    T_PERIODSET);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_as_wkb);
/**
 * Output a temporal box in WKB format.
 */
PGDLLEXPORT Datum
Tbox_as_wkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  bytea *result = datum_as_wkb_ext(fcinfo, box, T_TBOX, false);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tbox_as_hexwkb);
/**
 * Output a temporal box in HexWKB format.
 */
PGDLLEXPORT Datum
Tbox_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  text *result = datum_as_hexwkb_ext(fcinfo, box, T_TBOX);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_as_wkb);
/**
 * Output a spatiotemporal box in WKB format.
 */
PGDLLEXPORT Datum
Stbox_as_wkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  bytea *result = datum_as_wkb_ext(fcinfo, box, T_STBOX, false);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Stbox_as_hexwkb);
/**
 * Output a spatiotemporal box in HexWKB format.
 */
PGDLLEXPORT Datum
Stbox_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  text *result = datum_as_hexwkb_ext(fcinfo, box, T_STBOX);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_as_wkb);
/**
 * @brief Output a temporal value in WKB format.
 * @note This will have no 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Temporal_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkb);
/**
 * @brief Output a temporal point in EWKB format.
 * @note This will have 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Tpoint_as_ewkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype, true);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_as_hexwkb);
/**
 * @brief Output a temporal value in HexEWKB format.
 * @note This will have 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Temporal_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/
