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
 * @brief Static buffer type
 */

#include "cbuffer/tcbuffer.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "geo/pgis_types.h"
#include "geo/tgeo.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "general/type_parser.h"
#include "geo/tgeo_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
cbufferset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_CBUFFERSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a circular buffer set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
cbufferset_out(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_CBUFFERSET))
    return NULL;
  return set_out(s, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular
 * buffer set
 * @csqlfn #Cbufferset_as_text()
 */
char *
cbufferset_as_text(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_CBUFFERSET))
    return NULL;
  return set_out_fn(s, maxdd, &cbuffer_wkt_out);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a 
 * circular buffer set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbufferset_as_ewkt()
 */
char *
cbufferset_as_ewkt(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_spatialset_type(s->settype))
    return NULL;

  int32_t srid = spatialset_srid(s);
  char str1[18];
  if (srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d;", srid);
  else
    str1[0] = '\0';
  char *str2 = set_out_fn(s, maxdd, &cbuffer_wkt_out);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a circular buffer set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
cbufferset_make(const Cbuffer **values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_CBUFFER, ORDER);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the start value of a circular buffer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
Cbuffer *
cbufferset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_CBUFFERSET))
    return NULL;
  return DatumGetCbufferP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the end value of a circular buffer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
Cbuffer *
cbufferset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_CBUFFERSET))
    return NULL;
  return DatumGetCbufferP(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument a copy of the n-th value of a circular
 * buffer set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
cbufferset_value_n(const Set *s, int n, Cbuffer **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_CBUFFERSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetCbufferP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of copies of the values of a circular buffer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
Cbuffer **
cbufferset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_CBUFFERSET))
    return NULL;

  Cbuffer **result = palloc(sizeof(Cbuffer *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetCbufferP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/*****************************************************************************
 * Conversions functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a circular buffer converted to a circular buffer set
 * @param[in] cbuf Value
 * @csqlfn #Value_to_set()
 */
Set *
cbuffer_to_set(const Cbuffer *cbuf)
{
#if MEOS
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */

  Datum v = PointerGetDatum(cbuf);
  return set_make_exp(&v, 1, 1, T_CBUFFER, ORDER_NO);
}

/*****************************************************************************/
