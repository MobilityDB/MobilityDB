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
 * @file tpoint_out.c
 * @brief Output of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON
 * format.
 */

#include "point/tpoint_out.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
#include <utils/builtins.h>
/* PostGIS */
#if POSTGIS_VERSION_NUMBER >= 30000
#include <liblwgeom_internal.h>
#endif
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_as_text);
/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Tpoint_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = tpoint_as_text(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkt);
/**
 * Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Tpoint_as_ewkt(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = tpoint_as_ewkt(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
static Datum
geoarr_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }

  Datum *geoarr = datumarr_extract(array, &count);
  char **strarr = geoarr_as_text(geoarr, count, extended);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(geoarr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Geoarr_as_text);
/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Geoarr_as_text(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Geoarr_as_ewkt);
/**
 * Output a geometry/geography array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Geoarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, true);
}

/**
 * Output a temporal point array in Well-Known Text (WKT) or
 * Extended Well-Known Text (EWKT) format
 */
static Datum
tpointarr_as_text_ext(FunctionCallInfo fcinfo, bool extended)
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
  char **strarr = tpointarr_as_text((const Temporal **) temparr, count,
    extended);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Tpointarr_as_text);
/**
 * Output a temporal point array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Tpointarr_as_text(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tpointarr_as_ewkt);
/**
 * Output a temporal point array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Tpointarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text_ext(fcinfo, true);
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_as_mfjson);
/**
 * Return the temporal point represented in MF-JSON format
 */
PGDLLEXPORT Datum
Tpoint_as_mfjson(PG_FUNCTION_ARGS)
{
  int has_bbox = 0;
  int precision = DBL_DIG;
  int option = 0;
  char *srs = NULL;

  /* Get the temporal point */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);

  /* Retrieve precision if any (default is max) */
  if (PG_NARGS() > 1 && !PG_ARGISNULL(1))
  {
    precision = PG_GETARG_INT32(1);
    if (precision > DBL_DIG)
      precision = DBL_DIG;
    else if (precision < 0)
      precision = 0;
  }

  /* Retrieve output option
   * 0 = without option (default)
   * 1 = bbox
   * 2 = short crs
   * 4 = long crs
   */
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
    option = PG_GETARG_INT32(2);

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
  if (option & 1)
    has_bbox = 1;

  char *mfjson = tpoint_as_mfjson(temp, precision, has_bbox, srs);
  text *result = cstring_to_text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in WKB or EWKB format
 *
 * The format of the MobilityDB binary format builds upon the one of PostGIS.
 * In particular, many of the flags defined in liblwgeom.h such as WKB_NDR vs
 * WKB_XDR (for little- vs big-endian), WKB_EXTENDED (for the SRID), etc.
 * In addition, we need additional flags such as MOBDB_WKB_LINEAR_INTERP for
 * linear interporation, etc.
 *
 * The binary format obviously depends on the subtype of the temporal type
 * (instant, instant set, ...). The specific binary format is specified in
 * the function corresponding to the subtype below.
 *****************************************************************************/

/**
 * Ensures that the spatiotemporal boxes have the same type of coordinates,
 * either planar or geodetic
 */
static void
ensure_valid_endian_flag(const char *endian)
{
  if (strncasecmp(endian, "ndr", 3) != 0 && strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  return;
}

/**
 * Output the temporal point in WKB or EWKB format
 */
Datum
tpoint_as_binary_ext(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text2cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else /* type = XDR */
      variant = variant | (uint8_t) WKB_XDR;
  }

  /* Create WKB hex string */
  size_t wkb_size = VARSIZE_ANY_EXHDR(temp);
  uint8_t *wkb = extended ?
    tpoint_to_wkb(temp, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    tpoint_to_wkb(temp, variant, &wkb_size);

  /* Prepare the PostgreSQL text return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_binary);
/**
 * Output a temporal point in WKB format.
 * This will have no 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_binary(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkb);
/**
 * Output the temporal point in EWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_ewkb(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary_ext(fcinfo, true);
}

PG_FUNCTION_INFO_V1(Tpoint_as_hexewkb);
/**
 * Output the temporal point in HexEWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_hexewkb(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text2cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else
      variant = variant | (uint8_t) WKB_XDR;
  }

  /* Create WKB hex string */
  size_t hexwkb_size;
  char *hexwkb = tpoint_as_hexewkb(temp, variant, &hexwkb_size);

  /* Prepare the PgSQL text return type */
  size_t text_size = hexwkb_size - 1 + VARHDRSZ;
  text *result = palloc(text_size);
  memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
  SET_VARSIZE(result, text_size);

  /* Clean up and return */
  pfree(hexwkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/
