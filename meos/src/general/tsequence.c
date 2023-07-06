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
 * @brief General functions for temporal sequences.
 */

#include "general/tsequence.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/pg_types.h"
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporaltypes.h"
#include "general/temporal_boxops.h"
#include "general/tnumber_distance.h"
#include "general/type_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
  #include "npoint/tnpoint_distance.h"
#endif

/*****************************************************************************
 * Collinear functions
 * Are the three temporal instants collinear?
 * These functions suppose that the segments are not constant.
 *****************************************************************************/

/**
 * @brief Return true if the three values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
float_collinear(double x1, double x2, double x3, double ratio)
{
  double x = x1 + (x3 - x1) * ratio;
  return (fabs(x2 - x) <= MEOS_EPSILON);
}

/**
 * @brief Return true if the three double2 values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double2_collinear(const double2 *x1, const double2 *x2, const double2 *x3,
  double ratio)
{
  double2 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio;
  bool result = (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON);
  return result;
}

/**
 * @brief Return true if the three values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double3_collinear(const double3 *x1, const double3 *x2, const double3 *x3,
  double ratio)
{
  double3 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio,
  x.c = x1->c + (x3->c - x1->c) * ratio;
  bool result = (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON && fabs(x2->c - x.c) <= MEOS_EPSILON);
  return result;
}

/**
 * @brief Return true if the three values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double4_collinear(const double4 *x1, const double4 *x2, const double4 *x3,
  double ratio)
{
  double4 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio;
  x.c = x1->c + (x3->c - x1->c) * ratio;
  x.d = x1->d + (x3->d - x1->d) * ratio;
  bool result = (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON && fabs(x2->c - x.c) <= MEOS_EPSILON &&
    fabs(x2->d - x.d) <= MEOS_EPSILON);
  return result;
}

#if NPOINT
/**
 * @brief Return true if the three values are collinear
 * @param[in] np1,np2,np3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `np1` and `np2` divided by the duration
 * of the timestamps associated to `np1` and `np3`
 */
static bool
npoint_collinear(Npoint *np1, Npoint *np2, Npoint *np3, double ratio)
{
  return float_collinear(np1->pos, np2->pos, np3->pos, ratio);
}
#endif

/**
 * @brief Return true if the three values are collinear
 * @param[in] value1,value2,value3 Input values
 * @param[in] basetype Base type
 * @param[in] t1,t2,t3 Input timestamps
 */
static bool
datum_collinear(Datum value1, Datum value2, Datum value3, meosType basetype,
  TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
  double duration1 = (double) (t2 - t1);
  double duration2 = (double) (t3 - t1);
  double ratio = duration1 / duration2;
  if (basetype == T_FLOAT8)
    return float_collinear(DatumGetFloat8(value1), DatumGetFloat8(value2),
      DatumGetFloat8(value3), ratio);
  if (basetype == T_DOUBLE2)
    return double2_collinear(DatumGetDouble2P(value1), DatumGetDouble2P(value2),
      DatumGetDouble2P(value3), ratio);
  if (geo_basetype(basetype))
  {
    GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value1);
    bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
    bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
    return geopoint_collinear(value1, value2, value3, ratio, hasz, geodetic);
  }
  if (basetype == T_DOUBLE3)
    return double3_collinear(DatumGetDouble3P(value1), DatumGetDouble3P(value2),
      DatumGetDouble3P(value3), ratio);
  if (basetype == T_DOUBLE4)
    return double4_collinear(DatumGetDouble4P(value1), DatumGetDouble4P(value2),
      DatumGetDouble4P(value3), ratio);
#if NPOINT
  if (basetype == T_NPOINT)
    return npoint_collinear(DatumGetNpointP(value1), DatumGetNpointP(value2),
      DatumGetNpointP(value3), ratio);
#endif
  elog(ERROR, "unknown collinear operation for base type: %d", basetype);
  return false; /* make compiler quiet */
}

/*****************************************************************************
 * Normalization functions
 *****************************************************************************/

/**
 * @brief Test whether we can remove the middle instant among 3 consecutive
 * ones during normalization
 */
bool
tsequence_norm_test(Datum value1, Datum value2, Datum value3, meosType basetype,
  interpType interp, TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
  bool v1v2eq = datum_eq(value1, value2, basetype);
  bool v2v3eq = datum_eq(value2, value3, basetype);
  if (
    /* step sequences and 2 consecutive instants that have the same value
      ... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
    */
    (interp == STEP && v1v2eq)
    ||
    /* 3 consecutive linear instants that have the same value
      ... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
    */
    (interp == LINEAR && v1v2eq && v2v3eq)
    ||
    /* collinear linear instants
      ... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
    */
    (interp == LINEAR && datum_collinear(value1, value2, value3, basetype,
      t1, t2, t3))
    )
    return true;
  else
    return false;
}

/**
 * @brief Normalize the array of temporal instants
 * @param[in] instants Array of input instants
 * @param[in] interp Interpolation
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @result Array of normalized temporal instants
 * @pre The input array has at least two elements
 * @note The function does not create new instants, it creates an array of
 * pointers to a subset of the input instants
 */
static TInstant **
tinstarr_normalize(const TInstant **instants, interpType interp, int count,
  int *newcount)
{
  assert(count > 1);
  meosType basetype = temptype_basetype(instants[0]->temptype);
  TInstant **result = palloc(sizeof(TInstant *) * count);
  /* Remove redundant instants */
  TInstant *inst1 = (TInstant *) instants[0];
  Datum value1 = tinstant_value(inst1);
  TInstant *inst2 = (TInstant *) instants[1];
  Datum value2 = tinstant_value(inst2);
  result[0] = inst1;
  int ninsts = 1;
  for (int i = 2; i < count; i++)
  {
    TInstant *inst3 = (TInstant *) instants[i];
    Datum value3 = tinstant_value(inst3);
    if (tsequence_norm_test(value1, value2, value3, basetype, interp, inst1->t,
      inst2->t, inst3->t))
    {
      inst2 = inst3; value2 = value3;
    }
    else
    {
      result[ninsts++] = inst2;
      inst1 = inst2; value1 = value2;
      inst2 = inst3; value2 = value3;
    }
  }
  result[ninsts++] = inst2;
  *newcount = ninsts;
  return result;
}

/**
 * @brief Test whether two sequences can be joined during normalization
 *
 * The input sequences must ordered and either (1) are non-overlapping, or
 * (2) share the same last/first instant in the case we are merging temporal
 *  sequences.
 *
 * @param[in] seq1,seq2 Input sequences
 * @param[out] removelast,removefirst State the instants to remove if the
 * sequences can be joined
 * @result True when the input sequences can be joined
 * @pre Both sequences are normalized
 */
bool
tsequence_join_test(const TSequence *seq1, const TSequence *seq2,
  bool *removelast, bool *removefirst)
{
  assert(seq1->temptype == seq2->temptype);
  meosType basetype = temptype_basetype(seq1->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);
  bool result = false;
  TInstant *last2 = (seq1->count == 1 || interp == DISCRETE) ? NULL :
    (TInstant *) TSEQUENCE_INST_N(seq1, seq1->count - 2);
  Datum last2value = ! last2 ? 0 : tinstant_value(last2);
  TInstant *last1 = (TInstant *) TSEQUENCE_INST_N(seq1, seq1->count - 1);
  Datum last1value = tinstant_value(last1);
  TInstant *first1 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  Datum first1value = tinstant_value(first1);
  TInstant *first2 = (seq2->count == 1 || interp == DISCRETE) ? NULL :
    (TInstant *) TSEQUENCE_INST_N(seq2, 1);
  Datum first2value = ! first2 ? 0 : tinstant_value(first2);
  bool eq_last2_last1 = ! last2 ? false :
    datum_eq(last2value, last1value, basetype);
  bool eq_last1_first1 = datum_eq(last1value, first1value, basetype);
  bool eq_first1_first2 = ! first2 ? false :
    datum_eq(first1value, first2value, basetype);

  /* We do not use DatumGetTimestampTz() for testing equality */
  bool adjacent = seq1->period.upper == seq2->period.lower &&
    (seq1->period.upper_inc || seq2->period.lower_inc);
  /* If they are adjacent and not instantaneous */
  if (adjacent && last2 && first2 &&
    (
    /* If step and the last segment of the first sequence is constant
       ..., 1@t1, 1@t2) [1@t2, 1@t3, ... -> ..., 1@t1, 2@t3, ...
       ..., 1@t1, 1@t2) [1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ...
       ..., 1@t1, 1@t2] (1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ...
     */
    (interp == STEP && eq_last2_last1 && eq_last1_first1)
    ||
    /* If the last/first segments are constant and equal
       ..., 1@t1, 1@t2] (1@t2, 1@t3, ... -> ..., 1@t1, 1@t3, ...
     */
    (eq_last2_last1 && eq_last1_first1 && eq_first1_first2)
    ||
    /* If float/point sequences and collinear last/first segments having the same duration
       ..., 1@t1, 2@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 3@t3, ...
    */
    (temptype_continuous(seq1->temptype) && eq_last1_first1 &&
      datum_collinear(last2value, first1value, first2value, basetype,
        last2->t, first1->t, first2->t))
    ))
  {
    /* Remove the last and first instants of the sequences */
    *removelast = true;
    *removefirst = true;
    result = true;
  }
  /* If step sequences and the first one has an exclusive upper bound,
     by definition the first sequence has the last segment constant
     ..., 1@t1, 1@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 2@t2, 3@t3, ...
     ..., 1@t1, 1@t2) [2@t2] -> ..., 1@t1, 2@t2]
   */
  else if (adjacent && interp == STEP && ! seq1->period.upper_inc)
  {
    /* Remove the last instant of the first sequence */
    *removelast = true;
    *removefirst = false;
    result = true;
  }
  /* If they are adjacent and have equal last/first value respectively
    Step
    ... 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
    [1@t1], (1@t1, 2@t2, ... -> ..., 1@t1, 2@t2
    Linear
    ..., 1@t1, 2@t2), [2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
    ..., 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
    ..., 1@t1, 2@t2), [2@t2] -> ..., 1@t1, 2@t2]
    [1@t1],(1@t1, 2@t2, ... -> [1@t1, 2@t2, ...
  */
  else if (adjacent && eq_last1_first1)
  {
    /* Remove the first instant of the second sequence */
    *removelast = false;
    *removefirst = true;
    result = true;
  }
  return result;
}

/**
 * @brief Join two temporal sequences
 * @param[in] seq1,seq2 Temporal sequences
 * @param[in] removelast,removefirst Remove the last and/or the
 * first instant of the first/second sequence
 * @pre The two input sequences are adjacent and have the same interpolation
 * @note The function is called when normalizing an array of sequences
 * and thus, all validity tests have been already made
 */
TSequence *
tsequence_join(const TSequence *seq1, const TSequence *seq2,
  bool removelast, bool removefirst)
{
  int count1 = removelast ? seq1->count - 1 : seq1->count;
  int start2 = removefirst ? 1 : 0;
  int count = count1 + (seq2->count - start2);
  const TInstant **instants = palloc(sizeof(TSequence *) * count);
  int i, ninsts = 0;
  for (i = 0; i < count1; i++)
    instants[ninsts++] = TSEQUENCE_INST_N(seq1, i);
  for (i = start2; i < seq2->count; i++)
    instants[ninsts++] = TSEQUENCE_INST_N(seq2, i);
  /* Get the bounding box size */
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(seq1->temptype));
  bboxunion bbox;
  memcpy(&bbox, TSEQUENCE_BBOX_PTR(seq1), bboxsize);
  bbox_expand(TSEQUENCE_BBOX_PTR(seq2), &bbox, seq1->temptype);
  TSequence *result = tsequence_make1_exp(instants, count, count,
    seq1->period.lower_inc, seq2->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE_NO, &bbox);
  pfree(instants);
  return result;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return the index of the segment of a temporal continuous sequence
 * containing a timestamp using binary search
 *
 * If the timestamp is contained in a temporal sequence, the index of the
 * segment containing the timestamp is returned in the output parameter.
 * Otherwise, return -1.
 * For example, given a value composed of 3 sequences and a timestamp,
 * the value returned in the output parameter is as follows:
 * @code
 *            0     1     2     3
 *            |-----|-----|-----|
 * 1)    t^                             => result = -1
 * 2)        t^                         => result = 0 if the lower bound is inclusive, -1 otherwise
 * 3)              t^                   => result = 1
 * 4)                 t^                => result = 1
 * 5)                          t^       => result = 3 if the upper bound is inclusive, -1 otherwise
 * 6)                             t^    => result = -1
 * @endcode
 *
 * @param[in] seq Temporal continuous sequence
 * @param[in] t Timestamp
 * @result Return -1 if the timestamp is not contained in a temporal sequence
 */
int
tcontseq_find_timestamp(const TSequence *seq, TimestampTz t)
{
  int first = 0;
  int last = seq->count - 1;
  while (first <= last)
  {
    int middle = (first + last) / 2;
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, middle);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, middle + 1);
    bool lower_inc = (middle == 0) ? seq->period.lower_inc : true;
    bool upper_inc = (middle == seq->count - 1) ? seq->period.upper_inc : false;
    if ((inst1->t < t && t < inst2->t) ||
        (lower_inc && inst1->t == t) || (upper_inc && inst2->t == t))
      return middle;
    if (t <= inst1->t)
      last = middle - 1;
    else
      first = middle + 1;
  }
  return -1;
}

/**
 * @brief Return the index of a timestamp in a temporal discrete sequence
 * value using binary search
 *
 * If the timestamp is contained in the temporal discrete sequence, the index
 * of the composing instant is returned in the output parameter. Otherwise,
 * return -1.
 * For example, given a value composed of 3 instants and a timestamp,
 * the value returned in the output parameter is as follows:
 * @code
 *            0        1        2
 *            |        |        |
 * 1)    t^                             => result = -1
 * 2)        t^                         => result = 0
 * 3)                 t^                => result = 1
 * 4)                          t^       => result = 2
 * 5)                              t^   => result = -1
 * @endcode
 *
 * @param[in] seq Temporal discrete sequence
 * @param[in] t Timestamp
 * @result Return true if the timestamp is contained in the discrete sequence
 */
int
tdiscseq_find_timestamp(const TSequence *seq, TimestampTz t)
{
  int first = 0;
  int last = seq->count - 1;
  while (first <= last)
  {
    int middle = (first + last) / 2;
    const TInstant *inst = TSEQUENCE_INST_N(seq, middle);
    int cmp = timestamptz_cmp_internal(inst->t, t);
    if (cmp == 0)
      return middle;
    if (cmp > 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  return -1;
}

/**
 * @brief Convert an array of arrays of temporal sequences into an array of
 * temporal sequences.
 *
 * This function is called by all the functions in which the number of output
 * sequences cannot be determined in advance, typically when each segment
 * of the input sequence can produce an arbitrary number of output sequences,
 * as in the case of tcontains.
 *
 * @param[in] sequences Array of array of temporal sequences
 * @param[in] countseqs Array of counters
 * @param[in] count Number of elements in the first dimension of the arrays
 * @param[in] totalseqs Number of elements in the output array
 * @pre count and totalseqs are greater than 0
 */
TSequence **
tseqarr2_to_tseqarr(TSequence ***sequences, int *countseqs, int count,
  int totalseqs)
{
  assert(count > 0);
  assert(totalseqs > 0);
  TSequence **result = palloc(sizeof(TSequence *) * totalseqs);
  int nseqs = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < countseqs[i]; j++)
      result[nseqs++] = sequences[i][j];
    if (countseqs[i])
      pfree(sequences[i]);
  }
  pfree(sequences); pfree(countseqs);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding box of a temporal sequence
 * @sqlfunc period(), tbox(), stbox()
 * @sqlop @p ::
 */
void
tsequence_set_bbox(const TSequence *seq, void *box)
{
  memset(box, 0, seq->bboxsize);
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), seq->bboxsize);
  return;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence from its Well-Known Text (WKT)
 * representation.
 * @param[in] str String
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_in(const char *str, meosType temptype, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, temptype);
  else
    return tcontseq_parse(&str, temptype, interp, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence boolean from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tboolseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TBOOL);
  else
    return tcontseq_parse(&str, T_TBOOL, STEP, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence integer from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tintseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TINT);
  else
    return tcontseq_parse(&str, T_TINT, STEP, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence float from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tfloatseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TFLOAT);
  else
    return tcontseq_parse(&str, T_TFLOAT, interp, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence text from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
ttextseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TTEXT);
  else
    return tcontseq_parse(&str, T_TTEXT, STEP, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence geometric point from its Well-Known Text
 * (WKT) representation.
 */
TSequence *
tgeompointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal sequence geographic point from its Well-Known Text
 * (WKT) representation.
 */
TSequence *
tgeogpointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}
#endif

/**
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * sequence.
 * @param[in] seq Temporal sequence
 * @param[in] maxdd Maximum number of decimal digits to output for floating point
 * values
 * @param[in] component True if the output string is a component of a
 * temporal sequence set and thus no interpolation string at the begining of
 * the string should be output
 * @param[in] value_out Function called to output the base value
 */
char *
tsequence_to_string(const TSequence *seq, int maxdd, bool component,
  outfunc value_out)
{
  char **strings = palloc(sizeof(char *) * seq->count);
  size_t outlen = 0;
  char prefix[20];
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (! component && MEOS_FLAGS_GET_CONTINUOUS(seq->flags) &&
      interp == STEP)
    sprintf(prefix, "Interp=Step;");
  else
    prefix[0] = '\0';
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    strings[i] = tinstant_to_string(inst, maxdd, value_out);
    outlen += strlen(strings[i]) + 1;
  }
  char open, close;
  if (MEOS_FLAGS_GET_DISCRETE(seq->flags))
  {
    open = (char) '{';
    close = (char) '}';
  }
  else
  {
    open = seq->period.lower_inc ? (char) '[' : (char) '(';
    close = seq->period.upper_inc ? (char) ']' : (char) ')';
  }
  return stringarr_to_string(strings, seq->count, outlen, prefix, open, close,
    QUOTES_NO, SPACES);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal sequence.
 */
char *
tsequence_out(const TSequence *seq, int maxdd)
{
  return tsequence_to_string(seq, maxdd, false, &basetype_out);
}

/*****************************************************************************
 * Constructor functions
 * ---------------------
 * The basic constructor functions for temporal sequences is the function
 * #tsequence_make1_exp. This funtion is called in several contexts by the
 * following functions
 * - #tsequence_make_exp: Construct a sequence from an array of instants
 * - #tsequence_append_tinstant: Append an instant to an existing sequence
 * - #tsequence_join: Merge two consecutive sequences during the normalization
 *   of sequence sets
 * - #tsequenceset_make_gaps (in file tsequenceset.c): Construct a sequence
 *   set from an array of instants where the composing sequences are determined
 *   by space or time gaps between consecutive instants
 * In all these cases, it is necessary to verify the validity of the array of
 * instants and to compute the bounding box of the resulting sequence. In some
 * cases, the computation of the bounding box does not need an iteration and
 * the bounding box is passed as an additional argument to #tsequence_make1_exp.
 * - #tsequence_append_tinstant: The bounding box is computed by expanding the
 *   bounding box of the sequence with the instant
 * - #tsequence_join: The bounding box is computed from the ones of the two
 *   sequences
 * Otherwise, a NULL bounding box is passed to the function so that it does
 * an iteration for computing it.
 *****************************************************************************/

#ifdef DEBUG_BUILD
/**
 * @brief Function version of the the macro of the same name for
 * debugging purposes
 */
static size_t *
TSEQUENCE_OFFSETS_PTR(const TSequence *seq)
{
  return (size_t *)( ((char *) &seq->period) + seq->bboxsize );
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the n-th instant of a temporal sequence.
 * @note The period component of the bbox is already declared in the struct
 * @pre The argument @p index is less than the number of instants in the
 * sequence
 */
const TInstant *
TSEQUENCE_INST_N(const TSequence *seq, int index)
{
  return (TInstant *)(
    ((char *) &seq->period) + seq->bboxsize +
    sizeof(size_t) * seq->maxcount + (TSEQUENCE_OFFSETS_PTR(seq))[index] );
}
#endif /* DEBUG_BUILD */

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 *
 * For example, the memory structure of a temporal sequence with two instants
 * is as follows:
 * @code
 * ---------------------------------------------------------
 * ( TSequence )_X | ( bbox )_X | offset_0 | offset_1 | ...
 * ---------------------------------------------------------
 * -------------------------------------
 * ( TInstant_0 )_X | ( TInstant_1 )_X |
 * -------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding instants
 *
 * @pre The validity of the arguments has been tested before
 */
TSequence *
tsequence_make1_exp(const TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize,
  void *bbox)
{
  assert(maxcount >= count);

  /* Normalize the array of instants */
  TInstant **norminsts = (TInstant **) instants;
  int newcount = count;
  if (interp != DISCRETE && normalize && count > 1)
    norminsts = tinstarr_normalize(instants, interp, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(instants[0]->temptype));
  /* The period component of the bbox is already declared in the struct */
  size_t bboxsize_extra = bboxsize - sizeof(Span);

  /* Compute the size of the temporal sequence */
  size_t insts_size = 0;
  /* Size of composing instants */
  for (int i = 0; i < newcount; i++)
    insts_size += DOUBLE_PAD(VARSIZE(norminsts[i]));
  /* Compute the total size for maxcount instants as a proportion of the size
   * of the count instants provided. Note that this is only an initial
   * estimation. The functions adding instants to a sequence must verify both
   * the maximum number of instants and the remaining space for adding an
   * additional variable-length instant of arbitrary size */
  if (count != maxcount)
    insts_size *= maxcount / count;
  else
    maxcount = newcount;
  /* Total size of the struct */
  size_t memsize = DOUBLE_PAD(sizeof(TSequence)) + bboxsize_extra +
    sizeof(size_t) * maxcount + insts_size;

  /* Create the temporal sequence */
  TSequence *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->maxcount = maxcount;
  result->temptype = instants[0]->temptype;
  result->subtype = TSEQUENCE;
  result->bboxsize = (int16) bboxsize;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags,
    MEOS_FLAGS_GET_CONTINUOUS(norminsts[0]->flags));
  MEOS_FLAGS_SET_INTERP(result->flags, interp);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(instants[0]->temptype))
  {
    MEOS_FLAGS_SET_Z(result->flags, MEOS_FLAGS_GET_Z(instants[0]->flags));
    MEOS_FLAGS_SET_GEODETIC(result->flags,
      MEOS_FLAGS_GET_GEODETIC(instants[0]->flags));
  }
  /* Initialization of the variable-length part */
  /* Store the bounding box passed as parameter or compute it if not given */
  if (bbox)
    memcpy(TSEQUENCE_BBOX_PTR(result), bbox, bboxsize);
  else
    tinstarr_compute_bbox((const TInstant **) norminsts, newcount, lower_inc,
      upper_inc, interp, TSEQUENCE_BBOX_PTR(result));
  /* Store the composing instants */
  size_t pdata = DOUBLE_PAD(sizeof(TSequence)) + bboxsize_extra +
    sizeof(size_t) * maxcount;
  size_t pos = 0;
  for (int i = 0; i < newcount; i++)
  {
    memcpy(((char *) result) + pdata + pos, norminsts[i],
      VARSIZE(norminsts[i]));
    (TSEQUENCE_OFFSETS_PTR(result))[i] = pos;
    pos += DOUBLE_PAD(VARSIZE(norminsts[i]));
  }
  if (interp != DISCRETE && normalize && count > 1)
    pfree(norminsts);
  return result;
}

/**
 * @brief Ensure that the timestamp of the first temporal instant is smaller
 * (or equal if the merge parameter is true) than the one of the second
 * temporal instant. Moreover, ensures that the values are the same
 * if the timestamps are equal
 */
void
ensure_increasing_timestamps(const TInstant *inst1, const TInstant *inst2,
  bool merge)
{
  if ((merge && inst1->t > inst2->t) || (!merge && inst1->t >= inst2->t))
  {
    char *t1 = pg_timestamptz_out(inst1->t);
    char *t2 = pg_timestamptz_out(inst2->t);
    elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s", t1, t2);
  }
  if (merge && inst1->t == inst2->t &&
    ! datum_eq(tinstant_value(inst1), tinstant_value(inst2),
        temptype_basetype(inst1->temptype)))
  {
    char *t1 = pg_timestamptz_out(inst1->t);
    elog(ERROR, "The temporal values have different value at their overlapping instant %s", t1);
  }
  return;
}

/**
 * @brief Expand the second bounding box with the first one
 */
void
bbox_expand(const void *box1, void *box2, meosType temptype)
{
  assert(temptype);
  if (talpha_type(temptype))
    span_expand((Span *) box1, (Span *) box2);
  else if (tnumber_type(temptype))
    tbox_expand((TBox *) box1, (TBox *) box2);
  else if (tspatial_type(temptype))
    stbox_expand((STBox *) box1, (STBox *) box2);
  else
    elog(ERROR, "Undefined temporal type for bounding box operation");
  return;
}

/**
 * @brief Ensure that all temporal instants of the array have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 * If the bounding box output argument is not NULL, the bounding box of the
 * resulting sequence is computed
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] merge True if a merge operation, which implies that two
 * consecutive instants may be equal
 * @param[in] interp Interpolation
 */
void
ensure_valid_tinstarr(const TInstant **instants, int count, bool merge,
  interpType interp __attribute__((unused)))
{
  for (int i = 0; i < count; i++)
  {
    if (instants[i]->subtype != TINSTANT)
      elog(ERROR, "Input values must be temporal instants");
    if (i > 0)
    {
      ensure_increasing_timestamps(instants[i - 1], instants[i], merge);
      ensure_spatial_validity((Temporal *) instants[i - 1],
        (Temporal *) instants[i]);
#if NPOINT
      if (interp != DISCRETE && instants[i]->temptype == T_TNPOINT)
        ensure_same_rid_tnpointinst(instants[i - 1], instants[i]);
#endif /* NPOINT */
    }
  }
  return;
}

/**
 * @brief Ensure the validity of the arguments when creating a temporal sequence
 */
void
tsequence_make_valid1(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp)
{
  assert(count > 0);
  /* Test the validity of the instants */
  ensure_valid_interpolation(instants[0]->temptype, interp);
  if (count == 1 && (! lower_inc || ! upper_inc))
    elog(ERROR, "Instant sequence must have inclusive bounds");
  meosType basetype = temptype_basetype(instants[0]->temptype);
  if (interp == STEP && count > 1 && ! upper_inc &&
    datum_ne(tinstant_value(instants[count - 1]),
      tinstant_value(instants[count - 2]), basetype))
    elog(ERROR, "Invalid end value for temporal sequence with step interpolation");
  return;
}

/**
 * @brief Ensure the validity of the arguments when creating a temporal sequence
 */
static void
tsequence_make_valid(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp)
{
  tsequence_make_valid1(instants, count, lower_inc, upper_inc, interp);
  ensure_valid_tinstarr(instants, count, MERGE_NO, interp);
  return;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence from an array of temporal instants
 * enabling the data structure to expand.
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 */
TSequence *
tsequence_make_exp(const TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  tsequence_make_valid(instants, count, lower_inc, upper_inc, interp);
  return tsequence_make1_exp(instants, count, maxcount, lower_inc, upper_inc,
    interp, normalize, NULL);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence from an array of temporal instants.
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tsequence_make(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, bool normalize)
{
  return tsequence_make_exp(instants, count, count, lower_inc, upper_inc,
    interp, normalize);
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 * and free the array and the instants after the creation.
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @see tsequence_make
 */
TSequence *
tsequence_make_free_exp(TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  assert(count > 0);
  TSequence *result = tsequence_make_exp((const TInstant **) instants, count,
    maxcount, lower_inc, upper_inc, interp, normalize);
  pfree_array((void **) instants, count);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal sequence from an array of temporal instants
 * and free the array and the instants after the creation.
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @see tsequence_make
 */
TSequence *
tsequence_make_free(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, bool normalize)
{
  return tsequence_make_free_exp(instants, count, count, lower_inc, upper_inc,
    interp, normalize);
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal sequence from arrays of coordinates, one per
 * dimension, and timestamps.
 * @param[in] xcoords Array of x coordinates
 * @param[in] ycoords Array of y coordinates
 * @param[in] zcoords Array of z coordinates
 * @param[in] times Array of z timestamps
 * @param[in] count Number of elements in the arrays
 * @param[in] srid SRID of the spatial coordinates
 * @param[in] geodetic True for tgeogpoint, false for tgeompoint
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 */
TSequence *
tpointseq_make_coords(const double *xcoords, const double *ycoords,
  const double *zcoords, const TimestampTz *times, int count, int32 srid,
  bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  assert(count > 0);
  bool hasz = (zcoords != NULL);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i ++)
  {
    Datum point = PointerGetDatum(gspoint_make(xcoords[i], ycoords[i],
      hasz ? zcoords[i] : 0.0, hasz, geodetic, srid));
    instants[i] = tinstant_make(point, geodetic ? T_TGEOGPOINT : T_TGEOMPOINT,
      times[i]);
  }
  return tsequence_make_free(instants, count, lower_inc, upper_inc, interp,
    normalize);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Return a copy of a temporal sequence.
 */
TSequence *
tsequence_copy(const TSequence *seq)
{
  TSequence *result = palloc0(VARSIZE(seq));
  memcpy(result, seq, VARSIZE(seq));
  return result;
}

/*****************************************************************************/

/**
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
 */
TSequence *
tdiscseq_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype, (TSEQUENCE_INST_N(seq, i))->t);
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq(),
 * etc.
 */
TSequence *
tsequence_from_base_timestampset(Datum value, meosType temptype, const Set *ts)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    instants[i] = tinstant_make(value, temptype,
      DatumGetTimestampTz(SET_VAL_N(ts, i)));
  return tsequence_make_free(instants, ts->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean discrete sequence from a boolean and a
 * timestamp set.
 */
TSequence *
tboolseq_from_base_timestampset(bool b, const Set *ts)
{
  return tsequence_from_base_timestampset(BoolGetDatum(b), T_TBOOL, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer discrete sequence from an integer and a
 * timestamp set.
 */
TSequence *
tintseq_from_base_timestampset(int i, const Set *ts)
{
  return tsequence_from_base_timestampset(Int32GetDatum(i), T_TINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float discrete sequence from a float and a
 * timestamp set.
 */
TSequence *
tfloatseq_from_base_timestampset(double d, const Set *ts)
{
  return tsequence_from_base_timestampset(Float8GetDatum(d), T_TFLOAT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text discrete sequence from a text and a
 * timestamp set.
 */
TSequence *
ttextseq_from_base_timestampset(const text *txt, const Set *ts)
{
  return tsequence_from_base_timestampset(PointerGetDatum(txt), T_TTEXT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point discrete sequence from a point
 * and a timestamp set.
 */
TSequence *
tgeompointseq_from_base_timestampset(const GSERIALIZED *gs, const Set *ts)
{
  return tsequence_from_base_timestampset(PointerGetDatum(gs), T_TGEOMPOINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point discrete sequence from a point
 * and a timestamp set.
 */
TSequence *
tgeogpointseq_from_base_timestampset(const GSERIALIZED *gs, const Set *ts)
{
  return tsequence_from_base_timestampset(PointerGetDatum(gs), T_TGEOGPOINT, ts);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal sequence from a base value and the time frame
 * of another temporal sequence.
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] seq Temporal value
 */
TSequence *
tsequence_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == DISCRETE)
    return tdiscseq_from_base_temp(value, temptype, seq);

  bool continuous = temptype_continuous(temptype);
  if (interp == LINEAR && ! continuous)
    interp = STEP;
  return tsequence_from_base_period(value, temptype, &seq->period, interp);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean sequence from a boolean and the time
 * frame of another temporal sequence.
 */
TSequence *
tboolseq_from_base_temp(bool b, const TSequence *seq)
{
  return tsequence_from_base_temp(BoolGetDatum(b), T_TBOOL, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer sequence from an integer and the time
 * frame of another temporal sequence.
 */
TSequence *
tintseq_from_base_temp(int i, const TSequence *seq)
{
  return tsequence_from_base_temp(Int32GetDatum(i), T_TINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float sequence from a float and the time frame
 * of another temporal sequence.
 */
TSequence *
tfloatseq_from_base_temp(double d, const TSequence *seq)
{
  return tsequence_from_base_temp(Float8GetDatum(d), T_TFLOAT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text sequence from a text and the time frame
 * of another temporal sequence.
 */
TSequence *
ttextseq_from_base_temp(const text *txt, const TSequence *seq)
{
  return tsequence_from_base_temp(PointerGetDatum(txt), T_TTEXT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point sequence from a point and the
 * time frame of another temporal sequence.
 */
TSequence *
tgeompointseq_from_base_temp(const GSERIALIZED *gs, const TSequence *seq)
{
  return tsequence_from_base_temp(PointerGetDatum(gs), T_TGEOMPOINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point sequence from a point and the
 * time frame of another temporal sequence.
 */
TSequence *
tgeogpointseq_from_base_temp(const GSERIALIZED *gs, const TSequence *seq)
{
  return tsequence_from_base_temp(PointerGetDatum(gs), T_TGEOGPOINT, seq);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal sequence from a base value and a period.
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] p Period
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_from_base_period(Datum value, meosType temptype, const Span *p,
  interpType interp)
{
  int count = 1;
  TInstant *instants[2];
  instants[0] = tinstant_make(value, temptype, p->lower);
  if (p->lower != p->upper)
  {
    instants[1] = tinstant_make(value, temptype, p->upper);
    count = 2;
  }
  TSequence *result = tsequence_make((const TInstant **) instants, count,
    p->lower_inc, p->upper_inc, interp, NORMALIZE_NO);
  pfree(instants[0]);
  if (count == 2)
    pfree(instants[1]);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean sequence from a boolean and a period.
 */
TSequence *
tboolseq_from_base_period(bool b, const Span *p)
{
  return tsequence_from_base_period(BoolGetDatum(b), T_TBOOL, p, STEP);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer sequence from an integer and a period.
 */
TSequence *
tintseq_from_base_period(int i, const Span *p)
{
  return tsequence_from_base_period(Int32GetDatum(i), T_TINT, p, STEP);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float sequence from a float and a period.
 */
TSequence *
tfloatseq_from_base_period(double d, const Span *p, interpType interp)
{
  return tsequence_from_base_period(Float8GetDatum(d), T_TFLOAT, p, interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text sequence from a text and a period.
 */
TSequence *
ttextseq_from_base_period(const text *txt, const Span *p)
{
  return tsequence_from_base_period(PointerGetDatum(txt), T_TTEXT, p, STEP);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point sequence from a point and a
 * period.
 */
TSequence *
tgeompointseq_from_base_period(const GSERIALIZED *gs, const Span *p,
  interpType interp)
{
  return tsequence_from_base_period(PointerGetDatum(gs), T_TGEOMPOINT, p,
    interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point sequence from a point and a
 * period.
 */
TSequence *
tgeogpointseq_from_base_period(const GSERIALIZED *gs, const Span *p,
  interpType interp)
{
  return tsequence_from_base_period(PointerGetDatum(gs), T_TGEOGPOINT, p,
    interp);
}
#endif /* MEOS */

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @brief Return the distance between two datums.
 */
double
datum_distance(Datum value1, Datum value2, meosType basetype, int16 flags)
{
  datum_func2 point_distance = NULL;
  if (geo_basetype(basetype))
    point_distance = pt_distance_fn(flags);
  double result = -1.0; /* make compiler quiet */
  if (tnumber_basetype(basetype))
    result = (basetype == T_INT4) ?
      (double) DatumGetInt32(number_distance(value1, value2, basetype, basetype)) :
      DatumGetFloat8(number_distance(value1, value2, basetype, basetype));
  else if (geo_basetype(basetype))
    result = DatumGetFloat8(point_distance(value1, value2));
#if NPOINT
  else if (basetype == T_NPOINT)
    result = DatumGetFloat8(npoint_distance(value1, value2));
#endif
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Append an instant to a temporal sequence accounting for potential gaps.
 * @param[in,out] seq Temporal sequence
 * @param[in] inst Temporal instant
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap
 * @param[in] expand True when reserving space for additional instants
 * @sqlfunc appendInstantGaps
 * @note It is the responsibility of the calling function to free the memory,
 * that is, delete the old value of seq when it is expanded or when the
 * result is a sequence set.
 */
Temporal *
tsequence_append_tinstant(TSequence *seq, const TInstant *inst, double maxdist,
  const Interval *maxt, bool expand)
{
  /* Ensure validity of the arguments */
  assert(seq->temptype == inst->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  meosType basetype = temptype_basetype(seq->temptype);
  TInstant *last = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 1);
  int16 flags = seq->flags;
#if NPOINT
  if (last->temptype == T_TNPOINT && interp != DISCRETE)
    ensure_same_rid_tnpointinst(inst, last);
#endif
  /* We cannot call ensure_increasing_timestamps since we must take into
   * account inclusive/exclusive bounds */
  if (last->t > inst->t)
  {
    char *t1 = pg_timestamptz_out(last->t);
    char *t = pg_timestamptz_out(inst->t);
    elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s",
      t1, t);
  }

  Datum value1 = tinstant_value(last);
  Datum value = tinstant_value(inst);
  if (last->t == inst->t)
  {
    bool eqv1v = datum_eq(value1, value, basetype);
    if (seq->period.upper_inc)
    {
      if (! eqv1v)
      {
        char *t1 = pg_timestamptz_out(last->t);
        elog(ERROR, "The temporal values have different value at their common timestamp %s", t1);
      }
      /* Do not add the new instant if sequence is discrete and new instant is
       * equal to be last one */
      else if (interp == DISCRETE)
        return (Temporal *) tsequence_copy(seq);
    }
    /* Exclusive upper bound and different value => result is a sequence set */
    else if (interp == LINEAR && ! eqv1v)
    {
      TSequence *sequences[2];
      sequences[0] = (TSequence *) seq;
      sequences[1] = tinstant_to_tsequence(inst, LINEAR);
      TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
        2, NORMALIZE_NO);
      pfree(sequences[1]);
      return (Temporal *) result;
    }
  }

  /* Take into account the maximum distance and/or the maximum interval */
  if (maxdist > 0.0 || maxt != NULL)
  {
    bool split = false;
    if (maxdist > 0.0 && ! datum_eq(value1, value, basetype))
    {
      double dist = datum_distance(value1, value, basetype, flags);
      if (dist > maxdist)
        split = true;
    }
    /* If there is not already a split by distance */
    if (maxt != NULL && ! split)
    {
      Interval *duration = pg_timestamp_mi(inst->t, last->t);
      if (pg_interval_cmp(duration, maxt) > 0)
        split = true;
      // CANNOT pfree(duration);
    }
    /* If split => result is a sequence set */
    if (split)
    {
      TSequence *sequences[2];
      sequences[0] = (TSequence *) seq;
      /* Arbitrary initialization to 64 elements if in expandable mode */
      sequences[1] = tsequence_make_exp((const TInstant **) &inst, 1,
        expand ? 64 : 1, true, true, interp, NORMALIZE_NO);
      TSequenceSet *result = tsequenceset_make_exp(
        (const TSequence **) sequences, 2, expand ? 64 : 2, NORMALIZE_NO);
      pfree(sequences[1]);
      return (Temporal *) result;
    }
  }

  /* The result is a sequence */
  int count = seq->count + 1;
  /* Normalize the result */
  if (interp != DISCRETE && seq->count > 1)
  {
    TInstant *penult = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 2);
    Datum value2 = tinstant_value(penult);
    if (tsequence_norm_test(value2, value1, value, basetype, interp,
      penult->t, last->t, inst->t))
    {
      /* The new instant replaces the last instant of the sequence */
      count--;
    }
  }

  /* Account for expandable structures
   * A while is used instead of an if to enable to break the loop if there is
   * no more available space */
  while (expand && count <= seq->maxcount)
  {
    /* Determine whether there is enough available space */
    size_t size = DOUBLE_PAD(VARSIZE(inst));
    /* Get the last instant to keep. It is either the last instant or the
     * penultimate one if the last one is redundant through normalization */
    last = (TInstant *) TSEQUENCE_INST_N(seq, count - 2);
    TInstant *new = (TInstant *) ((char *) last + DOUBLE_PAD(VARSIZE(last)));
    size_t avail_size = (char *) seq + VARSIZE(seq) - (char *) new;
    if (size > avail_size)
      /* There is not enough available space */
      break;

    /* There is enough space to add the new instant */
    if (count != seq->count)
    {
      /* Update the offsets array and the count when adding one instant */
      (TSEQUENCE_OFFSETS_PTR(seq))[count - 1] =
        (TSEQUENCE_OFFSETS_PTR(seq))[count - 2] + size;
      seq->count++;
    }
    memcpy(new, inst, size);
    /* Expand the bounding box and return */
    tsequence_expand_bbox(seq, inst);
    return (Temporal *) seq;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  const TInstant **instants = palloc(sizeof(TInstant *) * count);
  int ninsts = 0;
  for (int i = 0; i < count - 1; i++)
    instants[ninsts++] = TSEQUENCE_INST_N(seq, i);
  instants[ninsts++] = inst;
  int maxcount = count;
  if (expand && count > seq->maxcount)
  {
    maxcount = seq->maxcount * 2;
    // printf(" seq -> %d\n", maxcount);
  }
  /* Get the bounding box size */
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(seq->temptype));
  bboxunion bbox, bbox1;
  memcpy(&bbox, TSEQUENCE_BBOX_PTR(seq), bboxsize);
  tinstant_set_bbox(inst, &bbox1);
  bbox_expand(&bbox1, &bbox, seq->temptype);
  TSequence *result = tsequence_make1_exp(instants, count, maxcount,
    seq->period.lower_inc, true, interp, NORMALIZE_NO, &bbox);
  pfree(instants);
  return (Temporal *) result;
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Append a sequence to a temporal sequence.
 * @param[in,out] seq1 Temporal sequence
 * @param[in] seq2 Temporal sequence to append
 * @param[in] expand True when reserving space for additional sequences
 * @sqlfunc appendSequence()
 * @note It is the responsibility of the calling function to free the memory,
 * that is, delete the old value of seq when it cannot be expanded or when the
 * result is a sequence set.
 */
Temporal *
tsequence_append_tsequence(TSequence *seq1, const TSequence *seq2,
  bool expand __attribute__((unused)))
{
  /* Ensure validity of the arguments */
  assert(seq1->temptype == seq2->temptype);
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  assert(interp1 == MEOS_FLAGS_GET_INTERP(seq2->flags));
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
  /* We cannot call ensure_increasing_timestamps since we must take into
   * account inclusive/exclusive bounds */
  char *t1;
  if (inst1->t > inst2->t)
  {
    t1 = pg_timestamptz_out(inst1->t);
    char *t2 = pg_timestamptz_out(inst2->t);
    elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s",
      t1, t2);
  }
  else if (inst1->t == inst2->t && seq1->period.upper_inc &&
    seq2->period.lower_inc)
  {
    meosType basetype = temptype_basetype(seq1->temptype);
    Datum value1 = tinstant_value(inst1);
    Datum value2 = tinstant_value(inst2);
    if (! datum_eq(value1, value2, basetype))
    {
      t1 = pg_timestamptz_out(inst1->t);
      elog(ERROR, "The temporal values have different value at their common timestamp %s", t1);
    }
  }
#if NPOINT
  if (inst1->temptype == T_TNPOINT && interp1 != DISCRETE)
    ensure_same_rid_tnpointinst(inst1, inst2);
#endif

  bool removelast, removefirst;
  bool join = tsequence_join_test(seq1, seq2, &removelast, &removefirst);

  /* TODO Account for expandable structures */

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  if (join)
    /* Result is a continuous sequence */
    return (Temporal *) tsequence_join(seq1, seq2, removelast, removefirst);

  /* Result is a discrete sequence or a sequence set */
  Temporal *result;
  const TSequence *sequences[2];
  sequences[0] = seq1;
  sequences[1] = seq2;
  if (interp1 == DISCRETE)
    result = tsequence_merge_array((const TSequence **) sequences, 2);
  else
    result = (Temporal *) tsequenceset_make((const TSequence **) sequences, 2,
      NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Merge two temporal sequences.
 * @sqlfunc merge()
 */
Temporal *
tsequence_merge(const TSequence *seq1, const TSequence *seq2)
{
  const TSequence *sequences[] = {seq1, seq2};
  return tsequence_merge_array(sequences, 2);
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Merge an array of temporal discrete sequences.
 * @note The function does not assume that the values in the array are strictly
 * ordered on time, i.e., the intersection of the bounding boxes of two values
 * may be a period. For this reason two passes are necessary.
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @result Result value that can be either a temporal instant or a
 * temporal discrete sequence
 * @sqlfunc merge()
 */
Temporal *
tdiscseq_merge_array(const TSequence **sequences, int count)
{
  /* Validity test will be done in #tinstant_merge_array */
  /* Collect the composing instants */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += sequences[i]->count;
  const TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
  int ninsts = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < sequences[i]->count; j++)
      instants[ninsts++] = TSEQUENCE_INST_N(sequences[i], j);
  }
  /* Create the result */
  Temporal *result = tinstant_merge_array(instants, totalcount);
  pfree(instants);
  return result;
}

/**
 * @brief Normalize the array of temporal sequences
 *
 * The input sequences may be non-contiguous but must ordered and
 * either (1) are non-overlapping, or (2) share the same last/first
 * instant in the case we are merging temporal sequences.
 *
 * @param[in] sequences Array of input sequences
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @result Array of normalized temporal sequences values
 * @pre Each sequence in the input array is normalized
 * @pre When merging sequences, the test whether the value is the same
 * at the common instant should be ensured by the calling function.
 * @note The function creates new sequences and does not free the original
 * sequences
 */
TSequence **
tseqarr_normalize(const TSequence **sequences, int count, int *newcount)
{
  assert(count > 0);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  /* seq1 is the sequence to which we try to join subsequent seq2 */
  TSequence *seq1 = (TSequence *) sequences[0];
  /* newseq is the result of joining seq1 and seq2 */
  bool isnew = false;
  int nseqs = 0;
  for (int i = 1; i < count; i++)
  {
    TSequence *seq2 = (TSequence *) sequences[i];
    bool removelast, removefirst;
    if (tsequence_join_test(seq1, seq2, &removelast, &removefirst))
    {
      TSequence *newseq1 = tsequence_join(seq1, seq2, removelast, removefirst);
      if (isnew)
        pfree(seq1);
      seq1 = newseq1;
      isnew = true;
    }
    else
    {
      result[nseqs++] = isnew ? seq1 : tsequence_copy(seq1);
      seq1 = seq2;
      isnew = false;
    }
  }
  result[nseqs++] = isnew ? seq1 : tsequence_copy(seq1);
  *newcount = nseqs;
  return result;
}

/**
 * @brief Merge an array of temporal sequences.
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @param[out] totalcount Number of elements in the resulting array
 * @result Array of merged sequences
 * @note The values in the array may overlap on a single instant.
 */
TSequence **
tsequence_merge_array1(const TSequence **sequences, int count,
  int *totalcount)
{
  if (count > 1)
    tseqarr_sort((TSequence **) sequences, count);
  /* Test the validity of the composing sequences */
  const TSequence *seq1 = sequences[0];
  meosType basetype = temptype_basetype(seq1->temptype);
  for (int i = 1; i < count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
    const TSequence *seq2 = sequences[i];
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
    char *t1;
    if (inst1->t > inst2->t)
    {
      char *t2;
      t1 = pg_timestamptz_out(inst1->t);
      t2 = pg_timestamptz_out(inst2->t);
      elog(ERROR, "The temporal values cannot overlap on time: %s, %s", t1, t2);
    }
    else if (inst1->t == inst2->t && seq1->period.upper_inc &&
      seq2->period.lower_inc)
    {
      if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype))
      {
        t1 = pg_timestamptz_out(inst1->t);
        elog(ERROR, "The temporal values have different value at their common instant %s", t1);
      }
    }
    seq1 = seq2;
  }
  return tseqarr_normalize(sequences, count, totalcount);
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Merge an array of temporal sequences.
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @note The values in the array may overlap on a single instant.
 * @sqlfunc merge()
 */
Temporal *
tsequence_merge_array(const TSequence **sequences, int count)
{
  assert(count > 0);

  /* Discrete sequences */
  if (MEOS_FLAGS_GET_DISCRETE(sequences[0]->flags))
    return tdiscseq_merge_array(sequences, count);

  /* Continuous sequences */
  int totalcount;
  TSequence **newseqs = tsequence_merge_array1(sequences, count, &totalcount);
  Temporal *result;
  if (totalcount == 1)
  {
    result = (Temporal *) newseqs[0];
    pfree(newseqs);
  }
  else
    /* Normalization was done at function tsequence_merge_array1 */
    result = (Temporal *) tsequenceset_make_free(newseqs, totalcount,
      NORMALIZE_NO);
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_cast
 * @brief Cast a temporal sequence integer to a temporal sequence float.
 * @sqlop @p ::
 */
TSequence *
tintseq_to_tfloatseq(const TSequence *seq)
{
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TFLOAT;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, true);
  MEOS_FLAGS_SET_INTERP(result->flags, MEOS_FLAGS_GET_INTERP(result->flags));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(result, i);
    inst->temptype = T_TFLOAT;
    inst->value = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  }
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_cast
 * @brief Cast a temporal sequence float to a temporal sequence integer.
 * @sqlop @p ::
 */
TSequence *
tfloatseq_to_tintseq(const TSequence *seq)
{
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
    elog(ERROR, "Cannot cast temporal float with linear interpolation to temporal integer");
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TINT;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, false);
  MEOS_FLAGS_SET_INTERP(result->flags, MEOS_FLAGS_GET_INTERP(result->flags));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(result, i);
    inst->temptype = T_TINT;
    inst->value = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a copy of a temporal sequence with no additional free space.
 * @note We cannot simply test whether seq->count == seq->maxcount since there
 * could be extra space allocated for the (variable-length) instants
 */
TSequence *
tsequence_compact(const TSequence *seq)
{
  /* Compute the new total size of the sequence and allocate memory for it */
  size_t bboxsize_extra = seq->bboxsize - sizeof(Span);
  /* Size of composing instants */
  size_t insts_size = 0;
  for (int i = 0; i < seq->count; i++)
    insts_size += DOUBLE_PAD(VARSIZE(TSEQUENCE_INST_N(seq, i)));
  size_t seqsize = DOUBLE_PAD(sizeof(TSequence)) + bboxsize_extra +
    seq->count * sizeof(size_t) + insts_size;
  TSequence *result = palloc0(seqsize);

  /* Copy until the end of the offsets array */
  memcpy(result, seq, seqsize - insts_size);
  /* Set the maxcount */
  result->maxcount = result->count;
  /* Copy until the end of the offsets array */
  memcpy((char *) TSEQUENCE_INST_N(result, 0), TSEQUENCE_INST_N(seq, 0),
    insts_size);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Restart a temporal sequence by keeping only the last n instants
 */
void
tsequence_restart(TSequence *seq, int count)
{
  /* Ensure validity of arguments */
  assert (count > 0 && count < seq->count);
  /* Instantaneous sequence */
  if (seq->count == 1)
    return;

  /* General case */
  TInstant *first = (TInstant *) TSEQUENCE_INST_N(seq, 0);
  const TInstant *last_n;
  size_t inst_size = 0;
  /* Compute the size of the instants to be copied */
  for (int i = 0; i < count; i++)
  {
    last_n = TSEQUENCE_INST_N(seq, seq->count - i - 1);
    inst_size += DOUBLE_PAD(VARSIZE(last_n));
  }
  /* Copy the last instants at the beginning */
  last_n = TSEQUENCE_INST_N(seq, seq->count - count);
  memcpy(first, last_n, inst_size);
  /* Update the count and the bounding box */
  seq->count = count;
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(seq->temptype));
  if (bboxsize != 0)
    tsequence_compute_bbox(seq);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a subsequence specified by the two component instants
 */
TSequence *
tsequence_subseq(const TSequence *seq, int from, int to, bool lower_inc,
  bool upper_inc)
{
  /* Ensure validity of arguments */
  assert (from <= to && from >= 0 && to >= 0 && from < seq->count &&
    to < seq->count);
  /* General case */
  int count = to - from + 1;
  const TInstant **instants = palloc (sizeof(TInstant *) * count);
  for (int i = 0; i < to - from; i++)
    instants[i] = TSEQUENCE_INST_N(seq, i + from);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result = tsequence_make(instants, count, lower_inc, upper_inc,
    interp, NORMALIZE_NO);
  pfree(instants);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal instant transformed into a temporal sequence.
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tinstant_to_tsequence(const TInstant *inst, interpType interp)
{
  return tsequence_make(&inst, 1, true, true, interp, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Transform a temporal discrete sequence to a given interpolation.
 */
Temporal *
tdiscseq_set_interp(const TSequence *seq, interpType interp)
{
  assert(MEOS_FLAGS_GET_DISCRETE(seq->flags));
  /* If the requested interpolation is discrete return a copy */
  if (interp == DISCRETE)
    return (Temporal *) tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    return (Temporal *) tsequence_make(&inst, 1, true, true, interp,
      NORMALIZE_NO);
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    sequences[i] = tinstant_to_tsequence(inst, interp);
  }
  return (Temporal *) tsequenceset_make_free(sequences, seq->count,
    NORMALIZE_NO);
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into discrete interpolation.
 */
TSequence *
tcontseq_to_discrete(const TSequence *seq)
{
  assert(! MEOS_FLAGS_GET_DISCRETE(seq->flags));
  if (seq->count != 1)
    elog(ERROR, "Cannot transform input value to a temporal discrete sequence");
  return tinstant_to_tsequence(TSEQUENCE_INST_N(seq, 0), DISCRETE);
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence transformed to step interpolation.
 */
TSequence *
tcontseq_to_step(const TSequence *seq)
{
  /* If the sequence has step interpolation return a copy */
  if (MEOS_FLAGS_GET_STEP(seq->flags))
    return tsequence_copy(seq);

  /* interp == LINEAR */
  if (seq->count > 2)
    elog(ERROR, "Cannot transform input value to step interpolation");

  meosType basetype = temptype_basetype(seq->temptype);
  const TInstant *instants[2];
  for (int i = 0; i < seq->count; i++)
    instants[i] = TSEQUENCE_INST_N(seq, i);
  if (seq->count == 2 && ! datum_eq(tinstant_value(instants[0]),
      tinstant_value(instants[1]), basetype))
    elog(ERROR, "Cannot transform input value to step interpolation");
  return tsequence_make(instants, 2, seq->period.lower_inc,
    seq->period.upper_inc, STEP, NORMALIZE_NO);
}

/**
 * @brief Return a temporal sequence with continuous base type transformed from
 * step to linear interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set.
 */
int
tstepseq_to_linear_iter(const TSequence *seq, TSequence **result)
{
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    MEOS_FLAGS_SET_INTERP(result[0]->flags, LINEAR);
    return 1;
  }

  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  const TInstant *inst2 = NULL; /* keep compiler quiet */
  Datum value2;
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i);
    value2 = tinstant_value(inst2);
    TInstant *instants[2];
    instants[0] = (TInstant *) inst1;
    instants[1] = tinstant_make(value1, seq->temptype, inst2->t);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc &&
      datum_eq(value1, value2, basetype) : false;
    result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, LINEAR, NORMALIZE_NO);
    inst1 = inst2;
    value1 = value2;
    lower_inc = true;
    pfree(instants[1]);
  }
  if (seq->period.upper_inc)
  {
    value1 = tinstant_value(TSEQUENCE_INST_N(seq, seq->count - 2));
    value2 = tinstant_value(inst2);
    if (datum_ne(value1, value2, basetype))
      result[nseqs++] = tinstant_to_tsequence(inst2, LINEAR);
  }
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence with continuous base type transformed from
 * step to linear interpolation.
 * @param[in] seq Temporal sequence
 * @sqlfunc toLinear()
 */
TSequenceSet *
tstepseq_to_linear(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tstepseq_to_linear_iter(seq, sequences);
  /* We are sure that count > 0 */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence transformed to linear interpolation.
 */
Temporal *
tcontseq_to_linear(const TSequence *seq)
{
  /* If the sequence has linear interpolation return a copy */
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
    return (Temporal *) tsequence_copy(seq);

  /* interp == STEP */
  return (Temporal *) tstepseq_to_linear(seq);
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal value transformed to the given interpolation.
 * @sqlfunc setInterp
 */
Temporal *
tsequence_set_interp(const TSequence *seq, interpType interp)
{
  interpType seq_interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (seq_interp == DISCRETE)
    return tdiscseq_set_interp(seq, interp);

  if (interp == DISCRETE)
    return (Temporal *) tcontseq_to_discrete(seq);
  else if (interp == STEP)
    return (Temporal *) tcontseq_to_step(seq);
  else /* interp == LINEAR */
    return tcontseq_to_linear(seq);
}

/*****************************************************************************/


/**
 * @brief Shift and/or scale the instants of a temporal sequence
 * (iterator function).
 * @note This function is called for each sequence of a temporal sequence set.
 */
void
tsequence_shift_tscale_iter(TSequence *seq, TimestampTz delta, double scale)
{
  /* Set the first instant from the bounding period which has been already
   * shifted and/or scaled */
  TInstant *inst = (TInstant *) TSEQUENCE_INST_N(seq, 0);
  inst->t = DatumGetTimestampTz(seq->period.lower);
  if (seq->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    for (int i = 1; i < seq->count - 1; i++)
    {
      inst = (TInstant *) TSEQUENCE_INST_N(seq, i);
      /* The default value when there is not shift is 0 */
      if (delta != 0)
        inst->t += delta;
      /* The default value when there is not scale is 1.0 */
      if (scale != 1.0)
      /* The potential shift has been already taken care in the previous if */
        inst->t = DatumGetTimestampTz(seq->period.lower) + (TimestampTz)
          ((inst->t - DatumGetTimestampTz(seq->period.lower)) * scale);
    }
    /* Set the last instant */
    inst = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 1);
    inst->t = DatumGetTimestampTz(seq->period.upper);
  }
  return;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence shifted and/or scaled by the intervals.
 * @pre The duration is greater than 0 if it is not NULL
 * @sqlfunc shift(), tscale(), shiftTscale().
 */
TSequence *
tsequence_shift_tscale(const TSequence *seq, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);

  /* Copy the input sequence to the result */
  TSequence *result = tsequence_copy(seq);

  /* Shift and/or scale the bounding period */
  TimestampTz delta = 0; /* Default value when shift == NULL */
  double scale = 1.0;    /* Default value when duration == NULL */
  period_shift_tscale1(&result->period, shift, duration, &delta, &scale);

  /* Shift and/or scale the result */
  tsequence_shift_tscale_iter(result, delta, scale);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the array of distinct values of a temporal sequence.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of values in the resulting array
 * @result Array of values
 * @sqlfunc getValues()
 */
Datum *
tsequence_values(const TSequence *seq, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = tinstant_value(TSEQUENCE_INST_N(seq, i));
  if (seq->count > 1)
  {
    meosType basetype = temptype_basetype(seq->temptype);
    datumarr_sort(result, seq->count, basetype);
    *count = datumarr_remove_duplicates(result, seq->count, basetype);
  }
  else
    *count = 1;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the base values of a temporal number sequence as a span set.
 *
 * For temporal floats with linear interpolation the result is a singleton.
 * Otherwise, the result is a span set composed of instantenous spans, one for
 * each distinct value.
 * @sqlfunc getValues()
 */
SpanSet *
tnumberseq_valuespans(const TSequence *seq)
{
  /* Temporal sequence number with linear interpolation */
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
  {
    Span span;
    memcpy(&span, &((TBox *) TSEQUENCE_BBOX_PTR(seq))->span, sizeof(Span));
    return span_to_spanset(&span);
  }

  /* Temporal sequence number with discrete or step interpolation */
  int count;
  meosType basetype = temptype_basetype(seq->temptype);
  Datum *values = tsequence_values(seq, &count);
  Span *spans = palloc(sizeof(Span) * count);
  for (int i = 0; i < count; i++)
    span_set(values[i], values[i], true, true, basetype, &spans[i]);
  SpanSet *result = spanset_make_free(spans, count, NORMALIZE);
  pfree(values);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the time frame of a temporal sequence as a period set.
 * @sqlfunc getTime()
 */
SpanSet *
tsequence_time(const TSequence *seq)
{
  /* Continuous sequence */
  if (! MEOS_FLAGS_GET_DISCRETE(seq->flags))
    return span_to_spanset(&seq->period);

  /* Discrete sequence */
  Span *periods = palloc(sizeof(Span) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    span_set(inst->t, inst->t, true, true, T_TIMESTAMPTZ, &periods[i]);
  }
  SpanSet *result = spanset_make_free(periods, seq->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal sequence.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @sqlfunc minInstant()
 */
const TInstant *
tsequence_min_instant(const TSequence *seq)
{
  Datum min = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  int idx = 0;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_lt(value, min, basetype))
    {
      min = value;
      idx = i;
    }
  }
  return TSEQUENCE_INST_N(seq, idx);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal sequence.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not.
 * @sqlfunc maxInstant()
 */
const TInstant *
tsequence_max_instant(const TSequence *seq)
{
  Datum max = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  int idx = 0;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_gt(value, max, basetype))
    {
      max = value;
      idx = i;
    }
  }
  return TSEQUENCE_INST_N(seq, idx);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the minimum base value of a temporal sequence.
 * @sqlfunc minValue()
 */
Datum
tsequence_min_value(const TSequence *seq)
{
  if (tnumber_type(seq->temptype))
  {
    TBox *box = TSEQUENCE_BBOX_PTR(seq);
    Datum dmin = box->span.lower;
    meosType basetype = temptype_basetype(seq->temptype);
    Datum min = double_datum(DatumGetFloat8(dmin), basetype);
    return min;
  }

  meosType basetype = temptype_basetype(seq->temptype);
  Datum result = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the maximum base value of a temporal sequence.
 * @sqlfunc maxValue()
 */
Datum
tsequence_max_value(const TSequence *seq)
{
  if (tnumber_type(seq->temptype))
  {
    TBox *box = TSEQUENCE_BBOX_PTR(seq);
    Datum dmax = box->span.upper;
    /* The span in a TBox is always a double span */
    meosType basetype = temptype_basetype(seq->temptype);
    Datum max = double_datum(DatumGetFloat8(dmax), basetype);
    return max;
  }

  meosType basetype = temptype_basetype(seq->temptype);
  Datum result = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the duration of a temporal sequence.
 * @sqlfunc duration()
 */
Interval *
tsequence_duration(const TSequence *seq)
{
  return period_duration(&seq->period);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding period of a temporal sequence
 * @sqlfunc period()
 * @sqlop @p ::
 */
void
tsequence_set_period(const TSequence *seq, Span *p)
{
  memcpy(p, &seq->period, sizeof(Span));
  return;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the singleton array of sequences of a temporal sequence.
 * @sqlfunc sequences()
 */
TSequence **
tsequence_sequences(const TSequence *seq, int *count)
{
  TSequence **result = palloc(sizeof(TSequence *));
  result[0] = tsequence_copy(seq);
  *count = 1;
  return result;
}

/**
 * @brief Return the array of segments of a temporal sequence
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array on which the pointers of the newly constructed
 * segments are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_segments_iter(const TSequence *seq, TSequence **result)
{
  assert(! MEOS_FLAGS_GET_DISCRETE(seq->flags));
  /* Singleton sequence */
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  TInstant *instants[2];
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool lower_inc = seq->period.lower_inc;
  TInstant *inst1, *inst2;
  int nseqs = 0;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq, i - 1);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq, i);
    instants[0] = inst1;
    instants[1] = (interp == LINEAR) ? inst2 :
      tinstant_make(tinstant_value(inst1), seq->temptype, inst2->t);
    bool upper_inc;
    if (i == seq->count - 1 && (interp == LINEAR ||
      datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype)))
      upper_inc = seq->period.upper_inc;
    else
      upper_inc = false;
    result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    if (interp != LINEAR)
      pfree(instants[1]);
    lower_inc = true;
  }
  if (interp != LINEAR && seq->period.upper_inc)
  {
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 1);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 2);
    if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype))
      result[nseqs++] = tinstant_to_tsequence(inst1, interp);
  }
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the array of segments of a temporal sequence.
 * @sqlfunc segments()
 */
TSequence **
tsequence_segments(const TSequence *seq, int *count)
{
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);

  /* Discrete sequence */
  if (MEOS_FLAGS_GET_DISCRETE(seq->flags))
  {
    /* Discrete sequence */
    interpType interp = MEOS_FLAGS_GET_CONTINUOUS(seq->flags) ? LINEAR : STEP;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      result[i] = tinstant_to_tsequence(inst, interp);
    }
    *count = seq->count;
    return result;
  }

  /* Continuous sequence */
  *count = tsequence_segments_iter(seq, result);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the array of distinct instants of a temporal sequence.
 * @note By definition, all instants of a sequence are distinct
 * @sqlfunc instants()
 */
const TInstant **
tsequence_instants(const TSequence *seq)
{
  const TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = TSEQUENCE_INST_N(seq, i);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the start timestamp of a temporal sequence.
 * @sqlfunc startTimestamp()
 */
TimestampTz
tsequence_start_timestamp(const TSequence *seq)
{
  return (TSEQUENCE_INST_N(seq, 0))->t;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the end timestamp of a temporal sequence.
 * @sqlfunc endTimestamp()
 */
TimestampTz
tsequence_end_timestamp(const TSequence *seq)
{
  return (TSEQUENCE_INST_N(seq, seq->count - 1))->t;
}

/**
 * @brief Return the array of timestamps of a temporal sequence
 * (iterator function).
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_timestamps_iter(const TSequence *seq, TimestampTz *times)
{
  for (int i = 0; i < seq->count; i++)
    times[i] = (TSEQUENCE_INST_N(seq, i))->t;
  return seq->count;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the array of timestamps of a temporal sequence.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 * @post The output parameter count is equal to the number of instants of the
 * input temporal sequence
 * @sqlfunc timestamps()
 */
TimestampTz *
tsequence_timestamps(const TSequence *seq, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * seq->count);
  tsequence_timestamps_iter(seq, result);
  *count = seq->count;
  return result;
}

/**
 * @brief Return the base value of the segment of a temporal sequence at a
 * timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] linear True if the segment has linear interpolation
 * @param[in] t Timestamp
 * @pre The timestamp t satisfies inst1->t <= t <= inst2->t
 * @note The function creates a new value that must be freed
 */
Datum
tsegment_value_at_timestamp(const TInstant *inst1, const TInstant *inst2,
  bool linear, TimestampTz t)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  /* Constant segment or t is equal to lower bound or step interpolation */
  if (datum_eq(value1, value2, temptype_basetype(inst1->temptype)) ||
    inst1->t == t || (! linear && t < inst2->t))
    return tinstant_value_copy(inst1);

  /* t is equal to upper bound */
  if (inst2->t == t)
    return tinstant_value_copy(inst2);

  /* Interpolation for types with linear interpolation */
  long double duration1 = (long double) (t - inst1->t);
  long double duration2 = (long double) (inst2->t - inst1->t);
  long double ratio = duration1 / duration2;
  // TEST !!!! USED FOR ASSESSING FLOATINGING POINT PRECISION IN MOBILITYDB !!!
  // long double ratio = (double)(t - inst1->t) / (double)(inst2->t - inst1->t);
  assert(temptype_continuous(inst1->temptype));
  if (inst1->temptype == T_TFLOAT)
  {
    double start = DatumGetFloat8(value1);
    double end = DatumGetFloat8(value2);
    double dresult = start + (double) ((long double)(end - start) * ratio);
    return Float8GetDatum(dresult);
  }
  if (inst1->temptype == T_TDOUBLE2)
  {
    double2 *start = DatumGetDouble2P(value1);
    double2 *end = DatumGetDouble2P(value2);
    double2 *dresult = palloc(sizeof(double2));
    dresult->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
    dresult->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
    return Double2PGetDatum(dresult);
  }
  if (inst1->temptype == T_TDOUBLE3)
  {
    double3 *start = DatumGetDouble3P(value1);
    double3 *end = DatumGetDouble3P(value2);
    double3 *dresult = palloc(sizeof(double3));
    dresult->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
    dresult->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
    dresult->c = start->c + (double) ((long double)(end->c - start->c) * ratio);
    return Double3PGetDatum(dresult);
  }
  if (inst1->temptype == T_TDOUBLE4)
  {
    double4 *start = DatumGetDouble4P(value1);
    double4 *end = DatumGetDouble4P(value2);
    double4 *dresult = palloc(sizeof(double4));
    dresult->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
    dresult->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
    dresult->c = start->c + (double) ((long double)(end->c - start->c) * ratio);
    dresult->d = start->d + (double) ((long double)(end->d - start->d) * ratio);
    return Double4PGetDatum(dresult);
  }
  if (tgeo_type(inst1->temptype))
  {
    return geosegm_interpolate_point(value1, value2, ratio);
  }
#if NPOINT
  if (inst1->temptype == T_TNPOINT)
  {
    Npoint *np1 = DatumGetNpointP(value1);
    Npoint *np2 = DatumGetNpointP(value2);
    double pos = np1->pos + (double) ((long double)(np2->pos - np1->pos) * ratio);
    Npoint *result = npoint_make(np1->rid, pos);
    return PointerGetDatum(result);
  }
#endif
  elog(ERROR, "unknown interpolation function for continuous temporal type: %d",
    inst1->temptype);
  return 0; /* make compiler quiet */
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the base value of a temporal sequence at a timestamp.
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @param[in] strict True if inclusive/exclusive bounds are taken into account
 * @param[out] result Base value
 * @result Return true if the timestamp is contained in the temporal sequence
 * @sqlfunc valueAtTimestamp()
 */
bool
tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, bool strict,
  Datum *result)
{
  /* Return the value even when the timestamp is at an exclusive bound */
  if (! strict)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    /* Instantaneous sequence or t is at lower bound */
    if (inst->t == t)
    {
      *result = tinstant_value_copy(inst);
      return true;
    }
    inst = TSEQUENCE_INST_N(seq, seq->count - 1);
    if (inst->t == t)
    {
      *result = tinstant_value_copy(inst);
      return true;
    }
  }

  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return false;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    *result = tinstant_value_copy(TSEQUENCE_INST_N(seq, 0));
    return true;
  }

  /* General case */
  int n = tcontseq_find_timestamp(seq, t);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, n);
  if (t == inst1->t)
    *result = tinstant_value_copy(inst1);
  else
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, n + 1);
    bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
    *result = tsegment_value_at_timestamp(inst1, inst2, linear, t);
  }
  return true;
}

/*****************************************************************************
 * Synchronization functions
 *****************************************************************************/

/**
 * @brief Synchronize two temporal sequences
 *
 * The resulting values are composed of denormalized sequences covering the
 * intersection of their time spans. The argument crossings determines
 * whether potential crossings between successive pair of instants are added.
 * Crossings are only added when at least one of the sequences has linear
 * interpolation.
 *
 * @param[in] seq1,seq2 Input values
 * @param[in] crossings True if turning points are added in the segments
 * @param[out] sync1,sync2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
  TSequence **sync1, TSequence **sync2, bool crossings)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return false;

  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq2->flags);
  TInstant *inst1, *inst2;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    inst1 = tsequence_at_timestamp(seq1, inter.lower);
    inst2 = tsequence_at_timestamp(seq2, inter.lower);
    *sync1 = tinstant_to_tsequence(inst1, interp1);
    *sync2 = tinstant_to_tsequence(inst2, interp2);
    pfree(inst1); pfree(inst2);
    return true;
  }

  /*
   * General case
   * seq1 =  ... *     *   *   *      *>
   * seq2 =       <*            *     * ...
   * sync1 =      <X C * C * C X C X C *>
   * sync2 =      <* C X C X C * C * C X>
   * where X are values added for synchronization and C are values added
   * for the crossings
   */
  inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 0, j = 0, ninsts = 0, nfree = 0;
  if (inst1->t < DatumGetTimestampTz(inter.lower))
  {
    i = tcontseq_find_timestamp(seq1, inter.lower) + 1;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
  }
  else if (inst2->t < DatumGetTimestampTz(inter.lower))
  {
    j = tcontseq_find_timestamp(seq2, inter.lower) + 1;
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count * 2);
  meosType basetype1 = temptype_basetype(seq1->temptype);
  meosType basetype2 = temptype_basetype(seq2->temptype);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= DatumGetTimestampTz(inter.upper) ||
     inst2->t <= DatumGetTimestampTz(inter.upper)))
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      inst2 = tsequence_at_timestamp(seq2, inst1->t);
      tofree[nfree++] = inst2;
    }
    else
    {
      j++;
      inst1 = tsequence_at_timestamp(seq1, inst2->t);
      tofree[nfree++] = inst1;
    }
    /* If not the first instant add potential crossing before adding the new
       instants */
    if (crossings && (interp1 == LINEAR || interp2 == LINEAR) && ninsts > 0)
    {
      TimestampTz crosstime;
      Datum inter1, inter2;
      if (tsegment_intersection(instants1[ninsts - 1], inst1,
        (interp1 == LINEAR), instants2[ninsts - 1], inst2, (interp2 == LINEAR),
        &inter1, &inter2, &crosstime))
      {
        instants1[ninsts] = tofree[nfree++] = tinstant_make(inter1,
          seq1->temptype, crosstime);
        instants2[ninsts++] = tofree[nfree++] = tinstant_make(inter2,
          seq2->temptype, crosstime);
      }
    }
    instants1[ninsts] = inst1; instants2[ninsts++] = inst2;
    if (i == seq1->count || j == seq2->count)
      break;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  /* We are sure that ninsts != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and
     exclusive upper bound must be equal */
  if (! inter.upper_inc && ninsts > 1 && (interp1 != LINEAR) &&
      datum_ne(tinstant_value(instants1[ninsts - 2]),
        tinstant_value(instants1[ninsts - 1]), basetype1))
  {
    instants1[ninsts - 1] = tinstant_make(tinstant_value(instants1[ninsts - 2]),
      instants1[ninsts - 1]->temptype, instants1[ninsts - 1]->t);
    tofree[nfree++] = instants1[ninsts - 1];
  }
  if (! inter.upper_inc && ninsts > 1 && (interp2 != LINEAR) &&
      datum_ne(tinstant_value(instants2[ninsts - 2]),
        tinstant_value(instants2[ninsts - 1]), basetype2))
  {
    instants2[ninsts - 1] = tinstant_make(tinstant_value(instants2[ninsts - 2]),
      instants2[ninsts - 1]->temptype, instants2[ninsts - 1]->t);
    tofree[nfree++] = instants2[ninsts - 1];
  }
  *sync1 = tsequence_make((const TInstant **) instants1, ninsts,
    inter.lower_inc, inter.upper_inc, interp1, NORMALIZE_NO);
  *sync2 = tsequence_make((const TInstant **) instants2, ninsts,
    inter.lower_inc, inter.upper_inc, interp2, NORMALIZE_NO);

  pfree_array((void **) tofree, nfree);
  pfree(instants1); pfree(instants2);

  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Temporally intersect two temporal discrete sequences
 * @param[in] seq1,seq2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tdiscseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  TSequence **inter1, TSequence **inter2)
{
  /* Bounding period test */
  if (!overlaps_span_span(&seq1->period, &seq2->period))
    return false;

  int count = Min(seq1->count, seq2->count);
  const TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  int i = 0, j = 0, ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      instants1[ninsts] = inst1;
      instants2[ninsts++] = inst2;
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
    else
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
  }
  if (ninsts != 0)
  {
    *inter1 = tsequence_make(instants1, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
    *inter2 = tsequence_make(instants2, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  }

  pfree(instants1); pfree(instants2);
  return ninsts != 0;
}

/**
 * @brief Temporally intersect two temporal sequences
 * @param[in] seq1,seq2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tcontseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  TSequence **inter1, TSequence **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  if (! overlaps_span_span(&seq1->period, &seq2->period))
    return false;

  TInstant **instants1 = palloc(sizeof(TInstant *) * seq2->count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * seq2->count);
  int ninsts = 0;
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq2, i);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      instants1[ninsts] = tsequence_at_timestamp(seq1, inst->t);
      instants2[ninsts++] = inst;
    }
    if (DatumGetTimestampTz(seq1->period.upper) < inst->t)
      break;
  }
  if (ninsts == 0)
  {
    pfree(instants1); pfree(instants2);
    return false;
  }

  *inter1 = tsequence_make_free(instants1, ninsts, true, true, DISCRETE,
    MERGE_NO);
  *inter2 = tsequence_make(instants2, ninsts, true, true, DISCRETE, MERGE_NO);
  pfree(instants2);
  return true;
}

/**
 * @brief Temporally intersect two temporal values
 * @param[in] seq1,seq2 Temporal values
 * @param[out] inter1,inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tdiscseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  TSequence **inter1, TSequence **inter2)
{
  return intersection_tcontseq_tdiscseq(seq2, seq1, inter2, inter1);
}

/*****************************************************************************
 * Compute the intersection, if any, of a segment of a temporal sequence and
 * a value. The functions only return true when there is an intersection at
 * the middle of the segment, i.e., they return false if they intersect at a
 * bound. When they return true, they also return in the output parameter
 * the intersection timestampt t. The value taken by the segment and the
 * target value are equal up to the floating point precision.
 * There is no need to add functions for DoubleN, which are used for computing
 * avg and centroid aggregates, since these computations are based on sum and
 * thus they do not need to add intermediate points.
 *****************************************************************************/

/**
 * @brief Return true if the segment of a temporal number intersects
 * the base value at a timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[in] basetype Base type
 * @param[out] t Timestamp
 */
static bool
tfloatsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, meosType basetype, TimestampTz *t)
{
  assert(inst1->temptype == T_TFLOAT);
  double dvalue1 = DatumGetFloat8(tinstant_value(inst1));
  double dvalue2 = DatumGetFloat8(tinstant_value(inst2));
  double dvalue = datum_double(value, basetype);
  double min = Min(dvalue1, dvalue2);
  double max = Max(dvalue1, dvalue2);
  /* if value is to the left or to the right of the span */
  if (dvalue < min || dvalue > max)
    return false;

  double span = (max - min);
  double partial = (dvalue - min);
  double fraction = dvalue1 < dvalue2 ? partial / span : 1 - partial / span;
  if (fraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < fraction)
    return false;

  if (t != NULL)
  {
    double duration = (double) (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/**
 * @brief Return true if the segment of a temporal sequence intersects
 * the base value at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[in] basetype Base type
 * @param[out] inter Base value taken by the segment at the timestamp.
 * This value is equal to the input base value up to the floating
 * point precision.
 * @param[out] t Timestamp
 */
bool
tlinearsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, meosType basetype, Datum *inter, TimestampTz *t)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  if (datum_eq2(value, value1, basetype, temptype_basetype(inst1->temptype)) ||
      datum_eq2(value, value2, basetype, temptype_basetype(inst2->temptype)))
    return false;

  assert(temptype_continuous(inst1->temptype));
  bool result = false; /* make compiler quiet */
  if (inst1->temptype == T_TFLOAT)
    result = tfloatsegm_intersection_value(inst1, inst2, value, basetype, t);
  else if (tgeo_type(inst1->temptype))
    result = tpointsegm_intersection_value(inst1, inst2, value, t);
#if NPOINT
  else if (inst1->temptype == T_TNPOINT)
    result = tnpointsegm_intersection_value(inst1, inst2, value, t);
#endif
  else
    elog(ERROR, "unknown intersection function for continuous temporal type: %d",
      inst1->temptype);

  if (result && inter != NULL)
    /* We are sure it is linear interpolation */
    *inter = tsegment_value_at_timestamp(inst1, inst2, true, *t);
  return result;
}

/*****************************************************************************/

/**
 * Compute the intersection, if any, of two segments of temporal sequences.
 * These functions suppose that the instants are synchronized, i.e.,
 * `start1->t = start2->t` and `end1->t = end2->t`.
 * The functions return true if there is an intersection at the middle of
 * the segments, i.e., they return false if they intersect at a bound. If
 * they return true, they also return in the output parameter t the
 * intersection timestamp. The two values taken by the segments at the
 * intersection timestamp t are equal up to the floating point precision.
 * For the temporal point case we cannot use the PostGIS functions
 * `lw_dist2d_seg_seg` and `lw_dist3d_seg_seg` since they do not take time
 * into consideration and would return, e.g., that the two segments
 * `[Point(1 1)@t1, Point(3 3)@t2]` and `[Point(3 3)@t1, Point(1 1)@t2]`
 * intersect at `Point(1 1)`, instead of `Point(2 2)`.
 * These functions are used to add intermediate points when lifting
 * operators, in particular for temporal comparisons such as
 * `tfloat <comp> tfloat` where `<comp>` is `<`, `<=`, ... since the
 * comparison changes its value before/at/after the intersection point.
 */

/**
 * @brief Return true if two segments of two temporal numbers intersect at a
 * timestamp
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 * @note Only the intersection inside the segments is considered
 */
static bool
tnumbersegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1 = tnumberinst_double(start1);
  double x2 = tnumberinst_double(end1);
  double x3 = tnumberinst_double(start2);
  double x4 = tnumberinst_double(end2);

  /* Segments intersecting in the boundaries */
  if (float8_eq(x1, x3) || float8_eq(x2, x4))
    return false;

  /*
   * Using the parametric form of the segments, compute the instant t at which
   * the two segments are equal: x1 + (x2 - x1) t = x3 + (x4 -x3) t
   * that is t = (x3 - x1) / (x2 - x1 - x4 + x3).
   */
  long double denom = x2 - x1 - x4 + x3;
  if (denom == 0)
    /* Parallel segments */
    return false;

  /*
   * Potentially avoid the division based on
   * Franklin Antonio, Faster Line Segment Intersection, Graphic Gems III
   * https://github.com/erich666/GraphicsGems/blob/master/gemsiii/insectc.c
   */
  long double num = x3 - x1;
  if (denom > 0)
  {
    if (num < 0 || num > denom)
      return false;
  }
  else
  {
    if (num > 0 || num < denom)
      return false;
  }

  long double fraction = num / denom;
  if (fraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < fraction )
    /* Intersection occurs out of the period */
    return false;

  double duration = (double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * @brief Return true if  two segments of a temporal sequence intersect at a
 * timestamp
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] linear1 True if the first segment has linear interpolation
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] linear2 True if the second segment has linear interpolation
 * @param[out] inter1, inter2 Base values taken by the two segments
 * at the timestamp
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
bool
tsegment_intersection(const TInstant *start1, const TInstant *end1,
  bool linear1, const TInstant *start2, const TInstant *end2, bool linear2,
  Datum *inter1, Datum *inter2, TimestampTz *t)
{
  bool result = false; /* Make compiler quiet */
  Datum value;
  meosType basetype1 = temptype_basetype(start1->temptype);
  meosType basetype2 = temptype_basetype(start2->temptype);
  if (! linear1)
  {
    value = tinstant_value(start1);
    if (inter1 != NULL)
      *inter1 = value;
    result = tlinearsegm_intersection_value(start2, end2, value, basetype1,
      inter2, t);
  }
  else if (! linear2)
  {
    value = tinstant_value(start2);
    if (inter2 != NULL)
      *inter2 = value;
    result = tlinearsegm_intersection_value(start1, end1, value, basetype2,
      inter1, t);
  }
  else
  {
    /* Both segments have linear interpolation */
    assert(temporal_type(start1->temptype));
    if (tnumber_type(start1->temptype))
      result = tnumbersegm_intersection(start1, end1, start2, end2, t);
    else if (start1->temptype == T_TGEOMPOINT)
      result = tgeompointsegm_intersection(start1, end1, start2, end2, t);
    else if (start1->temptype == T_TGEOGPOINT)
      result = tgeogpointsegm_intersection(start1, end1, start2, end2, t);
    /* We are sure it is linear interpolation */
    if (result && inter1 != NULL)
      *inter1 = tsegment_value_at_timestamp(start1, end1, true, *t);
    if (result && inter2 != NULL)
      *inter2 = tsegment_value_at_timestamp(start2, end2, true, *t);
  }
  return result;
}

/**
 * @brief Temporally intersect two temporal sequences
 * @param[in] seq,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tsequence_tinstant(const TSequence *seq, const TInstant *inst,
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
 * @brief Temporally intersect two temporal values
 * @param[in] inst,seq Temporal values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tinstant_tsequence(const TInstant *inst, const TSequence *seq,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tsequence_tinstant(seq, inst, inter2, inter1);
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is ever equal to a base value.
 * @sqlop @p ?=
 */
bool
tsequence_ever_eq(const TSequence *seq, Datum value)
{
  int i;
  Datum value1;
  meosType basetype = temptype_basetype(seq->temptype);

  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) seq, value, EVER))
    return false;

  /* Step interpolation or instantaneous sequence */
  if (! MEOS_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(TSEQUENCE_INST_N(seq, i));
      if (datum_eq(value1, value, basetype))
        return true;
    }
    return false;
  }

  /* Linear interpolation*/
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value(inst1);
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value(inst2);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Constant segment */
    if (datum_eq(value1, value2, basetype) &&
        datum_eq(value1, value, basetype))
      return true;
    /* Test bounds */
    if (datum_eq(value1, value, basetype))
    {
      if (lower_inc) return true;
    }
    else if (datum_eq(value2, value, basetype))
    {
      if (upper_inc) return true;
    }
    /* Interpolation for continuous base type */
    else if (tlinearsegm_intersection_value(inst1, inst2, value, basetype,
      NULL, NULL))
      return true;
    inst1 = inst2;
    value1 = value2;
    lower_inc = true;
  }
  return false;
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is always equal to a base value.
 * @sqlop @p %=
 */
bool
tsequence_always_eq(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) seq, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for
   * temporal numbers and points */
  if (tnumber_type(seq->temptype))
    return true;

  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_ne(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Return true if the segment of a temporal sequence with linear
 * interpolation is ever less than or equal to a base value
 * (iterator function)
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_ever_le_iter(Datum value1, Datum value2, meosType basetype,
  bool lower_inc, bool upper_inc, Datum value)
{
  /* Constant segment */
  if (datum_eq(value1, value2, basetype))
    return datum_le(value1, value, basetype);
  /* Increasing segment */
  if (datum_lt(value1, value2, basetype))
    return datum_lt(value1, value, basetype) ||
      (lower_inc && datum_eq(value1, value, basetype));
  /* Decreasing segment */
  return datum_lt(value2, value, basetype) ||
    (upper_inc && datum_eq(value2, value, basetype));
}

/**
 * @brief Return true if the segment of a temporal sequence with linear
 * interpolation is always less than a base value (iterator function).
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_always_lt_iter(Datum value1, Datum value2, meosType basetype,
  bool lower_inc, bool upper_inc, Datum value)
{
  /* Constant segment */
  if (datum_eq(value1, value2, basetype))
    return datum_lt(value1, value1, basetype);
  /* Increasing segment */
  if (datum_lt(value1, value2, basetype))
    return datum_lt(value2, value, basetype) ||
      (! upper_inc && datum_eq(value, value2, basetype));
  /* Decreasing segment */
  return datum_lt(value1, value, basetype) ||
    (! lower_inc && datum_eq(value1, value, basetype));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is ever less than a base value.
 * @sqlop @p ?<
 */
bool
tsequence_ever_lt(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, EVER))
    return false;

  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (datum_lt(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is ever less than or equal to a
 * base value
 * @sqlop @p ?<=
 */
bool
tsequence_ever_le(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, EVER))
    return false;

  Datum value1;
  int i;
  meosType basetype = temptype_basetype(seq->temptype);

  /* Step interpolation or instantaneous sequence */
  if (! MEOS_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(TSEQUENCE_INST_N(seq, i));
      if (datum_le(value1, value, basetype))
        return true;
    }
    return false;
  }

  /* Linear interpolation */
  value1 = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    Datum value2 = tinstant_value(TSEQUENCE_INST_N(seq, i));
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (tlinearseq_ever_le_iter(value1, value2, basetype, lower_inc, upper_inc,
        value))
      return true;
    value1 = value2;
    lower_inc = true;
  }
  return false;
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is always less than a base value.
 * @sqlop @p %<
 */
bool
tsequence_always_lt(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, ALWAYS))
    return false;

  Datum value1;
  int i;
  meosType basetype = temptype_basetype(seq->temptype);

  /* Step interpolation or instantaneous sequence */
  if (! MEOS_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(TSEQUENCE_INST_N(seq, i));
      if (! datum_lt(value1, value, basetype))
        return false;
    }
    return true;
  }

  /* Linear interpolation */
  value1 = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    Datum value2 = tinstant_value(TSEQUENCE_INST_N(seq, i));
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (! tlinearseq_always_lt_iter(value1, value2, basetype, lower_inc,
        upper_inc, value))
      return false;
    value1 = value2;
    lower_inc = true;
  }
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal sequence is always less than or equal to a
 * base value
 * @sqlop @p %<=
 */
bool
tsequence_always_le(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute
   * the answer for temporal numbers */
  if (tnumber_type(seq->temptype))
    return true;

  /* We are sure that the type has stewpwise interpolation since
   * there are currenty no other continuous base type besides tfloat
   * to which the always <= comparison applies */
  assert(! MEOS_FLAGS_GET_LINEAR(seq->flags));
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(TSEQUENCE_INST_N(seq, i));
    if (! datum_le(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a base
 * value.
 * @param[in] seq Temporal sequence
 * @param[in] value Base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 * @sqlfunc atValue(), minusValue()
 */
TSequence *
tdiscseq_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  meosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    Datum value1 = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    bool equal = datum_eq(value, value1, basetype);
    if ((atfunc && ! equal) || (! atfunc && equal))
      return NULL;
    return tsequence_copy(seq);
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool equal = datum_eq(value, tinstant_value(inst), basetype);
    if ((atfunc && equal) || (! atfunc && ! equal))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) an array
 * of base values.
 * @param[in] seq Temporal sequence
 * @param[in] set Set of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 * @sqlfunc atValues(), minusValues()
 */
TSequence *
tdiscseq_restrict_values(const TSequence *seq, const Set *set, bool atfunc)
{
  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tinstant_restrict_values_test(inst, set, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    if (tinstant_restrict_values_test(inst, set, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a segment of a temporal sequence to (the complement of) a
 * base value.
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] interp Interpolation
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 * @return Number of resulting sequences returned
 */
static int
tsegment_restrict_value(const TInstant *inst1, const TInstant *inst2,
  interpType interp, bool lower_inc, bool upper_inc, Datum value, bool atfunc,
  TSequence **result)
{
  assert(interp != DISCRETE);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  meosType basetype = temptype_basetype(inst1->temptype);
  TInstant *instants[2];
  /* Is the segment constant? */
  bool isconst = datum_eq(value1, value2, basetype);
  /* Does the lower bound belong to the answer? */
  bool lower = atfunc ? datum_eq(value1, value, basetype) :
    datum_ne(value1, value, basetype);
  /* Does the upper bound belong to the answer? */
  bool upper = atfunc ? datum_eq(value2, value, basetype) :
    datum_ne(value2, value, basetype);
  /* For linear interpolation and not constant segment is the value in the
   * interior of the segment? */
  Datum projvalue = 0; /* make compiler quiet */
  TimestampTz t = 0; /* make compiler quiet */
  bool interior = (interp == LINEAR) && ! isconst &&
    tlinearsegm_intersection_value(inst1, inst2, value, basetype, &projvalue, &t);

  /* Overall segment does not belong to the answer */
  if ((isconst && ! lower) ||
    (! isconst && atfunc && (interp == LINEAR) && ((lower && ! lower_inc) ||
      (upper && ! upper_inc) || (! lower && ! upper && ! interior))))
    return 0;

  /* Segment belongs to the answer but bounds may not */
  if ((isconst && lower) ||
    /* Linear interpolation: Test of bounds */
    (! isconst && (interp == LINEAR) && ! atfunc &&
    (! lower || ! upper || (lower && upper && ! interior))))
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc && lower, upper_inc && upper, interp, NORMALIZE_NO);
    return 1;
  }

  /* Step interpolation */
  if (interp == STEP)
  {
    int nseqs = 0;
    if (lower)
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, STEP, NORMALIZE_NO);
      pfree(instants[1]);
    }
    if (upper_inc && upper)
      result[nseqs++] = tinstant_to_tsequence(inst2, STEP);
    return nseqs;
  }

  /* Linear interpolation: Test of bounds */
  if (atfunc && ((lower && lower_inc) || (upper && upper_inc)))
  {
    result[0] = tinstant_to_tsequence(lower ? inst1 : inst2, LINEAR);
    return 1;
  }
  /* Interpolation */
  if (atfunc)
  {
    TInstant *inst = tinstant_make(projvalue, inst1->temptype, t);
    result[0] = tinstant_to_tsequence(inst, LINEAR);
    pfree(inst);
    DATUM_FREE(projvalue, basetype);
    return 1;
  }
  else
  {
    /* Due to roundoff errors t may be equal to inst1-> or ins2->t */
    if (t == inst1->t)
    {
      DATUM_FREE(projvalue, basetype);
      if (! lower_inc)
        return 0;

      instants[0] = (TInstant *) inst1;
      instants[1] = (TInstant *) inst2;
      result[0] = tsequence_make((const TInstant **) instants, 2,
        ! lower_inc, upper_inc, LINEAR, NORMALIZE_NO);
      return 1;
    }
    else if (t == inst2->t)
    {
      DATUM_FREE(projvalue, basetype);
      if (! upper_inc)
        return 0;

      instants[0] = (TInstant *) inst1;
      instants[1] = (TInstant *) inst2;
      result[0] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, ! upper_inc, LINEAR, NORMALIZE_NO);
      return 1;
    }
    else
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(projvalue, inst1->temptype, t);
      result[0] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, LINEAR, NORMALIZE_NO);
      instants[0] = instants[1];
      instants[1] = (TInstant *) inst2;
      result[1] = tsequence_make((const TInstant **) instants, 2,
        false, upper_inc, LINEAR, NORMALIZE_NO);
      pfree(instants[0]);
      DATUM_FREE(projvalue, basetype);
      return 2;
    }
  }
}

/**
 * @brief Restrict a temporal sequence to (the complement of) a base value
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] value Base value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set.
 * For this reason the bounding box and the instantaneous sequence sets are
 * repeated here.
 */
int
tcontseq_restrict_value_iter(const TSequence *seq, Datum value, bool atfunc,
  TSequence **result)
{
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    /* We do not call the function tinstant_restrict_value since this
     * would create a new unnecessary instant that needs to be freed */
    bool equal = datum_eq(tinstant_value(inst1), value,
      temptype_basetype(seq->temptype));
    if ((atfunc && ! equal) || (! atfunc && equal))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Bounding box test */
  if (! temporal_bbox_restrict_value((Temporal *) seq, value))
  {
    if (atfunc)
      return 0;
    /* Minus function */
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Each iteration adds between 0 and 2 sequences */
    nseqs += tsegment_restrict_value(inst1, inst2, interp, lower_inc, upper_inc,
      value, atfunc, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) a base value.
 * @param[in] seq Temporal sequence
 * @param[in] value Base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note There is no bounding box or instantaneous test in this function,
 * they are done in the atValue and minusValue functions since the latter are
 * called for each sequence in a sequence set or for each element in the array
 * for the atValues and minusValues functions.
 * @sqlfunc atValue(), minusValue()
 */
TSequenceSet *
tcontseq_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tcontseq_restrict_value_iter(seq, value, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to an array of base values
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] set Set of base values
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre There are no duplicates values in the array
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_at_values_iter(const TSequence *seq, const Set *set,
  TSequence **result)
{
  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tinstant_restrict_values(inst1, set, REST_AT);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Bounding box test */
  if (! temporal_bbox_restrict_set((Temporal *) seq, set))
    return 0;

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    for (int j = 0; j < set->count; j++)
      /* Each iteration adds between 0 and 2 sequences */
      nseqs += tsegment_restrict_value(inst1, inst2, interp, lower_inc,
        upper_inc, SET_VAL_N(set, j), REST_AT, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  if (nseqs > 1)
    tseqarr_sort(result, nseqs);

  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) an array of base
 * values.
 * @param[in] seq Temporal sequence
 * @param[in] set Set of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function #tsequence_at_values_iter since the latter is called
 * for each composing sequence of a temporal sequence set number.
 * @sqlfunc atValues(), minusValues()
 */
TSequenceSet *
tcontseq_restrict_values(const TSequence *seq, const Set *set, bool atfunc)
{
  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count *
    set->count * 2);
  int newcount = tsequence_at_values_iter(seq, set, sequences);
  TSequenceSet *atresult = tsequenceset_make_free(sequences, newcount, NORMALIZE);
  if (atfunc)
    return atresult;

  /*
   * MINUS function
   * Compute the complement of the previous value.
   */
  if (newcount == 0)
    return tsequence_to_tsequenceset(seq);

  SpanSet *ps1 = tsequenceset_time(atresult);
  SpanSet *ps2 = minus_span_spanset(&seq->period, ps1);
  TSequenceSet *result = NULL;
  if (ps2 != NULL)
  {
    result = tcontseq_restrict_periodset(seq, ps2, REST_AT);
    pfree(ps2);
  }
  pfree(atresult); pfree(ps1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete number sequence to (the complement of) a
 * span of base values.
 * @param[in] seq Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpan(), minusSpan()
 */
TSequence *
tnumberdiscseq_restrict_span(const TSequence *seq, const Span *span,
  bool atfunc)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (tnumberinst_restrict_span_test(inst, span, atfunc))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence number to (the complement of) an
 * array of spans of base values.
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpanset(), minusSpanset()
 */
TSequence *
tnumberdiscseq_restrict_spanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE,
      NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a segment of a temporal number to (the complement of) a span
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] linear True if the segment has linear interpolation
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 */
static int
tnumbersegm_restrict_span(const TInstant *inst1, const TInstant *inst2,
  bool linear, bool lower_inc, bool upper_inc, const Span *span,
  bool atfunc, TSequence **result)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  meosType basetype = temptype_basetype(inst1->temptype);
  interpType interp = linear ? LINEAR : STEP;
  TInstant *instants[2];
  bool found;

  /* Constant segment (step or linear interpolation) */
  if (datum_eq(value1, value2, basetype))
  {
    found = contains_span_value(span, value1, basetype);
    if ((atfunc && ! found) || (! atfunc && found))
      return 0;
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    return 1;
  }

  /* Step interpolation */
  if (! linear)
  {
    int nseqs = 0;
    found = contains_span_value(span, value1, basetype);
    if ((atfunc && found) || (! atfunc && ! found))
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, interp, NORMALIZE_NO);
      pfree(instants[1]);
    }
    found = contains_span_value(span, value2, basetype);
    if (upper_inc &&
      ((atfunc && found) || (! atfunc && ! found)))
    {
      result[nseqs++] = tinstant_to_tsequence(inst2, interp);
    }
    return nseqs;
  }

  /* Linear interpolation */

  /* Compute the intersection of the spans */
  Span valuespan, inter;
  bool increasing = DatumGetFloat8(value1) < DatumGetFloat8(value2);
  if (increasing)
    span_set(value1, value2, lower_inc, upper_inc, basetype, &valuespan);
  else
    span_set(value2, value1, upper_inc, lower_inc, basetype, &valuespan);
  found = inter_span_span(&valuespan, span, &inter);
  /* The intersection is empty */
  if (! found)
  {
    if (atfunc)
      return 0;
    /* MINUS */
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    return 1;
  }

  /* Compute the instants of the intersection */
  TInstant *inter1, *inter2;
  bool tofree1 = false, tofree2 = false;
  TimestampTz t1, t2;
  Datum lower, upper;
  bool lower_inc1, upper_inc1;
  if (increasing)
  {
    lower = inter.lower; upper = inter.upper;
    lower_inc1 = inter.lower_inc; upper_inc1 = inter.upper_inc;
  }
  else
  {
    lower = inter.upper; upper = inter.lower;
    lower_inc1 = inter.upper_inc; upper_inc1 = inter.lower_inc;
  }
  tfloatsegm_intersection_value(inst1, inst2, lower, basetype, &t1);
  if (t1 == inst1->t)
    inter1 = (TInstant *) inst1;
  else if (t1 == inst2->t)
    inter1 = (TInstant *) inst2;
  else
  {
    /* To reduce the roundoff errors we project the temporal number to the
     * timestamp instead of taking the bound value */
    inter1 = tsegment_at_timestamp(inst1, inst2, linear, t1);
    tofree1 = true;
  }
  int j = 1;
  if (! datum_eq(lower, upper, basetype))
  {
    tfloatsegm_intersection_value(inst1, inst2, upper, basetype, &t2);
    if (t2 == inst1->t)
      inter2 = (TInstant *) inst1;
    else if (t2 == inst2->t)
      inter2 = (TInstant *) inst2;
    else
    {
      /* To reduce the roundoff errors we project the temporal number to the
       * timestamp instead of taking the bound value */
      inter2 = tsegment_at_timestamp(inst1, inst2, linear, t2);
      tofree2 = true;
    }
    j = 2;
  }

  /* Compute the result */
  int nseqs = 0;
  if (atfunc)
  {
    /* We need order the instants */
    if (j > 1 && inter1->t > inter2->t)
    {
      TInstant *swap = inter1;
      inter1 = inter2;
      inter2 = swap;
      tofree1 = ! tofree1;
      tofree2 = ! tofree2;
    }
    instants[0] = inter1;
    if (j > 1)
      instants[1] = inter2;
    result[nseqs++] = tsequence_make((const TInstant **) instants, j,
        lower_inc1, upper_inc1, interp, NORMALIZE_NO);
  }
  else
  {
    /* First segment if any */
    if (j == 1)
      inter2 = inter1;
    if (inter1->t == inst1->t)
    {
      if (lower_inc && ! lower_inc1)
      {
        instants[0] = inter1;
        result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, interp, NORMALIZE_NO);
      }
    }
    else
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = inter1;
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, ! lower_inc1, interp, NORMALIZE_NO);
    }
    /* Second segment if any */
    if (inter2->t < inst2->t)
    {
      instants[0] = (j == 1) ? inter1 : inter2;
      instants[1] = (TInstant *) inst2;
      result[nseqs++] = tsequence_make((const TInstant **) instants, 2,
          ! upper_inc1, upper_inc, interp, NORMALIZE_NO);
    }
    else
    {
      if (upper_inc && ! upper_inc1)
      {
        instants[0] = (j == 1) ? inter1 : inter2;
        result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, interp, NORMALIZE_NO);
      }
    }
  }
  if (tofree1)
    pfree(inter1);
  if (j > 1 && tofree2)
    pfree(inter2);
  return nseqs;
}

/**
 * @brief Restrict a temporal number to (the complement of) a span
 * (iterator function).
 * @param[in] seq temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_span_iter(const TSequence *seq, const Span *span,
  bool atfunc, TSequence **result)
{
  /* Bounding box test */
  TBox box1, box2;
  tsequence_set_bbox(seq, &box1);
  numspan_set_tbox(span, &box2);
  if (! overlaps_tbox_tbox(&box1, &box2))
  {
    if (atfunc)
      return 0;
    else
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }
  }

  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    /* The bounding box test above does not distinguish between
     * inclusive/exclusive bounds */
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tnumberinst_restrict_span(inst1, span, atfunc);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = TSEQUENCE_INST_N(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    nseqs += tnumbersegm_restrict_span(inst1, inst2, linear, lower_inc,
      upper_inc, span, atfunc, &result[nseqs]);
    inst1 = inst2;
    lower_inc = true;
  }
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence number to (the complement of) a span.
 * @param[in] seq Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note It is supposed that a bounding box test has been done in the dispatch
 * function.
 * @sqlfunc atSpan(), minusSpan()
 */
TSequenceSet *
tnumbercontseq_restrict_span(const TSequence *seq, const Span *span,
  bool atfunc)
{
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tnumbercontseq_restrict_span_iter(seq, span, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal number to (the complement of) an array of spans
 * of base values (iterator function).
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre The array of spans is normalized
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_spanset_iter(const TSequence *seq, const SpanSet *ss,
  bool atfunc, TSequence **result)
{
  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tnumberinst_restrict_spanset(inst1, ss, atfunc);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  if (atfunc)
  {
    /* AT function */
    inst1 = TSEQUENCE_INST_N(seq, 0);
    bool lower_inc = seq->period.lower_inc;
    int nseqs = 0;
    for (int i = 1; i < seq->count; i++)
    {
      inst2 = TSEQUENCE_INST_N(seq, i);
      bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
      for (int j = 0; j < ss->count; j++)
      {
        const Span *s = spanset_sp_n(ss, j);
        nseqs += tnumbersegm_restrict_span(inst1, inst2, linear, lower_inc,
          upper_inc, s, REST_AT, &result[nseqs]);
      }
      inst1 = inst2;
      lower_inc = true;
    }
    if (nseqs > 1)
      tseqarr_sort(result, nseqs);
    return nseqs;
  }
  else
  {
    /*
     * MINUS function
     * Compute first the tnumberseq_at_spans, then compute its complement
     * Notice that in this case due to rounoff errors it may be the case
     * that temp is not equal to merge(atSpans(temp, .),minusSpans(temp, .),
     * since we kept the span values instead of the projected values when
     * computing atSpans
     */
    TSequenceSet *seqset = tnumbercontseq_restrict_spanset(seq, ss, REST_AT);
    if (seqset == NULL)
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }

    SpanSet *ps1 = tsequenceset_time(seqset);
    SpanSet *ps2 = minus_span_spanset(&seq->period, ps1);
    int newcount = 0;
    if (ps2 != NULL)
    {
      newcount = tcontseq_at_periodset1(seq, ps2, result);
      pfree(ps2);
    }
    pfree(seqset); pfree(ps1);
    return newcount;
  }
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal number to (the complement of) an array of spans.
 * @param[in] seq Temporal number
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @sqlfunc atSpanset(), minusSpanset()
 */
TSequenceSet *
tnumbercontseq_restrict_spanset(const TSequence *seq, const SpanSet *ss,
  bool atfunc)
{
  /* General case */
  int maxcount = seq->count * ss->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MEOS_FLAGS_GET_LINEAR(seq->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int newcount = tnumbercontseq_restrict_spanset_iter(seq, ss, atfunc,
    sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) its
 * minimum/maximum base value
 * @param[in] seq Temporal sequence
 * @param[in] min True if restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @sqlfunc atMin(), atMax(), minusMin(), minusMax()
 */
TSequence *
tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  Datum minmax = min ? tsequence_min_value(seq) : tsequence_max_value(seq);
  return tdiscseq_restrict_value(seq, minmax, atfunc);
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) its
 * minimum/maximum base value.
 * @param[in] seq Temporal sequence
 * @param[in] min True if restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @sqlfunc atMin(), atMax(), minusMin(), minusMax()
 */
TSequenceSet *
tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  Datum minmax = min ? tsequence_min_value(seq) : tsequence_max_value(seq);
  return tcontseq_restrict_value(seq, minmax, atfunc);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the base value of a temporal discrete sequence at a timestamp
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc valueAtTimestamp()
 */
bool
tdiscseq_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result)
{
  int loc = tdiscseq_find_timestamp(seq, t);
  if (loc < 0)
    return false;

  const TInstant *inst = TSEQUENCE_INST_N(seq, loc);
  *result = tinstant_value_copy(inst);
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc atTime()
 */
TInstant *
tdiscseq_at_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return NULL;

  /* Instantenous sequence */
  if (seq->count == 1)
    return tinstant_copy(TSEQUENCE_INST_N(seq, 0));

  /* General case */
  const TInstant *inst;
  int loc = tdiscseq_find_timestamp(seq, t);
  if (loc < 0)
    return NULL;
  inst = TSEQUENCE_INST_N(seq, loc);
  return tinstant_copy(inst);
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc minusTime()
 */
TSequence *
tdiscseq_minus_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return tsequence_copy(seq);

  /* Instantenous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (inst->t != t)
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp set.
 * @sqlfunc atTime(), minusTime()
 */
TSequence *
tdiscseq_restrict_timestampset(const TSequence *seq, const Set *ts,
  bool atfunc)
{
  TSequence *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    Temporal *temp = atfunc ?
      (Temporal *) tdiscseq_at_timestamp(seq,
        DatumGetTimestampTz(SET_VAL_N(ts, 0))) :
      (Temporal *) tdiscseq_minus_timestamp(seq,
        DatumGetTimestampTz(SET_VAL_N(ts, 0)));
    if (temp == NULL || ! atfunc)
      return (TSequence *) temp;
    /* Transform the result of tdiscseq_at_timestamp into a sequence */
    result = tinstant_to_tsequence((const TInstant *) temp, DISCRETE);
    pfree(temp);
    return result;
  }

  /* Bounding box test */
  Span p;
  set_set_span(ts, &p);
  if (! overlaps_span_span(&seq->period, &p))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, ninsts = 0;
  while (i < seq->count && j < ts->count)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(ts, j));
    int cmp = timestamptz_cmp_internal(inst->t, t);
    if (cmp == 0)
    {
      if (atfunc)
        instants[ninsts++] = inst;
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      if (! atfunc)
        instants[ninsts++] = inst;
      i++;
    }
    else
      j++;
  }
  /* For minus copy the instants after the discrete sequence */
  if (! atfunc)
  {
    while (i < seq->count)
      instants[ninsts++] = TSEQUENCE_INST_N(seq, i++);
  }
  result = (ninsts == 0) ? NULL : tsequence_make(instants, ninsts, true, true,
    DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a period.
 * @sqlfunc atTime()
 */
TSequence *
tdiscseq_restrict_period(const TSequence *seq, const Span *period, bool atfunc)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, period))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    bool contains = contains_period_timestamp(period, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a discrete temporal sequence to (the complement of) a period set.
 * @sqlfunc atTime(), minusTime()
 */
TSequence *
tdiscseq_restrict_periodset(const TSequence *seq, const SpanSet *ps,
  bool atfunc)
{
  const TInstant *inst;

  /* Singleton period set */
  if (ps->count == 1)
    return tdiscseq_restrict_period(seq, spanset_sp_n(ps, 0), atfunc);

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->span))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (tinstant_restrict_periodset_test(inst, ps, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    bool contains = contains_periodset_timestamp(ps, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict the segment of a temporal sequence to a timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] linear True if linear interpolation
 * @param[in] t Timestamp
 * @pre The timestamp t satisfies inst1->t <= t <= inst2->t
 * @note The function creates a new value that must be freed
 */
TInstant *
tsegment_at_timestamp(const TInstant *inst1, const TInstant *inst2,
  bool linear, TimestampTz t)
{
  Datum value = tsegment_value_at_timestamp(inst1, inst2, linear, t);
  TInstant *result = tinstant_make(value, inst1->temptype, t);
  DATUM_FREE(value, temptype_basetype(inst1->temptype));
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal continuous sequence to a timestamp.
 * @sqlfunc atTimestamp()
 */
TInstant *
tcontseq_at_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tinstant_copy(TSEQUENCE_INST_N(seq, 0));

  /* General case */
  int n = tcontseq_find_timestamp(seq, t);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, n);
  if (t == inst1->t)
    return tinstant_copy(inst1);
  else
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, n + 1);
    return tsegment_at_timestamp(inst1, inst2,
      MEOS_FLAGS_GET_LINEAR(seq->flags), t);
  }
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to a timestamp.
 * @sqlfunc atTimestamp()
 */
TInstant *
tsequence_at_timestamp(const TSequence *seq, TimestampTz t)
{
  if (MEOS_FLAGS_GET_DISCRETE(seq->flags))
    return tdiscseq_at_timestamp(seq, t);
  else
    return tcontseq_at_timestamp(seq, t);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to the complement of a timestamp
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tcontseq_minus_timestamp_iter(const TSequence *seq, TimestampTz t,
  TSequence **result)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  const TInstant *inst1, *inst2;
  inst1 = TSEQUENCE_INST_N(seq, 0);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  int i, nseqs = 0;
  int n = tcontseq_find_timestamp(seq, t);
  /* Compute the first sequence until t */
  if (n != 0 || inst1->t < t)
  {
    for (i = 0; i < n; i++)
      instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i);
    inst1 = TSEQUENCE_INST_N(seq, n);
    inst2 = TSEQUENCE_INST_N(seq, n + 1);
    if (inst1->t == t)
    {
      if (interp == LINEAR)
      {
        instants[n] = (TInstant *) inst1;
        result[nseqs++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, interp, NORMALIZE_NO);
      }
      else
      {
        instants[n] = tinstant_make(tinstant_value(instants[n - 1]),
          inst1->temptype, t);
        result[nseqs++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, interp, NORMALIZE_NO);
        pfree(instants[n]);
      }
    }
    else
    {
      /* inst1->t < t */
      instants[n] = (TInstant *) inst1;
      instants[n + 1] = (interp == LINEAR) ?
        tsegment_at_timestamp(inst1, inst2, (interp == LINEAR), t) :
        tinstant_make(tinstant_value(inst1), inst1->temptype, t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, n + 2,
        seq->period.lower_inc, false, interp, NORMALIZE_NO);
      pfree(instants[n + 1]);
    }
  }
  /* Compute the second sequence after t */
  inst1 = TSEQUENCE_INST_N(seq, n);
  inst2 = TSEQUENCE_INST_N(seq, n + 1);
  if (t < inst2->t)
  {
    instants[0] = tsegment_at_timestamp(inst1, inst2, (interp == LINEAR), t);
    for (i = 1; i < seq->count - n; i++)
      instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i + n);
    result[nseqs++] = tsequence_make((const TInstant **) instants,
      seq->count - n, false, seq->period.upper_inc, interp, NORMALIZE_NO);
    pfree(instants[0]);
  }
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a timestamp.
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @sqlfunc minusTimestamp()
 */
TSequenceSet *
tcontseq_minus_timestamp(const TSequence *seq, TimestampTz t)
{
  TSequence *sequences[2];
  int count = tcontseq_minus_timestamp_iter(seq, t, sequences);
  if (count == 0)
    return NULL;
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(sequences[i]);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to a timestamp set.
 * @sqlfunc atTstzSet()
 */
TSequence *
tcontseq_at_timestampset(const TSequence *seq, const Set *ts)
{
  TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    inst = tsequence_at_timestamp(seq,
      DatumGetTimestampTz(SET_VAL_N(ts, 0)));
    if (inst == NULL)
      return NULL;
    TSequence *result = tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
    pfree(inst);
    return result;
  }

  /* Bounding box test */
  Span p;
  set_set_span(ts, &p);
  if (! overlaps_span_span(&seq->period, &p))
    return NULL;

  inst = (TInstant *) TSEQUENCE_INST_N(seq, 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (! contains_set_value(ts, TimestampTzGetDatum(inst->t),
        T_TIMESTAMPTZ))
      return NULL;
    return tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
  }

  /* General case */
  TimestampTz t = Max(DatumGetTimestampTz(seq->period.lower),
    DatumGetTimestampTz(SET_VAL_N(ts, 0)));
  int loc;
  set_find_value(ts, TimestampTzGetDatum(t), &loc);
  TInstant **instants = palloc(sizeof(TInstant *) * (ts->count - loc));
  int ninsts = 0;
  for (int i = loc; i < ts->count; i++)
  {
    t = DatumGetTimestampTz(SET_VAL_N(ts, i));
    inst = tcontseq_at_timestamp(seq, t);
    if (inst != NULL)
      instants[ninsts++] = inst;
  }
  if (ninsts == 0)
  {
    pfree(instants);
    return NULL;
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to the complement of a timestamp set
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] ts Tstzset
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @note This function is called for each sequence of a temporal sequence set
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_timestampset_iter(const TSequence *seq, const Set *ts,
  TSequence **result)
{
  /* Singleton timestamp set */
  if (ts->count == 1)
    return tcontseq_minus_timestamp_iter(seq,
      DatumGetTimestampTz(SET_VAL_N(ts, 0)), result);

  /* Bounding box test */
  Span p;
  set_set_span(ts, &p);
  if (! overlaps_span_span(&seq->period, &p))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
    if (contains_set_value(ts, TimestampTzGetDatum(inst1->t),
        T_TIMESTAMPTZ))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant **tofree = palloc(sizeof(TInstant *) * Min(ts->count, seq->count));
  bool lower_inc = seq->period.lower_inc;
  int i = 0,    /* current instant of the argument sequence */
    j = 0,      /* current timestamp of the argument timestamp set */
    nseqs = 0,  /* current number of new sequences */
    ninsts = 0, /* number of instants in the currently constructed sequence */
    nfree = 0;  /* number of instants to free */
  while (i < seq->count && j < ts->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(ts, j));
    Datum value;
    if (inst->t < t)
    {
      instants[ninsts++] = (TInstant *) inst;
      i++; /* advance instants */
    }
    else if (inst->t == t)
    {
      /* Close the current sequence */
      if (ninsts > 0)
      {
        if (interp == LINEAR)
          instants[ninsts++] = (TInstant *) inst;
        else /* interp == STEP */
        {
          /* Take the value of the previous instant */
          value = tinstant_value(instants[ninsts - 1]);
          instants[ninsts] = tinstant_make(value, inst->temptype, inst->t);
          tofree[nfree++] = instants[ninsts++];
        }
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, false, interp, NORMALIZE_NO);
        ninsts = 0;
      }
      /* If it is not the last instant start a new sequence */
      if (i < seq->count - 1)
      {
        instants[ninsts++] = (TInstant *) inst;
        lower_inc = false;
      }
      i++; /* advance instants */
      j++; /* advance timestamps */
    }
    else /* inst->t > t */
    {
      if (ninsts > 0)
      {
        /* Close the current sequence */
        if (interp == LINEAR)
          /* Interpolate */
          value = tsegment_value_at_timestamp(instants[ninsts - 1], inst,
            LINEAR, t);
        else
          /* Take the value of the previous instant */
          value = tinstant_value(instants[ninsts - 1]);
        instants[ninsts] = tinstant_make(value, inst->temptype, t);
        tofree[nfree] = instants[ninsts++];
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, false, interp, NORMALIZE_NO);
        /* Restart a new sequence */
        instants[0] = tofree[nfree++];
        ninsts = 1;
        lower_inc = false;
      }
      j++; /* advance timestamps */
    }
  }
  /* Compute the sequence after the timestamp set */
  if (i < seq->count)
  {
    for (j = i; j < seq->count; j++)
      instants[ninsts++] = (TInstant *) TSEQUENCE_INST_N(seq, j);
  }
  if (ninsts > 0)
  {
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      lower_inc, seq->period.upper_inc, interp, NORMALIZE_NO);
  }
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a timestamp set.
 * @sqlfunc minusTstzSet()
 */
TSequenceSet *
tcontseq_minus_timestampset(const TSequence *seq, const Set *ts)
{
  TSequence **sequences = palloc0(sizeof(TSequence *) * (ts->count + 1));
  int count = tcontseq_minus_timestampset_iter(seq, ts, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a continuous temporal sequence to a period.
 */
TSequence *
tcontseq_at_period(const TSequence *seq, const Span *p)
{
  /* Bounding box test */
  Span inter;
  if (! inter_span_span(&seq->period, p, &inter))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tsequence_copy(seq);

  /* General case */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result;
  /* Intersecting period is instantaneous */
  if (inter.lower == inter.upper)
  {
    TInstant *inst = tcontseq_at_timestamp(seq, inter.lower);
    result = tinstant_to_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  const TInstant *inst1, *inst2;
  int n = tcontseq_find_timestamp(seq, inter.lower);
  /* If the lower bound of the intersecting period is exclusive */
  if (n == -1)
    n = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * (seq->count - n));
  /* Compute the value at the beginning of the intersecting period */
  inst1 = TSEQUENCE_INST_N(seq, n);
  inst2 = TSEQUENCE_INST_N(seq, n + 1);
  instants[0] = tsegment_at_timestamp(inst1, inst2, (interp == LINEAR),
    inter.lower);
  int ninsts = 1;
  for (int i = n + 2; i < seq->count; i++)
  {
    /* If the end of the intersecting period is between inst1 and inst2 */
    if (inst1->t <= DatumGetTimestampTz(inter.upper) &&
        DatumGetTimestampTz(inter.upper) <= inst2->t)
      break;

    inst1 = inst2;
    inst2 = TSEQUENCE_INST_N(seq, i);
    /* If the intersecting period contains inst1 */
    if (DatumGetTimestampTz(inter.lower) <= inst1->t &&
        inst1->t <= DatumGetTimestampTz(inter.upper))
      instants[ninsts++] = (TInstant *) inst1;
  }
  /* The last two values of sequences with step interpolation and
   * exclusive upper bound must be equal */
  if (interp == LINEAR || inter.upper_inc)
    instants[ninsts++] = tsegment_at_timestamp(inst1, inst2,
      (interp == LINEAR), inter.upper);
  else
  {
    Datum value = tinstant_value(instants[ninsts - 1]);
    instants[ninsts++] = tinstant_make(value, seq->temptype, inter.upper);
  }
  /* Since by definition the sequence is normalized it is not necessary to
   * normalize the projection of the sequence to the period */
  result = tsequence_make((const TInstant **) instants, ninsts,
    inter.lower_inc, inter.upper_inc, interp, NORMALIZE_NO);

  pfree(instants[0]); pfree(instants[ninsts - 1]); pfree(instants);

  return result;
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to a period.
 * @sqlfunc atPeriod()
 */
TSequence *
tsequence_at_period(const TSequence *seq, const Span *p)
{
  if(MEOS_FLAGS_GET_DISCRETE(seq->flags))
    return tdiscseq_restrict_period(seq, p, REST_AT);
  else
    return tcontseq_at_period(seq, p);
}
#endif /* MEOS */

/**
 * @brief Restrict a temporal sequence to the complement of a period.
 * (iterator function).
 * @param[in] seq Temporal sequence
 * @param[in] p Period
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_period_iter(const TSequence *seq, const Span *p,
  TSequence **result)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, p))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  SpanSet *ps = minus_span_span(&seq->period, p);
  if (ps == NULL)
    return 0;
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p1 = spanset_sp_n(ps, i);
    result[i] = tcontseq_at_period(seq, p1);
  }
  int count = ps->count;
  pfree(ps);
  return count;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a period.
 * @sqlfunc minusPeriod()
 */
TSequenceSet *
tcontseq_minus_period(const TSequence *seq, const Span *p)
{
  TSequence *sequences[2];
  int count = tcontseq_minus_period_iter(seq, p, sequences);
  if (count == 0)
    return NULL;
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(sequences[i]);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period.
 * @sqlfunc atTime, minusTime
 */
Temporal *
tsequence_restrict_period(const TSequence *seq, const Span *p, bool atfunc)
{
  Temporal *result;
  if (MEOS_FLAGS_GET_DISCRETE(seq->flags))
    result = (Temporal *) tdiscseq_restrict_period(seq, p, atfunc);
  else
    result = atfunc ?
      (Temporal *) tcontseq_at_period(seq, p) :
      (Temporal *) tcontseq_minus_period(seq, p);
  return result;
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal sequence to a period set
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is NOT called for each sequence of a temporal sequence
 * set but is called when computing tpointseq minus geometry
 */
int
tcontseq_at_periodset1(const TSequence *seq, const SpanSet *ps,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
  {
    result[0] = tcontseq_at_period(seq, spanset_sp_n(ps, 0));
    return 1;
  }

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->span))
    return 0;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    if (! contains_periodset_timestamp(ps, inst->t))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  int loc;
  /* The second argument in the following call should be a Datum */
  spanset_find_value(ps, seq->period.lower, &loc);
  int nseqs = 0;
  for (int i = loc; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    TSequence *seq1 = tcontseq_at_period(seq, p);
    if (seq1 != NULL)
      result[nseqs++] = seq1;
    if (DatumGetTimestampTz(seq->period.upper) < DatumGetTimestampTz(p->upper))
      break;
  }
  return nseqs;
}

/**
 * @brief Restrict a temporal sequence to the complement of a period set
 * (iterator function).
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of elements in the output array
 * @note This function is called for each sequence of a temporal sequence set
 * @note To avoid roundoff errors in the loop we must use (1) compute the
 * complement of the period set and (2) compute the "at" function
 */
int
tcontseq_minus_periodset_iter(const TSequence *seq, const SpanSet *ps,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
    return tcontseq_minus_period_iter(seq, spanset_sp_n(ps, 0), result);

  /* The sequence can be split at most into (count + 1) sequences
   *    |----------------------|
   *        |---| |---| |---|
   */

  /* Compute the complement of the period set */
  SpanSet *ps1 = minus_span_spanset(&seq->period, ps);
  if (! ps1)
    return 0;
  int nseqs = 0;
  for (int i = 0; i < ps1->count; i++)
  {
    const Span *p1 = spanset_sp_n(ps1, i);
    result[nseqs++] = tcontseq_at_period(seq, p1);
  }
  pfree(ps1);
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) a period set.
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @sqlfunc atTime(), minusTime()
 */
TSequenceSet *
tcontseq_restrict_periodset(const TSequence *seq, const SpanSet *ps,
  bool atfunc)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->span))
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    if (contains_periodset_timestamp(ps, inst->t))
      return atfunc ? tsequence_to_tsequenceset(seq) : NULL;
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);
  }

  /* General case */
  int count = atfunc ? ps->count : ps->count + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int count1 = atfunc ? tcontseq_at_periodset1(seq, ps, sequences) :
    tcontseq_minus_periodset_iter(seq, ps, sequences);
  return tsequenceset_make_free(sequences, count1, NORMALIZE_NO);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Insert the second temporal value into the first one.
 */
Temporal *
tcontseq_insert(const TSequence *seq1, const TSequence *seq2)
{
  /* Order the two sequences */
  const TSequence *seq; /* for swaping */
  const TInstant *instants[2] = {0};
  instants[0] = TSEQUENCE_INST_N(seq1, seq1->count - 1);
  instants[1] = TSEQUENCE_INST_N(seq2, 0);
  if (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) > 0)
  {
    seq = seq1; seq1 = seq2; seq2 = seq;
    instants[0] = TSEQUENCE_INST_N(seq1, seq1->count - 1);
    instants[1] = TSEQUENCE_INST_N(seq2, 0);
  }

  /* Add the sequences in the array to merge */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);
  TSequence *tofree = NULL;
  const TSequence **sequences = palloc(sizeof(TSequence *) * 3);
  sequences[0] = seq1;
  int nseqs = 1;
  if (left_span_span(&seq1->period, &seq2->period))
  {
    if (seq1->period.upper_inc && seq2->period.lower_inc)
    {
      /* We put true so that it works with step interpolation */
      int count =
        (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) == 0) ? 1 : 2;
      tofree = tsequence_make(instants, count, true, true, interp,
        NORMALIZE_NO);
      sequences[nseqs++] = (const TSequence *) tofree;
   }
  }
  else /* overlap on the boundary*/
  {
    meosType basetype = temptype_basetype(seq1->temptype);
    if (! datum_eq(tinstant_value(instants[0]), tinstant_value(instants[1]),
      basetype))
    {
      char *str = pg_timestamptz_out(instants[0]->t);
      elog(ERROR, "The temporal values have different value at their common instant %s", str);
    }
  }
  sequences[nseqs++] = (TSequence *) seq2;

  int count;
  TSequence **newseqs = tsequence_merge_array1(sequences, nseqs, &count);
  Temporal *result;
  if (count == 1)
  {
    result = (Temporal *) newseqs[0];
    pfree(newseqs);
  }
  else
    /* Normalization was done at function tsequence_merge_array1 */
    result = (Temporal *) tsequenceset_make_free(newseqs, count, NORMALIZE_NO);
  if (tofree)
    pfree(tofree);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Delete a timestamp from a continuous temporal sequence.
 *
 * If an instant has the same timestamp, it will be removed. If the instant is
 * in the middle, it will be connected to the next and previous instants in the
 * result. If the instant is at the beginning or at the end, the time span of
 * the sequence is reduced. In this case the bounds of the sequence will be
 * adjusted accordingly, inclusive at the beginning and exclusive at the end.
 *
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 */
TSequence *
tcontseq_delete_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  bool lower_inc1 = seq->period.lower_inc;
  bool upper_inc1 = seq->period.upper_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (timestamptz_cmp_internal(inst->t, t) != 0)
      instants[ninsts++] = (TInstant *) inst;
    else /* inst->t == t */
    {
      if (i == 0)
        lower_inc1 = true;
      else if (i == seq->count - 1)
        upper_inc1 = false;
    }
  }
  if (ninsts == 0)
    return NULL;
  else if (ninsts == 1)
    lower_inc1 = upper_inc1 = true;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result = tsequence_make((const TInstant **) instants, ninsts,
    lower_inc1, upper_inc1, interp, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Delete a timestamp from a continuous temporal sequence.
 *
 * If an instant has the same timestamp, it will be removed. If the instant is
 * in the middle, it will be connected to the next and previous instants in the
 * result. If the instant is at the beginning or at the end, the time span of
 * the sequence is reduced. In this case the bounds of the sequence will be
 * adjusted accordingly, inclusive at the beginning and exclusive at the end.
 *
 * @param[in] seq Temporal sequence
 * @param[in] ts Timestamp set
 */
TSequence *
tcontseq_delete_timestampset(const TSequence *seq, const Set *ts)
{
  /* Singleton timestamp set */
  if (ts->count == 1)
    return tcontseq_delete_timestamp(seq,
      DatumGetTimestampTz(SET_VAL_N(ts, 0)));

  /* Bounding box test */
  Span p;
  set_set_span(ts, &p);
  if (! overlaps_span_span(&seq->period, &p))
    return tsequence_copy(seq);

  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (contains_set_value(ts, TimestampTzGetDatum(inst->t), T_TIMESTAMPTZ))
      return NULL;
    return tsequence_copy(seq);
  }

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  int i = 0,    /* current instant of the argument sequence */
    j = 0,      /* current timestamp of the argument timestamp set */
    ninsts = 0, /* number of instants in the currently constructed sequence */
    nfree = 0;  /* number of instants removed */
  bool lower_inc1 = seq->period.lower_inc;
  bool upper_inc1 = seq->period.upper_inc;
  while (i < seq->count && j < ts->count)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(ts, j));
    if (inst->t < t)
    {
      instants[ninsts++] = (TInstant *) inst;
      i++; /* advance instants */
    }
    else if (inst->t == t)
    {
      if (i == 0)
        lower_inc1 = true;
      else if (i == seq->count - 1)
        upper_inc1 = true;
      i++; /* advance instants */
      j++; /* advance timestamps */
      nfree++; /* advance number of instants removed */
      }
    else
    {
      /* inst->t > t */
      j++; /* advance timestamps */
    }
  }
  /* Compute the sequence after the timestamp set */
  if (i < seq->count)
  {
    for (j = i; j < seq->count; j++)
      instants[ninsts++] = (TInstant *) TSEQUENCE_INST_N(seq, j);
  }
  if (ninsts == 0)
    return NULL;
  else if (ninsts == 1)
    lower_inc1 = upper_inc1 = true;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result = tsequence_make((const TInstant **) instants, ninsts,
    lower_inc1, upper_inc1, interp, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Delete a period from a continuous temporal sequence.
 * @param[in] seq Temporal sequence
 * @param[in] p Period
 */
TSequence *
tcontseq_delete_period(const TSequence *seq, const Span *p)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, p))
    return tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  bool lower_inc1 = seq->period.lower_inc;
  bool upper_inc1 = seq->period.upper_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (! contains_period_timestamp(p, inst->t))
      instants[ninsts++] = (TInstant *) inst;
    else /* instant is inside the period */
    {
      if (i == 0)
        lower_inc1 = true;
      else if (i == seq->count - 1)
        upper_inc1 = false;
    }
  }
  if (ninsts == 0)
    return NULL;
  else if (ninsts == 1)
    lower_inc1 = upper_inc1 = true;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result = tsequence_make((const TInstant **) instants, ninsts,
    lower_inc1, upper_inc1, interp, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Delete a period set from a continuous temporal sequence.
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @sqlfunc deleteTime()
 */
TSequence *
tcontseq_delete_periodset(const TSequence *seq, const SpanSet *ps)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->span))
    return tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    if (contains_periodset_timestamp(ps, inst->t))
      return NULL;
    return tsequence_copy(seq);
  }

  /* Singleton period set */
  if (ps->count == 1)
    return tcontseq_delete_period(seq, spanset_sp_n(ps, 0));

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  bool lower_inc1 = seq->period.lower_inc;
  bool upper_inc1 = seq->period.upper_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (! contains_periodset_timestamp(ps, inst->t))
      instants[ninsts++] = (TInstant *) inst;
    else /* instant is inside the period set */
    {
      if (i == 0)
        lower_inc1 = true;
      else if (i == seq->count - 1)
        upper_inc1 = false;
    }
  }
  if (ninsts == 0)
    return NULL;
  else if (ninsts == 1)
    lower_inc1 = upper_inc1 = true;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *result = tsequence_make((const TInstant **) instants, ninsts,
    lower_inc1, upper_inc1, interp, NORMALIZE);
  pfree(instants);
  return result;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal sequence
 * number.
 */
double
tnumberseq_integral(const TSequence *seq)
{
  double result = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    if (MEOS_FLAGS_GET_LINEAR(seq->flags))
    {
      /* Linear interpolation */
      double min = Min(DatumGetFloat8(tinstant_value(inst1)),
        DatumGetFloat8(tinstant_value(inst2)));
      double max = Max(DatumGetFloat8(tinstant_value(inst1)),
        DatumGetFloat8(tinstant_value(inst2)));
      result += (max + min) * (double) (inst2->t - inst1->t) / 2.0;
    }
    else
    {
      /* Step interpolation */
      result += datum_double(tinstant_value(inst1),
        temptype_basetype(inst1->temptype)) * (double) (inst2->t - inst1->t);
    }
    inst1 = inst2;
  }
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the time-weighted average of a temporal discrete sequence number
 * @note Since a discrete sequence does not have duration, the function returns
 * the traditional average of the values
 * @sqlfunc twAvg()
 */
double
tnumberdiscseq_twavg(const TSequence *seq)
{
  meosType basetype = temptype_basetype(seq->temptype);
  double result = 0.0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    result += datum_double(tinstant_value(inst), basetype);
  }
  return result / seq->count;
}

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the time-weighted average of a temporal sequence number.
 * @sqlfunc twAvg()
 */
double
tnumbercontseq_twavg(const TSequence *seq)
{
  double duration = (double) (DatumGetTimestampTz(seq->period.upper) -
    DatumGetTimestampTz(seq->period.lower));
  double result;
  if (duration == 0.0)
    /* Instantaneous sequence */
    result = datum_double(tinstant_value(TSEQUENCE_INST_N(seq, 0)),
      temptype_basetype(seq->temptype));
  else
    result = tnumberseq_integral(seq) / duration;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the time-weighted average of a temporal sequence number
 * @sqlfunc twAvg()
 */
double
tnumberseq_twavg(const TSequence *seq)
{
  double result = MEOS_FLAGS_GET_DISCRETE(seq->flags) ?
      tnumberdiscseq_twavg(seq) : tnumbercontseq_twavg(seq);
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_comp
 * @brief Return true if two temporal sequences are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 */
bool
tsequence_eq(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1->temptype == seq2->temptype);
  /* If number of sequences, flags, or periods are not equal */
  if (seq1->count != seq2->count || seq1->flags != seq2->flags ||
      ! span_eq(&seq1->period, &seq2->period))
    return false;

  /* If bounding boxes are not equal */
  if (! temporal_bbox_eq(TSEQUENCE_BBOX_PTR(seq1), TSEQUENCE_BBOX_PTR(seq2),
      seq1->temptype))
    return false;

  /* Compare the composing instants */
  for (int i = 0; i < seq1->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, i);
    if (! tinstant_eq(inst1, inst2))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal sequence
 * is less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function temporal_cmp
 * @sqlfunc tbool_cmp(), tint_cmp(), tfloat_cmp(), ttext_cmp(), etc.
 */
int
tsequence_cmp(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1->temptype == seq2->temptype);

  /* Compare composing instants */
  int count = Min(seq1->count, seq2->count);
  for (int i = 0; i < count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, i);
    int result = tinstant_cmp(inst1, inst2);
    if (result)
      return result;
  }

  /* seq1->count == seq2->count because of the bounding box and the
   * composing instant tests above */

  /* Compare flags  */
  if (seq1->flags < seq2->flags)
    return -1;
  if (seq1->flags > seq2->flags)
    return 1;

  /* The two values are equal */
  return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements and the approach for span types for combining the period
 * bounds.
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal sequence.
 * @sqlfunc tbool_hash(), tint_hash(), tfloat_hash(), ttext_hash(), etc.
 */
uint32
tsequence_hash(const TSequence *seq)
{
  /* Create flags from the lower_inc and upper_inc values */
  char flags = '\0';
  if (seq->period.lower_inc)
    flags |= 0x01;
  if (seq->period.upper_inc)
    flags |= 0x02;
  uint32 result = hash_bytes_uint32((uint32) flags);

  /* Merge with hash of instants */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    uint32 inst_hash = tinstant_hash(inst);
    result = (result << 5) - result + inst_hash;
  }
  return result;
}

/*****************************************************************************/
