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
 * @brief Temporal text functions
 */

#include "general/ttext_textfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/formatting.h"
/* MEOS */
#include "general/lifting.h"

/*****************************************************************************
 * Generic text functions
 *****************************************************************************/

/**
 * @brief Return the concatenation of the two text values
 * @note Function adapted from the external function in file @p varlena.c
 */
text *
text_catenate(const text *txt1, const text *txt2)
{
  text *result;
  int len1, len2, len;
  char *ptr;

  len1 = VARSIZE_ANY_EXHDR(txt1);
  len2 = VARSIZE_ANY_EXHDR(txt2);

  /* paranoia ... probably should throw error instead? */
  if (len1 < 0)
    len1 = 0;
  if (len2 < 0)
    len2 = 0;

  len = len1 + len2 + VARHDRSZ;
  result = palloc(len);

  /* Set size of result string... */
  SET_VARSIZE(result, len);

  /* Fill data field of result string... */
  ptr = VARDATA(result);
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
  return PointerGetDatum(text_catenate(DatumGetTextP(l), DatumGetTextP(r)));
}

#if MEOS
char *
pnstrdup(const char *in, Size size)
{
  char *tmp;
  int len;

  if (!in)
  {
    fprintf(stderr, "cannot duplicate null pointer (internal error)\n");
    exit(EXIT_FAILURE);
  }

  len = strnlen(in, size);
  tmp = palloc(len + 1);
  if (tmp == NULL)
  {
    fprintf(stderr, "out of memory\n");
    exit(EXIT_FAILURE);
  }

  memcpy(tmp, in, len);
  tmp[len] = '\0';

  return tmp;
}
#endif /* MEOS */

/**
 * @brief Return the text value transformed to lowercase
 * @note Function adapted from the external function @p lower() in file
 * @p varlena.c. Notice that @p DEFAULT_COLLATION_OID is used instead of
 * @p PG_GET_COLLATION().
 */
static Datum
pg_lower(text *txt)
{
  char *out_string;
  text *result;

#if MEOS
  out_string = asc_tolower(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  out_string = str_tolower(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  result = cstring2text(out_string);
  pfree(out_string);

  return PointerGetDatum(result);
}

/**
 * @brief Return the text value transformed to lowercase
 */
Datum
datum_lower(Datum value)
{
  return pg_lower(DatumGetTextP(value));
}

/**
 * @brief Return the text value transformed to uppercase
 * @note Function adapted from the external function @p upper() in file
 * @p varlena.c. Notice that @p DEFAULT_COLLATION_OID` is used instead of
 * @p PG_GET_COLLATION().
 */
Datum
pg_upper(text *txt)
{
  char *out_string;
  text *result;

#if MEOS
  out_string = asc_toupper(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  out_string = str_toupper(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  result = cstring2text(out_string);
  pfree(out_string);

  return PointerGetDatum(result);
}

/**
 * @brief Return the text value transformed to uppercase
 */
Datum
datum_upper(Datum value)
{
  return pg_upper(DatumGetTextP(value));
}

/**
 * @brief Convert the text value to uppercase
 * @note Function adapted from the external function @p upper() in file
 * @p varlena.c. Notice that @p DEFAULT_COLLATION_OID is used instead of
 * @p PG_GET_COLLATION().
 */
text *
pg_initcap(text *txt)
{
  char *out_string;
  text *result;

#if MEOS
  out_string = asc_initcap(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
#else /* ! MEOS */
  out_string = str_initcap(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
#endif /* MEOS */
  result = cstring2text(out_string);
  pfree(out_string);

  return result;
}

/**
 * @brief Convert the text value to uppercase
 */
Datum
datum_initcap(Datum value)
{
  return PointerGetDatum(pg_initcap(DatumGetTextP(value)));
}

/*****************************************************************************
 * Generic functions on temporal texts
 *****************************************************************************/

/**
 * @brief Apply the function to transform the temporal text value
 */
Temporal *
textfunc_ttext(const Temporal *temp, Datum (*func)(Datum value))
{
  /* Ensure validity of the arguments */
  assert(temp); assert(func);
  assert(temp->temptype == T_TTEXT);

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TTEXT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @brief Apply the function to the temporal text value and the base text value
 */
Temporal *
textfunc_ttext_text(const Temporal *temp, Datum value, datum_func2 func,
  bool invert)
{
  /* Ensure validity of the arguments */
  assert(temp);
  assert(temp->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Apply the function to the temporal text value and the base text value
 */
Temporal *
textfunc_ttext_ttext(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
  /* Ensure validity of the arguments */
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(temp1->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
