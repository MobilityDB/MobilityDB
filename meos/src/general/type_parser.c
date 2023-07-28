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
 * @brief Functions for parsing time types and temporal types
 *
 * Many functions make two passes for parsing, the first one to obtain the
 * number of elements in order to do memory allocation with `palloc`, the
 * second one to create the type. This is the only approach we can see at the
 * moment which is both correct and simple.
 */

#include "general/type_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/type_util.h"

/*****************************************************************************/

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
 * @brief Ensure there is no more input excepted white spaces
 */
void
ensure_end_input(const char **str, bool end, const char *type)
{
  if (end)
  {
    p_whitespace(str);
    if (**str != 0)
      elog(ERROR, "Could not parse %s value", type);
  }
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
void
ensure_oparen(const char **str, const char *type)
{
  if (!p_oparen(str))
    elog(ERROR, "Could not parse %s: Missing opening parenthesis", type);
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
void
ensure_cparen(const char **str, const char *type)
{
  if (!p_cparen(str))
    elog(ERROR, "Could not parse %s: Missing closing parenthesis", type);
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

/**
 * @brief Input a double from the buffer
 *
 * @param[in,out] str Pointer to the current position of the input buffer
 */

double
double_parse(const char **str)
{
  char *nextstr = (char *) *str;
  double result = strtod(*str, &nextstr);
  if (*str == nextstr)
    elog(ERROR, "Invalid input syntax for type double");
  *str = nextstr;
  return result;
}

/**
 * @brief Parse a base value from the buffer
 */
Datum
temporal_basetype_parse(const char **str, meosType basetype)
{
  p_whitespace(str);
  int delim = 0;
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
    elog(ERROR, "Could not parse element value");

  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  Datum result = basetype_in(str1, basetype, false);
  pfree(str1);
  /* since there's an @ here, let's take it with us */
  *str += delim + 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Parse a temporal box value from the buffer.
 */
TBox *
tbox_parse(const char **str)
{
  bool hasx = false, hast = false;
  Span span;
  Span period;

  p_whitespace(str);
  if (pg_strncasecmp(*str, "TBOX", 4) == 0)
  {
    *str += 4;
    p_whitespace(str);
  }
  else
    elog(ERROR, "Could not parse temporal box");

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
    elog(ERROR, "Could not parse temporal box: Missing dimension information");

  /* Parse opening parenthesis */
  ensure_oparen(str, "temporal box");

  if (hasx)
  {
    span_parse(str, T_FLOATSPAN, false, &span);
    if (hast)
    {
      /* Consume the optional comma separating the span and the period */
      p_whitespace(str);
      p_comma(str);
    }
  }

  if (hast)
    span_parse(str, T_TSTZSPAN, false, &period);

  /* Parse closing parenthesis */
  p_whitespace(str);
  ensure_cparen(str, "temporal box");

  /* Ensure there is no more input */
  ensure_end_input(str, true, "temporal box");

  return tbox_make(hasx ? &span: NULL, hast ? &period : NULL);
}

/*****************************************************************************/
/* Time Types */

/**
 * @brief Parse a timestamp value from the buffer.
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
  Datum result = pg_timestamptz_in(str1, -1);
  pfree(str1);
  *str += delim;
  return result;
}

/*****************************************************************************/
/* Set and Span Types */

/**
 * @brief Parse a element value from the buffer.
 */
Datum
elem_parse(const char **str, meosType basetype)
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
  Datum result = basetype_in(str1, basetype, false);
  pfree(str1);
  *str += delim + dquote;
  return result;
}

/**
 * @brief Parse a timestamp set value from the buffer.
 */
Set *
set_parse(const char **str, meosType settype)
{
  if (!p_obrace(str))
    elog(ERROR, "Could not parse the set: Missing open brace");

  /* First parsing */
  meosType basetype = settype_basetype(settype);
  const char *bak = *str;
  elem_parse(str, basetype);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    elem_parse(str, basetype);
  }
  if (!p_cbrace(str))
    elog(ERROR, "Could not parse the set: Missing closing brace");

  *str = bak;
  Datum *values = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    values[i] = elem_parse(str, basetype);
  }
  p_cbrace(str);
  return set_make_free(values, count, basetype, ORDERED);
}

/**
 * @brief Parse a element value from the buffer
 */
Datum
bound_parse(const char **str, meosType basetype)
{
  p_whitespace(str);
  int delim = 0;
  while ((*str)[delim] != ',' && (*str)[delim] != ']' &&
    (*str)[delim] != '}' && (*str)[delim] != ')' &&  (*str)[delim] != '\0')
    delim++;
  char *str1 = palloc(sizeof(char) * (delim + 1));
  strncpy(str1, *str, delim);
  str1[delim] = '\0';
  Datum result = basetype_in(str1, basetype, false);
  pfree(str1);
  *str += delim;
  return result;
}

/**
 * @brief Parse a span value from the buffer
 */
void
span_parse(const char **str, meosType spantype, bool end, Span *span)
{
  bool lower_inc = false, upper_inc = false;
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;
  else
    elog(ERROR, "Could not parse span: Missing opening bracket/parenthesis");

  meosType basetype = spantype_basetype(spantype);
  /* The next two instructions will throw an exception if they fail */
  Datum lower = bound_parse(str, basetype);
  p_comma(str);
  Datum upper = bound_parse(str, basetype);

  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
    elog(ERROR, "Could not parse span: Missing closing bracket/parenthesis");

  /* Ensure there is no more input */
  ensure_end_input(str, end, "span");

  if (! span)
    return;
  span_set(lower, upper, lower_inc, upper_inc, basetype, span);
  return;
}

/**
 * @brief Parse a span set value from the buffer
 */
SpanSet *
spanset_parse(const char **str, meosType spansettype)
{
  if (! p_obrace(str))
    elog(ERROR, "Could not parse span set: Missing open brace");

  meosType spantype = spansettype_spantype(spansettype);
  /* First parsing */
  const char *bak = *str;
  span_parse(str, spantype, false, NULL);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    span_parse(str, spantype, false, NULL);
  }
  if (! p_cbrace(str))
    elog(ERROR, "Could not parse span set: Missing closing brace");

  /* Second parsing */
  *str = bak;
  Span *spans = palloc(sizeof(Span) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    span_parse(str, spantype, false, &spans[i]);
  }
  p_cbrace(str);
  SpanSet *result = spanset_make_free(spans, count, NORMALIZE);
  return result;
}

/*****************************************************************************/
/* Temporal Types */

/**
 * @brief Parse a temporal instant value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no more input after the instant
 * @param[in] make Set to false for the first pass to do not create the instant
 */
TInstant *
tinstant_parse(const char **str, meosType temptype, bool end, bool make)
{
  p_whitespace(str);
  meosType basetype = temptype_basetype(temptype);
  /* The next two instructions will throw an exception if they fail */
  Datum elem = temporal_basetype_parse(str, basetype);
  TimestampTz t = timestamp_parse(str);
  /* Ensure there is no more input */
  ensure_end_input(str, end, "temporal");
  if (! make)
    return NULL;
  return tinstant_make(elem, temptype, t);
}

/**
 * @brief Parse a temporal discrete sequence from the buffer
 * @param[in] str Input string
 * @param[in] temptype Base type
 */
TSequence *
tdiscseq_parse(const char **str, meosType temptype)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  tinstant_parse(str, temptype, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tinstant_parse(str, temptype, false, false);
  }
  if (!p_cbrace(str))
    elog(ERROR, "Could not parse temporal value: Missing closing brace");
  /* Ensure there is no more input */
  ensure_end_input(str, true, "temporal");

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    instants[i] = tinstant_parse(str, temptype, false, true);
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
 * no moreinput after the sequence
 * @param[in] make Set to false for the first pass to do not create the sequence
 */
TSequence *
tcontseq_parse(const char **str, meosType temptype, interpType interp, bool end,
  bool make)
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
  tinstant_parse(str, temptype, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tinstant_parse(str, temptype, false, false);
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
    elog(ERROR, "Could not parse temporal value: Missing closing bracket/parenthesis");
  /* Ensure there is no more input */
  ensure_end_input(str, end, "temporal");
  if (! make)
    return NULL;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    instants[i] = tinstant_parse(str, temptype, false, true);
  }
  p_cbracket(str);
  p_cparen(str);
  return tsequence_make_free(instants, count, lower_inc, upper_inc, interp,
    NORMALIZE);
}

/**
 * @brief Parse a temporal sequence set value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequenceSet *
tsequenceset_parse(const char **str, meosType temptype, interpType interp)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  tcontseq_parse(str, temptype, interp, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tcontseq_parse(str, temptype, interp, false, false);
  }
  if (!p_cbrace(str))
    elog(ERROR, "Could not parse temporal value: Missing closing brace");
  /* Ensure there is no more input */
  ensure_end_input(str, true, "temporal");

  /* Second parsing */
  *str = bak;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    sequences[i] = tcontseq_parse(str, temptype, interp, false, true);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Parse a temporal value from the buffer (dispatch function)
 * @param[in] str Input string
 * @param[in] temptype Temporal type
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

  /* Allow spaces after the Interpolation */
  p_whitespace(str);

  if (**str != '{' && **str != '[' && **str != '(')
    result = (Temporal *) tinstant_parse(str, temptype, true, true);
  else if (**str == '[' || **str == '(')
    result = (Temporal *) tcontseq_parse(str, temptype, interp, true, true);
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
