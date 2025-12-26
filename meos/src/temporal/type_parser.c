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
 * @brief Functions for parsing base, set, span, tbox, and temporal types
 * @details Many functions make two passes for parsing, the first one to obtain
 * the number of elements in order to do memory allocation with @p palloc, the
 * second one to create the type. This is the only approach we can see at the
 * moment which is both correct and simple.
 */

#include "temporal/type_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/tspatial_parser.h"

// #include <utils/jsonb.h>
// #include <utils/numeric.h>
// #include <pgtypes.h>

/*****************************************************************************/

/**
 * @brief Initializes an expandable array that keeps the elements parsed so far
 * @details The array is initialized with a fix number of elements but it
 * expands when adding elements and the array is full
 */
meos_array *
meos_array_init(meosType type)
{
  meos_array *array = (meos_array *) palloc0(sizeof(meos_array));
  array->size = MEOS_ARRAY_INITIAL_SIZE;
  array->count = 0;
  array->type = type;
  array->values = (Datum *) palloc0(sizeof(Datum) * MEOS_ARRAY_INITIAL_SIZE);
  return array;
}

/**
 * @brief Add a value to the array
 */
void
meos_array_add(meos_array *array, Datum value)
{
  /* Enlarge the values array if necessary */
  if (array->count >= array->size)
  {
    array->size *= 2;
    array->values = (Datum *) repalloc(array->values, sizeof(Datum) * 
      array->size);
  }
  /* Store the value */
  array->values[array->count++] = value;
  return;
}

/**
 * @brief Get the n-th value of the array (0-based)
 */
Datum
meos_array_get_n(meos_array *array, int n)
{
  /* Ensure that the index is valid */
  if ((size_t) n < 0 || (size_t) n >= array->count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Invalid array index: %d", n);
    return (Datum) NULL;
  }
  /* Return the value */
  return array->values[n];
}

/**
 * @brief Reset the array
 */
void
meos_array_reset(meos_array *array)
{
  if (! array || ! array->count)
    return;
  for (size_t i = 0; i < array->count; i++)
    DATUM_FREE(array->values[i], array->type);
  array->count = 0;
  return;
}

/**
 * @brief Destroy the array
 */
void
meos_array_destroy(meos_array *array)
{
  if (! array)
    return;
  if (array->values)
  {
    for (size_t i = 0; i < array->count; ++i)
     DATUM_FREE(array->values[i], array->type);
    pfree(array->values);
  }
  pfree(array);
  return;
}

/*****************************************************************************
 * Parsing functions
 *****************************************************************************/

/**
 * @brief Ensure there is no more input excepted white spaces
 */
bool
ensure_end_input(const char **str, const char *type)
{
  p_whitespace(str);
  if (**str != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Extraneous characters at the end", type);
    return false;
  }
  return true;
}

/**
 * @brief Input a white space from the buffer
 */
void
p_whitespace(const char **str)
{
  while (**str == ' ' || **str == '\n' || **str == '\r' || **str == '\t')
    *str += 1;
  return;
}

/**
 * @brief Input an opening brace from the buffer
 */
bool
p_delimchar(const char **str, char delim)
{
  p_whitespace(str);
  if (**str == delim)
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Input an opening brace from the buffer
 */
bool
p_obrace(const char **str)
{
  p_whitespace(str);
  if (**str == '{')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Ensure to input an opening brace from the buffer
 */
bool
ensure_obrace(const char **str, const char *type)
{
  if (! p_obrace(str))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing opening brace", type);
    return false;
  }
  return true;
}

/**
 * @brief Input a closing brace from the buffer
 */
bool
p_cbrace(const char **str)
{
  p_whitespace(str);
  if (**str == '}')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Ensure to input a closing brace from the buffer
 */
bool
ensure_cbrace(const char **str, const char *type)
{
  if (! p_cbrace(str))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing closing brace", type);
    return false;
  }
  return true;
}

/**
 * @brief Input an opening bracket from the buffer
 */
bool
p_obracket(const char **str)
{
  p_whitespace(str);
  if (**str == '[')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Input a closing bracket from the buffer
 */
bool
p_cbracket(const char **str)
{
  p_whitespace(str);
  if (**str == ']')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Input an opening parenthesis from the buffer
 */
bool
p_oparen(const char **str)
{
  p_whitespace(str);
  if (**str == '(')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Ensure to input an opening parenthesis from the buffer
 */
bool
ensure_oparen(const char **str, const char *type)
{
  if (!p_oparen(str))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse %s value: Missing opening parenthesis", type);
    return false;
  }
  return true;
}

/**
 * @brief Input a closing parenthesis from the buffer
 */
bool
p_cparen(const char **str)
{
  p_whitespace(str);
  if (**str == ')')
  {
    *str += 1;
    return true;
  }
  return false;
}

/**
 * @brief Ensure to input a closing parenthesis from the buffer
 */
bool
ensure_cparen(const char **str, const char *type)
{
  if (!p_cparen(str))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing closing parenthesis", type);
    return false;
  }
  return true;
}

/**
 * @brief Input a comma from the buffer
 */
bool
p_comma(const char **str)
{
  p_whitespace(str);
  if (**str == ',')
  {
    *str += 1;
    return true;
  }
  return false;
}

/*****************************************************************************/

/**
 * @brief Input a double from the buffer
 * @return On error return false
 */
bool
double_parse(const char **str, double *result)
{
  char *nextstr = (char *) *str;
  *result = strtod(*str, &nextstr);
  if (*str == nextstr)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Invalid input syntax for type double");
    return false;
  }
  *str = nextstr;
  return true;
}

/**
 * @brief Parse a base value from the buffer
 * @return On error return false
 */
bool
basetype_parse(const char **str, meosType basetype, char delim, Datum *result)
{
  p_whitespace(str);
  int pos = 0;
  /* Save the original string for error handling */
  char *origstr = (char *) *str;

  /* ttext values must be enclosed between double quotes */
  if (**str == '"')
  {
    /* Consume the double quote */
    *str += 1;
    while ( ( (*str)[pos] != '"' || (*str)[pos - 1] == '\\' )  &&
      (*str)[pos] != '\0' )
      pos++;
  }
  else
  {
    while ((*str)[pos] != delim && (*str)[pos] != '\0')
      pos++;
  }
  if ((*str)[pos] == '\0')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Missing delimeter character '%c': %s", delim, origstr);
    return false;
  }
  char *str1 = palloc(sizeof(char) * (pos + 1));
  strncpy(str1, *str, pos);
  str1[pos] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  /* The delimeter is NOT consumed */
  *str += pos;
  return true;
}

/*****************************************************************************/

/**
 * @brief Parse a temporal box value from the buffer
 * @return On error return @p NULL
 */
TBox *
tbox_parse(const char **str)
{
  bool hasx = false, hast = false;
  Span span;
  Span period;
  const char *type_str = meostype_name(T_TBOX);

  p_whitespace(str);
  /* By default the span type is float span */
  meosType spantype = T_FLOATSPAN;
  if (pg_strncasecmp(*str, "TBOXINT", 7) == 0)
  {
    spantype = T_INTSPAN;
    *str += 7;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "TBOXFLOAT", 9) == 0)
  {
    *str += 9;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "TBOX", 4) == 0)
  {
    *str += 4;
    p_whitespace(str);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT, 
      "Could not parse %s value: Missing prefix 'TBox'", type_str);
    return NULL;
  }
  /* Determine whether the box has X and/or T dimensions */
  if (pg_strncasecmp(*str, "XT", 2) == 0)
  {
    hasx = hast = true;
    *str += 2;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "X", 1) == 0)
  {
    hasx = true;
    *str += 1;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "T", 1) == 0)
  {
    hast = true;
    *str += 1;
    p_whitespace(str);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing dimension information", type_str);
    return NULL;
  }
  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  if (hasx)
  {
    if (! span_parse(str, spantype, false, &span))
      return NULL;
    if (hast)
    {
      /* Consume the optional comma separating the span and the period */
      p_whitespace(str);
      p_comma(str);
    }
  }

  if (hast)
    if (! span_parse(str, T_TSTZSPAN, false, &period))
      return NULL;

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
      /* Ensure there is no more input */
      ! ensure_end_input(str, type_str))
    return NULL;

  return tbox_make(hasx ? &span: NULL, hast ? &period : NULL);
}

/*****************************************************************************/
/* Time Types */

/**
 * @brief Parse a timestamp value from the buffer
 * @return On error return DT_NOEND
 */
TimestampTz
timestamp_parse(const char **str)
{
  p_whitespace(str);
  int pos = 0;
  while ((*str)[pos] != ',' && (*str)[pos] != ']' && (*str)[pos] != ')' &&
    (*str)[pos] != '}' && (*str)[pos] != '\0')
    pos++;

  char *str1 = palloc(sizeof(char) * (pos + 1));
  strncpy(str1, *str, pos);
  str1[pos] = '\0';
  /* The last argument is for an unused typmod */
  TimestampTz result = pg_timestamptz_in(str1, -1);
  pfree(str1);
  *str += pos;
  return result;
}

/*****************************************************************************/
/* Set and Span Types */

/**
 * @brief Parse a element value from the buffer
 * @return On error return false
 */
bool
elem_parse(const char **str, meosType basetype, Datum *result)
{
  p_whitespace(str);
  int pos = 0, dquote = 0;
  /* ttext and geometry/geography values must be enclosed between double quotes */
  if (**str == '"')
  {
    /* Consume the double quote */
    *str += 1;
    while ( ( (*str)[pos] != '"' || (*str)[pos - 1] == '\\' )  &&
      (*str)[pos] != '\0' )
      pos++;
    dquote = 1;
  }
  else
  {
    while ((*str)[pos] != ',' && (*str)[pos] != '}' && 
        (*str)[pos] != '\0')
      pos++;
  }
  char *str1 = palloc(sizeof(char) * (pos + 1));
  strncpy(str1, *str, pos);
  str1[pos] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  *str += pos + dquote;
  return true;
}

/**
 * @brief Parse a set value from the buffer
 * @return On error return @p NULL
 */
Set *
set_parse(const char **str, meosType settype)
{
  meosType basetype = settype_basetype(settype);
  meos_array *array = meos_array_init(basetype);
  const char *type_str = meostype_name(settype);
  Set *result = NULL;

  /* Parsing */
  p_whitespace(str);

  /* Determine whether there is an SRID. If there is one we decode it and
   * advance the bak pointer after the SRID to do not parse it again in the
   * second parsing */
  int set_srid = SRID_UNKNOWN;
  (void) srid_parse(str, &set_srid);

  if (! ensure_obrace(str, type_str))
    goto error;

  Datum d;
  if (! elem_parse(str, basetype, &d))
    goto error;
  meos_array_add(array, d);
  while (p_comma(str))
  {
    if (! elem_parse(str, basetype, &d))
      goto error;
    meos_array_add(array, d);
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    goto error;

  /* Create the array of values now with the actual size */
  p_obrace(str);
  Datum *values = palloc(sizeof(Datum) * array->count);
  for (int i = 0; i < (int) array->count; i++)
    values[i] = meos_array_get_n(array, i);
  p_cbrace(str);
  if (set_srid != SRID_UNKNOWN)
  {
    for (int i = 0; i < (int) array->count; i++)
      spatial_set_srid(values[i], basetype, set_srid);
  }
  result = set_make(values, array->count, basetype, ORDER);
  pfree(values);

error:
  meos_array_destroy(array);
  return result;
}

/**
 * @brief Parse a bound value from the buffer
 * @return On error return false
 */
bool
bound_parse(const char **str, meosType basetype, Datum *result)
{
  p_whitespace(str);
  int pos = 0;
  while ((*str)[pos] != ',' && (*str)[pos] != ']' &&
    (*str)[pos] != '}' && (*str)[pos] != ')' && (*str)[pos] != '\0')
    pos++;
  char *str1 = palloc(sizeof(char) * (pos + 1));
  strncpy(str1, *str, pos);
  str1[pos] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  *str += pos;
  return true;
}

/**
 * @brief Parse a span value from the buffer
 * @return On error return false
 */
bool
span_parse(const char **str, meosType spantype, bool end, Span *span)
{
  const char *type_str = meostype_name(spantype);
  bool lower_inc = false, upper_inc = false;
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing opening bracket/parenthesis", type_str);
    return false;
  }
  meosType basetype = spantype_basetype(spantype);
  Datum lower, upper;
  if (! bound_parse(str, basetype, &lower))
    return false;
  p_comma(str);
  if (! bound_parse(str, basetype, &upper))
    return false;

  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing closing bracket/parenthesis", type_str);
    return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, meostype_name(spantype)))
    return false;

  if (span)
    span_set(lower, upper, lower_inc, upper_inc, basetype, spantype, span);
  return true;
}

/**
 * @brief Parse a span set value from the buffer
 * @return On error return @p NULL
 */
SpanSet *
spanset_parse(const char **str, meosType spansettype)
{
  const char *type_str = meostype_name(spansettype);
  if (! ensure_obrace(str, type_str))
    return NULL;
  meosType spantype = spansettype_spantype(spansettype);

  /* First parsing */
  const char *bak = *str;
  if (! span_parse(str, spantype, false, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! span_parse(str, spantype, false, NULL))
      return NULL;
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    return NULL;

  /* Second parsing */
  *str = bak;
  Span *spans = palloc(sizeof(Span) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    span_parse(str, spantype, false, &spans[i]);
  }
  p_cbrace(str);
  return spanset_make_free(spans, count, NORMALIZE, ORDER_NO);
}

/*****************************************************************************/
/* Temporal Types */

/**
 * @brief Parse a temporal instant value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no more input after the instant
 * @return On error return NULL
 */
TInstant *
tinstant_parse(const char **str, meosType temptype, bool end)
{
  p_whitespace(str);
  meosType basetype = temptype_basetype(temptype);
  /* The next two instructions will throw an exception if they fail */
  Datum elem;
  if (! basetype_parse(str, basetype, '@', &elem))
    return NULL;
  p_delimchar(str, '@');
  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND ||
    /* Ensure there is no more input */
    (end && ! ensure_end_input(str, meostype_name(temptype))))
  {
    DATUM_FREE(elem, basetype);
    return NULL;
  }
  TInstant *result = tinstant_make(elem, temptype, t);
  DATUM_FREE(elem, basetype);
  return result;
}

/**
 * @brief Parse a temporal discrete sequence from the buffer
 * @param[in] str Input string
 * @param[in] temptype Base type
 * @return On error return @p NULL
 */
TSequence *
tdiscseq_parse(const char **str, meosType temptype)
{
  meos_array *array = meos_array_init(temptype);
  const char *type_str = meostype_name(temptype);
  TSequence *result = NULL;

  /* Parsing */
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function #temporal_parse */
  p_obrace(str);

  TInstant *inst = tinstant_parse(str, temptype, false);
  if (! inst)
    goto error;
  meos_array_add(array, PointerGetDatum(inst));
  while (p_comma(str))
  {
    inst = tinstant_parse(str, temptype, false);
    if (! inst)
      goto error;
    meos_array_add(array, PointerGetDatum(inst));
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    goto error;
  p_cbrace(str);

  /* Create the array of instants now with the actual size */
  TInstant **instants = palloc(sizeof(TInstant *) * array->count);
  for (size_t i = 0; i < array->count; i++)
    instants[i] = DatumGetTInstantP(meos_array_get_n(array, i));
  result = tsequence_make(instants, array->count, true, true, DISCRETE,
    NORMALIZE_NO);
  pfree(instants);

error:
  meos_array_destroy(array);
  return result;
}

/**
 * @brief Parse a temporal sequence value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in] end Set to true when reading a single sequence to ensure there is
 * no more input after the sequence
 * @param[out] result New sequence, may be NULL
 * @return On error return false
 */
TSequence *
tcontseq_parse(const char **str, meosType temptype, interpType interp,
  bool end)
{
  meos_array *array = meos_array_init(temptype);
  TSequence *result = NULL;

  /* Parsing */
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function temporal_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;
  TInstant *inst = tinstant_parse(str, temptype, false);
  if (! inst)
    goto error;
  meos_array_add(array, PointerGetDatum(inst));
  while (p_comma(str))
  {
    inst = tinstant_parse(str, temptype, false);
    if (! inst)
      goto error;
    meos_array_add(array, PointerGetDatum(inst));
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing closing bracket/parenthesis",
      meostype_name(temptype));
      goto error;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, meostype_name(temptype)))
    goto error;
  p_cbracket(str);
  p_cparen(str);

  /* Create the array of instants now with the actual size */
  TInstant **instants = palloc(sizeof(TInstant *) * array->count);
  for (size_t i = 0; i < array->count; i++)
    instants[i] = DatumGetTInstantP(meos_array_get_n(array, i));
  result = tsequence_make(instants, array->count, lower_inc, upper_inc, interp,
    NORMALIZE);
  pfree(instants);

error:
  meos_array_destroy(array);
  return result;
}

/**
 * @brief Parse a temporal sequence set from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @return On error return @p NULL
 */
TSequenceSet *
tsequenceset_parse(const char **str, meosType temptype, interpType interp)
{
  meos_array *array = meos_array_init(temptype);
  const char *type_str = meostype_name(temptype);
  TSequenceSet *result = NULL;

  /* Parsing */
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);
  TSequence *seq = tcontseq_parse(str, temptype, interp, false);
  if (! seq)
    goto error;
  meos_array_add(array, PointerGetDatum(seq));
  while (p_comma(str))
  {
    seq = tcontseq_parse(str, temptype, interp, false);
    if (! seq)
      goto error;
    meos_array_add(array, PointerGetDatum(seq));
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    goto error;
  p_cbrace(str);

  /* Create the array of sequences now with the actual size */
  TSequence **sequences = palloc(sizeof(TSequence *) * array->count);
  for (size_t i = 0; i < array->count; i++)
    sequences[i] = DatumGetTSequenceP(meos_array_get_n(array, i));
  result = tsequenceset_make(sequences, array->count, NORMALIZE);
  pfree(sequences);

error:
  meos_array_destroy(array);
  return result;
}

/**
 * @brief Parse a temporal value from the buffer (dispatch function)
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @return On error return @p NULL
 */
Temporal *
temporal_parse(const char **str, meosType temptype)
{
  p_whitespace(str);
  Temporal *result = NULL;  /* keep compiler quiet */
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  /* Starts with "Interp=Step;" */
  if (pg_strncasecmp(*str, "Interp=Step;", 12) == 0)
  {
    /* Move str after the semicolon */
    *str += 12;
    interp = STEP;
  }

  /* Allow spaces after Interp */
  p_whitespace(str);

  if (**str != '{' && **str != '[' && **str != '(')
  {
    TInstant *inst = tinstant_parse(str, temptype, true);
    if (! inst)
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq = tcontseq_parse(str, temptype, interp, true);
    if (! seq)
      return NULL;
    result = (Temporal *) seq;
  }
  else if (**str == '{')
  {
    const char *bak = *str;
    p_obrace(str);
    p_whitespace(str);
    if (**str == '[' || **str == '(')
    {
      *str = bak;
      result = (Temporal *) tsequenceset_parse(str, temptype, interp);
    }
    else
    {
      *str = bak;
      result = (Temporal *) tdiscseq_parse(str, temptype);
    }
  }
  return result;
}

/*****************************************************************************/
