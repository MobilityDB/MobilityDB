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
 * @brief Functions for parsing temporal points
 */

#include "geo/tgeo_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************/

/**
 * @brief Parse a SRID specification from the buffer
 */
bool
srid_parse(const char **str, int *srid)
{
  int32_t srid_read = SRID_UNKNOWN;
  bool result = false;
  p_whitespace(str);
  /* Starts with "SRID=". The SRID specification must be gobbled for all
   * types excepted TInstant. We cannot use the atoi() function
   * because this requires a string terminated by '\0' and we cannot
   * modify the string in case it must be passed to the #tcbufferinst_parse
   * function. */
  if (pg_strncasecmp(*str, "SRID=", 5) == 0)
  {
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    /* Delimiter will be either ',' or ';' depending on whether interpolation
       is given after */
    while ((*str)[delim] != ',' && (*str)[delim] != ';' && 
      (*str)[delim] != '\0')
    {
      srid_read = srid_read * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str after the separator */
    *str += delim + 1;
    result = true;
  }
  *srid = srid_read;
  return result;
}

/**
 * @brief Parse a spatiotemporal box from the buffer
 */
STBox *
stbox_parse(const char **str)
{
  /* make compiler quiet */
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  Span period;
  bool hasx = false, hasz = false, hast = false, geodetic = false;
  const char *type_str = "spatiotemporal box";

  /* Determine whether there is an SRID */
  int32_t srid;
  bool hassrid = srid_parse(str, &srid);

  /* Determine whether the box is geodetic or not */
  if (pg_strncasecmp(*str, "STBOX", 5) == 0)
  {
    *str += 5;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "GEODSTBOX", 9) == 0)
  {
    *str += 9;
    geodetic = true;
    p_whitespace(str);
    if (! hassrid)
      srid = WGS84_SRID;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse spatiotemporal box");
    return NULL;
  }

  /* Determine whether the box has X, Z, and/or T dimensions */
  if (pg_strncasecmp(*str, "ZT", 2) == 0)
  {
    hasx = hasz = hast = true;
    *str += 2;
  }
  else if (pg_strncasecmp(*str, "XT", 2) == 0)
  {
    hasx = hast = true;
    *str += 2;
  }
  else if (pg_strncasecmp(*str, "Z", 1) == 0)
  {
    *str += 1;
    hasx = hasz = true;
  }
  else if (pg_strncasecmp(*str, "X", 1) == 0)
  {
    *str += 1;
    hasx = true;
  }
  else if (pg_strncasecmp(*str, "T", 1) == 0)
  {
    *str += 1;
    hast = true;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse spatiotemporal box: Missing dimension information");
    return NULL;
  }

  /* Parse external opening parenthesis (if both space and time dimensions) */
  if (hast)
  {
    p_whitespace(str);
    if (! ensure_oparen(str, type_str))
      return NULL;
  }

  if (hasx)
  {
    /* Parse enclosing opening parenthesis */
    p_whitespace(str);
    if (! ensure_oparen(str, type_str))
      return NULL;

    /* Parse lower bounds */
    p_whitespace(str);
    if (! ensure_oparen(str, type_str))
      return NULL;
    /* xmin */
    if (! double_parse(str, &xmin))
      return NULL;
    /* ymin */
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    if (! double_parse(str, &ymin))
      return NULL;
    if (hasz)
    {
      /* zmin */
      p_whitespace(str);
      p_comma(str);
      p_whitespace(str);
      if (! double_parse(str, &zmin))
        return NULL;
    }
    p_whitespace(str);
    if (! ensure_cparen(str, type_str))
      return NULL;

    /* Parse optional comma */
    p_whitespace(str);
    p_comma(str);

    /* Parse upper bounds */
    p_whitespace(str);
    if (! ensure_oparen(str, type_str))
      return NULL;
    /* xmax */
    if (! double_parse(str, &xmax))
      return NULL;
    /* ymax */
    p_whitespace(str);
    p_comma(str);
    p_whitespace(str);
    if (! double_parse(str, &ymax))
      return NULL;
    if (hasz)
    {
      /* zmax */
      p_whitespace(str);
      p_comma(str);
      p_whitespace(str);
      if (! double_parse(str, &zmax))
        return NULL;
    }
    p_whitespace(str);
    if (! ensure_cparen(str, type_str))
      return NULL;

    /* Parse enclosing closing parenthesis */
    p_whitespace(str);
    if (! ensure_cparen(str, type_str))
      return NULL;

    if (hast)
    {
      p_whitespace(str);
      if (((*str)[0]) == ',')
      {
        hasx = true;
        *str += 1;
        p_whitespace(str);
      }
    }
  }

  if (hast)
  {
    if (! span_parse(str, T_TSTZSPAN, false, &period))
      return NULL;
    /* Parse external closing parenthesis (if both space and time dimensions) */
    p_whitespace(str);
    if (! ensure_cparen(str, type_str))
      return NULL;
  }

  /* Ensure there is no more input */
  if (! ensure_end_input(str, type_str))
    return NULL;

  return stbox_make(hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax, zmin,
    zmax, hast ? &period : NULL);
}

/*****************************************************************************/

/**
 * @brief Parse a geometry/gegraphy from the buffer
 * @param[in] str Input string
 * @param[in] basetype Base type
 * @param[in] sep Separation character
 * @param[in,out] srid SRID of the result. If it is SRID_UNKNOWN, it may take
 * the value from the geometry if it is not SRID_UNKNOWN.
 * @param[out] result New geometry, may be NULL
 */
bool 
geo_parse(const char **str, meosType basetype, char sep, int *srid,
  GSERIALIZED **result)
{
  p_whitespace(str);
  /* The next instruction will throw an exception if it fails */
  Datum geo;
  if (! basetype_parse(str, basetype, sep, &geo))
    return false;
  GSERIALIZED *gs = DatumGetGserializedP(geo);
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
  {
    pfree(gs);
    return false;
  }
  /* If one of the SRID of the temporal point and of the geometry
   * is SRID_UNKNOWN and the other not, copy the SRID */
  int gs_srid = gserialized_get_srid(gs);
  if (*srid == SRID_UNKNOWN && gs_srid != SRID_UNKNOWN)
    *srid = gs_srid;
  else if (*srid != SRID_UNKNOWN &&
    ( gs_srid == SRID_UNKNOWN || gs_srid == SRID_DEFAULT ))
    gserialized_set_srid(gs, *srid);
  /* If the SRID of the temporal point and of the geometry do not match */
  else if (*srid != SRID_UNKNOWN && gs_srid != SRID_UNKNOWN &&
    *srid != gs_srid)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Geometry SRID (%d) does not match temporal type SRID (%d)",
      gs_srid, *srid);
    pfree(gs);
    return false;
  }
  if (result)
    *result = gs;
  else 
    pfree(gs);
  return true;
}

/*****************************************************************************/

/**
 * @brief Parse a temporal geo instant from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tgeo_srid SRID of the temporal geo
 * @param[out] result New instant, may be NULL
 */
bool 
tgeoinst_parse(const char **str, meosType temptype, bool end, int *tgeo_srid,
  TInstant **result)
{
  meosType basetype = temptype_basetype(temptype);
  GSERIALIZED *gs;
  if (! geo_parse(str, basetype, '@', tgeo_srid, &gs))
    return false;

  p_sepchar(str, '@');

  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND ||
    /* Ensure there is no more input */
    (end && ! ensure_end_input(str, "temporal geo")))
  {
    pfree(gs);
    return false;
  }

  if (result)
    *result = tinstant_make(PointerGetDatum(gs), temptype, t);
  pfree(gs);
  return true;
}

/**
 * @brief Parse a temporal geo discrete sequence from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in,out] tgeo_srid SRID of the temporal geo
 */
TSequence *
tgeoseq_disc_parse(const char **str, meosType temptype, int *tgeo_srid)
{
  const char *type_str = "temporal geo";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function #tgeo_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tgeoinst_parse(str, temptype, false, tgeo_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tgeoinst_parse(str, temptype, false, tgeo_srid, NULL))
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
    tgeoinst_parse(str, temptype, false, tgeo_srid, &instants[i]);
  }
  p_cbrace(str);
  return tsequence_make_free(instants, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Parse a temporal geo sequence from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tgeo_srid SRID of the temporal geo
 * @param[out] result New sequence, may be NULL
 */
bool
tgeoseq_cont_parse(const char **str, meosType temptype, interpType interp,
  bool end, int *tgeo_srid, TSequence **result)
{
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function tgeo_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  const char *bak = *str;
  if (! tgeoinst_parse(str, temptype, false, tgeo_srid, NULL))
    return false;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tgeoinst_parse(str, temptype, false, tgeo_srid, NULL))
      return false;
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse temporal geo value: Missing closing bracket/parenthesis");
    return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "temporal geo"))
    return false;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tgeoinst_parse(str, temptype, false, tgeo_srid, &instants[i]);
  }
  p_cbracket(str);
  p_cparen(str);
  if (result)
    *result = tsequence_make((const TInstant **) instants, count,
      lower_inc, upper_inc, interp, NORMALIZE);
  pfree_array((void **) instants, count);
  return true;
}

/**
 * @brief Parse a temporal geo sequence set from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in,out] tgeo_srid SRID of the temporal geo
 */
TSequenceSet *
tgeoseqset_parse(const char **str, meosType temptype, interpType interp,
  int *tgeo_srid)
{
  const char *type_str = "temporal geo";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tgeo_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tgeoseq_cont_parse(str, temptype, interp, false, tgeo_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tgeoseq_cont_parse(str, temptype, interp, false, tgeo_srid, NULL))
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
    tgeoseq_cont_parse(str, temptype, interp, false, tgeo_srid,
      &sequences[i]);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Parse a temporal geo value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 */
Temporal *
tgeo_parse(const char **str, meosType temptype)
{
  const char *bak = *str;
  p_whitespace(str);

  /* Determine whether there is an SRID */
  int tgeo_srid;
  srid_parse(str, &tgeo_srid);

  /* Ensure that the SRID is geodetic for geography */
  if (temptype == T_TGEOGRAPHY && tgeo_srid != SRID_UNKNOWN && 
      ! ensure_srid_is_latlong(tgeo_srid))
    return NULL;

  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  /* Starts with "Interp=Step" */
  if (pg_strncasecmp(*str, "Interp=Step;", 12) == 0)
  {
    /* Move str after the semicolon */
    *str += 12;
    interp = STEP;
  }

  /* Allow spaces after the SRID and/or Interpolation */
  p_whitespace(str);

  Temporal *result = NULL; /* keep compiler quiet */
  /* Determine the type of the temporal geo */
  if (**str != '{' && **str != '[' && **str != '(')
  {
    /* Pass the SRID specification */
    *str = bak;
    TInstant *inst;
    if (! tgeoinst_parse(str, temptype, true, &tgeo_srid, &inst))
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq;
    if (! tgeoseq_cont_parse(str, temptype, interp, true, &tgeo_srid, &seq))
      return NULL;
    result = (Temporal *) seq;
  }
  else if (**str == '{')
  {
    bak = *str;
    p_obrace(str);
    p_whitespace(str);
    if (**str == '[' || **str == '(')
    {
      *str = bak;
      result = (Temporal *) tgeoseqset_parse(str, temptype, interp,
        &tgeo_srid);
    }
    else
    {
      *str = bak;
      result = (Temporal *) tgeoseq_disc_parse(str, temptype, &tgeo_srid);
    }
  }
  return result;
}

/**
 * @brief Parse a temporal point value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 */
Temporal *
tpoint_parse(const char **str, meosType temptype)
{
  Temporal *result = tgeo_parse(str, temptype);
  const TInstant *inst = temporal_start_inst(result);
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_val(inst));
  if (! ensure_point_type(gs))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geometry point from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tgeompoint_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tpoint_parse(&str, T_TGEOMPOINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geography point from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tgeogpoint_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tpoint_parse(&str, T_TGEOGPOINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geometry from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tgeometry_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tgeo_parse(&str, T_TGEOMETRY);
}
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geography from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tgeography_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tgeo_parse(&str, T_TGEOGRAPHY);
}
#endif /* MEOS */

/*****************************************************************************/
