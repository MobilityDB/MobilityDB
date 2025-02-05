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
 * @brief Basic functions for temporal network points.
 */

#include "cbuffer/tcbuffer.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_parser.h"
#include "general/type_round.h"
#include "general/type_util.h"
#include "geo/tgeo_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#define TCBUFFER_MAX_TYPMOD 2

/**
 * @brief Check the consistency of the metadata specified in the typmod: SRID,
 * type, and dimensionality. If things are inconsistent, shut down the query.
 */
static Temporal *
tcbuffer_valid_typmod(Temporal *temp, int32_t typmod)
{
  int32 srid = tspatial_srid(temp);
  uint8 subtype = temp->subtype;
  uint8 typmod_subtype = TYPMOD_GET_TEMPSUBTYPE(typmod);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);

  /* No typmod (-1) */
  if (typmod < 0 && typmod_subtype == ANYTEMPSUBTYPE)
    return temp;
  /* Typmod has a preference for SRID? Circular buffer SRID had better match */
  if (typmod_srid > 0 && typmod_srid != srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal circular buffer SRID (%d) does not match column SRID (%d)",
        srid, typmod_srid) ));
  /* Typmod has a preference for temporal subtype */
  if (typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal subtype (%s) does not match column type (%s)",
        tempsubtype_name(subtype), tempsubtype_name(typmod_subtype)) ));

  return temp;
}

PGDLLEXPORT Datum Tcbuffer_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from its Well-Known Text (WKT) representation
 * @sqlfn tcbuffer_in()
 */
Datum
Tcbuffer_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Temporal *result = tcbuffer_parse(&input);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Input typmod information for temporal geos
 */
static uint32
tcbuffer_typmod_in(ArrayType *arr)
{
  uint32 typmod = 0;
  Datum *elem_values;
  int n = 0;

  if (ARR_ELEMTYPE(arr) != CSTRINGOID)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("typmod array must be type cstring[]")));
  if (ARR_NDIM(arr) != 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("typmod array must be one-dimensional")));
  if (ARR_HASNULL(arr))
    ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
      errmsg("typmod array must not contain nulls")));

  /*
   * There are several ways to define a column wrt type modifiers:
   *   column_type(TempSubType, SRID) => All modifiers are determined.
   *   column_type(TempSubType) => The SRID is generic.
   *   column_type => The temporal type and SRID are generic.
   *
   * For example, if the user did not set the temporal type, we can use any
   * temporal type in the same column. Similarly for all generic modifiers.
   */
  deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
  if (n > TCBUFFER_MAX_TYPMOD)
    elog(ERROR, "Incorrect number of type modifiers for temporal points");

  /* Set default values for typmod if they are not given */
  int16 tempsubtype = ANYTEMPSUBTYPE;
  int srid = SRID_UNKNOWN;
  bool has_srid = false;

  /* Get the string values from the input array */
  char *s[2] = {0,0};
  for (int i = 0; i < n; i++)
  {
    s[i] = DatumGetCString(elem_values[i]);
    if (strlen(s[i]) == 0)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Empty temporal subtype modifier")));
  }

  /* Extract the typmod values */
  if (n == 2)
  {
    /* Type_modifier is (TempSubType, SRID) */
    if (tempsubtype_from_string(s[0], &tempsubtype) == false)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal type modifier: %s", s[0])));
#if POSTGRESQL_VERSION_NUMBER >= 150000
    srid = pg_strtoint32(s[1]);
#else
    srid = pg_atoi(s[1], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
    srid = clamp_srid(srid);
    has_srid = true;
  }
  else if (n == 1)
  {
    /* Type modifier: either (TempSubType) or (SRID) */
    has_srid = false;
    if (tempsubtype_from_string(s[0], &tempsubtype))
    {
      ;
    }
    else
    {
#if POSTGRESQL_VERSION_NUMBER >= 150000
      srid = pg_strtoint32(s[0]);
#else
      srid = pg_atoi(s[0], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
      has_srid = true;
    }
  }
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid temporal point type modifier:")));

  /* Set the temporal type */
  if (tempsubtype != ANYTEMPSUBTYPE)
    TYPMOD_SET_TEMPSUBTYPE(typmod, tempsubtype);

  /* Set default SRID */
  TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

  /* Set the SRID */
  if (has_srid)
  {
    if (srid != SRID_UNKNOWN)
      TYPMOD_SET_SRID(typmod, srid);
  }

  pfree(elem_values);
  return typmod;
}

PGDLLEXPORT Datum Tcbuffer_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_typmod_in);
/**
 * @brief Input typmod information for temporal circular buffers
 */
Datum
Tcbuffer_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tcbuffer_typmod_in(array);
  PG_RETURN_INT32(typmod);
}

PGDLLEXPORT Datum Tcbuffer_typmod_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_typmod_out);
/**
 * @brief Output typmod information for temporal circular buffers
 */
Datum
Tcbuffer_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  int16 tempsubtype = TYPMOD_GET_TEMPSUBTYPE(typmod);
  int32 srid = TYPMOD_GET_SRID(typmod);

  /* No temporal subtype? Then no typmod at all. Return empty string. */
  if (typmod < 0 || (tempsubtype == ANYTEMPSUBTYPE))
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  /* Opening bracket */
  str += sprintf(str, "(");
  /* Has temporal subtype?  */
  if (tempsubtype != ANYTEMPSUBTYPE)
    str += sprintf(str, "%s", tempsubtype_name(tempsubtype));
  /* Has SRID?  */
  if (srid)
    str += sprintf(str, ",%d", srid);
  /* Closing bracket.  */
  sprintf(str, ")");

  PG_RETURN_CSTRING(s);
}

PGDLLEXPORT Datum Tcbuffer_enforce_typmod(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal circular buffers with 
 * respect to temporal subtype and SRID
 */
Datum
Tcbuffer_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if the typmod of the temporal circular buffer is consistent with the
   * supplied one */
  temp = tcbuffer_valid_typmod(temp, typmod);
  PG_RETURN_TEMPORAL_P(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_constructor);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from a temporal point and a temporal float
 * @sqlfn tcbuffer_constructor()
 */
Datum
Tcbuffer_constructor(PG_FUNCTION_ARGS)
{
  Temporal *point = PG_GETARG_TEMPORAL_P(0);
  Temporal *radius = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tcbuffer_constructor(point, radius);
  PG_FREE_IF_COPY(point, 0);
  PG_FREE_IF_COPY(radius, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal geometry point constructed from the points of a 
 * temporal circular buffer
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_to_tfloat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_tfloat);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeompoint_to_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal geometry point converted to a temporal circular
 * buffer with a zero radius
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tgeompoint_to_tcbuffer(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal circular buffer with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Tcbuffer_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tcbuffer_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Cbufferset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferset_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a buffer set with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Cbufferset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Set *result = cbufferset_round(s, size);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_points);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of points of a temporal circular buffer
 * @sqlfn points()
 */
Datum
Tcbuffer_points(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tcbuffer_points(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

// PGDLLEXPORT Datum Tcbuffer_line(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_line);
// /**
 // * @ingroup mobilitydb_temporal_accessor
 // * @brief Return the central line of a temporal circular buffer
 // * @sqlfn point()
 // */
// Datum
// Tcbuffer_line(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // GSERIALIZED *result = tcbuffer_line(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_GSERIALIZED_P(result);
// }

/*****************************************************************************/
