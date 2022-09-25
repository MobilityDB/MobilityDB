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
 * @brief General functions for temporal sequences.
 */

#include "general/tsequence.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/periodset.h"
#include "general/pg_call.h"
#include "general/temporaltypes.h"
#include "general/temporal_boxops.h"
#include "general/temporal_parser.h"
#include "general/timestampset.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif


/*****************************************************************************
 * Collinear functions
 * Are the three temporal instants collinear?
 * These functions suppose that the segments are not constant.
 *****************************************************************************/

/**
 * Return true if the three values are collinear
 *
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
float_collinear(double x1, double x2, double x3, double ratio)
{
  double x = x1 + (x3 - x1) * ratio;
  return (fabs(x2 - x) <= MOBDB_EPSILON);
}

/**
 * Return true if the three double2 values are collinear
 *
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
  bool result = (fabs(x2->a - x.a) <= MOBDB_EPSILON &&
    fabs(x2->b - x.b) <= MOBDB_EPSILON);
  return result;
}

/**
 * Return true if the three values are collinear
 *
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
  bool result = (fabs(x2->a - x.a) <= MOBDB_EPSILON &&
    fabs(x2->b - x.b) <= MOBDB_EPSILON && fabs(x2->c - x.c) <= MOBDB_EPSILON);
  return result;
}

/**
 * Return true if the three values are collinear
 *
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
  bool result = (fabs(x2->a - x.a) <= MOBDB_EPSILON &&
    fabs(x2->b - x.b) <= MOBDB_EPSILON && fabs(x2->c - x.c) <= MOBDB_EPSILON &&
    fabs(x2->d - x.d) <= MOBDB_EPSILON);
  return result;
}

#if NPOINT
/**
 * Return true if the three values are collinear
 *
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
 * Return true if the three values are collinear
 *
 * @param[in] basetype Base type
 * @param[in] value1,value2,value3 Input values
 * @param[in] t1,t2,t3 Input timestamps
 */
static bool
datum_collinear(mobdbType basetype, Datum value1, Datum value2, Datum value3,
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
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
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
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Set the second argument to the bounding box of a temporal sequence
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

/**
 * Return a pointer to the offsets array of a temporal sequence
 */
size_t *
tsequence_offsets_ptr(const TSequence *seq)
{
  return (size_t *)(((char *)seq) + double_pad(sizeof(TSequence)) +
    ((seq->bboxsize == 0) ? 0 : double_pad(seq->bboxsize - sizeof(Period))));
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the n-th instant of a temporal sequence.
 * @pre The argument @p index is less than the number of instants in the
 * sequence
 */
const TInstant *
tsequence_inst_n(const TSequence *seq, int index)
{
  return (TInstant *)(
    /* start of data */
    ((char *)seq) + double_pad(sizeof(TSequence)) +
      ((seq->bboxsize == 0) ? 0 : (seq->bboxsize - sizeof(Period))) +
      seq->maxcount * sizeof(size_t) +
      /* offset */
      (tsequence_offsets_ptr(seq))[index]);
}

/**
 * Ensure the validity of the arguments when creating a temporal sequence
 */
void
tsequence_make_valid1(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp)
{
  /* Test the validity of the instants */
  assert(count > 0);
  ensure_tinstarr(instants, count);
  if (count == 1 && (!lower_inc || !upper_inc))
    elog(ERROR, "Instant sequence must have inclusive bounds");
  mobdbType basetype = temptype_basetype(instants[0]->temptype);
  if (interp == STEPWISE && count > 1 && ! upper_inc &&
    datum_ne(tinstant_value(instants[count - 1]),
      tinstant_value(instants[count - 2]), basetype))
    elog(ERROR, "Invalid end value for temporal sequence with stepwise interpolation");
  return;
}

/**
 * Ensure the validity of the arguments when creating a temporal sequence
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
 * Normalize the array of temporal instants
 *
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
  mobdbType basetype = temptype_basetype(instants[0]->temptype);
  TInstant **result = palloc(sizeof(TInstant *) * count);
  /* Remove redundant instants */
  TInstant *inst1 = (TInstant *) instants[0];
  Datum value1 = tinstant_value(inst1);
  TInstant *inst2 = (TInstant *) instants[1];
  Datum value2 = tinstant_value(inst2);
  result[0] = inst1;
  int k = 1;
  for (int i = 2; i < count; i++)
  {
    TInstant *inst3 = (TInstant *) instants[i];
    Datum value3 = tinstant_value(inst3);
    bool v1v2eq = datum_eq(value1, value2, basetype);
    bool v2v3eq = datum_eq(value2, value3, basetype);
    if (
      /* step sequences and 2 consecutive instants that have the same value
        ... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
      */
      (interp == STEPWISE && v1v2eq)
      ||
      /* 3 consecutive linear instants that have the same value
        ... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
      */
      (interp == LINEAR && v1v2eq && v2v3eq)
      ||
      /* collinear linear instants
        ... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
      */
      (interp == LINEAR && datum_collinear(basetype, value1, value2, value3,
        inst1->t, inst2->t, inst3->t))
      )
    {
      inst2 = inst3; value2 = value3;
    }
    else
    {
      result[k++] = inst2;
      inst1 = inst2; value1 = value2;
      inst2 = inst3; value2 = value3;
    }
  }
  result[k++] = inst2;
  *newcount = k;
  return result;
}

/**
 * Construct a temporal sequence from an array of temporal instants
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
tsequence_make1(const TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  assert(maxcount >= count);

  /* Normalize the array of instants */
  TInstant **norminsts = (TInstant **) instants;
  int newcount = count;
  if (interp != DISCRETE && normalize && count > 1)
    norminsts = tinstarr_normalize(instants, interp, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = double_pad(temporal_bbox_size(instants[0]->temptype));
  /* The period component of the bbox is already declared in the struct */
  size_t bboxsize_extra = (bboxsize == 0) ? 0 : bboxsize - sizeof(Period);

  /* Compute the size of the temporal sequence */
  size_t insts_size = 0;
  /* Size of composing instants */
  for (int i = 0; i < newcount; i++)
    insts_size += double_pad(VARSIZE(norminsts[i]));
  /* Compute the total size for maxcount instants as a proportion of the size
   * of the count instants provided. Note that this is only an initial
   * estimation. The functions adding instants to a sequence must verify both
   * the maximum number of instants and the remaining space for adding an
   * additional variable-length instant or arbitrary size */
  int totalcount;
  if (count != maxcount)
  {
    insts_size *= maxcount / count;
    totalcount = maxcount;
  }
  else
    totalcount = newcount;
  /* Total size of the struct, the offset array */
  size_t memsize = double_pad(sizeof(TSequence)) + bboxsize_extra +
    totalcount * sizeof(size_t) + insts_size;

  /* Create the temporal sequence */
  TSequence *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->maxcount = totalcount;
  result->temptype = instants[0]->temptype;
  result->subtype = TSEQUENCE;
  result->bboxsize = bboxsize;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags,
    MOBDB_FLAGS_GET_CONTINUOUS(norminsts[0]->flags));
  MOBDB_FLAGS_SET_INTERP(result->flags, interp);
  MOBDB_FLAGS_SET_X(result->flags, true);
  MOBDB_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(instants[0]->temptype))
  {
    MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(instants[0]->flags));
    MOBDB_FLAGS_SET_GEODETIC(result->flags,
      MOBDB_FLAGS_GET_GEODETIC(instants[0]->flags));
  }
  /* Initialization of the variable-length part */
  /*
   * Compute the bounding box
   * Only external types have bounding box, internal types such
   * as double2, double3, or double4 do not have bounding box but
   * require to set the period attribute
   */
  if (bboxsize != 0)
  {
    tsequence_compute_bbox((const TInstant **) norminsts, newcount, lower_inc,
      upper_inc, interp, TSEQUENCE_BBOX_PTR(result));
  }
  else
  {
    span_set(TimestampTzGetDatum(norminsts[0]->t),
      TimestampTzGetDatum(norminsts[newcount - 1]->t), lower_inc, upper_inc,
      T_TIMESTAMPTZ, &result->period);
  }
  /* Store the composing instants */
  size_t pdata = double_pad(sizeof(TSequence)) + bboxsize_extra +
    totalcount * sizeof(size_t);
  size_t pos = 0;
  for (int i = 0; i < newcount; i++)
  {
    memcpy(((char *) result) + pdata + pos, norminsts[i],
      VARSIZE(norminsts[i]));
    (tsequence_offsets_ptr(result))[i] = pos;
    pos += double_pad(VARSIZE(norminsts[i]));
  }
  if (interp != DISCRETE && normalize && count > 1)
    pfree(norminsts);
  return result;
}

/**
 * Join two temporal sequences
 *
 * @param[in] seq1,seq2 Temporal sequences
 * @param[in] removelast,removefirst Remove the last and/or the
 * first instant of the first/second sequence
 * @pre The two input sequences are adjacent and have the same interpolation
 * @note The function is called when normalizing an array of sequences
 * and thus, all validity tests have been already made
 */
static TSequence *
tsequence_join(const TSequence *seq1, const TSequence *seq2,
  bool removelast, bool removefirst)
{
  int count1 = removelast ? seq1->count - 1 : seq1->count;
  int start2 = removefirst ? 1 : 0;
  int count = count1 + (seq2->count - start2);
  const TInstant **instants = palloc(sizeof(TSequence *) * count);
  int i, k = 0;
  for (i = 0; i < count1; i++)
    instants[k++] = tsequence_inst_n(seq1, i);
  for (i = start2; i < seq2->count; i++)
    instants[k++] = tsequence_inst_n(seq2, i);
  TSequence *result = tsequence_make1(instants, count, count,
    seq1->period.lower_inc, seq2->period.upper_inc,
    MOBDB_FLAGS_GET_INTERP(seq1->flags), NORMALIZE_NO);
  pfree(instants);
  return result;
}

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
    const TInstant *inst1 = tsequence_inst_n(seq, middle);
    const TInstant *inst2 = tsequence_inst_n(seq, middle + 1);
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
    const TInstant *inst = tsequence_inst_n(seq, middle);
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
 * Convert an array of arrays of temporal sequences into an array of
 * temporal sequences.
 *
 * This function is called by all the functions in which the number of
 * output sequences is not bounded, typically when each segment of the
 * input sequence can produce an arbitrary number of output sequences,
 * as in the case of atGeometry.
 *
 * @param[in] sequences Array of array of temporal sequences
 * @param[in] countseqs Array of counters
 * @param[in] count Number of elements in the first dimension of the arrays
 * @param[in] totalseqs Number of elements in the output array
 * @pre count and totalseqs are greater than 0
 */
TSequence **
tseqarr2_to_tseqarr(TSequence ***sequences, int *countseqs,
  int count, int totalseqs)
{
  assert(count > 0);
  assert(totalseqs > 0);
  TSequence **result = palloc(sizeof(TSequence *) * totalseqs);
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < countseqs[i]; j++)
      result[k++] = sequences[i][j];
    if (countseqs[i] != 0)
      pfree(sequences[i]);
  }
  pfree(sequences); pfree(countseqs);
  return result;
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Append an instant to a temporal sequence.
 * @param[in,out] seq Temporal sequence
 * @param[in] inst Temporal instant
 * @sqlfunc appendInstant()
 */
Temporal *
tsequence_append_tinstant(TSequence *seq, const TInstant *inst, bool expand)
{
  /* Ensure validity of the arguments */
  assert(seq->temptype == inst->temptype);
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  mobdbType basetype = temptype_basetype(seq->temptype);
  const TInstant *inst1 = tsequence_inst_n(seq, seq->count - 1);
#if NPOINT
  if (inst1->temptype == T_TNPOINT && interp != DISCRETE)
    ensure_same_rid_tnpointinst(inst, inst1);
#endif

  /* Notice that we cannot call ensure_increasing_timestamps since we must
   * take into account the inclusive/exclusive bounds */
  if (inst1->t > inst->t)
  {
    char *t1 = pg_timestamptz_out(inst1->t);
    char *t2 = pg_timestamptz_out(inst->t);
    elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s", t1, t2);
  }

  bool eqvalue = datum_eq(tinstant_value(inst1), tinstant_value(inst),
    basetype);
  if (inst1->t == inst->t)
  {
    if (seq->period.upper_inc)
    {
      if (! eqvalue)
      {
        char *t1 = pg_timestamptz_out(inst1->t);
        elog(ERROR, "The temporal values have different value at their common timestamp %s", t1);
      }
      /* Do not add the new instant if sequence is discrete and new instant is
       * equal to be last one */
      else if (interp == DISCRETE)
        return (Temporal *) tsequence_copy(seq);
    }
    /* Exclusive upper bound and different value => result is a sequence set */
    else if (interp == LINEAR && ! eqvalue)
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

  /* The result is a sequence */
  int count = seq->count + 1;
  /* Normalize the result */
  if (interp != DISCRETE && seq->count > 1)
  {
    inst1 = tsequence_inst_n(seq, seq->count - 2);
    Datum value1 = tinstant_value(inst1);
    const TInstant *inst2 = tsequence_inst_n(seq, seq->count - 1);
    Datum value2 = tinstant_value(inst2);
    Datum value3 = tinstant_value(inst);
    bool v1v2eq = datum_eq(value1, value2, basetype);
    bool v2v3eq = datum_eq(value2, value3, basetype);
    if (
      /* step sequences and 2 consecutive instants that have the same value
        ... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
      */
      (interp == STEPWISE && v1v2eq)
      ||
      /* 3 consecutive float/point instants that have the same value
        ... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
      */
      (interp == LINEAR && v1v2eq && v2v3eq)
      ||
      /* collinear float/point instants that have the same duration
        ... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
      */
      (interp == LINEAR && datum_collinear(basetype, value1, value2, value3,
        inst1->t, inst2->t, inst->t))
      )
    {
      /* The new instant replaces the last instant of the sequence */
      count--;
    }
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * count);
  int k = 0;
  for (int i = 0; i < count - 1; i++)
    instants[k++] = tsequence_inst_n(seq, i);
  instants[k++] = inst;
  int maxcount = count;
  if (expand)
    maxcount *= 2;
  TSequence *result = tsequence_make1(instants, count, maxcount,
    seq->period.lower_inc, true, interp, NORMALIZE_NO);
  pfree(instants);
  return (Temporal *) result;
}

/**
 * @ingroup libmeos_int_temporal_transf
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
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge an array of temporal discrete sequences.
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
Temporal *
tdiscseq_merge_array(const TSequence **sequences, int count)
{
  /* Validity test will be done in tinstant_merge_array */
  /* Collect the composing instants */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += sequences[i]->count;
  const TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < sequences[i]->count; j++)
      instants[k++] = tsequence_inst_n(sequences[i], j);
  }
  /* Create the result */
  Temporal *result = tinstant_merge_array(instants, totalcount);
  pfree(instants);
  return result;
}

/**
 * Normalize the array of temporal sequences
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
  mobdbType basetype = temptype_basetype(seq1->temptype);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool isnew = false;
  int k = 0;
  for (int i = 1; i < count; i++)
  {
    TSequence *seq2 = (TSequence *) sequences[i];
    TInstant *last2 = (seq1->count == 1) ? NULL :
      (TInstant *) tsequence_inst_n(seq1, seq1->count - 2);
    Datum last2value = (seq1->count == 1) ? 0 :
      tinstant_value(last2);
    TInstant *last1 = (TInstant *) tsequence_inst_n(seq1, seq1->count - 1);
    Datum last1value = tinstant_value(last1);
    TInstant *first1 = (TInstant *) tsequence_inst_n(seq2, 0);
    Datum first1value = tinstant_value(first1);
    TInstant *first2 = (seq2->count == 1) ? NULL :
      (TInstant *) tsequence_inst_n(seq2, 1);
    Datum first2value = (seq2->count == 1) ? 0 :
      tinstant_value(first2);
    bool adjacent = seq1->period.upper == seq2->period.lower &&
      (seq1->period.upper_inc || seq2->period.lower_inc);
    /* If they are adjacent and not instantaneous */
    if (adjacent && last2 != NULL && first2 != NULL &&
      (
      /* If step and the last segment of the first sequence is constant
         ..., 1@t1, 1@t2) [1@t2, 1@t3, ... -> ..., 1@t1, 2@t3, ...
         ..., 1@t1, 1@t2) [1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ...
         ..., 1@t1, 1@t2] (1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ...
       */
      (! linear &&
      datum_eq(last2value, last1value, basetype) &&
      datum_eq(last1value, first1value, basetype))
      ||
      /* If the last/first segments are constant and equal
         ..., 1@t1, 1@t2] (1@t2, 1@t3, ... -> ..., 1@t1, 1@t3, ...
       */
      (datum_eq(last2value, last1value, basetype) &&
      datum_eq(last1value, first1value, basetype) &&
      datum_eq(first1value, first2value, basetype))
      ||
      /* If float/point sequences and collinear last/first segments having the same duration
         ..., 1@t1, 2@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 3@t3, ...
      */
      (temptype_continuous(seq1->temptype) &&
        datum_eq(last1value, first1value, basetype) &&
        datum_collinear(basetype, last2value, first1value, first2value,
          last2->t, first1->t, first2->t))
      ))
    {
      /* Remove the last and first instants of the sequences */
      seq1 = tsequence_join(seq1, seq2, true, true);
      isnew = true;
    }
    /* If step sequences and the first one has an exclusive upper bound,
       by definition the first sequence has the last segment constant
       ..., 1@t1, 1@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 2@t2, 3@t3, ...
       ..., 1@t1, 1@t2) [2@t2] -> ..., 1@t1, 2@t2]
     */
    else if (adjacent && ! linear && ! seq1->period.upper_inc)
    {
      /* Remove the last instant of the first sequence */
      seq1 = tsequence_join(seq1, seq2, true, false);
      isnew = true;
    }
    /* If they are adjacent and have equal last/first value respectively
      Stewise
      ... 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
      [1@t1], (1@t1, 2@t2, ... -> ..., 1@t1, 2@t2
      Linear
      ..., 1@t1, 2@t2), [2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
      ..., 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
      ..., 1@t1, 2@t2), [2@t2] -> ..., 1@t1, 2@t2]
      [1@t1],(1@t1, 2@t2, ... -> [1@t1, 2@t2, ...
    */
    else if (adjacent && datum_eq(last1value, first1value, basetype))
    {
      /* Remove the first instant of the second sequence */
      seq1 = tsequence_join(seq1, seq2, false, true);
      isnew = true;
    }
    else
    {
      result[k++] = isnew ? seq1 : tsequence_copy(seq1);
      seq1 = seq2;
      isnew = false;
    }
  }
  result[k++] = isnew ? seq1 : tsequence_copy(seq1);
  *newcount = k;
  return result;
}

/**
 * Merge an array of temporal sequences.
 * The values in the array may overlap on a single instant.
 *
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @param[out] totalcount Number of elements in the resulting array
 * @result Array of merged sequences
 */
TSequence **
tsequence_merge_array1(const TSequence **sequences, int count, int *totalcount)
{
  if (count > 1)
    tseqarr_sort((TSequence **) sequences, count);
  /* Test the validity of the composing sequences */
  const TSequence *seq1 = sequences[0];
  mobdbType basetype = temptype_basetype(seq1->temptype);
  for (int i = 1; i < count; i++)
  {
    const TInstant *inst1 = tsequence_inst_n(seq1, seq1->count - 1);
    const TSequence *seq2 = sequences[i];
    const TInstant *inst2 = tsequence_inst_n(seq2, 0);
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
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge an array of temporal sequences.
 *
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @note The values in the array may overlap on a single instant.
 * @sqlfunc merge()
 */
Temporal *
tsequence_merge_array(const TSequence **sequences, int count)
{
  /* Discrete sequences */
  if (MOBDB_FLAGS_GET_DISCRETE(sequences[0]->flags))
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
 * Synchronization functions
 *****************************************************************************/

/**
 * Synchronize two temporal sequences
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
  Period inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return false;

  interpType interp1 = MOBDB_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MOBDB_FLAGS_GET_INTERP(seq2->flags);
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
  inst1 = (TInstant *) tsequence_inst_n(seq1, 0);
  inst2 = (TInstant *) tsequence_inst_n(seq2, 0);
  int i = 0, j = 0, k = 0, l = 0;
  if (inst1->t < (TimestampTz) inter.lower)
  {
    i = tcontseq_find_timestamp(seq1, inter.lower) + 1;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
  }
  else if (inst2->t < (TimestampTz) inter.lower)
  {
    j = tcontseq_find_timestamp(seq2, inter.lower) + 1;
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count * 2);
  mobdbType basetype1 = temptype_basetype(seq1->temptype);
  mobdbType basetype2 = temptype_basetype(seq2->temptype);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= (TimestampTz) inter.upper ||
     inst2->t <= (TimestampTz) inter.upper))
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
      tofree[l++] = inst2;
    }
    else
    {
      j++;
      inst1 = tsequence_at_timestamp(seq1, inst2->t);
      tofree[l++] = inst1;
    }
    /* If not the first instant add potential crossing before adding
       the new instants */
    if (crossings && (interp1 == LINEAR || interp2 == LINEAR) && k > 0)
    {
      TimestampTz crosstime;
      Datum inter1, inter2;
      if (tsegment_intersection(instants1[k - 1], inst1, (interp1 == LINEAR),
        instants2[k - 1], inst2, (interp2 == LINEAR), &inter1, &inter2, &crosstime))
      {
        instants1[k] = tofree[l++] = tinstant_make(inter1, seq1->temptype,
          crosstime);
        instants2[k++] = tofree[l++] = tinstant_make(inter2, seq2->temptype,
          crosstime);
      }
    }
    instants1[k] = inst1; instants2[k++] = inst2;
    if (i == seq1->count || j == seq2->count)
      break;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  /* We are sure that k != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and
     exclusive upper bound must be equal */
  if (! inter.upper_inc && k > 1 && (interp1 != LINEAR) &&
      datum_ne(tinstant_value(instants1[k - 2]),
        tinstant_value(instants1[k - 1]), basetype1))
  {
    instants1[k - 1] = tinstant_make(tinstant_value(instants1[k - 2]),
      instants1[k - 1]->temptype, instants1[k - 1]->t);
    tofree[l++] = instants1[k - 1];
  }
  if (! inter.upper_inc && k > 1 && (interp2 != LINEAR) &&
      datum_ne(tinstant_value(instants2[k - 2]),
        tinstant_value(instants2[k - 1]), basetype2))
  {
    instants2[k - 1] = tinstant_make(tinstant_value(instants2[k - 2]),
      instants2[k - 1]->temptype, instants2[k - 1]->t);
    tofree[l++] = instants2[k - 1];
  }
  *sync1 = tsequence_make((const TInstant **) instants1, k, k, inter.lower_inc,
    inter.upper_inc, interp1, NORMALIZE_NO);
  *sync2 = tsequence_make((const TInstant **) instants2, k, k, inter.lower_inc,
    inter.upper_inc, interp2, NORMALIZE_NO);

  pfree_array((void **) tofree, l);
  pfree(instants1); pfree(instants2);

  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * Temporally intersect two temporal discrete sequences
 *
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
    *inter1 = tsequence_make(instants1, k, k, true, true, DISCRETE, NORMALIZE_NO);
    *inter2 = tsequence_make(instants2, k, k, true, true, DISCRETE, NORMALIZE_NO);
  }

  pfree(instants1); pfree(instants2);
  return k != 0;
}

/**
 * Temporally intersect two temporal sequences
 *
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
  int k = 0;
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq2, i);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      instants1[k] = tsequence_at_timestamp(seq1, inst->t);
      instants2[k++] = inst;
    }
    if ((TimestampTz) seq1->period.upper < inst->t)
      break;
  }
  if (k == 0)
  {
    pfree(instants1); pfree(instants2);
    return false;
  }

  *inter1 = tsequence_make_free(instants1, k, k, true, true, DISCRETE, MERGE_NO);
  *inter2 = tsequence_make(instants2, k, k, true, true, DISCRETE, MERGE_NO);
  pfree(instants2);
  return true;
}

/**
 * Temporally intersect two temporal values
 *
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

/*****************************************************************************/

/**
 * Compute the intersection, if any, of a segment of a temporal sequence and
 * a value. The functions only return true when there is an intersection at
 * the middle of the segment, i.e., they return false if they intersect at a
 * bound. When they return true, they also return in the output parameter
 * the intersection timestampt t. The value taken by the segment and the
 * target value are equal up to the floating point precision.
 * There is no need to add functions for DoubleN, which are used for computing
 * avg and centroid aggregates, since these computations are based on sum and
 * thus they do not need to add intermediate points.
 */

/**
 * Return true if the segment of a temporal number intersects
 * the base value at a timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[in] basetype Base type
 * @param[out] t Timestamp
 */
static bool
tfloatsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, mobdbType basetype, TimestampTz *t)
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
  if (fraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < fraction)
    return false;

  if (t != NULL)
  {
    double duration = (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/**
 * Return true if the segment of a temporal sequence intersects
 * the base value at the timestamp
 *
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
  Datum value, mobdbType basetype, Datum *inter, TimestampTz *t)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  if (datum_eq2(value, value1, basetype, temptype_basetype(inst1->temptype)) ||
      datum_eq2(value, value2, basetype, temptype_basetype(inst2->temptype)))
    return false;

  ensure_temptype_continuous(inst1->temptype);
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
 * Return true if two segments of two temporal numbers intersect at a timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
static bool
tnumbersegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1 = tnumberinst_double(start1);
  double x2 = tnumberinst_double(end1);
  double x3 = tnumberinst_double(start2);
  double x4 = tnumberinst_double(end2);
  /* Compute the instant t at which the linear functions of the two segments
     are equal: at + b = ct + d that is t = (d - b) / (a - c).
     To reduce problems related to floating point precision, t1 and t2
     are shifted, respectively, to 0 and 1 before the computation */
  long double denum = x2 - x1 - x4 + x3;
  if (denum == 0)
    /* Parallel segments */
    return false;

  long double fraction = ((long double) (x3 - x1)) / denum;
  if (fraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < fraction )
    /* Intersection occurs out of the period */
    return false;

  double duration = (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * Return true if  two segments of a temporal sequence intersect at a
 * timestamp
 *
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
  mobdbType basetype1 = temptype_basetype(start1->temptype);
  mobdbType basetype2 = temptype_basetype(start2->temptype);
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
    ensure_temporal_type(start1->temptype);
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
 * Temporally intersect two temporal sequences
 *
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
 * Temporally intersect two temporal values
 *
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
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence from its Well-Known Text (WKT) representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_in(const char *str, mobdbType temptype, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, temptype);
  else
    return tcontseq_parse(&str, temptype, interp, true, true);
}
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence boolean from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tboolseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TBOOL);
  else
    return tcontseq_parse(&str, T_TBOOL, STEPWISE, true, true);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence integer from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tintseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TINT);
  else
    return tcontseq_parse(&str, T_TINT, STEPWISE, true, true);
}

/**
 * @ingroup libmeos_int_temporal_in_out
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
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence text from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
ttextseq_in(const char *str, interpType interp)
{
  if (interp == DISCRETE)
    return tdiscseq_parse(&str, T_TTEXT);
  else
    return tcontseq_parse(&str, T_TTEXT, STEPWISE, true, true);
}

/**
 * @ingroup libmeos_int_temporal_in_out
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
 * @ingroup libmeos_int_temporal_in_out
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
 * @brief Return the Well-Known Text (WKT) representation of a temporal sequence.
 *
 * @param[in] seq Temporal sequence
 * @param[in] arg Maximum number of decimal digits to output for floating point
 * values
 * @param[in] component True if the output string is a component of a
 * temporal sequence set and thus no interpolation string at the begining of
 * the string should be output
 * @param[in] value_out Function called to output the base value
 */
char *
tsequence_to_string(const TSequence *seq, Datum arg, bool component,
  char *(*value_out)(mobdbType, Datum, Datum))
{
  char **strings = palloc(sizeof(char *) * seq->count);
  size_t outlen = 0;
  char prefix[20];
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  if (! component && MOBDB_FLAGS_GET_CONTINUOUS(seq->flags) &&
      interp == STEPWISE)
    sprintf(prefix, "Interp=Stepwise;");
  else
    prefix[0] = '\0';
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    strings[i] = tinstant_to_string(inst, arg, value_out);
    outlen += strlen(strings[i]) + 2;
  }
  char open, close;
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
  {
    open = (char) '{';
    close = (char) '}';
  }
  else
  {
    open = seq->period.lower_inc ? (char) '[' : (char) '(';
    close = seq->period.upper_inc ? (char) ']' : (char) ')';
  }
  return stringarr_to_string(strings, seq->count, outlen, prefix,
    open, close);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return the Well-Known Text (WKT) representation of a temporal sequence.
 */
char *
tsequence_out(const TSequence *seq, Datum arg)
{
  return tsequence_to_string(seq, arg, false, &basetype_output);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence from an array of temporal instants.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tsequence_make(const TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  tsequence_make_valid(instants, count, lower_inc, upper_inc, interp);
  return tsequence_make1(instants, count, maxcount, lower_inc, upper_inc,
    interp, normalize);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence from an array of temporal instants
 * and free the array and the instants after the creation.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @see tsequence_make
 */
TSequence *
tsequence_make_free(TInstant **instants, int count, int maxcount,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  if (count == 0)
  {
    pfree(instants);
    return NULL;
  }
  TSequence *result = tsequence_make((const TInstant **) instants, count,
    maxcount, lower_inc, upper_inc, interp, normalize);
  pfree_array((void **) instants, count);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence from arrays of coordinates, one per
 * dimension, and timestamps.
 *
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
  return tsequence_make_free(instants, count, count, lower_inc, upper_inc,
    interp, normalize);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_constructor
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
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq(),
 * etc.
 */
TSequence *
tdiscseq_from_base(Datum value, mobdbType temptype, const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype, tsequence_inst_n(seq, i)->t);
  return tsequence_make_free(instants, seq->count, seq->count, true, true,
    DISCRETE, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
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
  return tsequence_make_free(instants, ts->count, ts->count, true, true,
    DISCRETE, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean discrete sequence from a boolean and a
 * timestamp set.
 */
TSequence *
tbooldiscseq_from_base_time(bool b, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(BoolGetDatum(b), T_TBOOL, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer discrete sequence from an integer and a
 * timestamp set.
 */
TSequence *
tintdiscseq_from_base_time(int i, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(Int32GetDatum(i), T_TINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float discrete sequence from a float and a
 * timestamp set.
 */
TSequence *
tfloatdiscseq_from_base_time(bool b, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(BoolGetDatum(b), T_TFLOAT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text discrete sequence from a text and a
 * timestamp set.
 */
TSequence *
ttextdiscseq_from_base_time(const text *txt, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(txt), T_TTEXT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point discrete sequence from a point
 * and a timestamp set.
 */
TSequence *
tgeompointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(gs), T_TGEOMPOINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point discrete sequence from a point
 * and a timestamp set.
 */
TSequence *
tgeogpointdiscseq_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tdiscseq_from_base_time(PointerGetDatum(gs), T_TGEOGPOINT, ts);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal sequence from a base value and the time frame
 * of another temporal sequence.
 *
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] seq Temporal value
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_from_base(Datum value, mobdbType temptype, const TSequence *seq,
  interpType interp)
{
  return MOBDB_FLAGS_GET_DISCRETE(seq->flags) ?
    tdiscseq_from_base(value, temptype, seq) :
    tsequence_from_base_time(value, temptype,
      &seq->period, interp);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean sequence from a boolean and the time
 * frame of another temporal sequence.
 */
TSequence *
tboolseq_from_base(bool b, const TSequence *seq)
{
  return tsequence_from_base(BoolGetDatum(b), T_TBOOL, seq, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer sequence from an integer and the time
 * frame of another temporal sequence.
 */
TSequence *
tintseq_from_base(int i, const TSequence *seq)
{
  return tsequence_from_base(Int32GetDatum(i), T_TINT, seq, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float sequence from a float and the time frame
 * of another temporal sequence.
 */
TSequence *
tfloatseq_from_base(bool b, const TSequence *seq, interpType interp)
{
  return tsequence_from_base(BoolGetDatum(b), T_TFLOAT, seq, interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text sequence from a text and the time frame
 * of another temporal sequence.
 */
TSequence *
ttextseq_from_base(const text *txt, const TSequence *seq)
{
  return tsequence_from_base(PointerGetDatum(txt), T_TTEXT, seq, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point sequence from a point and the
 * time frame of another temporal sequence.
 */
TSequence *
tgeompointseq_from_base(const GSERIALIZED *gs, const TSequence *seq,
  interpType interp)
{
  return tsequence_from_base(PointerGetDatum(gs), T_TGEOMPOINT, seq, interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point sequence from a point and the
 * time frame of another temporal sequence.
 */
TSequence *
tgeogpointseq_from_base(const GSERIALIZED *gs, const TSequence *seq,
  interpType interp)
{
  return tsequence_from_base(PointerGetDatum(gs), T_TGEOGPOINT, seq, interp);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal sequence from a base value and a period.
 *
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] p Period
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_from_base_time(Datum value, mobdbType temptype, const Period *p,
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
    count, p->lower_inc, p->upper_inc, interp, NORMALIZE_NO);
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
tboolseq_from_base_time(bool b, const Period *p)
{
  return tsequence_from_base_time(BoolGetDatum(b), T_TBOOL, p, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer sequence from an integer and a period.
 */
TSequence *
tintseq_from_base_time(int i, const Period *p)
{
  return tsequence_from_base_time(Int32GetDatum(i), T_TINT, p, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float sequence from a float and a period.
 */
TSequence *
tfloatseq_from_base_time(bool b, const Period *p, interpType interp)
{
  return tsequence_from_base_time(BoolGetDatum(b), T_TFLOAT, p, interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text sequence from a text and a period.
 */
TSequence *
ttextseq_from_base_time(const text *txt, const Period *p)
{
  return tsequence_from_base_time(PointerGetDatum(txt), T_TTEXT, p, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point sequence from a point and a
 * period.
 */
TSequence *
tgeompointseq_from_base_time(const GSERIALIZED *gs, const Period *p,
  interpType interp)
{
  return tsequence_from_base_time(PointerGetDatum(gs), T_TGEOMPOINT, p, interp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point sequence from a point and a
 * period.
 */
TSequence *
tgeogpointseq_from_base_time(const GSERIALIZED *gs, const Period *p,
  interpType interp)
{
  return tsequence_from_base_time(PointerGetDatum(gs), T_TGEOGPOINT, p, interp);
}
#endif /* MEOS */

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Cast a temporal sequence integer to a temporal sequence float.
 * @sqlop @p ::
 */
TSequence *
tintseq_to_tfloatseq(const TSequence *seq)
{
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TFLOAT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, true);
  MOBDB_FLAGS_SET_INTERP(result->flags, MOBDB_FLAGS_GET_INTERP(result->flags));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) tsequence_inst_n(result, i);
    inst->temptype = T_TFLOAT;
    inst->value = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Cast a temporal sequence float to a temporal sequence integer.
 * @sqlop @p ::
 */
TSequence *
tfloatseq_to_tintseq(const TSequence *seq)
{
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
    elog(ERROR, "Cannot cast temporal float with linear interpolation to temporal integer");
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TINT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, false);
  MOBDB_FLAGS_SET_INTERP(result->flags, MOBDB_FLAGS_GET_INTERP(result->flags));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) tsequence_inst_n(result, i);
    inst->temptype = T_TINT;
    inst->value = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a copy of a temporal sequence with no additional free space.
 */
TSequence *
tsequence_compact(const TSequence *seq)
{
  /* Return a copy of the input sequence is there is not extra space */
  if (seq->count == seq->maxcount)
    return tsequence_copy(seq);

  /* Compute the new total size of the sequence and allocate memory for it */
  size_t bboxsize_extra = (seq->bboxsize == 0) ? 0 :
    seq->bboxsize - sizeof(Period);
  /* Size of composing instants */
  size_t insts_size = 0;
  for (int i = 0; i < seq->count; i++)
    insts_size += double_pad(VARSIZE(tsequence_inst_n(seq, i)));
  size_t seqsize = double_pad(sizeof(TSequence)) + bboxsize_extra +
    seq->count * sizeof(size_t) + insts_size;
  TSequence *result = palloc0(seqsize);

  /* Copy until the end of the offsets array */
  memcpy(result, seq, seqsize - insts_size);
  /* Set the maxcount */
  result->maxcount = result->count;
  /* Copy until the end of the offsets array */
  memcpy((char *) tsequence_inst_n(result, 0), tsequence_inst_n(seq, 0),
    insts_size);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal instant transformed into a temporal sequence.
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tinstant_to_tsequence(const TInstant *inst, interpType interp)
{
  return tsequence_make(&inst, 1, 1, true, true, interp, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence transformed from discrete interpolation to
 * linear or stepwise interpolation.
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tdiscseq_to_tsequence(const TSequence *seq, interpType interp)
{
  if (seq->count != 1)
    elog(ERROR, "Cannot transform input value to a temporal sequence");
  return tinstant_to_tsequence(tsequence_inst_n(seq, 0), interp);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence transformed into discrete interpolation.
 * @return Return an error if a temporal sequence has more than one instant
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq(),
 * etc.
 */
TSequence *
tsequence_to_tdiscseq(const TSequence *seq)
{
  /* If the sequence has discrete interpolation return a copy */
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return tsequence_copy(seq);

  /* General case */
  if (seq->count != 1)
    elog(ERROR, "Cannot transform input to a temporal discrete sequence");

  const TInstant *inst = tsequence_inst_n(seq, 0);
  return tinstant_to_tsequence(inst, DISCRETE);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal sequence
 * value.
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
TSequence *
tsequenceset_to_tsequence(const TSequenceSet *ss)
{
  if (ss->count != 1)
    elog(ERROR, "Cannot transform input to a temporal sequence");
  return tsequence_copy(tsequenceset_seq_n(ss, 0));
}

/**
 * Return a temporal sequence with continuous base type transformed from
 * stepwise to linear interpolation
 *
 * @param[in] seq Temporal sequence
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 */
int
tstepseq_tlinearseq1(const TSequence *seq, TSequence **result)
{
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    MOBDB_FLAGS_SET_INTERP(result[0]->flags, LINEAR);
    return 1;
  }

  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  const TInstant *inst2 = NULL; /* keep compiler quiet */
  Datum value2;
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = tsequence_inst_n(seq, i);
    value2 = tinstant_value(inst2);
    TInstant *instants[2];
    instants[0] = (TInstant *) inst1;
    instants[1] = tinstant_make(value1, seq->temptype, inst2->t);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc &&
      datum_eq(value1, value2, basetype) : false;
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inc, upper_inc, LINEAR, NORMALIZE_NO);
    inst1 = inst2;
    value1 = value2;
    lower_inc = true;
    pfree(instants[1]);
  }
  if (seq->period.upper_inc)
  {
    value1 = tinstant_value(tsequence_inst_n(seq, seq->count - 2));
    value2 = tinstant_value(inst2);
    if (datum_ne(value1, value2, basetype))
      result[k++] = tinstant_to_tsequence(inst2, LINEAR);
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence with continuous base type transformed from
 * stepwise to linear interpolation.
 *
 * @param[in] seq Temporal sequence
 * @return Resulting temporal sequence set
 * @sqlfunc toLinear()
 */
TSequenceSet *
tsequence_step_to_linear(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tstepseq_tlinearseq1(seq, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_int_temporal_transf
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
  period_shift_tscale(shift, duration, &result->period);
  TimestampTz delta;
  if (shift != NULL)
    delta = result->period.lower - seq->period.lower;
  double scale;
  bool instant = (result->period.lower == result->period.upper);
  /* If the sequence set is instantaneous we cannot scale */
  if (duration != NULL && ! instant)
  {
    scale = (double) (result->period.upper - result->period.lower) /
      (double) (seq->period.upper - seq->period.lower);
  }

  /* Set the first instant */
  TInstant *inst = (TInstant *) tsequence_inst_n(result, 0);
  inst->t = result->period.lower;
  if (seq->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    for (int i = 1; i < seq->count - 1; i++)
    {
      inst = (TInstant *) tsequence_inst_n(result, i);
      if (shift != NULL)
        inst->t += delta;
      if (duration != NULL && ! instant)
        inst->t = result->period.lower +
          (inst->t - result->period.lower) * scale;
    }
    /* Set the last instant */
    inst = (TInstant *) tsequence_inst_n(result, seq->count - 1);
    inst->t = result->period.upper;
  }
  /* Shift and/or scale bounding box */
  void *bbox = TSEQUENCE_BBOX_PTR(result);
  temporal_bbox_shift_tscale(shift, duration, seq->temptype, bbox);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of base values of a temporal sequence with stepwise
 * interpolation.
 *
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
    result[i] = tinstant_value(tsequence_inst_n(seq, i));
  if (seq->count > 1)
  {
    mobdbType basetype = temptype_basetype(seq->temptype);
    datumarr_sort(result, seq->count, basetype);
    *count = datumarr_remove_duplicates(result, seq->count, basetype);
  }
  else
    *count = 1;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the float span of a temporal sequence float.
 * @sqlop @p ::
 */
Span *
tfloatseq_span(const TSequence *seq)
{
  TBOX *box = TSEQUENCE_BBOX_PTR(seq);
  Datum min = box->span.lower;
  Datum max = box->span.upper;
  /* It step interpolation or equal bounding box bounds */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) ||
      box->span.lower == box->span.upper)
    return span_make(min, max, true, true, T_FLOAT8);

  /* For sequences with linear interpolation the bounds of the span are
   * those in the bounding box but we need to determine whether the bounds
   * are inclusive or not */
  Datum start = tinstant_value(tsequence_inst_n(seq, 0));
  Datum end = tinstant_value(tsequence_inst_n(seq, seq->count - 1));
  Datum lower, upper;
  bool lower_inc, upper_inc;
  if (datum_lt(start, end, T_FLOAT8))
  {
    lower = start; lower_inc = seq->period.lower_inc;
    upper = end; upper_inc = seq->period.upper_inc;
  }
  else
  {
    lower = end; lower_inc = seq->period.upper_inc;
    upper = start; upper_inc = seq->period.lower_inc;
  }
  bool min_inc = datum_lt(box->span.lower, lower, T_FLOAT8) ||
    (box->span.lower == lower && lower_inc);
  bool max_inc = datum_gt(box->span.upper, upper, T_FLOAT8) ||
    (box->span.upper == upper && upper_inc);
  /* Determine whether the minimum and/or maximum appear inside the sequence */
  if (! min_inc || ! max_inc)
  {
    for (int i = 1; i < seq->count - 1; i++)
    {
      Datum value = tinstant_value(tsequence_inst_n(seq, i));
      min_inc |= (box->span.lower == value);
      max_inc |= (box->span.upper == value);
      if (min_inc && max_inc)
        break;
    }
  }
  return span_make(min, max, min_inc, max_inc, T_FLOAT8);
}

/**
 * Return the float spans of a temporal float
 *
 * @param[in] seq Temporal sequence
 * @param[out] result Array on which the pointers of the newly constructed
 * spans are stored
 * @result Number of spans in the result
 */
int
tfloatseq_spans1(const TSequence *seq, Span **result)
{
  /* Temporal float with linear interpolation */
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    result[0] = tfloatseq_span(seq);
    return 1;
  }

  /* Temporal float with discrete or step interpolation */
  int count;
  Datum *values = tsequence_values(seq, &count);
  for (int i = 0; i < count; i++)
    result[i] = span_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  return count;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of spans of base values of a temporal float sequence.
 *
 * For temporal floats with linear interpolation the result will be a
 * singleton, which is the result of @ref tfloatseq_span. Otherwise, the
 * result will be an array of spans, one for each distinct value.
 * @sqlfunc getValues()
 */
Span **
tfloatseq_spans(const TSequence *seq, int *count)
{
  int count1 = MOBDB_FLAGS_GET_LINEAR(seq->flags) ? 1 : seq->count;
  Span **result = palloc(sizeof(Span *) * count1);
  *count = tfloatseq_spans1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the time frame of a temporal sequence as a period set.
 * @sqlfunc getTime()
 */
PeriodSet *
tsequence_time(const TSequence *seq)
{
  /* Continuous sequence */
  if (! MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return period_to_periodset(&seq->period);

  /* Discrete sequence */
  Period **periods = palloc(sizeof(Period *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    periods[i] = span_make(inst->t, inst->t, true, true, T_TIMESTAMPTZ);
  }
  PeriodSet *result = periodset_make_free(periods, seq->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
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
  Datum min = tinstant_value(tsequence_inst_n(seq, 0));
  int k = 0;
  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_lt(value, min, basetype))
    {
      min = value;
      k = i;
    }
  }
  return tsequence_inst_n(seq, k);
}

/**
 * @ingroup libmeos_int_temporal_accessor
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
  Datum max = tinstant_value(tsequence_inst_n(seq, 0));
  int k = 0;
  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_gt(value, max, basetype))
    {
      max = value;
      k = i;
    }
  }
  return tsequence_inst_n(seq, k);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the minimum base value of a temporal sequence.
 * @sqlfunc minValue()
 */
Datum
tsequence_min_value(const TSequence *seq)
{
  if (seq->temptype == T_TINT || seq->temptype == T_TFLOAT)
  {
    TBOX *box = TSEQUENCE_BBOX_PTR(seq);
    Datum min = box->span.lower;
    if (seq->temptype == T_TINT)
      min = Int32GetDatum((int) DatumGetFloat8(min));
    return min;
  }

  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum result = tinstant_value(tsequence_inst_n(seq, 0));
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the maximum base value of a temporal sequence.
 * @sqlfunc maxValue()
 */
Datum
tsequence_max_value(const TSequence *seq)
{
  if (seq->temptype == T_TINT || seq->temptype == T_TFLOAT)
  {
    TBOX *box = TSEQUENCE_BBOX_PTR(seq);
    Datum max = box->span.upper;
    /* The upper bound for integer spans is exclusive due to canonicalization */
    if (seq->temptype == T_TINT)
      max = Int32GetDatum((int) DatumGetFloat8(max));
    return max;
  }

  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum result = tinstant_value(tsequence_inst_n(seq, 0));
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the duration of a temporal sequence.
 * @sqlfunc duration()
 */
Interval *
tsequence_duration(const TSequence *seq)
{
  return period_duration(&seq->period);
}

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Return the bounding period of a temporal sequence
 * @sqlfunc period()
 * @sqlop @p ::
 */
void
tsequence_set_period(const TSequence *seq, Period *p)
{
  memcpy(p, &seq->period, sizeof(Span));
  return;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the singleton array of sequences of a temporal sequence.
 * @post The output parameter @p count is equal to 1
 * @sqlfunc sequences()
 */
TSequence **
tsequence_sequences(const TSequence *seq, int *count)
{
  TSequence **result;
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
  {
    /* Discrete sequence */
    result = palloc(sizeof(TSequence *) * seq->count);
    interpType interp = MOBDB_FLAGS_GET_CONTINUOUS(seq->flags) ? LINEAR : STEPWISE;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = tsequence_inst_n(seq, i);
      result[i] = tinstant_to_tsequence(inst, interp);
    }
    *count = seq->count;
  }
  else
  {
    /* Continuous sequence */
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
  }
  return result;
}

/**
 * Return the array of segments of a temporal sequence
 */
int
tsequence_segments1(const TSequence *seq, TSequence **result)
{
  assert(! MOBDB_FLAGS_GET_DISCRETE(seq->flags));
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  TInstant *instants[2];
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  bool lower_inc = seq->period.lower_inc;
  TInstant *inst1, *inst2;
  int k = 0;
  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst1 = (TInstant *) tsequence_inst_n(seq, i - 1);
    inst2 = (TInstant *) tsequence_inst_n(seq, i);
    instants[0] = inst1;
    instants[1] = (interp == LINEAR) ? inst2 :
      tinstant_make(tinstant_value(inst1), seq->temptype, inst2->t);
    bool upper_inc;
    if (i == seq->count - 1 && (interp == LINEAR ||
      datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype)))
      upper_inc = seq->period.upper_inc;
    else
      upper_inc = false;
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inc, upper_inc, interp, NORMALIZE_NO);
    if (interp != LINEAR)
      pfree(instants[1]);
    lower_inc = true;
  }
  if (interp != LINEAR && seq->period.upper)
  {
    inst1 = (TInstant *) tsequence_inst_n(seq, seq->count - 1);
    inst2 = (TInstant *) tsequence_inst_n(seq, seq->count - 2);
    if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype))
      result[k++] = tinstant_to_tsequence(inst1, interp);
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of segments of a temporal sequence.
 * @sqlfunc segments()
 */
TSequence **
tsequence_segments(const TSequence *seq, int *count)
{
  /* Discrete sequence */
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return tsequence_sequences(seq, count);

  /* Continuous sequence */
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);
  *count = tsequence_segments1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of distinct instants of a temporal sequence.
 * @note By definition, all instants of a sequence are distinct
 * @post The output parameter @p count is equal to the number of instants of the
 * input temporal sequence
 * @sqlfunc instants()
 */
const TInstant **
tsequence_instants(const TSequence *seq, int *count)
{
  const TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = tsequence_inst_n(seq, i);
  *count = seq->count;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the start timestamp of a temporal sequence.
 * @sqlfunc startTimestamp()
 */
TimestampTz
tsequence_start_timestamp(const TSequence *seq)
{
  return (tsequence_inst_n(seq, 0))->t;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the end timestamp of a temporal sequence.
 * @sqlfunc endTimestamp()
 */
TimestampTz
tsequence_end_timestamp(const TSequence *seq)
{
  return (tsequence_inst_n(seq, seq->count - 1))->t;
}

/**
 * Return the array of timestamps of a temporal sequence
 */
int
tsequence_timestamps1(const TSequence *seq, TimestampTz *times)
{
  for (int i = 0; i < seq->count; i++)
    times[i] = tsequence_inst_n(seq, i)->t;
  return seq->count;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of timestamps of a temporal sequence.
 *
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
  tsequence_timestamps1(seq, result);
  *count = seq->count;
  return result;
}

/**
 * Return the base value of the segment of a temporal sequence at a
 * timestamp
 *
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
  if (datum_eq(value1, value2,  temptype_basetype(inst1->temptype)) ||
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
  ensure_temptype_continuous(inst1->temptype);
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
  if (inst1->temptype == T_TGEOMPOINT || inst1->temptype == T_TGEOGPOINT)
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
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the base value of a temporal sequence at a timestamp.
 *
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
    const TInstant *inst = tsequence_inst_n(seq, 0);
    /* Instantaneous sequence or t is at lower bound */
    if (seq->count == 1 || inst->t == t)
      return tinstant_value_at_timestamp(inst, t, result);
    inst = tsequence_inst_n(seq, seq->count - 1);
    if (inst->t == t)
      return tinstant_value_at_timestamp(inst, t, result);
  }

  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return false;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    *result = tinstant_value_copy(tsequence_inst_n(seq, 0));
    return true;
  }

  /* General case */
  int n = tcontseq_find_timestamp(seq, t);
  const TInstant *inst1 = tsequence_inst_n(seq, n);
  if (t == inst1->t)
    *result = tinstant_value_copy(inst1);
  else
  {
    const TInstant *inst2 = tsequence_inst_n(seq, n + 1);
    bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
    *result = tsegment_value_at_timestamp(inst1, inst2, linear, t);
  }
  return true;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence is ever equal to a base value.
 * @sqlop @p ?=
 */
bool
tsequence_ever_eq(const TSequence *seq, Datum value)
{
  int i;
  Datum value1;
  mobdbType basetype = temptype_basetype(seq->temptype);

  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) seq, value, EVER))
    return false;

  /* Stepwise interpolation or instantaneous sequence */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(tsequence_inst_n(seq, i));
      if (datum_eq(value1, value, basetype))
        return true;
    }
    return false;
  }

  /* Linear interpolation*/
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  value1 = tinstant_value(inst1);
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
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
 * @ingroup libmeos_int_temporal_ever
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

  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_ne(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * Return true if the segment of a temporal sequence with linear
 * interpolation is ever less than or equal to a base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_ever_le1(Datum value1, Datum value2, mobdbType basetype,
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
 * Return true if the segment of a temporal sequence with linear
 * interpolation is always less than a base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_always_lt1(Datum value1, Datum value2, mobdbType basetype,
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
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence is ever less than a base value.
 * @sqlop @p ?<
 */
bool
tsequence_ever_lt(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, EVER))
    return false;

  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_lt(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
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
  mobdbType basetype = temptype_basetype(seq->temptype);

  /* Stepwise interpolation or instantaneous sequence */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(tsequence_inst_n(seq, i));
      if (datum_le(value1, value, basetype))
        return true;
    }
    return false;
  }

  /* Linear interpolation */
  value1 = tinstant_value(tsequence_inst_n(seq, 0));
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    Datum value2 = tinstant_value(tsequence_inst_n(seq, i));
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (tlinearseq_ever_le1(value1, value2, basetype,
      lower_inc, upper_inc, value))
      return true;
    value1 = value2;
    lower_inc = true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
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
  mobdbType basetype = temptype_basetype(seq->temptype);

  /* Stepwise interpolation or instantaneous sequence */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(tsequence_inst_n(seq, i));
      if (! datum_lt(value1, value, basetype))
        return false;
    }
    return true;
  }

  /* Linear interpolation */
  value1 = tinstant_value(tsequence_inst_n(seq, 0));
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    Datum value2 = tinstant_value(tsequence_inst_n(seq, i));
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (! tlinearseq_always_lt1(value1, value2, basetype,
      lower_inc, upper_inc, value))
      return false;
    value1 = value2;
    lower_inc = true;
  }
  return true;
}

/**
 * @ingroup libmeos_int_temporal_ever
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
  assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
  mobdbType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(tsequence_inst_n(seq, i));
    if (! datum_le(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a base
 * value.
 *
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
  mobdbType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    Datum value1 = tinstant_value(tsequence_inst_n(seq, 0));
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
    const TInstant *inst = tsequence_inst_n(seq, i);
    bool equal = datum_eq(value, tinstant_value(inst), basetype);
    if ((atfunc && equal) || (! atfunc && ! equal))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) an array
 * of base values.
 *
 * @param[in] seq Temporal sequence
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 * @sqlfunc atValues(), minusValues()
 */
TSequence *
tdiscseq_restrict_values(const TSequence *seq, const Datum *values,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, newcount, true, true, DISCRETE,
      NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * Restrict a segment of a temporal sequence to (the complement of) a base
 * value.
 *
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
  mobdbType basetype = temptype_basetype(inst1->temptype);
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
  Datum projvalue;
  TimestampTz t;
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
    result[0] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inc && lower, upper_inc && upper, interp, NORMALIZE_NO);
    return 1;
  }

  /* Stepwise interpolation */
  if (interp == STEPWISE)
  {
    int k = 0;
    if (lower)
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
        lower_inc, false, STEPWISE, NORMALIZE_NO);
      pfree(instants[1]);
    }
    if (upper_inc && upper)
      result[k++] = tinstant_to_tsequence(inst2, STEPWISE);
    return k;
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
      result[0] = tsequence_make((const TInstant **) instants, 2, 2,
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
      result[0] = tsequence_make((const TInstant **) instants, 2, 2,
        lower_inc, ! upper_inc, LINEAR, NORMALIZE_NO);
      return 1;
    }
    else
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(projvalue, inst1->temptype, t);
      result[0] = tsequence_make((const TInstant **) instants, 2, 2,
        lower_inc, false, LINEAR, NORMALIZE_NO);
      instants[0] = instants[1];
      instants[1] = (TInstant *) inst2;
      result[1] = tsequence_make((const TInstant **) instants, 2, 2,
        false, upper_inc, LINEAR, NORMALIZE_NO);
      pfree(instants[0]);
      DATUM_FREE(projvalue, basetype);
      return 2;
    }
  }
}

/**
 * Restrict a temporal sequence to (the complement of) a base value
 *
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
tcontseq_restrict_value1(const TSequence *seq, Datum value, bool atfunc,
  TSequence **result)
{
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
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
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Each iteration adds between 0 and 2 sequences */
    k += tsegment_restrict_value(inst1, inst2, interp, lower_inc, upper_inc,
      value, atfunc, &result[k]);
    inst1 = inst2;
    lower_inc = true;
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) a base value.
 *
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
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tcontseq_restrict_value1(seq, value, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * Restrict a temporal sequence to an array of base values
 *
 * @param[in] seq Temporal sequence
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre There are no duplicates values in the array
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_at_values1(const TSequence *seq, const Datum *values, int count,
  TSequence **result)
{
  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    TInstant *inst = tinstant_restrict_values(inst1, values, count, REST_AT);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* Bounding box test */
  int count1;
  Datum *values1 = temporal_bbox_restrict_values((Temporal *) seq, values,
    count, &count1);
  if (count1 == 0)
    return 0;

  /* General case */
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = tsequence_inst_n(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    for (int j = 0; j < count1; j++)
      /* Each iteration adds between 0 and 2 sequences */
      k += tsegment_restrict_value(inst1, inst2, interp, lower_inc,
        upper_inc, values1[j], REST_AT, &result[k]);
    inst1 = inst2;
    lower_inc = true;
  }
  if (k > 1)
    tseqarr_sort(result, k);

  pfree(values1);
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) an array of base
 * values.
 *
 * @param[in] seq Temporal sequence
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal sequence set.
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function tsequence_at_values1 since the latter is called
 * for each composing sequence of a temporal sequence set number.
 * @sqlfunc atValues(), minusValues()
 */
TSequenceSet *
tcontseq_restrict_values(const TSequence *seq, const Datum *values, int count,
  bool atfunc)
{
  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count * 2);
  int newcount = tsequence_at_values1(seq, values, count, sequences);
  TSequenceSet *atresult = tsequenceset_make_free(sequences, newcount, NORMALIZE);
  if (atfunc)
    return atresult;

  /*
   * MINUS function
   * Compute the complement of the previous value.
   */
  if (newcount == 0)
    return tsequence_to_tsequenceset(seq);

  PeriodSet *ps1 = tsequenceset_time(atresult);
  PeriodSet *ps2 = minus_period_periodset(&seq->period, ps1);
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete number sequence to (the complement of) a
 * span of base values.
 *
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
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (tnumberinst_restrict_span_test(inst, span, atfunc))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence number to (the complement of) an
 * array of spans of base values.
 *
 * @param[in] seq Temporal number
 * @param[in] normspans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre The array of spans is normalized
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpans(), minusSpans()
 */
TSequence *
tnumberdiscseq_restrict_spans(const TSequence *seq, Span **normspans,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, newcount, true, true, DISCRETE,
      NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * Restrict a segment of a temporal number to (the complement of) a span
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] lower_inclu,upper_inclu Upper and lower bounds of the segment
 * @param[in] linear True if the segment has linear interpolation
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 * @return Resulting temporal sequence
 */
static int
tnumbersegm_restrict_span(const TInstant *inst1, const TInstant *inst2,
  bool linear, bool lower_inclu, bool upper_inclu, const Span *span,
  bool atfunc, TSequence **result)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  mobdbType basetype = temptype_basetype(inst1->temptype);
  interpType interp = linear ? LINEAR : STEPWISE;
  TInstant *instants[2];
  bool contains;

  /* Constant segment (step or linear interpolation) */
  if (datum_eq(value1, value2, basetype))
  {
    contains = contains_span_elem(span, value1, basetype);
    if ((atfunc && ! contains) || (! atfunc && contains))
      return 0;
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inclu, upper_inclu, interp, NORMALIZE_NO);
    return 1;
  }

  /* Stepwise interpolation */
  if (! linear)
  {
    int k = 0;
    contains = contains_span_elem(span, value1, basetype);
    if ((atfunc && contains) || (! atfunc && ! contains))
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, inst2->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
        lower_inclu, false, interp, NORMALIZE_NO);
      pfree(instants[1]);
    }
    contains = contains_span_elem(span, value2, basetype);
    if (upper_inclu &&
      ((atfunc && contains) || (! atfunc && ! contains)))
    {
      result[k++] = tinstant_to_tsequence(inst2, interp);
    }
    return k;
  }

  /* Linear interpolation */
  bool lower_inc1, upper_inc1;
  bool increasing = DatumGetFloat8(value1) < DatumGetFloat8(value2);
  Span valuespan;
  if (increasing)
    span_set(value1, value2, lower_inclu, upper_inclu, basetype, &valuespan);
  else
    span_set(value2, value1, upper_inclu, lower_inclu, basetype, &valuespan);
  Span inter;
  bool found = inter_span_span(&valuespan, span, &inter);
  if (! found)
  {
    if (atfunc)
      return 0;
    /* MINUS */
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inclu, upper_inclu, interp, NORMALIZE_NO);
    return 1;
  }

  /* We are sure that neither lower or upper are infinite */
  Datum lower = inter.lower;
  Datum upper = inter.upper;
  bool lower_inc2 = inter.lower_inc;
  bool upper_inc2 = inter.upper_inc;
  double dlower = DatumGetFloat8(lower);
  double dupper = DatumGetFloat8(upper);
  double dvalue1 = DatumGetFloat8(value1);
  double dvalue2 = DatumGetFloat8(value2);
  TimestampTz t1, t2;
  /* Intersection span is a single value */
  if (dlower == dupper)
  {
    if (atfunc)
    {
      t1 = (dlower == dvalue1) ? inst1->t : inst2->t;
      instants[0] = tinstant_make(lower, inst1->temptype, t1);
      result[0] = tinstant_to_tsequence(instants[0], interp);
      pfree(instants[0]);
      return 1;
    }
    /* MINUS */
    if (dvalue1 == dlower)
    {
      lower_inc1 = ! lower_inclu;
      upper_inc1 = upper_inclu;
    }
    else
    {
      lower_inc1 = lower_inclu;
      upper_inc1 = ! upper_inclu;
    }
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inc1, upper_inc1, interp, NORMALIZE_NO);
    return 1;
  }

  /* Prepare the result depending on whether the segment is increasing
   * or decreasing */
  int i, j;
  if (increasing)
  {
    i = 0; j = 1;
    lower_inc1 = atfunc ? lower_inc2 : ! lower_inc2;
    upper_inc1 = atfunc ? upper_inc2 : ! upper_inc2;
  }
  else
  {
    i = 1; j = 0;
    lower_inc1 = atfunc ? upper_inc2 : ! upper_inc2;
    upper_inc1 = atfunc ? lower_inc2 : ! lower_inc2;
  }

  if (atfunc)
  {
    /* Find lower and upper bound of intersection */
    bool freei = false, freej = false;
    if (dvalue1 == dlower)
      instants[i] = (TInstant *) inst1;
    else if (dvalue2 == dlower)
      instants[i] = (TInstant *) inst2;
    else
    {
      freei = tfloatsegm_intersection_value(inst1, inst2, lower, basetype, &t1);
      /* To reduce the roundoff errors we may take the bound instead of
       * projecting the value to the timestamp */
      instants[i] = SPAN_ROUNDOFF ?
        tinstant_make(lower, inst1->temptype, t1) :
        tsegment_at_timestamp(inst1, inst2, linear, t1);
    }

    if (dvalue1 == dupper)
      instants[j] = (TInstant *) inst1;
    else if (dvalue2 == dupper)
      instants[j] = (TInstant *) inst2;
    else
    {
      freej = tfloatsegm_intersection_value(inst1, inst2, upper, basetype, &t2);
      /* To reduce the roundoff errors we may take the bound instead of
       * projecting the value to the timestamp */
      instants[j] = SPAN_ROUNDOFF ?
        tinstant_make(upper, inst1->temptype, t2) :
        tsegment_at_timestamp(inst1, inst2, linear, t2);
    }

    /* Create the result */
    result[0] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inc1, upper_inc1, interp, NORMALIZE_NO);
    if (freei)
      pfree(instants[i]);
    if (freej)
      pfree(instants[j]);
    return 1;
  }

  /* MINUS */
  /* Find lower and upper bound of intersection */
  int k = 0;
  TInstant *instbounds[2] = {NULL, NULL};
  if (dlower != dvalue1 && dlower != dvalue2)
  {
    tfloatsegm_intersection_value(inst1, inst2, lower, basetype, &t1);
    /* To reduce the roundoff errors we may take the bound instead of
     * projecting the value to the timestamp */
    instbounds[i] = SPAN_ROUNDOFF ?
      tinstant_make(lower, inst1->temptype, t1) :
      tsegment_at_timestamp(inst1, inst2, linear, t1);
  }
  if (dupper != dvalue1 && dupper != dvalue2)
  {
    tfloatsegm_intersection_value(inst1, inst2, upper, basetype, &t2);
    /* To reduce the roundoff errors we may take the bound instead of
     * projecting the value to the timestamp */
    instbounds[j] = SPAN_ROUNDOFF ?
      tinstant_make(upper, inst1->temptype, t2) :
      tsegment_at_timestamp(inst1, inst2, linear, t2);
  }

  /* Create the result */
  if (instbounds[0] == NULL && instbounds[1] == NULL)
  {
    if (lower_inclu && lower_inc1)
      result[k++] = tinstant_to_tsequence(inst1, interp);
    if (upper_inclu && upper_inc1)
      result[k++] = tinstant_to_tsequence(inst2, interp);
  }
  else if (instbounds[0] != NULL && instbounds[1] != NULL)
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = instbounds[0];
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inclu, lower_inc1, interp, NORMALIZE_NO);
    instants[0] = instbounds[1];
    instants[1] = (TInstant *) inst2;
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      upper_inc1, upper_inclu, interp, NORMALIZE_NO);
  }
  else if (instbounds[0] != NULL)
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = instbounds[0];
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      lower_inclu, lower_inc1, interp, NORMALIZE_NO);
    if (upper_inclu && upper_inc1)
      result[k++] = tinstant_to_tsequence(inst2, interp);
  }
  else /* if (instbounds[1] != NULL) */
  {
    if (lower_inclu && lower_inc1)
      result[k++] = tinstant_to_tsequence(inst1, interp);
    instants[0] = instbounds[1];
    instants[1] = (TInstant *) inst2;
    result[k++] = tsequence_make((const TInstant **) instants, 2, 2,
      upper_inc1, upper_inclu, interp, NORMALIZE_NO);
  }

  for (i = 0; i < 2; i++)
  {
    if (instbounds[i])
      pfree(instbounds[i]);
  }
  return k;
}

/**
 * @brief Restrict a temporal number to (the complement of) a span
 *
 * @param[in] seq temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_span2(const TSequence *seq, const Span *span,
  bool atfunc, TSequence **result)
{
  /* Bounding box test */
  TBOX box1, box2;
  tsequence_set_bbox(seq, &box1);
  span_set_tbox(span, &box2);
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
    inst1 = tsequence_inst_n(seq, 0);
    TInstant *inst = tnumberinst_restrict_span(inst1, span, atfunc);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = tsequence_inst_n(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    k += tnumbersegm_restrict_span(inst1, inst2, linear, lower_inc, upper_inc,
      span, atfunc, &result[k]);
    inst1 = inst2;
    lower_inc = true;
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence number to (the complement of) a span.
 *
 * @param[in] seq Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal number
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
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tnumbercontseq_restrict_span2(seq, span, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * Restrict a temporal number to (the complement of) an array of spans
 * of base values
 *
 * @param[in] seq Temporal number
 * @param[in] normspans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[in] bboxtest True if the bounding box test should be performed
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @pre The array of spans is normalized
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumbercontseq_restrict_spans1(const TSequence *seq, Span **normspans,
  int count, bool atfunc, bool bboxtest, TSequence **result)
{
  Span **newspans;
  int newcount;

  /* Bounding box test */
  if (bboxtest)
  {
    newspans = tnumber_bbox_restrict_spans((Temporal *) seq, normspans,
      count, &newcount);
    if (newcount == 0)
    {
      if (atfunc)
        return 0;
      else
      {
        result[0] = tsequence_copy(seq);
        return 1;
      }
    }
  }
  else
  {
    newspans = normspans;
    newcount = count;
  }

  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    TInstant *inst = tnumberinst_restrict_spans(inst1, newspans, newcount,
      atfunc);
    if (bboxtest)
      pfree(newspans);
    if (inst == NULL)
      return 0;
    pfree(inst);
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  if (atfunc)
  {
    /* AT function */
    inst1 = tsequence_inst_n(seq, 0);
    bool lower_inc = seq->period.lower_inc;
    int k = 0;
    for (int i = 1; i < seq->count; i++)
    {
      inst2 = tsequence_inst_n(seq, i);
      bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
      for (int j = 0; j < newcount; j++)
      {
        k += tnumbersegm_restrict_span(inst1, inst2, linear, lower_inc,
          upper_inc, newspans[j], REST_AT, &result[k]);
      }
      inst1 = inst2;
      lower_inc = true;
    }
    if (bboxtest)
      pfree(newspans);
    if (k > 1)
      tseqarr_sort(result, k);
    return k;
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
    TSequenceSet *ss = tnumbercontseq_restrict_spans(seq, newspans, newcount,
      REST_AT, bboxtest);
    if (ss == NULL)
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }

    PeriodSet *ps1 = tsequenceset_time(ss);
    PeriodSet *ps2 = minus_period_periodset(&seq->period, ps1);
    int newcount = 0;
    if (ps2 != NULL)
    {
      newcount = tcontseq_at_periodset1(seq, ps2, result);
      pfree(ps2);
    }
    pfree(ss); pfree(ps1);
    if (bboxtest)
      pfree(newspans);
    return newcount;
  }
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal number to (the complement of) an array of spans.
 *
 * @param[in] seq Temporal number
 * @param[in] normspans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True if the restriction is at, false for minus
 * @param[in] bboxtest True if the bounding box test should be performed
 * @return Resulting temporal number
 * @pre The array of spans is normalized
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function @ref tnumbercontseq_restrict_spans1 since the latter is called
 * for each composing sequence of a temporal sequence set number.
 * @sqlfunc atSpans(), minusSpans()
 */
TSequenceSet *
tnumbercontseq_restrict_spans(const TSequence *seq, Span **normspans,
  int count, bool atfunc, bool bboxtest)
{
  /* General case */
  int maxcount = seq->count * count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int newcount = tnumbercontseq_restrict_spans1(seq, normspans, count, atfunc,
    bboxtest, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) its
 * minimum/maximum base value
 *
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) its
 * minimum/maximum base value.
 *
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
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the base value of a temporal discrete sequence at a timestamp
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

  const TInstant *inst = tsequence_inst_n(seq, loc);
  *result = tinstant_value_copy(inst);
  return true;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc atTimestamp(), minusTimestamp()
 */
TInstant *
tdiscseq_at_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return NULL;

  /* Instantenous sequence */
  if (seq->count == 1)
    return tinstant_copy(tsequence_inst_n(seq, 0));

  /* General case */
  const TInstant *inst;
  int loc = tdiscseq_find_timestamp(seq, t);
  if (loc < 0)
    return NULL;
  inst = tsequence_inst_n(seq, loc);
  return tinstant_copy(inst);
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc atTimestamp(), minusTimestamp()
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
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (inst->t != t)
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a timestamp set.
 * @sqlfunc atTimestampSet(), minusTimestampSet()
 */
TSequence *
tdiscseq_restrict_timestampset(const TSequence *seq, const TimestampSet *ts,
  bool atfunc)
{
  TSequence *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    Temporal *temp = atfunc ?
      (Temporal *) tdiscseq_at_timestamp(seq, timestampset_time_n(ts, 0)) :
      (Temporal *) tdiscseq_minus_timestamp(seq, timestampset_time_n(ts, 0));
    if (temp == NULL || ! atfunc)
      return (TSequence *) temp;
    /* Transform the result of tdiscseq_at_timestamp into a sequence */
    result = tinstant_to_tsequence((const TInstant *) temp, DISCRETE);
    pfree(temp);
    return result;
  }

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ts->period))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, k = 0;
  while (i < seq->count && j < ts->count)
  {
    inst = tsequence_inst_n(seq, i);
    TimestampTz t = timestampset_time_n(ts, j);
    int cmp = timestamptz_cmp_internal(inst->t, t);
    if (cmp == 0)
    {
      if (atfunc)
        instants[k++] = inst;
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      if (! atfunc)
        instants[k++] = inst;
      i++;
    }
    else
      j++;
  }
  /* For minus copy the instants after the discrete sequence */
  if (! atfunc)
  {
    while (i < seq->count)
      instants[k++] = tsequence_inst_n(seq, i++);
  }
  result = (k == 0) ? NULL : tsequence_make(instants, k, k, true, true,
    DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
TSequence *
tdiscseq_at_period(const TSequence *seq, const Period *period)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, period))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tsequence_copy(seq);

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (contains_period_timestamp(period, inst->t))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal discrete sequence to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
TSequence *
tdiscseq_minus_period(const TSequence *seq, const Period *period)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, period))
    return tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (! contains_period_timestamp(period, inst->t))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a discrete temporal sequence to (the complement of) a period set.
 * @sqlfunc atPeriodSet(), minusPeriodSet()
 */
TSequence *
tdiscseq_restrict_periodset(const TSequence *seq, const PeriodSet *ps,
  bool atfunc)
{
  const TInstant *inst;

  /* Singleton period set */
  if (ps->count == 1)
    return atfunc ?
      tdiscseq_at_period(seq, periodset_per_n(ps, 0)) :
      tdiscseq_minus_period(seq, periodset_per_n(ps, 0));

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->period))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_periodset_test(inst, ps, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    bool contains = contains_periodset_timestamp(ps, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the segment of a temporal sequence to a timestamp
 *
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
    return tinstant_copy(tsequence_inst_n(seq, 0));

  /* General case */
  int n = tcontseq_find_timestamp(seq, t);
  const TInstant *inst1 = tsequence_inst_n(seq, n);
  if (t == inst1->t)
    return tinstant_copy(inst1);
  else
  {
    const TInstant *inst2 = tsequence_inst_n(seq, n + 1);
    return tsegment_at_timestamp(inst1, inst2,
      MOBDB_FLAGS_GET_LINEAR(seq->flags), t);
  }
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to a timestamp.
 * @sqlfunc atTimestamp()
 */
TInstant *
tsequence_at_timestamp(const TSequence *seq, TimestampTz t)
{
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return tdiscseq_at_timestamp(seq, t);
  else
    return tcontseq_at_timestamp(seq, t);
}

/*****************************************************************************/

/**
 * Restrict a temporal sequence to the complement of a timestamp
 *
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tcontseq_minus_timestamp1(const TSequence *seq, TimestampTz t,
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
  inst1 = tsequence_inst_n(seq, 0);
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  int i, k = 0;
  int n = tcontseq_find_timestamp(seq, t);
  /* Compute the first sequence until t */
  if (n != 0 || inst1->t < t)
  {
    for (i = 0; i < n; i++)
      instants[i] = (TInstant *) tsequence_inst_n(seq, i);
    inst1 = tsequence_inst_n(seq, n);
    inst2 = tsequence_inst_n(seq, n + 1);
    if (inst1->t == t)
    {
      if (interp == LINEAR)
      {
        instants[n] = (TInstant *) inst1;
        result[k++] = tsequence_make((const TInstant **) instants, n + 1,
          n + 1, seq->period.lower_inc, false, interp, NORMALIZE_NO);
      }
      else
      {
        instants[n] = tinstant_make(tinstant_value(instants[n - 1]),
          inst1->temptype, t);
        result[k++] = tsequence_make((const TInstant **) instants, n + 1,
          n + 1, seq->period.lower_inc, false, interp, NORMALIZE_NO);
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
      result[k++] = tsequence_make((const TInstant **) instants, n + 2,
        n + 2, seq->period.lower_inc, false, interp, NORMALIZE_NO);
      pfree(instants[n + 1]);
    }
  }
  /* Compute the second sequence after t */
  inst1 = tsequence_inst_n(seq, n);
  inst2 = tsequence_inst_n(seq, n + 1);
  if (t < inst2->t)
  {
    instants[0] = tsegment_at_timestamp(inst1, inst2, (interp == LINEAR), t);
    for (i = 1; i < seq->count - n; i++)
      instants[i] = (TInstant *) tsequence_inst_n(seq, i + n);
    result[k++] = tsequence_make((const TInstant **) instants, seq->count - n,
       seq->count - n, false, seq->period.upper_inc, interp, NORMALIZE_NO);
    pfree(instants[0]);
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a timestamp.
 *
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @return Resulting temporal sequence set
 * @sqlfunc minusTimestamp()
 */
TSequenceSet *
tcontseq_minus_timestamp(const TSequence *seq, TimestampTz t)
{
  TSequence *sequences[2];
  int count = tcontseq_minus_timestamp1(seq, t, sequences);
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to a timestamp set.
 * @sqlfunc atTimestampSet()
 */
TSequence *
tcontseq_at_timestampset(const TSequence *seq, const TimestampSet *ts)
{
  TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    inst = tsequence_at_timestamp(seq, timestampset_time_n(ts, 0));
    if (inst == NULL)
      return NULL;
    TSequence *result = tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
    pfree(inst);
    return result;
  }

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ts->period))
    return NULL;

  inst = (TInstant *) tsequence_inst_n(seq, 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (! contains_timestampset_timestamp(ts, inst->t))
      return NULL;
    return tinstant_to_tsequence((const TInstant *) inst, DISCRETE);
  }

  /* General case */
  TimestampTz t = Max(seq->period.lower, ts->period.lower);
  int loc;
  timestampset_find_timestamp(ts, t, &loc);
  TInstant **instants = palloc(sizeof(TInstant *) * (ts->count - loc));
  int k = 0;
  for (int i = loc; i < ts->count; i++)
  {
    t = timestampset_time_n(ts, i);
    inst = tcontseq_at_timestamp(seq, t);
    if (inst != NULL)
      instants[k++] = inst;
  }
  return tsequence_make_free(instants, k, k, true, true, DISCRETE,
    NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * Restrict a temporal sequence to the complement of a timestamp set
 *
 * @param[in] seq Temporal sequence
 * @param[in] ts Timestampset
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_timestampset1(const TSequence *seq, const TimestampSet *ts,
  TSequence **result)
{
  /* Singleton timestamp set */
  if (ts->count == 1)
    return tcontseq_minus_timestamp1(seq, timestampset_time_n(ts, 0),
      result);

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ts->period))
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  const TInstant *inst;
  TInstant *tofree = NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (contains_timestampset_timestamp(ts,inst->t))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  instants[0] = (TInstant *) tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int i = 1,  /* current instant of the argument sequence */
    j = 0,  /* current timestamp of the argument timestamp set */
    k = 0,  /* current number of new sequences */
    l = 1;  /* number of instants in the currently constructed sequence */
  while (i < seq->count && j < ts->count)
  {
    inst = tsequence_inst_n(seq, i);
    TimestampTz t = timestampset_time_n(ts, j);
    if (inst->t < t)
    {
      instants[l++] = (TInstant *) inst;
      i++; /* advance instants */
    }
    else if (inst->t == t)
    {
      if (interp == LINEAR)
      {
        instants[l] = (TInstant *) inst;
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          l + 1, lower_inc, false, interp, NORMALIZE_NO);
        instants[0] = (TInstant *) inst;
      }
      else
      {
        instants[l] = tinstant_make(tinstant_value(instants[l - 1]),
          inst->temptype, t);
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          l + 1, lower_inc, false, interp, NORMALIZE_NO);
        pfree(instants[l]);
        if (tofree)
        {
          pfree(tofree);
          tofree = NULL;
        }
        instants[0] = (TInstant *) inst;
      }
      l = 1;
      lower_inc = false;
      i++; /* advance instants */
      j++; /* advance timestamps */
    }
    else
    {
      /* inst->t > t */
      if (instants[l - 1]->t < t)
      {
        /* The instant to remove is not the first one of the sequence */
        instants[l] = (interp == LINEAR) ?
          tsegment_at_timestamp(instants[l - 1], inst, (interp == LINEAR), t) :
          tinstant_make(tinstant_value(instants[l - 1]), inst->temptype, t);
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          l + 1, lower_inc, false, interp, NORMALIZE_NO);
        if (tofree)
          pfree(tofree);
        instants[0] = tofree = instants[l];
        l = 1;
      }
      lower_inc = false;
      j++; /* advance timestamps */
    }
  }
  /* Compute the sequence after the timestamp set */
  if (i < seq->count)
  {
    for (j = i; j < seq->count; j++)
      instants[l++] = (TInstant *) tsequence_inst_n(seq, j);
    result[k++] = tsequence_make((const TInstant **) instants, l, l,
      false, seq->period.upper_inc, interp, NORMALIZE_NO);
  }
  if (tofree)
    pfree(tofree);
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a timestamp set.
 * @sqlfunc minusTimestampSet()
 */
TSequenceSet *
tcontseq_minus_timestampset(const TSequence *seq, const TimestampSet *ts)
{
  TSequence **sequences = palloc0(sizeof(TSequence *) * (ts->count + 1));
  int count = tcontseq_minus_timestampset1(seq, ts, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Restrict a continuous temporal sequence to a period.
 */
TSequence *
tcontseq_at_period(const TSequence *seq, const Period *p)
{
  /* Bounding box test */
  Period inter;
  if (! inter_span_span(&seq->period, p, &inter))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tsequence_copy(seq);

  /* General case */
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
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
  inst1 = tsequence_inst_n(seq, n);
  inst2 = tsequence_inst_n(seq, n + 1);
  instants[0] = tsegment_at_timestamp(inst1, inst2, (interp == LINEAR), inter.lower);
  int k = 1;
  for (int i = n + 2; i < seq->count; i++)
  {
    /* If the end of the intersecting period is between inst1 and inst2 */
    if (inst1->t <= (TimestampTz) inter.upper &&
        (TimestampTz) inter.upper <= inst2->t)
      break;

    inst1 = inst2;
    inst2 = tsequence_inst_n(seq, i);
    /* If the intersecting period contains inst1 */
    if ((TimestampTz) inter.lower <= inst1->t &&
        inst1->t <= (TimestampTz) inter.upper)
      instants[k++] = (TInstant *) inst1;
  }
  /* The last two values of sequences with step interpolation and
   * exclusive upper bound must be equal */
  if (interp == LINEAR || inter.upper_inc)
    instants[k++] = tsegment_at_timestamp(inst1, inst2, (interp == LINEAR), inter.upper);
  else
  {
    Datum value = tinstant_value(instants[k - 1]);
    instants[k++] = tinstant_make(value, seq->temptype, inter.upper);
  }
  /* Since by definition the sequence is normalized it is not necessary to
   * normalize the projection of the sequence to the period */
  result = tsequence_make((const TInstant **) instants, k, k,
    inter.lower_inc, inter.upper_inc, interp, NORMALIZE_NO);

  pfree(instants[0]); pfree(instants[k - 1]); pfree(instants);

  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to a period.
 * @sqlfunc atPeriod()
 */
TSequence *
tsequence_at_period(const TSequence *seq, const Period *p)
{
  if(MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return tdiscseq_at_period(seq, p);
  else
    return tcontseq_at_period(seq, p);
}

/**
 * Restrict a temporal sequence to the complement of a period.
 *
 * @param[in] seq Temporal sequence
 * @param[in] p Period
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 */
int
tcontseq_minus_period1(const TSequence *seq, const Period *p,
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
  PeriodSet *ps = minus_period_period(&seq->period, p);
  if (ps == NULL)
    return 0;
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p1 = periodset_per_n(ps, i);
    result[i] = tcontseq_at_period(seq, p1);
  }
  pfree(ps);
  return ps->count;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to the complement of a period.
 * @sqlfunc minusPeriod()
 */
TSequenceSet *
tcontseq_minus_period(const TSequence *seq, const Period *p)
{
  TSequence *sequences[2];
  int count = tcontseq_minus_period1(seq, p, sequences);
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
 * Restrict a temporal sequence to a period set
 *
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is not called for each sequence of a temporal sequence
 * set but is called when computing tpointseq minus geometry
 * @sqlfunc atPeriodSet()
 */
int
tcontseq_at_periodset1(const TSequence *seq, const PeriodSet *ps,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
  {
    result[0] = tcontseq_at_period(seq, periodset_per_n(ps, 0));
    return 1;
  }

  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->period))
    return 0;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = tsequence_inst_n(seq, 0);
    if (! contains_periodset_timestamp(ps, inst->t))
      return 0;
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  int loc;
  periodset_find_timestamp(ps, seq->period.lower, &loc);
  int k = 0;
  for (int i = loc; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    TSequence *seq1 = tcontseq_at_period(seq, p);
    if (seq1 != NULL)
      result[k++] = seq1;
    if (seq->period.upper < p->upper)
      break;
  }
  return k;
}

/**
 * Restrict a temporal sequence to the complement of a period set
 *
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[in] from Index from which the processing starts
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 * @sqlfunc minusPeriodSet()
 */
int
tcontseq_minus_periodset1(const TSequence *seq, const PeriodSet *ps, int from,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
    return tcontseq_minus_period1(seq, periodset_per_n(ps, 0), result);

  /* The sequence can be split at most into (count + 1) sequences
   *    |----------------------|
   *        |---| |---| |---|
   */
  TSequence *curr = tsequence_copy(seq);
  int k = 0;
  for (int i = from; i < ps->count; i++)
  {
    const Period *p1 = periodset_per_n(ps, i);
    /* If the remaining periods are to the left of the current period */
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(curr->period.upper),
      DatumGetTimestampTz(p1->lower));
    if (cmp < 0 || (cmp == 0 && curr->period.upper_inc && ! p1->lower_inc))
    {
      result[k++] = curr;
      break;
    }
    TSequence *minus[2];
    int countminus = tcontseq_minus_period1(curr, p1, minus);
    pfree(curr);
    /* minus can have from 0 to 2 periods */
    if (countminus == 0)
      break;
    else if (countminus == 1)
      curr = minus[0];
    else /* countminus == 2 */
    {
      result[k++] = minus[0];
      curr = minus[1];
    }
    /* There are no more periods left */
    if (i == ps->count - 1)
      result[k++] = curr;
  }
  return k;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal sequence to (the complement of) a period set.
 *
 * @param[in] seq Temporal sequence
 * @param[in] ps Period set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal sequence set
 * @sqlfunc atPeriodSet(), minusPeriodSet()
 */
TSequenceSet *
tcontseq_restrict_periodset(const TSequence *seq, const PeriodSet *ps,
  bool atfunc)
{
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ps->period))
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = tsequence_inst_n(seq, 0);
    if (contains_periodset_timestamp(ps, inst->t))
      return atfunc ? tsequence_to_tsequenceset(seq) : NULL;
    return atfunc ? NULL : tsequence_to_tsequenceset(seq);
  }

  /* General case */
  int count = atfunc ? ps->count : ps->count + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int count1 = atfunc ? tcontseq_at_periodset1(seq, ps, sequences) :
    tcontseq_minus_periodset1(seq, ps, 0, sequences);
  return tsequenceset_make_free(sequences, count1, NORMALIZE_NO);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal sequence intersects a timestamp.
 * @sqlfunc intersectsTimestamp()
 */
bool
tsequence_intersects_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Discrete sequence */
  if (MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return (tdiscseq_find_timestamp(seq, t) >= 0);
  /* Continuous sequence */
  return contains_period_timestamp(&seq->period, t);
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal sequence intersects a timestamp set.
 * @sqlfunc intersectsTimestampSet()
 */
bool
tsequence_intersects_timestampset(const TSequence *seq, const TimestampSet *ts)
{
  for (int i = 0; i < ts->count; i++)
    if (tsequence_intersects_timestamp(seq, timestampset_time_n(ts, i)))
      return true;
  return false;
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal sequence intersects a period.
 * @sqlfunc intersectsPeriod()
 */
bool
tsequence_intersects_period(const TSequence *seq, const Period *p)
{
  /* Bounding period test */
  if (! overlaps_span_span(&seq->period, p))
    return false;

  /* For continuous sequences return true given the above test */
  if (! MOBDB_FLAGS_GET_DISCRETE(seq->flags))
    return true;

  /* Discrete sequence */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (contains_period_timestamp(p, inst->t))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal sequence intersects a period set.
 * @sqlfunc intersectsPeriodSet()
 */
bool
tsequence_intersects_periodset(const TSequence *seq, const PeriodSet *ps)
{
  for (int i = 0; i < ps->count; i++)
    if (tsequence_intersects_period(seq, periodset_per_n(ps, i)))
      return true;
  return false;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal sequence
 * number.
 */
double
tnumberseq_integral(const TSequence *seq)
{
  double result = 0;
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
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
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighted average of a temporal discrete sequence number
 * @note Since a discrete sequence does not have duration, the function returns
 * the traditional average of the values
 * @sqlfunc twAvg()
 */
double
tnumberdiscseq_twavg(const TSequence *seq)
{
  mobdbType basetype = temptype_basetype(seq->temptype);
  double result = 0.0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    result += datum_double(tinstant_value(inst), basetype);
  }
  return result / seq->count;
}

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighted average of a temporal sequence number.
 * @sqlfunc twAvg()
 */
double
tnumbercontseq_twavg(const TSequence *seq)
{
  double duration = (double) (seq->period.upper - seq->period.lower);
  double result;
  if (duration == 0.0)
    /* Instantaneous sequence */
    result = datum_double(tinstant_value(tsequence_inst_n(seq, 0)),
      temptype_basetype(seq->temptype));
  else
    result = tnumberseq_integral(seq) / duration;
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_comp
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
    const TInstant *inst1 = tsequence_inst_n(seq1, i);
    const TInstant *inst2 = tsequence_inst_n(seq2, i);
    if (! tinstant_eq(inst1, inst2))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_int_temporal_comp
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
    const TInstant *inst1 = tsequence_inst_n(seq1, i);
    const TInstant *inst2 = tsequence_inst_n(seq2, i);
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
 * @ingroup libmeos_int_temporal_accessor
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
  uint32 result = UInt32GetDatum(hash_uint32((uint32) flags));

  /* Merge with hash of instants */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    uint32 inst_hash = tinstant_hash(inst);
    result = (result << 5) - result + inst_hash;
  }
  return result;
}

/*****************************************************************************/
