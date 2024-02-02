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
 * @brief General functions for temporal sequence sets
 */

#include "general/tsequenceset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/spanset.h"
#include "general/tsequence.h"
#include "general/temporal_boxops.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return the location of a timestamp in a temporal sequence set using
 * binary search
 * @details If the timestamp is contained in the temporal sequence set, the
 * index of the sequence is returned in the output parameter. Otherwise,
 * returns a number encoding whether the timestamp is before, between two
 * sequences, or after the temporal sequence set.
 *
 * For example, given a value composed of 3 sequences
 * and a timestamp, the value returned in the output parameter is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|
 * 1)    t^                                         => loc = 0
 * 2)                 t^                            => loc = 1
 * 3)                       t^                      => loc = 1
 * 4)                             t^                => loc = 2
 * 5)                                         t^    => loc = 3
 * @endcode
 * @param[in] ss Temporal sequence set
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the temporal sequence
 * set.
 */
bool
tsequenceset_find_timestamptz(const TSequenceSet *ss, TimestampTz t, int *loc)
{
  int first = 0, last = ss->count - 1;
  int middle = 0; /* make compiler quiet */
  const TSequence *seq = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    seq = TSEQUENCESET_SEQ_N(ss, middle);
    if (contains_span_timestamptz(&seq->period, t))
    {
      *loc = middle;
      return true;
    }
    if (t <= DatumGetTimestampTz(seq->period.lower))
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (seq && t >= DatumGetTimestampTz(seq->period.upper))
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Constructor functions
 * ---------------------
 * There are two main constructor functions for temporal sequence sets
 * - #tsequenceset_make_exp: Constructs a sequence set from an array of
 *   sequences
 * - #tsequenceset_make_gaps: Constructs a sequence set from an array of
 *   instants where the composing sequences are determined by space or time
 *   gaps between consecutive instants
 * In the two cases, it is necessary to verify the validity of the argument
 * array and compute the bounding box of the resulting sequence set. In the
 * first constructor above, the computation of the bounding box is done while
 * verifying the validity of the instants to avoid an additional iteration over
 * the instants. This is not possible for the second constructor, since the
 * loop for verifying the validity of the instants is used for determining the
 * splits for constructing the composing sequences.
 *****************************************************************************/

/**
 * @brief Ensure that all temporal sequences of the array have increasing
 * timestamp, and if they are temporal points, have the same srid and the
 * same dimensionality
 */
static bool
ensure_valid_tseqarr(const TSequence **sequences, int count)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(sequences[0]->flags);
  if (interp == DISCRETE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Input sequences must be continuous");
    return false;
  }
  for (int i = 0; i < count; i++)
  {
    if (sequences[i]->subtype != TSEQUENCE)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Input values must be temporal sequences");
      return false;
    }
    if (i > 0)
    {
      if (MEOS_FLAGS_GET_INTERP(sequences[i]->flags) != interp)
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "The temporal values must have the same interpolation");
        return false;
      }
      TimestampTz upper1 = DatumGetTimestampTz(sequences[i - 1]->period.upper);
      TimestampTz lower2 = DatumGetTimestampTz(sequences[i]->period.lower);
      if ( upper1 > lower2 ||
           ( upper1 == lower2 && sequences[i - 1]->period.upper_inc &&
             sequences[i]->period.lower_inc ) )
      {
        char *t1 = pg_timestamptz_out(upper1);
        char *t2 = pg_timestamptz_out(lower2);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Timestamps for temporal value must be increasing: %s, %s", t1, t2);
        return false;
      }
      if (! ensure_spatial_validity((Temporal *) sequences[i - 1],
          (Temporal *) sequences[i]))
        return false;
    }
  }
  return true;
}

#ifdef DEBUG_BUILD
/**
 * @brief Function version of the the macro of the same name for debugging
 * purposes
 */
size_t *
TSEQUENCESET_OFFSETS_PTR(const TSequenceSet *ss)
{
  return (size_t *)( ((char *) &ss->period) + ss->bboxsize );
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the n-th sequence of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] i Index
 * @note The period component of the bbox is already declared in the struct
 * @pre The argument @p i is less than the number of sequences in the
 * sequence set
 */
const TSequence *
TSEQUENCESET_SEQ_N(const TSequenceSet *ss, int i)
{
  return (TSequence *)(
    ((char *) &ss->period) + ss->bboxsize +
    (sizeof(size_t) * ss->maxcount) + (TSEQUENCESET_OFFSETS_PTR(ss))[i] );
}
#endif /* DEBUG_BUILD */

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal sequences
 * @details For example, the memory structure of a temporal sequence set with
 * two sequences is as follows
 * @code
 * ------------------------------------------------------------
 * ( TSequenceSet )_X | ( bbox )_X | offset_0 | offset_1 | ...
 * ------------------------------------------------------------
 * ---------------------------------------
 * ( TSequence_0 )_X | ( TSequence_1 )_X |
 * ---------------------------------------
 * @endcode
 * where the @p _X are unused bytes added for double padding, @p offset_0 and
 * @p offset_1 are offsets for the corresponding sequences.
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two temporal sequence
 * sets before applying an operation to them.
 */
TSequenceSet *
tsequenceset_make_exp(const TSequence **sequences, int count, int maxcount,
  bool normalize)
{
  assert(sequences); assert(count > 0); assert(count <= maxcount);
  /* Ensure validity of the arguments */
  if (! ensure_valid_tseqarr(sequences, count))
    return NULL;

  /* Normalize the array of sequences */
  TSequence **normseqs = (TSequence **) sequences;
  int newcount = count;
  if (normalize && count > 1)
    normseqs = tseqarr_normalize(sequences, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(sequences[0]->temptype));
  /* The period component of the bbox is already declared in the struct */
  size_t bboxsize_extra = bboxsize - sizeof(Span);

  /* Compute the size of the temporal sequence set */
  size_t seqs_size = 0;
  int totalcount = 0;
  for (int i = 0; i < newcount; i++)
  {
    totalcount += normseqs[i]->count;
    seqs_size += DOUBLE_PAD(VARSIZE(normseqs[i]));
  }
  /* Compute the total size for maxcount sequences as a proportion of the size
   * of the count sequences provided. Note that this is only an initial
   * estimation. The functions adding sequences to a sequence set must verify
   * both the maximum number of sequences and the remaining space for adding an
   * additional variable-length sequences of arbitrary size */
  if (count != maxcount)
    seqs_size = DOUBLE_PAD((size_t) ((double) seqs_size * maxcount / count));
  else
    maxcount = newcount;
  /* Size of the struct and the offset array */
  size_t memsize = DOUBLE_PAD(sizeof(TSequenceSet)) + bboxsize_extra +
    sizeof(size_t) * maxcount + seqs_size;

  /* Create the temporal sequence set */
  TSequenceSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->maxcount = maxcount;
  result->totalcount = totalcount;
  result->temptype = sequences[0]->temptype;
  result->subtype = TSEQUENCESET;
  result->bboxsize = (int16) bboxsize;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags,
    MEOS_FLAGS_GET_CONTINUOUS(sequences[0]->flags));
  MEOS_FLAGS_SET_INTERP(result->flags,
    MEOS_FLAGS_GET_INTERP(sequences[0]->flags));
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(sequences[0]->temptype))
  {
    MEOS_FLAGS_SET_Z(result->flags, MEOS_FLAGS_GET_Z(sequences[0]->flags));
    MEOS_FLAGS_SET_GEODETIC(result->flags,
      MEOS_FLAGS_GET_GEODETIC(sequences[0]->flags));
  }
  /* Initialization of the variable-length part */
  /* Compute the bounding box */
  tseqarr_compute_bbox((const TSequence **) normseqs, newcount,
    TSEQUENCESET_BBOX_PTR(result));
  /* Store the composing sequences */
  size_t pdata = DOUBLE_PAD(sizeof(TSequenceSet)) + bboxsize_extra +
    sizeof(size_t) * maxcount;
  size_t pos = 0;
  for (int i = 0; i < newcount; i++)
  {
    memcpy(((char *) result) + pdata + pos, normseqs[i],
      VARSIZE(normseqs[i]));
    (TSEQUENCESET_OFFSETS_PTR(result))[i] = pos;
    pos += DOUBLE_PAD(VARSIZE(normseqs[i]));
  }
  if (normalize && count > 1)
    pfree_array((void **) normseqs, newcount);
  return result;
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal sequences
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence sets before applying an operation to them.
 * @csqlfn #Tsequenceset_constructor()
 */
TSequenceSet *
tsequenceset_make(const TSequence **sequences, int count, bool normalize)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) sequences) || ! ensure_positive(count))
    return NULL;
  return tsequenceset_make_exp(sequences, count, count, normalize);
}

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal
 * sequences and free the array and the sequences after the creation
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @see #tsequenceset_make
 */
TSequenceSet *
tsequenceset_make_free(TSequence **sequences, int count, bool normalize)
{
  assert(sequences);
  if (count == 0)
  {
    pfree(sequences);
    return NULL;
  }
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, normalize);
  pfree_array((void **) sequences, count);
  return result;
}

/**
 * @brief Ensure that all temporal instants of the array have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality
 * @details This function extends function #ensure_valid_tinstarr by
 * determining the splits that must be made according the maximum distance or
 * interval between consecutive instants.
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] merge True if a merge operation, which implies that the two
 *   consecutive instants may be equal
 * @param[in] maxdist Maximum distance to split the temporal sequence
 * @param[in] maxt Maximum time interval to split the temporal sequence
 * @param[out] nsplits Number of splits
 * @result Array of indices at which the temporal sequence is split
 */
static int *
ensure_valid_tinstarr_gaps(const TInstant **instants, int count, bool merge,
  double maxdist, Interval *maxt, int *nsplits)
{
  meosType basetype = temptype_basetype(instants[0]->temptype);
  /* Ensure that zero-fill is done */
  int *result = palloc0(sizeof(int) * count);
  Datum value1 = tinstant_val(instants[0]);
  int16 flags = instants[0]->flags;
  int k = 0;
  for (int i = 1; i < count; i++)
  {
    if (! ensure_increasing_timestamps(instants[i - 1], instants[i], merge) ||
        ! ensure_spatial_validity((Temporal *) instants[i - 1],
          (Temporal *) instants[i]))
      return NULL;
#if NPOINT
    if (instants[i]->temptype == T_TNPOINT &&
        ! ensure_same_rid_tnpointinst(instants[i - 1], instants[i]))
      return NULL;
#endif
    /* Determine if there should be a split */
    bool split = false;
    Datum value2 = tinstant_val(instants[i]);
    if (maxdist > 0.0 && ! datum_eq(value1, value2, basetype))
    {
      double dist = datum_distance(value1, value2, basetype, flags);
      if (dist > maxdist)
        split = true;
    }
    /* If there is not already a split by distance */
    if (maxt != NULL && ! split)
    {
      Interval *duration = minus_timestamptz_timestamptz(instants[i]->t,
        instants[i - 1]->t);
      if (pg_interval_cmp(duration, maxt) > 0)
        split = true;
      // CANNOT pfree(duration);
    }
    if (split)
      result[k++] = i;
    value1 = value2;
  }
  *nsplits = k;
  return result;
}

/**
 * @brief Ensure the validity of the arguments when creating a temporal value
 * This function extends function #tsequence_make_valid by spliting the
 * sequences according the maximum distance or interval between instants
 */
static int *
tsequenceset_make_gaps_valid(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, double maxdist,
  Interval *maxt, int *nsplits)
{
  assert(interp != DISCRETE);
  if (! ensure_valid_tinstarr_common(instants, count, lower_inc, upper_inc,
      interp))
    return NULL;
  return ensure_valid_tinstarr_gaps(instants, count, MERGE_NO, maxdist, maxt,
    nsplits);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal instants
 * introducing a gap when two consecutive instants are separated from each
 * other by at least a distance or a time interval
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap
 * @csqlfn #Tsequenceset_constructor_gaps()
 */
TSequenceSet *
tsequenceset_make_gaps(const TInstant **instants, int count, interpType interp,
  Interval *maxt, double maxdist)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) instants) || ! ensure_positive(count))
    return NULL;

  TSequence *seq;
  TSequenceSet *result;

  /* If no gaps are given call the standard sequence constructor */
  if (maxt == NULL && maxdist <= 0.0)
  {
    seq = tsequence_make_exp((const TInstant **) instants, count, count, true,
      true, interp, NORMALIZE);
    result = tsequenceset_make_exp((const TSequence **) &seq, 1, 1,
      NORMALIZE_NO);
    pfree(seq);
    return result;
  }

  /* Ensure that the array of instants is valid and determine the splits */
  int nsplits = 0;
  int *splits = tsequenceset_make_gaps_valid((const TInstant **) instants,
    count, true, true, interp, maxdist, maxt, &nsplits);
  if (! splits)
    return NULL;
  if (nsplits == 0)
  {
    /* There are no gaps  */
    pfree(splits);
    seq = tsequence_make_exp1((const TInstant **) instants, count, count, true,
      true, interp, NORMALIZE, NULL);
    result = tsequenceset_make((const TSequence **) &seq, 1, NORMALIZE_NO);
    pfree(seq);
  }
  else
  {
    int nseqs = 0;
    /* Split according to gaps  */
    const TInstant **newinsts = palloc(sizeof(TInstant *) * count);
    TSequence **sequences = palloc(sizeof(TSequence *) * (nsplits + 1));
    int j = 0, ninsts = 0;
    for (int i = 0; i < count; i++)
    {
      if (j < nsplits && splits[j] == i)
      {
        /* Finalize the current sequence and start a new one */
        assert(ninsts > 0);
        sequences[nseqs++] = tsequence_make_exp1((const TInstant **) newinsts,
          ninsts, ninsts, true, true, interp, NORMALIZE, NULL);
        j++; ninsts = 0;
      }
      /* Continue with the current sequence */
      newinsts[ninsts++] = instants[i];
    }
    /* Construct the last sequence */
    if (ninsts > 0)
      sequences[nseqs++] = tsequence_make_exp1((const TInstant **) newinsts,
        ninsts, ninsts, true, true, interp, NORMALIZE, NULL);
    result = tsequenceset_make((const TSequence **) sequences, nseqs,
      NORMALIZE);
    pfree(newinsts); pfree(sequences);
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a copy of a temporal sequence set
 * @param[in] ss Temporal sequence set
 */
TSequenceSet *
tsequenceset_copy(const TSequenceSet *ss)
{
  assert(ss);
  TSequenceSet *result = palloc0(VARSIZE(ss));
  memcpy(result, ss, VARSIZE(ss));
  return result;
}

/**
 * @brief Return an array of temporal sequence sets converted into an array of
 * temporal sequences
 * @details This function is called by all the functions in which the number of
 * output sequences cannot be determined in advance, typically when each
 * segment of the input sequence can produce an arbitrary number of output
 * sequences, as in the case of @p atGeometries.
 * @param[in] seqsets Array of array of temporal sequence sets
 * @param[in] count Number of elements in the input array
 * @param[in] totalseqs Number of elements in the output array
 * @pre The arguments @p count and @p totalseqs are greater than 0
 */
TSequenceSet *
tseqsetarr_to_tseqset(TSequenceSet **seqsets, int count, int totalseqs)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * totalseqs);
  int nseqs = 0;
  for (int i = 0; i < count; i++)
  {
    TSequenceSet *ss1 = seqsets[i];
    if (ss1)
    {
      for (int j = 0; j < ss1->count; j++)
        sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss1, j);
    }
  }
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    nseqs, NORMALIZE);
  pfree(sequences);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a temporal sequence set from a base value and a span set
 * @param[in] value Value
 * @param[in] temptype Temporal type
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tsequenceset_from_base_tstzspanset(Datum value, meosType temptype,
  const SpanSet *ss, interpType interp)
{
  assert(ss);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_from_base_tstzspan(value, temptype,
      SPANSET_SP_N(ss, i), interp);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean sequence set from a boolean and a
 * timestamptz span set
 * @param[in] b Value
 * @param[in] ss Span set
 */
TSequenceSet *
tboolseqset_from_base_tstzspanset(bool b, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return tsequenceset_from_base_tstzspanset(BoolGetDatum(b), T_TBOOL, ss, STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer sequence set from an integer and a
 * timestamptz span set
 * @param[in] i Value
 * @param[in] ss Span set
 */
TSequenceSet *
tintseqset_from_base_tstzspanset(int i, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return tsequenceset_from_base_tstzspanset(Int32GetDatum(i), T_TINT, ss, STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float sequence set from a float and a timestamptz
 * span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tfloatseqset_from_base_tstzspanset(double d, const SpanSet *ss, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return tsequenceset_from_base_tstzspanset(Float8GetDatum(d), T_TFLOAT, ss,
    interp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text sequence set from a text and a timestamptz
 * span set
 * @param[in] txt Value
 * @param[in] ss Span set
 */
TSequenceSet *
ttextseqset_from_base_tstzspanset(const text *txt, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt) || ! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(txt), T_TTEXT, ss,
    STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal geometry point sequence set from a point and a
 * timestamptz span set
 * @param[in] gs Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tpointseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss,
  interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_empty(gs) ||
      ! ensure_point_type(gs) || ! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(gs), temptype, ss,
    interp);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of (pointers to the) distinct values of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 * @result Array of Datums
 * @csqlfn #Temporal_valueset()
 */
Datum *
tsequenceset_vals(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count);
  Datum *result = palloc(sizeof(Datum *) * ss->totalcount);
  int nvals = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
      result[nvals++] = tinstant_val(TSEQUENCE_INST_N(seq, j));
  }
  if (nvals > 1)
  {
    meosType basetype = temptype_basetype(ss->temptype);
    datumarr_sort(result, nvals, basetype);
    nvals = datumarr_remove_duplicates(result, nvals, basetype);
  }
  *count = nvals;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the base values of a temporal number sequence set as a number
 * span set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tnumber_valuespans()
 */
SpanSet *
tnumberseqset_valuespans(const TSequenceSet *ss)
{
  assert(ss);
  int count, i;
  Span *spans;

  /* Temporal sequence number with linear interpolation */
  if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
  {
    spans = palloc(sizeof(Span) * ss->count);
    for (i = 0; i < ss->count; i++)
    {
      TBox *box = TSEQUENCE_BBOX_PTR(TSEQUENCESET_SEQ_N(ss, i));
      memcpy(&spans[i], &box->span, sizeof(Span));
    }
    return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED_NO);
  }

  /* Temporal sequence number with discrete or step interpolation */
  meosType basetype = temptype_basetype(ss->temptype);
  meosType spantype = basetype_spantype(basetype);
  Datum *values = tsequenceset_vals(ss, &count);
  spans = palloc(sizeof(Span) * count);
  for (i = 0; i < count; i++)
    span_set(values[i], values[i], true, true, basetype, spantype, &spans[i]);
  SpanSet *result = spanset_make_free(spans, count, NORMALIZE, ORDERED_NO);
  pfree(values);
  return result;
}

/**
 * @brief Return a pointer to the instant with minimum/maximum base value of a
 * temporal sequence set
 * @details The function does not take into account whether the instant is at
 * an exclusive bound or not.
 */
const TInstant *
tsequenceset_minmax_inst(const TSequenceSet *ss,
  bool (*func)(Datum, Datum, meosType))
{
  assert(ss);
  const TSequence *seq = TSEQUENCESET_SEQ_N(ss, 0);
  const TInstant *result = TSEQUENCE_INST_N(seq, 0);
  Datum min = tinstant_val(result);
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      Datum value = tinstant_val(inst);
      if (func(value, min, basetype))
      {
        min = value;
        result = inst;
      }
    }
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal sequence set
 * @details The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @param[in] ss Temporal sequence set
 * @note This function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance.
 * @csqlfn #Temporal_min_instant()
 */
const TInstant *
tsequenceset_min_inst(const TSequenceSet *ss)
{
  return tsequenceset_minmax_inst(ss, &datum_lt);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of a
 * temporal sequence set
 * @details The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_max_instant()
 */
const TInstant *
tsequenceset_max_inst(const TSequenceSet *ss)
{
  return tsequenceset_minmax_inst(ss, &datum_gt);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a pointer to) the minimum base value of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_min_value()
 */
Datum
tsequenceset_min_val(const TSequenceSet *ss)
{
  assert(ss);
  if (tnumber_type(ss->temptype))
  {
    TBox *box = TSEQUENCESET_BBOX_PTR(ss);
    return box->span.lower;
  }

  meosType basetype = temptype_basetype(ss->temptype);
  Datum result = tsequence_min_val(TSEQUENCESET_SEQ_N(ss, 0));
  for (int i = 1; i < ss->count; i++)
  {
    Datum value = tsequence_min_val(TSEQUENCESET_SEQ_N(ss, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a pointer to) the maximum base value of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_max_value()
 */
Datum
tsequenceset_max_val(const TSequenceSet *ss)
{
  assert(ss);
  if (tnumber_type(ss->temptype))
  {
    TBox *box = TSEQUENCESET_BBOX_PTR(ss);
    Datum max = box->span.upper;
    /* The upper bound of an integer span in canonical form is non exclusive */
    meosType basetype = temptype_basetype(ss->temptype);
    if (basetype == T_INT4)
      max = Int32GetDatum(DatumGetInt32(max) - 1);
    return max;
  }

  meosType basetype = temptype_basetype(ss->temptype);
  Datum result = tsequence_max_val(TSEQUENCESET_SEQ_N(ss, 0));
  for (int i = 1; i < ss->count; i++)
  {
    Datum value = tsequence_max_val(TSEQUENCESET_SEQ_N(ss, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the time frame a temporal sequence set as a timestamptz span
 * set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_time()
 */
SpanSet *
tsequenceset_time(const TSequenceSet *ss)
{
  assert(ss);
  Span *periods = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    periods[i] = (TSEQUENCESET_SEQ_N(ss, i))->period;
  return spanset_make_free(periods, ss->count, NORMALIZE_NO, ORDERED);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the duration of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] boundspan True when the potential time gaps are ignored
 * @csqlfn #Temporal_duration()
 */
Interval *
tsequenceset_duration(const TSequenceSet *ss, bool boundspan)
{
  assert(ss);
  /* Compute the duration of the bounding period */
  if (boundspan)
    return minus_timestamptz_timestamptz(DatumGetTimestampTz(ss->period.upper),
      DatumGetTimestampTz(ss->period.lower));

  const TSequence *seq = TSEQUENCESET_SEQ_N(ss, 0);
  Interval *result = minus_timestamptz_timestamptz(
    DatumGetTimestampTz(seq->period.upper),
    DatumGetTimestampTz(seq->period.lower));
  for (int i = 1; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    Interval *interv1 = minus_timestamptz_timestamptz(
      DatumGetTimestampTz(seq->period.upper),
      DatumGetTimestampTz(seq->period.lower));
    Interval *inter2 = add_interval_interval(result, interv1);
    pfree(result); pfree(interv1);
    result = inter2;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the time span of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] s Span
 */
void
tsequenceset_set_tstzspan(const TSequenceSet *ss, Span *s)
{
  assert(ss); assert(s);
  memcpy(s, &ss->period, sizeof(Span));
  return;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return an array of pointers to the sequences of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 */
const TSequence **
tsequenceset_seqs(const TSequenceSet *ss)
{
  assert(ss);
  const TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    result[i] = TSEQUENCESET_SEQ_N(ss, i);
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of segments of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] count Number of values in the output array
 * @csqlfn #Temporal_segments()
 */
TSequence **
tsequenceset_segments(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count);
  TSequence **result = palloc(sizeof(TSequence *) * ss->totalcount);
  int nsegms = 0;
  for (int i = 0; i < ss->count; i++)
    nsegms += tsequence_segments_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nsegms]);
  *count = nsegms;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the number of distinct instants of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_num_instants()
 */
int
tsequenceset_num_instants(const TSequenceSet *ss)
{
  assert(ss);
  const TInstant *lastinst = NULL; /* make compiler quiet */
  bool first = true;
  int result = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    result += seq->count;
    if (! first)
    {
      if (tinstant_eq(lastinst, TSEQUENCE_INST_N(seq, 0)))
        result --;
    }
    lastinst = TSEQUENCE_INST_N(seq, seq->count - 1);
    first = false;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the n-th distinct instant of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @param[in] n Number
 * @csqlfn #Temporal_instant_n()
 */
const TInstant *
tsequenceset_inst_n(const TSequenceSet *ss, int n)
{
  assert(ss);
  assert(n >= 1 && n <= ss->totalcount);
  if (n == 1)
    return TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, 0), 0);

  /* Continue the search 0-based */
  n--;
  const TInstant *prev = NULL, *next = NULL; /* make compiler quiet */
  bool first = true, found = false;
  int i = 0, count = 0, prevcount = 0;
  while (i < ss->count)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    count += seq->count;
    if (! first && tinstant_eq(prev, TSEQUENCE_INST_N(seq, 0)))
    {
        prevcount --;
        count --;
    }
    if (prevcount <= n && n < count)
    {
      next = TSEQUENCE_INST_N(seq, n - prevcount);
      found = true;
      break;
    }
    prevcount = count;
    prev = TSEQUENCE_INST_N(seq, seq->count - 1);
    first = false;
    i++;
  }
  if (! found)
    return NULL;
  return next;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return an array of pointers to the instants of a temporal sequence
 * set
 * @note The function does NOT remove duplicate instants
 * @param[in] ss Temporal sequence set
 */
const TInstant **
tsequenceset_insts(const TSequenceSet *ss)
{
  assert(ss);
  const TInstant **result = palloc(sizeof(TInstant *) * ss->totalcount);
  int ninsts = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
      result[ninsts++] = TSEQUENCE_INST_N(seq, j);
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the start timestamptz of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_start_timestamptz()
 */
TimestampTz
tsequenceset_start_timestamptz(const TSequenceSet *ss)
{
  assert(ss);
  const TSequence *seq = TSEQUENCESET_SEQ_N(ss, 0);
  return DatumGetTimestampTz(seq->period.lower);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the end timestamptz of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_end_timestamptz()
 */
TimestampTz
tsequenceset_end_timestamptz(const TSequenceSet *ss)
{
  assert(ss);
  const TSequence *seq = TSEQUENCESET_SEQ_N(ss, ss->count - 1);
  return DatumGetTimestampTz(seq->period.upper);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the number of distinct timestamptz values of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_num_timestamps()
 */
int
tsequenceset_num_timestamps(const TSequenceSet *ss)
{
  assert(ss);
  TimestampTz lasttime = 0; /* make compilier quiet */
  bool first = true;
  int result = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    result += seq->count;
    if (! first)
    {
      if (lasttime == TSEQUENCE_INST_N(seq, 0)->t)
        result --;
    }
    lasttime = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
    first = false;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the n-th distinct
 * timestamptz of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] n Number
 * @param[out] result Timestamps
 * @csqlfn #Temporal_timestamptz_n()
 */
bool
tsequenceset_timestamptz_n(const TSequenceSet *ss, int n, TimestampTz *result)
{
  assert(ss); assert(result);
  bool found = false;
  if (n < 1)
    return false;
  if (n == 1)
  {
    *result = TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, 0), 0)->t;
    return true;
  }

  /* Continue the search 0-based */
  n--;
  TimestampTz prev = 0, next = 0; /* make compiler quiet */
  bool first = true;
  int i = 0, count = 0, prevcount = 0;
  while (i < ss->count)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    count += seq->count;
    if (! first && prev == TSEQUENCE_INST_N(seq, 0)->t)
    {
        prevcount --;
        count --;
    }
    if (prevcount <= n && n < count)
    {
      next = TSEQUENCE_INST_N(seq, n - prevcount)->t;
      found = true;
      break;
    }
    prevcount = count;
    prev = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
    first = false;
    i++;
  }
  if (! found)
    return false;
  *result = next;
  return true;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of distinct timestamps of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] count Number of elements in the output array
 * @csqlfn #Temporal_timestamps()
 */
TimestampTz *
tsequenceset_timestamps(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count);
  TimestampTz *result = palloc(sizeof(TimestampTz) * ss->totalcount);
  int ntimes = 0;
  for (int i = 0; i < ss->count; i++)
    ntimes += tsequence_timestamps_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[ntimes]);
  if (ntimes > 1)
  {
    timestamparr_sort(result, ntimes);
    ntimes = timestamparr_remove_duplicates(result, ntimes);
  }
  *count = ntimes;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the value of a temporal
 * sequence set at a timestamptz
 * @param[in] ss Temporal sequence set
 * @param[in] t Timestamp
 * @param[in] strict True if exclusive bounds are taken into account
 * @param[out] result Base value
 * @result Return true if the timestamp is contained in the temporal sequence set
 * @pre A bounding box test has been done before by the calling function
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tsequenceset_value_at_timestamptz(const TSequenceSet *ss, TimestampTz t,
  bool strict, Datum *result)
{
  assert(ss); assert(result);
  /* Return the value even when the timestamp is at an exclusive bound */
  if (! strict)
  {
    /* Singleton sequence set */
    if (ss->count == 1)
      return tsequence_value_at_timestamptz(TSEQUENCESET_SEQ_N(ss, 0), t, false,
        result);

    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      /* Test whether the timestamp is at one of the bounds */
      const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
      if (inst->t == t)
        return tinstant_value_at_timestamptz(inst, t, result);
      inst = TSEQUENCE_INST_N(seq, seq->count - 1);
      if (inst->t == t)
        return tinstant_value_at_timestamptz(inst, t, result);
      /* Call the function on the sequence with strict set to true */
      if (contains_span_timestamptz(&seq->period, t))
        return tsequence_value_at_timestamptz(seq, t, true, result);
    }
    /* Since this function is always called with a timestamp that appears
     * in the sequence set the next statement is never reached */
    return false;
  }

  /* Singleton sequence set */
  if (ss->count == 1)
    return tsequence_value_at_timestamptz(TSEQUENCESET_SEQ_N(ss, 0), t, true,
      result);

  /* General case */
  int loc;
  if (! tsequenceset_find_timestamptz(ss, t, &loc))
    return false;
  return tsequence_value_at_timestamptz(TSEQUENCESET_SEQ_N(ss, loc), t, true,
    result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a copy of a temporal sequence set without any extra storage
 * space
 * @param[in] ss Temporal sequence set
 * @note We cannot simply test whether `s->count == ss->maxcount` since
 * there could be extra space allocated for the (variable-length) sequences
 */
TSequenceSet *
tsequenceset_compact(const TSequenceSet *ss)
{
  assert(ss);
  TSequence **sequences = palloc0(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_compact(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set restared by keeping only the last n
 * sequences
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of sequences kept
 */
void
tsequenceset_restart(TSequenceSet *ss, int count)
{
  assert(ss);
  assert(count > 0 && count < ss->count);
  /* Singleton sequence set */
  if (ss->count == 1)
    return;

  /* General case */
  TSequence *first = (TSequence *) TSEQUENCESET_SEQ_N(ss, 0);
  const TSequence *last_n;
  size_t seq_size = 0;
  int totalcount = 0;
  for (int i = 0; i < count; i++)
  {
    last_n = TSEQUENCESET_SEQ_N(ss, ss->count - i - 1);
    totalcount += last_n->count;
    seq_size += DOUBLE_PAD(VARSIZE(last_n));
  }
  /* Copy the last instants at the beginning */
  last_n = TSEQUENCESET_SEQ_N(ss, ss->count - count);
  memcpy(first, last_n, seq_size);
  /* Update the counts and the bounding box */
  ss->count = count;
  ss->totalcount = totalcount;
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(ss->temptype));
  if (bboxsize != 0)
    tsequenceset_compute_bbox(ss);
  return;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal instant transformed into a temporal sequence set
 * @param[in] inst Temporal instant
 * @param[out] interp Interpolation
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
tinstant_to_tsequenceset(const TInstant *inst, interpType interp)
{
  assert(inst); assert(interp == STEP || interp == LINEAR);
  return tsequence_to_tsequenceset_free(tinstant_to_tsequence(inst, interp));
}

/**
 * @brief Return a temporal discrete sequence transformed into a temporal
 * sequence set
 */
TSequenceSet *
tdiscseq_to_tsequenceset(const TSequence *seq, interpType interp)
{
  assert(seq); assert(interp == STEP || interp == LINEAR);
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    sequences[i] = tinstant_to_tsequence(TSEQUENCE_INST_N(seq, i), interp);
  return tsequenceset_make_free(sequences, seq->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal sequence set
 * @param[in] seq Temporal sequence
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
tsequence_to_tsequenceset(const TSequence *seq)
{
  assert(seq);
  /* For discrete sequences, each composing instant will be transformed in
   * an instantaneous sequence in the resulting sequence set */
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
  {
    interpType interp = MEOS_FLAGS_GET_CONTINUOUS(seq->flags) ? LINEAR : STEP;
    return tdiscseq_to_tsequenceset(seq, interp);
  }
  return tsequenceset_make(&seq, 1, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal sequence set
 * @param[in] seq Temporal sequence
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
tsequence_to_tsequenceset_free(TSequence *seq)
{
  assert(seq);
  TSequenceSet *result = tsequence_to_tsequenceset((const TSequence *) seq);
  pfree(seq);
  return result;
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal sequence set
 * @param[in] seq Temporal sequence
 * @param[out] interp Interpolation
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
tsequence_to_tsequenceset_interp(const TSequence *seq, interpType interp)
{
  assert(seq);
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == interp1)
    return tsequenceset_make(&seq, 1, NORMALIZE_NO);

  Temporal *temp = tsequence_set_interp(seq, interp);
  if (! temp)
    return NULL;
  if (temp->subtype == TSEQUENCESET)
    return (TSequenceSet *) temp;
  else
    return tsequence_to_tsequenceset_free((TSequence *) temp);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal sequence
 * value
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_to_tsequence()
 */
TSequence *
tsequenceset_to_tsequence(const TSequenceSet *ss)
{
  assert(ss);
  if (ss->count != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal sequence");
    return NULL;
  }
  return tsequence_copy(TSEQUENCESET_SEQ_N(ss, 0));
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal discrete
 * sequence
 * @param[in] ss Temporal sequence set
 * @note Return an error if any of the composing temporal sequences has
 * more than one instant
 */
TSequence *
tsequenceset_to_discrete(const TSequenceSet *ss)
{
  assert(ss);
  if (ss->count != ss->totalcount)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal discrete sequence");
    return NULL;
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    instants[i] = TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, i), 0);
  TSequence *result = tsequence_make(instants, ss->count, true, true, DISCRETE,
    NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_transf
 * @param[in] ss Temporal sequence set
 * @brief Return a temporal sequence set with continuous base type from
 * linear to step interpolation
 */
TSequenceSet *
tsequenceset_to_step(const TSequenceSet *ss)
{
  assert(ss);
  /* If the sequence set has step interpolation return a copy */
  if (! MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    return tsequenceset_copy(ss);

  /* Test whether it is possible to set the interpolation to step */
  meosType basetype = temptype_basetype(ss->temptype);
  const TSequence *seq;
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    if ((seq->count > 2) ||
        (seq->count == 2 && ! datum_eq(
          tinstant_val(TSEQUENCE_INST_N(seq, 0)),
          tinstant_val(TSEQUENCE_INST_N(seq, 1)), basetype)))
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Cannot transform input value to step interpolation");
      return NULL;
    }
  }

  /* Construct the result */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *instants[2];
    for (int j = 0; j < seq->count; j++)
      instants[j] = TSEQUENCE_INST_N(seq, j);
    sequences[i] = tsequence_make(instants, seq->count, seq->period.lower_inc,
      seq->period.upper_inc, STEP, NORMALIZE_NO);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set with continuous base type from
 * step to linear interpolation
 * @param[in] ss Temporal sequence set
 */
TSequenceSet *
tsequenceset_to_linear(const TSequenceSet *ss)
{
  assert(ss);
  /* If the sequence set has linear interpolation return a copy */
  if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    return tsequenceset_copy(ss);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tstepseq_to_linear(TSEQUENCESET_SEQ_N(ss, 0));

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tstepseq_to_linear_iter(TSEQUENCESET_SEQ_N(ss, i),
      &sequences[nseqs]);
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal value transformed to the given interpolation
 * @param[in] ss Temporal sequence set
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_set_interp()
 */
Temporal *
tsequenceset_set_interp(const TSequenceSet *ss, interpType interp)
{
  assert(ss);
  if (interp == DISCRETE)
    return (Temporal *) tsequenceset_to_discrete(ss);
  else if (interp == STEP)
    return (Temporal *) tsequenceset_to_step(ss);
  else /* interp == LINEAR */
    return (Temporal *) tsequenceset_to_linear(ss);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set where the value dimension is shifted
 * and/or scaled by two values
 * @param[in] ss Temporal sequence set
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @pre The duration is greater than 0 if it is not NULL
 * @csqlfn #Tnumber_shift_value(), #Tnumber_scale_value(),
 * #Tnumber_shift_scale_value()
 */
TSequenceSet *
tnumberseqset_shift_scale_value(const TSequenceSet *ss, Datum shift,
  Datum width, bool hasshift, bool haswidth)
{
  assert(ss); assert(hasshift || haswidth);

  /* Copy the input sequence set to the result */
  TSequenceSet *result = tsequenceset_copy(ss);
  /* Shift and/or scale the bounding span */
  Datum delta;
  double scale;
  TBox *box = TSEQUENCESET_BBOX_PTR(result);
  numspan_shift_scale1(&box->span, shift, width, hasshift, haswidth, &delta,
    &scale);
  Datum origin = box->span.lower;

  /* Shift and/or scale each composing sequence */
  for (int i = 0; i < result->count; i++)
  {
    TSequence *seq = (TSequence *) TSEQUENCESET_SEQ_N(result, i);
    /* Shift and/or scale the bounding span of the sequence */
    box = TSEQUENCE_BBOX_PTR(seq);
    numspan_delta_scale_iter(&box->span, origin, delta, hasshift, scale);
    /* Shift and/or scale each instant of the composing sequence */
    tnumberseq_shift_scale_value_iter(seq, origin, delta, hasshift, scale);
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set shifted and/or scaled by two intervals
 * @param[in] ss Temporal sequence set
 * @param[in] shift Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if it is not NULL
 * @csqlfn #Temporal_shift_time(), #Temporal_scale_time(),
 * #Temporal_shift_scale_time()
 */
TSequenceSet *
tsequenceset_shift_scale_time(const TSequenceSet *ss, const Interval *shift,
  const Interval *duration)
{
  assert(ss);
  assert(shift || duration);

  /* Copy the input sequence set to the result */
  TSequenceSet *result = tsequenceset_copy(ss);

  /* Shift and/or scale the bounding period */
  TimestampTz delta;
  double scale;
  tstzspan_shift_scale1(&result->period, shift, duration, &delta, &scale);
  TimestampTz origin = DatumGetTimestampTz(result->period.lower);

  /* Shift and/or scale each composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = (TSequence *) TSEQUENCESET_SEQ_N(result, i);
    /* Shift and/or scale the bounding period of the sequence */
    tstzspan_delta_scale_iter(&seq->period, origin, delta, scale);
    /* Shift and/or scale each instant of the composing sequence */
    tsequence_shift_scale_time_iter(seq, delta, scale);
  }
  return result;
}

/*****************************************************************************
 * Synchronize functions
 *****************************************************************************/

/**
 * @brief Temporally intersect or synchronize a temporal sequence set and a
 * temporal sequence
 * @details The resulting values are composed of denormalized sequences
 * covering the intersection of their time spans
 * @param[in] ss,seq Input values
 * @param[in] mode Enumeration for either intersect or synchronize
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequenceset_tsequence(const TSequenceSet *ss, const TSequence *seq,
  SyncMode mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
  assert(ss); assert(seq);
  assert(inter1); assert(inter2);
  /* The temporal types of the arguments may be different */
  /* Bounding period test */
  if (! over_span_span(&ss->period, &seq->period))
    return false;

  int loc;
  tsequenceset_find_timestamptz(ss, DatumGetTimestampTz(seq->period.lower), &loc);
  /* We are sure that loc < ss->count due to the bounding period test above */
  TSequence **sequences1 = palloc(sizeof(TSequence *) * ss->count - loc);
  TSequence **sequences2 = palloc(sizeof(TSequence *) * ss->count - loc);
  int nseqs = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    TSequence *interseq1, *interseq2;
    bool hasinter;
    /* mode == SYNCHRONIZE or SYNCHRONIZE_CROSS */
    hasinter = synchronize_tsequence_tsequence(seq, seq1,
      &interseq1, &interseq2, mode == SYNCHRONIZE_CROSS);
    if (hasinter)
    {
      sequences1[nseqs] = interseq1;
      sequences2[nseqs++] = interseq2;
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq->period.upper),
      DatumGetTimestampTz(seq1->period.upper));
    if (cmp < 0 ||
      (cmp == 0 && (! seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  *inter1 = tsequenceset_make_free(sequences1, nseqs, NORMALIZE_NO);
  *inter2 = tsequenceset_make_free(sequences2, nseqs, NORMALIZE_NO);
  return nseqs > 0;
}

/**
 * @brief Temporally intersect or synchronize two temporal sequence sets
 * @param[in] ss1,ss2 Input values
 * @param[in] mode Intersection or synchronization (with or without adding crossings)
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, SyncMode mode, TSequenceSet **inter1,
  TSequenceSet **inter2)
{
  assert(ss1); assert(ss2);
  /* The temporal types of the arguments may be different */
  assert(inter1); assert(inter2);
  /* Bounding period test */
  if (! over_span_span(&ss1->period, &ss2->period))
    return false;

  int count = ss1->count + ss2->count;
  TSequence **sequences1 = palloc(sizeof(TSequence *) * count);
  TSequence **sequences2 = palloc(sizeof(TSequence *) * count);
  int i = 0, j = 0, nseqs = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    TSequence *interseq1, *interseq2;
    bool hasinter;
    /* mode == SYNCHRONIZE or SYNCHRONIZE_CROSS */
    hasinter = synchronize_tsequence_tsequence(seq1, seq2,
      &interseq1, &interseq2, mode == SYNCHRONIZE_CROSS);
    if (hasinter)
    {
      sequences1[nseqs] = interseq1;
      sequences2[nseqs++] = interseq2;
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
    if (cmp == 0 && seq1->period.upper_inc == seq2->period.upper_inc)
    {
      i++; j++;
    }
    else if (cmp < 0 ||
      (cmp == 0 && ! seq1->period.upper_inc && seq2->period.upper_inc))
      i++;
    else
      j++;
  }
  *inter1 = tsequenceset_make_free(sequences1, nseqs, NORMALIZE_NO);
  *inter2 = tsequenceset_make_free(sequences2, nseqs, NORMALIZE_NO);
  return nseqs > 0;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Temporally intersect two temporal values
 * @param[in] ss,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tinstant(const TSequenceSet *ss, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  assert(ss); assert(inst);
  /* The temporal types of the arguments may be different */
  assert(inter1); assert(inter2);
  TInstant *inst1 = (TInstant *)
    tsequenceset_restrict_timestamptz(ss, inst->t, REST_AT);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * @brief Temporally intersect two temporal values
 * @param[in] inst,ss Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ss,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tsequenceset_tinstant(ss, inst, inter2, inter1);
}

/**
 * @brief Temporally intersect two temporal values
 * @param[in] ss,seq Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tdiscseq(const TSequenceSet *ss,
  const TSequence *seq, TSequence **inter1, TSequence **inter2)
{
  assert(ss); assert(seq);
  /* The temporal types of the arguments may be different */
  assert(inter1); assert(inter2);
  /* Bounding period test */
  if (! over_span_span(&ss->period, &seq->period))
    return false;

  TInstant **instants1 = palloc(sizeof(TInstant *) * seq->count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, ninsts = 0;
  while (i < ss->count && j < seq->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, j);
    if (contains_span_timestamptz(&seq1->period, inst->t))
    {
      instants1[ninsts] = tsequence_at_timestamptz(seq1, inst->t);
      instants2[ninsts++] = inst;
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  if (ninsts == 0)
  {
    pfree(instants1); pfree(instants2);
    return false;
  }

  *inter1 = tsequence_make_free(instants1, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
  *inter2 = tsequence_make(instants2, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
  pfree(instants2);
  return true;
}

/**
 * @brief Temporally intersect two temporal values
 * @param[in] seq,ss Input values
 * @param[out] inter1,inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tdiscseq_tsequenceset(const TSequence *seq,
  const TSequenceSet *ss, TSequence **inter1, TSequence **inter2)
{
  return intersection_tsequenceset_tdiscseq(ss, seq, inter2, inter1);
}

/**
 * @brief Temporally intersect or synchronize two temporal values
 * @param[in] seq,ss Input values
 * @param[in] mode Enumeration for either intersect or synchronize
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on times
 */
bool
intersection_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  SyncMode mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
  return synchronize_tsequenceset_tsequence(ss, seq, mode, inter2, inter1);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequenceSet *
tsequenceset_in(const char *str, meosType temptype, interpType interp)
{
  assert(str);
  return tsequenceset_parse(&str, temptype, interp);
}
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set boolean from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tboolseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TBOOL, true);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set integer from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tintseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TINT, true);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set float from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tfloatseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the interpolation at the beginning (if any) */
  Temporal *temp = temporal_parse(&str, T_TFLOAT);
  assert(temp->subtype == TSEQUENCE);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set text from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
ttextseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TTEXT, true);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set geometry point from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeompointseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set geography point from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeogpointseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}
#endif

/**
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] maxdd Maximum number of decimal digits to output for floating point
 * values
 * @param[in] value_out Function called to output the base value
 */
char *
tsequenceset_to_string(const TSequenceSet *ss, int maxdd, outfunc value_out)
{
  assert(ss);
  assert(maxdd >= 0);

  char **strings = palloc(sizeof(char *) * ss->count);
  size_t outlen = 0;
  char prefix[20];
  if (MEOS_FLAGS_GET_CONTINUOUS(ss->flags) &&
      ! MEOS_FLAGS_LINEAR_INTERP(ss->flags))
    sprintf(prefix, "Interp=Step;");
  else
    prefix[0] = '\0';
  for (int i = 0; i < ss->count; i++)
  {
    strings[i] = tsequence_to_string(TSEQUENCESET_SEQ_N(ss, i), maxdd, true,
      value_out);
    outlen += strlen(strings[i]) + 1;
  }
  return stringarr_to_string(strings, ss->count, outlen, prefix, '{', '}',
    QUOTES_NO, SPACES);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tsequenceset_out(const TSequenceSet *ss, int maxdd)
{
  assert(ss);
  return tsequenceset_to_string(ss, maxdd, &basetype_out);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the integral (area under the curve) of a temporal number
 * @param[in] ss Temporal sequence set
 */
double
tnumberseqset_integral(const TSequenceSet *ss)
{
  assert(ss);
  assert(tnumber_type(ss->temptype));
  double result = 0;
  for (int i = 0; i < ss->count; i++)
    result += tnumberseq_integral(TSEQUENCESET_SEQ_N(ss, i));
  return result;
}

/**
 * @brief Return the duration of a temporal sequence set as a double
 */
static double
tsequenceset_interval_double(const TSequenceSet *ss)
{
  assert(ss);
  assert(tnumber_type(ss->temptype));
  double result = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    result += (double) (DatumGetTimestampTz(seq->period.upper) -
      DatumGetTimestampTz(seq->period.lower));
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the time-weighted average of a temporal number
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tnumber_twavg()
 */
double
tnumberseqset_twavg(const TSequenceSet *ss)
{
  assert(ss);
  assert(tnumber_type(ss->temptype));
  double duration = tsequenceset_interval_double(ss);
  double result;
  if (duration == 0.0)
  {
    result = 0;
    for (int i = 0; i < ss->count; i++)
      result += tnumbercontseq_twavg(TSEQUENCESET_SEQ_N(ss, i));
    return result / ss->count;
  }
  else
    result = tnumberseqset_integral(ss) / duration;
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_trad
 * @brief Return true if two temporal sequence sets are equal
 * @param[in] ss1,ss2 Temporal sequence sets
 * @pre The arguments are of the same base type
 * @note The function #tsequenceset_cmp() is not used to increase efficiency
 * @csqlfn #Temporal_eq()
 */
bool
tsequenceset_eq(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2);
  assert(ss1->temptype == ss2->temptype);
  /* If number of sequences or flags are not equal */
  if (ss1->count != ss2->count || ss1->flags != ss2->flags)
    return false;

  /* If bounding boxes are not equal */
  if (! temporal_bbox_eq(TSEQUENCESET_BBOX_PTR(ss1),
      TSEQUENCESET_BBOX_PTR(ss2), ss1->temptype))
    return false;

  /* Compare the composing sequences */
  for (int i = 0; i < ss1->count; i++)
  {
    if (! tsequence_eq(TSEQUENCESET_SEQ_N(ss1, i), TSEQUENCESET_SEQ_N(ss2, i)))
      return false;
  }
  return true;
}

/**
 * @ingroup meos_internal_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first temporal sequence
 * set is less than, equal, or greater than the second one
 * @param[in] ss1,ss2 Temporal sequence sets
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function #temporal_cmp
 * @csqlfn #Temporal_cmp()
 */
int
tsequenceset_cmp(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2);
  assert(ss1->temptype == ss2->temptype);

  /* Compare composing instants */
  int count = Min(ss1->count, ss2->count);
  for (int i = 0; i < count; i++)
  {
    int result = tsequence_cmp(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i));
    if (result)
      return result;
  }

  /* ss1->count == ss2->count because of the bounding box and the
   * composing sequence tests above */

  /* ss1->flags == ss2->flags since all the composing sequences are equal */

  /* The two values are equal */
  return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_hash()
 */
uint32
tsequenceset_hash(const TSequenceSet *ss)
{
  assert(ss);
  uint32 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    uint32 seq_hash = tsequence_hash(TSEQUENCESET_SEQ_N(ss, i));
    result = (result << 5) - result + seq_hash;
  }
  return result;
}

/*****************************************************************************/
