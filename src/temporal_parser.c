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
 * @file temporal_parser.c
 * Functions for parsing time types and temporal types.
 *
 * Many functions make two passes for parsing, the first one to obtain the
 * number of elements in order to do memory allocation with `palloc`, the
 * second one to create the type. This is the only approach we can see at the
 * moment which is both correct and simple.
 */

#include "temporal_parser.h"

#include "periodset.h"
#include "period.h"
#include "timestampset.h"
#include "temporaltypes.h"
#include "temporal_util.h"

/*****************************************************************************/

/**
 * Input a white space from the buffer
 */
void
p_whitespace(char **str)
{
  while (**str == ' ' || **str == '\n' || **str == '\r' || **str == '\t')
    *str += 1;
}

/**
 * Ensure there is no more input excepted white spaces
 */
void
ensure_end_input(char **str, bool end)
{
  if (end)
  {
    p_whitespace(str);
    if (**str != 0)
      ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("Could not parse temporal value")));
  }
}

/**
 * Input an opening brace from the buffer
 */
bool
p_obrace(char **str)
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
 * Input a closing brace from the buffer
 */
bool
p_cbrace(char **str)
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
 * Input an opening bracket from the buffer
 */
bool
p_obracket(char **str)
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
 * Input a closing bracket from the buffer
 */
bool
p_cbracket(char **str)
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
 * Input an opening parenthesis from the buffer
 */
bool
p_oparen(char **str)
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
 * Input a closing parenthesis from the buffer
 */
bool
p_cparen(char **str)
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
 * Input a comma from the buffer
 */
bool
p_comma(char **str)
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
 * Input a double from the buffer
 *
 * @param[inout] str Pointer to the current position of the input buffer
 */

double
double_parse(char **str)
{
  char *nextstr = *str;
  double result = strtod(*str, &nextstr);
  if (*str == nextstr)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Invalid input syntax for type double")));
  *str = nextstr;
  return result;
}

/**
 * Parse a base value from the buffer
 */
Datum
basetype_parse(char **str, Oid basetype)
{
  p_whitespace(str);
  int delim = 0;
  bool isttext = false;
  /* ttext values must be enclosed between double quotes */
  if (**str == '"')
  {
    isttext = true;
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
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse element value")));
  (*str)[delim] = '\0';
  Datum result = call_input(basetype, *str);
  if (isttext)
    /* Replace the double quote */
    (*str)[delim++] = '"';
  else
    /* Replace the at */
    (*str)[delim] = '@';
  /* since we know there's an @ here, let's take it with us */
  *str += delim + 1;
  return result;
}

/*****************************************************************************/

/**
 * Parse a temporal box value from the buffer
 */
TBOX *
tbox_parse(char **str)
{
  double xmin, xmax;
  TimestampTz tmin, tmax;
  bool hasx = false, hast = false;

  p_whitespace(str);
  if (strncasecmp(*str, "TBOX", 4) == 0)
  {
    *str += 4;
    p_whitespace(str);
  }
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX")));

  /* Parse double opening parenthesis */
  if (!p_oparen(str) || !p_oparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX: Missing opening parenthesis")));

  /* Determine whether there is an X dimension */
  p_whitespace(str);
  if (((*str)[0]) != ',')
  {
    xmin = double_parse(str);
    hasx = true;
  }

  p_whitespace(str);
  p_comma(str);
  p_whitespace(str);

  /* Determine whether there is a T dimension */
  if (((*str)[0]) != ')')
  {
    tmin = timestamp_parse(str);
    hast = true;
  }

  if (! hasx && ! hast)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX: Both value and time dimensions are empty")));

  p_whitespace(str);
  if (!p_cparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX: Missing closing parenthesis")));
  p_whitespace(str);
  p_comma(str);
  p_whitespace(str);

  /* Parse upper bounds */
  if (!p_oparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX: Missing opening parenthesis")));

  if (hasx)
    xmax = double_parse(str);
  p_whitespace(str);
  p_comma(str);
  p_whitespace(str);
  if (hast)
    tmax = timestamp_parse(str);
  p_whitespace(str);
  if (!p_cparen(str) || !p_cparen(str) )
  ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse TBOX: Missing closing parenthesis")));

  return tbox_make(hasx, hast, xmin, xmax, tmin, tmax);
}

/*****************************************************************************/
/* Time Types */

/**
 * Parse a timestamp value from the buffer
 */
TimestampTz
timestamp_parse(char **str)
{
  p_whitespace(str);
  int delim = 0;
  while ((*str)[delim] != ',' && (*str)[delim] != ']' && (*str)[delim] != ')' &&
    (*str)[delim] != '}' && (*str)[delim] != '\0')
    delim++;
  char bak = (*str)[delim];
  (*str)[delim] = '\0';
  Datum result = call_input(TIMESTAMPTZOID, *str);
  (*str)[delim] = bak;
  *str += delim;
  return result;
}

/**
 * Parse a period value from the buffer
 */
Period *
period_parse(char **str, bool make)
{
  bool lower_inc = false, upper_inc = false;
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse period")));

  TimestampTz lower = timestamp_parse(str);
  p_comma(str);
  TimestampTz upper = timestamp_parse(str);

  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse period")));

  if (! make)
    return NULL;
  return period_make(lower, upper, lower_inc, upper_inc);
}

/**
 * Parse a timestamp set value from the buffer
 */
TimestampSet *
timestampset_parse(char **str)
{
  if (!p_obrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse timestamp set")));

  /* First parsing */
  char *bak = *str;
  timestamp_parse(str);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    timestamp_parse(str);
  }
  if (!p_cbrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse timestamp set")));

  *str = bak;
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    times[i] = timestamp_parse(str);
  }
  p_cbrace(str);
  return timestampset_make_free(times, count);
}

/**
 * Parse a period set value from the buffer
 */
PeriodSet *
periodset_parse(char **str)
{
  if (!p_obrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse period set")));

  /* First parsing */
  char *bak = *str;
  period_parse(str, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    period_parse(str, false);
  }
  if (!p_cbrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse period set")));

  /* Second parsing */
  *str = bak;
  Period **periods = palloc(sizeof(Period *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    periods[i] = period_parse(str, true);
  }
  p_cbrace(str);
  PeriodSet *result = periodset_make_free(periods, count, NORMALIZE);
  return result;
}

/*****************************************************************************/
/* Temporal Types */

/**
 * Parse a temporal instant value from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no more input after the instant
 * @param[in] make Set to false for the first pass to do not create the instant
 */
TInstant *
tinstant_parse(char **str, Oid basetype, bool end, bool make)
{
  p_whitespace(str);
  /* The next two instructions will throw an exception if they fail */
  Datum elem = basetype_parse(str, basetype);
  TimestampTz t = timestamp_parse(str);
  ensure_end_input(str, end);
  if (! make)
    return NULL;
  return tinstant_make(elem, t, basetype);
}

/**
 * Parse a temporal instant set value from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 */
static TInstantSet *
tinstantset_parse(char **str, Oid basetype)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);

  /* First parsing */
  char *bak = *str;
  tinstant_parse(str, basetype, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tinstant_parse(str, basetype, false, false);
  }
  if (!p_cbrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse temporal value")));
  ensure_end_input(str, true);

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    instants[i] = tinstant_parse(str, basetype, false, true);
  }
  p_cbrace(str);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * Parse a temporal sequence value from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence has linear interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in] make Set to false for the first pass to do not create the instant
 */
static TSequence *
tsequence_parse(char **str, Oid basetype, bool linear, bool end, bool make)
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
  char *bak = *str;
  tinstant_parse(str, basetype, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tinstant_parse(str, basetype, false, false);
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse temporal value")));
  ensure_end_input(str, end);
  if (! make)
    return NULL;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    instants[i] = tinstant_parse(str, basetype, false, true);
  }
  p_cbracket(str);
  p_cparen(str);
  return tsequence_make_free(instants, count, lower_inc, upper_inc,
    linear, NORMALIZE);
}

/**
 * Parse a temporal sequence set value from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence set has linear interpolation
 */
static TSequenceSet *
tsequenceset_parse(char **str, Oid basetype, bool linear)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function temporal_parse */
  p_obrace(str);

  /* First parsing */
  char *bak = *str;
  tsequence_parse(str, basetype, linear, false, false);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tsequence_parse(str, basetype, linear, false, false);
  }
  if (!p_cbrace(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse temporal value")));
  ensure_end_input(str, true);

  /* Second parsing */
  *str = bak;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    sequences[i] = tsequence_parse(str, basetype, linear, false, true);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Parse a temporal value from the buffer (dispatch function)
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 */
Temporal *
temporal_parse(char **str, Oid basetype)
{
  p_whitespace(str);
  Temporal *result = NULL;  /* keep compiler quiet */
  bool linear = base_type_continuous(basetype);
  /* Starts with "Interp=Stepwise" */
  if (strncasecmp(*str, "Interp=Stepwise;", 16) == 0)
  {
    /* Move str after the semicolon */
    *str += 16;
    linear = false;
  }
  if (**str != '{' && **str != '[' && **str != '(')
    result = (Temporal *) tinstant_parse(str, basetype, true, true);
  else if (**str == '[' || **str == '(')
    result = (Temporal *) tsequence_parse(str, basetype, linear, true, true);
  else if (**str == '{')
  {
    char *bak = *str;
    p_obrace(str);
    p_whitespace(str);
    if (**str == '[' || **str == '(')
    {
      *str = bak;
      result = (Temporal *) tsequenceset_parse(str, basetype, linear);
    }
    else
    {
      *str = bak;
      result = (Temporal *) tinstantset_parse(str, basetype);
    }
  }
  return result;
}

/*****************************************************************************/
