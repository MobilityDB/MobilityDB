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
 * @brief Functions for parsing temporal pose objects.
 */

#include "pose/tpose_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_parser.h"
// #include "pose/tpose.h"
#include "pose/tpose_static.h"

/*****************************************************************************/

/**
 * Parse a pose value from the buffer
 */
Pose *
pose_parse(const char **str, bool end)
{
  Pose *result;
  int srid = 0;
  bool hasZ = false;
  const char *type_str = "pose";

  /* Determine whether the box has an SRID */
  p_whitespace(str);
  if (pg_strncasecmp(*str, "SRID=", 5) == 0)
  {
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    /* Delimiter is ';' */
    while ((*str)[delim] != ';' && (*str)[delim] != '\0')
    {
      srid = srid * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str to the start of the temporal pose */
    *str += delim + 1;
  }

  if (strncasecmp(*str,"POSE",4) == 0)
  {
    *str += 4;
    p_whitespace(str);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse pose value");
    return NULL;
  }

  /* Determine whether the pose is 3D */
  if (strncasecmp(*str,"Z",1) == 0)
  {
    hasZ = true;
    *str += 1;
    p_whitespace(str);
  }

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse first 3 values: (x, y, theta) in 2D or (x, y, z, ...) in 3D */
  double x, y, z;
  p_whitespace(str);
  if (! double_parse(str, &x)) return NULL;
  p_whitespace(str); p_comma(str); p_whitespace(str);
  if (! double_parse(str, &y)) return NULL;
  p_whitespace(str); p_comma(str); p_whitespace(str);
  if (! double_parse(str, &z)) return NULL;

  if (!hasZ)
  {
    /* use z as theta in 2D */
    if (z < -M_PI || z > M_PI)
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse 2D pose: Rotation angle must be in ]-pi, pi]. Recieved: %f", z);
      return NULL;
    }
    result = pose_make_2d(x, y, z);
  }
  else
  {
    double W, X, Y, Z;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &W)) return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &X)) return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &Y)) return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &Z)) return NULL;
    if (fabs(sqrt(W*W + X*X + Y*Y + Z*Z) - 1)  > MEOS_EPSILON)
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse 3D pose: Rotation quaternion must be of unit norm. Recieved: %f",
        sqrt(W*W + X*X + Y*Y + Z*Z));
      return NULL;
    }
    result = pose_make_3d(x, y, z, W, X, Y, Z);
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  pose_set_srid(result, srid);
  return result;
}

/*****************************************************************************/

/**
 * @brief Parse a temporal instant pose from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tpose_srid SRID of the temporal pose
 * @param[out] result New instant, may be NULL
 */
bool
tposeinst_parse(const char **str, meosType temptype, bool end,
  int *tpose_srid, TInstant **result)
{
  p_whitespace(str);
  meosType basetype = temptype_basetype(temptype);
  /* The next instruction will throw an exception if it fails */
  Datum value;
  if (! temporal_basetype_parse(str, basetype, &value))
    return false;
  Pose *pose = DatumGetPoseP(value);
  /* If one of the SRID of the temporal pose and of the geometry
   * is SRID_UNKNOWN and the other not, copy the SRID */
  int pose_srid = pose_get_srid(pose);
  if (*tpose_srid == SRID_UNKNOWN && pose_srid != SRID_UNKNOWN)
    *tpose_srid = pose_srid;
  else if (*tpose_srid != SRID_UNKNOWN && pose_srid == SRID_UNKNOWN)
    pose_set_srid(pose, *tpose_srid);
  /* If the SRID of the temporal pose and of the geometry do not match */
  else if (*tpose_srid != SRID_UNKNOWN && pose_srid != SRID_UNKNOWN &&
    *tpose_srid != pose_srid)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Pose SRID (%d) does not match temporal type SRID (%d)",
      pose_srid, *tpose_srid);
    pfree(pose);
    return false;
  }
  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND ||
    (end && ! ensure_end_input(str, "temporal pose")))
  {
    pfree(pose);
    return false;
  }
  if (result)
    *result = tinstant_make_free(PosePGetDatum(pose), temptype, t);
  return true;
}

/**
 * @brief Parse a temporal discrete sequence pose from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in,out] tpose_srid SRID of the temporal pose
 */
TSequence *
tposeseq_disc_parse(const char **str, meosType temptype, int *tpose_srid)
{
  const char *type_str = "temporal pose";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tpose_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tposeinst_parse(str, temptype, false, tpose_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tposeinst_parse(str, temptype, false, tpose_srid, NULL))
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
    tposeinst_parse(str, temptype, false, tpose_srid, &instants[i]);
  }
  p_cbrace(str);
  return tsequence_make_free(instants, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Parse a temporal sequence pose from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tpose_srid SRID of the temporal pose
 * @param[out] result New sequence, may be NULL
*/
bool
tposeseq_cont_parse(const char **str, meosType temptype, interpType interp,
  bool end, int *tpose_srid, TSequence **result)
{
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function tpose_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  const char *bak = *str;
  if (! tposeinst_parse(str, temptype, false, tpose_srid, NULL))
    return false;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tposeinst_parse(str, temptype, false, tpose_srid, NULL))
      return false;
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse temporal pose value: Missing closing bracket/parenthesis");
    return NULL;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "temporal point"))
    return false;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tposeinst_parse(str, temptype, false, tpose_srid, &instants[i]);
  }
  p_cbracket(str);
  p_cparen(str);
  if (result)
    *result = tsequence_make_free(instants, count, lower_inc, upper_inc,
      interp, NORMALIZE);
  return true;
}

/**
 * @brief Parse a temporal sequence set pose from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 * @param[in,out] tpose_srid SRID of the temporal pose
 */
TSequenceSet *
tposeseqset_parse(const char **str, meosType temptype, interpType interp,
  int *tpose_srid)
{
  const char *type_str = "temporal point";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tpose_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tposeseq_cont_parse(str, temptype, interp, false, tpose_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tposeseq_cont_parse(str, temptype, interp, false, tpose_srid, NULL))
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
    tposeseq_cont_parse(str, temptype, interp, false,
      tpose_srid, &sequences[i]);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Parse a temporal pose value from the buffer.
 * @param[in] str Input string
 * @param[in] temptype Temporal type
 */
Temporal *
tpose_parse(const char **str, meosType temptype)
{
  int tpose_srid = 0;
  const char *bak = *str;
  p_whitespace(str);

  /* Starts with "SRID=". The SRID specification must be gobbled for all
   * types excepted TInstant. We cannot use the atoi() function
   * because this requires a string terminated by '\0' and we cannot
   * modify the string in case it must be passed to the tposeinst_parse
   * function. */
  if (pg_strncasecmp(*str, "SRID=", 5) == 0)
  {
    /* Move str to the start of the number part */
    *str += 5;
    int delim = 0;
    tpose_srid = 0;
    /* Delimiter will be either ',' or ';' depending on whether interpolation
       is given after */
    while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
    {
      tpose_srid = tpose_srid * 10 + (*str)[delim] - '0';
      delim++;
    }
    /* Set str to the start of the temporal pose */
    *str += delim + 1;
  }

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
  /* Determine the type of the temporal pose */
  if (**str != '{' && **str != '[' && **str != '(')
  {
    /* Pass the SRID specification */
    *str = bak;
    TInstant *inst;
    if (! tposeinst_parse(str, temptype, true, &tpose_srid, &inst))
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq;
    if (! tposeseq_cont_parse(str, temptype, interp, true, &tpose_srid, &seq))
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
      result = (Temporal *) tposeseqset_parse(str, temptype, interp,
        &tpose_srid);
    }
    else
    {
      *str = bak;
      result = (Temporal *) tposeseq_disc_parse(str, temptype, &tpose_srid);
    }
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal pose from its Well-Known Text (WKT)
 * representation.
 * @param[in] str String
 */
Temporal *
tpose_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tpose_parse(&str, T_TPOSE);
}
#endif /* MEOS */

/*****************************************************************************/
