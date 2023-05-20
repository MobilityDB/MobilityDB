/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Output of temporal points in WKT, EWKT, and MF-JSON format.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/type_util.h"

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a temporal point in Well-Known Text (WKT) format
 * @sqlfunc asText()
 */
static Datum
Tpoint_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ?
    tpoint_as_ewkt(temp, dbl_dig_for_wkt) :
    tpoint_as_text(temp, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Tpoint_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a temporal point in Well-Known Text (WKT) format
 * @sqlfunc asText()
 */
Datum
Tpoint_as_text(PG_FUNCTION_ARGS)
{
  return Tpoint_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Tpoint_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 * @sqlfunc asEWKT()
 */
Datum
Tpoint_as_ewkt(PG_FUNCTION_ARGS)
{
  return Tpoint_as_text_ext(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Output a geometry/geography or temporal geometry/geography point
 * array in Well-Known Text (WKT) format
 */
static Datum
geoarr_as_text_ext(FunctionCallInfo fcinfo, bool temporal, bool extended)
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

  char **strarr;
  if (temporal)
  {
    Temporal **temparr = temporalarr_extract(array, &count);
    strarr = tpointarr_as_text((const Temporal **) temparr, count,
      dbl_dig_for_wkt, extended);
    pfree(temparr);
  }
  else
  {
    Datum *geoarr = datumarr_extract(array, &count);
    strarr = geoarr_as_text(geoarr, count, dbl_dig_for_wkt, extended);
    pfree(geoarr);
  }
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Geoarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoarr_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a geometry/geography array in Well-Known Text (WKT) format
 * @sqlfunc asText()
 */
Datum
Geoarr_as_text(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, false, false);
}

PGDLLEXPORT Datum Geoarr_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoarr_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a geometry/geography array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 * @sqlfunc asEWKT()
 */
Datum
Geoarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tpointarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointarr_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a temporal point array in Well-Known Text (WKT) format
 * @sqlfunc asText()
 */
Datum
Tpointarr_as_text(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tpointarr_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointarr_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Output a temporal point array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 * @sqlfunc asEWKT()
 */
Datum
Tpointarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, true, true);
}

/*****************************************************************************/
