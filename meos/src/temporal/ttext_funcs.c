/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Temporal text functions
 */

#include "temporal/ttext_funcs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "utils/formatting.h"
#include "utils/varlena.h"
#include <pgtypes.h>

/*****************************************************************************
 * Generic functions on temporal texts
 *****************************************************************************/

/**
 * @brief Apply the function to transform the temporal text value
 */
Temporal *
textfunc_ttext(const Temporal *temp, datum_func1 func)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(func);
  assert(temp->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.argtype[0] = T_TTEXT;
  lfinfo.restype = T_TTEXT;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @brief Apply the function to the temporal text value and the base text value
 */
Temporal *
textfunc_ttext_text(const Temporal *temp, Datum value, datum_func2 func,
  bool invert)
{
  /* Ensure the validity of the arguments */
  assert(temp);
  assert(temp->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.argtype[0] = T_TTEXT;
  lfinfo.argtype[1] = T_TEXT;
  lfinfo.restype = T_TTEXT;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Apply the function to the temporal text value and the base text value
 */
Temporal *
textfunc_ttext_ttext(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
  /* Ensure the validity of the arguments */
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(temp1->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.argtype[0] = lfinfo.argtype[1] = T_TTEXT;
  lfinfo.restype = T_TTEXT;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/

/*****************************************************************************
 * Text base type functions
 *****************************************************************************/

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
 * @p PG_GET_COLLATION().
 *
 * In standalone MEOS this is an ASCII-only fold: bytes 0x41..0x5A
 * become 0x61..0x7A; bytes >= 0x80 (UTF-8 continuation/lead bytes)
 * are left unchanged. So `text_lower("ZÜRICH")` returns `"zÜRICH"`
 * (the ASCII letters are folded but `Ü` is preserved verbatim). This
 * is the documented UTF-8 contract — see meos.h.
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
 * @p PG_GET_COLLATION().
 *
 * In standalone MEOS this is an ASCII-only fold: bytes 0x61..0x7A
 * become 0x41..0x5A; bytes >= 0x80 (UTF-8 continuation/lead bytes)
 * are left unchanged. So `text_upper("Zürich")` returns `"ZüRICH"`,
 * not `"ZÜRICH"`. This is the documented UTF-8 contract — see meos.h.
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
 * @p PG_GET_COLLATION().
 *
 * In standalone MEOS this is an ASCII-only fold; bytes >= 0x80 are
 * passed through unchanged. See meos.h for the UTF-8 contract.
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
