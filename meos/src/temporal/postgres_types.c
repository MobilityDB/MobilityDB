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
 * @brief Functions for base and time types corresponding to external
 * PostgreSQL functions in order to bypass the function manager @p fmgr.c.
 */

/* C */
#include <float.h>
#include <math.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <common/hashfn.h>
#include <common/int.h>
#include <common/int128.h>
#include <utils/datetime.h>
#include <utils/float.h>
#include "utils/formatting.h"
#include "utils/timestamp.h"
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/varlena.h"
/* PostGIS */
#include <liblwgeom_internal.h> /* for OUT_DOUBLE_BUFFER_SIZE */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

#if ! MEOS
  extern Datum call_function1(PGFunction func, Datum arg1);
  extern Datum call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3);
  extern Datum date_in(PG_FUNCTION_ARGS);
  extern Datum timestamp_in(PG_FUNCTION_ARGS);
  extern Datum timestamptz_in(PG_FUNCTION_ARGS);
  extern Datum date_out(PG_FUNCTION_ARGS);
  extern Datum timestamp_out(PG_FUNCTION_ARGS);
  extern Datum timestamptz_out(PG_FUNCTION_ARGS);
  extern Datum interval_out(PG_FUNCTION_ARGS);
#endif /* ! MEOS */

#if POSTGRESQL_VERSION_NUMBER >= 150000 || MEOS
  extern int64 pg_strtoint64(const char *s);
#else
  extern bool scanint8(const char *str, bool errorOK, int64 *result);
#endif


/*****************************************************************************
 * Text and binary string functions
 *****************************************************************************/

// /**
 // * @brief Convert a C binary string into a bytea
 // */
// bytea *
// bstring2bytea(const uint8_t *wkb, size_t size)
// {
  // bytea *result = palloc(size + VARHDRSZ);
  // memcpy(VARDATA(result), wkb, size);
  // SET_VARSIZE(result, size + VARHDRSZ);
  // return result;
// }

// /**
 // * @ingroup meos_base_text
 // * @brief Convert a C string into a text
 // * @param[in] str String
 // * @note Function taken from PostGIS file `lwgeom_in_geojson.c`
 // */
// text *
// cstring2text(const char *str)
// {
  // /* Ensure the validity of the arguments */
  // VALIDATE_NOT_NULL(str, NULL);

  // size_t len = strlen(str);
  // text *result = palloc(len + VARHDRSZ);
  // SET_VARSIZE(result, len + VARHDRSZ);
  // memcpy(VARDATA(result), str, len);
  // return result;
// }

// /**
 // * @ingroup meos_base_text
 // * @brief Convert a text into a C string
 // * @param[in] txt Text
 // * @note Function taken from PostGIS file @p lwgeom_in_geojson.c
 // */
// char *
// text2cstring(const text *txt)
// {
  // /* Ensure the validity of the arguments */
  // VALIDATE_NOT_NULL(txt, NULL);

  // size_t size = VARSIZE_ANY_EXHDR(txt);
  // char *str = palloc(size + 1);
  // memcpy(str, VARDATA(txt), size);
  // str[size] = '\0';
  // return str;
// }

/**
 * @ingroup meos_base_text
 * @brief Return the concatenation of the two text values
 * @param[in] txt1,txt2 Text values
 * @note Function adapted from the external function @p text_catenate in file
 * @p varlena.c
 */
text *
textcat_text_text(const text *txt1, const text *txt2)
{
  size_t len1 = VARSIZE_ANY_EXHDR(txt1);
  size_t len2 = VARSIZE_ANY_EXHDR(txt2);
  size_t len = len1 + len2 + VARHDRSZ;
  text *result = palloc(len);

  /* Set size of result string... */
  SET_VARSIZE(result, len);

  /* Fill data field of result string... */
  char *ptr = VARDATA(result);
  if (len1 > 0)
    memcpy(ptr, VARDATA_ANY(txt1), len1);
  if (len2 > 0)
    memcpy(ptr + len1, VARDATA_ANY(txt2), len2);

  return result;
}

/**
 * @brief Return the concatenation of the two text values
 */
Datum
datum_textcat(Datum l, Datum r)
{
  return PointerGetDatum(textcat_text_text(DatumGetTextP(l), DatumGetTextP(r)));
}

/**
 * @ingroup meos_base_text
 * @brief Return the text value transformed to lowercase
 * @param[in] txt Text value
 * @note PostgreSQL function: @p lower() in file @p varlena.c.
 * Notice that @p DEFAULT_COLLATION_OID is used instead of
 * @p PG_GET_COLLATION()
 */
text *
text_lower(const text *txt)
{
#if MEOS
  VALIDATE_NOT_NULL(txt, NULL);
  char *out_string = asc_tolower(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  char *out_string = str_tolower(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  text *result = cstring_to_text(out_string);
  pfree(out_string);
  return result;
}

/**
 * @brief Return the text value transformed to lowercase
 */
Datum
datum_lower(Datum value)
{
  return PointerGetDatum(text_lower(DatumGetTextP(value)));
}

/**
 * @ingroup meos_base_text
 * @brief Return the text value transformed to uppercase
 * @param[in] txt Text value
 * @note PostgreSQL function: @p upper() in file @p varlena.c.
 * Notice that @p DEFAULT_COLLATION_OID is used instead of
 * @p PG_GET_COLLATION()
 */
text *
text_upper(const text *txt)
{
#if MEOS
  VALIDATE_NOT_NULL(txt, NULL);
  char *out_string = asc_toupper(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  char *out_string = str_toupper(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  text *result = cstring_to_text(out_string);
  pfree(out_string);
  return result;
}

/**
 * @brief Return the text value transformed to uppercase
 */
Datum
datum_upper(Datum value)
{
  return PointerGetDatum(text_upper(DatumGetTextP(value)));
}

/**
 * @ingroup meos_base_text
 * @brief Convert the text value to initcap
 * @param[in] txt Text value
 * @note PostgreSQL function: @p initcap() in file @p varlena.c.
 * Notice that @p DEFAULT_COLLATION_OID is used instead of
 * @p PG_GET_COLLATION()
 */
text *
text_initcap(const text *txt)
{
#if MEOS
  VALIDATE_NOT_NULL(txt, NULL);
  char *out_string = asc_initcap(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  char *out_string = str_initcap(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  text *result = cstring_to_text(out_string);
  pfree(out_string);
  return result;
}

/**
 * @brief Convert the text value to uppercase
 */
Datum
datum_initcap(Datum value)
{
  return PointerGetDatum(text_initcap(DatumGetTextP(value)));
}

/*****************************************************************************/

