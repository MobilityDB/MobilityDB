/*****************************************************************************
 *
 * tpoint_parser.c
 *    Functions for parsing temporal points.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#include "tpoint_parser.h"

#include "temporaltypes.h"
#include "oidcache.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "temporal_parser.h"
#include "stbox.h"

/*****************************************************************************/

/**
 * Parse a spatiotemporal box value from the buffer
 */
STBOX *
stbox_parse(char **str)
{
  double xmin, xmax, ymin, ymax,
    zmin = 0, zmax = 0; /* make Codacy quiet */
  TimestampTz tmin, tmax;
  bool hasx = false, hasz = false, hast = false, geodetic = false;
  int srid = 0;
  bool hassrid = false;

  p_whitespace(str);
  if (strncasecmp(*str,"SRID=",5) == 0)
  {
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    /* Delimiter will be either ',' or ';' depending on whether interpolation
       is given after */
    while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
    {
      srid = srid * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str to the start of the temporal point */
    *str += delim + 1;
    hassrid = true;
  }
  if (strncasecmp(*str, "STBOX", 5) == 0)
  {
    *str += 5;
    p_whitespace(str);
  }
  else if (strncasecmp(*str, "GEODSTBOX", 9) == 0)
  {
    *str += 9;
    geodetic = true;
    p_whitespace(str);
    if (!hassrid)
      srid = 4326;
  }
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX")));

  if (strncasecmp(*str, "ZT", 2) == 0)
  {
    hasz = hast = true;
    *str += 2;
  }
  else if (strncasecmp(*str, "Z", 1) == 0)
  {
    *str += 1;
    hasz = true;
  }
  else if (strncasecmp(*str, "T", 1) == 0)
  {
    *str += 1;
    hast = true;
  }
  p_whitespace(str);

  /* Parse double opening parenthesis */
  if (!p_oparen(str) || !p_oparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX: Missing opening parenthesis")));

  /* Determine whether there is an XY(Z) dimension */
  p_whitespace(str);
  if (((*str)[0]) != ',')
    hasx = true;

  if (!hasx && !hast)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX")));
  if (!hasx && hassrid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("An SRID is specified but not coordinates are given")));

  if (hasx)
  {
    /* xmin */
    xmin = double_parse(str);
    /* ymin */
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    ymin = double_parse(str);
    if (hasz || geodetic)
    {
      p_whitespace(str);
      p_comma(str);
      p_whitespace(str);
      zmin = double_parse(str);
    }
  }
  else
  {
    /* Empty XY(Z) dimension */
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    p_comma(str);
  }
  if (hast)
  {
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    /* The next instruction will throw an exception if it fails */
    tmin = timestamp_parse(str);
  }
  p_whitespace(str);
  if (!p_cparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX: Missing closing parenthesis")));
  p_whitespace(str);
  p_comma(str);
  p_whitespace(str);

  /* Parse upper bounds */
  if (!p_oparen(str))
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX: Missing opening parenthesis")));

  if (hasx)
  {
    xmax = double_parse(str);
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    ymax = double_parse(str);
    if (hasz || geodetic)
    {
      p_whitespace(str);
      p_comma(str);
      p_whitespace(str);
      zmax = double_parse(str);
    }
  }
  else
  {
    /* Empty XY dimensions */
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    p_comma(str);
  }
  if (hast)
  {
    p_whitespace(str);
    p_comma(str);
    /* The next instruction will throw an exception if it fails */
    tmax = timestamp_parse(str);
  }
  p_whitespace(str);
  if (!p_cparen(str) || !p_cparen(str) )
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Could not parse STBOX: Missing closing parenthesis")));

  return stbox_make(hasx, hasz, hast, geodetic, srid, xmin, xmax, ymin, ymax,
    zmin, zmax, tmin, tmax);
}

/*****************************************************************************/

/**
 * Parse a temporal point value of instant duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in] make Set to false for the first pass to do not create the instant
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TInstant *
tpointinst_parse(char **str, Oid basetype, bool end, bool make, int *tpoint_srid)
{
  p_whitespace(str);
  /* The next instruction will throw an exception if it fails */
  Datum geo = basetype_parse(str, basetype);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geo);
  int geo_srid = gserialized_get_srid(gs);
  ensure_point_type(gs);
  ensure_non_empty(gs);
  ensure_has_not_M_gs(gs);
  if (*tpoint_srid != SRID_UNKNOWN && geo_srid != SRID_UNKNOWN && *tpoint_srid != geo_srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("Geometry SRID (%d) does not match temporal type SRID (%d)",
      geo_srid, *tpoint_srid)));
  if (basetype == type_oid(T_GEOMETRY))
  {
    if (*tpoint_srid != SRID_UNKNOWN && geo_srid == SRID_UNKNOWN)
      gserialized_set_srid(gs, *tpoint_srid);
    if (*tpoint_srid == SRID_UNKNOWN && geo_srid != SRID_UNKNOWN)
      *tpoint_srid = geo_srid;
  }
  else
  {
    if (*tpoint_srid != SRID_UNKNOWN && geo_srid == SRID_DEFAULT)
      gserialized_set_srid(gs, *tpoint_srid);
    if (*tpoint_srid == SRID_UNKNOWN && geo_srid != SRID_DEFAULT)
      *tpoint_srid = geo_srid;
  }
  /* The next instruction will throw an exception if it fails */
  TimestampTz t = timestamp_parse(str);
  ensure_end_input(str, end);
  TInstant *result = make ?
    tinstant_make(PointerGetDatum(gs), t, basetype) : NULL;
  pfree(gs);
  return result;
}

/**
 * Parse a temporal point value of instant set duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TInstantSet *
tpointinstset_parse(char **str, Oid basetype, int *tpoint_srid)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tpoint_parse */
  p_obrace(str);

  /* First parsing */
  char *bak = *str;
  tpointinst_parse(str, basetype, false, false, tpoint_srid);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tpointinst_parse(str, basetype, false, false, tpoint_srid);
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
    instants[i] = tpointinst_parse(str, basetype, false, true, tpoint_srid);
  }
  p_cbrace(str);
  return tinstantset_make_free(instants, count);
}

/**
 * Parse a temporal point value of sequence duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence set has linear interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in] make Set to false for the first pass to do not create the instant
 * @param[in] tpoint_srid SRID of the temporal point
*/
static TSequence *
tpointseq_parse(char **str, Oid basetype, bool linear, bool end, bool make, int *tpoint_srid)
{
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function tpoint_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  char *bak = *str;
  tpointinst_parse(str, basetype, false, false, tpoint_srid);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tpointinst_parse(str, basetype, false, false, tpoint_srid);
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
    instants[i] = tpointinst_parse(str, basetype, false, true, tpoint_srid);
  }
  p_cbracket(str);
  p_cparen(str);
  return tsequence_make_free(instants, count, lower_inc, upper_inc,
    linear, NORMALIZE);
}

/**
 * Parse a temporal point value of sequence set duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence set has linear interpolation
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TSequenceSet *
tpointseqset_parse(char **str, Oid basetype, bool linear, int *tpoint_srid)
{
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tpoint_parse */
  p_obrace(str);

  /* First parsing */
  char *bak = *str;
  tpointseq_parse(str, basetype, linear, false, false, tpoint_srid);
  int count = 1;
  while (p_comma(str))
  {
    count++;
    tpointseq_parse(str, basetype, linear, false, false, tpoint_srid);
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
    sequences[i] = tpointseq_parse(str, basetype, linear, false, true,
      tpoint_srid);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Parse a temporal point value from the buffer (dispatch function)
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 */
Temporal *
tpoint_parse(char **str, Oid basetype)
{
  int tpoint_srid = 0;
  p_whitespace(str);

  /* Starts with "SRID=". The SRID specification must be gobbled for all
   * durations excepted TInstant. We cannot use the atoi() function
   * because this requires a string terminated by '\0' and we cannot
   * modify the string in case it must be passed to the tpointinst_parse
   * function. */
  char *bak = *str;
  if (strncasecmp(*str, "SRID=", 5) == 0)
  {
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    tpoint_srid = 0;
    /* Delimiter will be either ',' or ';' depending on whether interpolation
       is given after */
    while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
    {
      tpoint_srid = tpoint_srid * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str to the start of the temporal point */
    *str += delim + 1;
  }
  /* We cannot ensure that the SRID is geodetic for geography since
   * the srid_is_latlong function is not exported by PostGIS
  if (basetype == type_oid(T_GEOGRAPHY))
    srid_is_latlong(fcinfo, tpoint_srid);
   */

  bool linear = linear_interpolation(basetype);
  /* Starts with "Interp=Stepwise" */
  if (strncasecmp(*str,"Interp=Stepwise;",16) == 0)
  {
    /* Move str after the semicolon */
    *str += 16;
    linear = false;
  }
  Temporal *result = NULL; /* keep compiler quiet */
  /* Determine the type of the temporal point */
  if (**str != '{' && **str != '[' && **str != '(')
  {
    /* Pass the SRID specification */
    *str = bak;
    result = (Temporal *)tpointinst_parse(str, basetype, true, true, &tpoint_srid);
  }
  else if (**str == '[' || **str == '(')
    result = (Temporal *)tpointseq_parse(str, basetype, linear, true, true, &tpoint_srid);
  else if (**str == '{')
  {
    bak = *str;
    p_obrace(str);
    p_whitespace(str);
    if (**str == '[' || **str == '(')
    {
      *str = bak;
      result = (Temporal *)tpointseqset_parse(str, basetype, linear, &tpoint_srid);
    }
    else
    {
      *str = bak;
      result = (Temporal *)tpointinstset_parse(str, basetype, &tpoint_srid);
    }
  }
  return result;
}

/*****************************************************************************/
