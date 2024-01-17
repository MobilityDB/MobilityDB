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
 * @brief Functions for parsing base, set, span, tbox, and temporal types
 * @details Many functions make two passes for parsing, the first one to obtain
 * the number of elements in order to do memory allocation with @p palloc, the
 * second one to create the type. This is the only approach we can see at the
 * moment which is both correct and simple.
 */

#include "general/type_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"

/*****************************************************************************/

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
        "Could not parse %s: Missing opening parenthesis", type);
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
      "Could not parse %s: Missing closing parenthesis", type);
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
temporal_basetype_parse(const char **str, meosType basetype,
  Datum *result)
{
  p_whitespace(str);
  int delim = 0;
  /* Save the original string for error handling */
  char *origstr = (char *) *str;

  /* ttext values must be enclosed between double quotes */
  if (**str == '"')
  {
    /* Consume the double quote */
    *str += 1;
    while ( ( (*str)[delim] != '"' || (*str)[delim - 1] == '\\' )  &&
      (*str)[delim] != '\0' )
      delim++;
  }
  else
  {
    while ((*str)[delim] != '@' && (*str)[delim] != '\0')
      delim++;
  }
  if ((*str)[delim] == '\0')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse temporal value: %s", origstr);
    return false;
  }
  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  /* since there's an @ here, let's take it with us */
  *str += delim + 1;
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
  const char *type_str = "temporal box";

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
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT, "Could not parse temporal box");
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
      "Could not parse temporal box: Missing dimension information");
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
  int delim = 0;
  while ((*str)[delim] != ',' && (*str)[delim] != ']' && (*str)[delim] != ')' &&
    (*str)[delim] != '}' && (*str)[delim] != '\0')
    delim++;

  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  /* The last argument is for an unused typmod */
  TimestampTz result = pg_timestamptz_in(str1, -1);
  pfree(str1);
  *str += delim;
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
  int delim = 0, dquote = 0;
  /* ttext and geometry/geography values must be enclosed between double quotes */
  if (**str == '"')
  {
    /* Consume the double quote */
    *str += 1;
    while ( ( (*str)[delim] != '"' || (*str)[delim - 1] == '\\' )  &&
      (*str)[delim] != '\0' )
      delim++;
    dquote = 1;
  }
  else
  {
    while ((*str)[delim] != ',' && (*str)[delim] != ']' &&
      (*str)[delim] != '}' && (*str)[delim] != ')' && (*str)[delim] != '\0')
      delim++;
  }
  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  *str += delim + dquote;
  return true;
}

/**
 * @brief Parse a set value from the buffer
 * @return On error return @p NULL
 */
Set *
set_parse(const char **str, meosType settype)
{
  const char *type_str = "set";
  int set_srid = 0;
  p_whitespace(str);
  
  /* Starts with "SRID=". The SRID specification must be gobbled. We cannot use 
   * the atoi() function because this requires a string terminated by '\0' 
   * and we cannot modify the string. */
  if (pg_strncasecmp(*str, "SRID=", 5) == 0)
  {
    if (! ensure_geoset_type(settype))
      return NULL;
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    set_srid = 0;
    /* Delimiter will be either ',' or ';' depending on whether interpolation
       is given after */
    while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
    {
      set_srid = set_srid * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str to the start of the temporal point */
    *str += delim + 1;
  }
  /* For the second pass we start after the SRID=xxx;{ including the '{' */
  const char *bak = *str + 1;

  if (! ensure_obrace(str, type_str))
    return NULL;
  meosType basetype = settype_basetype(settype);

  /* First parsing */
  Datum d;
  if (! elem_parse(str, basetype, &d))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! elem_parse(str, basetype, &d))
      return NULL;
  }
  if (! ensure_cbrace(str, type_str) ||
      ! ensure_end_input(str, type_str))
    return NULL;

  /* Second parsing */
  *str = bak;
  Datum *values = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    elem_parse(str, basetype, &values[i]);
  }
  p_cbrace(str);
  if (set_srid != SRID_UNKNOWN)
  {
    for (int i = 0; i < count; i++)
      gserialized_set_srid(DatumGetGserializedP(values[i]), set_srid);
  }
  return set_make_free(values, count, basetype, ORDERED_NO);
}

/**
 * @brief Parse a bound value from the buffer
 * @return On error return false
 */
bool
bound_parse(const char **str, meosType basetype, Datum *result)
{
  p_whitespace(str);
  int delim = 0;
  while ((*str)[delim] != ',' && (*str)[delim] != ']' &&
    (*str)[delim] != '}' && (*str)[delim] != ')' && (*str)[delim] != '\0')
    delim++;
  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  bool success = basetype_in(str1, basetype, false, result);
  pfree(str1);
  if (! success)
    return false;
  *str += delim;
  return true;
}

/**
 * @brief Parse a span value from the buffer
 * @return On error return false
 */
bool
span_parse(const char **str, meosType spantype, bool end, Span *span)
{
  bool lower_inc = false, upper_inc = false;
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse span: Missing opening bracket/parenthesis");
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
      "Could not parse span: Missing closing bracket/parenthesis");
    return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "span"))
    return false;

  if (! span)
    return true;
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
  const char *type_str = "span set";
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
  return spanset_make_free(spans, count, NORMALIZE, ORDERED);
}

/*****************************************************************************/
/* Temporal Types */

/**
 * @brief Parse a temporal instant value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no more input after the instant
 * @param[out] result New instant, may be NULL
 * @return On error return false
 */
bool
tinstant_parse(const char **str, meosType temptype, bool end,
  TInstant **result)
{
  p_whitespace(str);
  meosType basetype = temptype_basetype(temptype);
  /* The next two instructions will throw an exception if they fail */
  Datum elem;
  if (! temporal_basetype_parse(str, basetype, &elem))
    return false;
  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND)
    return false;
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "temporal"))
    return false;
  if (result)
    *result = tinstant_make_free(elem, temptype, t);
  return true;
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
  const char *type_str = "temporal";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function #temporal_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tinstant_parse(str, temptype, false, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tinstant_parse(str, temptype, false, NULL))
      return NULL;
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    return NULL;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tinstant_parse(str, temptype, false, &instants[i]);
  }
  p_cbrace(str);
  return tsequence_make_free(instants, count, true, true, DISCRETE,
    NORMALIZE_NO);
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
bool
tcontseq_parse(const char **str, meosType temptype, interpType interp,
  bool end, TSequence **result)
{
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function temporal_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  const char *bak = *str;
  if (! tinstant_parse(str, temptype, false, NULL))
    return false;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tinstant_parse(str, temptype, false, NULL))
      return false;
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse temporal value: Missing closing bracket/parenthesis");
      return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "temporal"))
    return NULL;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tinstant_parse(str, temptype, false, &instants[i]);
  }
  p_cbracket(str);
  p_cparen(str);
  if (result)
    *result = tsequence_make_free(instants, count, lower_inc, upper_inc,
      interp, NORMALIZE);
  return true;
}

/**
 * @brief Parse a temporal sequence set value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @return On error return @p NULL
 */
TSequenceSet *
tsequenceset_parse(const char **str, meosType temptype, interpType interp)
{
  const char *type_str = "temporal";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tcontseq_parse(str, temptype, interp, false, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tcontseq_parse(str, temptype, interp, false, NULL))
      return NULL;
  }
  if (! ensure_cbrace(str, type_str) || ! ensure_end_input(str, type_str))
    return NULL;

  /* Second parsing */
  *str = bak;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tcontseq_parse(str, temptype, interp, false, &sequences[i]);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
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
    TInstant *inst;
    if (! tinstant_parse(str, temptype, true, &inst))
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq;
    if (! tcontseq_parse(str, temptype, interp, true, &seq))
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
