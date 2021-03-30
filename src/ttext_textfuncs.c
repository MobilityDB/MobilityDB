/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Temporal text functions: `textcat`, `lower`, `upper`.
 */

#include "ttext_textfuncs.h"

#include <utils/builtins.h>
#include "temporal.h"
#include "temporal_util.h"
#include "lifting.h"

/*****************************************************************************
 * Textual functions on datums
 *****************************************************************************/

/**
 * Returns the concatenation of the two text values
 */
static Datum
datum_textcat(Datum l, Datum r)
{
  return call_function2(textcat, l, r);
}

/**
 * Convert the text value to lowercase
 */
static Datum
datum_lower(Datum value)
{
  return call_function1(lower, value);
}

/**
 * Convert the text value to uppercase
 */
static Datum
datum_upper(Datum value)
{
  return call_function1(upper, value);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Applies the function to transform the temporal text value
 */
static Temporal *
textfunc_ttext(Temporal *temp, Datum (*func)(Datum value))
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.restypid = TEXTOID;
  return tfunc_temporal(temp, (Datum) NULL, lfinfo);
}

/**
 * Applies the function to the temporal text value and the base text value
 */
static Temporal *
textfunc_ttext_text(Temporal *temp, Datum value, datum_func2 func,
  bool invert)
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 2;
  lfinfo.restypid = TEXTOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, value, TEXTOID, (Datum) NULL, lfinfo);
}

/**
 * Applies the function to the temporal text value and the base text value
 */
static Temporal *
textfunc_ttext_ttext(Temporal *temp1, Temporal *temp2, datum_func2 func)
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 2;
  lfinfo.restypid = TEXTOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = NULL;
  return sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL, lfinfo);
}

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(textcat_base_ttext);
/**
 * Returns the concatenation of the text value and the temporal text values
 */
PGDLLEXPORT Datum
textcat_base_ttext(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(textcat_ttext_base);
/**
 * Returns the concatenation of the temporal text value and the text value
 */
PGDLLEXPORT Datum
textcat_ttext_base(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_DATUM(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(textcat_ttext_ttext);
/**
 * Returns the concatenation of the two temporal text values
 */
PGDLLEXPORT Datum
textcat_ttext_ttext(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Temporal *result = textfunc_ttext_ttext(temp1, temp2, &datum_textcat);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_upper);
/**
 * Transform the temporal text value into uppercase
 */
PGDLLEXPORT Datum
ttext_upper(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = textfunc_ttext(temp, &datum_upper);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_lower);
/**
 * Transform the temporal text value into lowercase
 */
PGDLLEXPORT Datum
ttext_lower(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = textfunc_ttext(temp, &datum_lower);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
