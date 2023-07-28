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
 * @brief Temporal text functions: `textcat`, `lower`, `upper`.
 */

#include "general/ttext_textfuncs.h"

/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/formatting.h"
/* MEOS */
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/type_util.h"

/*****************************************************************************
 * Textual functions on datums
 *****************************************************************************/

/*
 * @brief Return the concatenation of the two text values.
 *
 * Arguments can be in short-header form, but not compressed or out-of-line
 * @note Function adapted from the external function in varlena.c
 */
static text *
text_catenate(text *t1, text *t2)
{
  text *result;
  int len1, len2, len;
  char *ptr;

  len1 = VARSIZE_ANY_EXHDR(t1);
  len2 = VARSIZE_ANY_EXHDR(t2);

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
    memcpy(ptr, VARDATA_ANY(t1), len1);
  if (len2 > 0)
    memcpy(ptr + len1, VARDATA_ANY(t2), len2);

  return result;
}

/**
 * @brief Return the concatenation of the two text values.
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
  tmp = malloc(len + 1);
  if (tmp == NULL)
  {
    fprintf(stderr, "out of memory\n");
    exit(EXIT_FAILURE);
  }

  memcpy(tmp, in, len);
  tmp[len] = '\0';

  return tmp;
}

/*
 * ASCII-only lower function
 *
 * We pass the number of bytes so we can pass varlena and char*
 * to this function.  The result is a palloc'd, null-terminated string.
 */
char *
asc_tolower(const char *buff, size_t nbytes)
{
  char *result;
  char *p;

  if (!buff)
    return NULL;

  result = pnstrdup(buff, nbytes);

  for (p = result; *p; p++)
    *p = pg_ascii_tolower((unsigned char) *p);

  return result;
}

/*
 * ASCII-only upper function
 *
 * We pass the number of bytes so we can pass varlena and char*
 * to this function.  The result is a palloc'd, null-terminated string.
 */
char *
asc_toupper(const char *buff, size_t nbytes)
{
  char *result;
  char *p;

  if (!buff)
    return NULL;

  result = pnstrdup(buff, nbytes);

  for (p = result; *p; p++)
    *p = pg_ascii_toupper((unsigned char) *p);

  return result;
}

#endif /* MEOS */

/**
 * @brief Convert the text value to lowercase
 * @note Function adapted from the external function lower() in varlena.c.
 * Notice that `DEFAULT_COLLATION_OID` is used instead of `PG_GET_COLLATION()`.
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
 * @brief Convert the text value to lowercase
 */
Datum
datum_lower(Datum value)
{
  return pg_lower(DatumGetTextP(value));
}

/**
 * @brief Convert the text value to uppercase
 * @note Function adapted from the external function upper() in varlena.c.
 * Notice that `DEFAULT_COLLATION_OID` is used instead of `PG_GET_COLLATION()`.
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
 * @brief Convert the text value to uppercase
 */
Datum
datum_upper(Datum value)
{
  return pg_upper(DatumGetTextP(value));
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Apply the function to transform the temporal text value
 */
Temporal *
textfunc_ttext(const Temporal *temp, Datum (*func)(Datum value))
{
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
 * @brief Apply the function to the temporal text value and the base text value.
 */
Temporal *
textfunc_ttext_ttext(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
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
