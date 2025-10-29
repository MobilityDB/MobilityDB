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
 * @brief Modification functions for temporal values
 */


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
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_boxops.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#if NPOINT
  #include "npoint/tnpoint_distance.h"
  #include "npoint/tnpoint_spatialfuncs.h"
#endif

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Ensure that two temporal instants have the same value
 */
bool
ensure_tinstant_same_value(const TInstant *inst1, const TInstant *inst2,
  meosType basetype)
{
  assert(inst1->t == inst2->t);
  if (! datum_eq(tinstant_value_p(inst1), tinstant_value_p(inst2), basetype))
  {
    char *str1 = pg_timestamptz_out(inst1->t);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal values have different value at their common timestamp %s",
      str1);
    pfree(str1);
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the first temporal instant is before the second one
 * @note This function is a variant of function #ensure_increasing_timestamps
 * since we must take into account inclusive/exclusive bounds
 */
bool
ensure_tinstant_increasing_time(const TInstant *inst1, const TInstant *inst2)
{
  if (inst1->t > inst2->t)
  {
    char *str1 = pg_timestamptz_out(inst1->t);
    char *str2 = pg_timestamptz_out(inst2->t);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Timestamps for temporal value must be increasing: %s, %s", str1, str2);
    pfree(str1); pfree(str2);
    return false;
  }
  return true;
}

/*****************************************************************************
 * Merge functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge two temporal instants
 * @param[in] inst1,inst2 Temporal instants
 * @csqlfn #Temporal_merge()
 */
Temporal *
tinstant_merge(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2);
  assert(inst1->temptype == inst2->temptype);
  TInstant *instants[] = {(TInstant *) inst1, (TInstant *) inst2};
  return tinstant_merge_array(instants, 2);
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge two temporal instants
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @csqlfn #Temporal_merge()
 */
GSERIALIZED *
geoarr_merge(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 0);
  GSERIALIZED *result = geom_array_union(gsarr, count);
  /*
   * ST_Union creates MultiLineString and does not sew LineStrings into a
   * single LineString. Use ST_LineMerge to sew LineStrings.
   * https://postgis.net/docs/ST_Union.html
   */
  if (gserialized_get_type(result) == MULTILINETYPE)
  {
    LWGEOM *geom = lwgeom_from_gserialized(result);
    LWGEOM *geom1 = lwgeom_linemerge_directed(geom, 0);
    GSERIALIZED *tmp = gserialized_from_lwgeom(geom1, NULL);
    pfree(result);
    result = tmp;
  }
  return result;
}

/**
 * @brief Merge an array of temporal geo instants performing a spatial union
 * of the values that have the same timestamp (iterator function)
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the array
 * @param[out] newcount Number of values in the output array
 * @pre The number of elements in the array is greater than 1 and the instants
 * are sorted
 */
TInstant **
tgeoinst_merge_array_iter(TInstant **instants, int count, int *newcount)
{
  assert(instants); assert(count > 1);
  assert(tgeo_type(instants[0]->temptype));

  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  int i = 0, count1 = 0;
  while (i < count)
  {
    int j = i;
    TimestampTz t = instants[i]->t;
    while (j < count && instants[j]->t == t)
      j++;
    if (j == i + 1)
    {
      newinstants[count1++] = tinstant_copy(instants[i++]);
      continue;
    }
    int ngeos = j - i;
    GSERIALIZED **gsarr = palloc(sizeof(GSERIALIZED *) * ngeos);
    for (int k = 0; k < ngeos; k++)
      gsarr[k] = DatumGetGserializedP(tinstant_value_p(instants[k + i]));
    GSERIALIZED *gs = geoarr_merge(gsarr, ngeos);
    pfree(gsarr);
    newinstants[count1++] = tinstant_make(PointerGetDatum(gs),
      instants[i]->temptype, instants[i]->t);
    i = j;
  }
  *newcount = count1;
  return newinstants;
}

/**
 * @brief Merge an array of temporal instants (iterator function)
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the array
 * @param[out] newcount Number of values in the output array
 * @pre The number of elements in the array is greater than 1 and the elemets
 * are sorted
 */
TInstant **
tinstant_merge_array_iter(TInstant **instants, int count, int *newcount)
{
  assert(instants); assert(count > 1);

  /* For temporal geo types, we need to perform the spatial union of the
   * values having the same timestamp */
  TInstant **instants1;
  int count1;
  if (tgeo_type(instants[0]->temptype))
    instants1 = tgeoinst_merge_array_iter(instants, count, &count1);
  else
  {
    instants1 = (TInstant **) instants;
    count1 = count;
  }

  /* Ensure the validity of the arguments and compute the bounding box */
  if (! ensure_valid_tinstarr(instants1, count1, MERGE, DISCRETE))
    return NULL;

  TInstant **newinstants = palloc(sizeof(TInstant *) * count1);
  memcpy(newinstants, instants1, sizeof(TInstant *) * count1);
  *newcount = tinstarr_remove_duplicates(newinstants, count1);
  if (tgeo_type(instants[0]->temptype))
    pfree(instants1);
  return newinstants;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge an array of temporal instants
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the array
 * @pre The number of elements in the array is greater than 1
 * @csqlfn #Temporal_merge_array()
 */
Temporal *
tinstant_merge_array(TInstant **instants, int count)
{
  assert(instants); assert(count > 1);
  tinstarr_sort((TInstant **) instants, count);
  int count1;
  TInstant **instants1 = tinstant_merge_array_iter(instants, count, &count1);

  /* Ensure the validity of the arguments and TODO compute the bounding box */
  if (! ensure_valid_tinstarr(instants1, count1, MERGE, DISCRETE))
  {
    pfree_array((void **) instants1, count1);
    return NULL;
  }

  TInstant **newinstants = palloc(sizeof(TInstant *) * count1);
  memcpy(newinstants, instants1, sizeof(TInstant *) * count1);
  int newcount = tinstarr_remove_duplicates(newinstants, count1);
  Temporal *result = (newcount == 1) ?
    (Temporal *) tinstant_copy(newinstants[0]) :
    (Temporal *) tsequence_make_exp1(newinstants, newcount, newcount, true,
      true, DISCRETE, NORMALIZE_NO, NULL);
  pfree(newinstants);
  if (tgeo_type(instants[0]->temptype))
    pfree_array((void **) instants1, count1);
  else
    pfree(instants1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge two temporal sequences
 * @param[in] seq1,seq2 Temporal sequences
 * @csqlfn #Temporal_merge()
 */
Temporal *
tsequence_merge(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq2);
  assert(seq1->temptype == seq2->temptype);
  TSequence *sequences[] = {(TSequence *) seq1, (TSequence *) seq2};
  return tsequence_merge_array(sequences, 2);
}

/**
 * @brief Merge an array of temporal discrete sequences
 * @note The function does not assume that the values in the array are strictly
 * ordered on time, i.e., the intersection of the bounding boxes of two values
 * may be a period. For this reason two passes are necessary.
 * @param[in] sequences Array of temporal sequences
 * @param[in] count Number of elements in the array
 * @return Result value that can be either a temporal instant or a temporal
 * discrete sequence
 */
Temporal *
tdiscseq_merge_array(TSequence **sequences, int count)
{
  assert(sequences);
  /* Validity test will be done in #tinstant_merge_array */
  /* Collect the composing instants */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += sequences[i]->count;
  TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
  int ninsts = 0;
  for (int i = 0; i < count; i++)
    for (int j = 0; j < sequences[i]->count; j++)
      instants[ninsts++] = (TInstant *) TSEQUENCE_INST_N(sequences[i], j);
  /* Create the result */
  Temporal *result = tinstant_merge_array(instants, totalcount);
  pfree(instants);
  return result;
}

/**
 * @brief Merge an array of temporal geo sequences (iterator function)
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @param[out] totalcount Number of elements in the resulting array
 * @note The values in the array may overlap on a single instant.
 */
static TSequence **
tgeoseq_merge_array_iter(TSequence **sequences, int count,
  int *totalcount)
{
  assert(sequences); assert(count > 0); assert(totalcount);

  /* Test the validity of the composing sequences, while performing spatial
   * union of the values for the instants with the same timestamp */
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  TSequence **tofree = palloc(sizeof(TSequence *) * 2 * count);
  int nfree = 0;
  /* Test the validity of the composing sequences */
  const TSequence *seq1 = sequences[0];
  for (int i = 1; i < count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
    const TSequence *seq2 = sequences[i];
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
    TInstant *newinst = NULL;
    if (inst1->t > inst2->t)
    {
      char *str1 = pg_timestamptz_out(inst1->t);
      char *str2 = pg_timestamptz_out(inst2->t);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The temporal values cannot overlap on time: %s, %s", str1, str2);
      pfree(str1); pfree(str2);
      return NULL;
    }
    else if (inst1->t == inst2->t && seq1->period.upper_inc &&
      seq2->period.lower_inc)
    {
      GSERIALIZED *gsarr[2];
      gsarr[0] = DatumGetGserializedP(tinstant_value_p(inst1));
      gsarr[1] = DatumGetGserializedP(tinstant_value_p(inst2));
      GSERIALIZED *gs = geoarr_merge(gsarr, 2);
      newinst = tinstant_make(PointerGetDatum(gs), inst1->temptype, inst1->t);
      pfree(gs);
    }
    if (newinst)
    {
      int ninsts = Max(seq1->count, seq2->count);
      TInstant **instants = palloc(sizeof(TInstant *) * ninsts);
      for (int j = 0; j < seq1->count - 1; j++)
        instants[j] = (TInstant *) TSEQUENCE_INST_N(seq1, j);
      instants[seq1->count - 1] = newinst;
      TSequence *newseq1 = tsequence_make_exp1(instants, seq1->count,
        seq1->count, seq1->period.lower_inc, seq1->period.upper_inc, STEP,
        NORMALIZE_NO, NULL);
      instants[0] = newinst;
      for (int j = 1; j < seq2->count; j++)
        instants[j] = (TInstant *) TSEQUENCE_INST_N(seq2, j);
      TSequence *newseq2 = tsequence_make_exp1(instants, seq2->count,
        seq2->count, seq2->period.lower_inc, seq2->period.upper_inc, STEP,
        NORMALIZE_NO, NULL);
      tofree[nfree++] = newsequences[i - 1] = newseq1;
      tofree[nfree++] = newseq2;
      pfree(instants); pfree(newinst);
      seq1 = newseq2;
    }
    else
    {
      newsequences[i - 1] = (TSequence *) seq1;
      seq1 = seq2;
    }
  }
  newsequences[count - 1] = (TSequence *) seq1;
  TSequence **result = tseqarr_normalize(newsequences, count, totalcount);
  pfree_array((void **) tofree, nfree);
  return result;
}

/**
 * @brief Merge an array of temporal sequences
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @param[out] totalcount Number of elements in the resulting array
 * @note The values in the array may overlap on a single instant.
 */
static TSequence **
tcontseq_merge_array_iter(TSequence **sequences, int count, int *totalcount)
{
  assert(sequences); assert(totalcount);
  if (count > 1)
    tseqarr_sort((TSequence **) sequences, count);

  /* For temporal geo types, we need to perform the spatial union of the
   * values having the same timestamp */
  if (tgeo_type(sequences[0]->temptype))
    return tgeoseq_merge_array_iter(sequences, count, totalcount);

  /* Test the validity of the composing sequences */
  const TSequence *seq1 = sequences[0];
  meosType basetype = temptype_basetype(seq1->temptype);
  for (int i = 1; i < count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
    const TSequence *seq2 = sequences[i];
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
    char *str1;
    if (inst1->t > inst2->t)
    {
      char *str2;
      str1 = pg_timestamptz_out(inst1->t);
      str2 = pg_timestamptz_out(inst2->t);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The temporal values cannot overlap on time: %s, %s", str1, str2);
      pfree(str1); pfree(str2);
      return NULL;
    }
    else if (inst1->t == inst2->t && seq1->period.upper_inc &&
      seq2->period.lower_inc)
    {
      if (! ensure_tinstant_same_value(inst1, inst2, basetype))
        return NULL;
    }
    seq1 = seq2;
  }
  return tseqarr_normalize(sequences, count, totalcount);
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge an array of temporal sequences
 * @param[in] sequences Array of values
 * @param[in] count Number of elements in the array
 * @note The values in the array may overlap on a single instant.
 * @csqlfn #Temporal_merge_array()
 */
Temporal *
tsequence_merge_array(TSequence **sequences, int count)
{
  assert(sequences);
  assert(count > 0);

  /* Discrete sequences */
  if (MEOS_FLAGS_DISCRETE_INTERP(sequences[0]->flags))
    return tdiscseq_merge_array(sequences, count);

  /* Continuous sequences */
  int totalcount;
  TSequence **newseqs = tcontseq_merge_array_iter(sequences, count, &totalcount);
  Temporal *result;
  if (totalcount == 1)
  {
    result = (Temporal *) newseqs[0];
    pfree(newseqs);
  }
  else
    /* Normalization was done at function tcontseq_merge_array_iter */
    result = (Temporal *) tsequenceset_make_free(newseqs, totalcount,
      NORMALIZE_NO);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge two temporal sequence sets
 * @param[in] ss1,ss2 Temporal sequence set
 * @csqlfn #Temporal_merge()
 */
TSequenceSet *
tsequenceset_merge(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2);
  assert(ss1->temptype == ss2->temptype);
  TSequenceSet *seqsets[] = {(TSequenceSet *) ss1, (TSequenceSet *) ss2};
  return tsequenceset_merge_array(seqsets, 2);
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Merge an array of temporal sequence sets
 * @param[in] seqsets Array of sequence sets
 * @param[in] count Number of elements in the array
 * @note The values in the array may overlap in a single instant.
 * @csqlfn #Temporal_merge_array()
 */
TSequenceSet *
tsequenceset_merge_array(TSequenceSet **seqsets, int count)
{
  assert(seqsets);
  assert(count > 0);
  /* Collect the composing sequences */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += seqsets[i]->count;
  TSequence **sequences = palloc0(sizeof(TSequence *) * totalcount);
  int nseqs = 0;
  for (int i = 0; i < count; i++)
    for (int j = 0; j < seqsets[i]->count; j++)
      sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(seqsets[i], j);
  int newcount;
  TSequence **newseqs = tcontseq_merge_array_iter(sequences, totalcount,
    &newcount);
  pfree(sequences);
  return tsequenceset_make_free(newseqs, newcount, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Return two temporal values transformed into a common subtype
 * @param[in] temp1,temp2 Input values
 * @param[out] out1,out2 Output values
 * @note Each of the output values may be equal to the input values to avoid
 * unnecessary calls to palloc. The calling function must test whether
 * (tempx == outx) to determine if a pfree is needed.
 */
static void
temporal_convert_same_subtype(const Temporal *temp1, const Temporal *temp2,
  Temporal **out1, Temporal **out2)
{
  assert(temptype_subtype(temp1->subtype));
  assert(temp1->temptype == temp2->temptype);

  /* If both are of the same subtype do nothing */
  if (temp1->subtype == temp2->subtype)
  {
    interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
    interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
    if (interp1 == interp2)
    {
      *out1 = (Temporal *) temp1;
      *out2 = (Temporal *) temp2;
    }
    else
    {
      interpType interp = Max(interp1, interp2);
      *out1 = (Temporal *) temporal_tsequenceset(temp1, interp);
      *out2 = (Temporal *) temporal_tsequenceset(temp2, interp);
    }
    return;
  }

  /* Different subtype */
  bool swap = false;
  const Temporal *new1, *new2;
  if (temp1->subtype > temp2->subtype)
  {
    new1 = temp2;
    new2 = temp1;
    swap = true;
  }
  else
  {
    new1 = temp1;
    new2 = temp2;
  }

  Temporal *new;
  if (new1->subtype == TINSTANT)
  {
    interpType interp = MEOS_FLAGS_GET_INTERP(new2->flags);
    if (new2->subtype == TSEQUENCE)
      new = (Temporal *) tinstant_to_tsequence((TInstant *) new1, interp);
    else /* new2->subtype == TSEQUENCESET */
      new = (Temporal *) tinstant_to_tsequenceset((TInstant *) new1, interp);
  }
  else /* new1->subtype == TSEQUENCE && new2->subtype == TSEQUENCESET */
    new = (Temporal *) tsequence_to_tsequenceset((TSequence *) new1);
  if (swap)
  {
    *out1 = (Temporal *) temp1;
    *out2 = new;
  }
  else
  {
    *out1 = new;
    *out2 = (Temporal *) temp2;
  }
  return;
}

/**
 * @ingroup meos_temporal_modif
 * @brief Merge two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @return Return @p NULL if both arguments are @p NULL.
 * If one argument is null, return the other argument.
 * @csqlfn #Temporal_merge()
 */
Temporal *
temporal_merge(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *result;
  /* Cannot do anything with null inputs */
  if (! temp1 && ! temp2)
    return NULL;
  /* One argument is null, return a copy of the other temporal */
  if (! temp1)
    return temporal_copy(temp2);
  if (! temp2)
    return temporal_copy(temp1);

  /* Ensure the validity of the arguments */
  if (! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_continuous_interp(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  /* Convert to the same subtype */
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  assert(temptype_subtype(new1->subtype));
  switch (new1->subtype)
  {
    case TINSTANT:
      result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tsequence_merge((TSequence *) new1,
        (TSequence *) new2);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tsequenceset_merge((TSequenceSet *) new1,
        (TSequenceSet *) new2);
      break;
  }
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  return result;
}

/**
 * @brief Return an array of temporal values transformed into a common subtype
 * @param[in] temparr Array of values
 * @param[in] count Number of values in the array
 * @param[in] subtype common subtype
 * @param[in] interp Interpolation
 */
static Temporal **
temporalarr_convert_subtype(Temporal **temparr, int count, uint8 subtype,
  interpType interp)
{
  assert(temparr);
  assert(temptype_subtype(subtype));
  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
  {
    uint8 subtype1 = temparr[i]->subtype;
    assert(subtype >= subtype1);
    if (subtype == subtype1)
      result[i] = temporal_copy(temparr[i]);
    else if (subtype1 == TINSTANT)
    {
      if (subtype == TSEQUENCE)
        result[i] = (Temporal *) tinstant_to_tsequence((TInstant *) temparr[i],
          interp);
      else /* subtype == TSEQUENCESET */
        result[i] = (Temporal *) tinstant_to_tsequenceset((TInstant *) temparr[i],
          interp);
    }
    else /* subtype1 == TSEQUENCE && subtype == TSEQUENCESET */
      result[i] = (Temporal *) tsequence_to_tsequenceset((TSequence *) temparr[i]);
  }
  return result;
}

/**
 * @ingroup meos_temporal_modif
 * @brief Merge an array of temporal values
 * @param[in] temparr Array of values
 * @param[in] count Number of values in the array
 * @csqlfn #Temporal_merge_array()
 */
Temporal *
temporal_merge_array(Temporal **temparr, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temparr, NULL);
  if (! ensure_positive(count))
    return NULL;

  if (count == 1)
    return temporal_copy(temparr[0]);

  /* Ensure all values have the same interpolation and, if they are spatial,
   * have the same SRID and dimensionality, and determine subtype of the
   * result */
  uint8 subtype, origsubtype;
  subtype = origsubtype = temparr[0]->subtype;
  interpType interp = MEOS_FLAGS_GET_INTERP(temparr[0]->flags);
  bool spatial = tspatial_type(temparr[0]->temptype);
  bool convert = false;
  for (int i = 1; i < count; i++)
  {
    uint8 subtype1 = temparr[i]->subtype;
    interpType interp1 = MEOS_FLAGS_GET_INTERP(temparr[i]->flags);
    if (subtype != subtype1 || interp != interp1)
    {
      convert = true;
      uint8 newsubtype = Max(subtype, subtype1);
      interpType newinterp = Max(interp, interp1);
      /* A discrete TSequence cannot be converted to a continuous TSequence */
      if (subtype == TSEQUENCE && subtype1 == TSEQUENCE && interp != newinterp)
        newsubtype = TSEQUENCESET;
      subtype = newsubtype;
      interp |= newinterp;
    }
    if (spatial && ! ensure_spatial_validity(temparr[0], temparr[i]))
      return NULL;
  }
  /* Convert all temporal values to a single subtype if needed */
  Temporal **newtemps;
  if (convert)
    newtemps = temporalarr_convert_subtype(temparr, count, subtype, interp);
  else
    newtemps = (Temporal **) temparr;

  Temporal *result;
  assert(temptype_subtype(subtype));
  switch (subtype)
  {
    case TINSTANT:
      result = (Temporal *) tinstant_merge_array((TInstant **) newtemps,
        count);
      break;
    case TSEQUENCE:
      result = (Temporal *) tsequence_merge_array((TSequence **) newtemps,
        count);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tsequenceset_merge_array(
        (TSequenceSet **) newtemps, count);
  }
  if (newtemps != (Temporal **) temparr)
    pfree_array((void **) newtemps, count);
  return result;
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @brief Insert the second temporal value into the first one
 */
Temporal *
tcontseq_insert(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq2);
  assert(seq1->temptype == seq2->temptype);
  /* Order the two sequences */
  const TSequence *seq; /* for swaping */
  TInstant *instants[2] = {0};
  instants[0] = (TInstant *) TSEQUENCE_INST_N(seq1, seq1->count - 1);
  instants[1] = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  if (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) > 0)
  {
    seq = seq1; seq1 = seq2; seq2 = seq;
    instants[0] = (TInstant *) TSEQUENCE_INST_N(seq1, seq1->count - 1);
    instants[1] = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  }

  /* Add the sequences in the array to merge */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);
  TSequence *tofree = NULL;
  TSequence **sequences = palloc(sizeof(TSequence *) * 3);
  sequences[0] = (TSequence *) seq1;
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
      sequences[nseqs++] = tofree;
   }
  }
  else /* overlap on the boundary */
  {
    meosType basetype = temptype_basetype(seq1->temptype);
    if (! ensure_tinstant_same_value(instants[0], instants[1], basetype))
      return NULL;
  }
  sequences[nseqs++] = (TSequence *) seq2;

  int count;
  TSequence **newseqs = tcontseq_merge_array_iter(sequences, nseqs, &count);
  Temporal *result;
  if (count == 1)
  {
    result = (Temporal *) newseqs[0];
    pfree(newseqs);
  }
  else
    /* Normalization was done at function tcontseq_merge_array_iter */
    result = (Temporal *) tsequenceset_make_free(newseqs, count, NORMALIZE_NO);
  if (tofree)
    pfree(tofree);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Insert the second temporal value into the first one
 * @param[in] seq1,seq2 Temporal sequences
 * @param[in] connect True when the second temporal sequence is connected in
 * the result to the instants before and after, if any
 */
Temporal *
tsequence_insert(const TSequence *seq1, const TSequence *seq2, bool connect)
{
  assert(seq1); assert(seq2);
  assert(seq1->temptype == seq2->temptype);

  if (MEOS_FLAGS_DISCRETE_INTERP(seq1->flags) || ! connect)
    return (Temporal *) tsequence_merge(seq1, seq2);
  else
    return (Temporal *) tcontseq_insert(seq1, seq2);
}

/**
 * @brief Delete a timestamp from a continuous temporal sequence
 * @details If an instant has the same timestamp, it will be removed. If the
 * instant is in the middle, it will be connected to the next and previous
 * instants in the result. If the instant is at the beginning or at the end,
 * the time span of the sequence is reduced. In this case the bounds of the
 * sequence will be adjusted accordingly, inclusive at the beginning and
 * exclusive at the end.
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 */
TSequence *
tcontseq_delete_timestamptz(const TSequence *seq, TimestampTz t)
{
  assert(seq);
  /* Bounding box test */
  if (! contains_span_timestamptz(&seq->period, t))
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
  TSequence *result = tsequence_make(instants, ninsts, lower_inc1, upper_inc1,
    interp, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz from a temporal value
 * @param[in] seq Temporal sequence
 * @param[in] t Timestamp
 * @param[in] connect True when the instants before and after the timestamp,
 * if any, are connected in the result
 * @csqlfn #Temporal_delete_timestamptz()
 */
Temporal *
tsequence_delete_timestamptz(const TSequence *seq, TimestampTz t, bool connect)
{
  assert(seq);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return (Temporal *) tdiscseq_minus_timestamptz(seq, t);
  else
    return connect ?
      (Temporal *) tcontseq_minus_timestamptz(seq, t) :
      (Temporal *) tcontseq_delete_timestamptz(seq, t);
}

/**
 * @brief Delete a timestamptz from a continuous temporal sequence
 * @details If an instant has the same timestamp, it will be removed. If the
 * instant is in the middle, it will be connected to the next and previous
 * instants in the result. If the instant is at the beginning or at the end,
 * the time span of the sequence is reduced. In this case the bounds of the
 * sequence will be adjusted accordingly, inclusive at the beginning and
 * exclusive at the end.
 * @param[in] seq Temporal sequence
 * @param[in] s Timestamp set
 */
TSequence *
tcontseq_delete_tstzset(const TSequence *seq, const Set *s)
{
  assert(seq); assert(s);
  /* Singleton timestamp set */
  if (s->count == 1)
    return tcontseq_delete_timestamptz(seq,
      DatumGetTimestampTz(SET_VAL_N(s, 0)));

  /* Bounding box test */
  Span p;
  set_set_span(s, &p);
  if (! overlaps_span_span(&seq->period, &p))
    return tsequence_copy(seq);

  const TInstant *inst;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst = TSEQUENCE_INST_N(seq, 0);
    if (contains_set_value(s, TimestampTzGetDatum(inst->t)))
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
  while (i < seq->count && j < s->count)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(s, j));
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
  TSequence *result = tsequence_make(instants, ninsts, lower_inc1, upper_inc1,
    interp, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz set from a temporal value
 * @param[in] seq Temporal sequence
 * @param[in] s Set
 * @param[in] connect True when the instants before and after the set,
 * if any, are connected in the result
 * @csqlfn #Temporal_delete_tstzset()
 */
Temporal *
tsequence_delete_tstzset(const TSequence *seq, const Set *s, bool connect)
{
  assert(seq); assert(s);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return (Temporal *) tdiscseq_restrict_tstzset(seq, s, REST_MINUS);
  else
    return connect ?
      (Temporal *) tcontseq_delete_tstzset(seq, s) :
      (Temporal *) tcontseq_minus_tstzset(seq, s);
}

/**
 * @brief Delete a timestamptz span from a continuous temporal sequence
 * @param[in] seq Temporal sequence
 * @param[in] s Span
 */
TSequence *
tcontseq_delete_tstzspan(const TSequence *seq, const Span *s)
{
  assert(seq); assert(s);
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, s))
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
    if (! contains_span_timestamptz(s, inst->t))
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
  TSequence *result = tsequence_make(instants, ninsts, lower_inc1, upper_inc1,
    interp, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz span from a temporal sequence
 * @param[in] seq Temporal sequence
 * @param[in] s Span
 * @param[in] connect True when the instants before and after the span, if any,
 * are connected in the result
 * @csqlfn #Temporal_delete_tstzspan()
 */
Temporal *
tsequence_delete_tstzspan(const TSequence *seq, const Span *s, bool connect)
{
  assert(seq); assert(s);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return (Temporal *) tsequence_restrict_tstzspan(seq, s, REST_MINUS);
  else
    return connect ?
      (Temporal *) tcontseq_delete_tstzspan(seq, s) :
      (Temporal *) tcontseq_minus_tstzspan(seq, s);
}

/**
 * @brief Delete a timestamptz span set from a continuous temporal sequence
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 */
TSequence *
tcontseq_delete_tstzspanset(const TSequence *seq, const SpanSet *ss)
{
  assert(seq); assert(ss);
  /* Bounding box test */
  if (! overlaps_span_span(&seq->period, &ss->span))
    return tsequence_copy(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (contains_spanset_timestamptz(ss, TSEQUENCE_INST_N(seq, 0)->t))
      return NULL;
    return tsequence_copy(seq);
  }

  /* Singleton span set */
  if (ss->count == 1)
    return tcontseq_delete_tstzspan(seq, SPANSET_SP_N(ss, 0));

  /* General case */
  TInstant **instants = palloc0(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  bool lower_inc1 = seq->period.lower_inc;
  bool upper_inc1 = seq->period.upper_inc;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (! contains_spanset_timestamptz(ss, inst->t))
      instants[ninsts++] = (TInstant *) inst;
    else /* instant is inside the span set */
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
  TSequence *result = tsequence_make(instants, ninsts, lower_inc1, upper_inc1,
    interp, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz span set from a temporal value
 * @param[in] seq Temporal sequence
 * @param[in] ss Span set
 * @param[in] connect True when the instants before and after the span set, if
 * any, are connected in the result
 * @csqlfn #Temporal_delete_tstzspanset()
 */
Temporal *
tsequence_delete_tstzspanset(const TSequence *seq, const SpanSet *ss,
  bool connect)
{
  assert(seq); assert(ss);
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
    return (Temporal *) tdiscseq_restrict_tstzspanset(seq, ss, REST_MINUS);
  else
    return connect ?
      (Temporal *) tcontseq_delete_tstzspanset(seq, ss) :
      (Temporal *) tcontseq_restrict_tstzspanset(seq, ss, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Insert the second temporal value into the first one
 * @param[in] ss1,ss2 Temporal sequence sets
 */
TSequenceSet *
tsequenceset_insert(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2);
  assert(ss1->temptype == ss2->temptype);
  TSequenceSet *result;
  TInstant *instants[2] = {0};
  interpType interp = MEOS_FLAGS_GET_INTERP(ss1->flags);
  int count;

  /* Order the two sequence sets */
  const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, 0);
  const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, 0);
  const TSequenceSet *ss; /* for swaping */
  if (left_span_span(&seq2->period, &seq1->period))
  {
    ss = ss1; ss1 = ss2; ss2 = ss;
  }

  /* Singleton sequence sets */
  if (ss1->count == 1 && ss2->count == 1)
  {
    Temporal *temp = tcontseq_insert(seq1, seq2);
    if (temp->subtype == TSEQUENCESET)
      return (TSequenceSet *) temp;
    return tsequence_to_tsequenceset_free((TSequence *) temp);
  }

  /* If one sequence set is before the other one add the potential gap between
   * the two and call directly the merge function */
  if (left_span_span(&ss1->period, &ss2->period))
  {
    if (ss1->period.upper_inc && ss2->period.lower_inc)
    {
      seq1 = TSEQUENCESET_SEQ_N(ss1, ss1->count - 1);
      seq2 = TSEQUENCESET_SEQ_N(ss2, 0);
      instants[0] = (TInstant *) TSEQUENCE_INST_N(seq1, seq1->count - 1);
      instants[1] = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
      count = (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) == 0) ?
        1 : 2;
      TSequence *seq = tsequence_make(instants, count, true, true, interp,
        NORMALIZE_NO);
      TSequenceSet *gap = tsequence_to_tsequenceset_free(seq);
      TSequenceSet *seqsets[] = {(TSequenceSet *) ss1, gap,
        (TSequenceSet *) ss2};
      return tsequenceset_merge_array(seqsets, 3);
    }
    else
    {
      TSequenceSet *seqsets[] = {(TSequenceSet *) ss1, (TSequenceSet *) ss2};
      return tsequenceset_merge_array(seqsets, 2);
    }
  }

  /*
   * ss1   |---|         |---|         |---|
   * ss2          |---|         |---|
   * additional sequences
   *           |--|   |--|   |--|   |--|
   */
  count = ss1->count + ss2->count + Min(ss1->count, ss2->count) * 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  TSequence **tofree = palloc(sizeof(TSequence *) *
    Min(ss1->count, ss2->count) * 2);
  meosType basetype = temptype_basetype(ss1->temptype);
  /* Add the first sequence of ss1 to the result */
  sequences[0] = (TSequence *) TSEQUENCESET_SEQ_N(ss1, 0);
  int i = 1, /* counter for the first sequence */
    j = 0,   /* counter for the second sequence */
    nseqs = 1,   /* counter for the sequences in the result */
    nfree = 0;   /* counter for the new sequences to be freed */
  while (i < ss1->count && j < ss2->count)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    int cmp1 = timestamptz_cmp_internal(
      DatumGetTimestampTz(sequences[nseqs - 1]->period.upper),
      DatumGetTimestampTz(seq2->period.lower));
    int cmp2 = timestamptz_cmp_internal(DatumGetTimestampTz(seq2->period.upper),
      DatumGetTimestampTz(seq1->period.lower));
    /* If seq2 is between the last sequence added and seq1 */
    if (cmp1 <= 0 && cmp2 <= 0)
    {
      /* Verify that the two sequences have the same value at common instants */
      const TInstant *inst1, *inst2;
      if (cmp1 == 0 && sequences[nseqs - 1]->period.upper_inc &&
          seq2->period.lower_inc)
      {
        inst1 = TSEQUENCE_INST_N(sequences[nseqs - 1],
          sequences[nseqs - 1]->count - 1);
        inst2 = TSEQUENCE_INST_N(seq2, 0);
        if (! ensure_tinstant_same_value(inst1, inst2, basetype))
          return NULL;
      }
      if (cmp2 == 0 && seq2->period.upper_inc && seq1->period.lower_inc)
      {
        inst1 = TSEQUENCE_INST_N(seq2, seq2->count - 1);
        inst2 = TSEQUENCE_INST_N(seq1, 0);
        if (! ensure_tinstant_same_value(inst1, inst2, basetype))
          return NULL;
      }
      /* Fill the gap between the last sequence added and seq2 */
      if (sequences[nseqs - 1]->period.upper_inc && seq2->period.lower_inc)
      {
        instants[0] = (TInstant *) TSEQUENCE_INST_N(sequences[nseqs - 1],
          sequences[nseqs - 1]->count - 1);
        instants[1] = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
        count = (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) == 0) ?
          1 : 2;
        /* We put true so that it works with step interpolation */
        tofree[nfree] = tsequence_make(instants, count, true, true, interp,
          NORMALIZE_NO);
        sequences[nseqs++] = tofree[nfree++];
      }
      /* Add seq2 */
      sequences[nseqs++] = (TSequence *) seq2;
      /* Fill the gap between the seq2 and seq1 */
      if (seq2->period.upper_inc && seq1->period.lower_inc)
      {
        instants[0] = (TInstant *) TSEQUENCE_INST_N(seq2, seq2->count - 1);
        instants[1] = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
        count = (timestamptz_cmp_internal(instants[0]->t, instants[1]->t) == 0) ?
          1 : 2;
        /* We put true so that it works with step interpolation */
        tofree[nfree] = tsequence_make(instants, count, true, true, interp,
          NORMALIZE_NO);
        sequences[nseqs++] = tofree[nfree++];
      }
      i++;
      j++;
    }
    else /* consume seq1 and advance i */
    {
      sequences[nseqs++] = (TSequence *) seq1;
      i++;
    }
  }
  /* Add the remaining sequences */
  while (i < ss1->count)
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss1, i++);
  while (j < ss2->count)
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss2, j++);
  /* Construct the result */
  int newcount;
  TSequence **normseqs = tseqarr_normalize(sequences, nseqs, &newcount);
  result = tsequenceset_make_free(normseqs, newcount, NORMALIZE_NO);
  pfree_array((void **) tofree, nfree);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz from a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] t Timestamp
 * @csqlfn #Temporal_minus_timestamptz(), #Temporal_delete_timestamptz()
 */
TSequenceSet *
tsequenceset_delete_timestamptz(const TSequenceSet *ss, TimestampTz t)
{
  assert(ss);
  /* Bounding box test */
  if (! contains_span_timestamptz(&ss->period, t))
    return tsequenceset_copy(ss);

  TSequence *seq;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq = tcontseq_delete_timestamptz(TSEQUENCESET_SEQ_N(ss, 0), t);
    if (seq)
      return tsequence_to_tsequenceset_free(seq);
    return NULL;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count));
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    seq = tcontseq_delete_timestamptz(TSEQUENCESET_SEQ_N(ss, i), t);
    if (seq)
      sequences[nseqs++] = seq;
  }
  assert(nseqs > 0);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz span from a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] s Set
 * @csqlfn #Temporal_minus_tstzset(), #Temporal_delete_tstzset()
 */
TSequenceSet *
tsequenceset_delete_tstzset(const TSequenceSet *ss, const Set *s)
{
  assert(ss); assert(s);
  /* Singleton timestamp set */
  if (s->count == 1)
    return tsequenceset_delete_timestamptz(ss,
      DatumGetTimestampTz(SET_VAL_N(s, 0)));

  /* Bounding box test */
  Span s1;
  set_set_span(s, &s1);
  if (! overlaps_span_span(&ss->period, &s1))
    return tsequenceset_copy(ss);

  TSequence *seq;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq = tcontseq_delete_tstzset(TSEQUENCESET_SEQ_N(ss, 0), s);
    if (seq)
      return tsequence_to_tsequenceset_free(seq);
    return NULL;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    seq = tcontseq_delete_tstzset(TSEQUENCESET_SEQ_N(ss, i), s);
    if (seq)
      sequences[nseqs++] = seq;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz span from a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] s Span
 * @csqlfn #Temporal_minus_tstzspan(), #Temporal_delete_tstzspan()
 */
TSequenceSet *
tsequenceset_delete_tstzspan(const TSequenceSet *ss, const Span *s)
{
  assert(ss); assert(ss);
  SpanSet *sps = span_to_spanset(s);
  TSequenceSet *result = tsequenceset_delete_tstzspanset(ss, sps);
  pfree(sps);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Delete a timestamptz span from a temporal sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] ps Span set
 * @csqlfn #Temporal_minus_tstzspanset(), #Temporal_delete_tstzspanset()
 */
TSequenceSet *
tsequenceset_delete_tstzspanset(const TSequenceSet *ss, const SpanSet *ps)
{
  assert(ss); assert(ps);
  /* Bounding box test */
  if (! overlaps_span_span(&ss->period, &ps->span))
    return tsequenceset_copy(ss);

  TSequence *seq;
  interpType interp = MEOS_FLAGS_GET_INTERP(ss->flags);

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq = tcontseq_delete_tstzspanset(TSEQUENCESET_SEQ_N(ss, 0), ps);
    if (seq)
    {
      return tsequence_to_tsequenceset_free(seq);
    }
    return NULL;
  }

  /* General case */
  TSequenceSet *minus = tsequenceset_restrict_tstzspanset(ss, ps, REST_MINUS);
  /* The are minus->count - 1 holes that may be filled */
  TSequence **sequences = palloc(sizeof(TSequence *) * (minus->count * 2 - 1));
  TSequence **tofree = palloc(sizeof(TSequence *) * (minus->count - 1));
  TInstant *instants[2] = {0};
  sequences[0] = seq = (TSequence *) TSEQUENCESET_SEQ_N(minus, 0);
  const Span *s = SPANSET_SP_N(ps, 0);
  int i = 1,    /* current composing sequence */
    j = 0,      /* current composing period */
    nseqs = 1,  /* number of sequences in the currently constructed sequence */
    nfree = 0;  /* number of sequences to be freed */
  /* Skip all composing periods that are before or adjacent to seq */
  while (j < ps->count)
  {
    if (timestamptz_cmp_internal(DatumGetTimestampTz(s->upper),
          DatumGetTimestampTz(seq->period.lower)) > 0)
      break;
    s = SPANSET_SP_N(ps, j++);
  }
  seq = (TSequence *) TSEQUENCESET_SEQ_N(minus, 1);
  while (i < ss->count && j < ps->count)
  {
    if (timestamptz_cmp_internal(DatumGetTimestampTz(s->upper),
          DatumGetTimestampTz(seq->period.lower) <= 0))
    {
      instants[0] = (TInstant *) TSEQUENCE_INST_N(sequences[nseqs - 1],
        sequences[nseqs - 1]->count - 1);
      instants[1] = (TInstant *) TSEQUENCE_INST_N(seq, 0);
      int count = (timestamptz_cmp_internal(instants[0]->t,
        instants[1]->t) == 0) ? 1 : 2;
      /* We put true so that it works with step interpolation */
      tofree[nfree] = tsequence_make(instants, count, true, true, interp,
        NORMALIZE_NO);
      sequences[nseqs++] = tofree[nfree++];
    }
    sequences[nseqs++] = seq;
    seq = (TSequence *) TSEQUENCESET_SEQ_N(minus, ++i);
    s = SPANSET_SP_N(ps, j++);
  }
  /* Add remaining sequences to the result */
  while (i < ss->count)
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(minus, i++);
  /* Construct the result */
  int newcount;
  TSequence **normseqs = tseqarr_normalize(sequences, nseqs, &newcount);
  TSequenceSet *result = tsequenceset_make_free(normseqs, newcount,
    NORMALIZE_NO);
  pfree_array((void **) tofree, nfree); pfree(minus);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_modif
 * @brief Insert the second temporal value into the first one
 * @param[in] temp1,temp2 Temporal values
 * @param[in] connect True when the second temporal value is connected in the
 * result to the instants before and after, if any
 * @csqlfn #Temporal_insert()
 */
Temporal *
temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_same_continuous_interp(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  /* Convert to the same subtype */
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  Temporal *result;
  assert(temptype_subtype(new1->subtype));
  switch (new1->subtype)
  {
    case TINSTANT:
      result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tsequence_insert((TSequence *) new1,
        (TSequence *) new2, connect);
      break;
    default: /* TSEQUENCESET */
      result = connect ?
        (Temporal *) tsequenceset_merge((TSequenceSet *) new1,
          (TSequenceSet *) new2) :
        (Temporal *) tsequenceset_insert((TSequenceSet *) new1,
          (TSequenceSet *) new2);
  }
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  return result;
}

/**
 * @ingroup meos_temporal_modif
 * @brief Update the first temporal value with the second one
 * @param[in] temp1,temp2 Temporal values
 * @param[in] connect True when the second temporal value is connected in the
 * result to the instants before and after, if any
 * @csqlfn #Temporal_update()
 */
Temporal *
temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2) ||
      ! ensure_same_continuous_interp(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  SpanSet *ss = temporal_time(temp2);
  Temporal *rest = temporal_restrict_tstzspanset(temp1, ss, REST_MINUS);
  if (! rest)
    return temporal_copy((Temporal *) temp2);
  Temporal *result = temporal_insert(rest, temp2, connect);
  pfree(rest); pfree(ss);
  return (Temporal *) result;
}

/**
 * @ingroup meos_temporal_modif
 * @brief Delete a timestamp from a temporal value
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] connect True when the instants before and after the timestamp,
 * if any, are connected in the result
 * @csqlfn #Temporal_delete_timestamptz()
 */
Temporal *
temporal_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_timestamptz((TInstant *) temp, t,
        REST_MINUS);
    case TSEQUENCE:
      return (Temporal *) tsequence_delete_timestamptz((TSequence *) temp, t,
        connect);
    default: /* TSEQUENCESET */
      return connect ?
        (Temporal *) tsequenceset_restrict_timestamptz((TSequenceSet *) temp, t,
          REST_MINUS) :
        (Temporal *) tsequenceset_delete_timestamptz((TSequenceSet *) temp, t);
  }
}

/**
 * @ingroup meos_temporal_modif
 * @brief Delete a timestamp set from a temporal value connecting the instants
 * before and after the given timestamp, if any
 * @param[in] temp Temporal value
 * @param[in] s Timestamp set
 * @param[in] connect True when the instants before and after the timestamp
 * set are connected in the result
 * @csqlfn #Temporal_delete_tstzset()
 */
Temporal *
temporal_delete_tstzset(const Temporal *temp, const Set *s, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_TSTZSET(s, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzset((TInstant *) temp, s,
        REST_MINUS);
    case TSEQUENCE:
      return (Temporal *) tsequence_delete_tstzset((TSequence *) temp, s,
        connect);
    default: /* TSEQUENCESET */
      return connect ?
        (Temporal *) tsequenceset_delete_tstzset((TSequenceSet *) temp, s) :
        (Temporal *) tsequenceset_restrict_tstzset((TSequenceSet *) temp, s,
          REST_MINUS);
  }
}

/**
 * @ingroup meos_temporal_modif
 * @brief Delete a timestamptz span from a temporal value
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @param[in] connect True when the instants before and after the span, if any,
 * are connected in the result
 * @csqlfn #Temporal_delete_tstzspan()
 */
Temporal *
temporal_delete_tstzspan(const Temporal *temp, const Span *s, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_TSTZSPAN(s, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzspan((TInstant *) temp, s,
        REST_MINUS);
    case TSEQUENCE:
      return (Temporal *) tsequence_delete_tstzspan((TSequence *) temp, s,
        connect);
    default: /* TSEQUENCESET */
      return connect ?
        (Temporal *) tsequenceset_delete_tstzspan((TSequenceSet *) temp, s) :
        (Temporal *) tsequenceset_restrict_tstzspan((TSequenceSet *) temp, s,
          REST_MINUS);
  }
}

/**
 * @ingroup meos_temporal_modif
 * @brief Delete a timestamptz span set from a temporal value
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @param[in] connect True when the instants before and after the span set, if
 * any, are connected in the result
 * @csqlfn #Temporal_delete_tstzspanset()
 */
Temporal *
temporal_delete_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_TSTZSPANSET(ss, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_restrict_tstzspanset((TInstant *) temp, ss,
        REST_MINUS);
    case TSEQUENCE:
      return (Temporal *) tsequence_delete_tstzspanset((TSequence *) temp, ss,
        connect);
    default: /* TSEQUENCESET */
      return connect ?
        (Temporal *) tsequenceset_delete_tstzspanset((TSequenceSet *) temp, ss) :
        (Temporal *) tsequenceset_restrict_tstzspanset((TSequenceSet *) temp, ss,
          REST_MINUS);
  }
}

/*****************************************************************************
 * Append functions
 ****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Append an instant to a temporal sequence accounting for potential gaps
 * @param[in,out] seq Temporal sequence
 * @param[in] inst Temporal instant
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap, may be `NULL`
 * @param[in] expand True when reserving space for additional instants
 * @csqlfn #Temporal_append_tinstant()
 * @return When the sequence passed as first argument has space for adding the
 * instant, the function returns the updated sequence. Otherwise, a NEW
 * sequence is returned and the input sequence is freed.
 * @note Always use the function to overwrite the existing sequence as in:
 * @code
 * seq = tsequence_append_tinstant(seq, inst, ...);
 * @endcode
 */
Temporal *
tsequence_append_tinstant(TSequence *seq, const TInstant *inst, double maxdist,
  const Interval *maxt, bool expand)
{
  assert(seq); assert(inst); assert(seq->temptype == inst->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  meosType basetype = temptype_basetype(seq->temptype);
  TInstant *last = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 1);
#if NPOINT
  if (last->temptype == T_TNPOINT && interp != DISCRETE &&
      ! ensure_same_rid_tnpointinst(inst, last))
    return NULL;
#endif
  if (! ensure_tinstant_increasing_time(last, inst))
    return NULL;

  Datum value1 = tinstant_value_p(last);
  Datum value = tinstant_value_p(inst);
  if (last->t == inst->t)
  {
    if (seq->period.upper_inc)
    {
      if (! ensure_tinstant_same_value(last, inst, basetype))
        return NULL;
      /* Do not add the new instant if new instant is equal to be last one */
      return (Temporal *) tsequence_copy(seq);
    }
    /* Exclusive upper bound and different value => result is a sequence set */
    else if (interp == LINEAR && ! datum_eq(value1, value, basetype))
    {
      TSequence *sequences[2];
      sequences[0] = (TSequence *) seq;
      sequences[1] = tinstant_to_tsequence(inst, LINEAR);
      TSequenceSet *result = tsequenceset_make(sequences, 2, NORMALIZE_NO);
      pfree(sequences[1]);
      return (Temporal *) result;
    }
  }

  /* Take into account the maximum distance and/or the maximum interval */
  if (maxdist > 0.0 || maxt)
  {
    bool split = false;
    if (maxdist > 0.0 && ! datum_eq(value1, value, basetype))
    {
      double dist = datum_distance(value1, value, basetype, seq->flags);
      if (dist > maxdist)
        split = true;
    }
    /* If there is not already a split by distance */
    if (maxt && ! split)
    {
      Interval *duration = minus_timestamptz_timestamptz(inst->t, last->t);
      if (pg_interval_cmp((Interval *) duration, (Interval *) maxt) > 0)
        split = true;
      pfree(duration);
    }
    /* If split => result is a sequence set */
    if (split)
    {
      TSequence *sequences[2];
      sequences[0] = (seq->count < seq->maxcount) ?
        tsequence_compact((TSequence *) seq) : seq;
      /* Arbitrary initialization to 64 elements if in expandable mode */
      sequences[1] = tsequence_make_exp((TInstant **) &inst, 1,
        expand ? 64 : 1, true, true, interp, NORMALIZE_NO);
      TSequenceSet *result = tsequenceset_make_exp(sequences, 2,
        expand ? 64 : 2, NORMALIZE_NO);
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
    Datum value2 = tinstant_value_p(penult);
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
    size_t size_last = DOUBLE_PAD(VARSIZE(last));
    char *new = (char *) last + size_last;
    size_t size_seq = VARSIZE(seq);
    size_t avail_size = (char *) seq + size_seq - new;
    if (size > avail_size)
      /* There is not enough available space */
      break;

    /* There is enough space to add the new instant */
    if (count != seq->count)
    {
      /* Update the offsets array and the count when adding one instant */
      (TSEQUENCE_OFFSETS_PTR(seq))[count - 1] =
        (TSEQUENCE_OFFSETS_PTR(seq))[count - 2] + size_last;
      seq->count++;
    }
    memcpy(new, inst, VARSIZE(inst));
    /* Expand the bounding box and return */
    tsequence_expand_bbox(seq, inst);
    return (Temporal *) seq;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count - 1; i++)
    instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i);
  instants[count - 1] = (TInstant *) inst;
  int maxcount;
  if (expand)
  {
    maxcount = seq->maxcount;
    if (count > seq->maxcount)
    {
      maxcount *= 2;
#if DEBUG_EXPAND
      meos_error(WARNING, 0, " Sequence -> %d ", maxcount);
#endif /* DEBUG_EXPAND */
    }
  }
  else
    maxcount = count;

  /* Get the bounding box size */
  size_t bboxsize = DOUBLE_PAD(temporal_bbox_size(seq->temptype));
  bboxunion bbox, bbox1;
  memcpy(&bbox, TSEQUENCE_BBOX_PTR(seq), bboxsize);
  tinstant_set_bbox(inst, &bbox1);
  bbox_expand(&bbox1, &bbox, seq->temptype);
  TSequence *result = tsequence_make_exp1(instants, count, maxcount,
    seq->period.lower_inc, true, interp, NORMALIZE_NO, &bbox);
  pfree(instants);
#if MEOS
  if (expand)
    pfree(seq);
#endif /* MEOS */
  return (Temporal *) result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Append a sequence to a temporal sequence
 * @param[in,out] seq1 Temporal sequence
 * @param[in] seq2 Temporal sequence to append
 * @param[in] expand True when reserving space for additional sequences
 * @csqlfn #Temporal_append_tsequence()
 */
Temporal *
tsequence_append_tsequence(const TSequence *seq1, const TSequence *seq2,
  bool expand UNUSED)
{
  assert(seq1); assert(seq2);
  assert(seq1->temptype == seq2->temptype);
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  assert(interp1 == MEOS_FLAGS_GET_INTERP(seq2->flags));
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
  if (! ensure_tinstant_increasing_time(inst1, inst2))
    return NULL;
  else if (inst1->t == inst2->t && seq1->period.upper_inc &&
    seq2->period.lower_inc)
  {
    meosType basetype = temptype_basetype(seq1->temptype);
    if (! ensure_tinstant_same_value(inst1, inst2, basetype))
      return NULL;
  }
#if NPOINT
  if (inst1->temptype == T_TNPOINT && interp1 != DISCRETE &&
      ! ensure_same_rid_tnpointinst(inst1, inst2))
    return NULL;
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
  TSequence *sequences[2];
  sequences[0] = (TSequence *) seq1;
  sequences[1] = (TSequence *) seq2;
  if (interp1 == DISCRETE)
    return tsequence_merge_array(sequences, 2);
  else
    return (Temporal *) tsequenceset_make(sequences, 2, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Append an instant to a temporal sequence set
 * @param[in,out] ss Temporal sequence set
 * @param[in] inst Temporal instant
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap, may be `NULL`
 * @param[in] expand True when reserving space for additional instants
 * @csqlfn #Temporal_append_tinstant()
 * @return When the sequence set passed as first argument has space for adding
 * the instant, the  function returns the sequence set. Otherwise, a NEW
 * sequence set is returned and the input sequence set is freed.
 * @note Always use the function to overwrite the existing sequence set as in:
 * @code
 * ss = tsequenceset_append_tinstant(ss, inst, ...);
 * @endcode
 */
TSequenceSet *
tsequenceset_append_tinstant(TSequenceSet *ss, const TInstant *inst,
  double maxdist, const Interval *maxt, bool expand)
{
  assert(ss); assert(inst);
  assert(ss->temptype == inst->temptype);
  /* Append the instant to the last sequence */
  TSequence *last = (TSequence *) TSEQUENCESET_SEQ_N(ss, ss->count - 1);
  Temporal *temp = tsequence_append_tinstant(last, inst, maxdist, maxt,
    expand);
  /* The result may be a single sequence or a sequence set with 2 sequences */
  TSequence *seq1 = NULL, *seq2 = NULL;
  TSequenceSet *ss1 = NULL;
  int count;
  if (temp->subtype == TSEQUENCE)
  {
    seq1 = (TSequence *) temp;
    count = ss->count;
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    ss1 = (TSequenceSet *) temp;
    seq1 = (TSequence *) TSEQUENCESET_SEQ_N(ss1, 0);
    seq2 = (TSequence *) TSEQUENCESET_SEQ_N(ss1, 1);
    count = ss->count + 1;
  }

  /* Account for expandable structures
   * A while is used instead of an if to enable to break the loop if there is
   * no more available space */
  while (expand && count <= ss->maxcount)
  {
    /* Append the new sequence(s) if there is enough space: only one if the
     * instant was appended to the last sequence, or the two sequences
     * composing the sequence set that results from appending the instant */
    size_t size_last = DOUBLE_PAD(VARSIZE(last));
    size_t size_seq1 = DOUBLE_PAD(VARSIZE(seq1));
    size_t size = size_seq1;
    if (temp->subtype == TSEQUENCESET)
      size += DOUBLE_PAD(VARSIZE(seq2));
    /* Remove the size of the current last sequence */
    size_t avail_size = ((char *) ss + VARSIZE(ss)) -
      ((char *) last + size_last);
    if (size > avail_size)
      break;

    /* There is enough space to add the new sequence(s) */
    /* Copy the new sequence if its address is different from last */
    if (last != seq1)
      memcpy(last, seq1, VARSIZE(seq1));
    /* Update the offsets array and the counts when adding two sequences */
    if (temp->subtype == TSEQUENCESET)
    {
      (TSEQUENCESET_OFFSETS_PTR(ss))[count - 1] =
        (TSEQUENCESET_OFFSETS_PTR(ss))[count - 2] + size_seq1;
      ss->count++;
      ss->totalcount++;
      last = (TSequence *) ((char *) last + size_seq1);
      memcpy(last, seq2, VARSIZE(seq2));
    }
    /* Expand the bounding box and return */
    tsequenceset_expand_bbox(ss, (TSequence *) seq1);
    if (temp->subtype == TSEQUENCESET)
      tsequenceset_expand_bbox(ss, seq2);
    return ss;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count + 1));
  int nseqs = 0;
  for (int i = 0; i < ss->count - 1; i++)
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
  assert(temp->subtype == TSEQUENCE || temp->subtype == TSEQUENCESET);
  if (temp->subtype == TSEQUENCE)
    sequences[nseqs++] = (TSequence *) temp;
  else /* temp->subtype == TSEQUENCESET */
  {
    ss1 = (TSequenceSet *) temp;
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss1, 0);
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss1, 1);
  }
  TSequenceSet *result = tsequenceset_make(sequences, nseqs, NORMALIZE_NO);
  pfree(sequences);
#if MEOS
  if (expand)
    pfree(ss);
#endif /* MEOS */
  if ((void *) last != (void *) temp)
    pfree(temp);
  return result;
}

/**
 * @ingroup meos_internal_temporal_modif
 * @brief Append a sequence to a temporal sequence set
 * @param[in,out] ss Temporal sequence set
 * @param[in] seq Temporal sequence
 * @param[in] expand True when reserving space for additional sequences
 * @csqlfn #Temporal_append_tsequence()
 */
TSequenceSet *
tsequenceset_append_tsequence(TSequenceSet *ss, const TSequence *seq,
  bool expand)
{
  assert(ss); assert(seq);
  assert(ss->temptype == seq->temptype);

  /* The last sequence below may be modified with expandable structures */
  TSequence *last = (TSequence *) TSEQUENCESET_SEQ_N(ss, ss->count - 1);
  const TInstant *inst1 = TSEQUENCE_INST_N(last, last->count - 1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, 0);
  if (! ensure_tinstant_increasing_time(inst1, inst2))
    return NULL;
  else if (inst1->t == inst2->t && ss->period.upper_inc &&
    seq->period.lower_inc)
  {
    meosType basetype = temptype_basetype(ss->temptype);
    if (! ensure_tinstant_same_value(inst1, inst2, basetype))
      return NULL;
  }

  bool removelast, removefirst;
  bool join = tsequence_join_test(last, seq, &removelast, &removefirst);
  /* We are sure that the result will be a SINGLE sequence */
  TSequence *newseq = join ?
    (TSequence *) tsequence_append_tsequence(last, seq, expand) : NULL;
  int count = join ? ss->count : ss->count + 1;

  /* Account for expandable structures
   * A while is used instead of an if to be able to break the loop if there is
   * not enough available space to append the new sequence */
  while (expand && count <= ss->maxcount)
  {
    /* Determine whether there is enough available space */
    size_t size_last = DOUBLE_PAD(VARSIZE(last));
    size_t size_seq = DOUBLE_PAD(VARSIZE(seq));
    size_t size = join ? DOUBLE_PAD(VARSIZE(newseq)) - size_last : size_seq;
    /* Remove the size of the current last sequence */
    size_t avail_size = ((char *) ss + VARSIZE(ss)) -
      ((char *) last + size_last);
    if (size > avail_size)
      /* There is not enough available space */
      break;

    /* There is enough space to add the new sequence */
    if (join)
    {
      /* Set to 0 in case the new sequence is smaller than the current one */
      memset(last, 0, VARSIZE(last));
      memcpy(last, newseq, VARSIZE(newseq));
      pfree(newseq);
    }
    else
    {
      /* Update the offsets array and the counts when adding one sequence */
      (TSEQUENCESET_OFFSETS_PTR(ss))[count - 1] =
        (TSEQUENCESET_OFFSETS_PTR(ss))[count - 2] + size_last;
      ss->count++;
      ss->totalcount += seq->count;
      /* Copy the sequence at the end */
      TSequence *new = (TSequence *) ((char *) last + size_last);
      memcpy(new, seq, VARSIZE(seq));
    }
    /* Expand the bounding box and return */
    tsequenceset_expand_bbox(ss, seq);
    return ss;
  }

  /* This is the first time we use an expandable structure or there is not
   * enough available space */
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = 0;
  for (int i = 0; i < ss->count - 1; i++)
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
  if (join)
    sequences[nseqs++] = newseq;
  else
  {
    sequences[nseqs++] = (TSequence *) TSEQUENCESET_SEQ_N(ss, ss->count - 1);
    sequences[nseqs++] = (TSequence *) seq;
  }
  int maxcount;
  if (expand)
  {
    maxcount = ss->maxcount;
    if (count > ss->maxcount)
    {
      maxcount *= 2;
#if DEBUG_EXPAND
      meos_error(WARNING, " Sequence set -> %d ", maxcount);
#endif
    }
  }
  else
    maxcount = count;
  TSequenceSet *result = tsequenceset_make_exp(sequences, count, maxcount,
    NORMALIZE_NO);
  pfree(sequences);
  if (newseq)
    pfree(newseq);
#if MEOS
  if (expand)
    pfree(ss);
#endif /* MEOS */
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_modif
 * @brief Append an instant to a temporal value
 * @param[in,out] temp Temporal value
 * @param[in] inst Temporal instant
 * @param[in] interp Interpolation
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap, may be `NULL`
 * @param[in] expand True when reserving space for additional instants
 * @csqlfn #Temporal_append_tinstant()
 * @return When the temporal value passed as first argument has space for
 * adding the instant, the function returns the temporal value. Otherwise,
 * a NEW temporal value is returned and the input value is freed.
 * @note Always use the function to overwrite the existing temporal value as in:
 * @code
 * temp = temporal_append_tinstant(temp, inst, ...);
 * @endcode
 */
Temporal *
temporal_append_tinstant(Temporal *temp, const TInstant *inst,
  interpType interp, double maxdist, const Interval *maxt, bool expand)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp, (Temporal *) inst) ||
      ! ensure_spatial_validity(temp, (const Temporal *) inst) ||
      ! ensure_temporal_isof_subtype((Temporal *) inst, TINSTANT))
    return NULL;

  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      TSequence *seq = tinstant_to_tsequence((const TInstant *) temp, interp);
      Temporal *result = (Temporal *) tsequence_append_tinstant(seq, inst,
        maxdist, maxt, expand);
      pfree(seq);
      return result;
    }
    case TSEQUENCE:
      return (Temporal *) tsequence_append_tinstant((TSequence *) temp,
        inst, maxdist, maxt, expand);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_append_tinstant((TSequenceSet *) temp,
        inst, maxdist, maxt, expand);
  }
}

/**
 * @ingroup meos_temporal_modif
 * @brief Append a sequence to a temporal value
 * @param[in,out] temp Temporal value
 * @param[in] seq Temporal sequence
 * @param[in] expand True when reserving space for additional sequences
 * @csqlfn #Temporal_append_tsequence()
 */
Temporal *
temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp, (Temporal *) seq) ||
      (temp->subtype != TINSTANT && ! ensure_same_interp(temp, (Temporal *) seq)) ||
      ! ensure_spatial_validity(temp, (Temporal *) seq) ||
      ! ensure_temporal_isof_subtype((Temporal *) seq, TSEQUENCE))
    return NULL;

  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */

  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq->flags);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      TSequence *seq1 = tinstant_to_tsequence((const TInstant *) temp, interp2);
      Temporal *result = (Temporal *) tsequence_append_tsequence(
        (const TSequence *) seq1, seq, expand);
      pfree(seq1);
      return result;
    }
    case TSEQUENCE:
      return (Temporal *) tsequence_append_tsequence((const TSequence *) temp, seq,
        expand);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_append_tsequence((TSequenceSet *) temp,
        seq, expand);
  }
}

/*****************************************************************************/
