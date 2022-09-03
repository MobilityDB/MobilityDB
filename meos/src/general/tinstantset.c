/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief General functions for temporal instant sets.
 */

#include "general/tinstantset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/temporaltypes.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "point/tpoint_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set from its Well-Known Text (WKT)
 * representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
TSequence *
tdiscseq_in(char *str, mobdbType temptype)
{
  return tdiscseq_parse(&str, temptype);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set boolean from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tbooldiscseq_in(char *str)
{
  return tdiscseq_parse(&str, T_TBOOL);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set integer from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tintdiscseq_in(char *str)
{
  return tdiscseq_parse(&str, T_TINT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set float from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tfloatdiscseq_in(char *str)
{
  return tdiscseq_parse(&str, T_TFLOAT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set text from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
ttextdiscseq_in(char *str)
{
  return tdiscseq_parse(&str, T_TTEXT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geometric point from its Well-Known Text
 * (WKT) representation.
 */
TSequence *
tgeompointdiscseq_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TINSTANT);
  return (TSequence *) temp;
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geographic point from its Well-Known
 * Text (WKT) representation.
 */
TSequence *
tgeogpointdiscseq_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert (temp->subtype == TINSTANTSET);
  return (TSequence *) temp;
}
#endif

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and the time frame
 * of another temporal instant set.
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq(),
 * etc.
 */
TSequence *
tdiscseq_from_base(Datum value, mobdbType temptype, const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype,
      tsequence_inst_n(seq, i)->t);
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TSequence *
tbooldiscseq_from_base(bool b, const TSequence *seq)
{
  return tdiscseq_from_base(BoolGetDatum(b), T_TBOOL, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TSequence *
tintdiscseq_from_base(int i, const TSequence *seq)
{
  return tdiscseq_from_base(Int32GetDatum(i), T_TINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TSequence *
tfloatdiscseq_from_base(bool b, const TSequence *seq)
{
  return tdiscseq_from_base(BoolGetDatum(b), T_TFLOAT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TSequence *
ttextdiscseq_from_base(const text *txt, const TSequence *seq)
{
  return tdiscseq_from_base(PointerGetDatum(txt), T_TTEXT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeompointdiscseq_from_base(const GSERIALIZED *gs, const TSequence *seq)
{
  return tdiscseq_from_base(PointerGetDatum(gs), T_TGEOMPOINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeogpointdiscseq_from_base(const GSERIALIZED *gs, const TSequence *seq)
{
  return tdiscseq_from_base(PointerGetDatum(gs), T_TGEOGPOINT, seq);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and a timestamp set.
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq(),
 * etc.
 */
TSequence *
tdiscseq_from_base_time(Datum value, mobdbType temptype,
  const TimestampSet *ts)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    instants[i] = tinstant_make(value, temptype, timestampset_time_n(ts, i));
  return tsequence_make_free(instants, ts->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TSequence *
tbooldiscseq_from_base_time(bool b, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(BoolGetDatum(b), T_TBOOL, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TSequence *
tintdiscseq_from_base_time(int i, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(Int32GetDatum(i), T_TINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TSequence *
tfloatdiscseq_from_base_time(bool b, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(BoolGetDatum(b), T_TFLOAT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TSequence *
ttextdiscseq_from_base_time(const text *txt, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(txt), T_TTEXT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeompointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(gs), T_TGEOMPOINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeogpointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(gs), T_TGEOGPOINT, ts);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Return the array of base values of a temporal instant set
 *
 * @param[in] seq Temporal instant set
 * @param[out] result Array of base values
 * @result Number of elements in the output array
 */
int
tdiscseq_values1(const TSequence *seq, Datum *result)
{
  for (int i = 0; i < seq->count; i++)
    result[i] = tinstant_value(tsequence_inst_n(seq, i));
  if (seq->count == 1)
    return 1;
  mobdbType basetype = temptype_basetype(seq->temptype);
  datumarr_sort(result, seq->count, basetype);
  return datumarr_remove_duplicates(result, seq->count, basetype);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of base values of a temporal instant set.
 * @sqlfunc getValues()
 */
Datum *
tdiscseq_values(const TSequence *seq, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * seq->count);
  *count = tdiscseq_values1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of spans of a temporal instant set float.
 * @sqlfunc getValues()
 */
Span **
tfloatdiscseq_spans(const TSequence *seq, int *count)
{
  int newcount;
  Datum *values = tdiscseq_values(seq, &newcount);
  Span **result = palloc(sizeof(Span *) * newcount);
  for (int i = 0; i < newcount; i++)
    result[i] = span_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  *count = newcount;
  return result;
}


/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Append an instant to a temporal instant set.
 * @sqlfunc appendInstant()
 */
TSequence *
tdiscseq_append_tinstant(const TSequence *seq, const TInstant *inst)
{
  /* Ensure validity of the arguments */
  assert(seq->temptype == inst->temptype);
  const TInstant *inst1 = tsequence_inst_n(seq, seq->count - 1);
  ensure_increasing_timestamps(inst1, inst, MERGE);
  if (inst1->t == inst->t)
    return tsequence_copy(seq);

  /* Create the result */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count + 1);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tsequence_inst_n(seq, i);
  instants[seq->count] = (TInstant *) inst;
  TSequence *result = tsequence_make1(instants, seq->count + 1, true, true,
    DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge two temporal instant sets.
 * @sqlfunc merge()
 */
Temporal *
tdiscseq_merge(const TSequence *seq1, const TSequence *seq2)
{
  const TSequence *sequences[] = {seq1, seq2};
  return tdiscseq_merge_array(sequences, 2);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge an array of temporal instants.
 *
 * @note The function does not assume that the values in the array are strictly
 * ordered on time, i.e., the intersection of the bounding boxes of two values
 * may be a period. For this reason two passes are necessary.
 *
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @result Result value that can be either a temporal instant or a
 * temporal instant set
 * @sqlfunc merge()
 */
// Temporal *
// tdiscseq_merge_array(const TSequence **sequences, int count)
// {
  // /* Validity test will be done in tinstant_merge_array */
  // /* Collect the composing instants */
  // int totalcount = 0;
  // for (int i = 0; i < count; i++)
    // totalcount += sequences[i]->count;
  // const TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
  // int k = 0;
  // for (int i = 0; i < count; i++)
  // {
    // for (int j = 0; j < sequences[i]->count; j++)
      // instants[k++] = tsequence_inst_n(sequences[i], j);
  // }
  // /* Create the result */
  // Temporal *result = tinstant_merge_array(instants, totalcount);
  // pfree(instants);
  // return result;
// }

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * Temporally intersect a temporal instant set and a temporal instant
 *
 * @param[in] seq,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tdiscseq_tinstant(const TSequence *seq, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  TInstant *inst1 = tsequence_at_timestamp(seq, inst->t);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] inst,seq Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tinstantset(const TInstant *inst, const TSequence *seq,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tdiscseq_tinstant(seq, inst, inter2, inter1);
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] seq1,seq2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tdiscseq_tinstantset(const TSequence *seq1, const TSequence *seq2,
  TSequence **inter1, TSequence **inter2)
{
  /* Bounding period test */
  if (!overlaps_span_span(&seq1->period, &seq2->period))
    return false;

  int count = Min(seq1->count, seq2->count);
  const TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tsequence_inst_n(seq1, i);
  const TInstant *inst2 = tsequence_inst_n(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      instants1[k] = inst1;
      instants2[k++] = inst2;
      inst1 = tsequence_inst_n(seq1, ++i);
      inst2 = tsequence_inst_n(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = tsequence_inst_n(seq1, ++i);
    else
      inst2 = tsequence_inst_n(seq2, ++j);
  }
  if (k != 0)
  {
    *inter1 = tsequence_make(instants1, k, true, true, DISCRETE, NORMALIZE_NO);
    *inter2 = tsequence_make(instants2, k, true, true, DISCRETE, NORMALIZE_NO);
  }

  pfree(instants1); pfree(instants2);
  return k != 0;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighted average of a temporal instant set number
 * @note Since an instant set does not have duration, the function returns the
 * traditional average of the values
 * @sqlfunc twAvg()
 */
// double
// tnumberdiscseq_twavg(const TSequence *seq)
// {
  // mobdbType basetype = temptype_basetype(seq->temptype);
  // double result = 0.0;
  // for (int i = 0; i < seq->count; i++)
  // {
    // const TInstant *inst = tsequence_inst_n(seq, i);
    // result += datum_double(tinstant_value(inst), basetype);
  // }
  // return result / seq->count;
// }

/*****************************************************************************/
