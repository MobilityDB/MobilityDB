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
 * @brief Functions for parsing temporal rigid geometries
 */

#include "rgeo/trgeo_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "rgeo/trgeo_all.h"

/*****************************************************************************/

/**
 * @brief Parse a temporal rigid geometry instant from the input buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no more input after the sequence
 * @param[in,out] temp_srid SRID of the temporal rigid geometry
 * @param[out] result New instant, may be NULL
 * @param[in] geom Reference geometry
 */
bool 
trgeoinst_parse(const char **str, meosType temptype, bool end,
  int *temp_srid, const GSERIALIZED *geom, TInstant **result)
{
  Datum base;
  if (! spatial_parse_elem(str, temptype, '@', temp_srid, &base))
    return false;
  Pose *pose = DatumGetPoseP(base);

  p_delimchar(str, '@');

  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND ||
    /* Ensure there is no more input */
    (end && ! ensure_end_input(str, meostype_name(temptype))))
  {
    pfree(pose);
    return false;
  }

  if (result)
    *result = trgeoinst_make(geom, pose, t);
  pfree(pose);
  return true;
}

/**
 * @brief Parse a temporal discrete sequence spatial value from the buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in,out] temp_srid SRID of the temporal rigid geometry
 * @param[in] geom Reference geometry
 */
TSequence *
trgeoseq_disc_parse(const char **str, meosType temptype, int *temp_srid,
  GSERIALIZED *geom)
{
  const char *type_str = meostype_name(temptype);
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function #tspatial_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! trgeoinst_parse(str, temptype, false, temp_srid, geom, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! trgeoinst_parse(str, temptype, false, temp_srid, geom, NULL))
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
    trgeoinst_parse(str, temptype, false, temp_srid, geom, &instants[i]);
  }
  p_cbrace(str);
  TSequence *result = trgeoseq_make_free(geom, instants, count, true, true,
    DISCRETE, NORMALIZE_NO);
  return result;
}

/**
 * @brief Parse a temporal sequence spatial value from the input buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] temp_srid SRID of the temporal rigid geometry
 * @param[in] geom Reference geometry
 * @param[out] result New sequence, may be NULL
 */
bool
trgeoseq_cont_parse(const char **str, meosType temptype, interpType interp, 
  bool end, int *temp_srid, const GSERIALIZED *geom, TSequence **result)
{
  const char *type_str = meostype_name(temptype);
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function tspatial_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  const char *bak = *str;
  if (! trgeoinst_parse(str, temptype, false, temp_srid, geom, NULL))
    return false;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! trgeoinst_parse(str, temptype, false, temp_srid, geom, NULL))
      return false;
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing closing bracket/parenthesis", 
      type_str);
    return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, type_str))
    return false;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    trgeoinst_parse(str, temptype, false, temp_srid, geom, &instants[i]);
  }
  p_cbracket(str);
  p_cparen(str);
  if (result)
    *result = trgeoseq_make_free(geom, instants, count, lower_inc, upper_inc,
      interp, NORMALIZE);
  return true;

}

/**
 * @brief Parse a temporal sequence set spatial value from the input buffer
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in,out] temp_srid SRID of the temporal rigid geometry
 * @param[in] geom Reference geometry
 */
TSequenceSet *
trgeoseqset_parse(const char **str, meosType temptype, interpType interp,
  int *temp_srid, const GSERIALIZED *geom)
{
  const char *type_str = meostype_name(temptype);
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tspatial_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! trgeoseq_cont_parse(str, temptype, interp, false, temp_srid, geom,
      NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! trgeoseq_cont_parse(str, temptype, interp, false, temp_srid, geom,
        NULL))
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
    trgeoseq_cont_parse(str, temptype, interp, false, temp_srid, geom, 
      &sequences[i]);
  }
  p_cbrace(str);
  return trgeoseqset_make_free(geom, sequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Parse the reference geometry of a temporal rigid geometry
 * @param[in] str Input string
 * @param[in,out] temp_srid SRID of the temporal rigid geometry
 */
GSERIALIZED *
trgeo_parse_geom(const char **str, int32_t temp_srid)
{
  p_whitespace(str);
  int pos = 0;
  while ((*str)[pos] != ';' && (*str)[pos] != '\0')
    pos++;
  if ((*str)[pos] == '\0')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse element value");
    return NULL;
  }
  char *str1 = palloc(sizeof(char) * (pos + 1));
  strncpy(str1, *str, pos);
  str1[pos] = '\0';
  Datum geom;
  bool success = basetype_in(str1, T_GEOMETRY, false, &geom);
  pfree(str1);
  if (! success)
    return NULL;
  /* The delimeter is consumed */
  *str += pos + 1;

  GSERIALIZED *result = DatumGetGserializedP(geom);
  ensure_not_empty(result);
  ensure_has_not_M_geo(result);
  /* If one of the SRID of the temporal rigid geometry and of the geometry
   * is SRID_UNKNOWN and the other not, copy the SRID */
  int32_t geo_srid = gserialized_get_srid(result);
  if (temp_srid == SRID_UNKNOWN && geo_srid != SRID_UNKNOWN)
    temp_srid = geo_srid;
  else if (temp_srid != SRID_UNKNOWN && geo_srid == SRID_UNKNOWN)
    gserialized_set_srid(result, temp_srid);
  /* If the SRID of the temporal rigid geometry and of the geometry do not match */
  else if (temp_srid != SRID_UNKNOWN && geo_srid != SRID_UNKNOWN &&
    temp_srid != geo_srid)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Geometry SRID (%d) does not match temporal type SRID (%d)",
      geo_srid, temp_srid);
    pfree(result);
    return NULL;
  }
  return result;
}

/**
 * @brief Parse a temporal rigid geometry from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 */
Temporal *
trgeo_parse(const char **str, meosType temptype)
{
  p_whitespace(str);

  /* Get the SRID if it is given */
  int temp_srid = SRID_UNKNOWN;
  srid_parse(str, &temp_srid);

  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  /* Starts with "Interp=Step" */
  if (strncasecmp(*str, "Interp=Step;", 12) == 0)
  {
    /* Move str after the semicolon */
    *str += 12;
    interp = STEP;
  }

  /* Parse the reference geometry */
  GSERIALIZED *geom = NULL;
  if (temptype == T_TRGEOMETRY)
  {
    geom = trgeo_parse_geom(str, temp_srid);
    if (! geom)
      return NULL;
  }

  p_whitespace(str);

  const char *bak = *str;
  Temporal *result = NULL; /* keep compiler quiet */
  /* Determine the subtype of the temporal rigid geometry and call the
   * function corresponding to the subtype passing the SRID */
  if (**str != '{' && **str != '[' && **str != '(')
  {
    TInstant *inst;
    if (! trgeoinst_parse(str, temptype, true, &temp_srid, geom, &inst))
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq;
    if (! trgeoseq_cont_parse(str, temptype, interp, true, &temp_srid, geom, 
        &seq))
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
      result = (Temporal *) trgeoseqset_parse(str, temptype, interp,
        &temp_srid, geom);
    }
    else
    {
      *str = bak;
      result = (Temporal *) trgeoseq_disc_parse(str, temptype, &temp_srid,
        geom);
    }
  }
  return result;
}

/*****************************************************************************/
