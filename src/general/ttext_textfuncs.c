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
 * @file ttext_textfuncs.c
 * @brief Temporal text functions: `textcat`, `lower`, `upper`.
 */

#include "general/ttext_textfuncs.h"

/* PostgreSQL */
#include "utils/formatting.h"
/* MobilityDB */
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "general/lifting.h"

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
  result = (text *) palloc(len);

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

  out_string = str_tolower(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
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
  return PointerGetDatum(pg_lower(DatumGetTextP(value)));
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

  out_string = str_toupper(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    DEFAULT_COLLATION_OID);
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
  return PointerGetDatum(pg_upper(DatumGetTextP(value)));
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Apply the function to transform the temporal text value
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
 * Apply the function to the temporal text value and the base text value
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
  lfinfo.reslinear = STEP;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * Apply the function to the temporal text value and the base text value.
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
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Textcat_text_ttext);
/**
 * Return the concatenation of the text value and the temporal text values
 */
PGDLLEXPORT Datum
Textcat_text_ttext(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Textcat_ttext_text);
/**
 * Return the concatenation of the temporal text value and the text value
 */
PGDLLEXPORT Datum
Textcat_ttext_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Textcat_ttext_ttext);
/**
 * Return the concatenation of the two temporal text values
 */
PGDLLEXPORT Datum
Textcat_ttext_ttext(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = textfunc_ttext_ttext(temp1, temp2, &datum_textcat);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Ttext_upper);
/**
 * Transform the temporal text value into uppercase
 */
PGDLLEXPORT Datum
Ttext_upper(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = textfunc_ttext(temp, &datum_upper);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Ttext_lower);
/**
 * Transform the temporal text value into lowercase
 */
PGDLLEXPORT Datum
Ttext_lower(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = textfunc_ttext(temp, &datum_lower);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
