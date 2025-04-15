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
 * @brief Basic functions for temporal types of any subtype
 */

#include "general/temporal.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_meos.h"
#include "general/lifting.h"
#include "general/pg_types.h"
#include "general/temporal_boxops.h"
#include "general/temporal_tile.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tbool_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL); 
  return temporal_parse(&str, T_TBOOL);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal integer from its Well-Known Text (WKT) 
 * representation
 * @param[in] str String
 */
Temporal *
tint_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL); 
  return temporal_parse(&str, T_TINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
tfloat_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL); 
  return temporal_parse(&str, T_TFLOAT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
ttext_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL); 
  return temporal_parse(&str, T_TTEXT);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal boolean
 * @param[in] temp Temporal boolean
 */
char *
tbool_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL); 
  return temporal_out(temp, 0);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal integer
 * @param[in] temp Temporal integer
 */
char *
tint_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); 
  return temporal_out(temp, 0);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal float
 * @param[in] temp Temporal float
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tfloat_out(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); 
  return temporal_out(temp, maxdd);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal text
 * @param[in] temp Temporal text
 */
char *
ttext_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL); 
  return temporal_out(temp, 0);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean from a boolean and the time frame of
 * another temporal value
 * @param[in] b Value
 * @param[in] temp Temporal value
 */
Temporal *
tbool_from_base_temp(bool b, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); 
  return temporal_from_base_temp(BoolGetDatum(b), T_TBOOL, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer from an integer and the time frame of
 * another temporal value
 * @param[in] i Value
 * @param[in] temp Temporal value
 */
Temporal *
tint_from_base_temp(int i, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); 
  return temporal_from_base_temp(Int32GetDatum(i), T_TINT, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float from a float and the time frame of
 * another temporal value
 * @param[in] d Value
 * @param[in] temp Temporal value
 */
Temporal *
tfloat_from_base_temp(double d, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); 
  return temporal_from_base_temp(Float8GetDatum(d), T_TFLOAT, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text from a text and the time frame of
 * another temporal value
 * @param[in] txt Value
 * @param[in] temp Temporal value
 */
Temporal *
ttext_from_base_temp(const text *txt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(txt, NULL); VALIDATE_NOT_NULL(temp, NULL); 
  return temporal_from_base_temp(PointerGetDatum(txt), T_TTEXT, temp);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value
 * @csqlfn #Tnumber_shift_value()
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 */
Temporal *
tint_shift_value(const Temporal *temp, int shift)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); 
  return tnumber_shift_scale_value(temp, Int32GetDatum(shift), 0, true, false);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value
 * @csqlfn #Tnumber_shift_value()
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 */
Temporal *
tfloat_shift_value(const Temporal *temp, double shift)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); 
  return tnumber_shift_scale_value(temp, Float8GetDatum(shift), 0, true, false);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_scale_value()
 */
Temporal *
tint_scale_value(const Temporal *temp, int width)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); 
  return tnumber_shift_scale_value(temp, 0, Int32GetDatum(width), false, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_scale_value()
 */
Temporal *
tfloat_scale_value(const Temporal *temp, double width)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); 
  return tnumber_shift_scale_value(temp, 0, Float8GetDatum(width), false, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted and scaled
 * by two values
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_shift_scale_value()
 */
Temporal *
tint_shift_scale_value(const Temporal *temp, int shift, int width)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); 
  return tnumber_shift_scale_value(temp, Int32GetDatum(shift),
    Int32GetDatum(width), true, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number whose value dimension is shifted and scaled
 * by two values
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_shift_scale_value()
 */
Temporal *
tfloat_shift_scale_value(const Temporal *temp, double shift, double width)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); 
  return tnumber_shift_scale_value(temp, Float8GetDatum(shift),
    Float8GetDatum(width), true, true);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal boolean
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
bool *
tbool_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL); VALIDATE_NOT_NULL(count, NULL); 
  Datum *datumarr = temporal_values_p(temp, count);
  bool *result = palloc(sizeof(bool) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetBool(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal integer
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
int *
tint_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL); 
  Datum *datumarr = temporal_values_p(temp, count);
  int *result = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetInt32(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal float
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
double *
tfloat_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL); 
  Datum *datumarr = temporal_values_p(temp, count);
  double *result = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetFloat8(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of copies of base values of a temporal text
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
text **
ttext_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL); VALIDATE_NOT_NULL(count, NULL); 
  Datum *datumarr = temporal_values_p(temp, count);
  text **result = palloc(sizeof(text *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = text_copy(DatumGetTextP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal boolean
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_start_value()
 */
bool
tbool_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, false);
  return DatumGetBool(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_start_value()
 */
int
tint_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  return DatumGetInt32(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_start_value()
 */
double
tfloat_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  return DatumGetFloat8(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
text *
ttext_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  return DatumGetTextP(temporal_start_value(temp));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal boolean
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_end_value()
 */
bool
tbool_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, false);
  return DatumGetBool(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_end_value()
 */
int
tint_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  return DatumGetInt32(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_end_value()
 */
double
tfloat_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  return DatumGetFloat8(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
text *
ttext_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return DatumGetTextP(temporal_end_value(temp));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the minimum value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_min_value()
 */
int
tint_min_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  return DatumGetInt32(temporal_min_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the minimum value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_min_value()
 */
double
tfloat_min_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  return DatumGetFloat8(temporal_min_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the minimum value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_min_value()
 */
text *
ttext_min_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return DatumGetTextP(temporal_min_value(temp));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the maximum value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_max_value()
 */
int
tint_max_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  return DatumGetInt32(temporal_max_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the maximum value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_max_value()
 */
double
tfloat_max_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  return DatumGetFloat8(temporal_max_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the maximum value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_max_value()
 */
text *
ttext_max_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  return DatumGetTextP(temporal_max_value(temp));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the n-th value of a temporal boolean
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tbool_value_n(const Temporal *temp, int n, bool *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetBool(dresult);
  return true;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the n-th value of a temporal integer
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tint_value_n(const Temporal *temp, int n, int *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetInt32(dresult);
  return true;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the n-th value of a temporal float
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tfloat_value_n(const Temporal *temp, int n, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetFloat8(dresult);
  return true;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the n-th value of a temporal text
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
ttext_value_n(const Temporal *temp, int n, text **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetTextP(dresult);
  return true;
}

/*****************************************************************************/
