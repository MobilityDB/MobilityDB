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
 * @brief Functions for temporal rigid geometries of sequence subtype
 */

#include "rgeo/trgeo_seq.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/tsequence.h"
#include "general/type_util.h"
#include "general/temporal_boxops.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_boxops.h"
#include "rgeo/trgeo_utils.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Returns the reference geometry of the temporal value
 */
const GSERIALIZED *
trgeoseq_geom_p(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TRGEOMETRY);
  assert(MEOS_FLAGS_GET_GEOM(seq->flags));
  return (GSERIALIZED *) (
    /* start of data */
    ((char *) seq) + DOUBLE_PAD(sizeof(TSequence)) +
      ((seq->bboxsize == 0) ? 0 : (seq->bboxsize - sizeof(Span))) +
      sizeof(size_t) * seq->maxcount +
      /* offset */
      (TSEQUENCE_OFFSETS_PTR(seq))[seq->count]);
}

/*****************************************************************************/

/**
 * @brief Returns the size of the trgeometryseq without reference geometry
 */
size_t
trgeoseq_pose_varsize(const TSequence *seq)
{
  const GSERIALIZED *geom = trgeoseq_geom_p(seq);
  return VARSIZE(seq) - DOUBLE_PAD(VARSIZE(geom));
}

/**
 * @brief Set the size of the trgeometryseq without reference geometry
 */
void
trgeoseq_set_pose(TSequence *seq)
{
  SET_VARSIZE(seq, trgeoseq_pose_varsize(seq));
  MEOS_FLAGS_SET_GEOM(seq->flags, NO_GEOM);
  return;
}

/**
 * @brief Returns a new temporal pose sequence obtained by removing the 
 * reference geometry of a temporal rigid geometry
 */
TSequence *
trgeoseq_tposeseq(const TSequence *seq)
{
  assert(seq->temptype == T_TRGEOMETRY);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = trgeoinst_tposeinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @brief Ensure the validity of the arguments when creating a temporal value
 */
bool
trgeoseq_make_valid(const GSERIALIZED *geom, const TInstant **instants,
  int count, bool lower_inc, bool upper_inc, bool linear)
{
  if (! tsequence_make_valid(instants, count, lower_inc, upper_inc, linear))
    return false;
  if (! ensure_valid_tinstarr(instants, count, MERGE_NO,
      linear ? LINEAR : STEP))
    return false;
  for (int i = 0; i < count; ++i)
    if (MEOS_FLAGS_GET_GEOM(instants[i]->flags))
    {
      if (! ensure_same_geom(geom, trgeoinst_geom_p(instants[i])))
        return false;
    }
  return true;
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 * @details For example, the memory structure of a temporal sequence with two
 * instants is as follows:
 * @code
 * ----------------------------------------------------------------------
 * ( TSequence )_X | ( bbox )_X | offset_0 | offset_1 | offset_geom | ...
 * ----------------------------------------------------------------------
 * --------------------------------------------------
 * ( TInstant_0 )_X | ( TInstant_1 )_X | ( Geom )_X |
 * --------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding instants
 * @pre The validity of the arguments has been tested before
 */
TSequence *
trgeoseq_make1_exp(const GSERIALIZED *geom, const TInstant **instants,
  int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize)
{
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
  /* Size of composing instants */
  for (int i = 0; i < newcount; i++)
    insts_size += DOUBLE_PAD(trgeoinst_pose_varsize(norminsts[i]));
  /* Compute the total size for maxcount instants as a proportion of the size
   * of the count instants provided. Note that this is only an initial
   * estimation. The functions adding instants to a sequence must verify both
   * the maximum number of instants and the remaining space for adding an
   * additional variable-length instant of arbitrary size */
  if (count != maxcount)
    insts_size *= (double) maxcount / count;
  else
    maxcount = newcount;
  /* Size of the reference geometry */
  size_t geom_size = DOUBLE_PAD(VARSIZE(geom));
  /* Total size of the struct */
  size_t memsize = DOUBLE_PAD(sizeof(TSequence)) + bboxsize_extra +
    (maxcount + 1) * sizeof(size_t) + insts_size + geom_size;

  /* Create the temporal sequence */
  TSequence *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->maxcount = maxcount;
  result->temptype = instants[0]->temptype;
  result->subtype = TSEQUENCE;
  result->bboxsize = bboxsize;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags,
    MEOS_FLAGS_GET_CONTINUOUS(norminsts[0]->flags));
  MEOS_FLAGS_SET_INTERP(result->flags, interp);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, MEOS_FLAGS_GET_Z(instants[0]->flags));
  MEOS_FLAGS_SET_GEODETIC(result->flags,
    MEOS_FLAGS_GET_GEODETIC(instants[0]->flags));
  MEOS_FLAGS_SET_GEOM(result->flags, WITH_GEOM);
  /* Initialization of the variable-length part */
  /* Compute the bounding box */
  trgeoinstarr_compute_bbox(geom, (const TInstant **) norminsts,
    newcount, interp, TSEQUENCE_BBOX_PTR(result));
  /* Set the lower_inc and upper_inc bounds of the period at the beginning
   * of the bounding box */
  Span *p = (Span *) TSEQUENCE_BBOX_PTR(result);
  p->lower_inc = lower_inc;
  p->upper_inc = upper_inc;
  /* Store the composing instants */
  size_t pdata = DOUBLE_PAD(sizeof(TSequence)) + bboxsize_extra +
    sizeof(size_t) * maxcount;
  size_t pos = sizeof(size_t); /* Account for geom offset pointer */
  for (int i = 0; i < newcount; i++)
  {
    size_t inst_size = trgeoinst_pose_varsize(norminsts[i]);
    memcpy(((char *)result) + pdata + pos, norminsts[i], inst_size);
    (TSEQUENCE_OFFSETS_PTR(result))[i] = pos;
    trgeoinst_set_pose((TInstant *) (((char *) result) + pdata + pos));
    pos += DOUBLE_PAD(inst_size);
  }
  /* Store the reference geometry */
  void *geom_from = (void *) geom;
  memcpy(((char *) result) + pdata + pos, geom_from, VARSIZE(geom_from));
  (TSEQUENCE_OFFSETS_PTR(result))[newcount] = pos;

  if (interp != DISCRETE && normalize && count > 1)
    pfree(norminsts);
  return result;
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 * @pre The validity of the arguments has been tested before
 */
inline TSequence *
trgeoseq_make1(const GSERIALIZED *geom, const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  return trgeoseq_make1_exp(geom, instants, count, count, lower_inc, upper_inc,
    interp, normalize);
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 * @param[in] geom Reference geometry
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 */
TSequence *
trgeoseq_make_exp(const GSERIALIZED *geom, const TInstant **instants,
  int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(instants, NULL);
  if (! ensure_positive(count) ||
      ! trgeoseq_make_valid(geom, instants, count, lower_inc, upper_inc,
          interp))
    return NULL;
  return trgeoseq_make1_exp(geom, instants, count, maxcount, lower_inc,
    upper_inc, interp, normalize);
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal sequence from an array of temporal instants.
 * @param[in] geom Reference geometry
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 */
inline TSequence *
trgeoseq_make(const GSERIALIZED *geom, const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  return trgeoseq_make_exp(geom, instants, count, count, lower_inc, upper_inc,
    interp, normalize);
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants
 * and free the array and the instants after the creation.
 * @param[in] geom Reference geometry
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @note Does not free the reference geometry
 * @see tsequence_make
 */
TSequence *
trgeoseq_make_free_exp(const GSERIALIZED *geom, TInstant **instants, int count,
  int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(instants, NULL);
  if (! ensure_not_negative(count))
    return NULL;

  if (count == 0)
  {
    pfree(instants);
    return NULL;
  }
  TSequence *result = trgeoseq_make_exp(geom, (const TInstant **) instants,
    count, maxcount, lower_inc, upper_inc, interp, normalize);
  pfree_array((void **) instants, count);
  return result;
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal sequence from an array of temporal instants
 * and free the array and the instants after the creation
 * @param[in] geom Reference geometry
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True if the respective bound is inclusive
 * @param[in] interp Interpolation
 * @param[in] normalize True if the resulting value should be normalized
 * @note Does not free the reference geometry
 * @see tsequence_make
 */
inline TSequence *
trgeoseq_make_free(const GSERIALIZED *geom, TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, bool normalize)
{
  return trgeoseq_make_free_exp(geom, instants, count, count, lower_inc,
    upper_inc, interp, normalize);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal instant transformed into a temporal sequence
 * @param[in] inst Temporal instant
 * @param[in] interp Interpolation
 */
TSequence *
trgeoinst_to_tsequence(const TInstant *inst, interpType interp)
{
  assert(inst);
  return trgeoseq_make(trgeoinst_geom_p(inst), &inst, 1, true, true, interp,
    NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal sequence set transformed into a temporal sequence
 * @param[in] ss Temporal sequence set
 */
TSequence *
trgeoseqset_to_tsequence(const TSequenceSet *ss)
{
  assert(ss);
  if (ss->totalcount != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal instant");
    return NULL;
  }
  return tsequence_copy(TSEQUENCESET_SEQ_N(ss, 0));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/


/*****************************************************************************/
