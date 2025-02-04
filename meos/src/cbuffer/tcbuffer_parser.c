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
 * @brief Functions for parsing temporal circular buffers
 */

#include "cbuffer/tcbuffer_parser.h"

/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"

/*****************************************************************************/

/**
 * @brief Parse a circular buffer from its string representation
 */
Cbuffer *
cbuffer_parse(const char **str, bool end)
{
  const char *type_str = "circular buffer";
  p_whitespace(str);
  if (pg_strncasecmp(*str, "CBUFFER", 7) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse circular buffer");
    return NULL;
  }

  *str += 7;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse geo */
  p_whitespace(str);
  GSERIALIZED *gs;
  int32_t srid = SRID_UNKNOWN;
  /* The following call consumes also the separator passed as parameter */
  if (! geo_parse(str, T_GEOMETRY, ',', &srid, &gs))
    return NULL;
  if (! ensure_point_type(gs) || ! ensure_not_empty(gs) ||
      ! ensure_has_not_M_gs(gs))
  {
    pfree(gs);
    return false;
  }

  /* Parse radius */
  p_whitespace(str);
  Datum d;
  if (! elem_parse(str, T_FLOAT8, &d))
    return NULL;
  double radius = DatumGetFloat8(d);
  if (radius < 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The radius must be a real number greater than or equal to 0");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  Cbuffer *result = cbuffer_make(gs, radius);
  pfree(gs);
  return result;
}

/**
 * @brief Parse a circular buffer from the input buffer
 * @param[in] str Input string
 * @param[in,out] srid SRID of the result. If it is SRID_UNKNOWN, it may take
 * the value from the circular buffer if it is not SRID_UNKNOWN.
 * @param[out] result New circular buffer, may be NULL
 */
static bool 
cbuffer_parse_elem(const char **str, char sep, int *srid, Cbuffer **result)
{
  p_whitespace(str);
  /* The next instruction will throw an exception if it fails */
  Datum d;
  if (! basetype_parse(str, T_CBUFFER, sep, &d))
    return false;
  Cbuffer *cbuf = DatumGetCbufferP(d);

  /* If one of the SRID of the temporal circular buffer and of the circular
   * buffer is SRID_UNKNOWN and the other not, copy the SRID */
  d = PointerGetDatum(&cbuf->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  int gs_srid = gserialized_get_srid(gs);
  if (*srid == SRID_UNKNOWN && gs_srid != SRID_UNKNOWN)
    *srid = gs_srid;
  else if (*srid != SRID_UNKNOWN &&
    ( gs_srid == SRID_UNKNOWN || gs_srid == SRID_DEFAULT ))
    gserialized_set_srid(gs, *srid);
  /* If the SRID of the temporal circular buffer and of the circular buffer
   * do not match */
  else if (*srid != SRID_UNKNOWN && gs_srid != SRID_UNKNOWN &&
    *srid != gs_srid)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Geometry SRID (%d) does not match temporal type SRID (%d)",
      gs_srid, *srid);
    pfree(cbuf);
    return false;
  }
  if (result)
    *result = cbuf;
  else 
    pfree(cbuf);
  return true;
}

/*****************************************************************************/

/**
 * @brief Parse a temporal instant circular buffer from the inputbuffer
 * @param[in] str Input string
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tpoint_srid SRID of the temporal circular buffer
 * @param[out] result New instant, may be NULL
 */
bool 
tcbufferinst_parse(const char **str, bool end, int *tcbuffer_srid,
  TInstant **result)
{
  Cbuffer *cbuf;
  bool success = cbuffer_parse_elem(str, '@', tcbuffer_srid, &cbuf);
  if (! success)
    return false;
  
  TimestampTz t = timestamp_parse(str);
  if (t == DT_NOEND ||
    /* Ensure there is no more input */
    (end && ! ensure_end_input(str, "temporal circular buffer")))
  {
    pfree(cbuf);
    return false;
  }

  if (result)
    *result = tinstant_make(PointerGetDatum(cbuf), T_TCBUFFER, t);
  pfree(cbuf);
  return true;
}

/**
 * @brief Parse a temporal discrete sequence circular buffer from the buffer
 * @param[in] str Input string
 * @param[in,out] tcbuffer_srid SRID of the temporal circular buffer
 */
TSequence *
tcbufferseq_disc_parse(const char **str, int *tcbuffer_srid)
{
  const char *type_str = "temporal circular buffer";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function #tcbuffer_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tcbufferinst_parse(str, false, tcbuffer_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tcbufferinst_parse(str, false, tcbuffer_srid, NULL))
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
    tcbufferinst_parse(str, false, tcbuffer_srid, &instants[i]);
  }
  p_cbrace(str);
  return tsequence_make_free(instants, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Parse a temporal sequence circular buffer from the input buffer
 * @param[in] str Input string
 * @param[in] interp Interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in,out] tcbuffer_srid SRID of the temporal circular buffer
 * @param[out] result New sequence, may be NULL
 */
bool
tcbufferseq_cont_parse(const char **str, interpType interp, bool end,
  int *tcbuffer_srid, TSequence **result)
{
  p_whitespace(str);
  bool lower_inc = false, upper_inc = false;
  /* We are sure to find an opening bracket or parenthesis because that was the
   * condition to call this function in the dispatch function tcbuffer_parse */
  if (p_obracket(str))
    lower_inc = true;
  else if (p_oparen(str))
    lower_inc = false;

  /* First parsing */
  const char *bak = *str;
  if (! tcbufferinst_parse(str, false, tcbuffer_srid, NULL))
    return false;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tcbufferinst_parse(str, false, tcbuffer_srid, NULL))
      return false;
  }
  if (p_cbracket(str))
    upper_inc = true;
  else if (p_cparen(str))
    upper_inc = false;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse temporal circular buffer value: Missing closing bracket/parenthesis");
    return false;
  }
  /* Ensure there is no more input */
  if (end && ! ensure_end_input(str, "temporal circular buffer"))
    return false;

  /* Second parsing */
  *str = bak;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    p_comma(str);
    tcbufferinst_parse(str, false, tcbuffer_srid, &instants[i]);
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
 * @brief Parse a temporal sequence set circular buffer from the input buffer
 * @param[in] str Input string
 * @param[in] interp Interpolation
 * @param[in,out] tcbuffer_srid SRID of the temporal circular buffer
 */
TSequenceSet *
tcbufferseqset_parse(const char **str, interpType interp, int *tcbuffer_srid)
{
  const char *type_str = "temporal circular buffer";
  p_whitespace(str);
  /* We are sure to find an opening brace because that was the condition
   * to call this function in the dispatch function tcbuffer_parse */
  p_obrace(str);

  /* First parsing */
  const char *bak = *str;
  if (! tcbufferseq_cont_parse(str, interp, false, tcbuffer_srid, NULL))
    return NULL;
  int count = 1;
  while (p_comma(str))
  {
    count++;
    if (! tcbufferseq_cont_parse(str, interp, false, tcbuffer_srid, NULL))
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
    tcbufferseq_cont_parse(str, interp, false, tcbuffer_srid, &sequences[i]);
  }
  p_cbrace(str);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Parse a temporal circular buffer value from the input buffer
 * @param[in] str Input string
 */
Temporal *
tcbuffer_parse(const char **str)
{
  const char *bak = *str;
  p_whitespace(str);

  /* Determine whether there is an SRID. If there is one we decode it and
   * advance the bak pointer after the SRID to do not parse in the */
  int tcbuffer_srid = SRID_UNKNOWN;
  bool has_srid = srid_parse(str, &tcbuffer_srid);

  interpType interp = LINEAR;
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
  /* Determine the type of the temporal circular buffer */
  if (**str != '{' && **str != '[' && **str != '(')
  {
    /* Pass the SRID specification */
    *str = bak;
    TInstant *inst;
    if (! tcbufferinst_parse(str, true, &tcbuffer_srid, &inst))
      return NULL;
    result = (Temporal *) inst;
  }
  else if (**str == '[' || **str == '(')
  {
    TSequence *seq;
    if (! tcbufferseq_cont_parse(str, interp, true, &tcbuffer_srid, &seq))
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
      result = (Temporal *) tcbufferseqset_parse(str, interp, &tcbuffer_srid);
    }
    else
    {
      *str = bak;
      result = (Temporal *) tcbufferseq_disc_parse(str, &tcbuffer_srid);
    }
  }
  return result;
}

/*****************************************************************************/
