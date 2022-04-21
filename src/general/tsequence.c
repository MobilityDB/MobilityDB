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
 * @file tsequence.c
 * @brief General functions for temporal sequences.
 */

#include "general/tsequence.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
#include <access/hash.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/doublen.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "general/rangetypes_ext.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Collinear functions
 * Are the three temporal instant values collinear?
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

/**
 * Return true if the three values are collinear
 *
 * @param[in] basetype Base type
 * @param[in] value1,value2,value3 Input values
 * @param[in] t1,t2,t3 Input timestamps
 */
static bool
datum_collinear(CachedType basetype, Datum value1, Datum value2, Datum value3,
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
    bool hasz = (bool) FLAGS_GET_Z(GS_FLAGS(gs));
    bool geodetic = (bool) FLAGS_GET_GEODETIC(GS_FLAGS(gs));
    return geopoint_collinear(value1, value2, value3, ratio, hasz, geodetic);
  }
  if (basetype == T_DOUBLE3)
    return double3_collinear(DatumGetDouble3P(value1), DatumGetDouble3P(value2),
      DatumGetDouble3P(value3), ratio);
  if (basetype == T_DOUBLE4)
    return double4_collinear(DatumGetDouble4P(value1), DatumGetDouble4P(value2),
      DatumGetDouble4P(value3), ratio);
  if (basetype == T_NPOINT)
    return npoint_collinear(DatumGetNpointP(value1), DatumGetNpointP(value2),
      DatumGetNpointP(value3), ratio);
  elog(ERROR, "unknown collinear operation for base type: %d", basetype);
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Return a pointer to the bounding box of the temporal value
 */
void *
tsequence_bbox_ptr(const TSequence *seq)
{
  return (void *)(((char *)seq) + double_pad(sizeof(TSequence)));
}

/**
 * Copy in the second argument the bounding box of the temporal value
 */
void
tsequence_bbox(const TSequence *seq, void *box)
{
  memset(box, 0, seq->bboxsize);
  memcpy(box, tsequence_bbox_ptr(seq), seq->bboxsize);
  return;
}

/**
 * Return a pointer to the offsets array of the temporal value
 */
static size_t *
tsequence_offsets_ptr(const TSequence *seq)
{
  return (size_t *)(((char *)seq) + double_pad(sizeof(TSequence)) +
    double_pad(seq->bboxsize));
}

/**
 * @ingroup libmeos_temporal_acessor
 * @brief Return the n-th instant of the temporal value.
 */
const TInstant *
tsequence_inst_n(const TSequence *seq, int index)
{
  return (TInstant *)(
    /* start of data */
    ((char *)seq) + double_pad(sizeof(TSequence)) + seq->bboxsize +
      seq->count * sizeof(size_t) +
      /* offset */
      (tsequence_offsets_ptr(seq))[index]);
}

/**
 * Ensure the validity of the arguments when creating a temporal value
 */
void
tsequence_make_valid1(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear)
{
  /* Test the validity of the instants */
  assert(count > 0);
  ensure_tinstarr(instants, count);
  if (count == 1 && (!lower_inc || !upper_inc))
    ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
        errmsg("Instant sequence must have inclusive bounds")));
  CachedType basetype = temptype_basetype(instants[0]->temptype);
  if (! linear && count > 1 && !upper_inc &&
    datum_ne(tinstant_value(instants[count - 1]),
      tinstant_value(instants[count - 2]), basetype))
    ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
      errmsg("Invalid end value for temporal sequence")));
  return;
}

/**
 * Ensure the validity of the arguments when creating a temporal value
 */
static void
tsequence_make_valid(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear)
{
  tsequence_make_valid1(instants, count, lower_inc, upper_inc, linear);
  ensure_valid_tinstarr(instants, count, MERGE_NO, SEQUENCE);
  return;
}

/**
 * Normalize the array of temporal instant values
 *
 * @param[in] instants Array of input instants
 * @param[in] linear True when the instants have linear interpolation
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @result Array of normalized temporal instant values
 * @pre The input array has at least two elements
 * @note The function does not create new instants, it creates an array of
 * pointers to a subset of the input instants
 */
static TInstant **
tinstarr_normalize(const TInstant **instants, bool linear, int count,
  int *newcount)
{
  assert(count > 1);
  CachedType basetype = temptype_basetype(instants[0]->temptype);
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
    if (
      /* step sequences and 2 consecutive instants that have the same value
        ... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
      */
      (!linear && datum_eq(value1, value2, basetype))
      ||
      /* 3 consecutive linear instants that have the same value
        ... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
      */
      (linear && datum_eq(value1, value2, basetype) &&
        datum_eq(value2, value3, basetype))
      ||
      /* collinear linear instants
        ... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
      */
      (linear && datum_collinear(basetype, value1, value2, value3,
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
 * Construct a temporal sequence value from the array of temporal
 * instant values
 *
 * For example, the memory structure of a temporal sequence value with
 * two instants is as follows:
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
tsequence_make1(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear, bool normalize)
{
  /* Normalize the array of instants */
  TInstant **norminsts = (TInstant **) instants;
  int newcount = count;
  if (normalize && count > 1)
    norminsts = tinstarr_normalize(instants, linear, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = double_pad(temporal_bbox_size(instants[0]->temptype));

  /* Compute the size of the temporal sequence */
  /* Bounding box size */
  size_t memsize = bboxsize;
  /* Size of composing instants */
  for (int i = 0; i < newcount; i++)
    memsize += double_pad(VARSIZE(norminsts[i]));
  /* Size of the struct and the offset array */
  memsize += double_pad(sizeof(TSequence)) + newcount * sizeof(size_t);
  /* Create the temporal sequence */
  TSequence *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->temptype = instants[0]->temptype;
  result->subtype = SEQUENCE;
  result->bboxsize = bboxsize;
  period_set(norminsts[0]->t, norminsts[newcount - 1]->t, lower_inc, upper_inc,
    &result->period);
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags,
    MOBDB_FLAGS_GET_CONTINUOUS(norminsts[0]->flags));
  MOBDB_FLAGS_SET_LINEAR(result->flags, linear);
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
   * as double2, double3, or double4 do not have bounding box
   */
  if (bboxsize != 0)
  {
    tsequence_make_bbox((const TInstant **) norminsts, newcount, lower_inc,
      upper_inc, linear, tsequence_bbox_ptr(result));
  }
  /* Store the composing instants */
  size_t pdata = double_pad(sizeof(TSequence)) + double_pad(bboxsize) +
    newcount * sizeof(size_t);
  size_t pos = 0;
  for (int i = 0; i < newcount; i++)
  {
    memcpy(((char *) result) + pdata + pos, norminsts[i],
      VARSIZE(norminsts[i]));
    (tsequence_offsets_ptr(result))[i] = pos;
    pos += double_pad(VARSIZE(norminsts[i]));
  }
  if (normalize && count > 1)
    pfree(norminsts);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence value from the array of temporal
 * instant values.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True when the respective bound is inclusive
 * @param[in] linear True when the interpolation is linear
 * @param[in] normalize True when the resulting value should be normalized
 */
TSequence *
tsequence_make(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear, bool normalize)
{
  tsequence_make_valid(instants, count, lower_inc, upper_inc, linear);
  return tsequence_make1(instants, count, lower_inc, upper_inc, linear,
    normalize);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence value from the array of temporal
 * instant values and free the array and the instants after the creation.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True when the respective bound is inclusive
 * @param[in] linear True when the interpolation is linear
 * @param[in] normalize True when the resulting value should be normalized
 */
TSequence *
tsequence_make_free(TInstant **instants, int count, bool lower_inc,
   bool upper_inc, bool linear, bool normalize)
{
  if (count == 0)
  {
    pfree(instants);
    return NULL;
  }
  TSequence *result = tsequence_make((const TInstant **) instants, count,
    lower_inc, upper_inc, linear, normalize);
  pfree_array((void **) instants, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of the temporal value.
 */
TSequence *
tsequence_copy(const TSequence *seq)
{
  TSequence *result = palloc0(VARSIZE(seq));
  memcpy(result, seq, VARSIZE(seq));
  return result;
}

/**
 * Join the two temporal sequence values
 *
 * @param[in] seq1,seq2 Temporal sequence values
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
  TSequence *result = tsequence_make1(instants, count,
    seq1->period.lower_inc, seq2->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq1->flags), NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * Return the index of the segment of the temporal sequence value
 * containing the timestamp using binary search
 *
 * If the timestamp is contained in the temporal value, the index of the
 * segment containing the timestamp is returned in the output parameter.
 * For example, given a value composed of 3 sequences and a timestamp,
 * the value returned in the output parameter is as follows:
 * @code
 *            0     1     2     3
 *            |-----|-----|-----|
 * 1)    t^                             => result = -1
 * 2)        t^                         => result = 0 if the lower bound is inclusive, -1 otherwise
 * 3)              t^                   => result = 1
 * 4)                 t^                => result = 1
 * 5)                             t^    => result = -1
 * @endcode
 *
 * @param[in] seq Temporal sequence value
 * @param[in] t Timestamp
 * @result Return -1 if the timestamp is not contained in the temporal value
 */
int
tsequence_find_timestamp(const TSequence *seq, TimestampTz t)
{
  int first = 0;
  int last = seq->count - 1;
  int middle = (first + last)/2;
  while (first <= last)
  {
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
    middle = (first + last)/2;
  }
  return -1;
}

/**
 * Convert an array of arrays of temporal sequence values into an array of
 * sequence values.
 *
 * This function is called by all the functions in which the number of
 * output sequences is not bounded, typically when each segment of the
 * input sequence can produce an arbitrary number of output sequences,
 * as in the case of atGeometry.
 *
 * @param[in] sequences Array of array of temporal sequence values
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
 * @ingroup libmeos_temporal_transf
 * @brief Append an instant to the temporal value.
 */
Temporal *
tsequence_append_tinstant(const TSequence *seq, const TInstant *inst)
{
  /* Ensure validity of the arguments */
  assert(seq->temptype == inst->temptype);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  const TInstant *inst1 = tsequence_inst_n(seq, seq->count - 1);
  bool isnpoint = inst1->temptype == T_TNPOINT;
  CachedType basetype = temptype_basetype(seq->temptype);
  if (isnpoint)
    ensure_same_rid_tnpointinst(inst, inst1);
  /* Notice that we cannot call ensure_increasing_timestamps since we must
   * take into account the inclusive/exclusive bounds */
  if (inst1->t > inst->t)
  {
    char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
    char *t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
    ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
      errmsg("Timestamps for temporal value must be increasing: %s, %s", t1, t2)));
  }
  if (inst1->t == inst->t)
  {
    bool seqresult = datum_eq(tinstant_value(inst1), tinstant_value(inst),
      basetype);
    if (seq->period.upper_inc && ! seqresult)
    {
      char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("The temporal values have different value at their common instant %s", t1)));
    }
    /* The result is a sequence set */
    if (linear && ! seqresult)
    {
      TSequence *sequences[2];
      sequences[0] = (TSequence *) seq;
      sequences[1] = tinstant_tsequence(inst, linear);
      TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
        2, NORMALIZE_NO);
      pfree(sequences[1]);
      return (Temporal *) result;
    }
  }

  /* The result is a sequence */
  int count = seq->count + 1;
  if (seq->count > 1)
  {
    /* Normalize the result */
    inst1 = tsequence_inst_n(seq, seq->count - 2);
    Datum value1 = tinstant_value(inst1);
    const TInstant *inst2 = tsequence_inst_n(seq, seq->count - 1);
    Datum value2 = tinstant_value(inst2);
    Datum value3 = tinstant_value(inst);
    if (
      /* step sequences and 2 consecutive instants that have the same value
        ... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
      */
      (! linear && datum_eq(value1, value2, basetype))
      ||
      /* 3 consecutive float/point instants that have the same value
        ... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
      */
      (datum_eq(value1, value2, basetype) && datum_eq(value2, value3, basetype))
      ||
      /* collinear float/point instants that have the same duration
        ... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
      */
      (linear && datum_collinear(basetype, value1, value2, value3, inst1->t,
        inst2->t, inst->t))
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
  TSequence *result = tsequence_make1(instants, count, seq->period.lower_inc,
    true, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE_NO);
  pfree(instants);
  return (Temporal *) result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the two temporal values.
 */
Temporal *
tsequence_merge(const TSequence *seq1, const TSequence *seq2)
{
  const TSequence *sequences[] = {seq1, seq2};
  return tsequence_merge_array(sequences, 2);
}

/**
 * Normalize the array of temporal sequence values
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
  CachedType basetype = temptype_basetype(seq1->temptype);
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
      (!linear &&
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
    else if (adjacent && !linear && !seq1->period.upper_inc)
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
 * Merge the array of temporal sequence values.
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
  CachedType basetype = temptype_basetype(seq1->temptype);
  for (int i = 1; i < count; i++)
  {
    const TInstant *inst1 = tsequence_inst_n(seq1, seq1->count - 1);
    const TSequence *seq2 = sequences[i];
    const TInstant *inst2 = tsequence_inst_n(seq2, 0);
    char *t1;
    if (inst1->t > inst2->t)
    {
      char *t2;
      t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
      t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst2->t));
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("The temporal values cannot overlap on time: %s, %s", t1, t2)));
    }
    else if (inst1->t == inst2->t && seq1->period.upper_inc &&
      seq2->period.lower_inc)
    {
      if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype))
      {
        t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
          errmsg("The temporal values have different value at their common instant %s", t1)));
      }
    }
    seq1 = seq2;
  }
  return tseqarr_normalize(sequences, count, totalcount);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the array of temporal sequence values.
 *
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value
 * @note The values in the array may overlap on a single instant.
 */
Temporal *
tsequence_merge_array(const TSequence **sequences, int count)
{
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
 * Synchronize two temporal values
 *
 * The resulting values are composed of denormalized sequences covering the
 * intersection of their time spans. The argument crossings determines
 * whether potential crossings between successive pair of instants are added.
 * Crossings are only added when at least one of the sequences has linear
 * interpolation.
 *
 * @param[in] seq1,seq2 Input values
 * @param[out] sync1,sync2 Output values
 * @param[in] crossings State whether turning points are added in the segments
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
  TSequence **sync1, TSequence **sync2, bool crossings)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period inter;
  if (! inter_period_period(&seq1->period, &seq2->period, &inter))
    return false;

  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  TInstant *inst1, *inst2;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    inst1 = tsequence_at_timestamp(seq1, inter.lower);
    inst2 = tsequence_at_timestamp(seq2, inter.lower);
    *sync1 = tinstant_tsequence(inst1, linear1);
    *sync2 = tinstant_tsequence(inst2, linear2);
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
  if (inst1->t < inter.lower)
  {
    i = tsequence_find_timestamp(seq1, inter.lower) + 1;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
  }
  else if (inst2->t < inter.lower)
  {
    j = tsequence_find_timestamp(seq2, inter.lower) + 1;
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count * 2);
  CachedType basetype1 = temptype_basetype(seq1->temptype);
  CachedType basetype2 = temptype_basetype(seq2->temptype);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= inter.upper || inst2->t <= inter.upper))
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
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
    if (crossings && (linear1 || linear2) && k > 0)
    {
      TimestampTz crosstime;
      Datum inter1, inter2;
      if (tsegment_intersection(instants1[k - 1], inst1, linear1,
        instants2[k - 1], inst2, linear2, &inter1, &inter2, &crosstime))
      {
        instants1[k] = tofree[l++] = tinstant_make(inter1, crosstime,
          seq1->temptype);
        instants2[k++] = tofree[l++] = tinstant_make(inter2, crosstime,
          seq2->temptype);
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
  if (! inter.upper_inc && k > 1 && ! linear1 &&
      datum_ne(tinstant_value(instants1[k - 2]),
        tinstant_value(instants1[k - 1]), basetype1))
  {
    instants1[k - 1] = tinstant_make(tinstant_value(instants1[k - 2]),
      instants1[k - 1]->t, instants1[k - 1]->temptype);
    tofree[l++] = instants1[k - 1];
  }
  if (! inter.upper_inc && k > 1 && ! linear2 &&
      datum_ne(tinstant_value(instants2[k - 2]),
        tinstant_value(instants2[k - 1]), basetype2))
  {
    instants2[k - 1] = tinstant_make(tinstant_value(instants2[k - 2]),
      instants2[k - 1]->t, instants2[k - 1]->temptype);
    tofree[l++] = instants2[k - 1];
  }
  *sync1 = tsequence_make((const TInstant **) instants1, k, inter.lower_inc,
    inter.upper_inc, linear1, NORMALIZE_NO);
  *sync2 = tsequence_make((const TInstant **) instants2, k, inter.lower_inc,
    inter.upper_inc, linear2, NORMALIZE_NO);

  pfree_array((void **) tofree, l);
  pfree(instants1); pfree(instants2);

  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

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
 * Return true if the segment of the temporal number intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[in] basetype Base type
 * @param[out] t Timestamp
 */
static bool
tfloatsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, CachedType basetype, TimestampTz *t)
{
  assert(inst1->temptype == T_TFLOAT);
  double dvalue1 = DatumGetFloat8(tinstant_value(inst1));
  double dvalue2 = DatumGetFloat8(tinstant_value(inst2));
  double dvalue = datum_double(value, basetype);
  double min = Min(dvalue1, dvalue2);
  double max = Max(dvalue1, dvalue2);
  /* if value is to the left or to the right of the range */
  if (dvalue < min || dvalue > max)
    return false;

  double range = (max - min);
  double partial = (dvalue - min);
  double fraction = dvalue1 < dvalue2 ? partial / range : 1 - partial / range;
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
 * Return true if the segment of the temporal value intersects
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
  Datum value, CachedType basetype, Datum *inter, TimestampTz *t)
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
  else if (inst1->temptype == T_TNPOINT)
    result = tnpointsegm_intersection_value(inst1, inst2, value, t);
  else
    elog(ERROR, "unknown intersection function for continuous temporal type: %d",
      inst1->temptype);

  if (result && inter != NULL)
    /* We are sure it is linear interpolation */
    *inter = tsegment_value_at_timestamp(inst1, inst2, LINEAR, *t);
  return result;
}

/*****************************************************************************/

/**
 * Compute the intersection, if any, of two segments of temporal sequences.
 * These functions suppose that the instants are synchronized, i.e.,
 * start1->t = start2->t and end1->t = end2->t.
 * The functions return true if there is an intersection at the middle of
 * the segments, i.e., they return false if they intersect at a bound. If
 * they return true, they also return in the output parameter t the
 * intersection timestamp. The two values taken by the segments at the
 * intersection timestamp t are equal up to the floating point precision.
 * For the temporal point case we cannot use the PostGIS functions
 * lw_dist2d_seg_seg and lw_dist3d_seg_seg since they do not take time into
 * consideration and would return, e.g., that the two segments
 * [Point(1 1)@t1, Point(3 3)@t2] and [Point(3 3)@t1, Point(1 1)@t2]
 * intersect at Point(1 1), instead of Point(2 2).
 * These functions are used to add intermediate points when lifting
 * operators, in particular for temporal comparisons such as
 * tfloat <comp> tfloat where <comp> is <, <=, ... since the comparison
 * changes its value before/at/after the intersection point.
 */

/**
 * Return true if the two segments of the temporal numbers
 * intersect at the timestamp
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
 * Return true if the two segments of the temporal values intersect at the
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
  CachedType basetype1 = temptype_basetype(start1->temptype);
  CachedType basetype2 = temptype_basetype(start2->temptype);
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
      *inter1 = tsegment_value_at_timestamp(start1, end1, LINEAR, *t);
    if (result && inter2 != NULL)
      *inter2 = tsegment_value_at_timestamp(start2, end2, LINEAR, *t);
  }
  return result;
}

/**
 * Temporally intersect the two temporal values
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
  *inter2 = tinstant_copy(inst1);
  return true;
}

/**
 * Temporally intersect the two temporal values
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

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] seq,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tsequence_tinstantset(const TSequence *seq, const TInstantSet *ti,
  TInstantSet **inter1, TInstantSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p;
  tinstantset_period(ti, &p);
  if (! overlaps_period_period(&seq->period, &p))
    return false;

  TInstant **instants1 = palloc(sizeof(TInstant *) * ti->count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      instants1[k] = tsequence_at_timestamp(seq, inst->t);
      instants2[k++] = inst;
    }
    if (seq->period.upper < inst->t)
      break;
  }
  if (k == 0)
  {
    pfree(instants1); pfree(instants2);
    return false;
  }

  *inter1 = tinstantset_make_free(instants1, k, MERGE_NO);
  *inter2 = tinstantset_make(instants2, k, MERGE_NO);
  pfree(instants2);
  return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,seq Temporal values
 * @param[out] inter1,inter2 Output values
 * @result Return false if the input values do not overlap on time.
 */
bool
intersection_tinstantset_tsequence(const TInstantSet *ti, const TSequence *seq,
  TInstantSet **inter1, TInstantSet **inter2)
{
  return intersection_tsequence_tinstantset(seq, ti, inter2, inter1);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the string representation of the temporal value.
 *
 * @param[in] seq Temporal value
 * @param[in] component True when the output string is a component of
 * a temporal sequence set value and thus no interpolation string
 * at the begining of the string should be output
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
tsequence_to_string(const TSequence *seq, bool component,
  char *(*value_out)(Oid, Datum))
{
  char **strings = palloc(sizeof(char *) * seq->count);
  size_t outlen = 0;
  char prefix[20];
  if (! component && MOBDB_FLAGS_GET_CONTINUOUS(seq->flags) &&
      ! MOBDB_FLAGS_GET_LINEAR(seq->flags))
    sprintf(prefix, "Interp=Stepwise;");
  else
    prefix[0] = '\0';
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    strings[i] = tinstant_to_string(inst, value_out);
    outlen += strlen(strings[i]) + 2;
  }
  char open = seq->period.lower_inc ? (char) '[' : (char) '(';
  char close = seq->period.upper_inc ? (char) ']' : (char) ')';
  return stringarr_to_string(strings, seq->count, outlen, prefix,
    open, close);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the temporal value into the buffer.
 *
 * @param[in] seq Temporal value
 * @param[in] buf Buffer
 */
void
tsequence_write(const TSequence *seq, StringInfo buf)
{
  pq_sendint32(buf, seq->count);
  pq_sendbyte(buf, seq->period.lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, seq->period.upper_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, MOBDB_FLAGS_GET_LINEAR(seq->flags) ? (uint8) 1 : (uint8) 0);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    tinstant_write(inst, buf);
  }
  return;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new temporal value from its binary representation read from
 * the buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TSequence *
tsequence_read(StringInfo buf, CachedType temptype)
{
  int count = (int) pq_getmsgint(buf, 4);
  bool lower_inc = (char) pq_getmsgbyte(buf);
  bool upper_inc = (char) pq_getmsgbyte(buf);
  bool linear = (char) pq_getmsgbyte(buf);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = tinstant_read(buf, temptype);
  return tsequence_make_free(instants, count, lower_inc,
    upper_inc, linear, NORMALIZE);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence value from a base value and a period.
 *
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] p Period
 * @param[in] linear True when the resulting value has linear interpolation
 */
TSequence *
tsequence_from_base(Datum value, CachedType temptype, const Period *p,
  bool linear)
{
  int count;
  TInstant *instants[2];
  instants[0] = tinstant_make(value, p->lower, temptype);
  if (p->lower != p->upper)
  {
    instants[1] = tinstant_make(value, p->upper, temptype);
    count = 2;
  }
  else
    count = 1;
  TSequence *result = tsequence_make((const TInstant **) instants, count,
    p->lower_inc, p->upper_inc, linear, NORMALIZE_NO);
  pfree(instants[0]);
  if (p->lower != p->upper)
    pfree(instants[1]);
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal integer value as a temporal float value.
 */
TSequence *
tintseq_tfloatseq(const TSequence *seq)
{
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TFLOAT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, true);
  MOBDB_FLAGS_SET_LINEAR(result->flags, false);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) tsequence_inst_n(result, i);
    inst->temptype = T_TFLOAT;
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal float value as a temporal integer value.
 */
TSequence *
tfloatseq_tintseq(const TSequence *seq)
{
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot cast temporal float with linear interpolation to temporal integer")));
  TSequence *result = tsequence_copy(seq);
  result->temptype = T_TINT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, false);
  MOBDB_FLAGS_SET_LINEAR(result->flags, false);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) tsequence_inst_n(result, i);
    inst->temptype = T_TINT;
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant value into a temporal sequence value.
 */
TSequence *
tinstant_tsequence(const TInstant *inst, bool linear)
{
  return tsequence_make(&inst, 1, true, true, linear, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant set value into a temporal sequence
 * value.
 */
TSequence *
tinstantset_tsequence(const TInstantSet *ti, bool linear)
{
  if (ti->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input value to a temporal sequence")));
  return tinstant_tsequence(tinstantset_inst_n(ti, 0), linear);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence set value into a temporal sequence
 * value.
 */
TSequence *
tsequenceset_tsequence(const TSequenceSet *ts)
{
  if (ts->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input to a temporal sequence")));
  return tsequence_copy(tsequenceset_seq_n(ts, 0));
}

/**
 * Transform the temporal sequence value with continuous base type
 * from stepwise to linear interpolation
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @return Number of resulting sequences returned
 */
int
tstepseq_tlinearseq1(const TSequence *seq, TSequence **result)
{
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    MOBDB_FLAGS_SET_LINEAR(result[0]->flags, true);
    return 1;
  }

  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  const TInstant *inst2 = NULL; /* keep compiler quiet */
  Datum value2;
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = tsequence_inst_n(seq, i);
    value2 = tinstant_value(inst2);
    TInstant *instants[2];
    instants[0] = (TInstant *) inst1;
    instants[1] = tinstant_make(value1, inst2->t, seq->temptype);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc &&
      datum_eq(value1, value2, basetype) : false;
    result[k++] = tsequence_make((const TInstant **) instants, 2,
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
      result[k++] = tinstant_tsequence(inst2, LINEAR);
  }
  return k;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence value with continuous base type from
 * stepwise to linear interpolation.
 *
 * @param[in] seq Temporal value
 * @return Resulting temporal sequence set value
 */
TSequenceSet *
tstepseq_tlinearseq(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tstepseq_tlinearseq1(seq, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift and/or scale the time span of the temporal value by the two
 * intervals.
 *
 * @pre The duration is greater than 0 if it is not NULL
 */
TSequence *
tsequence_shift_tscale(const TSequence *seq, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);

  /* Copy the input sequence to the result */
  TSequence *result = tsequence_copy(seq);

  /* Shift and/or scale the bounding period */
  period_shift_tscale(start, duration, &result->period);
  TimestampTz shift;
  if (start != NULL)
    shift = result->period.lower - seq->period.lower;
  double scale;
  bool instant = (result->period.lower == result->period.upper);
  /* If the sequence set is instantaneous we cannot scale */
  if (duration != NULL && ! instant)
  {
    scale =
      (double) (result->period.upper - result->period.lower) /
      (double) (seq->period.upper - seq->period.lower) ;
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
      if (start != NULL)
        inst->t += shift;
      if (duration != NULL && ! instant)
        inst->t = result->period.lower +
          (inst->t - result->period.lower) * scale;
    }
    /* Set the last instant */
    inst = (TInstant *) tsequence_inst_n(result, seq->count - 1);
    inst->t = result->period.upper;
  }
  /* Shift and/or scale bounding box */
  void *bbox = tsequence_bbox_ptr(result);
  temporal_bbox_shift_tscale(bbox, start, duration, seq->temptype);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of the temporal value with stepwise
 * interpolation.
 *
 * @param[in] seq Temporal value
 * @param[out] count Number of values in the resulting array
 * @result Array of values
 */
Datum *
tsequence_values(const TSequence *seq, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = tinstant_value(tsequence_inst_n(seq, i));
  if (seq->count > 1)
  {
    CachedType basetype = temptype_basetype(seq->temptype);
    datumarr_sort(result, seq->count, basetype);
    *count = datumarr_remove_duplicates(result, seq->count, basetype);
  }
  else
    *count = 1;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the range of base values of the temporal float.
 */
RangeType *
tfloatseq_range(const TSequence *seq)
{
  TBOX *box = tsequence_bbox_ptr(seq);
  Datum min = Float8GetDatum(box->xmin);
  Datum max = Float8GetDatum(box->xmax);
  /* It step interpolation or equal bounding box bounds */
  if(! MOBDB_FLAGS_GET_LINEAR(seq->flags) ||
    box->xmin == box->xmax)
    return range_make(min, max, true, true, T_FLOAT8);

  Datum start = tinstant_value(tsequence_inst_n(seq, 0));
  Datum end = tinstant_value(tsequence_inst_n(seq, seq->count - 1));
  Datum lower, upper;
  bool lower_inc, upper_inc;
  if (DatumGetFloat8(start) < DatumGetFloat8(end))
  {
    lower = start; lower_inc = seq->period.lower_inc;
    upper = end; upper_inc = seq->period.upper_inc;
  }
  else
  {
    lower = end; lower_inc = seq->period.upper_inc;
    upper = start; upper_inc = seq->period.lower_inc;
  }
  bool min_inc = DatumGetFloat8(min) < DatumGetFloat8(lower) ||
    (DatumGetFloat8(min) == DatumGetFloat8(lower) && lower_inc);
  bool max_inc = DatumGetFloat8(max) > DatumGetFloat8(upper) ||
    (DatumGetFloat8(max) == DatumGetFloat8(upper) && upper_inc);
  if (! min_inc || ! max_inc)
  {
    for (int i = 1; i < seq->count - 1; i++)
    {
      const TInstant *inst = tsequence_inst_n(seq, i);
      if (min_inc || DatumGetFloat8(min) == DatumGetFloat8(tinstant_value(inst)))
        min_inc = true;
      if (max_inc || DatumGetFloat8(max) == DatumGetFloat8(tinstant_value(inst)))
        max_inc = true;
      if (min_inc && max_inc)
        break;
    }
  }
  return range_make(min, max, min_inc, max_inc, T_FLOAT8);
}

/**
 * Return the ranges of base values of the temporal float
 * with stepwise interpolation
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * ranges are stored
 * @param[in] seq Temporal value
 * @result Number of ranges in the result
 */
int
tfloatseq_ranges1(const TSequence *seq, RangeType **result)
{
  /* Temporal float with linear interpolation */
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    result[0] = tfloatseq_range(seq);
    return 1;
  }

  /* Temporal float with step interpolation */
  int count;
  Datum *values = tsequence_values(seq, &count);
  for (int i = 0; i < count; i++)
    result[i] = range_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  return count;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of ranges of base values of the temporal float value.
 */
RangeType **
tfloatseq_ranges(const TSequence *seq, int *count)
{
  RangeType **result = palloc(sizeof(RangeType *) * seq->count);
  *count = tfloatseq_ranges1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time on which the temporal value is defined as a period set.
 */
PeriodSet *
tsequence_time(const TSequence *seq)
{
  return period_periodset(&seq->period);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 */
const TInstant *
tsequence_min_instant(const TSequence *seq)
{
  Datum min = tinstant_value(tsequence_inst_n(seq, 0));
  int k = 0;
  CachedType basetype = temptype_basetype(seq->temptype);
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not.
 */
const TInstant *
tsequence_max_instant(const TSequence *seq)
{
  Datum max = tinstant_value(tsequence_inst_n(seq, 0));
  int k = 0;
  CachedType basetype = temptype_basetype(seq->temptype);
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum base value of the temporal value.
 */
Datum
tsequence_min_value(const TSequence *seq)
{
  if (seq->temptype == T_TINT)
  {
    TBOX *box = tsequence_bbox_ptr(seq);
    return Int32GetDatum((int)(box->xmin));
  }
  if (seq->temptype == T_TFLOAT)
  {
    TBOX *box = tsequence_bbox_ptr(seq);
    return Float8GetDatum(box->xmin);
  }
  Datum result = tinstant_value(tsequence_inst_n(seq, 0));
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum base value of the temporal value.
 */
Datum
tsequence_max_value(const TSequence *seq)
{
  if (seq->temptype == T_TINT)
  {
    TBOX *box = tsequence_bbox_ptr(seq);
    return Int32GetDatum((int)(box->xmax));
  }
  if (seq->temptype == T_TFLOAT)
  {
    TBOX *box = tsequence_bbox_ptr(seq);
    return Float8GetDatum(box->xmax);
  }
  Datum result = tinstant_value(tsequence_inst_n(seq, 0));
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the duration of the temporal value.
 */
Interval *
tsequence_duration(const TSequence *seq)
{
  return period_duration(&seq->period);
}

/**
 * Return the bounding period on which the temporal value is defined
 */
void
tsequence_period(const TSequence *seq, Period *p)
{
  period_set(seq->period.lower, seq->period.upper, seq->period.lower_inc,
    seq->period.upper_inc, p);
  return;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of the temporal value.
 */
TSequence **
tsequence_sequences(const TSequence *seq)
{
  TSequence **result = palloc(sizeof(TSequence *));
  result[0] = tsequence_copy(seq);
  return result;
}

/**
 * Return the array of segments of the temporal value
 */
int
tsequence_segments1(const TSequence *seq, TSequence **result)
{
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  TInstant *instants[2];
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  bool lower_inc = seq->period.lower_inc;
  TInstant *inst1, *inst2;
  int k = 0;
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 1; i < seq->count; i++)
  {
    inst1 = (TInstant *) tsequence_inst_n(seq, i - 1);
    inst2 = (TInstant *) tsequence_inst_n(seq, i);
    instants[0] = inst1;
    instants[1] = linear ? inst2 :
      tinstant_make(tinstant_value(inst1), inst2->t, seq->temptype);
    bool upper_inc;
    if (i == seq->count - 1 &&
      (linear || datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype)))
      upper_inc = seq->period.upper_inc;
    else
      upper_inc = false;
    result[k++] = tsequence_make((const TInstant **) instants, 2,
      lower_inc, upper_inc, linear, NORMALIZE_NO);
    if (! linear)
      pfree(instants[1]);
    lower_inc = true;
  }
  if (! linear && seq->period.upper)
  {
    inst1 = (TInstant *) tsequence_inst_n(seq, seq->count - 1);
    inst2 = (TInstant *) tsequence_inst_n(seq, seq->count - 2);
    if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), basetype))
      result[k++] = tsequence_make((const TInstant **) &inst1, 1,
        true, true, linear, NORMALIZE_NO);
  }
  return k;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of the temporal value.
 */
TSequence **
tsequence_segments(const TSequence *seq, int *count)
{
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);
  *count = tsequence_segments1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct instants of the temporal value.
 */
const TInstant **
tsequence_instants(const TSequence *seq)
{
  const TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = tsequence_inst_n(seq, i);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of the temporal value.
 */
TimestampTz
tsequence_start_timestamp(const TSequence *seq)
{
  return (tsequence_inst_n(seq, 0))->t;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of the temporal value.
 */
TimestampTz
tsequence_end_timestamp(const TSequence *seq)
{
  return (tsequence_inst_n(seq, seq->count - 1))->t;
}

/**
 * Return the array of timestamps of the temporal value
 */
int
tsequence_timestamps1(const TSequence *seq, TimestampTz *times)
{
  for (int i = 0; i < seq->count; i++)
    times[i] = tsequence_inst_n(seq, i)->t;
  return seq->count;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array timestamps of the temporal value.
 */
TimestampTz *
tsequence_timestamps(const TSequence *seq)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * seq->count);
  tsequence_timestamps1(seq, result);
  return result;
}

/**
 * Return the base value of the segment of the temporal value at the
 * timestamp
 *
 * @param[in] inst1,inst2 Temporal values defining the segment
 * @param[in] linear True when the segment has linear interpolation
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
  if (inst1->temptype == T_TNPOINT)
  {
    Npoint *np1 = DatumGetNpointP(value1);
    Npoint *np2 = DatumGetNpointP(value2);
    double pos = np1->pos + (double) ((long double)(np2->pos - np1->pos) * ratio);
    Npoint *result = npoint_make(np1->rid, pos);
    return PointerGetDatum(result);
  }
  elog(ERROR, "unknown interpolation function for continuous temporal type: %d",
    inst1->temptype);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base value of the temporal value at the timestamp.
 *
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Return true if the timestamp is contained in the temporal value
 */
bool
tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result)
{
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
  int n = tsequence_find_timestamp(seq, t);
  const TInstant *inst1 = tsequence_inst_n(seq, n);
  if (t == inst1->t)
    *result = tinstant_value_copy(inst1);
  else
  {
    const TInstant *inst2 = tsequence_inst_n(seq, n + 1);
    *result = tsegment_value_at_timestamp(inst1, inst2,
      MOBDB_FLAGS_GET_LINEAR(seq->flags), t);
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base value of the temporal value at the timestamp when the
 * timestamp may be at an exclusive bound.
 *
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Return true if the timestamp is found in the temporal value
 */
bool
tsequence_value_at_timestamp_inc(const TSequence *seq, TimestampTz t,
  Datum *result)
{
  const TInstant *inst = tsequence_inst_n(seq, 0);
  /* Instantaneous sequence or t is at lower bound */
  if (seq->count == 1 || inst->t == t)
    return tinstant_value_at_timestamp(inst, t, result);
  inst = tsequence_inst_n(seq, seq->count - 1);
  if (inst->t == t)
    return tinstant_value_at_timestamp(inst, t, result);
  return tsequence_value_at_timestamp(seq, t, result);
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever equal to the base value.
 */
bool
tsequence_ever_eq(const TSequence *seq, Datum value)
{
  int i;
  Datum value1;
  CachedType basetype = temptype_basetype(seq->temptype);

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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always equal to the base value.
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

  /* The following test assumes that the sequence is in normal form */
  if (seq->count > 2)
    return false;
  CachedType basetype = temptype_basetype(seq->temptype);
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
 * Return true if the segment of the temporal value with linear
 * interpolation is ever less than or equal to the base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_ever_le1(Datum value1, Datum value2, CachedType basetype,
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
 * Return true if the segment of the temporal value with linear
 * interpolation is always less than the base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] basetype Base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_always_lt1(Datum value1, Datum value2, CachedType basetype,
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than the base value.
 */
bool
tsequence_ever_lt(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, EVER))
    return false;

  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count; i++)
  {
    Datum valueinst = tinstant_value(tsequence_inst_n(seq, i));
    if (datum_lt(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than or equal to the
 * base value
 */
bool
tsequence_ever_le(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, EVER))
    return false;

  Datum value1;
  int i;
  CachedType basetype = temptype_basetype(seq->temptype);

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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than the base value.
 */
bool
tsequence_always_lt(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) seq, value, ALWAYS))
    return false;

  Datum value1;
  int i;
  CachedType basetype = temptype_basetype(seq->temptype);

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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than or equal to the
 * base value
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
  CachedType basetype = temptype_basetype(seq->temptype);
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
 * Restrict the segment of a temporal value to (the complement of) the base
 * value.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 * @param[in] inst1,inst2 Temporal values defining the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Number of resulting sequences returned
 */
static int
tsegment_restrict_value(const TInstant *inst1, const TInstant *inst2,
  bool linear, bool lower_inc, bool upper_inc, Datum value, bool atfunc,
  TSequence **result)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  CachedType basetype = temptype_basetype(inst1->temptype);
  TInstant *instants[2];
  /* Is the segment constant? */
  bool isconst = datum_eq(value1, value2, basetype);
  /* Does the lower bound belong to the answer? */
  bool lower = atfunc ? datum_eq(value1, value, basetype) :
    datum_ne(value1, value, basetype);
  /* Does the upper bound belong to the answer? */
  bool upper = atfunc ? datum_eq(value2, value, basetype) :
    datum_ne(value2, value, basetype);
  /* For linear interpolation and not constant segment is the
   * value in the interior of the segment? */
  Datum projvalue;
  TimestampTz t;
  bool inter = linear && !isconst && tlinearsegm_intersection_value(
    inst1, inst2, value, basetype, &projvalue, &t);

  /* Overall segment does not belong to the answer */
  if ((isconst && !lower) ||
    (!isconst && atfunc && linear && ((lower && !lower_inc) ||
      (upper && !upper_inc) || (!lower && !upper && !inter))))
    return 0;

  /* Segment belongs to the answer but bounds may not */
  if ((isconst && lower) ||
    /* Linear interpolation: Test of bounds */
    (!isconst && linear && !atfunc &&
    (!lower || !upper || (lower && upper && !inter))))
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc && lower, upper_inc && upper, linear, NORMALIZE_NO);
    return 1;
  }

  /* Stepwise interpolation */
  if (! linear)
  {
    int k = 0;
    if (lower)
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst2->t, inst1->temptype);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, linear, NORMALIZE_NO);
      pfree(instants[1]);
    }
    if (upper_inc && upper)
      result[k++] = tinstant_tsequence(inst2, linear);
    return k;
  }

  /* Linear interpolation: Test of bounds */
  if (atfunc && ((lower && lower_inc) || (upper && upper_inc)))
  {
    result[0] = tinstant_tsequence(lower ? inst1 : inst2, linear);
    return 1;
  }
  /* Interpolation */
  assert(inter);
  if (atfunc)
  {
    TInstant *inst = tinstant_make(projvalue, t, inst1->temptype);
    result[0] = tinstant_tsequence(inst, linear);
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
      instants[1] = tinstant_make(projvalue, t, inst1->temptype);
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
 * Restrict the temporal value to the base value
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set.
 * For this reason the bounding box and the instantaneous sequence sets are
 * repeated here.
 */
int
tsequence_restrict_value1(const TSequence *seq, Datum value, bool atfunc,
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
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Each iteration adds between 0 and 2 sequences */
    k += tsegment_restrict_value(inst1, inst2, linear, lower_inc, upper_inc,
      value, atfunc, &result[k]);
    inst1 = inst2;
    lower_inc = true;
  }
  return k;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the base value.
 *
 * @param[in] seq Temporal value
 * @param[in] value Base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @note There is no bounding box or instantaneous test in this function,
 * they are done in the atValue and minusValue functions since the latter are
 * called for each sequence in a sequence set or for each element in the array
 * for the atValues and minusValues functions.
 */
TSequenceSet *
tsequence_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tsequence_restrict_value1(seq, value, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the array of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
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
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  int k = 0;
  for (int i = 1; i < seq->count; i++)
  {
    inst2 = tsequence_inst_n(seq, i);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    for (int j = 0; j < count1; j++)
      /* Each iteration adds between 0 and 2 sequences */
      k += tsegment_restrict_value(inst1, inst2, linear, lower_inc,
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
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the array of base
 * values.
 *
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal sequence set value.
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function tsequence_at_values1 since the latter is called
 * for each composing sequence of a temporal sequence set number.
 */
TSequenceSet *
tsequence_restrict_values(const TSequence *seq, const Datum *values, int count,
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
    return tsequence_tsequenceset(seq);

  PeriodSet *ps1 = tsequenceset_time(atresult);
  PeriodSet *ps2 = minus_period_periodset(&seq->period, ps1);
  TSequenceSet *result = NULL;
  if (ps2 != NULL)
  {
    result = tsequence_restrict_periodset(seq, ps2, REST_AT);
    pfree(ps2);
  }
  pfree(atresult); pfree(ps1);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the segment of a temporal number to the (complement of the)
 * range of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequence is stored
 * @param[in] inst1,inst2 Temporal values defining the segment
 * @param[in] lower_inclu,upper_inclu Upper and lower bounds of the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal sequence value
 */
static int
tnumberseq_restrict_range1(const TInstant *inst1, const TInstant *inst2,
  bool linear, bool lower_inclu, bool upper_inclu, const RangeType *range,
  bool atfunc, TSequence **result)
{
  TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid,
    TYPECACHE_RANGE_INFO);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  CachedType basetype = temptype_basetype(inst1->temptype);
  TInstant *instants[2];
  bool contains;

  /* Constant segment (step or linear interpolation) */
  if (datum_eq(value1, value2, basetype))
  {
#if POSTGRESQL_VERSION_NUMBER < 130000
    contains = range_contains_elem_internal(typcache, (RangeType *) range, value1);
#else
    contains = range_contains_elem_internal(typcache, range, value1);
#endif
    if ((atfunc && ! contains) || (! atfunc && contains))
      return 0;
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inclu, upper_inclu, linear, NORMALIZE_NO);
    return 1;
  }

  /* Stepwise interpolation */
  if (! linear)
  {
    int k = 0;
#if POSTGRESQL_VERSION_NUMBER < 130000
    contains = range_contains_elem_internal(typcache, (RangeType *) range, value1);
#else
    contains = range_contains_elem_internal(typcache, range, value1);
#endif
    if ((atfunc && contains) || (! atfunc && ! contains))
    {
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst2->t, inst1->temptype);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inclu, false, linear, NORMALIZE_NO);
      pfree(instants[1]);
    }
#if POSTGRESQL_VERSION_NUMBER < 130000
    contains = range_contains_elem_internal(typcache, (RangeType *) range, value2);
#else
    contains = range_contains_elem_internal(typcache, range, value2);
#endif
    if (upper_inclu &&
      ((atfunc && contains) || (! atfunc && ! contains)))
    {
      result[k++] = tinstant_tsequence(inst2, linear);
    }
    return k;
  }

  /* Linear interpolation */
  bool lower_inc1, upper_inc1;
  bool increasing = DatumGetFloat8(value1) < DatumGetFloat8(value2);
  RangeType *valuerange = increasing ?
    range_make(value1, value2, lower_inclu, upper_inclu, basetype) :
    range_make(value2, value1, upper_inclu, lower_inclu, basetype);
  RangeType *intersect = DatumGetRangeTypeP(call_function2(range_intersect,
    PointerGetDatum(valuerange), PointerGetDatum(range)));
  pfree(valuerange);
  if (RangeIsEmpty(intersect))
  {
    pfree(intersect);
    if (atfunc)
      return 0;
    /* MINUS */
    instants[0] = (TInstant *) inst1;
    instants[1] = (TInstant *) inst2;
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inclu, upper_inclu, linear, NORMALIZE_NO);
    return 1;
  }

  /* We are sure that neither lower or upper are infinite */
  Datum lower = lower_datum(intersect);
  Datum upper = upper_datum(intersect);
  bool lower_inc2 = lower_inc(intersect);
  bool upper_inc2 = upper_inc(intersect);
  pfree(intersect);
  double dlower = DatumGetFloat8(lower);
  double dupper = DatumGetFloat8(upper);
  double dvalue1 = DatumGetFloat8(value1);
  double dvalue2 = DatumGetFloat8(value2);
  TimestampTz t1, t2;
  /* Intersection range is a single value */
  if (dlower == dupper)
  {
    if (atfunc)
    {
      t1 = (dlower == dvalue1) ? inst1->t : inst2->t;
      instants[0] = tinstant_make(lower, t1, inst1->temptype);
      result[0] = tinstant_tsequence(instants[0], linear);
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
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc1, upper_inc1, linear, NORMALIZE_NO);
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
      instants[i] = RANGE_ROUNDOFF ?
        tinstant_make(lower, t1, inst1->temptype) :
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
      instants[j] = RANGE_ROUNDOFF ?
        tinstant_make(upper, t2, inst1->temptype) :
        tsegment_at_timestamp(inst1, inst2, linear, t2);
    }

    /* Create the result */
    result[0] = tsequence_make((const TInstant **) instants, 2,
      lower_inc1, upper_inc1, linear, NORMALIZE_NO);
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
    instbounds[i] = RANGE_ROUNDOFF ?
      tinstant_make(lower, t1, inst1->temptype) :
      tsegment_at_timestamp(inst1, inst2, linear, t1);
  }
  if (dupper != dvalue1 && dupper != dvalue2)
  {
    tfloatsegm_intersection_value(inst1, inst2, upper, basetype, &t2);
    /* To reduce the roundoff errors we may take the bound instead of
     * projecting the value to the timestamp */
    instbounds[j] = RANGE_ROUNDOFF ?
      tinstant_make(upper, t2, inst1->temptype) :
      tsegment_at_timestamp(inst1, inst2, linear, t2);
  }

  /* Create the result */
  if (instbounds[0] == NULL && instbounds[1] == NULL)
  {
    if (lower_inclu && lower_inc1)
      result[k++] = tinstant_tsequence(inst1, linear);
    if (upper_inclu && upper_inc1)
      result[k++] = tinstant_tsequence(inst2, linear);
  }
  else if (instbounds[0] != NULL && instbounds[1] != NULL)
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = instbounds[0];
    result[k++] = tsequence_make((const TInstant **) instants, 2,
      lower_inclu, lower_inc1, linear, NORMALIZE_NO);
    instants[0] = instbounds[1];
    instants[1] = (TInstant *) inst2;
    result[k++] = tsequence_make((const TInstant **) instants, 2,
      upper_inc1, upper_inclu, linear, NORMALIZE_NO);
  }
  else if (instbounds[0] != NULL)
  {
    instants[0] = (TInstant *) inst1;
    instants[1] = instbounds[0];
    result[k++] = tsequence_make((const TInstant **) instants, 2,
      lower_inclu, lower_inc1, linear, NORMALIZE_NO);
    if (upper_inclu && upper_inc1)
      result[k++] = tinstant_tsequence(inst2, linear);
  }
  else /* if (instbounds[1] != NULL) */
  {
    if (lower_inclu && lower_inc1)
      result[k++] = tinstant_tsequence(inst1, linear);
    instants[0] = instbounds[1];
    instants[1] = (TInstant *) inst2;
    result[k++] = tsequence_make((const TInstant **) instants, 2,
      upper_inc1, upper_inclu, linear, NORMALIZE_NO);
  }

  for (i = 0; i < 2; i++)
  {
    if (instbounds[i])
      pfree(instbounds[i]);
  }
  return k;
}

/**
 * Restrict the temporal number to the (complement of the) range of
 * base values
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq temporal number
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumberseq_restrict_range2(const TSequence *seq, const RangeType *range,
  bool atfunc, TSequence **result)
{
  /* Bounding box test */
  TBOX box1, box2;
  tsequence_bbox(seq, &box1);
  range_tbox(range, &box2);
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
    TInstant *inst = tnumberinst_restrict_range(inst1, range, atfunc);
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
    k += tnumberseq_restrict_range1(inst1, inst2, linear, lower_inc, upper_inc,
      range, atfunc, &result[k]);
    inst1 = inst2;
    lower_inc = true;
  }
  return k;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the (complement of the) range of base
 * values.
 *
 * @param[in] seq Temporal number
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @note It is supposed that a bounding box test has been done in the dispatch
 * function. */
TSequenceSet *
tnumberseq_restrict_range(const TSequence *seq, const RangeType *range,
  bool atfunc)
{
  int count = seq->count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tnumberseq_restrict_range2(seq, range, atfunc, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * Restrict the temporal number to the (complement of the) array of ranges
 * of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @param[in] bboxtest True when the bounding box test should be performed
 * @return Number of resulting sequences returned
 * @pre The array of ranges is normalized
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tnumberseq_restrict_ranges1(const TSequence *seq, RangeType **normranges,
  int count, bool atfunc, bool bboxtest, TSequence **result)
{
  RangeType **newranges;
  int newcount;

  /* Bounding box test */
  if (bboxtest)
  {
    newranges = tnumber_bbox_restrict_ranges((Temporal *) seq, normranges,
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
    newranges = normranges;
    newcount = count;
  }

  const TInstant *inst1, *inst2;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    TInstant *inst = tnumberinst_restrict_ranges(inst1, newranges, newcount,
      atfunc);
    if (bboxtest)
      pfree(newranges);
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
        k += tnumberseq_restrict_range1(inst1, inst2, linear, lower_inc,
          upper_inc, newranges[j], REST_AT, &result[k]);
      }
      inst1 = inst2;
      lower_inc = true;
    }
    if (bboxtest)
      pfree(newranges);
    if (k > 1)
      tseqarr_sort(result, k);
    return k;
  }
  else
  {
    /*
     * MINUS function
     * Compute first the tnumberseq_at_ranges, then compute its complement
     * Notice that in this case due to rounoff errors it may be the case
     * that temp is not equal to merge(atRanges(temp, .),minusRanges(temp, .),
     * since we kept the range values instead of the projected values when
     * computing atRanges
     */
    TSequenceSet *ts = tnumberseq_restrict_ranges(seq, newranges, newcount,
      REST_AT, bboxtest);
    if (ts == NULL)
    {
      result[0] = tsequence_copy(seq);
      return 1;
    }

    PeriodSet *ps1 = tsequenceset_time(ts);
    PeriodSet *ps2 = minus_period_periodset(&seq->period, ps1);
    int newcount = 0;
    if (ps2 != NULL)
    {
      newcount = tsequence_at_periodset(seq, ps2, result);
      pfree(ps2);
    }
    pfree(ts); pfree(ps1);
    if (bboxtest)
      pfree(newranges);
    return newcount;
  }
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to (the complement of) the array of
 * ranges of base values
 *
 * @param[in] seq Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @param[in] bboxtest True when the bounding box test should be performed
 * @return Resulting temporal number
 * @pre The array of ranges is normalized
 * @note A bounding box test and an instantaneous sequence test are done in
 * the function tnumberseq_restrict_ranges1 since the latter is called
 * for each composing sequence of a temporal sequence set number.
 */
TSequenceSet *
tnumberseq_restrict_ranges(const TSequence *seq, RangeType **normranges,
  int count, bool atfunc, bool bboxtest)
{
  /* General case */
  int maxcount = seq->count * count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(seq->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int newcount = tnumberseq_restrict_ranges1(seq, normranges, count, atfunc,
    bboxtest, sequences);
  return tsequenceset_make_free(sequences, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the minimum/maximum
 * base value.
 */
TSequenceSet *
tsequence_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  Datum minmax = min ? tsequence_min_value(seq) : tsequence_max_value(seq);
  return tsequence_restrict_value(seq, minmax, atfunc);
}

/*****************************************************************************/

/**
 * Restrict the segment of a temporal value to the timestamp
 *
 * @param[in] inst1,inst2 Temporal values defining the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] t Timestamp
 * @pre The timestamp t satisfies inst1->t <= t <= inst2->t
 * @note The function creates a new value that must be freed
 */
TInstant *
tsegment_at_timestamp(const TInstant *inst1, const TInstant *inst2,
  bool linear, TimestampTz t)
{
  Datum value = tsegment_value_at_timestamp(inst1, inst2, linear, t);
  TInstant *result = tinstant_make(value, t, inst1->temptype);
  DATUM_FREE(value, temptype_basetype(inst1->temptype));
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the timestamp.
 */
TInstant *
tsequence_at_timestamp(const TSequence *seq, TimestampTz t)
{
  /* Bounding box test */
  if (! contains_period_timestamp(&seq->period, t))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tinstant_copy(tsequence_inst_n(seq, 0));

  /* General case */
  int n = tsequence_find_timestamp(seq, t);
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

/*****************************************************************************/

/**
 * Restrict the temporal value to the complement of the timestamp
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tsequence_minus_timestamp1(const TSequence *seq, TimestampTz t,
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
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  int i, k = 0;
  int n = tsequence_find_timestamp(seq, t);
  /* Compute the first sequence until t */
  if (n != 0 || inst1->t < t)
  {
    for (i = 0; i < n; i++)
      instants[i] = (TInstant *) tsequence_inst_n(seq, i);
    inst1 = tsequence_inst_n(seq, n);
    inst2 = tsequence_inst_n(seq, n + 1);
    if (inst1->t == t)
    {
      if (linear)
      {
        instants[n] = (TInstant *) inst1;
        result[k++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, linear, NORMALIZE_NO);
      }
      else
      {
        instants[n] = tinstant_make(tinstant_value(instants[n - 1]), t,
          inst1->temptype);
        result[k++] = tsequence_make((const TInstant **) instants, n + 1,
          seq->period.lower_inc, false, linear, NORMALIZE_NO);
        pfree(instants[n]);
      }
    }
    else
    {
      /* inst1->t < t */
      instants[n] = (TInstant *) inst1;
      instants[n + 1] = linear ?
        tsegment_at_timestamp(inst1, inst2, true, t) :
        tinstant_make(tinstant_value(inst1), t, inst1->temptype);
      result[k++] = tsequence_make((const TInstant **) instants, n + 2,
        seq->period.lower_inc, false, linear, NORMALIZE_NO);
      pfree(instants[n + 1]);
    }
  }
  /* Compute the second sequence after t */
  inst1 = tsequence_inst_n(seq, n);
  inst2 = tsequence_inst_n(seq, n + 1);
  if (t < inst2->t)
  {
    instants[0] = tsegment_at_timestamp(inst1, inst2, linear, t);
    for (i = 1; i < seq->count - n; i++)
      instants[i] = (TInstant *) tsequence_inst_n(seq, i + n);
    result[k++] = tsequence_make((const TInstant **) instants, seq->count - n,
      false, seq->period.upper_inc, linear, NORMALIZE_NO);
    pfree(instants[0]);
  }
  return k;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the timestamp.
 *
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @return Resulting temporal sequence set
 */
TSequenceSet *
tsequence_minus_timestamp(const TSequence *seq, TimestampTz t)
{
  TSequence *sequences[2];
  int count = tsequence_minus_timestamp1(seq, t, sequences);
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
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the timestamp set.
 */
TInstantSet *
tsequence_at_timestampset(const TSequence *seq, const TimestampSet *ts)
{
  TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    inst = tsequence_at_timestamp(seq, timestampset_time_n(ts, 0));
    if (inst == NULL)
      return NULL;
    return tinstantset_make((const TInstant **) &inst, 1, MERGE_NO);
  }

  /* Bounding box test */
  const Period *p = timestampset_bbox_ptr(ts);
  if (! overlaps_period_period(&seq->period, p))
    return NULL;

  inst = (TInstant *) tsequence_inst_n(seq, 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (! contains_timestampset_timestamp(ts, inst->t))
      return NULL;
    return tinstantset_make((const TInstant **) &inst, 1, MERGE_NO);
  }

  /* General case */
  TimestampTz t = Max(seq->period.lower, p->lower);
  int loc;
  timestampset_find_timestamp(ts, t, &loc);
  TInstant **instants = palloc(sizeof(TInstant *) * (ts->count - loc));
  int k = 0;
  for (int i = loc; i < ts->count; i++)
  {
    t = timestampset_time_n(ts, i);
    inst = tsequence_at_timestamp(seq, t);
    if (inst != NULL)
      instants[k++] = inst;
  }
  return tinstantset_make_free(instants, k, MERGE_NO);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the complement of the timestamp set
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ts Timestampset
 * @return Number of resulting sequences returned
 */
int
tsequence_minus_timestampset1(const TSequence *seq, const TimestampSet *ts,
  TSequence **result)
{
  /* Singleton timestamp set */
  if (ts->count == 1)
    return tsequence_minus_timestamp1(seq, timestampset_time_n(ts, 0),
      result);

  /* Bounding box test */
  const Period *p = timestampset_bbox_ptr(ts);
  if (! overlaps_period_period(&seq->period, p))
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
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
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
      if (linear)
      {
        instants[l] = (TInstant *) inst;
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          lower_inc, false, linear, NORMALIZE_NO);
        instants[0] = (TInstant *) inst;
      }
      else
      {
        instants[l] = tinstant_make(tinstant_value(instants[l - 1]),
          t, inst->temptype);
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          lower_inc, false, linear, NORMALIZE_NO);
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
        instants[l] = linear ?
          tsegment_at_timestamp(instants[l - 1], inst, true, t) :
          tinstant_make(tinstant_value(instants[l - 1]), t, inst->temptype);
        result[k++] = tsequence_make((const TInstant **) instants, l + 1,
          lower_inc, false, linear, NORMALIZE_NO);
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
    result[k++] = tsequence_make((const TInstant **) instants, l,
      false, seq->period.upper_inc, linear, NORMALIZE_NO);
  }
  if (tofree)
    pfree(tofree);
  return k;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the timestamp set.
 */
TSequenceSet *
tsequence_minus_timestampset(const TSequence *seq, const TimestampSet *ts)
{
  TSequence **sequences = palloc0(sizeof(TSequence *) * (ts->count + 1));
  int count = tsequence_minus_timestampset1(seq, ts, sequences);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the period.
 */
TSequence *
tsequence_at_period(const TSequence *seq, const Period *p)
{
  /* Bounding box test */
  Period inter;
  if (! inter_period_period(&seq->period, p, &inter))
    return NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tsequence_copy(seq);

  /* General case */
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  TSequence *result;
  /* Intersecting period is instantaneous */
  if (inter.lower == inter.upper)
  {
    TInstant *inst = tsequence_at_timestamp(seq, inter.lower);
    result = tinstant_tsequence(inst, linear);
    pfree(inst);
    return result;
  }

  const TInstant *inst1, *inst2;
  int n = tsequence_find_timestamp(seq, inter.lower);
  /* If the lower bound of the intersecting period is exclusive */
  if (n == -1)
    n = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * (seq->count - n));
  /* Compute the value at the beginning of the intersecting period */
  inst1 = tsequence_inst_n(seq, n);
  inst2 = tsequence_inst_n(seq, n + 1);
  instants[0] = tsegment_at_timestamp(inst1, inst2, linear, inter.lower);
  int k = 1;
  for (int i = n + 2; i < seq->count; i++)
  {
    /* If the end of the intersecting period is between inst1 and inst2 */
    if (inst1->t <= inter.upper && inter.upper <= inst2->t)
      break;

    inst1 = inst2;
    inst2 = tsequence_inst_n(seq, i);
    /* If the intersecting period contains inst1 */
    if (inter.lower <= inst1->t && inst1->t <= inter.upper)
      instants[k++] = (TInstant *) inst1;
  }
  /* The last two values of sequences with step interpolation and
   * exclusive upper bound must be equal */
  if (linear || inter.upper_inc)
    instants[k++] = tsegment_at_timestamp(inst1, inst2, linear, inter.upper);
  else
  {
    Datum value = tinstant_value(instants[k - 1]);
    instants[k++] = tinstant_make(value, inter.upper, seq->temptype);
  }
  /* Since by definition the sequence is normalized it is not necessary to
   * normalize the projection of the sequence to the period */
  result = tsequence_make((const TInstant **) instants, k,
    inter.lower_inc, inter.upper_inc, linear, NORMALIZE_NO);

  pfree(instants[0]); pfree(instants[k - 1]); pfree(instants);

  return result;
}

/**
 * Restrict the temporal value to the complement of the period.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] p Period
 * @return Number of resulting sequences returned
 */
int
tsequence_minus_period1(const TSequence *seq, const Period *p,
  TSequence **result)
{
  /* Bounding box test */
  if (! overlaps_period_period(&seq->period, p))
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
    result[i] = tsequence_at_period(seq, p1);
  }
  pfree(ps);
  return ps->count;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the period.
 */
TSequenceSet *
tsequence_minus_period(const TSequence *seq, const Period *p)
{
  TSequence *sequences[2];
  int count = tsequence_minus_period1(seq, p, sequences);
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
 * Restrict the temporal value to the period set
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @return Number of resulting sequences returned
 * @note This function is not called for each sequence of a temporal sequence
 * set but is called when computing tpointseq minus geometry
*/
int
tsequence_at_periodset(const TSequence *seq, const PeriodSet *ps,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
  {
    result[0] = tsequence_at_period(seq, periodset_per_n(ps, 0));
    return 1;
  }

  /* Bounding box test */
  const Period *p = periodset_bbox_ptr(ps);
  if (! overlaps_period_period(&seq->period, p))
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
    p = periodset_per_n(ps, i);
    TSequence *seq1 = tsequence_at_period(seq, p);
    if (seq1 != NULL)
      result[k++] = seq1;
    if (seq->period.upper < p->upper)
      break;
  }
  return k;
}

/**
 * Restrict the temporal value to the complement of the period set
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @param[in] from Index from which the processing starts
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
*/
int
tsequence_minus_periodset(const TSequence *seq, const PeriodSet *ps, int from,
  TSequence **result)
{
  /* Singleton period set */
  if (ps->count == 1)
    return tsequence_minus_period1(seq, periodset_per_n(ps, 0), result);

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
    int cmp = timestamp_cmp_internal(curr->period.upper, p1->lower);
    if (cmp < 0 || (cmp == 0 && curr->period.upper_inc && ! p1->lower_inc))
    {
      result[k++] = curr;
      break;
    }
    TSequence *minus[2];
    int countminus = tsequence_minus_period1(curr, p1, minus);
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
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) period set.
 *
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal sequence set
 */
TSequenceSet *
tsequence_restrict_periodset(const TSequence *seq, const PeriodSet *ps,
  bool atfunc)
{
  /* Bounding box test */
  const Period *p = periodset_bbox_ptr(ps);
  if (! overlaps_period_period(&seq->period, p))
    return atfunc ? NULL : tsequence_tsequenceset(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = tsequence_inst_n(seq, 0);
    if (contains_periodset_timestamp(ps, inst->t))
      return atfunc ? tsequence_tsequenceset(seq) : NULL;
    return atfunc ? NULL : tsequence_tsequenceset(seq);
  }

  /* General case */
  int count = atfunc ? ps->count : ps->count + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int count1 = atfunc ? tsequence_at_periodset(seq, ps, sequences) :
    tsequence_minus_periodset(seq, ps, 0, sequences);
  return tsequenceset_make_free(sequences, count1, NORMALIZE_NO);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp.
 */
bool
tsequence_intersects_timestamp(const TSequence *seq, TimestampTz t)
{
  return contains_period_timestamp(&seq->period, t);
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp set.
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
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period.
 */
bool
tsequence_intersects_period(const TSequence *seq, const Period *p)
{
  return overlaps_period_period(&seq->period, p);
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period set.
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
 * @ingroup libmeos_temporal_agg
 * @brief Return the integral (area under the curve) of the temporal number.
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
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of the temporal number.
 */
double
tnumberseq_twavg(const TSequence *seq)
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
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal sequence values are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tsequence_eq(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1->temptype == seq2->temptype);
  /* If number of sequences, flags, or periods are not equal */
  if (seq1->count != seq2->count || seq1->flags != seq2->flags ||
      ! period_eq(&seq1->period, &seq2->period))
    return false;

  /* If bounding boxes are not equal */
  void *box1 = tsequence_bbox_ptr(seq1);
  void *box2 = tsequence_bbox_ptr(seq2);
  if (! temporal_bbox_eq(box1, box2, seq1->temptype))
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
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function temporal_cmp
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
 * the elements and the approach for range types for combining the period
 * bounds.
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the hash value of the temporal value.
 */
uint32
tsequence_hash(const TSequence *seq)
{
  uint32 result;
  char flags = '\0';

  /* Create flags from the lower_inc and upper_inc values */
  if (seq->period.lower_inc)
    flags |= 0x01;
  if (seq->period.upper_inc)
    flags |= 0x02;
  result = DatumGetUInt32(hash_uint32((uint32) flags));

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
