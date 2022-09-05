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
 * @brief General functions for temporal discrete sequences.
 */

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
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Append an instant to a temporal discrete sequence.
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
 * @brief Merge two temporal discrete sequences.
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
 * temporal discrete sequence
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

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighted average of a temporal discrete sequence number
 * @note Since an discrete sequence does not have duration, the function returns the
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
