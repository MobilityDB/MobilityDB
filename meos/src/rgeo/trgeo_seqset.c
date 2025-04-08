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
 * @brief Functions for temporal rigid geometries of sequence set subtype
 */

#include "rgeo/trgeo_seqset.h"

/* C */
#include <assert.h>
/* C */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "general/tsequenceset.h"
#include "general/temporal.h"
#include "general/type_util.h"
#include "general/temporal_boxops.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_boxops.h"
#include "rgeo/trgeo_seq.h"
#include "rgeo/trgeo_utils.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Returns the reference geometry of the temporal value
 */
const GSERIALIZED *
trgeoseqset_geom_p(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TRGEOMETRY);
  assert(ensure_has_geom(ss->flags)); 
  return (GSERIALIZED *) (
    /* start of data */
    ((char *) ss) + DOUBLE_PAD(sizeof(TSequenceSet)) +
      /* The period component of the bbox is already declared in the struct */
      (ss->bboxsize - sizeof(Span)) + ss->count * sizeof(size_t) +
      /* offset */
      (TSEQUENCESET_OFFSETS_PTR(ss))[ss->count]);
}

/**
 * @brief Returns a new temporal pose sequence obtained by removing the 
 * reference geometry of a temporal rigid geometry
 */
TSequenceSet *
trgeoseqset_tposeseqset(const TSequenceSet *ss)
{
  assert(ss->temptype == T_TRGEOMETRY);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = trgeoseq_tposeseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @brief Construct a temporal sequence set from an array of temporal sequences
 * @details For example, the memory structure of a temporal sequence set with
 * two sequences is as follows
 * @code
 * -------------------------------------------------------------------------
 * ( TSequenceSet )_X | ( bbox )_X | offset_0 | offset_1 | offset_geom | ...
 * -------------------------------------------------------------------------
 * ----------------------------------------------------
 * ( TSequence_0 )_X | ( TSequence_1 )_X | ( Geom )_X |
 * ----------------------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding sequences.
 * @param[in] geom Reference geometry
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two temporal sequence
 * sets before applying an operation to them.
 */
TSequenceSet *
trgeoseqset_make1_exp(const GSERIALIZED *geom, const TSequence **sequences,
  int count, int maxcount, bool normalize)
{
  assert(maxcount >= count);

  /* Test the validity of the sequences */
  assert(count > 0);
  if (! ensure_valid_tseqarr(sequences, count))
    return NULL;
  for (int i = 0; i < count; ++i)
    if (MEOS_FLAGS_GET_GEOM(sequences[i]->flags))
    {
      if (! ensure_same_geom(geom, trgeoseq_geom_p(sequences[i])))
        return NULL;
      // TODO free
    }

  /* Normalize the array of sequences */
  TSequence **normseqs = (TSequence **) sequences;
  int newcount = count;
  if (normalize && count > 1)
    normseqs = tseqarr_normalize(sequences, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = temporal_bbox_size(sequences[0]->temptype);
  /* The period component of the bbox is already declared in the struct */
  size_t bboxsize_extra = bboxsize - sizeof(Span);

  /* Compute the size of the temporal sequence set */
  size_t seqs_size = 0;
  int totalcount = 0;
  for (int i = 0; i < newcount; i++)
  {
    totalcount += normseqs[i]->count;
    seqs_size += DOUBLE_PAD(trgeoseq_pose_varsize(normseqs[i]));
  }
  /* Compute the total size for maxcount sequences as a proportion of the size
   * of the count sequences provided. Note that this is only an initial
   * estimation. The functions adding sequences to a sequence set must verify
   * both the maximum number of sequences and the remaining space for adding an
   * additional variable-length sequences of arbitrary size */
  if (count != maxcount)
    seqs_size *= (double) maxcount / count;
  else
    maxcount = newcount;
  /* Size of the reference geometry */
  size_t geom_size = DOUBLE_PAD(VARSIZE(geom));
  /* Total size of the struct */
  size_t memsize = DOUBLE_PAD(sizeof(TSequenceSet)) + bboxsize_extra +
    (maxcount + 1) * sizeof(size_t) + seqs_size + geom_size;

  /* Create the temporal sequence set */
  TSequenceSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->maxcount = maxcount;
  result->totalcount = totalcount;
  result->temptype = sequences[0]->temptype;
  result->subtype = TSEQUENCESET;
  result->bboxsize = bboxsize;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags,
    MEOS_FLAGS_GET_CONTINUOUS(sequences[0]->flags));
  MEOS_FLAGS_SET_INTERP(result->flags,
    MEOS_FLAGS_GET_INTERP(sequences[0]->flags));
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags,
    MEOS_FLAGS_GET_Z(sequences[0]->flags));
  MEOS_FLAGS_SET_GEODETIC(result->flags,
    MEOS_FLAGS_GET_GEODETIC(sequences[0]->flags));
  MEOS_FLAGS_SET_GEOM(result->flags, WITH_GEOM);
  /* Initialization of the variable-length part */
  /* Compute the bounding box */
  tseqarr_compute_bbox((const TSequence **) normseqs, newcount,
    TSEQUENCESET_BBOX_PTR(result));
  /* Store the composing instants */
  size_t pdata = DOUBLE_PAD(sizeof(TSequenceSet)) + bboxsize_extra +
    sizeof(size_t) * newcount;
  size_t pos = sizeof(size_t); /* Account for geom offset pointer */
  for (int i = 0; i < newcount; i++)
  {
    size_t seq_size = trgeoseq_pose_varsize(normseqs[i]);
    memcpy(((char *) result) + pdata + pos, normseqs[i], seq_size);
    (TSEQUENCESET_OFFSETS_PTR(result))[i] = pos;
    trgeoseq_set_pose((TSequence *) (((char *)result) + pdata + pos));
    pos += DOUBLE_PAD(seq_size);
  }
  /* Store the reference geometry */
  void *geom_from = (void *) geom;
  memcpy(((char *) result) + pdata + pos, geom_from, VARSIZE(geom_from));
  (TSEQUENCESET_OFFSETS_PTR(result))[newcount] = pos;

  if (normalize && count > 1)
    pfree_array((void **) normseqs, newcount);
  return result;
}

/**
 * @ingroup meos_internal_rgeo_constructor
 * @brief Construct a temporal sequence set from an array of temporal sequences.
 * @param[in] geom Reference geometry
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence sets before applying an operation to them.
 */
TSequenceSet *
trgeoseqset_make_exp(const GSERIALIZED *geom, const TSequence **sequences,
  int count, int maxcount, bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(sequences, NULL);
  if (! ensure_positive(count) ||
      ! ensure_valid_tseqarr((const TSequence **) sequences, count))
    return NULL;
  return trgeoseqset_make1_exp(geom, sequences, count, maxcount, normalize);
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal sequence set from an array of temporal sequences
 * @param[in] geom Reference geometry
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence sets before applying an operation to them.
 * @sqlfn tbool_seqset(), tint_seqset(), tfloat_seqset(), ttext_seqset(), etc.
 */
inline TSequenceSet *
trgeoseqset_make(const GSERIALIZED *geom, const TSequence **sequences,
  int count, bool normalize)
{
  return trgeoseqset_make_exp(geom, sequences, count, count, normalize);
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal sequence set from an array of temporal
 * sequences and free the array and the sequences after the creation
 * @param[in] geom Reference geometry
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized
 * @note Does not free the reference geometry
 * @see tsequenceset_make
 */
TSequenceSet *
trgeoseqset_make_free(const GSERIALIZED *geom, TSequence **sequences,
  int count, bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(sequences, NULL);
  if (! ensure_positive(count) ||
      ! ensure_valid_tseqarr((const TSequence **) sequences, count))
    return NULL;

  if (count == 0)
  {
    pfree(sequences);
    return NULL;
  }
  TSequenceSet *result = trgeoseqset_make(geom, (const TSequence **) sequences,
    count, normalize);
  pfree_array((void **) sequences, count);
  return result;
}

/**
 * @brief Ensure the validity of the arguments when creating a temporal value
 * @details This function extends function #tsequence_make_valid by spliting
 * the sequences according the maximum distance or interval between instants
 */
static int *
trgeoseqset_make_valid_gaps(const GSERIALIZED *geom, const TInstant **instants,
  int count, bool lower_inc, bool upper_inc, interpType interp, double maxdist,
  Interval *maxt, int *nsplits)
{
  assert(interp != DISCRETE);
  trgeoseq_make_valid(geom, instants, count, lower_inc, upper_inc,
    interp);
  return ensure_valid_tinstarr_gaps(instants, count, MERGE_NO, maxdist, maxt,
    nsplits);
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal sequence set from an array of temporal instants
 * introducing a gap when two consecutive instants are separated from each
 * other by at least the given distance or the given time interval.
 * @param[in] geom Geometry
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap
 * @sqlfn tint_seqset_gaps(), tfloat_seqset_gaps(),
 * tgeompoint_seqset_gaps(), tgeogpoint_seqset_gaps()
 */
TSequenceSet *
trgeoseqset_make_gaps(const GSERIALIZED *geom, const TInstant **instants,
  int count, interpType interp, Interval *maxt, double maxdist)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(instants, NULL);
  if (! ensure_positive(count))
    return NULL;

  TSequence *seq;
  TSequenceSet *result;

  /* If no gaps are given construt call the standard sequence constructor */
  if (maxt == NULL && maxdist <= 0.0)
  {
    seq = trgeoseq_make(geom, (const TInstant **) instants,
      count, true, true, interp, NORMALIZE);
    result = trgeoseqset_make(geom, (const TSequence **) &seq, 1, NORMALIZE_NO);
    pfree(seq);
    return result;
  }

  /* Ensure that the array of instants is valid and determine the splits */
  int countsplits;
  int *splits = trgeoseqset_make_valid_gaps(geom, (const TInstant **) instants,
    count, true, true, interp, maxdist, maxt, &countsplits);
  if (countsplits == 0)
  {
    /* There are no gaps */
    pfree(splits);
    seq = trgeoseq_make1(geom, (const TInstant **) instants, count, true, true,
      interp, NORMALIZE);
    result = trgeoseqset_make(geom, (const TSequence **) &seq, 1,
      NORMALIZE_NO);
    pfree(seq);
  }
  else
  {
    int newcount = 0;
    /* Split according to gaps */
    const TInstant **newinsts = palloc(sizeof(TInstant *) * count);
    TSequence **sequences = palloc(sizeof(TSequence *) * (countsplits + 1));
    int j = 0, k = 0;
    for (int i = 0; i < count; i++)
    {
      if (j < countsplits && splits[j] == i)
      {
        /* Finalize the current sequence and start a new one */
        assert(k > 0);
        sequences[newcount++] = trgeoseq_make1(geom, 
          (const TInstant **) newinsts, k, true, true, interp, NORMALIZE);
        j++; k = 0;
      }
      /* Continue with the current sequence */
      newinsts[k++] = instants[i];
    }
    /* Construct last sequence */
    if (k > 0)
      sequences[newcount++] = trgeoseq_make1(geom,
        (const TInstant **) newinsts, k, true, true, interp, NORMALIZE);
    result = trgeoseqset_make(geom, (const TSequence **) sequences, newcount,
      NORMALIZE);
    pfree(newinsts); pfree(sequences);
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal instant transformed into a temporal sequence set
 * @param[in] inst Temporal instant
 * @param[out] interp Interpolation
 */
TSequenceSet *
trgeoinst_to_tsequenceset(const TInstant *inst, interpType interp)
{
  assert(inst); assert(inst->temptype == T_TRGEOMETRY);
  assert(interp == STEP || interp == LINEAR);
  TSequenceSet *res = tinstant_to_tsequenceset(inst, interp);
  TSequenceSet *result = geo_tposeseqset_to_trgeo(trgeoinst_geom_p(inst), res);
  pfree(res);
  return result;
}

/**
 * @brief Return a temporal discrete sequence transformed into a temporal
 * sequence set
 */
TSequenceSet *
trgeodiscseq_to_tsequenceset(const TSequence *seq, interpType interp)
{
  assert(seq); assert(seq->temptype == T_TRGEOMETRY);
  assert(interp == STEP || interp == LINEAR);
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    sequences[i] = trgeoinst_to_tsequence(TSEQUENCE_INST_N(seq, i), interp);
  return trgeoseqset_make_free(trgeoseq_geom_p(seq), sequences, seq->count,
    NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal sequence transformed into a temporal sequence set
 * @param[in] seq Temporal sequence
 */
TSequenceSet *
trgeoseq_to_tsequenceset(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TRGEOMETRY);
  /* For discrete sequences, each composing instant will be transformed in
   * an instantaneous sequence in the resulting sequence set */
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
  {
    interpType interp = MEOS_FLAGS_GET_CONTINUOUS(seq->flags) ? LINEAR : STEP;
    return trgeodiscseq_to_tsequenceset(seq, interp);
  }
  return trgeoseqset_make(trgeoseq_geom_p(seq), &seq, 1, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal sequence transformed into a temporal sequence set
 * @param[in] seq Temporal sequence
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
trgeoseq_to_tsequenceset_free(TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TRGEOMETRY);
  TSequenceSet *result = trgeoseq_to_tsequenceset((const TSequence *) seq);
  pfree(seq);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/


/*****************************************************************************/
